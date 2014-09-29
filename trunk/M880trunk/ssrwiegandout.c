/*************************************************
  Copyright(C), 2007-2008, zksoftware
  File name: ssrwiegandout.c
Author: liming  Version: 0.1  Date: 2008.01.16
Description: 配置Wiegand输出格式
Function List:
History:
1. Date:
Author:
Modification:
 *************************************************/
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
#include "wiegand.h"

#define WG_OUT_STATIC	2000
#define WG_OUT_CONTRL	2050
#define WG_OUT_SAVE	2060
#define WG_OUT_EXIT	2061

static int WiegandInOutMode;
static int gWiegandFmtCount = 0;

//#define WG_TYPE_CNT	2


HWND WGOutWnd[12];		//控件句柄
static int curindex;		//当前控件序号
//BITMAP WGOutbkgbmp;

static int tmpoutformat, curoutformat;	//Wiegand格式
int tmpfaildid;
int tmpsitecode;
int tmpoemcode;
int tmpduressid;
int tmppulswidth;
int tmppulsinterval;
int tmpoutput, curoutput;

/*change by zxz 2012-11-29*/
const char* WGFormat[] = 
{
	"WiegandFmt26a",		//Wiegand 26 with sitecode
	"Wiegand34",			//Wiegand 34 with sitecode
	"Wiegand26",			//Wiegand 26 without sitecode
	"WiegandFmt34a",		//Wiegand 34 without sitecode
	"WiegandUserFmt",		//Wiegand User Format
	//	"PEEEEEEEEOOOOOOOOOOOOOOOOP",
	//	"PEEEEEEEEOOOOOOOOOOOOOOOOOOOOOOOOP"
};
/*
   static void changeOutFormat(HWND hWnd, int id, int nc, DWORD add_data)
   {
   if (nc == CBN_EDITCHANGE)
   {
   ;	
   }
   }
   */
extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static void CheckBoxChanged(HWND hWnd, int id, int nc, DWORD add_data)
{
	if (nc == CBN_EDITCHANGE)
	{
		switch(id)
		{
			case WG_OUT_CONTRL+1:
				if (SendMessage(WGOutWnd[1], CB_GETCURSEL, 0, 0))
					SendMessage(WGOutWnd[2], MSG_ENABLE, 1, 0);
				else
					SendMessage(WGOutWnd[2], MSG_ENABLE, 0, 0);
				break;
			case WG_OUT_CONTRL+3:
				if (SendMessage(WGOutWnd[3], CB_GETCURSEL, 0, 0))
					SendMessage(WGOutWnd[4], MSG_ENABLE, 1, 0);
				else
					SendMessage(WGOutWnd[4], MSG_ENABLE, 0, 0);
				break;
		}
	}
}

