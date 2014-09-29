#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"
#include "exfun.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

//BITMAP mhdaybmpbg;
BITMAP mhdaybmps1;
BITMAP mhdaybmps2;
BITMAP mhdaybmps3;
BITMAP mhdaybmps4;
//menu icon
BITMAP micon_hday1;
BITMAP micon_hday2;
BITMAP micon_hday3;

#define IDL_HDLISTVIEW 0xF2
PLOGFONT hdayfont;
HWND hdayGVWnd;
int hdaycount=0;
THTimeZone thtz;

static void InitWindow(HWND hWnd)
{
        int i;
        LVCOLUMN lvcol;
	char* itemstr[3];
	itemstr[0]=LoadStrByID(MID_GP_INDEX);
	itemstr[1]=LoadStrByID(MID_HTZ_DATE);
	itemstr[2]=LoadStrByID(MID_USER_TZ);

        hdayGVWnd = CreateWindowEx (CTRL_LISTVIEW, "holiday View", WS_CHILD | WS_VISIBLE | WS_VSCROLL, WS_EX_NONE,
                                IDL_HDLISTVIEW, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight-25, hWnd, 0);
        SendMessage(hdayGVWnd,LVM_SETHEADHEIGHT,20,0);
        SendMessage(hdayGVWnd,SVM_SETSCROLLPAGEVAL,0,192);

        for (i=0;i<3;i++)
        {
                lvcol.nCols=i;
                lvcol.pszHeadText = itemstr[i];

                switch(i)
                {
                        case 0:
                                lvcol.width=45+gOptions.GridWidth;
                                break;
                        case 1:
                                lvcol.width=200+gOptions.GridWidth;
                                break;
			case 2:
				lvcol.width=75+gOptions.GridWidth;
				break;
                }

                lvcol.pfnCompare = NULL;
                lvcol.colFlags = 0;
                SendMessage (hdayGVWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
        }

}

static HMENU CreateQuickMenu(HWND hWnd)
{
        int i;
        HMENU hNewMenu;
        MENUITEMINFO mii;
        HMENU hMenuFloat;
	char* menustr[3];

        menustr[0]=LoadStrByID(MID_SMSADD);
        menustr[1]=LoadStrByID(MID_SMSEDIT);
        menustr[2]=LoadStrByID(MID_SMSDEL);

        memset(&mii, 0, sizeof(MENUITEMINFO));
        mii.type = MFT_BMPSTRING;
        mii.id = 0;
        mii.uncheckedbmp = &micon_hday1;	//
        mii.typedata = (DWORD)"opt";

        hNewMenu = CreatePopupMenu(&mii);
        for (i=0; i<3; i++)
        {
                memset(&mii,0,sizeof(MENUITEMINFO));
                mii.type = MFT_BMPSTRING;
                mii.id = 3200+i;
                mii.state = 0;
                switch (i)
                {
                        case 0:
                                mii.uncheckedbmp=&micon_hday1;
                                break;
                        case 1:
                                mii.uncheckedbmp=&micon_hday2;
                                break;
                        case 2:
                                mii.uncheckedbmp=&micon_hday3;
                                break;
                }
                mii.typedata= (DWORD)menustr[i];
                InsertMenuItem(hNewMenu,i,TRUE,&mii);
        }
        hMenuFloat = StripPopupHead(hNewMenu);
        TrackPopupMenu(hMenuFloat,TPM_LEFTALIGN|TPM_VCENTERALIGN,5,160,hWnd);

        return hMenuFloat;

}

static void SetLVData(HWND hWnd, int itemindex, int opmode)
{
        LVITEM item;
        LVSUBITEM subdata;
        HLVITEM hitem;

        char itemstr[50];

        item.nItemHeight = 24;
        if(opmode==0)		//add
        {
                item.nItem = itemindex;
                hitem = SendMessage (hWnd, LVM_ADDITEM, 0, (LPARAM)&item);
        }
        else			//edit
        {
                hitem = SendMessage (hWnd, LVM_GETSELECTEDITEM, 0, 0);
                SendMessage(hWnd,LVM_GETITEM,hitem,(LPARAM)&item);
        }

        subdata.nItem = item.nItem;

	memset(itemstr,0,5);
	sprintf(itemstr,"%02d",thtz.ID);
	subdata.subItem = 0;
	subdata.flags = 0;
	subdata.pszText = itemstr;
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM)&subdata);

	memset(itemstr,0,50);
	subdata.subItem = 1;
	subdata.flags = 0;
	sprintf(itemstr,"%02d.%02d-%02d.%02d",thtz.HDate[0][0],thtz.HDate[0][1],thtz.HDate[1][0],thtz.HDate[1][1]);
	subdata.pszText = itemstr;
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM)&subdata);

        memset(itemstr,0,5);
        sprintf(itemstr,"%02d",thtz.TZID);
        subdata.subItem = 2;
        subdata.flags = 0;
        subdata.pszText = itemstr;
        subdata.nTextColor = 0;
        SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM)&subdata);

}

