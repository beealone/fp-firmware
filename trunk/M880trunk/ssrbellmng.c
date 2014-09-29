/*new bell manage window*/
/*liming*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>
#include "options.h"
#include "flashdb.h"
#include "ssrpub.h"
#include "ssrcommon.h"
#include "exfun.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

#define BELL_DAY 	40
#define BELL_HOUR	41
#define BELL_MIN	42
#define BELL_WAVE	43
#define BELL_VOLUME	44
#define BELL_TIMES	45
#define BELL_STATUS	46 
#define BELL_STATIC	50
#define BELL_SAVE	60
#define BELL_EXIT	61

TBellNew sbell;
int bellopmode;
int sbellid;
//BITMAP sbellbkg;
HWND hBellWnd[9];
int curBellWnd;
int curDay;
int g_selWave, g_selStatus, g_selDay;
static int BellChanged = 0;
char* winHint[9];

int CheckBellSetError(PBellNew pbell)
{
	return FDB_BellSettingErr(pbell->BellID, pbell->BellTime[0], pbell->BellTime[1], pbell->SchInfo);
}

static int GetBellSchValue(void)
{
	int i;
	int bitvalue;
	int rvalue=0;

	for (i=0; i<7; i++)
	{
		bitvalue = (SendMessage(hBellWnd[6], LB_GETCHECKMARK, i, 0)==CMFLAG_CHECKED)?1:0;
		rvalue |= (bitvalue<<i);
	}
	rvalue |= (g_selStatus<<7); 
	return rvalue;
}

static int isBellChanged(void)
{
	char tmpc[4];
	int ihour, imin, itimes;
	int chgflag=0;

	memset(tmpc, 0, 4);
	GetWindowText(hBellWnd[0], tmpc, 2);
	ihour = atoi(tmpc);
	if (ihour != sbell.BellTime[0]) chgflag++;

	memset(tmpc, 0, 4);
	GetWindowText(hBellWnd[1], tmpc, 2);
	imin = atoi(tmpc);
	if (imin != sbell.BellTime[1]) chgflag++;

	if (GetBellSchValue() != sbell.SchInfo) chgflag++;
	if (g_selWave != sbell.BellWaveIndex) chgflag++;
#ifdef ZEM600
	//if ((SendMessage(hBellWnd[3], CB_GETCURSEL, 0, 0)*8+14)*3 != sbell.BellVolume) chgflag++;
	int value = 0;
	if(sbell.BellVolume <= VOICE_LOW)
		value = 0;
	else if(sbell.BellVolume>VOICE_LOW && sbell.BellVolume <= VOICE_MIDDLE)
		value = 1;
	else
		value = 2;

	if (SendMessage(hBellWnd[3], CB_GETCURSEL, 0, 0) != value) 
		chgflag++;
#else	
	if (SendMessage(hBellWnd[3], CB_GETSPINVALUE, 0, 0) != sbell.BellVolume) chgflag++;
#endif		
	memset(tmpc, 0, 4);
	GetWindowText(hBellWnd[4], tmpc, 2);
	itimes = atoi(tmpc);
	if (itimes != sbell.BellTimes) chgflag++;

	return chgflag;
}

static int SaveBellInfo(HWND hWnd)
{
	char tmpc[4];
	int ihour, imin, itimes;
	int err=0;

	memset(tmpc, 0, 4);
	GetWindowText(hBellWnd[0], tmpc, 2);
	ihour = atoi(tmpc);
	if (ihour<0 || ihour>23)
		err++;

	memset(tmpc, 0, 4);
	GetWindowText(hBellWnd[1], tmpc, 2);
	imin=atoi(tmpc);
	if (imin<0 || imin>59)
		err++;

	memset(tmpc, 0, 4);
	GetWindowText(hBellWnd[4], tmpc, 2);
	itimes = atoi(tmpc);
	if(itimes<=0)
		err++;

	if (!err)
	{
		sbell.SchInfo = GetBellSchValue();	// Bell schedule as status. bit7: ON/OFF, bit0-bit6:日,一,二,三,四,五,六 
		sbell.BellTime[0] = ihour;		// Bell time (hh:mm)
		sbell.BellTime[1] = imin;
		sbell.BellWaveIndex = g_selWave;	// Bell wave file name
#ifdef ZEM600
		//sbell.BellVolume = (SendMessage(hBellWnd[3], CB_GETCURSEL, 0, 0)*8+14)*3;     // Bell volume
		int value = SendMessage(hBellWnd[3], CB_GETCURSEL, 0, 0);
		if(value==0)
			sbell.BellVolume=VOICE_LOW;
		else if (value == 1)
			sbell.BellVolume=VOICE_MIDDLE;
		else if (value == 2)
			sbell.BellVolume=VOICE_HEIGH;
#else
		sbell.BellVolume = SendMessage(hBellWnd[3], CB_GETSPINVALUE, 0, 0);	// Bell volume 
#endif
		sbell.BellTimes = itimes;		// Bell times

		if (!(((sbell.SchInfo>>7)&1) && CheckBellSetError(&sbell)))
		{
			if (bellopmode==0)	//修改
				return (FDB_ChgBell(&sbell)==FDB_OK);
			else
				return (FDB_AddBell(&sbell)==FDB_OK);
		}
		else
			MessageBox1(hWnd, LoadStrByID(MID_TIMEERROR), LoadStrByID(MID_APPNAME), MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
        }
        else
		MessageBox1(hWnd, LoadStrByID(HIT_ERROR0), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);

        return 0;
}

static void InitWinData(void)
{
	LISTBOXITEMINFO lbii;
	char tmpstr[40];
	int i;

	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "%02d", sbell.BellTime[0]);
	SetWindowText(hBellWnd[0], tmpstr);

	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "%02d", sbell.BellTime[1]);
	SetWindowText(hBellWnd[1], tmpstr);

	SendMessage(hBellWnd[6], LB_SETITEMHEIGHT, 0, 23);
	for (i=0; i<7; i++)
	{		
		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr, "%s", LoadStrByID(HID_DAY0+i));
		lbii.string = tmpstr;
		lbii.cmFlag = (BellIsValid(sbell, i)==1) ? CMFLAG_CHECKED:CMFLAG_BLANK;
		lbii.hIcon = 0;
		SendMessage(hBellWnd[6], LB_ADDSTRING, 0, (LPARAM)&lbii);
	}

	for(i=0; i<10; i++)
	{
		memset(tmpstr, 0, 40);
		sprintf(tmpstr, "bell%02d.wav", i+1);
		SendMessage(hBellWnd[2], CB_ADDSTRING, 0, (LPARAM)tmpstr);
	}
#ifdef ZEM600
        for(i=0;i<3;i++)
	{
                SendMessage(hBellWnd[3],CB_ADDSTRING ,0,(LPARAM)LoadStrByID(MID_VIDEO_QLOW+i));
	}
	int value=0;
	if(sbell.BellVolume<=VOICE_LOW)
	{
		sbell.BellVolume=VOICE_LOW;
		value=0;
	}
	else if(sbell.BellVolume>VOICE_LOW && sbell.BellVolume<=VOICE_MIDDLE)
	{
		sbell.BellVolume=VOICE_MIDDLE;
		value=1;
	}
	else
	{
		sbell.BellVolume=VOICE_HEIGH;
		value=2;
	}
	SendMessage(hBellWnd[3],CB_SETCURSEL,value,0);

        //SendMessage(hBellWnd[3],CB_SETCURSEL,((sbell.BellVolume<42?42:sbell.BellVolume)/3-14)/8,0);
#else
	SendMessage(hBellWnd[3], CB_SETSPINRANGE, 0, 100);
	SendMessage(hBellWnd[3], CB_SETSPINPACE, 1, 1);
	SendMessage(hBellWnd[3], CB_SETSPINFORMAT, 0, (LPARAM)"%d%%");
#endif
	SendMessage(hBellWnd[5], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_ALARMSTOP));
	SendMessage(hBellWnd[5], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_ALARMSTART1));

	g_selWave = sbell.BellWaveIndex;
	g_selStatus = (sbell.SchInfo>>7)&1;

	SendMessage(hBellWnd[2], CB_SETCURSEL, g_selWave, 0);
#ifndef ZEM600
	SendMessage(hBellWnd[3], CB_SETSPINVALUE, sbell.BellVolume, 0);
#endif	
	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "%d", sbell.BellTimes);
	SetWindowText(hBellWnd[4], tmpstr);

	SendMessage(hBellWnd[5], CB_SETCURSEL, g_selStatus, 0);
}

void LoadBellStr()
{
	winHint[0] = LoadStrByID(MID_ALARMTIME);
	winHint[1] = LoadStrByID(MID_HOUR1);
	winHint[2] = LoadStrByID(MID_MINUTE);
	winHint[3] = LoadStrByID(MID_ALARMWVSEL);
	winHint[4] = LoadStrByID(MID_ALARMVOLADV);
	winHint[5] = LoadStrByID(MID_ALARM_TIMES);
	winHint[6] = LoadStrByID(MID_ALARMSTATUS);
	winHint[7] = LoadStrByID(MID_SAVE);
	winHint[8] = LoadStrByID(MID_EXIT);
}

static void InitBellSetWindow(HWND hWnd)
{

 	int posX1,posX2,posX3,posX4,posX5,posX6,posX7;
	if (fromRight==1)  //modify by jazzy 2008.11.15
	{
		posX1=240;
		posX2=165;
		posX3=145;
		posX4=115;
		posX5=105;
		posX6=114;
		posX7=10;
	}
	else
	{
		posX1=10;
		posX2=100+gOptions.GridWidth;
		posX3=135+gOptions.GridWidth;
		posX4=160+gOptions.GridWidth;
		posX5=195+gOptions.GridWidth;
		posX6=100+gOptions.GridWidth;
		posX7=220+gOptions.ControlOffset;
	}

        CreateWindow(CTRL_STATIC, winHint[0], WS_VISIBLE | SS_CENTER, BELL_STATIC, posX1, 10, 75, 23, hWnd, 0);
        hBellWnd[0] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, 
				BELL_HOUR, posX2, 6, 30, 23, hWnd, 0);
	SendMessage(hBellWnd[0], EM_LIMITTEXT, 2, 0);
        
	CreateWindow(CTRL_STATIC, winHint[1], WS_VISIBLE | SS_CENTER, BELL_STATIC, posX3, 10, 15, 23, hWnd, 0);
	hBellWnd[1] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				BELL_MIN, posX4, 6, 30, 23, hWnd, 0);
	SendMessage(hBellWnd[1], EM_LIMITTEXT, 2, 0);
        
	CreateWindow(CTRL_STATIC, winHint[2], WS_VISIBLE | SS_CENTER, BELL_STATIC, posX5, 10, 15, 23, hWnd, 0);

        CreateWindow(CTRL_STATIC, winHint[3], WS_VISIBLE | SS_CENTER, BELL_STATIC, posX1, 70, 75, 23, hWnd, 0);
	hBellWnd[2] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
				BELL_WAVE, posX6-9, 66, 120, 23, hWnd, 0);

	CreateWindow(CTRL_STATIC, winHint[4], WS_VISIBLE | SS_CENTER, BELL_STATIC, posX1, 100, 75, 23, hWnd, 0);
#ifdef ZEM600
	hBellWnd[3] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST |
                                        CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, BELL_VOLUME, posX2-9, 96, 65, 23, hWnd, 0);
#else
	hBellWnd[3] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | WS_TABSTOP | CBS_READONLY | CBS_AUTOSPIN | CBS_SPINARROW_LEFTRIGHT | 
				CBS_AUTOLOOP | CBS_EDITBASELINE, BELL_VOLUME, posX2-9, 96, 65, 23, hWnd, 0);
#endif
	CreateWindow(CTRL_STATIC, winHint[5], WS_VISIBLE | SS_CENTER, BELL_STATIC, posX1, 130, 75, 23, hWnd, 0);
	hBellWnd[4] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				BELL_TIMES, posX2, 126, 45, 23, hWnd, 0);
	SendMessage(hBellWnd[4], EM_LIMITTEXT, 2, 0);

        CreateWindow (CTRL_STATIC, winHint[6], WS_VISIBLE | SS_CENTER, BELL_STATIC, posX1, 160, 75, 23, hWnd, 0);
        hBellWnd[5] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
				BELL_STATUS, posX2-9, 156, 75, 23, hWnd, 0);

	hBellWnd[6] = CreateWindow(CTRL_LISTBOX, CTRL_LISTBOX, WS_CHILD | WS_VISIBLE | WS_BORDER /*| WS_VSCROLL*/ | LBS_CHECKBOX,
				BELL_DAY, posX7, 6, 85, 175, hWnd, 0);

	hBellWnd[7] = CreateWindow(CTRL_BUTTON, winHint[7], WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER, BELL_SAVE, 10, 190, 85, 23, hWnd, 0);
	hBellWnd[8] = CreateWindow(CTRL_BUTTON, winHint[8], WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER, BELL_EXIT, 220+gOptions.ControlOffset, 190, 85, 23, hWnd, 0);
}