static void InitWGOutWindow(HWND hWnd)
{
	int i;
	char *WGOutStr[21];
	char rangstr[20];
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9,posX10;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=240+gOptions.GridWidth*2;
		posX2=20+gOptions.GridWidth*2;
		posX3=140;
		posX4=70+gOptions.GridWidth*2;
		posX5=110;
		posX6=20;
		posX7=5;
		posX8=110;
		posX9=170+gOptions.GridWidth*2;
		posX10=150;
	}
	else
	{
		posX1=10;
		posX2=81;
		posX3=100;
		posX4=150;
		posX5=185;
		posX6=220;
		posX7=240;
		posX8=81;
		posX9=81;
		posX10=81;
	}
	/*change by zxz 2012-11-29*/
	WGOutStr[20] = LoadStrByID(MID_WG_DEFFMT);		//已定义格式
	WGOutStr[4] = LoadStrByID(MID_WG_USRFMT);		//Wiegand User Format
	WGOutStr[0] = LoadStrByID(MID_WG_FMT1);			//Wiegand 26 with sitecode
	WGOutStr[1] = LoadStrByID(MID_WG_FMT2);			//Wiegand 34 with sitecode
	WGOutStr[2] = LoadStrByID(MID_WG_FMT3);			//Wiegand 26 without sitecode
	WGOutStr[3] = LoadStrByID(MID_WG_FMT4);			//Wiegand 34 without sitecode
	WGOutStr[5] = LoadStrByID(MID_WG_FAILID);		//失败ID
	WGOutStr[6] = LoadStrByID(MID_WG_SITECODE);		//区位码
	WGOutStr[7] = LoadStrByID(MID_WG_DURESSID);		//胁迫ID
	WGOutStr[8] = LoadStrByID(MID_WG_OEMCODE);		//OEM码
	WGOutStr[9] = LoadStrByID(HIT_SWITCH2);				//关闭
	WGOutStr[10] = LoadStrByID(HIT_SWITCH1);				//开启
	WGOutStr[11] = LoadStrByID(MID_WG_PULSWIDTH);		//脉冲宽度
	WGOutStr[12] = LoadStrByID(MID_WG_PULSINTERVAL);	//脉冲间隔
	WGOutStr[13] = LoadStrByID(MID_WG_US);			//微秒
	WGOutStr[14] = LoadStrByID(MID_WG_DEFAULT);		//默认值
	if(WiegandInOutMode == WG_OUT_MODE) {
		WGOutStr[15] = LoadStrByID(MID_WG_OUTPUT);		//输出内容
	} else {
		WGOutStr[15] = LoadStrByID(MID_WG_INPUT);
	}
	WGOutStr[16] = LoadStrByID(MID_ACNO);			//工号
	WGOutStr[17] = LoadStrByID(MID_CARD_CODE);			//卡号
	WGOutStr[18] = LoadStrByID(MID_SAVE);
	WGOutStr[19] = LoadStrByID(MID_EXIT);

	//Wiegand格式	
	CreateWindow(CTRL_STATIC, WGOutStr[20], WS_VISIBLE | SS_LEFT, WG_OUT_STATIC, posX1, 10, 100, 23, hWnd, 0);
	WGOutWnd[0] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS |
			CBS_NOTIFY, WG_OUT_CONTRL, posX2, 6, 230, 23, hWnd, 0);

    gWiegandFmtCount = 0;
	for(i=0; i<WG_TYPE_CNT; i++)
	{
		SendMessage(WGOutWnd[0], CB_ADDSTRING, 0, (LPARAM)WGOutStr[i]);
		gWiegandFmtCount++;
	}

	if (gOptions.IsWGSRBFunOn && (WiegandInOutMode == WG_OUT_MODE))
	{
		SendMessage(WGOutWnd[0], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_SRB_FUN));
		gWiegandFmtCount++;
	}

	//FaildID
	CreateWindow(CTRL_STATIC, WGOutStr[5], WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+1, posX1, 40, 90, 23, hWnd, 0);
	WGOutWnd[1] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS |
			CBS_NOTIFY, WG_OUT_CONTRL+1, posX9, 36, 60, 23, hWnd, 0);
	SendMessage(WGOutWnd[1], CB_ADDSTRING, 0, (LPARAM)WGOutStr[9]);
	SendMessage(WGOutWnd[1], CB_ADDSTRING, 0, (LPARAM)WGOutStr[10]);
	WGOutWnd[2] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, WG_OUT_CONTRL+2,
			posX4, 36, 90, 23,hWnd,0);
	sprintf(rangstr, "(0-%d)", MAX_FAILD_ID);
	CreateWindow(CTRL_STATIC, rangstr, WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+2, posX7, 40, 70, 23, hWnd, 0);

	//SiteCode
	CreateWindow(CTRL_STATIC, WGOutStr[6], WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+3, posX1, 70, 90, 23, hWnd, 0);
	WGOutWnd[3] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS |
			CBS_NOTIFY, WG_OUT_CONTRL+3, posX9, 66, 60, 23, hWnd, 0);
	SendMessage(WGOutWnd[3], CB_ADDSTRING, 0, (LPARAM)WGOutStr[9]);
	SendMessage(WGOutWnd[3], CB_ADDSTRING, 0, (LPARAM)WGOutStr[10]);
	WGOutWnd[4] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, WG_OUT_CONTRL+4,
			posX4, 66, 90, 23,hWnd,0);
	sprintf(rangstr, "(0-%d)", MAX_SITE_CODE);
	CreateWindow(CTRL_STATIC, rangstr, WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+4, posX7, 70, 70, 23, hWnd, 0);
	//	SendMessage(WGOutWnd[2], EM_LIMITTEXT, 3, 0L);
	//	SendMessage(WGOutWnd[4], EM_LIMITTEXT, 3, 0L);

	//pulse width
	CreateWindow(CTRL_STATIC, WGOutStr[11], WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+5, posX1, 100, 90, 23, hWnd,0);
	WGOutWnd[5] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, WG_OUT_CONTRL+5,
			posX3, 96, 80, 23,hWnd,0);
	CreateWindow(CTRL_STATIC, WGOutStr[13], WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+6, posX5, 100, 40, 23, hWnd,0);
	WGOutWnd[6] = CreateWindow(CTRL_BUTTON, WGOutStr[14], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, WG_OUT_CONTRL+6, 
			posX6, 96, 80, 23, hWnd, 0);
	//	SendMessage(WGWnd[6], EM_LIMITTEXT, 9, 0L);

	//pulse interval
	CreateWindow(CTRL_STATIC, WGOutStr[12], WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+7, posX1, 130, 90, 23, hWnd,0);
	WGOutWnd[7] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, WG_OUT_CONTRL+7,
			posX3, 126, 80, 23,hWnd,0);
	CreateWindow(CTRL_STATIC, WGOutStr[13], WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+8, posX5, 130, 40, 23, hWnd,0);
	WGOutWnd[8] = CreateWindow(CTRL_BUTTON, WGOutStr[14], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, WG_OUT_CONTRL+8,
			posX6, 126, 80, 23, hWnd, 0);
	//	SendMessage(WGWnd[7], EM_LIMITTEXT, 9, 0L);

	CreateWindow(CTRL_STATIC, WGOutStr[15], WS_VISIBLE | SS_LEFT, WG_OUT_STATIC+5, posX1, 160, 90, 23, hWnd, 0);
	WGOutWnd[9] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
			WG_OUT_CONTRL+9, posX8, 156, 120, 23, hWnd, 0);
	SendMessage(WGOutWnd[9], CB_ADDSTRING, 0, (LPARAM)WGOutStr[16]);
	SendMessage(WGOutWnd[9], CB_ADDSTRING, 0, (LPARAM)WGOutStr[17]);

	WGOutWnd[10] = CreateWindow(CTRL_BUTTON, WGOutStr[18], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, 
			WG_OUT_SAVE, 10, 190, 85, 23, hWnd, 0);
	WGOutWnd[11] = CreateWindow(CTRL_BUTTON, WGOutStr[19], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, 
			WG_OUT_EXIT, 220+gOptions.ControlOffset, 190, 85, 23, hWnd, 0);

	//	SetNotificationCallback(WGOutWnd[0], changeOutFormat);
	SetNotificationCallback(WGOutWnd[1], CheckBoxChanged);
	SetNotificationCallback(WGOutWnd[3], CheckBoxChanged);

}

