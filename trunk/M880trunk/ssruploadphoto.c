#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <minigui/tftmullan.h>

#include <string.h>
#include "ssrcommon.h"
#include "options.h"
#include "flashdb.h"
#include "ssrpub.h"
#include "exfun.h"
#include "main.h"
#include "libdlcl.h"
#include "tftmsgbox.h"
#include "pushapi.h"

static HWND UploadPhotoItemWnd[7];
static int UploadPhotoItem;
static char ServerIP[4][4];

#define IDC_UPLOAD_IP1	100
#define IDC_UPLOAD_IP2	101
#define IDC_UPLOAD_IP3	102
#define IDC_UPLOAD_IP4	103
#define IDC_UPLOAD_PORT	104

static int getUploadPhotoheight(int rows, int offset)
{
	return (10+30*rows-offset);
}

static void InitUploadPhotoWindow(HWND hWnd)
{
	int rows = 0;
	HWND tmpstatic[2];
	char tmpbuf[100] = {0};
	
	//int i;
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9,posX10,posX11,posX12,posX13;
	//char temp[4];

	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=240+gOptions.GridWidth*2+20;
		posX2=220;
		posX3=175;
		posX4=130;
		posX5=60;
		posX6=120;
		posX7=10;
		posX8=50;
		posX9=70;
		posX10=130;
		posX11=120;
		posX12=10;
		posX13=10;
	}
	else
	{
		posX1=10;
		posX2=55;
		posX3=100;
		posX4=145;
		posX5=190;
		posX6=145+35;
		posX7=225+gOptions.ControlOffset;
		posX8=75-20;
		posX9=10;
		posX10=240;
		posX11=250;
		posX12=260;
		posX13=145;
	}
	
	memset(ServerIP, 0, sizeof(ServerIP));
	LoadStr("WebServerIP",tmpbuf);
	if(!ParseIP(tmpbuf, ServerIP[0], ServerIP[1], ServerIP[2], ServerIP[3]))
	{
		memset(ServerIP, 0, sizeof(ServerIP));
	}

	tmpstatic[0] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_PHOTO_SERVER), WS_VISIBLE |SS_LEFT, 0x11010, posX9, getUploadPhotoheight(rows, 0), 180, 23, hWnd, 0);

	rows++;
	
	UploadPhotoItemWnd[0] = CreateWindow(CTRL_SLEDIT, ServerIP[0], WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_UPLOAD_IP1, 
			posX1, getUploadPhotoheight(rows, 4), 30, 23, hWnd, 0);

	UploadPhotoItemWnd[1] = CreateWindow(CTRL_SLEDIT, ServerIP[1], WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_UPLOAD_IP2, 
			posX2, getUploadPhotoheight(rows, 4), 30, 23, hWnd, 0);

	UploadPhotoItemWnd[2] = CreateWindow(CTRL_SLEDIT, ServerIP[2], WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_UPLOAD_IP3, 
			posX3, getUploadPhotoheight(rows, 4), 30, 23, hWnd, 0);

	UploadPhotoItemWnd[3] = CreateWindow(CTRL_SLEDIT, ServerIP[3], WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_UPLOAD_IP4, 
			posX4, getUploadPhotoheight(rows, 4), 30, 23, hWnd, 0);

	CreateWindow(CTRL_STATIC, ":", WS_VISIBLE |SS_LEFT, 0x11011, posX6, getUploadPhotoheight(rows, 0), 10, 23, hWnd, 0);
	memset(tmpbuf, 0, sizeof(tmpbuf));
	sprintf(tmpbuf, "%d", gOptions.WebServerPort);
	UploadPhotoItemWnd[4] = CreateWindow(CTRL_SLEDIT, tmpbuf, WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_UPLOAD_PORT, 
		posX5, getUploadPhotoheight(rows, 4), 50, 23, hWnd, 0);

	
	UploadPhotoItemWnd[5] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_OK), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDOK, 
			225+gOptions.ControlOffset, 160, 85, 23, hWnd, 0);
	UploadPhotoItemWnd[6] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_CANCEL), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDCANCEL, 
			225+gOptions.ControlOffset, 190, 85, 23, hWnd, 0);

	SendMessage(UploadPhotoItemWnd[0], EM_LIMITTEXT, 3, 0);
	SendMessage(UploadPhotoItemWnd[1], EM_LIMITTEXT, 3, 0);
	SendMessage(UploadPhotoItemWnd[2], EM_LIMITTEXT, 3, 0);
	SendMessage(UploadPhotoItemWnd[3], EM_LIMITTEXT, 3, 0);
	SendMessage(UploadPhotoItemWnd[4], EM_LIMITTEXT, 5, 0);

	UploadPhotoItem = 0;
	SetFocus(UploadPhotoItemWnd[UploadPhotoItem]);
}

static int isUploadPhotoChanged()
{
	int count;
	int changeflag;
	char c1[5];
	changeflag = 0;
	for(count=0; count < 4; count++) {
		GetWindowText(UploadPhotoItemWnd[count], c1, 3);
		if(strncmp(ServerIP[count], c1, 3) != 0) {
			changeflag++;
			break;
		}
	}
	GetWindowText(UploadPhotoItemWnd[4], c1, 5);
	count = atoi(c1);
	if(gOptions.WebServerPort != count) {
		changeflag++;
	}
	return changeflag;
}

