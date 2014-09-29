#ifndef _SSR_WIRELESS_H
#define _SSR_WIRELESS_H

#include <minigui/window.h>

#define WIRELESS_INTERFACE "rausb0"

int wireless_get_ipaddr(unsigned int *addr);
int wireless_get_mask(unsigned int *mask);
int wireless_get_default_route(unsigned int *gw);
int is_wireless_connected(char *ssid, char *bssid);
int wireless_thread_create(void);
int wireless_is_online(void);
void wireless_load_configuration(void);
void wireless_request_reconnect(void);
void wireless_exit_thread(void);

void show_wireless_status_icon(HWND hWnd, int LCDWidth);
#endif
