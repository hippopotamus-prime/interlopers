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

#ifndef __PALM__
#define __PALM__

#include "gfx.h"

#define FPS			30
#define CREATOR		'INLO'
#define NKEYS		12			//the last 3 are reserved

#define ROMVERSION2 sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0)
#define MINROMVERSION sysMakeROMVersion(3, 5, 0, sysROMStageRelease, 0)

#define df_5way		1
#define df_68k		2
#define df_lores	4

#define GRAFRIGHTBUTTON	vchrFind
#define GRAFLEFTBUTTON	vchrMenu
#define NOGRAF			0x7fff

typedef struct
{
	Int32 interval;
	Int32 gamewait;
	Int32 gamenext;
	Int32 soundwait;
	Int32 soundnext;
}type_time;

typedef struct
{
	UInt16					npenbtns;
	const PenBtnInfoType*	penbtnlist;
	Coord					right;
	Coord					left;
}type_grafinfo;

typedef struct
{
	UInt16			flags;
	type_grafinfo	grinfo;
	void 			(*func_copybuffer)(void);
	void			(*func_erasebuffer)(void);
	void			(*func_drawsprite)(Coord, Coord, sprite*);
}type_deviceinfo;

extern UInt32			keyvals[NKEYS];
extern type_deviceinfo	device;
extern UInt16			globalerr;

extern void setkeymask(const UInt32 forcekeys);

#define isgcon()		((device.grinfo.right != NOGRAF) && (device.grinfo.left != NOGRAF) \
						&& (prefs.settings.flags & set_graf))

#define gettopkey()		((device.flags & df_5way) ? 8 : 5)

#endif