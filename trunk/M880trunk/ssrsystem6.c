/* 
 * SSR 2.0 界面设置入口文件
 * 设计：hed     2007.5.14
 * 原始版本:1.0.0   
 * 修改记录:
 * 编译环境:mipsel-gcc
 */
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h> 
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <string.h>
#include "ssrcommon.h"
#include "options.h"
#include "flashdb.h"
#include "ssrpub.h"
#include "exfun.h"
#include <minigui/tftmullan.h>

#define IDC_FPRETRY	902
#define IDC_FPINPUT	903
#define IDC_ADMRETRY	904
#define IDC_ADMINPUT	905
#define IDC_PWDRETRY	906
#define IDC_PWDINPUT	907
#define IDC_SHOWPIC	908
#define	IDC_PICTIME	909
#define IDC_PICINPUT	910
#define IDC_CLOCKTIME	911
#define IDC_CLOCKINPUT	912
#define IDC_SHOWCLOCK1	913
#define IDC_SHOWCLOCK2	914
#define IDC_OK		915
#define IDC_CANCEL	916
#define IDC_CLOCKSTYLE	917
#define IDC_RANGE1	918
#define IDC_RANGE2	919
#define IDC_SECOND1	920
#define IDC_SECOND2	921
#define IDC_CAMERA	922
#define IDC_CAMERAMODE	923

BITMAP  Clock1, Clock2;
int g_fpretry, g_pwdretry, g_interstyle, g_capturepic;
static HWND Sys6Wnd[8];
int curSys6Item;
char* Sys6WinStr[10];
int picheight;

static void LoadWinStr(void)
{
	Sys6WinStr[0] = LoadStrByID(MID_FPRETRY);
	Sys6WinStr[1] = LoadStrByID(MID_PWDRETRY);
	Sys6WinStr[2] = LoadStrByID(MID_CLOCKSTYLE);
	Sys6WinStr[3] = LoadStrByID(MID_PICTIME);
	Sys6WinStr[4] = LoadStrByID(MID_CLOCKTIME);
	Sys6WinStr[5] = LoadStrByID(MID_CAMERA_MODE);
	Sys6WinStr[6] = LoadStrByID(MID_SAVE);
	Sys6WinStr[7] = LoadStrByID(MID_EXIT);
	Sys6WinStr[8] = LoadStrByID(MID_SECOND);
	Sys6WinStr[9] = LoadStrByID(MID_DATARANGE);
}

