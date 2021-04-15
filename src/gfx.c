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
#include "gfx.h"
#include "rc.h"
#include "game.h"
#include "palm.h"
#include "prefs.h"

#define TRANSPARENT			4			//hot pink

#define NSTARCOLORS			6
#define STARAMT0			120
#define STARAMT1			80
#define STARAMT2			120
#define STARAMT3			90
#define STARAMT4			60
#define STARAMT5			40
#ifdef COLOR
#define STARCOLOR0			203
#define STARCOLOR1			107
#define STARCOLOR2			217
#define STARCOLOR3			219
#define STARCOLOR4			221
#define STARCOLOR5			224
#elif defined GRAY4 
#define STARCOLOR0			1
#define STARCOLOR1			1
#define STARCOLOR2			1
#define STARCOLOR3			1
#define STARCOLOR4			2
#define STARCOLOR5			2
#else
#define STARCOLOR0			1
#define STARCOLOR1			2
#define STARCOLOR2			1
#define STARCOLOR3			2
#define STARCOLOR4			5
#define STARCOLOR5			8
#endif

#define MONEYX				94
#define MONEYDIGITS			6
#define POWERX				40
#define POWERDIGITS			3
#define SHIELDX				135
#define SHIELDDIGITS		3
#define INFOY				2
#define INFOCOLOR			44
#define INFOGRAY			4
#define INFOZEROCOLOR		161

#define bgstartline()		(SCREENHEIGHT-1-(gv.counter>>2)%SCREENHEIGHT)

type_gfx gfx;

#ifdef COLOR
static UInt16 createmaskwin(WinHandle* maskptr, WinHandle win,
	const UInt16 width, const UInt16 height, const UInt16 nframes);
#endif

static void drawaliens(void);
static void drawplayer(void);
static void drawbullets(void);
static void drawblocks(void);
static void draweffects(void);
static void drawwavetext(void);

static void erasebuffer_compat(void);
static void erasebuffer_asm(void);

static void copybuffer_compat(void);
static void copybuffer_asm(void);

static void drawsprite_compat(const Coord x, const Coord y, sprite* sp);
static void drawsprite_asm(const Coord x, const Coord y, sprite* sp);


void display(void)
{
	device.func_erasebuffer();
	drawaliens();
	drawblocks();
	drawbullets();
	drawplayer();
	draweffects();
	if(gv.wavedisplay > gv.counter)
	{
		drawwavetext();
	}
	device.func_copybuffer();
}


UInt16 setdisplaydepth(void)
{
	UInt32 depth;

	WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depth, NULL);

#ifdef COLOR
	if(depth & 128)
	{
		depth = 8;
	}
#elif defined GRAY4
	if(depth & 2)
	{
		depth = 2;
	}
#else
	if(depth & 8)
	{
		depth = 4;
	}
#endif

	else
	{
		return error_displaydepth;
	}

	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
	return 0;
}


void drawwavetext(void)
{
	RectangleType	rect;
	char			wavetext[maxStrIToALen+5];
	FontID			oldfont = FntSetFont(2);
	WinHandle		oldwin = WinGetDrawWindow();
	
	WinSetDrawWindow(gfx.main.screenbuffer);
	RctSetRectangle(&rect, WAVETEXTX, WAVETEXTY, WAVETEXTWIDTH, WAVETEXTHEIGHT);
	WinEraseRectangle(&rect, 0);
	WinDrawRectangleFrame(roundFrame, &rect);
	StrCopy(wavetext, "Wave ");
	StrIToA(&wavetext[5], gv.wave+1);
	WinDrawChars(wavetext, StrLen(wavetext),
		WAVETEXTX + ((100-FntCharsWidth(wavetext, StrLen(wavetext)))>>1),
		WAVETEXTY + 6);				

	FntSetFont(oldfont);
	WinSetDrawWindow(oldwin);
}


void drawinfobar(void)
{
	MemHandle memhand	= DmGetResource(bitmapRsc, bmp_infobar);
	BitmapPtr bmpptr	= MemHandleLock(memhand);
	if(!bmpptr) return;		

	WinDrawBitmap(bmpptr, 0, 0);
	
	MemHandleUnlock(memhand);
	DmReleaseResource(memhand);

	drawinfo(info_money);
	drawinfo(info_power);
	drawinfo(info_shield);
}


