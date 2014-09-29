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

#define IDC_ATT_PHOTO	100
#define IDC_ATT_YEAR		101
#define IDC_ATT_MON		102
#define IDC_ATT_DAY		103
#define IDC_ATT_HOUR	104
#define IDC_ATT_MIN		105
#define IDC_ATT_SEC		106
#define IDC_ATT_DEL		107
#define IDC_ATT_COUNT	108

static HWND AttPhotoItemWnd[14];
static int AttPhotoItem;

static int isChangedPhotoOpt(void)
{
	int count;
	char c1[5];
	GetWindowText(AttPhotoItemWnd[7], c1, 4);
	count = atoi(c1);
	if(count != gOptions.DelPictureCnt)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static int PhotoOptOK(HWND hWnd, HWND Item[], int Size)
{
	int count;
	char c1[5];
	GetWindowText(AttPhotoItemWnd[7], c1, 4);
	count = atoi(c1);
	if(count != gOptions.DelPictureCnt)
	{
		gOptions.DelPictureCnt = count;
		SaveOptions(&gOptions);
		return 1;
	}
	else
	{
		return 0;
	}
}

static int getAttheight(int rows, int offset)
{
	return (10+30*rows-offset);
}

static void InitAttPhotoWindow(HWND hWnd)
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
		posX2=195+20+20;
		posX3=175+20;
		posX4=130+20+20;
		posX5=110+20;
		posX6=80+20+10;
		posX7=10;
		posX8=50;
		posX9=50;
		posX10=130;
		posX11=120;
		posX12=10;
		posX13=10;
	}
	else
	{
		posX1=10;
		posX2=75-20;
		posX3=95-20;
		posX4=130-20;
		posX5=150-20;
		posX6=185-20;
		posX7=225+gOptions.ControlOffset;
		posX8=75-20;
		posX9=10;
		posX10=240;
		posX11=250;
		posX12=260;
		posX13=145;
	}

	tmpstatic[0] = CreateWindow(CTRL_STATIC, LoadStrByID(PID_PHOTO_DEL_TIME), WS_VISIBLE |SS_LEFT, 0x11010, posX9, getAttheight(rows, 0), 265, 23, hWnd, 0);
	//AttPhotoItemWnd[0] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_ATT_PHOTO, 
	//		posX4, getAttheight(rows, 4), 30, 23, hWnd, 0);
	//SendMessage(AttPhotoItemWnd[0], EM_LIMITTEXT, 2, 0);

	rows++;
	
	AttPhotoItemWnd[0] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_ATT_YEAR, 
			posX1, getAttheight(rows, 4), 40, 23, hWnd, 0);
	 CreateWindow(CTRL_STATIC, LoadStrByID(MID_YEAR), WS_VISIBLE |SS_LEFT, 0x11010, posX2, getAttheight(rows, 0), 110, 23, hWnd, 0);

	AttPhotoItemWnd[1] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_ATT_MON, 
			posX3, getAttheight(rows, 4), 30, 23, hWnd, 0);
	 CreateWindow(CTRL_STATIC, LoadStrByID(MID_MONTH), WS_VISIBLE |SS_LEFT, 0x11010, posX4, getAttheight(rows, 0), 110, 23, hWnd, 0);

	AttPhotoItemWnd[2] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_ATT_DAY, 
			posX5, getAttheight(rows, 4), 30, 23, hWnd, 0);
	 CreateWindow(CTRL_STATIC, LoadStrByID(MID_DAY1), WS_VISIBLE |SS_LEFT, 0x11010, posX6, getAttheight(rows, 0), 110, 23, hWnd, 0);

	rows++;

	AttPhotoItemWnd[3] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_ATT_HOUR, 
			posX1, getAttheight(rows, 4), 40, 23, hWnd, 0);
	 CreateWindow(CTRL_STATIC, LoadStrByID(MID_HOUR1), WS_VISIBLE |SS_LEFT, 0x11010, posX2, getAttheight(rows, 0), 110, 23, hWnd, 0);

	AttPhotoItemWnd[4] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_ATT_MIN, 
			posX3, getAttheight(rows, 4), 30, 23, hWnd, 0);
	 CreateWindow(CTRL_STATIC, LoadStrByID(MID_MINUTE), WS_VISIBLE |SS_LEFT, 0x11010, posX4, getAttheight(rows, 0), 110, 23, hWnd, 0);

	AttPhotoItemWnd[5] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_ATT_SEC, 
			posX5, getAttheight(rows, 4), 30, 23, hWnd, 0);
	 CreateWindow(CTRL_STATIC, LoadStrByID(HIT_DATE8), WS_VISIBLE |SS_LEFT, 0x11010, posX6, getAttheight(rows, 0), 110, 23, hWnd, 0);

