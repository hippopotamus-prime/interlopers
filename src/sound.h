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