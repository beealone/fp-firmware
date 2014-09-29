#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "modem.h"
#include "exfun.h"
#include "options.h"
#include "thread.h"
#include "ssrmodem.h"
#include "utils2.h"

#define ICON_PADDING_TOP 	10
#define MAX_STRING_LENGTH 	32
#define ICON_SIGNAL_WIDTH	64
#define ICON_WIDTH		8

int modemmodule = 0;
int modemband = MODEM_BAND_DEFAULT;
char modemapn[MAX_STRING_LENGTH];
char modemdialnumber[MAX_STRING_LENGTH];
char modemusername[MAX_STRING_LENGTH];
char modempassword[MAX_STRING_LENGTH];
char heartbeatserver[MAX_STRING_LENGTH];
char modemcmdport[MAX_STRING_LENGTH];
char modemdataport[MAX_STRING_LENGTH];

static int modem_enable = 0;
static pthread_t *modem_thread_id = NULL;
static int gOldModemMode = 0;

static void Draw_Rectangle(HDC hdc,int x1,int y1,int x2,int y2)
{
	LineEx(hdc,x1,y1,x2,y1);
	LineEx(hdc,x2,y1,x2,y2);
	LineEx(hdc,x2,y2,x1,y2);
	LineEx(hdc,x1,y2,x1,y1);
}

