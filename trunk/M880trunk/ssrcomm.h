/* 
* SSR 2.0 Self Service Record 主入口
* 设计：CWX        2007.1.5
* 原始版本:1.0.0
* 修改记录:
* 编译环境:mipsel-gcc
*/

#ifndef __CWX_GUI_COMM
#define __CWX_GUI_COMM

#include "ssrcommon.h"
#include "ssrmenu.h"
#include "ssrcomm1.h"
#include "ssrcomm2.h"
#include "ssrcomm3.h"
#include "ssrpub.h"
#include "options.h"

HWND hCommWnd=HWND_INVALID;

int MaxCommMenuItem=0;
static MainMenu gCommMenu;

static char* GetItemStr(int index, char* itemname)
{
	static char itemstr[20];
	sprintf(itemstr, "%s",itemname);
	return (char *)&itemstr;
}

static void WiFiSetWindow(hWnd)
{
	if (gOptions.WifiModule==0) {
		CreateWiFiWindow(hWnd, "");
	} else {
		CreateNewWiFiWindow(hWnd, "");
	}
}

extern void CreateWebWindow(HWND hWnd);
extern void SSR_MENU_UPLOADPHOTO(HWND hWnd);
static void initcommmemu(void)
{
	int i=0;

       	if (gOptions.VoiceOn)
	{
		ExPlayOtherVoice(VOICE_MENU_ENTER);
	}
	//MaxCommMenuItem=3;
	MaxCommMenuItem=1;
	/*增加串口判断,add  by yangxiaolong,2011-9-9,start*/
	/*当RS232,RS485,USB232功能不支持时，隐藏串口设置界面*/
	if (gOptions.RS485FunOn || gOptions.RS232FunOn || gOptions.USB232FunOn)
	{
		MaxCommMenuItem++;
	}
	/*增加串口判断,add  by yangxiaolong,2011-9-9,end*/
	if (gOptions.NetworkFunOn) 	//NETWORK设置菜单
	{
		MaxCommMenuItem+=1;
	}

	if(gOptions.IclockSvrFun==1)
	{
		MaxCommMenuItem+=1;
	}

	if (gOptions.IsSupportWIFI) 	//WiFi设置菜单
	{
		MaxCommMenuItem+=2;
	}

	if (gOptions.IsSupportModem){
		MaxCommMenuItem+=1;		// 3g 移动网络
	}

	if (gOptions.ExtWGInFunOn) 	//Wiegand设置菜单
	{
		MaxCommMenuItem+=1;
	}

	if (Menu_Init(&gCommMenu,MaxCommMenuItem,"mainmenu.jpg")!=MaxCommMenuItem)
	{
		printf("Main Menu init error!\n");
	}

	Menu_SetMenuItem(&gCommMenu,MainMenuWidth,MainMenuHeight,0);

	if (gOptions.NetworkFunOn)
	{
		Menu_Create(&gCommMenu, i, GetItemStr(i, LoadStrByID(HIT_COMM1)), "mi21.gif", i+1, (MENUBACKCALL)SSR_MENU_COMM1);
		i++;
	}
	/*增加串口判断,add  by yangxiaolong,2011-9-9,start*/
	/*当RS232,RS485,USB232功能不支持时，隐藏串口设置界面*/
	if (gOptions.RS485FunOn || gOptions.RS232FunOn || gOptions.USB232FunOn)
	{
		Menu_Create(&gCommMenu, i, GetItemStr(i, LoadStrByID(HIT_COMM2)), "mi22.gif", i+1, SSR_MENU_COMM2);
		i++;
	}
	/*增加串口判断,add  by yangxiaolong,2011-9-9,end*/
	Menu_Create(&gCommMenu, i, GetItemStr(i, LoadStrByID(HIT_COMM3)), "mi23.gif", i+1, SSR_MENU_COMM3);
	i++;

	if (gOptions.IsSupportWIFI)	//WiFi设置
	{
		Menu_Create(&gCommMenu, i, GetItemStr(i, LoadStrByID(HIT_COMM4)), "wifi.gif", i+1, WiFiSetWindow);
		i++;
		Menu_Create(&gCommMenu, i, GetItemStr(i, LoadStrByID(HIT_COMM5)), "single.gif", i+1,(void *) CreateWiFiInfoWindow);
		i++;
	}


	//dsl 2012.4.10
	if (gOptions.IsSupportModem){
		Menu_Create(&gCommMenu, i, GetItemStr(i, LoadStrByID(MID_MODEM_MOBILENET)), "modem.gif", i+1, (void *)CreateModemWindow);	
		i++;
	}
	
	if (gOptions.ExtWGInFunOn)	//Wiegand设置
	{
		Menu_Create(&gCommMenu, i, GetItemStr(i, LoadStrByID(MID_WG_SETTING)), "wiegand.gif", i+1, (void *)CreateWiegandWindow);
		i++;
	}

	if(gOptions.IclockSvrFun==1)
	{
		if(gOptions.IsUploadPhotoOnly) {
			Menu_Create(&gCommMenu, i, GetItemStr(i,LoadStrByID(MID_PHOTO_UPLOAD)), "mi21.gif", i+1, SSR_MENU_UPLOADPHOTO);
		} else {
	 		Menu_Create(&gCommMenu, i, GetItemStr(i,LoadStrByIDDef(MID_WENSERVER_SETUP, "Web Setup")), "mi21.gif", i+1, CreateWebWindow);
		}
		i++;
	}
}

void CreateWebWindow(HWND hWnd);

static int CommMenuWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			{
				hdc=GetClientDC(hWnd);
				//gCommMenu.dc=CreateCompatibleDCEx(hdc,320,240);
				gCommMenu.dc=CreateCompatibleDCEx(hdc,gOptions.LCDWidth,gOptions.LCDHeight);
				ReleaseDC(hdc);

				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gCommMenu.menu[gCommMenu.active].Name);
				}
