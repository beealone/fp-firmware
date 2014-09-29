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

//BITMAP mgpbmpbg;
BITMAP mgpbmps1;
BITMAP mgpbmps2;
BITMAP mgpbmps3;
BITMAP mgpbmps4;

//menu icon
BITMAP micon_gp1;
BITMAP micon_gp2;
BITMAP micon_gp3;

#define IDL_GPLISTVIEW 0xF0
PLOGFONT gpfont;
HWND hgpGVWnd;
int gpcount=0;
TGroup tmpgp;
static char gpstr[10];

static void InitWindow(HWND hWnd)
{
        int i;
        LVCOLUMN lvcol;
	char* itemstr[2];

	itemstr[0]=LoadStrByID(MID_GP_INDEX);
	itemstr[1]=LoadStrByID(MID_GP_DEFTZ);
	sprintf(gpstr, "%s", LoadStrByID(MID_USER_TZ));

        hgpGVWnd = CreateWindowEx (CTRL_LISTVIEW, "group View", WS_CHILD | WS_VISIBLE | WS_VSCROLL, WS_EX_NONE,
                                IDL_GPLISTVIEW, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight-25, hWnd, 0);
        SendMessage(hgpGVWnd,LVM_SETHEADHEIGHT,20,0);
        SendMessage(hgpGVWnd,SVM_SETSCROLLPAGEVAL,0,192);

        for (i=0;i<2;i++)
        {
                lvcol.nCols=i;
                lvcol.pszHeadText = itemstr[i];
                switch(i)
                {
                        case 0:
                                lvcol.width=45+gOptions.GridWidth;
                                break;
                        case 1:
                                lvcol.width=275+gOptions.GridWidth*2;
                                break;
                }

                lvcol.pfnCompare = NULL;
                lvcol.colFlags = 0;
                SendMessage (hgpGVWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
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
        mii.uncheckedbmp = &micon_gp1;	//
        mii.typedata = (DWORD)"opt";

        hNewMenu = CreatePopupMenu(&mii);
        for(i=0; i<3; i++)
        {
                memset(&mii,0,sizeof(MENUITEMINFO));
                mii.type = MFT_BMPSTRING;
                mii.id = 200+i;
                mii.state = 0;
                switch (i)
                {
                        case 0:
                                mii.uncheckedbmp=&micon_gp1;
                                break;
                        case 1:
                                mii.uncheckedbmp=&micon_gp2;
                                break;
                        case 2:
                                mii.uncheckedbmp=&micon_gp3;
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
        if(opmode==0)
        {
                item.nItem = itemindex;
                hitem = SendMessage (hWnd, LVM_ADDITEM, 0, (LPARAM)&item);
        }
        else
        {
                hitem = SendMessage (hWnd, LVM_GETSELECTEDITEM, 0, 0);
                SendMessage(hWnd,LVM_GETITEM,hitem,(LPARAM)&item);
        }

        subdata.nItem = item.nItem;

	memset(itemstr,0,50);
	sprintf(itemstr,"%02d",tmpgp.ID);
	subdata.subItem = 0;
	subdata.flags = 0;
	subdata.pszText = itemstr;
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM)&subdata);

	memset(itemstr,0,50);
	subdata.subItem = 1;
	subdata.flags = 0;
	sprintf(itemstr, "%s%02d,%s%02d,%s%02d", gpstr, tmpgp.TZID[0], gpstr, tmpgp.TZID[1], gpstr, tmpgp.TZID[2]);
	subdata.pszText = itemstr;
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM)&subdata);

}

