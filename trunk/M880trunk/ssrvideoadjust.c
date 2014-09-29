#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h> 
#include <sys/types.h>
#include <dirent.h>
#include <minigui/tftmullan.h>
#include "exfun.h"
#include "ssrcommon.h"
#include "ssrpub.h"
#include "camerafun.h"

#define IDC_BRIGHTNESS	9010
#define IDC_CONTRAST	9011
#define IDC_QUALITY	9012
#define IDC_SCENE	9013
#define IDC_DEFAULT	9014
#define IDC_OK		9015
#define IDC_CANCEL	9016
#define IDC_VIDEO_TIMER	9020
#define IDC_ROTAE_VIDEO	9021

#define SIZE_320_240	0
#define SIZE_640_480	1

//BITMAP videoadbkg;
HWND VideoSetWnd[8];
int videomode;
int curSetWnd;
int tmpbrightness, tmpcontrast, tmpquality, tmpscene;
int g_brightness, g_contrast, g_selquality, g_selscene;
/*用户照片主动上传，add by yangxiaolong,2011-6-24,start*/
BOOL  g_bIsRegUserPic = FALSE;	//用户注册时，是否注册了用户照片
/*用户照片主动上传，add by yangxiaolong,2011-6-24,end*/

extern int i353;

extern void ShowCapturePic(HDC hdc, int left, int top, int width, int height);
extern void SetCameraParameter(int bright, int contrast, int quality, int scene, int size);
static int isVideoSetChanged(HWND hWnd)
{
	//int res = 0;
	if (tmpbrightness != SendMessage(VideoSetWnd[0], CB_GETSPINVALUE, 0, 0))
		return 1;
	if (tmpcontrast != SendMessage(VideoSetWnd[1], CB_GETSPINVALUE, 0, 0))
		return 1;
	if (tmpquality != g_selquality)
		return 1;
	if (tmpscene != g_selscene)
		return 1;
	if (gOptions.isRotateCamera != SendDlgItemMessage(hWnd,IDC_ROTAE_VIDEO,CB_GETCURSEL,0,0))
		return 1;

	return 0;	
}

static void InitVideoSetWindow(HWND hWnd)
{
	char optbtnstr[20];
	int posX1;
	if (fromRight==1)  //modify by jazzy 2008.07.24
		posX1=5;
	else
		posX1=5;

	//亮度
	CreateWindow(CTRL_STATIC, LoadStrByID(MID_VIDEO_BRIGHTNESS), WS_VISIBLE | SS_LEFT, 0x11050, posX1, 10, 65, 23, hWnd, 0);
	VideoSetWnd[0] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | WS_TABSTOP | CBS_READONLY | CBS_AUTOSPIN | CBS_SPINARROW_LEFTRIGHT |
			CBS_AUTOLOOP | CBS_EDITBASELINE, IDC_BRIGHTNESS, posX1, 30, 65, 23, hWnd, 0);
	SendMessage(VideoSetWnd[0], CB_SETSPINRANGE, 10, 100);
	SendMessage(VideoSetWnd[0], CB_SETSPINPACE, 5, 5);
	SendMessage(VideoSetWnd[0], CB_SETSPINVALUE, tmpbrightness, 0);
	//SendMessage(VideoSetWnd[0], CB_SETSPINVALUE, (tmpbrightness/5)-2, 0);

	//对比度
	CreateWindow(CTRL_STATIC, LoadStrByID(MID_VIDEO_CONTRAST), WS_VISIBLE | SS_LEFT, 0x11051, posX1, 55, 65, 23, hWnd, 0);
	VideoSetWnd[1] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | WS_TABSTOP | CBS_READONLY | CBS_AUTOSPIN | CBS_SPINARROW_LEFTRIGHT |
			CBS_AUTOLOOP | CBS_EDITBASELINE, IDC_CONTRAST, posX1, 75, 65, 23, hWnd, 0);
	SendMessage(VideoSetWnd[1], CB_SETSPINRANGE, 10, 100);
	SendMessage(VideoSetWnd[1], CB_SETSPINPACE, 5, 5);
	SendMessage(VideoSetWnd[1], CB_SETSPINVALUE, tmpcontrast, 0);

	//质量
	CreateWindow(CTRL_STATIC, LoadStrByID(MID_VIDEO_QULITY), WS_VISIBLE | SS_LEFT, 0x11052, posX1, 100, 65, 23, hWnd, 0);
	VideoSetWnd[2] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | 
			CBS_AUTOFOCUS, IDC_BRIGHTNESS, posX1, 120, 65, 23, hWnd, 0);
	SendMessage(VideoSetWnd[2], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_VIDEO_QLOW));
	SendMessage(VideoSetWnd[2], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_VIDEO_QMID));
	SendMessage(VideoSetWnd[2], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_VIDEO_QHIGHT));
	SendMessage(VideoSetWnd[2], CB_SETCURSEL, tmpquality, 0);

	//环境
	CreateWindow(CTRL_STATIC, LoadStrByID(MID_CAMERA_EN), WS_VISIBLE | SS_LEFT, 0x11053, posX1, 145, 65, 23, hWnd, 0);
	VideoSetWnd[3] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | 
			CBS_AUTOFOCUS, IDC_BRIGHTNESS, posX1, 165, 65, 23, hWnd, 0);
	SendMessage(VideoSetWnd[3], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_CAMERA_IN));
	SendMessage(VideoSetWnd[3], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_CAMERA_OUT));
	SendMessage(VideoSetWnd[3], CB_SETCURSEL, tmpscene, 0);

	if(gOptions.isRotateCamera)
	{
		//旋转90度
		if(gOptions.enableVideoRotate==1)
		{
			if (i353 !=2 ){
				CreateWindow(CTRL_STATIC, LoadStrByID(MID_VIDEO_ROTATE), WS_VISIBLE | SS_LEFT, 0x11054, 230,10 , 90, 23, hWnd, 0);
			}	
			VideoSetWnd[4] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, IDC_ROTAE_VIDEO, 230,30, 65, 23, hWnd, 0);

#ifdef ZEM600
			if (gOptions.IsRotatePhoto)
			{
				SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_YES));
				SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_NO));
			}
			else
			{
				SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_NO));
				SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_YES));
			}
