#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"
#include "exfun.h"
#include <minigui/tftmullan.h>

#define LB_TIME 50
#define ID_HOUR 51
#define LB_HOUR 52
#define ID_MIN 53
#define LB_MIN 54
#define LB_WAVE 55
#define CB_WAVE 56
#define LB_VOL 58
#define CB_VOL 59
#define LB_STATUS 60
#define CB_STATUS 61
#define ID_SAVE 62
#define ID_EXIT 63
#define LB_TIMES 64
#define ID_TIMES 65

HWND EdHour,EdMin,CbWave,CbVol,CbStatus,EdTimes;
HWND Edtmp,focuswnd;
HWND btnSave,btnExit;//btnView,
int  MessageBox1 (HWND hParentWnd, const char* pszText,
		const char* pszCaption, DWORD dwStyle);

static const char* wavefilename[] =
{
	"bell01.wav",
	"bell02.wav",
	"bell03.wav",
	"bell04.wav",
	"bell05.wav",
	"bell06.wav",
	"bell07.wav",
	"bell08.wav",
	"bell09.wav",
	"bell10.wav"
};

ALARM tmpalarm;
U16 g_alarmpin=0;			//alarm id
int g_selstatus=0;			//alarm status
int g_selwave=0;
static int bChanged;
//BITMAP alarmmngbk;

static void SelectNext(WPARAM wParam)
{
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == EdHour)
			focuswnd = EdMin;
		else if (Edtmp == EdMin)
			focuswnd = CbWave;
		else if (Edtmp == CbWave)
			focuswnd = CbVol;
		else if (Edtmp == CbVol)
			focuswnd = EdTimes;
		else if (Edtmp == EdTimes)
			focuswnd = CbStatus;
		else if (Edtmp == CbStatus)
			focuswnd = btnSave;
		else if (Edtmp == btnSave)
			focuswnd = btnExit;
		else if (Edtmp == btnExit)
			focuswnd = EdHour;
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == EdHour)
			focuswnd = btnExit;
		else if (Edtmp == btnExit)
			focuswnd = btnSave;
		else if (Edtmp == btnSave)
			focuswnd = CbStatus;
		else if (Edtmp == CbStatus)
			focuswnd = EdTimes;
		else if (Edtmp == EdTimes)
			focuswnd = CbVol;
		else if (Edtmp == CbVol)
			focuswnd = CbWave;			
		else if (Edtmp == CbWave)
			focuswnd = EdMin;
		else if (Edtmp == EdMin)
			focuswnd = EdHour;
	}

	SetFocusChild(focuswnd);
}

static void InitWindow (HWND hWnd)
{
	char* winHint[9];
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=220;
		posX2=150;
		posX3=140;
		posX4=80;
		posX5=70;
		posX6=70;
		posX7=15;
		posX8=10;
	}
	else
	{
		posX1=20+gOptions.ControlOffset;
		posX2=100+gOptions.ControlOffset;
		posX3=135+gOptions.ControlOffset;
		posX4=160+gOptions.ControlOffset;
		posX5=195+gOptions.ControlOffset;
		posX6=100+gOptions.ControlOffset;
		posX7=0;
		posX8=0;
	}
	winHint[0] = LoadStrByID(MID_ALARMTIME);
	winHint[1] = LoadStrByID(MID_HOUR1);
	winHint[2] = LoadStrByID(MID_MINUTE);
	winHint[3] = LoadStrByID(MID_ALARMWVSEL);
	winHint[4] = LoadStrByID(MID_ALARMVOLADV);
	winHint[5] = LoadStrByID(MID_ALARM_TIMES);
	winHint[6] = LoadStrByID(MID_ALARMSTATUS);
	winHint[7] = LoadStrByID(MID_SAVE);
	winHint[8] = LoadStrByID(MID_EXIT);

	CreateWindow(CTRL_STATIC,winHint[0],WS_VISIBLE|SS_CENTER,LB_TIME,posX1,20,75,23,hWnd,0);
	EdHour=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,ID_HOUR,posX2+posX7,16,30,23,hWnd,0);
	CreateWindow(CTRL_STATIC,winHint[1],WS_VISIBLE|SS_CENTER,LB_HOUR,posX3,20,15,23,hWnd,0);
	EdMin=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,ID_MIN,posX4,16,30,23,hWnd,0);
	CreateWindow(CTRL_STATIC,winHint[2],WS_VISIBLE|SS_CENTER,LB_MIN,posX5,20,15,23,hWnd,0);

	CreateWindow(CTRL_STATIC,winHint[3],WS_VISIBLE|SS_CENTER,LB_WAVE,posX1,53,75,23,hWnd,0);
	CbWave=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
			CB_WAVE,posX6-10,49,145,23,hWnd, 0);

	CreateWindow (CTRL_STATIC,winHint[4], WS_VISIBLE | SS_CENTER, LB_VOL, posX1, 86, 75, 23, hWnd, 0);
