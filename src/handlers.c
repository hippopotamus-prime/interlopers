#include <PalmOS.h>
#include <DLServer.h>
#include "handlers.h"
#include "game.h"
#include "rc.h"
#include "gfx.h"
#include "prefs.h"
#include "sound.h"
#include "palm.h"
#include "title.h"

static Boolean handlemenus(UInt16 itemID);
static void hiscores_initscrollwindow(void);
static void hiscores_drawscrollwindow(ScrollBarPtr sbptr);
static void shop_drawmoney(void);
static void shop_updatebuybutton(void);
static void shop_drawtrade(Int16 shoplist);
static void shop_changelist(Int16 prevlist, Int16 newlist);
static void shop_freeprices(void);
static void shop_updateprices(void);
static void records_update(const UInt8 oldvalue, const UInt8 value,
	const UInt8 olddiff, const UInt8 diff);

static char* wprices[NWEAPONS];
static char* eprices[NSPECIALS];


Boolean handlemenus(UInt16 itemID)
{
	Boolean handled = false;

	switch(itemID)
	{
		case menuitem_start:
			FrmGotoForm(form_main);
			handled = true;
			break;

		case menuitem_retire:
			if((!(prefs.settings.flags & set_rconf))
			|| (!FrmAlert(alert_rconf)))
			{
				if(FrmGetFormId(FrmGetActiveForm()) == form_pause)
				{
					FrmReturnToForm(0);
				}

				setstate(state_retire);
			}
			handled = true;
			break;

		case menuitem_pause:
			FrmPopupForm(form_pause);
			handled = true;
			break;

		case menuitem_resume:
			//FrmEraseForm(FrmGetActiveForm());
			//I forget why that was there...
			FrmReturnToForm(0);
			handled = true;
			break;

		case menuitem_settings:
			FrmPopupForm(form_settings);
			handled = true;
			break;

		case menuitem_keys:
			FrmPopupForm(form_keys);
			handled = true;
			break;
	
		case menuitem_scores:
			FrmPopupForm(form_highscores);
			handled = true;
			break;

		case menuitem_records:
			FrmPopupForm(form_records);
			handled = true;
			break;

		case menuitem_about:
			FrmPopupForm(form_about);
			handled = true;
			break;

		case menuitem_register:
			FrmPopupForm(form_register);
			handled = true;
			break;

		case menuitem_quit:
		{
			EventType quitevent;

			quitevent.eType = appStopEvent;
			EvtAddEventToQueue(&quitevent);
			handled = true;
		}
	}

	return handled;
}


Boolean mainhandler(EventPtr eventptr)
{
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		FrmDrawForm(FrmGetActiveForm());

		if(!gfx.main.screenbuffer)
		{
			globalerr = loadgfxwin(&gfx.main.screenbuffer, BUFFERWIDTH, BUFFERHEIGHT, 1, 0);			
		}

		if(gv.gamestate & state_restore)
		{
			gv.gamestate &= ~state_restore;
			gv.gamestate |= state_pause;
			globalerr = loadbackground();
			drawinfobar();
			display();

			switch(gv.gamestate & INPLAYMASK)
			{
				default:
					FrmPopupForm(form_pause);
					break;
				case state_die:
					FrmPopupForm(form_die);
					break;
				case state_diehs:
					FrmPopupForm(form_diehs);
					break;
				case state_wingame:
					FrmPopupForm(form_wingame);
					break;
				case state_wingamehs:
					FrmPopupForm(form_wingamehs);
					break;
				case state_shop:
					FrmPopupForm(form_shop);
					break;
				case state_score:
					FrmPopupForm(form_score);
			}
		}
		else
		{
			initgame(prefs.settings.startwave[prefs.settings.difficulty]);
		}

		handled = true;
	}

	else if(eventptr->eType == frmUpdateEvent)
	{
		FrmDrawForm(FrmGetActiveForm());
		drawinfobar();

		handled = true;
	}

	else if(eventptr->eType == menuEvent)
	{
		handled = handlemenus(eventptr->data.menu.itemID);
	}

	return handled;
}


void title_regcheck(void)
{
	FormPtr formptr = FrmGetActiveForm();
	UInt16 regcheckid = FrmGetObjectIndex(formptr, label_regcheck);

	FrmHideObject(formptr, regcheckid);

	if((prefs.regcode)
	&& (checkregcode(prefs.regcode)))
	{
		FrmCopyLabel(formptr, label_regcheck, "registered");
		FrmSetObjectPosition(formptr, regcheckid, REGCHECKX, REGCHECKY);
	}

	FrmShowObject(formptr, regcheckid);
}


Boolean titlehandler(EventPtr eventptr)
{
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		title_regcheck();

		if(!(globalerr = inittitle()))
		{
			setstate(state_title);
			FrmDrawForm(FrmGetActiveForm());
			handled = true;
		}
	}
	else if(eventptr->eType == frmUpdateEvent)
	{
		title_regcheck();
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		switch(eventptr->data.ctlSelect.controlID)
		{
			case button_titlestart:
				FrmGotoForm(form_main);
				handled = true;
				break;

			case button_titlesettings:
				FrmPopupForm(form_settings);
				break;

			case button_titlerecords:
				FrmPopupForm(form_records);
				break;

			case button_titlekeys:
				FrmPopupForm(form_keys);
				break;

			case button_titlescores:
				FrmPopupForm(form_highscores);
				break;
		}
	}
	else if(eventptr->eType == menuEvent)
	{
		handled = handlemenus(eventptr->data.menu.itemID);
	}
	else if(eventptr->eType == frmCloseEvent)
	{
		setstate(state_none);
		closetitle();
	}

	return handled;
}


