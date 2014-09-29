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
#include "options.h"
#include "ssrcommon.h"
#include "exfun.h"
#include "main.h"
#include <minigui/tftmullan.h>

#define IDC_FREETIME	301
#define IDC_BRIGHTNESS	302
#define IDC_BELL	303
#define IDC_VIDEO	304
#define IDC_FPIMG	305
#define IDC_SMSTIME	306
#define IDC_WORKCODE	307
#define IDC_LOCKPOWER	308
#define ID_POWER	309
#define IDC_MUST121 310
#define IDC_ATTPHOTO 312

#ifdef FACE
#define IDC_FACE        311
extern int CreateFaceSettingWindow(HWND hWnd);
#endif

HWND SysOtherWnd[15];//change 11 to 12 Liaozz 20080925 13 mullanguage
//BITMAP SysOtherBkg;
int SysOtherCurWnd;
int v_brightness, v_contrast, v_quality, v_scene;
int g_fpsel;
int isPowerOff = 0;


extern int freetime;
extern void GPIO_TFT_BACK_LIGHT(BOOL, int);
extern int ifUseWorkCode;
extern HWND hMainWindowWnd;

int LngCnt=0;
int Lngs[50];
U32  mullan=0;
char* OtherStr[26];

static void LoadWindowStr(void)
{
	OtherStr[0]=LoadStrByID(HIT_SYSTEM3ITEM1);
	OtherStr[1]=LoadStrByID(HIT_FREETIME);
	if (gOptions.LcdMode)
	{
		OtherStr[2]=LoadStrByID(MID_BRIGHT_ADV);
		OtherStr[3]=LoadStrByID(MID_BRIGHT_HINT);
	}
	if (gOptions.IsSupportExtBell)
	{
		OtherStr[4]=LoadStrByID(MID_BELL_EXT);
		OtherStr[7]=LoadStrByID(MID_ALARMSTOP);
		OtherStr[8]=LoadStrByID(MID_ALARMSTART1);
	}
	if (gOptions.CameraOpen)
	{
		OtherStr[5]=LoadStrByID(MID_VIDEO_SETTING);
		OtherStr[6]=LoadStrByID(MID_VIDEO_ADJUST);
	}
#ifdef FACE
	else if(gOptions.FaceFunOn)
	{
		OtherStr[24]=LoadStrByID(FACE_FACESET);
		OtherStr[25]=LoadStrByID(MID_FACE_SET);
	}
#endif
	if (!gOptions.IsOnlyRFMachine)
	{
		OtherStr[9]=LoadStrByID(MID_FPIMG);
		OtherStr[10]=LoadStrByID(MID_FPREGSHOW);
		OtherStr[11]=LoadStrByID(MID_FPVFSHOW);
		OtherStr[12]=LoadStrByID(MID_FPSHOW);
		OtherStr[13]=LoadStrByID(MID_FPNONESHOW);
	}
#ifdef IKIOSK
	OtherStr[14]=LoadStrByID(MID_IKIOSK_SMSDELAY);
	OtherStr[15]=LoadStrByID(MID_WORKCODE);
#endif
	if (gOptions.HavePowerKey)
	{
		OtherStr[16]=LoadStrByID(HIT_SWITCH2);
		OtherStr[17]=LoadStrByID(HIT_SWITCH1);
		OtherStr[18]=LoadStrByID(MID_LOCK_POWER);
		OtherStr[19]=LoadStrByID(MID_POWER_OFF);
	}
	OtherStr[20]=LoadStrByID(MID_OS_LANGUAGE);
	if (IDTFlag) 
	{
		OtherStr[21]=LoadStrByID(MID_MUST121);
		OtherStr[22]=LoadStrByID(MID_MUST121_NO);
		OtherStr[23]=LoadStrByID(MID_MUST121_YES);
	}
#ifdef _TTS_
	OtherStr[24]=LoadStrByID(MID_TTS_OPEN);
#endif		
}