#ifdef ZEM600
	CbVol=CreateWindow(CTRL_COMBOBOX,"", WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST |
			CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, CB_VOL, posX2-10, 82, 65, 23, hWnd, 0);
#else
	CbVol=CreateWindow(CTRL_COMBOBOX,"", WS_VISIBLE|WS_TABSTOP|CBS_READONLY|CBS_AUTOSPIN|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOLOOP|
			CBS_EDITBASELINE, CB_VOL, posX2-10, 82, 65, 23, hWnd, 0);
#endif
	CreateWindow (CTRL_STATIC,winHint[5], WS_VISIBLE | SS_CENTER, LB_TIMES, posX1, 119, 75, 23, hWnd, 0);
	EdTimes=CreateWindow(CTRL_SLEDIT,"", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
			ID_TIMES,posX2,113,45,23,hWnd,0);

	CreateWindow (CTRL_STATIC,winHint[6], WS_VISIBLE | SS_CENTER,	LB_STATUS, posX1, 152, 75, 23, hWnd, 0);
	CbStatus=CreateWindow(CTRL_COMBOBOX,"",	WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
			CB_STATUS, posX2-10-posX8, 146, 75, 23, hWnd, 0);

	btnSave = CreateWindow(CTRL_BUTTON, winHint[7], WS_VISIBLE | BS_DEFPUSHBUTTON|WS_BORDER, ID_SAVE,10,190,85,23,hWnd,0);
	btnExit = CreateWindow(CTRL_BUTTON, winHint[8], WS_VISIBLE | BS_DEFPUSHBUTTON|WS_BORDER, ID_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);
}

int CheckSameTime(int h, int m,int pin)
{
	int i, ret=0;
	ALARM talarm1;

	for(i=1; i<gOptions.AlarmMaxCount+1; i++)
	{
		if ((FDB_GetAlarm(i, &talarm1)!=NULL) && (talarm1.AlarmIDX!=pin))
		{
			if(talarm1.AlarmHour==h && talarm1.AlarmMin==m && talarm1.AlarmStatus==1)
			{
				ret=1;
				break;
			}
		}
	}
	return ret;
}

