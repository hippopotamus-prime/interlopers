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
#include <PalmNavigator.h>
#include <SonyCLIE.h>
#include <HsCommon.h>
#include "palm.h"
#include "handlers.h"
#include "rc.h"
#include "game.h"
#include "gfx.h"
#include "prefs.h"
#include "sound.h"

static void		eventhandler(void);
static UInt16	startapp(void);
static void		stopapp(void);
static Boolean	preprocess(EventPtr eventptr);
static Boolean	appdoevent(EventPtr eventptr);
static Int32	timeleft(void);
static UInt16	initdevice(void);
static void		initgraf(void);
static Boolean	dopen(Coord x, Coord y);

static type_time time;

UInt32 keyvals[NKEYS] =
	{keyBitHard1, keyBitHard2, keyBitHard3, keyBitHard4, keyBitPageUp,
	keyBitPageDown, keyBitNavLeft, keyBitNavRight, keyBitNavSelect, 0, 0, 0};

static UInt16 keychars[NKEYS] =
	{vchrHard1, vchrHard2, vchrHard3, vchrHard4, vchrPageUp,
	vchrPageDown, vchrNavChange, vchrNavChange, vchrNavChange, 0, 0, 0};

static UInt16 keycodes[NKEYS] =
	{0, 0, 0, 0, 0, 0, navBitLeft, navBitRight, navBitSelect, 0, 0, 0};

type_deviceinfo	device;
UInt16			globalerr;


UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
	UInt32 err = 0;			//can't use globals unless it's a Normal Launch

	if(cmd == sysAppLaunchCmdNormalLaunch)
	{
		globalerr = startapp();
		if(!globalerr)
		{
			eventhandler();
		}
		stopapp();
		if(globalerr)
		{
			switch(globalerr)
			{
				case error_displaydepth:
				case sysErrRomIncompatible:
					FrmAlert(alert_incompat);
					break;
				default:
					FrmAlert(alert_error);
			}
		}
		err = globalerr;
	}

	return err;
}


UInt16 startapp(void)
{
	UInt16 err;
	UInt16 gvsize = 0;

	time.interval = SysTicksPerSecond()/FPS;
	gv.gamestate = state_none;	

	err = initdevice();
	if(err) return err;
	setgfxfunctions();

	getprefs();
	setkeymask(0);
	initgraf();

	err = loadgfx();
	if(err) return err;

	loadinfo();
	loadsounds();
	switch(PrefGetAppPreferences(CREATOR, 0, NULL, &gvsize, false))
	{
		default:					//unsupported versions...
		case noPreferenceFound:
			FrmGotoForm(form_title);
			break;
		case 0:
		case 1:
		case UPREFVERSION:
			FrmGotoForm(form_restoring);
	}

	return 0;
}


void stopapp(void)
{
	if((gv.gamestate & INPLAYMASK) >= state_play)
	{
		storegamevars();
	}

	setkeymask(keyBitsAll);
	EvtEnableGraffiti(true);

	if(prefs.checksum == getprefschecksum())
	{
		PrefSetAppPreferences(CREATOR, 0, SPREFVERSION, &prefs,
			sizeof(type_prefs), true);
	}

	freegfx();
	freeinfo();
	freesounds();

	FrmCloseAllForms();
}


void eventhandler(void)
{
	EventType event;

	do
	{
		EvtGetEvent(&event, timeleft());

		if(!preprocess(&event))
		{
			if(!SysHandleEvent(&event))
			{
				if(!MenuHandleEvent(NULL, &event, &globalerr))
				{
					if(!appdoevent(&event))
					{
						FrmDispatchEvent(&event);
					}
				}
			}
		}
	}while((event.eType != appStopEvent) && (!globalerr));
}


