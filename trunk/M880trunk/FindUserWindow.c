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
#include "t9.h"
#include <minigui/tftmullan.h>

#define LB_ACNO 50
#define ID_FINDACNO 51
#define LB_FINDNAME 52
#define ID_FINDNAME 53
#define ID_SAVE 63
#define ID_EXIT 64

HWND Edacno1,Edtmp,btnsavef,btnexitf,focuswnd,EdName1;

char *resacno;
//BITMAP findbmp;
BITMAP findhintbmp;
extern HWND hIMEWnd;
int FindResult;

int myMessageBox1 (HWND hwnd, DWORD dwStyle, char* title, char* text, ...);

static int ProcessFind(HWND hWnd)
{
	char resname[24];

	int namelen,acnolen;
	int exitpass;

	memset(resname,0,24);
	exitpass = 0;
	GetWindowText(Edacno1,resacno,PINLIMITLEN);
	GetWindowText(EdName1,resname,PINLIMITLEN);
	acnolen = strlen(resacno);
	namelen = strlen(resname);

	GetUsermestolb_NameOrAcno(resname, resacno, &exitpass);
/*
	if (acnolen > 0)
	{
		if (namelen > 0)
		{
			if (FDB_GetUserByNameAndAcno(resname,resacno,NULL))
				exitpass=1;
			else
				memset(resacno,0,24);
		}
		else
		{
			if (FDB_GetUserByCharPIN2(resacno,NULL))
				exitpass=1;
			else
				memset(resacno,0,24);
		}
	}
	else
	{
		if (namelen > 0)
		{
			TUser resfinduser;
			memset(&resfinduser,0,sizeof(TUser));
			if (FDB_GetUserByName(resname,&resfinduser))
			{
				memset(resacno,0,24);
				strncpy(resacno,resfinduser.PIN2,strlen(resfinduser.PIN2));
				exitpass=1;
			}
		}
		else
			exitpass=1;

	}
*/
	if (exitpass) {
		FindResult = 1;
		SendMessage (hWnd, MSG_CLOSE, 0, 0L);
	} else {
		FindResult = 0;
		myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(HID_NOTENROLLED));
	}
	return 1;

}



static void SelectNext(HWND hWnd,WPARAM wParam)
{
	Edtmp  = GetFocusChild(hWnd);
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == Edacno1)
		{
			if(gOptions.IMEFunOn==1&& gOptions.TFTKeyLayout!=3)
				focuswnd = EdName1;
			else
				focuswnd = btnsavef;
		}

		else if (Edtmp==EdName1)
		{
			if(hIMEWnd != HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
				hIMEWnd=HWND_INVALID;
			}
			focuswnd = btnsavef;
		}
		else if (Edtmp == btnsavef && LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			focuswnd = btnexitf;
		else if (Edtmp == btnexitf && LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			focuswnd = Edacno1;


		SetFocusChild(focuswnd);
	}
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == Edacno1)
			focuswnd = btnexitf;
		else if (Edtmp==EdName1)
		{
			if(hIMEWnd != HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd=HWND_INVALID;
			}
			focuswnd = Edacno1;
		}
		else if (Edtmp == btnsavef)
		{
			if(gOptions.IMEFunOn==1&& gOptions.TFTKeyLayout!=3)
				focuswnd = EdName1;
			else
				focuswnd = Edacno1;
		}
		else if (Edtmp == btnexitf)
			focuswnd = btnsavef;

		SetFocusChild(focuswnd);
	}
}

static void InitWindow (HWND hWnd)
{
	int posX1,posX2;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=120+gOptions.ControlOffset;
		posX2=80+gOptions.ControlOffset;
	}
	else
	{
		posX1=20+gOptions.ControlOffset;
		posX2=60+gOptions.ControlOffset;
	}
	CreateWindow (CTRL_STATIC, LoadStrByID(MID_ACNO), WS_VISIBLE | SS_CENTER, LB_ACNO, posX1, 70, 180, 23, hWnd, 0);
	Edacno1 = CreateWindow (CTRL_SLEDIT, "", WS_VISIBLE | WS_BORDER, ID_FINDACNO, posX2, 90, 180, 23, hWnd, 0);

	if(gOptions.IMEFunOn==1&& gOptions.TFTKeyLayout!=3)
	{
		CreateWindow (CTRL_STATIC, LoadStrByID(MID_NAME), WS_VISIBLE | SS_CENTER, LB_FINDNAME, posX1, 120, 180, 23, hWnd, 0);
		EdName1 = CreateWindow (CTRL_SLEDIT, "", WS_VISIBLE | WS_BORDER, ID_FINDNAME, posX2, 140, 180, 23, hWnd, 0);
	}

	btnsavef = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_SAVE), WS_VISIBLE | BS_DEFPUSHBUTTON|WS_BORDER, ID_SAVE,10,190,85,23,hWnd,0);
	btnexitf = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_EXIT), WS_VISIBLE | BS_DEFPUSHBUTTON|WS_BORDER, ID_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);
	SendMessage(Edacno1,EM_LIMITTEXT,gOptions.PIN2Width,0L);
}

static int Findwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;

	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//LoadBitmap(HDC_SCREEN,&findbmp,GetBmpPath("submenubg.jpg"));
			LoadBitmap(HDC_SCREEN,&findhintbmp,GetBmpPath("finduser.gif"));

			InitWindow(hWnd);
			UpdateWindow(hWnd,TRUE);
			SetFocusChild(Edacno1);
			break;

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
				if(1 == keyupFlag)
					keyupFlag=0;
				else
					break;
			}
			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			if(LOWORD(wParam)==SCANCODE_F9 ||
					((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && (LOWORD(wParam)==SCANCODE_F11)))
			{
				if((GetFocusChild(hWnd)==EdName1) && (gOptions.IMEFunOn==1&& gOptions.TFTKeyLayout!=3))
					T9IMEWindow(hWnd,0,200, gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
			}

			SelectNext(hWnd,wParam);
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
				PostMessage(hWnd, MSG_CLOSE, 0, 0L);

			if ((LOWORD(wParam)==SCANCODE_ENTER)|| (LOWORD(wParam)==SCANCODE_MENU) || (LOWORD(wParam)==SCANCODE_F10))
			{
				if((GetFocusChild(hWnd)==Edacno1) || (GetFocusChild(hWnd)==EdName1))
					ProcessFind(hWnd);
			}
			break;

		case MSG_CLOSE:
			if(hIMEWnd!=HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd=HWND_INVALID;
			}
			//UnloadBitmap(&findbmp);

			DestroyMainWindow(hWnd);
			return 0;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			switch (id) {
				case ID_SAVE:
					ProcessFind(hWnd);
					break;
				case ID_EXIT:
					PostMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;
			}
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
				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
				FillBoxWithBitmap (hdc, 200+gOptions.ControlOffset, 10, 0, 0, &findhintbmp);

				if (fGetDC)
					ReleaseDC (hdc);
				return 0;
			}

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateFindUserWindow(HWND hOwner,char *acno)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	resacno = acno;
	FindResult = 0;

	hOwner = GetMainWindowHandle (hOwner);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_QUERYUSER);
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = Findwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0xFF00A2BE;
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

	if (FindResult == 1) {
		return 1;
	} else {
		return 0;
	}

}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

