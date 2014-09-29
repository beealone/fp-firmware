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
#include "t9.h"
#include "main.h"
#include <minigui/tftmullan.h>

#define LD_TEXT 80
#define IDC_TIMERSMS 0xff05

HWND EdText;
HWND hSMSWnd;

BITMAP smsshowbk,barbmp;
BITMAP smsshowbmp1,smsshowbmp2;
char msgContent[MAX_SMS_CONTENT_SIZE*2+1];
int bsmscount;
int curIdx;
int exitcount;
char* sshowhint[6];

PLOGFONT smsshowfont;
PLOGFONT titlefont;

pSmsIdx pSID;
pSmsIdx tpSID;

extern int IsValidTimeDuration(U32 TestTime, PSms sms);
static void LoadsshowHint(void)
{
	sshowhint[0]=LoadStrByID(MID_SMSALLHINT);
	sshowhint[1]=LoadStrByID(MID_PAGEUP);
	sshowhint[2]=LoadStrByID(MID_PAGEDOWN);
	sshowhint[3]=LoadStrByID(MID_SMSCOUNT);
	sshowhint[4]=LoadStrByID(MID_SMSTIAO);
	sshowhint[5]=LoadStrByID(MID_SMSCURINDEX);

}

static void InitWindow (HWND hWnd)
{
	EdText=CreateWindow (CTRL_MLEDIT, "", WS_VISIBLE | WS_VSCROLL |

		WS_BORDER | ES_AUTOWRAP, LD_TEXT, 10, 40,  gOptions.LCDWidth-20,  gOptions.LCDHeight-80, hWnd, 0);


	SendMessage(EdText,EM_SETREADONLY,1,0);
}

static void freeList(void)
{
	pSmsIdx tp;
	if(pSID!=NULL)
	{
		tp=pSID;
		while(tp!=NULL)
		{
			pSID = tp;
			tp = tp->next;
			FREE(pSID);
		}
		pSID=NULL;
	}
}

static int GetIDX2List(int count)
{
	pSmsIdx tmpsid;
	pSmsIdx tpsid;
	TSms tsms;
	int tcount=0;

	pSID = (pSmsIdx)MALLOC(sizeof(TSMSIDX)); //create head node.
	memset(pSID,0,sizeof(TSMSIDX));
	tpsid=pSID;

	if(FDB_InitSmsPoint())
	{
		printf("init fdsms point\n");
		while(FDB_ReadSms(&tsms))
		{
			if(tsms.ID && (tsms.Tag==UDATA_TAG_ALL) && (!IsValidTimeDuration(EncodeTime(&gCurTime), &tsms)))
			{
				tmpsid = (pSmsIdx)MALLOC(sizeof(TSMSIDX));
				memset(tmpsid,0,sizeof(TSMSIDX));
				tmpsid->smsPIN = tsms.ID;
				tmpsid->ID = ++tcount;
				tmpsid->next=NULL;

				tpsid->next = tmpsid;
				tpsid = tmpsid;
			}
		}
	}

	if(tcount==count)
		return 1;
	else
	{
		freeList();
		return 0;
	}
}

static int GetSmsPIN(int ID)
{
	pSmsIdx tmpsid;
	pSmsIdx tpsid;

	tmpsid  = pSID;
	tpsid = tmpsid;
	while(tmpsid!=NULL)
	{
		tpsid = tmpsid;
		tmpsid= tmpsid->next;
		if(tpsid->ID==ID)
			return (tpsid->smsPIN);
	}
	return 0;
}

static void ShowMessage(HWND hWnd, int sid)
{
	TSms tsms;
	memset(&tsms,0,sizeof(TSms));
	if(FDB_GetSms(sid,&tsms)!=NULL)
	{
		memset(msgContent,0,MAX_SMS_CONTENT_SIZE*2+1);

		Str2UTF8(tftlocaleid,tsms.Content, (unsigned char*)msgContent);	////modify by jazzy 2008.07.26
		//nmemcpy(msgContent,tsms.Content,MAX_SMS_CONTENT_SIZE*2);

		SetWindowText(EdText,msgContent);
	}
	UpdateWindow(hWnd,TRUE);

}

static int boardwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char sstr[20];
	static char keyupFlag=0;
	int tmpvalue = 0;
	switch (message)
	{
		case MSG_CREATE:
			LoadBitmap(HDC_SCREEN,&smsshowbk,GetBmpPath("sms.jpg"));
			LoadBitmap(HDC_SCREEN,&barbmp, GetBmpPath("bar.bmp"));
			LoadBitmap(HDC_SCREEN,&smsshowbmp1,GetBmpPath("left2.gif"));
			LoadBitmap(HDC_SCREEN,&smsshowbmp2,GetBmpPath("right2.gif"));

			if (gfont==NULL)
			{
				smsshowfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
						FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,10, 0);
				titlefont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
						FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
			}
			else
			{
				smsshowfont=gfont;
				titlefont=gfont1;
			}
			LoadsshowHint();
			InitWindow(hWnd);		//add controls
			curIdx=1;
			ShowMessage(hWnd,GetSmsPIN(curIdx));

			exitcount = 0;
