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
#include "tftmsgbox.h"

#define HTZ_STATIC 	0x70

#define HTZ_ID		0x80
#undef HTZ_DAY
#define HTZ_DAY		0x81
#define HTZ_TZ		0x85

#define ID_SAVE		0x86
#define ID_EXIT		0x87

HWND EdHDID,EdHDTZ;
HWND EdHDD[2][2];
HWND BtSave,BtExit;
HWND Edtmp,focuswnd;

//BITMAP hdmngbmpbk;
int hdOptFlag;
int bhdchged=0;
int hdID;
THTimeZone mythtz;
char htzstr[20];
static int getMaxDaysByMonth(int mmonth);

extern int GetFirstTimeZone(void);

int formatValue(HWND hWnd)
{
	int tmpvalue;
	memset(htzstr,0,5);
	GetWindowText(hWnd,htzstr,2);
	if(strlen(htzstr)==0)
		return 0;

        tmpvalue=atoi(htzstr);
        memset(htzstr,0,5);
        sprintf(htzstr,"%02d",tmpvalue);        
        SetWindowText(hWnd,htzstr);
	return 1;
}

static int  SelectNext(HWND hWnd,WPARAM wParam)
{
	//int tmpvalue;

	Edtmp  = GetFocusChild(hWnd);
	if(Edtmp!=BtSave && Edtmp!=BtExit)
	{
		if(!formatValue(Edtmp)) return 0;
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == EdHDID)
			focuswnd = EdHDD[0][0];
		else if (Edtmp == EdHDD[0][0])
			focuswnd = EdHDD[0][1];
		else if (Edtmp == EdHDD[0][1])
			focuswnd = EdHDD[1][0];
		else if (Edtmp == EdHDD[1][0])
			focuswnd = EdHDD[1][1];
		else if (Edtmp == EdHDD[1][1])
			focuswnd = EdHDTZ;
		else if (Edtmp == EdHDTZ)
			focuswnd = BtSave;
		else if (Edtmp == BtSave)
			focuswnd = BtExit;
		else if (Edtmp == BtExit)
			focuswnd = (hdOptFlag)?EdHDD[0][0]:EdHDID;
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == EdHDID)
			focuswnd = BtExit;
		else if (Edtmp == BtExit)
			focuswnd = BtSave;
		else if (Edtmp == BtSave)
			focuswnd = EdHDTZ;
		else if (Edtmp == EdHDTZ)
			focuswnd = EdHDD[1][1];
		else if (Edtmp == EdHDD[1][1])
			focuswnd = EdHDD[1][0];
		else if (Edtmp == EdHDD[1][0])
			focuswnd = EdHDD[0][1];
		else if (Edtmp == EdHDD[0][1])
			focuswnd = EdHDD[0][0];
		else if (Edtmp == EdHDD[0][0])
			focuswnd = (hdOptFlag)?BtExit:EdHDID;
	}

	SetFocusChild(focuswnd);
	return 1;
}

