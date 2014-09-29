/*The new wifi use follow file
 *interface.c interface.h newwireless.c newwireless.h ssrwireless.c ssrwireless.h thread.c thread.h 
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>
#include <minigui/tftmullan.h>

#include "options.h"
#include "ssrpub.h"
#include "ssrcommon.h"

/*The new wifi module used the follow .h file*/
#include "newwireless.h"
/*end*/

/*The old wifi module(SP18870E0) used the follow .h file */
#include "iwlib.h"
#include "wireless.h"
/*end*/

#define IDC_WIFILIST	0x20

HWND hWiFiGVWnd;
PLOGFONT wfinfofont;
//static BITMAP wfinfobkg;
static BITMAP wfinfobmp[6];
BITMAP wfhintbmp1;
BITMAP wfhintbmp2;
BITMAP wfhintbmp3;
BITMAP wfhintbmp4;

static int convert_sign_qulity(int qulity)
{
	if (qulity>0 && qulity<=20){
		return 1;
	} else if(qulity>20 && qulity<=40) {
		return 2;
	} else if(qulity>40 && qulity<=60) {
		return 3;
	} else if(qulity>60 && qulity<=80) {
		return 4;
	} else if(qulity>80 && qulity<=100) {
		return 5;
	}
	return 0;
}