extern char *str2upper(char *str);
/*从Options参数中获取当前的输出格式*/
static int GetOutFormat(char* FormatStr)
{
	/*
	if (!FormatStr || FormatStr[0]==0 || strcmp(FormatStr,"26")==0 || 
			strcmp(FormatStr,"WIEGAND26")==0 || strcmp(FormatStr,"Wiegand26")==0)
	{
		return 0;
	}
	else if (strcmp(FormatStr,"34")==0 || strcmp(FormatStr,"WIEGAND34")==0 || strcmp(FormatStr,"Wiegand34")==0)
	{
		return 1;
	}
	else if (strcmp(FormatStr, "PEEEEEEEEOOOOOOOOOOOOOOOOP")==0)
	{
		return 2;
	}
	else if (strcmp(FormatStr, "PEEEEEEEEOOOOOOOOOOOOOOOOOOOOOOOOP")==0)
	{
		return 3;
	}
	*/
	if(strcmp(str2upper(FormatStr), "WIEGANDFMT26A")==0)
	{
		return 0;
	}

	if(strcmp(str2upper(FormatStr), "WIEGAND34")==0)
	{
		return 1;
	}

	if(strcmp(str2upper(FormatStr), "WIEGAND26")==0)
	{
		return 2;
	}

	if(strcmp(str2upper(FormatStr), "WIEGANDFMT34A")==0)
	{
		return 3;
	}

	if(strcmp(str2upper(FormatStr), "WIEGANDUSERFMT")==0)
	{
		return 4;
	}
	return 0;
}

