#include "palm.h"
#include "game.h"

//when updating these, see startapp() in palm.c, restorehandler() in handlers.c, getprefs() in prefs.c
#define SPREFVERSION	1			//saved and unsaved
#define UPREFVERSION	2

#define MAXNAMELEN		15
#define NHISCORES		10
#define NOHISCORE		0xff		

#define NFLAGS			4			//number of checkboxes shown on the settings form
#define set_rconf		1
#define set_qrest		2
#define set_graf		4
#define set_compat		8
#define set_kevents		128
#define set_kevents_bit	7

#define pc_movement		0			//pen control options
#define pc_fire			1
#define pc_pause		2
#define pc_off			3

#define NDIFFS			4
#define diff_easy		0
#define diff_normal 	1
#define diff_hard		2
#define diff_nightmare	3

typedef struct
{
	UInt8	keyactions[NKEYS];
	UInt32	reserved1;
	UInt32	reserved2;
	UInt32	reserved3;
	UInt16	flags;
	UInt8	topwave[NDIFFS];	//I guess this isn't really a setting...
	UInt8	startwave[NDIFFS];
	UInt8	lastkey;
	UInt8	pencontrol;
	UInt8	volume;
	UInt8	difficulty;
	UInt8	hsdifficulty;		//difficulty viewed on the high score form
	UInt8	reserved4;
	UInt8	reserved5;
	UInt8	reserved6;
}type_settings;


typedef struct
{
	UInt32	score;
	UInt32	time;
	UInt32	reserved1;
	UInt32	reserved2;
	UInt8	startwave;
	UInt8	topwave;
	UInt8	reserved3;
	UInt8	reserved4;
	UInt8	reserved5;
	UInt8	reserved6;
	char	name[MAXNAMELEN+1];
}type_highscore;


typedef struct
{
	UInt8	weapon;
	UInt8	special;
}type_gear;


typedef struct
{
	UInt32			checksum;
	UInt32			reserved1;
	UInt16			regcode;
	UInt16			reserved2;
	type_settings	settings;
	type_highscore	highscores[NDIFFS][NHISCORES];
	type_gear		gear[NDIFFS][NWAVES];
}type_prefs;


extern type_prefs prefs;

extern void		getprefs(void);
extern UInt32	getchecksum(UInt8* ptr, const UInt16 size);
extern void		setdefaultkeys(UInt8* keyactions);
extern UInt16	loadgamevars(void);
extern void		storegamevars(void);
extern void		inserthighscore(UInt8 hsindex, type_highscore* scoreptr);
extern UInt8	checkhighscore(UInt32 score);
extern void		clearhighscores(void);
extern void		clear_records(void);
extern Boolean	checkregcode(UInt16 testcode);
extern UInt32	genhscode(void);

#define getprefschecksum() getchecksum((UInt8*)&prefs, sizeof(type_prefs))
