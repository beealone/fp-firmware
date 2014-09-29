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

#define IDL_USER 9987


HWND LTUser;

int OptMod_send;
int smstmppin=0;			//sms id
//BITMAP smssendbmpgbk;
BITMAP smssendbmps1;
BITMAP smssendbmps2;
BITMAP smssendbmps3;
BITMAP smssendbmps4;

PLOGFONT hintfont;
PUDataLink tmppudlk=NULL;
int issend=0;
int isselchange=0;
int listcount=0;
extern int userdatacuroffset_e;
extern int userdatacuroffset_s;

static void InitWindow (HWND hWnd)
{
	LTUser = CreateWindow (CTRL_LISTBOX,CTRL_LISTBOX, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
		LBS_CHECKBOX,//LBS_USEICON,
               	IDL_USER,0, 0, gOptions.LCDWidth, gOptions.LCDHeight-50, hWnd, 0);

}

/*
static int ismodified(void)
{
        TUData udata;
	int i,usridx;

	for(i=0;i<listcount;i++)
	{
		usridx = SendMessage(LTUser,LB_GETITEMADDDATA,i,0);
		memset(&udata,0,sizeof(TUData));
		if(FDB_GetUData(usridx,&udata)!=NULL)
		{
			if(SendMessage(LTUser,LB_GETCHECKMARK,i,0)==CMFLAG_BLANK)
				return 1;
		}
		else
		{
			if(SendMessage(LTUser,LB_GETCHECKMARK,i,0)==CMFLAG_CHECKED)
				return 1;
		}
        }

	return 0;

}


static int bHaveSelected(void)	//if have selected item
{
	int i;
	for(i=0;i<listcount;i++)
	{
		if(SendMessage(LTUser,LB_GETCHECKMARK,i,0)==CMFLAG_CHECKED) return 1;
	}
	return 0;
}

static int SaveSmsSendInfo(void)
{
	PUDataLink pdlk,ptmp;
	int i,usridx;

	pdlk = tmppudlk;	//save first point
	for(i=0;i<listcount;i++)
	{
		usridx = SendMessage(LTUser,LB_GETITEMADDDATA,i,0);

		ptmp = (PUDataLink)MALLOC(sizeof(TUDataLink));
		memset(ptmp,0,sizeof(TUDataLink));
		ptmp->PIN = usridx;
		ptmp->bSelect = (SendMessage(LTUser,LB_GETCHECKMARK,i,0)==CMFLAG_CHECKED)?1:0;
		pdlk->nextnode = ptmp;
		pdlk = pdlk->nextnode;
	}
	return 1;
}
*/

static void ChangeSmsSendInfo(int usridx, int bSelect)
{
	PUDataLink pdlk,ptmp;
	int findflag = 0;
	if(tmppudlk == NULL) {
		ptmp = (PUDataLink)MALLOC(sizeof(TUDataLink));
		memset(ptmp,0,sizeof(TUDataLink));
		ptmp->PIN = usridx;
		ptmp->bSelect = bSelect;
		tmppudlk = ptmp;
		return;
	}
	pdlk = tmppudlk;
	while(pdlk != NULL) {
		if(pdlk->PIN == usridx) {
			printf("______%s,%d,bSelect = %d\n", __FILE__, __LINE__, bSelect);
			pdlk->bSelect = bSelect;
			findflag = 1;
			break;
		}
		ptmp = pdlk;
		pdlk = ptmp->nextnode;
	}
	pdlk = ptmp;
	if(!findflag) {
		ptmp = (PUDataLink)MALLOC(sizeof(TUDataLink));
		memset(ptmp,0,sizeof(TUDataLink));
		ptmp->PIN = usridx;
		ptmp->bSelect = bSelect;
		pdlk->nextnode = ptmp;
	}
}