static void InitSystemOtherWin(HWND hWnd)
{
	HWND tmpstatic[2];
	HWND must121static;//Static text control for must 121 Liaozz 20080925 xs0809191001
	char tmpstr[8];
	int rows = 0;
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=220;
		posX2=150;
		posX3=50;
		posX4=10;
		posX5=50;
		posX6=30;
		posX7=85;
	}
	else
	{
		posX1=10;
		posX2=115;
		posX3=165;
		posX4=175;
		posX5=115;
		posX6=0;
		posX7=115;
	}

	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"%d",gOptions.FreeTime);
	CreateWindow(CTRL_STATIC, OtherStr[0], WS_VISIBLE | SS_LEFT, 0x11018, posX1, rows*25+10, 100, 23, hWnd, 0);
	SysOtherWnd[0] = CreateWindow(CTRL_SLEDIT, tmpstr, WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT | WS_BORDER, IDC_FREETIME, 
			posX2, rows*25+6, 40, 23, hWnd, 0);
	SendMessage(SysOtherWnd[0], EM_LIMITTEXT, 3, 0);
	CreateWindow(CTRL_STATIC, OtherStr[1], WS_VISIBLE | SS_LEFT, 0x11019, posX3, rows*25+10, 150, 23, hWnd, 0);
	rows++;

	if (gOptions.LcdMode)
	{
		tmpstatic[0] = CreateWindow(CTRL_STATIC, OtherStr[2], SS_LEFT, 0x11020, posX1, rows*25+10, 100, 23, hWnd, 0);
		SysOtherWnd[1] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | CBS_READONLY | CBS_AUTOSPIN | CBS_SPINARROW_LEFTRIGHT | 
				CBS_AUTOLOOP | CBS_EDITBASELINE, IDC_BRIGHTNESS, posX2, rows*25+6, 60, 23, hWnd, 0);
		tmpstatic[1] = CreateWindow(CTRL_STATIC, OtherStr[3], SS_LEFT, 0x11021, posX4, rows*25+10, 150, 23, hWnd, 0);

		SendMessage(SysOtherWnd[1], CB_SETSPINRANGE, 10, 100);
		SendMessage(SysOtherWnd[1], CB_SETSPINPACE, 10, 10); 
		SendMessage(SysOtherWnd[1], CB_SETSPINFORMAT, 0, (LPARAM)"%d%%");
		SendMessage(SysOtherWnd[1], CB_SETSPINVALUE, gOptions.Brightness, 0);
		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(SysOtherWnd[1], SW_SHOW);
		ShowWindow(tmpstatic[1], SW_SHOW);
		rows++;
	}

	// Extra bell
	if (gOptions.IsSupportExtBell)
	{
		tmpstatic[0] = CreateWindow(CTRL_STATIC, OtherStr[4], SS_LEFT, 0x11022, posX1, rows*25+10, 100, 23, hWnd, 0);
		SysOtherWnd[2] = CreateWindow(CTRL_COMBOBOX, "", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, IDC_BELL,
				posX2, rows*25+6, 60, 23, hWnd, 0);

		SendMessage(SysOtherWnd[2], CB_ADDSTRING, 0, (LPARAM)OtherStr[7]);
		SendMessage(SysOtherWnd[2], CB_ADDSTRING, 0, (LPARAM)OtherStr[8]);
		SendMessage(SysOtherWnd[2], CB_SETCURSEL, gOptions.isSupportAlarmExt, 0);
		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(SysOtherWnd[2], SW_SHOW);
		rows++;   
	}

	// Video adjust
	if (gOptions.CameraOpen)
	{
		tmpstatic[0] = CreateWindow(CTRL_STATIC, OtherStr[5], SS_LEFT, 0x11023, posX1, rows*25+10, 100, 23, hWnd, 0);
		SysOtherWnd[3] = CreateWindow(CTRL_BUTTON, OtherStr[6], WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_VIDEO, 
				posX5, rows*25+6, 160, 23, hWnd, 0);

		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(SysOtherWnd[3], SW_SHOW);	
		rows++;
	}
#ifdef FACE
	else if(gOptions.FaceFunOn)	//Face Param setting
	{
		tmpstatic[0] = CreateWindow(CTRL_STATIC, OtherStr[24],SS_LEFT, 0x11031, posX1, rows*25+10, 100, 23, hWnd, 0);
		SysOtherWnd[3] = CreateWindow(CTRL_BUTTON, OtherStr[25], WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_FACE, 
				posX5, rows*25+6, 160, 23, hWnd, 0);

		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(SysOtherWnd[3], SW_SHOW);	
		rows++;
	}