void show_modem_status_icon(HWND hWnd, int LCDWidth)
{
	int sig;
	int i = 0;
	int x = 0;//ICON_PADDING_LEFT+ICON_SIGNAL_WIDTH;
	int y = ICON_PADDING_TOP + 13;
	int mode, tmpvalue = 0;
	int module;
	unsigned int old_pen_width;
	char sign = 'G';
	modem_state_t state;

	RECT rect;
	gal_pixel old_TextColor;
	gal_pixel old_pen_color;
	gal_pixel old_BKColor;
	PTCapStyle old_style;
	PLOGFONT  old_font;
	HDC hdc = GetClientDC(hWnd);

	old_font = GetCurFont(hdc);
	old_TextColor = GetTextColor(hdc);
	old_pen_color = GetPenColor(hdc);
	old_BKColor = GetBkMode(hdc);
	old_style = GetPenCapStyle(hdc);
	old_pen_width = GetPenWidth(hdc);

	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
	tmpvalue = SetPenCapStyle(hdc, PT_CAP_ROUND);
	SetPenWidth(hdc, 2);

	state = get_modem_current_state();
	if (state == MODEM_STATE_CONNECTING || state == MODEM_STATE_CONNECTED) {
		SetTextColor(hdc, PIXEL_green);
		SetPenColor(hdc, PIXEL_green);
		SetBrushColor(hdc, PIXEL_green);
	} else {
		SetTextColor(hdc, PIXEL_darkgray);
		SetPenColor(hdc, PIXEL_darkgray);
		SetBrushColor(hdc, PIXEL_darkgray);
	}
	rect.left = LCDWidth-ICON_WIDTH*4-ICON_SIGNAL_WIDTH-4;
	rect.top = ICON_PADDING_TOP;
	rect.right = LCDWidth - ICON_WIDTH*2 - ICON_SIGNAL_WIDTH - 4;
	rect.bottom = ICON_PADDING_TOP + 15;

	mode = get_modem_mode();
	module = LoadInteger("ModemModule", MODEM_MODULE_Q24PL);

	if((module != MODEM_MODULE_Q24PL) && (mode != gOldModemMode))
	{
		gOldModemMode = mode;
		InvalidateRect (hWnd, &rect, TRUE);
	}

	if (module == MODEM_MODULE_Q24PL) {
		sign = 'G';
	} else if (module == MODEM_MODULE_EM660) {
		if (mode == MODEM_HDRMODE_CDMA) {
			sign = 'C';
		} else if (mode == MODEM_HDRMODE_HDR || mode == MODEM_HDRMODE_HYBRID) {
			sign = 'H';
		} else {
			sign = '?';
		}
	} else {
		if (mode == MODEM_MODE_GPRS) {
			sign = 'G';
		} else if (mode == MODEM_MODE_EDGE){
			sign = 'E';
		} else if (mode == MODEM_MODE_WCDMA) {
			sign = 'W';
		} else if (mode == MODEM_MODE_HSDPA || mode == MODEM_MODE_HSUPA || mode == MODEM_MODE_HSUPAHSDPA) {
			sign = 'H';
		} else if (mode == MODEM_MODE_TDSCDMA) {
			sign = 'T';
		} else {
			sign = '?';
		}
	}
	//printf("______%s%d,mode = %d, sign = %c\n", __FILE__, __LINE__, mode, sign);
	DrawTextEx(hdc, &sign, 1, &rect, 0, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	if (get_modem_current_state() == MODEM_STATE_CONNECTED) {
		SetTextColor(hdc, PIXEL_green);
		SetPenColor(hdc, PIXEL_green);
		SetBrushColor(hdc, PIXEL_green);
	} else {
		SetTextColor(hdc, PIXEL_darkgray);
		SetPenColor(hdc, PIXEL_darkgray);
		SetBrushColor(hdc, PIXEL_darkgray);
	}
	Draw_Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	if (get_modem_current_state() >= MODEM_STATE_SIM_READY) {
		SetTextColor(hdc, PIXEL_green);
		SetPenColor(hdc, PIXEL_green);
		SetBrushColor(hdc, PIXEL_green);
	} else {
		SetTextColor(hdc, PIXEL_darkgray);
		SetPenColor(hdc, PIXEL_darkgray);
		SetBrushColor(hdc, PIXEL_darkgray);
	}

	x = LCDWidth-ICON_SIGNAL_WIDTH+3;
	SetPenWidth(hdc, 3);
	LineEx(hdc, x - 20, ICON_PADDING_TOP, x - 4, ICON_PADDING_TOP);
	LineEx(hdc, x - 20, ICON_PADDING_TOP, x - 12, ICON_PADDING_TOP + 8);
	LineEx(hdc, x - 12, ICON_PADDING_TOP + 8, x - 4, ICON_PADDING_TOP);
	LineEx(hdc, x - 12, ICON_PADDING_TOP + 8, x - 12, ICON_PADDING_TOP + 14);
	sig = get_modem_signal_indicator();
	for(i = 0; i < sig; i++) {
		LineEx(hdc, x + (i * 8), y, x + (i * 8), y - (i * 3));
	}

	SetTextColor(hdc, PIXEL_darkgray);
	SetPenColor(hdc, PIXEL_darkgray);
	SetBrushColor(hdc, PIXEL_darkgray);
	for(i = sig; i < 5; i++) {
		LineEx(hdc, x + (i * 8), y, x + (i * 8), y - (i * 3));
	}

	SetTextColor(hdc, old_TextColor);
	SelectFont(hdc, old_font);
	SetPenColor(hdc, old_pen_color);
	SetPenWidth(hdc, old_pen_width);
	tmpvalue = SetPenCapStyle(hdc, old_style);
	tmpvalue = SetBkMode(hdc, old_BKColor);

	ReleaseDC(hdc);
}

static void *modem_handle_thread(void *arg)
{
	if (((strlen(modemapn) == 0) && (modemmodule != MODEM_MODULE_EM660)) || strlen(modemdialnumber) == 0) {
		return NULL;
	}

	modem_enable = 1;
	while (modem_enable) {
		modem_process(modemapn, modemdialnumber, modemusername, modempassword, heartbeatserver, modemmodule, modemband, ExModemPowerByMcu);
		msleep(100);
	}

	modem_disable();

	modem_thread_id = NULL;
	return NULL;
}

int modem_thread_init(void)
{
	if (modem_thread_id != NULL) {
		return -1;
	}

	modem_thread_id = thread_create(modem_handle_thread, NULL);
	if (modem_thread_id == NULL) {
		return -1;
	}

	return 0;
}

void modem_thread_exit(void)
{
	modem_enable = 0;
}

void reload_modem_configuration(void)
{
	memset(modemapn, 0x00, sizeof(modemapn));
	memset(modemdialnumber, 0x00, sizeof(modemdialnumber));
	memset(modemusername, 0x00, sizeof(modemusername));
	memset(modempassword, 0x00, sizeof(modempassword));
	memset(heartbeatserver, 0x00, sizeof(heartbeatserver));
	memset(modemcmdport, 0x00, sizeof(modemcmdport));
	memset(modemdataport, 0x00, sizeof(modemdataport));

	LoadStr("APN", modemapn);
	LoadStr("ModemDialNumber", modemdialnumber);
	LoadStr("ModemUserName", modemusername);
	LoadStr("ModemPassword", modempassword);
	LoadStr("HeartBeatServer", heartbeatserver);
	LoadStr("CommandSerialPort", modemcmdport);
	LoadStr("DataSerialPort", modemdataport);

	modemmodule = LoadInteger("ModemModule", MODEM_MODULE_Q24PL);
	modemband = LoadInteger("ModemBand", MODEM_BAND_DEFAULT);
}