#endif			
				break;
			}
		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				TTS_PlayWav(GetWavPath("item.wav"));
			}
#endif			
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

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				if(gCommMenu.active < MaxCommMenuItem-1)
					gCommMenu.active++;
				else
					gCommMenu.active=0;
				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gCommMenu.menu[gCommMenu.active].Name);
				}
#endif                
			}
			else
				if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)||(gOptions.TFTKeyLayout==3&&LOWORD(wParam)==SCANCODE_BACKSPACE))
				{
					if(gCommMenu.active>0)
					{
						gCommMenu.active--;
					}
					else
						gCommMenu.active=MaxCommMenuItem-1;

					InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
					if(gOptions.TTS_KEY)
					{
						usleep(5000);
						TTS_Say(gCommMenu.menu[gCommMenu.active].Name);
					}
#endif				
				}
				else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
				{
					if(gCommMenu.active-4 >= 0)
						gCommMenu.active -= 4;
					InvalidateRect(hWnd, &gMainMenu_rc, TRUE);
#ifdef _TTS_
					if(gOptions.TTS_KEY)
					{
						usleep(5000);
						TTS_Say(gCommMenu.menu[gCommMenu.active].Name);
					}
#endif				
				}
				else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
				{
					if(gCommMenu.active+4 < MaxCommMenuItem)
						gCommMenu.active += 4;
					else
						gCommMenu.active = MaxCommMenuItem-1;

					InvalidateRect(hWnd, &gMainMenu_rc, TRUE);
#ifdef _TTS_
					if(gOptions.TTS_KEY)
					{
						usleep(5000);
						TTS_Say(gCommMenu.menu[gCommMenu.active].Name);
					}
#endif				
				}
				else if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10))
				{
					if(gCommMenu.active>=0 && gCommMenu.active<MaxCommMenuItem)
					{
						if(gCommMenu.menu[gCommMenu.active].Proc)
						{
#ifdef _TTS_
							if(gOptions.TTS_KEY)
								TTS_Stop();
#endif                    	
							gCommMenu.menu[gCommMenu.active].Proc(hWnd);
						}
						return 0;
					}
				}
				else if ((LOWORD(wParam)>=SCANCODE_1 && (LOWORD(wParam)<=SCANCODE_9)))
				{
					int i;
					for(i=0;i<MaxCommMenuItem;i++)
					{
						if(gCommMenu.menu[i].HotKey==LOWORD(wParam)-1)
						{
							if(gCommMenu.menu[i].Proc)
								gCommMenu.menu[i].Proc(hWnd);
							return 0;
						}
					}
				}
				else if (LOWORD(wParam)==SCANCODE_ESCAPE)
				{
					PostMessage(hWnd,MSG_CLOSE,0,0);
				}
			break;

		case MSG_ERASEBKGND:
			return 1;
			/*
			   {
			   HDC hdc = (HDC)wParam;
			   const RECT* clip = (const RECT*) lParam;
			   BOOL fGetDC = FALSE;
			   RECT rcTemp;

			   if (hdc == 0) {
			   hdc = GetClientDC (hWnd);
			   fGetDC = TRUE;
			   }

			   if (clip) {
			   rcTemp = *clip;
			   ScreenToClient (hWnd, &rcTemp.left, &rcTemp.top);
			   ScreenToClient (hWnd, &rcTemp.right, &rcTemp.bottom);
			   IncludeClipRect (hdc, &rcTemp);
			   }

			   FillBoxWithBitmap (hdc, 0, 0, 0, 0, &gCommMenu.bmp_bkgnd);

			   if (fGetDC)
			   ReleaseDC (hdc);
			   return 0;
			   }
			   */
		case MSG_PAINT: 
			hdc=BeginPaint(hWnd);
			MainMenu_Show(hdc,&gCommMenu,MaxCommMenuItem);
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
				TTS_Stop();
#endif	    	
			Menu_Free(&gCommMenu);
			//       hCommWnd = HWND_INVALID;
			//MainWindowCleanup (hWnd);
			DestroyMainWindow (hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

void SSR_MENU_COMM(HWND hwnd)
{
	MSG msg;
	MAINWINCREATE CreateInfo;

	//g_rcScr.right = gOptions.LCDWidth;
	//g_rcScr.bottom = gOptions.LCDHeight;
    	gMainMenu_rc.right = gOptions.LCDWidth-1;
    	gMainMenu_rc.bottom = gOptions.LCDHeight-1;

	CreateInfo.dwStyle=WS_VISIBLE;
	CreateInfo.dwExStyle=WS_EX_NONE;
	CreateInfo.spCaption=LoadStrByID(HIT_COMM);
	CreateInfo.hMenu=0;
	CreateInfo.hCursor=GetSystemCursor(0);
	CreateInfo.hIcon=0;
	CreateInfo.MainWindowProc=CommMenuWinProc;
	CreateInfo.lx=0;
	CreateInfo.ty=0;
	CreateInfo.rx=g_rcScr.right;
	CreateInfo.by=g_rcScr.bottom;
	CreateInfo.iBkColor=0;
	CreateInfo.dwAddData=0;
	CreateInfo.hHosting = hwnd;
	
	initcommmemu();

	hCommWnd = CreateMainWindow (&CreateInfo);

//#ifdef IKIOSK
        if (hCommWnd == HWND_INVALID){
                return;
	}
        ShowWindow(hCommWnd, SW_SHOWNORMAL);

        while (GetMessage(&msg, hCommWnd))
        {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
        MainWindowThreadCleanup(hCommWnd);
        return;
//#endif
}

#endif
