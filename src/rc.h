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

#define alert_error 		3000
#define alert_rconf			3001
#define alert_incompat		3002
#define alert_clearhs		3003
#define alert_nohsid		3004
#define alert_badcode		3005
#define alert_goodcode		3006
#define alert_needreg		3007
#define alert_clearrecords	3008
#define alert_hscode		3009
#define alert_hserror		3010

#define OK					10
#define CANCEL				11
#define DEFAULTS			12
#define DONE				13

#define string_prset		6000
#define string_gaset		6001
#define string_keys			6002
#define string_hs			6003

#define menu_main			7600
#define menuitem_retire		7601
#define menuitem_pause		7602
#define menuitem_settings	7603
#define menuitem_keys		7604
#define menuitem_scores		7605
#define menuitem_about		7606
#define menuitem_register	7607
#define menuitem_quit		7608
#define menuitem_records	7609
#define menu_title			7610
#define menuitem_start		7611
#define menu_pause			7620
#define menuitem_resume		7621

#define form_main			100

#define form_title				200
#define button_titlestart		210
#define button_titlesettings	211
#define button_titlekeys		212
#define button_titlescores		213
#define button_titlerecords		214
#define label_regcheck			220
#define REGCHECKY				144
#define REGCHECKX				53

#define form_keys			300
#define button_datebook		310
#define button_address		311
#define button_todolist		312
#define button_memopad		313
#define button_pageup		314
#define button_pagedown		315
#define button_nav_up		316
#define button_nav_down		317
#define button_nav_left		318
#define button_nav_right	319
#define button_nav_select	320
#define label_datebook		330
#define label_address		331
#define label_todolist		332
#define label_memopad		333
#define label_pageup		334
#define label_pagedown		335
#define label_left			336
#define label_right			337
#define label_select		338
#define checkbox_none		350
#define checkbox_left		351
#define checkbox_right		352
#define checkbox_fire		353
#define checkbox_special	354
#define checkbox_pause		355
#define list_keys			360
#define popup_keys			361
#define checkbox_kevents	362

#define form_pause			400
#define button_pauseresume	410
#define label_pausewave		420

#define form_settings		500
#define checkbox_rconf		510
#define checkbox_qrest		511
#define checkbox_graf		512
#define checkbox_compat		513
#define popup_pen			520
#define list_pen			521
#define popup_volume		522
#define list_volume			523

#define form_restoring		600

#define form_die			700

#define form_diehs			800
#define field_highscore		801

#define form_highscores		900
#define scrollbar_hs		901
#define list_hsdiffs		902
#define popup_hsdiffs		903
#define button_hsclear		904
#define button_hscode		905
#define HSWINWIDTH			138
#define HSWINHEIGHT			260
#define HSPAGESIZE			91

#define form_shop			1000
#define button_buy			1001
#define list_shopview		1002
#define popup_shopview		1003
#define label_money			1004
#define label_trade			1005
#define button_more			1006
#define SHOPLISTBASE		1010
#define NSHOPLISTS			2
#define list_weapons		1010
#define list_weaponprices	1020
#define list_equip			1011
#define list_equipprices	1021
#define shoplist_weapons	0
#define shoplist_equip		1

#define form_about			1100
#define bmp_formicon		1101

#define form_wingame		1200
#define form_wingamehs		1300

#define form_score			1400
#define SCORELABELX			112
#define SCORELABELY			15
#define label_wavesx100		1401
#define label_credits		1402
#define label_weaponval		1403
#define label_equipval		1404
#define label_penalties		1405
#define label_total			1406
#define button_retry		1410
#define button_retire		1411

#define form_register		1500
#define label_hotsyncid		1501
#define HSIDY				73
#define field_regcode		1502
#define MAXREGCODELEN		12

#define form_records		1600
#define list_diffs			1601
#define popup_diffs			1602
#define label_topwave		1603
#define button_waveplus		1604
#define button_waveminus	1605
#define label_startwave		1606
#define button_rclear		1607
#define label_startweapons	1620
#define label_startequip	1630
#define topwavex			118
#define topwavey			33
#define startwavex			96
#define startwavey			49


