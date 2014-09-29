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
#include "flashdb.h"
#include "commu.h"
#include <minigui/tftmullan.h>
#include "ssrcommon.h"

#define LB_TITLE 50
#define LB_ACNO 65
#define ID_ACNO 66

char *propin;
static 	HWND hMainWnd1=HWND_INVALID;
HWND editacno3,Edtmp,focuswnd;
HWND temptemp;
extern HWND inputhwnd;
extern int myMessageBox1 (HWND hwnd, DWORD dwStyle, char* title, char* text, ...);

static int procverifychoose(HWND hWnd)
{
	TUser myuser;
	char myacchar[24];
	
	memset(&myuser,0,sizeof(TUser));
	memset(myacchar,0,24);
	GetWindowText(editacno3,myacchar,PINLIMITLEN);
	if (strlen(myacchar))
		memcpy(myuser.PIN2,myacchar,strlen(myacchar));
	if ((strlen(myacchar)==0)||(NULL==FDB_GetUserByCharPIN2(myacchar,&myuser)))
        {
                myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,"SSR",LoadStrByID(HID_NOTENROLLED));
                SetFocusChild(editacno3);
                return 0;
        }
	SendMessage(temptemp, MSG_TYPE_CMD, News_VerifiedByPIN, myuser.PIN);
	PostMessage (hWnd, MSG_CLOSE, 0, 0L);

	return 1;

}




static void InitWindow (HWND hWnd)
{
	int posX1,posX2;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=280;
		posX2=100;
	}
	else
	{
		posX1=10;
		posX2=70;
	}
	CreateWindow (CTRL_STATIC, LoadStrByID(MID_ACNO),
				  WS_VISIBLE | SS_CENTER|WS_BORDER|WS_CHILD,
				  LB_ACNO, posX1, 30, 50, 23, hWnd, 0);
    editacno3 = CreateWindow (CTRL_SLEDIT, "",
                WS_VISIBLE | WS_BORDER,
                ID_ACNO, posX2, 30, 180, 23, hWnd, 0);
	SendMessage(editacno3,EM_LIMITTEXT,PINLIMITLEN,0L);
}

static int Verifywinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc, tmpvalue = 0;
	static char keyupFlag=0;
	switch (message)	
	{
		case MSG_CREATE:

			InitWindow(hWnd);
			SetFocusChild(editacno3);			
			SetWindowText(editacno3,propin);
			SendMessage(editacno3,EM_SETCARETPOS,0,1);
			return 0;
		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
		    tmpvalue= SetBkMode(hdc,BM_TRANSPARENT);
		    SetTextColor(hdc,PIXEL_lightwhite);		
			TextOut(hdc,60,70,LoadStrByID(HID_PLACEFINGER));			     
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
			if ((GetFocusChild(hWnd)==editacno3)&&((LOWORD(wParam)==SCANCODE_ENTER)
				||(LOWORD(wParam)==SCANCODE_F10)))
				procverifychoose(hWnd);		
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))		
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
			break;		
	    case MSG_DESTROY:
        	DestroyAllControls (hWnd);
	        hMainWnd1 = HWND_INVALID;
        	return 0;
		case MSG_CLOSE:
			DestroyMainWindow(hWnd);
			MainWindowThreadCleanup (hWnd);
			return 0;
		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if (id==ID_ACNO)
			{
				if (nc==EN_CHANGE)
				{
					memset(propin,0,24);
                    GetWindowText (editacno3, propin, 24);
					printf("keybuffer:%s\n",propin);
				}
			}
			break;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateVerifyModeWindow(HWND hOwner,char *keyvalue)
{
	MAINWINCREATE CreateInfo;
printf("%s\n",__FUNCTION__);
    if (hMainWnd1 != HWND_INVALID) {
        ShowWindow (hMainWnd1, SW_SHOWNORMAL);
        return 0;
    }

	propin = keyvalue;
	//hOwner = GetMainWindowHandle (hOwner);
	//CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(HID_SVERIFY);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = Verifywinproc;
	CreateInfo.lx = 30;
	CreateInfo.ty = 30;
	//CreateInfo.rx = g_rcScr.right;
        //CreateInfo.by = g_rcScr.bottom;
	CreateInfo.rx = 290;
    CreateInfo.by = 150;
	CreateInfo.iBkColor = COLOR_lightgray;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;

	temptemp=hOwner;

	hMainWnd1 = CreateMainWindow(&CreateInfo);
	//inputhwnd=hMainWnd1;
/*
	hMainWnd1 = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd1, SW_SHOWNORMAL);
	
	while (GetMessage(&msg,hMainWnd1))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hMainWnd1);

	if (fpv)
		return 1;
	if (pwdv)
		return 2;

	return 0;
*/
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