static void AddLVData(HWND hWnd)
{
        int i;
        hdaycount=0;

        SendMessage(hWnd, MSG_FREEZECTRL, TRUE, 0);
        if(SendMessage(hWnd,LVM_GETITEMCOUNT,0,0)>0)
	{
                SendMessage(hWnd, LVM_DELALLITEM,0,0);
		UpdateWindow(hWnd,TRUE);
	}

        for(i=1;i<=HTZ_MAX;i++)
        {
                memset(&thtz,0,sizeof(THTimeZone));
                if(FDB_GetHTimeZone(i,&thtz)!=NULL)
                {
                        SetLVData(hWnd,hdaycount++,0);
                }
        }
        SendMessage(hWnd, MSG_FREEZECTRL, FALSE, 0);
}

static int GetSelID(HWND listwnd)
{
        HLVITEM hItemSelected;
        LVSUBITEM subitem;
	char mybuff[24];

        memset(mybuff,0,24);
        hItemSelected=SendMessage(listwnd,LVM_GETSELECTEDITEM,0,0);
        if (hItemSelected==0)
                return 0;

        subitem.pszText=mybuff;
        subitem.subItem=0;
        SendMessage(listwnd,LVM_GETSUBITEMTEXT,hItemSelected,(LPARAM)&subitem);
        if(subitem.pszText)
		return atoi(mybuff);

	return 0;

}

