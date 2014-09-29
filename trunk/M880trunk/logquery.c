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
#include <minigui/tftmullan.h>
#include "options.h"
#include "flashdb.h"
#include "ssrpub.h"
#include "main.h"
#include "ssrcommon.h"
#include "tftmsgbox.h"
#include "truncdate.h"

#define ID_MENU 55
#define ID_FIND 56
#define QUERYCELCOUNT 5

enum {
	IDC_GRIDVIEW,
	IDC_LISTVIEW,
};

int btnexitlog,btnfindlog;
HWND hLogWnd=0;
static int secondcome=0;
PLOGFONT logqyfont2;
static BITMAP logmp8;
static BITMAP logmp9;
static BITMAP logmp10;

static char *resacno2;
static time_t resst1=0;
static time_t resed1=0;
int noacnopass=0;
int logret=0;
char *tmplogbuf = NULL;

/*dsl 2012.4.17*/
int gQueryLogByMenu = 0;
int gRecordCnt = 0;

extern int enterkeydown;
extern int myMessageBox1 (HWND hwnd, DWORD dwStyle, char* title, char* text, ...);
extern HWND createStatusWin1 (HWND hParentWnd, int width, int height,char* title, char* text, ...);
extern void destroyStatusWin1(HWND hwnd);
extern TTime * ChrisToHejiraTTime(TTime *christm);
char *FormatStatus(BYTE status)
{
	static char *StaName[]={"I","O","T","B","i","o","N"," "};
	if(gOptions.ShowState) {
		if(status<6){
			return StaName[status];
		} else {
			return StaName[6];
		}
	} else {
		return StaName[7];
	}
}

char *FormatVerified(BYTE verified)
{
	static char *vername1[]={"P","F","I","C","G","O"};
	if(verified<6) return vername1[verified];

#ifdef FACE
	if(gOptions.FaceFunOn && verified == 15)
	{
		return "Fa";
	}
#endif
	return vername1[5];
}
char *FormatExtendVerified(BYTE verified)
{
	static char *vername1[]={"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"};
	if(verified>14){
		verified = 15;	//Other
	}
	return vername1[verified];
}