void drawinfo(const UInt8 type)
{
	Int16	x;
	Int8	digits;
	Int32	value;

#ifndef GRAY4
#ifdef COLOR
	Boolean	zeroflag = false;
	IndexedColorType oldtextcolor = 0;
	IndexedColorType oldbackcolor = WinSetBackColor(INFOCOLOR);
#else
	IndexedColorType oldbackcolor = WinSetBackColor(INFOGRAY);
#endif
#endif

	switch(type)
	{
		default:
		case info_money:
			x = MONEYX;
			digits = MONEYDIGITS;
			value = gv.player.money;
			break;
		case info_power:
			x = POWERX;
			digits = POWERDIGITS;
			value = gv.player.power;
			break;
		case info_shield:
			x = SHIELDX;
			digits = SHIELDDIGITS;
			value = gv.player.shield;
	}

#ifdef COLOR
	if((!value) && (type != info_money))
	{
		zeroflag = true;
		oldtextcolor = WinSetTextColor(INFOZEROCOLOR);
	}
#endif

	while(digits--)
	{
		WinDrawChar('0' + value%10, x, INFOY);
		value /= 10;
		x -= FntCharWidth('0');
	}

#ifndef GRAY4
	WinSetBackColor(oldbackcolor);
#ifdef COLOR
	if(zeroflag)
	{
		WinSetTextColor(oldtextcolor);
	}
#endif
#endif
}


void drawsprite_compat(const Coord x, const Coord y, sprite* sp)
{
	RectangleType lamerect;

	RctSetRectangle(&lamerect, 0, sp->index,
		sp->width, sp->height);

	WinCopyRectangle(sp->mask, gfx.main.screenbuffer,
		&lamerect, x, y, winMask);

	WinCopyRectangle(sp->graphic, gfx.main.screenbuffer,
		&lamerect, x, y, winOverlay);
}


void setgfxfunctions(void)
{
	device.func_copybuffer	= copybuffer_compat;
	device.func_erasebuffer	= erasebuffer_compat;
	device.func_drawsprite	= drawsprite_compat;

	if((device.flags & df_68k)
	&& (!(prefs.settings.flags & set_compat)))
	{
		device.func_erasebuffer	= erasebuffer_asm;
		device.func_drawsprite	= drawsprite_asm;			

		if(device.flags & df_lores)
		{
			device.func_copybuffer = copybuffer_asm;
		}
	}
}


void draweffects(void)
{
	UInt8	i;
	sprite	sprite;

	for(i = 0; i < gv.neffects; i++)
	{
		switch(geteffectgraphic(gv.effects[i].type))
		{
			default:
			case ALIENEFFECT:
				sprite.graphic	= gfx.main.alienwin;
				sprite.mask		= gfx.main.alienmaskwin;
				sprite.width	= ALIENWIDTH;
				sprite.height	= ALIENHEIGHT;
				sprite.index	= gv.effects[i].frame * ALIENHEIGHT;
				break;
			case PLAYEREFFECT:
				sprite.graphic	= gfx.main.playerwin;
				sprite.mask		= gfx.main.playermaskwin;
				sprite.width	= PLAYERWIDTH;
				sprite.height	= PLAYERHEIGHT;
				sprite.index	= gv.effects[i].frame * PLAYERHEIGHT;
				break;
		}

		device.func_drawsprite(gv.effects[i].x, gv.effects[i].y, &sprite);

		if(gv.counter & 1)
		{
			if(iseffectreverse(gv.effects[i].type))
			{
				if(--gv.effects[i].frame <
				geteffectstartframe(gv.effects[i].type))
				{
					removeeffect(i--);
				}
			}
			else
			{
				if(++gv.effects[i].frame >=
				geteffectstartframe(gv.effects[i].type) +
				geteffectnframes(gv.effects[i].type))
				{
					removeeffect(i--);
				}
			}
		}
	}
}


