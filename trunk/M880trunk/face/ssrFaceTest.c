#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h> 
#include <sys/types.h>
#include <dirent.h>
#include "exfun.h"
#include "ssrcommon.h"
#include "ssrpub.h"
#include <minigui/tftmullan.h>

#include "libcam.h"
#include "flashdb.h"
#include "zkface.h"
#include "facedb.h"
#include "bmp.h"

#define IDC_FACE_TIMER_T	9988
extern void InitParam(int mod);
int ShowCaptureFace(HWND hWnd , HDC hdc , int flag);
void ShowFaceQuality(HWND hWnd ,HDC hdc ,int Quality, PBITMAP bmpbg,int flag);
int InitShowFaceImage(void);

extern PLOGFONT gfont;	//display font
extern BITMAP fvbar;
extern RECT title_rc;
extern RECT quality_rc;
extern BITMAP FaceImage;
extern int TestFlage;

BITMAP facetestdbkg;


static void UpdateFaceTestWindow(HWND hWnd,HDC hdc)
{

	hdc=GetClientDC(hWnd);
	FillBoxWithBitmap (hdc, 0, MENUBAR_HEIGHT,320, 240, &facetestdbkg);
	FillBoxWithBitmap (hdc, 0, -1, 0, MENUBAR_HEIGHT, &fvbar);

	SelectFont(hdc,gfont);
        SetTextColor(hdc,PIXEL_lightwhite);
        SetBkMode(hdc,BM_TRANSPARENT);
	DrawText(hdc,(char*)LoadStrByID(MID_FACE_TEST),-1,&title_rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	ReleaseDC(hdc);
}

static int FaceTestWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//HWND hCtrl;

	switch (message) 
	{
		case MSG_CREATE:  // GetBmpPath("desktop.jpg")


			UpdateFaceTestWindow(hWnd,HDC_SCREEN);
			InitShowFaceImage();				
			SetTimer(hWnd, IDC_FACE_TIMER_T, IDC_TIMER_NUM);
			TestFlage=0;
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
                        if(fGetDC) ReleaseDC (hdc);
                        return 0;
                }
		
                case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			EndPaint(hWnd,hdc);
                        return 0;

		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
                        if (gOptions.KeyPadBeep)
                                ExKeyBeep();
			if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
                        {
                                PostMessage(hWnd,MSG_CLOSE,0,0);
                                return 1;
                        }
			else if(LOWORD(wParam) == SCANCODE_ESCAPE)
			{
				TestFlage=-1;
				PostMessage(hWnd, MSG_CLOSE, 0, 0);
			}
			break;

		case MSG_TIMER:
			if (wParam == IDC_FACE_TIMER_T)
			{
				KillTimer(hWnd, IDC_FACE_TIMER_T);
				if(ismenutimeout)
				{
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
					return 0;
				}
				if(ShowCaptureFace( hWnd,hdc ,2))
					SetMenuTimeOut(time(NULL));
				SetTimer(hWnd, IDC_FACE_TIMER_T, IDC_TIMER_NUM);
			}
			break;
                case MSG_IDLE:
                        {
                                timer_t ctimer=time(NULL);
                                if(((int)ctimer >= (gMenuTimeOut+60)) && !busyflag)
                                {
                                        ismenutimeout=1;
                                        PostMessage(hWnd,MSG_CLOSE,0,0);
                                }
                        }
                        break;

		case MSG_CLOSE:
			KillTimer(hWnd, IDC_FACE_TIMER_T);
			UnloadBitmap(&fvbar);
			UnloadBitmap(&facetestdbkg);

			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}


int CreateFaceTestWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	
	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE ;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "Face Verify";
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = FaceTestWinProc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
        CreateInfo.iBkColor = 0x00FFA2BE;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hWnd;

	ismenutimeout=0;
	SetMenuTimeOut(time(NULL));
	LoadBitmap(HDC_SCREEN, &facetestdbkg, GetBmpPath("mainmenu.jpg"));
	LoadBitmap(HDC_SCREEN, &fvbar, GetBmpPath("bar.bmp"));
	
        hMainWnd = CreateMainWindow(&CreateInfo);
        if (hMainWnd == HWND_INVALID)
	{
		UnloadBitmap(&fvbar);
		UnloadBitmap(&facetestdbkg);
                return 0;
	}

        ShowWindow(hMainWnd, SW_SHOWNORMAL);
        while (GetMessage(&msg, hMainWnd))
        {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
        MainWindowThreadCleanup(hMainWnd);
	//UnloadBitmap(&fvbar);
	//UnloadBitmap(&facetestdbkg);

        return 1;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

