/*short key manage window*/
/*liming*/

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
#include "options.h"
#include "flashdb.h"
#include "ssrpub.h"
#include "ssrcommon.h"
#include "main.h"
#include <minigui/tftmullan.h>

#define IDC_LISTVIEW 90

HWND hSTKEYGVWnd;
BITMAP stkeybmp1, stkeybmp2, stkeybmp3;
PLOGFONT stkeyfont;
int selkeyindex = 0;

static void CreateListView(HWND hWnd)//ok
{
	int i;
	LVCOLUMN lvcol;
	char* colnames[4];

	colnames[0]=LoadStrByID(MID_WKCDKEY);
	colnames[1]=LoadStrByID(MID_SHORTKEYFUN);
	colnames[2]=LoadStrByID(MID_WKCDCODE);
	colnames[3]=LoadStrByID(MID_WKCDNAME);

	//hSTKEYGVWnd = CreateWindowEx(CTRL_LISTVIEW,"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_BORDER,WS_EX_NONE,IDC_LISTVIEW,0,0,340+gOptions.ControlOffset,220,hWnd,0);
	hSTKEYGVWnd = CreateWindowEx(CTRL_LISTVIEW,"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_BORDER,WS_EX_NONE,IDC_LISTVIEW,0,0,gOptions.LCDWidth,gOptions.LCDHeight-25,hWnd,0);
	SendMessage(hSTKEYGVWnd, LVM_SETHEADHEIGHT,22,0);
	SendMessage(hSTKEYGVWnd, SVM_SETSCROLLPAGEVAL,0,192);

	for (i=0;i<4;i++)
	{
		lvcol.nCols=i;
		lvcol.pszHeadText = colnames[i];
		switch(i)
		{
			case 0:
				lvcol.width=90+gOptions.GridWidth;
				break;
			case 1:
				lvcol.width=97+gOptions.GridWidth;
				break;
			case 2:
				lvcol.width=40+gOptions.GridWidth;
				break;
			case 3:
				lvcol.width=88+gOptions.GridWidth;
				break;
		}

		lvcol.pfnCompare = NULL;
		lvcol.colFlags = 0;
		SendMessage (hSTKEYGVWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
	}
}

char keynamestr[24];
char keyfunstr[24];
static char* getKeyFun(int idx)
{
	memset(keyfunstr, 0, sizeof(keyfunstr));
#ifdef IKIOSK
	sprintf(keyfunstr, "%s",  LoadStrByID(MID_IKIOSK+idx));
#else
	switch (idx)
	{
		case 0:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN1));
			break;
		case 1:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN2));
			break;
		case 2:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN3));
			break;
		case 3:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN4));
			break;
		case 4:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN5));
			break;
		case STK_FUN:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN6));
			break;
		case STK_VERIFY:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_ONE_FACE));
			break;
		case STK_GMATCH:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_G_FACE));
			break;
		case STK_GROUP1:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN10));
			break;
		case STK_GROUP2:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN11));
			break;
		case STK_GROUP3:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN12));
			break;
		case STK_GROUP4:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN13));
			break;
		case STK_GROUP5:
			sprintf(keyfunstr, "%s", LoadStrByID(MID_STKEYFUN14));
			break;

	}
#endif
	return keyfunstr;
}

