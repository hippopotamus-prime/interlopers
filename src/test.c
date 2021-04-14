#include <PalmOS.h>
#include "game.h"
#include "gfx.h"
#include "rc.h"
#include "prefs.h"
#include "palm.h"
#include "sound.h"
#include "title.h"

#define MAXSHIELD			100
#define MAXPOWER			100		//there's the right way, the wrong way, and the Max Power way
#define SHIELDBONUSVALUE	50
#define POWERBONUSVALUE		50
#define MONEYBONUSVALUE		50
#define MONEY2BONUSVALUE	25
#define REPELRANGE			50
#define REPELPOWER			5500
#define BONUSCRITVALUE		0x7580
#define PLAYER_EXPLOSIONS	10		//number of small explosions when the player dies
#define CRIT_SHIELD			15		//draw explosions if shield is below this

static void movealiens(void);
static void addalienbullets(void);
static void doplayer(void);
static void addplayerbullets(void);
static void dobullets(void);
static void initblocks(void);
static void dospecial(void);
static void runspecial(void);
static UInt16 sqroot(const UInt32 hyp);

static void removebullet(UInt8 index);
static void addbullet(const Int16 x, const Int16 y, const UInt8 type, const UInt8 dir);
static void insertbullet(const Int16 x, const Int16 y, const UInt8 type, const UInt8 dir);
static void addeffect(const Int16 x, const Int16 y, const UInt8 type);

static Boolean collisiondetect(const rect* r1, const rect* r2);
static Boolean checkallaliens(rect* brectptr);
static Boolean checkalien(rect* brectptr, Int8 i, Int8 j);
static Boolean checkblocks(rect* brectptr);
static Boolean checkplayer(rect* brectptr);
static void killalien(const UInt8 i, const UInt8 j);
static void loadalien(const UInt8 i, const UInt8 j, const UInt8 type);
static void blowup_player(const UInt8 nexplosions);

static const Int8 cosines[32] = {64, 63, 59, 53, 45, 35, 24, 12,
								0, -12, -24, -35, -45, -53, -59, -62,
								-64, -62, -59, -53, -45, -35, -24, -12,
								0, 12, 24, 35, 45, 53, 59, 63};
#define TRIGBITS	6
#define UP			24
#define DOWN		8

type_gamevars	gv;
type_info		info;


void dogame(void)
{
	gv.counter++;

	switch(gv.gamestate)
	{
		case state_title:
			dotitle();
			break;

		case state_play:
			movealiens();

			if(gv.counter & 1)
			{
				addalienbullets();
			}
			else if(!(gv.counter & 3))
			{
				EvtResetAutoOffTimer();
			}
			
			doplayer();
			dobullets();
			display();
			break;

		case state_form:
			//player is doing the "teleport in" animation

			display();

			if(!(gv.counter & 3))
			{
				if(++gv.player.frame == playerframe_main)
				{
					setstate(state_play);
				}
			}
			break;

		case state_nearwin:
		{
			//all aliens are dead, but powerups are still falling

			UInt8 i;
			Boolean powerups = false;

			for(i = 0; i < gv.nbullets; i++)
			{
				if(isbonus(gv.bullets[i].type))
				{
					powerups = true;
				}
			}

			if(powerups)
			{
				doplayer();
			}
			else if(!(gv.counter & 3))
			{
				if(!(--gv.player.frame))
				{
					setstate(state_win);
				}
			}

			dobullets();
			display();
			break;
		}

		case state_win:
			//current level has been won

			if((gv.wave >= LASTFREEWAVE)
			&& ((!prefs.regcode) || (!checkregcode(prefs.regcode))))
			{
				setstate(state_noreg);

				if(!FrmAlert(alert_needreg))
				{
					//try to register if the player hit 'yes'...
					//this still goes to the noreg state after form_register
					//closes, but if registration was successful, state_win is reset
					FrmPopupForm(form_register);
					gv.gamestate |= state_regwait;
					//manually force the game to pause, since we don't want it to
					//go into state_noreg before form_register finishes
				}
			}
			else
			{
				if(++gv.wave >= ENDWAVE)
				{
					setsound(sound_wingame);
					setstate(state_wingame);
				}
				else
				{
					setstate(state_shop);
					FrmPopupForm(form_shop);
				}
			}
			break;

		case state_enter:
			//starting a new level...

			globalerr = loadbackground();
			loadwave();
			initblocks();
			gv.player.x				= PLAYERSTARTX;
			gv.player.frame			= playerframe_form0;
			gv.player.firetimer		= 0;
			gv.player.firedata		= 0;
			gv.player.specialtimer	= 0;
			gv.player.specialdata	= 0;
			gv.player.shield		= MAXSHIELD;
			gv.player.power			= MAXPOWER;
			gv.nbullets				= 0;
			gv.neffects				= 0;
			gv.pentarget			= NOTARGET;
			gv.jog					= NOJOG;
			gv.counter				= 0;
			gv.wavedisplay			= WAVEDISPLAYTIME;
			setstate(state_form);

			drawinfobar();

			if(gv.wave > prefs.settings.topwave[gv.difficulty])
			{
				prefs.settings.topwave[gv.difficulty] = gv.wave;
			}

			if(getweaponcost(gv.player.weapon) +
			getspecialcost(gv.player.special) >
			getweaponcost(prefs.gear[gv.difficulty][gv.wave].weapon) +
			getspecialcost(prefs.gear[gv.difficulty][gv.wave].special))
			{
				prefs.gear[gv.difficulty][gv.wave].weapon	= gv.player.weapon;
				prefs.gear[gv.difficulty][gv.wave].special	= gv.player.special;
			}

			prefs.checksum = getprefschecksum();
			break;

		case state_wingame:
			//final level has been won

			if(checkhighscore(getplayerscore()) != NOHISCORE)
			{
				setstate(state_wingamehs);
				FrmPopupForm(form_wingamehs);
			}
			else
			{
				setstate(state_wingame);
				FrmPopupForm(form_wingame);
			}
			break;

		case state_blowup0:
		case state_blowup1:
		case state_blowup2:
			//player is dead, but still doing the "explode" animation

			if(gv.neffects > 0)
			{
				movealiens();
				dobullets();
				display();
			}
			else if(gv.gamestate < state_blowup2)
			{
				blowup_player(PLAYER_EXPLOSIONS);
				gv.gamestate++;
			}
			else
			{
				setstate(state_lose);
			}
			break;

		case state_noreg:
			//last free wave was won, no registration

			if((prefs.regcode)
			&& (checkregcode(prefs.regcode)))
			{
				//the player gets a chance to register at the last minute...
				setstate(state_win);
				break;
			}
			//else fall into state_lose
	
		case state_lose:
			//player was killed

			if(checkhighscore(getplayerscore()) != NOHISCORE)
			{
				setstate(state_diehs);
				FrmPopupForm(form_diehs);
			}
			else if((prefs.settings.flags & set_qrest)
			&& (gv.wave < ENDWAVE)			//this check is probably not necessary
			&& (gv.gamestate != state_noreg))	//ok, so this is kind of a cheap hack...
			{
				initgame(REPLAY);
			}
			else
			{
				setstate(state_die);
				FrmPopupForm(form_die);
			}
			break;

		case state_retire:
			setstate(state_none);
			FrmGotoForm(form_title);
			break;
	}
}