#ifdef ZEM600
			SetTimer(hWnd,IDC_TIMERSMS,50);
#else
			SetTimer(hWnd,IDC_TIMERSMS,100);
#endif
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);

			//write title
			SelectFont(hdc,titlefont);
			SetTextColor(hdc,PIXEL_lightwhite);
			tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
			memset(sstr,0,20);
			sprintf(sstr,"%s",sshowhint[0]);
			TextOut(hdc,140,15,sstr);

			//                        SetBkColor(hdc,0x00FFA2BE);
			//SelectFont(hdc,smsshowfont);
			SelectFont(hdc,titlefont);
			SetTextColor(hdc,PIXEL_lightwhite);

			memset(sstr,0,20);
			if (fromRight)
				sprintf(sstr,"%s",sshowhint[1]);
			else
				sprintf(sstr,"%s:",sshowhint[1]);
			TextOut(hdc,5,220,sstr);
			FillBoxWithBitmap (hdc, 65, 218,16, 16, &smsshowbmp1);

			memset(sstr,0,20);
			if (fromRight)
				sprintf(sstr,"%s",sshowhint[2]);
			else
				sprintf(sstr,"%s:",sshowhint[2]);
			TextOut(hdc,90,220,sstr);
			FillBoxWithBitmap (hdc, 140, 218,16, 16, &smsshowbmp2);

			memset(sstr,0,20);
			if (fromRight)
				sprintf(sstr,"%s%d:%s,%s%d:%s",sshowhint[3],bsmscount,sshowhint[4],sshowhint[5],curIdx,sshowhint[4]);
			else
				sprintf(sstr,"%s:%d%s,%s:%d%s",sshowhint[3],bsmscount,sshowhint[4],sshowhint[5],curIdx,sshowhint[4]);
			TextOut(hdc,170,220,sstr);

			EndPaint(hWnd,hdc);
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

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight-30, &smsshowbk);
				FillBoxWithBitmap (hdc, 0, 210, gOptions.LCDWidth,30,&barbmp);

				if(fGetDC) ReleaseDC (hdc);
				break;
			}

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
			exitcount=0;

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
				PostMessage(hWnd,MSG_CLOSE,0,0L);

			/*SMS快捷键查看，modify by yangxiaolong,start*/
			//del
			/*
			   if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN || LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			   {
			   SendMessage(EdText,MSG_KEYDOWN,wParam,lParam);
			   }*/
			//add
			//快捷键查看SMS时，上下键翻动换行反应缓慢，调成翻页处理
			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				SendMessage(EdText,MSG_KEYDOWN,SCANCODE_PAGEDOWN,lParam);
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				SendMessage(EdText,MSG_KEYDOWN,SCANCODE_PAGEUP,lParam);
			}
			/*SMS快捷键查看，modify by yangxiaolong,end*/	

			if(LOWORD(wParam)==SCANCODE_F11)
				SendMessage(EdText,MSG_KEYDOWN,SCANCODE_PAGEUP,lParam);
			if(LOWORD(wParam)==SCANCODE_F12)
				SendMessage(EdText,MSG_KEYDOWN,SCANCODE_PAGEDOWN,lParam);

			//if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT && bsmscount>1)
			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || (LOWORD(wParam)==SCANCODE_BACKSPACE && 3==gOptions.TFTKeyLayout)) && bsmscount>1)
			{
				if(--curIdx < 1) curIdx = bsmscount;
				ShowMessage(hWnd,GetSmsPIN(curIdx));
			}

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT && bsmscount>1)
			{
				if(++curIdx > bsmscount) curIdx = 1;
				ShowMessage(hWnd,GetSmsPIN(curIdx));
			}

			break;

		case MSG_TIMER:
			if(wParam==IDC_TIMERSMS)
			{
				if(++exitcount>60) SendMessage(hWnd,MSG_CLOSE,0,0);
			}
			break;

		case MSG_CLOSE:
			UnloadBitmap(&smsshowbk);
			UnloadBitmap(&barbmp);
			UnloadBitmap(&smsshowbmp1);
			UnloadBitmap(&smsshowbmp2);
			if (gfont1==NULL)
			{
				DestroyLogFont(smsshowfont);
				DestroyLogFont(titlefont);
			}
			KillTimer(hWnd,IDC_TIMERSMS);
			freeList();
			DestroyMainWindow(hWnd);
			//hSMSWnd=NULL;
			break;

		default:
			return DefaultMainWinProc(hWnd,message,wParam,lParam);

	}
	return (0);

}

int CreateSMSBoard(HWND hWnd, int smscount)
{
	MSG msg;
	MAINWINCREATE CreateInfo;

	bsmscount = smscount;
	//	printf("bsmscount:%d\n",bsmscount);
	if(!GetIDX2List(bsmscount)) return 0;

	hWnd= GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE;// | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = boardwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;//g_rcScr.right;
	CreateInfo.by = gOptions.LCDHeight;//g_rcScr.bottom;
	//CreateInfo.iBkColor = COLOR_lightgray;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hSMSWnd = CreateMainWindow(&CreateInfo);
	if (hSMSWnd == HWND_INVALID)
		return 0;
	ShowWindow(hSMSWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hSMSWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hSMSWnd);
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