static void LoadParameters(void)
{
	char tmpstr[20];

	if (gOptions.IsWGSRBFunOn && gOptions.SRBOn && (WiegandInOutMode == WG_OUT_MODE))
	{
		tmpoutformat = WG_TYPE_CNT;
	}
	else
	{
		tmpoutformat = GetOutFormat(LoadStrOld("WiegandFmt"));
	}
	
	//tmpoutformat = GetOutFormat(LoadStrOld("WiegandFmt"));
	curoutformat = tmpoutformat;
	SendMessage(WGOutWnd[0], CB_SETCURSEL, tmpoutformat, 0);

	tmpfaildid = LoadInteger("WGFailedID", -1);
	SendMessage(WGOutWnd[1], CB_SETCURSEL, (tmpfaildid>=0)?1:0, 0);
	sprintf(tmpstr, "%d", (tmpfaildid >= 0) ? tmpfaildid : 0);
	SetWindowText(WGOutWnd[2], tmpstr);

	tmpsitecode = LoadInteger("WGSiteCode", -1);
	SendMessage(WGOutWnd[3], CB_SETCURSEL, (tmpsitecode>=0)?1:0, 0);
	sprintf(tmpstr, "%d", (tmpsitecode >= 0) ? tmpsitecode : 0);
	SetWindowText(WGOutWnd[4], tmpstr);

	tmppulswidth = LoadInteger("WGPulseWidth", DEFAULTPULSEWIDTH);
	sprintf(tmpstr, "%d", tmppulswidth);
	SetWindowText(WGOutWnd[5], tmpstr);

	tmppulsinterval = LoadInteger("WGPulseInterval", DEFAULTPULSEINTERVAL);
	sprintf(tmpstr, "%d", tmppulsinterval);
	SetWindowText(WGOutWnd[7], tmpstr);

	tmpoutput = gOptions.WiegandOutType;				//输出类型
	curoutput = tmpoutput;
	SendMessage(WGOutWnd[9], CB_SETCURSEL, tmpoutput, 0);
}

static void LoadParameters_WG_IN(void)
{
	char tmpstr[20];

	tmpoutformat = GetOutFormat(LoadStrOld("ExtWiegandInFmt"));
	curoutformat = tmpoutformat;
	SendMessage(WGOutWnd[0], CB_SETCURSEL, tmpoutformat, 0);

	tmpfaildid = LoadInteger("WGInFailedID", -1);
	SendMessage(WGOutWnd[1], CB_SETCURSEL, (tmpfaildid>=0)?1:0, 0);
	sprintf(tmpstr, "%d", (tmpfaildid >= 0) ? tmpfaildid : 0);
	SetWindowText(WGOutWnd[2], tmpstr);

	tmpsitecode = LoadInteger("WGInSiteCode", -1);
	SendMessage(WGOutWnd[3], CB_SETCURSEL, (tmpsitecode>=0)?1:0, 0);
	sprintf(tmpstr, "%d", (tmpsitecode >= 0) ? tmpsitecode : 0);
	SetWindowText(WGOutWnd[4], tmpstr);

	tmppulswidth = LoadInteger("ExtWGInPulseWidth", DEFAULTPULSEWIDTH);
	sprintf(tmpstr, "%d", tmppulswidth);
	SetWindowText(WGOutWnd[5], tmpstr);

	tmppulsinterval = LoadInteger("ExtWGInPulseInterval", DEFAULTPULSEINTERVAL);
	sprintf(tmpstr, "%d", tmppulsinterval);
	SetWindowText(WGOutWnd[7], tmpstr);

	tmpoutput = gOptions.WiegandInType;				//输出类型
	curoutput = tmpoutput;
	SendMessage(WGOutWnd[9], CB_SETCURSEL, tmpoutput, 0);

}

