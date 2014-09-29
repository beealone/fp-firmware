#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#include "msg.h"
#include "sensor.h"
#include "commu.h"
#include "exfun.h"
#include "ssrcommon.h"
#include "main.h"
#include "arca.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

//BITMAP mfmngbmpbkg;
#define LB_MFPARAMETER       	2000
#define IDC_MFPARAMETER         2010
#define ID_SAVE			2030
#define ID_EXIT			2031

HWND MFPWnd[10];
int ctrlcnt=0, actctrlindex=0;
BYTE tpkey[6];

static void InitMFManageWindow(HWND hWnd)
{
	int i,r;
	char hstr[4];
	char* windowstr[7];
	int posX1,posX2,posX3,posX4;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=190;
		posX2=160;
		posX3=100;
		posX4=-35;
	}
	else
	{
		posX1=10;
		posX2=100;
		posX3=165;
		posX4=35;
	}

        windowstr[0]=LoadStrByID(MID_ONLY_PIN);
        windowstr[1]=LoadStrByID(HID_NO);
        windowstr[2]=LoadStrByID(HID_YES);
        windowstr[3]=LoadStrByID(MID_SAVE_LOCAL);
        windowstr[4]=LoadStrByID(MID_MFC_PWD);
        windowstr[5]=LoadStrByID(MID_SAVE);
        windowstr[6]=LoadStrByID(MID_EXIT);

	ctrlcnt=0;
	r=0;
	if(!gOptions.UserExtendFormat)
	{
	        CreateWindow(CTRL_STATIC,windowstr[0], WS_VISIBLE | SS_LEFT, LB_MFPARAMETER,posX1,(10+r*30),120,23,hWnd,0);
        	MFPWnd[ctrlcnt] = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
                	                IDC_MFPARAMETER,posX2,(6+r*30),60,23,hWnd,0);
		SendMessage(MFPWnd[ctrlcnt], CB_ADDSTRING, 0, (LPARAM)windowstr[1]);
		SendMessage(MFPWnd[ctrlcnt], CB_ADDSTRING, 0, (LPARAM)windowstr[2]);
		SendMessage(MFPWnd[ctrlcnt], CB_SETCURSEL, gOptions.OnlyPINCard, 0);
		ctrlcnt++;
		r++;
	}

        CreateWindow(CTRL_STATIC,windowstr[4], WS_VISIBLE | SS_LEFT, LB_MFPARAMETER+1,posX1,(10+r*30),120,23,hWnd,0);
        for(i=0;i<6;i++)
        {
		tpkey[i]=gOptions.CardKey[i];
		sprintf(hstr,"%3d",gOptions.CardKey[i]);
		printf("hstr:%x\n",gOptions.CardKey[i]);
                MFPWnd[i+ctrlcnt] = CreateWindow(CTRL_SLEDIT,hstr, WS_VISIBLE | ES_PASSWORD | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
                        IDC_MFPARAMETER+1+i, posX2+posX4*i, (6+r*30), 35, 23, hWnd, 0);
                SendMessage(MFPWnd[i+ctrlcnt], EM_LIMITTEXT, 3, 0L);
        }
	r++;
	ctrlcnt+=6;
#if 1
        CreateWindow(CTRL_STATIC,windowstr[3], WS_VISIBLE | SS_LEFT, LB_MFPARAMETER+2,10,(10+r*30),150,23,hWnd,0);
        MFPWnd[ctrlcnt] = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
                                IDC_MFPARAMETER+7,posX3,(6+r*30),60,23,hWnd,0);
        SendMessage(MFPWnd[ctrlcnt],CB_ADDSTRING,0,(LPARAM)windowstr[1]);
        SendMessage(MFPWnd[ctrlcnt],CB_ADDSTRING,0,(LPARAM)windowstr[2]);
	SendMessage(MFPWnd[ctrlcnt], CB_SETCURSEL, gOptions.MustEnroll, 0);
	ctrlcnt++;
