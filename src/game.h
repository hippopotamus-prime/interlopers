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

#ifndef __GAME__
#define __GAME__

#include "gfx.h"

#define state_none		0
#define state_title		1
#define state_play		2
#define state_nearwin	3
#define state_form		4
#define state_win		5
#define state_enter		6
#define state_lose		7
#define state_retire	8
#define state_shop		9
#define state_die		10
#define state_diehs		11
#define state_wingame	12
#define state_wingamehs	13
#define state_score		14
#define state_noreg		15
#define state_blowup0	16
#define state_blowup1	17
#define state_blowup2	18
#define state_pause		32
#define state_restore	64
#define state_regwait	128
#define INPLAYMASK		31

#define WAVEDISPLAYTIME	45

#define FRACBITS		5

#define ROWS			8
#define COLUMNS			8
#define NWAVES			50


#ifdef DEMO				//obsolete with the addition of the registration system
#define ENDWAVE			20
#else
#define ENDWAVE			NWAVES
#endif

#define LASTFREEWAVE	19			//keep in mind the waves start at 0 internally

#define REPLAY			255			//can be passed to initgame() instead of a wave number

#define NOALIEN			255

#define dir_right			0
#define dir_downr			1
#define dir_left			2
#define dir_downl			3
#define ALEFTEDGE			16
#define ARIGHTEDGE			(160+16)
#define ABOTTOMEDGE			BUFFERHEIGHT
#define ALIENSTARTX_LEFT	16
#define ALIENSTARTX_RIGHT	(160+16)
#define ALIENSTARTY			0

#define DEFAULTFIREVAL		10

#define BTMASK			63
#define MAXBULLETS		58
#define BRIGHTEDGE		(SCREENX+SCREENWIDTH)
#define BLEFTEDGE		(SCREENX-BULLETWIDTH)
#define BTOPEDGE		(SCREENY-BULLETHEIGHT)
#define BBOTTOMEDGE		(SCREENY+SCREENHEIGHT-BULLETHEIGHT)

#define MAXEFFECTS		10
#define ALIENEFFECT		0
#define PLAYEREFFECT	1
#define effect_explode	0
#define effect_teleport	1

#define PLAYERSTARTX	90

#define action_none		0
#define action_left		1
#define action_right	2
#define action_fire		3
#define action_special	4
#define action_pause	5

#define NOTARGET		0x7fff
#define FIRETARGET		0x7ffe
#define JOGFIRE			127
#define NOJOG			0

#define BLOCK_GROUPS 	3
#define BLOCK_COLUMNS	2
#define BLOCK_ROWS		3
#define BLOCK_TOP		112	
#define BLOCK_LEFT0		40
#define BLOCK_LEFT1		88
#define BLOCK_LEFT2		136
#define BLOCKWIDTH		8
#define BLOCKHEIGHT		8
#define NOBLOCK			-1

#define NWEAPONS		7
#define weapon_wimpy	0
#define weapon_cannon	1
#define weapon_auto		2
#define weapon_2cannon	3
#define weapon_ray		4
#define weapon_war		5
#define weapon_plasma	6

#define NSPECIALS			7
#define special_randtel		0
#define special_barrier		1
#define special_teleport	2
#define special_shield		3
#define special_seeker		4
#define special_repel		5
#define special_bomb		6

#define NOPTARGET 		0xff		//used for guided missiles, no relation to NOTARGET

typedef struct
{
	UInt8 width;
	UInt8 height;
	UInt8 corner;
	UInt8 startframe;
	UInt8 nframes;
	UInt8 speed;
	UInt8 damage;
	UInt8 reserved1;
}type_bulletinfo;

typedef struct
{
	UInt8	width;
	UInt8	height;
	UInt8	corner;
	UInt8	hp;
	UInt8	fireval;
	UInt8	bullet;
	UInt8	sound;
	UInt8	reserved1;
}type_alieninfo;