void initgame(const UInt8 startwave)
{
	if(startwave != REPLAY)
	{
		gv.difficulty		= prefs.settings.difficulty;
		gv.startwave		= startwave;
		gv.wave				= startwave;
		gv.player.weapon	= prefs.gear[gv.difficulty][startwave].weapon;
		gv.player.special	= prefs.gear[gv.difficulty][startwave].special;
	}
	else
	{
		//weapon, special, wave, difficulty stay the same
		gv.startwave = gv.wave;
	}

	gv.penalties = getweaponcost(gv.player.weapon) + getspecialcost(gv.player.special);

#ifdef CHEAT
	gv.player.money	= 1000;
#else
	gv.player.money	= 0;
#endif

	setstate(state_enter);
}


void killalien(const UInt8 i, const UInt8 j)
{
	UInt8 n = 0;

	gv.aliens[i][j].type = NOALIEN;

	if(!(--gv.naliens))
	{
		setsound(sound_winlevel);
		setstate(state_nearwin);
	}
	else
	{
		if(i == gv.arightbound)
		{
			while(gv.aliens[gv.arightbound][n].type == NOALIEN)
			{
				if(++n > gv.abottombound)
				{
					n = 0;
					gv.arightbound--;
				}
			}
		}
		else if(i == gv.aleftbound)
		{
			while(gv.aliens[gv.aleftbound][n].type == NOALIEN)
			{
				if(++n > gv.abottombound)
				{
					n = 0;
					gv.aleftbound++;
				}
			}
		}

		n = gv.aleftbound;
		if(j == gv.abottombound)
		{
			while(gv.aliens[n][gv.abottombound].type == NOALIEN)
			{
				if(++n > gv.arightbound)
				{
					n = gv.aleftbound;
					gv.abottombound--;
				}
			}
		}
	}

	addeffect((gv.alienx>>FRACBITS) + i*ALIENWIDTH +
		((ALIENWIDTH - geteffectwidth(effect_explode))>>1),
		(gv.alieny>>FRACBITS) + j*ALIENHEIGHT +
		((ALIENHEIGHT - geteffectheight(effect_explode))>>1),
		effect_explode);

	setsound(sound_boom);

	if(SysRandom(0) > BONUSCRITVALUE)
	{
		UInt8 type = ((SysRandom(0)>>12)&3) + bullet_bonus_base;

		Int16 x = (gv.alienx>>FRACBITS) + i*ALIENWIDTH +
			((ALIENWIDTH - getbulletwidth(type))>>1);

		Int16 y = (gv.alieny>>FRACBITS) + j*ALIENHEIGHT +
			((ALIENHEIGHT - getbulletheight(type))>>1);

		addbullet(x, y, makebonus(type), DOWN);
	}
}


Boolean collisiondetect(const rect* r1, const rect* r2)
{
	Boolean	collision = false;
	Int16	intwidth;
	Int16	intheight;
	Int32	intarea = 0;

	if(r1->x > r2->x)
	{
		intwidth = r2->x + r2->width - r1->x;
	}
	else
	{
		intwidth = r1->x + r1->width - r2->x;
	}
	if(r1->y > r2->y)
	{
		intheight = r2->y + r2->height - r1->y;
	}
	else
	{
		intheight = r1->y + r1->height - r2->y;
	}

	if((intwidth > 0) && (intheight > 0))
	{
		intarea = intwidth*intheight;

		if((intarea > r1->corner) && (intarea > r2->corner))
		{
			collision = true;
		}
	}

	return collision;
}


