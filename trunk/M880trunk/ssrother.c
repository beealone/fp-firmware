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
#include "exfun.h"
#include "main.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

#define OT_STATIC 	0x70
#define OT_ANTI		0x71
#define OT_SAVE		0x90
#define OT_EXIT		0x91

HWND EdOTSet[4];
int curOTFocus=0;
//BITMAP otset_bkg;
int oldset[2];

static void InitWindow (HWND hWnd)
{
	char* OTStr[11];
	int posX1,posX2;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=220+gOptions.ControlOffset;
		posX2=80+gOptions.ControlOffset;
	}
	else
	{
		posX1=10+gOptions.ControlOffset;
		posX2=100+gOptions.ControlOffset;
	}
	OTStr[0]=LoadStrByID(MID_ANTI_WAY);             //反潜方向
	OTStr[1]=LoadStrByID(MID_ANTI_NONE);            //不反潜
	
	OTStr[2]=LoadStrByID(MID_ANTI_OUT);             //出反潜
	OTStr[3]=LoadStrByID(MID_ANTI_IN);              //入反潜
	//OTStr[2]=LoadStrByID(MID_ANTI_IN);             //出反潜
	//OTStr[3]=LoadStrByID(MID_ANTI_OUT);              //入反潜

	OTStr[4]=LoadStrByID(MID_ANTI_LOCAL);           //本机状态
	OTStr[5]=LoadStrByID(MID_CTRL_OUT);             //控制出门
	OTStr[6]=LoadStrByID(MID_CTRL_IN);              //控制入门
	OTStr[7]=LoadStrByID(HIT_NULLKEY);                      //无
	OTStr[8]=LoadStrByID(MID_SAVE);
	OTStr[9]=LoadStrByID(MID_EXIT);
	OTStr[10]=LoadStrByID(MID_WG_INOUT);		//出入反潜

	//Set parameter value
	oldset[0]=gOptions.AntiPassbackOn;		//0:不反潜;1:出反潜,必须有入才能出;2:入反潜,必须有出才能入;3:出入反潜
	if(gOptions.MasterState>=0)
		oldset[1]=gOptions.MasterState;		//主从机通讯中主机的状态
	//若主机为0,则从机为1,若主机为1则从机为0,若为其他则主从都为主机当前的状态gOptions.AttState
	else
		oldset[1]=2;

	//反潜方向
	CreateWindow(CTRL_STATIC, OTStr[0], WS_VISIBLE | SS_LEFT, OT_STATIC, posX1, 10, 90, 23, hWnd, 0);
	EdOTSet[0]=CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
			OT_ANTI, posX2, 6, 120, 23, hWnd, 0);
	SendMessage(EdOTSet[0], CB_ADDSTRING, 0, (LPARAM)OTStr[1]);               //
	SendMessage(EdOTSet[0], CB_ADDSTRING, 0, (LPARAM)OTStr[2]);               //
	SendMessage(EdOTSet[0], CB_ADDSTRING, 0, (LPARAM)OTStr[3]);               //
	SendMessage(EdOTSet[0], CB_ADDSTRING, 0, (LPARAM)OTStr[10]);              //

	//本机状态
	CreateWindow(CTRL_STATIC, OTStr[4], WS_VISIBLE | SS_LEFT, OT_STATIC+1, posX1, 40, 90, 23, hWnd, 0);
	EdOTSet[1]=CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
			OT_ANTI+1, posX2, 36, 120, 23, hWnd, 0);
	SendMessage(EdOTSet[1], CB_ADDSTRING, 0, (LPARAM)OTStr[5]);               //
	SendMessage(EdOTSet[1], CB_ADDSTRING, 0, (LPARAM)OTStr[6]);               //
	SendMessage(EdOTSet[1], CB_ADDSTRING, 0, (LPARAM)OTStr[7]);               //

	EdOTSet[2]=CreateWindow(CTRL_BUTTON, OTStr[8], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, OT_SAVE, 10, 190, 90, 23, hWnd, 0);
	EdOTSet[3]=CreateWindow(CTRL_BUTTON, OTStr[9], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, OT_EXIT, 220+gOptions.ControlOffset, 190, 90, 23, hWnd, 0);

}