typedef struct
{
	UInt8	width;
	UInt8	height;
	UInt8	graphic;
	UInt8	startframe;
	UInt8	nframes;
	UInt8	reserved1;
}type_effectinfo;

typedef struct
{
	UInt16	cost;
	UInt8	bullet;
	UInt8	timer;
	UInt8	bcount;
	UInt8	sound;
	UInt8	reserved1;
	UInt8	reserved2;
}type_weaponinfo;

typedef struct
{
	UInt16	cost;
	UInt8	timer;
	UInt8	drain;
	UInt8	reserved1;
	UInt8	reserved2;
}type_specialinfo;

typedef struct
{
	type_bulletinfo*	bullets;
	type_alieninfo*		aliens;
	type_effectinfo*	effects;
	type_weaponinfo*	weapons;
	type_specialinfo*	specials;
}type_info;

typedef struct
{
	UInt8 type;
	UInt8 hp;
}alien;

typedef struct
{
	Int16 x;
	Int16 y;
	Int8 dx;
	Int8 dy;
	UInt8 type;
	UInt8 frame;
}bullet;

typedef struct
{
	Int16	x;
	Int16	y;
	UInt8	type;
	Int8	frame;
}effect;

typedef struct
{
	UInt32	money;
	UInt32	reserved1;
	UInt32	reserved2;
	Int16	x;
	UInt8	frame;
	UInt8	firetimer;
	UInt8	firedata;
	UInt8	specialtimer;
	UInt8	specialdata;
	UInt8	targeti;
	UInt8	targetj;
	Int8	shield;
	Int8	power;
	UInt8	weapon;
	UInt8	special;
	UInt8	reserved3;
	UInt8	reserved4;
	UInt8	reserved5;
	UInt8	reserved6;
	UInt8	reserved7;
}type_player;


typedef struct
{
	UInt32		counter;
	UInt32		wavedisplay;
	UInt32		reserved1;
	UInt32		reserved2;
	UInt32		reserved3;
	UInt32		reserved4;
	UInt16		gamestate;
	Int16		pentarget;
	alien		aliens[COLUMNS][ROWS];
	bullet		bullets[MAXBULLETS];
	effect		effects[MAXEFFECTS];
	Int8		blocks[BLOCK_GROUPS][BLOCK_COLUMNS][BLOCK_ROWS];
	Int16		alieny;
	Int16		alienx;
	Int16		alienstopy;
	UInt16		alienframecount;
	type_player	player;
	UInt16		penalties;
	UInt16		reserved6;
	UInt16		reserved7;
	UInt16		reserved8;
	Int8		arightbound;
	Int8		aleftbound;
	Int8		abottombound;
	UInt8		naliens;
	UInt8		aliendir;
	UInt8		nbullets;
	UInt8		neffects;
	UInt8		startwave;
	UInt8		wave;
	UInt8		difficulty;
	UInt8		cursound;
	UInt8		curbeep;
	Int8		jog;
	UInt8		reserved9;
	UInt8		reserved10;
	UInt8		reserved11;
	UInt8		reserved12;
	UInt8		reserved13;
}type_gamevars;


//old version - MAXBULLETS was lower
typedef struct
{
	UInt32		counter;
	UInt32		wavedisplay;
	UInt32		reserved1;
	UInt32		reserved2;
	UInt32		reserved3;
	UInt32		reserved4;
	UInt16		gamestate;
	Int16		pentarget;
	alien		aliens[COLUMNS][ROWS];
	bullet		bullets[50];
	effect		effects[MAXEFFECTS];
	Int8		blocks[BLOCK_GROUPS][BLOCK_COLUMNS][BLOCK_ROWS];
	Int16		alieny;
	Int16		alienx;
	Int16		alienstopy;
	UInt16		alienframecount;
	type_player	player;
	UInt16		penalties;
	UInt16		reserved6;
	UInt16		reserved7;
	UInt16		reserved8;
	Int8		arightbound;
	Int8		aleftbound;
	Int8		abottombound;
	UInt8		naliens;
	UInt8		aliendir;
	UInt8		nbullets;
	UInt8		neffects;
	UInt8		startwave;
	UInt8		wave;
	UInt8		difficulty;
	UInt8		cursound;
	UInt8		curbeep;
	Int8		jog;
	UInt8		reserved9;
	UInt8		reserved10;
	UInt8		reserved11;
	UInt8		reserved12;
	UInt8		reserved13;
}type_gamevars_v1;



