/*workcode manage window*/
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
#include <minigui/tftmullan.h>
#include "ssrcommon.h"
#define IDC_LISTVIEW 80
#define IDC_WKCD 81
#define LB_STATIC 82
#define TIMER_WKCD	8008

#define MENU_ADDWORKCODE	1010
#define MENU_EDITWORKCODE	1011
#define MENU_DELWORKCODE	1012

HWND hWorkCodeWnd;
HWND hSMSGVWnd=(HWND)NULL,EdWkcd;
TWORKCODE tmpworkcode;		//save workcode.

static BITMAP mbmp2, mbmp3, mbmp4, mbmp5,mbmp6, mbmp7, mbmp8;
PLOGFONT wkcdfont;

extern int wkcdwinparam;
extern char ownwkcd[MAX_WORKCODE_LEN+1];
char tmpwkc[MAX_WORKCODE_LEN+1];

int selwkcdpin;			//selected workcode pin.
int wkcdcount=0;			//record count.
int timercount = 0;	//timer counts

static HMENU CreateQuickMenu(HWND hWnd)
{
	int i;
	HMENU hNewMenu;
	MENUITEMINFO mii;
	HMENU hMenuFloat;

	char *msg[3];
	msg[0] = LoadStrByID(MID_SMSADD);
	msg[1] = LoadStrByID(MID_SMSEDIT);
	msg[2] = LoadStrByID(MID_SMSDEL);

	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.type = MFT_BMPSTRING;
	mii.id = 0;
	mii.uncheckedbmp=&mbmp6;
	mii.typedata = (DWORD)"opt";

	hNewMenu = CreatePopupMenu(&mii);
	for (i=0;i<3; i++){
		memset(&mii,0,sizeof(MENUITEMINFO));
		mii.type = MFT_BMPSTRING;
		mii.id = MENU_ADDWORKCODE + i;
		mii.state = 0;
		switch (i)
		{
			case 0:
				mii.uncheckedbmp=&mbmp6;
				break;
			case 1:
				mii.uncheckedbmp=&mbmp7;
				break;
			case 2:
				mii.uncheckedbmp=&mbmp8;
				break;
		}
		mii.typedata= (DWORD)msg[i];
		InsertMenuItem(hNewMenu,i,TRUE,&mii);
	}
	hMenuFloat = StripPopupHead(hNewMenu);
	TrackPopupMenu(hMenuFloat,TPM_LEFTALIGN|TPM_VCENTERALIGN,5,155,hWnd);

	return hMenuFloat;

}

static int GetPINFromGrid(void)
{
	HLVITEM hItemSelected;
	hItemSelected=SendMessage(hSMSGVWnd,LVM_GETSELECTEDITEM,0,0);
	if (hItemSelected==0)
		return 0;
	return SendMessage(hSMSGVWnd,LVM_GETITEMADDDATA,0,hItemSelected);
}

static void CreateListView(HWND hWnd)
{
	int i;
	LVCOLUMN lvcol;
	char* colnames[3];

	colnames[0]=LoadStrByID(MID_WKCDCODE);
	colnames[1]=LoadStrByID(MID_WKCDNAME);

	if(wkcdwinparam)
	{
		if(gOptions.IMEFunOn==1)
		{
			CreateWindow(CTRL_STATIC,LoadStrByID(MID_WKCDINPUT), WS_CHILD | WS_VISIBLE, LB_STATIC,10,10,120,23,hWnd,0);
			EdWkcd=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
					IDC_WKCD,140,6,170,23,hWnd,0);
			SendMessage(EdWkcd,EM_LIMITTEXT,MAX_WORKCODE_LEN,0L);

			hSMSGVWnd = CreateWindowEx (CTRL_LISTVIEW, "WorkCode View",
					WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER, WS_EX_NONE, IDC_LISTVIEW, 0,30,gOptions.LCDWidth,185,hWnd,0);
		}
		else
		{
			CreateWindow(CTRL_STATIC,LoadStrByID(MID_WKCDINPUT), WS_CHILD | WS_VISIBLE, LB_STATIC,10+(gOptions.ControlOffset/2),80,120,23,hWnd,0);
			EdWkcd = CreateWindow(CTRL_SLEDIT,"", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
					IDC_WKCD,140+(gOptions.ControlOffset/2),76,170,23,hWnd,0);
			SendMessage(EdWkcd,EM_LIMITTEXT,MAX_WORKCODE_LEN,0L);
		}
	}
	else
	{
		hSMSGVWnd = CreateWindowEx (CTRL_LISTVIEW, "WorkCode View",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER, WS_EX_NONE, IDC_LISTVIEW, 0,0,gOptions.LCDWidth,215,hWnd,0);
	}

	if(hSMSGVWnd)
	{
		SendMessage(hSMSGVWnd,LVM_SETHEADHEIGHT,20,0);
		SendMessage(hSMSGVWnd,SVM_SETSCROLLPAGEVAL,0,192);

		for (i=0;i<2;i++)
		{
			lvcol.nCols=i;
			lvcol.pszHeadText = colnames[i];
			switch(i)
			{
				case 0:
					lvcol.width=105+gOptions.GridWidth;
					break;
				case 1:
					lvcol.width=210+gOptions.GridWidth*3;
					break;
			}

			lvcol.pfnCompare = NULL;
			lvcol.colFlags = 0;
			SendMessage (hSMSGVWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
		}
	}
}

