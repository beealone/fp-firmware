/*alarm manage window*/
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
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

#define IDC_LISTVIEW 10

HWND hBellGVWnd;
static int g_colcount = 4;		//列数
int selPIN = 0;				//当前选择的行
BITMAP alarmbmp1, alarmbmp2, alarmbmp3, alarmbmp4, alarmbmp5;
PLOGFONT alarmfont;

static void CreateListView(HWND hWnd)
{
	int i;
	LVCOLUMN lvcol;
	char* colnames[4];

	colnames[0]=LoadStrByID(MID_ALARM);
	colnames[1]=LoadStrByID(MID_ALARMTIME);
	colnames[2]=LoadStrByID(MID_ALARMWAVE);
	colnames[3]=LoadStrByID(MID_STATUS);

	hBellGVWnd = CreateWindowEx (CTRL_LISTVIEW, "alarm View", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER, WS_EX_NONE,
				IDC_LISTVIEW, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight-25, hWnd, 0);


	SendMessage(hBellGVWnd,LVM_SETHEADHEIGHT,20,0);
	SendMessage(hBellGVWnd,SVM_SETSCROLLPAGEVAL,0,192);

	for (i=0;i<g_colcount;i++)
	{
		lvcol.nCols=i;
		lvcol.pszHeadText = colnames[i];
		switch(i)
		{
			case 0:
				lvcol.width=65+gOptions.GridWidth;
				break;
			case 1:
				lvcol.width=80+gOptions.GridWidth;
				break;
			case 2:
				lvcol.width=110+gOptions.GridWidth;
				break;
			case 3:
				lvcol.width=45+gOptions.GridWidth;
				break;
		}

		lvcol.pfnCompare = NULL;
		lvcol.colFlags = 0;
		SendMessage (hBellGVWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
	}
}

static void SetLVData(HWND hWnd, int itemindex, int opmode)
{
	LVITEM item;
	LVSUBITEM subdata;
	HLVITEM hitem;
	U8 alarmstatus;
	ALARM alarm1;
	TBellNew bell;			//new bell
	char itemstr[20];

	item.nItemHeight = 24;
	if(opmode==0)
	{
		item.nItem = itemindex;
		hitem = SendMessage (hWnd, LVM_ADDITEM, 0, (LPARAM)&item);
	}
	else
	{
		hitem = SendMessage(hWnd,LVM_GETSELECTEDITEM,0,0);
		SendMessage(hWnd,LVM_GETITEM,hitem,(LPARAM)&item);
	}

	subdata.nItem = item.nItem;
	if (gOptions.UseNewBell)
	{
		memset(&bell, 0, sizeof(TBellNew));
		if (FDB_GetBell(itemindex+1, &bell)!=NULL)
		{
			memset(itemstr, 0, 20);
			sprintf(itemstr, "%s%d", LoadStrByID(MID_ALARM), bell.BellID);
			subdata.subItem = 0;
			subdata.flags = 0;
			subdata.pszText = itemstr;
			subdata.nTextColor = 0;
			SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

			memset(itemstr, 0, 20);
			subdata.subItem = 1;
			subdata.flags = 0;
			sprintf(itemstr,"%02d:%02d", bell.BellTime[0], bell.BellTime[1]);
			subdata.pszText = itemstr;
			subdata.nTextColor = 0;
			SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

			memset(itemstr, 0, 20);
			sprintf(itemstr, "bell%02d.wav", bell.BellWaveIndex+1);
			subdata.subItem = 2;
			subdata.flags = 0;
			subdata.pszText = itemstr;
			subdata.nTextColor = 0;
			SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

			subdata.subItem = 3;
			subdata.flags = LVFLAG_BITMAP;
			subdata.pszText = NULL;
			alarmstatus = (bell.SchInfo>>7)&1;
			if(alarmstatus ==1) subdata.image = (DWORD)&alarmbmp1;
			else subdata.image = 0;
			SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);
		}
	}
	else
	{
		memset(&alarm1, 0, sizeof(ALARM));
		if(FDB_GetAlarm(itemindex+1,&alarm1)!=NULL)
		{
			memset(itemstr,0,20);
			sprintf(itemstr,"%s%d",LoadStrByID(MID_ALARM),alarm1.AlarmIDX);
			subdata.subItem = 0;
			subdata.flags = 0;
			subdata.pszText = itemstr;
			subdata.nTextColor = 0;
			SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

			memset(itemstr,0,20);
			subdata.subItem = 1;
			subdata.flags = 0;
			sprintf(itemstr,"%02d:%02d",alarm1.AlarmHour,alarm1.AlarmMin);
			subdata.pszText = itemstr;
			subdata.nTextColor = 0;
			SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

			memset(itemstr,0,20);
			sprintf(itemstr,"bell%02d.wav",alarm1.AlarmWaveIDX+1);//
			subdata.subItem = 2;
			subdata.flags = 0;
			subdata.pszText = itemstr;
			subdata.nTextColor = 0;
			SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

			subdata.subItem = 3;
			subdata.flags = LVFLAG_BITMAP;
			subdata.pszText = NULL;
			alarmstatus = alarm1.AlarmStatus;
			if(alarmstatus ==1) subdata.image = (DWORD)&alarmbmp1;
			else subdata.image = 0;

			SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);
		}
	}
}