static void ClearItemString(HWND hwnd)
{
	//int i;
	SendMessage(LTUser,LB_RESETCONTENT, 0, 0);
	//for(i=0; i<listcount; i++) {
	//	SendMessage(LTUser,LB_DELETESTRING, i, 0);
	//}
	listcount = 0;
}
static int CheckUserSMS_Select(U16 userpin, PUDataLink tmpusms)
{
	int ret = -1;
	PUDataLink tmp = tmpusms;
	while(tmp) {
		if(tmp->PIN == userpin) {
			return tmp->bSelect;
		}
		tmp = tmp->nextnode;
	}
	return ret;
}
extern void FreePUserlb(PUserlb tmpuserlb);
static void FillUserListData(int OptMod, int directflag)
{
//	HICON myhIcon;
	LISTBOXITEMINFO lbii;

	U16 usrPIN;
	char *(usrInfo[2]);
	char tmpstr[40];
	char tmp999[10];
	TUData udata;
	int usercnt = 0;
	int checkflag = -1;
//	myhIcon = LoadIconFromFile(HDC_SCREEN,"",1);	//加载图标

        PUserlb puserlb,tmpuserlb;
	PUserlb pb1,userlb1;
        //puserlb = GetUsermestolb(0);
        /*zxz 2012-11-20*/
	tmpuserlb = NULL;
	puserlb = NULL;
	userlb1 = NULL;
	pb1 = NULL;
	if(OptMod!=0) {
		if(directflag == DIRECTINON_FLAG_FORWARD) {
			puserlb = GetUsermestolb_PageForward(0, &usercnt, smstmppin);
		} else {
			puserlb = GetUsermestolb_PageBackward(0, &usercnt, smstmppin);
		}
		if(usercnt > 0) {
			ClearItemString(LTUser);
		} else {
			return;
		}
		SendMessage(LTUser,LB_SETITEMHEIGHT,0,24);
		
		if(directflag && (gOptions.PIN2Width > 5)){
			tmpuserlb = puserlb;
			while(puserlb != NULL)
			{
				//printf("pin2=%s\n",puserlb->userlb.PIN2);
				pb1 = (PUserlb)malloc(sizeof(TUserlb));
				memset(pb1, 0, sizeof(TUserlb));
				memcpy(&pb1->userlb, &puserlb->userlb, sizeof(TUser));
				pb1->next = userlb1;
				userlb1 = pb1;
				puserlb = puserlb->next;
			}
			FreePUserlb(tmpuserlb);
			puserlb = userlb1;
			tmpuserlb = puserlb;
		}
	}  else {
		puserlb = GetUsermestolb_SMS(smstmppin);
	}
	listcount=0;
	tmpuserlb = puserlb;
	
        while (puserlb != NULL)
        {
		usrPIN=puserlb->userlb.PIN;
		checkflag = CheckUserSMS_Select(usrPIN, tmppudlk);
		usrInfo[0]=puserlb->userlb.PIN2;

		char mynamename[100];		//modify by jazzy 2008.07.26
		memset(mynamename,0,100);
		Str2UTF8(tftlocaleid,(unsigned char*)puserlb->userlb.Name, (unsigned char*)mynamename);
		usrInfo[1]=mynamename;
	        //usrInfo[1]=puserlb->userlb.Name;
		memset(&udata,0,sizeof(TUData));
		if(FDB_GetUData(usrPIN,&udata)==NULL)	//用户未分配短消息
		{
			/*OptMod 0表示查看短消息，1表示短消息分发*/
			if(OptMod!=0)
			{
				
				memset(tmpstr,0,40);
				sprintf(tmp999,"%%%ds   %%s",gOptions.PIN2Width);
				sprintf(tmpstr,tmp999,usrInfo[0],usrInfo[1]);
				lbii.string = tmpstr;
				if(checkflag == 0 || checkflag == -1) {
					lbii.cmFlag = CMFLAG_BLANK;
				} else {
					lbii.cmFlag = CMFLAG_CHECKED;
				} 
				lbii.hIcon = 0;//myhIcon;
				SendMessage(LTUser,LB_ADDSTRING,0,(LPARAM)&lbii);
				SendMessage(LTUser,LB_SETITEMADDDATA,listcount++,usrPIN);
			}
		}
		else					//用户已分配短消息
		{
			if((OptMod==1 || OptMod==0) && udata.SmsID == smstmppin)
			{
				memset(tmpstr,0,40);
				sprintf(tmp999,"%%%ds   %%s",gOptions.PIN2Width);
				sprintf(tmpstr,tmp999,usrInfo[0],usrInfo[1]);
				lbii.hIcon = 0;// myhIcon;
				if(checkflag == 1 || checkflag == -1) {
					lbii.cmFlag = CMFLAG_CHECKED;
				} else {
					lbii.cmFlag = CMFLAG_BLANK;
				}
				lbii.string = tmpstr;
				SendMessage(LTUser,LB_ADDSTRING,0,(LPARAM)&lbii);
				SendMessage(LTUser,LB_SETITEMADDDATA,listcount++,usrPIN);
			}
		}

                puserlb = puserlb->next;
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
                }
		tmptmpuserlb = NULL;
	}

	SendMessage(LTUser,LB_SETCURSEL,0,0);
	//if(directflag == DIRECTINON_FLAG_FORWARD){
	//	SendMessage(LTUser,LB_SETCURSEL,0,0);
	//} else {
	//	SendMessage(LTUser,LB_SETCURSEL,(GRIDVIEWROWCNT-1),0);
	//}
}

