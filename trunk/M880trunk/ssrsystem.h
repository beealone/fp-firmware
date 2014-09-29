/*
 * SSR 2.0 Self Service Record 主入口
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0
 * 修改记录:
 * 编译环境:mipsel-gcc
 */

#ifndef __CWX_GUI_SYSTEM

#define __CWX_GUI_SYSTEM

#include "ssrcommon.h"
#include "ssrmenu.h"
#include "ssrsystem2.h"
#include "ssrsystem4.h"
#include "ssrsystem5.h"
#include "ssrpub.h"
#include "options.h"

HWND hSystemWnd=HWND_INVALID;
int MaxSystemMenuItem;
extern void SSR_MENU_SYSTEM6(HWND);
static MainMenu gSystemMenu;

static void initsysttemmenu(void)
{
	char iconhint[MENUNAMELEN];
	int menuidx;
	if (gOptions.VoiceOn)
		ExPlayOtherVoice(VOICE_MENU_ENTER);
	MaxSystemMenuItem = 5;

	if(gOptions.IsSupportUSBDisk)   MaxSystemMenuItem++;

	if(gOptions.ShowState) MaxSystemMenuItem++;
	if(gOptions.AutoAlarmFunOn) MaxSystemMenuItem++;

	if(Menu_Init(&gSystemMenu, MaxSystemMenuItem, "mainmenu.jpg") != MaxSystemMenuItem)
		printf("Main Menu init error!\n");
	Menu_SetMenuItem(&gSystemMenu, MainMenuWidth, MainMenuHeight, 0);
	menuidx=0;
	memset(iconhint,0,sizeof(iconhint));
	sprintf(iconhint,"%s", LoadStrByID(HIT_SYSTEM1));
	Menu_Create(&gSystemMenu,menuidx,iconhint,"mi31.gif",menuidx+1,(void *)SSR_MENU_SYSTEM1);

	menuidx++;
	memset(iconhint,0,sizeof(iconhint));
	sprintf(iconhint,"%s",LoadStrByID(HIT_SYSTEM2));
	Menu_Create(&gSystemMenu,menuidx,iconhint,"mi32.gif",menuidx+1,SSR_MENU_SYSTEM2);

	if(gOptions.IsSupportUSBDisk)
	{
		menuidx++;
		memset(iconhint,0,sizeof(iconhint));
		sprintf(iconhint,"%s",LoadStrByID(HIT_SYSTEM4));
		Menu_Create(&gSystemMenu,menuidx,iconhint,"mi33.gif",menuidx+1,SSR_MENU_UPDATE);
	}
	if(gOptions.ShowState)
	{
		menuidx++;
		memset(iconhint,0,sizeof(iconhint));
		sprintf(iconhint,"%s",LoadStrByID(HIT_SYSTEM5));
		Menu_Create(&gSystemMenu,menuidx,iconhint,"mi34.gif",menuidx+1,(void *)CreateShortKeyWindow);
	}

	menuidx++;
	memset(iconhint,0,sizeof(iconhint));
	sprintf(iconhint,"%s",LoadStrByID(HIT_SYSTEM6));
	Menu_Create(&gSystemMenu,menuidx,iconhint,"mi35.gif",menuidx+1, (void *)CreateSystem6Window);//SSR_MENU_SYSTEM6);

	menuidx++;
	memset(iconhint,0,sizeof(iconhint));
	sprintf(iconhint,"%s",LoadStrByID(HIT_SYSTEM8));
	Menu_Create(&gSystemMenu,menuidx,iconhint,"mi37.gif",menuidx+1,(void *)SSR_MENU_RESTORE);
	if(gOptions.AutoAlarmFunOn)
	{
		menuidx++;
		memset(iconhint,0,sizeof(iconhint));
		sprintf(iconhint,"%s",LoadStrByID(MID_ALARMSETTING));
		Menu_Create(&gSystemMenu,menuidx,iconhint,"mi38.gif",menuidx+1,(void *)CreateArlarmManageWindow);
	}

	menuidx++;
	memset(iconhint,0,sizeof(iconhint));
	sprintf(iconhint,"%s", LoadStrByID(HIT_SYSTEM7));
	Menu_Create(&gSystemMenu,menuidx,iconhint,"mi36.gif",menuidx+1,(void *) CreateSystemOtherWindow);//SSR_MENU_SYSTEM3);
}

static int SystemMenuWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			hdc=GetClientDC(hWnd);
			gSystemMenu.dc=CreateCompatibleDCEx(hdc,gOptions.LCDWidth, gOptions.LCDHeight);
			ReleaseDC(hdc);
			InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				usleep(5000);
				TTS_Say(gSystemMenu.menu[gSystemMenu.active].Name);
			}