static void InitSys6Window(HWND hWnd)
{
	HWND tmpstatic[3];
	int rows=0;
	char rangestr[20];
	char temp[4];
	int i;
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9,posX10, posX11;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=180;
		posX2=180;
		posX3=80;//180->80 dsl 2011.9.27
		posX4=160;
		posX5=130;
		posX6=10;
		posX7=30;
		posX8=10;
		posX9=225;
		posX10=70;
		posX11=180;//dsl 2011.12.26
	}
	else
	{
		posX1=10;
		posX2=90;
		posX3=110;
		posX4=140;
		posX5=170;
		posX6=215;
		posX7=270;
		posX8=300;
		posX9=10;
		posX10=170;
		posX11=110;//dsl 2011.12.26
	}

	memset(rangestr, 0, sizeof(rangestr));
	sprintf(rangestr, Sys6WinStr[9], 1, 9);
	tmpstatic[0] = CreateWindow(CTRL_STATIC, Sys6WinStr[0], SS_LEFT, IDC_FPRETRY, posX1, rows*30+10, 160, 23, hWnd, 0);
	Sys6Wnd[0] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
			IDC_FPINPUT, posX5, rows*30+6, 40, 23, hWnd, 0);
	tmpstatic[1] = CreateWindow(CTRL_STATIC, rangestr, SS_LEFT, IDC_RANGE1, posX6, rows*30+10, 100, 23, hWnd, 0);
	if (!gOptions.IsOnlyRFMachine)
	{
		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(tmpstatic[1], SW_SHOW);
		ShowWindow(Sys6Wnd[0], SW_SHOW);
		rows++;
	}

	CreateWindow(CTRL_STATIC, Sys6WinStr[1], WS_VISIBLE | SS_LEFT, IDC_PWDRETRY, posX1, rows*30+10, 160, 23, hWnd, 0);
	Sys6Wnd[1] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | 
			CBS_AUTOFOCUS, IDC_PWDINPUT, posX5, rows*30+6, 40, 23, hWnd, 0);
	CreateWindow(CTRL_STATIC, rangestr, WS_VISIBLE | SS_LEFT, IDC_RANGE2, posX6, rows*30+10, 100, 23, hWnd, 0);
	rows++;

	for (i=0; i<9; i++)
	{
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "%d", i+1);
		SendMessage(Sys6Wnd[0], CB_ADDSTRING, 0, (LPARAM)temp);
		SendMessage(Sys6Wnd[1], CB_ADDSTRING, 0, (LPARAM)temp);
	}

	picheight = rows*30+6;
	CreateWindow(CTRL_STATIC, Sys6WinStr[2], WS_VISIBLE | SS_LEFT, IDC_CLOCKSTYLE, posX9, rows*30+10, 80, 23, hWnd, 0);
	Sys6Wnd[2] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT |
			CBS_AUTOFOCUS, IDC_SHOWCLOCK1, posX2, rows*30+6, 40, 23, hWnd, 0);
	SendMessage(Sys6Wnd[2], CB_ADDSTRING, 0, (LPARAM)"1");
	SendMessage(Sys6Wnd[2], CB_ADDSTRING, 0, (LPARAM)"2");
	rows+=2;

	CreateWindow(CTRL_STATIC, Sys6WinStr[3], WS_VISIBLE | SS_LEFT, IDC_PICTIME, posX9, rows*30+15, 95, 23, hWnd, 0);
	Sys6Wnd[3] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, IDC_PICINPUT, 
			posX11, rows*30+11, 28, 23, hWnd, 0);
	SendMessage(Sys6Wnd[3], EM_LIMITTEXT, 3, 0L);
	CreateWindow(CTRL_STATIC, Sys6WinStr[8], WS_VISIBLE | SS_LEFT, IDC_SECOND1, posX4, rows*30+15, 30, 23, hWnd, 0);
	CreateWindow(CTRL_STATIC, Sys6WinStr[4], WS_VISIBLE | SS_LEFT, IDC_CLOCKTIME, posX10, rows*30+15, 95, 23, hWnd, 0);
	Sys6Wnd[4] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, IDC_CLOCKINPUT,
			posX7, rows*30+11, 28, 23, hWnd, 0);
	SendMessage(Sys6Wnd[4], EM_LIMITTEXT, 3, 0L);
	CreateWindow(CTRL_STATIC, Sys6WinStr[8], WS_VISIBLE | SS_LEFT, IDC_SECOND2, posX8, rows*30+15, 30, 23, hWnd, 0);
	rows++;

	tmpstatic[2] = CreateWindow(CTRL_STATIC, Sys6WinStr[5], SS_LEFT, IDC_CAMERA, posX9, rows*30+15, 90, 23, hWnd, 0);
	Sys6Wnd[5] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT |
			CBS_AUTOFOCUS, IDC_CAMERAMODE, posX3, rows*30+11, 140, 23, hWnd, 0);
	for (i=0; i<4; i++)
	{
		SendMessage(Sys6Wnd[5], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_CAMERA_MODE1+i));
	}
	if (gOptions.CameraOpen)
	{
		ShowWindow(tmpstatic[2], SW_SHOW);
		ShowWindow(Sys6Wnd[5], SW_SHOW);
		rows++;
	}

	Sys6Wnd[6] = CreateWindow(CTRL_BUTTON, Sys6WinStr[6], WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDOK, 10, 190, 85, 23, hWnd, 0);
	Sys6Wnd[7] = CreateWindow(CTRL_BUTTON, Sys6WinStr[7], WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDCANCEL, 220+gOptions.ControlOffset, 190, 85, 23, hWnd, 0);
}

static void SetWindowValue(void)
{
	char tmpstr[4];

	g_fpretry = gOptions.FPRetry-1;
	g_pwdretry = gOptions.PwdRetry-1;
	g_interstyle = gOptions.InterStyle-1;
	g_capturepic = gOptions.CapturePic;

	SendMessage(Sys6Wnd[0], CB_SETCURSEL, g_fpretry, 0);
	SendMessage(Sys6Wnd[1], CB_SETCURSEL, g_pwdretry, 0);
	SendMessage(Sys6Wnd[2], CB_SETCURSEL, g_interstyle, 0);
	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "%d", gOptions.LogoTime);
	SetWindowText(Sys6Wnd[3], tmpstr);
	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "%d", gOptions.ClockTime);
	SetWindowText(Sys6Wnd[4], tmpstr);
	SendMessage(Sys6Wnd[5], CB_SETCURSEL, g_capturepic, 0);
}

