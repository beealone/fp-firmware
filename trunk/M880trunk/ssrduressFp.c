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
#include "tftmsgbox.h"
#include "finger.h"

//BITMAP mdurbmpbg;
BITMAP mdurbmps1;
BITMAP mdurbmps2;
BITMAP mdurbmps3;
BITMAP mdurbmps4;
BITMAP mdurbmps5;
//menu icon
static BITMAP micon1;
static BITMAP micon2;
static BITMAP micon3;

#define IDL_FP 80
HWND LTDuressFp;
static int usrid = 0;	//用户编号
static int listcount=0;
static int tempenroll[MAX_USER_FINGER_COUNT];

int isdfp[10];		//是否为胁迫指纹（数组下标为指纹序号，值为胁迫标志）
int dfpcounts=0;
int DuressFp=0;

PLOGFONT hintfont,captionfont;

static void InitWindow(HWND hWnd)
{
	memset((void*)usertmp,0,sizeof(usertmp));
	memset((void *)usertmplen,0,sizeof(usertmplen));
	memset((void *)userisenroll,0,sizeof(userisenroll));
	isfpdbload=0;
	LTDuressFp=CreateWindow(CTRL_LISTBOX,CTRL_LISTBOX,WS_CHILD|WS_VISIBLE|WS_BORDER|WS_VSCROLL|LBS_CHECKBOX,IDL_FP,0,25,gOptions.LCDWidth, gOptions.LCDHeight-50,hWnd,0);
}

static HMENU CreateQuickMenu(HWND hWnd)
{
	int i;
	HMENU hNewMenu;
	MENUITEMINFO mii;
	HMENU hMenuFloat;
	char* menustr[3];

	menustr[0]=LoadStrByID(MID_ENROLL_FP);
	menustr[1]=LoadStrByID(MID_CANCEL_ALL);
	menustr[2]=LoadStrByID(MID_DFP_SAVE);

	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.type = MFT_BMPSTRING;
	mii.id = 0;
	mii.uncheckedbmp = &micon1;	//
	mii.typedata = (DWORD)"opt";

	hNewMenu = CreatePopupMenu(&mii);
	for (i=0; i<3; i++)
	{
		memset(&mii,0,sizeof(MENUITEMINFO));
		mii.type = MFT_BMPSTRING;
		mii.id = 200+i;
		mii.state = 0;
		switch (i)
		{
			case 0:
				mii.uncheckedbmp=&micon1;
				break;
			case 1:
				mii.uncheckedbmp=&micon2;
				break;
			case 2:
				mii.uncheckedbmp=&micon3;
				break;
		}
		mii.typedata= (DWORD)menustr[i];
		InsertMenuItem(hNewMenu,i,TRUE,&mii);
	}
	hMenuFloat = StripPopupHead(hNewMenu);
	TrackPopupMenu(hMenuFloat,TPM_LEFTALIGN|TPM_VCENTERALIGN,5,160,hWnd);

	return hMenuFloat;
}

static int ismodified(void)
{
	int i;
	if(listcount != dfpcounts) return 1;
	for(i=0;i<listcount;i++)
	{
		if(FDB_IsDuressTmp(usrid, (char)i))
		{
			if(SendMessage(LTDuressFp, LB_GETCHECKMARK, i, 0)==CMFLAG_BLANK)
				return 1;
		}
		else
		{
			if(SendMessage(LTDuressFp, LB_GETCHECKMARK, i, 0)==CMFLAG_CHECKED)
				return 1;
		}
	}
	return 0;
}
/*
   static int bHaveSelected(void)	//if have selected item
   {
   int i;
   for(i=0;i<listcount;i++)
   {
   if(SendMessage(LTDuressFp,LB_GETCHECKMARK,i,0)==CMFLAG_CHECKED)
   return 1;
   }
   return 0;
   }
   */
