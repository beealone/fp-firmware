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


#define LB_STATIC 	100
#define LB_TIMEZONE	108
#define ID_TZID		109
#define ID_TZVALUE	110

#define LB_TS		140		
#define LB_TO		141		

#define ID_SAVE 	142 
#define ID_EXIT 	143

HWND edTZID;
HWND tmpWnd[7][2][2];
HWND btSave,btExit;
//BITMAP tzbkbmp;
TTimeZone myttz;
int ttzid;
char* timezonestr[12];
U8 tmptimevalue[7][2][2];
int actvieindex;
char tmpc[20];
int optflag;		//operation modal
//int filldataflag=0;

HWND tmpfocusWnd,focuswnd;
int act_x,act_y,act_z;
static int GetTimeZoneID(void);

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int  SelectNext(HWND hWnd,WPARAM wParam)	
{
	if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if(tmpfocusWnd==edTZID)
		{
			act_x=0; act_y=0; act_z=0;
			focuswnd=tmpWnd[act_x][act_y][act_z];
		}
		else if(tmpfocusWnd==btSave)
			focuswnd=btExit;
		else if(tmpfocusWnd==btExit)
		{
			if(optflag)		//point to some id
			{
				act_x=0; act_y=0; act_z=0;
				focuswnd=tmpWnd[act_x][act_y][act_z];
			}
			else
				focuswnd=edTZID;
		}
		else
		{

			if(++act_x>6)
				focuswnd=btSave;
			else
				focuswnd=tmpWnd[act_x][act_y][act_z];
		}

	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if(tmpfocusWnd==edTZID)
			focuswnd=btExit;
		else if(tmpfocusWnd==btExit)
			focuswnd=btSave;
		else if(tmpfocusWnd==btSave)
		{
			act_x=6; act_y=1; act_z=1;
			focuswnd=tmpWnd[act_x][act_y][act_z];
		}
		else
		{
			if(--act_x<0)
			{
				focuswnd=(optflag)?btExit:edTZID;
			}
			else
				focuswnd=tmpWnd[act_x][act_y][act_z];
		}
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
	{
		if(tmpfocusWnd==edTZID)
			SendMessage(tmpfocusWnd,MSG_KEYDOWN,wParam,0L);
		else if(tmpfocusWnd==btExit)
			focuswnd=btSave;
		else if(tmpfocusWnd==btSave)
		{
			act_x=6; act_y=1; act_z=1;
			focuswnd=tmpWnd[act_x][act_y][act_z];
		}
		else
		{
			if(act_x==0 && act_y==0 && act_z==0)
				focuswnd=(optflag)?btExit:edTZID;
			else
			{
				if(act_y==0)
				{
					if(act_z==0)
					{
						--act_x; act_y=1; act_z=1;
					}
					else
						act_z=0;
				}
				else
				{
					if(act_z==0)
					{
						act_y=0; act_z=1;
					}
					else
						act_z=0;
				}
				focuswnd=tmpWnd[act_x][act_y][act_z];
			}
		}
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
	{
		if(tmpfocusWnd==edTZID)
			SendMessage(tmpfocusWnd,MSG_KEYDOWN,wParam,0L);
		else if(tmpfocusWnd==btSave)
			focuswnd=btExit;
		else if(tmpfocusWnd==btExit)
		{
			if(optflag)             //point to some id
			{
				act_x=0; act_y=0; act_z=0;
				focuswnd=tmpWnd[act_x][act_y][act_z];
			}
			else
				focuswnd=edTZID;
		}
		else
		{
			if(act_x==6 && act_y==1 && act_z==1)
				focuswnd=btSave;
			else
			{
				if(act_y==0)
				{
					if(act_z==0)
						act_z=1;
					else
					{
						act_y=1; act_z=0;
					}
				}
				else
				{
					if(act_z==0)
						act_z=1;
					else
					{
						++act_x; act_y=0; act_z=0;
					}
				}
				focuswnd=tmpWnd[act_x][act_y][act_z];
			}
		}
	}

	SetFocusChild(focuswnd);
	return 1;
}

static void InitWindow (HWND hWnd)
{
	int i;
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9,posX10;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=180;
		posX2=140;
		posX3=130;
		posX4=100;
		posX5=80;
		posX6=50;
		posX7=40;
		posX8=10;
		posX9=90;
		posX10=20;
	}
	else
	{
		posX1=10;
		posX2=60+gOptions.ControlOffset;
		posX3=89+gOptions.ControlOffset;
		posX4=96+gOptions.ControlOffset;
		posX5=124+gOptions.ControlOffset;
		posX6=149+gOptions.ControlOffset;
		posX7=178+gOptions.ControlOffset;
		posX8=185+gOptions.ControlOffset;
		posX9=115+gOptions.ControlOffset;
		posX10=0;
	}
	CreateWindow(CTRL_STATIC,LoadStrByID(MID_USER_TZ), WS_VISIBLE | SS_LEFT, LB_STATIC,posX1,10,60,23,hWnd,0);
	edTZID = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
			ID_TZID,posX2-posX10,6,45,23,hWnd,0);
	SendMessage(edTZID,EM_LIMITTEXT,2,0L);

	memset(tmpc,0,20);
	sprintf(tmpc,"(1-%d)",TZ_MAX);
	CreateWindow(CTRL_STATIC,tmpc, WS_VISIBLE | SS_LEFT, LB_STATIC,posX9-posX10,10,60,23,hWnd,0);

	for(i=0;i<7;i++)
	{
		CreateWindow(CTRL_STATIC,timezonestr[i+1], WS_VISIBLE | SS_LEFT, LB_STATIC+i+1,posX1,38+26*i,60,23,hWnd,0);

		tmpWnd[i][0][0] = CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
				ID_TZVALUE+4*i+0,posX2,38+26*i-4,28,23,hWnd,0);
		SendMessage(tmpWnd[i][0][0],EM_LIMITTEXT,2,0L);

		CreateWindow(CTRL_STATIC,timezonestr[8], WS_VISIBLE | SS_CENTER, LB_TS,posX3,38+26*i,10,23,hWnd,0);
		tmpWnd[i][0][1] = CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
				ID_TZVALUE+4*i+1,posX4,38+26*i-4,28,23,hWnd,0);
		SendMessage(tmpWnd[i][0][1],EM_LIMITTEXT,2,0L);

		CreateWindow(CTRL_STATIC,timezonestr[9], WS_VISIBLE | SS_CENTER, LB_TO,posX5,38+26*i,24,23,hWnd,0);

		tmpWnd[i][1][0] = CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
				ID_TZVALUE+4*i+2,posX6,38+26*i-4,28,23,hWnd,0);
		SendMessage(tmpWnd[i][1][0],EM_LIMITTEXT,2,0L);

		CreateWindow(CTRL_STATIC,timezonestr[8], WS_VISIBLE | SS_CENTER, LB_TS,posX7,38+26*i,10,23,hWnd,0);
		tmpWnd[i][1][1] = CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
				ID_TZVALUE+4*i+3,posX8,38+26*i-4,28,23,hWnd,0);
		SendMessage(tmpWnd[i][1][1],EM_LIMITTEXT,2,0L);
	}

	btSave = CreateWindow(CTRL_BUTTON, timezonestr[10], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, ID_SAVE,220+gOptions.ControlOffset,160,90,23,hWnd,0);
	btExit = CreateWindow(CTRL_BUTTON, timezonestr[11], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, ID_EXIT,220+gOptions.ControlOffset,190,90,23,hWnd,0);
}

