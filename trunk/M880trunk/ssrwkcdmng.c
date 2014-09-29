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
#include <minigui/tftmullan.h>

#define LB_STATIC1 50
#define LB_STATIC2 51
#define LB_STATIC3 52
#define LB_STATIC4 53
#define LB_STATIC5 54
#define LB_STATIC6 55
#define LB_STATIC7 56
#define LB_STATIC8 57
#define LB_STATIC9 58
#define LB_STATIC10 59
#define LB_STATIC11 60
//#define LB_STATIC12 61
#define LB_STATIC13 84

#define ID_CODE 62
#define CB_KEY	63
#define ID_NAME 64
#define ID_H1 65
#define ID_H2 66
#define ID_H3 67
#define ID_H4 68
#define ID_H5 69
#define ID_H6 70
#define ID_H7 71
#define ID_M1 72
#define ID_M2 73
#define ID_M3 74
#define ID_M4 75
#define ID_M5 76
#define ID_M6 77
#define ID_M7 78
#define LB_CODE 82

#define CB_CHANGE 83
//#define CB_DOOR 79

#define ID_SAVE 80
#define ID_EXIT 81

HWND EdCode,EdName;
HWND BtSave,BtExit;
HWND Edtmp,focuswnd;

//BITMAP smsmngbk;
int OptMode=0;
char tmpstr[50];
TWORKCODE tmpworkcode;
static int tmpPin;
static int bChanged = 0;
extern HWND hIMEWnd;