static int BellEditWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//int i;
	int tmpvalue;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitBellSetWindow(hWnd);
			UpdateWindow(hWnd, TRUE);
			InitWinData();
			curBellWnd = 0;
			SetFocusChild(hBellWnd[curBellWnd]);
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			EndPaint(hWnd,hdc);
			return 0;

                case MSG_ERASEBKGND:
                {
                        HDC hdc = (HDC)wParam;
                        const RECT* clip = (const RECT*)lParam;
                        BOOL fGetDC = FALSE;
                        RECT rcTemp;
                        if(hdc == 0)
                        {
                                hdc = GetClientDC(hWnd);
                                fGetDC = TRUE;
                        }

                        if(clip)
                        {
                                rcTemp = *clip;
                                ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
                                ScreenToClient(hWnd,&rcTemp.right, &rcTemp.bottom);
                                IncludeClipRect(hdc, &rcTemp);
                        }

                        FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());
                        if(fGetDC) ReleaseDC (hdc);
			return 0;
                }

		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
			if (gOptions.KeyPadBeep) ExKeyBeep();

			if ( LOWORD(wParam)==SCANCODE_ESCAPE )
				PostMessage (hWnd, MSG_COMMAND, BELL_EXIT, 0L);

			if (LOWORD(wParam)==SCANCODE_MENU)
				PostMessage(hWnd, MSG_COMMAND, BELL_SAVE, 0);

			if (LOWORD(wParam)==SCANCODE_ENTER || LOWORD(wParam)==SCANCODE_F10)
			{
				if (curBellWnd==6)
				{
					int selindex = SendMessage(hBellWnd[curBellWnd], LB_GETCURSEL, 0, 0);
					if (SendMessage(hBellWnd[curBellWnd], LB_GETCHECKMARK, selindex, 0) != CMFLAG_CHECKED)
						SendMessage(hBellWnd[curBellWnd], LB_SETCHECKMARK, selindex, (LPARAM)CMFLAG_CHECKED);
					else
						SendMessage(hBellWnd[curBellWnd], LB_SETCHECKMARK, selindex, (LPARAM)CMFLAG_BLANK);
					return 0;
				}
				else if (curBellWnd==8)
					PostMessage(hWnd, MSG_COMMAND, BELL_EXIT, 0);
				else
					PostMessage(hWnd, MSG_COMMAND, BELL_SAVE, 0);
				return 0;
			}
			
			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP || LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if (curBellWnd==6)
				{
					if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP && g_selDay==0)
						curBellWnd--;
					else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN && g_selDay==6)
						curBellWnd++;
					else
					{
						SendMessage(hBellWnd[curBellWnd], MSG_KEYDOWN, wParam, lParam);
						g_selDay = SendMessage(hBellWnd[curBellWnd], LB_GETCURSEL, 0, 0);
						return 0;
					}
				}
				else
				{				
					if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
					{
						if (--curBellWnd<0)
							curBellWnd=8;
					}
					else
					{
						if (++curBellWnd>8)
							curBellWnd=0;
					}
				}
				SetFocusChild(hBellWnd[curBellWnd]);
				if (curBellWnd==6)
					SendMessage(hBellWnd[curBellWnd], LB_SETCURSEL, g_selDay, 0);
				return 0;
			}

			if (((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || ((gOptions.TFTKeyLayout==3)&&(LOWORD(wParam)==SCANCODE_BACKSPACE))) || LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if (curBellWnd==6)
				{
					if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || ((gOptions.TFTKeyLayout==3)&&(LOWORD(wParam)==SCANCODE_BACKSPACE)))
						curBellWnd--;
					else
						curBellWnd++;
					SetFocusChild(hBellWnd[curBellWnd]);
					return 0;
				}
				else if (curBellWnd==2)		//Wave index
				{
					int tvol;

					g_selWave = SendMessage(hBellWnd[curBellWnd], CB_GETCURSEL, 0, 0);
#ifdef ZEM600
					//tmpvalue = (SendMessage(hBellWnd[3], CB_GETSPINVALUE, 0, 0)*8+14)*3;
					int value = SendMessage(hBellWnd[3],CB_GETCURSEL,0,0);
					if(value==0)
					{
						tmpvalue  = VOICE_LOW;
					}
					else if(value==1)
					{
						tmpvalue = VOICE_MIDDLE;
					}
					else
					{
						tmpvalue = VOICE_HEIGH;
					}
#else
					tmpvalue = SendMessage(hBellWnd[3], CB_GETSPINVALUE, 0, 0);
#endif
					tvol = gOptions.AudioVol;

					if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) ||
						((gOptions.TFTKeyLayout==3)&&(LOWORD(wParam)==SCANCODE_BACKSPACE)))
					{
						if(--g_selWave<0) 
						{
							g_selWave=9;
						}
					}
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						if(++g_selWave>9) 
						{
							g_selWave=0;
						}
					}

					SendMessage(hBellWnd[curBellWnd], CB_SETCURSEL, g_selWave, 0);