Boolean checkplayer(rect* brectptr)
{
	Boolean hit = false;
	rect prect;
	
	prect.x			= gv.player.x;
	prect.y			= PLAYERY;
	prect.width		= PLAYERWIDTH;
	prect.height	= PLAYERHEIGHT;
	prect.corner	= PLAYERCORNER;

	if(collisiondetect(brectptr, &prect))
	{
		if(isbonus(brectptr->data))
		{
			switch(brectptr->data & BTMASK)
			{
				case bullet_bonus_shield:
					if(gv.player.shield < MAXSHIELD - SHIELDBONUSVALUE)
					{
						gv.player.shield += SHIELDBONUSVALUE;
					}
					else
					{
						gv.player.shield = MAXSHIELD;
					}
					drawinfo(info_shield);
					break;

				case bullet_bonus_power:
					if(gv.player.power < MAXPOWER - POWERBONUSVALUE)
					{
						gv.player.power += POWERBONUSVALUE;
					}
					else
					{
						gv.player.power = MAXPOWER;
					}
					drawinfo(info_power);
					break;
		
				case bullet_bonus_money:
					gv.player.money += MONEYBONUSVALUE;
					drawinfo(info_money);
					break;

				case bullet_bonus_money2:
					gv.player.money += MONEY2BONUSVALUE;
					drawinfo(info_money);
			}
			setsound(sound_bonus);
		}
		else
		{
			if(gv.difficulty == diff_nightmare)
			{
				gv.player.shield -= (brectptr->data << 1);
			}
			else
			{
				gv.player.shield -= brectptr->data;
			}

			if(gv.player.shield < 0)
			{
				gv.player.shield = 0;		//for display purposes
				setsound(sound_die);
				blowup_player(PLAYER_EXPLOSIONS);

				if(gv.gamestate < state_blowup0)
				{
					//bullets are still active while the player is exploding,
					//so more collisions are possible
					setstate(state_blowup0);
				}
			}
			else
			{
				setsound(sound_damage);
			}

			drawinfo(info_shield);
		}

		hit = true;
	}

	return hit;
}


void blowup_player(const UInt8 nexplosions)
{
	UInt8 i;
	Coord x, y;

	for(i = 0; i < nexplosions; i++)
	{
		x = gv.player.x - geteffectwidth(effect_explode) +
			SysRandom(0)%(PLAYERWIDTH+geteffectwidth(effect_explode));
		y = PLAYERY - geteffectheight(effect_explode) +
			SysRandom(0)%PLAYERHEIGHT;
		addeffect(x, y, effect_explode);
	}
}



Boolean checkalien(rect* brectptr, Int8 i, Int8 j)
{
	Boolean hit = false;
	rect arect;

	arect.x			= (gv.alienx>>FRACBITS) +
						i*ALIENWIDTH + getalienxoffset(gv.aliens[i][j].type);
	arect.y			= (gv.alieny>>FRACBITS) +
						j*ALIENHEIGHT + getalienyoffset(gv.aliens[i][j].type);
	arect.width		= getalienwidth(gv.aliens[i][j].type);
	arect.height	= getalienheight(gv.aliens[i][j].type);
	arect.corner	= getaliencorner(gv.aliens[i][j].type);

	if(collisiondetect(brectptr, &arect))
	{
		hit = true;

		if(brectptr->data >= gv.aliens[i][j].hp)
		{
			killalien(i, j);
		}
		else
		{
			setsound(sound_hitalien);
			gv.aliens[i][j].hp -= brectptr->data;
		}
	}

	return hit;
}


Boolean checkblocks(rect* brectptr)
{
	const Int16 groupx[BLOCK_GROUPS] = {BLOCK_LEFT0, BLOCK_LEFT1, BLOCK_LEFT2};
	UInt8 n, i, j;
	rect blockrect;
	Boolean hitblock = false;

	for(n = 0; n < BLOCK_GROUPS; n++)
	{
		if((brectptr->x + brectptr->width > groupx[n])
		&& (brectptr->x < groupx[n]+BLOCK_COLUMNS*BLOCKWIDTH))
		{
			break;
		}
	}

	if(n < BLOCK_GROUPS)
	{
		blockrect.width = BLOCKWIDTH;
		blockrect.height = BLOCKHEIGHT;
		blockrect.corner = 0;

		for(j = 0; j < BLOCK_ROWS; j++)
		{
			for(i = 0; i < BLOCK_COLUMNS; i++)
			{
				if(gv.blocks[n][i][j] != NOBLOCK)
				{
					blockrect.x = groupx[n]+i*BLOCKWIDTH;
					blockrect.y = BLOCK_TOP+j*BLOCKHEIGHT;

					if(collisiondetect(brectptr, &blockrect))
					{
						gv.blocks[n][i][j]--;
						setsound(sound_hitbarrier);
						hitblock = true;
					}
				}
			}
		}
	}

	return hitblock;
}


Boolean checkallaliens(rect* brectptr)
{
	Int8 i1, i2, j1, j2;
	Boolean hitalien = false;

	i1 = (brectptr->x - (gv.alienx>>FRACBITS))/ALIENWIDTH;
	i2 = (brectptr->x - (gv.alienx>>FRACBITS) + brectptr->width - 1)/ALIENWIDTH;
	j1 = (brectptr->y - (gv.alieny>>FRACBITS))/ALIENHEIGHT;
	j2 = (brectptr->y - (gv.alieny>>FRACBITS) + brectptr->height - 1)/ALIENHEIGHT;

	//bullets are allowed to hit multiple targets...

	if((brectptr->y >= SCREENY)
	&& (i1 >= 0) && (i1 < COLUMNS)
	&& (j1 >= 0) && (j1 <= gv.abottombound)
	&& (gv.aliens[i1][j1].type != NOALIEN))
	{
		hitalien |= checkalien(brectptr, i1, j1);
	}

	if((brectptr->y >= SCREENY)
	&& (i2 != i1)
	&& (i2 >= 0) && (i2 < COLUMNS)
	&& (j1 >= 0) && (j1 <= gv.abottombound)
	&& (gv.aliens[i2][j1].type != NOALIEN))
	{
		hitalien |= checkalien(brectptr, i2, j1);	
	}	

	if((j2 != j1)
	&& (i1 >= 0) && (i1 < COLUMNS)
	&& (j2 >= 0) && (j2 <= gv.abottombound)
	&& (gv.aliens[i1][j2].type != NOALIEN))
	{
		hitalien |= checkalien(brectptr, i1, j2);
	}

	if((i2 != i1) && (j2 != j1)
	&& (i2 >= 0) && (i2 < COLUMNS)
	&& (j2 >= 0) && (j2 <= gv.abottombound)
	&& (gv.aliens[i2][j2].type != NOALIEN))
	{
		hitalien |= checkalien(brectptr, i2, j2);
	}

	return hitalien;
}