static void  SelectNext(WPARAM wParam)
{
	if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == EdCode || Edtmp == EdName)
		{
			if(hIMEWnd != HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd = HWND_INVALID;
			}
			if(Edtmp==EdCode)
				focuswnd = EdName;
			else
				focuswnd = BtSave;
		}
		else if (Edtmp==BtSave)
			focuswnd = BtExit;
		else if(Edtmp==BtExit)
		{
			if(OptMode)	//add workcode.
				focuswnd=EdCode;
			else		//edit workcode.
				focuswnd=EdName;
		}
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == EdCode)
			focuswnd = BtExit;
		else if (Edtmp == BtExit)
			focuswnd = BtSave;
		else if (Edtmp == BtSave)
			focuswnd = EdName;
		else if(Edtmp == EdName || Edtmp == EdCode)
		{
			if(hIMEWnd != HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd = HWND_INVALID;
			}
			if(Edtmp == EdName)
				focuswnd = (OptMode)?EdCode:BtExit;
			else
				focuswnd = BtExit;
		}
	}

	SetFocusChild(focuswnd);
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int InfoRepeat(HWND hWnd)
{
	TWORKCODE twc;
	char tstr[50];

	if(OptMode)
	{
		memset(tstr,0,MAX_WORKCODE_LEN+1);
		GetWindowText(EdCode,tstr,MAX_WORKCODE_LEN);
		if(strlen(tstr)==0)
		{
			MessageBox1(hWnd,LoadStrByID(MID_WKCDEMPTYHINT),LoadStrByID(MID_APPNAME),MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
			if(!ismenutimeout) SetFocusChild(EdCode);
			return 1;
		}
	}

	memset(&twc,0,sizeof(TWORKCODE));
	if(OptMode && (FDB_GetWorkCode(tmpPin,&twc)!=NULL)) return 1;

	if(FDB_InitWkcdPoint())
	{
		while(FDB_ReadWorkCode(&twc))
		{
			if(twc.PIN && (twc.PIN!=tmpPin))
			{
				//code:
				if(OptMode)
				{
					memset(tstr,0,MAX_WORKCODE_LEN+1);
					GetWindowText(EdCode,tstr,MAX_WORKCODE_LEN);
					if(strncmp(tstr,twc.Code,MAX_WORKCODE_LEN)==0)
					{
						MessageBox1(hWnd,LoadStrByID(MID_WKCDCODEREPEAT), LoadStrByID(MID_APPNAME),
							MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
						if(!ismenutimeout) SetFocusChild(EdCode);
						return 1;
					}
				}

				//name:
				memset(tstr,0,MAX_WORKCODE_NAME_LEN+1);
				GetWindowText(EdName,tstr,MAX_WORKCODE_NAME_LEN);
				if((strlen(twc.Name)>0) && (strncmp(tstr,twc.Name,MAX_WORKCODE_NAME_LEN)==0))
				{
					MessageBox1(hWnd,LoadStrByID(MID_WKCDNAMEREPEAT), LoadStrByID(MID_APPNAME),
						MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
					if(!ismenutimeout) SetFocusChild(EdName);
					return 1;
				}
			}
		}
	}
	return 0;
}

static void InitWindow (HWND hWnd)
{
	//int i;
	char* wkcdstr[4];
	int posX1,posX2,posX3;

	wkcdstr[0]=LoadStrByID(MID_WKCDCODE);
	wkcdstr[1]=LoadStrByID(MID_WKCDNAME);
	wkcdstr[2]=LoadStrByID(MID_SAVE);
	wkcdstr[3]=LoadStrByID(MID_EXIT);

	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=250+gOptions.ControlOffset;
		posX2=20+gOptions.ControlOffset;
		posX3=190+gOptions.ControlOffset;
	}
	else
	{
		posX1=10+gOptions.ControlOffset;
		posX2=40+gOptions.ControlOffset;
		posX3=40+gOptions.ControlOffset;
	}
	CreateWindow(CTRL_STATIC, wkcdstr[0], WS_VISIBLE | SS_LEFT, LB_STATIC1,posX1,10,90,23,hWnd,0);

	if(OptMode)//add workcode.
	{
		EdCode=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,ID_CODE,posX3,35,90,23,hWnd,0);
		SendMessage(EdCode, EM_LIMITTEXT, MAX_WORKCODE_LEN , 0L);
	}
	else	//edit workcode.
	{
		memset(tmpstr, 0, MAX_WORKCODE_LEN+1);
		nmemcpy((unsigned char*)tmpstr, (unsigned char*)tmpworkcode.Code, MAX_WORKCODE_LEN);
		CreateWindow(CTRL_STATIC, tmpstr, WS_VISIBLE | SS_LEFT, LB_CODE, posX3, 35, 90, 23, hWnd, 0);
	}

	CreateWindow(CTRL_STATIC,wkcdstr[1], WS_VISIBLE | SS_LEFT, LB_STATIC3,posX1,70,90,23,hWnd,0);
	EdName=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,ID_NAME,posX2,95,260,23,hWnd,0);
	SendMessage(EdName,EM_LIMITTEXT,MAX_WORKCODE_NAME_LEN,0L);

	BtSave=CreateWindow(CTRL_BUTTON,wkcdstr[2],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_SAVE,10,190,85,23,hWnd,0);
	BtExit=CreateWindow(CTRL_BUTTON,wkcdstr[3],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);
}

static void FillWorkCodeData()
{
	if(OptMode)
	{
		SetWindowText(EdCode,"");
		SetWindowText(EdName,"");
	}
	else
	{
		memset(tmpstr,0,MAX_WORKCODE_NAME_LEN+1);
		nmemcpy((unsigned char*)tmpstr,(unsigned char*)tmpworkcode.Name,MAX_WORKCODE_NAME_LEN);
		SetWindowText(EdName,tmpstr);
	}
}

static int ismodified(void)
{
	if(OptMode)	//add new workcode
	{
		memset(tmpstr,0,sizeof(tmpstr));
		GetWindowText(EdCode, tmpstr, MAX_WORKCODE_LEN);
		if(tmpstr && atoi(tmpstr)>0)
			return 1;
	}

	//name
	memset(tmpstr,0,sizeof(tmpstr));
	GetWindowText(EdName,tmpstr,24);
	if(OptMode)
	{
		if(tmpstr && strlen(tmpstr)>0) return 1;
	}
	else
	{
		if(strncmp(tmpstr,tmpworkcode.Name,MAX_WORKCODE_NAME_LEN)!=0) return 1;
	}
	return 0;

}