static int isSettingValid(void)
{
	char tmpstr[20];

	if (SendMessage(WGOutWnd[1], CB_GETCURSEL, 0, 0))
	{
		GetWindowText(WGOutWnd[2], tmpstr, 20);
		if (!tmpstr || atoi(tmpstr) > MAX_FAILD_ID)
		{
			return 2;
		}
	}

	if (SendMessage(WGOutWnd[3], CB_GETCURSEL, 0, 0))
	{
		memset(tmpstr, 0, 20);
		GetWindowText(WGOutWnd[4], tmpstr, 20);
		if (tmpstr[0] == '\0' || atoi(tmpstr) > MAX_SITE_CODE)
		{
			return 4;
		}
	}

	memset(tmpstr, 0, 20);
	GetWindowText(WGOutWnd[5], tmpstr, 20);
	if (tmpstr[0] == '\0' || atoi(tmpstr) > MAX_PULSEWIDTH)
	{
		return 5;
	}

	memset(tmpstr, 0, 20);
	GetWindowText(WGOutWnd[7], tmpstr, 20);
	if (tmpstr[0] == '\0' || atoi(tmpstr)>MAX_PULSEINTERVAL )
	{
		return 7;
	}

	return 0;	
}

static int isModified(void)
{
	char tmpstr[20];

	if (curoutformat != tmpoutformat)
	{
		return 1;
	}

	if (SendMessage(WGOutWnd[1], CB_GETCURSEL, 0, 0))
	{
		GetWindowText(WGOutWnd[2], tmpstr, 20);
		if (atoi(tmpstr) != tmpfaildid)
		{
			return 1;
		}
	}
	else
	{
		if (tmpfaildid != -1)
		{
			return 1;
		}
	}

	if (SendMessage(WGOutWnd[3], CB_GETCURSEL, 0, 0))
	{
		GetWindowText(WGOutWnd[4], tmpstr, 20);
		if (atoi(tmpstr) != tmpsitecode)
		{
			return 1;
		}
	}
	else
	{
		if (tmpsitecode != -1)
		{
			return 1;
		}
	} 

	GetWindowText(WGOutWnd[5], tmpstr, 20);
	if (tmppulswidth != atoi(tmpstr))
	{
		return 1;
	}

	GetWindowText(WGOutWnd[7], tmpstr, 20);
	if (tmppulsinterval != atoi(tmpstr))
	{
		return 1;
	}

	if (curoutput != tmpoutput)
	{
		return 1;
	}

	return 0;
}

static void SaveWGOutSetting(void)
{
	char tmpstr[20];
	//printf("_____%s%d,,WGFormat[%d] = %s\n", __FILE__, __LINE__, curoutformat, WGFormat[curoutformat]);

	if (curoutformat < WG_TYPE_CNT)
	{
		if (gOptions.IsWGSRBFunOn)
		{
			gOptions.SRBOn = 0;
		}
		

		SaveStr("WiegandFmt", WGFormat[curoutformat], 1);
	}
	else
	{
		if (gOptions.IsWGSRBFunOn)
		{
			gOptions.SRBOn = 1;
		}
	}

	//SaveStr("WiegandFmt", WGFormat[curoutformat], 1);

	if (SendMessage(WGOutWnd[1], CB_GETCURSEL, 0, 0))
	{
		GetWindowText(WGOutWnd[2], tmpstr, 20);
		SaveInteger("WGFailedID", atoi(tmpstr));
	}
	else
	{
		SaveInteger("WGFailedID", -1);
	}

	if (SendMessage(WGOutWnd[3], CB_GETCURSEL, 0, 0))
	{
		GetWindowText(WGOutWnd[4], tmpstr, 20);
		SaveInteger("WGSiteCode", atoi(tmpstr));
	}
	else
	{
		SaveInteger("WGSiteCode", -1);
	}

	GetWindowText(WGOutWnd[5], tmpstr, 20);
	SaveInteger("WGPulseWidth", atoi(tmpstr));
	GetWindowText(WGOutWnd[7], tmpstr, 20);
	SaveInteger("WGPulseInterval", atoi(tmpstr));	

	gOptions.WiegandOutType = curoutput;
	SaveOptions(&gOptions);
}

