#include <stdio.h>
#include <string.h>
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
#include "exfun.h"
#include "t9.h"
#include "ssrwireless.h"
#include "interface.h"
#include "utils.h"
#include "tftmsgbox.h"

#define WIFI_STATIC     1000
#define WIFI_SSID       1010
#define WIFI_PWD        1011
#define WIFI_DHCPIP     1012
#define WIFI_ENCRYP     1013
#define WIFI_SAVE       1040
#define WIFI_EXIT       1041

HWND lable_state[3];
HWND WifiWnd[17];
static HWND WififHwnd;
BITMAP wifibkg;
char* WifiStr[10];
int curFocusWnd;

char oldip[3][18];
char newip[3][18];			//保存IP设置
char oldpwdid[2];
char newpwdid[2];
WifiPWD oldpwd[4];
WifiPWD newpwd[4];
char oldwpapsk[255];
char newwpapsk[255];
extern char DHCPIPAdress[18];
int wifidhcpflag=0;
int netmode, authmode, encrytype;
char my_ssid[128];

//int OptMod_wifi;

#if 0
static void enable_ip_control(unsigned int flag)
{
	int i;
	for (i=3; i<15; i++) {
		SendMessage(WifiWnd[i], MSG_ENABLE, flag, 0);/*flag:0-disable, 1-enable*/
	}
}
#endif

static void LoadWindowStr(void)
{
	int i=0;
	WifiStr[i++]=LoadStrByID(MID_WIFI_ID);		//网络识别ID
	WifiStr[i++]=LoadStrByID(MID_WIFI_PWDSET);	//password
	WifiStr[i++]=LoadStrByID(MID_LOCAL_IP);         //ip setting mode(dhcp or fix ip)
	WifiStr[i++]=LoadStrByID(MID_NET_IP);         	//ip address
	WifiStr[i++]=LoadStrByID(MID_NETMASK_ADDR);     //netmask
	WifiStr[i++]=LoadStrByID(MID_GATEWAY_IP);       //gateway ip
	WifiStr[i++]=LoadStrByID(MID_WIFI_MANUIP);      //手动分配
	WifiStr[i++]=LoadStrByID(MID_NET_DHCP);         //DHCP
	WifiStr[i++]=LoadStrByID(MID_SAVE);             //保存
	WifiStr[i++]=LoadStrByID(MID_EXIT);             //退出
}

static void InitWifiWindow (HWND hWnd)
{
	int posX1,posX2,posX3,posX4,posX5;
	int y1=10;
	int y2=6;
	int id=0;
	int iw=0;
	int i=0;
	int j=0;

	if (fromRight==1) {
		posX1=220+gOptions.GridWidth*2;
		posX2=220+gOptions.GridWidth*2;
		//posX3=110+gOptions.GridWidth*2;
		posX3=10;
		posX4=10+gOptions.GridWidth*2;
		posX5=10;
	} else {
		posX1=10;
		posX2=20;
		posX3=101;
		posX4=220;
		posX5=110;
	}
	/*create lable*/
	//网络识别ID
	CreateWindow(CTRL_STATIC,WifiStr[0],WS_VISIBLE|SS_LEFT,WIFI_STATIC,posX1,y1,100,23,hWnd,0);
	//passwd
	y1+=30;
	CreateWindow(CTRL_STATIC,WifiStr[1],WS_VISIBLE|SS_LEFT,WIFI_STATIC+1,posX1, y1,100,23,hWnd,0);
	//ip setting mode
	y1+=30;
	CreateWindow(CTRL_STATIC,WifiStr[2],WS_VISIBLE|SS_LEFT,WIFI_STATIC+2,posX1,y1,100,23,hWnd,0);
	//ip
	y1+=30;
	lable_state[0] = CreateWindow(CTRL_STATIC,WifiStr[3],WS_VISIBLE|SS_LEFT,WIFI_STATIC+3,posX1,y1,100,23,hWnd,0);
	//mask
	y1+=30;
	lable_state[1] = CreateWindow(CTRL_STATIC,WifiStr[4],WS_VISIBLE|SS_LEFT,WIFI_STATIC+4,posX1,y1,100,23,hWnd,0);
	//gateway
	y1+=30;
	lable_state[2] = CreateWindow(CTRL_STATIC,WifiStr[5],WS_VISIBLE|SS_LEFT,WIFI_STATIC+5,posX1,y1,100,23,hWnd,0);

	/*create input box and button*/
	//ssid
	WifiWnd[i++]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,WIFI_SSID,posX3,6,200,23,hWnd,0);
	//passwd
	y2+=30;
	WifiWnd[i++]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,WIFI_PWD,posX3,y2,200,23,hWnd,0);
	//dhcp
	y2+=30;
	WifiWnd[i]=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS|CBS_NOTIFY,
			WIFI_DHCPIP,posX3,y2,100,23,hWnd,0);
	SendMessage(WifiWnd[i], CB_ADDSTRING, 0, (LPARAM)WifiStr[6]);
	SendMessage(WifiWnd[i], CB_ADDSTRING, 0, (LPARAM)WifiStr[7]);
	SendMessage(WifiWnd[i],CB_SETCURSEL, gOptions.wifidhcpfunon, 0);
	i++;

	//ip, submask,gateway
	y2+=30;
	for (j=1; j<13; j++) {
		WifiWnd[i++]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
					WIFI_ENCRYP + id++, posX3 + iw*50, y2, 50, 23, hWnd, 0);
		iw++;
		if ((j % 4) == 0) {
			y2+=30;
			iw = 0;
		}
	}

	WifiWnd[i++]=CreateWindow(CTRL_BUTTON,WifiStr[8],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,
			WIFI_SAVE,10,190,85,23,hWnd,0);
	WifiWnd[i++]=CreateWindow(CTRL_BUTTON,WifiStr[9],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,
			WIFI_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);

	for (i=3; i<15; i++) {
		SendMessage(WifiWnd[i], EM_LIMITTEXT, 3, 0);
	}
}

