/*
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0
* 修改记录:
* 编译环境:mipsel-gcc
*/

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

#define IDC_USERLOAD	601
#define IDC_DEFINEPIC	602
#define IDC_SMSLOAD	603
#define IDC_PHOTOLOAD	604
#define IDC_EXIT2	605
#define IDC_THEME	606

static HWND DataItemWnd2[6];
static int DataItem2;
//static BITMAP databk2;

extern void SSR_MENU_DATADLG(HWND hWnd, int i);

static int DataOK2(HWND hWnd, int item)
{
	SSR_MENU_DATADLG(hWnd, item);
	return 0;
}

static void InitUSBUploadWindow(HWND hWnd)
{
	int rows=0;

	DataItemWnd2[0] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_UDATA3), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDC_USERLOAD, 
					60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	rows++;

	DataItemWnd2[1] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_DATASMSLOAD), WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_SMSLOAD, 
					60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	if (gOptions.IsSupportSMS)
	{
		ShowWindow(DataItemWnd2[1], SW_SHOW);
		rows++;
	}

	DataItemWnd2[2] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_LOAD_PHOTO), WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_PHOTOLOAD, 
					60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	if (gOptions.IsSupportPhoto && (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))!=0))
	{
		ShowWindow(DataItemWnd2[2], SW_SHOW);
		rows++;
	}

	DataItemWnd2[3] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_UDATA4), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDC_DEFINEPIC, 
					60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	rows++;

	DataItemWnd2[4] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_IKIOSK_THEME), WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_THEME, 
					60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
#ifdef IKIOSK
	ShowWindow(DataItemWnd2[4], SW_SHOW);
	rows++;
#endif

	DataItemWnd2[5] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_EXIT), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
					IDC_EXIT2, 60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
}

static int USBUpLoadWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &databk2, GetBmpPath("submenubg.jpg")))
                        //	return 0;
			InitUSBUploadWindow(hWnd);
			UpdateWindow(hWnd, TRUE);
	                DataItem2=0;
			SetFocusChild(DataItemWnd2[DataItem2]);
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				char buffer[32];
				
				GetWindowText(DataItemWnd2[DataItem2],buffer,32);
				TTS_Say(buffer);
			}
#endif
        		break;

	        case MSG_ERASEBKGND:
        	{
			HDC hdc = (HDC)wParam;
			const RECT* clip = (const RECT*) lParam;
			BOOL fGetDC = FALSE;
			RECT rcTemp;

			if (hdc == 0) 
			{
				hdc = GetClientDC (hWnd);
				fGetDC = TRUE;
			}

			if (clip) {
		                rcTemp = *clip;
                		ScreenToClient (hWnd, &rcTemp.left, &rcTemp.top);
		                ScreenToClient (hWnd, &rcTemp.right, &rcTemp.bottom);
		                IncludeClipRect (hdc, &rcTemp);
			}

			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

			if (fGetDC)
		                ReleaseDC (hdc);
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
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				TTS_PlayWav(GetWavPath("item.wav"));
			}
#endif			
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

                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
                        {
                                if(++DataItem2 > 5)
					DataItem2=0;
				if(!gOptions.IsSupportSMS && DataItem2==1)
					DataItem2++;
				if (((!gOptions.IsSupportPhoto) || (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))==0))
					&& DataItem2==2)
					DataItem2++;
 #ifndef IKIOSK
				if (DataItem2==4)
					DataItem2++;
#endif
                               SetFocus(DataItemWnd2[DataItem2]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];

					GetWindowText(DataItemWnd2[DataItem2],buffer,32);
					TTS_Say(buffer);
				}
#endif                               
                                return 0;
                        }

                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
                        {
                                if(--DataItem2 < 0)
					DataItem2 = 5;
#ifndef IKIOSK
				if (DataItem2==4)
					DataItem2--;
#endif
				if (((!gOptions.IsSupportPhoto) || (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))==0))
					&& DataItem2==2)
					DataItem2--;
				if(!gOptions.IsSupportSMS && DataItem2==1)
					DataItem2--;
                                SetFocus(DataItemWnd2[DataItem2]);
                                return 0;
                        }
                        if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_MENU) || (LOWORD(wParam)==SCANCODE_F10))
                        {
				int ItemList[] = {4, 5, 6, 7, 11};
				if (DataItem2 != 5)
				{
	                                if(DataOK2(hWnd, ItemList[DataItem2]))
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				else
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				return 0;
                        }

                        if ((LOWORD(wParam)==SCANCODE_ESCAPE))
                        {
				PostMessage(hWnd, MSG_CLOSE, 0, 0);
                                return 0;
                        }
			
			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
				return 0;

                        break;

		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
				TTS_Stop();
#endif			
			//UnloadBitmap(&databk2);
			//MainWindowCleanup(hWnd);
                        DestroyMainWindow(hWnd);
			return 0;
	}
    
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

void SSR_DATA_UPLOAD(HWND hWnd)
{
        MSG msg;
        HWND hMainWnd;
        MAINWINCREATE CreateInfo;

        hWnd = GetMainWindowHandle(hWnd);
        CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
        CreateInfo.dwExStyle = WS_EX_NONE;
        CreateInfo.spCaption = LoadStrByID(MID_USB_UPLOAD);
        CreateInfo.hMenu = 0;
        //CreateInfo.hCursor = GetSystemCursor(0);
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = USBUpLoadWinProc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = gOptions.LCDWidth;
        CreateInfo.by = gOptions.LCDHeight;
        CreateInfo.iBkColor = 0x00FFA2BE;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hWnd;

        hMainWnd = CreateMainWindow(&CreateInfo);
        if (hMainWnd == HWND_INVALID)
                return;
        ShowWindow(hMainWnd, SW_SHOWNORMAL);

        while (GetMessage(&msg,hMainWnd))
        {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        MainWindowThreadCleanup(hMainWnd);
        return ;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