static int processscrollview_SMS(HWND listwnd, int down,int incseed)
{
	int additemindex;
	additemindex = SendMessage(listwnd,LB_GETCURSEL,0,0);
	if (additemindex<0) {
		SendMessage(listwnd,LB_SETCURSEL,0,0);
		return 0;
	}
	
	if(down == DIRECTINON_FLAG_FORWARD){
		if(additemindex >= (GRIDVIEWROWCNT-1)) {
			FillUserListData(OptMod_send, DIRECTINON_FLAG_FORWARD);
			return 1;
		}
	} else {
		if(additemindex <= 0) {
			FillUserListData(OptMod_send, DIRECTINON_FLAG_BACKWARD);
			return 1;
		}
	}
	
	if (down) {
		additemindex--;
		SendMessage(listwnd,LB_SETCURSEL,additemindex,0);
	} else {
		additemindex++;
		SendMessage(listwnd,LB_SETCURSEL,additemindex,0);
	}
	return 1;
}

static int smssendwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char hintchar[20];
	int selindex;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
//			if(LoadBitmap(HDC_SCREEN,&smssendbmpgbk,GetBmpPath("submenubg.jpg")))
//	                        return 0;

                	LoadBitmap(HDC_SCREEN,&smssendbmps1,GetBmpPath("pageup.gif"));
	                LoadBitmap(HDC_SCREEN,&smssendbmps2,GetBmpPath("pagedown.gif"));
	                LoadBitmap(HDC_SCREEN,&smssendbmps3,GetBmpPath("okkey.gif"));
			if(gOptions.TFTKeyLayout!=3)
		                LoadBitmap(HDC_SCREEN,&smssendbmps4,GetBmpPath("function.gif"));
			else
		                LoadBitmap(HDC_SCREEN,&smssendbmps4,GetBmpPath("fun2.gif"));

			if (gfont1==NULL)
               			hintfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_LIGHT, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);

			InitWindow(hWnd);		//add controls
			userdatacuroffset_e = 0;
			userdatacuroffset_s = 0;
			FillUserListData(OptMod_send, DIRECTINON_FLAG_FORWARD);

			UpdateWindow(hWnd,TRUE);
			SetFocusChild(LTUser);
			SendMessage(LTUser,LB_SETCURSEL,0,0);
			break;
#if 0
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

			FillBoxWithBitmap (hdc, 0, 0, 0, 0, &smssendbmpgbk);
			if(fGetDC)
				ReleaseDC (hdc);
			return 0;
		}
#endif
		case MSG_PAINT:
			hdc=BeginPaint(hWnd);

//			SetBkMode(hdc,BM_TRANSPARENT);
                        SetBkColor(hdc,0x00FFA2BE);
			if (gfont1==NULL)
                        	SelectFont(hdc,hintfont);
			else	SelectFont(hdc,gfont1);
                        SetTextColor(hdc,PIXEL_lightwhite);

			if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,20);
				if (fromRight)
					 sprintf(hintchar,"%s",LoadStrByID(MID_PAGEUP));
				else
        	                	sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEUP));
                	        TextOut(hdc,5,196,hintchar);
                        	FillBoxWithBitmap (hdc,60,194,16,16,&smssendbmps1);

	                        memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEDOWN));
                                else
        	                	sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEDOWN));
                	        TextOut(hdc,90,196,hintchar);
	                        FillBoxWithBitmap (hdc,140,194,16,16,&smssendbmps2);
			}
			if(OptMod_send!=0)
			{
	                        memset(hintchar,0,20);
				if (fromRight)
					sprintf(hintchar,"%s",LoadStrByID(MID_SMSSELECT));
                                else
       		                	sprintf(hintchar,"%s:",LoadStrByID(MID_SMSSELECT));
                	        TextOut(hdc,175,196,hintchar);
                       		FillBoxWithBitmap (hdc, 215, 194,16, 16, &smssendbmps3);
			}

                        memset(hintchar,0,20);
			if (fromRight)
				sprintf(hintchar,"%s",LoadStrByID(MID_SMSCANCEL));
                        else
                        	sprintf(hintchar,"%s:",LoadStrByID(MID_SMSCANCEL));
                        TextOut(hdc,250,196,hintchar);
                        FillBoxWithBitmap (hdc, 285, 195,24, 16, &smssendbmps4);

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

			if ((LOWORD(wParam)==SCANCODE_ESCAPE) || (LOWORD(wParam)==SCANCODE_MENU))	//退出
			{
				PostMessage(hWnd,MSG_CLOSE,0,0L);
			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
			 {
			 	if(OptMod_send !=0 ) {
					FillUserListData(OptMod_send, DIRECTINON_FLAG_BACKWARD);
			 	} else {
			 		SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
			 	}
				return 0;
			 }

			 if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT ){
			 	if(OptMod_send !=0 ) {
					FillUserListData(OptMod_send, DIRECTINON_FLAG_FORWARD);
			 	} else {
			 		SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
			 	}
				return 0;
			 }

			if ((LOWORD(wParam)==SCANCODE_ENTER || LOWORD(wParam)==SCANCODE_F10) && OptMod_send!=0) //选择/取消
			{
				int usridx;
				isselchange = 1;
				issend = 1;
				selindex = SendMessage(LTUser,LB_GETCURSEL,0,0);
				usridx = SendMessage(LTUser, LB_GETITEMADDDATA, selindex, 0);
				if(SendMessage(LTUser,LB_GETCHECKMARK,selindex,0)!=CMFLAG_CHECKED) {
					ChangeSmsSendInfo(usridx, 1);
					SendMessage(LTUser,LB_SETCHECKMARK,selindex,(LPARAM)CMFLAG_CHECKED);
				} else {
					ChangeSmsSendInfo(usridx, 0);
					SendMessage(LTUser,LB_SETCHECKMARK,selindex,(LPARAM)CMFLAG_BLANK);
				}
				
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_F12)
			{
				if(OptMod_send !=0 ) {
					FillUserListData(OptMod_send, DIRECTINON_FLAG_FORWARD);
			 	} else {
			 		SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
			 	}
				return 0;
				//SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				//processscrollview_SMS(LTUser,DIRECTINON_FLAG_FORWARD,GRIDVIEWROWCNT);
				//SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_CURSORBLOCKDOWN,0);
				//return 0;
			}

			if(LOWORD(wParam)==SCANCODE_F11)
			{
				if(OptMod_send !=0 ) {
					FillUserListData(OptMod_send, DIRECTINON_FLAG_BACKWARD);
			 	} else {
			 		SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
			 	}
				return 0;
				//SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				//processscrollview_SMS(LTUser,DIRECTINON_FLAG_BACKWARD, GRIDVIEWROWCNT);
				//SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_CURSORBLOCKUP,0);
				//return 0;
			}

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if(OptMod_send !=0 ) {
					processscrollview_SMS(LTUser,DIRECTINON_FLAG_BACKWARD, GRIDVIEWROWCNT);
				}else{
					SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_CURSORBLOCKUP,0);
				}	
				return 0;
			}
			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				
				if(OptMod_send !=0 ) {
					processscrollview_SMS(LTUser,DIRECTINON_FLAG_FORWARD,GRIDVIEWROWCNT);
				}else{
					SendMessage(LTUser,MSG_KEYDOWN,SCANCODE_CURSORBLOCKDOWN,0);
				}	
				return 0;
			}
			break;

		case MSG_CLOSE:
			if(OptMod_send!=0)
			{
				//SaveSmsSendInfo();
				//isselchange = ismodified();
				//issend = bHaveSelected();
				//isselchange = 1;
				//issend = 1;
			}
			else
			{
				isselchange = 0;
				issend = 0;
			}

//			UnloadBitmap(&smssendbmpgbk);
			UnloadBitmap(&smssendbmps1);
			UnloadBitmap(&smssendbmps2);
			UnloadBitmap(&smssendbmps3);
			UnloadBitmap(&smssendbmps4);
			if (gfont1==NULL)
				DestroyLogFont(hintfont);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateSmsSendWindow(HWND hWnd, int optmod, int *SMSPIN,int *bchanged, PUDataLink pudlk)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	char sstr[20];
	OptMod_send=optmod;
	smstmppin=*SMSPIN;
	tmppudlk = pudlk;
	isselchange = 0;
	issend = 1;

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	if(OptMod_send==0)
	{
		memset(sstr,0,20);
		sprintf(sstr,"%s%s",LoadStrByID(MID_SMSCHECK),LoadStrByID(MID_SMSEDITCAPTION1));
		CreateInfo.spCaption = sstr;
	}
	else
		CreateInfo.spCaption = LoadStrByID(MID_SMSSENDCAPTION);

	CreateInfo.hMenu = 0;
//	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = smssendwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(OptMod_send) *bchanged = isselchange;

	MainWindowThreadCleanup(hMainWnd);
	return ((OptMod_send)?issend:0);
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

