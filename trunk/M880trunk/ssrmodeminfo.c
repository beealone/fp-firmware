#include <stdio.h>
#include <string.h>
#include <net/ppp_defs.h>
#include <net/if.h>
#include <net/if_ppp.h>
#include <netinet/in.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include "ssrpub.h"
#include "options.h"
#include "modem/modem.h"

//BITMAP ModemInfoBkg;
#define IDC_TIMER_FRESH 1000

static int ModemInfoWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int ret = -1;
	char buffer[64] = {0};
	char ipaddr[18] = {0};
	int in_bytes;
	int out_bytes;
	int module_type, tmpvalue = 0;

	switch (message){
		case MSG_CREATE:
			//LoadBitmap(HDC_SCREEN, &ModemInfoBkg, GetBmpPath("submenubg.jpg"));
			UpdateWindow(hWnd, TRUE);
			SetTimer(hWnd, IDC_TIMER_FRESH, 1);
			return 0;

		case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
			SetTextColor(hdc,PIXEL_lightwhite);
			SelectFont(hdc,gfont1);
			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

			TextOut(hdc, 10, 20,LoadStrByID(MID_MODEM_REGSTATUS));
			if (get_modem_current_state() == MODEM_STATE_CONNECTED){
				TextOut(hdc, 120, 20, LoadStrByID(MID_GPRS_ON));				
				module_type = LoadInteger("ModemModule", MODEM_MODULE_Q24PL);
				TextOut(hdc, 10, 50,LoadStrByID(MID_PT_MODE));
				if (module_type == MODEM_MODULE_Q24PL) {	
				TextOut(hdc, 120, 50,LoadStrByID(MID_GPRS_TYPE1));
				} else if (module_type == MODEM_MODULE_EM660)  {
				switch (get_modem_mode()) {
					case MODEM_HDRMODE_NONE:
						TextOut(hdc,  120,  50, LoadStrByID(MID_MODEM_NOSERVICE));
						break;

					case MODEM_HDRMODE_CDMA:
						TextOut(hdc, 120, 50,LoadStrByID(MID_MODEM_CDMA20001X));
						break;

					case MODEM_HDRMODE_HDR:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_CDMA2000EVDO));
						break;

					case MODEM_HDRMODE_HYBRID:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_MIXMODE));
						break;

					default:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_UNKNOWE));	
						break;
				}
			} else {
				switch (get_modem_mode()) {
					case MODEM_MODE_NONE:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_NOSERVICE));
						break;

					case MODEM_MODE_GSM:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_GSM));
						break;

					case MODEM_MODE_EDGE:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_EDGE));
						break;

					case MODEM_MODE_WCDMA:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_WCDMA));
						break;

					case MODEM_MODE_HSDPA:
						TextOut(hdc, 120, 50, LoadStrByID(MODEM_MODE_HSDPA));
						break;

					case MODEM_MODE_HSUPA:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_HSUPA));
						break;

					case MODEM_MODE_HSUPAHSDPA:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_HSDPAHSUPA));
						break;

					case MODEM_MODE_TDSCDMA:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_TDSCDMA));
						break;

					default:
						TextOut(hdc, 120, 50, LoadStrByID(MID_MODEM_UNKNOWE));	
						break;
				}
			}

			ret = modem_stats(&in_bytes, &out_bytes, ipaddr);
			if (ret == 0){
				TextOut(hdc, 10, 80,LoadStrByID(MID_NET_IP));
				TextOut(hdc, 120, 80, ipaddr);

				memset(buffer, 0x00, sizeof(buffer));
				TextOut(hdc, 10, 110, LoadStrByID(MID_MID_PPP_FLOW));
				sprintf(buffer, "%s %u bytes",  LoadStrByID(MID_MODEM_RX), in_bytes);
				TextOut(hdc, 120, 110, buffer);
				sprintf(buffer, "%s %u bytes", LoadStrByID(MID_MODEM_TX), out_bytes);
				TextOut(hdc, 120, 140, buffer);
			}

			ret = modem_connect_time();
			if(ret >= 0){
				TextOut(hdc, 10, 170,LoadStrByID(MID_MODEM_ONLINETIME));
				memset(buffer, 0x00, sizeof(buffer));
				sprintf(buffer, "%u %s %u %s %u %s %u %s", (ret / (3600 * 24)) % 365, LoadStrByID(MID_MODEM_DAY), (ret / 3600) % 24, LoadStrByID(MID_MODEM_HOUR), (ret / 60) % 60, LoadStrByID(MID_MINUTE), ret % 60, LoadStrByID(HIT_DATE8));
				TextOut(hdc, 120, 170, buffer);
			}

		} else if (get_modem_current_state() == MODEM_STATE_UNAVAILABLE) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_UNAVAILABE));
		} else if(get_modem_current_state() == MODEM_STATE_INITIALIZING) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_INITING));
		} else if(get_modem_current_state() == MODEM_STATE_INIT_FAILED) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_INITFAIL));
		} else if(get_modem_current_state() == MODEM_STATE_READY) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_MODULE_READY));
		} else if(get_modem_current_state() == MODEM_STATE_SIM_NOT_READY) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_SIM_UNREADY));
		} else if (get_modem_current_state() == MODEM_STATE_SIM_ABSENT) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_SIM_UNDETECTE));
		} else if (get_modem_current_state() == MODEM_STATE_SIM_LOCKED) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_SIM_LOCED));
		} else if (get_modem_current_state() == MODEM_STATE_SIM_READY) {
			TextOut(hdc, 120, 20,LoadStrByID(MID_MODEM_SIM_READY));
		} else if (get_modem_current_state() == MODEM_STATE_CONNECTING) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_CONNECTING));
		} else if (get_modem_current_state() == MODEM_STATE_DISCONNECT) {
			TextOut(hdc, 120, 20, LoadStrByID(MID_GPRS_DISCONNECTED));
		} else {
			TextOut(hdc, 120, 20, LoadStrByID(MID_MODEM_UNKNOWNSTATUS));
		}
		
			EndPaint(hWnd, hdc);
			return 0;

		case MSG_KEYDOWN:
			if(wParam == SCANCODE_ESCAPE) {
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&ModemInfoBkg);
			KillTimer(hWnd, IDC_TIMER_FRESH);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			break;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateModemInfoWindow(HWND hWnd)
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
	CreateInfo.MainWindowProc = ModemInfoWinProc;
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

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hMainWnd);

	return 0;
}