void dobullets(void)
{
	rect brect;
	Boolean removeflag;
	UInt8 i;
	
	for(i = 0; i < gv.nbullets; i++)
	{
		removeflag = false;
	
		gv.bullets[i].x += gv.bullets[i].dx;
		gv.bullets[i].y += gv.bullets[i].dy;

		brect.x			= gv.bullets[i].x>>FRACBITS;
		brect.y			= gv.bullets[i].y>>FRACBITS;
		brect.width		= getbulletwidth(gv.bullets[i].type);
		brect.height	= getbulletheight(gv.bullets[i].type);
		brect.corner	= getbulletcorner(gv.bullets[i].type);

		if((gv.bullets[i].x>>FRACBITS > BRIGHTEDGE)
		|| (gv.bullets[i].x>>FRACBITS < BLEFTEDGE)
		|| (gv.bullets[i].y>>FRACBITS < BTOPEDGE)
		|| (gv.bullets[i].y>>FRACBITS > BBOTTOMEDGE))
		{
			removeflag = true;
		}
		else if(isplayerbullet(gv.bullets[i].type))
		{
			brect.data = getbulletdamage(gv.bullets[i].type);

			if(checkallaliens(&brect))
			{
				removeflag = true;
			}
			else if((brect.y < BLOCK_TOP+BLOCK_ROWS*BLOCKHEIGHT)
			&& (brect.y+brect.height > BLOCK_TOP)
			&& (checkblocks(&brect)))
			{
				removeflag = true;
			}
		}
		else if(isalienbullet(gv.bullets[i].type))
		{
			brect.data = getbulletdamage(gv.bullets[i].type);

			if((brect.y+brect.height > PLAYERY)
			&& (checkplayer(&brect)))
			{
				removeflag = true;
			}
			else if((brect.y < BLOCK_TOP+BLOCK_ROWS*BLOCKHEIGHT)
			&& (brect.y+brect.height > BLOCK_TOP)
			&& (checkblocks(&brect)))
			{
				removeflag = true;
			}
		}
		else if(isbonus(gv.bullets[i].type))
		{
			brect.data = gv.bullets[i].type;

			if((brect.y+brect.height > PLAYERY)
			&& (checkplayer(&brect)))
			{
				removeflag = true;
			}
		}

		if(removeflag)
		{
			removebullet(i--);
		}
	}
}


void addeffect(const Int16 x, const Int16 y, const UInt8 type)
{
	if(gv.neffects < MAXEFFECTS)
	{
		if(iseffectreverse(type))
		{
			gv.effects[gv.neffects].frame = geteffectendframe(type);
		}
		else
		{
			gv.effects[gv.neffects].frame = geteffectstartframe(type);
		}

		gv.effects[gv.neffects].type	= type;
		gv.effects[gv.neffects].x		= x;
		gv.effects[gv.neffects++].y		= y;
	}
}


void removeeffect(UInt8 index)
{
	gv.neffects--;

	while(index < gv.neffects)
	{
		gv.effects[index] = gv.effects[(index++)+1];
	}
}


void addbullet(const Int16 x, const Int16 y, const UInt8 type, const UInt8 dir)
{
	if(gv.nbullets < MAXBULLETS)
	{
		gv.bullets[gv.nbullets].type	= type;
		gv.bullets[gv.nbullets].frame	= getbulletstartframe(type);
		gv.bullets[gv.nbullets].x		= x<<FRACBITS;
		gv.bullets[gv.nbullets].y		= y<<FRACBITS;

		gv.bullets[gv.nbullets].dx		= (cosines[(dir)&31]*getbulletspeed(type))>>TRIGBITS;
		gv.bullets[gv.nbullets++].dy	= (cosines[(dir-8)&31]*getbulletspeed(type))>>TRIGBITS;
	}
}



/*************************************************
Put a new bullet at the start of the bullet array.
*************************************************/
void insertbullet(const Int16 x, const Int16 y, const UInt8 type, const UInt8 dir)
{
	if(gv.nbullets < MAXBULLETS)
	{
		UInt8 i;

		for(i = gv.nbullets; i > 0; i--)
		{
			gv.bullets[i] = gv.bullets[i-1];
		}

		gv.bullets[0].type	= type;
		gv.bullets[0].frame	= getbulletstartframe(type);
		gv.bullets[0].x		= x<<FRACBITS;
		gv.bullets[0].y		= y<<FRACBITS;

		gv.bullets[0].dx	= (cosines[(dir)&31]*getbulletspeed(type))>>TRIGBITS;
		gv.bullets[0].dy	= (cosines[(dir-8)&31]*getbulletspeed(type))>>TRIGBITS;

		gv.nbullets++;
	}
}