static int SaveAlarmInfo(HWND hWnd)
{
	char tmpc[4];
	int ihour,imin,itimes;
	int ret=0;

	memset(tmpc,0,4);
	GetWindowText(EdHour,tmpc,2);
	ihour=atoi(tmpc);
	memset(tmpc,0,4);
	GetWindowText(EdMin,tmpc,2);
	imin=atoi(tmpc);
	memset(tmpc,0,4);
	GetWindowText(EdTimes,tmpc,2);
	itimes=atoi(tmpc);
	if(itimes<=0)
	{
		MessageBox1(hWnd,LoadStrByID(MID_ALARMVOLHINT), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
		if(!ismenutimeout) SetFocusChild(EdTimes);
		return 0;
	}

	tmpalarm.AlarmIDX = g_alarmpin;
	tmpalarm.AlarmHour = ihour;
	tmpalarm.AlarmMin = imin;
	tmpalarm.AlarmWaveIDX = SendMessage(CbWave,CB_GETCURSEL,0,0);
#ifdef ZEM600
	//tmpalarm.AlarmAudioVol = (SendMessage(CbVol,CB_GETCURSEL,0,0)*8+14)*3;
	int value = SendMessage(CbVol,CB_GETCURSEL,0,0);
	if(value==0)
		tmpalarm.AlarmAudioVol=VOICE_LOW;
	else if (value == 1)
		tmpalarm.AlarmAudioVol=VOICE_MIDDLE;
	else if (value == 2)
		tmpalarm.AlarmAudioVol=VOICE_HEIGH;	
#else
	tmpalarm.AlarmAudioVol = SendMessage(CbVol,CB_GETSPINVALUE,0,0);
#endif
	tmpalarm.AlarmStatus = SendMessage(CbStatus,CB_GETCURSEL,0,0);
	tmpalarm.AlarmTimes = itimes;

	if(tmpalarm.AlarmStatus==1)
	{
		if(CheckSameTime(ihour,imin,g_alarmpin) == 0)
			ret=(FDB_ChgAlarm(&tmpalarm)==FDB_OK);
		else
			MessageBox1(hWnd,LoadStrByID(MID_TIMEERROR), LoadStrByID(MID_APPNAME),MB_OK | MB_ICONSTOP | MB_BASEDONPARENT);
	}
	else
		ret=(FDB_ChgAlarm(&tmpalarm)==FDB_OK);
	return ret;
}

static int ismodified()
{
	char tmpstr[4];

	memset(tmpstr,0,4);
	GetWindowText(EdHour,tmpstr,2);
	if(atoi(tmpstr) != tmpalarm.AlarmHour) return 1;

	memset(tmpstr,0,4);
	GetWindowText(EdMin,tmpstr,2);
	if(atoi(tmpstr) != tmpalarm.AlarmMin) return 1;

	memset(tmpstr,0,4);
	GetWindowText(EdTimes,tmpstr,2);
	if(atoi(tmpstr) != tmpalarm.AlarmTimes) return 1;

	if(SendMessage(CbWave,CB_GETCURSEL,0,0)!=tmpalarm.AlarmWaveIDX) return 1;
#ifdef ZEM600
	//if(SendMessage(CbVol,CB_GETSPINVALUE,0,0)!=(tmpalarm.AlarmAudioVol/3-14)/8) return 1;
	int value = 0;
	if(tmpalarm.AlarmAudioVol<=VOICE_LOW)
		value = 0;
	else if(tmpalarm.AlarmAudioVol>VOICE_LOW && tmpalarm.AlarmAudioVol<=VOICE_MIDDLE)
		value = 1;
	else
		value = 2;

	if(SendMessage(CbVol,CB_GETCURSEL,0,0)!=value)
		return 1;
#else
	if(SendMessage(CbVol,CB_GETSPINVALUE,0,0)!=tmpalarm.AlarmAudioVol) return 1;
#endif
	if(SendMessage(CbStatus,CB_GETCURSEL,0,0)!=tmpalarm.AlarmStatus) return 1;

	return 0;
}

static int alarmwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char tmpstr[20];
	int tmpvalue = 0;
	int id,nc;
	int i;
	static char keyupFlag=0;
	switch (message)	
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&alarmmngbk,GetBmpPath("submenubg.jpg")))
			//        return 0;
			InitWindow(hWnd);		//add controls

			memset(tmpstr,0,4);
			sprintf(tmpstr,"%02d", tmpalarm.AlarmHour);
			SetWindowText(EdHour,tmpstr);
			memset(tmpstr,0,4);
			sprintf(tmpstr,"%02d",tmpalarm.AlarmMin);
			SetWindowText(EdMin,tmpstr);
			memset(tmpstr,0,4);
			sprintf(tmpstr,"%d",tmpalarm.AlarmTimes);
			SetWindowText(EdTimes,tmpstr);

			for(i=0;i<10;i++)
			{
				SendMessage(CbWave, CB_ADDSTRING, 0, (LPARAM)wavefilename[i]);
			}