static void CreateListView(HWND hWnd)
{
	int i=0;
	LVCOLUMN lvcol;
	char* colnames[3];

	colnames[i++]=LoadStrByID(MID_WFINFO_SSID);
	colnames[i++]=LoadStrByID(HIT_MACINFO);
	colnames[i++]=LoadStrByID(MID_WFINFO_SINGLE);

	hWiFiGVWnd = CreateWindowEx(CTRL_LISTVIEW, "user View", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER, WS_EX_NONE,
			IDC_WIFILIST, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight-50, hWnd, 0);
	SendMessage(hWiFiGVWnd, LVM_SETHEADHEIGHT, 20, 0);
	SendMessage(hWiFiGVWnd, SVM_SETSCROLLPAGEVAL, 0, 192);

	for (i=0;i<3;i++)
	{
		lvcol.nCols=i;
		lvcol.pszHeadText = colnames[i];
		switch(i)
		{
			case 0:
				lvcol.width=120+gOptions.GridWidth;
				break;
			case 1:
				lvcol.width=150+gOptions.GridWidth;
				break;
			case 2:
				lvcol.width=50+gOptions.GridWidth*2;
				break;
		}
		lvcol.pfnCompare = NULL;
		lvcol.colFlags = 0;
		SendMessage(hWiFiGVWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
	}
}

static void SetSingleList(HWND hWnd, int itemindex, char* wifiname, char* mac, int infoValue)
{
	LVITEM item;
	LVSUBITEM subdata;
	HLVITEM subitem;

	item.nItemHeight = 28;
	item.nItem=itemindex;
	subitem = SendMessage(hWnd, LVM_ADDITEM, 0, (LPARAM)&item);
	subdata.nItem=item.nItem;

	subdata.subItem=0;
	subdata.flags=0;
	subdata.pszText=wifiname;
	subdata.nTextColor=0;
	SendMessage(hWnd, LVM_SETSUBITEM, subitem, (LPARAM)&subdata);

	subdata.subItem=1;
	subdata.flags=0;
	subdata.pszText=mac;
	subdata.nTextColor=0;
	SendMessage(hWnd, LVM_SETSUBITEM, subitem, (LPARAM)&subdata);

	subdata.subItem=2;
	subdata.pszText=NULL;
	subdata.flags|=LVFLAG_BITMAP;
	subdata.image=(DWORD)&(wfinfobmp[infoValue]);
	SendMessage(hWnd, LVM_SETSUBITEM, subitem, (LPARAM)&subdata);

}

extern int print_scanning_info(char *ifname, wireless_scan_result* result);
static void GetWifiInfo(HWND hWnd)
{
	int apcount;
	int i;
	wireless_scan_result wresult[IW_MAX_AP];
	char wifiname[128];
	char macadr[128];
	int qulity;

	if(SendMessage(hWnd, LVM_GETITEMCOUNT, 0, 0)>0) {
		SendMessage(hWnd, LVM_DELALLITEM, 0, 0);
	}

	apcount=print_scanning_info("rausb0",(wireless_scan_result*)&wresult);
	if(apcount>0) {
		for(i=0;i<apcount;i++) {
			memset(wifiname, 0, sizeof(wifiname));
			memset(macadr, 0, sizeof(macadr));
			sprintf(wifiname, "%s",wresult[i].ap_name);
			sprintf(macadr, "%s", wresult[i].ap_address);
			qulity = convert_sign_qulity(wresult[i].qualvalue);
			SetSingleList(hWnd, i, wifiname, macadr, qulity);
		}
	}
}

/*dsl 2012.4.17, for WiFi of the thread version */
static void GetWifiInfoEx(HWND hWnd)
{
	int i;
	int qulity;
	iw_list *list = NULL;
#if 1
	if (SendMessage(hWnd, LVM_GETITEMCOUNT, 0, 0)>0)
		SendMessage(hWnd, LVM_DELALLITEM, 0, 0);
#endif
	printf("del list item\n");
	list  = new_wireless_scan("rausb0");
	printf("finish scan list\n");
	if(list != NULL){
		for(i = 0; i< list->num;  i++){
			qulity=convert_sign_qulity(list->node[i].rssi);
			SetSingleList(hWnd, i, list->node[i].ssid,list->node[i].bssid, qulity);
		}
		printf("free list\n");
		wireless_free_results(list);
		list=NULL;
		printf("end free list\n");
	}
}

extern HWND createStatusWin1 (HWND hParentWnd, int width, int height,char* title, char* text, ...);
extern void destroyStatusWin1(HWND hwnd);
static int WifiInfoWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HWND stahwnd;
	char hintchar[20];
	static char keyupFlag=0;
	int tmpvalue = 0;
	switch (message)
	{
		case MSG_CREATE:
			//wireless_exit_thread();
			CreateListView(hWnd);
			UpdateWindow(hWnd,TRUE);

			stahwnd = createStatusWin1(hWnd, 250, 50, LoadStrByID(MID_APPNAME), LoadStrByID(MID_WFINFO_HINT));

			if (gOptions.WifiModule == 0 ) {
				GetWifiInfo(hWiFiGVWnd);
			} else {
				GetWifiInfoEx(hWiFiGVWnd);
			}

			destroyStatusWin1 (stahwnd);

			SetFocusChild(hWiFiGVWnd);
			SendMessage(hWiFiGVWnd, LVM_CHOOSEITEM, 0, 0);
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);

			if (gfont1==NULL)
				SelectFont(hdc, wfinfofont);
			else 	
				SelectFont(hdc,gfont1);

			SetTextColor(hdc,PIXEL_lightwhite);
			if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar, 0, 20);
				if (fromRight==1)
					sprintf(hintchar, "%s", LoadStrByID(MID_PAGEUP));
				else
					sprintf(hintchar, "%s:", LoadStrByID(MID_PAGEUP));
				TextOut(hdc, 3, 198, hintchar);
				FillBoxWithBitmap(hdc, 55, 195, 16, 16, &wfhintbmp1);
				memset(hintchar, 0, 20);
				if (fromRight==1)
					sprintf(hintchar, "%s", LoadStrByID(MID_PAGEDOWN));
				else
					sprintf(hintchar, "%s:", LoadStrByID(MID_PAGEDOWN));
				TextOut(hdc, 73, 198,hintchar);
				FillBoxWithBitmap(hdc, 125, 195, 16, 16, &wfhintbmp2);
			}
			if(gOptions.IMEFunOn)
			{
				memset(hintchar, 0, 20);
				if (fromRight==1)
					sprintf(hintchar, "%s", LoadStrByID(MID_SETTING));
				else
					sprintf(hintchar, "%s:", LoadStrByID(MID_SETTING));
				TextOut(hdc, 149, 198, hintchar);
				FillBoxWithBitmap(hdc, 210, 195, 24, 16, &wfhintbmp4);
			}

			memset(hintchar, 0, 20);
			if (fromRight==1)
				sprintf(hintchar, "%s", LoadStrByID(MID_WFINFO_REFRESH));
			else
				sprintf(hintchar, "%s:", LoadStrByID(MID_WFINFO_REFRESH));
			TextOut(hdc, 240, 198, hintchar);
			FillBoxWithBitmap(hdc, 298, 195, 16, 16, &wfhintbmp3);

			EndPaint(hWnd,hdc);
			return 0;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*)lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;
				if(hdc == 0)
				{
					hdc = GetClientDC(hWnd);
					fGetDC = TRUE;
				}

				if(clip)
				{
					rcTemp = *clip;
					ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient(hWnd,&rcTemp.right, &rcTemp.bottom);
					IncludeClipRect(hdc, &rcTemp);
				}

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());;
					if(fGetDC) ReleaseDC (hdc);
				return 0;
			}

		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout) {
				if(1==keyupFlag) {
					keyupFlag=0;
				} else {
					break;
				}
			}
			if(gOptions.KeyPadBeep)	{
				ExKeyBeep();
			}

			if(LOWORD(wParam)==SCANCODE_ESCAPE){
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
			} else if ((((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && LOWORD(wParam)==SCANCODE_MENU) 
				|| (gOptions.TFTKeyLayout == 1 && (LOWORD(wParam)==SCANCODE_MENU || LOWORD(wParam)==SCANCODE_ENTER)) 
				|| (gOptions.TFTKeyLayout == 2 && (LOWORD(wParam)==SCANCODE_MENU || LOWORD(wParam)==SCANCODE_ENTER)) 
				|| (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_ENTER))&&(gOptions.IMEFunOn)) {

				char ssidbuf[40];
				HLVITEM hItemSel;
				LVSUBITEM subItem;

				hItemSel = SendMessage(hWiFiGVWnd, LVM_GETSELECTEDITEM, 0, 0);
				if (hItemSel)
				{
					memset(ssidbuf,0,sizeof(ssidbuf));
					subItem.pszText = ssidbuf;
					subItem.subItem = 0;
					SendMessage(hWiFiGVWnd, LVM_GETSUBITEMTEXT, hItemSel, (LPARAM)&subItem);
					if (gOptions.WifiModule == 0) {
						CreateWiFiWindow(hWnd, ssidbuf);
					} else {
						CreateNewWiFiWindow(hWnd, ssidbuf);
					}
				}
				return 0;

			} else if (((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && LOWORD(wParam)==SCANCODE_ENTER) ||
					(gOptions.TFTKeyLayout == 1 && LOWORD(wParam)==SCANCODE_F10) ||
					(gOptions.TFTKeyLayout == 2 && LOWORD(wParam)==SCANCODE_F10) ||
					(gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE)) {
#if 1
				//stahwnd = createStatusWin1(hWnd, 250, 50, LoadStrByID(MID_APPNAME), LoadStrByID(MID_WFINFO_HINT));
				if (gOptions.WifiModule == 0 ) {
					GetWifiInfo(hWiFiGVWnd);
				} else {
					GetWifiInfoEx(hWiFiGVWnd);
				}
				//destroyStatusWin1(stahwnd);
				SendMessage(hWiFiGVWnd, LVM_CHOOSEITEM, 0, 0);
#endif
				return 0;

			} else 	if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)){
				return 0;

			} else 	if(LOWORD(wParam)==SCANCODE_F12) {
				SendMessage(hWiFiGVWnd, MSG_KEYDOWN, SCANCODE_PAGEDOWN, 0);
				processscrollview(hWiFiGVWnd,1,8);
				return 0;

			} else 	if(LOWORD(wParam)==SCANCODE_F11) {
				SendMessage(hWiFiGVWnd, MSG_KEYDOWN, SCANCODE_PAGEUP, 0);
				processscrollview(hWiFiGVWnd,0,8);
				return 0;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&wfinfobkg);
			UnloadBitmap(&wfhintbmp1);
			UnloadBitmap(&wfhintbmp2);
			UnloadBitmap(&wfhintbmp3);
			UnloadBitmap(&wfhintbmp4);
			UnloadBitmap(&(wfinfobmp[0]));
			UnloadBitmap(&(wfinfobmp[1]));
			UnloadBitmap(&(wfinfobmp[2]));
			UnloadBitmap(&(wfinfobmp[3]));
			UnloadBitmap(&(wfinfobmp[4]));
			UnloadBitmap(&(wfinfobmp[5]));

			if (gfont1==NULL && wfinfofont != NULL) {
				DestroyLogFont(wfinfofont);
				wfinfofont = NULL;
			}

			//MainWindowCleanup (hWnd);
			DestroyMainWindow(hWnd);

			//wireless_thread_create();
			return 0;

	}

	return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

