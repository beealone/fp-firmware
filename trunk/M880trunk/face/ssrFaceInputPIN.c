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

#include "zkface.h"
#include "facedb.h"
#include "main.h"

#define IDC_LB_PIN	 500
#define IDC_PIN	 	 510
#define IDC_FG	 	 511
#define IDC_FACE 	 512

extern PLOGFONT gfont;
extern RECT title_rc;
extern char KeyBuffer[24];
extern BITMAP facetestdbkg;


static int f_mode=0;	//0= esc ; 1 = 1:g ; face  2= 1:1 face
static int f_ret=0;	//0 =esc ; 1=1:g ; 2= 1:1 
static BITMAP fbmpbar;

static void UpdateFaceInputWnd(HWND hWnd)
{
	HDC hdc=GetClientDC(hWnd);
	char buff[100];

	FillBoxWithBitmap (hdc, 0, 0, 320, 240, &facetestdbkg);
	FillBoxWithBitmap (hdc, 0, -1, 0, MENUBAR_HEIGHT, &fbmpbar);

	SelectFont(hdc,gfont);
	SetTextColor(hdc,PIXEL_lightwhite);
	SetBkMode(hdc,BM_TRANSPARENT);
	
	if(f_mode) //1:G
		sprintf(buff,"1:G%s",LoadStrByID(MID_FACE_VERIFY));
	else
		sprintf(buff,"1:1:%s",LoadStrByID(MID_FACE_VERIFY));
	
	DrawText(hdc,buff,-1,&title_rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	ReleaseDC(hdc);
}

static int FacePINWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HWND hCtrl;
	HWND focuswnd=(HWND)0;

	switch (message) 
	{
		case MSG_CREATE:
		{
			char*txt=NULL;


			UpdateFaceInputWnd(hWnd);
		#if 0
			CreateWindow(CTRL_BUTTON,(char*)LoadStrByID(MID_G_FACE), WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,\
					IDC_FG,50,180,90,23,hWnd,0);
			CreateWindow(CTRL_BUTTON,(char*)LoadStrByID(MID_ONE_FACE), WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,\
					IDC_FACE,180,180,90,23,hWnd,0);
		#endif
			if(f_mode ==1) //1:G
				txt=LoadStrByID(MID_VF_MAIN);
			else
				txt=LoadStrByID(MID_NOACNOHINT);
			CreateWindow(CTRL_STATIC, txt, WS_VISIBLE | SS_CENTER, IDC_LB_PIN, 50,90, 220, 23, hWnd, 0);
                        hCtrl=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_BORDER,IDC_PIN,50,120,220,23,hWnd,0);
			SendMessage(hCtrl,EM_LIMITTEXT,gOptions.PIN2Width,0L);
			SetWindowText(hCtrl,"");
                	SendMessage(hCtrl,EM_SETCARETPOS,0,0);
			SetFocusChild(hCtrl);

		}
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
			UpdateFaceInputWnd(hWnd);

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
		#if 0
			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) 
			{
				hCtrl  = GetFocusChild(hWnd);
				if(hCtrl == GetDlgItem(hWnd,IDC_PIN))
					focuswnd=GetDlgItem(hWnd,IDC_FG);
				else if(hCtrl == GetDlgItem(hWnd,IDC_FG))
					focuswnd=GetDlgItem(hWnd,IDC_FACE);
				else if(hCtrl == GetDlgItem(hWnd,IDC_FACE))
					focuswnd=GetDlgItem(hWnd,IDC_PIN);
				SetFocusChild(focuswnd);
			}
			else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				hCtrl  = GetFocusChild(hWnd);
				if(hCtrl == GetDlgItem(hWnd,IDC_FACE))
					focuswnd=GetDlgItem(hWnd,IDC_FG);
				else if(hCtrl == GetDlgItem(hWnd,IDC_FG))
					focuswnd=GetDlgItem(hWnd,IDC_PIN);
				else if(hCtrl == GetDlgItem(hWnd,IDC_PIN))
					focuswnd=GetDlgItem(hWnd,IDC_FACE);
				SetFocusChild(focuswnd);
			}
			else 
		#endif
			if (LOWORD(wParam)== SCANCODE_ESCAPE)
			{
				memset(KeyBuffer,0,sizeof(KeyBuffer));
				PostMessage(hWnd,MSG_CLOSE,0,0);
			}
			if (LOWORD(wParam)==SCANCODE_ENTER)
			{
			#if 0
				hCtrl  = GetFocusChild(hWnd);
				if(hCtrl == GetDlgItem(hWnd,IDC_FG))
			#endif
				if(f_mode) //1:G
				{
					int group=0;
					memset(KeyBuffer,0,sizeof(KeyBuffer));
					GetWindowText(GetDlgItem(hWnd,IDC_PIN),KeyBuffer,24);
					if(strlen(KeyBuffer))
					{
						group=atoi(KeyBuffer);
						if(group <= 0 || group > gFaceGroupNum )
				                {
							memset(KeyBuffer,0,sizeof(KeyBuffer));
				                       	SetWindowText(GetDlgItem(hWnd,IDC_PIN),"");
				                }
				                else
				                {
							f_ret=1; // 1:g face
							PostMessage(hWnd,MSG_CLOSE,0,0L);
				                }
					}
					else
						memset(KeyBuffer,0,sizeof(KeyBuffer));
				}
				else //if(hCtrl == GetDlgItem(hWnd,IDC_FACE))
				{
					memset(KeyBuffer,0,sizeof(KeyBuffer));
					GetWindowText(GetDlgItem(hWnd,IDC_PIN),KeyBuffer,24);
					if(strlen(KeyBuffer) && (FDB_GetUserByCharPIN2(KeyBuffer, NULL)!=NULL))
					{
						f_ret=2; // 1:1 face
						PostMessage(hWnd,MSG_CLOSE,0,0);
					}
					else
					{
						memset(KeyBuffer,0,sizeof(KeyBuffer));
				                SetWindowText(GetDlgItem(hWnd,IDC_PIN),"");
						if(gOptions.VoiceOn)
                                                        ExPlayVoice(VOICE_INVALID_ID);
                                                else
                                                        ExBeep(2);
					}
				}
				SetFocusChild(GetDlgItem(hWnd,IDC_PIN));
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
                        //MainWindowCleanup(hWnd);
                        DestroyMainWindow(hWnd);
		        return 0;
        
	}
    
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

//f_mode=1: 1:G face ; f_mode=2: 1:1 face
int CreatFacePinWindow(HWND hWnd , int mode)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	f_mode=mode;
	f_ret=0;
	vfwndflag=1;
	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE ;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "Input PIN";
	CreateInfo.hMenu = 0;

//	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = FacePINWinProc;
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
	LoadBitmap(HDC_SCREEN, &fbmpbar, GetBmpPath("bar.bmp"));
        hMainWnd = CreateMainWindow(&CreateInfo);
        if (hMainWnd == HWND_INVALID)
	{
		vfwndflag=0;
		UnloadBitmap(&facetestdbkg);
		UnloadBitmap(&fbmpbar);
                return 0;
	}

        ShowWindow(hMainWnd, SW_SHOWNORMAL);
        while (GetMessage(&msg, hMainWnd))
        {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
        MainWindowThreadCleanup(hMainWnd);
	vfwndflag=0;
	UnloadBitmap(&facetestdbkg);
	UnloadBitmap(&fbmpbar);
        return f_ret;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

