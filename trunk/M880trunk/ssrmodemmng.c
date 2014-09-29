#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <minigui/tftmullan.h>

#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"
#include "t9.h"
#include "sensor.h"
#include "main.h"
#include "modem/modem.h"
#include "modem/ssrmodem.h"
#include "tftmsgbox.h"

#define MAX_NUM_LEN 32
HWND ModemWnd[12];
BITMAP ModemBkg;
int ModemCurWnd;
char HeartBeatServer[MAX_NUM_LEN];
char ModemAPN[MAX_NUM_LEN];
char ModemDialNumber[MAX_NUM_LEN];
char ModemUserName[MAX_NUM_LEN];
char ModemPassWord[MAX_NUM_LEN];
static int module_type;

static int isModemSettingChanged(void)
{
	int n;
	char ipaddr[4][4];
	char string[MAX_NUM_LEN];
	
	if (gOptions.ModemEnable != SendMessage(ModemWnd[0], CB_GETCURSEL, 0 ,0)){
		return 1;
	}

	if (module_type != MODEM_MODULE_EM660) {
		memset(string, 0, sizeof(string));
		GetWindowText(ModemWnd[1], string, sizeof(string));
		if (strncmp(ModemAPN, string,sizeof(string)) != 0){
			return 1;
		}
	}

	memset(string, 0, sizeof(string));
	GetWindowText(ModemWnd[2], string, sizeof(string));
	if (strncmp(ModemDialNumber, string,sizeof(string)) != 0){
		return 1;
	}

	memset(string, 0, sizeof(string));
	GetWindowText(ModemWnd[3], string, sizeof(string));
	if (strncmp(ModemUserName, string,sizeof(string)) != 0){
		return 1;
	}

	memset(string, 0, sizeof(string));
	GetWindowText(ModemWnd[4], string, sizeof(string));
	if (strncmp(ModemPassWord, string,sizeof(string)) != 0){
		return 1;
	}

	memset(string, 0, sizeof(string));
	GetWindowText(ModemWnd[5], ipaddr[0], sizeof(ipaddr[0]));
	GetWindowText(ModemWnd[6], ipaddr[1], sizeof(ipaddr[1]));
	GetWindowText(ModemWnd[7], ipaddr[2], sizeof(ipaddr[2]));
	GetWindowText(ModemWnd[8], ipaddr[3], sizeof(ipaddr[3]));
	if (CheckIP(ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3], 1) || 
		((strcmp(ipaddr[0], "0")==0) && (strcmp(ipaddr[1], "0")==0) && (strcmp(ipaddr[2], "0")==0) && (strcmp(ipaddr[3], "0")==0))) {
		n = sprintf(string, "%d.%d.%d.%d", atoi(ipaddr[0]), atoi(ipaddr[1]), atoi(ipaddr[2]), atoi(ipaddr[3]));
		string[n] = '\0';
		if (strncmp(HeartBeatServer, string, sizeof(string)) != 0) {
			return 1;
		}
	}

	return 0;
}

static void SaveModemSetting(HWND hWnd)
{
	int n;
	char string[MAX_NUM_LEN];
	char ipaddr[4][4];

	gOptions.ModemEnable = SendMessage(ModemWnd[0], CB_GETCURSEL, 0, 0);

	if (module_type != MODEM_MODULE_EM660) {
		memset(string, 0, sizeof(string));
		GetWindowText(ModemWnd[1], string, sizeof(string));
		SaveStr("APN", string, 1);
	}

	memset(string, 0, sizeof(string));
	GetWindowText(ModemWnd[2], string, sizeof(string));
	SaveStr("ModemDialNumber", string, 1);

	memset(string, 0, sizeof(string));
	GetWindowText(ModemWnd[3], string, sizeof(string));
	SaveStr("ModemUserName", string, 1);

	memset(string, 0, sizeof(string));
	GetWindowText(ModemWnd[4], string, sizeof(string));
	SaveStr("ModemPassword", string, 1);

	GetWindowText(ModemWnd[5], ipaddr[0], sizeof(ipaddr[0]));
	GetWindowText(ModemWnd[6], ipaddr[1], sizeof(ipaddr[1]));
	GetWindowText(ModemWnd[7], ipaddr[2], sizeof(ipaddr[2]));
	GetWindowText(ModemWnd[8], ipaddr[3], sizeof(ipaddr[3]));

	if (CheckIP(ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3], 1) 
		|| ((strcmp(ipaddr[0], "0")==0) && (strcmp(ipaddr[1], "0")==0) &&(strcmp(ipaddr[2], "0")==0) && (strcmp(ipaddr[3], "0")==0))) {
		n = sprintf(string, "%d.%d.%d.%d", atoi(ipaddr[0]), atoi(ipaddr[1]), atoi(ipaddr[2]), atoi(ipaddr[3]));
		string[n] = '\0';
		SaveStr("HeartBeatServer", string, 1);
	}

	SaveOptions(&gOptions);

        reload_modem_configuration();
        modem_reconnect();
        if (gOptions.ModemEnable == 0) {
                modem_thread_exit();
        } else {
                modem_thread_init();
        }
	
	MessageBox1 (hWnd ,LoadStrByID(HID_RESTART) ,LoadStrByID(HIT_RUN),MB_OK | MB_ICONINFORMATION);
}