static void InitWindow (HWND hWnd)
{
	int i,j;
	char* hdmngstr[8];
	int posX1,posX2,posX3,posX4,posX5;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=210+gOptions.ControlOffset;
		posX2=150+gOptions.ControlOffset;
		posX3=120+gOptions.ControlOffset;
		posX4=-80+gOptions.ControlOffset;
		posX5=30+gOptions.ControlOffset;
	}
	else
	{
		posX1=10+gOptions.ControlOffset;
		posX2=90+gOptions.ControlOffset;
		posX3=140+gOptions.ControlOffset;
		posX4=80+gOptions.ControlOffset;
		posX5=30+gOptions.ControlOffset;
	}
	hdmngstr[0]=LoadStrByID(MID_GP_INDEX);
	hdmngstr[1]=LoadStrByID(MID_HTZ_DATES);
	hdmngstr[2]=LoadStrByID(MID_HTZ_DATEE);
	hdmngstr[3]=LoadStrByID(MID_MONTH);
	hdmngstr[4]=LoadStrByID(MID_DAY1);
	hdmngstr[5]=LoadStrByID(MID_USER_TZ);
	hdmngstr[6]=LoadStrByID(MID_SAVE);
	hdmngstr[7]=LoadStrByID(MID_EXIT);
	
	CreateWindow(CTRL_STATIC, hdmngstr[0],WS_VISIBLE | SS_LEFT,HTZ_STATIC, posX1, 10, 90, 23, hWnd, 0);
	if(hdOptFlag)
	{
		memset(htzstr,0,20);
		sprintf(htzstr,"%02d",mythtz.ID);
		CreateWindow(CTRL_STATIC, htzstr, WS_VISIBLE | SS_LEFT, HTZ_STATIC+1, posX2, 10, 90, 23, hWnd, 0);
	}
	else
	{
		EdHDID = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				HTZ_ID, posX2, 6, 45, 23, hWnd, 0);
		SendMessage(EdHDID,EM_LIMITTEXT,2,0L);
	}

	for(i=0;i<2;i++)
	{
	        CreateWindow(CTRL_STATIC, hdmngstr[i+1],WS_VISIBLE | SS_LEFT,HTZ_STATIC+2+i, posX1, 40+30*i, 90, 23, hWnd, 0);
		for(j=0;j<2;j++)
		{
			EdHDD[i][j] = CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
                                HTZ_DAY+i*2+j, posX2+posX4*j, 36+30*i, 45, 23, hWnd, 0);
			CreateWindow(CTRL_STATIC, hdmngstr[3+j], WS_VISIBLE | SS_LEFT, HTZ_STATIC+3, posX3+posX4*j,40+30*i,30,23, hWnd, 0);
			SendMessage(EdHDD[i][j], EM_LIMITTEXT, 2, 0L);
		}
	}

        CreateWindow(CTRL_STATIC, hdmngstr[5], WS_VISIBLE | SS_LEFT, HTZ_STATIC+8, posX1,100,90,23, hWnd, 0);
	EdHDTZ = CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
                                HTZ_TZ, posX2, 96, 45, 23, hWnd, 0);
	SendMessage(EdHDTZ,EM_LIMITTEXT,2,0L);

	BtSave = CreateWindow(CTRL_BUTTON, hdmngstr[6],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
				ID_SAVE, 10, 190, 90, 23, hWnd, 0);
	BtExit = CreateWindow(CTRL_BUTTON, hdmngstr[7],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
				ID_EXIT, 220+gOptions.ControlOffset, 190, 90, 23, hWnd, 0);

}

static void ShowErrorHint(HWND hWnd, HWND errwnd)
{
	MessageBox1(hWnd,LoadStrByID(HIT_ERROR0), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONSTOP | MB_BASEDONPARENT);
	if(!ismenutimeout) SetFocusChild(errwnd);
}

static int isValidData(HWND hWnd, PHTimeZone phtz)
{
	char tstr[4];
	int mmaxday;
	int tmpval;

	GetWindowText(EdHDD[0][0],tstr,2);
	tmpval = atoi(tstr);
	if(tmpval<1 || tmpval>12)
	{
		ShowErrorHint(hWnd, EdHDD[0][0]);
		return 0;
	}
	else
	{
		phtz->HDate[0][0]=tmpval;
		mmaxday=getMaxDaysByMonth(tmpval);
	}

	GetWindowText(EdHDD[0][1],tstr,2);
	tmpval = atoi(tstr);
	if(tmpval<1 || tmpval>mmaxday)
	{
		ShowErrorHint(hWnd, EdHDD[0][1]);
		return 0;
	}
	else
		phtz->HDate[0][1]=tmpval;
	
	GetWindowText(EdHDD[1][0],tstr,2);
	tmpval = atoi(tstr);
	if(tmpval<1 || tmpval>12)
	{
		ShowErrorHint(hWnd, EdHDD[1][0]);
		return 0;
	}
	else
	{
		phtz->HDate[1][0]=tmpval;
		mmaxday=getMaxDaysByMonth(tmpval);
	}

	GetWindowText(EdHDD[1][1],tstr,2);
	tmpval = atoi(tstr);
	if(tmpval<1 || tmpval>mmaxday)
	{
		ShowErrorHint(hWnd, EdHDD[1][1]);
		return 0;
	}
	else
		phtz->HDate[1][1]=tmpval;

	GetWindowText(EdHDTZ,tstr,2);
	tmpval=atoi(tstr);
	if(tmpval<1 || tmpval>TZ_MAX)
	{
		ShowErrorHint(hWnd, EdHDTZ);
		return 0;
	}

	return 1;
}