static void CreatePackedLogListView(HWND mainwnd, int allrec,PUser packuser)
{
	int i;
	LVCOLUMN lvcol;
	char* (colnames[3]);
	int packedcol;

	if (allrec>0)
	{
		char captioncol[100];

		memset(captioncol,0,100);
		packedcol=2;
		colnames[0]=LoadStrByID(REP_DATE);
		if (packuser->Name[0]) {
			char mynamename[100];		//modify by jazzy 2008.07.26
			memset(mynamename,0,100);
			Str2UTF8(tftlocaleid,(unsigned char *)packuser->Name,(unsigned char *) mynamename);
			sprintf(captioncol,"%s %s:%s %s:%s",
					LoadStrByID(MID_ATTQUERY),LoadStrByID(MID_ACNO),
					packuser->PIN2,LoadStrByID(MID_NAME),
					mynamename);
		} else {
			sprintf(captioncol,"%s %s:%s",LoadStrByID(MID_ATTQUERY),LoadStrByID(MID_ACNO),packuser->PIN2);
		}
		colnames[1]=captioncol;
		colnames[2]=NULL;
	} else	{
		packedcol=3;
		colnames[0]=LoadStrByID(REP_DATE);
		colnames[1]=LoadStrByID(MID_ACNO);
		colnames[2]=LoadStrByID(MID_ATT);
	}

	hLogWnd = CreateWindowEx (CTRL_LISTVIEW, "user View",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL |
			WS_HSCROLL | WS_BORDER, WS_EX_NONE, IDC_LISTVIEW, 0, 0, gOptions.LCDWidth,gOptions.LCDHeight-30, mainwnd, 0);

	SendMessage(hLogWnd,SVM_SETSCROLLPAGEVAL,0,180);

	for (i=0;i<packedcol;i++)
	{
		lvcol.nCols=i;
		lvcol.pszHeadText = colnames[i];
		switch(i) {
			case 0:
				lvcol.width=40+gOptions.GridWidth/2;
				break;
			case 1:
				if (allrec>0) {
					lvcol.width=278+gOptions.GridWidth*2;
				} else {
					lvcol.width=50+gOptions.GridWidth*2;
				}
				break;
			case 2:
				lvcol.width=229+gOptions.GridWidth/2;
				break;
		}
		lvcol.pfnCompare =  0;
		lvcol.colFlags = 0;
		SendMessage (hLogWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
	}
	SetWindowFont(hLogWnd,logqyfont2);
}
static void SetListViewPackedLog(HWND mainwnd,int itemindex,char **userdata,int allrec,long adddata)
{
	LVITEM item;
	LVSUBITEM subdata;
	HLVITEM item1;
	int i;
	int kcol=0;


	kcol=(allrec)?2:3;

	item.nItemHeight = 18;
	item.nItem=itemindex;
	item.itemData=adddata;
	item1 = SendMessage (mainwnd, LVM_ADDITEM, 0, (LPARAM)&item);
	for (i=0;i<kcol;i++)
	{
		subdata.nItem = item.nItem;
		subdata.subItem = i;
		subdata.pszText = userdata[i];
		subdata.nTextColor = 0;
		subdata.flags = 0;
		subdata.image = 0;
		SendMessage (mainwnd, LVM_FILLSUBITEM, item1, (LPARAM) &subdata);
	}
}

void SetGridLog(HWND grdwnd, int column, int datatype, char *userdata, double numdata,BOOL checked,int row)
{
	GRIDCELLS cellsel;
	GRIDCELLDATA celldata;

	memset(&celldata, 0, sizeof(celldata));
	memset(&cellsel,0,sizeof(cellsel));

	if (datatype==GV_TYPE_NUMBER)
	{
		GRIDCELLDATANUMBER cellnum;

		memset(&cellnum, 0, sizeof(cellnum));
		cellnum.number = numdata;
		cellnum.format = "%.0f";
		cellnum.len_format = -1;
		celldata.content = &cellnum;
	}
	else if (datatype==GV_TYPE_TEXT)
	{
		GRIDCELLDATATEXT celltext;

		memset(&celltext,0,sizeof(celltext));
		celltext.buff=userdata;
		celltext.len_buff = -1;
		celldata.content = &celltext;
	}
	else if (datatype==GV_TYPE_CHECKBOX)
	{
		GRIDCELLDATACHECKBOX cellcheckbox;

		memset(&cellcheckbox,0,sizeof(cellcheckbox));
		cellcheckbox.checked = checked;
		cellcheckbox.text = userdata;
		cellcheckbox.len_text = -1;
		celldata.content = &cellcheckbox;
	}


	celldata.mask = GVITEM_MAINCONTENT|GVITEM_STYLE | GVITEM_BGCOLOR | GVITEM_FGCOLOR;
	celldata.style = datatype;
	if (!row){
		cellsel.row = SendMessage(grdwnd,GRIDM_GETROWCOUNT,0,0);
	} else {
		cellsel.row = row;
	}

	if (cellsel.row %2 == 0){
		celldata.color_bg = COLOR_lightgray;
		celldata.color_fg = COLOR_lightwhite;
	} else {
		celldata.color_bg = COLOR_lightwhite;
		celldata.color_fg = COLOR_black;
	}
	cellsel.column = column;
	cellsel.width = 1;
	cellsel.height = 1;
	SendMessage(grdwnd, GRIDM_SETCELLPROPERTY, (WPARAM)&cellsel, (LPARAM)&celldata);
}

int havesameday=TRUE;
static int ProcessOneDayLog(PAttLog daylog,int logcount,PUserlb loguserlb,int *lvindex ,int pin,time_t qdata)
{
	int i=0;
	int k=0;
	TTime tt;
	char logbuf1[6];
	char logbuf2[600];
	char logbuf3[10];
	char *(logdata[3]);
	int onelinelimit=5;
	BOOL havefirst=TRUE;

	i=logcount-1;
	logdata[0]=NULL;
	logdata[1]=NULL;
	logdata[2]=NULL;

	memset(logbuf2,0,600);
	memset(logbuf1,0,6);
	if (pin){
		onelinelimit=7;
	} else {
		onelinelimit=6;
	}

	while (loguserlb != NULL){
		i=logcount-1;
		k=0;
		while (i>=0 && i<logcount){
			if ((daylog[i].PIN==loguserlb->userlb.PIN) &&
					(strcmp((char *)daylog[i].PIN2,loguserlb->userlb.PIN2)==0)){
				k++;
				//printf("%d,          %s,             %d,      %d\n", daylog[i].PIN, (char *)daylog[i].PIN2, daylog[i].time_second, count++);
				if (!pin)
				{
					logdata[1]=(char *)daylog[i].PIN2;
				}
				OldDecodeTime(daylog[i].time_second, &tt);

				//zsliu add
				if(gOptions.isUseHejiraCalendar)
				{
					ChrisToHejiraTTime(&tt);        //公历转伊朗日历
					//tt.tm_mon -= 1;
				}
				//zsliu add end ... ...

				sprintf(logbuf1,"%02d:%02d ",tt.tm_hour, tt.tm_min);
				strcat(logbuf2,logbuf1);
				if (k>=onelinelimit){
					if (pin)
					{
						memset(logbuf3,0,10);
						sprintf(logbuf3,"%02d/%02d",tt.tm_mon+1,tt.tm_mday);
						if (havesameday)
						{
							logdata[0]=logbuf3;
							havesameday=FALSE;
						}
						else
							logdata[0]=NULL;
						logdata[1]=logbuf2;
					}
					else
					{
						if (havefirst)
						{
							havefirst=FALSE;
						}
						else
							logdata[1]=NULL;
						logdata[2]=logbuf2;
					}
					SetListViewPackedLog(hLogWnd,*lvindex,logdata,pin,qdata);
					k=0;
					(*lvindex)++;
					memset(logbuf1,0,6);
					memset(logbuf2,0,600);
				}


			}
			i--;
		}
		if (k){
			if (pin)
			{
				memset(logbuf3,0,10);
				sprintf(logbuf3,"%02d/%02d",tt.tm_mon+1,tt.tm_mday);
				logdata[0]=logbuf3;
				if (!havesameday)
					logdata[0]=NULL;
				logdata[1]=logbuf2;
			}
			else
			{
				if (!havefirst)
					logdata[1]=NULL;
				logdata[2]=logbuf2;
			}
			SetListViewPackedLog(hLogWnd,*lvindex,logdata,pin,qdata);
			k=0;
			(*lvindex)++;
			memset(logbuf1,0,6);
			memset(logbuf2,0,600);
		}


		havefirst=TRUE;
		loguserlb = loguserlb->next;
	}
	return 0;
}
static int processalllog(HWND hWnd,time_t st1, time_t ed1,int logpin)
{
	PUserlb tmpuserlb, puserlb;
	PAttLog logs=NULL;
	TTime tt1,tt2;
	int j;
	int c;
	time_t mid_t;
	time_t mid_t2;
	char *logdata[3];
	char datechar1[30];
	char datechar2[50];

	//dsl 2012.4.17.
	if (gQueryLogByMenu) {
		/*When push the enu, support 30 records */
		if (tmplogbuf==NULL){
			tmplogbuf = (char*)MALLOC(sizeof(TAttLog) * 30);
		}
		gRecordCnt = 30;
	} else {
		/*Other, support 186(31*6) records*/
		if (tmplogbuf==NULL){
			/*临时方案，查找个人时最大3个月记录，查找所有用户时最大31*6*3*10条记录*/
			if(logpin > 0) {
				gRecordCnt = 31*6*3;
			} else {
				gRecordCnt = 31*6*3*10;
			}
			tmplogbuf = (char*)MALLOC(sizeof(TAttLog) * gRecordCnt);
		}		
	}
	if (tmplogbuf) {
		logs=(PAttLog)tmplogbuf;
	} else {
		logret=1;
		PostMessage (hWnd, MSG_CLOSE, 0, 0L);
		return 0;
	}

	mid_t=0;
	mid_t2=0;
	j=0;

	c=FDB_GetAttLog((U16)logpin,st1,ed1,logs,gRecordCnt);
	if (c==0){
		logret=0;
		PostMessage(hWnd,MSG_CLOSE,0,0);
		return 0;
	}

	logret=1;
	logdata[0]=NULL;
	logdata[1]=NULL;
	logdata[2]=NULL;

	if (hLogWnd) {
		DestroyWindow(hLogWnd);
	}

	if (logpin){
		puserlb = GetOneUsertolb(logpin);
	} else {
		puserlb = GetUsermestolb(0);
	}

	tmpuserlb = puserlb;
	CreatePackedLogListView(hWnd,logpin,&(puserlb->userlb));
	mid_t=st1;
	mid_t2=st1;
	OldDecodeTime(mid_t, &tt1);
	tt1.tm_hour=0;
	tt1.tm_min=0;
	tt1.tm_sec=0;
	tt1.tm_isdst=-1;
	mid_t = OldEncodeTime(&tt1);

	while(mid_t<=ed1){
		if (logpin) {
			if (mid_t < mid_t2) {
				if (ed1<mid_t+OneDay-OneSecond){
					c=FDB_GetAttLog(logpin,mid_t2,ed1,logs,gRecordCnt);
				} else {
					c=FDB_GetAttLog(logpin,mid_t2,mid_t+OneDay-OneSecond,logs,gRecordCnt);
				}
			} else	if (ed1 < mid_t+OneDay-OneSecond) {
				c=FDB_GetAttLog(logpin,mid_t,ed1,logs, gRecordCnt);
			} else {
				c=FDB_GetAttLog(logpin,mid_t,mid_t+OneDay-OneSecond,logs, gRecordCnt);
			}
		} else {
			if (mid_t < mid_t2) {
				if (ed1<mid_t+OneDay-OneSecond){
					c=FDB_GetAttLog(0,mid_t2,ed1,logs, gRecordCnt);
				} else {
					c=FDB_GetAttLog(0,mid_t2,mid_t+OneDay-OneSecond,logs, gRecordCnt);
				}
			} else if (ed1<mid_t+OneDay-OneSecond){
				c=FDB_GetAttLog(0,mid_t,ed1,logs,gRecordCnt);
			} else {
				c=FDB_GetAttLog(0,mid_t,mid_t+OneDay-OneSecond,logs,gRecordCnt);
			}
		}
		if(c>0){
			memset(datechar1,0, sizeof(datechar1));
			memset(datechar2,0, sizeof(datechar2));
			OldDecodeTime(mid_t, &tt2);

			//zsliu add
			if(gOptions.isUseHejiraCalendar){
				ChrisToHejiraTTime(&tt2);       //公历转伊朗日历
			}
			//zsliu add end ... ...

			sprintf(datechar1,"%02d/%02d",tt2.tm_mon+1,tt2.tm_mday);
			logdata[0]=datechar1;
			sprintf(datechar2,"%s:%02d", LoadStrByID(MID_RECTOTAL),c);
			if (logpin){
				logdata[1]=datechar2;
			} else {
				logdata[2]=datechar2;
				SetListViewPackedLog(hLogWnd,j,logdata,logpin,0);
				j++;
			}
			ProcessOneDayLog(logs,c,puserlb,&j,logpin,mid_t);
			havesameday=TRUE;
		}
		mid_t+=OneDay;
	}

	if (tmpuserlb != NULL)
	{
		PUserlb tmptmpuserlb;
		tmptmpuserlb = tmpuserlb;
		while (tmptmpuserlb != NULL)
		{
			tmpuserlb = tmptmpuserlb;
			tmptmpuserlb = tmpuserlb->next;
			FREE(tmpuserlb);
			tmpuserlb = NULL;
		}
	}
	SetFocusChild(hLogWnd);
	return 0;
}

static void processlogfind(HWND hWnd)
{
	char finduseracno[24];
	time_t sttm1=0;
	time_t edtm1=0;
	TUser queryuser;

	sttm1=0;
	edtm1=0;
	memset(finduseracno,0, sizeof(finduseracno));
	if (!strlen(resacno2)) {
		noacnopass=1;
		processalllog(hWnd,resst1,resed1,0);
		SetFocusChild(hLogWnd);
	} else {
		noacnopass=0;
		memset(&queryuser,0,sizeof(TUser));
		queryuser.PIN=1;
		if(FDB_GetUserByCharPIN2(resacno2,&queryuser)){
			processalllog(hWnd,resst1,resed1,queryuser.PIN);
		} else {
			myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(MID_QUERYNONE));
		}
	}
}