#endif			
			break;
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
			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
			{
				if(gSystemMenu.active==0)
					gSystemMenu.active=4;
				else if(gSystemMenu.active==1)
					gSystemMenu.active=5;
				else if(gSystemMenu.active==2)
					gSystemMenu.active=6;
				else if(gSystemMenu.active==3)
				{
					if(gOptions.AutoAlarmFunOn)
						gSystemMenu.active=7;
					else
						gSystemMenu.active=4;
				}
				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gSystemMenu.menu[gSystemMenu.active].Name);
				}
#endif				
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				if(gSystemMenu.active==4)
					gSystemMenu.active=0;
				else if(gSystemMenu.active==5)
					gSystemMenu.active=1;
				else if(gSystemMenu.active==6)
					gSystemMenu.active=2;
				else if(gSystemMenu.active==7)
					gSystemMenu.active=3;
				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gSystemMenu.menu[gSystemMenu.active].Name);
				}
#endif				
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				if(gSystemMenu.active<MaxSystemMenuItem-1)
					gSystemMenu.active++;
				else
					gSystemMenu.active=0;
				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gSystemMenu.menu[gSystemMenu.active].Name);
				}
#endif				
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)||(gOptions.TFTKeyLayout==3&&LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if(gSystemMenu.active>0)
					gSystemMenu.active--;
				else
					gSystemMenu.active=MaxSystemMenuItem-1;
				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gSystemMenu.menu[gSystemMenu.active].Name);
				}
#endif				
			}
			else if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10))
			{
				if(gSystemMenu.active>=0 && gSystemMenu.active<MaxSystemMenuItem)
				{
					if(gSystemMenu.menu[gSystemMenu.active].Proc)
					{
#ifdef _TTS_
						if(gOptions.TTS_KEY)
							TTS_Stop();
#endif
						gSystemMenu.menu[gSystemMenu.active].Proc(hWnd);
					}
					return 0;
				}
			}
			else if ((LOWORD(wParam)>=SCANCODE_1)&&(LOWORD(wParam)<=SCANCODE_9))
			{
				int i;
				for(i=0;i<MaxSystemMenuItem;i++)
				{
					if(gSystemMenu.menu[i].HotKey==LOWORD(wParam)-1)
					{
						gSystemMenu.menu[i].Proc(hWnd);
						break;
					}
				}
			}
			else if (LOWORD(wParam)==SCANCODE_ESCAPE)
				PostMessage(hWnd,MSG_CLOSE,0,0);
			break;

		case MSG_ERASEBKGND:
#if 0
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

				//FillBoxWithBitmap (hdc, 0, 0, 0, 0, &gSystemMenu.bmp_bkgnd);

				if (fGetDC)
					ReleaseDC (hdc);
				return 0;
			}
#endif
			return 0;
		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			MainMenu_Show(hdc,&gSystemMenu,MaxSystemMenuItem);
			EndPaint(hWnd,hdc);
			return 0;
		case MSG_DESTROY:
			DestroyAllControls (hWnd);
			return 0;
		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
				TTS_Stop();
#endif	    	
			Menu_Free(&gSystemMenu);
			//hSystemWnd = HWND_INVALID;
			// MainWindowCleanup (hWnd);
			DestroyMainWindow (hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}


void SSR_MENU_SYSTEM(HWND hwnd)
{
	MSG msg;
	MAINWINCREATE CreateInfo;

	gMainMenu_rc.right = gOptions.LCDWidth-1;
	gMainMenu_rc.bottom = gOptions.LCDHeight-1;

#if 0
	if (hSystemWnd != HWND_INVALID) {
		ShowWindow (hSystemWnd, SW_SHOWNORMAL);
		return;
	}
#endif

	CreateInfo.dwStyle=WS_VISIBLE;
	CreateInfo.dwExStyle=WS_EX_NONE;
	CreateInfo.spCaption=LoadStrByID(HIT_SYSTEM);
	CreateInfo.hMenu=0;
	//    CreateInfo.hCursor=GetSystemCursor(0);
	CreateInfo.hIcon=0;
	CreateInfo.MainWindowProc=SystemMenuWinProc;
	CreateInfo.lx=0;
	CreateInfo.ty=0;
	CreateInfo.rx=g_rcScr.right;
	CreateInfo.by=g_rcScr.bottom;
	CreateInfo.iBkColor=0;
	CreateInfo.dwAddData=0;
	CreateInfo.hHosting = hwnd;

	initsysttemmenu();

	hSystemWnd = CreateMainWindow (&CreateInfo);
	if (hSystemWnd == HWND_INVALID)
		return;
	ShowWindow(hSystemWnd, SW_SHOWNORMAL);
	while (GetMessage(&msg, hSystemWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hSystemWnd);
	return ;
}


#endif