void drawblocks(void)
{
	const Int16 groupx[BLOCK_GROUPS] = {BLOCK_LEFT0, BLOCK_LEFT1, BLOCK_LEFT2};
	UInt8	n, i, j;
	sprite	sprite;

	sprite.graphic	= gfx.main.blockwin;
	sprite.mask		= gfx.main.blockmaskwin;
	sprite.width	= BLOCKWIDTH;
	sprite.height	= BLOCKHEIGHT;

	for(n = 0; n < BLOCK_GROUPS; n++)
	{
		for(j = 0; j < BLOCK_ROWS; j++)
		{
			for(i = 0; i < BLOCK_COLUMNS; i++)
			{
				if(gv.blocks[n][i][j] != NOBLOCK)
				{
					sprite.index = gv.blocks[n][i][j] * BLOCKHEIGHT;

					device.func_drawsprite(groupx[n]+i*BLOCKWIDTH,
						BLOCK_TOP+j*BLOCKHEIGHT, &sprite);
				}
			}
		}
	}
}


void drawplayer(void)
{
	sprite psprite;

	psprite.graphic	= gfx.main.playerwin;
	psprite.mask	= gfx.main.playermaskwin;
	psprite.index	= gv.player.frame * PLAYERHEIGHT;
	psprite.width	= PLAYERWIDTH;
	psprite.height	= PLAYERHEIGHT;

	device.func_drawsprite(gv.player.x, PLAYERY, &psprite);
}


void drawbullets(void)
{
	UInt8 i;
	sprite bsprite;

	bsprite.graphic	= gfx.main.bulletwin;
	bsprite.mask	= gfx.main.bulletmaskwin;
	bsprite.width	= BULLETWIDTH;

	for(i = 0; i < gv.nbullets; i++)
	{
		bsprite.index	= gv.bullets[i].frame * BULLETHEIGHT;
		bsprite.height	= getbulletheight(gv.bullets[i].type);

		device.func_drawsprite(gv.bullets[i].x>>FRACBITS,
			gv.bullets[i].y>>FRACBITS, &bsprite);

		if((gv.counter & 1)
		&& (++gv.bullets[i].frame >=
		getbulletstartframe(gv.bullets[i].type) +
		getbulletframes(gv.bullets[i].type)))
		{
			gv.bullets[i].frame = getbulletstartframe(gv.bullets[i].type);
		}
	}
}


void drawaliens(void)
{
	UInt8 i, j;
	sprite asprite;

	asprite.graphic	= gfx.main.alienwin;
	asprite.mask	= gfx.main.alienmaskwin;
	asprite.width	= ALIENWIDTH;

	for(j = 0; j <= gv.abottombound; j++)
	{
		for(i = gv.aleftbound; i <= gv.arightbound; i++)
		{
			if((gv.aliens[i][j].type != NOALIEN)
			&& ((gv.alieny>>FRACBITS) + j*ALIENHEIGHT > 0))
			{
				asprite.index = ((gv.aliens[i][j].type<<1) +
					((gv.alienframecount>>(FRACBITS+2))&1)) * ALIENHEIGHT;
				asprite.height = getalienheight(gv.aliens[i][j].type);

				device.func_drawsprite((gv.alienx>>FRACBITS) +
					i*ALIENWIDTH + getalienxoffset(gv.aliens[i][j].type),
					(gv.alieny>>FRACBITS) + j*ALIENHEIGHT +
					getalienyoffset(gv.aliens[i][j].type),
					&asprite);
			}
		}
	}
}


