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
#include "exfun.h"

#define MSG_WRITELCD 	(MSG_USER+100)
#define IDC_TIMER_LCD	1000

BITMAP mhdaybmpbg;
HWND hWriteLCDWnd = HWND_NULL;
int CloseDelay;

static int WriteLCDWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char msg[50], msgtimer[5];
	switch (message)
	{
		case MSG_CREATE:
			SetTimer(hWnd, IDC_TIMER_LCD, 100);
			break;
		case MSG_CLOSE:
			KillTimer(hWriteLCDWnd, IDC_TIMER_LCD);
			//MainWindowCleanup(hWriteLCDWnd);
			DestroyMainWindow(hWriteLCDWnd);
			//hWriteLCDWnd = HWND_INVALID;
			break;
		//case MSG_PAINT:			
		//	hdc  = BeginPaint(hWriteLCDWnd);
		//	sprintf(msgtimer, "%d", CloseDelay);
		//	TextOut(hdc, g_rcScr.right-32, g_rcScr.bottom-40, msgtimer);
		//	EndPaint(hWriteLCDWnd,hdc);
		//	break;
		case MSG_WRITELCD:
			CloseDelay = gOptions.WriteLCDDelay;
			memset(msg, 0, sizeof(msg));
			strcpy(msg, (char*)lParam);
			//printf("wparam:%d\n", wParam);

			int row = ((wParam >> 8) & 0xFF);
			int col = (wParam & 0xFF);
			//printf("row:%d, col:%d\n", row, col);
			if (row > 1)
				row = (row-1) * 16;
			if (col > 1)
				col = (col-1) * 16;
			//printf("row:%d, col:%d\n", row, col);
			hdc  = BeginPaint(hWnd);
                        TextOut(hdc, col, row, msg);
                        EndPaint(hWnd,hdc);
			//ExPlayVoice(VOICE_HAND1);
			return 0;
		case MSG_KEYDOWN:
			if(wParam==SCANCODE_ESCAPE)
			{
				CloseDelay=1;
			}
			break;
		case MSG_TIMER:
			if (wParam == IDC_TIMER_LCD && CloseDelay)
			{
				CloseDelay--;
				sprintf(msgtimer, "%.2d", CloseDelay);
				hdc  = BeginPaint(hWnd);
	                        TextOut(hdc, g_rcScr.right-20, g_rcScr.bottom-64, msgtimer);
				EndPaint(hWnd,hdc);
				if (CloseDelay <= 0)
				{
					PostMessage(hWriteLCDWnd, MSG_CLOSE, 0, 0L);
				}
			}
			break;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateWriteLCDWindow(HWND hwnd)
{
	MSG msg;
	MAINWINCREATE CreateInfo;

	hwnd = GetMainWindowHandle(hwnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(627);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WriteLCDWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
      	CreateInfo.by = g_rcScr.bottom-23;
	CreateInfo.iBkColor = COLOR_lightwhite;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hwnd;

	hWriteLCDWnd = CreateMainWindow(&CreateInfo);
	if (hWriteLCDWnd == HWND_INVALID)
		return -1;

	ShowWindow(hWriteLCDWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg, hWriteLCDWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hWriteLCDWnd);
	hWriteLCDWnd = HWND_INVALID;
	return 0;

}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

