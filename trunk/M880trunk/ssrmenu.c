/*
 * SSR 2.0 Self Service Record 主入口
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0
 * 修改记录:
 * 编译环境:mipsel-gcc
 */

#include <linux/keyboard.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/mman.h>
#include <asm/page.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <termio.h>
#include <math.h>
#include <time.h>
#include <termios.h>

#include "pushapi.h"
#include "options.h"
#include "ssrcommon.h"
#include "ssrmenu.h"
#include "ssrdate.h"
#include "ssrdata.h"
#include "ssrinfo.h"
#include "ssrauto.h"
#include "ssrcomm.h"
#include "ssrsystem.h"
#include "ssruser.h"
#include "ssrpub.h"
//#include "ssrupdate.h"
#include "exfun.h"
#include "commu.h"

#ifdef FACE
#include "facedb.h"
#endif

extern int ledbool;
extern BOOL RTCTimeValidSign;
extern int FlashRedLED;

HWND hMainMenuWnd=HWND_INVALID;
extern HWND hIMEWnd;
MainMenu gMainMenu;

static int Menuitem;
//static int menuindex=0;

extern int gAlarmDelay;
extern int gAlarmStrip;		//拆机报警标志
extern int enterkeydown;
extern int gErrTimes;

void freesubwindow(HWND subhwnd)
{
	HWND hChild;
	while ((hChild = GetFirstHosted(subhwnd)) != 0)
	{
		freesubwindow(hChild);
		if (IsDialog(hChild))
			EndDialog(hChild,IDCANCEL);
		else
			SendMessage(hChild,MSG_CLOSE,0,0);
		printf("freesubwindow.. 1\n");
	}
}

void ShowQueryDataWindow(HWND hWnd)
{
	if (gOptions.CameraOpen)
		CreateRecordWindow(hWnd);
	else
		ShowLogSetWindow(hWnd);
}