Boolean keyshandler(EventPtr eventptr)
{
	static UInt8 tempactions[NKEYS];

	Boolean handled = false;
	FormPtr formptr = FrmGetActiveForm();

	ListPtr keylistptr = FrmGetObjectPtr(formptr,	
		FrmGetObjectIndex(formptr, list_keys));
	ControlType* keypopupptr = FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, popup_keys));

	if(eventptr->eType == frmOpenEvent)
	{
		MemMove(tempactions, prefs.settings.keyactions, NKEYS);

		//the "select a key:" text
		FrmShowObject(formptr, FrmGetObjectIndex(formptr,
			prefs.settings.lastkey+label_datebook));

		//the "function:" popup
		LstSetSelection(keylistptr, tempactions[prefs.settings.lastkey]);
		CtlSetLabel(keypopupptr, LstGetSelectionText(keylistptr,
			tempactions[prefs.settings.lastkey]));

		//"allow key events" checkbox
		CtlSetValue(FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
			checkbox_kevents)), (prefs.settings.flags>>set_kevents_bit)&1);

		//all the button graphics
		if(device.flags & df_5way)
		{
			FrmSetObjectPosition(formptr, FrmGetObjectIndex(formptr,
				button_todolist), 104, 32);
			FrmSetObjectPosition(formptr, FrmGetObjectIndex(formptr,
				button_address), 32, 32);

			FrmHideObject(formptr, FrmGetObjectIndex(formptr,
				button_pageup));
			FrmHideObject(formptr, FrmGetObjectIndex(formptr,
				button_pagedown));

			FrmShowObject(formptr, FrmGetObjectIndex(formptr,
				button_nav_left));
			FrmShowObject(formptr, FrmGetObjectIndex(formptr,
				button_nav_right));
			FrmShowObject(formptr, FrmGetObjectIndex(formptr,
				button_nav_up));
			FrmShowObject(formptr, FrmGetObjectIndex(formptr,
				button_nav_down));
			FrmShowObject(formptr, FrmGetObjectIndex(formptr,
				button_nav_select));
		}

		FrmDrawForm(formptr);
		handled = true;
	}
	else if((eventptr->eType == popSelectEvent)
	&& (eventptr->data.popSelect.controlID == popup_keys))
	{
		tempactions[prefs.settings.lastkey] =
			eventptr->data.popSelect.selection;
		CtlSetLabel(keypopupptr, LstGetSelectionText(keylistptr,
			eventptr->data.popSelect.selection));
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		if((eventptr->data.ctlSelect.controlID >= button_datebook)
		&& (eventptr->data.ctlSelect.controlID <= button_nav_select))
		{
			UInt8 key = eventptr->data.ctlSelect.controlID - button_datebook;

			if(key >= (button_nav_up - button_datebook))
			{
				//shift the 5-way keys down by 2,
				//so nav up matches page up
				key -= 2;
			}
			
			FrmHideObject(formptr, FrmGetObjectIndex(formptr,
				prefs.settings.lastkey+label_datebook));

			LstSetSelection(keylistptr, tempactions[key]);
			CtlSetLabel(keypopupptr, LstGetSelectionText(keylistptr,
				tempactions[key]));

			FrmShowObject(formptr, FrmGetObjectIndex(formptr,
				key+label_datebook));

			prefs.settings.lastkey = key;
			prefs.checksum = getprefschecksum();

			handled = true;
		}
		else
		{
			switch(eventptr->data.ctlSelect.controlID)
			{
				case DEFAULTS:
					setdefaultkeys(tempactions);
					LstSetSelection(keylistptr, tempactions[prefs.settings.lastkey]);
					CtlSetLabel(keypopupptr, LstGetSelectionText(keylistptr,
						tempactions[prefs.settings.lastkey]));

					CtlSetValue(FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
						checkbox_kevents)), 0);
					handled = true;
					break;

				case OK:
					prefs.settings.flags &= ~(1<<set_kevents_bit);
					prefs.settings.flags |= CtlGetValue(FrmGetObjectPtr(formptr,
						FrmGetObjectIndex(formptr, checkbox_kevents))) << set_kevents_bit;

					MemMove(prefs.settings.keyactions, tempactions, NKEYS);
					setkeymask(0);

				case CANCEL:
					prefs.checksum = getprefschecksum();
					handled = true;
					FrmReturnToForm(0);
			}
		}
	}

	return handled;
}


Boolean mischandler(EventPtr eventptr)
{
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		FrmDrawForm(FrmGetActiveForm());
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		FrmEraseForm(FrmGetActiveForm());
		FrmReturnToForm(0);
		handled = true;
	}

	return handled;
}


Boolean pausehandler(EventPtr eventptr)
{
	FormPtr formptr = FrmGetActiveForm();
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		char wavetext[9] = "Wave ";
		RectangleType textrect;
		const UInt16 textindex = FrmGetObjectIndex(formptr, label_pausewave);

		StrIToA(&wavetext[5], gv.wave+1);
		FrmCopyLabel(formptr, label_pausewave, wavetext);
		FrmGetObjectBounds(formptr, textindex, &textrect);
		FrmSetObjectPosition(formptr, textindex, (100 - textrect.extent.x)>>1,
			textrect.topLeft.y);
		FrmShowObject(formptr, textindex);

		FrmDrawForm(formptr);
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		FrmEraseForm(formptr);
		FrmReturnToForm(0);
		handled = true;
	}
	else if(eventptr->eType == menuEvent)
	{
		handled = handlemenus(eventptr->data.menu.itemID);
	}

	return handled;
}