static void RefreshLVData(HWND hWnd)
{
	static LVITEM item;
	static LVSUBITEM subdata;
	static HLVITEM hitem;
	TWORKCODE myworkcode;
	static char tmpstr[40];
	int workcodeid;

	hitem = SendMessage(hWnd,LVM_GETSELECTEDITEM,0,0);
	workcodeid = SendMessage(hWnd,LVM_GETITEMADDDATA,0,hitem);

	item.nItemHeight = 24;
	memset(&myworkcode,0,sizeof(TWORKCODE));
	myworkcode.PIN=workcodeid;
	if(FDB_GetWorkCode(workcodeid,&myworkcode)!=NULL)
	{
		subdata.nItem = item.nItem;

		memset(tmpstr,0,MAX_WORKCODE_LEN+1);
		nmemcpy((unsigned char*)tmpstr,(unsigned char*)myworkcode.Code,MAX_WORKCODE_LEN);
		subdata.subItem = 0;
		subdata.flags = 0;
		subdata.pszText = tmpstr;
		subdata.nTextColor = 0;
		SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

		memset(tmpstr,0,MAX_WORKCODE_NAME_LEN+1);
		nmemcpy((unsigned char*)tmpstr,(unsigned char*)myworkcode.Name,MAX_WORKCODE_NAME_LEN);
		subdata.subItem = 1;
		subdata.flags = 0;
		subdata.pszText = tmpstr;
		subdata.nTextColor = 0;
		SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);
	}
}

static void SetLVData(HWND hWnd, int itemindex,int workcodeid)
{
	static LVITEM item;
	static LVSUBITEM subdata;
	static HLVITEM hitem;
	static char tmpstr[40];

	item.nItemHeight = 20;
	item.nItem = itemindex;
	hitem = SendMessage (hWnd, LVM_ADDITEM, 0, (LPARAM)&item);

	subdata.nItem = item.nItem;
	memset(tmpstr,0,MAX_WORKCODE_LEN+1);
	nmemcpy((unsigned char*)tmpstr,(unsigned char*)tmpworkcode.Code,MAX_WORKCODE_LEN);
	subdata.subItem = 0;
	subdata.flags = 0;
	subdata.pszText = tmpstr;
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

	memset(tmpstr,0,MAX_WORKCODE_NAME_LEN+1);
	nmemcpy((unsigned char*)tmpstr,(unsigned char*)tmpworkcode.Name,MAX_WORKCODE_NAME_LEN);
	subdata.subItem = 1;
	subdata.flags = 0;
	subdata.pszText = tmpstr;
	subdata.nTextColor = 0;
	SendMessage (hWnd, LVM_SETSUBITEM, hitem, (LPARAM) &subdata);

	SendMessage(hWnd,LVM_SETITEMADDDATA,hitem,(LPARAM)workcodeid);
}

static void refreshLVBox(HWND hWnd)
{
	int i=0;
	SendMessage(hWnd, MSG_FREEZECTRL, TRUE, 0);

	if(SendMessage(hWnd,LVM_GETITEMCOUNT,0,0)>0)
		SendMessage(hWnd, LVM_DELALLITEM,0,0);

	if(FDB_InitWkcdPoint()==1)
	{
		while(FDB_ReadWorkCode(&tmpworkcode)==1)
		{
			if(tmpworkcode.PIN != 0)
			{
				SetLVData(hWnd, i++,tmpworkcode.PIN);
				wkcdcount++;
			}
		}
	}

	SendMessage(hWnd, MSG_FREEZECTRL, FALSE, 0);
}