#endif

	// Finger print
	if (!gOptions.IsOnlyRFMachine)
	{
		tmpstatic[0]=CreateWindow(CTRL_STATIC, OtherStr[9], SS_LEFT, 0x11024, posX1, rows*25+10, 100, 23, hWnd, 0);
		SysOtherWnd[4] = CreateWindow(CTRL_COMBOBOX, "", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,	
				IDC_FPIMG, posX5, rows*25+6, 160, 23, hWnd, 0);

		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(SysOtherWnd[4], SW_SHOW);
		SendMessage(SysOtherWnd[4], CB_ADDSTRING, 0, (LPARAM)OtherStr[10]);
		SendMessage(SysOtherWnd[4], CB_ADDSTRING, 0, (LPARAM)OtherStr[11]);
		SendMessage(SysOtherWnd[4], CB_ADDSTRING, 0, (LPARAM)OtherStr[12]);
		SendMessage(SysOtherWnd[4], CB_ADDSTRING, 0, (LPARAM)OtherStr[13]);
		g_fpsel = gOptions.FpSelect;
		SendMessage(SysOtherWnd[4], CB_SETCURSEL, g_fpsel, 0);
		rows++;
	}

	// SMS delay
#ifdef IKIOSK
	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "%d", gOptions.SMSTime);
	tmpstatic[0] = CreateWindow(CTRL_STATIC, OtherStr[14], SS_LEFT, 0x11025, posX1, rows*28+10, 100, 23, hWnd, 0);
	SysOtherWnd[5] = CreateWindow(CTRL_SLEDIT, tmpstr, WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT | WS_BORDER, IDC_SMSTIME,
			posX2, rows*28+6, 40, 23, hWnd, 0);
	tmpstatic[1] = CreateWindow(CTRL_STATIC, OtherStr[15], SS_LEFT, 0x11026, posX1, (rows+1)*28+10, 100, 23, hWnd, 0);
	SysOtherWnd[6] = CreateWindow(CTRL_COMBOBOX, "", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
			IDC_WORKCODE, posX2, (rows+1)*28+6, 60, 23, hWnd, 0);
	SendMessage(SysOtherWnd[6], CB_ADDSTRING, 0, (LPARAM)OtherStr[16]);
	SendMessage(SysOtherWnd[6], CB_ADDSTRING, 0, (LPARAM)OtherStr[17]);
	if (gOptions.IsSupportSMS)
	{
		SendMessage(SysOtherWnd[5], EM_LIMITTEXT, 3, 0L);
		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(SysOtherWnd[5], SW_SHOW);
		rows++;
	}
	if (gOptions.WorkCode)
	{
		ShowWindow(tmpstatic[1], SW_SHOW);
		ShowWindow(SysOtherWnd[6], SW_SHOW);
		SendMessage(SysOtherWnd[6], CB_SETCURSEL, ifUseWorkCode, 0);
		rows++;
	}
