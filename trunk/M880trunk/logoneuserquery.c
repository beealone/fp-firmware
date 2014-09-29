/*
 * ** $Id: gridview.c,v 1.3.6.2 2006/06/28 07:21:40 xwyan Exp $
 * **
 * ** gridview.c: Sample program for MiniGUI Programming Guide
 * **      Usage of GRID control.
 * **
 * ** Copyright (C) 2004 ~ 2006 Feynman Software.
 * **
 * ** License: GPL
 * */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "main.h"
#include "ssrcommon.h"
#include "tftmsgbox.h"

enum {
	IDC_LISTVIEW
};

static HWND hOneLogWnd=0;

static int g_celcount=5;

//static int g_logrowcount=0;
//static int templogrow=0;
//static char mylogbuff[24];
int mylogcount=0;
PLOGFONT logqyfont;
PLOGFONT logqyfont1;

U16 oneuserpin=0;
time_t onesttm=0;
time_t oneedtm=0;
int findret=1;
BITMAP  onequerybmp;
static RECT log_State_rc={200,215,320,235};
extern char *tmplogbuf;
TTime * ChrisToHejiraTTime(TTime *christm);

/*dsl 2012.4.17*/
extern int gRecordCnt;

static void CreateLogListView(HWND mainwnd)
{
	int i;
	LVCOLUMN lvcol;
	char* (colnames[5]);

	colnames[0]=LoadStrByID(MID_ACNO);
	colnames[1]=LoadStrByID(MID_NAME);
	colnames[2]=LoadStrByID(MID_ATT);
	colnames[3]=LoadStrByID(HID_SVERIFY);
	colnames[4]=LoadStrByID(MID_STATUS);


	hOneLogWnd = CreateWindowEx (CTRL_LISTVIEW, "user View",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL |
			WS_HSCROLL | WS_BORDER, WS_EX_NONE, IDC_LISTVIEW, 0, 0, gOptions.LCDWidth,gOptions.LCDHeight-30, mainwnd, 0);

	SendMessage(hOneLogWnd,SVM_SETSCROLLPAGEVAL,0,180);

	for (i=0;i<g_celcount;i++)
	{
		lvcol.nCols=i;
		lvcol.pszHeadText = colnames[i];
		switch(i)
		{
			case 0:
				lvcol.width=67+gOptions.GridWidth/2;
				break;
			case 1:
				if (gOptions.ShowState)
					lvcol.width=80+gOptions.GridWidth;
				else
					lvcol.width=118+gOptions.GridWidth;
				break;
			case 2:
				lvcol.width=80+gOptions.GridWidth/2;
				break;
			case 3:
				lvcol.width=40+gOptions.GridWidth/2;
				break;
			case 4:
				lvcol.width=46+gOptions.GridWidth/2;
				break;
		}
		lvcol.pfnCompare = NULL;
		lvcol.colFlags = 0;
		SendMessage (hOneLogWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
	}
	SetWindowFont(hOneLogWnd,logqyfont);

}

static void SetListViewLog(HWND mainwnd,int itemindex,char **userdata)
{
	LVITEM item;
	LVSUBITEM subdata;
	HLVITEM item1;
	int i;
	char tchar[5];

	item.nItemHeight = 18;
	item.nItem=itemindex;
	item1 = SendMessage (mainwnd, LVM_ADDITEM, 0, (LPARAM)&item);
	for (i=0;i<g_celcount;i++)
	{
		subdata.nItem = item.nItem;
		subdata.subItem = i;
		if(i<4) {
			subdata.pszText = userdata[i];
		} else {
			memset(tchar,0,sizeof(tchar));
			sprintf(tchar,"%d",(unsigned char)*(userdata[i]));
			subdata.pszText = tchar;
		}
		subdata.nTextColor = 0;
		subdata.flags = 0;
		subdata.image = 0;
		SendMessage (mainwnd, LVM_SETSUBITEM, item1, (LPARAM) &subdata);
	}
}

static int processlogstateandvf(HWND hWnd)
{
	HLVITEM hItemSelected;
	LVSUBITEM subitem;
	char findbuff[5]={0};
	char statestr[20]={0};
	char vfstr[20]={0};
	char lastprintstr[80]={0};
	char bottomstr[50]={0};
	HDC mydc;
	int statecode;
	TSHORTKEY tstkey;

	memset(&subitem,0,sizeof(LVSUBITEM));
	memset(findbuff,0,5);
	memset(vfstr,0,20);
	memset(statestr,0,20);
	memset(lastprintstr,0,80);

	hItemSelected=SendMessage(hOneLogWnd,LVM_GETSELECTEDITEM,0,0);
	if (hItemSelected==0){
		return 0;
	}

	subitem.pszText=findbuff;
	subitem.subItem=3;
	SendMessage(hOneLogWnd,LVM_GETSUBITEMTEXT,hItemSelected,(LPARAM)&subitem);
	if ((!gOptions.UserExtendFormat) && subitem.pszText)
	{
		if (findbuff[0]=='F')
		{
#ifdef FACE
			if(gOptions.FaceFunOn && findbuff[1]=='a') //add by caona for face
			{
				sprintf(vfstr,"%c%c:%s",findbuff[0],findbuff[1],LoadStrByID(MID_FACE));
			}
#else
			sprintf(vfstr,"%c:%s",findbuff[0],LoadStrByID(MID_FINGER));
#endif
		}
		if (findbuff[0]=='P')
			sprintf(vfstr,"%c:%s",findbuff[0],LoadStrByID(MID_PWD));
		if (findbuff[0]=='I')
			sprintf(vfstr,"%c:%s",findbuff[0],LoadStrByID(MID_CARD_NUM));
		if (findbuff[0]=='C')
			sprintf(vfstr,"%c:%s",findbuff[0],LoadStrByID(MID_CARD));
		if (findbuff[0]=='G')
			sprintf(vfstr,"%c:%s",findbuff[0],LoadStrByID(MID_LOCK_GP));
		if (findbuff[0]=='O')
			sprintf(vfstr,"%c:%s",findbuff[0],LoadStrByID(MID_LOGHINTOTHER));

		sprintf(lastprintstr,"%s",vfstr);
	}
	else if(gOptions.UserExtendFormat)
	{
		int verifyStyle=atoi(findbuff);
		char *verify[15]={
			"FP/PW/RF",
			"FP",
			"PIN",
			"PW",
			"RF",
			"FP/PW",
			"FP/RF",
			"PW/RF",
			"PIN&FP",
			"FP&PW",
			"FP&RF",
			"PW&RF",
			"FP&PW&RF",
			"PIN&FP&PW",
			"FP&(RF/PIN)"
		};
		if(verifyStyle>14)
		{
			sprintf(lastprintstr,"%d:%s",verifyStyle, "Other");
			sprintf(vfstr,"%d:%s",verifyStyle, "Other");

		}
		else
		{
			sprintf(lastprintstr,"%d:%s",verifyStyle, verify[verifyStyle]);
			sprintf(vfstr,"%d:%s",verifyStyle, verify[verifyStyle]);
		}
	}
	if (gOptions.ShowState)
	{
		memset(&subitem,0,sizeof(LVSUBITEM));
		memset(findbuff,0,5);
		subitem.pszText=findbuff;
		subitem.subItem=4;
		SendMessage(hOneLogWnd,LVM_GETSUBITEMTEXT,hItemSelected,(LPARAM)&subitem);
		statecode = atoi(findbuff);
		memset(&tstkey,0,sizeof(TSHORTKEY));
		//		printf("statecode------------:%d\n",statecode);
		if(!gOptions.AntiPassbackFunOn) 
		{
			//Liaozz 20081014, change attlog display model
			if(FDB_GetShortKeyByState(statecode,&tstkey)!=NULL)
			{
				char mynamename[40];           //modify by jazzy 2008.12.04
				memset(mynamename,0,40);
				Str2UTF8(tftlocaleid,(unsigned char *)tstkey.stateName,(unsigned char *) mynamename);
				sprintf(statestr,"S:%s",mynamename);
			}
			//Liaozz 20081014, change attlog display model
		} 
		else 
		{
			if(gOptions.IMEFunOn==1)
			{
				if(FDB_GetShortKeyByState(statecode,&tstkey)!=NULL)
				{
					char mynamename[40]={0};           //modify by jazzy 2008.12.04
					memset(mynamename,0,40);
					Str2UTF8(tftlocaleid,(unsigned char *)tstkey.stateName,(unsigned char *) mynamename);
					sprintf(statestr,"S:%s",mynamename);
				}
			}
			else
			{
				if (statecode == 0)
					sprintf(statestr,"S:%s",LoadStrByID(HIT_SYSTEM5KEY1));
				if (statecode == 1)
					sprintf(statestr,"S:%s",LoadStrByID(HIT_SYSTEM5KEY2));
				if (statecode == 2)
					sprintf(statestr,"S:%s",LoadStrByID(HIT_SYSTEM5KEY5));
				if (statecode == 3)
					sprintf(statestr,"S:%s",LoadStrByID(HIT_SYSTEM5KEY6));
				if (statecode == 4)
					sprintf(statestr,"S:%s",LoadStrByID(HIT_SYSTEM5KEY3));
				if (statecode == 5)
					sprintf(statestr,"S:%s",LoadStrByID(HIT_SYSTEM5KEY4));
			}
		}
		//Liaozz end
		//printf("query state:%d\n",statecode);
		//printf("ddddddddddddddd 3 lastprintstr:%s, vfstr:%s, statestr=%s\n",lastprintstr, vfstr, statestr);
		sprintf(lastprintstr,"%s %s",lastprintstr, statestr);
		//printf("ddddddddddddddd lastprintstr:%s\n",lastprintstr);
	}

	//zsliu add
	log_State_rc.right = gOptions.LCDWidth;
	log_State_rc.bottom = gOptions.LCDHeight-5;
	
	InvalidateRect(hWnd,&log_State_rc,TRUE);

	mydc = GetClientDC(hWnd);
	SetBkColor(mydc,0x00FFA2BE);
	SelectFont(mydc,logqyfont1);
	SetTextColor(mydc,PIXEL_lightwhite);
	sprintf(bottomstr,"%s:%02d",LoadStrByID(MID_RECTOTAL),mylogcount);
	TextOut(mydc,50+gOptions.GridWidth,220,bottomstr);
	TextOut(mydc,190+gOptions.GridWidth,220,lastprintstr);
	ReleaseDC(mydc);

	return 0;
}

char *FormatExtendVerified(BYTE verified);
static int ProcessFindLog(HWND hWnd,int findpin,time_t st1, time_t ed1)
{
	PAttLog logs=(PAttLog)tmplogbuf;
	TTime tt;
	TUser uuser;

	int c=gRecordCnt;
	int i=0,j=0;
	char line[20];
	char line1[100];
	char *logdata[5];
	memset(&uuser,0,sizeof(TUser));
	memset(line1,0,sizeof(line1));
	logdata[0]=NULL;
	logdata[1]=NULL;
	logdata[2]=NULL;
	logdata[3]=NULL;
	logdata[4]=NULL;
	if (findpin>0) {
		uuser.PIN=findpin;
		FDB_GetUser(findpin,&uuser);
	}

	if (findpin){
		c=FDB_GetAttLog(findpin,st1,ed1,logs,c);
	} else {
		c=FDB_GetAttLog(0,st1,ed1,logs,c);
	}
	if (c==0) {
		findret=0;
		SendMessage(hWnd,MSG_CLOSE,0,0);
		return 0;
	}
	mylogcount=c;
	i=c-1;
	if (i >= 0){
		CreateLogListView(hWnd);
	}
	SendMessage (hOneLogWnd, MSG_FREEZECTRL, TRUE, 0);
	while(i>=0 && i<c)
	{
		OldDecodeTime(logs[i].time_second, &tt);
		//zsliu add
		if(gOptions.isUseHejiraCalendar){
			ChrisToHejiraTTime(&tt);
		}
		//zsliu add end ... ...

		if(logs[i].PIN>0){
			memset(line,0,20);
			memset(&uuser,0,sizeof(TUser));
			uuser.PIN = logs[i].PIN;
			FDB_GetUserByCharPIN2((char *)logs[i].PIN2,&uuser);
			sprintf(line, "%02d-%02d %02d:%02d", tt.tm_mon+1, tt.tm_mday,tt.tm_hour, tt.tm_min);
#if 0
			//Liaozz 20081014, change attlog diaplay model
			if(gOptions.AntiPassbackFunOn)
			{
				if(logs[i].status==0)
					logs[i].status=2;
			}
#endif
			logdata[0]=(char *)logs[i].PIN2;

			char mynamename[100];		//modify by jazzy 2008.07.26
			memset(mynamename,0,sizeof(mynamename));
			Str2UTF8(tftlocaleid,(unsigned char *)uuser.Name,(unsigned char *) mynamename);
			logdata[1]=mynamename;
			//logdata[1]=uuser.Name;

			logdata[2]=line;
			if(gOptions.UserExtendFormat){
				logdata[3]=FormatExtendVerified(logs[i].verified);
			} else {
				logdata[3]=FormatVerified(logs[i].verified);
			}
			logdata[4]=(char *)&(logs[i].status);//FormatStatus(logs[i].status);	liming
			//printf("logs[i].status:%s logdata:%s\n", logs[i].status,logdata[4]);
			SetListViewLog(hOneLogWnd,j,logdata);
			j++;
		}
		i--;
	}
	SendMessage (hOneLogWnd, MSG_FREEZECTRL, FALSE, 0);
	if(gImageBuffer != NULL) {
		memset(gImageBuffer, 0, c);
	}
	SetFocusChild(hOneLogWnd);
	return 0;
}


static int ControlOneLogWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{

	HDC hdc;
	HWND statushWnd;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			{
				LoadBitmap(HDC_SCREEN,&onequerybmp,GetBmpPath("warningsmall.gif"));

				if (gfont1==NULL) {
					logqyfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, 
							FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
					logqyfont1 = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, 
							FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
				} else {
					logqyfont1=gfont1;
					logqyfont=gfont1;
				}
				if (gOptions.ShowState){
					g_celcount=5;
				} else {
					g_celcount=4;
				}
				statushWnd = createStatusWin1(hWnd , 250 , 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(MID_WAIT));
				ProcessFindLog(hWnd,oneuserpin,onesttm,oneedtm);
				destroyStatusWin1(statushWnd);
				SetFocusChild(hOneLogWnd);
				SendMessage(hOneLogWnd,LVM_CHOOSEITEM,0,0);
				processlogstateandvf(hWnd);
				return 0;
			}
			/*
			   case MSG_ERASEBKGND:
			   {
			   HDC hdc = (HDC)wParam;
			   const RECT* clip = (const RECT*) lParam;
			   BOOL fGetDC = FALSE;
			   RECT rcTemp;

			   if (hdc == 0) {
			   hdc = GetClientDC (hWnd);
			   fGetDC = TRUE;
			   }

			   if (clip) {
			   rcTemp = *clip;
			   ScreenToClient (hWnd, &rcTemp.left, &rcTemp.top);
			   ScreenToClient (hWnd, &rcTemp.right, &rcTemp.bottom);
			   IncludeClipRect (hdc, &rcTemp);
			   }

			   FillBoxWithBitmap (hdc, 0, 210, 32, 32, &onequerybmp);
			   if (fGetDC)
			   ReleaseDC (hdc);
			   return 0;
			   }
			   */
		case MSG_PAINT:
			SetMenuTimeOut(time(NULL));
			hdc=BeginPaint(hWnd);
			//FillBoxWithBitmap (hdc, 10, 217, 24, 24, &onequerybmp);
			FillBoxWithBitmap (hdc, 10+gOptions.GridWidth, 215, 24, 24, &onequerybmp);
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_COMMAND:
			if (wParam == IDCANCEL){
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
				break;
			}

			if ((LOWORD(wParam)==IDC_LISTVIEW) &&(HIWORD(wParam)==LVN_SELCHANGE)){
				processlogstateandvf(hWnd);
			}
			break;

		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout) {
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

			if (gOptions.KeyPadBeep) {
				ExKeyBeep();
			}

			if ( LOWORD(wParam)==SCANCODE_ESCAPE ) {
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			}

			if(LOWORD(wParam)==SCANCODE_F12) {
				SendMessage(hOneLogWnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				processscrollview(hOneLogWnd,1,10);
			}

			if(LOWORD(wParam)==SCANCODE_F11) {
				SendMessage(hOneLogWnd,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				processscrollview(hOneLogWnd,0,10);
			}
			break;
		case MSG_CLOSE:
			UnloadBitmap(&onequerybmp);
			if (gfont==NULL) {
				DestroyLogFont(logqyfont);
				DestroyLogFont(logqyfont1);
			}
			DestroyMainWindow(hWnd);
			return TRUE;

	}
	return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

int CreateOneLogWindow(HWND hOwner,int pin,time_t st1,time_t ed1)
{

	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hOwner = GetMainWindowHandle (hOwner);
	InitMiniGUIExt();

	findret=1;
	oneuserpin=pin;
	onesttm=st1;
	oneedtm=ed1;
	mylogcount=0;
	//CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwStyle = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_ATTQUERY);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	printf("______%s%d\n", __FILE__, __LINE__);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ControlOneLogWinProc;
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
	return findret;
}
