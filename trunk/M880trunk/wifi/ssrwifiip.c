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
#include <minigui/tftmullan.h>

#define WIFIIP_STATIC	1050
#define WIFI_IP		1060
#define WIFI_SUB	1064
#define WIFI_GATE	1068
#define WIFIIP_SAVE	1069
#define WIFIIP_EXIT	1070

HWND EdIPWnd[14];
int curFocusidx=0;
int curipmode=0;

//BITMAP wifiipbkg;
char* WifiIPStr[5];
int oldipvalue[4];			//原始IP
int oldsubmask[4];		//原始SubMask
int oldgateway[4];		//原始GateWay
extern char newip[3][18];
extern int OptMod_wifi;

static int beModified(void)
{
	char ip[3][16];
	int i;

	for(i=0;i<4;i++)
	{
		GetWindowText(EdIPWnd[i], ip[0], 16);
		GetWindowText(EdIPWnd[4+i], ip[1], 16);
		GetWindowText(EdIPWnd[8+i], ip[2], 16);
		if((strlen(ip[0])>0 && atoi(ip[0])!=oldipvalue[i]) || 
				(strlen(ip[1])>0 && atoi(ip[1])!=oldsubmask[i]) || 
				(strlen(ip[2])>0 && atoi(ip[2])!=oldgateway[i]))
			return 1;
	}
	return 0;
}

static void InitIPSetting(void)
{

	if (OptMod_wifi==1)
	{
		//unsigned char ipcfg[40];
		char ipcfg[40];
		memset(ipcfg,0,sizeof(ipcfg));

		LoadStr("wifiip", ipcfg);
		SetWindowText(EdIPWnd[0],ipcfg);

		memset(ipcfg,0,sizeof(ipcfg));
		LoadStr("wifimask", ipcfg);
		SetWindowText(EdIPWnd[1],ipcfg);

		memset(ipcfg,0,sizeof(ipcfg));
		LoadStr("wifigateway", ipcfg);
		SetWindowText(EdIPWnd[2],ipcfg);
	}
	else
	{
		char ip[4][16];
		int i;
		if(ParseIP(newip[0],ip[0],ip[1],ip[2],ip[3]))
		{
			for(i=0;i<4;i++)
			{
				if(strlen(ip[i])>0)
				{
					SetWindowText(EdIPWnd[i],ip[i]);
					oldipvalue[i]=atoi(ip[i]);
				}
				else
				{
					SetWindowText(EdIPWnd[i],"0");
					oldipvalue[i]=0;
				}
			}
		}

		//Sub Mask
		if(ParseIP(newip[1],ip[0],ip[1],ip[2],ip[3]))
		{
			for(i=0;i<4;i++)
			{
				SetWindowText(EdIPWnd[4+i],ip[i]);
				oldsubmask[i]=atoi(ip[i]);
			}
		}

		//Gate Way
		if(ParseIP(newip[2],ip[0],ip[1],ip[2],ip[3]))
		{
			for(i=0;i<4;i++)
			{
				SetWindowText(EdIPWnd[8+i],ip[i]);
				oldgateway[i]=atoi(ip[i]);
			}
		}
	}
}

static void LoadWindowStr(void)
{
	WifiIPStr[0]=LoadStrByID(MID_NET_IP);		//IP地址
	WifiIPStr[1]=LoadStrByID(MID_NETMASK_ADDR);	//子网掩码
	WifiIPStr[2]=LoadStrByID(MID_GATEWAY_IP);	//网关地址
	WifiIPStr[3]=LoadStrByID(MID_SAVE);		//保存
	WifiIPStr[4]=LoadStrByID(MID_EXIT);		//退出
}

