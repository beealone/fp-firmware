#ifndef __CWX_GUI_COMMON_H_
#define __CWX_GUI_COMMON_H_

#define _HAVE_TYPE_BYTE
#define _HAVE_TYPE_WORD
#define _HAVE_TYPE_DWORD
#define _HAVE_TYPE_LONG
#define IDC_TESTINFO   0xff00ff
#undef ROP_XOR	//add by jazzy 2009.06.06

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"
#include "exfun.h"
#include "options.h"
#include "flashdb.h"
#include "usb_helper.h"

int uPic;
char uPicList[16][16];
char StateStr[6][32];
char KeyStr[7][32];
char SpeedStr[3][32];
char SwitchStr[2][32];
char FreeSetStr[2][32];
char dhcpstr[2][32];

typedef struct _tag_State
{
        char Name[16];
        int Key;
}
State;
State gKeyState[6];

time_t gMenuTimeOut;
int busyflag;

char *GetAttName(int attstate);
long GetKeyState(int i);
void GetKeyStateName(short i,char *info);
void InfoShowStr(HDC hdc,char *name,char *text,int x,int y);
void InfoShow(HDC hdc,char *name,long num,int x,int y,int sd);
long CheckText(char *str);
long CheckText2(char *str,int ul,int dl);
long CheckIP(char *c1,char *c2,char *c3,char *c4,int fag);
long ParseIP(char *buffer,char *c1,char *c2,char *c3,char *c4);
unsigned long Delay(unsigned long sec);
char *LoadFirmware(char *FirmwareFile, char *Version, int *Length);
int UpdateFirmware(char *filename);
void RESETDEFAULT(HWND hWnd);
void SetMenuTimeOut(time_t timer);

void SetStateStr(void);
void SetKeyStr(void);
void SetSpeedStr(void);
void SetSwitchStr(void);
void SetFreeSetStr(void);
void SetMenuTimeOut(time_t timer);

int GetUpdatingFirmeareErrorNo(void);

#endif
