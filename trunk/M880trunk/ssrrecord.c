/*************************************************
  Copyright(C), 2007-2008, zksoftware
  File name: ssrrecord.c
  Author: liming  Version: 0.1  Date: 2008.01.16
  Description: 记录查询
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

#define RD_ATTLOG		2080
#define RD_ATTPHOTO		2081
#define RD_HACKPHOTO		2082
#define RD_EXIT			2083

HWND RDWnd[4];			//控件句柄
static int curindex;		//当前控件序号
//BITMAP RDbkgbmp;

static void InitRDWindow(HWND hWnd)
{
	char * RDStr[4];

	RDStr[0]=LoadStrByID(MID_QUERY_ATTLOG);
	RDStr[1]=LoadStrByID(MID_QUERY_SET);
	RDStr[2]=LoadStrByID(MID_QUERY_SET2);
	RDStr[3]=LoadStrByID(MID_EXIT);

	RDWnd[0] = CreateWindow(CTRL_BUTTON, RDStr[0], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, RD_ATTLOG, 60+gOptions.ControlOffset/2, 20, 200, 23, hWnd, 0);
	RDWnd[1] = CreateWindow(CTRL_BUTTON, RDStr[1], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, RD_ATTPHOTO, 60+gOptions.ControlOffset/2, 50, 200, 23, hWnd, 0);
	RDWnd[2] = CreateWindow(CTRL_BUTTON, RDStr[2], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, RD_HACKPHOTO, 60+gOptions.ControlOffset/2, 80, 200, 23, hWnd, 0);
	RDWnd[3] = CreateWindow(CTRL_BUTTON, RDStr[3], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, RD_EXIT, 60+gOptions.ControlOffset/2, 110, 200, 23, hWnd, 0);
}

static int RecordWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	static char keyupFlag=0;
	
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &RDbkgbmp, GetBmpPath("submenubg.jpg")))
	                //        return 0;

			InitRDWindow(hWnd);		//add controls
			curindex = 0;
			SetFocusChild(RDWnd[curindex]);
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

			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
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
				if (++curindex > 3)
				{
					curindex = 0;
				}
				SetFocusChild(RDWnd[curindex]);
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if (--curindex < 0)
				{
					curindex = 3;
				}
				SetFocusChild(RDWnd[curindex]);
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
					PostMessage(hWnd, MSG_COMMAND, RD_ATTLOG, 0);
				}
				else if (curindex == 1)
				{
					PostMessage(hWnd, MSG_COMMAND, RD_ATTPHOTO, 0);
				}
				else if (curindex == 2)
				{
					PostMessage(hWnd, MSG_COMMAND, RD_HACKPHOTO, 0);
				}
				else if (curindex == 3)
				{
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}

			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
                        nc = HIWORD(wParam);

			if (id == RD_ATTLOG)
			{
				ShowLogSetWindow(hWnd);
				return 0;
			}
			else if (id == RD_ATTPHOTO)
			{
				CreateFindPhotoSetWindow(hWnd, 0, "");
				return 0;
			}
			else if (id == RD_HACKPHOTO)
			{
				CreateFindPhotoSetWindow(hWnd, 2, "");
				return 0;
			}
			else if (id == RD_EXIT)
			{
				PostMessage(hWnd, MSG_CLOSE, 0, 0);
			}
			break;
	
		case MSG_CLOSE:
			//UnloadBitmap(&RDbkgbmp);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateRecordWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER|WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_ATTQUERY);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = RecordWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
        CreateInfo.by = gOptions.LCDHeight;
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