static int FindAddedItem(HWND hWnd, int workcodeid)
{
	HLVITEM hFound;
	LVFINDINFO findInfo;

	findInfo.flags = LVFF_ADDDATA;
	findInfo.iStart = 0;
	findInfo.addData = workcodeid;
	hFound = SendMessage(hWnd,LVM_FINDITEM,0,(LPARAM)&findInfo);
	if(hFound)
	{
		SendMessage(hWnd,LVM_CHOOSEITEM,0,hFound);
		return 1;
	}
	return 0;
}

static int FindInputItem(HWND hWnd, char *inputwkcd, int type)
{
	LVITEM item;
	LVSUBITEM subdata;
	HLVITEM hitem;
	char tmps[MAX_WORKCODE_LEN+1];
	int i, wkcdcount = 0;

	wkcdcount = FDB_CntWorkCode();
	for(i=0;i<wkcdcount;i++)
	{
		memset(tmps,0,MAX_WORKCODE_LEN+1);
		item.nItem = i;
		hitem = SendMessage (hWnd, LVM_GETITEM, 0, (LPARAM)&item);
		subdata.nItem = item.nItem;
		subdata.subItem = 0;
		subdata.flags = 0;
		subdata.pszText = tmps;
		subdata.nTextColor = 0;
		SendMessage(hWnd,LVM_GETSUBITEMTEXT,hitem,(LPARAM)&subdata);
		if(type)	//部分匹配
		{
			if(strncmp(tmps,inputwkcd,strlen(inputwkcd))==0)
			{
				SendMessage(hWnd,LVM_CHOOSEITEM,i,0);
				return 1;
			}
		}
		else		//全部匹配
		{
			if(strcmp(tmps,inputwkcd)==0)
			{
				SendMessage(hWnd,LVM_CHOOSEITEM,i,0);
				return 1;
			}
		}
	}
	return 0;
}

static int bchgflag=0;
static void input_notif_proc(HWND hWnd, int id, int nc, DWORD add_data)
{
	//Liaozz 20081007 fix bug second 3,
	//not allow 0 as workcode when no workcode define or have workcode define.
	char tmpinput[MAX_WORKCODE_LEN+1];
	memset(tmpinput,0,MAX_WORKCODE_LEN+1);
	GetWindowText(EdWkcd,tmpinput,MAX_WORKCODE_LEN);
	if (strcmp(tmpinput, "0")==0)
	{
		SetWindowText(EdWkcd, "");
		return;
	}
	//Liaozz end
	if(nc == EN_CHANGE && wkcdwinparam && bchgflag)
	{
		memset(tmpwkc,0,MAX_WORKCODE_LEN+1);
		GetWindowText(EdWkcd,tmpwkc,MAX_WORKCODE_LEN);
		//Comment by Liaozz 20081007 fix bug second 3, this way cannot process the case in workcode have no define.
		//		if (strcmp(tmpwkc, "0")==0)
		//		{
		//			SetWindowText(EdWkcd, "");
		//			return;
		//		}
		//Liaozz end
		FindInputItem(hSMSGVWnd,tmpwkc,1);
	}
}

