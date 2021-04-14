#include <PalmOS.h>
#include "title.h"
#include "gfx.h"
#include "rc.h"
#include "game.h"

#define TITLEWIDTH		136
#define TITLEHEIGHT		40
#define TITLEX			12
#define TITLEY			6


UInt16 inittitle(void)
{
	UInt16			err = 0;
	WinHandle		oldwin = WinGetDrawWindow();
	RectangleType	rect;

#ifdef COLOR
	UInt8				i;
	RGBColorType		colors[16];
	IndexedColorType	oldforecolor, oldbackcolor;

	WinSetDrawWindow(WinGetDisplayWindow());

	for(i = 0; i < 16; i++)
	{
		colors[i].r = i*8;
		colors[i].g = 180+i*5;
		colors[i].b = i*i;
	}

	err = WinPalette(winPaletteSet, 240, 16, colors);
	if(err) return err;
#endif

	if(gfx.title.effectbuffer)
	{
		WinDeleteWindow(gfx.title.effectbuffer, false);
		gfx.title.effectbuffer = NULL;
	}
	if(gfx.title.titlegraphic)
	{
		WinDeleteWindow(gfx.title.titlegraphic, false);
		gfx.title.titlegraphic = NULL;
	}

	err = loadgfxwin(&gfx.title.effectbuffer, TITLEWIDTH, TITLEHEIGHT*2, 1, 0);
	if(err) return err;

	WinSetDrawWindow(gfx.title.effectbuffer);
	RctSetRectangle(&rect, 0, TITLEHEIGHT/2-1, TITLEWIDTH, 2);
#ifdef COLOR
	oldforecolor = WinSetForeColor(255);
	oldbackcolor = WinSetBackColor(0);
#endif
	WinDrawRectangle(&rect, 0);

	//fore color must be 255 here so bmp_title gets converted right (1 bit -> 8 bit)
	err = loadgfxwin(&gfx.title.titlegraphic, TITLEWIDTH, TITLEHEIGHT, 1, bmp_title);
	if(err) return err;

#ifdef COLOR
	WinSetBackColor(oldbackcolor);
	WinSetForeColor(oldforecolor);
#endif
	WinSetDrawWindow(oldwin);

	return err;
}


void closetitle(void)
{
#ifdef COLOR
	WinHandle oldwin = WinGetDrawWindow();
	WinSetDrawWindow(WinGetDisplayWindow());
	WinPalette(winPaletteSetToDefault, 0, 256, NULL);
	WinSetDrawWindow(oldwin);
#endif

	if(gfx.title.effectbuffer)
	{
		WinDeleteWindow(gfx.title.effectbuffer, false);
		gfx.title.effectbuffer = NULL;
	}
	if(gfx.title.titlegraphic)
	{
		WinDeleteWindow(gfx.title.titlegraphic, false);
		gfx.title.titlegraphic = NULL;
	}
}


#ifdef COLOR
void dotitle(void)
{
	RectangleType	rect;
	UInt32*			effectbuffer = BmpGetBits(WinGetBitmap(gfx.title.effectbuffer));
	UInt32*			titlegraphic = BmpGetBits(WinGetBitmap(gfx.title.titlegraphic));
	UInt16			i;
	UInt32			temp;
	UInt32			sub;

	if(!(gv.counter%3))
	{
		for(i = TITLEWIDTH/4; i < (TITLEHEIGHT/2)*TITLEWIDTH/4; i++)
		{
			//top half
			sub = 0;
			temp = SysRandom(0);
	
			if((effectbuffer[i] & 0x000000ff) > 0x000000f0)
			{
				sub |= temp&1;
			}
			if((effectbuffer[i] & 0x0000ff00) > 0x0000f000)
			{
				sub |= (temp&2)<<7;
			}
			if((effectbuffer[i] & 0x00ff0000) > 0x00f00000)
			{
				sub |= (temp&4)<<14;
			}
			if((effectbuffer[i] & 0xff000000) > 0xf0000000)
			{
				sub |= (temp&512)<<15;
			}
			effectbuffer[i-TITLEWIDTH/4] = effectbuffer[i] - sub;
		}
	}
	else if(gv.counter%3 == 1)
	{
		for(i = TITLEWIDTH/4; i < (TITLEHEIGHT/2)*TITLEWIDTH/4; i++)
		{
			//bottom half
			sub = 0;
			temp = SysRandom(0);

			if((effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 0x000000ff) > 0x000000f0)
			{
				sub |= temp&1;
			}
			if((effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 0x0000ff00) > 0x0000f000)
			{
				sub |= (temp&2)<<7;
			}
			if((effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 0x00ff0000) > 0x00f00000)
			{
				sub |= (temp&4)<<14;
			}
			if((effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 0xff000000) > 0xf0000000)
			{
				sub |= (temp&512)<<15;
			}
			effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i+TITLEWIDTH/4] =
				effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] - sub;
		}
	}
	else
	{
		for(i = 0; i < TITLEHEIGHT*TITLEWIDTH/4; i++)
		{
			effectbuffer[TITLEHEIGHT*TITLEWIDTH/4+i] =
				effectbuffer[i] & titlegraphic[i];
		}

		RctSetRectangle(&rect, 0, TITLEHEIGHT, TITLEWIDTH, TITLEHEIGHT);
		WinCopyRectangle(gfx.title.effectbuffer, NULL, &rect, TITLEX, TITLEY, winPaint);
	}
}