//	 AttPhotoItemWnd[6] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_OK), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDOK, 
//			225+gOptions.ControlOffset, 160, 85, 23, hWnd, 0);
	 AttPhotoItemWnd[6] = CreateWindow(CTRL_BUTTON, LoadStrByID(PID_PHOTO_DEL), WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, IDC_ATT_DEL, 
				posX7, getAttheight(rows, 0), 85, 23, hWnd, 0);

	rows++;
	rows++;
	sprintf(tmpbuf, "%d", gOptions.DelPictureCnt);
	AttPhotoItemWnd[7] = CreateWindow(CTRL_SLEDIT, tmpbuf, WS_VISIBLE |WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_ATT_COUNT, 
			posX1, getAttheight(rows, 4), 40, 23, hWnd, 0);
	 CreateWindow(CTRL_STATIC, LoadStrByID(PID_PHOTO_AUTO_CNT), WS_VISIBLE |SS_LEFT, 0x11010, posX8, getAttheight(rows, 0), 240, 23, hWnd, 0);

	AttPhotoItemWnd[8] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_OK), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDOK, 
			225+gOptions.ControlOffset, 160, 85, 23, hWnd, 0);
	AttPhotoItemWnd[9] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_CANCEL), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDCANCEL, 
			225+gOptions.ControlOffset, 190, 85, 23, hWnd, 0);

	SendMessage(AttPhotoItemWnd[0], EM_LIMITTEXT, 4, 0);
	SendMessage(AttPhotoItemWnd[1], EM_LIMITTEXT, 2, 0);
	SendMessage(AttPhotoItemWnd[2], EM_LIMITTEXT, 2, 0);
	SendMessage(AttPhotoItemWnd[3], EM_LIMITTEXT, 2, 0);
	SendMessage(AttPhotoItemWnd[4], EM_LIMITTEXT, 2, 0);
	SendMessage(AttPhotoItemWnd[5], EM_LIMITTEXT, 2, 0);
	SendMessage(AttPhotoItemWnd[7], EM_LIMITTEXT, 4, 0);
	AttPhotoItem = 0;
	SetFocus(AttPhotoItemWnd[AttPhotoItem]);
}

static int DelAttPhoto(void)
{
	char c1[5];
	int errorflag;
	TTime ts;
	time_t timedot;
	int iYear,iMon,iDay,iHour,iMin,iSec;
//ts.tm_year+1900,ts.tm_mon+1,ts.tm_mday,ts.tm_hour,ts.tm_min,ts.tm_sec
	errorflag = 0;
	GetWindowText(AttPhotoItemWnd[0], c1, 4);
	iYear = atoi(c1);
	if(iYear > 2030)
	{
		errorflag = 1;
	}
	GetWindowText(AttPhotoItemWnd[1], c1, 2);
	iMon = atoi(c1);
	if(iMon > 12 || iMon == 0)
	{
		errorflag = 2;
	}
	GetWindowText(AttPhotoItemWnd[2], c1, 2);
	iDay = atoi(c1);
	if(iDay == 0 ||iDay >31)
	{
		errorflag = 3;
	}
	GetWindowText(AttPhotoItemWnd[3], c1, 2);
	iHour = atoi(c1);
	if(iHour > 23)
	{
		errorflag = 4;
	}
	GetWindowText(AttPhotoItemWnd[4], c1, 2);
	iMin = atoi(c1);
	if(iMin > 59)
	{
		errorflag = 5;
	}
	GetWindowText(AttPhotoItemWnd[5], c1, 2);
	iSec = atoi(c1);
	if(iSec > 59)
	{
		errorflag = 6;
	}
	if(errorflag == 0)
	{
		ts.tm_year = iYear - 1900;
		ts.tm_mon = iMon -1;
		ts.tm_mday = iDay;
		ts.tm_hour = iHour;
		ts.tm_min = iMin;
		ts.tm_sec = iSec;
		timedot = OldEncodeTime(&ts);
		FDB_DelPhoto_By_Time(timedot);
	}
	return errorflag;
}

static int AttPhotoProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitAttPhotoWindow(hWnd);
			UpdateWindow(hWnd, TRUE);
			SetFocusChild(AttPhotoItemWnd[0]);
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
				if (++AttPhotoItem>9)
				{
					AttPhotoItem = 0;
				}

				SetFocus(AttPhotoItemWnd[AttPhotoItem]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if (--AttPhotoItem < 0)
				{
					AttPhotoItem=9; 
				}

				SetFocus(AttPhotoItemWnd[AttPhotoItem]);
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
				if (isChangedPhotoOpt())
				{
					if(PhotoOptOK(hWnd, AttPhotoItemWnd, 8) && !ismenutimeout)
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					else
						return 0;
				}
				else
					PostMessage(hWnd, MSG_CLOSE, 0, 0);

				return 0;
			}

			if(LOWORD(wParam) == SCANCODE_ENTER)
			{
				if(AttPhotoItem == 9)
				{
					PostMessage(hWnd, MSG_COMMAND, IDCANCEL, 0);
					return 0;
				}
				if(AttPhotoItem != 6)
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
					if(PhotoOptOK(hWnd, AttPhotoItemWnd, 8) && !ismenutimeout)
					{
						MessageBox1 (hWnd, LoadStrByID(HIT_RIGHT),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
						PostMessage(hWnd, MSG_CLOSE,0,0);
					}
					else
					{
						PostMessage(hWnd, MSG_CLOSE,0,0);
					}
					break;
				case IDCANCEL:
					if(isChangedPhotoOpt())
					{
						if(MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
									MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
						{
							if(PhotoOptOK(hWnd, AttPhotoItemWnd, 8))
							{
								MessageBox1 (hWnd, LoadStrByID(HIT_RIGHT),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
								PostMessage(hWnd, MSG_CLOSE,0,0);
							}
							else
							{
								MessageBox1 (hWnd, LoadStrByID(HIT_ERR),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
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
				case IDC_ATT_DEL:
					if(DelAttPhoto() == 0)
					{
						MessageBox1 (hWnd, LoadStrByID(HIT_RIGHT),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
					}
					else
					{
						MessageBox1 (hWnd, LoadStrByID(HIT_ERROR0),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
					}
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

int SSR_MENU_ATTPHOTO(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(PID_PHOTO_MNG);
	CreateInfo.hMenu = 0;
	//        CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = AttPhotoProc;
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
		return 0;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg, hMainWnd))
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