void removebullet(UInt8 index)
{
	gv.nbullets--;

	while(index < gv.nbullets)
	{
		gv.bullets[index] = gv.bullets[(index++)+1];
	}
}


void addalienbullets(void)
{
	Int8	i = (gv.counter>>1)%COLUMNS;
	Int8	j;
	UInt16	sum = 0;
	UInt16	critval;
	const UInt16 cvals[] = {400, 220, 150, 95};

	if(gv.difficulty > diff_normal)
	{
		j = ROWS-1;
	}
	else
	{
		j = gv.abottombound;
	}

	critval	= SysRandom(0)%(cvals[gv.difficulty]);

	do
	{
		if((gv.aliens[i][j].type != NOALIEN)
		&& ((gv.alieny>>FRACBITS) + j*ALIENHEIGHT > 0)
		&& (gv.nbullets < MAXBULLETS - 10))				//leave some bullets for the player
		{
			sum += getalienfireval(gv.aliens[i][j].type);

			if(sum > critval)
			{
				//UInt8 sound		= getaliensound(gv.aliens[i][j].type);
				UInt8 bullet	= getalienbullet(gv.aliens[i][j].type);
				UInt8 nbullets	= 1;
				UInt8 dir		= DOWN;
				UInt8 deldir	= 0;
				Int16 x, y;

				switch(gv.aliens[i][j].type)
				{	
					case alien_dumbell:
						if(SysRandom(0) & 512)
						{
							nbullets	= 3;
							dir			= DOWN-1;
							deldir		= 1;
						}
						break;

					case alien_boss:
						nbullets	= 3;
						dir			= DOWN-1;
						deldir		= 1;

					case alien_3eye:
					case alien_squiddy:
					case alien_cube:
					case alien_tailguy:
					{
						Int16 dx = (gv.alienx>>FRACBITS) +
							i*ALIENWIDTH - gv.player.x;
						Int16 dy = BUFFERHEIGHT -
							(gv.alieny>>FRACBITS) - (i+1)*ALIENHEIGHT;

						if(dx > 0)
						{
							if(dx > (dy>>1))
							{
								dir += 2;
							}
						}
						else
						{
							if(-dx > (dy>>1))
							{
								dir -= 2;
							}
						}
						break;
					}

					case alien_armdude:
					case alien_hex:
						nbullets = 2;
						break;
				}

				x = (gv.alienx>>FRACBITS) + i*ALIENWIDTH +
					((ALIENWIDTH - (nbullets-1)*BULLETWIDTH -
					getbulletwidth(bullet))>>1);

				y = (gv.alieny>>FRACBITS) + j*ALIENHEIGHT +
					getalienyoffset(gv.aliens[i][j].type) +
					getalienheight(gv.aliens[i][j].type);

				while(nbullets-- > 0)
				{	
					addbullet(x, y, makealienbullet(bullet), dir);
					x	+= BULLETWIDTH;
					dir	+= deldir;
				}

				//setsound(sound);
				break;
			}			
		}
		else
		{
			sum += DEFAULTFIREVAL;
		}
	}while(--j > 0);
}


void movealiens(void)
{
	Int16 moveamount	= 0;
	UInt8 minmove		= 6+(gv.difficulty>>1);

	if(gv.naliens)
	{
		moveamount = (152+(gv.difficulty<<3)) / gv.naliens;
	}

	if(moveamount < minmove)
	{
		moveamount = minmove;
	}

	gv.alienframecount += moveamount+4;

	switch(gv.aliendir)
	{
		case dir_right:
			if(((gv.alienx + moveamount)>>FRACBITS) +
			(gv.arightbound+1)*ALIENWIDTH > ARIGHTEDGE)
			{
				gv.alienx = (ARIGHTEDGE - (gv.arightbound+1)*ALIENWIDTH)<<FRACBITS;
				gv.aliendir = dir_downr;
			}
			break;

		case dir_left:
			if(((gv.alienx - moveamount)>>FRACBITS) +
			gv.aleftbound*ALIENWIDTH < ALEFTEDGE)
			{
				gv.alienx = (ALEFTEDGE - gv.aleftbound*ALIENWIDTH)<<FRACBITS;
				gv.aliendir = dir_downl;
			}
			break;

		case dir_downr:
		case dir_downl:
			if(gv.alieny + moveamount > gv.alienstopy)
			{
				gv.alieny = gv.alienstopy;
				gv.alienstopy += ALIENHEIGHT<<FRACBITS;
				gv.aliendir = (gv.aliendir+1)%4;

				if((gv.alieny>>FRACBITS) +
				(gv.abottombound+1)*ALIENHEIGHT >= ABOTTOMEDGE)
				{
					setsound(sound_land);
					setstate(state_lose);
				}
			}
	}

	switch(gv.aliendir)
	{
		case dir_right:
			gv.alienx += moveamount;
			break;

		case dir_left:
			gv.alienx -= moveamount;
			break;

		case dir_downl:
		case dir_downr:
			gv.alieny += moveamount;
	}

	if(((gv.alieny>>FRACBITS)+(gv.abottombound+1)*ALIENHEIGHT > BLOCK_TOP)
	&& (gv.counter & 1))
	{
		UInt8 n, i, j;
		Boolean rowflag = true;

		for(j = 0; (j < BLOCK_ROWS) && (rowflag); j++)
		{
			for(n = 0; n < BLOCK_GROUPS; n++)
			{
				for(i = 0; i < BLOCK_COLUMNS; i++)
				{
					if(gv.blocks[n][i][j] != NOBLOCK)
					{
						rowflag = false;
						gv.blocks[n][i][j]--;
					}
				}
			}
		}
	}
}