UInt16 loadbackground(void)
{
	UInt16 err = 0;
	UInt16 i, j;
	Coord x, y;
	IndexedColorType oldbackcolor = WinSetBackColor(215);	//black
	IndexedColorType oldforecolor = WinSetForeColor(0);
	WinHandle oldwin = WinGetDrawWindow();
	UInt16 staramts[] = {STARAMT0, STARAMT1, STARAMT2, STARAMT3, STARAMT4, STARAMT5};
	IndexedColorType starcolors[] = {STARCOLOR0, STARCOLOR1, STARCOLOR2,
		STARCOLOR3, STARCOLOR4, STARCOLOR5};

	if(!gfx.main.background)
	{
		err = loadgfxwin(&gfx.main.background, SCREENWIDTH, SCREENHEIGHT, 1, 0);
		if(err) return err;
	}

	WinSetDrawWindow(gfx.main.background);
	WinEraseWindow();

	for(j = 0; j < NSTARCOLORS; j++)
	{
		WinSetForeColor(starcolors[j]);

		for(i = 0; i < staramts[j]; i++)
		{
			x = SysRandom(0)%SCREENWIDTH;
			y = SysRandom(0)%SCREENHEIGHT;
			WinDrawPixel(x, y);
		}
	}

	WinSetBackColor(oldbackcolor);
	WinSetForeColor(oldforecolor);
	WinSetDrawWindow(oldwin);

	return err;
}


UInt16 loadgfx(void)
{
	UInt16 err;

	err = loadgfxwin(&gfx.main.alienwin, ALIENWIDTH, ALIENHEIGHT,
		NALIENFRAMES, bmp_alien_base);
	if(err) return err;

	err = loadgfxwin(&gfx.main.playerwin, PLAYERWIDTH, PLAYERHEIGHT,
		NPLAYERFRAMES, bmp_player_base);
	if(err) return err;

	err = loadgfxwin(&gfx.main.bulletwin, BULLETWIDTH, BULLETHEIGHT,
		NBULLETFRAMES, bmp_bullet_base);
	if(err) return err;

	err = loadgfxwin(&gfx.main.blockwin, BLOCKWIDTH, BLOCKHEIGHT,
		NBLOCKFRAMES, bmp_block0);
	if(err) return err;

#ifdef COLOR
	err = createmaskwin(&gfx.main.alienmaskwin, gfx.main.alienwin, ALIENWIDTH,
		ALIENHEIGHT, NALIENFRAMES);
	if(err) return err;

	err = createmaskwin(&gfx.main.playermaskwin, gfx.main.playerwin, PLAYERWIDTH,
		PLAYERHEIGHT, NPLAYERFRAMES);
	if(err) return err;

	err = createmaskwin(&gfx.main.bulletmaskwin, gfx.main.bulletwin, BULLETWIDTH,
		BULLETHEIGHT, NBULLETFRAMES);
	if(err) return err;

	err = createmaskwin(&gfx.main.blockmaskwin, gfx.main.blockwin, BLOCKWIDTH,
		BLOCKHEIGHT, NBLOCKFRAMES);

#else
	//GRAY4 and GRAY16 both use this
	err = loadgfxwin(&gfx.main.alienmaskwin, ALIENWIDTH, ALIENHEIGHT,
		NALIENFRAMES, mask_alien_base);
	if(err) return err;

	err = loadgfxwin(&gfx.main.playermaskwin, PLAYERWIDTH, PLAYERHEIGHT,
		NPLAYERFRAMES, mask_player_base);
	if(err) return err;

	err = loadgfxwin(&gfx.main.bulletmaskwin, BULLETWIDTH, BULLETHEIGHT,
		NBULLETFRAMES, mask_bullet_base);
	if(err) return err;

	err = loadgfxwin(&gfx.main.blockmaskwin, BLOCKWIDTH, BLOCKHEIGHT,
		NBLOCKFRAMES, mask_block0);

#endif

	return err;
}


void freegfx(void)
{
	if(gfx.main.screenbuffer)
	{
		WinDeleteWindow(gfx.main.screenbuffer, false);
		gfx.main.screenbuffer = NULL;
	}

	if(gfx.main.alienwin)
	{
		WinDeleteWindow(gfx.main.alienwin, false);
	}
	if(gfx.main.playerwin)
	{
		WinDeleteWindow(gfx.main.playerwin, false);
	}
	if(gfx.main.bulletwin)
	{
		WinDeleteWindow(gfx.main.bulletwin, false);
	}
	if(gfx.main.blockwin)
	{
		WinDeleteWindow(gfx.main.blockwin, false);
	}
	if(gfx.main.alienmaskwin)
	{
		WinDeleteWindow(gfx.main.alienmaskwin, false);
	}
	if(gfx.main.playermaskwin)
	{
		WinDeleteWindow(gfx.main.playermaskwin, false);
	}
	if(gfx.main.bulletmaskwin)
	{
		WinDeleteWindow(gfx.main.bulletmaskwin, false);
	}
	if(gfx.main.blockmaskwin)
	{
		WinDeleteWindow(gfx.main.blockmaskwin, false);
	}

	if(gfx.main.background)
	{
		WinDeleteWindow(gfx.main.background, false);
		gfx.main.background = NULL;					//needed by the title form handler
	}
}