static void InitWindowText(HWND hWnd)
{
	int i;
	for(i=0;i<2;i++)
		SendMessage(EdOTSet[i], CB_SETCURSEL, oldset[i], 0);
}

static int ismodified(void)
{
	int i;
	for(i=0;i<2;i++)
	{
		if(oldset[i]!=SendMessage(EdOTSet[i], CB_GETCURSEL, 0, 0))
			return 1;
	}
	return 0;
}

static void SaveOTParameter(void)
{
	int tmpvalue;

	gOptions.AntiPassbackOn=SendMessage(EdOTSet[0], CB_GETCURSEL, 0, 0);
	tmpvalue=SendMessage(EdOTSet[1], CB_GETCURSEL, 0, 0);
	gOptions.MasterState=(tmpvalue<2) ? tmpvalue : -1;
	SaveOptions(&gOptions);
	sync();
}

static int otparawinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &otset_bkg, GetBmpPath("submenubg.jpg")))
			//        return 0;

			InitWindow(hWnd);		//add controls
			InitWindowText(hWnd);
			UpdateWindow(hWnd,TRUE);
			curOTFocus=0;
			SetFocusChild(EdOTSet[curOTFocus]);
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

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
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
			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
				PostMessage(hWnd, MSG_COMMAND, OT_EXIT, 0L);

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
				{
					if(++curOTFocus>3) curOTFocus=0;
				}
				else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
				{
					if(--curOTFocus<0) curOTFocus=3;
				}
				SetFocusChild(EdOTSet[curOTFocus]);
				return 0;	
			}
			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				int tmpi;
				int maxrang[2]= {3,2};

				if(curOTFocus<2)
				{
					tmpi=SendMessage(EdOTSet[curOTFocus], CB_GETCURSEL, 0, 0);
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
					{
						if(--tmpi<0) tmpi=maxrang[curOTFocus];
					}
					else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						if(++tmpi>maxrang[curOTFocus]) tmpi=0;
					}
					SendMessage(EdOTSet[curOTFocus], CB_SETCURSEL, tmpi, 0);
				}
				else
				{
					if(curOTFocus==2)
						curOTFocus=3;
					else
						curOTFocus=2;
					SetFocusChild(EdOTSet[curOTFocus]);
				}
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_ENTER)
			{
				if(curOTFocus<2)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)OT_SAVE,0);
			}

			if(LOWORD(wParam)==SCANCODE_F10)
			{
				if(curOTFocus<3)
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)OT_SAVE, 0);
				else
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)OT_EXIT, 0);
			}

			if(LOWORD(wParam)==SCANCODE_MENU)
			{
				PostMessage (hWnd, MSG_COMMAND, (WPARAM)OT_SAVE, 0L);
			}
			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);

			switch(id)
			{
				case OT_SAVE:
					if(ismodified()) SaveOTParameter();
					MessageBox1(hWnd, LoadStrByID(HIT_RIGHT) ,LoadStrByID( HIT_RUN),MB_OK | MB_ICONINFORMATION);	
					if(!ismenutimeout) PostMessage(hWnd,MSG_CLOSE,0,0);
					break;

				case OT_EXIT:				
					if(ismodified() && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
								MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
						PostMessage(hWnd, MSG_COMMAND, OT_SAVE, 0);
					else
						PostMessage(hWnd,MSG_CLOSE,0,0L);
					break;
			}			

			break;

		case MSG_CLOSE:
			//UnloadBitmap(&otset_bkg);
			DestroyMainWindow(hWnd);
			return 0;
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int OTSetWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_OS_CUST);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = otparawinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	//CreateInfo.iBkColor = COLOR_lightgray;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
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