/***********************************************
Find the square root of a (large) number...
************************************************/
UInt16 sqroot(const UInt32 hyp)
{
	UInt32 sqrt = 0;
	UInt32 jump = 0x8000;

	while(jump)
	{
		if(hyp > sqrt*sqrt)
		{
			sqrt += jump;
		}
		else if(hyp < sqrt*sqrt)
		{
			sqrt -= jump;
		}
		else
		{
			break;
		}
		jump >>= 1;
	}

	return (UInt16)sqrt;
}


void runspecial(void)
{
	switch(gv.player.special)
	{
		case special_seeker:
			if((gv.nbullets == 0)
			|| ((gv.bullets[0].type & BTMASK) != bullet_seeker))
			{
				//check if the seeker was destroyed
				gv.player.specialtimer = 1;
			}
			else if((!(gv.counter & 3))
			&& (gv.player.targeti != NOPTARGET)
			&& (gv.player.targetj != NOPTARGET)
			&& ((gv.bullets[0].y>>FRACBITS < BLOCK_TOP + BLOCKHEIGHT/2)
			|| ((gv.alieny>>FRACBITS)+(gv.abottombound+2)*ALIENHEIGHT > BLOCK_TOP)))
			{
				//distance to target
				Int16 tdx = (gv.alienx>>FRACBITS) + gv.player.targeti*ALIENWIDTH +
					ALIENWIDTH/2 - (gv.bullets[0].x>>FRACBITS) - BULLETWIDTH/2;
				Int16 tdy = (gv.alieny>>FRACBITS) + gv.player.targetj*ALIENHEIGHT +
					ALIENHEIGHT/2 - (gv.bullets[0].y>>FRACBITS) - BULLETHEIGHT/2;

				Int16 cross = gv.bullets[0].dx * tdy - gv.bullets[0].dy * tdx;

				if(cross > 0)
				{
					//target is on the right
					gv.player.specialdata++;
				}
				else if(cross < 0)
				{
					//or the left
					gv.player.specialdata--;
				}

				gv.bullets[0].dx = (cosines[(gv.player.specialdata)&31] *
					getbulletspeed(bullet_seeker))>>TRIGBITS;
				gv.bullets[0].dy = (cosines[(gv.player.specialdata-8)&31] *
					getbulletspeed(bullet_seeker))>>TRIGBITS;
			}	
			break;

		case special_repel:
		{
			Int16 dx, dy;
			UInt8 i;

			for(i = gv.counter&3; i < gv.nbullets; i += 4)
			{
				dx = gv.player.x + PLAYERWIDTH/2 -
					(gv.bullets[i].x>>FRACBITS) - BULLETWIDTH/2;
				dy = PLAYERY + PLAYERHEIGHT/4 -
					(gv.bullets[i].y>>FRACBITS) - BULLETHEIGHT/2;

				if((dx > -REPELRANGE)
				&& (dx < REPELRANGE)
				&& (dy < REPELRANGE)
				&& (isalienbullet(gv.bullets[i].type)))
				{
					Int32 distsq	= dx*dx + dy*dy;
					Int32 dist		= sqroot(distsq);
					Int32 acc		= -REPELPOWER / distsq;
	
					gv.bullets[i].dx += acc * dx / dist;
					gv.bullets[i].dy += acc * dy / dist;
				}
			}

			if(!(gv.player.specialtimer%26))
			{
				setsound(sound_repel);
			}
			break;
		}

		case special_shield:
			if(gv.player.shield < MAXSHIELD-2)
			{
				gv.player.shield += 3;
			}
			else if(gv.player.shield < MAXSHIELD)
			{
				gv.player.shield = MAXSHIELD;
			}

			drawinfo(info_shield);
			break;
		
		case special_barrier:
			if(gv.counter & 1)
			{
				Int8 i, j;
				Boolean rowflag = true;

				for(j = BLOCK_ROWS-1; (j >= 0) && (rowflag); j--)
				{
					for(i = 0; i < BLOCK_COLUMNS; i++)
					{
						if(gv.blocks[gv.player.specialdata][i][j] != NBLOCKFRAMES-1)
						{
							rowflag = false;
							gv.blocks[gv.player.specialdata][i][j]++;
						}
					}
				}
			}
	}

	if(!(--gv.player.specialtimer))
	{
		gv.player.frame -= 2;
	}
}