Boolean settingshandler(EventPtr eventptr)
{
	Boolean handled	= false;
	FormPtr formptr	= FrmGetActiveForm();

	ListPtr penlistptr		= FrmGetObjectPtr(formptr,	
		FrmGetObjectIndex(formptr, list_pen));
	ControlType* penptr		= FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, popup_pen));
	ListPtr volumelistptr	= FrmGetObjectPtr(formptr,	
		FrmGetObjectIndex(formptr, list_volume));
	ControlType* volumeptr	= FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, popup_volume));

	if((eventptr->eType == frmOpenEvent)
	|| (eventptr->eType == frmUpdateEvent))
	{
		UInt8 i;
		ControlType* cbptr;
		Coord dyncby = 66;

		//initialize the checkboxes
		for(i = 0; i < NFLAGS; i++)
		{
			cbptr = FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
				checkbox_rconf+i));
			CtlSetValue(cbptr, (prefs.settings.flags>>i)&1);
		}

		//set graffiti control to visible if it's working
		if((device.grinfo.right != NOGRAF)
		&& (device.grinfo.left != NOGRAF))
		{
			FrmShowObject(formptr, FrmGetObjectIndex(formptr,
				checkbox_graf));
			dyncby += 12;
		}

		//maybe set compatibility mode to visible
		if(device.flags & df_68k)
		{
			UInt16 cbindex = FrmGetObjectIndex(formptr, checkbox_compat);

			FrmSetObjectPosition(formptr, cbindex, 6, dyncby);
			FrmShowObject(formptr, cbindex);
		}

		//the pen control popup...
		LstSetSelection(penlistptr, prefs.settings.pencontrol);
		CtlSetLabel(penptr, LstGetSelectionText(penlistptr,
			prefs.settings.pencontrol));

		//volume popup...
		LstSetSelection(volumelistptr, prefs.settings.volume);
		CtlSetLabel(volumeptr, LstGetSelectionText(volumelistptr,
			prefs.settings.volume));

		FrmDrawForm(formptr);
	}
	else if(eventptr->eType == popSelectEvent)
	{
		//set the popup labels when something is selected...
		switch(eventptr->data.popSelect.controlID)
		{
			case popup_pen:
				CtlSetLabel(penptr, LstGetSelectionText(penlistptr,
					LstGetSelection(penlistptr)));
				handled = true;
				break;

			case popup_volume:
				CtlSetLabel(volumeptr, LstGetSelectionText(volumelistptr,
					LstGetSelection(volumelistptr)));
				handled = true;
				break;
		}
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		switch(eventptr->data.ctlSelect.controlID)
		{
			case DEFAULTS:
				CtlSetValue(FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
					checkbox_rconf)), 0);
				CtlSetValue(FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
					checkbox_qrest)), 0);
				CtlSetValue(FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
					checkbox_graf)), 0);
				CtlSetValue(FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
					checkbox_compat)), 0);

				LstSetSelection(penlistptr, pc_movement);
				CtlSetLabel(penptr, LstGetSelectionText(penlistptr,
					pc_movement));

				LstSetSelection(volumelistptr, vol_sys);
				CtlSetLabel(volumeptr, LstGetSelectionText(volumelistptr,
					vol_sys));

				FrmDrawForm(formptr);
				break;

			case OK:
			{
				UInt8 i;
				ControlType* cbptr;

				prefs.settings.flags &= (1<<set_kevents_bit);

				for(i = 0; i < NFLAGS; i++)
				{
					cbptr = FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
						checkbox_rconf+i));
					prefs.settings.flags |= CtlGetValue(cbptr)<<i;
				}

				prefs.settings.pencontrol	= LstGetSelection(penlistptr);
				prefs.settings.volume		= LstGetSelection(volumelistptr);

				prefs.checksum = getprefschecksum();			
				setvolume();
				setgfxfunctions();
			}
			case CANCEL:
				handled = true;
				FrmReturnToForm(0);
		}
	}

	return handled;
}


Boolean restorehandler(EventPtr eventptr)
{
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		UInt16 version;

		FrmDrawForm(FrmGetActiveForm());
		version = loadgamevars();

		//fix up old versions of the gv struct...
		if(version <= 1)
		{
			//MAXBULLETS was increased in version 2
			type_gamevars_v1* gv1 = MemPtrNew(sizeof(type_gamevars_v1));

			if(gv1)
			{
				MemMove(gv1, &gv, sizeof(type_gamevars_v1));
				MemMove(&gv, gv1, (UInt32)gv1->effects - (UInt32)gv1);
				MemMove(gv.effects, gv1->effects, (UInt32)gv1 + sizeof(type_gamevars_v1) - (UInt32)gv1->effects);

				MemPtrFree(gv1);
			}
			//there should probably be an error message or something if the above fails...
		}

		if(!version)
		{
			//prior to version 1, penalties were calculated after you died...
			gv.penalties = getweaponcost(prefs.gear[gv.difficulty][gv.startwave].weapon) +
				getspecialcost(prefs.gear[gv.difficulty][gv.startwave].special);
		}

		FrmGotoForm(form_main);
		handled = true;
	}

	return handled;
}