static int MainMenuWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag = 0;
	int idx=0;
	int shortcut=0;

	switch (message)
	{
		case MSG_CREATE:
			SetMenuTimeOut(0);
#ifndef ZEM600
			SetTimer(hWnd,0x998866,100);
#else
			SetTimer(hWnd,0x998866,50);
#endif

			if(!gOptions.IsSupportUSBDisk) {
				Menuitem--;
			}

			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}

			/*dsl 2012.4.17*/
			//if (TESTPRIVILLEGE(PRI_ENROLL) && !TESTPRIVILLEGE(PRI_SUPERVISOR)) {
			if (TEST_ONLY_ENROLL_PRIVILLEGE){
				Menuitem-=3;/*此处修改菜单中出现空白菜单问题2013-01-07*/
			}

			busyflag=0;
			if (gOptions.VoiceOn) {
				ExPlayOtherVoice(VOICE_MENU_ENTER);
			}

			if (Menu_Init(&gMainMenu,Menuitem,"mainmenu.jpg")!=Menuitem) {
				printf("Menu_Init error\n");
			}

			Menu_SetMenuItem(&gMainMenu,MainMenuWidth,MainMenuHeight,0);

			Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_USER),"mi1.gif",++shortcut,(MENUBACKCALL)SSR_MENU_USER);

			/*dsl 2012.4.17*/
			if (TESTPRIVILLEGE((PRI_SUPERVISOR|PRI_OPTIONS)) || (ADMINPIN==0)) {
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_COMM),"mi2.gif",++shortcut,(MENUBACKCALL)SSR_MENU_COMM);
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_SYSTEM),"mi3.gif",++shortcut,(MENUBACKCALL)SSR_MENU_SYSTEM);

			    Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_DATETIME),"mi4.gif",++shortcut,(MENUBACKCALL)SSR_MENU_DATE);
			}
			if(gOptions.IsSupportUSBDisk)
			{
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_UDATA),"mi5.gif",++shortcut,(MENUBACKCALL)SSR_MENU_DATA);
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_AUTO),"mi6.gif",++shortcut,(MENUBACKCALL)SSR_MENU_AUTO);
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_RECORD),"mi7.gif",++shortcut, (MENUBACKCALL)ShowQueryDataWindow);
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_INFO),"mi8.gif",++shortcut,(MENUBACKCALL)SSR_MENU_INFO);
			}
			else
			{
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_AUTO),"mi6.gif",++shortcut,(MENUBACKCALL)SSR_MENU_AUTO);
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_RECORD),"mi7.gif",++shortcut, (MENUBACKCALL)ShowQueryDataWindow);
				Menu_Create(&gMainMenu,idx++,LoadStrByID(HIT_INFO),"mi8.gif",++shortcut,(MENUBACKCALL)SSR_MENU_INFO);
			}

			hdc=GetClientDC(hWnd);
			gMainMenu.dc=CreateCompatibleDCEx(hdc,gOptions.LCDWidth, gOptions.LCDHeight);
			ReleaseDC(hdc);

			InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
			if(gOptions.LockFunOn && gAlarmDelay) {
				if(MessageBox1(hWnd, LoadStrByID(MID_CLEARALARM), LoadStrByID(MID_APPNAME),
							MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK) {
					DoAlarmOff(0);
					gAlarmStrip=0;	//清除拆机报警标志
					gErrTimes=0;
					AppendRTLogToFile(EF_ALARMOFF, (unsigned char*)"AlarmOff", 0);
					FDB_AddOPLog(ADMINPIN, OP_ALARM, 0xffff,0xffff,0xffff,0xffff);
					gAlarmDelay=0;
				}
			}
#ifdef _TTS_
			if(gOptions.TTS_KEY) {
				usleep(5000);
				TTS_Say(gMainMenu.menu[gMainMenu.active].Name);
			}
#endif
			break;

		case MSG_TIMER:
			if(wParam==0x998866)
			{
				time_t ctimer=time(NULL);
				if((ctimer >= (gMenuTimeOut+60)) && !busyflag)
				{
					ismenutimeout=1;
					freesubwindow(hWnd);
					if (isfpdbload) FPDBInit();
					PostMessage(hWnd,MSG_CLOSE,0,0);
				}

				if (ledbool)
				{
					if(gOptions.IsFlashLed==1)
					{
						if(RTCTimeValidSign)
							ExLightLED(LED_GREEN, FlashRedLED);
						else
							ExLightLED(LED_RED, FlashRedLED);
						FlashRedLED=!FlashRedLED;
					}

					if(gOptions.IsFlashLed==2 || gOptions.IsFlashLed==3 || gOptions.IsFlashLed==4)
					{
						if(RTCTimeValidSign)
							ExLightLED(LED_RED, FlashRedLED);
						else
							ExLightLED(LED_GREEN, FlashRedLED);
						FlashRedLED=!FlashRedLED;
					}
					if(gOptions.IsFlashLed==0)
					{
						if(RTCTimeValidSign)
							ExLightLED(LED_GREEN, FlashRedLED);
						else
							ExLightLED(LED_RED, FlashRedLED);
						FlashRedLED=FALSE;
					}
				}
			}
			break;

		case MSG_LBUTTONDOWN:
			break;

		case MSG_LBUTTONUP:
			break;

		case MSG_RBUTTONDOWN:
			break;

		case MSG_RBUTTONUP:
			break;

		case MSG_KEYUP:
			if(wParam==SCANCODE_ENTER)
				enterkeydown=0;
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
				if(++gMainMenu.active>Menuitem-1)
					gMainMenu.active=0;
				InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					usleep(5000);
					TTS_Say(gMainMenu.menu[gMainMenu.active].Name);
				}