void dospecial(void)
{
	Boolean success = true;

	switch(gv.player.special)
	{
		case special_bomb:
			if(gv.nbullets < MAXBULLETS)
			{
				insertbullet(gv.player.x + ((PLAYERWIDTH -
					getbulletwidth(bullet_bomb))>>1),
					PLAYERY - getbulletheight(bullet_bomb),
					makeplayerbullet(bullet_bomb), UP);

				setsound(sound_missile);
			}
			else
			{
				success = false;
			}
			break;

		case special_seeker:
			if(gv.nbullets < MAXBULLETS)
			{
				gv.player.specialdata = UP;

				insertbullet(gv.player.x + ((PLAYERWIDTH -
					getbulletwidth(bullet_seeker))>>1),
					PLAYERY - getbulletheight(bullet_seeker),
					makeplayerbullet(bullet_seeker), UP);

				if(gv.naliens)
				{
					gv.player.targeti = gv.aleftbound +
						SysRandom(0) % (gv.arightbound-gv.aleftbound+1);
					gv.player.targetj = SysRandom(0) % (gv.abottombound+1);

					while(gv.aliens[gv.player.targeti][gv.player.targetj].type == NOALIEN)
					{
						if(++gv.player.targeti > gv.arightbound)
						{
							gv.player.targeti = gv.aleftbound;
	
							if(++gv.player.targetj > gv.abottombound)
							{
								gv.player.targetj = 0;
							}
						}
					}	
				}
				else
				{
					gv.player.targeti = NOPTARGET;
					gv.player.targetj = NOPTARGET;
				}

				setsound(sound_missile);
			}
			else
			{
				success = false;
			}
			break;

		case special_randtel:
			addeffect(gv.player.x, PLAYERY, effect_teleport);

			gv.player.x = (SysRandom(0)%(SCREENWIDTH-PLAYERWIDTH) + SCREENX) & ~1;

			if(prefs.settings.pencontrol == pc_movement)
			{
				gv.pentarget = NOTARGET;
			}

			setsound(sound_teleport);
			break;

		case special_teleport:
			addeffect(gv.player.x, PLAYERY, effect_teleport);

			if(gv.player.x + PLAYERWIDTH/2 >= SCREENX + SCREENWIDTH/2)
			{
				gv.player.x = SCREENX + 16;
			}
			else
			{
				gv.player.x = SCREENX + SCREENWIDTH - PLAYERWIDTH - 16;
			}

			if(prefs.settings.pencontrol == pc_movement)
			{
				gv.pentarget = NOTARGET;
			}

			setsound(sound_teleport);
			break;

		case special_barrier:
			if(gv.player.x + PLAYERWIDTH/2 < BLOCK_LEFT0 +
			BLOCKWIDTH*BLOCK_COLUMNS + (BLOCK_LEFT1 - BLOCK_LEFT0 -
			BLOCKWIDTH*BLOCK_COLUMNS)/2)
			{
				gv.player.specialdata = 0;
			}
			else if(gv.player.x + PLAYERWIDTH/2 > BLOCK_LEFT1 +
			BLOCKWIDTH*BLOCK_COLUMNS + (BLOCK_LEFT2 - BLOCK_LEFT1 -
			BLOCKWIDTH*BLOCK_COLUMNS)/2)
			{
				gv.player.specialdata = 2;
			}
			else
			{
				gv.player.specialdata = 1;
			}

			setsound(sound_fixbarrier);
			break;
	
		case special_shield:
			if(gv.player.shield == MAXSHIELD)
			{
				success = false;
			}
			else
			{
				setsound(sound_reshield);
			}
	}

	if(success)
	{
		gv.player.specialtimer	= getspecialtimer(gv.player.special);
		gv.player.power			-= getspecialdrain(gv.player.special);
		gv.player.frame 		+= 2;
		drawinfo(info_power);
	}
}


void doplayer(void)
{
	UInt32	keystate	= KeyCurrentState();
	Int8	moveamount	= 0;
	Boolean	fireflag	= false;
	Boolean specialflag	= false;
	Int8	i;

	if((gv.player.shield < CRIT_SHIELD)
	&& !(gv.counter & 15)
	&& (SysRandom(0)%(CRIT_SHIELD*3/2) >= gv.player.shield))
	{
		blowup_player(1);
	}

	if(gv.player.firetimer)
	{
		if(--gv.player.firetimer == getweapontimer(gv.player.weapon) - FIREFRAMETIME)
		{
			gv.player.frame--;
		}
	}
	if(gv.player.specialtimer)
	{
		runspecial();
	}

	//deal with jog dial control
	if(gv.jog == JOGFIRE)
	{
		fireflag = true;
	}
	else if(gv.jog != NOJOG)
	{
		moveamount = gv.jog;
		gv.jog = NOJOG;
	}

	//then deal with pen control

	if(gv.pentarget == FIRETARGET)
	{
		fireflag = true;
		//gv.pentarget = NOTARGET;
	}
	else if(gv.pentarget != NOTARGET)
	{
		if(gv.player.x > gv.pentarget+1)
		{
			moveamount = -2;
		}
		else if(gv.player.x > gv.pentarget)
		{
			moveamount = -1;
		}
		else if(gv.player.x < gv.pentarget-1)
		{
			moveamount = 2;
		}
		else if(gv.player.x < gv.pentarget)
		{
			moveamount = 1;
		}
	}

	//and then key control
	for(i = gettopkey(); i >= 0; i--)
	{
		if(keystate & keyvals[i])
		{
			switch(prefs.settings.keyactions[i])
			{
				case action_right:
					if(prefs.settings.pencontrol == pc_movement)
					{
						gv.pentarget = NOTARGET;
					}
					moveamount = 2;
					break;
				case action_left:
					if(prefs.settings.pencontrol == pc_movement)
					{
						gv.pentarget = NOTARGET;
					}
					moveamount = -2;
					break;
				case action_fire:
					fireflag = true;
					break;
				case action_special:
					specialflag = true;
			}
		}
	}

	//perform some actions if any of the above say so
	if((specialflag)
	&& (!gv.player.specialtimer)
	&& (gv.player.power >= getspecialdrain(gv.player.special)))
	{
		dospecial();
	}

	if((fireflag)
	&& (!gv.player.firetimer))
	{
		gv.player.frame++;
		addplayerbullets();
	}

	if(gv.player.x + moveamount < SCREENX)
	{
		gv.player.x = SCREENX;
	}
	else if(gv.player.x + moveamount >= SCREENX + SCREENWIDTH - PLAYERWIDTH)
	{
		gv.player.x = SCREENX + SCREENWIDTH - PLAYERWIDTH;
	}
	else
	{
		gv.player.x += moveamount;
	}
}