static int SaveWorkCodeInfo(void)
{
	char tmpcode[MAX_WORKCODE_LEN+1];
	char tmpname[MAX_WORKCODE_NAME_LEN+1];
	TWORKCODE mywkcd;

	tmpworkcode.PIN = tmpPin;

	memset(tmpcode,0,MAX_WORKCODE_LEN+1);
	if(OptMode)
		GetWindowText(EdCode,tmpcode,MAX_WORKCODE_LEN);
	else
		nstrcpy(tmpcode,tmpworkcode.Code,MAX_WORKCODE_LEN);

	memset(tmpname,0,MAX_WORKCODE_NAME_LEN+1);
	GetWindowText(EdName,tmpname,MAX_WORKCODE_NAME_LEN);

	if(FDB_CreateWorkCode(&mywkcd,tmpPin,tmpcode,tmpname)!=NULL)
	{
		if(OptMode)	//add new workcode
		{
			if(FDB_AddWorkCode(&mywkcd)==FDB_OK);
			{
				sync();
//				printf("Save Workcode scussess!\n");
				return 1;
			}
		}
		else		//edit workcode
		{
			if(FDB_ChgWorkCode(&mywkcd)==FDB_OK)
			{
				sync();
//				printf("Modify workcode scussess!\n");
				return 1;
			}
		}
	}
	return 0;
}

static int wcodewinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	//int tmpvalue;
	//int tmphour=0;
	//int tmpmin=0;
	//int tmpselkey=0;
	char tmpchar2[MAX_WORKCODE_NAME_LEN+1];

	//int tmpcode,codelen;
	//int i;
        char tmpss[12];
	//int tvalue;
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_CREATE:
			InitWindow(hWnd);		//add controls
			FillWorkCodeData();
			UpdateWindow(hWnd,TRUE);
			if(OptMode)
				SetFocusChild(EdCode);
			else
				SetFocusChild(EdName);
			break;

                case MSG_PAINT:
                        hdc=BeginPaint(hWnd);
                        EndPaint(hWnd,hdc);
                        return 0;

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
			if(fGetDC) ReleaseDC (hdc);
			return 0;
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
			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			Edtmp = GetFocusChild(hWnd);
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
				PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0L);