static int FillData(HWND hWnd, int flag)
{
	char tmpstr[20];
	int i,j,k;

	//	filldataflag=1;
	if(!flag)
	{
		memset(tmpstr,0,5);
		sprintf(tmpstr,"%d",ttzid);
		SetWindowText(edTZID,tmpstr);
	}

	memset(&myttz,0,sizeof(TTimeZone));
	if(FDB_GetTimeZone(ttzid,&myttz)!=NULL)
	{
		for(i=0;i<7;i++)
		{
			for(j=0;j<2;j++)
			{
				for(k=0;k<2;k++)
				{
					memset(tmpstr,0,4);
					sprintf(tmpstr,"%02d",myttz.ITime[i][j][k]);
					tmptimevalue[i][j][k]=myttz.ITime[i][j][k];
					SetWindowText(tmpWnd[i][j][k],tmpstr);
				}
			}
		}
	}
	else
	{
		for(i=0;i<7;i++)
		{
			memset(tmpstr,0,4);
			sprintf(tmpstr,"%02d",0);
			SetWindowText(tmpWnd[i][0][0],tmpstr);
			SetWindowText(tmpWnd[i][0][1],tmpstr);
			tmptimevalue[i][0][0]=0;
			tmptimevalue[i][0][1]=0;

			memset(tmpstr,0,4);
			sprintf(tmpstr,"%02d",23);
			SetWindowText(tmpWnd[i][1][0],tmpstr);
			tmptimevalue[i][1][0]=23;

			memset(tmpstr,0,4);
			sprintf(tmpstr,"%02d",59);
			SetWindowText(tmpWnd[i][1][1],tmpstr);
			tmptimevalue[i][1][1]=59;
		}
	}		

	//	filldataflag=0;
	return 1;

}