#define bmp_datebook		5000
#define bmp_address			5001
#define bmp_todolist		5002
#define bmp_memopad			5003
#define bmp_pageup			5004
#define bmp_pagedown		5005
#define bmp_nav_left		5006
#define bmp_nav_right		5007
#define bmp_nav_up			5008
#define bmp_nav_down		5009
#define bmp_nav_select		5010

#define bmp_datebook_alt	5100
#define bmp_address_alt		5101
#define bmp_todolist_alt	5102
#define bmp_memopad_alt		5103
#define bmp_pageup_alt		5104
#define bmp_pagedown_alt	5105
#define bmp_nav_left_alt	5106
#define bmp_nav_right_alt	5107
#define bmp_nav_up_alt		5108
#define bmp_nav_down_alt	5109
#define bmp_nav_select_alt	5110


#define NALIENFRAMES			30
#define bmp_alien_base			2000
#define bmp_alien_conehead0		2000
#define bmp_alien_conehead1		2001
#define bmp_alien_floop0		2002
#define bmp_alien_floop1		2003
#define bmp_alien_bigeye0		2004
#define bmp_alien_bigeye1		2005
#define bmp_alien_3eye0			2006
#define bmp_alien_3eye1			2007
#define bmp_alien_bigdude0		2008
#define bmp_alien_bigdude1		2009
#define bmp_alien_hex0			2010
#define bmp_alien_hex1			2011
#define bmp_alien_squiddy0		2012
#define bmp_alien_squiddy1		2013
#define bmp_alien_cube0			2014
#define bmp_alien_cube1			2015
#define bmp_alien_diamond0		2016
#define bmp_alien_diamond1		2017
#define bmp_alien_armdude0		2018
#define bmp_alien_armdude1		2019
#define bmp_alien_tailguy0		2020
#define bmp_alien_tailguy1		2021
#define bmp_alien_dumbell0		2022
#define bmp_alien_dumbell1		2023
#define bmp_alien_boss0			2024
#define bmp_alien_boss1			2025
#define bmp_effect_explode0		2026
#define bmp_effect_explode1		2027
#define bmp_effect_explode2		2028
#define bmp_effect_explode3		2029

#define mask_alien_base			2100
#define mask_alien_conehead0	2100
#define mask_alien_conehead1	2101
#define mask_alien_floop0		2102
#define mask_alien_floop1		2103
#define mask_alien_bigeye0		2104
#define mask_alien_bigeye1		2105
#define mask_alien_3eye0		2106
#define mask_alien_3eye1		2107
#define mask_alien_bigdude0		2108
#define mask_alien_bigdude1		2109
#define mask_alien_hex0			2110
#define mask_alien_hex1			2111
#define mask_alien_squiddy0		2112
#define mask_alien_squiddy1		2113
#define mask_alien_cube0		2114
#define mask_alien_cube1		2115
#define mask_alien_diamond0		2116
#define mask_alien_diamond1		2117
#define mask_alien_armdude0		2118
#define mask_alien_armdude1		2119
#define mask_alien_tailguy0		2120
#define mask_alien_tailguy1		2121
#define mask_alien_dumbell0		2122
#define mask_alien_dumbell1		2123
#define mask_alien_boss0		2124
#define mask_alien_boss1		2125
#define mask_effect_explode0	2126
#define mask_effect_explode1	2127
#define mask_effect_explode2	2128
#define mask_effect_explode3	2129

#define alien_conehead			0
#define alien_floop				1
#define alien_bigeye			2
#define alien_3eye				3
#define alien_bigdude			4
#define alien_hex				5
#define alien_squiddy			6
#define alien_cube				7
#define alien_diamond			8
#define alien_armdude			9
#define alien_tailguy			10
#define alien_dumbell			11
#define alien_boss				12


#define NBULLETFRAMES			25
#define bmp_bullet_base			2200
#define bmp_bullet_wimpy		2200
#define bmp_bullet_greenzap0	2201
#define bmp_bullet_greenzap1	2202
#define bmp_bonus_shield		2203
#define bmp_bonus_money			2204
#define bmp_bonus_power			2205
#define bmp_bonus_money2		2206
#define bmp_bullet_blob0		2207
#define bmp_bullet_blob1		2208
#define bmp_bullet_blob2		2209
#define bmp_bullet_zap			2210
#define bmp_bullet_greenshot0	2211
#define bmp_bullet_greenshot1	2212
#define bmp_bullet_ray			2213
#define bmp_bullet_pointy		2214
#define bmp_bullet_star0		2215
#define bmp_bullet_star1		2216
#define bmp_bullet_spinner0		2217
#define bmp_bullet_spinner1		2218
#define bmp_bullet_plasma0		2219
#define bmp_bullet_plasma1		2220
#define bmp_bullet_plasma2		2221
#define bmp_bullet_seeker0		2222
#define bmp_bullet_seeker1		2223
#define bmp_bullet_bomb			2224