static void SaveWGInSettings(void)
{
	char tmpstr[20];
	//printf("_____%s%d,,WGFormat[%d] = %s\n", __FILE__, __LINE__, curoutformat, WGFormat[curoutformat]);
	SaveStr("ExtWiegandInFmt", WGFormat[curoutformat], 1);

	if (SendMessage(WGOutWnd[1], CB_GETCURSEL, 0, 0))
	{
		GetWindowText(WGOutWnd[2], tmpstr, 20);
		SaveInteger("WGInFailedID", atoi(tmpstr));
	}
	else
	{
		SaveInteger("WGInFailedID", -1);
	}

	if (SendMessage(WGOutWnd[3], CB_GETCURSEL, 0, 0))
	{
		GetWindowText(WGOutWnd[4], tmpstr, 20);
		SaveInteger("WGInSiteCode", atoi(tmpstr));
	}
	else
	{
		SaveInteger("WGInSiteCode", -1);
	}

	GetWindowText(WGOutWnd[5], tmpstr, 20);
	SaveInteger("ExtWGInPulseWidth", atoi(tmpstr));
	GetWindowText(WGOutWnd[7], tmpstr, 20);
	SaveInteger("ExtWGInPulseInterval", atoi(tmpstr));	

	gOptions.WiegandInType = curoutput;
	SaveOptions(&gOptions);
}


static int WiegandOutWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	//int i;
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_CREATE:
			InitWGOutWindow(hWnd);		//add controls
			if(WiegandInOutMode == WG_OUT_MODE) {
				LoadParameters();
			} else {
				LoadParameters_WG_IN();
			}
			curindex = 0;
			SetFocusChild(WGOutWnd[curindex]);
			UpdateWindow(hWnd,TRUE);
			break;
