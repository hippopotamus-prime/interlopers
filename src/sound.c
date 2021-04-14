#include <PalmOS.h>
#include "sound.h"
#include "game.h"
#include "rc.h"
#include "prefs.h"


static Int16	soundamp;
static noise*	soundlist[NSOUNDS];


void dosound(void)
{
	if(gv.cursound != NOSOUND)
	{
		Int16 amp = soundamp + soundlist[gv.cursound]->beeps[gv.curbeep].relamp;

		if(amp < 0)
		{
			amp = 0;
		}
		else if(amp > sndMaxAmp)
		{
			amp = sndMaxAmp;
		}

		if((amp)
		&& (soundlist[gv.cursound]->beeps[gv.curbeep].freq)
		&& (soundlist[gv.cursound]->beeps[gv.curbeep].duration))
		{
			SndCommandType command;

			command.cmd		= sndCmdFrqOn;
			command.param1	= soundlist[gv.cursound]->beeps[gv.curbeep].freq;
			command.param2	= soundlist[gv.cursound]->beeps[gv.curbeep].duration;
			command.param3	= amp;

			SndDoCmd(NULL, &command, 0);
		}

		if((++gv.curbeep) >= soundlist[gv.cursound]->nbeeps)
		{
			gv.cursound = NOSOUND;
		}
	}
}


Int32 getnextbeeptime(void)
{
	Int32 beeptime = evtWaitForever;

	if(gv.cursound != NOSOUND)
	{
		beeptime = SysTicksPerSecond() *
			soundlist[gv.cursound]->beeps[gv.curbeep].duration / 1000;
	}

	return beeptime;
}


void setsound(const UInt8 index)
{
	if((index == NOSOUND) || (!soundamp))
	{
		gv.cursound = NOSOUND;
	}
	else if((gv.cursound == NOSOUND)
	|| (soundlist[index]->priority >= soundlist[gv.cursound]->priority))
	{
		gv.cursound = index;
		gv.curbeep = 0;
	}
}


void setvolume(void)
{
	if(prefs.settings.volume != vol_sys)
	{
		const UInt8 amps[4] = {0, sndMaxAmp/4, sndMaxAmp/2, sndMaxAmp};

		soundamp = amps[prefs.settings.volume-1];
	}
	else
	{
		soundamp = PrefGetPreference(prefGameSoundVolume);
	}
}


void loadsounds(void)
{
	UInt8 i;
	MemHandle beepshand;

	for(i = 0; i < NSOUNDS; i++)
	{
		beepshand = DmGetResource(soundrc, sound_start+i);
		soundlist[i] = (noise*)MemHandleLock(beepshand);
	}

	setvolume();
	setsound(NOSOUND);
}


void freesounds(void)
{
	UInt8 i;
	MemHandle beepshand;

	for(i = 0; i < NSOUNDS; i++)
	{
		if(soundlist[i])		//in case startapp failed
		{
			beepshand = MemPtrRecoverHandle(soundlist[i]);
			MemHandleUnlock(beepshand);
			DmReleaseResource(beepshand);
		}
	}
}