#ifdef _TTS_
                                        TTS_ExPlayAlarm(g_selWave,tmpvalue);
#else
	                                ExPlayAlarm(g_selWave,tmpvalue);
#endif
					gOptions.AudioVol = tvol;
                                        return 0;
				}
				else if (curBellWnd==3)		//Bell volumn
				{
					if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) ||
						((gOptions.TFTKeyLayout==3)&&(LOWORD(wParam)==SCANCODE_BACKSPACE)))
					{
						//printf("%s index:%d 1\n",__FUNCTION__,SendMessage(hBellWnd[curBellWnd], CB_GETCURSEL, 0, 0));
#ifdef ZEM600                   
                                                if(SendMessage(hBellWnd[curBellWnd], CB_GETCURSEL, 0, 0)<=0)
						        SendMessage(hBellWnd[curBellWnd], CB_SETCURSEL, 2, 0);
                                                else
                                                        SendMessage(hBellWnd[curBellWnd],CB_SPIN,0,0L);
#else 
						SendMessage(hBellWnd[curBellWnd], CB_SPIN, 1, 0L);
#endif
					}
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						//printf("%s index:%d 3\n",__FUNCTION__,SendMessage(hBellWnd[curBellWnd], CB_GETCURSEL, 0, 0));
#ifdef ZEM600                   
                                                if(SendMessage(hBellWnd[curBellWnd], CB_GETCURSEL, 0, 0)==2)
                                                        SendMessage(hBellWnd[curBellWnd], CB_SETCURSEL, 0, 0);
                                                else
                                                        SendMessage(hBellWnd[curBellWnd],CB_SPIN,1,0L);