#endif

	//锁定关机键
	if (gOptions.HavePowerKey)
	{
		tmpstatic[0] = CreateWindow(CTRL_STATIC, OtherStr[18], SS_LEFT, 0x11027, posX1, rows*25+10, 100, 23, hWnd, 0);
		SysOtherWnd[7] = CreateWindow(CTRL_COMBOBOX, "", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
				IDC_LOCKPOWER, posX2, rows*25+6, 60, 23, hWnd, 0);
		SysOtherWnd[8] = CreateWindow(CTRL_BUTTON, OtherStr[19], WS_TABSTOP | BS_DEFPUSHBUTTON,
				ID_POWER, 220, 120, 85, 23, hWnd, 0);

		SendMessage(SysOtherWnd[7], CB_ADDSTRING, 0, (LPARAM)OtherStr[16]);
		SendMessage(SysOtherWnd[7], CB_ADDSTRING, 0, (LPARAM)OtherStr[17]);
		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(SysOtherWnd[7], SW_SHOW);
		//		if (gOptions.LockPowerKey) ShowWindow(SysOtherWnd[8], SW_SHOW);
		SendMessage(SysOtherWnd[7], CB_SETCURSEL, gOptions.LockPowerKey, 0);
		rows++;
	}

	//process mullanguage
	LngCnt=GetSupportedLang(Lngs, 50);
	if (gOptions.MultiLanguage && LngCnt>1)
	{
		static int i,sys6_index;
		tmpstatic[1] = CreateWindow(CTRL_STATIC, OtherStr[20], SS_LEFT,0x11028, posX1, rows*25+10, 100, 23, hWnd,0);
		SysOtherWnd[9] = CreateWindow(CTRL_COMBOBOX, "", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, 0x11029, posX7, rows*25+6, 120, 23, hWnd, 0);

		ShowWindow(tmpstatic[1], SW_SHOW);
		ShowWindow(SysOtherWnd[9], SW_SHOW);
		rows++;
		for(i=0;i<LngCnt;i++)
		{
			char *ln=GetLangName(Lngs[i]);

			if (ln && strlen(ln))
			{
				SendMessage(SysOtherWnd[9],CB_ADDSTRING ,0,(LPARAM)ln);
				if(Lngs[i]==gOptions.Language) sys6_index=i;
			}
		}
		SendMessage(SysOtherWnd[9],CB_SETCURSEL,sys6_index,0);
		mullan=1;
	}



	if (IDTFlag) {
		must121static = CreateWindow(CTRL_STATIC, OtherStr[21], SS_LEFT, 0x11030, posX1, rows*25+10, 100, 23, hWnd, 0);
		SysOtherWnd[10] = CreateWindow(CTRL_COMBOBOX, "", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
				IDC_MUST121, posX2, rows*25+6, 60, 23, hWnd, 0);
		SendMessage(SysOtherWnd[10], CB_ADDSTRING, 0, (LPARAM)OtherStr[22]);
		SendMessage(SysOtherWnd[10], CB_ADDSTRING, 0, (LPARAM)OtherStr[23]);
		ShowWindow(must121static, SW_SHOW);
		ShowWindow(SysOtherWnd[10], SW_SHOW);
		SendMessage(SysOtherWnd[10], CB_SETCURSEL, gOptions.Must1To1, 0);
		rows++;
	}

	//控制TTS整点报时的开关
#ifdef _TTS_
	tmpstatic[1] = CreateWindow(CTRL_STATIC, OtherStr[24], SS_LEFT,0x11028, posX1, rows*25+10, 100, 23, hWnd,0);
	SysOtherWnd[11] = CreateWindow(CTRL_COMBOBOX, "", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, 0x11029, posX2, rows*25+6, 120, 23, hWnd, 0);

	ShowWindow(tmpstatic[1], SW_SHOW);
	ShowWindow(SysOtherWnd[11], SW_SHOW);
	rows++;

	char name[10]="No";
	int index=0, i=0;
	sprintf(name, "%s", LoadStrByID(HID_NO));
	for(i=0;i<2;i++)
	{
		SendMessage(SysOtherWnd[11],CB_ADDSTRING ,0,(LPARAM)name);
		sprintf(name, "%s", LoadStrByID(HID_YES));
	}
	SendMessage(SysOtherWnd[11],CB_SETCURSEL, gOptions.ttsIntegralPointOpen, 0);
#endif

/*
	if (gOptions.CameraOpen)
	{
		tmpstatic[0] = CreateWindow(CTRL_STATIC, LoadStrByID(PID_PHOTO_MNG), SS_LEFT, 0x11023, posX1, rows*25+10, 100, 23, hWnd, 0);
		SysOtherWnd[12] = CreateWindow(CTRL_BUTTON, LoadStrByID(PID_PHOTO_MNG), WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_ATTPHOTO, 
				posX5, rows*25+6, 160, 23, hWnd, 0);

		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(SysOtherWnd[12], SW_SHOW);	
		rows++;
	}
*/
	SysOtherWnd[12] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_OK), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDOK, 220+gOptions.ControlOffset, 160, 85, 23, hWnd, 0);
	SysOtherWnd[13] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_CANCEL), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDCANCEL, 220+gOptions.ControlOffset, 190, 85, 23, hWnd, 0);
}

