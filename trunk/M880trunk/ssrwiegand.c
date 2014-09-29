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

#define WG_SETIN		2040
#define WG_SETOUT		2041
#define WG_EXIT			2042

HWND WGWnd[3];			//控件句柄
static int curindex;		//当前控件序号
//BITMAP WGbkgbmp;

static void InitWGWindow(HWND hWnd)
{
	char * WGStr[3];

	WGStr[0]=LoadStrByID(MID_WG_SETIN);
	WGStr[1]=LoadStrByID(MID_WG_SETOUT);
	WGStr[2]=LoadStrByID(MID_EXIT);

	WGWnd[0] = CreateWindow(CTRL_BUTTON, WGStr[0], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, WG_SETIN, 100+gOptions.GridWidth*2, 20, 120, 23, hWnd, 0);
	WGWnd[1] = CreateWindow(CTRL_BUTTON, WGStr[1], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, WG_SETOUT, 100+gOptions.GridWidth*2, 50, 120, 23, hWnd, 0);
	WGWnd[2] = CreateWindow(CTRL_BUTTON, WGStr[2], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, WG_EXIT, 100+gOptions.GridWidth*2, 80, 120, 23, hWnd, 0);
}

static int WiegandWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	//int i;
	static char keyupFlag=0;
	
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &WGbkgbmp, GetBmpPath("submenubg.jpg")))
	                //        return 0;

			InitWGWindow(hWnd);		//add controls
			curindex = 0;
			SetFocusChild(WGWnd[curindex]);
			UpdateWindow(hWnd,TRUE);
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
				PostMessage(hWnd, MSG_CLOSE, 0, 0);

			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if (++curindex > 2)
				{
					curindex = 0;
				}
				SetFocusChild(WGWnd[curindex]);
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if (--curindex < 0)
				{
					curindex = 2;
				}
				SetFocusChild(WGWnd[curindex]);
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_F10)
			{
				if (curindex == 0)
				{
					PostMessage(hWnd, MSG_COMMAND, WG_SETIN, 0);
				}
				else if (curindex == 1)
				{
					PostMessage(hWnd, MSG_COMMAND, WG_SETOUT, 0);
				}
				else if (curindex == 2)
				{
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}

			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
                        nc = HIWORD(wParam);

			if (id == WG_SETIN)
			{
				CreateWiegandInWindow(hWnd);
				//CreateWiegandOutWindow(hWnd, WG_IN_MODE);
				return 0;
			}
			else if (id == WG_SETOUT)
			{
				CreateWiegandOutWindow(hWnd, WG_OUT_MODE);
				return 0;
			}
			else if (id == WG_EXIT)
			{
				PostMessage(hWnd, MSG_CLOSE, 0, 0);
			}
			break;
	
		case MSG_CLOSE:
			//UnloadBitmap(&WGbkgbmp);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateWiegandWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER|WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_WG_SETTING);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WiegandWinProc;
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