static int GetDayrecbystr(HWND hWnd,char *logstr,char *logstr2,long seltm)
{
	char midstr[10];
	int m,d;
	struct tm ttlog;
	time_t logst,loged;
	TUser finduser;

	memset(&finduser,0,sizeof(TUser));
	if(seltm==0) {
		memset(midstr,0,10);
		m=0;
		d=0;
		midstr[0]=logstr[0];
		midstr[1]=logstr[1];
		m=str2int(midstr,0);
		memset(midstr,0,10);
		midstr[0]=logstr[3];
		midstr[1]=logstr[4];
		d=str2int(midstr,0);
		GetTime(&ttlog);
		ttlog.tm_mon=m-1;
		ttlog.tm_mday=d;
		ttlog.tm_hour = 0;
		ttlog.tm_min = 0;
		ttlog.tm_sec = 0;
		ttlog.tm_isdst = -1;
		logst = OldEncodeTime(&ttlog);
	} else {
		logst = seltm;
	}

	loged =logst+OneDay-OneSecond;
	if (logstr2) {
		finduser.PIN=1;
		FDB_GetUserByCharPIN2(logstr2,&finduser);
	} else {
		finduser.PIN=0;
	}

	CreateOneLogWindow(hWnd,finduser.PIN,logst,loged);
	return 0;
}