static int isChanged(void)		//-1:无改变，0:已改变，其它:设置错误
{	
	int chgflag = 0;
	char temp[4];

	if ((g_fpretry+1 != gOptions.FPRetry) || (g_pwdretry+1 != gOptions.PwdRetry) || 
			(g_interstyle != gOptions.InterStyle-1) || (g_capturepic != gOptions.CapturePic))
		chgflag++;

	memset(temp, 0, sizeof(temp));
	GetWindowText(Sys6Wnd[3], temp, 4);
	if (strlen(temp)>0 && atoi(temp)>=3)
	{
		if (atoi(temp) != gOptions.LogoTime)
			chgflag++;
	}
	else
		return 3;

	memset(temp, 0, sizeof(temp));
	GetWindowText(Sys6Wnd[4], temp, 4);
	if (strlen(temp)>0)
	{
		if (atoi(temp) != gOptions.ClockTime)
			chgflag++;
	}
	else
		return 4;

	return (chgflag) ? 0:-1;
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static void SaveParam(void)
{
	//int change=0;
	char temp[10];
	//int i,j;

	gOptions.FPRetry = g_fpretry+1;
	gOptions.PwdRetry = g_pwdretry+1;
	gOptions.InterStyle = g_interstyle+1;

	memset(temp, 0, sizeof(temp));
	GetWindowText(Sys6Wnd[3], temp, 4);
	gOptions.LogoTime=atoi(temp);

	memset(temp, 0, sizeof(temp));
	GetWindowText(Sys6Wnd[4], temp, 4);
	gOptions.ClockTime = atoi(temp);

	gOptions.CapturePic = g_capturepic;

	SaveOptions(&gOptions);
}

static int InputError(HWND hWnd, int idx)
{
	char tmpstr[4];

	memset(tmpstr, 0, sizeof(tmpstr));
	GetWindowText(Sys6Wnd[idx], tmpstr, 4);

	if (idx==3)
	{
		if (strlen(tmpstr)==0 || atoi(tmpstr)<3)
		{
			MessageBox1(hWnd, LoadStrByID(MID_INVALIDDATA), LoadStrByID(MID_WARNING), MB_OK | MB_ICONINFORMATION);
			return 1;
		}
	}
	else if (idx==4)
	{
		if (strlen(tmpstr)==0)
		{
			MessageBox1 (hWnd, LoadStrByID(MID_INVALIDDATA2), LoadStrByID(MID_WARNING), MB_OK | MB_ICONINFORMATION);
			return 1;
		}
	}
	return 0;
}

static int System6WinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	int tmpvalue = 0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &Interface, GetBmpPath("submenubg.jpg")))
			//        return 0;
			if (LoadBitmap(HDC_SCREEN, &Clock1, GetBmpPath("res/clockview1.jpg")))
				return 0;
			if (LoadBitmap(HDC_SCREEN, &Clock2, GetBmpPath("res/clockview2.jpg")))
				return 0;

			InitSys6Window(hWnd);
			SetWindowValue();
			curSys6Item = (gOptions.IsOnlyRFMachine)?1:0;
			SetFocusChild(Sys6Wnd[curSys6Item]);
			UpdateWindow(hWnd, TRUE);
			break;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;

				if (hdc == 0) {
					hdc = GetClientDC (hWnd);
					fGetDC = TRUE;
				}
				if (clip) {
					rcTemp = *clip;
					ScreenToClient (hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient (hWnd, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect (hdc, &rcTemp);
				}
#if 1
				FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

				if (fromRight==1)
				{
					FillBoxWithBitmap(hdc, 100, picheight, 60, 60, &Clock1);
					FillBoxWithBitmap(hdc, 10, picheight, 60, 60, &Clock2);
				}
				else
				{
					FillBoxWithBitmap(hdc, 160, picheight, 60, 60, &Clock1);
					FillBoxWithBitmap(hdc, 250, picheight, 60, 60, &Clock2);
				}
#endif
				if (fGetDC)
					ReleaseDC (hdc);
				return 0;
			}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
			SetTextColor(hdc, COLOR_lightwhite);

			if (fromRight==1)
			{
				TextOut(hdc, 170, picheight, "1");
				TextOut(hdc, 80, picheight, "2");
			}
			else
			{
				TextOut(hdc, 145, picheight, "1");
				TextOut(hdc, 235, picheight, "2");
			}

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
			if(gOptions.KeyPadBeep)
				ExKeyBeep();

			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage(hWnd, MSG_COMMAND, IDCANCEL, 0);
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP || LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{

				if ((curSys6Item==3 || curSys6Item==4) && InputError(hWnd, curSys6Item))
					return 0;

				if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
				{
					if (++curSys6Item == 5 && !gOptions.CameraOpen)
						curSys6Item++;

					if (curSys6Item > 7)
					{
						if (!gOptions.IsOnlyRFMachine)
							curSys6Item=0;
						else
							curSys6Item=1;
					}
				}

				if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
				{
					if (--curSys6Item == 5 && !gOptions.CameraOpen)
						curSys6Item--;

					if (curSys6Item < ((gOptions.IsOnlyRFMachine)?1:0))	
						curSys6Item=7;
				}
				SetFocusChild(Sys6Wnd[curSys6Item]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if (curSys6Item==0)
				{
					if (++g_fpretry>8)
						g_fpretry=0;
					SendMessage(Sys6Wnd[curSys6Item], CB_SETCURSEL, g_fpretry, 0);
					return 0;
				}
				else if (curSys6Item==1)
				{
					if (++g_pwdretry>8)
						g_pwdretry=0;
					SendMessage(Sys6Wnd[curSys6Item], CB_SETCURSEL, g_pwdretry, 0);
					return 0;
				}
				else if (curSys6Item==2)
				{
					if (++g_interstyle>1)
						g_interstyle=0;
					SendMessage(Sys6Wnd[curSys6Item], CB_SETCURSEL, g_interstyle, 0);
					return 0;
				}
				else if (curSys6Item==5)
				{
					if (++g_capturepic>3)
						g_capturepic=0;
					SendMessage(Sys6Wnd[curSys6Item], CB_SETCURSEL, g_capturepic, 0);
					return 0;
				}
				else if (curSys6Item>5)
				{
					if (++curSys6Item>7)
						curSys6Item=6;
					SetFocusChild(Sys6Wnd[curSys6Item]);
					return 0;
				}
				break;
			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if (curSys6Item==0)
				{
					if (--g_fpretry<0)
						g_fpretry=8;
					SendMessage(Sys6Wnd[curSys6Item], CB_SETCURSEL, g_fpretry, 0);
					return 0;
				}
				else if (curSys6Item==1)
				{
					if (--g_pwdretry<0)
						g_pwdretry=8;
					SendMessage(Sys6Wnd[curSys6Item], CB_SETCURSEL, g_pwdretry, 0);
					return 0;
				}
				else if (curSys6Item==2)
				{
					if (--g_interstyle<0)
						g_interstyle=1;
					SendMessage(Sys6Wnd[curSys6Item], CB_SETCURSEL, g_interstyle, 0);
					return 0;
				}
				else if (curSys6Item==5)
				{
					if (--g_capturepic<0)
						g_capturepic=3;
					SendMessage(Sys6Wnd[curSys6Item], CB_SETCURSEL, g_capturepic, 0);
					return 0;
				}
				else if (curSys6Item>5)
				{
					if (--curSys6Item<6)
						curSys6Item=7;
					SetFocusChild(Sys6Wnd[curSys6Item]);
					return 0;
				}
				break;
			}

			if (LOWORD(wParam)==SCANCODE_MENU)
			{
				PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_ENTER)
			{
				if (curSys6Item < 6)
				{
					PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
				}
				break;
			}

			if (LOWORD(wParam)==SCANCODE_F10)
			{
				if (curSys6Item == 7)
					PostMessage(hWnd, MSG_COMMAND, IDCANCEL, 0);
				else
					PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
				return 0;
			}
			break;

		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					{
						int ret;
						ret = isChanged();
						if (ret >= 0)
						{
							if(ret==0)
							{
								SaveParam();
								MessageBox1(hWnd, LoadStrByID(MID_RIGHT), LoadStrByID(MID_RUN),
										MB_OK | MB_ICONINFORMATION);
								if(!ismenutimeout)
									PostMessage(hWnd,MSG_CLOSE,0,0);
								return 0;
							}
							else
							{
								if(ret == 3)
								{
									MessageBox1(hWnd, LoadStrByID(MID_INVALIDDATA), LoadStrByID(MID_WARNING),
											MB_OK | MB_ICONINFORMATION);
								}
								else if(ret == 4)	
								{
									MessageBox1(hWnd, LoadStrByID(MID_INVALIDDATA2), LoadStrByID(MID_WARNING),
											MB_OK | MB_ICONINFORMATION);
								}
								if (!ismenutimeout)
								{
									curSys6Item=ret;
									SetFocusChild(Sys6Wnd[curSys6Item]);
								}
								return 0;
							}
						}
						else
							PostMessage(hWnd, MSG_CLOSE, 0, 0);
						break;
					}
				case IDCANCEL:
					if (isChanged() >= 0)
					{
						if(MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
									MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
						{
							PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
						}
						else
						{
							if (!ismenutimeout)
								PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
					}
					else
						PostMessage(hWnd, MSG_CLOSE, 0, 0);

					break;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&Interface);
			UnloadBitmap(&Clock1);
			UnloadBitmap(&Clock2);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int CreateSystem6Window(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	LoadWinStr();

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_INTERFACE);
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = System6WinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
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