Boolean preprocess(EventPtr eventptr)
{
	Boolean handled = false;

	if((eventptr->eType == winExitEvent)
	&& ((eventptr->data.winExit.exitWindow == FrmGetWindowHandle(FrmGetFormPtr(form_main)))
	|| (eventptr->data.winExit.exitWindow == FrmGetWindowHandle(FrmGetFormPtr(form_title)))))
	{
		//this means some window has come up over top the main one
		gv.gamestate |= state_pause;
		EvtEnableGraffiti(true);
	}

	else if((eventptr->eType == winEnterEvent)
	&& (eventptr->data.winEnter.enterWindow == FrmGetWindowHandle(FrmGetFirstForm())))
	{
		//this means everything is returning to normal...
		if(eventptr->data.winEnter.enterWindow == FrmGetWindowHandle(FrmGetFormPtr(form_main)))
		{
			gv.gamestate &= ~state_pause;
			EvtEnableGraffiti(!isgcon());
		}
		else if(eventptr->data.winEnter.enterWindow == FrmGetWindowHandle(FrmGetFormPtr(form_title)))
		{
			gv.gamestate &= ~state_pause;
			EvtEnableGraffiti(true);
		}
	}

	else if(((eventptr->eType == penDownEvent) || (eventptr->eType == penMoveEvent))
	&& (acceptspen(gv.gamestate)))
	{
		handled = dopen(eventptr->screenX, eventptr->screenY);
	}

	else if((eventptr->eType == nilEvent)
	&& (acceptspen(gv.gamestate)))
	{
		//OS 5 devices can skip penMoveEvent if it's time for a nilEvent
		Int16 x, y;
		Boolean pendown;

		EvtGetPen(&x, &y, &pendown);

		if(pendown)
		{
			handled = dopen(x, y);
		}
		//penUpEvent is sometimes missed too
		else if(prefs.settings.pencontrol == pc_fire)
		{
			gv.pentarget = NOTARGET;
		}
	}

	else if((eventptr->eType == keyDownEvent)
	&& (!(eventptr->data.keyDown.modifiers & poweredOnKeyMask)))
	{
		Boolean pause = false;
		Int8 i;

		for(i = gettopkey(); i >= 0; i--)
		{
			if((eventptr->data.keyDown.chr == keychars[i])
			&& (!(eventptr->data.keyDown.modifiers & autoRepeatKeyMask))
			&& (prefs.settings.keyactions[i] == action_pause)
			&& ((!keycodes[i]) || (eventptr->data.keyDown.keyCode & keycodes[i])))
				//5-way nav keys have the same char, so the keyCode differentiates them
			{
				pause = true;
			}
		}

		if(pause)
		{
			FormPtr formptr = FrmGetActiveForm();

			if(FrmGetFormId(formptr) == form_pause)
			{
				CtlHitControl(FrmGetObjectPtr(formptr,
					FrmGetObjectIndex(formptr, button_pauseresume)));
			}
			else if((gv.gamestate >= state_play)
			&& (gv.gamestate <= INPLAYMASK ))
			{
				FrmPopupForm(form_pause);
			}

			handled = true;
		}
		else if((eventptr->data.keyDown.chr >= vchrJogUp)
		&& (eventptr->data.keyDown.chr <= vchrJogRelease)
		&& (gv.gamestate == state_play))
		{
			switch(eventptr->data.keyDown.chr)
			{
				case vchrJogPush:
					gv.jog = JOGFIRE;
					break;
				case vchrJogPushedDown:
				case vchrJogDown:
					gv.jog = 2;
					break;
				case vchrJogPushedUp:
				case vchrJogUp:
					gv.jog = -2;
			}
		}
		else if(TxtCharIsHardKey(eventptr->data.keyDown.modifiers, eventptr->data.keyDown.chr))
		{
			//maybe it should only eat events for keys in use?  *shrug*

			if((eventptr->data.keyDown.chr >= vchrHard1)
			&& (eventptr->data.keyDown.chr <= vchrHard4))
			{
				handled = true;
			}
		}
	}

	if(!time.soundwait)
	{
		dosound();
	}
	if(!time.gamewait)
	{
		dogame();
	}

	return handled;
}