static int ismodified(int OptMod)
{
	int i,j;

	if(OptMod)	//edit
	{
		for(i=0;i<2;i++)
		{
			for(j=0;j<2;j++)
			{
				memset(htzstr,0,5);
				GetWindowText(EdHDD[i][j],htzstr,2);
				if(mythtz.HDate[i][j]!=atoi(htzstr))
					return 1;
			}
		}
		memset(htzstr,0,5);
		GetWindowText(EdHDTZ,htzstr,2);
		if(mythtz.TZID!=atoi(htzstr))
			return 1;
	}
	else
	{
		for(i=0;i<2;i++)
		{
			for(j=0;j<2;j++)
			{
				memset(htzstr,0,5);
				GetWindowText(EdHDD[i][j],htzstr,2);
				if(atoi(htzstr)!=0)
					return 1;
			}
		}
		
                memset(htzstr,0,5);
                GetWindowText(EdHDTZ,htzstr,2);
                if(atoi(htzstr)!=0)
                        return 1;
	}
	return 0;

}

static int SaveHolidayInfo(HWND hWnd)
{
	//int i,j;
	int tmpvalue;
	TTimeZone ttz;

	memset(&mythtz,0,sizeof(TGroup));
	if(hdOptFlag)
		mythtz.ID = hdID;
	else
	{
		memset(htzstr,0,5);
		GetWindowText(EdHDID,htzstr,2);
		if(htzstr && atoi(htzstr)>0)
		{
			hdID = atoi(htzstr);
			if(FDB_GetHTimeZone(hdID,&mythtz)!=NULL)
			{
				MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT10), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONQUESTION |
					MB_BASEDONPARENT);
				if(!ismenutimeout) SetFocusChild(EdHDID);
				return 0;
			}
			else
				mythtz.ID = hdID;
		}
		else
		{
			MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT11), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONSTOP | MB_BASEDONPARENT);
			if(!ismenutimeout) SetFocusChild(EdHDID);
			return 0;
		}
	}

	if(!isValidData(hWnd,&mythtz))
		return 0;

	memset(htzstr,0,5);
	GetWindowText(EdHDTZ,htzstr,2);
	tmpvalue=atoi(htzstr);
	memset(&ttz,0,sizeof(TTimeZone));
	if(FDB_GetTimeZone(tmpvalue,&ttz)==NULL)
	{
		if(MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT2), LoadStrByID(MID_APPNAME),
				MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
			TimeZoneMngWindow(hWnd,tmpvalue,1);               //add new time zone
		return 0;
	}
	else
		mythtz.TZID = tmpvalue;

	if(hdOptFlag)
	{
		if(FDB_ChgHTimeZone(&mythtz)==FDB_OK)
		{
			sync();
			return 1;
		}
	}
	else
	{
		if(FDB_AddHTimeZone(&mythtz)==FDB_OK)
		{
			sync();
			return 1;
		}
	}

	return 0;

}

static void FillHolidayData(int OptMod)
{
	int i,j;
	char dfchar[10];

	if(OptMod)	//edit
	{
		for(i=0;i<2;i++)
		{
			for(j=0;j<2;j++)
			{
				memset(htzstr,0,5);
				sprintf(htzstr,"%02d",mythtz.HDate[i][j]);
				SetWindowText(EdHDD[i][j],htzstr);
			}
		}

		memset(htzstr,0,5);
		sprintf(htzstr,"%02d",mythtz.TZID);
		SetWindowText(EdHDTZ,htzstr);
	}
	else		//add
	{
		memset(htzstr,0,5);
		sprintf(htzstr,"%02d",hdID);
		SetWindowText(EdHDID,htzstr);
		for(i=0;i<2;i++)
		{
			for(j=0;j<2;j++)
			{
				SetWindowText(EdHDD[i][j],"01");
			}
		}
		sprintf(dfchar,"%02d", GetFirstTimeZone());
		SetWindowText(EdHDTZ,dfchar);
	}

}