static int ismodified(void)
{
	int i,j,k;

	for(i=0;i<7;i++)
	{
		for(j=0;j<2;j++)
		{
			for(k=0;k<2;k++)
			{
				memset(tmpc,0,5);
				GetWindowText(tmpWnd[i][j][k],tmpc,2);
				if(tmptimevalue[i][j][k]!=(unsigned char)atoi(tmpc))
					return 1;				
			}
		}
	}	
	return 0;

}

static int SaveTimeZone(HWND hWnd)
{
	int i,j,k;
	char tmppin[5];
	int tmptimevalue;

	memset(tmppin,0,5);
	GetWindowText(edTZID,tmppin,3);
	if(strlen(tmppin)>0)
	{
		ttzid=atoi(tmppin);
		if(ttzid)
		{
			memset(&myttz,0,sizeof(TTimeZone));
			myttz.ID=ttzid;
			for(i=0;i<7;i++)
			{
				for(j=0;j<2;j++)
				{
					for(k=0;k<2;k++)
					{
						memset(tmppin,0,5);
						GetWindowText(tmpWnd[i][j][k],tmppin,2);
						if(strlen(tmppin)>0)
						{
							tmptimevalue=atoi(tmppin);
							if(tmptimevalue>0)
							{
								if((k==0 && (tmptimevalue<0 || tmptimevalue>23)) || 
										(k==1 && (tmptimevalue<0 || tmptimevalue>59)))
								{
									MessageBox1(hWnd,LoadStrByID(HIT_ERROR0), LoadStrByID(MID_APPNAME), MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
									if(!ismenutimeout) SetFocusChild(tmpWnd[i][j][k]);
									return 0;
								}
								else
									myttz.ITime[i][j][k]=atoi(tmppin);
							}
						}
						else
						{
							SetFocusChild(tmpWnd[i][j][k]);
							return 0;	
						}
					}
				}
			}

			if(FDB_GetTimeZone(ttzid,NULL)!=NULL)
			{
				if(FDB_ChgTimeZone(&myttz)==FDB_OK)
				{
					sync();
					return 1;
				}
			}
			else
			{
				if(FDB_AddTimeZone(&myttz)==FDB_OK)
				{
					sync();
					return 1;
				}
			}
		}
		else
			MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT5), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONQUESTION | MB_BASEDONPARENT);
	}
	else
	{
		MessageBox1(hWnd,LoadStrByID(HIT_ERROR0), LoadStrByID(MID_APPNAME), MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
		if(!ismenutimeout) SetFocusChild(edTZID);
	}
	return 0;
}