#else 
						SendMessage(hBellWnd[curBellWnd], CB_SPIN, 0, 0L);
#endif
					}
					return 0;
				}
				else if (curBellWnd==5)		//Status
				{
					g_selStatus = SendMessage(hBellWnd[curBellWnd], CB_GETCURSEL, 0, 0);
					if (g_selStatus ==0) g_selStatus =1;
					else g_selStatus =0; 
                                        SendMessage(hBellWnd[curBellWnd], CB_SETCURSEL, g_selStatus, 0);
                                        return 0;
				}
				else if (curBellWnd==7)
				{
					curBellWnd=8;
					SetFocusChild(hBellWnd[curBellWnd]);
					return 0;
				}
				else if (curBellWnd==8)
				{
					curBellWnd=7;
					SetFocusChild(hBellWnd[curBellWnd]);
					return 0;
				}
			}
			break;	

		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
				case BELL_SAVE:
					if (SaveBellInfo(hWnd))
					{
						BellChanged = 1;
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
					break;
				case BELL_EXIT:
					if (isBellChanged() && 
						MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME), 
								MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
					{
						PostMessage(hWnd, MSG_COMMAND, BELL_SAVE, 0);
					}
					else
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					break;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&sbellbkg);
			DestroyMainWindow(hWnd);
			return 0;
		
	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);

}