static U16 GetFreeHolidayID(U16 id)
{
        int i;

        for(i=id;i<=HTZ_MAX;i++)
        {
                memset(&mythtz,0,sizeof(THTimeZone));
                if(FDB_GetHTimeZone(i,&mythtz)==NULL)
                        return i;
        }
        return 0;

}

static int getMaxDaysByMonth(int mmonth)
{
	TTime tt;
	GetTime(&tt);
	return GetMonDay(tt.tm_year+1900,mmonth);	
}

static int holidaymngwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	char sstr[20];
	int tmpvalue;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&hdmngbmpbk,GetBmpPath("submenubg.jpg")))
	                 //       return 0;

			InitWindow(hWnd);		//add controls
			FillHolidayData(hdOptFlag);
			UpdateWindow(hWnd,TRUE);
			if(hdOptFlag)
				SetFocusChild(EdHDD[0][0]);
			else
				SetFocusChild(EdHDID);
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
			Edtmp = GetFocusChild(hWnd);

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
				PostMessage(hWnd, MSG_COMMAND, ID_EXIT, 0L);

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN || LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				SelectNext(hWnd,wParam);
				return 0;
			}
			
			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
                                if(Edtmp==BtExit)
                                        SetFocusChild(BtSave);
				else if(Edtmp==BtSave)
					SetFocusChild(BtExit);
				break;
			}

			if(LOWORD(wParam)==SCANCODE_ENTER)
			{
				if(Edtmp!=BtExit && Edtmp!=BtSave)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0);
			}
		
			if(LOWORD(wParam)==SCANCODE_F10)
			{
				if(Edtmp!=BtExit)
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0);
				else
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_EXIT, 0);
			}
	
			if(LOWORD(wParam)==SCANCODE_MENU)
				PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);

			break;
	
		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(id==HTZ_ID && nc==EN_CHANGE)
			{
				memset(sstr,0,5);
				GetWindowText(EdHDID,sstr,2);
				tmpvalue = atoi(sstr);
				if(tmpvalue>HTZ_MAX)
				{
					memset(sstr,0,5);
					sprintf(sstr,"%d",GetFreeHolidayID(1));
					SetWindowText(EdHDID,sstr);
					SendMessage(EdHDID, EM_SETCARETPOS, 0, strlen(sstr));
				}
			}

			switch(id)
			{
				case ID_SAVE:	
					if(SaveHolidayInfo(hWnd))	
					{
						bhdchged = 1;
						PostMessage(hWnd,MSG_CLOSE,0,0);
					}
					break;

				case ID_EXIT:				
	                                if(ismodified(hdOptFlag) && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
                                                	MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
                                                PostMessage(hWnd, MSG_COMMAND, ID_SAVE, 0);
       	                                else
						if(!ismenutimeout) PostMessage(hWnd,MSG_CLOSE,0,0L);
	                                break;
			}			

			break;
		
		case MSG_CLOSE:
			//UnloadBitmap(&hdmngbmpbk);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateHolidayOptWindow(HWND hWnd, int optmod, int *hdid)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	//char sstr[10];
	char sstr[20];

	hdOptFlag = optmod;

	memset(sstr,0,sizeof(sstr));
	switch(hdOptFlag)
	{
		case 0:	//add holiday
			hdID = (*hdid) ? *hdid:GetFreeHolidayID(1);
			sprintf(sstr,"%s",LoadStrByID(MID_HTZ_ADD));
			break;
		case 1:	//edit group
			hdID = *hdid;
			memset(&mythtz,0,sizeof(THTimeZone));
			if(FDB_GetHTimeZone(hdID,&mythtz)==NULL) 
				return 0;
			sprintf(sstr,"%s",LoadStrByID(MID_HTZ_EDIT));
			break;
	}
	
	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = sstr;
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = holidaymngwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
        CreateInfo.by = gOptions.LCDHeight;
	//CreateInfo.iBkColor = COLOR_lightgray;
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

	*hdid=hdID;
	MainWindowThreadCleanup(hMainWnd);
	return bhdchged;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