static int SaveDuressFp(void)
{
	int i,j,findex;
	BYTE valid=0;
	TZKFPTemplate template;

	for(i=0;i<listcount;i++)
	{
		j= FDB_ERROR_IO;
		findex=SendMessage(LTDuressFp,LB_GETITEMADDDATA,i,0);

		if(SendMessage(LTDuressFp, LB_GETCHECKMARK,i,0)==CMFLAG_CHECKED)
			valid=1|DURESSFINGERTAG;
		else
			valid=1;

		memset(&template,0,sizeof(template));
		if(FDB_GetTmp(usrid, (char)findex, (char *)&template)==0)
		{
			//登记新指纹
			if(usertmplen[findex])
			{
				j = FDB_AddTmp((char *)FDB_CreateTemplate((char*)&template, usrid, (char)findex, usertmp[findex], usertmplen[findex],valid));
			}
		}
		else
		{
			if(gOptions.ZKFPVersion == ZKFPV10)
				FDB_SetDuressTemp(usrid,findex,valid);
			else
			{
				if(SendMessage(LTDuressFp, LB_GETCHECKMARK,i,0)==CMFLAG_CHECKED)
				{
					FDB_ChgTmpValidTag((PTemplate)&template, DURESSFINGERTAG, 0);
				}
				else
					FDB_ChgTmpValidTag((PTemplate)&template, 1, 0xFE);
			}
		}

		if(j==FDB_OK)
		{
			char Buffer[6+4];
			((U16*)Buffer)[2]=0;
			memcpy(Buffer+4, &template, 6);
			CheckSessionSend(EF_ENROLLFINGER,Buffer+2, 6+2);
			FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, usrid, j, i, usertmplen[findex]);
			sync();
		}
	}
	sync();
	return 1;
}

static void addListData(int usrPIN, int rows, int fingerid,int flag)
{
	//      HICON myhIcon;
	LISTBOXITEMINFO lbii;
	char tmpstr[60];
	TUser dusr;
	int rows1=rows;
	SendMessage(LTDuressFp,LB_SETITEMHEIGHT,0,24);

	memset(tmpstr,0,sizeof(tmpstr));
	memset(&dusr, 0, sizeof(TUser));
	if(FDB_GetUser(usrPIN, &dusr)!=NULL)
	{
		//add by jazzy 2009.05.18
		if ((rows>=5)&&(rows<=9))
			rows1-=5;
		else if((rows<5)&&(rows>=0))
			rows1=9-rows;

		sprintf(tmpstr,"%5s-%02d        %s%02d%s",dusr.PIN2,rows1+1,LoadStrByID(MID_FP_HINT1),listcount+1,LoadStrByID(MID_FP_HINT2));
	}
	lbii.string = tmpstr;
	lbii.cmFlag = (flag)?CMFLAG_CHECKED:CMFLAG_BLANK;
	lbii.hIcon = 0;//myhIcon;
	SendMessage(LTDuressFp,LB_ADDSTRING,0,(LPARAM)&lbii);

	SendMessage(LTDuressFp,LB_SETITEMADDDATA,listcount,(LPARAM)rows);
}

static void FillFpListData(void)
{
	TZKFPTemplate template;
	int i;

	listcount = 0;
	dfpcounts =0;

	memset(&template, 0, sizeof(template));
	for (i=0;i<MAX_USER_FINGER_COUNT;i++)
	{
		if (FDB_GetTmp(usrid, (char)i, (char *)&template)!=0)
		{
			if(FDB_IsDuressTmp(usrid,(char)i))
				addListData(usrid,i,template.FingerID,1);
			else
				addListData(usrid,i,template.FingerID,0);
			listcount++;
			dfpcounts++;
			memset(&template,0,sizeof(TZKFPTemplate));
		}
	}
}

