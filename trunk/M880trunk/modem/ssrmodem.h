#ifndef _SSRMODEM_H
#define _SSRMODEM_H

#include <minigui/window.h>

void reload_modem_configuration(void);
void show_modem_status_icon(HWND hWnd, int LCDWidth);
int modem_thread_init(void);
void modem_thread_exit(void);
#endif