#else
			SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_NO));
			SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_YES));
#endif			
			SendMessage(VideoSetWnd[4], CB_SETCURSEL, gOptions.isRotateCamera, 0);
		}

		VideoSetWnd[5] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_WG_DEFAULT), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDC_DEFAULT, 227-4, 139, 85+8, 23, hWnd, 0);
		if (videomode==0)
			sprintf(optbtnstr, "%s", LoadStrByID(HIT_OK));
		else
			sprintf(optbtnstr, "%s(F8)", LoadStrByID(MID_CAMERA_MODE2));

		VideoSetWnd[6] = CreateWindow(CTRL_BUTTON, optbtnstr, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
				IDC_OK, 227-4+gOptions.ControlOffset, 164, 85+8, 23, hWnd, 0);
		VideoSetWnd[7] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_CANCEL), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
				IDC_CANCEL+gOptions.ControlOffset, 227-4, 189, 85+8, 23, hWnd, 0);
	}
	else
	{
		if(gOptions.enableVideoRotate==1)
		{
			//旋转90度
			if (i353 !=2 ){
				CreateWindow(CTRL_STATIC, LoadStrByID(MID_VIDEO_ROTATE), WS_VISIBLE | SS_LEFT, 0x11054, 90,165 , 85, 23, hWnd, 0);
			}
			VideoSetWnd[4] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, IDC_ROTAE_VIDEO, 175,165, 65, 23, hWnd, 0);

#ifdef ZEM600
			if (gOptions.IsRotatePhoto)
			{
				SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_YES));
				SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_NO));
			}
			else
			{
				SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_NO));
				SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_YES));
			}
#else
			SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_NO));
			SendMessage(VideoSetWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_YES));
#endif
			SendMessage(VideoSetWnd[4], CB_SETCURSEL, gOptions.isRotateCamera, 0);
		}


		VideoSetWnd[5] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_WG_DEFAULT), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDC_DEFAULT, 73+gOptions.ControlOffset, 189, 60, 23, hWnd, 0);
		if (videomode==0)
			sprintf(optbtnstr, "%s", LoadStrByID(HIT_OK));
		else
			sprintf(optbtnstr, "%s(F8)", LoadStrByID(MID_CAMERA_MODE2));

		VideoSetWnd[6] = CreateWindow(CTRL_BUTTON, optbtnstr, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
				IDC_OK, 137+gOptions.ControlOffset, 189, 85, 23, hWnd, 0);
		VideoSetWnd[7] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_CANCEL), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
				IDC_CANCEL, 227+gOptions.ControlOffset, 189, 85, 23, hWnd, 0);
	}
	if (i353 ==2 ){

		int retIsWindow = IsWindowVisible(VideoSetWnd[4]);
		if(retIsWindow > 0)
		{
			ShowWindow(VideoSetWnd[4],SW_HIDE);
		}
	}
}

