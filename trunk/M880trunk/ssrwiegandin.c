/*************************************************
  Copyright(C), 2007-2008, zksoftware
  File name: ssrwiegandin.c
Author: liming  Version: 0.1  Date: 2008.01.16
Description: 配置Wiegand输入格式
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
#include "t9.h"
#include <minigui/tftmullan.h>

#define WG_IN_STATIC	3000
#define WG_IN_CONTRL	3050
#define WG_IN_SAVE	3060
#define WG_IN_EXIT	3061

extern HWND hIMEWnd;		//T9输入法窗口句柄
HWND WGInWnd[9];		//控件句柄
static int curindex;		//当前控件序号
//BITMAP WGInbkgbmp;

char tmpinformat[256];		//Wiegand格式
int bitcounts;
int pulsewidth;
int pulseinterval;
int wgintype, curintype;	
static int curoutformat,tmpoutformat;
static int gWiegandFmtCount = 0;

const char* WGINFormat[] = 
{
	"WiegandFmt26a",		//Wiegand 26 with sitecode
	"Wiegand34",			//Wiegand 34 with sitecode
	"Wiegand26",			//Wiegand 26 without sitecode
	"WiegandFmt34a",		//Wiegand 34 without sitecode
	"WiegandUserFmt",		//Wiegand User Format
};

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static void InitWGInWindow(HWND hWnd)
{
	int i;
	char *WGInStr[11];
	char *WGFormatType[5];
	int posX1,posX2,posX3,posX4,posX5, posX9, posX10;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=220+gOptions.GridWidth*2;
		posX2=130;
		posX3=100;
		posX4=10;
		posX5=100;
		posX9=10;
		posX10=150;
	}
	else
	{
		posX1=10;
		posX2=100;
		posX3=185;
		posX4=220;
		posX5=100;
		posX9=81;
		posX10=81;
	}
	WGFormatType[4] = LoadStrByID(MID_WG_USRFMT);		//Wiegand User Format
	WGFormatType[0] = LoadStrByID(MID_WG_FMT1);			//Wiegand 26 with sitecode
	WGFormatType[1] = LoadStrByID(MID_WG_FMT2);			//Wiegand 34 with sitecode
	WGFormatType[2] = LoadStrByID(MID_WG_FMT3);			//Wiegand 26 without sitecode
	WGFormatType[3] = LoadStrByID(MID_WG_FMT4);			//Wiegand 34 without sitecode

	//WGInStr[0] = LoadStrByID(MID_WG_USRFMT);		//定义格式
	WGInStr[0] = LoadStrByID(MID_WG_DEFFMT);
	WGInStr[1] = LoadStrByID(MID_WG_BITCOUNT);		//Bit为数
	WGInStr[2] = LoadStrByID(MID_WG_PULSWIDTH);           	//脉冲宽度
	WGInStr[3] = LoadStrByID(MID_WG_PULSINTERVAL);        	//脉冲间隔
	WGInStr[4] = LoadStrByID(MID_WG_US);                  	//微秒
	WGInStr[5] = LoadStrByID(MID_WG_DEFAULT);             	//默认值
	WGInStr[6] = LoadStrByID(MID_WG_INPUT);			//输入内容
	WGInStr[7] = LoadStrByID(MID_ACNO);				//工号
	WGInStr[8] = LoadStrByID(MID_CARD_CODE);				//卡号
	WGInStr[9] = LoadStrByID(MID_SAVE);
	WGInStr[10] = LoadStrByID(MID_EXIT);

	//Wiegand格式	
	//CreateWindow(CTRL_STATIC, WGInStr[0], WS_VISIBLE | SS_LEFT, WG_IN_STATIC, posX1, 10, 100, 23, hWnd, 0);
	CreateWindow(CTRL_STATIC, WGInStr[0], WS_VISIBLE | SS_LEFT, WG_IN_STATIC, posX1, 10, 100, 23, hWnd, 0);
	
	if (fromRight==1) {
		WGInWnd[0] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS |
			CBS_NOTIFY, WG_IN_CONTRL, posX9, 6, 210, 23, hWnd, 0);
	} else {
		WGInWnd[0] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS |
			CBS_NOTIFY, WG_IN_CONTRL, posX9, 6, 230, 23, hWnd, 0);
	}
	gWiegandFmtCount = 0;
	for(i=0; i<WG_TYPE_CNT; i++)
	{
		gWiegandFmtCount++;
		SendMessage(WGInWnd[0], CB_ADDSTRING, 0, (LPARAM)WGFormatType[i]);
	}

	CreateWindow(CTRL_STATIC, WGInStr[1], WS_VISIBLE | SS_LEFT, WG_IN_STATIC+1, posX1, 70 - 30, 90, 23, hWnd, 0);
	WGInWnd[1] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, WG_IN_CONTRL+1,
			posX2, 66 - 30, 80, 23, hWnd, 0);

	//pulse width
	CreateWindow(CTRL_STATIC, WGInStr[2], WS_VISIBLE | SS_LEFT, WG_IN_STATIC+2, posX1, 100 - 30, 90, 23, hWnd, 0);
	WGInWnd[2] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, WG_IN_CONTRL+2,
			posX2, 96 - 30, 80, 23, hWnd, 0);
	CreateWindow(CTRL_STATIC, WGInStr[4], WS_VISIBLE | SS_LEFT, WG_IN_STATIC+3, posX3, 100 - 30, 35, 23, hWnd,0);
	WGInWnd[3] = CreateWindow(CTRL_BUTTON, WGInStr[5], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, WG_IN_CONTRL+3, 
			posX4, 96 - 30, 80, 23, hWnd, 0);
	//	SendMessage(WGWnd[2], EM_LIMITTEXT, 9, 0L);

	//pulse interval
	CreateWindow(CTRL_STATIC, WGInStr[3], WS_VISIBLE | SS_LEFT, WG_IN_STATIC+4, posX1, 130 - 30, 90, 23, hWnd,0);
	WGInWnd[4] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, WG_IN_CONTRL+4,
			posX2, 126 - 30, 80, 23,hWnd,0);
	CreateWindow(CTRL_STATIC, WGInStr[4], WS_VISIBLE | SS_LEFT, WG_IN_STATIC+5, posX3, 130 - 30, 35, 23, hWnd,0);
	WGInWnd[5] = CreateWindow(CTRL_BUTTON, WGInStr[5], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, WG_IN_CONTRL+5,
			posX4, 126 - 30, 80, 23, hWnd, 0);
	//	SendMessage(WGWnd[4], EM_LIMITTEXT, 9, 0L);

	CreateWindow(CTRL_STATIC, WGInStr[6], WS_VISIBLE | SS_LEFT, WG_IN_STATIC+6, posX1, 160 - 30, 90, 23, hWnd, 0);
	WGInWnd[6] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
			WG_IN_CONTRL+6, posX5-9, 156 - 30, 120, 23, hWnd, 0);
	SendMessage(WGInWnd[6], CB_ADDSTRING, 0, (LPARAM)WGInStr[7]);
	SendMessage(WGInWnd[6], CB_ADDSTRING, 0, (LPARAM)WGInStr[8]);

	WGInWnd[7] = CreateWindow(CTRL_BUTTON, WGInStr[9], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, 
			WG_IN_SAVE, 10, 190, 85, 23, hWnd, 0);
	WGInWnd[8] = CreateWindow(CTRL_BUTTON, WGInStr[10], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, 
			WG_IN_EXIT, 220+gOptions.ControlOffset, 190, 85, 23, hWnd, 0);

}

extern char *str2upper(char *str);
static int GetINFormat(char* FormatStr)
{
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

	//LoadStr("ExtWiegandInFmt", tmpinformat);
	//SetWindowText(WGInWnd[0], tmpinformat);
	tmpoutformat = GetINFormat(LoadStrOld("ExtWiegandInFmt"));
	curoutformat = tmpoutformat;
	SendMessage(WGInWnd[0], CB_SETCURSEL, tmpoutformat, 0);

	bitcounts = LoadInteger("ExtWGInBitsCount", 0);	
	sprintf(tmpstr, "%d", bitcounts);
	SetWindowText(WGInWnd[1], tmpstr);

	pulsewidth = LoadInteger("ExtWGInPulseWidth", DEFAULTPULSEWIDTH);
	sprintf(tmpstr, "%d", pulsewidth);
	SetWindowText(WGInWnd[2], tmpstr);

	pulseinterval = LoadInteger("ExtWGInPulseInterval", DEFAULTPULSEINTERVAL);
	sprintf(tmpstr, "%d", pulseinterval);
	SetWindowText(WGInWnd[4], tmpstr);

	wgintype = gOptions.WiegandInType;				//输入类型
	curintype = wgintype;
	SendMessage(WGInWnd[6], CB_SETCURSEL, wgintype, 0);
}

static int isSettingValid(void)
{
	char formatstring[256];
	char tmpstr[20];

	GetWindowText(WGInWnd[0], formatstring, 256);
	if (formatstring[0] == '\0')
	{
		return 0;
	}

	GetWindowText(WGInWnd[1], tmpstr, 20);
	if (tmpstr[0] == '\0') // || atoi(tmpstr) != strlen(formatstring))
	{
		return 1;
	}

	memset(tmpstr, 0, 20);
	GetWindowText(WGInWnd[2], tmpstr, 20);
	if (tmpstr[0] == '\0' || atoi(tmpstr) > MAX_PULSEWIDTH)
	{
		return 2;
	}

	memset(tmpstr, 0, 20);
	GetWindowText(WGInWnd[4], tmpstr, 20);
	if (tmpstr[0] == '\0' || atoi(tmpstr) > MAX_PULSEINTERVAL )
	{
		return 4;
	}

	return 0;	
}

static int isModified(void)
{
	char tmpstr[20];
	//char formatstring[256];

	//GetWindowText(WGInWnd[0], formatstring, 256);
	//if (strlen(formatstring) != strlen(tmpinformat) || nstrcmp(formatstring, tmpinformat, strlen(formatstring)) != 0)
	//{
	//	return 1;
	//}
	
	if (curoutformat != tmpoutformat)
	{
		return 1;
	}

	memset(tmpstr, 0, sizeof(tmpstr));
	GetWindowText(WGInWnd[1], tmpstr, 20);
	if (atoi(tmpstr) != bitcounts)
	{
		return 1;
	}

	memset(tmpstr, 0, sizeof(tmpstr));
	GetWindowText(WGInWnd[2], tmpstr, 20);
	if (atoi(tmpstr) != pulsewidth)
	{
		return 1;
	}

	memset(tmpstr, 0, sizeof(tmpstr));
	GetWindowText(WGInWnd[4], tmpstr, 20);
	if (atoi(tmpstr) != pulseinterval)
	{
		return 1;
	}

	if (curintype != wgintype)
	{
		return 1;
	}

	return 0;
}

static void SaveWGInSetting(void)
{
	char tmpstr[20];
	//char formatstring[256];

	//GetWindowText(WGInWnd[0], formatstring, 256);
	//SaveStr("ExtWiegandInFmt", formatstring, 1);
	SaveStr("ExtWiegandInFmt", WGINFormat[curoutformat], 1);

	GetWindowText(WGInWnd[1], tmpstr, 20);
	SaveInteger("ExtWGInBitsCount", atoi(tmpstr));

	GetWindowText(WGInWnd[2], tmpstr, 20);
	SaveInteger("ExtWGInPulseWidth", atoi(tmpstr));

	GetWindowText(WGInWnd[4], tmpstr, 20);
	SaveInteger("ExtWGInPulseInterval", atoi(tmpstr));

	gOptions.WiegandInType = curintype;
	SaveOptions(&gOptions);
}

static int WiegandInWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &WGInbkgbmp, GetBmpPath("submenubg.jpg")))
			//       return 0;

			InitWGInWindow(hWnd);		//add controls
			LoadParameters();
			curindex = 0;
			SetFocusChild(WGInWnd[curindex]);
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
				PostMessage(hWnd, MSG_COMMAND, WG_IN_EXIT, 0);

			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if (curindex == 0)
				{
					if(hIMEWnd!=HWND_INVALID)
					{
						SendMessage(hIMEWnd, MSG_CLOSE, 0, 0L);
						hIMEWnd=HWND_INVALID;
					}
					curindex++;
				}
				else if (curindex == 1 || curindex == 2 || curindex == 4)
				{
					char tmpstr[20];
					GetWindowText(WGInWnd[curindex], tmpstr, 20);
					if (tmpstr && strlen(tmpstr) > 0)
					{
						curindex++;
					}
				}
				else
				{
					if (++curindex > 8)
					{
						curindex = 0;
					}
				}

				SetFocusChild(WGInWnd[curindex]);
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if (curindex == 0)
				{
					if(hIMEWnd!=HWND_INVALID)
					{
						SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
						hIMEWnd=HWND_INVALID;
					}
					curindex = 8;
				}
				else if (curindex == 1 || curindex == 2 || curindex == 4)
				{
					char tmpstr[20];
					GetWindowText(WGInWnd[curindex], tmpstr, 20);
					if (tmpstr && strlen(tmpstr) > 0)
					{
						curindex--;
					}
				}
				else
				{
					if (--curindex < 0)
					{
						curindex = 8;
					}
				}

				SetFocusChild(WGInWnd[curindex]);
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

					SendMessage(WGInWnd[curindex], CB_SETCURSEL, curoutformat, 0);
					return 0;
				} else if (curindex == 6)
				{
					if (curintype == 1)
						curintype = 0;
					else
						curintype = 1;

					SendMessage(WGInWnd[curindex], CB_SETCURSEL, curintype, 0);
					return 0;
				}
				else if (curindex == 7 || curindex == 8)
				{
					if (curindex == 7)
						curindex = 8;
					else 
						curindex = 7;
					SetFocusChild(WGInWnd[curindex]);
					return 0;
				}
				else if(((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)&&gOptions.IMEFunOn==1&& gOptions.TFTKeyLayout==3)&&curindex ==0)
				{
					T9IMEWindow(hWnd, 0, 200, gOptions.LCDWidth, gOptions.LCDHeight, 0);
					return 0;
				}

			}
			else if (LOWORD(wParam) == SCANCODE_MENU)
			{
				PostMessage(hWnd, MSG_COMMAND, WG_IN_SAVE, 0);
			}
			else if (LOWORD(wParam) == SCANCODE_ENTER)
			{
				char tmpstr[20];

				if (curindex != 3 && curindex != 5 && curindex < 7)
				{
					PostMessage(hWnd, MSG_COMMAND, WG_IN_SAVE, 0);
				}
				else if (curindex == 3 || curindex == 5)
				{
					sprintf(tmpstr, "%d", (curindex==3) ? DEFAULTPULSEWIDTH : DEFAULTPULSEINTERVAL);
					SetWindowText(WGInWnd[curindex-1], tmpstr);
					return 0;
				}

			}
			else if (LOWORD(wParam) == SCANCODE_F10)
			{
				char tmpstr[20];

				if (curindex == 3 || curindex ==5)
				{
					sprintf(tmpstr, "%d", (curindex==3) ? DEFAULTPULSEWIDTH : DEFAULTPULSEINTERVAL);
					SetWindowText(WGInWnd[curindex-1], tmpstr);
					return 0;
				}
				else if (curindex == 8)
				{
					PostMessage(hWnd, MSG_COMMAND, WG_IN_EXIT, 0);
				}
				else
				{
					PostMessage(hWnd, MSG_COMMAND, WG_IN_SAVE, 0);
				}
			}
			//			else if (LOWORD(wParam) == gOptions.IMESwitchKey)
			else if((LOWORD(wParam)==SCANCODE_F9) || 
					(LOWORD(wParam)==SCANCODE_F11 && (gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)))
			{
				if(curindex == 0 && gOptions.IMEFunOn == 1)
				{
					T9IMEWindow(hWnd, 0, 200, gOptions.LCDWidth, gOptions.LCDHeight, 0);     //涓嶆敮鎸佷腑鏂囪緭鍏??
				}
				return 0;
			}

			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);

			if (id == WG_IN_SAVE)
			{
				int ret = isSettingValid();
				if (ret)
				{
					MessageBox1(hWnd, LoadStrByID(HIT_ERROR0), LoadStrByID(HIT_RUN), MB_OK | MB_ICONSTOP);
					if (!ismenutimeout)
					{
						SetFocusChild(WGInWnd[ret]);
						return 0;
					}
				}
				else
				{		
					if (isModified())
					{
						SaveWGInSetting();
						MessageBox1(hWnd, LoadStrByID(HIT_RIGHT), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
						MessageBox1(hWnd, LoadStrByID(HID_RESTART), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
						if (!ismenutimeout)
						{
							PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
					}
					else
					{
						PostMessage(hWnd, MSG_CLOSE, 0, 0);	
					}
				}
			}
			else if (id == WG_IN_EXIT)
			{
				if(isModified() &&
						MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
							MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
				{
					PostMessage(hWnd, MSG_COMMAND, WG_IN_SAVE, 0);
				}
				else
				{
					if (!ismenutimeout)
					{
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
				}
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&WGInbkgbmp);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateWiegandInWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER|WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_WG_SETIN);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WiegandInWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	//	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

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