#if 1
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
#endif
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
			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
				PostMessage(hWnd, MSG_COMMAND, WG_OUT_EXIT, 0);

			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if (curindex == 1 || curindex == 3)
				{
					if (SendMessage(WGOutWnd[curindex], CB_GETCURSEL, 0, 0))
					{
						curindex += 1;
					}
					else
					{
						curindex += 2;
					}
				}
				else if (curindex == 2 || curindex == 4 || curindex == 5 || curindex == 7)
				{
					char tmpstr[20];
					GetWindowText(WGOutWnd[curindex], tmpstr, 20);
					if (tmpstr && strlen(tmpstr) > 0)
					{
						curindex++;
					}
				}
				else
				{
					if (++curindex > 11)
					{
						curindex = 0;
					}
				}

				SetFocusChild(WGOutWnd[curindex]);
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if (curindex == 2 || curindex == 4 || curindex == 5 || curindex == 7)
				{
					char tmpstr[20];
					GetWindowText(WGOutWnd[curindex], tmpstr, 20);
					if (tmpstr && strlen(tmpstr) > 0)
					{
						if (curindex == 5 && !SendMessage(WGOutWnd[3], CB_GETCURSEL, 0, 0))
						{
							curindex -= 2;
						}
						else
						{
							curindex--;
						}
					}
				}
				else if (curindex == 3 && !SendMessage(WGOutWnd[1], CB_GETCURSEL, 0, 0))
				{
					curindex -= 2;
				}
				else
				{
					if (--curindex < 0)
					{
						curindex = 11;
					}
				}

				SetFocusChild(WGOutWnd[curindex]);
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if (curindex == 0)
				{
					if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
					{
						if (--curoutformat < 0)
						{
							curoutformat = gWiegandFmtCount-1;
						}
					}
					else
					{
						if (++curoutformat >= gWiegandFmtCount)
						{
							curoutformat = 0;
						}
					}

					SendMessage(WGOutWnd[curindex], CB_SETCURSEL, curoutformat, 0);
					return 0;
				}
				else if (curindex == 1 || curindex == 3)
				{
					int curstate = SendMessage(WGOutWnd[curindex], CB_GETCURSEL, 0, 0);
					if (curstate == 0)
						curstate = 1;
					else
						curstate = 0;
					SendMessage(WGOutWnd[curindex], CB_SETCURSEL, curstate, 0);
					return 0;
				}
				else if (curindex == 9)
				{
					if (curoutput == 0)
						curoutput = 1;
					else
						curoutput = 0;
					SendMessage(WGOutWnd[curindex], CB_SETCURSEL, curoutput, 0);
					return 0;
				}
				else if (curindex == 10 || curindex == 11)
				{
					if (curindex == 10)
						curindex = 11;
					else 
						curindex = 10;
					SetFocusChild(WGOutWnd[curindex]);
					return 0;
				}

			}
			else if (LOWORD(wParam) == SCANCODE_MENU)
			{
				PostMessage(hWnd, MSG_COMMAND, WG_OUT_SAVE, 0);
			}
			else if (LOWORD(wParam) == SCANCODE_ENTER)
			{
				char tmpstr[20];

				if (curindex < 10 && curindex != 6 && curindex != 8)
				{
					PostMessage(hWnd, MSG_COMMAND, WG_OUT_SAVE, 0);
					return 0;
				}
				else if (curindex == 6 || curindex == 8)
				{
					sprintf(tmpstr, "%d", (curindex==6) ? DEFAULTPULSEWIDTH : DEFAULTPULSEINTERVAL);
					SetWindowText(WGOutWnd[curindex-1], tmpstr);
					return 0;
				}					
			}
			else if (LOWORD(wParam) == SCANCODE_F10)
			{
				char tmpstr[20];

				if (curindex == 6 || curindex == 8)
				{
					sprintf(tmpstr, "%d", (curindex==6) ? DEFAULTPULSEWIDTH : DEFAULTPULSEINTERVAL);
					SetWindowText(WGOutWnd[curindex-1], tmpstr);
					return 0;
				}
				else if (curindex == 11)
				{
					PostMessage(hWnd, MSG_COMMAND, WG_OUT_EXIT, 0);
				}
				else
				{	
					PostMessage(hWnd, MSG_COMMAND, WG_OUT_SAVE, 0);
				}
			}

			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if (id == WG_OUT_SAVE)
			{
				int ret = isSettingValid();
				if (ret)
				{
					MessageBox1(hWnd, LoadStrByID(HIT_ERROR0), LoadStrByID(HIT_RUN), MB_OK | MB_ICONSTOP);
					if (!ismenutimeout)
					{
						SetFocusChild(WGOutWnd[ret]);
						return 0;
					}
				}
				else if (isModified()){
					if(WiegandInOutMode == WG_OUT_MODE) {
						SaveWGOutSetting();
						ReloadWiegandOutFmt();/*dsl 2012.4.25*/
					} else {
						SaveWGInSettings();
					}
					MessageBox1(hWnd, LoadStrByID(HIT_RIGHT), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
					MessageBox1(hWnd, LoadStrByID(HID_RESTART), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
					if (!ismenutimeout) {
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
				} else {
					PostMessage(hWnd, MSG_CLOSE, 0, 0);	
				}
			}
			else if (id == WG_OUT_EXIT)
			{
				if(isModified() && MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
							MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK) {
					PostMessage(hWnd, MSG_COMMAND, WG_OUT_SAVE, 0);
				} else if (!ismenutimeout) {
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&WGOutbkgbmp);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateWiegandOutWindow(HWND hWnd, int InOutMode)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	WiegandInOutMode = InOutMode;
	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER|WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	if(WiegandInOutMode == WG_OUT_MODE) {
		CreateInfo.spCaption = LoadStrByID(MID_WG_SETOUT);
	} else {
		CreateInfo.spCaption = LoadStrByID(MID_WG_SETIN);
	}
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WiegandOutWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	//	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//if (LoadBitmap(HDC_SCREEN, &WGOutbkgbmp, GetBmpPath("submenubg.jpg")))
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