static int SaveChanges()
{
	char str[20] = {0};
	char WebIP0[4],WebIP1[4],WebIP2[4],WebIP3[4];
	GetWindowText(UploadPhotoItemWnd[0], WebIP0, 3);
	GetWindowText(UploadPhotoItemWnd[1], WebIP1, 3);
	GetWindowText(UploadPhotoItemWnd[2], WebIP2, 3);
	GetWindowText(UploadPhotoItemWnd[3], WebIP3, 3);
	if(CheckIP(WebIP0, WebIP1, WebIP2, WebIP3, 1))
	{
		gOptions.WebServerIP[0] = atoi(WebIP0);
		gOptions.WebServerIP[1] = atoi(WebIP1);
		gOptions.WebServerIP[2] = atoi(WebIP2);
		gOptions.WebServerIP[3] = atoi(WebIP3);
		memset(str, 0, sizeof(str));
		sprintf(str,"%s.%s.%s.%s", WebIP0, WebIP1, WebIP2, WebIP3);
		SaveStr("WebServerIP",str,1);
	} else {
		return -1;
	}
	
	memset(str, 0, sizeof(str));
	GetWindowText(UploadPhotoItemWnd[4], str, 5);
	gOptions.WebServerPort = atoi(str);
	SaveInteger("WebServerPort", atoi(str));
	if (pushsdk_is_running()) {
		if (!gOptions.WebServerURLModel) {
			snprintf((char *)gOptions.IclockSvrURL, sizeof(gOptions.IclockSvrURL),"%d.%d.%d.%d", \
			gOptions.WebServerIP[0], gOptions.WebServerIP[1], gOptions.WebServerIP[2], gOptions.WebServerIP[3]);
			SaveStr("ICLOCKSVRURL", str, 1);
		}
		pushsdk_init_cfg();
	}
	return 0;// if 0,success; if 1 nothing change
}

static int CheckIPValid(char* ipstr, int ipfirstflag)
{
	if(ipfirstflag == 0) {
		return CheckIP(ipstr, "123", "123", "123", 1);
	} else {
		return CheckIP("123", ipstr, "123", "123", 1);
	}
	return 0;
}

static int UploadPhotoProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char str[5];
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitUploadPhotoWindow(hWnd);
			UpdateWindow(hWnd, TRUE);
			SetFocusChild(UploadPhotoItemWnd[0]);
			break;

		case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			EndPaint(hWnd, hdc);
			return 0;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;

				if (hdc == 0) {
					hdc = GetClientDC(hWnd);
					fGetDC = TRUE;
				}

				if (clip) {
					rcTemp = *clip;
					ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient(hWnd, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect(hdc, &rcTemp);
				}

				FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

				if (fGetDC)
					ReleaseDC(hdc);
				return 0;
			}
		case MSG_KEYUP:
			{
				if(gOptions.TFTKeyLayout==3)
				{
					keyupFlag=1;
				}
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(gOptions.TFTKeyLayout==3)
			{
				if(keyupFlag==1)
				{
					keyupFlag=0;
				}
				else
					break;
			}
			if(gOptions.KeyPadBeep)
				ExKeyBeep();

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if(UploadPhotoItem < 4) {
					GetWindowText(UploadPhotoItemWnd[UploadPhotoItem], str, 5);
					if(CheckIPValid(str, UploadPhotoItem) == 1) {
						UploadPhotoItem++;
					} else {
						MessageBox1 (hWnd, LoadStrByID(HIT_NETERROR1),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
					}
				} else {
					UploadPhotoItem++;
				}
				if (UploadPhotoItem>6)
				{
					UploadPhotoItem = 0;
				}

				SetFocus(UploadPhotoItemWnd[UploadPhotoItem]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if(UploadPhotoItem < 4) {
					GetWindowText(UploadPhotoItemWnd[UploadPhotoItem], str, 5);
					if(CheckIPValid(str, UploadPhotoItem) == 1) {
						UploadPhotoItem--;
					} else {
						MessageBox1 (hWnd, LoadStrByID(HIT_NETERROR1),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
					}
				} else {
					UploadPhotoItem--;
				}
				if (UploadPhotoItem < 0)
				{
					UploadPhotoItem = 6; 
				}

				SetFocus(UploadPhotoItemWnd[UploadPhotoItem]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{

			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
			}

			if ((LOWORD(wParam)==SCANCODE_MENU)||(LOWORD(wParam)==SCANCODE_F10))
			{
				return 0;
			}

			if(LOWORD(wParam) == SCANCODE_ENTER)
			{
				if(UploadPhotoItem == 6)
				{
					PostMessage(hWnd, MSG_COMMAND, IDCANCEL, 0);
					return 0;
				}
				if(UploadPhotoItem != 6)
				{
					PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
					return 0;
				}
			}

			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage(hWnd, MSG_COMMAND, IDCANCEL, 0);
				return 0;
			}
			break;
		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					if(isUploadPhotoChanged() > 0)
					{
						if(SaveChanges() == 0)
						{
							MessageBox1 (hWnd, LoadStrByID(HIT_RIGHT),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
							PostMessage(hWnd, MSG_CLOSE,0,0);
						}
						else
						{
							MessageBox1 (hWnd, LoadStrByID(MID_MF_SAVEFL),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
							PostMessage(hWnd, MSG_CLOSE,0,0);
						}
					}
					else
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					break;
				case IDCANCEL:
					if(isUploadPhotoChanged() > 0)
					{
						if(MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
									MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
						{
							if(SaveChanges() == 0)
							{
								MessageBox1 (hWnd, LoadStrByID(HIT_RIGHT),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
								PostMessage(hWnd, MSG_CLOSE,0,0);
							}
							else
							{
								MessageBox1 (hWnd, LoadStrByID(MID_MF_SAVEFL),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
								PostMessage(hWnd, MSG_CLOSE,0,0);
							}
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
				default:
					break;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&system1bk);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

void SSR_MENU_UPLOADPHOTO(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_PHOTO_UPLOAD);
	CreateInfo.hMenu = 0;
	//        CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = UploadPhotoProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//LoadSystem1Str();

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return ;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg, hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hMainWnd);
	return ;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif




