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

#define WIFI_PWDSTATIC	1080
#define WIFI_PWDID	1090
#define WIFI_PWDTYPE	1091
#define WIFI_PWD	1092
#define WIFI_WPAPSK	1096

#define WIFI_PWDSAVE	1100
#define WIFI_PWDEXIT	1101

//BITMAP wifipwdbkg;
WifiPWD pwdbak[4];		//原始密码设置
char pwdidbak[2];		//原始密码编号
char wpapskbak[255];		//原始WPAPSK密码

HWND WifiPWDWnd[8];
int ControlCnt=0;
int curCtlidx=0;

int pwdmode;
extern char newpwdid[2];
extern WifiPWD newpwd[4];
extern char newwpapsk[255];
 
int tmppwdmode[4];	//临时保存各密码类型
int curpwdidx=0;	//当前密码序号 
int lenValue[4]={10,26,5,13};
int tmp26flag[4]={0,0,0,0};//Liaozz 20080928 fix bug second 43
static int InitPasswordMode(int len)
{
	int i;
	for(i=0;i<4;i++)
	{
		if(len==lenValue[i])
			return i;
	}
	return 0;
}

static int getPasswordMode(int idx)
{
        if(idx==0 || idx==1) return 0;
        else if(idx==2 || idx==3) return 1;
	return -1;
}

static int beModified(void)
{
	char mdchar[255];
	if(pwdmode==0)
	{
		if(SendMessage(WifiPWDWnd[0], CB_GETCURSEL, 0, 0)!=(atoi(newpwdid)-1))
			return 1;
		if(SendMessage(WifiPWDWnd[1], CB_GETCURSEL, 0, 0)!=tmppwdmode[curpwdidx])
			return 1;
		
		memset(mdchar, 0, sizeof(mdchar));
		GetWindowText(WifiPWDWnd[2+curpwdidx], mdchar, 255);
		if(strncmp((char*)newpwd[curpwdidx].pwdchar, mdchar, 30)!=0)
			return 1;
	}
	else
	{
		memset(mdchar, 0, sizeof(mdchar));
		GetWindowText(WifiPWDWnd[0], mdchar, 255);
		if(strncmp(newwpapsk, mdchar, 255)!=0)
			return 1;
	} 	
		
	return 0;
}

static void RefreshPWDWindow(int pwdidx)
{
	int i;
	//char pwdchar[255];

	for(i=0;i<4;i++)
	{
		if(i!=pwdidx)
			SendMessage(WifiPWDWnd[2+i], MSG_ENABLE, 0, 0);
		else
			SendMessage(WifiPWDWnd[2+i], MSG_ENABLE, 1, 0);
	}
#if 1	
	SendMessage(WifiPWDWnd[1], CB_SETCURSEL, tmppwdmode[pwdidx], 0);
	SendMessage(WifiPWDWnd[2+pwdidx], EM_LIMITTEXT, lenValue[tmppwdmode[pwdidx]], 0L);
#endif
}

static void InitPWDSetting(void)
{
	int i;
	int pwdlen;

	if(pwdmode==0)
	{
		if(newpwdid && strlen(newpwdid)>0)
			curpwdidx=atoi(newpwdid)-1;
		else
			curpwdidx=0;
		SendMessage(WifiPWDWnd[0], CB_SETCURSEL, curpwdidx, 0);
	
		for(i=0;i<4;i++)
		{
			pwdlen = strlen((char*)newpwd[i].pwdchar);
			if(pwdlen>0)
			{
				tmppwdmode[i]=InitPasswordMode(pwdlen);
				SetWindowText(WifiPWDWnd[2+i], (char*)newpwd[i].pwdchar);
			}
			else
			{
				tmppwdmode[i]=0;
				SetWindowText(WifiPWDWnd[2+i], "");
			}
		}
		RefreshPWDWindow(curpwdidx);
	}
	else
	{
		if(newwpapsk && strlen(newwpapsk)>0)
			SetWindowText(WifiPWDWnd[0], newwpapsk);
	}
}
 
static void SelectNext(WPARAM wParam)
{
	//int i;
	if(wParam==SCANCODE_CURSORBLOCKUP)
	{
		if(--curCtlidx<0)
			curCtlidx=ControlCnt-1;
	}
	else if(wParam==SCANCODE_CURSORBLOCKDOWN)
	{
		if(++curCtlidx>ControlCnt-1) curCtlidx=0;
	}	
	if(pwdmode==0)
	{
		while((curCtlidx>=2 && curCtlidx<=5) && ((curCtlidx-2)!=curpwdidx))
		{
			if(wParam==SCANCODE_CURSORBLOCKUP)
				curCtlidx--;
			else if(wParam==SCANCODE_CURSORBLOCKDOWN)
				curCtlidx++;
		}
	}	
	SetFocusChild(WifiPWDWnd[curCtlidx]);
}

static void InitWindow (HWND hWnd)
{
	char* WifiPWDStr[10];
	int posX1,posX2,posX3,posX4;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=235+gOptions.GridWidth*2;
		posX2=5+gOptions.GridWidth*2;
		posX3=200+gOptions.GridWidth*2;
		posX4=160+gOptions.GridWidth*2;
	}
	else
	{
		posX1=10;
		posX2=81;
		posX3=20;
		posX4=81;
	}

        WifiPWDStr[0]=LoadStrByID(MID_WIFI_PWDIDX);     //密码序号
        WifiPWDStr[1]=LoadStrByID(MID_WIFI_PWDTYPE);    //密码类型
        WifiPWDStr[2]=LoadStrByID(MID_WIFI_PWDFMT1);
        WifiPWDStr[3]=LoadStrByID(MID_WIFI_PWDFMT2);
        WifiPWDStr[4]=LoadStrByID(MID_WIFI_PWDFMT3);
        WifiPWDStr[5]=LoadStrByID(MID_WIFI_PWDFMT4);
        WifiPWDStr[6]=LoadStrByID(MID_PWD);             //密码
        WifiPWDStr[7]=LoadStrByID(MID_SAVE);            //保存
        WifiPWDStr[8]=LoadStrByID(MID_EXIT);            //退出
	WifiPWDStr[9]=LoadStrByID(MID_WIFI_HINT2);

	ControlCnt=0;
	if(pwdmode==0)		//WEP
	{
		int i;
		char pwdstr[48] = {0};
		CreateWindow(CTRL_STATIC,WifiPWDStr[0],WS_VISIBLE|SS_LEFT,WIFI_PWDSTATIC,posX1,10,80,23,hWnd,0);
		WifiPWDWnd[ControlCnt]=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|
							CBS_AUTOFOCUS, WIFI_PWDID,posX4,6,60,23,hWnd,0);
		for(i=0;i<4;i++)
		{
			memset(pwdstr, 0, sizeof(pwdstr));
			sprintf(pwdstr, "%d", i+1);
			SendMessage(WifiPWDWnd[ControlCnt], CB_ADDSTRING, 0, (LPARAM)pwdstr);
		}
		ControlCnt++;

		CreateWindow(CTRL_STATIC,WifiPWDStr[1],WS_VISIBLE|SS_LEFT,WIFI_PWDSTATIC+1,posX1,40,80,23,hWnd,0);
		WifiPWDWnd[ControlCnt]=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|
							CBS_AUTOFOCUS, WIFI_PWDTYPE,posX2,36,230,23,hWnd,0);
		for(i=0;i<4;i++)
			SendMessage(WifiPWDWnd[ControlCnt], CB_ADDSTRING, 0, (LPARAM)WifiPWDStr[2+i]);
		ControlCnt++;
		
		for(i=0;i<4;i++)
		{
			memset(pwdstr, 0, sizeof(pwdstr));
			sprintf(pwdstr, "%s%d", WifiPWDStr[6], i+1);
			CreateWindow(CTRL_STATIC,pwdstr,WS_VISIBLE|SS_LEFT,WIFI_PWDSTATIC+2+i,posX1,70+30*i,80,23,hWnd,0);
			WifiPWDWnd[ControlCnt++]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|ES_AUTOSELECT|ES_BASELINE|WS_BORDER|ES_PASSWORD,
								WIFI_PWD+i,posX2+9,66+30*i,210,23,hWnd,0);
		}
	}
	else			//WPA
	{
		CreateWindow(CTRL_STATIC, WifiPWDStr[6], WS_VISIBLE | SS_LEFT, WIFI_PWDSTATIC, posX1, 10, 80, 23, hWnd, 0);
		WifiPWDWnd[ControlCnt++]=CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | ES_AUTOSELECT | ES_BASELINE | WS_BORDER | ES_PASSWORD,
							WIFI_WPAPSK, 10, 36, 300, 23, hWnd, 0);
		SendMessage(WifiPWDWnd[0], EM_LIMITTEXT, 64, 0L);
		CreateWindow(CTRL_STATIC, WifiPWDStr[9], WS_VISIBLE | SS_LEFT, WIFI_PWDSTATIC+1, 20, 70, 300, 23, hWnd, 0);
	}

	WifiPWDWnd[ControlCnt++]=CreateWindow(CTRL_BUTTON, WifiPWDStr[7], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
						WIFI_PWDSAVE, 10,190,85,23,hWnd,0);
	WifiPWDWnd[ControlCnt++]=CreateWindow(CTRL_BUTTON, WifiPWDStr[8], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
						WIFI_PWDEXIT, 220+gOptions.ControlOffset,190,85,23,hWnd,0);
}

static int ispwdstrerr(char* pwd, int type)
{
	int i;
	int len = strlen(pwd);
	if(type<=1)	//Hex
	{
		for(i=0;i<len;i++)
		{
			if(!((pwd[i]>='0' && pwd[i]<='9') || (pwd[i]>='a' && pwd[i]<='f') || (pwd[i]>='A' && pwd[i]<='F')))
				return 1;
		}
	}
	return 0;
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int SaveWifiPWDSetting(HWND hWnd)
{
        int i;
        char schar[255];
	int noerr=1;
	int slen;

        if(pwdmode==0)
        {
		curpwdidx=SendMessage(WifiPWDWnd[0], CB_GETCURSEL, 0, 0);			//password index
		tmppwdmode[curpwdidx]=SendMessage(WifiPWDWnd[1], CB_GETCURSEL, 0, 0);		//password type
		GetWindowText(WifiPWDWnd[2+curpwdidx], schar, 255);				//password string
		if(ispwdstrerr(schar, tmppwdmode[curpwdidx]))		//字符类型不正确
		{
			MessageBox1(hWnd, LoadStrByID(MID_WIFI_ERR3), LoadStrByID(HIT_RUN), MB_OK | MB_ICONSTOP);
	                if(!ismenutimeout) SetFocusChild(WifiPWDWnd[2+curpwdidx]);
			return 0;
		}

		slen=strlen(schar);
		if(slen==lenValue[tmppwdmode[curpwdidx]])
		{
			memset(newpwdid, 0, 2);
			sprintf(newpwdid, "%d", curpwdidx+1);
			
			for(i=0;i<4;i++)	//清除原密码(08.01.10)
			{
				memset(&(newpwd[i]), 0, sizeof(WifiPWD));
			}
//			memset(newpwd[curpwdidx].pwdchar, 0, 30);
			memcpy(newpwd[curpwdidx].pwdchar, schar, slen);

			sprintf((char*)newpwd[curpwdidx].pwdtype, "%d", getPasswordMode(tmppwdmode[curpwdidx]));
		}
		else
			noerr=0;
        }
        else
        {
                GetWindowText(WifiPWDWnd[0], schar, 255);
		int slen = strlen(schar);
		if(slen>=8 && slen<=64)
		{
			memset(newwpapsk, 0, 255);
			memcpy(newwpapsk, schar, slen);
		}
		else
			noerr=0;
        }

	if(!noerr)
	{
		MessageBox1(hWnd, LoadStrByID(MID_WIFI_ERR2), LoadStrByID(HIT_RUN), MB_OK | MB_ICONSTOP);
		if(!ismenutimeout)
		{
			if(pwdmode==0)
				SetFocusChild(WifiPWDWnd[2+curpwdidx]);
			else
				SetFocusChild(WifiPWDWnd[0]);
		}
	}
	else
	{
		MessageBox1(hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
	}

	return (noerr);
}

extern HWND hIMEWnd;
static int WiFiPWDWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int nc, id;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitWindow(hWnd);		//add controls
			InitPWDSetting();
			UpdateWindow(hWnd,TRUE);
			curCtlidx=0;
			SetFocusChild(WifiPWDWnd[curCtlidx]);
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

			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());
			if(fGetDC) ReleaseDC (hdc);
			return 0;
		}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);	
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

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				PostMessage(hWnd, MSG_COMMAND, WIFI_PWDEXIT, 0);
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
                                if(hIMEWnd!=HWND_INVALID)
                                {
                                        SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
                                        hIMEWnd=HWND_INVALID;
                                }

				SelectNext(wParam);
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
				|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if(pwdmode==0)
				{
					if(curCtlidx==0 || curCtlidx==1)
					{
						int curidx = SendMessage(WifiPWDWnd[curCtlidx], CB_GETCURSEL, 0, 0);
						if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT  || ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
						{
							if(--curidx<0) curidx=3;
						}
						else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
						{
							if(++curidx>3) curidx=0;
						}
						SendMessage(WifiPWDWnd[curCtlidx], CB_SETCURSEL, curidx, 0);
						
						if(curCtlidx==0)
						{
							curpwdidx=curidx;
							RefreshPWDWindow(curpwdidx);
						}
						else
						{
							tmppwdmode[curpwdidx]=curidx;
//							SetWindowText(WifiPWDWnd[2+curpwdidx],"");
							SendMessage(WifiPWDWnd[2+curpwdidx], EM_LIMITTEXT, lenValue[curidx], 0L);
						}
						return 0;
					}
					 else if((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) &&(curCtlidx>=2 && curCtlidx<=5)&&
					    gOptions.IMEFunOn==1&&gOptions.TFTKeyLayout==3)
					{
                        T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight, 0);	//不支持中文输入
						return 0;
					}
					else if(curCtlidx>=6 && curCtlidx<=7)
					{
						if(curCtlidx==6) curCtlidx=7;
						else curCtlidx=6;
						SetFocusChild(WifiPWDWnd[curCtlidx]);
						return 0;
					}
				}
				else
				{
					if(curCtlidx>=1 && curCtlidx<=2)
					{
						if(curCtlidx==1) curCtlidx=2;
						else curCtlidx=1;
						SetFocusChild(WifiPWDWnd[curCtlidx]);
						return 0;
					}
					else if((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) && curCtlidx==0 &&
					    gOptions.IMEFunOn==1 && gOptions.TFTKeyLayout==3)
					{
                        T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight, 0);	//不支持中文输入
					}
				}
			}
			else if(LOWORD(wParam)==SCANCODE_ENTER) 
			{
				if((pwdmode==0 && curCtlidx<6) || (pwdmode==1 && curCtlidx<1))
					PostMessage(hWnd, MSG_COMMAND, WIFI_PWDSAVE, 0L);
			}
			else if(LOWORD(wParam)==SCANCODE_F10)
			{
				if((pwdmode==0 && curCtlidx==7)||(pwdmode==1 && curCtlidx==2))
					PostMessage(hWnd, MSG_COMMAND, WIFI_PWDEXIT, 0L);
				else
					PostMessage(hWnd, MSG_COMMAND, WIFI_PWDSAVE, 0L);
			}
			else if(LOWORD(wParam)==SCANCODE_MENU)
			{
				PostMessage(hWnd, MSG_COMMAND, WIFI_PWDSAVE, 0);
			}
//			else if (LOWORD(wParam) == gOptions.IMESwitchKey)
			else if((LOWORD(wParam)==SCANCODE_F9) || 
				(LOWORD(wParam)==SCANCODE_F11 && (gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)))
                        {
                                if(((pwdmode==0 && curCtlidx>=2 && curCtlidx<=5) || (pwdmode==1 && curCtlidx==0)) && gOptions.IMEFunOn==1)
                                        T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight, 0);	//不支持中文输入
                                break;
                        }
			break;
//Liaozz 20080928 fix bug second 43
		case MSG_CHAR:
		{
			char schar[255];
			int slen;
//			printf("*************char=======l:%d,,,w:%d===\n",lParam, wParam);
			HWND Edtmp = GetFocusChild(hWnd);
			if(pwdmode==0)
			{
				curpwdidx=SendMessage(WifiPWDWnd[0], CB_GETCURSEL, 0, 0);			//password index
				tmppwdmode[curpwdidx]=SendMessage(WifiPWDWnd[1], CB_GETCURSEL, 0, 0);		//password type
				GetWindowText(WifiPWDWnd[2+curpwdidx], schar, 255);				//password string
				slen=strlen(schar);
				//13:Enter; 27:ESC; 127:Backspace
				if(slen==lenValue[tmppwdmode[curpwdidx]] && Edtmp == WifiPWDWnd[2+curpwdidx]
				         && wParam != 127 && wParam != 27 && wParam != 13) {
						ExBeep(1);
					}
			} else {//Liaozz 20081009 fix bug second 43
				 GetWindowText(WifiPWDWnd[0], schar, 255);
					int slen = strlen(schar);
					if (slen == 64 && Edtmp == WifiPWDWnd[0]
					               && wParam != 127 && wParam != 27 && wParam != 13)
						ExBeep(1);
			}
			break;
		}
		//Liaozz end
		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(id==WIFI_PWDSAVE)
			{
				if(beModified())
				{
					if(SaveWifiPWDSetting(hWnd) && !ismenutimeout)
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				else
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
					
			}
			else if(id==WIFI_PWDEXIT)
			{
				if(beModified() && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA),LoadStrByID(MID_APPNAME),
									MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK)
					PostMessage(hWnd, MSG_COMMAND, WIFI_PWDSAVE, 0);
				else
				{
					if(!ismenutimeout) PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&wifipwdbkg);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateWiFiPWDWindow(HWND hWnd, int ptype)
{
	char wincap[20];

	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	pwdmode=ptype;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER|WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	if(pwdmode)		//WPA
		sprintf(wincap, "%s%s", LoadStrByID(MID_WIFI_PWDWPA), LoadStrByID(MID_PWD));
	else			//WEP
		sprintf(wincap, "%s%s", LoadStrByID(MID_WIFI_PWDWEP), LoadStrByID(MID_PWD));
	CreateInfo.spCaption = wincap;

	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WiFiPWDWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//if (LoadBitmap(HDC_SCREEN,&wifipwdbkg,GetBmpPath("submenubg.jpg")))
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

	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