static char* getKeyName(int idx)
{
	memset(keynamestr, 0, sizeof(keynamestr));
	if(gOptions.TFTKeyLayout!=3)
	{
		sprintf(keynamestr, "%s", LoadStrByID(MID_SHORTKEY01+idx));

		if (gOptions.TFTKeyLayout!=0 && gOptions.TFTKeyLayout!=4 && (idx==8 || idx==9))
		{
			memset(keynamestr, 0, sizeof(keynamestr));
			if (idx==8)
				sprintf(keynamestr, "%s", LoadStrByID(MID_WKCDKEY71));
			else if (idx==9)
				sprintf(keynamestr, "%s", LoadStrByID(MID_WKCDKEY72));
		}
	}
	else
	{
		switch(idx)	//iMF4快捷键名称
		{
			case 2:
				sprintf(keynamestr, "%s", LoadStrByID(MID_WKCDKEY73));        //ESC
				break;
			case 3:
				sprintf(keynamestr, "%s", LoadStrByID(MID_SHORTKEY12));       //Up
				break;
			case 5:
				sprintf(keynamestr, "%s", LoadStrByID(MID_SHORTKEY13));       //Down
				break;
			case 4:
				sprintf(keynamestr, "%s", LoadStrByID(MID_WKCDKEY74));        //Enter
				break;
			case 0:
				sprintf(keynamestr, "%s", LoadStrByID(MID_SHORTKEY11));       //Back
				break;
			case 6:
				sprintf(keynamestr, "%s", LoadStrByID(MID_WKCDKEY75));        //0
				break;
			case 1:
				sprintf(keynamestr, "%s", LoadStrByID(MID_SHORTKEY15));       //Right
				break;
		}
	}
	return keynamestr;
}