#ifdef COLOR
UInt16 createmaskwin(WinHandle* maskptr, WinHandle win,
const UInt16 width, const UInt16 height, const UInt16 nframes)
{
	UInt16 err;
	UInt16 i;
	UInt8* dstbits;
	UInt8* srcbits	= BmpGetBits(WinGetBitmap(win));
	UInt16 size		= BmpBitsSize(WinGetBitmap(win));

	*maskptr = WinCreateOffscreenWindow(width, height*nframes,
		screenFormat, &err);
	if(err) return err;

	dstbits = BmpGetBits(WinGetBitmap(*maskptr));

	for(i = 0; i < size; i++)
	{
		if(srcbits[i] == TRANSPARENT)
		{
			srcbits[i] = 0;
			dstbits[i] = 0;
		}
		else
		{
			dstbits[i] = 255;
		}
	}

	return 0;
}
#endif


UInt16 loadgfxwin(WinHandle* winptr, const UInt16 width,
const UInt16 height, const UInt16 nframes, const UInt16 startid)
{
	UInt16 err;

	if(!*winptr)
	{
		*winptr = WinCreateOffscreenWindow(width, height*nframes,
			screenFormat, &err);
		if(err) return err;
	}

	if(startid)
	{
		UInt16		i;
		MemHandle	memhand;
		BitmapPtr	bmpptr;
		WinHandle	oldwin	= WinGetDrawWindow();

#ifdef COLOR
		UInt8*		winbits	= BmpGetBits(WinGetBitmap(*winptr));
		UInt16		size	= BmpBitsSize(WinGetBitmap(*winptr));

		for(i = 0; i < size; i++)
		{
			winbits[i] = TRANSPARENT;
		}
#endif

		WinSetDrawWindow(*winptr);

		for(i = 0; i < nframes; i++)
		{
			memhand = DmGetResource(bitmapRsc, startid+i);
			if(!memhand) return -1;
			bmpptr = MemHandleLock(memhand);
			if(!bmpptr) return -1;		

			WinDrawBitmap(bmpptr, 0, i*height);
	
			MemHandleUnlock(memhand);
			DmReleaseResource(memhand);
		}

		WinSetDrawWindow(oldwin);
	}

	return 0;
}


void copybuffer_compat(void)
{
	RectangleType screenrect;

	RctSetRectangle(&screenrect, SCREENX, SCREENY, SCREENWIDTH, SCREENHEIGHT);
	WinCopyRectangle(gfx.main.screenbuffer, NULL,
		&screenrect, 0, STATBARHEIGHT, winPaint);
}


void erasebuffer_compat(void)
{
	Coord startline = bgstartline();
	RectangleType bgrect = {{0, startline}, {SCREENWIDTH, SCREENHEIGHT-startline}};
	
	WinCopyRectangle(gfx.main.background, gfx.main.screenbuffer,
		&bgrect, SCREENX, SCREENY, winPaint);

	if(startline)
	{
		RctSetRectangle(&bgrect, 0, 0, SCREENWIDTH, startline);
		WinCopyRectangle(gfx.main.background, gfx.main.screenbuffer,
			&bgrect, SCREENX, SCREENY+SCREENHEIGHT-startline, winPaint);
	}
}