static void InitModemWin(HWND hWnd)
{
	int rows = 0;
	int item = 0;
	HWND tmpstatic[9];
	char ipaddr[4][4];
	char staticbuf[20];

	module_type = LoadInteger("ModemModule", MODEM_MODULE_Q24PL);
	memset(ModemAPN, 0x00, sizeof(ModemAPN));
	memset(ModemDialNumber, 0x00, sizeof(ModemDialNumber));
	memset(ModemUserName, 0x00, sizeof(ModemUserName));
	memset(ModemPassWord, 0x00, sizeof(ModemPassWord));
	memset(HeartBeatServer, 0x00, sizeof(HeartBeatServer));

	if (module_type != MODEM_MODULE_EM660) {
		LoadStr("APN", ModemAPN);
	}
	LoadStr("ModemDialNumber", ModemDialNumber);
	LoadStr("ModemUserName", ModemUserName);
	LoadStr("ModemPassword", ModemPassWord);
	LoadStr("HeartBeatServer", HeartBeatServer);
	
	//tmpstatic[0] = CreateWindow(CTRL_STATIC, LoadStrByID(1192), WS_VISIBLE | SS_LEFT, 0xbf, 10, rows * 30 + 12, 100, 23, hWnd, 0);
	tmpstatic[0] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_GPRS_CONNECT), WS_VISIBLE | SS_LEFT, 0xbf, 10, rows * 30 + 12, 100, 23, hWnd, 0);
	ModemWnd[0] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, 0xd0, 115, rows * 30 + 6, 55, 23, hWnd, 0);
	SendMessage(ModemWnd[0], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_NO));
	SendMessage(ModemWnd[0], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_YES));
	SendMessage(ModemWnd[0], CB_SETCURSEL, gOptions.ModemEnable, 0);
	rows++;

	if (module_type != MODEM_MODULE_EM660){
		tmpstatic[1] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_GPRS_APN), WS_VISIBLE | SS_LEFT, 0xc0, 10, rows * 30 + 12, 100, 23, hWnd, 0);
		ModemWnd[1] = CreateWindow(CTRL_SLEDIT, ModemAPN, WS_VISIBLE | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, 0xd0, 115, rows * 30 + 8, 120, 24, hWnd, 0);
		SendMessage(ModemWnd[1], MAX_NUM_LEN, 0, 0);
		rows++;
	}

	tmpstatic[2] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_GPRS_NMB), WS_VISIBLE | SS_LEFT, 0xc1, 10, rows * 30 + 12, 100, 23, hWnd, 0);
	ModemWnd[2] = CreateWindow(CTRL_SLEDIT, ModemDialNumber, WS_VISIBLE | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, 0xd1, 115, rows*30 + 8, 120, 24, hWnd, 0);
	SendMessage(ModemWnd[2], MAX_NUM_LEN, 0, 0);
	rows++;

	tmpstatic[3] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_GPRS_USER), WS_VISIBLE | SS_LEFT, 0xc2, 10, rows * 30 + 12, 100, 23, hWnd, 0);
	ModemWnd[3] = CreateWindow(CTRL_SLEDIT, ModemUserName, WS_VISIBLE | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, 0xd2, 115, rows * 30 + 8, 120, 24, hWnd, 0);
	SendMessage(ModemWnd[3], MAX_NUM_LEN, 0, 0);
	rows++;

	tmpstatic[4] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_GPRS_PWD), WS_VISIBLE | SS_LEFT, 0xc3, 10, rows * 30 + 12, 100, 23, hWnd, 0);
	ModemWnd[4] = CreateWindow(CTRL_SLEDIT, ModemPassWord, WS_VISIBLE | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, 0xd3, 115, rows * 30 + 8, 120, 24, hWnd, 0);
	SendMessage(ModemWnd[4], MAX_NUM_LEN, 0, 0);
	rows++;

	tmpstatic[5] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_MODEM_HTSERVER), WS_VISIBLE | SS_LEFT, 0xc4, 10, rows * 30 + 12, 100, 23, hWnd, 0);
	ModemWnd[5] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE |  ES_AUTOSELECT| ES_BASELINE | WS_BORDER, 0xd4, 115,  rows * 30 + 8, 35, 24, hWnd, 0);
	SendMessage(ModemWnd[rows + item], 3, 0, 0);

	tmpstatic[6] = CreateWindow(CTRL_STATIC, ".", WS_VISIBLE | SS_LEFT, 0xc5, 150, rows * 30 + 12, 10, 23, hWnd, 0);
	ModemWnd[6] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, 0xd5, 160, rows * 30 + 8, 35, 24, hWnd, 0);
	SendMessage(ModemWnd[rows + item], 3, 0, 0);

	tmpstatic[7] = CreateWindow(CTRL_STATIC, ".", WS_VISIBLE | SS_LEFT, 0xc6, 195, rows * 30 + 12, 10, 23, hWnd, 0);
	ModemWnd[7] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, 0xd6, 205, rows * 30 + 8, 35, 24, hWnd, 0);
	SendMessage(ModemWnd[rows + item], 3, 0, 0);

	tmpstatic[8] = CreateWindow(CTRL_STATIC, ".", WS_VISIBLE | SS_LEFT, 0xc7, 245, rows * 30 + 12, 10, 23, hWnd, 0);
	ModemWnd[8] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, 0xd7, 250, rows * 30 + 8, 35, 24, hWnd, 0);
	SendMessage(ModemWnd[rows + item], 3, 0, 0);
	rows++;
	if(ParseIP(HeartBeatServer, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3])){
		SetWindowText(ModemWnd[5], ipaddr[0]);
		SetWindowText(ModemWnd[6], ipaddr[1]);
		SetWindowText(ModemWnd[7], ipaddr[2]);
		SetWindowText(ModemWnd[8], ipaddr[3]);
	}
	SendMessage(ModemWnd[5],EM_LIMITTEXT,3,0);
	SendMessage(ModemWnd[6],EM_LIMITTEXT,3,0);
	SendMessage(ModemWnd[7],EM_LIMITTEXT,3,0);
	SendMessage(ModemWnd[8],EM_LIMITTEXT,3,0);

	sprintf(staticbuf, "%s", LoadStrByID(1504));
	ModemWnd[9] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_OK), WS_VISIBLE | BS_DEFPUSHBUTTON, IDOK, 5, 185, 85, 23, hWnd, 0);
	ModemWnd[10] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_CANCEL), WS_VISIBLE | BS_DEFPUSHBUTTON, IDCANCEL, 103+gOptions.ControlOffset, 185, 85, 23, hWnd, 0);
	ModemWnd[11] = CreateWindow(CTRL_BUTTON, staticbuf, WS_VISIBLE | BS_DEFPUSHBUTTON, IDCANCEL, 202+gOptions.ControlOffset, 185, 115, 23, hWnd, 0);

}