typedef struct
{
	Int16 x;
	Int16 y;
	UInt8 width;
	UInt8 height;
	UInt8 corner;
	UInt8 data;
}rect;


extern void dogame(void);
extern void initgame(const UInt8 startwave);
extern void loadwave(void);
extern void removeeffect(UInt8 index);

extern void loadinfo(void);
extern void freeinfo(void);


#define isplayerbullet(type)		((type >= 128) && (type < 192))
#define makeplayerbullet(type)		(type | 128)
#define isalienbullet(type)			((type >= 64) && (type < 128))
#define makealienbullet(type)		(type | 64)
#define isbonus(type)				(type >= 192)
#define makebonus(type)				(type | 192)

#define getalienxoffset(type)		((ALIENWIDTH - info.aliens[type].width)>>1)
#define getalienyoffset(type)		((ALIENHEIGHT - info.aliens[type].height)>>1)
#define getalienwidth(type)			(info.aliens[type].width)
#define getalienheight(type)		(info.aliens[type].height)
#define getaliencorner(type)		(info.aliens[type].corner)
#define getalienfireval(type)		(info.aliens[type].fireval)
#define getalienhp(type)			(info.aliens[type].hp)
#define getalienbullet(type)		(info.aliens[type].bullet)
#define getaliensound(type)			(info.aliens[type].sound)

#define getbulletwidth(type)		(info.bullets[type&BTMASK].width)
#define getbulletheight(type)		(info.bullets[type&BTMASK].height)
#define getbulletcorner(type)		(info.bullets[type&BTMASK].corner)
#define getbulletstartframe(type)	(info.bullets[type&BTMASK].startframe)
#define getbulletframes(type)		(info.bullets[type&BTMASK].nframes)
#define getbulletspeed(type)		(info.bullets[type&BTMASK].speed)
#define getbulletdamage(type)		(info.bullets[type&BTMASK].damage)

#define getweaponcost(type)			(info.weapons[type].cost)
#define getweaponbullet(type)		(info.weapons[type].bullet)
#define getweapontimer(type)		(info.weapons[type].timer)
#define getweaponbcount(type)		(info.weapons[type].bcount)
#define getweaponsound(type)		(info.weapons[type].sound)
#define getweapontrade(type)		(gv.difficulty == diff_easy ? info.weapons[type].cost : \
										info.weapons[type].cost >> 1)

#define getspecialcost(type)		(info.specials[type].cost)
#define getspecialtimer(type)		(info.specials[type].timer)
#define getspecialdrain(type)		(info.specials[type].drain)
#define getspecialtrade(type)		(gv.difficulty == diff_easy ? info.specials[type].cost : \
										info.specials[type].cost >> 1)

#define geteffectwidth(type)		(info.effects[type].width)
#define geteffectheight(type)		(info.effects[type].height)
#define geteffectgraphic(type)		(info.effects[type].graphic & 127)
#define geteffectstartframe(type)	(info.effects[type].startframe)
#define geteffectnframes(type)		(info.effects[type].nframes)
#define geteffectendframe(type)		(info.effects[type].startframe + info.effects[type].nframes - 1)
#define iseffectreverse(type)		(info.effects[type].graphic & 128)

#define getplayerscore() 			(gv.player.money + getweaponcost(gv.player.weapon) + \
										getspecialcost(gv.player.special) - \
										gv.penalties + 100*(gv.wave - gv.startwave))

#define setstate(state)				(gv.gamestate = (state & ~INPLAYMASK) | state)
#define acceptspen(state)			((state >= state_play) && (state <= state_form))

extern type_gamevars	gv;
extern type_info		info;

#endif