#ifdef COLOR
void copybuffer_asm(void)
{
	UInt8* bufferbits	= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8* visbits		= BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));

	visbits		+= STATBARHEIGHT*SCREENWIDTH;
	bufferbits	+= SCREENY*BUFFERWIDTH + SCREENX;

	asm volatile("
		movem.l %%a2-%%a6,-(%%a7)
		move.l %4, %%d0

	colorcbloop:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1			
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1
		movem.l (%0)+, %%d1-%%d4
		movem.l %%d1-%%d4, (%1)
		lea 16(%1),%1
		lea 32(%0),%0
		dbra %%d0, colorcbloop

		movem.l (%%a7)+,%%a2-%%a6
	"
	: "=a" (bufferbits),
		"=a" (visbits)
	: "0" (bufferbits),
		"1" (visbits),
		"i" (SCREENHEIGHT-1)
	: "%%d0", "%%d1", "%%d2", "%%d3", "%%d4", "%%d5", "%%d6", "%%d7",
		 "%%a2", "%%a3", "%%a4", "%%a5", "%%a6");
}


void erasebuffer_asm(void)
{
	UInt8* bufferbits	= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8* bgbits		= BmpGetBits(WinGetBitmap(gfx.main.background));
	Coord startline		= bgstartline();

	bufferbits += SCREENY*BUFFERWIDTH + SCREENX;
	bgbits += startline*SCREENWIDTH;

	asm volatile("
		movem.l %%a2-%%a6, -(%%a7)
		move.w %4, %%d0
		swap %%d0
		move.w %5,%%d0
		sub.w %4,%%d0

	colorebloop1:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1			
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1
		movem.l (%0)+, %%d1-%%d4
		movem.l %%d1-%%d4, (%1)
		lea 48(%1),%1
		dbra %%d0, colorebloop1

		swap %%d0
		tst.w %%d0
		beq colorskiploop2

		subq #1,%%d0
		lea -23040(%0),%0

	colorebloop2:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1			
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1
		movem.l (%0)+, %%d1-%%d4
		movem.l %%d1-%%d4, (%1)
		lea 48(%1),%1
		dbra %%d0, colorebloop2

	colorskiploop2:
		movem.l (%%a7)+, %%a2-%%a6
	"
	: "=a" (bgbits),
		"=a" (bufferbits)
	: "0" (bgbits),
		"1" (bufferbits),
		"m" (startline),
		"i" (SCREENHEIGHT-1)
	: "%%d0", "%%d1", "%%d2", "%%d3", "%%d4", "%%d5", "%%d6", "%%d7",
		"%%a2", "%%a3", "%%a4", "%%a5", "%%a6");
}


void drawsprite_asm(const Coord x, const Coord y, sprite* sp)
{
	UInt8*	srcbits		= BmpGetBits(WinGetBitmap(sp->graphic));
	UInt8*	maskbits	= BmpGetBits(WinGetBitmap(sp->mask));
	UInt8*	dstbits		= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8	roffset		= (x&1)<<3;
	UInt16	srcoffset 	= sp->index * sp->width;

	srcbits		+= srcoffset;
	maskbits	+= srcoffset;
	dstbits		+= y*BUFFERWIDTH + (x&~1);
	
	asm volatile("
		move.b #32,%%d3
		sub.b %6,%%d3
		moveq.l #0,%%d5
		move.b %8,%%d5

	color_dsrows:
		moveq.l #0,%%d4
		move.b %9,%%d4

	color_dscols:
			move.l (%1)+,%%d0
			move.l %%d0,%%d1
			lsr.l %6,%%d0
			lsl.l %%d3,%%d1
			not.l %%d0
			not.l %%d1
			and.l %%d0,(%2)
			and.l %%d1,4(%2)

			move.l (%0)+,%%d0
			move.l %%d0,%%d1
			lsr.l %6,%%d0
			lsl.l %%d3,%%d1
			or.l %%d0,(%2)+
			or.l %%d1,(%2)
			dbra %%d4,color_dscols

		adda.w %7,%2
		dbra %%d5,color_dsrows
		
	" 
	: "=a" (srcbits),
		"=a" (maskbits),
		"=a" (dstbits)
	: "0" (srcbits),
		"1" (maskbits),
		"2" (dstbits),
		"d" (roffset),
		"g" (BUFFERWIDTH - sp->width),
		"md" (sp->height - 1),
		"md" ((sp->width >> 2) - 1)
	: "%%d0", "%%d1", "%%d3", "%%d4", "%%d5");
}