#endif				
			}
			else
				if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
				{
					if(--gMainMenu.active<0)
						gMainMenu.active=Menuitem-1;
					InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
					if(gOptions.TTS_KEY)
					{
						usleep(5000);
						TTS_Say(gMainMenu.menu[gMainMenu.active].Name);
					}
#endif
				}
				else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
				{
					if(gMainMenu.active<(MaxMenuItem/2))
					{
						/*modified by zxz 2012-10-15*/
						if(gMainMenu.active > (Menuitem-5))
						{
							gMainMenu.active=4;
						}
						else
						{
							gMainMenu.active+=(MaxMenuItem/2);
						}
						/*
						if(!gOptions.IsSupportUSBDisk && gMainMenu.active==3)
						{
							gMainMenu.active=4;
						}
						else
						{
							gMainMenu.active+=(MaxMenuItem/2);
						}
						*/
					}
					InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
					if(gOptions.TTS_KEY)
					{
						usleep(5000);
						TTS_Say(gMainMenu.menu[gMainMenu.active].Name);
					}
#endif                                
				}
				else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
				{
					if(gMainMenu.active>=(MaxMenuItem/2))
						gMainMenu.active-=(MaxMenuItem/2);

					InvalidateRect(hWnd,&gMainMenu_rc,TRUE);
#ifdef _TTS_
					if(gOptions.TTS_KEY)
					{
						usleep(5000);
						TTS_Say(gMainMenu.menu[gMainMenu.active].Name);
					}
#endif
				}
				else
					if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10))
					{
						//printf("enterkeydown %d \n", enterkeydown);
						if((gOptions.TFTKeyLayout==3) && enterkeydown )
							break;
						if(gMainMenu.active>=0&&gMainMenu.active<Menuitem)
						{
							if(gMainMenu.menu[gMainMenu.active].Proc)
							{
#ifdef _TTS_
								if(gOptions.TTS_KEY)
									TTS_Stop();
#endif						
								gMainMenu.menu[gMainMenu.active].Proc(hWnd);
							}
						}
					}
					else if ((LOWORD(wParam)>=SCANCODE_1&&(LOWORD(wParam)<=SCANCODE_9)))
					{
						int i;
						for(i=0;i<Menuitem;i++)
						{
							if(gMainMenu.menu[i].HotKey==LOWORD(wParam)-1)
							{
								if(gMainMenu.menu[i].Proc)
								{
									gMainMenu.menu[i].Proc(hWnd);
								}
							}
						}
					}
					else if (LOWORD(wParam)==SCANCODE_ESCAPE)
						PostMessage(hWnd,MSG_CLOSE,0,0);
			break;

		case MSG_ERASEBKGND:
			return 0;
		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			MainMenu_Show(hdc,&gMainMenu,Menuitem);
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_DESTROY:
			DestroyAllControls(hWnd);
			return 0;

		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
				TTS_Stop();
#endif			
			KillTimer(hWnd,0x998866);
			Menu_Free(&gMainMenu);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

long SSR_MENU_MAINMENU(HWND hwnd)
{
	MSG Msg;
	MAINWINCREATE CreateInfo;
	ismenutimeout=0;	

	Menuitem=MaxMenuItem;
	gMainMenu_rc.right = gOptions.LCDWidth-1;
	gMainMenu_rc.bottom = gOptions.LCDHeight-1;

	CreateInfo.dwStyle=WS_VISIBLE;
	CreateInfo.dwExStyle=WS_EX_NONE;
	CreateInfo.spCaption="MAINMENU";
	CreateInfo.hMenu=0;
	CreateInfo.hIcon=0;
	CreateInfo.MainWindowProc=MainMenuWinProc;
	CreateInfo.lx=0;
	CreateInfo.ty=0;
	CreateInfo.rx=g_rcScr.right;
	CreateInfo.by=g_rcScr.bottom;
	CreateInfo.iBkColor=COLOR_black;
	CreateInfo.dwAddData=0;
	CreateInfo.hHosting=HWND_DESKTOP;

	//printf("%s, %d, hOwner=0x%X\n", __FUNCTION__, __LINE__, HWND_DESKTOP);

	hMainMenuWnd = CreateMainWindow(&CreateInfo);

	if (hMainMenuWnd == HWND_INVALID){
		return -1;
	}

	ShowWindow(hMainMenuWnd, SW_SHOWNORMAL);
	FDB_AddOPLog(ADMINPIN, OP_MENU, 0,0,0,0);

	while (GetMessage(&Msg, hMainMenuWnd))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	MainWindowThreadCleanup(hMainMenuWnd);

#ifdef FACE
	if( gOptions.FaceFunOn && FaceDBChg)
	{
		FDB_LoadAllFaceTmp();
		FaceDBChg=0;
	}
#endif	

	return 0;
}