Boolean diehandler(EventPtr eventptr)
{
	Boolean handled = false;

	if(eventptr->eType == frmOpenEvent)
	{
		FrmDrawForm(FrmGetActiveForm());
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		FrmReturnToForm(0);
		setstate(state_score);
		FrmPopupForm(form_score);
		handled = true;
	}

	return handled;
}


Boolean diehshandler(EventPtr eventptr)
{
	FormPtr		formptr		= FrmGetActiveForm();
	UInt16		fieldindex	= FrmGetObjectIndex(formptr, field_highscore);
	FieldType*	fieldptr	= FrmGetObjectPtr(formptr, fieldindex);
	Boolean		handled 	= false;

	if(eventptr->eType == frmOpenEvent)
	{
		#if (dlkUserNameBufSize > MAXNAMELEN+1)
			MemHandle namehand = MemHandleNew(dlkUserNameBufSize);
		#else
			MemHandle namehand = MemHandleNew(MAXNAMELEN+1);
		#endif

		char* nameptr = MemHandleLock(namehand);

		if(DlkGetSyncInfo(NULL, NULL, NULL, nameptr, NULL, NULL) != errNone)
		{
			StrCopy(nameptr, "");
		}
		else
		{
			char* spaceptr = StrChr(nameptr, ' ');
			if(spaceptr)
			{
				*spaceptr = '\0';
			}
			nameptr[MAXNAMELEN] = '\0';
		}

		MemPtrUnlock(nameptr);
		FldSetTextHandle(fieldptr, namehand);

		FrmSetFocus(formptr, fieldindex);
		FrmDrawForm(formptr);

		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		char* nameptr = FldGetTextPtr(fieldptr);
		type_highscore hs;

		if(nameptr)
		{
			StrCopy(hs.name, nameptr);
			hs.score		= getplayerscore();
			hs.time			= TimGetSeconds();
			hs.startwave	= gv.startwave;
			hs.topwave		= gv.wave;
		}

		FrmReturnToForm(0);
		setstate(state_score);
		FrmPopupForm(form_score);
		handled = true;

		if((nameptr) && (handled))
		{
			inserthighscore(checkhighscore(hs.score), &hs);

			//make the high score screen show scores for the last difficulty
			//that had a high score
			prefs.settings.hsdifficulty = gv.difficulty;
			prefs.checksum = getprefschecksum();
		}
	}

	return handled;
}


void hiscores_initscrollwindow(void)
{
	UInt8		i;
	char 		text[32];
	DateType	date;
	WinHandle	oldwin = WinGetDrawWindow();

	WinSetDrawWindow(gfx.main.scrollwindow);
	WinEraseWindow();

	for(i = 0; i < NHISCORES; i++)
	{
		WinDrawTruncChars(prefs.highscores[prefs.settings.hsdifficulty][i].name,
			StrLen(prefs.highscores[prefs.settings.hsdifficulty][i].name),
			0, i*26, 100);

		StrIToA(text, prefs.highscores[prefs.settings.hsdifficulty][i].score);
		WinDrawChars(text, StrLen(text), 110, i*26);

		if(prefs.highscores[prefs.settings.hsdifficulty][i].topwave >= ENDWAVE)
		{
			StrCopy(text, "finished wave ");
			StrIToA(&text[StrLen(text)], ENDWAVE);
		}
		else
		{ 
			StrCopy(text, "died on wave ");
			StrIToA(&text[StrLen(text)],
				prefs.highscores[prefs.settings.hsdifficulty][i].topwave+1);
		}

		StrCat(text, " on ");
		DateSecondsToDate(prefs.highscores[prefs.settings.hsdifficulty][i].time,
			&date);
		DateToAscii(date.month, date.day, date.year+1904,
			PrefGetPreference(prefDateFormat), &text[StrLen(text)]);

		WinDrawChars(text, StrLen(text), 0, i*26+11);
	}

	WinSetDrawWindow(oldwin);
}


void hiscores_drawscrollwindow(ScrollBarPtr sbptr)
{
	Int16 min, max, value, pagesize;
	RectangleType hsrect;

	SclGetScrollBar(sbptr, &value, &min, &max, &pagesize);

	RctSetRectangle(&hsrect, 0, value, HSWINWIDTH, HSPAGESIZE);

	WinCopyRectangle(gfx.main.scrollwindow, NULL, &hsrect, 8, 32, winPaint);
}