#endif

        MFPWnd[ctrlcnt++] = CreateWindow(CTRL_BUTTON,windowstr[5],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_SAVE,10,190,85,23,hWnd,0);
        //MFPWnd[ctrlcnt++] = CreateWindow(CTRL_BUTTON,windowstr[6],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_EXIT,220,190,85,23,hWnd,0);
        MFPWnd[ctrlcnt++] = CreateWindow(CTRL_BUTTON,windowstr[6],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_EXIT,225+gOptions.ControlOffset,190,85,23,hWnd,0);
}

static void SaveMFParameter(void)
{
	int i;
	char tsr[4];//, ms[4];
	if(ctrlcnt==10)
	{
		gOptions.OnlyPINCard=SendMessage(MFPWnd[0], CB_GETCURSEL, 0,0);
		for(i=0;i<6;i++)
		{
			memset(tsr,0,4);
			GetWindowText(MFPWnd[i+1],tsr,3);
			if(!strlen(tsr))
				gOptions.CardKey[i]='\0';
			else
				gOptions.CardKey[i]=(BYTE)atoi(tsr);
		}
		gOptions.MustEnroll=SendMessage(MFPWnd[7], CB_GETCURSEL,0,0);
	}
	else if(ctrlcnt==9)
	{
                for(i=0;i<6;i++)
                {
                        memset(tsr,0,4);
                        GetWindowText(MFPWnd[i],tsr,3);
                        if(!strlen(tsr))
                                gOptions.CardKey[i]='\0';
                        else
                                gOptions.CardKey[i]=(BYTE)atoi(tsr);
                }
                gOptions.MustEnroll=SendMessage(MFPWnd[6], CB_GETCURSEL,0,0);

	}
	SaveOptions(&gOptions);
	LoadOptions(&gOptions);
}

static int isModified(void)
{
	int i;
	char tsr[4];

	if(ctrlcnt==10)
	{
		if(gOptions.OnlyPINCard != SendMessage(MFPWnd[0], CB_GETCURSEL, 0,0))
			return 1;

		for(i=0;i<6;i++)
		{
			memset(tsr,0,4);
			GetWindowText(MFPWnd[i+1], tsr, 3);
			if(atoi(tsr)!=tpkey[i])
				return 1;
		}

		if(gOptions.MustEnroll != SendMessage(MFPWnd[7], CB_GETCURSEL, 0,0))
			return 1;
	}
	else if(ctrlcnt==9)
	{
                for(i=0;i<6;i++)
                {
                        memset(tsr,0,4);
                        GetWindowText(MFPWnd[i], tsr, 3);
                        if(atoi(tsr)!=tpkey[i])
                                return 1;
                }

		if(gOptions.MustEnroll!=SendMessage(MFPWnd[6], CB_GETCURSEL,0,0))
			return 1;
	}

	return 0;
}

static void SelectNext(WPARAM wParam)
{
	if(wParam==SCANCODE_CURSORBLOCKUP)
	{
		if(--actctrlindex<0) actctrlindex=ctrlcnt-1;
	}
	if(wParam==SCANCODE_CURSORBLOCKDOWN)
	{
		if(++actctrlindex>ctrlcnt-1) actctrlindex=0;
	}
	if(wParam==SCANCODE_CURSORBLOCKLEFT)
		actctrlindex--;
	if(wParam==SCANCODE_CURSORBLOCKRIGHT)
		actctrlindex++;

	SetFocusChild(MFPWnd[actctrlindex]);

}