extern int SavePhoto(void);
extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int VideoAdustWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;

	switch (message) 
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &videoadbkg, GetBmpPath("submenubg.jpg")))
			//        return 0;

			InitVideoSetWindow(hWnd);
			curSetWnd = (videomode) ? 6:0;
			SetFocusChild(VideoSetWnd[curSetWnd]);
			UpdateWindow(hWnd, TRUE);
			SetTimer(hWnd, IDC_VIDEO_TIMER, 1);		//700毫秒取一次照片
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
			hdc = BeginPaint(hWnd);
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

			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage(hWnd, MSG_COMMAND, IDC_CANCEL, 0);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if(++curSetWnd>7)
					curSetWnd = 0;

				if(curSetWnd==4 && gOptions.enableVideoRotate==0)
				{
					++curSetWnd;
				}
				 if (i353 ==2 && curSetWnd==4) {
					curSetWnd++;
				 }				
				SetFocusChild(VideoSetWnd[curSetWnd]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if (--curSetWnd < 0)
					curSetWnd = 7;

				if(curSetWnd==4 && gOptions.enableVideoRotate==0)
				{
					--curSetWnd;
				}
				 if (i353 ==2 && curSetWnd==4) {
					--curSetWnd;
				 }
				SetFocusChild(VideoSetWnd[curSetWnd]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT 
					|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if (curSetWnd == 0 || curSetWnd == 1)
				{
					SendMessage(VideoSetWnd[curSetWnd], CB_SPIN, 1, 0L);
					if (curSetWnd == 0)
					{
						g_brightness = SendMessage(VideoSetWnd[curSetWnd], CB_GETSPINVALUE, 0, 0);
						SetCameraParameter(g_brightness, 0, 3, 2, SIZE_320_240);
					}
					else
					{
						g_contrast = SendMessage(VideoSetWnd[curSetWnd], CB_GETSPINVALUE, 0, 0);
						SetCameraParameter(0, g_contrast, 3, 2, SIZE_320_240);
					}
				}
				else if (curSetWnd == 2)
				{
					if (--g_selquality < 0)
						g_selquality = 2;
					SendMessage(VideoSetWnd[curSetWnd], CB_SETCURSEL, g_selquality, 0);
					SetCameraParameter(0, 0, g_selquality, 2, SIZE_320_240);
				}
				else if (curSetWnd == 3)
				{
					if (g_selscene == 0)
						g_selscene = 1;
					else
						g_selscene = 0;
					SendMessage(VideoSetWnd[curSetWnd], CB_SETCURSEL, g_selscene, 0);
					SetCameraParameter(0, 0, 3, g_selscene, SIZE_320_240);
				}
				else if ( curSetWnd == 4)//add by cn
				{
					if (SendMessage(VideoSetWnd[curSetWnd], CB_GETCURSEL, 0, 0)==0)
						SendMessage(VideoSetWnd[curSetWnd], CB_SETCURSEL, 1, 0);
					else
						SendMessage(VideoSetWnd[curSetWnd], CB_SETCURSEL, 0, 0);
					return 1;
				}

				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if (curSetWnd == 0 || curSetWnd == 1)
				{
					SendMessage(VideoSetWnd[curSetWnd], CB_SPIN, 0, 0L);
					if (curSetWnd == 0)
					{
						g_brightness = SendMessage(VideoSetWnd[curSetWnd], CB_GETSPINVALUE, 0, 0);
						SetCameraParameter(g_brightness, 0, 3, 2, SIZE_320_240);
					}
					else
					{
						g_contrast = SendMessage(VideoSetWnd[curSetWnd], CB_GETSPINVALUE, 0, 0);
						SetCameraParameter(0, g_contrast, 3, 2, SIZE_320_240);
					}
				}
				else if (curSetWnd == 2)
				{
					if (++g_selquality > 2)
						g_selquality = 0;
					SendMessage(VideoSetWnd[curSetWnd], CB_SETCURSEL, g_selquality, 0);
					SetCameraParameter(0, 0, g_selquality, 2, SIZE_320_240);
				}
				else if (curSetWnd == 3)
				{
					if (g_selscene == 0)
						g_selscene = 1;
					else
						g_selscene = 0;
					SendMessage(VideoSetWnd[curSetWnd], CB_SETCURSEL, g_selscene, 0);
					SetCameraParameter(0, 0, 3, g_selscene, SIZE_320_240);
				}
				else if ( curSetWnd == 4)//add by cn 
				{
					if(gOptions.enableVideoRotate==1)
					{
						if (SendMessage(VideoSetWnd[curSetWnd], CB_GETCURSEL, 0, 0)==0)
							SendMessage(VideoSetWnd[curSetWnd], CB_SETCURSEL, 1, 0);
						else
							SendMessage(VideoSetWnd[curSetWnd], CB_SETCURSEL, 0, 0);
					}
					return 1;
				}

				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_ENTER)
			{
				if (curSetWnd < 5)// change 4 to 5 by cn
				{
					PostMessage(hWnd, MSG_COMMAND, IDC_OK, 0);
				}
			}

			if (LOWORD(wParam)==SCANCODE_F10)
			{
				{	//chnage by cn
					if (curSetWnd < 5 || curSetWnd == 6)
						PostMessage(hWnd, MSG_COMMAND, IDC_OK, 0);
					else if (curSetWnd == 5)
						PostMessage(hWnd, MSG_COMMAND, IDC_DEFAULT, 0);
					else
						PostMessage(hWnd, MSG_COMMAND, IDC_CANCEL, 0);
				}
				return 0;
			}

			if ((videomode==1 && LOWORD(wParam)==SCANCODE_F8) || (videomode==0 && LOWORD(wParam)==SCANCODE_MENU))
			{
				PostMessage(hWnd, MSG_COMMAND, IDC_OK, 0);
				return 0;
			}
			break;

		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_DEFAULT:
					g_brightness = 50;
					g_contrast = 50;
					g_selquality = 1;
					g_selscene = 0;
					SendMessage(VideoSetWnd[0], CB_SETSPINVALUE, g_brightness, 0);
					SendMessage(VideoSetWnd[1], CB_SETSPINVALUE, g_contrast, 0);
					SendMessage(VideoSetWnd[2], CB_SETCURSEL, g_selquality, 0);
					SendMessage(VideoSetWnd[3], CB_SETCURSEL, g_selscene, 0);
					SetCameraParameter(g_brightness, g_contrast, g_selquality, g_selscene, SIZE_320_240);
					break;

				case IDC_OK:
					if (videomode==0)
					{
						tmpbrightness = g_brightness;
						tmpcontrast = g_contrast;
						tmpquality = g_selquality;
						tmpscene = g_selscene;
						//add by cn 
						if(i353 != 2 && gOptions.isRotateCamera != SendDlgItemMessage(hWnd,IDC_ROTAE_VIDEO,CB_GETCURSEL,0,0))
						{
							KillTimer(hWnd, IDC_VIDEO_TIMER);
							gOptions.isRotateCamera=SendDlgItemMessage(hWnd,IDC_ROTAE_VIDEO,CB_GETCURSEL,0,0);
							SaveInteger("~isRotateCamera",gOptions.isRotateCamera);
						}
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
					else
					{
						KillTimer(hWnd, IDC_VIDEO_TIMER);
						if (SavePhoto())
						{
							/*用户照片主动上传，通过拍照上传，add by yangxiaolong,2011-6-24,start*/
							g_bIsRegUserPic = TRUE;
							/*用户照片主动上传，add by yangxiaolong,2011-6-24,end*/
							if (MessageBox1(hWnd, LoadStrByID(MID_CAPTURE_HINT), LoadStrByID(MID_APPNAME),
										MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
							{
								ShowCapturePic(HDC_SCREEN, 75, 30, 240, 155);
								SetTimer(hWnd, IDC_VIDEO_TIMER, 40);
							}
							else
							{
								if (!ismenutimeout)
									PostMessage(hWnd, MSG_CLOSE, 0, 0);
							}
						}
						else
						{
							MessageBox1(hWnd, LoadStrByID(MID_PATH_ERROR), LoadStrByID(MID_APPNAME), 
									MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
							if (!ismenutimeout)
								PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
					}
					break;

				case IDC_CANCEL:
					if (videomode==0 && isVideoSetChanged(hWnd))
					{
						KillTimer(hWnd, IDC_VIDEO_TIMER);
						if (MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
									MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
						{
							PostMessage(hWnd, MSG_COMMAND, IDC_OK, 0);
						}
						else
						{
							if(!ismenutimeout)
								PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
					}
					else
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					break;
			}
			break;

		case MSG_TIMER:
			if (wParam == IDC_VIDEO_TIMER)
			{
				ShowCapturePic(HDC_SCREEN, 75, 30, 240, 155);
				//				CameraPlay(HDC_SCREEN, g_brightness, g_contrast, g_selquality, g_selscene);
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&videoadbkg);
			KillTimer(hWnd, IDC_VIDEO_TIMER);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}
//flag=0:adjust video, flag=1:capture user photo
int CreateVideoAdjustWindow(HWND hWnd, int* v_brightness, int* v_contrast, int* v_quality, int* v_scene, int flag)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	tmpbrightness = *v_brightness;
	tmpcontrast = *v_contrast;
	tmpquality = *v_quality;
	tmpscene = *v_scene;

	g_brightness = tmpbrightness;
	g_contrast = tmpcontrast;
	g_selquality = tmpquality;
	g_selscene = tmpscene;

	videomode = flag;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_VIDEO_ADJUST);
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = VideoAdustWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg, hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	*v_brightness = tmpbrightness;
	*v_contrast = tmpcontrast;
	*v_quality = tmpquality;
	*v_scene = tmpscene;
	MainWindowThreadCleanup(hMainWnd);
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif
