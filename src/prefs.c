/****************************************************************************
** Interlopers
** Copyright 2004 Aaron Curtis
** 
** This file is part of Interlopers.
** 
** Interlopers is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** Interlopers is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with Interlopers; if not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#include <PalmOS.h>
#include <DLServer.h>
#include "prefs.h"
#include "game.h"
#include "palm.h"
#include "sound.h"


type_prefs prefs;


void getprefs(void)
{
	UInt16 prefsize = sizeof(type_prefs);
	UInt16 prefversion;

	prefversion = PrefGetAppPreferences(CREATOR, 0, &prefs, &prefsize, true);

	if((prefversion == noPreferenceFound)
	|| (prefs.checksum != getprefschecksum()))
	{
		clear_records();
		clearhighscores();
		setdefaultkeys(prefs.settings.keyactions);

		prefs.settings.lastkey		= 0;
		prefs.settings.flags		= 0;
		prefs.settings.pencontrol	= pc_movement;
		prefs.settings.volume		= vol_sys;
		prefs.settings.difficulty	= diff_normal;
		prefs.settings.hsdifficulty	= diff_normal;

		prefs.regcode = 0;
	}
	else if(prefversion == 0)
	{
		//version 0 had no registration system
		prefs.regcode = 0;
	}

	if((prefs.settings.lastkey > 5)
	&& (!(device.flags & df_5way)))
	{
		//the app could be moved to a new device without 5-way support
		prefs.settings.lastkey = 0;
	}

	if((prefs.regcode)
	&& (!checkregcode(prefs.regcode)))
	{
		//it shouldn't be possible to get 0 as a valid code for any hotsync id
		prefs.regcode = 0;
	}

	prefs.checksum = getprefschecksum();
}


void clear_records(void)
{
	UInt8 i, j;

	for(j = 0; j < NDIFFS; j++)
	{
		for(i = 0; i < NWAVES; i++)
		{
			prefs.gear[j][i].weapon		= weapon_wimpy;
			prefs.gear[j][i].special	= special_randtel;
		}

#ifdef CHEAT
		prefs.settings.topwave[j]	= NWAVES - 1;
#else
		prefs.settings.topwave[j]	= 0;
#endif

		prefs.settings.startwave[j]	= 0;
	}
}



void clearhighscores(void)
{
	UInt8 i, n;

	for(n = 0; n < NDIFFS; n++)
	{
		for(i = 0; i < NHISCORES; i++)
		{
			prefs.highscores[n][i].score = 100*(20+n-i);
			prefs.highscores[n][i].topwave = 9-i;
			StrCopy(prefs.highscores[n][i].name, "------------");
		}
	}
}


void setdefaultkeys(UInt8* keyactions)
{
	keyactions[0] = action_left;	//date book
	keyactions[1] = action_right;	//address
	keyactions[2] = action_fire;	//to do list
	keyactions[3] = action_special;	//memo pad
	keyactions[4] = action_fire;	//up
	keyactions[5] = action_pause;	//down
	keyactions[6] = action_left;	//nav-left
	keyactions[7] = action_right;	//nav-right
	keyactions[8] = action_none;	//nav-select
	keyactions[9] = action_none;	//unused
}


UInt32 getchecksum(UInt8* ptr, const UInt16 size)
{
	UInt16 i;
	UInt32 checksum = 0;	

	//assume the first 4 bytes are the checksum itself
	for(i = 4; i < size; i++)
	{
		checksum += ptr[i];
	}

	return checksum;
}


UInt16 loadgamevars(void)
{
	UInt16 gvsize	= sizeof(type_gamevars);
	UInt16 version	= PrefGetAppPreferences(CREATOR, 0, &gv, &gvsize, false);

	//wipe the old game vars
	PrefSetAppPreferences(CREATOR, 0, UPREFVERSION, NULL, 0, false);

	gv.gamestate |= state_restore;
	return version;
}


void storegamevars(void)
{
	gv.gamestate &= INPLAYMASK;

	PrefSetAppPreferences(CREATOR, 0, UPREFVERSION,
		&gv, sizeof(type_gamevars), false);
}


/***********************************************************
** Check a given score against the the current high
** scores and return a position in the high score table
** (or NOHIGHSCORE)
************************************************************/
UInt8 checkhighscore(UInt32 score)
{
	UInt8	i;
	UInt8	hsindex	= NOHISCORE;

	for(i = 0; i < NHISCORES; i++)
	{
		if(score > prefs.highscores[gv.difficulty][i].score)
		{
			hsindex = i;
			break;
		}
	}

	return hsindex;
}