static int GetLogSelData(HWND listwnd,HWND hWnd)
{
	U16 userpin=0;
	HLVITEM hItemSelected;
	LVSUBITEM subitem;
	TUser myuser;
	char findbuff[50];
	DWORD myaddata;

	memset(&myuser,0,sizeof(TUser));
	memset(&subitem,0,sizeof(LVSUBITEM));
	memset(findbuff,0,50);

	hItemSelected=SendMessage(listwnd,LVM_GETSELECTEDITEM,0,0);
	myaddata = SendMessage(listwnd,LVM_GETITEMADDDATA,0,hItemSelected);
	if (hItemSelected==0){
		return userpin;
	}

	subitem.pszText=findbuff;
	if (myaddata==0){
		subitem.subItem=0;
		SendMessage(listwnd,LVM_GETSUBITEMTEXT,hItemSelected,(LPARAM)&subitem);
		if (subitem.pszText[0])	{
			if(noacnopass){
				GetDayrecbystr(hWnd,findbuff,NULL,0);
			} else {
				GetDayrecbystr(hWnd,findbuff,resacno2,0);
			}
		}
	} else if (noacnopass) {
		subitem.subItem=1;
		SendMessage(listwnd,LVM_GETSUBITEMTEXT,hItemSelected,(LPARAM)&subitem);
		if (subitem.pszText[0]) {
			GetDayrecbystr(hWnd,NULL,findbuff,myaddata);
		}
	} else {
		GetDayrecbystr(hWnd,NULL,resacno2,myaddata);
	}
	return 0;
}