int CreateWiFiInfoWindow(HWND hWnd)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle(hWnd);
	InitMiniGUIExt();

	SetMenuTimeOut(time(NULL));

	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_WFINFO_NAME);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WifiInfoWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//LoadBitmap(HDC_SCREEN, &wfinfobkg, GetBmpPath("mainmenu.jpg"));
	//LoadBitmap(HDC_SCREEN, &wfinfobkg, GetBmpPath("submenubg.jpg"));
	LoadBitmap(HDC_SCREEN, &wfhintbmp1, GetBmpPath("pageup.gif"));
	LoadBitmap(HDC_SCREEN, &wfhintbmp2, GetBmpPath("pagedown.gif"));
	if(gOptions.TFTKeyLayout!=3)
	{
		LoadBitmap(HDC_SCREEN, &wfhintbmp3, GetBmpPath("okkey.gif"));		//刷新
		LoadBitmap(HDC_SCREEN, &wfhintbmp4, GetBmpPath("function.gif"));	//设置
	}
	else		// for iF4
	{
		LoadBitmap(HDC_SCREEN, &wfhintbmp3, GetBmpPath("back.gif"));
		LoadBitmap(HDC_SCREEN, &wfhintbmp4, GetBmpPath("fun2.gif"));
	}
	LoadBitmap(HDC_SCREEN,&(wfinfobmp[0]), GetBmpPath("single0.gif"));
	LoadBitmap(HDC_SCREEN,&(wfinfobmp[1]), GetBmpPath("single1.gif"));
	LoadBitmap(HDC_SCREEN,&(wfinfobmp[2]), GetBmpPath("single2.gif"));
	LoadBitmap(HDC_SCREEN,&(wfinfobmp[3]), GetBmpPath("single3.gif"));
	LoadBitmap(HDC_SCREEN,&(wfinfobmp[4]), GetBmpPath("single4.gif"));
	LoadBitmap(HDC_SCREEN,&(wfinfobmp[5]), GetBmpPath("single5.gif"));
	if (gfont1==NULL)
		wfinfofont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
				FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);

	hMainWnd = CreateMainWindow (&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);
	while (GetMessage(&Msg, hMainWnd))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	MainWindowThreadCleanup (hMainWnd);
	MiniGUIExtCleanUp ();
	return 0;
}