#elif defined GRAY4
void copybuffer_asm(void)
{
	UInt8* bufferbits	= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8* visbits		= BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));

	visbits		+= STATBARHEIGHT*SCREENWIDTH/4;
	bufferbits	+= 16*BUFFERWIDTH/4 + 16/4;

	asm volatile("
		move.l %4, %%d0

	gray4cbloop:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4, (%1)
		lea 40(%1),%1
		addq #8,%0
		dbra %%d0, gray4cbloop
	"
	: "=a" (bufferbits),
		"=a" (visbits)
	: "0" (bufferbits),
		"1" (visbits),
		"i" (SCREENHEIGHT-1)
	: "%%d0", "%%d1", "%%d2", "%%d3", "%%d4", "%%d5", "%%d6", "%%d7",
		 "%%a2", "%%a3", "%%a4");
}


void erasebuffer_asm(void)
{
	UInt8* bufferbits	= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8* bgbits		= BmpGetBits(WinGetBitmap(gfx.main.background));
	Coord startline		= bgstartline();

	bufferbits += SCREENY*BUFFERWIDTH/4 + SCREENX/4;
	bgbits += startline*SCREENWIDTH/4;

	asm volatile("
		move.w %4,%%d0
		swap %%d0
		move.w %5,%%d0
		sub.w %4,%%d0

	gray4ebloop1:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4, (%1)
		lea 48(%1),%1
		dbra %%d0, gray4ebloop1

		swap %%d0
		tst.w %%d0
		beq gray4skiploop2

		subq #1,%%d0
		lea -5760(%0),%0

	gray4ebloop2:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4, (%1)
		lea 48(%1),%1
		dbra %%d0, gray4ebloop2

	gray4skiploop2:

	"
	: "=a" (bgbits),
		"=a" (bufferbits)
	: "0" (bgbits),
		"1" (bufferbits),
		"m" (startline),
		"i" (SCREENHEIGHT-1)
	: "%%d0", "%%d1", "%%d2", "%%d3", "%%d4", "%%d5", "%%d6", "%%d7",
		 "%%a2", "%%a3", "%%a4");
}


void drawsprite_asm(const Coord x, const Coord y, sprite* sp)
{
	UInt8*	srcbits		= BmpGetBits(WinGetBitmap(sp->graphic));
	UInt8*	maskbits	= BmpGetBits(WinGetBitmap(sp->mask));
	UInt8*	dstbits		= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8	roffset		= (x&7)<<1;
	UInt16	srcoffset 	= (sp->index * sp->width)>>2;

	srcbits		+= srcoffset;
	maskbits	+= srcoffset;
	dstbits		+= y*(BUFFERWIDTH/4) + ((x>>2)&~1);

	asm volatile("
		moveq.l #0,%%d5
		move.b %8,%%d5

	gray4_dsrows:
		moveq.l #0,%%d4
		move.b %9,%%d4

	gray4_dscols:
			move.w (%1)+,%%d0
			swap %%d0
			clr.w %%d0
			lsr.l %6,%%d0
			not.l %%d0
			and.l %%d0,(%2)

			move.w (%0)+,%%d0
			swap %%d0
			clr.w %%d0
			lsr.l %6,%%d0
			or.l %%d0,(%2)
			addq.l #2,%2
			dbra %%d4,gray4_dscols

		adda.w %7,%2
		dbra %%d5,gray4_dsrows
		
	"
	: "=a" (srcbits),
		"=a" (maskbits),
		"=a" (dstbits)
	: "0" (srcbits),
		"1" (maskbits),
		"2" (dstbits),
		"d" (roffset),
		"g" ((BUFFERWIDTH - sp->width)>>2),
		"md" (sp->height - 1),
		"md" ((sp->width >> 3) - 1)
	: "%%d0", "%%d4", "%%d5");
}

