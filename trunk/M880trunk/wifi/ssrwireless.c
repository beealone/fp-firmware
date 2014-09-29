#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "utils.h"
#include "options.h"
#include "newwireless.h"
#include "ssrwireless.h"
#include "thread.h"
#include "interface.h"

#define ICON_PADDING_LEFT 30
#define ICON_PADDING_TOP 80
#define IPADDRESS_STRING_SIZ 16
#define PASSWORD_MAXSIZ 512

static int wireless_online_flag = 0;
static int wireless_reconncet_flag = 0;
static int wireless_start_flag = 0;
static char wireless_ipaddr[IPADDRESS_STRING_SIZ];
static char wireless_mask[IPADDRESS_STRING_SIZ];
static char wireless_gateway[IPADDRESS_STRING_SIZ];
static char wireless_passwd[PASSWORD_MAXSIZ];
static char wireless_ssid[PACKET_SSID_SIZ] = {0};
static pthread_mutex_t *wireless_lock = NULL;

static void Draw_Rectangle(HDC hdc,int x1,int y1,int x2,int y2)
{
	LineEx(hdc,x1,y1,x2,y1);
	LineEx(hdc,x2,y1,x2,y2);
	LineEx(hdc,x2,y2,x1,y2);
	LineEx(hdc,x1,y2,x1,y1);
}

void show_wireless_status_icon(HWND hWnd, int LCDWidth)
{
	HDC hdc;
	RECT rect;
	int tmpvalue = 0;
	hdc = GetClientDC(hWnd);

	gal_pixel old_TextColor = GetTextColor(hdc);
	gal_pixel old_pen_color = GetPenColor(hdc);
	gal_pixel old_BKColor = GetBkMode(hdc);
	PLOGFONT  old_font = GetCurFont(hdc);
	PTCapStyle old_style = GetPenCapStyle(hdc);
	unsigned int old_pen_width = GetPenWidth(hdc);

	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
	tmpvalue = SetPenCapStyle(hdc, PT_CAP_ROUND);
	SetPenWidth(hdc, 2);

	if (wireless_online_flag == 1) {
		SetTextColor(hdc, PIXEL_green);
		SetPenColor(hdc, PIXEL_green);
		SetBrushColor(hdc, PIXEL_green);
	} else {
		SetTextColor(hdc, PIXEL_darkgray);
		SetPenColor(hdc, PIXEL_darkgray);
		SetBrushColor(hdc, PIXEL_darkgray);
	}

	rect.left = LCDWidth-105;
	rect.top = 10;
	rect.right = rect.left-17;
	rect.bottom = rect.top+15;

	Draw_Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	DrawTextEx(hdc,"W", 1, &rect, 0,  DT_CENTER |DT_SINGLELINE | DT_VCENTER );

	SetTextColor (hdc,old_TextColor);
	SelectFont(hdc, old_font);
	SetPenColor(hdc,old_pen_color);
	SetPenWidth(hdc,old_pen_width);
	tmpvalue = SetPenCapStyle(hdc,old_style);
	tmpvalue = SetBkMode(hdc, old_BKColor);

	ReleaseDC(hdc);
}

void wireless_load_configuration(void)
{
	memset(wireless_ipaddr, 0x00, sizeof(wireless_ipaddr));
	memset(wireless_mask, 0x00, sizeof(wireless_mask));
	memset(wireless_gateway, 0x00, sizeof(wireless_gateway));
	memset(wireless_passwd, 0x00, sizeof(wireless_passwd));
	memset(wireless_ssid, 0x00, sizeof(wireless_ssid));
	LoadStr("wifiip",  wireless_ipaddr);
	LoadStr("wifimask", wireless_mask);
	LoadStr("wifigateway", wireless_gateway);
	wireless_start_flag = LoadStr("SSID",  wireless_ssid);
	LoadStr("wifipasswd", wireless_passwd);
}

int is_wireless_interface_up(void)
{
	return is_interface_up(WIRELESS_INTERFACE);
}

int wirelss_interface_up(void)
{
	return interface_up(WIRELESS_INTERFACE);
}

int wireless_get_ipaddr(unsigned int *addr)
{
	return interface_get_addr(WIRELESS_INTERFACE, addr);

}

int wireless_set_ipaddr(char *addr)
{
	return interface_set_addr(WIRELESS_INTERFACE, string_to_ipaddr(addr));
}

