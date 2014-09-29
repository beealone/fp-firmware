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
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"
#include "ssrcommon.h"

#define LB_PWD 50
#define ID_PWD 51
#define LB2_PWD 52
#define ID2_PWD 53
#define ID_SAVE 63
#define ID_EXIT 64

static HWND EdPwd,Edtmp,btnsave1,btnexit1,focuswnd,EdPwd1;
char tmpcharpwd[10];
char tmpchar2[10];
char *respwd;
//BITMAP pwdmngbmp;

static void Initnewinfo(void)
{
	memset(tmpcharpwd,0,10);
	memset(tmpchar2,0,10);
}

static int ProcessPwd(HWND hWnd)
{
	int ret=0;
	
	GetWindowText(EdPwd1,tmpcharpwd,PWDLIMITLEN);
	if(strlen(tmpcharpwd)<=0 )
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_EMPTYPWD));
		if(!ismenutimeout)
			SetFocusChild(EdPwd1);
		 return ret;
	}

	GetWindowText(EdPwd,tmpchar2,PWDLIMITLEN);
	if(nstrcmp(tmpcharpwd,tmpchar2,PWDLIMITLEN))
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_NOTSAMEPWD));
		if(!ismenutimeout)
		{
			Initnewinfo();
			SetFocusChild(EdPwd);
		}
		return ret;
	}

	memset(respwd,0,10);
	nstrcpy(respwd,tmpcharpwd,PWDLIMITLEN);
	return 1;
}

static void SelectNext(HWND hWnd,WPARAM wParam)
{
	Edtmp = GetFocusChild(hWnd);
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == EdPwd1)
			focuswnd = EdPwd;
		else if (Edtmp == EdPwd)
			focuswnd = btnsave1;
		else if (Edtmp == btnsave1 && LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			focuswnd = btnexit1;
		else if (Edtmp == btnexit1 && LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			focuswnd = EdPwd1;			
		SetFocusChild(focuswnd);
	}
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == EdPwd1)
			focuswnd = btnexit1;
		else if (Edtmp == EdPwd)
			focuswnd = EdPwd1;
		else if (Edtmp == btnsave1)
			focuswnd = EdPwd;
		else if (Edtmp == btnexit1)
			focuswnd = btnsave1;			
		SetFocusChild(focuswnd);
	}
}

static void InitWindow (HWND hWnd)
{
	
	char pwdstr[100];
	char pwdstr2[100];
	int posX1,posX2;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=20;
		posX2=40;
	}
	else
	{
		posX1=20;
		posX2=60;
	}

	sprintf(pwdstr2, LoadStrByID(MID_PWDLEN), PWDLIMITLEN);
	sprintf(pwdstr, "%s(%s)", LoadStrByID(HID_INPUTPWD),pwdstr2);
        CreateWindow (CTRL_STATIC, pwdstr, WS_VISIBLE | SS_CENTER, LB_PWD, posX1, 20,290, 23, hWnd, 0);
        EdPwd1 = CreateWindow (CTRL_SLEDIT, tmpcharpwd, WS_VISIBLE | WS_BORDER|ES_PASSWORD, ID_PWD, posX2, 60, 180, 23, hWnd, 0);

	sprintf(pwdstr,"%s(%s)",LoadStrByID(HID_VERINPUTPWD),pwdstr2);
        CreateWindow (CTRL_STATIC, pwdstr, WS_VISIBLE | SS_CENTER, LB2_PWD, posX1, 100, 290, 23, hWnd, 0);

	EdPwd=CreateWindow (CTRL_SLEDIT, "", WS_VISIBLE | WS_BORDER|ES_PASSWORD, ID2_PWD, posX2, 140, 180, 23, hWnd, 0);

	btnsave1 = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_SAVE), WS_VISIBLE | BS_DEFPUSHBUTTON|WS_BORDER,
			  ID_SAVE,10,190,85,23,hWnd,0);
	btnexit1 = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_EXIT), WS_VISIBLE | BS_DEFPUSHBUTTON|WS_BORDER,
			  ID_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);

	SendMessage(EdPwd1,EM_LIMITTEXT,PWDLIMITLEN,0L);
	SendMessage(EdPwd,EM_LIMITTEXT,PWDLIMITLEN,0L);
}

static int Pwdwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	static char keyupFlag=0;
	switch (message)	
	{
		case MSG_CREATE:
                        //LoadBitmap(HDC_SCREEN,&pwdmngbmp,GetBmpPath("submenubg.jpg"));

			InitWindow(hWnd);
			UpdateWindow(hWnd,TRUE);			
			SetFocusChild(EdPwd1);
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
			if(3==gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3==gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
                        if (gOptions.KeyPadBeep) ExKeyBeep();

			SelectNext(hWnd,wParam);
			if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
			{
				if (Edtmp == EdPwd1)
					SetFocusChild(EdPwd);
				else if (Edtmp == EdPwd)
					SetFocusChild(btnsave1);
				else if (Edtmp == btnsave1)
					SendMessage(hWnd,MSG_COMMAND,ID_SAVE,0);
				else if (Edtmp == btnexit1)
					SendMessage(hWnd,MSG_COMMAND,ID_EXIT,0);
				return 0;
			}

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			        PostMessage(hWnd, MSG_CLOSE, 0, 0L);

			if ((LOWORD(wParam)==SCANCODE_MENU))		
				PostMessage(hWnd,MSG_COMMAND,ID_SAVE,0);
			break;			

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
		        switch (id) 
			{
			        case ID_SAVE:
					if(ProcessPwd(hWnd))
				        	PostMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;
				case ID_EXIT:
				        PostMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;
			}
			break;

                case MSG_CLOSE:
                        //UnloadBitmap(&pwdmngbmp);
                        //MainWindowCleanup (hWnd);
                        DestroyMainWindow(hWnd);
                        //PostQuitMessage(hWnd);
                        return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreatePwdWindow(HWND hOwner,char *pwd, int isnewreg)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	Initnewinfo();
	respwd = pwd;	

	hOwner = GetMainWindowHandle (hOwner);

	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	if (isnewreg)
		CreateInfo.spCaption = LoadStrByID(MID_DATA_EU_PWD);
	else
		CreateInfo.spCaption = LoadStrByID(HID_CHGPWD);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = Pwdwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
	//CreateInfo.iBkColor = COLOR_lightgray;
	CreateInfo.iBkColor = 0x00FFA2BE;
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

	if (strlen(tmpcharpwd)) {
		return 1;
	} else {
		return 0;
	}

}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