static void SelectNext(HWND hWnd,WPARAM wParam)
{
	if(wParam==SCANCODE_CURSORBLOCKLEFT || ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
	{
		if(--curFocusidx<0)
			curFocusidx=13;
	}	
	else if(wParam==SCANCODE_CURSORBLOCKRIGHT)
	{
		if(++curFocusidx>13)
			curFocusidx=0;
	}
	else if(wParam==SCANCODE_CURSORBLOCKUP)
	{
		if(curFocusidx==13)
			curFocusidx=12;
		else if(curFocusidx==12)
			curFocusidx=11;
		else
		{
			if(curFocusidx>=4 && curFocusidx<=11)
				curFocusidx-=4;
			else
				curFocusidx=13;
		}
	}
	else if(wParam==SCANCODE_CURSORBLOCKDOWN)
	{
		if(curFocusidx==12)
			curFocusidx=13;
		else if(curFocusidx==13)
			curFocusidx=0;
		else
		{
			if(curFocusidx>=0 && curFocusidx<=7)
				curFocusidx+=4;
			else
				curFocusidx=12;
		}
	}	
	SetFocusChild(EdIPWnd[curFocusidx]);
}

static int CheckItem(int item)
{
	char cc[256];
	int mm;
	GetWindowText(EdIPWnd[item],cc,256);
	mm=atol(cc);
	if(curFocusidx>=0&&curFocusidx<=3)
	{
		if(strlen(cc) && (mm>=0&&mm<=255)) return 1;
		else return 0;
	}
	else
		if(curFocusidx>=4&&curFocusidx<=7)
		{
			if(strlen(cc) && (mm>=0&&mm<=255)) return 1;
			else return 0;
		}
		else
			if(curFocusidx>=8&&curFocusidx<=11)
			{
				if(strlen(cc) && (mm>=0&&mm<255)) return 1;
				else return 0;
			}
	return 0;
}

static void wifiip_notif_proc(HWND hwnd, int id, int nc, DWORD add_data)
{
	unsigned int len;
	if (nc == EN_CHANGE) 
	{
		if(id>=WIFI_IP&&id<=WIFI_GATE)
		{
			len=GetWindowTextLength(hwnd);
			if(len>=3)
			{
				if(CheckItem(curFocusidx))
				{
					if(curFocusidx>=0&&curFocusidx<=3)
						++curFocusidx;
					else
						if(curFocusidx>=4&&curFocusidx<=7)
							++curFocusidx;
						else
							if(curFocusidx>=8&&curFocusidx<=11)
								++curFocusidx;
					SetFocus(EdIPWnd[curFocusidx]);
				}
			}
		}
	}
}

static void InitWindow (HWND hWnd)
{
	int i;
	int posX1,posX2;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=230;
		posX2=60;
	}
	else
	{
		posX1=10;
		posX2=100;
	}
	CreateWindow(CTRL_STATIC,WifiIPStr[0],WS_VISIBLE|SS_LEFT,WIFIIP_STATIC,posX1,20,80,23,hWnd,0);
	CreateWindow(CTRL_STATIC,WifiIPStr[1],WS_VISIBLE|SS_LEFT,WIFIIP_STATIC+1,posX1,50,80,23,hWnd,0);
	CreateWindow(CTRL_STATIC,WifiIPStr[2],WS_VISIBLE|SS_LEFT,WIFIIP_STATIC+2,posX1,80,80,23,hWnd,0);
	if(OptMod_wifi==1)
	{
		EdIPWnd[0]=CreateWindow(CTRL_STATIC,"",WS_VISIBLE |  WS_BORDER | WS_THICKFRAME | SS_LEFT,WIFI_IP,posX2,16,140,23,hWnd,0);
		EdIPWnd[1]=CreateWindow(CTRL_STATIC,"",WS_VISIBLE |  WS_BORDER | WS_THICKFRAME | SS_LEFT,WIFI_SUB,posX2,46,140,23,hWnd,0);
		EdIPWnd[2]=CreateWindow(CTRL_STATIC, "",WS_VISIBLE |  WS_BORDER | WS_THICKFRAME | SS_LEFT,WIFI_GATE,posX2,76,140,23,hWnd,0);
	}
	else
	{
		for(i=0;i<4;i++)
		{
			EdIPWnd[i]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,WIFI_IP,posX2+40*i,16,35,23,hWnd,0);
			EdIPWnd[4+i]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,WIFI_SUB,posX2+40*i,46,35,23,hWnd,0);
			EdIPWnd[8+i]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,WIFI_GATE,posX2+40*i,76,35,23,hWnd,0);
			SendMessage(EdIPWnd[i], EM_LIMITTEXT, 3, 0);	
			SendMessage(EdIPWnd[4+i], EM_LIMITTEXT, 3, 0);	
			SendMessage(EdIPWnd[8+i], EM_LIMITTEXT, 3, 0);	
			SetNotificationCallback(EdIPWnd[i],wifiip_notif_proc);
			SetNotificationCallback(EdIPWnd[4+i],wifiip_notif_proc);
			SetNotificationCallback(EdIPWnd[8+i],wifiip_notif_proc);
		}

		EdIPWnd[12]=CreateWindow(CTRL_BUTTON,WifiIPStr[3],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,WIFIIP_SAVE,10,190,85,23,hWnd,0);
		EdIPWnd[13]=CreateWindow(CTRL_BUTTON,WifiIPStr[4],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,WIFIIP_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);
	}
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int SaveWifiIPSetting(HWND hWnd)
{
	char str[3][50];
	char cc[3][16];
	int err=0;
	int i;

	for(i=0;i<4;i++)
		GetWindowText(EdIPWnd[i],cc[i],16);
	if(CheckIP(cc[0],cc[1],cc[2],cc[3],1))
		sprintf(str[0],"%s.%s.%s.%s",cc[0],cc[1],cc[2],cc[3]);
	else err=1;

	for(i=0;i<4;i++)
		GetWindowText(EdIPWnd[4+i],cc[i],16);
	if(CheckIP(cc[0],cc[1],cc[2],cc[3],0))
		sprintf(str[1],"%s.%s.%s.%s",cc[0],cc[1],cc[2],cc[3]);
	else err=2;

	for(i=0;i<4;i++)
		GetWindowText(EdIPWnd[8+i],cc[i],16);
	if(CheckIP(cc[0],cc[1],cc[2],cc[3],0))
		sprintf(str[2],"%s.%s.%s.%s",cc[0],cc[1],cc[2],cc[3]);
	else err=3;

	if(err)
	{
		switch(err)
		{
			case 1:
				MessageBox1(hWnd ,LoadStrByID(HIT_NETERROR1) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
				break;
			case 2:
				MessageBox1(hWnd ,LoadStrByID(HIT_NETERROR2) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
				break;
			case 3:
				MessageBox1(hWnd ,LoadStrByID(HIT_NETERROR3) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
				break;
		}
		return 0;
	}
	else
	{
		memset(newip, 0, 54);
		sprintf(newip[0],"%s",str[0]);
		sprintf(newip[1],"%s",str[1]);
		sprintf(newip[2],"%s",str[2]);
		//		printf("newip:%s\n", newip[0]);
		//		printf("newmask:%s\n", newip[1]);
		//		printf("newgate:%s\n", newip[2]);
		/*
		   SaveStr("wifiip",str[0],1);
		   SaveStr("wifimask",str[1],1);
		   SaveStr("wifigate",str[2],1);
		   */
	}

	MessageBox1(hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
	return 1;
}

static int WiFiIPWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int nc, id,posX1=0, tmpvalue = 0;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitWindow(hWnd);		//add controls
			InitIPSetting();
			if(OptMod_wifi!=1)
			{
				curFocusidx=0;
				SetFocusChild(EdIPWnd[curFocusidx]);
			}
			UpdateWindow(hWnd,TRUE);
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

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());
				if(fGetDC) ReleaseDC (hdc);
				return 0;
			}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);	
			tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,PIXEL_lightwhite);
			if (fromRight==1)
				posX1=40;
			if (OptMod_wifi!=1)
			{
				TextOut(hdc,133-posX1,24,".");
				TextOut(hdc,173-posX1,24,".");
				TextOut(hdc,213-posX1,24,".");
				TextOut(hdc,133-posX1,54,".");
				TextOut(hdc,173-posX1,54,".");
				TextOut(hdc,213-posX1,54,".");
				TextOut(hdc,133-posX1,84,".");
				TextOut(hdc,173-posX1,84,".");
				TextOut(hdc,213-posX1,84,".");
			}
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
			{
				if (OptMod_wifi==1)
					SendMessage(hWnd,MSG_CLOSE,0,0);
				else
					SendMessage(hWnd, MSG_COMMAND, WIFIIP_EXIT, 0);
				return 0;
			}
			else if (((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || 
						(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)||
						(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || 
						(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
						|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
					&&(OptMod_wifi!=1))
			{
				if(curFocusidx<12)
				{
					//Liaozz 20080928 fix bug second 44
					//char ttc[16];
					//GetWindowText(EdIPWnd[curFocusidx], ttc, 16);
					//if(ttc[0] && strlen(ttc)>0)
					SelectNext(hWnd,wParam);
				}
				else
					SelectNext(hWnd, wParam);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_ENTER&&(OptMod_wifi!=1))
			{
				if(curFocusidx<12)
					PostMessage(hWnd, MSG_COMMAND, WIFIIP_SAVE, 0);
			}
			else if(LOWORD(wParam)==SCANCODE_F10&&(OptMod_wifi!=1))
			{
				if(curFocusidx!=13)
					PostMessage(hWnd, MSG_COMMAND, WIFIIP_SAVE, 0L);
				else
					PostMessage(hWnd, MSG_COMMAND, WIFIIP_EXIT, 0L);
			}
			else if(LOWORD(wParam)==SCANCODE_MENU&&(OptMod_wifi!=1))
			{
				PostMessage(hWnd, MSG_COMMAND, WIFIIP_SAVE, 0);
			}

			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(id==WIFIIP_SAVE)
			{
				if(beModified())
				{
					if(SaveWifiIPSetting(hWnd) && !ismenutimeout)
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				else
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
			}
			else if(id==WIFIIP_EXIT)
			{
				if(beModified() && 
						MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA),LoadStrByID(MID_APPNAME),
							MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK)
					PostMessage(hWnd, MSG_COMMAND, WIFIIP_SAVE, 0);
				else
				{
					if(!ismenutimeout) PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&wifiipbkg);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateWiFiIPWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	LoadWindowStr();

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER|WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_WIFI_IP);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WiFiIPWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//if (LoadBitmap(HDC_SCREEN,&wifiipbkg,GetBmpPath("submenubg.jpg")))
	//	return 0;
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