static int SetLVData(HWND hWnd, int itemindex,int keyid, int optMod)
{
	TSHORTKEY tmpstkey;
	LVITEM item;
	LVSUBITEM subdata;
	HLVITEM hitem;
	char tmpstr[40];

	memset(&tmpstkey,0,sizeof(TSHORTKEY));
	if(FDB_GetShortKey(keyid,&tmpstkey)==NULL) return 0;

	item.nItemHeight = 24;
	item.nItem = itemindex;

	if(optMod)	//新建列表
		hitem = SendMessage(hWnd, LVM_ADDITEM, 0, (LPARAM)&item);
	else
		hitem = SendMessage(hWnd, LVM_GETSELECTEDITEM,0,0);

	subdata.nItem = item.nItem;

	subdata.subItem = 0;
	subdata.flags = 0;
	subdata.pszText = getKeyName(itemindex);
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

	subdata.subItem = 1;
	subdata.flags = 0;
	subdata.pszText = getKeyFun(tmpstkey.keyFun);
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

	subdata.subItem = 2;
	subdata.flags = 0;
	if(tmpstkey.keyFun==1)
	{
		memset(tmpstr,0,20);
		sprintf(tmpstr,"%d",tmpstkey.stateCode);
		subdata.pszText = tmpstr;
	}
	else
		subdata.pszText = "";
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

	subdata.subItem = 3;
	subdata.flags = 0;
	if(tmpstkey.keyFun==1)
	{
		memset(tmpstr,0,40);
		Str2UTF8(tftlocaleid,(unsigned char*)tmpstkey.stateName, (unsigned char*)tmpstr);

		subdata.pszText = tmpstr;
	}
	else
		subdata.pszText = "";
	subdata.nTextColor = 0;
	SendMessage(hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

	return 0;
}

extern HWND createStatusWin1 (HWND hParentWnd, int width, int height,char* title, char* text, ...);
extern void destroyStatusWin1(HWND hwnd);
static void refreshLVBox(HWND hWnd)//ok
{
	int i;
	TSHORTKEY tmpstkey;

	SendMessage(hWnd, MSG_FREEZECTRL, TRUE, 0);

	if(SendMessage(hWnd,LVM_GETITEMCOUNT,0,0)>0) SendMessage(hWnd, LVM_DELALLITEM,0,0);

	for(i=0;i<STKEY_COUNT;i++)
	{
		memset(&tmpstkey,0,sizeof(TSHORTKEY));
		if(FDB_GetShortKey(i+1,&tmpstkey)!=NULL)
		{
			SetLVData(hWnd,i,tmpstkey.keyID,1);
		}
	}

	SendMessage(hWnd, MSG_FREEZECTRL, FALSE, 0);
	SendMessage(hWnd,SVM_SETCURSEL,selkeyindex,TRUE);

}

extern int processscrollview(HWND listwnd, int down , int incseed);
static int STKeyWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char hintchar[20];
	HWND statushWnd;
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_CREATE:
			if (gfont1==NULL)
				stkeyfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR,
						FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL,
						FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);

			CreateListView(hWnd);
			statushWnd = createStatusWin1(hWnd,250,50,LoadStrByID(MID_APPNAME), LoadStrByID(MID_WAIT));
			refreshLVBox(hSTKEYGVWnd);		//加载ShortKey数据
			destroyStatusWin1(statushWnd);

			SetFocusChild(hSTKEYGVWnd);
			selkeyindex=0;
			SendMessage(hSTKEYGVWnd, LVM_CHOOSEITEM, selkeyindex, 0);
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			SetBkColor(hdc,0x00FFA2BE);
			if (gfont1==NULL)
				SelectFont(hdc,stkeyfont);
			else 	SelectFont(hdc,gfont1);
			SetTextColor(hdc,PIXEL_lightwhite);

			if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s", LoadStrByID(MID_PAGEUP));
				else
					sprintf(hintchar,"%s:", LoadStrByID(MID_PAGEUP));
				TextOut(hdc,7,224,hintchar);
				FillBoxWithBitmap (hdc, 60, 222,16, 16, &stkeybmp1);	//pageup
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s", LoadStrByID(MID_PAGEDOWN));
				else
					sprintf(hintchar,"%s:", LoadStrByID(MID_PAGEDOWN));
				TextOut(hdc,90,224,hintchar);
				FillBoxWithBitmap (hdc, 140, 222,16, 16, &stkeybmp2);	//pagedown
			}
			memset(hintchar,0,20);
			if (fromRight)
				sprintf(hintchar,"%s", LoadStrByID(MID_EDIT));
			else
				sprintf(hintchar,"%s:", LoadStrByID(MID_EDIT));
			TextOut(hdc,245,224,hintchar);
			FillBoxWithBitmap (hdc, 285, 222,24, 16, &stkeybmp3);

			EndPaint(hWnd,hdc);
			return 0;

		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
			if (gOptions.KeyPadBeep)	ExKeyBeep();

			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_ENTER || LOWORD(wParam)==SCANCODE_F10)	//edit shortkey
			{
				selkeyindex = SendMessage(hSTKEYGVWnd,SVM_GETCURSEL,0,0);
				if(CreateShortKeyMngWindow(hWnd,selkeyindex+1))
					SetLVData(hSTKEYGVWnd,selkeyindex,selkeyindex+1,0);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_F12)		//page down
			{
				SendMessage(hSTKEYGVWnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				processscrollview(hSTKEYGVWnd,1,7);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_F11)		//page up
			{
				SendMessage(hSTKEYGVWnd,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				processscrollview(hSTKEYGVWnd,0,7);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP || LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				PostMessage(hSTKEYGVWnd,MSG_KEYDOWN,wParam,0L);
				return 0;
			}

			break;

		case MSG_CLOSE:
			UnloadBitmap(&stkeybmp1);
			UnloadBitmap(&stkeybmp2);
			UnloadBitmap(&stkeybmp3);
			if (gfont1==NULL)
				DestroyLogFont(stkeyfont);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateShortKeyWindow(HWND hOwner)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	hOwner = GetMainWindowHandle (hOwner);
	InitMiniGUIExt();

	CreateInfo.dwStyle = WS_VISIBLE ;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = STKeyWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;

	LoadBitmap(HDC_SCREEN,&stkeybmp1,GetBmpPath("pageup.gif"));
	LoadBitmap(HDC_SCREEN,&stkeybmp2,GetBmpPath("pagedown.gif"));
	if(gOptions.TFTKeyLayout!=3)
		LoadBitmap(HDC_SCREEN,&stkeybmp3,GetBmpPath("okkey.gif"));
	else
		LoadBitmap(HDC_SCREEN,&stkeybmp3,GetBmpPath("fun2.gif"));

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

	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif
