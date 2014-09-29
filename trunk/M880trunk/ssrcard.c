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

#define ID_CARD_OPT	0x50

HWND BtWndCard[6]={0};
int activeindex;
//BITMAP cardbgbmp;

static void InitWindow (HWND hWnd)
{
	int i;
	char* cardstr[6];

        for(i=0;i<5;i++)
        {
                cardstr[i]=LoadStrByID(MID_CARD_OP1+i);
        }
        cardstr[5]=LoadStrByID(MID_EXIT);

	for(i=0;i<6;i++)
	{
		BtWndCard[i] = CreateWindow(CTRL_BUTTON, cardstr[i], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
		ID_CARD_OPT+i, 40+(gOptions.ControlOffset/2),10+(23+5)*i,240,23,hWnd,0);
	}

}

static int cardwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &cardbgbmp, GetBmpPath("submenubg.jpg")))
	                //       return 0;
				
			InitWindow(hWnd);		//add controls
			UpdateWindow(hWnd,TRUE);
			activeindex=0;
			SetFocusChild(BtWndCard[activeindex]);
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
				if(++activeindex>5) activeindex=0;
				SetFocusChild(BtWndCard[activeindex]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if(--activeindex<0) activeindex=5;
				SetFocusChild(BtWndCard[activeindex]);
				return 0;
			}
			if((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10)) 
			{
				if(activeindex<4)
				{
					CreateMFCardWindow(hWnd, activeindex);
				}
				else if(activeindex==4)
					CreateMFManageWindow(hWnd);
				else
					PostMessage(hWnd,MSG_CLOSE,0,0L);
				return 0;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&cardbgbmp);
			DestroyMainWindow(hWnd);
//			MainWindowCleanup (hWnd);
			return 0;
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CardMngWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_CARD_MNG);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = cardwinproc;
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

