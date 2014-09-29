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

#define ID_LOCK_OPT	0x60

HWND BtWnd[7];
int activeindex;
static char* hintchar[7];
//static BITMAP bgbmp;
int optbtncount;

static void InitWindow (HWND hWnd)
{
	int i;
	for(i=0;i<6;i++)
	{
		hintchar[i]=LoadStrByID(MID_LOCK_OP1+i);
	}
	hintchar[6]=LoadStrByID(MID_OS_CUST);

	if (gOptions.LockFunOn)
	{
		optbtncount=6;
		if(gOptions.AntiPassbackFunOn)
			optbtncount++;

		for(i=0;i<optbtncount;i++)
		{
			BtWnd[i] = CreateWindow(CTRL_BUTTON, hintchar[i], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
					ID_LOCK_OPT+i, 40+gOptions.ControlOffset/2,10+(23+5)*i,240,23,hWnd,0);
		}
	}
	else
	{
		optbtncount=3;
		for(i=0;i<optbtncount;i++)
		{
			BtWnd[i] = CreateWindow(CTRL_BUTTON, hintchar[i], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
					ID_LOCK_OPT+i, 40+gOptions.ControlOffset/2,30+(43+5)*i,240,30,hWnd,0);
		}

	}
}

static int lockwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//int id,nc;
	//int i;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&bgbmp,GetBmpPath("submenubg.jpg")))
	                //        return 0;

			InitWindow(hWnd);		//add controls
			UpdateWindow(hWnd,TRUE);
			activeindex=0;
			SetFocusChild(BtWnd[activeindex]);
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
				PostMessage(hWnd,MSG_CLOSE,0,0L);

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if(++activeindex>optbtncount-1) activeindex=0;
				SetFocusChild(BtWnd[activeindex]);
				return 0;
			}
			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if(--activeindex<0) activeindex=optbtncount-1;
				SetFocusChild(BtWnd[activeindex]);
				return 0;
			}
			if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
			{
				switch(activeindex)
				{
					case 0:		//时间段设置
						TimeZoneMngWindow(hWnd, 0, 0);
						break;
					case 1:		//节假日设置
						HolidayMngWindow(hWnd);
						break;
					case 2:		//组时间段设置
						GroupMngWindow(hWnd);
						break;
					case 3:		//开锁组合设置
						CGroupMngWindow(hWnd);
						break;
					case 4:		//门禁管理参数
						DoorParameterWindow(hWnd);
						break;
					case 5:		//胁迫报警参数
						DuressParameterWindow(hWnd);
						break;
					case 6:		//其他设置
						OTSetWindow(hWnd);
						break;
				}
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&bgbmp);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int LockSettingWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(HIT_USER5);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = lockwinproc;
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