#elif defined GRAY4
void dotitle(void)
{
	RectangleType	rect;
	UInt8*			effectbuffer = BmpGetBits(WinGetBitmap(gfx.title.effectbuffer));
	UInt16*			titlegraphic = BmpGetBits(WinGetBitmap(gfx.title.titlegraphic));
	UInt16			i;
	UInt16			sub;

	if(!(gv.counter%3))
	{
		for(i = TITLEWIDTH/4; i < (TITLEHEIGHT/2)*TITLEWIDTH/4; i++)
		{
			//top half
			sub = SysRandom(0);

			if((!(effectbuffer[i] & 3)) 
			|| (sub & 256)
			|| (sub & 2))
			{
				sub &= ~3;
			}
			if((!(effectbuffer[i] & 12))
			|| (sub & 512)
			|| (sub & 8))
			{
				sub &= ~12;
			}
			if((!(effectbuffer[i] & 48))
			|| (sub & 1024)
			|| (sub & 32))
			{
				sub &= ~48;
			}
			if((!(effectbuffer[i] & 192))
			|| (sub & 2048)
			|| (sub & 128))
			{
				sub &= ~192;
			}
			effectbuffer[i-TITLEWIDTH/4] = effectbuffer[i]-(sub&(1+4+16+64));
		}
	}
	else if(gv.counter%3 == 1)
	{
		for(i = TITLEWIDTH/4; i < (TITLEHEIGHT/2)*TITLEWIDTH/4; i++)
		{
			//bottom half
			sub = SysRandom(0);

			if((!(effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 3)) 
			|| (sub & 256)
			|| (sub & 2))
			{
				sub &= ~3;
			}
			if((!(effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 12))
			|| (sub & 512)
			|| (sub & 8))
			{
				sub &= ~12;
			}
			if((!(effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 48))
			|| (sub & 1024)	
			|| (sub & 32))
			{
				sub &= ~48;
			}
			if((!(effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 192))
			|| (sub & 2048)
			|| (sub & 128))
			{
				sub &= ~192;
			}
			effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i+TITLEWIDTH/4] =
				effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i]-(sub&(1+4+16+64));
		}
	}
	else
	{
		for(i = 0; i < TITLEHEIGHT*TITLEWIDTH/8; i++)
		{
			((UInt16*)effectbuffer)[TITLEHEIGHT*TITLEWIDTH/8+i] =
				((UInt16*)effectbuffer)[i] & titlegraphic[i];
		}

		RctSetRectangle(&rect, 0, TITLEHEIGHT, TITLEWIDTH, TITLEHEIGHT);
		WinCopyRectangle(gfx.title.effectbuffer, NULL, &rect, TITLEX, TITLEY, winPaint);
	}
}

#else
void dotitle(void)
{
	RectangleType	rect;
	UInt16*			effectbuffer = BmpGetBits(WinGetBitmap(gfx.title.effectbuffer));
	UInt32*			titlegraphic = BmpGetBits(WinGetBitmap(gfx.title.titlegraphic));
	UInt16			i;
	UInt16			sub;

	if(!(gv.counter%3))
	{
		for(i = TITLEWIDTH/4; i < (TITLEHEIGHT/2)*TITLEWIDTH/4; i++)
		{
			//top half
			sub = SysRandom(0);

			if(!(effectbuffer[i] & 0x000f))
			{
				sub &= ~0x000f;
			}
			if(!(effectbuffer[i] & 0x00f0))
			{
				sub &= ~0x00f0;
			}
			if(!(effectbuffer[i] & 0x0f00))
			{
				sub &= ~0x0f00;
			}
			if(!(effectbuffer[i] & 0xf000))
			{
				sub &= ~0xf000;
			}
			effectbuffer[i-TITLEWIDTH/4] = effectbuffer[i]-(sub&(1+16+256+4096));
		}
	}
	else if(gv.counter%3 == 1)
	{
		for(i = TITLEWIDTH/4; i < (TITLEHEIGHT/2)*TITLEWIDTH/4; i++)
		{
			//bottom half
			sub = SysRandom(0);

			if(!(effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 0x000f))
			{
				sub &= ~0x000f;
			}
			if(!(effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 0x00f0))
			{
				sub &= ~0x00f0;
			}
			if(!(effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 0x0f00))
			{
				sub &= ~0x0f00;
			}	
			if(!(effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i] & 0xf000))
			{
				sub &= ~0xf000;
			}
			effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i+TITLEWIDTH/4] =
				effectbuffer[TITLEHEIGHT*TITLEWIDTH/4-1-i]-(sub&(1+16+256+4096));
		}
	}
	else
	{
		for(i = 0; i < TITLEHEIGHT*TITLEWIDTH/8; i++)
		{
			((UInt32*)effectbuffer)[TITLEHEIGHT*TITLEWIDTH/8+i] =
				((UInt32*)effectbuffer)[i] & titlegraphic[i];
		}

		RctSetRectangle(&rect, 0, TITLEHEIGHT, TITLEWIDTH, TITLEHEIGHT);
		WinCopyRectangle(gfx.title.effectbuffer, NULL, &rect, TITLEX, TITLEY, winPaint);
	}
}

#endif