extern int processscrollview(HWND listwnd, int down, int incseed);
static int Holidaywinproc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char hintchar[50];
	int selindex=0;
	int selID=0;
	int tmpvalue = 0;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&mhdaybmpbg,GetBmpPath("submenubg.jpg")))
	                //        return 0;

                	LoadBitmap(HDC_SCREEN,&mhdaybmps1,GetBmpPath("pageup.gif"));
	                LoadBitmap(HDC_SCREEN,&mhdaybmps2,GetBmpPath("pagedown.gif"));
	                LoadBitmap(HDC_SCREEN,&mhdaybmps3,GetBmpPath("okkey.gif"));
			if(gOptions.TFTKeyLayout!=3)
		                LoadBitmap(HDC_SCREEN,&mhdaybmps4,GetBmpPath("function.gif"));
			else
		                LoadBitmap(HDC_SCREEN,&mhdaybmps4,GetBmpPath("fun2.gif"));

	                LoadBitmap(HDC_SCREEN,&micon_hday1,GetBmpPath("hday_1.gif"));
	                LoadBitmap(HDC_SCREEN,&micon_hday2,GetBmpPath("hday_2.gif"));
	                LoadBitmap(HDC_SCREEN,&micon_hday3,GetBmpPath("hday_3.gif"));

			if (gfont1==NULL)
               			hdayfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_LIGHT, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
						FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);

			InitWindow(hWnd);		//add controls
			AddLVData(hdayGVWnd);
			UpdateWindow(hWnd,TRUE);
			SetFocusChild(hdayGVWnd);
			SendMessage(hdayGVWnd,LVM_CHOOSEITEM,0,0);
			break;

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

			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
			if(fGetDC)
				ReleaseDC (hdc);
			return 0;
		}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);

			tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
                        SetTextColor(hdc,PIXEL_lightwhite);
			//display operation hint
			if (gfont1==NULL)
                        	SelectFont(hdc,hdayfont);
			else	SelectFont(hdc,gfont1);

        		if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEUP));
				else
        	                	sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEUP));
                	        TextOut(hdc,6,218,hintchar);
                        	FillBoxWithBitmap (hdc,60,216,16,16,&mhdaybmps1);
	                        memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEDOWN));
                                else
        	                	sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEDOWN));
                	        TextOut(hdc,86,218,hintchar);
	                        FillBoxWithBitmap (hdc,140, 216,16, 16, &mhdaybmps2);
			}

			if(gOptions.TFTKeyLayout!=3)
			{
	                        memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_SMSEDIT));
                                else
		                	sprintf(hintchar,"%s:",LoadStrByID(MID_SMSEDIT));
               		        TextOut(hdc,175,218,hintchar);
               			FillBoxWithBitmap (hdc, 220, 216,16, 16, &mhdaybmps3);
			}
                        memset(hintchar,0,20);
			if (fromRight)
				sprintf(hintchar,"%s",LoadStrByID(MID_SMSMENU));
                        else
                        	sprintf(hintchar,"%s:",LoadStrByID(MID_SMSMENU));
                        TextOut(hdc,250,218,hintchar);
                        FillBoxWithBitmap (hdc, 290, 216,24, 16, &mhdaybmps4);

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
			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))	//exit
			{
				PostMessage(hWnd,MSG_CLOSE,0,0L);
				return 0;
			}

			if ((gOptions.TFTKeyLayout!=3) && (LOWORD(wParam)==SCANCODE_ENTER || (LOWORD(wParam)==SCANCODE_F10))) 	//edit
			{
				SendMessage(hWnd,MSG_COMMAND,3201,0L);
				return 0;
			}

			if((LOWORD(wParam)==SCANCODE_MENU) || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_ENTER))	//menu
			{
				CreateQuickMenu(hWnd);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_F12)
			{
                                SendMessage(hdayGVWnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				processscrollview(hdayGVWnd,1,8);
			}

			if(LOWORD(wParam)==SCANCODE_F11)
			{
                                SendMessage(hdayGVWnd,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				processscrollview(hdayGVWnd,0,8);
			}

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				return 0;
			}
			break;

		case MSG_COMMAND:
			switch(wParam)
			{
				case 3200:
					//show add holiday window here.
					if(FDB_CntHTimeZone() < HTZ_MAX)
					{
						selID=0;
						selindex = SendMessage(hdayGVWnd,SVM_GETCURSEL,0,0);
						if(CreateHolidayOptWindow(hWnd,0,&selID))
						{
							memset(&thtz,0,sizeof(THTimeZone));
							if(FDB_GetHTimeZone(selID,&thtz)!=NULL)
							{
								SetLVData(hdayGVWnd,hdaycount++,0);	//add record in listview box.
								SetFocusChild(hdayGVWnd);
								SendMessage(hdayGVWnd,LVM_CHOOSEITEM,hdaycount-1,0);
							}
						}
						if(ismenutimeout) return 0;
					}
					else
						MessageBox1(hWnd,LoadStrByID(MID_REC_FULL), LoadStrByID(MID_APPNAME),
							MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
					break;

				case 3201:
					//show edit holiday window here.
					selID = GetSelID(hdayGVWnd);
					if(selID)
					{
						selindex = SendMessage(hdayGVWnd,SVM_GETCURSEL,0,0);
						if(CreateHolidayOptWindow(hWnd, 1, &selID))
						{
					                memset(&thtz,0,sizeof(THTimeZone));
        					        if(FDB_GetHTimeZone(selID,&thtz)!=NULL)
					                {
					                        SetLVData(hdayGVWnd,selindex,1);
								SetFocusChild(hdayGVWnd);
								SendMessage(hdayGVWnd,LVM_CHOOSEITEM,selindex,0);
					                }
						}
						if(ismenutimeout) return 0;
					}
					break;

				case 3202:
					//show delete group window here.
					selID = GetSelID(hdayGVWnd);
					if(selID)
					{
						selindex = SendMessage(hdayGVWnd,SVM_GETCURSEL,0,0);
						if(MessageBox1 (hWnd,LoadStrByID(MID_HTZ_DELHINT), LoadStrByID(MID_APPNAME),
	                                                MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
						{
							if(FDB_DelHTimeZone(selID)==FDB_OK)
							{
								sync();
								AddLVData(hdayGVWnd);
							}
						}

						if(ismenutimeout) return 0;
						SetFocusChild(hdayGVWnd);
	                                        if(selindex<hdaycount)
	                                                SendMessage(hdayGVWnd,LVM_CHOOSEITEM,selindex,0);
	                                        else
	                                                SendMessage(hdayGVWnd,LVM_CHOOSEITEM,selindex-1,0);
					}
					break;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&mhdaybmpbg);
			UnloadBitmap(&mhdaybmps1);
			UnloadBitmap(&mhdaybmps2);
			UnloadBitmap(&mhdaybmps3);
			UnloadBitmap(&mhdaybmps4);
			UnloadBitmap(&micon_hday1);
			UnloadBitmap(&micon_hday2);
			UnloadBitmap(&micon_hday3);
			if (gfont1==NULL)
				DestroyLogFont(hdayfont);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int HolidayMngWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
        InitMiniGUIExt();

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
//	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = Holidaywinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while(GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hMainWnd);
        MiniGUIExtCleanUp ();
//	printf("close holiday window\n");
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