static int EnrollDuressFp(HWND hWnd)
{
	int i;
	int fingerid;
	TUser user;

	//clear template buffer
	/*memset((void*)usertmp,0,sizeof(usertmp));
	  memset((void *)usertmplen,0,sizeof(usertmplen));
	  memset((void *)userisenroll,0,sizeof(userisenroll));*/

	if(FDB_CntTmp()>=gOptions.MaxFingerCount*100)
	{
		myMessageBox1(hWnd, MB_OK | MB_ICONSTOP, LoadStrByID(MID_APPNAME), LoadStrByID(HID_EXCEED));
		return 0;
	}
	for(i=0;i<MAX_USER_FINGER_COUNT;i++)
	{
		if(0!=FDB_GetTmp((U16)usrid,(char)i,NULL))
			userisenroll[i] = 1;
	}

	//fingerid = listcount;
	fingerid = -1;
	// must deside whether other finger already enrooled
	for (i=0;i<MAX_USER_FINGER_COUNT;i++)
	{
		if (userisenroll[i] == 0)
		{
			fingerid = i;
			break;
		}
	}

	if(fingerid==-1)
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_EXCEEDFINGER));
		return 0;
	}

	usertmplen[fingerid]=0;
	memset(&user,0,sizeof(TUser));
	if(FDB_GetUser(usrid,&user)!=NULL)
	{
		for (i = 0; i < MAX_USER_FINGER_COUNT; i++)
			tempenroll[i] = userisenroll[i];

		DuressFp=1;
		CreateFpEnrollWindow(hWnd,usertmp[fingerid],&usertmplen[fingerid],fingerid,user.PIN2,1);
		if(ismenutimeout) 
			return 0;

		for(i=0;i<MAX_USER_FINGER_COUNT;i++)
		{
			if(tempenroll[i] != userisenroll[i])
			{
				addListData(usrid,i,fingerid,1);
				listcount++;
			}
		}
		SendMessage(LTDuressFp, LB_SETCURSEL, listcount-1, 0);
	} 
	else
	{
		return 0;
	}
	return 1;
}