Boolean appdoevent(EventPtr eventptr)
{
	Boolean handled = false;

	if(eventptr->eType == frmLoadEvent)
	{
		FormPtr formptr = FrmInitForm(eventptr->data.frmLoad.formID);
		FrmSetActiveForm(formptr);
		switch(eventptr->data.frmLoad.formID)
		{
			case form_main:
				FrmSetEventHandler(formptr, mainhandler);
				break;
			case form_title:
				FrmSetEventHandler(formptr, titlehandler);
				break;
			case form_keys:
				FrmSetEventHandler(formptr, keyshandler);
				break;
			case form_settings:
				FrmSetEventHandler(formptr, settingshandler);
				break;
			case form_restoring:
				FrmSetEventHandler(formptr, restorehandler);
				break;
			case form_wingame:
			case form_die:
				FrmSetEventHandler(formptr, diehandler);
				break;
			case form_wingamehs:
			case form_diehs:
				FrmSetEventHandler(formptr, diehshandler);
				break;
			case form_highscores:
				FrmSetEventHandler(formptr, hiscoreshandler);
				break;
			case form_shop:
				FrmSetEventHandler(formptr, shophandler);
				break;
			case form_pause:
				FrmSetEventHandler(formptr, pausehandler);
				break;
			case form_score:
				FrmSetEventHandler(formptr, scorehandler);
				break;
			case form_register:
				FrmSetEventHandler(formptr, reghandler);
				break;
			case form_records:
				FrmSetEventHandler(formptr, recordshandler);
				break;
			default:
				FrmSetEventHandler(formptr, mischandler);
		}

		handled = true;
	}

	return handled;
}


UInt16 initdevice(void)
{
	UInt16 refnum;
	UInt32 featureval;
	UInt32 width;
	UInt32 height;
	Err err;

	//check rom version
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &featureval);
	if(featureval < MINROMVERSION)
	{
		if(featureval < ROMVERSION2)
		{
			//apparently rom 1.0 was pretty lame...
			AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
		}
		return sysErrRomIncompatible;
	}

	//check for the 5-way navigator deal
	if(!FtrGet(navFtrCreator, navFtrVersion, &featureval))
	{
		device.flags |= df_5way;
	}
	else if((!FtrGet(sysFileCSystem, sysFtrNumUIHardwareFlags, &featureval))
	&& (featureval & sysFtrNumUIHardwareHas5Way))
	{
		//Handspring's alternate 5-way API.
		//Someone deserves a serious ass-kicking for creating YET ANOTHER ALTERNATE API
		//(even if theirs is better...)
		device.flags |= df_5way;

		//up/down should use pageup/pagedown vchars since the main form is non-modal
		//hopefully the keyBit- values work similarly...?
		keyvals[6] = keyBitRockerLeft;
		keyvals[7] = keyBitRockerRight;
		keyvals[8] = keyBitRockerCenter;
		keychars[6] = vchrRockerLeft;
		keychars[7] = vchrRockerRight;
		keychars[8] = vchrRockerCenter; 
		keycodes[6] = 0;
		keycodes[7] = 0;
		keycodes[8] = 0;
	}

	//check and set screen depth
	err = setdisplaydepth();
	if(err) return err;

	//check display resolution
	if((!FtrGet(sysFtrCreator, sysFtrNumWinVersion, &featureval))
	&& (featureval >= 4))
	{
		//Palm hi-res API is present
		WinScreenGetAttribute(winScreenWidth, &width);
		WinScreenGetAttribute(winScreenHeight, &height);
	}
	else if((!FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, &featureval))
	&& (((SonySysFtrSysInfoP)featureval)->libr & sonySysFtrSysInfoLibrHR)
	&& ((!SysLibFind(sonySysLibNameHR, &refnum)) || (!SysLibLoad('libr', sonySysFileCHRLib, &refnum)))
	&& (!HROpen(refnum)))
	{
		//Sony's hi-res library

		//this will disable hi-res assist
		width = 160;
		height = 160;
		HRWinScreenMode(refnum, winScreenModeSet, &width, &height, NULL, NULL);
		//check to see if it worked...
		HRWinScreenMode(refnum, winScreenModeGet, &width, &height, NULL, NULL);

		HRClose(refnum);
	}
	else
	{
		WinScreenMode(winScreenModeGetDefaults, &width, &height, NULL, NULL);
	}

	if((width == 160) && (height >= 160))
	{
		device.flags |= df_lores;
	}


	//check processor
	FtrGet(sysFtrCreator, sysFtrNumProcessorID, &featureval);
	if((featureval == sysFtrNumProcessor328)
	|| (featureval == sysFtrNumProcessorEZ)
	|| (featureval == sysFtrNumProcessorVZ)
	|| (featureval == sysFtrNumProcessorSuperVZ))
	{
		device.flags |= df_68k;
	}

	return 0;
}