static void dhcpswitch(int value)
{
	int i;
	for(i=0; i<12; i++)
	{
		if(value){
			SetWindowBkColor(WifiWnd[3+i], PIXEL_lightgray);
		}
		else {
			SetWindowBkColor(WifiWnd[3+i], PIXEL_lightwhite);
		}
	}
}

static void InitParameters(void)
{
	char cfg[128];
	char ip[4][16];
	char wifiippass[128];
	int i;
	unsigned int addr = 0;
	unsigned int mask = 0;
	unsigned int gateway = 0;

	memset(cfg, 0x00, sizeof(cfg));
	if (my_ssid[0]) {
		SetWindowText(WifiWnd[0], my_ssid);
	} else if (LoadStr("SSID", cfg)) {
		SetWindowText(WifiWnd[0], cfg);
	}
	LoadStr("wifipasswd", wifiippass);
	SetWindowText(WifiWnd[1], wifiippass);

	memset(oldip, 0, sizeof(oldip));
	memset(newip, 0, sizeof(newip));

	if (gOptions.wifidhcpfunon) {
		dhcpswitch(1);
		if (wireless_is_online()) {
			//printf("_____%s%d\n", __FILE__, __LINE__);
			unsigned char ip[4];
			memset(ip, 0, sizeof(ip));
			wireless_get_ipaddr(&addr);
			str2ip((char*)ipaddr_to_string(addr), (BYTE*)ip);
			sprintf((char*)oldip[0], "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			printf("dynaic wifi: %d:%d:%d:%d\n", ip[0], ip[1], ip[2], ip[3]);

			memset(ip, 0, sizeof(ip));
			wireless_get_mask(&mask);
			str2ip(ipaddr_to_string(mask), (BYTE*)ip);
			sprintf((char*)oldip[1], "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

			memset(ip, 0, sizeof(ip));
			wireless_get_default_route(&gateway);
			str2ip(ipaddr_to_string(gateway), (BYTE*)ip);
			sprintf((char*)oldip[2], "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
		}
	} else {
		LoadStr("wifiip", (char*)oldip[0]);
		LoadStr("wifimask", (char*)oldip[1]);
		LoadStr("wifigateway", (char*)oldip[2]);
	}

	memcpy(newip, oldip, sizeof(oldip));
	//set ip
	if(ParseIP(newip[0],ip[0],ip[1],ip[2],ip[3])){
		for(i=0;i<4;i++){
			if (strlen(ip[i])>0) {
				SetWindowText(WifiWnd[3+i],ip[i]);
			} else {
				SetWindowText(WifiWnd[3+i],"0");
			}
		}
	}
	//set sub mask
	if(ParseIP(newip[1],ip[0],ip[1],ip[2],ip[3])){
		for(i=0;i<4;i++){
			SetWindowText(WifiWnd[7+i],ip[i]);
		}
	}

	//set Gate Way
	if(ParseIP(newip[2],ip[0],ip[1],ip[2],ip[3])){
		for(i=0;i<4;i++){
			SetWindowText(WifiWnd[11+i],ip[i]);
		}
	}
}

static int get_new_ip(void)
{
	char str[3][50];
	char cc[3][16];
	int err=0;
	int i;
	if(wifidhcpflag) {
		return 1;
	}
	for(i=0;i<4;i++){
		GetWindowText(WifiWnd[3+i],cc[i],16);
	}
	if(CheckIP(cc[0],cc[1],cc[2],cc[3],1)) {
		sprintf(str[0],"%s.%s.%s.%s",cc[0],cc[1],cc[2],cc[3]);
	} else {
		err=1;
	}

	for(i=0;i<4;i++) {
		GetWindowText(WifiWnd[7+i],cc[i],16);
	}
	if(CheckIP(cc[0],cc[1],cc[2],cc[3],0)) {
		sprintf(str[1],"%s.%s.%s.%s",cc[0],cc[1],cc[2],cc[3]);
	} else {
		err=2;
	}

	for(i=0;i<4;i++) {
		GetWindowText(WifiWnd[11+i],cc[i],16);
	}
	if(CheckIP(cc[0],cc[1],cc[2],cc[3],0)) {
		sprintf(str[2],"%s.%s.%s.%s",cc[0],cc[1],cc[2],cc[3]);
	} else {
		err=3;
	}

	if(err) {
		switch(err)
		{
			case 1:
				MessageBox1(WififHwnd ,LoadStrByID(HIT_NETERROR1) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
				break;
			case 2:
				MessageBox1(WififHwnd ,LoadStrByID(HIT_NETERROR2) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                                break;
			case 3:
				MessageBox1(WififHwnd ,LoadStrByID(HIT_NETERROR3) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                                break;
			default:
				MessageBox1(WififHwnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                                break;
		}
		return 0;
	} else {
		memset(newip, 0, 54);
		sprintf(newip[0],"%s",str[0]);
		sprintf(newip[1],"%s",str[1]);
		sprintf(newip[2],"%s",str[2]);
	}
	return 1;
}
static int isModified(void)
{
	char cfg[128];
	char tmpstring[128];
	char wifiippass[128];
	//int idx=0;
	int changeflag = 0;


	memset(cfg, 0x00, sizeof(cfg));
	memset(tmpstring, 0, sizeof(tmpstring));
	LoadStr("SSID", cfg);
	GetWindowText(WifiWnd[0], tmpstring, sizeof(tmpstring));
	if(strncmp(cfg, tmpstring, sizeof(tmpstring))!=0) {
		changeflag = 1;
	}

	memset(tmpstring, 0, sizeof(tmpstring));
	if(SendMessage(WifiWnd[2], CB_GETCURSEL, 0, 0)!=gOptions.wifidhcpfunon) {
		changeflag = 1;
	}

	if(get_new_ip() ==0) {
		return 0;
	}
	
	if(strncmp(oldip[0], newip[0], sizeof(newip[0]))!=0){
		changeflag = 1;
	}

	if(strncmp(oldip[1], newip[1], sizeof(newip[1]))!=0) {
		changeflag = 1;
	}

	if(strncmp(oldip[2], newip[2], sizeof(newip[2]))!=0) {
		changeflag = 1;
	}

	memset(tmpstring, 0, sizeof(tmpstring));
	memset(wifiippass, 0, sizeof(wifiippass));
	LoadStr("wifipasswd", wifiippass);
	GetWindowText(WifiWnd[1], tmpstring, sizeof(tmpstring));
	if(strncmp(wifiippass, tmpstring, strlen(tmpstring)) != 0) {
		changeflag = 1;
	}

	return changeflag;
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int SaveNewWiFiSetting(HWND hWnd)
{
	char tmpstring[128];
	//int idx=0;

	memset(tmpstring, 0, sizeof(tmpstring));
	GetWindowText(WifiWnd[0], tmpstring, sizeof(tmpstring));
	if(tmpstring && strlen(tmpstring)>0){
		SaveStr("SSID", tmpstring, 1);
	} else {
		MessageBox1(hWnd, LoadStrByID(MID_WIFI_ERR1), LoadStrByID(HIT_RUN), MB_OK | MB_ICONSTOP);
		curFocusWnd = 0;
		SetFocusChild(WifiWnd[curFocusWnd]);	
		return 0;
	}

	//wifipasswd
	memset(tmpstring, 0, sizeof(tmpstring));
	GetWindowText(WifiWnd[1], tmpstring, sizeof(tmpstring));
	if (tmpstring[0]) {
		SaveStr("wifipasswd", tmpstring, 1);
		printf("wifi passwd:%s\n", tmpstring);
	}

	//ip assign mode: dhcp or fix
	if (wifidhcpflag != gOptions.wifidhcpfunon) {
		gOptions.wifidhcpfunon = wifidhcpflag;
		SaveInteger("wifidhcp", wifidhcpflag);
	}

	SaveStr("wifiip", (char*)newip[0], 1);
	SaveStr("wifimask", (char*)newip[1], 1);
	SaveStr("wifigateway", (char*)newip[2], 1);

	wireless_load_configuration();
	wireless_request_reconnect();
	return 1;
}

extern HWND hIMEWnd;
static int NewWiFiWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_CREATE:
			wifidhcpflag = gOptions.wifidhcpfunon;
			InitWifiWindow(hWnd);		//add controls
			InitParameters();

			curFocusWnd =0;
			SetFocusChild(WifiWnd[curFocusWnd]);
			UpdateWindow(hWnd,TRUE);
			break;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*)lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;
				if(hdc == 0) {
					hdc = GetClientDC(hWnd);
					fGetDC = TRUE;
				}

				if(clip) {
					rcTemp = *clip;
					ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient(hWnd,&rcTemp.right, &rcTemp.bottom);
					IncludeClipRect(hdc, &rcTemp);
				}

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, &wifibkg);
				if(fGetDC) {
					ReleaseDC (hdc);
				}
				return 0;
			}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);	
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout){
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if (3 == gOptions.TFTKeyLayout) {
				if (1==keyupFlag) {
					keyupFlag=0;
				} else {
					break;
				}
			}
			if (gOptions.KeyPadBeep){
				ExKeyBeep();
			}

			if ((LOWORD(wParam)==SCANCODE_ESCAPE)){
				PostMessage(hWnd, MSG_COMMAND, WIFI_EXIT, 0);
			} else 	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) {
				char tmpssid[128];
				if (curFocusWnd==0) {
					GetWindowText(WifiWnd[0], tmpssid, sizeof(tmpssid));
					if(tmpssid && strlen(tmpssid)>0) {
						if(hIMEWnd!=HWND_INVALID){
							SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
							hIMEWnd=HWND_INVALID;
						}
						curFocusWnd++;
					}
				} else if (curFocusWnd==1) {
					if(hIMEWnd!=HWND_INVALID){
						SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
						hIMEWnd=HWND_INVALID;
					}
					curFocusWnd++;
				} else if(++curFocusWnd>16) {
					curFocusWnd=0;
				}

				if (wifidhcpflag == 1 && curFocusWnd == 3) {
					curFocusWnd=15;
				}
				if (curFocusWnd == 4 || curFocusWnd == 5 ||curFocusWnd == 6 ){
					curFocusWnd = 7;
				}
				if (curFocusWnd == 8 || curFocusWnd == 9 || curFocusWnd == 10){
					curFocusWnd = 11;
				}
				if (curFocusWnd == 12 || curFocusWnd == 13 || curFocusWnd == 14){
					curFocusWnd = 15;
				}
				SetFocusChild(WifiWnd[curFocusWnd]);		
				return 0;
			} else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP){
				char tmpssid[128];
				if(curFocusWnd==0){
					GetWindowText(WifiWnd[0], tmpssid, sizeof(tmpssid));
					if(tmpssid && strlen(tmpssid)>0){
						if(hIMEWnd!=HWND_INVALID){
							SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
							hIMEWnd=HWND_INVALID;
						}
						curFocusWnd--;
					}
				} else if (curFocusWnd==1) {
					if(hIMEWnd!=HWND_INVALID){
						SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
						hIMEWnd=HWND_INVALID;
					}
					curFocusWnd--;
				
				} else if(--curFocusWnd<0){
					curFocusWnd=16;
				}

				if (wifidhcpflag == 1 && curFocusWnd == 14) {
					curFocusWnd= 2;
				}

				if (curFocusWnd == 10|| curFocusWnd == 11|| curFocusWnd == 12 ||curFocusWnd == 13 ){
					curFocusWnd = 7;
				} else if (curFocusWnd == 6|| curFocusWnd == 7|| curFocusWnd == 8 || curFocusWnd == 9){
					curFocusWnd = 3;
				} else if (curFocusWnd == 3 || curFocusWnd == 4 || curFocusWnd == 5){
					curFocusWnd = 2;
				} else if(curFocusWnd == 14){
					curFocusWnd = 11;
				}

				SetFocusChild(WifiWnd[curFocusWnd]);
				return 0;

			} else if(LOWORD(wParam)==SCANCODE_F9 || 
					((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && (LOWORD(wParam)==SCANCODE_F11))) {
				if((curFocusWnd==0 || curFocusWnd==1) && (gOptions.IMEFunOn==1 && gOptions.TFTKeyLayout!=3))
				{
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
				}
			} else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
				|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE)) {

				if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) && gOptions.IMEFunOn==1 
						&& gOptions.TFTKeyLayout==3 && (curFocusWnd==0 || curFocusWnd==1)) {
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight, 0);
					return 0;
				}

				if(curFocusWnd==2 && ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
					||LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)) {

					int tmpsel = SendMessage(WifiWnd[curFocusWnd], CB_GETCURSEL, 0, 0);		
					if (tmpsel == 0) {
						SendMessage(WifiWnd[curFocusWnd], CB_SETCURSEL, 1, 0);
						dhcpswitch(1);
						UpdateWindow(hWnd,TRUE);
						wifidhcpflag =1;
					} else {
						SendMessage(WifiWnd[curFocusWnd], CB_SETCURSEL, 0, 0);
						dhcpswitch(0);
						UpdateWindow(hWnd,TRUE);
						wifidhcpflag = 0;
					}
				}
				if (curFocusWnd >2 && curFocusWnd<15 && wifidhcpflag ==0) {
					if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT ){
						curFocusWnd --;
						if (curFocusWnd == 2){
							curFocusWnd =14;
						}
					}else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT){
						curFocusWnd ++;
						if (curFocusWnd ==15){
							curFocusWnd = 3;
						}
					}
					SetFocusChild(WifiWnd[curFocusWnd]);
				}
				return 0;	
			} else if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10)) {
				char tmpssid[128];
				switch(curFocusWnd)
				{
					case 0:
						GetWindowText(WifiWnd[0], tmpssid, sizeof(tmpssid));
						if(tmpssid && strlen(tmpssid)>0)
							SendMessage(hWnd, MSG_COMMAND, WIFI_SAVE, 0);
						else
							MessageBox1(hWnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
						break;
					case 16:		//exit
						SendMessage(hWnd, MSG_COMMAND, WIFI_EXIT, 0);
						break;
					default:
						SendMessage(hWnd, MSG_COMMAND, WIFI_SAVE, 0);
				}
				return 0;
			} else if(LOWORD(wParam)==SCANCODE_MENU) {
				SendMessage(hWnd, MSG_COMMAND, WIFI_SAVE, 0);
				return 0;
			}
			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(id==WIFI_SAVE) {
				if(isModified() && SaveNewWiFiSetting(hWnd)) {
					MessageBox1(hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
					if(!ismenutimeout) {
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
				} else if(!ismenutimeout) {
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			} else if (id==WIFI_EXIT) {
				if(isModified() && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA),LoadStrByID(MID_APPNAME),
					MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK) {
					PostMessage(hWnd, MSG_COMMAND, WIFI_SAVE, 0);
				} else if(!ismenutimeout) {
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
			break;

		case MSG_CLOSE:
			if(hIMEWnd!=HWND_INVALID){
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd=HWND_INVALID;
			}

			UnloadBitmap(&wifibkg);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateNewWiFiWindow(HWND hWnd, char* selssid)
{
	MSG msg;
	MAINWINCREATE CreateInfo;

	LoadWindowStr();
	memset(my_ssid, 0, sizeof(my_ssid));
	nstrcpy(my_ssid, selssid, strlen(selssid));

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER|WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_WIFI_SETTING);
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = NewWiFiWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	if (LoadBitmap(HDC_SCREEN,&wifibkg,GetBmpPath("submenubg.jpg"))) {
		return 0;
	}
	WififHwnd = CreateMainWindow(&CreateInfo);
	if (WififHwnd == HWND_INVALID) {
		return -1;
	}
	ShowWindow(WififHwnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,WififHwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(WififHwnd);
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