extern int processscrollview(HWND listwnd, int down, int incseed);
extern int DelFingerFromCache(HANDLE Handle);
static int DuressFpwinproc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char hintchar[50];
	//char tmpstr[20];
	int selindex;
	int i, tmpvalue;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&mdurbmpbg,GetBmpPath("submenubg.jpg")))
			//	return 0;

			LoadBitmap(HDC_SCREEN,&mdurbmps1,GetBmpPath("pageup.gif"));
			LoadBitmap(HDC_SCREEN,&mdurbmps2,GetBmpPath("pagedown.gif"));
			LoadBitmap(HDC_SCREEN,&mdurbmps3,GetBmpPath("okkey.gif"));
			if(gOptions.TFTKeyLayout!=3)
				LoadBitmap(HDC_SCREEN,&mdurbmps4,GetBmpPath("function.gif"));
			else
				LoadBitmap(HDC_SCREEN,&mdurbmps4,GetBmpPath("fun2.gif"));
			LoadBitmap(HDC_SCREEN,&mdurbmps5,GetBmpPath("back.gif"));
			LoadBitmap(HDC_SCREEN,&micon1,GetBmpPath("add.gif"));
			LoadBitmap(HDC_SCREEN,&micon2,GetBmpPath("delete.gif"));
			LoadBitmap(HDC_SCREEN,&micon3,GetBmpPath("edit.gif"));

			if (gfont==NULL)
			{
				hintfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_LIGHT, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
						FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
				captionfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_LIGHT, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
						FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,14, 0);
			}
			else
			{
				captionfont=gfont;
				hintfont=gfont1;
			}
			InitWindow(hWnd);		//add controls
			FillFpListData();
			UpdateWindow(hWnd,TRUE);
			SetFocusChild(LTDuressFp);
			SendMessage(LTDuressFp, LB_SETCURSEL, 0, 0);
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

			//display listbox caption
			SelectFont(hdc,captionfont);
			memset(hintchar,0,50);
			sprintf(hintchar,"%s",LoadStrByID(MID_LIST_CAPTION));
			TextOut(hdc,30,5,hintchar);

			//display operation hint
			SelectFont(hdc,hintfont);
			if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,20);
				sprintf(hintchar,"%s", LoadStrByID(MID_PAGEUP));
				TextOut(hdc,10,218,hintchar);
				FillBoxWithBitmap (hdc,60,216,16,16,&mdurbmps1);

				memset(hintchar,0,20);
				sprintf(hintchar,"%s", LoadStrByID(MID_PAGEDOWN));
				TextOut(hdc,90,218,hintchar);
				FillBoxWithBitmap (hdc,140, 216,16, 16, &mdurbmps2);
			}

			memset(hintchar,0,20);
			sprintf(hintchar,"%s", LoadStrByID(MID_DF_CC));
			TextOut(hdc,170,218,hintchar);
			if(gOptions.TFTKeyLayout!=3)
				FillBoxWithBitmap (hdc, 210, 216,16, 16, &mdurbmps3);
			else
				FillBoxWithBitmap (hdc, 210, 216,16, 16, &mdurbmps5);
			memset(hintchar,0,20);
			sprintf(hintchar,"%s", LoadStrByID(MID_SMSMENU));
			TextOut(hdc,250,218,hintchar);
			FillBoxWithBitmap (hdc, 285, 216,24, 16, &mdurbmps4);

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

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))	//退出
			{
				if(ismodified() && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA),LoadStrByID(MID_APPNAME),
							MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK)
					PostMessage(hWnd, MSG_COMMAND, 202, 0);
				else
				{
					if(!ismenutimeout)
						PostMessage(hWnd, MSG_CLOSE,0,0L);
				}
				return 0;
			}

			if(((gOptions.TFTKeyLayout!=3)&&((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10)))||
					((gOptions.TFTKeyLayout==3) && (LOWORD(wParam)==SCANCODE_BACKSPACE))) //选择/取消
			{
				selindex = SendMessage(LTDuressFp, LB_GETCURSEL,0,0);
				if(SendMessage(LTDuressFp,LB_GETCHECKMARK,selindex,0)!=CMFLAG_CHECKED)
					SendMessage(LTDuressFp,LB_SETCHECKMARK,selindex,(LPARAM)CMFLAG_CHECKED);
				else
					SendMessage(LTDuressFp,LB_SETCHECKMARK,selindex,(LPARAM)CMFLAG_BLANK);
				return 0;
			}

			if((LOWORD(wParam)==SCANCODE_MENU)||(gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_ENTER))	//菜单
			{
				CreateQuickMenu(hWnd);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_F12)
			{
				SendMessage(LTDuressFp,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				processscrollview(LTDuressFp,1,7);
				return 0;
			}
			if(LOWORD(wParam)==SCANCODE_F11)
			{
				SendMessage(LTDuressFp,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				processscrollview(LTDuressFp,0,7);
				return 0;
			}
			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				SendMessage(LTDuressFp,MSG_KEYDOWN,SCANCODE_CURSORBLOCKUP,0);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				SendMessage(LTDuressFp,MSG_KEYDOWN,SCANCODE_CURSORBLOCKDOWN,0);
				return 0;
			}

			break;

		case MSG_COMMAND:
			switch(wParam)
			{
				case 200:
					EnrollDuressFp(hWnd);
					break;
				case 201:
					for(i=0;i<listcount;i++)
					{
						SendMessage(LTDuressFp,LB_SETCHECKMARK,i,(LPARAM)CMFLAG_BLANK);
					}
					break;
				case 202:
					SaveDuressFp();
					PostMessage(hWnd,MSG_CLOSE,0,0);
					break;
			}
			break;

		case MSG_CLOSE:
			if(ismenutimeout)
				DelFingerFromCache(fhdl);
			if (isfpdbload)
				FPDBInit();

			//UnloadBitmap(&mdurbmpbg);
			UnloadBitmap(&mdurbmps1);
			UnloadBitmap(&mdurbmps2);
			UnloadBitmap(&mdurbmps3);
			UnloadBitmap(&mdurbmps4);
			UnloadBitmap(&micon1);
			UnloadBitmap(&micon2);
			UnloadBitmap(&micon3);
			if (gfont1==NULL)
			{
				DestroyLogFont(hintfont);
				DestroyLogFont(captionfont);
			}
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int DuressFpWindow(HWND hWnd,int usrPIN)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	if(!fhdl)
		return 0;

	usrid=usrPIN;
	//	printf("usrid:%d\n",usrid);
	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	//	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = DuressFpwinproc;
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
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

