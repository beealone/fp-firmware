/*
 * SSR 2.0 Self Service Record 主入口
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0
 * 修改记录:
 * 编译环境:mipsel-gcc
 */

#ifndef __CWX_GUI_USER

#define __CWX_GUI_USER

#include "ssrcommon.h"
#include "ssrmenu.h"
#include "ssrpub.h"
#include "options.h"

HWND hUserWnd=HWND_INVALID;

int MaxUserMenuItem;
//#define MaxUserMenuItem 3
extern int gMFOpened;
extern int giCLSRWOpened;
extern int ifUseWorkCode;
static MainMenu gUserMenu;

static void initusemenu(void)
{
	int menuidx;
	char menuhint[MENUNAMELEN];

	if (gOptions.VoiceOn)
		ExPlayOtherVoice(VOICE_MENU_ENTER);

	MaxUserMenuItem = 2;
	if(gOptions.IMEFunOn)
	{
		if(gOptions.IsSupportSMS) MaxUserMenuItem++;	//support SMS<liming>
		if(ifUseWorkCode) MaxUserMenuItem++;					//support WorkCode<liming>
	}
	if((gOptions.LockFunOn & LOCKFUN_ADV) || gOptions.AttUseTZ || gOptions.LockFunOn == 1) MaxUserMenuItem++;//support Lock<liming>

	if((gMFOpened && !(gOptions.MifareAsIDCard==1))|| (giCLSRWOpened && !(gOptions.iCLASSAsIDCard==1)))
		MaxUserMenuItem++;	//liming

	if(Menu_Init(&gUserMenu,MaxUserMenuItem,"mainmenu.jpg")!=MaxUserMenuItem)
		printf("Main Menu init error!\n");
	Menu_SetMenuItem(&gUserMenu,MainMenuWidth,MainMenuHeight,0);

	menuidx = 0;
	memset(menuhint,0,sizeof(menuhint));
	sprintf(menuhint,"%s",LoadStrByID(HIT_USER1));
	Menu_Create(&gUserMenu,menuidx,menuhint,"mi11.gif",menuidx+1,(void *)CreateNewUserWindow);

	menuidx++;
	memset(menuhint,0,sizeof(menuhint));
	sprintf(menuhint,"%s",LoadStrByID(HIT_USER2));
	Menu_Create(&gUserMenu,menuidx,menuhint,"mi12.gif",menuidx+1,(void *)CreateUserBrowseWindow);

	if(gOptions.IMEFunOn)
	{
		if(gOptions.IsSupportSMS)
		{
			menuidx++;
			memset(menuhint,0,sizeof(menuhint));
			sprintf(menuhint,"%s",LoadStrByID(HIT_USER3));
			Menu_Create(&gUserMenu,menuidx,menuhint,"mi13.gif",menuidx+1,(void *)CreateSmsManageWindow);
		}
		if (ifUseWorkCode)//gOptions.WorkCode)
		{
			menuidx++;
			memset(menuhint,0,sizeof(menuhint));
			sprintf(menuhint,"%s",LoadStrByID(MID_STKEYFUN3));
			Menu_Create(&gUserMenu,menuidx,menuhint,"workcode.gif",menuidx+1, (void *)CreateWorkCodeManageWindow);
		}
	}
	if((gOptions.LockFunOn & LOCKFUN_ADV) || gOptions.AttUseTZ || gOptions.LockFunOn == 1)
	{
		menuidx++;
		memset(menuhint,0,sizeof(menuhint));
		sprintf(menuhint,"%s",LoadStrByID(HIT_USER5));
		if(gOptions.LockFunOn ==1)
		{
			Menu_Create(&gUserMenu,menuidx,menuhint,"door_m.gif",menuidx+1, (void *)DoorParameterWindow);
		}
		else
		{
			Menu_Create(&gUserMenu,menuidx,menuhint,"door_m.gif",menuidx+1, (void *)LockSettingWindow);
		}
	}
	if((gMFOpened && !(gOptions.MifareAsIDCard==1)) || (giCLSRWOpened && !(gOptions.iCLASSAsIDCard==1)))
	{
		menuidx++;
		memset(menuhint,0,sizeof(menuhint));
		sprintf(menuhint,"%s",LoadStrByID(MID_CARD_MNG));
		Menu_Create(&gUserMenu,menuidx,menuhint,"card_m.gif",menuidx+1, (void *)CardMngWindow);
	}
}

