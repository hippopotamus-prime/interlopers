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

#define NOSOUND 	0xff

//volume settings
#define vol_sys		0
#define vol_off		1
#define vol_low		2
#define vol_med		3
#define vol_high	4

typedef struct
{
	UInt16 freq;
	UInt16 duration;
	Int16 relamp;
}beep;

typedef struct
{
	UInt8 priority;
	UInt8 nbeeps;
	beep beeps[0];
}noise;

extern void		loadsounds(void);
extern void		freesounds(void);
extern void		setvolume(void);
extern Int32	getnextbeeptime(void);
extern void		dosound(void);
extern void		setsound(const UInt8 index);