//			if (LOWORD(wParam) == gOptions.IMESwitchKey)
	                if(LOWORD(wParam)==SCANCODE_F9 ||
				((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && (LOWORD(wParam)==SCANCODE_F11)))
			{
				if(Edtmp==EdName)
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
				return 0;
			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				SelectNext(wParam);
				return 0;
			}

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
				|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) && gOptions.TFTKeyLayout==3&&Edtmp==EdName)
				{
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
					return 0;
				}
				if(Edtmp==BtSave)
					SetFocusChild(BtExit);
				else if(Edtmp==BtExit)
					SetFocusChild(BtSave);
			}

			if(LOWORD(wParam)==SCANCODE_ENTER)
			{
				char tmpcontent[50];
				if(Edtmp==EdCode)
				{
					memset(tmpcontent,0,50);
					GetWindowText(EdCode,tmpcontent, MAX_WORKCODE_LEN);
					if(tmpcontent && atoi(tmpcontent)>0)
						SetFocusChild(EdName);
				}
				else if(Edtmp!=BtExit && Edtmp!=BtSave)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0L);
				break;
			}

			if(LOWORD(wParam)==SCANCODE_F10)
			{
                                char tmpcontent[50];
                                if(Edtmp==EdCode)
                                {
                                        memset(tmpcontent,0,50);
                                        GetWindowText(EdCode,tmpcontent, MAX_WORKCODE_LEN);
                                        if(tmpcontent && atoi(tmpcontent)>0)
                                                SetFocusChild(EdName);
                                }
                                else if(Edtmp!=BtExit)
                                        PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
				else
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_EXIT, 0L);
                                break;
			}

			if(LOWORD(wParam)==SCANCODE_MENU&& gOptions.TFTKeyLayout !=3 )
				PostMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);

			break;

		case MSG_CHAR:
			Edtmp = GetFocusChild(hWnd);
			if(Edtmp==EdCode)
			{
				if(wParam > 0xa0) return 0;
			}
			else if(Edtmp==EdName)
			{
				memset(tmpchar2,0,MAX_WORKCODE_NAME_LEN+1);
				GetWindowText(EdName,tmpchar2,MAX_WORKCODE_NAME_LEN);
				if(strlen(tmpchar2)==MAX_WORKCODE_NAME_LEN-1 && wParam>0xa0)
					return 0;
			}
			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(nc==EN_CHANGE && id==ID_CODE)
			{
				memset(tmpss,0,MAX_WORKCODE_LEN+1);
				GetWindowText(EdCode,tmpss,MAX_WORKCODE_LEN);
				if(strlen(tmpss)>0)
                                {
                                        if(tmpss[0]=='0')
                                        {
                                                SetWindowText(EdCode,"");
                                                SetFocusChild(EdCode);
                                        }
                                }
			}

			if(id==ID_SAVE)
			{
				//if(ismodified() && !InfoRepeat(hWnd))
				if(ismodified())
				{
					if(!InfoRepeat(hWnd))
					{
						if(SaveWorkCodeInfo())
						{
							bChanged = 1;
							PostMessage(hWnd,MSG_CLOSE,0,0L);
						}
					}
				}
				else
				{
					if(!ismenutimeout)
					{
						bChanged=0;
						SendMessage(hWnd, MSG_CLOSE,0,0L);
					}
				}

			}
			else if(id == ID_EXIT)
			{
				if(ismodified() && MessageBox1 (hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
                                  		MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
					PostMessage(hWnd,MSG_COMMAND,ID_SAVE,0L);
				else
				{
					if(!ismenutimeout)
					{
						bChanged=0;
						SendMessage(hWnd, MSG_CLOSE,0,0L);
					}
				}
			}
			break;

		case MSG_CLOSE:
			if(hIMEWnd!=HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd = HWND_INVALID;
			}
			//UnloadBitmap(&smsmngbk);
			//MainWindowCleanup (hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

/**/
static U16 GetFreeWkcdPIN(U16 pin)
{
	//TWORKCODE testwkcd;
	//static U16 testpin;
	U16 testpin = pin;		//lyy

	while(FDB_GetWorkCode(testpin, NULL)!=NULL)
	{
		testpin++;
	}
	return testpin;
}

//optmod(0:edit,1:add)
int CreateWCodeManageWindow(HWND hWnd, int optmod, int *wkcdPIN)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	OptMode = optmod;
	memset(&tmpworkcode,0,sizeof(TWORKCODE));
	if(OptMode)	//add new workcode.
	{
		tmpPin = GetFreeWkcdPIN(1);
	}
	else		//edit workcode.
	{
		tmpPin = *wkcdPIN;
		FDB_GetWorkCode(tmpPin,&tmpworkcode);
	}

	bChanged = 0;

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;

	if(OptMode)
		CreateInfo.spCaption=LoadStrByID(MID_WKCDADD);
	else
		CreateInfo.spCaption=LoadStrByID(MID_WKCDEDIT);

	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = wcodewinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
        CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//if (LoadBitmap(HDC_SCREEN,&smsmngbk,GetBmpPath("submenubg.jpg")))
	//	return 0;
	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hMainWnd);
	*wkcdPIN = tmpPin;

	return bChanged;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

