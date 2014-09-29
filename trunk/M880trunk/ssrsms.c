/*sms manage window*/
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

#define IDC_LISTVIEW 10
#define MENU_ADDSMS	100
#define MENU_EDITSMS	101
#define MENU_DELSMS	102

HWND hGVWnd_sms;
TSms tmpsms;
int selSmsPIN = 0;			//当前选择的行
int menustatus=0;
char* smsHint[10];

BITMAP smsbmp2, smsbmp3, smsbmp4, smsbmp5,smsbmp6, smsbmp7, smsbmp8;

PLOGFONT smsfont;

static void LoadHint()
{
	smsHint[0]=LoadStrByID(MID_SMSADD);
	smsHint[1]=LoadStrByID(MID_SMSEDIT);
	smsHint[2]=LoadStrByID(MID_SMSDEL);
	smsHint[3]=LoadStrByID(MID_SMSPRIVATE);
	smsHint[4]=LoadStrByID(MID_SMSCOMMON);
	smsHint[5]=LoadStrByID(MID_SMSNOTSEND);
	smsHint[6]=LoadStrByID(MID_PAGEUP);
	smsHint[7]=LoadStrByID(MID_PAGEDOWN);
	smsHint[8]=LoadStrByID(MID_SMSCHECK);
	smsHint[9]=LoadStrByID(MID_SMSMENU);
}

static HMENU CreateQuickMenu(HWND hWnd)
{
	int i;
	HMENU hNewMenu;
	MENUITEMINFO mii;
	HMENU hMenuFloat;

	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.type = MFT_BMPSTRING;
	mii.id = 0;
	mii.uncheckedbmp=&smsbmp6;
	mii.typedata = (DWORD)"opt";

	hNewMenu = CreatePopupMenu(&mii);
	for (i=0;i<3; i++){
		memset(&mii,0,sizeof(MENUITEMINFO));
		mii.type = MFT_BMPSTRING;
		mii.id = MENU_ADDSMS+i;
		mii.state = 0;
		switch (i)
		{
			case 0:
				mii.uncheckedbmp=&smsbmp6;
				break;
			case 1:
				mii.uncheckedbmp=&smsbmp7;
				break;
			case 2:
				mii.uncheckedbmp=&smsbmp8;
				break;
		}
		mii.typedata= (DWORD)smsHint[i];
		InsertMenuItem(hNewMenu,i,TRUE,&mii);
	}
	hMenuFloat = StripPopupHead(hNewMenu);
	TrackPopupMenu(hMenuFloat,TPM_LEFTALIGN|TPM_VCENTERALIGN,5,155,hWnd);

	menustatus=1;
	return hMenuFloat;

}

static long GetPINFromGrid(void)
{
	HLVITEM hItemSelected;
	hItemSelected=SendMessage(hGVWnd_sms,LVM_GETSELECTEDITEM,0,0);
	if (hItemSelected==0)
		return 0;
	return SendMessage(hGVWnd_sms,LVM_GETITEMADDDATA,0,hItemSelected);
}