extern int CheckSameTime(int h, int m, int pin);
extern int CheckBellSetError(PBellNew pbell);
static void ChgAlarmStatus(HWND hWnd,int PIN)
{
	int h,m;
	ALARM alarm2;
	TBellNew bell;

	if (gOptions.UseNewBell)
	{
		memset(&bell, 0, sizeof(TBellNew));
		if (FDB_GetBell(PIN+1, &bell)!=NULL)
		{
			if (CheckBellSetError(&bell))
			{
				selPIN = SendMessage(hBellGVWnd, SVM_GETCURSEL, 0, 0);
				if (CreateBellEditWindow(hWnd, &selPIN));
					SetLVData(hBellGVWnd, selPIN, 1);
			}
			else
			{
				if (((bell.SchInfo>>7)&1)==0) bell.SchInfo|=0x80;
				else bell.SchInfo&=0x7F;
				if (FDB_ChgBell(&bell)==FDB_ERROR_IO)
					printf("change alarm status failed!\n");
			}
		}
	}
	else
	{
		if(FDB_GetAlarm(PIN+1,&alarm2)!=NULL)
		{
			h=alarm2.AlarmHour;
			m=alarm2.AlarmMin;
			if(CheckSameTime(h,m,alarm2.AlarmIDX) && alarm2.AlarmStatus==0)
			{
				MessageBox1(hWnd,LoadStrByID(MID_TIMEERROR), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
				selPIN =  SendMessage(hBellGVWnd,SVM_GETCURSEL,0,0);
				if(CreateAlarmSetingWindow(hWnd,&selPIN)==1);
					SetLVData(hBellGVWnd, selPIN, 1);
			}
			else
			{
				if(alarm2.AlarmStatus == 0) alarm2.AlarmStatus=1;
				else alarm2.AlarmStatus=0;
				if(FDB_ChgAlarm(&alarm2)==FDB_ERROR_IO)
					printf("change alarm status failed!\n");
			}
		}
	}
}

extern int processscrollview(HWND listwnd, int down , int incseed);
static int AlarmMngWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int i;
	char hintchar[20];
	HWND statushWnd;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			LoadBitmap(HDC_SCREEN,&alarmbmp1,GetBmpPath("speaker.gif"));
			LoadBitmap(HDC_SCREEN,&alarmbmp2,GetBmpPath("pageup.gif"));
			LoadBitmap(HDC_SCREEN,&alarmbmp3,GetBmpPath("pagedown.gif"));
			LoadBitmap(HDC_SCREEN,&alarmbmp4,GetBmpPath("okkey.gif"));
			if(gOptions.TFTKeyLayout!=3)
				LoadBitmap(HDC_SCREEN,&alarmbmp5,GetBmpPath("function.gif"));
			else
				LoadBitmap(HDC_SCREEN,&alarmbmp5,GetBmpPath("fun2.gif"));

			if (gfont1==NULL)
				alarmfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
							FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);

			CreateListView(hWnd);

			statushWnd = createStatusWin1(hWnd , 250 , 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(MID_WAIT));
	        	SendMessage (hBellGVWnd, MSG_FREEZECTRL, TRUE, 0);
			if (gOptions.UseNewBell)
			{
				for(i=0;i<FDB_CntBell();i++)
					SetLVData(hBellGVWnd, i, 0);

			}
			else
			{
				for(i=0;i<FDB_CntAlarm();i++)
					SetLVData(hBellGVWnd, i, 0);
			}
			SendMessage (hBellGVWnd, MSG_FREEZECTRL, FALSE, 0);
			destroyStatusWin1(statushWnd);

			SetFocusChild(hBellGVWnd);
			SendMessage(hBellGVWnd,LVM_CHOOSEITEM,0,0);
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			SetBkColor(hdc,0x00FFA2BE);
			if (gfont1==NULL)
				SelectFont(hdc,alarmfont);
			else	SelectFont(hdc,gfont1);

			SetTextColor(hdc,PIXEL_lightwhite);
			if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEUP));
				else
					sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEUP));
				TextOut(hdc,6,220,hintchar);
				FillBoxWithBitmap (hdc, 60, 218,16, 16, &alarmbmp2);
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEDOWN));
                                else
					sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEDOWN));
				TextOut(hdc,86,220,hintchar);
				FillBoxWithBitmap (hdc, 138, 218,16, 16, &alarmbmp3);
			}
			if(gOptions.TFTKeyLayout!=3)
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_SETTING));
                                else
					sprintf(hintchar,"%s:",LoadStrByID(MID_SETTING));
				TextOut(hdc,155,220,hintchar);
				FillBoxWithBitmap (hdc, 216, 218,16, 16, &alarmbmp4);

				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_ALARMSTART));
                                else
					sprintf(hintchar,"%s:",LoadStrByID(MID_ALARMSTART));
				TextOut(hdc,245,220,hintchar);
				FillBoxWithBitmap (hdc, 292, 219,24, 16, &alarmbmp5);
			}
			else
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_SETTING));
                                else
					sprintf(hintchar,"%s:",LoadStrByID(MID_SETTING));
				TextOut(hdc,225,220,hintchar);
				FillBoxWithBitmap (hdc, 292, 218,24, 16, &alarmbmp5);
			}
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
			if (gOptions.KeyPadBeep) ExKeyBeep();

			if ( LOWORD(wParam)==SCANCODE_ESCAPE )
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
			if (LOWORD(wParam)==SCANCODE_MENU)
			{
				selPIN = SendMessage(hBellGVWnd,SVM_GETCURSEL,0,0);
				ChgAlarmStatus(hWnd,selPIN);
				SetLVData(hBellGVWnd, selPIN, 1);
			}

			if (LOWORD(wParam)==SCANCODE_ENTER || LOWORD(wParam)==SCANCODE_F10)
			{
				int bret = 0;
				selPIN=SendMessage(hBellGVWnd,SVM_GETCURSEL,0,0);
				if (gOptions.UseNewBell)
				{
					if (FDB_GetBell(selPIN+1, NULL)!=NULL)
					{
						bret = CreateBellEditWindow(hWnd, &selPIN);
					}
				}
				else
				{
					if(FDB_GetAlarm(selPIN+1,NULL)!=NULL)
					{
						bret = CreateAlarmSetingWindow(hWnd,&selPIN);
					}
				}
				if (bret) SetLVData(hBellGVWnd, selPIN, 1);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_F12)	//page down
			{
	                        SendMessage(hBellGVWnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
	                        processscrollview(hBellGVWnd,1,7);
			}

			if (LOWORD(wParam)==SCANCODE_F11)	//page up
			{
	                        SendMessage(hBellGVWnd,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
	                        processscrollview(hBellGVWnd,0,7);
			}
			break;

		case MSG_CLOSE:
			UnloadBitmap(&alarmbmp1);
			UnloadBitmap(&alarmbmp2);
			UnloadBitmap(&alarmbmp3);
			UnloadBitmap(&alarmbmp4);
			UnloadBitmap(&alarmbmp5);
			if (gfont1==NULL)
				DestroyLogFont(alarmfont);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);

}

int CreateArlarmManageWindow(HWND hOwner)
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
    //CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = AlarmMngWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = gOptions.LCDWidth;
    CreateInfo.by = gOptions.LCDHeight;
    CreateInfo.iBkColor = 0x00FFA2BE;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = hOwner;

    hMainWnd = CreateMainWindow (&CreateInfo);

    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    MiniGUIExtCleanUp ();
    return 0;
}

