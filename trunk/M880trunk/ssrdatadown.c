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

#define IDC_DOWNLOADATT         700
#define IDC_USERDOWNLOAD        701     
#define IDC_SMSDOWNLOAD         702
#define IDC_PHOTODOWNLOAD       703
#define IDC_PICDOWNLOAD         704 
#define IDC_DOWN_EXIT           705

static HWND DataItemWnd1[6];
static int DataItem1;
//static BITMAP databk1;
extern void SSR_MENU_DATADLG(HWND hWnd, int i);

static int DataOK1(HWND hWnd, int item)
{
	SSR_MENU_DATADLG(hWnd, item);
	return 0;
}

static void InitUSBDownloadWindow(HWND hWnd)
{
	int rows=0;

	DataItemWnd1[0] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_UDATA1), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDC_DOWNLOADATT, 60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	rows++;

	DataItemWnd1[1] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_UDATA2), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDC_USERDOWNLOAD, 
					60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	rows++;

	DataItemWnd1[2] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_DATASMSDOWNLOAD), WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_SMSDOWNLOAD,
					60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	if (gOptions.IsSupportSMS)
	{
		ShowWindow(DataItemWnd1[2], SW_SHOW);
		rows++;
	}

	DataItemWnd1[3] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_DOWNLOAD_PHOTO), WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_PHOTODOWNLOAD, 
					60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	if (gOptions.IsSupportPhoto && (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))!=0))
	{
		ShowWindow(DataItemWnd1[3], SW_SHOW);
		rows++;
	}

	DataItemWnd1[4] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_DOWN_ATTPIC), WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_PICDOWNLOAD, 60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);
	if (gOptions.CameraOpen)
	{
		ShowWindow(DataItemWnd1[4], SW_SHOW);
		rows++;
	}

	DataItemWnd1[5] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_EXIT), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDC_DOWN_EXIT,60+gOptions.GridWidth*2, 10+30*rows, 200, 23, hWnd, 0);

}

static int USBDownLoadWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &databk1, GetBmpPath("submenubg.jpg")))
                        //	return 0;
			InitUSBDownloadWindow(hWnd);
			UpdateWindow(hWnd, TRUE);
	                DataItem1=0;
			SetFocusChild(DataItemWnd1[DataItem1]);
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				char buffer[32];

				GetWindowText(DataItemWnd1[DataItem1],buffer,32);
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
        	        EndPaint(hWnd, hdc);
	                return 0;

		case MSG_KEYDOWN:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				TTS_PlayWav(GetWavPath("item.wav"));
			}
#endif			
			SetMenuTimeOut(time(NULL));
			if(gOptions.KeyPadBeep)
                                ExKeyBeep();

                        if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
                        {
                                if(++DataItem1 > 5)
					DataItem1=0;
				if(!gOptions.IsSupportSMS && DataItem1==2)
					DataItem1++;
				if (((!gOptions.IsSupportPhoto) || (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))==0))
					&& DataItem1==3)
					DataItem1++;
				if (!gOptions.CameraOpen && DataItem1==4)
					DataItem1++;
                                SetFocus(DataItemWnd1[DataItem1]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];

					GetWindowText(DataItemWnd1[DataItem1],buffer,32);
					TTS_Say(buffer);
				}
#endif                                
                                return 0;
                        }

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
                        {
                                if(--DataItem1 < 0)
					DataItem1 = 5;
				if (!gOptions.CameraOpen && DataItem1==4)
					DataItem1--;
				if (((!gOptions.IsSupportPhoto) || (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))==0))
					&& DataItem1==3)
					DataItem1--;
				if(!gOptions.IsSupportSMS && DataItem1==2)
					DataItem1--;
                                SetFocus(DataItemWnd1[DataItem1]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];

					GetWindowText(DataItemWnd1[DataItem1],buffer,32);
					TTS_Say(buffer);
				}
#endif                          
                                return 0;
                        }

                        if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_MENU) || (LOWORD(wParam)==SCANCODE_F10))
                        {
				int ItemList[] = {0, 1, 2, 3};
				if (DataItem1 < 4)
				{
	                                if(DataOK1(hWnd, ItemList[DataItem1]))
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				else if (DataItem1==4)
					CaptureDownLoadWindow(hWnd);
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
			//UnloadBitmap(&databk1);
			//MainWindowCleanup(hWnd);
                        DestroyMainWindow(hWnd);
			return 0;
	}
    
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

void  SSR_DATA_DOWNLOAD(HWND hWnd)
{
        MSG msg;
        HWND hMainWnd;
        MAINWINCREATE CreateInfo;

        hWnd = GetMainWindowHandle(hWnd);
        CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
        CreateInfo.dwExStyle = WS_EX_NONE;
        CreateInfo.spCaption = LoadStrByID(MID_USB_DOWNLOAD);
        CreateInfo.hMenu = 0;
        //CreateInfo.hCursor = GetSystemCursor(0);
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = USBDownLoadWinProc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
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
        return;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