void addplayerbullets(void)
{
	UInt8 sound		= getweaponsound(gv.player.weapon);
	UInt8 bullet	= getweaponbullet(gv.player.weapon);
	UInt8 nbullets	= getweaponbcount(gv.player.weapon);
	UInt8 dir		= UP;
	Int8 deldir		= 0;
	Int16 bx;

	gv.player.firetimer += getweapontimer(gv.player.weapon);

	switch(gv.player.weapon)
	{
		case weapon_plasma:
			if(++(gv.player.firedata) & 1)
			{
				dir = UP + (gv.player.firedata&3) - 2;
				deldir = ((gv.player.firedata&3)-2) * (-2);
			}

			/********  old effect - was annoying to use
			deldir = (SysRandom(0)>>12)&3;
			if(deldir & 1)
			{
				dir = UP + deldir - 2;
			}
			*********/
			break;
		case weapon_auto:
			if((++gv.player.firedata) & 1)
			{
				dir = UP + (gv.player.firedata&3) - 2; 
			}
			break;
		case weapon_war:
			if((++gv.player.firedata) & 1)
			{
				bullet		= bullet_star;
				dir			= UP - 1;
				deldir		= 2;
			}
	}

	bx = gv.player.x + ((PLAYERWIDTH -
		(nbullets-1)*BULLETWIDTH - getbulletwidth(bullet))>>1);

	while(nbullets-- > 0)
	{	
		addbullet(bx,
			PLAYERY - getbulletheight(bullet),
			makeplayerbullet(bullet),
			dir);

		bx	+= BULLETWIDTH;
		dir	+= deldir;
	}

	setsound(sound);
}


void initblocks(void)
{
	UInt8 n, i, j;

	for(n = 0; n < BLOCK_GROUPS; n++)
	{
		for(j = 0; j < BLOCK_ROWS; j++)
		{
			for(i = 0; i < BLOCK_COLUMNS; i++)
			{
				if(gv.difficulty == diff_nightmare)
				{
					gv.blocks[n][i][j] = SysRandom(0)%NBLOCKFRAMES;
				}
				else
				{
					gv.blocks[n][i][j] = NBLOCKFRAMES-1;
				}
			}
		}
	}
}


void loadalien(const UInt8 i, const UInt8 j, const UInt8 type)
{
	gv.aliens[i][j].type = type;

	if(gv.aliens[i][j].type != NOALIEN)
	{
		gv.aliens[i][j].hp = getalienhp(gv.aliens[i][j].type);

		if(i > gv.arightbound)
		{
			gv.arightbound = i;
		}
		if(i < gv.aleftbound)
		{
			gv.aleftbound = i;
		}
		if(j > gv.abottombound)
		{
			gv.abottombound = j;
		}

		gv.naliens++;
	}
}


void loadwave(void)
{
	MemHandle wavehand;
	UInt8* waveptr;	
	UInt8 i, j;

	gv.aleftbound		= 7;
	gv.arightbound		= 0;
	gv.abottombound		= 0;
	gv.naliens			= 0;
	gv.alienframecount	= 0;

	wavehand	= DmGetResource(waverc, WAVEBASE+gv.wave);
	waveptr		= MemHandleLock(wavehand);

	for(j = 0; j < ROWS; j++)
	{
		for(i = 0; i < COLUMNS; i++)
		{
			loadalien(i, j, waveptr[j*COLUMNS+i]);
		}
	}

	MemHandleUnlock(wavehand);
	DmReleaseResource(wavehand);

	gv.alieny		= (ALIENSTARTY - gv.abottombound*ALIENHEIGHT)<<FRACBITS;
	gv.alienstopy	= SCREENY<<FRACBITS;

	if(SysRandom(0) & 1)
	{
		gv.alienx	= (ALIENSTARTX_LEFT - gv.aleftbound*ALIENWIDTH)<<FRACBITS;
		gv.aliendir	= dir_downl;
	}
	else
	{
		gv.alienx	= (ALIENSTARTX_RIGHT - (gv.arightbound+1)*ALIENWIDTH)<<FRACBITS;
		gv.aliendir	= dir_downr;
	}
}


void loadinfo(void)
{
	MemHandle memhand;

	memhand			= DmGetResource(inforc, info_aliens);
	info.aliens		= (type_alieninfo*)MemHandleLock(memhand);

	memhand			= DmGetResource(inforc, info_bullets);
	info.bullets	= (type_bulletinfo*)MemHandleLock(memhand);

	memhand			= DmGetResource(inforc, info_effects);
	info.effects	= (type_effectinfo*)MemHandleLock(memhand);

	memhand			= DmGetResource(inforc, info_weapons);
	info.weapons	= (type_weaponinfo*)MemHandleLock(memhand);

	memhand			= DmGetResource(inforc, info_specials);
	info.specials	= (type_specialinfo*)MemHandleLock(memhand);
}


void freeinfo(void)
{
	MemHandle memhand;

	if(info.aliens)
	{
		memhand = MemPtrRecoverHandle(info.aliens);
		MemHandleUnlock(memhand);
		DmReleaseResource(memhand);
	}

	if(info.bullets)
	{
		memhand = MemPtrRecoverHandle(info.bullets);
		MemHandleUnlock(memhand);
		DmReleaseResource(memhand);
	}

	if(info.effects)
	{
		memhand = MemPtrRecoverHandle(info.effects);
		MemHandleUnlock(memhand);
		DmReleaseResource(memhand);
	}

	if(info.weapons)
	{
		memhand = MemPtrRecoverHandle(info.weapons);
		MemHandleUnlock(memhand);
		DmReleaseResource(memhand);
	}

	if(info.specials)
	{
		memhand = MemPtrRecoverHandle(info.specials);
		MemHandleUnlock(memhand);
		DmReleaseResource(memhand);
	}
}