int CreateBellEditWindow(HWND hWnd, int* bellid)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	char capstr[40];

	sbellid = *bellid + 1;
	memset(&sbell, 0, sizeof(TBellNew));
	memset(capstr, 0, sizeof(capstr));

	if (sbellid)
	{
		bellopmode=0;		//修改
		if (FDB_GetBell(sbellid, &sbell)==NULL)
			return 0;
		sprintf(capstr, "%s%s%d", LoadStrByID(MID_SMSEDIT), LoadStrByID(MID_ALARM), sbell.BellID);
	}
	else
	{
		bellopmode=1;		//新增
		if (FDB_CreateBell(&sbell, 0, 0, 0, 0, gOptions.AudioVol, 1, 10)==NULL)	//schinfo, hour, min, wave, volume, status, times
			return 0;
		*bellid = sbell.BellID;
		sprintf(capstr, "%s%s%d", LoadStrByID(MID_SMSADD), LoadStrByID(MID_ALARM), sbell.BellID);
	}
	BellChanged = 0;
	
	hWnd = GetMainWindowHandle(hWnd);
	InitMiniGUIExt();

	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = capstr;
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = BellEditWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;
    
	//if (LoadBitmap(HDC_SCREEN, &sbellbkg, GetBmpPath("submenubg.jpg"))) return 0;
	LoadBellStr();
	hMainWnd = CreateMainWindow (&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);
	while(GetMessage(&Msg, hMainWnd))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	MainWindowThreadCleanup (hMainWnd);
	MiniGUIExtCleanUp ();
	return BellChanged;
}

