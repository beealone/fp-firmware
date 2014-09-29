/*
   showmainstate.c 鐢ㄦ埛蹇嵎閿樉绀虹晫闈�
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
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"


//BITMAP mainstatebmp;
int putkey=0;

extern int isShortKey(int wParam);

static int procgetstate(int wParam)
{
	int myindex=0;
	TSHORTKEY mystkey;

	myindex=isShortKey(LOWORD(wParam));
	if (myindex ==0 ) return 0;

	memset(&mystkey,0,sizeof(TSHORTKEY));
	if(FDB_GetShortKey(myindex, &mystkey)!=NULL && (mystkey.keyFun==1))
	{
		if (gOptions.ShowState && gOptions.AttState!=mystkey.stateCode)
		{
			gOptions.AttState = mystkey.stateCode;
			printf("gOptions.AttState:%d\n",gOptions.AttState);
			return 1;
		}
	}
	return 0;
}

#if 1
static int procmainstatekey(HDC hdc,HWND hWnd)
{
	int i,j;
	BITMAP bmp;
	int x,y;
	//HDC hdc;
	unsigned char temp[30]={0};
	int num=0;
	int tmpvalue = 0;
	//hdc=GetClientDC(hWnd);
	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	for(i=0;i<8;i++)
	{
		TSHORTKEY mystkey;
		memset(&mystkey,0,sizeof(TSHORTKEY));

		if(i<4)
		{
			x=0;
			//y=70+i*133;
			y=20+i*60;
		}
		else
		{
			//x=705;
			//y=70+(i-4)*133;
			x=160;
			y=20+(i-4)*60;
		}
		if(FDB_GetShortKey(i+1, &mystkey)!=NULL)
		{
			if(i==putkey)
				LoadBitmap(hdc,&bmp,GetBmpPath("status2.gif"));
			else
				LoadBitmap(hdc,&bmp,GetBmpPath("status.gif"));
			if(mystkey.keyFun)
				FillBoxWithBitmap(hdc,x,y,160,35,&bmp);
			UnloadBitmap(&bmp);
			memset(temp,0,sizeof(temp));
			strncpy((char *)temp,mystkey.stateName,STATE_NAME_LEN);
			//10
			for(j=0;j<STATE_NAME_LEN;j++)
			{
				if(temp[j]==0x0)
					break;
				if(temp[j]<0xa0)
					num++;
			}
			//if((num%2)&&(strlen((char *)temp)==10)&&temp[9]>=0xa0)
			if((num%2)&&(strlen((char *)temp)==STATE_NAME_LEN)&&temp[STATE_NAME_LEN-1]>=0xa0)
				temp[STATE_NAME_LEN-1]=0x0;

			//TextOut(hdc,x+(95-strlen(temp)*8)/2,y+20,temp);
			TextOut(hdc,x+(160-strlen((char *)temp)*8)/2,y+10,(char *)temp);
			num=0;
		}

	}
	//ReleaseDC(hdc);
	return 0;
}

#endif

static int mainstatewinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)	
	{
		case MSG_CREATE:
			//LoadBitmap(HDC_SCREEN,&mainstatebmp,GetBmpPath("submenubg.jpg"));
			UpdateWindow(hWnd,TRUE);		
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);		
			procmainstatekey(hdc,hWnd);
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

			if ((LOWORD(wParam)>=SCANCODE_F1) && ((LOWORD(wParam)<=SCANCODE_F8)))
			{
				putkey=LOWORD(wParam)-SCANCODE_F1;
				if (procgetstate(wParam))
				{
					InvalidateRect(hWnd,NULL,FALSE);

				}
			}
			else
				PostMessage(hWnd, MSG_CLOSE, 0, 0L);
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))		
				PostMessage(hWnd, MSG_CLOSE, 0, 0L);
			break;			

		case MSG_CLOSE:
			//UnloadBitmap(&mainstatebmp);
			DestroyMainWindow(hWnd);
			return 0;
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
				FillBoxWithBitmap (hdc, 0, 0, 0, 0, get_submenubg_jgp());
				if (fGetDC)
					ReleaseDC (hdc);
				return 0;
			}

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateMainStateWindow(HWND hOwner,int key)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;


	putkey=key;
	hOwner = GetMainWindowHandle (hOwner);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = mainstatewinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
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

	return 1;

}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