Int32 timeleft(void)
{
	Int32 minwait;
	Int32 now = TimGetTicks();

	if((gv.gamestate == state_none)
	|| (gv.gamestate > INPLAYMASK))
	{
		time.gamewait = evtWaitForever;
	}
	else
	{
		time.gamewait = time.gamenext - now;

		if(time.gamewait <= 0)
		{
			time.gamewait = 0;
			time.gamenext = now + time.interval;
		}
	}

	if(gv.cursound == NOSOUND)
	{
		time.soundwait = evtWaitForever;
	}
	else
	{
		time.soundwait = time.soundnext - now;

		if(time.soundwait <= 0)
		{
			time.soundwait = 0;
			time.soundnext = now + getnextbeeptime();
		}
	}

	minwait = time.gamewait;

	if((time.soundwait != evtWaitForever)
	&& ((time.soundwait < minwait) || (minwait == evtWaitForever)))
	{
		minwait = time.soundwait;
	}

	return minwait;
}


/*********************************************
** Turn off OS events for keys that are in
** use...
**********************************************/
void setkeymask(const UInt32 forcekeys)
{
	if(prefs.settings.flags & set_kevents)
	{
		KeySetMask(keyBitsAll);
	}
	else
	{
		Int8	i;
		UInt32	usedkeymask = 0;
	
		for(i = gettopkey(); i >= 0; i--)
		{
			if((prefs.settings.keyactions[i] != action_none)
			&& (prefs.settings.keyactions[i] != action_pause))
			{
				usedkeymask |= keyvals[i];
			}
		}

		KeySetMask(forcekeys | ~usedkeymask);
	}
}


void initgraf(void)
{
	UInt16 i;

	device.grinfo.right	= NOGRAF;
	device.grinfo.left	= NOGRAF;

	device.grinfo.penbtnlist = EvtGetPenBtnList(&device.grinfo.npenbtns);

	for(i = 0; i < device.grinfo.npenbtns; i++)
	{
		if(device.grinfo.penbtnlist[i].asciiCode == GRAFLEFTBUTTON)
		{
			device.grinfo.left = device.grinfo.penbtnlist[i].boundsR.topLeft.x +
				device.grinfo.penbtnlist[i].boundsR.extent.x;
		}
		else if(device.grinfo.penbtnlist[i].asciiCode == GRAFRIGHTBUTTON)
		{
			device.grinfo.right = device.grinfo.penbtnlist[i].boundsR.topLeft.x;
		}
	}
}


Boolean dopen(Coord x, Coord y)
{
	Coord extent_x, extent_y;
	Boolean eatpen		= false;
	Boolean flushpen	= false;

	WinGetDisplayExtent(&extent_x, &extent_y);

	if((y >= extent_y)
	&& (isgcon()))
	{
		UInt16 i;

		eatpen		= true;
		flushpen	= true;

		for(i = 0; i < device.grinfo.npenbtns; i++)
		{
			if(RctPtInRectangle(x, y,
			&device.grinfo.penbtnlist[i].boundsR))
			{
				flushpen = false;
			}
		}

		if(flushpen)
		{
			Coord newx = (x - device.grinfo.left) * extent_x /
				(device.grinfo.right - device.grinfo.left);

			gv.pentarget = newx - PLAYERWIDTH/2 + 16;
		}
	}
	else if((y < extent_y)
	&& (y >= STATBARHEIGHT))
	{
		flushpen	= true;

		switch(prefs.settings.pencontrol)
		{
			case pc_pause:
				FrmPopupForm(form_pause);
				break;

			case pc_fire:
				gv.pentarget = FIRETARGET;
				break;

			case pc_movement:
				gv.pentarget = x - PLAYERWIDTH/2 + 16;
		}
	}
	else if(y < STATBARHEIGHT)
	{
		EventType event;

		event.eType						= keyDownEvent;
		event.data.keyDown.chr			= vchrMenu;
		event.data.keyDown.modifiers	= commandKeyMask;
		EvtAddEventToQueue(&event);

		flushpen = true;
	}

	if(((gv.counter & 3) == 2)
	&& (flushpen))
	{
		EvtFlushPenQueue();
	}

	return eatpen;
}