static int ModemWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	switch (message){
		case MSG_CREATE:
			InitModemWin(hWnd);
			ModemCurWnd = 0;
			SetFocusChild(ModemWnd[ModemCurWnd]);
			UpdateWindow(hWnd, TRUE);
			break;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*)lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;
				if(hdc == 0){
					hdc = GetClientDC(hWnd);
					fGetDC = TRUE;
				}

				if(clip){
					rcTemp = *clip;
					ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient(hWnd, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect(hdc, &rcTemp);
				}

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, &ModemBkg);
				if(fGetDC){
					ReleaseDC(hdc);
				}

				return 0;
			}

		case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			EndPaint(hWnd, hdc);
			return 0;

		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(gOptions.KeyPadBeep)
				ExKeyBeep();

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN){
				if (ModemCurWnd == 11) {
					ModemCurWnd = 0;
				} else if (ModemCurWnd == 0 && module_type == MODEM_MODULE_EM660) {
					ModemCurWnd += 2;
				} else if (4 < ModemCurWnd && ModemCurWnd < 9) {
					ModemCurWnd = 9;
				} else {
					ModemCurWnd++;
				}
				SetFocusChild(ModemWnd[ModemCurWnd]);
				return 0;
			}

			if (LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT || LOWORD(wParam) == SCANCODE_CURSORBLOCKRIGHT ||
					(gOptions.TFTKeyLayout == 3 && LOWORD(wParam)  == SCANCODE_BACKSPACE)){
				if (ModemCurWnd == 0){
					if (SendMessage(ModemWnd[ModemCurWnd], CB_GETCURSEL, 0, 0)==0)
						SendMessage(ModemWnd[ModemCurWnd], CB_SETCURSEL, 1, 0);
					else
						SendMessage(ModemWnd[ModemCurWnd], CB_SETCURSEL, 0, 0);
					return 0;
				}

				if (4 < ModemCurWnd && ModemCurWnd < 8 && LOWORD(wParam) == SCANCODE_CURSORBLOCKRIGHT){
					ModemCurWnd += 1;
					SetFocusChild(ModemWnd[ModemCurWnd]);
				}

				if (5 < ModemCurWnd && ModemCurWnd < 9 && LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT){
					ModemCurWnd -= 1;
					SetFocusChild(ModemWnd[ModemCurWnd]);
				}

				if ((ModemCurWnd == 9 || ModemCurWnd == 10) && LOWORD(wParam) == SCANCODE_CURSORBLOCKRIGHT){
					ModemCurWnd += 1;
					SetFocusChild(ModemWnd[ModemCurWnd]);
					return 0;
				}

				if ((ModemCurWnd == 10 || ModemCurWnd == 11) && LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT){
					ModemCurWnd -= 1;
					SetFocusChild(ModemWnd[ModemCurWnd]);
					return 0;
				}
				if (ModemCurWnd == 9 && LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT){
					ModemCurWnd = 11;
					SetFocusChild(ModemWnd[ModemCurWnd]);
					return 0;
				}
				if (ModemCurWnd == 11 && LOWORD(wParam) == SCANCODE_CURSORBLOCKRIGHT){
					ModemCurWnd = 9;
					SetFocusChild(ModemWnd[ModemCurWnd]);
					return 0;
				}
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP){
				if (ModemCurWnd == 0) {
					ModemCurWnd = 11;
				} else if (ModemCurWnd == 2 && module_type == MODEM_MODULE_EM660) {
					ModemCurWnd -= 2;
				} else if(ModemCurWnd == 9) {
					ModemCurWnd = 5;
				} else if (4 < ModemCurWnd && ModemCurWnd < 9) {
					ModemCurWnd = 4;
				} else {
					ModemCurWnd--;
				}
				SetFocusChild(ModemWnd[ModemCurWnd]);
				return 0;
			}


			if (LOWORD(wParam)==SCANCODE_F9 || ((gOptions.TFTKeyLayout == 0 || 
							gOptions.TFTKeyLayout == 4) && (LOWORD(wParam) == SCANCODE_F11))) {
				if ((ModemCurWnd > 1 && ModemCurWnd < 5) || (module_type != MODEM_MODULE_EM660 && ModemCurWnd == 1)) {
					T9IMEWindow(hWnd, 0, 200,gOptions.LCDWidth, gOptions.LCDHeight, 0);
				}
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_F8){
				CreateModemInfoWindow(hWnd);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_ENTER){
				if (ModemCurWnd == 9){
					PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
				}

				if (ModemCurWnd == 10){
					PostMessage(hWnd, MSG_COMMAND, IDCANCEL, 0);
				}

				if (ModemCurWnd == 11){
					CreateModemInfoWindow(hWnd);
				}

				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_ESCAPE){
				PostMessage(hWnd, MSG_COMMAND, IDCANCEL, 0);
			}

			if (LOWORD(wParam)==SCANCODE_MENU){
				PostMessage(hWnd, MSG_COMMAND, IDOK, 0);
				return 0;
			}

			break;

		case MSG_COMMAND:
			switch(LOWORD(wParam)){
				case IDOK:
					if(isModemSettingChanged()){
						if(!ismenutimeout){
							SaveModemSetting(hWnd);
							PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
					}else{
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
					break;

				case IDCANCEL:
					if(isModemSettingChanged()){
						if(MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
						{
							if(!ismenutimeout){
								SaveModemSetting(hWnd);
								PostMessage(hWnd, MSG_CLOSE, 0, 0);
							}
						}
						else
						{
							if(!ismenutimeout)
								PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
					}else{
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
					break;

			}
			break;

		case MSG_CLOSE:
			UnloadBitmap(&ModemBkg);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);

}

int CreateModemWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_MODEM_MOBILENET);
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ModemWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	if (LoadBitmap(HDC_SCREEN, &ModemBkg, GetBmpPath("submenubg.jpg"))){
		return 0;
	}

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID){
		return 0;
	}

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hMainWnd);

	return 0;
}