#else
void copybuffer_asm(void)
{
	UInt8* bufferbits	= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8* visbits		= BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));

	visbits		+= STATBARHEIGHT*SCREENWIDTH/2;
	bufferbits	+= SCREENY*BUFFERWIDTH/2 + SCREENX/2;

	asm volatile("
		move.l %4, %%d0

	graycbloop:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4,(%1)
		lea 40(%1),%1			
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4,(%1)
		lea 40(%1),%1
		lea 16(%0),%0
		dbra %%d0, graycbloop
	"
	: "=a" (bufferbits),
		"=a" (visbits)
	: "0" (bufferbits),
		"1" (visbits),
		"i" (SCREENHEIGHT-1)
	: "%%d0", "%%d1", "%%d2", "%%d3", "%%d4", "%%d5", "%%d6", "%%d7",
		 "%%a2", "%%a3", "%%a4");
}

void erasebuffer_asm(void)
{
	UInt8* bufferbits	= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8* bgbits		= BmpGetBits(WinGetBitmap(gfx.main.background));
	Coord startline		= bgstartline();

	bufferbits += SCREENY*BUFFERWIDTH/2 + SCREENX/2;
	bgbits += startline*SCREENWIDTH/2;

	asm volatile("
		move.w %4,%%d0
		swap %%d0
		move.w %5,%%d0
		sub.w %4,%%d0

	grayebloop1:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4, (%1)
		lea 40(%1),%1			
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4, (%1)
		lea 56(%1),%1
		dbra %%d0, grayebloop1

		swap %%d0
		tst.w %%d0
		beq grayskiploop2

		subq #1,%%d0
		lea -11520(%0),%0

	grayebloop2:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4, (%1)
		lea 40(%1),%1			
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a4
		movem.l %%d1-%%d7/%%a2-%%a4, (%1)
		lea 56(%1),%1
		dbra %%d0, grayebloop2

	grayskiploop2:

	"
	: "=a" (bgbits),
		"=a" (bufferbits)
	: "0" (bgbits),
		"1" (bufferbits),
		"m" (startline),
		"i" (SCREENHEIGHT-1)
	: "%%d0", "%%d1", "%%d2", "%%d3", "%%d4", "%%d5", "%%d6", "%%d7",
		 "%%a2", "%%a3", "%%a4");
}


void drawsprite_asm(const Coord x, const Coord y, sprite* sp)
{
	UInt8*	srcbits		= BmpGetBits(WinGetBitmap(sp->graphic));
	UInt8*	maskbits	= BmpGetBits(WinGetBitmap(sp->mask));
	UInt8*	dstbits		= BmpGetBits(WinGetBitmap(gfx.main.screenbuffer));
	UInt8	roffset		= (x&3)<<2;
	UInt16	srcoffset 	= (sp->index * sp->width)>>1;

	srcbits		+= srcoffset;
	maskbits	+= srcoffset;
	dstbits		+= y*(BUFFERWIDTH/2) + ((x>>1)&~1);

	asm volatile("
		move.b #32,%%d3
		sub.b %6,%%d3
		moveq.l #0,%%d5
		move.b %8,%%d5

	gray_dsrows:
		moveq.l #0,%%d4
		move.b %9,%%d4

	gray_dscols:
			move.l (%1)+,%%d0
			move.l %%d0,%%d1
			lsr.l %6,%%d0
			lsl.l %%d3,%%d1
			not.l %%d0
			not.l %%d1
			and.l %%d0,(%2)
			and.l %%d1,4(%2)

			move.l (%0)+,%%d0
			move.l %%d0,%%d1
			lsr.l %6,%%d0
			lsl.l %%d3,%%d1
			or.l %%d0,(%2)+
			or.l %%d1,(%2)
			dbra %%d4,gray_dscols

		adda.w %7,%2
		dbra %%d5,gray_dsrows
		
	"
	: "=a" (srcbits),
		"=a" (maskbits),
		"=a" (dstbits)
	: "0" (srcbits),
		"1" (maskbits),
		"2" (dstbits),
		"d" (roffset),
		"g" ((BUFFERWIDTH - sp->width)>>1),
		"md" (sp->height - 1),
		"md" ((sp->width >> 3) - 1)
	: "%%d0", "%%d1", "%%d3", "%%d4", "%%d5");
}

#endif