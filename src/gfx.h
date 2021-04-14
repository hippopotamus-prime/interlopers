#ifndef __GFX___
#define __GFX___

#define ALIENWIDTH			16
#define ALIENHEIGHT			16

#define PLAYERWIDTH			16
#define PLAYERHEIGHT		15
#define PLAYERY				144
#define PLAYERCORNER		4
#define FIREFRAMETIME		4

#define BULLETWIDTH			8
#define BULLETHEIGHT		8

#define STATBARHEIGHT		16
#define SCREENWIDTH			160
#define SCREENHEIGHT		(160-STATBARHEIGHT)
#define SCREENX				16
#define SCREENY				16
#define BUFFERWIDTH			(SCREENWIDTH+16+16)
#define BUFFERHEIGHT		(SCREENHEIGHT+16)

#define WAVETEXTWIDTH		100
#define WAVETEXTHEIGHT		26
#define WAVETEXTX			((BUFFERWIDTH-WAVETEXTWIDTH)/2)
#define WAVETEXTY			54

#define info_money			0
#define info_power			1
#define info_shield			2

#define error_displaydepth	-6001

typedef struct
{
	WinHandle	screenbuffer;
	WinHandle	background;
	WinHandle	alienwin;
	WinHandle	bulletwin;
	WinHandle	playerwin;
	WinHandle	blockwin;
	WinHandle	alienmaskwin;
	WinHandle	bulletmaskwin;
	WinHandle	playermaskwin;
	WinHandle	blockmaskwin;
	WinHandle	scrollwindow;
}type_maingfx;

typedef struct
{
	WinHandle	titlegraphic;
	WinHandle	effectbuffer;
}type_titlegfx;

typedef union
{
	type_maingfx	main;
	type_titlegfx	title;
}type_gfx;

typedef struct
{
	WinHandle	graphic;
	WinHandle	mask;
	UInt16		index;
	UInt8		width;
	UInt8		height;
}sprite;

extern UInt16	loadgfx(void);
extern void		freegfx(void);
extern void		display(void);
extern void		setgfxfunctions(void);
extern void		drawinfobar(void);
extern void		drawinfo(const UInt8 type);
extern UInt16	setdisplaydepth(void);
extern UInt16	loadbackground(void);
extern UInt16	loadgfxwin(WinHandle* winptr, const UInt16 width,
	const UInt16 height, const UInt16 nframes, const UInt16 startid);

extern type_gfx	gfx;

#endif