Boolean hiscoreshandler(EventPtr eventptr)
{
	Boolean			handled	= false;
	FormPtr			formptr	= FrmGetActiveForm();
	ScrollBarPtr 	sbptr 	= FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, scrollbar_hs));
	ListPtr diffslistptr	= FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, list_hsdiffs));
	ControlType* diffsptr	= FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, popup_hsdiffs));

	if(eventptr->eType == frmOpenEvent)
	{
		UInt16		err;

		setkeymask(keyBitPageUp | keyBitPageDown);

		gfx.main.scrollwindow = WinCreateOffscreenWindow(HSWINWIDTH,
			HSWINHEIGHT, screenFormat, &err);
		if(err)
		{
			FrmReturnToForm(0);
			return false;
		}

		hiscores_initscrollwindow();

		LstSetSelection(diffslistptr, prefs.settings.hsdifficulty);
		CtlSetLabel(diffsptr, LstGetSelectionText(diffslistptr,
			prefs.settings.hsdifficulty));

		FrmDrawForm(formptr);
		hiscores_drawscrollwindow(sbptr);
		handled = true;
	}

	else if((eventptr->eType == frmUpdateEvent)
	|| (eventptr->eType == sclRepeatEvent))		
	{
		hiscores_drawscrollwindow(sbptr);
	}

	else if(eventptr->eType == popSelectEvent)
	{
		CtlSetLabel(diffsptr, LstGetSelectionText(diffslistptr,
			LstGetSelection(diffslistptr)));
		prefs.settings.hsdifficulty = LstGetSelection(diffslistptr);

		hiscores_initscrollwindow();
		hiscores_drawscrollwindow(sbptr);

		handled = true;
	}

	else if(eventptr->eType == keyDownEvent)
	{
		Int16 min, max, value, pagesize;
		SclGetScrollBar(sbptr, &value, &min, &max, &pagesize);

		switch(eventptr->data.keyDown.chr)
		{
			case vchrPageUp:
				if(value < pagesize)		//e.g. value-pagesize < min
				{
					value = 0;
				}
				else
				{
					value -= pagesize;
				}
				SclSetScrollBar(sbptr, value, min, max, pagesize);
				hiscores_drawscrollwindow(sbptr);
				handled = true;	
				break;

			case vchrPageDown:
				if(value+pagesize > max)
				{
					value = max;
				}
				else
				{
					value += pagesize;
				}
				SclSetScrollBar(sbptr, value, min, max, pagesize);
				hiscores_drawscrollwindow(sbptr);
				handled = true;
		}
	}

	else if(eventptr->eType == ctlSelectEvent)
	{
		switch(eventptr->data.ctlSelect.controlID)
		{
			case DONE:
				if(gfx.main.scrollwindow)
				{
					WinDeleteWindow(gfx.main.scrollwindow, false);
					gfx.main.scrollwindow = NULL;
				}
				prefs.checksum = getprefschecksum();
				FrmReturnToForm(0);
				setkeymask(0);
				handled = true;
				break;

			case button_hsclear:
				if(!FrmAlert(alert_clearhs))
				{
					Int16 min, max, value, pagesize;

					SclGetScrollBar(sbptr, &value, &min, &max, &pagesize);
					SclSetScrollBar(sbptr, 0, min, max, pagesize);

					clearhighscores();
					prefs.checksum = getprefschecksum();
					hiscores_initscrollwindow();
					hiscores_drawscrollwindow(sbptr);
				}
				handled = true;
				break;
			
			case button_hscode:
			{
				UInt32 code = genhscode();

				if(code)
				{
					char codetext[maxStrIToALen];
					char zcodetext[9];
					StrIToA(codetext, code);

					StrCopy(zcodetext, "00000000");
					StrCopy(&zcodetext[8-StrLen(codetext)], codetext);

					FrmCustomAlert(alert_hscode, zcodetext, "", "");
				}
				else
				{
					FrmAlert(alert_hserror);
				}

				handled = true;
			}
		}
	}

	else if(eventptr->eType == frmCloseEvent)
	{
		if(gfx.main.scrollwindow)
		{
			WinDeleteWindow(gfx.main.scrollwindow, false);
			gfx.main.scrollwindow = NULL;
		}
	}

	return handled;
}



void shop_drawmoney(void)
{
	FormPtr	formptr	= FrmGetActiveForm();
	UInt16	index	= FrmGetObjectIndex(formptr, label_money);
	char	text[maxStrIToALen+1] = "$";

	if(gv.player.money > 999999)
	{
		StrCopy(&text[1], "A Lot!");
	}
	else
	{
		StrIToA(&text[1], gv.player.money);
	}
	FrmHideObject(formptr, index);
	FrmCopyLabel(formptr, label_money, text);
	FrmShowObject(formptr, index);
}


void shop_drawtrade(Int16 shoplist)
{
	FormPtr formptr	= FrmGetActiveForm();
	UInt16	index	= FrmGetObjectIndex(formptr, label_trade);
	char	text[maxStrIToALen+1] = "$";
	UInt16	value;

	switch(shoplist)
	{
		default:
		case shoplist_weapons:
			value = getweapontrade(gv.player.weapon);
			break;
		case shoplist_equip:	
			value = getspecialtrade(gv.player.special);
	}

	StrIToA(&text[1], value);

	FrmHideObject(formptr, index);
	FrmCopyLabel(formptr, label_trade, text);
	FrmShowObject(formptr, index);
}



void shop_updatebuybutton(void)
{
	FormPtr formptr	= FrmGetActiveForm();
	Int16 shoplist	= LstGetSelection(FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, list_shopview)));
	Int16 selection	= LstGetSelection(FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, SHOPLISTBASE+shoplist)));
	Boolean show	= false;

	if(selection != noListSelection)
	{
		switch(shoplist)
		{
			case shoplist_weapons:
				if((gv.player.money + getweapontrade(gv.player.weapon)
					>= getweaponcost(selection))
				&& (gv.player.weapon != selection))
				{
					show = true;
				}
				break;
			case shoplist_equip:
				if((gv.player.money + getspecialtrade(gv.player.special)
					>= getspecialcost(selection))
				&& (gv.player.special != selection))
				{
					show = true;
				}
		}
	}

	if(show)
	{
		FrmShowObject(formptr, FrmGetObjectIndex(formptr, button_buy));
	}
	else
	{
		FrmHideObject(formptr, FrmGetObjectIndex(formptr, button_buy));
	}
}