static int ControlLogWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HWND waithwnd;
	char hintchar[64]={0};
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_CREATE:
			{
				secondcome=0;
				if (gfont1==NULL) {
					logqyfont2 = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
				} else {
					logqyfont2=gfont1;
				}

				LoadBitmap(HDC_SCREEN,&logmp8,GetBmpPath("pageup.gif"));
				LoadBitmap(HDC_SCREEN,&logmp9,GetBmpPath("pagedown.gif"));

				if(gOptions.TFTKeyLayout!=3) {
					LoadBitmap(HDC_SCREEN,&logmp10,GetBmpPath("function.gif"));
				} else {
					LoadBitmap(HDC_SCREEN,&logmp10,GetBmpPath("fun2.gif"));
				}

				waithwnd = createStatusWin1(hWnd , 250 , 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(MID_WAIT));
				processlogfind(hWnd);
				if (ismenutimeout) {
					destroyStatusWin1(waithwnd);
					return 0;
				}
				destroyStatusWin1(waithwnd);

				SetFocusChild(hLogWnd);
				SendMessage(hLogWnd,LVM_CHOOSEITEM,0,0);
				return 0;
			}
		case MSG_PAINT:
			SetMenuTimeOut(time(NULL));
			hdc=BeginPaint(hWnd);
			SetBkColor(hdc,0x00FFA2BE);
			SelectFont(hdc,logqyfont2);
			SetTextColor(hdc,PIXEL_lightwhite);
			if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,sizeof(hintchar));
				if (fromRight){
					snprintf(hintchar,sizeof(hintchar),"%s",LoadStrByID(MID_PAGEUP));
				} else {
					snprintf(hintchar,sizeof(hintchar),"%s:",LoadStrByID(MID_PAGEUP));
				}

				TextOut(hdc,6+gOptions.GridWidth/2,220,hintchar);
				FillBoxWithBitmap (hdc, 62+gOptions.GridWidth/2, 218,16, 16, &logmp8);
				memset(hintchar,0,sizeof(hintchar));
				if (fromRight) {
					snprintf(hintchar,sizeof(hintchar),"%s",LoadStrByID(MID_PAGEDOWN));
				} else {
					snprintf(hintchar,sizeof(hintchar),"%s:",LoadStrByID(MID_PAGEDOWN));
				}
				TextOut(hdc,90+gOptions.GridWidth/2,220,hintchar);
				FillBoxWithBitmap (hdc, 146+gOptions.GridWidth/2, 218,16, 16, &logmp9);
			}
			memset(hintchar,0,sizeof(hintchar));
			if (fromRight) {
				snprintf(hintchar,sizeof(hintchar),"%s",LoadStrByID(MID_DETAILREC));
			} else {
				snprintf(hintchar,sizeof(hintchar),"%s:",LoadStrByID(MID_DETAILREC));
			}
			TextOut(hdc,210+gOptions.GridWidth/2,220,hintchar);
			FillBoxWithBitmap (hdc, 275+gOptions.GridWidth/2, 218,24, 16, &logmp10);
			EndPaint(hWnd,hdc);
			return 0;
		case MSG_COMMAND:
			switch (wParam)
			{
				case IDCANCEL:
					SendMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;
				case ID_FIND:
					processlogfind(hWnd);
					break;
			}
			break;
		case MSG_KEYUP:
			enterkeydown=0;
			if(3 == gOptions.TFTKeyLayout) {
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout) {
				if(1==keyupFlag){
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
			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) {
				if (GetFocusChild(hWnd)==btnfindlog) {
					SetFocusChild(btnexitlog);
				} else if (GetFocusChild(hWnd)==btnexitlog) {
					SetFocusChild(hLogWnd);
				}
			}

			if ((LOWORD(wParam)==SCANCODE_MENU) || (LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10)){
				if((gOptions.TFTKeyLayout==3) && enterkeydown ){
					break;
				}
				GetLogSelData(hLogWnd,hWnd);
			}

			if (wParam==SCANCODE_F12){
				SendMessage(hLogWnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				processscrollview(hLogWnd,1,10);
			}
			if (wParam==SCANCODE_F11){
				SendMessage(hLogWnd,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				processscrollview(hLogWnd,0,10);
			}
			break;
		case MSG_CLOSE:
			UnloadBitmap(&logmp8);
			UnloadBitmap(&logmp9);
			UnloadBitmap(&logmp10);
			if(tmplogbuf){
				FREE(tmplogbuf);
				tmplogbuf=NULL;
			}
			if (gfont1==NULL){
				DestroyLogFont(logqyfont2);
			}
			DestroyMainWindow(hWnd);
			return TRUE;

	}
	return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

int CreateFindLogWindow(HWND hOwner)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hOwner = GetMainWindowHandle (hOwner);
	InitMiniGUIExt();
	noacnopass=0;
	CreateInfo.dwStyle = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_ATTQUERY);
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ControlLogWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;

	hMainWnd = CreateMainWindow (&CreateInfo);
	if (hMainWnd == HWND_INVALID){
		return -1;
	}

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&Msg, hMainWnd)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	MainWindowThreadCleanup (hMainWnd);

	MiniGUIExtCleanUp();
	hLogWnd=0;
	return 0;
}

int CreateAttLogWindow(HWND hOwner,char *acno, time_t st1, time_t ed1, int realquery)
{
	gQueryLogByMenu = realquery;
	resacno2 = acno;
	resst1=st1;
	resed1=ed1;
	logret=0;
	CreateFindLogWindow(hOwner);
	gRecordCnt = 0;
	return logret;
}