static int TimeZoneWinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	//	int tmpvalue;
	int tmpcode;
	//int i;
	char tmpss[12];
	//      int tvalue;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&tzbkbmp,GetBmpPath("submenubg.jpg")))
			//        return 0;
			InitWindow(hWnd);		//add controls
			FillData(hWnd,0);
			UpdateWindow(hWnd,TRUE);
			if(optflag)
			{
				act_x=0;act_y=0;act_z=0;
				SetFocusChild(tmpWnd[act_x][act_y][act_z]);
			}
			else
				SetFocusChild(edTZID);
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
				if(fGetDC) ReleaseDC (hdc);
				return 0;
			}

		case MSG_PAINT:
			hdc = BeginPaint(hWnd);	
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
			if (gOptions.KeyPadBeep) ExKeyBeep();

			tmpfocusWnd=GetFocusChild(hWnd);

			if(LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0L);
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
					|| (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				if(tmpfocusWnd==btSave || tmpfocusWnd==btExit) 
					SelectNext(hWnd,wParam);
				else
				{
					memset(tmpss,0,12);
					GetWindowText(tmpfocusWnd,tmpss,2);
					if(strlen(tmpss)>0)
					{
						int tmpvv;
						if(tmpfocusWnd!=edTZID)
						{
							tmpvv=atoi(tmpss);
							if((act_z==0 && (tmpvv<0 || tmpvv>23)) || (act_z==1 && (tmpvv<0 || tmpvv>59)))
								break;

							memset(tmpss,0,12);
							sprintf(tmpss,"%02d",tmpvv);
							SetWindowText(tmpfocusWnd,tmpss);
						}
						SelectNext(hWnd,wParam);
						return 0;
					}
				}
			}
			else if(LOWORD(wParam)==SCANCODE_ENTER) 
			{
				if(tmpfocusWnd!=btSave && tmpfocusWnd!=btExit)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0L);
			}
			else if(LOWORD(wParam)==SCANCODE_F10)
			{
				if(tmpfocusWnd!=btExit)
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
				else
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_EXIT, 0L);
			}
			else if(LOWORD(wParam)==SCANCODE_MENU)
			{	
				PostMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
			}

			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(nc==EN_CHANGE)
			{
				if(id == ID_TZID)
				{
					memset(tmpss,0,12);
					GetWindowText(edTZID,tmpss,2);
					if(strlen(tmpss)>0)
					{
						if(tmpss[0]=='0')
						{
							SetWindowText(edTZID,"");
							SetFocusChild(edTZID);
						}
						else
						{
							tmpcode=atoi(tmpss);
							if(tmpcode>TZ_MAX)      
							{
								SetWindowText(edTZID, "");
								SetFocusChild(edTZID);
							}
							else
							{
								ttzid=tmpcode;
								FillData(hWnd,1);
							}	
						}
					}	
					else
						break;
				}
#if 0
				else
				{
					if(!filldataflag)
					{
						memset(tmpss,0,12);
						GetWindowText(tmpWnd[act_x][act_y][act_z],tmpss,2);
						if(strlen(tmpss)>0)
						{
							tmpvalue = atoi(tmpss);
							if((act_z==0 && (tmpvalue<0 || tmpvalue>23)) || (act_z==1 && (tmpvalue<0 || tmpvalue>59)))
							{
								printf("x:%d,y:%d,z:%d\n",act_x,act_y,act_z);
								SetWindowText(tmpWnd[act_x][act_y][act_z],"");
								SetFocusChild(tmpWnd[act_x][act_y][act_z]);
							}
						}
					}
				}
#endif
				break;
			}

			switch(id)
			{
				case ID_SAVE:			//
					{
						char idstr[5];
						if(SaveTimeZone(hWnd))
						{
							if(!optflag && (MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT3),LoadStrByID(MID_APPNAME),
											MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK))
							{
								ttzid=GetTimeZoneID();
								memset(idstr,0,5);
								sprintf(idstr,"%d",ttzid);
								SetWindowText(edTZID,idstr);
								SetFocusChild(edTZID);
							}
							else
							{
								if(!ismenutimeout)
									PostMessage(hWnd, MSG_CLOSE,0,0);
							}
						}
						break;
					}
				case ID_EXIT:			//
					if(ismodified() && (MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT4), LoadStrByID(MID_APPNAME),
									MB_OKCANCEL | MB_ICONQUESTION |	MB_BASEDONPARENT)==IDOK))
						PostMessage(hWnd,MSG_COMMAND,ID_SAVE,0L);
					else
					{
						if(!ismenutimeout)
							PostMessage(hWnd,MSG_CLOSE,0,0L);
					}
					break;
			}			
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&tzbkbmp);
			//MainWindowCleanup (hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

static int GetTimeZoneID(void)
{
	int i;
	for(i=1;i<=TZ_MAX;i++)
	{
		if (FDB_GetTimeZone(i, NULL)==NULL)
		{
			return i;
		}
	}
	return TZ_MAX;
}

int TimeZoneMngWindow(HWND hWnd, int tzid, int flag)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	int i;

	optflag = flag;
	ttzid = (tzid) ? tzid:GetTimeZoneID();

	timezonestr[0]=LoadStrByID(MID_USER_TZ);
	for(i=0;i<7;i++)
		timezonestr[i+1]=LoadStrByID(HID_DAY0+i);
	timezonestr[8]=LoadStrByID(MID_TZ_SY);
	timezonestr[9]=LoadStrByID(MID_TZ_TO);
	timezonestr[10]=LoadStrByID(MID_SAVE);
	timezonestr[11]=LoadStrByID(MID_EXIT);

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_LOCK_OP1);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = TimeZoneWinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

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

