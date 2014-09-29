#ifndef _TFTMSGBOX_H_
#define _TFTMSGMOX_H_

int  MessageBox1 (HWND hParentWnd, const char* pszText,
                      const char* pszCaption, DWORD dwStyle);
int myMessageBox1 (HWND hwnd, DWORD dwStyle, char* title, char* text, ...);
HWND createStatusWin1 (HWND hParentWnd, int width, int height,
            char* title, char* text, ...);
void destroyStatusWin1(HWND hwnd);

#endif
