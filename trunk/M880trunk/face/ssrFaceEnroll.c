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


#define IDC_FACE_TIMER1	9020
extern void InitParam(int mod);
int InitShowFaceImage(void);
extern int ShowCaptureFace(HWND hWnd , HDC hdc , int flag);
extern void ShowFaceQuality(HWND hWnd ,HDC hdc ,int Quality, PBITMAP bmpbg,int flag);
extern void ShowFaceHint(HWND hWnd , int Fhint ,HDC hdc , PBITMAP bmpbg,int flag);

extern PLOGFONT gfont;	//display font
extern RECT title_rc;
extern RECT hint_rc;
extern RECT quality_rc;
extern BITMAP fvbar, fvbg;

//HDC g_hdcFaceReg;
static int f_pin=0;
static BOOL regok=0;
int FaceRegState=0;

static void UpdateFaceRegWindow(HWND hWnd,HDC hdc)
{
	hdc=GetClientDC(hWnd);
	SelectFont(hdc,gfont);
        SetTextColor(hdc,PIXEL_lightwhite);
        SetBkMode(hdc,BM_TRANSPARENT);
	FillBoxWithBitmap (hdc, 0, -1, 0, MENUBAR_HEIGHT, &fvbar);
	DrawText(hdc,(char*)LoadStrByID(FACE_REG),-1,&title_rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	FillBoxWithBitmap (hdc, 0, MENUBAR_HEIGHT, 0, 0, &fvbg);
	ReleaseDC(hdc);
}

static int FaceEnrollWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int ret=0;
	RECT rect={142,221,180,240};

	switch (message) 
	{
		case MSG_CREATE:
			UpdateFaceRegWindow( hWnd, hdc);
			InitShowFaceImage();
			FaceRegState=FACE_REG_PREPARE;

			SetTimer(hWnd, IDC_FACE_TIMER1, IDC_TIMER_NUM);
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
			UpdateFaceRegWindow(hWnd,hdc);

                        if(fGetDC) ReleaseDC (hdc);
                        	return 0;
                }
			return 0;
		
                case MSG_PAINT:
		{
			char buf[20];
                        hdc = BeginPaint(hWnd);
			SelectFont(hdc,gfont);
			SetTextColor(hdc,PIXEL_lightwhite);
       	 		SetBkMode(hdc,BM_TRANSPARENT);
                        SetPenColor(hdc,PIXEL_black);
    			SetBrushColor(hdc,PIXEL_lightgray);
			SetPenWidth(hdc, 2);
			LineEx(hdc,-1,221,322,222);
			FillBox(hdc,-1,223,322,20);
			if(RegBegin == REG_PREPARE)
			{
				sprintf(buf,"0%%");
				TextOut(hdc,145,225,buf);
			}
			else
			{
                        	SetBrushColor (hdc, PIXEL_blue);
                        	if(FaceCount>=FACE_NUM)  // 0,220,320,20
                        	{
                        	        FillBox(hdc,1,223,318,17);
					sprintf(buf,"100%%");
					TextOut(hdc,145,225,buf);
                        	}
                        	else if(FaceCount>0)
                        	{
                        	        FillBox(hdc,1,223,320*FaceCount/FACE_NUM,17);
					sprintf(buf,"%d%%",100*FaceCount/FACE_NUM);
					TextOut(hdc,145,225,buf);
                        	}
				else
				{
					sprintf(buf,"0%%");
					TextOut(hdc,145,225,buf);
				}
			}
                        EndPaint(hWnd,hdc);
		}
		return 0;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
                        if (gOptions.KeyPadBeep)
                                ExKeyBeep();

			if(LOWORD(wParam) == SCANCODE_ESCAPE)
				PostMessage (hWnd, MSG_CLOSE, 0, 0);
			break;

		case MSG_TIMER:
			if (wParam == IDC_FACE_TIMER1)
			{
				KillTimer(hWnd, IDC_FACE_TIMER1);
				if(ismenutimeout)
				{
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
					return 0;
				}
				hdc=GetClientDC(hWnd);
				if(FaceCount==0 && RegCnt==0 && RegBegin<= REG_PREPARE && gOptions.VoiceOn)
				{
					RegCnt=1;
					FaceRegState=FACE_REG_PREPARE;
					ShowFaceHint(hWnd , FACE_REG_PREPARE , hdc, &fvbg,1);
					if(gOptions.VoiceOn)
						ExPlayVoice(VOICE_FACE_START); //start...
					DelayMS(1000);
				}
				if(ShowCaptureFace( hWnd,HDC_SCREEN ,1)) //Extract face template ok
				{

					SetMenuTimeOut(time(NULL));
					ret=RegFaceTmpProc(f_pin);

					if(RET_SUCCESS==ret) //register success
					{
						SendMessage (hWnd, MSG_PAINT, FACE_NUM, 0) ;
						regok=1;
						if(gOptions.VoiceOn)
                                			ExPlayVoice(VOICE_THANK); 
						DelayMS(500);
						PostMessage (hWnd, MSG_CLOSE, 0, 0);
					}
					else if(RET_FAILED==ret) // register failed
					{
						PostMessage (hWnd, MSG_CLOSE, 0, 0);
					}
					else if(RET_SAME==ret && RegBegin != REG_PREPARE) // move face
					{
						ShowFaceHint(hWnd , HINT_FCHG ,hdc, &fvbg,1);
					}
					else if(RET_CONTINUE==ret)
					{
						if(RegBegin != REG_PREPARE)
						{
							SendMessage (hWnd, MSG_PAINT , FaceCount, 0) ;
							if(FaceCount == 0)
							{
								FaceRegState=FACE_REG_FRONT;
								ShowFaceHint(hWnd , FACE_REG_FRONT , hdc, &fvbg,1);
								if(gOptions.VoiceOn)
                                					ExPlayVoice(VOICE_FACE_FRONT); 
								DelayMS(600);
							}
							else if(FaceCount == 3)
							{
								FaceRegState=FACE_REG_SCREEN;
								ShowFaceHint(hWnd , FACE_REG_SCREEN ,hdc, &fvbg,1);
								if(gOptions.VoiceOn)
                                					ExPlayVoice(VOICE_FACE_SCREEN); 
								DelayMS(600);
							}
							else if(FaceCount == 6)
							{
								FaceRegState=FACE_REG_LEFT;
								ShowFaceHint(hWnd , FACE_REG_LEFT ,hdc, &fvbg,1);
								if(gOptions.VoiceOn)
                                					ExPlayVoice(VOICE_FACE_LEFT); 
								DelayMS(600);
							}
							else if(FaceCount == 9)
							{
								FaceRegState=FACE_REG_RIGHT;
								ShowFaceHint(hWnd , FACE_REG_RIGHT ,hdc, &fvbg,1);
								if(gOptions.VoiceOn)
                                					ExPlayVoice(VOICE_FACE_RIGHT); 
								DelayMS(600);
							}
							else if(FaceCount == 12)
							{
								FaceRegState=FACE_REG_CAMERA;
								ShowFaceHint(hWnd ,FACE_REG_CAMERA ,hdc, &fvbg,1);
								if(gOptions.VoiceOn)
                                					ExPlayVoice(VOICE_FACE_CAMERA); 
								DelayMS(600);
							}
							else
							{
			                        		ExBeep(1);
							}
						}
						else
						{
			                        	ExBeep(1);
						}
					}
				}
				ReleaseDC(hdc);
				SetTimer(hWnd, IDC_FACE_TIMER1,IDC_TIMER_NUM );
			}
			break;

		case MSG_IDLE:
			if(FaceCount==0 && RegCnt==0 && RegBegin<= REG_PREPARE && gOptions.VoiceOn)
 		        {
                        	RegCnt=1;
				FaceRegState=FACE_REG_PREPARE;
				hdc=GetClientDC(hWnd);
				ShowFaceHint(hWnd , FACE_REG_PREPARE ,hdc, &fvbg,1);
				if(gOptions.VoiceOn)
                			ExPlayVoice(VOICE_FACE_START); //start...
				DelayMS(1000);
				ReleaseDC(hdc);
                        }
			break;

		case MSG_CLOSE:
			KillTimer(hWnd, IDC_FACE_TIMER1);


         		// MainWindowCleanup(hWnd);
                        DestroyMainWindow(hWnd);
		        return 0;
        
	}
    
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}
//flag=0: Enroll , flag=1: Verify
int CreateFaceEnrollWindow(HWND hWnd, int pin)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	if(0==InitFaceEnroll())
        	return 0;

	f_pin=pin;
	regok=0;

	SetMenuTimeOut(time(NULL));
	ismenutimeout=0;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE ;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "Face";
	CreateInfo.hMenu = 0;
//	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = FaceEnrollWinProc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
        CreateInfo.iBkColor = 0x00FFA2BE;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hWnd;
	
	LoadBitmap(HDC_SCREEN, &fvbg, GetBmpPath("mainmenu.jpg"));
	LoadBitmap(HDC_SCREEN, &fvbar, GetBmpPath("bar.bmp"));

        hMainWnd = CreateMainWindow(&CreateInfo);
        if (hMainWnd == HWND_INVALID)
	{
		UnloadBitmap(&fvbg);
		UnloadBitmap(&fvbar);
                return 0;
	}

        ShowWindow(hMainWnd, SW_SHOWNORMAL);
        while (GetMessage(&msg, hMainWnd))
        {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        MainWindowThreadCleanup(hMainWnd);
	UnloadBitmap(&fvbg);
	UnloadBitmap(&fvbar);
        return regok;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