void shop_changelist(Int16 prevlist, Int16 newlist)
{
	FormPtr	formptr			= FrmGetActiveForm();
	ListPtr viewlistptr		= FrmGetObjectPtr(formptr,
								FrmGetObjectIndex(formptr, list_shopview));
	ControlType* viewptr	= FrmGetObjectPtr(formptr,
								FrmGetObjectIndex(formptr, popup_shopview));

	FrmHideObject(formptr, FrmGetObjectIndex(formptr,
		SHOPLISTBASE + prevlist));
	FrmHideObject(formptr, FrmGetObjectIndex(formptr,
		SHOPLISTBASE + 10 + prevlist));

	FrmShowObject(formptr, FrmGetObjectIndex(formptr,
		SHOPLISTBASE + newlist));
	FrmShowObject(formptr, FrmGetObjectIndex(formptr,
		SHOPLISTBASE + 10 + newlist));

	shop_updatebuybutton();
	shop_drawtrade(newlist);

	CtlSetLabel(viewptr, LstGetSelectionText(viewlistptr, newlist));
}


void shop_freeprices(void)
{
	UInt8 i;

	for(i = 0; i < NWEAPONS; i++)
	{
		if(wprices[i])
		{
			MemPtrFree(wprices[i]);
			wprices[i] = NULL;
		}
	}

	for(i = 0; i < NSPECIALS; i++)
	{
		if(eprices[i])
		{
			MemPtrFree(eprices[i]);
			eprices[i] = NULL;
		}
	}
}


void shop_updateprices(void)
{
	FormPtr formptr = FrmGetActiveForm();
	UInt8 i;

	shop_freeprices();

	for(i = 0; i < NWEAPONS; i++)
	{
		wprices[i] = (char*)MemPtrNew(8);

		if(i == gv.player.weapon)
		{
			StrCopy(wprices[i], "IN USE");
		}
		else if(getweaponcost(i))
		{
			StrIToA(wprices[i], getweaponcost(i));
		}
		else
		{
			StrCopy(wprices[i], "FREE");
		}
	}

	for(i = 0; i < NSPECIALS; i++)
	{
		eprices[i] = (char*)MemPtrNew(8);

		if(i == gv.player.special)
		{
			StrCopy(eprices[i], "IN USE");
		}
		else if(getspecialcost(i))
		{
			StrIToA(eprices[i], getspecialcost(i));
		}
		else
		{
			StrCopy(eprices[i], "FREE");
		}
	}

	LstSetListChoices(FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, list_weaponprices)),
		wprices, NWEAPONS);

	LstSetListChoices(FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, list_equipprices)),
		eprices, NSPECIALS);
}



Boolean shophandler(EventPtr eventptr)
{
	Boolean handled		= false;
	FormPtr	formptr		= FrmGetActiveForm();
	ListPtr viewlistptr	= FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, list_shopview));

	if(eventptr->eType == frmOpenEvent)
	{
		ControlType* viewptr = FrmGetObjectPtr(formptr,
			FrmGetObjectIndex(formptr, popup_shopview));

		shop_updateprices();
		FrmShowObject(formptr, FrmGetObjectIndex(formptr, list_weaponprices));

		LstSetSelection(viewlistptr, 0);
		CtlSetLabel(viewptr, LstGetSelectionText(viewlistptr, 0));

		shop_drawmoney();
		shop_updatebuybutton();
		shop_drawtrade(shoplist_weapons);

		FrmDrawForm(formptr);
		handled = true;
	}

	else if(eventptr->eType == popSelectEvent)
	{
		shop_changelist(eventptr->data.popSelect.priorSelection,
			eventptr->data.popSelect.selection);

		handled = true;
	}

	else if(eventptr->eType == lstSelectEvent)
	{
		if((eventptr->data.lstSelect.listID >= SHOPLISTBASE)
		&& (eventptr->data.lstSelect.listID < SHOPLISTBASE+NSHOPLISTS))
		{
			//items list was tapped, update the price list
			LstSetSelection(FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
				eventptr->data.lstSelect.listID + 10)),
				eventptr->data.lstSelect.selection);
			shop_updatebuybutton();
		}
		else if((eventptr->data.lstSelect.listID >= SHOPLISTBASE+10)
		&& (eventptr->data.lstSelect.listID < SHOPLISTBASE+10+NSHOPLISTS))
		{
			//price list was tapped, update items
			LstSetSelection(FrmGetObjectPtr(formptr, FrmGetObjectIndex(formptr,
				eventptr->data.lstSelect.listID - 10)),
				eventptr->data.lstSelect.selection);
			shop_updatebuybutton();
		}
	}

	else if(eventptr->eType == ctlSelectEvent)
	{
		Int16 shoplist = LstGetSelection(FrmGetObjectPtr(formptr,
			FrmGetObjectIndex(formptr, list_shopview)));

		switch(eventptr->data.ctlSelect.controlID)
		{
			case button_more:
			{
				Int16 newlist = (shoplist+1)%NSHOPLISTS;

				LstSetSelection(viewlistptr, newlist);
				shop_changelist(shoplist, newlist);
				handled = true;
				break;
			}

			case button_buy:
			{
				Int16 selection	= LstGetSelection(FrmGetObjectPtr(formptr,
					FrmGetObjectIndex(formptr, SHOPLISTBASE+shoplist)));

				if(selection != noListSelection)
				{
					switch(shoplist)
					{
						case shoplist_weapons:
							gv.player.money		+= getweapontrade(gv.player.weapon);
							gv.player.money		-= getweaponcost(selection);
							gv.player.weapon	= selection;
							break;
						case shoplist_equip:
							gv.player.money		+= getspecialtrade(gv.player.special);
							gv.player.money		-= getspecialcost(selection);
							gv.player.special	= selection;
					}
				}

				shop_updateprices();

				//for redraw purposes, also
				//calls updatebuybutton, drawtrade
				shop_changelist(shoplist, shoplist);

				shop_drawmoney();
				handled = true;
				break;
			}
			case DONE:
				shop_freeprices();
				FrmReturnToForm(0);
				setstate(state_enter);
				handled = true;
		}
	}
	else if(eventptr->eType == frmCloseEvent)
	{
		shop_freeprices();
	}

	return handled;
}