static void CreateListView(HWND hWnd)
{
	int i;
	LVCOLUMN lvcol;
	char* colnames[2];

	colnames[0]=LoadStrByID(MID_SMSCONTENTHIT);
	colnames[1]=LoadStrByID(MID_SMSTYPE);

	hGVWnd_sms = CreateWindowEx (CTRL_LISTVIEW, "sms View", WS_CHILD | WS_VISIBLE | WS_VSCROLL  | WS_BORDER, WS_EX_NONE, IDC_LISTVIEW, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight-25, hWnd, 0);

	SendMessage(hGVWnd_sms,LVM_SETHEADHEIGHT,20,0);
	SendMessage(hGVWnd_sms,SVM_SETSCROLLPAGEVAL,0,192);

	for (i=0;i<2;i++)
	{
		lvcol.nCols=i;
		lvcol.pszHeadText = colnames[i];
		switch(i)
		{
			case 0:
				lvcol.width=250+gOptions.GridWidth*2;
				break;
			case 1:
				if(gOptions.TFTKeyLayout==3)
					lvcol.width=85+gOptions.GridWidth;
				else
					lvcol.width=70+gOptions.GridWidth;
				break;
		}

		lvcol.pfnCompare = NULL;
		lvcol.colFlags = 0;
		SendMessage (hGVWnd_sms, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
	}
}
/*
static void RefreshLVData(HWND hWnd)
{
   	static LVITEM item;
	static LVSUBITEM subdata;
	HLVITEM hitem;
	TSms mysms;
	char itemstr[40];
	char tmpstr[40];
	int smsid;

	hitem = SendMessage(hWnd,LVM_GETSELECTEDITEM,0,0);
       	smsid = SendMessage(hWnd,LVM_GETITEMADDDATA,0,hitem);

	item.nItemHeight = 24;
	memset(&mysms,0,sizeof(TSms));
	mysms.ID=smsid;
        if(FDB_GetSms(smsid,&mysms)!=NULL)
	{
		subdata.nItem = item.nItem;

		memset(itemstr,0,40);
		memset(tmpstr,0,40);
		nmemcpy((unsigned char*)itemstr,mysms.Content,26);
		if(strlen(itemstr)>13)
			sprintf(tmpstr,"%s...",itemstr);
		else
			sprintf(tmpstr,"%s",itemstr);
//	        printf("tmpstr:%s\n",tmpstr);
		subdata.subItem = 0;
	        subdata.flags = 0;
	        subdata.pszText = tmpstr;
	        subdata.nTextColor = 0;

	        SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);
	        subdata.subItem = 1;
	        subdata.flags = 0;
	        switch(mysms.Tag)
       		{
	                case UDATA_TAG_USERSMS:
        	                subdata.pszText = smsHint[3];
                	        break;
	                case UDATA_TAG_ALL:
        	                subdata.pszText = smsHint[4];
                	        break;
	                case UDATA_TAG_TEMP:
        	                subdata.pszText = smsHint[5];
                	        break;
	        }
        	subdata.nTextColor = 0;
	        SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);
	}
}
*/
static void SetLVData(HWND hWnd, int itemindex,int smsid)
{
	LVITEM item;
	LVSUBITEM subdata;
	HLVITEM hitem;
	char itemstr[MAX_SMS_CONTENT_SIZE*2+1];
	//char tmpstr[40];

	item.nItemHeight = 24;
	item.nItem = itemindex;
	hitem = SendMessage (hWnd, LVM_ADDITEM, 0, (LPARAM)&item);

	subdata.nItem = item.nItem;
/*	memset(itemstr,0,40);
	memset(tmpstr,0,40);
	nmemcpy(itemstr,tmpsms.Content,26);
	if(strlen(itemstr)>13)
		sprintf(tmpstr,"%s...",itemstr);
	else
		sprintf(tmpstr,"%s",itemstr);
*/
	memset(itemstr,0,MAX_SMS_CONTENT_SIZE*2+1);

        Str2UTF8(tftlocaleid,tmpsms.Content, (unsigned char*)itemstr);	////modify by jazzy 2008.07.26
	//memcpy(itemstr,tmpsms.Content,MAX_SMS_CONTENT_SIZE*2);

	subdata.subItem = 0;
	subdata.flags = 0;
//	subdata.pszText = tmpstr;
	subdata.pszText = itemstr;
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

	subdata.subItem = 1;
	subdata.flags = 0;
	switch(tmpsms.Tag)
	{
		case UDATA_TAG_USERSMS:
			subdata.pszText = smsHint[3];
			break;
		case UDATA_TAG_ALL:
			subdata.pszText = smsHint[4];
			break;
		case UDATA_TAG_TEMP:
			subdata.pszText = smsHint[5];
			break;
	}
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

	SendMessage(hWnd,LVM_SETITEMADDDATA,hitem,(LPARAM)smsid);
}

static void refreshLVBox(HWND hWnd)
{
	int	i=0;

	SendMessage(hWnd, MSG_FREEZECTRL, TRUE, 0);

	if(SendMessage(hWnd,LVM_GETITEMCOUNT,0,0)>0)
		SendMessage(hWnd, LVM_DELALLITEM,0,0);

	if(FDB_InitSmsPoint()==1)
	{
		while(FDB_ReadSms(&tmpsms)==1)
		{
			if(tmpsms.ID != 0)
			{
				SetLVData(hWnd, i++,tmpsms.ID);
			}
		}
	}

	SendMessage(hWnd, MSG_FREEZECTRL, FALSE, 0);
}

static int FindAddedItem(HWND hWnd, int smsid)
{
	HLVITEM hFound;
	LVFINDINFO findInfo;

	findInfo.flags = LVFF_ADDDATA;
	findInfo.iStart = 0;
	findInfo.addData = smsid;
	hFound = SendMessage(hWnd,LVM_FINDITEM,0,(LPARAM)(&findInfo));
	if(hFound)
	{
		SendMessage(hWnd,LVM_CHOOSEITEM,0,hFound);
		return 1;
	}
	return 0;
}

extern int processscrollview(HWND listwnd, int down, int incseed);
extern HWND createStatusWin1 (HWND hParentWnd, int width, int height,char* title, char* text, ...);
extern void destroyStatusWin1(HWND hwnd);
extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
extern int myMessageBox1 (HWND hwnd, DWORD dwStyle, char* title, char* text, ...);
static int SmsMngWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int mysmspin;//U16 mysmspin; change to int . Liaozz 20081006 fix bug second 50
	char hintchar[20];
	HWND statushWnd;
	static char keyupFlag=0;
	//zsliu add
	int nRet2 = -1, nRet3 = -1, nRet4 = -1, nRet5 = -1, nRet6 = -1, nRet7 = -1, nRet8 = -1;
	//zsliu add end ......
	switch (message)
	{
		case MSG_CREATE:
			//zsliu change
			CreateListView(hWnd);
			statushWnd=createStatusWin1(hWnd , 250, 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(MID_WAIT));

			nRet2 = LoadBitmap(HDC_SCREEN,&smsbmp2,GetBmpPath("pageup.gif"));
			nRet3 = LoadBitmap(HDC_SCREEN,&smsbmp3,GetBmpPath("pagedown.gif"));
			nRet4 = LoadBitmap(HDC_SCREEN,&smsbmp4,GetBmpPath("okkey.gif"));
			if(gOptions.TFTKeyLayout!=3)
				nRet5 = LoadBitmap(HDC_SCREEN,&smsbmp5,GetBmpPath("function.gif"));
			else
				nRet5 = LoadBitmap(HDC_SCREEN,&smsbmp5,GetBmpPath("fun2.gif"));
			nRet6 = LoadBitmap(HDC_SCREEN,&smsbmp6,GetBmpPath("smsadd.gif"));
			nRet7 = LoadBitmap(HDC_SCREEN,&smsbmp7,GetBmpPath("edit.gif"));
			nRet8 = LoadBitmap(HDC_SCREEN,&smsbmp8,GetBmpPath("smsdel.gif"));
			//zsliu change end ... ...

			if (gfont1==NULL)
				smsfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);

			LoadHint();

			refreshLVBox(hGVWnd_sms);		//加载短信息数据
			destroyStatusWin1(statushWnd);

			SetFocusChild(hGVWnd_sms);
			SendMessage(hGVWnd_sms,LVM_CHOOSEITEM,0,0);
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			SetBkColor(hdc,0x00FFA2BE);
			if (gfont1==NULL)
				SelectFont(hdc,smsfont);
			else	SelectFont(hdc,gfont1);
			SetTextColor(hdc,PIXEL_lightwhite);
			if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",smsHint[6]);
				else
					sprintf(hintchar,"%s:",smsHint[6]);
				TextOut(hdc,6,220,hintchar);
				//zsliu change
				if(nRet2)
					FillBoxWithBitmap (hdc, 60, 218,16, 16, &smsbmp2);
				//zsliu change end ... ...
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",smsHint[7]);
                                else
					sprintf(hintchar,"%s:",smsHint[7]);
				TextOut(hdc,86,220,hintchar);
				//zsliu change
				if(nRet3)
					FillBoxWithBitmap (hdc, 140, 218,16, 16, &smsbmp3);
				//zsliu change end ... ...
			}
			if(gOptions.TFTKeyLayout!=3)
			{
				memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",smsHint[8]);
                                else
					sprintf(hintchar,"%s:",smsHint[8]);
				TextOut(hdc,171,220,hintchar);
				//zsliu change
				if(nRet4)
					FillBoxWithBitmap (hdc, 210, 218,16, 16, &smsbmp4);
				//zsliu change end ... ...
			}
			memset(hintchar,0,20);
			if (fromRight)
				sprintf(hintchar,"%s",smsHint[9]);
            else
				sprintf(hintchar,"%s:",smsHint[9]);
			TextOut(hdc,246,220,hintchar);
			//zsliu change
			if(nRet5)
				FillBoxWithBitmap (hdc, 287, 219,24, 16, &smsbmp5);
			//zsliu change end ... ...

			EndPaint(hWnd,hdc);
			return 0;

		case MSG_COMMAND:
			switch (wParam)
			{
				case MENU_ADDSMS:
					if(FDB_CntSms()>=gOptions.SMSCount)
					{
						MessageBox1(hWnd,LoadStrByID(MID_SMSCOUNTHINT), LoadStrByID(MID_APPNAME),MB_OK |
							MB_ICONINFORMATION | MB_BASEDONPARENT);
					}
					else
					{
						mysmspin = 0;
						if(CreateSmsOptWindow(hWnd,2,&mysmspin)==1)
						{
							if(mysmspin)		//添加了新短消息
							{
								refreshLVBox(hGVWnd_sms);
								SetFocusChild(hGVWnd_sms);
								FindAddedItem(hGVWnd_sms,mysmspin);
								selSmsPIN = SendMessage(hGVWnd_sms,SVM_GETCURSEL,0,0);
							}
						}
					}
					break;

				case MENU_EDITSMS:
					selSmsPIN =  SendMessage(hGVWnd_sms,SVM_GETCURSEL,0,0);
					mysmspin=GetPINFromGrid();
					if (mysmspin==0)	//add by jazzy 2008.11.03 if no sms,no edit
				{
					myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),
									LoadStrByID(MID_CHOOSEUSER));
					break;
				}
					if(CreateSmsOptWindow(hWnd,1, &mysmspin)==1) {
						if(mysmspin)		//添加了新短消息
						{
							refreshLVBox(hGVWnd_sms);
							SetFocusChild(hGVWnd_sms);
							FindAddedItem(hGVWnd_sms,mysmspin);
							selSmsPIN = SendMessage(hGVWnd_sms,SVM_GETCURSEL,0,0);
						}
					}

					break;

				case MENU_DELSMS:
					selSmsPIN = SendMessage(hGVWnd_sms,SVM_GETCURSEL,0,0);

					mysmspin=GetPINFromGrid();	//add by jazzy 2008.11.03 if no sms, no delete any
					if (mysmspin==0)
				{
					myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),
									LoadStrByID(MID_CHOOSEUSER));
					break;
				}

					if (MessageBox1 (hWnd,LoadStrByID(MID_SMSDELETEHIT), LoadStrByID(MID_APPNAME),
                                  		MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
					{
						mysmspin=GetPINFromGrid();
						if(FDB_DelSms(mysmspin)==FDB_OK)
						{
							sync();
							refreshLVBox(hGVWnd_sms);
						}
					}

					if(ismenutimeout) return 0;
					SetFocusChild(hGVWnd_sms);

					if(selSmsPIN<FDB_CntSms())
						SendMessage(hGVWnd_sms,LVM_CHOOSEITEM,selSmsPIN,0);
					else
						SendMessage(hGVWnd_sms,LVM_CHOOSEITEM,selSmsPIN-1,0);

					break;
			}
			break;

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
			if(gOptions.KeyPadBeep)	ExKeyBeep();

			if(LOWORD(wParam)==SCANCODE_ESCAPE)
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
			else if(LOWORD(wParam)==SCANCODE_MENU || (LOWORD(wParam)==SCANCODE_ENTER && gOptions.TFTKeyLayout==3))		// 创建菜单
			{
				CreateQuickMenu(hWnd);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_ENTER || (LOWORD(wParam)==SCANCODE_F10))		// 查看短信
			{
				selSmsPIN = SendMessage(hGVWnd_sms,SVM_GETCURSEL,0,0);
				mysmspin = GetPINFromGrid();		//获取短消息ID
				if(mysmspin)
					CreateSmsOptWindow(hWnd,0,&mysmspin);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_F11)
			{
 	                        SendMessage(hGVWnd_sms,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				processscrollview(hGVWnd_sms,0,8);
				/*zxz 2012-11-20*/
				//SendMessage(hGVWnd_sms,MSG_KEYDOWN,SCANCODE_CURSORBLOCKUP,0);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_F12)
			{
 	                        SendMessage(hGVWnd_sms,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				processscrollview(hGVWnd_sms,1,8);
				//SendMessage(hGVWnd_sms,MSG_KEYDOWN,SCANCODE_CURSORBLOCKDOWN,0);
				return 0;
			}

			break;

		case MSG_CLOSE:
			//zsliu change
			if(nRet2 )
				UnloadBitmap(&smsbmp2);
			if(nRet3)
				UnloadBitmap(&smsbmp3);
			if(nRet4)
				UnloadBitmap(&smsbmp4);
                        if(nRet5)
				UnloadBitmap(&smsbmp5);
                        if(nRet6)
				UnloadBitmap(&smsbmp6);
                        if(nRet7)
				UnloadBitmap(&smsbmp7);
                        if(nRet8)
				UnloadBitmap(&smsbmp8);
			//zsliu change end ... ...
			if (gfont1==NULL)
				DestroyLogFont(smsfont);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}
	return DefaultMainWinProc (hWnd, message, wParam, lParam);

}

int CreateSmsManageWindow(HWND hOwner)
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
    CreateInfo.MainWindowProc = SmsMngWinProc;
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