static int isSysOtherChanged(void)
{
	char temp[20];
	int res=0;

	GetWindowText(SysOtherWnd[0], temp, 4);
	if (strlen(temp)>0 && atoi(temp) != gOptions.FreeTime) {
		res++;
	}
	if(gOptions.LcdMode && SendMessage(SysOtherWnd[1], CB_GETSPINVALUE, 0, 0) != gOptions.Brightness) {
		res++;
	}
	if (!gOptions.LockFunOn && SendMessage(SysOtherWnd[2], CB_GETCURSEL, 0, 0) != gOptions.isSupportAlarmExt) {
		res++;
	}
	if (gOptions.CameraOpen && (gOptions.CameraBright != v_brightness || gOptions.CameraContrast != v_contrast ||
				gOptions.PicQuality != v_quality || gOptions.CameraScene != v_scene)) {
		res++;
	}
	if (!gOptions.IsOnlyRFMachine && g_fpsel != gOptions.FpSelect) {
		res++;
	}
#ifdef IKIOSK
	GetWindowText(SysOtherWnd[5], temp, 4);
	if (strlen(temp)>0 && atoi(temp)!=gOptions.SMSTime)
		res++;
	if (SendMessage(SysOtherWnd[6], CB_GETCURSEL, 0, 0) != LoadInteger("UseWorkCode", 0))
		res++;
#endif

	if (gOptions.HavePowerKey && SendMessage(SysOtherWnd[7], CB_GETCURSEL, 0, 0)!= gOptions.LockPowerKey) {
		res++;
	}
	//Liaozz 20080925 xs200809191001
	if (IDTFlag && SendMessage(SysOtherWnd[9], CB_GETCURSEL, 0, 0)!= gOptions.Must1To1) {
		res ++;
	}
	if(mullan && (gOptions.Language !=Lngs[SendMessage(SysOtherWnd[9],CB_GETCURSEL,0,0)]))
	{
		res++;
	}
	//Liaozz end
#ifdef _TTS_
	if(gOptions.ttsIntegralPointOpen !=SendMessage(SysOtherWnd[11],CB_GETCURSEL,0,0))
	{
		res++;
	}
#endif

	return res;
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
extern int SSR_MENU_ATTPHOTO(HWND hWnd);
static int SaveSysOtherSetting(HWND hWnd)
{
	char c1[4];
	int sel;
	int mulFlag=0;

	GetWindowText(SysOtherWnd[0], c1, 3);
	if(CheckText2(c1,0,999))
	{
		gOptions.FreeTime = atoi(c1);
	}
	else
	{
		MessageBox1 (hWnd, LoadStrByID(HIT_ERROR0), LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	if(gOptions.LcdMode)
		gOptions.Brightness = SendMessage(SysOtherWnd[1], CB_GETSPINVALUE, 0, 0);

	if (!gOptions.LockFunOn)
		gOptions.isSupportAlarmExt = SendMessage(SysOtherWnd[2], CB_GETCURSEL, 0, 0);

	if (gOptions.CameraOpen)
	{
		gOptions.CameraBright = v_brightness;
		gOptions.CameraContrast = v_contrast;
		gOptions.PicQuality = v_quality;
		gOptions.CameraScene = v_scene;
	}

#ifdef IKIOSK
	if (gOptions.IsSupportSMS)
	{
		GetWindowText(SysOtherWnd[5], c1, 4);
		if (CheckText2(c1, 10, 999))
		{
			gOptions.SMSTime = atoi(c1);
			//		printf ("SMS Time:%d\n", gOptions.SMSTime);
		}
		else
		{
			MessageBox1 (hWnd, LoadStrByID(HIT_ERROR0), LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
			return 0;
		}
	}

	if (gOptions.WorkCode)
	{
		SaveInteger("UseWorkCode", SendMessage(SysOtherWnd[6], CB_GETCURSEL, 0, 0));
		ifUseWorkCode = LoadInteger("UseWorkCode", 0);
	}
#endif

	if (!gOptions.IsOnlyRFMachine)
		gOptions.FpSelect = g_fpsel;

	if (gOptions.HavePowerKey)
		gOptions.LockPowerKey = SendMessage(SysOtherWnd[7], CB_GETCURSEL, 0, 0);		//锁定关机键
	//Liaozz 20080925 xs0809191001
	if (IDTFlag)
		gOptions.Must1To1 = SendMessage(SysOtherWnd[9], CB_GETCURSEL, 0, 0);

#ifdef _TTS_

	gOptions.ttsIntegralPointOpen = SendMessage(SysOtherWnd[11], CB_GETCURSEL, 0, 0);

#endif
	//Liaozz end
	SaveOptions(&gOptions);
	if (mullan)
	{
		sel=SendMessage(SysOtherWnd[9],CB_GETCURSEL,0,0);
		if (Lngs[sel] != gOptions.Language)
		{
			SaveInteger("Language", Lngs[sel]);
			SaveInteger("NewLng", Lngs[sel]);
			SaveInteger("ChangeLng", 1);
			if (gOptions.IMEFunOn)
				SaveInteger("HzImeOn",Lngs[sel]==83?1:0);       //add by jazzy 2009.06.10 for not Chinese set zero
			//gOptions.Language=Lngs[sel];
			//SelectLanguage(gOptions.Language);
			//FDB_ClearData(FCT_SHORTKEY);
			//FDB_InitShortKey();
			//MessageBox1(hWnd, LoadStrByID(HID_RESTART), HIT_RUN, MB_OK | MB_ICONINFORMATION);
			mulFlag=1;
		}
		else
			MessageBox1(hWnd, LoadStrByID(HIT_RIGHT), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
		if(mulFlag)
			MessageBox1(hWnd, LoadStrByID(HID_RESTART), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
	}
	else
		MessageBox1(hWnd, LoadStrByID(HIT_RIGHT), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
	return 1;
}



extern void ProcPowerOff(HWND hWnd);
static int SystemOtherWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitSystemOtherWin(hWnd);
			SysOtherCurWnd = 0;
			SetFocusChild(SysOtherWnd[SysOtherCurWnd]);
			if (gOptions.CameraOpen)
			{
				v_brightness = gOptions.CameraBright;
				v_contrast = gOptions.CameraContrast;
				v_quality = gOptions.PicQuality;
				v_scene = gOptions.CameraScene;
			}
			UpdateWindow(hWnd, TRUE);
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

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());
				if(fGetDC) ReleaseDC (hdc);
				return 0;
			}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			EndPaint(hWnd,hdc);
			return 0;
		case MSG_KEYUP:
			{
				if(gOptions.TFTKeyLayout==3)
				{
					keyupFlag=1;
				}
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(gOptions.TFTKeyLayout==3)
			{
				if(keyupFlag==1)
				{
					keyupFlag=0;
				}
				else
					break;
			}
			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage(hWnd, MSG_CLOSE, 0, 0);
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if (--SysOtherCurWnd < 0)
				{
					SysOtherCurWnd = 13;
				}
#ifndef _TTS_
				if (SysOtherCurWnd ==11)
				{
					SysOtherCurWnd --;
				}
#endif
				//Liaozz 20080925 xs0809191001
				if (!IDTFlag && SysOtherCurWnd == 10)
					SysOtherCurWnd --;
				if ((!mullan) && SysOtherCurWnd==9)
					SysOtherCurWnd--;
				if (SysOtherCurWnd==8)
					SysOtherCurWnd--;
				//Liaozz end
				if (!gOptions.HavePowerKey && SysOtherCurWnd==7)
					SysOtherCurWnd = 6;
#ifdef IKIOSK
				if (!gOptions.WorkCode && SysOtherCurWnd==6)
					SysOtherCurWnd--;
				if (!gOptions.IsSupportSMS && SysOtherCurWnd==5)
					SysOtherCurWnd--;
#else
				if (SysOtherCurWnd==5 || SysOtherCurWnd==6)
					SysOtherCurWnd=4;
#endif
				if (gOptions.IsOnlyRFMachine && SysOtherCurWnd==4)
					SysOtherCurWnd--;
				if (!gOptions.CameraOpen 
#ifdef FACE
						&& !gOptions.FaceFunOn 
#endif
						&&  SysOtherCurWnd==3)
				{
					SysOtherCurWnd--;
				}
				if (!gOptions.IsSupportExtBell && SysOtherCurWnd==2)
					SysOtherCurWnd--;
				if (!gOptions.LcdMode && SysOtherCurWnd==1)
					SysOtherCurWnd--;
				SetFocusChild(SysOtherWnd[SysOtherCurWnd]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if (++SysOtherCurWnd > 13)
				{					
					SysOtherCurWnd = 0;
				}
				if (!gOptions.LcdMode && SysOtherCurWnd==1)
					SysOtherCurWnd++;
				if (!gOptions.IsSupportExtBell && SysOtherCurWnd==2)
					SysOtherCurWnd++;
				if (!gOptions.CameraOpen 
#ifdef FACE
						&& !gOptions.FaceFunOn 
#endif
						&& SysOtherCurWnd==3)
				{
					SysOtherCurWnd++;
				}
				if (gOptions.IsOnlyRFMachine && SysOtherCurWnd==4)
					SysOtherCurWnd++;
#ifdef IKIOSK
				if (!gOptions.IsSupportSMS && SysOtherCurWnd==5)
					SysOtherCurWnd++;
				if (!gOptions.WorkCode && SysOtherCurWnd==6)
					SysOtherCurWnd++;
#else
				if (SysOtherCurWnd==5 || SysOtherCurWnd==6)
					SysOtherCurWnd=7;
				if (SysOtherCurWnd==8)
					SysOtherCurWnd ++;
#endif
				if (!gOptions.HavePowerKey && SysOtherCurWnd==7)
					SysOtherCurWnd=9;
				if (!mullan && SysOtherCurWnd==9)
					SysOtherCurWnd++;
				//Liaozz 20080925 xs0809191001
				if (!IDTFlag && SysOtherCurWnd == 10)
					SysOtherCurWnd ++;
#ifndef _TTS_
				if (SysOtherCurWnd == 11)
				{
					SysOtherCurWnd ++;
				}
#endif
				//Liaozz end
				SetFocusChild(SysOtherWnd[SysOtherCurWnd]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT ||
					(gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if (gOptions.LcdMode && SysOtherCurWnd==1)
				{
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || 
							(gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
					{
						SendMessage(SysOtherWnd[1], CB_SPIN, 1, 0L);
					}
					else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						SendMessage(SysOtherWnd[1], CB_SPIN, 0, 0L);
					}
					brightness = SendMessage(SysOtherWnd[1], CB_GETSPINVALUE, 0, 0);
					GPIO_TFT_BACK_LIGHT(TRUE, gOptions.LcdMode);
					return 0;
				}


				if ((!gOptions.LockFunOn && SysOtherCurWnd==2) || (gOptions.HavePowerKey && SysOtherCurWnd==7))
				{
					if (SendMessage(SysOtherWnd[SysOtherCurWnd], CB_GETCURSEL, 0, 0)==0)
						SendMessage(SysOtherWnd[SysOtherCurWnd], CB_SETCURSEL, 1, 0);
					else
						SendMessage(SysOtherWnd[SysOtherCurWnd], CB_SETCURSEL, 0, 0);
					return 0;
				}
				//Liaozz 20080925 xs0809191001
				if (IDTFlag && SysOtherCurWnd == 10) {
					if (SendMessage(SysOtherWnd[SysOtherCurWnd], CB_GETCURSEL, 0, 0)==0)
						SendMessage(SysOtherWnd[SysOtherCurWnd], CB_SETCURSEL, 1, 0);
					else
						SendMessage(SysOtherWnd[SysOtherCurWnd], CB_SETCURSEL, 0, 0);
					return 0;
				}
				//Liaozz end

#ifdef _TTS_
				if (SysOtherCurWnd == 11) {
					if (SendMessage(SysOtherWnd[SysOtherCurWnd], CB_GETCURSEL, 0, 0)==0)
						SendMessage(SysOtherWnd[SysOtherCurWnd], CB_SETCURSEL, 1, 0);
					else
						SendMessage(SysOtherWnd[SysOtherCurWnd], CB_SETCURSEL, 0, 0);

					return 0;
				}
#endif				
				if (!gOptions.IsOnlyRFMachine && SysOtherCurWnd==4)
				{
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT ||
							(gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
					{
						if (--g_fpsel < 0) g_fpsel = 3;
					}
					else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						if (++g_fpsel > 3) g_fpsel = 0;
					}
					SendMessage(SysOtherWnd[4], CB_SETCURSEL, g_fpsel, 0);
					return 0;
				}
#ifdef IKIOSK
				if (gOptions.WorkCode && SysOtherCurWnd==6)
				{
					if (SendMessage(SysOtherWnd[6], CB_GETCURSEL, 0, 0)==0)
						SendMessage(SysOtherWnd[6], CB_SETCURSEL, 1, 0);
					else
						SendMessage(SysOtherWnd[6], CB_SETCURSEL, 0, 0);
					return 0;
				}
#endif


				if( (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE) || LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)	//left
				{
					if (SysOtherCurWnd==9)
					{
						if(SendMessage(SysOtherWnd[9],CB_GETCURSEL,0,0)==0)
							SendMessage(SysOtherWnd[9],CB_SETCURSEL,LngCnt-1,0);
						else
							SendMessage(SysOtherWnd[9],CB_SPIN,0,0);
						return 0;
					}
				}
				if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
				{
					if (SysOtherCurWnd==9)
					{
						if(SendMessage(SysOtherWnd[9],CB_GETCURSEL,0,0)==(LngCnt-1))
							SendMessage(SysOtherWnd[9],CB_SETCURSEL,0,0);
						else
							SendMessage(SysOtherWnd[9],CB_SPIN,1,0);
						return 0;
					}
				}
				break;
			}

			if (LOWORD(wParam)==SCANCODE_ENTER)
			{
				if (SysOtherCurWnd < 3 || SysOtherCurWnd == 4 || SysOtherCurWnd == 9)
				{
					PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
					return 0;
				}
			}

			if (LOWORD(wParam)==SCANCODE_F10)
			{
				if (SysOtherCurWnd < 10)
				{
					if (SysOtherCurWnd == 3)
					{
						if (gOptions.CameraOpen)
						{
							PostMessage(hWnd, MSG_COMMAND, IDC_VIDEO, 0);
						}
					}
					else if (SysOtherCurWnd==8)
					{
						//关机
						isPowerOff = 1;
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
					else
					{
						PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
					}
				}
				else
				{
					PostMessage(hWnd, MSG_COMMAND, IDCANCEL, 0);
				}
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_MENU)
			{
				PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
			}

			break;

		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					if (isSysOtherChanged())
					{
						if (SaveSysOtherSetting(hWnd) && !ismenutimeout)
							PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
					else
						PostMessage(hWnd, MSG_CLOSE, 0, 0);

					break;

				case IDCANCEL:
					if(isSysOtherChanged())
					{
						if(MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
									MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
						{
							if(SaveSysOtherSetting(hWnd) && !ismenutimeout)
								PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
						else
						{
							if(!ismenutimeout)
								PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
					}
					else
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					break;

				case IDC_VIDEO:
					CreateVideoAdjustWindow(hWnd, &v_brightness, &v_contrast, &v_quality, &v_scene, 0);	
					break;

#ifdef FACE
				case IDC_FACE:
					CreateFaceSettingWindow(hWnd);
					break;
#endif
				case ID_POWER:
					//关机
					isPowerOff = 1;
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
					break;
				//case IDC_ATTPHOTO:
				//	SSR_MENU_ATTPHOTO(hWnd);
				//	break;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&SysOtherBkg);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			if (isPowerOff)
				ProcPowerOff(hMainWindowWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateSystemOtherWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	LoadWindowStr();

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(HIT_SYSTEM7);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = SystemOtherWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//if (LoadBitmap(HDC_SCREEN, &SysOtherBkg, GetBmpPath("submenubg.jpg")))
	//	return 0;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hMainWnd);
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