Boolean scorehandler(EventPtr eventptr)
{
	Boolean handled = false;
	FormPtr formptr = FrmGetActiveForm();

	if(eventptr->eType == frmOpenEvent)
	{
		char itoatext[maxStrIToALen];
		UInt16 index;

		index = FrmGetObjectIndex(formptr, label_wavesx100);
		StrIToA(itoatext, 100*(gv.wave-gv.startwave));
		FrmSetObjectPosition(formptr, index,
			SCORELABELX - FntCharsWidth(itoatext, StrLen(itoatext)), SCORELABELY);
		FrmCopyLabel(formptr, label_wavesx100, itoatext);
		FrmShowObject(formptr, index);

		index = FrmGetObjectIndex(formptr, label_credits);
		StrIToA(itoatext, gv.player.money);
		FrmSetObjectPosition(formptr, index,
			SCORELABELX - FntCharsWidth(itoatext, StrLen(itoatext)), SCORELABELY+11);
		FrmCopyLabel(formptr, label_credits, itoatext);
		FrmShowObject(formptr, index);

		index = FrmGetObjectIndex(formptr, label_weaponval);
		StrIToA(itoatext, getweaponcost(gv.player.weapon));
		FrmSetObjectPosition(formptr, index,
			SCORELABELX - FntCharsWidth(itoatext, StrLen(itoatext)), SCORELABELY+11*2);
		FrmCopyLabel(formptr, label_weaponval, itoatext);
		FrmShowObject(formptr, index);

		index = FrmGetObjectIndex(formptr, label_equipval);
		StrIToA(itoatext, getspecialcost(gv.player.special));
		FrmSetObjectPosition(formptr, index,
			SCORELABELX - FntCharsWidth(itoatext, StrLen(itoatext)), SCORELABELY+11*3);
		FrmCopyLabel(formptr, label_equipval, itoatext);
		FrmShowObject(formptr, index);

		index = FrmGetObjectIndex(formptr, label_penalties);
		StrIToA(itoatext, gv.penalties);
		FrmSetObjectPosition(formptr, index,
			SCORELABELX - FntCharsWidth(itoatext, StrLen(itoatext)), SCORELABELY+11*4);
		FrmCopyLabel(formptr, label_penalties, itoatext);
		FrmShowObject(formptr, index);

		index = FrmGetObjectIndex(formptr, label_total);
		StrIToA(itoatext, getplayerscore());
		FrmSetObjectPosition(formptr, index,
			SCORELABELX - FntCharsWidth(itoatext, StrLen(itoatext)), SCORELABELY+11*6);
		FrmCopyLabel(formptr, label_total, itoatext);
		FrmShowObject(formptr, index);

		FrmDrawForm(formptr);
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		switch(eventptr->data.ctlSelect.controlID)
		{
			case button_retire:
				FrmReturnToForm(0);
				setstate(state_retire);
				handled = true;
				break;

			case button_retry:
				FrmReturnToForm(0);
				if(gv.wave < ENDWAVE)
				{
					initgame(REPLAY);
				}
				else
				{
					//this handler is also used for the end-of-game form
					initgame(prefs.settings.startwave[prefs.settings.difficulty]);
				}
				handled = true;
				break;
		}
	}

	return handled;
}


Boolean reghandler(EventPtr eventptr)
{
	Boolean		handled		= false;
	FormPtr		formptr		= FrmGetActiveForm();
	UInt16		fieldindex	= FrmGetObjectIndex(formptr, field_regcode);
	FieldType*	fieldptr	= FrmGetObjectPtr(formptr, fieldindex);

	if(eventptr->eType == frmOpenEvent)
	{
		char		hotsyncid[dlkUserNameBufSize] = "";
		MemHandle	codehand	= MemHandleNew(MAXREGCODELEN+1);
		char*		codeptr		= MemHandleLock(codehand);
		UInt16		hsidindex	= FrmGetObjectIndex(formptr, label_hotsyncid);

		//set up the reg code field
		//show existing code if there is one...
		if(prefs.regcode)
		{
			char codestring[MAXREGCODELEN+1];

			//pad the code with leading 0's
			StrIToA(codestring, prefs.regcode);
			StrCopy(codeptr, "00000");
			StrCopy(&codeptr[5-StrLen(codestring)], codestring);
		}
		else
		{
			*codeptr = '\0';
		}		

		MemPtrUnlock(codeptr);
		FldSetTextHandle(fieldptr, codehand);
		FrmSetFocus(formptr, fieldindex);

		//show the user's hotsync id
		if((DlkGetSyncInfo(NULL, NULL, NULL, hotsyncid, NULL, NULL) != errNone)
		|| (!StrLen(hotsyncid)))
		{
			//no hotsync id, display error message
		}

		FrmCopyLabel(formptr, label_hotsyncid, hotsyncid);
		FrmSetObjectPosition(formptr, hsidindex,
			80 - FntCharsWidth(hotsyncid, StrLen(hotsyncid))/2, HSIDY);
		FrmShowObject(formptr, hsidindex);

		FrmDrawForm(formptr);
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		switch(eventptr->data.ctlSelect.controlID)
		{
			case OK:
			{
				char* codeptr = FldGetTextPtr(fieldptr);
				UInt16 testcode = StrAToI(codeptr);

				if(checkregcode(testcode))
				{
					FrmAlert(alert_goodcode);
					prefs.regcode = testcode;
					prefs.checksum = getprefschecksum();
					//no break - go to case CANCEL
				}
				else
				{
					FrmAlert(alert_badcode);
					handled = true;
					break;
				}
			}

			case CANCEL:
				if((gv.gamestate&INPLAYMASK) == state_title)
				{
					//the title screen says "(un)registered" at the bottom - update it
					FrmUpdateForm(form_title, frmRedrawUpdateCode);
				}
				gv.gamestate &= ~state_regwait;
				FrmReturnToForm(0);
				handled = true;
		}
	}

	return handled;
}



