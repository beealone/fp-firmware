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

#define LB_TITLE 99
#define LB_ACNO 100
#define ID_ACNO 101

static int pwdpin=0;
HWND editacno5;
static int pwdret=-1;
static int procverifypwd(HWND hWnd)
{
	TUser myuser;
	char mypwdchar[8];
	//HDC mydc;
	
	memset(&myuser,0,sizeof(TUser));
	memset(mypwdchar,0,8);
	GetWindowText(editacno5,mypwdchar,PWDLIMITLEN);

	myuser.PIN=pwdpin;
	FDB_GetUser(pwdpin,&myuser);
	if(myuser.Password[0]&&nstrcmp(mypwdchar, myuser.Password, 8)==0)
	{
		pwdret = 1;
		SendMessage (hWnd, MSG_CLOSE, 0, 0L);

	}
	else
	{
		//mydc=GetClientDC(hWnd);
		//SetBkMode(mydc,BM_TRANSPARENT);
		//SetTextColor(mydc,PIXEL_lightwhite);		
		//TextOut(mydc,60,70,LoadStrByID(HID_INPUTPWD));
                //myMessageBox(hWnd,MB_OK | MB_ICONSTOP,"SSR",LoadStrByID(HID_VPFAIL));
		//ReleaseDC(mydc);
                //SetFocusChild(editacno5);
		pwdret=0;
		printf("pwd faile\n");
		SendMessage (hWnd, MSG_CLOSE, 0, 0L);
	}

	return pwdret;

}




static void InitWindow (HWND hWnd)
{
	int posX1,posX2;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=260;
		posX2=70;
	}
	else
	{
		posX1=10;
		posX2=70;
	}

			CreateWindow (CTRL_STATIC, LoadStrByID(MID_PWD),
                        	  WS_VISIBLE | SS_CENTER|WS_BORDER|WS_CHILD,
	                          LB_ACNO, 10, 30, 50, 23, hWnd, 0);
        	        editacno5 = CreateWindow (CTRL_SLEDIT, "",
                          WS_VISIBLE | WS_BORDER|ES_PASSWORD,
                          ID_ACNO, 70, 30, 180, 23, hWnd, 0);


			SendMessage(editacno5,EM_LIMITTEXT,PWDLIMITLEN,0L);

}

static int Verifypwdwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int tmpvalue = 0;
	static char keyupFlag=0;
	switch (message)	
	{
		case MSG_CREATE:

			InitWindow(hWnd);
			SetFocusChild(editacno5);			
			return 0;
		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
		        tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
		        SetTextColor(hdc,PIXEL_lightwhite);		
			TextOut(hdc,60,70,LoadStrByID(HID_INPUTPWD));			     EndPaint(hWnd,hdc);
			return 0;
		case MSG_KEYUP:
			if(3==gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			if(3==gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
			if ((GetFocusChild(hWnd)==editacno5)&&((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10)))
				procverifypwd(hWnd);		
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))		
			{
				        SendMessage (hWnd, MSG_CLOSE, 0, 0L);
					pwdret=-1;
			}
		

			break;		
                case MSG_DESTROY:
                    DestroyAllControls (hWnd);
                    return 0;


		case MSG_CLOSE:
			DestroyMainWindow(hWnd);
			MainWindowThreadCleanup(hWnd);

			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateVerifyPWDWindow(HWND hOwner,int verpin)
{
	MSG msg;
	MAINWINCREATE CreateInfo;
	HWND hMainWnd3;

/*
    if (hMainWnd3 != HWND_INVALID) {
        ShowWindow (hMainWnd3, SW_SHOWNORMAL);
        return;
    }
*/

	pwdpin = verpin;
	hOwner = GetMainWindowHandle (hOwner);
	//CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(HID_SVERIFY);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = Verifypwdwinproc;
	CreateInfo.lx = 30;
	CreateInfo.ty = 30;
	//CreateInfo.rx = g_rcScr.right;
        //CreateInfo.by = g_rcScr.bottom;
	CreateInfo.rx = 290;
        CreateInfo.by = 150;
	CreateInfo.iBkColor = COLOR_lightgray;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;



	hMainWnd3 = CreateMainWindow(&CreateInfo);

	if (hMainWnd3 == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd3, SW_SHOWNORMAL);
	
	while (GetMessage(&msg,hMainWnd3))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hMainWnd3);

printf("pwdret:%d\n",pwdret);	
	return pwdret;

}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