/************************************************************
** Insert a high score / name combo into a high score list,
** shifting the old scores as needed.
*************************************************************/
void inserthighscore(UInt8 hsindex, type_highscore* scoreptr)
{
	Int8 i;

	if(hsindex != NOHISCORE)
	{
		for(i = NHISCORES-2; i >= hsindex; i--)
		{
			MemMove(&prefs.highscores[gv.difficulty][i+1],
				&prefs.highscores[gv.difficulty][i],
				sizeof(type_highscore));
		}

		MemMove(&prefs.highscores[gv.difficulty][hsindex],
			scoreptr, sizeof(type_highscore));
	}
}


Boolean checkregcode(UInt16 testcode)
{
	Boolean accept = false;
	char hotsyncid[dlkUserNameBufSize];

	if(DlkGetSyncInfo(NULL, NULL, NULL, hotsyncid, NULL, NULL) == errNone)
	{
		//rpn string for this:
		//i 0 == 72094 * key + c 57 * c 32 != * +

		char	newname[11];
		Int32	code = 72094;
		Int32	altcode = 72094;
		UInt8	i;
//char foo[20];
		//handango-ize the name...
		if(StrLen(hotsyncid) > 10)
		{
			StrNCopy(newname, hotsyncid, 5);
			StrCopy(&newname[5], &hotsyncid[StrLen(hotsyncid)-5]);
		}
		else
		{
			StrCopy(newname, hotsyncid);
		}

		//calculate the correct reg code (handango)
		for(i = 0; i < StrLen(newname); i++)
		{
			if(newname[i] != ' ')
			{
				code += newname[i]*57;
			}
		}

		//calculate the correct reg code (non-handango)
		for(i = 0; i < StrLen(hotsyncid); i++)
		{
			if(hotsyncid[i] != ' ')
			{
				altcode += hotsyncid[i]*57;
			}
		}	

		code &= 0xffff;
		altcode &= 0xffff;

		if((testcode == code)
		|| (testcode == altcode))
		{
			accept = true;
		}

//StrIToA(foo, code);
//DbgMessage(foo);
//StrIToA(foo, altcode);
//DbgMessage(foo);
//DbgMessage(hotsyncid);
	}

	return accept;
}


UInt32 genhscode(void)
{
	char hotsyncid[dlkUserNameBufSize];
	UInt32 finalcode = 0;

	if(DlkGetSyncInfo(NULL, NULL, NULL, hotsyncid, NULL, NULL) == errNone)
	{
		UInt8 i;
		char text[maxStrIToALen+dlkUserNameBufSize+MAXNAMELEN+1];
		UInt32 code1 = 86731;
		UInt8 code2 = prefs.highscores[prefs.settings.hsdifficulty][0].topwave;

		StrIToA(text, prefs.highscores[prefs.settings.hsdifficulty][0].score);
		StrCat(text, prefs.highscores[prefs.settings.hsdifficulty][0].name);
		StrCat(text, hotsyncid);

		for(i = 0; i < StrLen(text); i++)
		{
			if(text[i] != ' ')
			{
				//text[i] is an Int8 - but we don't want signed multiplication
				//because PHP's ord() function returns unsigned values
				code1 += ((UInt8)(text[i]))*193;

				//the decoder is a PHP script, but PHP doesn't allow ints to
				//overflow, so we have to fake it....
				code1 &= 0x3fffffff;
			}
		}

		code1 &= 0xffff;

		for(i = 0; i < 8; i++)
		{
			finalcode <<= 1;
			finalcode |= (code1 & 1);
			code1 >>= 1;
		}

		for(i = 0; i < 8; i++)
		{
			finalcode <<= 1;
			finalcode |= (code1 & 1);
			finalcode <<= 1;
			finalcode |= (code2 & 1);
			code1 >>= 1;
			code2 >>= 1;
		}

		switch(prefs.settings.hsdifficulty)
		{
			case diff_easy:
				finalcode ^= 0xf5a629;
				break;
			case diff_normal:
				finalcode ^= 0x3b81fe;
				break;
			case diff_hard:
				finalcode ^= 0x6890e4;
				break;
			default:
				finalcode ^= 0xcc747d;
		}
	}

	return finalcode;
}