void records_update(const UInt8 oldvalue, const UInt8 value,
const UInt8 olddiff, const UInt8 diff)
{
	FormPtr	formptr		= FrmGetActiveForm();
	UInt16	labelindex	= FrmGetObjectIndex(formptr, label_startwave);
	char	wavetext[6];

	FrmHideObject(formptr, FrmGetObjectIndex(formptr,
		label_startweapons + prefs.gear[olddiff][oldvalue].weapon));
	FrmShowObject(formptr, FrmGetObjectIndex(formptr,
		label_startweapons + prefs.gear[diff][value].weapon));

	FrmHideObject(formptr, FrmGetObjectIndex(formptr,
		label_startequip + prefs.gear[olddiff][oldvalue].special));
	FrmShowObject(formptr, FrmGetObjectIndex(formptr,
		label_startequip + prefs.gear[diff][value].special));

	StrIToA(wavetext, value+1);
	FrmHideObject(formptr, labelindex);
	FrmSetObjectPosition(formptr, labelindex,
		startwavex - ((FntCharsWidth(wavetext, StrLen(wavetext)))>>1), startwavey);
	FrmCopyLabel(formptr, label_startwave, wavetext);
	FrmShowObject(formptr, labelindex);

	labelindex = FrmGetObjectIndex(formptr, label_topwave);

	StrIToA(wavetext, prefs.settings.topwave[diff]+1);
	FrmHideObject(formptr, labelindex);
	FrmSetObjectPosition(formptr, labelindex,
		topwavex - ((FntCharsWidth(wavetext, StrLen(wavetext)))>>1), topwavey);
	FrmCopyLabel(formptr, label_topwave, wavetext);
	FrmShowObject(formptr, labelindex);
}



Boolean recordshandler(EventPtr eventptr)
{
	Boolean handled = false;
	FormPtr formptr = FrmGetActiveForm();

	ControlType* diffsptr = FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, popup_diffs));
	ListPtr difflistptr = FrmGetObjectPtr(formptr,
		FrmGetObjectIndex(formptr, list_diffs));

	UInt8 difficulty = LstGetSelection(difflistptr);
	UInt8 startwave = StrAToI(FrmGetLabel(formptr, label_startwave))-1;

	if(eventptr->eType == frmOpenEvent)
	{
		LstSetSelection(difflistptr, prefs.settings.difficulty);
		CtlSetLabel(diffsptr, LstGetSelectionText(difflistptr,
			prefs.settings.difficulty));

		records_update(0, prefs.settings.startwave[prefs.settings.difficulty],
			prefs.settings.difficulty, prefs.settings.difficulty);

		FrmDrawForm(formptr);
		handled = true;
	}
	else if((eventptr->eType == popSelectEvent)
	&& (eventptr->data.popSelect.controlID == popup_diffs))
	{
		prefs.settings.startwave[eventptr->data.popSelect.priorSelection] =
			startwave;

		CtlSetLabel(diffsptr, LstGetSelectionText(difflistptr,
			eventptr->data.popSelect.selection));

		records_update(startwave,
			prefs.settings.startwave[eventptr->data.popSelect.selection],
			eventptr->data.popSelect.priorSelection,
			eventptr->data.popSelect.selection);

		prefs.checksum = getprefschecksum();			
		handled = true;
	}
	else if(eventptr->eType == ctlSelectEvent)
	{
		switch(eventptr->data.ctlSelect.controlID)
		{
			case button_waveplus:
			{
				UInt8 newstartwave;

				if(startwave >= prefs.settings.topwave[difficulty])
				{
					newstartwave = 0;
				}
				else
				{
					newstartwave = startwave+1;
				}
				records_update(startwave, newstartwave,
					difficulty, difficulty);
				handled = true;
				break;
			}
			case button_waveminus:
			{
				UInt8 newstartwave;

				if(startwave <= 0)
				{
					newstartwave = prefs.settings.topwave[difficulty];
				}
				else
				{
					newstartwave = startwave-1;
				}
				records_update(startwave, newstartwave,
					difficulty, difficulty);
				handled = true;
				break;
			}
			case button_rclear:
				if(!FrmAlert(alert_clearrecords))
				{
					clear_records();
					prefs.checksum = getprefschecksum();
					records_update(startwave, 0,
						difficulty, difficulty);
				}
				handled = true;
				break;
			case OK:
			{
				prefs.settings.difficulty = difficulty;

				prefs.settings.startwave[difficulty] =
					startwave;

				prefs.checksum = getprefschecksum();			
			}
			case CANCEL:
				handled = true;
				FrmReturnToForm(0);
		}
	}

	return handled;
}