static void AddLVData(HWND hWnd)
{
        int i;
        gpcount=0;

        SendMessage(hWnd, MSG_FREEZECTRL, TRUE, 0);
        if(SendMessage(hWnd,LVM_GETITEMCOUNT,0,0)>0)
	{
                SendMessage(hWnd, LVM_DELALLITEM,0,0);
		UpdateWindow(hWnd,TRUE);
	}

        for(i=1;i<=GP_MAX;i++)
        {
                memset(&tmpgp,0,sizeof(TGroup));
                if(FDB_GetGroup(i,&tmpgp)!=NULL)
                {
                        SetLVData(hWnd,gpcount++,0);
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
static int Groupwinproc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char hintchar[50];
	int selindex,selID, tmpvalue;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&mgpbmpbg,GetBmpPath("submenubg.jpg")))
	                 //       return 0;

                	LoadBitmap(HDC_SCREEN,&mgpbmps1,GetBmpPath("pageup.gif"));
	                LoadBitmap(HDC_SCREEN,&mgpbmps2,GetBmpPath("pagedown.gif"));
	                LoadBitmap(HDC_SCREEN,&mgpbmps3,GetBmpPath("okkey.gif"));
			if(gOptions.TFTKeyLayout!=3)
		                LoadBitmap(HDC_SCREEN,&mgpbmps4,GetBmpPath("function.gif"));
			else
		                LoadBitmap(HDC_SCREEN,&mgpbmps4,GetBmpPath("fun2.gif"));
	                LoadBitmap(HDC_SCREEN,&micon_gp1,GetBmpPath("gp_1.gif"));
	                LoadBitmap(HDC_SCREEN,&micon_gp2,GetBmpPath("gp_2.gif"));
	                LoadBitmap(HDC_SCREEN,&micon_gp3,GetBmpPath("gp_3.gif"));

			if (gfont1==NULL)
               			gpfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_LIGHT, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
						FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
			InitWindow(hWnd);		//add controls
			AddLVData(hgpGVWnd);
			UpdateWindow(hWnd,TRUE);
			SetFocusChild(hgpGVWnd);
			SendMessage(hgpGVWnd,LVM_CHOOSEITEM,0,0);
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
			if (gfont1==NULL)
                        	SelectFont(hdc,gpfont);
			else	SelectFont(hdc,gfont1);

           		if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEUP));
				else
                                	sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEUP));

                	        TextOut(hdc,5,218,hintchar);
                        	FillBoxWithBitmap (hdc,61,216,16,16,&mgpbmps1);

				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEDOWN));
                                else
                                	sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEDOWN));
                	        TextOut(hdc,86,218,hintchar);
                        	FillBoxWithBitmap (hdc,140, 216,16, 16, &mgpbmps2);
			}
			if(gOptions.TFTKeyLayout!=3)
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_SMSEDIT));
                                else
                                	sprintf(hintchar,"%s:",LoadStrByID(MID_SMSEDIT));
	               	        TextOut(hdc,185,218, hintchar);
        	       		FillBoxWithBitmap (hdc, 220, 216,16, 16, &mgpbmps3);
			}
			memset(hintchar,0,20);
			if (fromRight)
				sprintf(hintchar,"%s",LoadStrByID(MID_SMSMENU));
                        else
                        	sprintf(hintchar,"%s:",LoadStrByID(MID_SMSMENU));
                        TextOut(hdc,250,218,hintchar);
                        FillBoxWithBitmap (hdc, 290, 216,24, 16, &mgpbmps4);

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
				PostMessage(hWnd,MSG_CLOSE,0,0L);

			if((gOptions.TFTKeyLayout!=3)&&((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10))) 	//edit
			{
				PostMessage(hWnd,MSG_COMMAND,201,0L);
				return 0;
			}
			if((LOWORD(wParam)==SCANCODE_MENU)||(gOptions.TFTKeyLayout==3 && wParam==SCANCODE_ENTER))	//menu
			{
				CreateQuickMenu(hWnd);
				return 0;
			}
			if(LOWORD(wParam)==SCANCODE_F12)
			{
                               	SendMessage(hgpGVWnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				processscrollview(hgpGVWnd,1,8);
				return 0;
			}
			if(LOWORD(wParam)==SCANCODE_F11)
			{
                               	SendMessage(hgpGVWnd,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				processscrollview(hgpGVWnd,0,8);
				return 0;
			}
                        if((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
                        {
                                return 0;
                        }

			break;

		case MSG_COMMAND:
			switch(wParam)
			{
				case 200:
					if(FDB_CntGroup() < GP_MAX)
					{
						//show add group window here.
						selID=0;
						selindex = SendMessage(hgpGVWnd,SVM_GETCURSEL,0,0);
						if(CreateGroupOptWindow(hWnd,0,&selID))
						{
							memset(&tmpgp,0,sizeof(TGroup));
							if(FDB_GetGroup(selID,&tmpgp)!=NULL)
							{
								SetLVData(hgpGVWnd,gpcount++,0);	//add record in listview box.
								SetFocusChild(hgpGVWnd);
								SendMessage(hgpGVWnd,LVM_CHOOSEITEM,gpcount-1,0);
							}
						}
						if(ismenutimeout) return 0;
					}
					else
						MessageBox1(hWnd,LoadStrByID(MID_REC_FULL), LoadStrByID(MID_APPNAME),
                                                        MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
					break;

				case 201:
					//show edit group window here.
					selID = GetSelID(hgpGVWnd);
					if(selID)
					{
						selindex = SendMessage(hgpGVWnd,SVM_GETCURSEL,0,0);
						if(CreateGroupOptWindow(hWnd, 1, &selID))
						{
					                memset(&tmpgp,0,sizeof(TGroup));
        					        if(FDB_GetGroup(selID,&tmpgp)!=NULL)
					                {
					                        SetLVData(hgpGVWnd,selindex,1);
								SetFocusChild(hgpGVWnd);
								SendMessage(hgpGVWnd,LVM_CHOOSEITEM,selindex,0);
					                }
						}
						if(ismenutimeout) return 0;
					}
					break;
				case 202:
					//show delete group window here.
					selID = GetSelID(hgpGVWnd);
					if(selID)
					{
						selindex = SendMessage(hgpGVWnd,SVM_GETCURSEL,0,0);
#if 0
						if(beUsedGroup(selID))	//有用户使用该分组
						{
							if(MessageBox1(hWnd, LoadStrByID(MID_GP_DELHINT1), LoadStrByID(MID_APPNAME),
								MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)!=IDOK)
								break;
						}
						else
						{
							if(MessageBox1 (hWnd,LoadStrByID(MID_GP_DELHINT), LoadStrByID(MID_APPNAME),
		                                                MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)!=IDOK)
								break;
						}
#endif
						if(beUsedGroup(selID))
						{
							MessageBox1(hWnd, LoadStrByID(MID_GP_DELHINT1),LoadStrByID(MID_APPNAME),
									MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
						}
						else
						{
							if(FDB_DelGroup(selID)==FDB_OK)
							{
								sync();
								AddLVData(hgpGVWnd);
							}

							SetFocusChild(hgpGVWnd);
		                                        if(selindex<gpcount)
	        	                                        SendMessage(hgpGVWnd,LVM_CHOOSEITEM,selindex,0);
	                	                        else
	                        	                        SendMessage(hgpGVWnd,LVM_CHOOSEITEM,selindex-1,0);
						}
					}
					break;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&mgpbmpbg);
			UnloadBitmap(&mgpbmps1);
			UnloadBitmap(&mgpbmps2);
			UnloadBitmap(&mgpbmps3);
			UnloadBitmap(&mgpbmps4);

			UnloadBitmap(&micon_gp1);
			UnloadBitmap(&micon_gp2);
			UnloadBitmap(&micon_gp3);

			if (gpfont!=NULL)
				DestroyLogFont(gpfont);
			//MainWindowCleanup (hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int GroupMngWindow(HWND hWnd)
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
	CreateInfo.MainWindowProc = Groupwinproc;
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

	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