#ifdef ZEM600
			for(i=0;i<3;i++)
			{
				SendMessage(CbVol,CB_ADDSTRING ,0,(LPARAM)LoadStrByID(MID_VIDEO_QLOW+i));
			}
			int value=0;
			if(tmpalarm.AlarmAudioVol<=VOICE_LOW)
			{
				tmpalarm.AlarmAudioVol=VOICE_LOW;
				value=0;
			}
			else if(tmpalarm.AlarmAudioVol>VOICE_LOW && tmpalarm.AlarmAudioVol<=VOICE_MIDDLE)
			{
				tmpalarm.AlarmAudioVol=VOICE_MIDDLE;
				value=1;
			}
			else
			{
				tmpalarm.AlarmAudioVol=VOICE_HEIGH;
				value=2;
			}
			SendMessage(CbVol,CB_SETCURSEL,value,0);


			//SendMessage(CbVol,CB_SETCURSEL,((tmpalarm.AlarmAudioVol<42?42:tmpalarm.AlarmAudioVol)/3-14)/8,0);
#else
			SendMessage(CbVol,CB_SETSPINRANGE,0,100);
			SendMessage(CbVol,CB_SETSPINPACE,1,1);
			SendMessage(CbVol,CB_SETSPINFORMAT,0,(LPARAM)"%d%%");
			SendMessage(CbVol,CB_SETSPINVALUE,tmpalarm.AlarmAudioVol,0);
#endif

			SendMessage(CbStatus,CB_ADDSTRING,0,(LPARAM)LoadStrByID(MID_ALARMSTOP));
			SendMessage(CbStatus,CB_ADDSTRING,0,(LPARAM)LoadStrByID(MID_ALARMSTART1));
			SendMessage(CbStatus,CB_SETCURSEL,tmpalarm.AlarmStatus,0);

			SendMessage(EdHour,EM_LIMITTEXT,2,0L);
			SendMessage(EdMin,EM_LIMITTEXT,2,0L);
			SendMessage(EdTimes,EM_LIMITTEXT,2,0L);

			SendMessage(CbWave,CB_SETCURSEL,tmpalarm.AlarmWaveIDX,0);

			UpdateWindow(hWnd,TRUE);
			SetFocusChild(EdHour);
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);	
			EndPaint(hWnd,hdc);
			return 0;

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

			Edtmp = GetFocusChild(hWnd);
			if(LOWORD(wParam)==SCANCODE_ENTER) 
			{
				if(Edtmp!=btnExit && Edtmp!=btnSave)
					PostMessage(hWnd,MSG_COMMAND, (WPARAM)ID_SAVE,0L);
			}

			if(LOWORD(wParam)==SCANCODE_F10)
			{
				if(Edtmp!=btnExit)
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
				else
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_EXIT, 0L);
			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				SelectNext(wParam);
				return 0;
			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) || 
					((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)||(gOptions.TFTKeyLayout==3&&LOWORD(wParam)==SCANCODE_BACKSPACE)))
			{
				if(Edtmp==btnSave)
					SetFocusChild(btnExit);
				else if(Edtmp==btnExit)
					SetFocusChild(btnSave);
				else if(Edtmp==CbStatus)
				{
					g_selstatus = SendMessage(CbStatus,CB_GETCURSEL,0,0);
					if (g_selstatus ==0) g_selstatus =1;
					else g_selstatus =0;
					SendMessage(CbStatus,CB_SETCURSEL,g_selstatus ,0);
					return 0;
				}
				else if(Edtmp==CbVol)
				{
					if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) ||
							((gOptions.TFTKeyLayout==3)&&(LOWORD(wParam)==SCANCODE_BACKSPACE)))
					{
#ifdef ZEM600
						if(SendMessage(CbVol, CB_GETCURSEL, 0, 0)==0)
							SendMessage(CbVol, CB_SETCURSEL, 2, 0);
						else
							SendMessage(CbVol,CB_SPIN,0,0L);
#else
						SendMessage(CbVol,CB_SPIN,1,0L);
#endif
					}

					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
#ifdef ZEM600
						if(SendMessage(CbVol, CB_GETCURSEL, 0, 0)==2)
							SendMessage(CbVol, CB_SETCURSEL, 0, 0);
						else
							SendMessage(CbVol,CB_SPIN,1,0L);