static int mfmanagewinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//char tmpstr[50];
	int nc, id;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
	                //LoadBitmap (HDC_SCREEN, &mfmngbmpbkg, GetBmpPath("submenubg.jpg"));
			InitMFManageWindow(hWnd);
			UpdateWindow(hWnd,TRUE);
			actctrlindex=0;
			SetFocusChild(MFPWnd[actctrlindex]);
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

			FillBoxWithBitmap(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight,get_submenubg_jgp());
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
			if(gOptions.KeyPadBeep)
				ExKeyBeep();

			if(wParam==SCANCODE_ESCAPE)
				PostMessage(hWnd, MSG_CLOSE, 0, 0);

			if(wParam==SCANCODE_CURSORBLOCKLEFT || wParam==SCANCODE_CURSORBLOCKRIGHT || (gOptions.TFTKeyLayout==3 && wParam==SCANCODE_BACKSPACE))
			{
				if(ctrlcnt==10)		//
				{
					if(actctrlindex==0 || actctrlindex==7)
					{
						if(SendMessage(MFPWnd[actctrlindex], CB_GETCURSEL,0,0)==0)
							SendMessage(MFPWnd[actctrlindex], CB_SETCURSEL,1,0);
						else
							SendMessage(MFPWnd[actctrlindex], CB_SETCURSEL,0,0);
					}
					else if(actctrlindex==8)
					{
						actctrlindex=9;
						SetFocusChild(MFPWnd[actctrlindex]);
					}
					else if(actctrlindex==9)
					{
						actctrlindex=8;
						SetFocusChild(MFPWnd[actctrlindex]);
					}
					else
						SelectNext(wParam);
				}
				else if(ctrlcnt==9)	//
				{
                                        if(actctrlindex==6)
                                        {
                                                if(SendMessage(MFPWnd[actctrlindex], CB_GETCURSEL,0,0)==0)
                                                        SendMessage(MFPWnd[actctrlindex], CB_SETCURSEL,1,0);
                                                else
                                                        SendMessage(MFPWnd[actctrlindex], CB_SETCURSEL,0,0);
                                        }
                                        else if(actctrlindex==7)
                                        {
                                                actctrlindex=8;
                                                SetFocusChild(MFPWnd[actctrlindex]);
                                        }
                                        else if(actctrlindex==8)
                                        {
                                                actctrlindex=7;
                                                SetFocusChild(MFPWnd[actctrlindex]);
                                        }
                                        else
                                                SelectNext(wParam);
				}
				return 0;
			}

			if(wParam==SCANCODE_CURSORBLOCKUP || wParam==SCANCODE_CURSORBLOCKDOWN)
			{
				SelectNext(wParam);
 				return 0;
			}

			if((wParam==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10))
			{
				if(actctrlindex<ctrlcnt-2)
					PostMessage(hWnd, MSG_COMMAND, ID_SAVE,0L);
			}

			if(LOWORD(wParam)==SCANCODE_F10)
			{
                                if(actctrlindex!=ctrlcnt-1)
                                        PostMessage(hWnd, MSG_COMMAND, ID_SAVE,0L);
                                else
                                        PostMessage(hWnd, MSG_COMMAND, ID_EXIT,0L);
			}

			if(wParam==SCANCODE_MENU)
				PostMessage(hWnd, MSG_COMMAND, ID_SAVE, 0L);
			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(id==ID_SAVE || id==ID_EXIT)
			{
				if(isModified() && MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME), MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
					SaveMFParameter();
				if(!ismenutimeout) PostMessage(hWnd, MSG_CLOSE,0,0);
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&mfmngbmpbkg);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}


int CreateMFManageWindow(HWND hOwner)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hOwner = GetMainWindowHandle (hOwner);
        CreateInfo.dwStyle = WS_VISIBLE| WS_CAPTION|WS_BORDER;
        CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption=LoadStrByID(MID_CARD_OP5);
        CreateInfo.hMenu = 0;
        //CreateInfo.hCursor = GetSystemCursor(0);
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = mfmanagewinproc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
        CreateInfo.iBkColor = COLOR_lightwhite;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hOwner;

        hMainWnd = CreateMainWindow(&CreateInfo);
        if (hMainWnd == HWND_INVALID)
                return -1;
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