static void select_notif_proc(HWND hWnd, int id, int nc, DWORD add_data)
{
	TWORKCODE tmpwc;

	if(nc == LVN_SELCHANGE && wkcdwinparam)
	{
		memset(tmpwkc,0,MAX_WORKCODE_LEN+1);
		selwkcdpin = GetPINFromGrid();
		if (FDB_GetWorkCode(selwkcdpin, &tmpwc)!=NULL)
		{
			memcpy(tmpwkc,tmpwc.Code,MAX_WORKCODE_LEN);
			bchgflag=0;
			SetWindowText(EdWkcd,tmpwkc);
			bchgflag=1;
		}
		memset(tmpwkc,0,MAX_WORKCODE_LEN+1);
		GetWindowText(EdWkcd,tmpwkc,MAX_WORKCODE_LEN);
		SendMessage(EdWkcd,EM_SETCARETPOS,0,strlen(tmpwkc));
	}
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static void processSelect(HWND hWnd)
{
	int bfindflag=0;

	if(wkcdwinparam)
	{
		memset(ownwkcd,0,MAX_WORKCODE_LEN+1);
		memset(tmpwkc,0,MAX_WORKCODE_LEN+1);
		GetWindowText(EdWkcd,tmpwkc,MAX_WORKCODE_LEN);
		if(strlen(tmpwkc)==0)
		{
			if(gOptions.IMEFunOn==1)
			{
				//默认选择列表第一条工作号
				selwkcdpin = GetPINFromGrid();
				if(!selwkcdpin)
				{
					MessageBox1(hWnd,LoadStrByID(MID_WKCDINPUT), LoadStrByID(MID_APPNAME),
							MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
				}
				else
					PostMessage(hWnd,MSG_CLOSE,0,0L);
			}
			else
			{
				MessageBox1(hWnd,LoadStrByID(MID_WKCDINPUT), LoadStrByID(MID_APPNAME),
						MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
			}
		}
		else
		{
			if(gOptions.IMEFunOn==1)
				bfindflag=FindInputItem(hSMSGVWnd,tmpwkc,0);
			else
			{
				if (FDB_GetWorkCodeByCode(tmpwkc, &tmpworkcode)!=NULL)
				{
					while(FDB_ReadWorkCode(&tmpworkcode)==1)
					{
						if(tmpworkcode.PIN!=0 && strncmp(tmpworkcode.Code,tmpwkc,MAX_WORKCODE_LEN)==0)
						{
							bfindflag=1;
							break;
						}
					}
				}
			}

			if(bfindflag)
			{
				if(gOptions.IMEFunOn==1)
					selwkcdpin = GetPINFromGrid();
				else
					selwkcdpin = tmpworkcode.PIN;
				PostMessage(hWnd,MSG_CLOSE,0,0L);
			}
			else
			{
				if(gOptions.MustCheckWorkCode)
				{
					MessageBox1(hWnd,LoadStrByID(MID_NOWKCDHINT), LoadStrByID(MID_APPNAME),
							MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
				}
				else
				{
					selwkcdpin = 0;
					nmemcpy((unsigned char*)ownwkcd,(unsigned char*)tmpwkc,MAX_WORKCODE_LEN);
					PostMessage(hWnd,MSG_CLOSE,0,0L);
				}
			}
		}
	}
	else
		SendMessage(hWnd,MSG_COMMAND,MENU_EDITWORKCODE,0);

}

extern int processscrollview(HWND listwnd, int down , int incseed);
extern HWND createStatusWin1 (HWND hParentWnd, int width, int height,char* title, char* text, ...);
extern void destroyStatusWin1(HWND hwnd);
extern int myMessageBox1 (HWND hwnd, DWORD dwStyle, char * title, char * text, ...);
static int WorkCodeMngWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int myworkcodepin;
	char hintchar[20];
	HWND statushWnd;
	int selid;

	//	int bfindflag=0;	//输入的workcode是否存在
	//   TWORKCODE tmpwc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			CreateListView(hWnd);
			if(gOptions.IMEFunOn==1)
			{
				LoadBitmap(HDC_SCREEN,&mbmp2,GetBmpPath("pageup.gif"));
				LoadBitmap(HDC_SCREEN,&mbmp3,GetBmpPath("pagedown.gif"));
				LoadBitmap(HDC_SCREEN,&mbmp4,GetBmpPath("okkey.gif"));
				if(gOptions.TFTKeyLayout!=3)
					LoadBitmap(HDC_SCREEN,&mbmp5,GetBmpPath("function.gif"));
				else
					LoadBitmap(HDC_SCREEN,&mbmp5,GetBmpPath("fun2.gif"));
				LoadBitmap(HDC_SCREEN,&mbmp6,GetBmpPath("keyadd.gif"));
				LoadBitmap(HDC_SCREEN,&mbmp7,GetBmpPath("edit.gif"));
				LoadBitmap(HDC_SCREEN,&mbmp8,GetBmpPath("keydel.gif"));

				if (gfont1==NULL)
					wkcdfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR,
							FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL,
							FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
			}

			if(gOptions.IMEFunOn==1)
			{
				statushWnd = createStatusWin1(hWnd,250,50,LoadStrByID(MID_APPNAME), LoadStrByID(MID_WAIT));
				refreshLVBox(hSMSGVWnd);		//加载workcode数据
				destroyStatusWin1(statushWnd);

				SetNotificationCallback(hSMSGVWnd, select_notif_proc);
				SetNotificationCallback(EdWkcd, input_notif_proc);
			}

			if(wkcdwinparam)
			{
				SetFocusChild(EdWkcd);
#ifdef ZEM600
				SetTimer(hWnd, TIMER_WKCD, 50);
#else
				SetTimer(hWnd, TIMER_WKCD, 100);
#endif
			}
			else
				SetFocusChild(hSMSGVWnd);

			if(gOptions.IMEFunOn==1)
			{
				SendMessage(hSMSGVWnd,LVM_CHOOSEITEM,0,0);
				//Resolved user inputed the workcode list view cannot focus to the corresponding row.
				select_notif_proc(0, 0, LVN_SELCHANGE, 0);
				SetWindowText(EdWkcd, "");
			}

			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);

			if(gOptions.IMEFunOn==1)
			{
				SetBkColor(hdc,0x00FFA2BE);
				if (gfont1==NULL)
					SelectFont(hdc,wkcdfont);
				else 	SelectFont(hdc,gfont1);
				SetTextColor(hdc,PIXEL_lightwhite);
				if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
				{
					memset(hintchar,0,20);
					if (fromRight==1)
						sprintf(hintchar,"%s",LoadStrByID(MID_PAGEUP));
					else
						sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEUP));
					TextOut(hdc,6,220,hintchar);
					FillBoxWithBitmap (hdc, 60, 218,16, 16, &mbmp2);
					memset(hintchar,0,20);
					if (fromRight==1)
						sprintf(hintchar,"%s",LoadStrByID(MID_PAGEDOWN));
					else
						sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEDOWN));
					TextOut(hdc,86,220,hintchar);
					FillBoxWithBitmap (hdc, 140, 218,16, 16, &mbmp3);
				}
				if(wkcdwinparam)
				{
					memset(hintchar,0,20);
					if (fromRight==1)
						sprintf(hintchar,"%s",LoadStrByID(MID_SMSSELECT));
					else
						sprintf(hintchar,"%s:",LoadStrByID(MID_SMSSELECT));
					//TextOut(hdc,226,220,hintchar);
					//zsliu change 20100312
					if(gOptions.TFTKeyLayout==3)
					{
						TextOut(hdc,210,220,hintchar);
					}
					if(gOptions.TFTKeyLayout!=3)
						FillBoxWithBitmap(hdc,287,219,24,16,&mbmp4);
					else
						FillBoxWithBitmap(hdc,287,219,24,16,&mbmp5);
				}
				else
				{
					if(gOptions.TFTKeyLayout!=3)
					{
						memset(hintchar,0,20);
						if (fromRight==1)
							sprintf(hintchar,"%s",LoadStrByID(MID_EDIT));
						else
							sprintf(hintchar,"%s:",LoadStrByID(MID_EDIT));
						TextOut(hdc,171,220,hintchar);
						FillBoxWithBitmap (hdc, 210, 218,16, 16, &mbmp4);
					}
					memset(hintchar,0,20);
					if (fromRight==1)
						sprintf(hintchar,"%s",LoadStrByID(MID_SMSMENU));
					else
						sprintf(hintchar,"%s:",LoadStrByID(MID_SMSMENU));
					TextOut(hdc,246,220,hintchar);
					FillBoxWithBitmap (hdc, 287, 219,24, 16, &mbmp5);
				}
			}
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_COMMAND:
			switch (wParam)
			{
				case MENU_ADDWORKCODE:
					myworkcodepin = 0;
					if(CreateWCodeManageWindow(hWnd,1,&myworkcodepin))
					{
						if(myworkcodepin)		//添加了新workcode
						{
							refreshLVBox(hSMSGVWnd);
							SetFocusChild(hSMSGVWnd);
							FindAddedItem(hSMSGVWnd,myworkcodepin);
						}
					}
					if(ismenutimeout) return 0;
					break;

				case MENU_EDITWORKCODE:
					myworkcodepin = GetPINFromGrid();
					if(myworkcodepin>0)
					{
						if(CreateWCodeManageWindow(hWnd,0,&myworkcodepin))
							RefreshLVData(hSMSGVWnd);
						if(ismenutimeout) return 0;
					}
					else
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(MID_CHOOSEUSER));
					}
					break;

				case MENU_DELWORKCODE:
					myworkcodepin=GetPINFromGrid();
					if(myworkcodepin>0)
					{
						selid=SendMessage(hSMSGVWnd,SVM_GETCURSEL,0,0);
						if (MessageBox1 (hWnd,LoadStrByID(MID_WKCDDELETE),LoadStrByID(MID_APPNAME),
									MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
						{
							if(FDB_DelWorkCode(myworkcodepin)==FDB_OK)
							{
								sync();
								refreshLVBox(hSMSGVWnd);
							}
							SetFocusChild(hSMSGVWnd);
							if(selid<FDB_CntWorkCode())
								SendMessage(hSMSGVWnd,LVM_CHOOSEITEM,selid,0);
							else
								SendMessage(hSMSGVWnd,LVM_CHOOSEITEM,selid-1,0);
						}
					}
					else
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(MID_CHOOSEUSER));
					}
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
			if (gOptions.KeyPadBeep) ExKeyBeep();

			if (wkcdwinparam) timercount = 0;

			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				if(wkcdwinparam)
				{
					selwkcdpin = 0;
					memset(ownwkcd,0,MAX_WORKCODE_LEN+1);
				}
				PostMessage(hWnd,MSG_CLOSE,0,0L);
			}
			else if((LOWORD(wParam)==SCANCODE_MENU)||(gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_ENTER))// 创建菜单
			{
				if(!wkcdwinparam)
					CreateQuickMenu(hWnd);
				else
					processSelect(hWnd);
				return 0;
			}
			if ((gOptions.TFTKeyLayout!=3) && (LOWORD(wParam)==SCANCODE_ENTER || LOWORD(wParam)==SCANCODE_F10))
			{
				processSelect(hWnd);
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_F12)	//page down
			{
				if(gOptions.IMEFunOn==1)
				{
					SendMessage(hSMSGVWnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
					if(wkcdwinparam)
						processscrollview(hSMSGVWnd,1,7);
					else
						processscrollview(hSMSGVWnd,1,8);
					return 0;
				}
			}
			else if (LOWORD(wParam)==SCANCODE_F11)		//page up
			{
				if(gOptions.IMEFunOn==1)
				{
					SendMessage(hSMSGVWnd,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
					if(wkcdwinparam)
						processscrollview(hSMSGVWnd,0,7);
					else
						processscrollview(hSMSGVWnd,0,8);
					return 0;
				}
			}
#if 1
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP || LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if(gOptions.IMEFunOn==1 && wkcdwinparam)
					SendMessage(hSMSGVWnd,MSG_KEYDOWN,wParam,0L);
			}
#endif
			break;

		case MSG_TIMER:
			if (wParam == TIMER_WKCD)
			{
				if (++timercount > 60)
				{
					timercount = 0;
					selwkcdpin = 0;
					memset(ownwkcd,0,MAX_WORKCODE_LEN+1);
					KillTimer(hWnd, TIMER_WKCD);
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
			break;

		case MSG_CLOSE:
			UnloadBitmap(&mbmp2);
			UnloadBitmap(&mbmp3);
			UnloadBitmap(&mbmp4);
			UnloadBitmap(&mbmp5);
			UnloadBitmap(&mbmp6);
			UnloadBitmap(&mbmp7);
			UnloadBitmap(&mbmp8);

			if (gfont1==NULL)
				DestroyLogFont(wkcdfont);

			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			//	hWorkCodeWnd=NULL;
			return 0;
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateWorkCodeManageWindow(HWND hOwner)
{
	MSG Msg;
	MAINWINCREATE CreateInfo;
	hOwner = GetMainWindowHandle (hOwner);
	InitMiniGUIExt();

	CreateInfo.dwStyle = WS_VISIBLE ;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WorkCodeMngWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;

	hWorkCodeWnd = CreateMainWindow (&CreateInfo);

	if (hWorkCodeWnd == HWND_INVALID)
		return -1;

	ShowWindow(hWorkCodeWnd, SW_SHOWNORMAL);

	while (GetMessage(&Msg, hWorkCodeWnd))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	MainWindowThreadCleanup (hWorkCodeWnd);

	MiniGUIExtCleanUp ();

	if(wkcdwinparam)
		return selwkcdpin;
	else
		return 0;
}