#else
						SendMessage(CbVol,CB_SPIN,0,0L);
#endif
					}
					return 0;
				}
				else if(Edtmp==CbWave)
				{
					int tvol;
					g_selwave = SendMessage(CbWave,CB_GETCURSEL,0,0);
#ifdef ZEM600
					//tmpvalue = (SendMessage(CbVol,CB_GETSPINVALUE,0,0)*8+14)*3;
					int value = SendMessage(CbVol,CB_GETCURSEL,0,0);
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
					tmpvalue = SendMessage(CbVol,CB_GETSPINVALUE,0,0);
#endif
					if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) ||
							((gOptions.TFTKeyLayout==3)&&(LOWORD(wParam)==SCANCODE_BACKSPACE)))
					{
						if(--g_selwave<0) 
							g_selwave=9;
						tvol = gOptions.AudioVol;
#ifdef _TTS_
						TTS_ExPlayAlarm(g_selwave,tmpvalue);
#else
						ExPlayAlarm(g_selwave,tmpvalue);
#endif                                        	

						gOptions.AudioVol = tvol;
						SendMessage(CbWave,CB_SETCURSEL,g_selwave,0);
					}
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						if(++g_selwave>9) g_selwave=0;
						tvol = gOptions.AudioVol;
#ifdef _TTS_
						TTS_ExPlayAlarm(g_selwave,tmpvalue);
#else
						ExPlayAlarm(g_selwave,tmpvalue);
#endif
						gOptions.AudioVol = tvol;
						SendMessage(CbWave,CB_SETCURSEL,g_selwave,0);
					}
					return 0;
				}
				break;
			}

			if(LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0L);
			}
			if(LOWORD(wParam)==SCANCODE_MENU)
			{
				PostMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
			}
			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(nc==EN_CHANGE)
			{
				if(id==ID_HOUR)
				{
					memset(tmpstr,0,4);
					GetWindowText(EdHour,tmpstr,2);
					tmpvalue = atoi(tmpstr);
					if(tmpvalue<0 || tmpvalue>23)
					{
						memset(tmpstr,0,20);
						sprintf(tmpstr,"%02d",tmpalarm.AlarmHour);
						SetWindowText(EdHour,tmpstr);
						SendMessage(EdHour, EM_SETCARETPOS, 0, strlen(tmpstr));
					}
				}
				else if(id==ID_MIN)
				{
					memset(tmpstr,0,4);
					GetWindowText(EdMin,tmpstr,2);
					tmpvalue = atoi(tmpstr);
					if(tmpvalue<0 || tmpvalue>59)
					{
						memset(tmpstr,0,20);
						sprintf(tmpstr,"%02d",tmpalarm.AlarmMin);
						SetWindowText(EdMin,tmpstr);
						SendMessage(EdMin, EM_SETCARETPOS, 0, strlen(tmpstr));
					}
				}
			}

			switch(id)
			{
				case ID_SAVE:			//±£´æ
					if(SaveAlarmInfo(hWnd))	
					{
						bChanged = 1;
						PostMessage(hWnd,MSG_CLOSE,0,0L);
					}
					break;
				case ID_EXIT:				//·µ»Ø
					if(ismodified() && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME), MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK)
					{
						if(SaveAlarmInfo(hWnd))
							bChanged = 1;
					}
					else
						bChanged = 0;

					if(!ismenutimeout) PostMessage(hWnd,MSG_CLOSE,0,0L);
					break;
			}			
			break;			

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

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
				if(fGetDC)
					ReleaseDC (hdc);

				return 0;
			}
		case MSG_CLOSE:
			//UnloadBitmap(&alarmmngbk);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateAlarmSetingWindow(HWND hWnd, int *selPIN)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	g_alarmpin = *selPIN + 1;
	FDB_GetAlarm(g_alarmpin,&tmpalarm);
	bChanged = 0;

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_ALARMEDIT);
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = alarmwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hMainWnd);
	return bChanged;

}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