#define mask_bullet_base		2300
#define mask_bullet_wimpy		2300
#define mask_bullet_greenzap0	2301
#define mask_bullet_greenzap1	2302
#define mask_bonus_shield		2303
#define mask_bonus_money		2304
#define mask_bonus_power		2305
#define mask_bonus_money2		2306
#define mask_bullet_blob0		2307
#define mask_bullet_blob1		2308
#define mask_bullet_blob2		2309
#define mask_bullet_zap			2310
#define mask_bullet_greenshot0	2311
#define mask_bullet_greenshot1	2312
#define mask_bullet_ray			2313
#define mask_bullet_pointy		2314
#define mask_bullet_star0		2315
#define mask_bullet_star1		2316
#define mask_bullet_spinner0	2317
#define mask_bullet_spinner1	2318
#define mask_bullet_plasma0		2319
#define mask_bullet_plasma1		2320
#define mask_bullet_plasma2		2321
#define mask_bullet_seeker0		2322
#define mask_bullet_seeker1		2323
#define mask_bullet_bomb		2324

#define bullet_wimpy			0
#define bullet_greenzap			1
#define bullet_bonus_base		2
#define bullet_bonus_shield		2
#define bullet_bonus_money		3
#define bullet_bonus_power		4
#define bullet_bonus_money2		5
#define bullet_blob				6
#define bullet_zap				7
#define bullet_greenshot		8
#define bullet_ray				9
#define bullet_pointy			10
#define bullet_star				11
#define bullet_spinner			12
#define bullet_plasma			13
#define bullet_seeker			14
#define bullet_bomb				15


#define NPLAYERFRAMES		11
#define bmp_player_base		2400
#define bmp_playerform0		2400
#define bmp_playerform1		2401
#define bmp_playerform2		2402
#define bmp_playerform3		2403
#define bmp_playerform4		2404
#define bmp_playerform5		2405
#define bmp_playerform6		2406
#define bmp_player			2407
#define bmp_playerfire		2408
#define bmp_playercharge	2409
#define bmp_playercf		2410

#define mask_player_base	2500
#define mask_playerform0	2500
#define mask_playerform1	2501
#define mask_playerform2	2502
#define mask_playerform3	2503
#define mask_playerform4	2504
#define mask_playerform5	2505
#define mask_playerform6	2506
#define mask_player			2507
#define mask_playerfire		2508
#define mask_playercharge	2509
#define mask_playercf		2510

#define playerframe_form0	0
#define playerframe_main	7
#define playerframe_fire	8


#define NBLOCKFRAMES		4
#define bmp_block0			2600
#define bmp_block1			2601
#define bmp_block2			2602
#define bmp_block3			2603

#define mask_block0			2700
#define mask_block1			2701
#define mask_block2			2702
#define mask_block3			2703

#define bmp_infobar			2810
#define bmp_title			2811


#define waverc				'WAVE'
#define WAVEBASE			0

#define NSOUNDS				23
#define soundrc				'PSND'
#define sound_start			0
#define sound_firewimpy		0
#define sound_firezap		1
#define sound_fireray		2
#define sound_firewar		3
#define sound_fireplasma	4
#define sound_bonus			5
#define sound_land			6
#define sound_boom			7
#define sound_die			8
#define sound_damage		9
#define sound_alienfirezap	10
#define sound_alienfireshot	11
#define sound_alienfireblob	12
#define sound_alienfirespin	13
#define sound_winlevel		14
#define sound_hitbarrier	15
#define sound_hitalien		16
#define sound_teleport		17
#define sound_fixbarrier	18
#define sound_reshield		19
#define sound_repel			20
#define sound_missile		21
#define sound_wingame		22

#define inforc				'INFO'
#define info_aliens			0
#define info_bullets		1
#define info_effects		2
#define info_weapons		3
#define info_specials		4