int wireless_get_mask(unsigned int *mask)
{
	return interface_get_mask(WIRELESS_INTERFACE, mask);
}

int wireless_set_mask(char *mask)
{
	return interface_set_mask(WIRELESS_INTERFACE, string_to_ipaddr(mask));
}

int wireless_remove_default_route(void)
{
	return interface_remove_default_route(WIRELESS_INTERFACE);
}

int wireless_get_default_route(unsigned int *gw)
{
	return interface_get_default_route(WIRELESS_INTERFACE, gw);
}

int wireless_add_default_route(char *addr)
{
	return interface_add_host_route(WIRELESS_INTERFACE, string_to_ipaddr(addr));
}

int is_wireless_connected(char *ssid, char *bssid)
{
	return wireless_stat(WIRELESS_INTERFACE, ssid, bssid);
}

iw_list* wireless_scan_node(void)
{
	iw_list *list = NULL;

	mutexP(wireless_lock);
	list = new_wireless_scan(WIRELESS_INTERFACE);
	mutexV(wireless_lock);

	return list;
}

int wireless_connect_to(iw_list *list, char *ssid, char *passwd)
{
	return wireless_connect(list, WIRELESS_INTERFACE, ssid, passwd);
}

void wireless_free_scan_node(iw_list *list)
{
	return wireless_free_results(list);
}

void wireless_request_reconnect(void)
{
	wireless_reconncet_flag = 1;
}

void wireless_exec_dhcpc(void)
{
	int pid = -1;
	struct stat st;
	char tmp[64];

	sprintf(tmp, "/var/run/udhcpc-%s.pid", WIRELESS_INTERFACE);
	if (stat(tmp, &st) == 0) {
		return;
	}

	{
		char *dhcp_argv[] = {"udhcpc",
			"-i", WIRELESS_INTERFACE,
			"-p", tmp,
			NULL
		};

		_eval(dhcp_argv, ">/dev/console", 0, &pid);
	}
}

void wireless_kill_dhcpc(void)
{
	char tmp[64];
	sprintf(tmp, "/var/run/udhcpc-%s.pid", WIRELESS_INTERFACE);
	kill_pidfile(tmp);
}

static int wireless_enable = 1;
void *wireless_mainloop_handle(void * arg)
{
	char ssid_conn[PACKET_SSID_SIZ] = {0};
	char bssid_conn[PACKET_BSSID_SIZ] = {0};
	iw_list *scan_list = NULL;

	wireless_enable = 1;
	while (wireless_enable) {
		if (is_wireless_interface_up() == 0) {
			wirelss_interface_up();
		}
		if (wireless_start_flag) {
			memset(ssid_conn, 0x00, sizeof(ssid_conn));
			memset(bssid_conn, 0x00, sizeof(bssid_conn));
			wireless_online_flag = is_wireless_connected(ssid_conn, bssid_conn);
		} else {
			wireless_online_flag = -1;
		}
		if (wireless_online_flag == 0 || wireless_reconncet_flag == 1 || strcmp(ssid_conn, wireless_ssid) != 0) {
			scan_list = wireless_scan_node();
			if (scan_list != NULL) {
				if (wireless_connect(scan_list, WIRELESS_INTERFACE, wireless_ssid, wireless_passwd) == 0) {
					if (gOptions.wifidhcpfunon == 0) {
						wireless_kill_dhcpc();
						wireless_set_ipaddr(wireless_ipaddr);
						wireless_set_mask(wireless_mask);
						wireless_remove_default_route();
						wireless_add_default_route(wireless_gateway);
					} else {
						wireless_exec_dhcpc();
					}
				}
				wireless_free_scan_node(scan_list);
				scan_list = NULL;
			}
			if (wireless_reconncet_flag == 1) {
				wireless_reconncet_flag = 0;
			}
		}
		sleep(10);
	}

	mutex_destroy(wireless_lock);
	thread_exit();

	return NULL;
}

int wireless_thread_create(void)
{
	pthread_t *wifi_thread_id = NULL;
	wireless_lock = mutex_init();
	if (wireless_lock == NULL) {
		return -1;
	}

	wifi_thread_id = thread_create(wireless_mainloop_handle, NULL);
	if (wifi_thread_id==NULL) {
		return -2;
	} else {
		return 0;
	}
}

int wireless_is_online(void)
{
	return ((wireless_online_flag > 0) ? 1: 0); 
}

void wireless_exit_thread(void)
{
	wireless_enable = 0;
}