static int UserMenuWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			hdc=GetClientDC(hWnd);
			gUserMenu.dc=CreateCompatibleDCEx(hdc,gOptions.LCDWidth, gOptions.LCDHeight);
			ReleaseDC(hdc);

			InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				usleep(5000);
				TTS_Say(gUserMenu.menu[gUserMenu.active].Name);
			}
#endif		
			return 0;
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
			if(gOptions.KeyPadBeep) ExKeyBeep();

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				if(gUserMenu.active<MaxUserMenuItem-1)
					gUserMenu.active++;
				else
					gUserMenu.active=0;
				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gUserMenu.menu[gUserMenu.active].Name);
				}
#endif			
			}
			else if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
					||(gOptions.TFTKeyLayout==3&&LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if(gUserMenu.active>0)
					gUserMenu.active--;
				else
					gUserMenu.active=MaxUserMenuItem-1;
				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gUserMenu.menu[gUserMenu.active].Name);
				}
#endif				
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				if(gUserMenu.active>3)
				{
					gUserMenu.active -= 4;
					InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
				}
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gUserMenu.menu[gUserMenu.active].Name);
				}
#endif			
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
			{
				if(gUserMenu.active<4 && MaxUserMenuItem >4)
				{
					if(gUserMenu.active+4 < MaxUserMenuItem) gUserMenu.active += 4;
					else gUserMenu.active = MaxUserMenuItem -1;
					InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
				}
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gUserMenu.menu[gUserMenu.active].Name);
				}
#endif			
			}
			else if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10))
			{
				if(gUserMenu.active>=0&&gUserMenu.active<MaxUserMenuItem)
				{
					if(gUserMenu.menu[gUserMenu.active].Proc)
					{
#ifdef _TTS_
						if(gOptions.TTS_KEY)
							TTS_Stop();
#endif
						gUserMenu.menu[gUserMenu.active].Proc(hWnd);
					}
				}
			}
			else if ((LOWORD(wParam)>=SCANCODE_1&&(LOWORD(wParam)<=SCANCODE_9)))
			{
				int i;
				for(i=0;i<MaxUserMenuItem;i++)
				{
					if(gUserMenu.menu[i].HotKey==LOWORD(wParam)-1)
					{
						if(gUserMenu.menu[i].Proc)
						{
							gUserMenu.menu[i].Proc(hWnd);
							break;
						}
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

				//FillBoxWithBitmap (hdc, 0, 0, 0, 0, &gUserMenu.bmp_bkgnd);

				if (fGetDC)
					ReleaseDC (hdc);
				return 0;
			}
#endif
			return 0;
		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			MainMenu_Show(hdc,&gUserMenu,MaxUserMenuItem);
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_DESTROY:
			DestroyAllControls (hWnd);
			return 0;
			//break;

		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
				TTS_Stop();
#endif	    	
			Menu_Free(&gUserMenu);
			//MainWindowCleanup (hWnd);
			DestroyMainWindow (hWnd);
			//hUserWnd = HWND_INVALID;
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

void SSR_MENU_USER(HWND hwnd)
{
	MSG msg;
	MAINWINCREATE CreateInfo;

	gMainMenu_rc.right = gOptions.LCDWidth-1;
	gMainMenu_rc.bottom = gOptions.LCDHeight-1;

#if 0
	if (hCommWnd != HWND_INVALID) {
		ShowWindow (hUserWnd, SW_SHOWNORMAL);
		return;
	}
#endif

	CreateInfo.dwStyle=WS_VISIBLE;
	CreateInfo.dwExStyle=WS_EX_NONE; //WS_EX_USEPRIVATECDC;
	CreateInfo.spCaption=LoadStrByID(HIT_COMM);
	CreateInfo.hMenu=0;
	CreateInfo.hCursor=GetSystemCursor(0);
	CreateInfo.hIcon=0;
	CreateInfo.MainWindowProc=UserMenuWinProc;
	CreateInfo.lx=0;
	CreateInfo.ty=0;
	CreateInfo.rx=g_rcScr.right;
	CreateInfo.by=g_rcScr.bottom;
	CreateInfo.iBkColor=0;
	CreateInfo.dwAddData=0;
	CreateInfo.hHosting = hwnd;
	initusemenu();
	hUserWnd = CreateMainWindow (&CreateInfo);
	//#ifdef IKIOSK
	if (hUserWnd == HWND_INVALID)
		return;
	ShowWindow(hUserWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg, hUserWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hUserWnd);
	return;
	//#endif
}


#endif
