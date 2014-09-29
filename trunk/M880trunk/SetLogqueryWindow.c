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
#include "exfun.h"
#include "ssrcommon.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"
#include "truncdate.h"

#define LB_ACNO 50
#define ID_ACNO 51
#define ID_LOGSAVE 332
#define ID_LOGEXIT 333

#define LB1_NO 65
#define LB2_NO 66
#define LB3_NO 67
#define LB4_NO 68
#define LB5_NO 69
#define LB6_NO 70
#define LB7_NO 71
#define LB8_NO 72
#define LB9_NO 73
#define LB10_NO 74
#define LB11_NO 75
#define LB12_NO 76
#define LB13_NO 77
#define LB14_NO 78
#define LB15_NO 79
#define LB16_NO 80
#define LB17_NO 81
#define LB18_NO 82

HWND editacno,Edtmp,btnsaves,btnexits,focuswnd;
HWND edmonth1,edday1,edhour1,edmin1;
HWND edmonth2,edday2,edhour2,edmin2;

static char *resacno1;
static time_t resst=0;
static time_t resed=0;

static char edtacno_cmp[PINLIMITLEN];

//BITMAP  querysetbmp;
BITMAP  querysetbmp1;

// find out whether a year is a leap year
int IsLeapYear (int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) return 1;
    else return 0;
}

int GetMonDayfromyear (int year, int month)
{
    int mon_len;

    if (month < 1 || month > 12) return -1;
    if ((month <= 7 && month % 2 == 1) || (month >= 8 && month % 2 == 0))
        mon_len = 31;
    else if (month == 2)
        if (IsLeapYear (year)) mon_len = 29;
        else mon_len = 28;
    else
        mon_len = 30;
    return mon_len;

}

static void inittimespin(void)
{
	TTime mytime1;
	char dateedit[5];

	memset(dateedit,0,5); 
        GetTime(&mytime1);
	//zsliu add
        if(gOptions.isUseHejiraCalendar)
        {
                ChrisToHejiraTTime(&mytime1); //公历转伊朗日厄17
                //mytime1.tm_mon -= 1;
        }
        //zsliu add end ... ...

        sprintf(dateedit,"%02d",mytime1.tm_mon+1);
	SetWindowText(edmonth1,dateedit);
	SetWindowText(edmonth2,dateedit);
        sprintf(dateedit,"%02d",mytime1.tm_mday);
	SetWindowText(edday1,dateedit);
	SetWindowText(edday2,dateedit);
	SetWindowText(edhour1,"0");
	SetWindowText(edhour2,"24");
}

static void processinputvalid(int comid)
{
	char editcontent[5];
	int midvalue=0;
	HWND editwnd=0;
	TTime mytime1;
	char dateedit[5];

	memset(dateedit,0,5); 
	memset(editcontent,0,5);
        GetTime(&mytime1);
	
	switch (comid)
	{
		case LB2_NO: 
			editwnd=edmonth1;
			break;
		case LB4_NO: 
			editwnd=edday1;
			break;
		case LB15_NO: 
			editwnd=edhour1;
			break;
		case LB9_NO: 
			editwnd=edmonth2;
			break;
		case LB11_NO: 
			editwnd=edday2;
			break;
		case LB17_NO: 
			editwnd=edhour2;
			break;
	}

	GetWindowText(editwnd,editcontent,5);
	if (strlen(editcontent))
	{
		midvalue=atoi(editcontent);
		if ((editwnd==edmonth1) || (editwnd==edmonth2))
		{
			if ((midvalue>12) || (midvalue<0))
			{
        			//sprintf(dateedit,"%d",mytime1.tm_mon+1);
				SetWindowText(editwnd,"");
				SetFocusChild(editwnd);
			}
		}

		if ((editwnd==edday1) || (editwnd==edday2))
		{
			if ((midvalue>31) || (midvalue<0))
			{
        			//sprintf(dateedit,"%d",mytime1.tm_mday);
				SetWindowText(editwnd,"");
				SetFocusChild(editwnd);
			}
		}

		if ((editwnd==edhour1) || (editwnd==edhour2))
		{
			if ((midvalue>24) || (midvalue<0))
			{
				if (editwnd==edhour1)
					SetWindowText(editwnd,"0");
				else
					SetWindowText(editwnd,"24");
				SetFocusChild(editwnd);
			}
		}
		
	}

}

static int ProcessSetFind(HWND hWnd)
{
	//int ret=0;
	int month1,day1,hour1;
	int month2,day2, hour2;
	char editcontent[5];
        int dayerror=0;
	int myyear;
	TTime mytime;

        dayerror=0;
        GetTime(&mytime);
	memset(editcontent,0,5);

	GetWindowText(editacno, resacno1, PINLIMITLEN);
	if ((fromRight==1)&&(strcmp(resacno1,edtacno_cmp)==0))
		memset(resacno1,0,PINLIMITLEN);
	else if (strcmp(resacno1, LoadStrByID(MID_ALL))==0)
	{
		memset(resacno1, 0, PINLIMITLEN);
	}

	GetWindowText(edmonth1,editcontent,5);
	month1=atoi(editcontent);
	GetWindowText(edmonth2,editcontent,5);
	month2=atoi(editcontent);
	
	GetWindowText(edday1,editcontent,5);
	day1=atoi(editcontent);
	GetWindowText(edday2,editcontent,5);
	day2=atoi(editcontent);

	//zsliu add
        if(gOptions.isUseHejiraCalendar)
        {
                TTime startTime, endTime;
                GetTime(&startTime);
                GetTime(&endTime);

                ChrisToHejiraTTime(&startTime); //公历转伊朗日厄17
                ChrisToHejiraTTime(&endTime);   //公历转伊朗日厄17

                startTime.tm_mon = month1 - 1;
                startTime.tm_mday = day1;
                HejiraToChrisTTime(&startTime); //伊朗日历转公厄17
                month1 = startTime.tm_mon + 1;
                day1 = startTime.tm_mday;

                endTime.tm_mon = month2 - 1;
                endTime.tm_mday = day2;
                HejiraToChrisTTime(&endTime); //伊朗日历转公厄17
                month2 = endTime.tm_mon + 1;
                day2 = endTime.tm_mday;

        }
	//zsliu add end ... ...
	
        if (day1 > GetMonDayfromyear((mytime.tm_year+1900),month1))
                dayerror=1;
        if (day2 > GetMonDayfromyear(mytime.tm_year+1900,month2))
                dayerror=1;
	if ((month1<1) ||(month2<1)||(month1>12)||(month2>12))	dayerror=1;	//add  by jazzy  2008.12.02 for first month2 more than month1
	GetWindowText(edhour1,editcontent,5);
	hour1=atoi(editcontent);
	if (hour1==24)
		hour1=23;
	GetWindowText(edhour2,editcontent,5);
	hour2=atoi(editcontent);
	if (hour2==24)
		hour2=23;
	if ((month1==0) || (month2==0) || (day1==0) || (day2==0) || dayerror)
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP, LoadStrByID(MID_APPNAME), LoadStrByID(MID_ERRORTIME));
		if(!ismenutimeout)
		{
			if (month1==0)
				SetFocusChild(edmonth1);
			if (month2==0)
				SetFocusChild(edmonth2);
			if (day1==0)
				SetFocusChild(edday1);
			if (day2==0)
				SetFocusChild(edday2);
		}
		return 0;

	}

        mytime.tm_mon=month1-1;
        mytime.tm_mday = day1;
        mytime.tm_hour = hour1;
        mytime.tm_min = 0;
        mytime.tm_sec = 0;
        mytime.tm_isdst = -1;
        myyear=mytime.tm_year;
        if (month2<month1)
                mytime.tm_year -= 1;
        resst = OldEncodeTime(&mytime);
	//printf("st:%04d-%2d-%2d %2d:%2d\n",mytime.tm_year+1900,mytime.tm_mon+1,mytime.tm_mday,mytime.tm_hour,mytime.tm_min);

        mytime.tm_year=myyear;
        mytime.tm_mon=month2-1;
        mytime.tm_mday=day2;
        mytime.tm_hour = hour2;
        mytime.tm_min = 59;
        mytime.tm_sec = 59;
        mytime.tm_isdst = -1;
        resed = OldEncodeTime(&mytime);
	
	if (resed < resst)
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP, LoadStrByID(MID_APPNAME), LoadStrByID(MID_ERRORTIME));
		if(!ismenutimeout) SetFocusChild(edmonth1);
		return 0;
	}

	if ((resacno1 && strlen(resacno1)) && (FDB_GetUserByCharPIN2(resacno1,NULL)==NULL))
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_NOTENROLLED));
		if(!ismenutimeout) SetFocusChild(editacno);
		return 0;
	}

	if (CreateAttLogWindow(hWnd,resacno1,resst,resed,0)==0)
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(HID_NOATTLOG));
	}

	return 1;

}

static void SelectNext(WPARAM wParam)
{
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == editacno)
			focuswnd = edmonth1;
		else if (Edtmp == edmonth1)
			focuswnd = edday1;
		else if (Edtmp == edday1)
			focuswnd = edhour1;
		else if (Edtmp == edhour1)
			focuswnd = edmonth2;
		else if (Edtmp == edmonth2)
			focuswnd = edday2;
		else if (Edtmp == edday2)
			focuswnd = edhour2;
		else if (Edtmp == edhour2)
			focuswnd = btnsaves;
		else if (Edtmp == btnsaves)
			focuswnd = btnexits;
		else if (Edtmp == btnexits)
			focuswnd = editacno;			
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == editacno)
			focuswnd = btnexits;
		else if (Edtmp == btnsaves)
			focuswnd = edhour2;
		else if (Edtmp == edhour2)
			focuswnd = edday2;
		else if (Edtmp == edday2)
			focuswnd = edmonth2;
		else if (Edtmp == edmonth2)
			focuswnd = edhour1;
		else if (Edtmp == edhour1)
			focuswnd = edday1;
		else if (Edtmp == edday1)
			focuswnd = edmonth1;			
		else if (Edtmp == edmonth1)
			focuswnd = editacno;			
		else if (Edtmp == btnexits)
			focuswnd = btnsaves;			
	}

	SetFocusChild(focuswnd);
}

static void InitWindow (HWND hWnd)
{
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=200+gOptions.ControlOffset;
		posX2=165+gOptions.ControlOffset;
		posX3=140+gOptions.ControlOffset;
		posX4=100+gOptions.ControlOffset;
		posX5=75+gOptions.ControlOffset;
		posX6=40+gOptions.ControlOffset;
		posX7=10+gOptions.ControlOffset;
		posX8=10+gOptions.ControlOffset;
	}
	else
	{
		posX1=10+gOptions.ControlOffset/2;
		posX2=55+gOptions.ControlOffset/2;
		posX3=80+gOptions.ControlOffset/2;
		posX4=120+gOptions.ControlOffset/2;
		posX5=145+gOptions.ControlOffset/2;
		posX6=180+gOptions.ControlOffset/2;
		posX7=205+gOptions.ControlOffset/2;
		posX8=55+gOptions.ControlOffset/2;
	}
	CreateWindow(CTRL_STATIC, LoadStrByID(MID_ACNO), WS_VISIBLE | SS_CENTER, LB_ACNO, posX1, 22, 80, 20, hWnd, 0);
        editacno = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_BORDER, ID_ACNO, posX8+10, 20, 180, 20, hWnd, 0);
        CreateWindow(CTRL_STATIC, LoadStrByID(REP_ATTTMST), WS_VISIBLE | SS_CENTER, LB1_NO, posX1, 72, 80, 20, hWnd, 0);
        edmonth1 = CreateWindow(CTRL_SLEDIT, "03", WS_VISIBLE | WS_CHILD|CBS_EDITNOBORDER| CBS_AUTOFOCUS, LB2_NO, posX2, 70, 20, 20, hWnd, 0);
        CreateWindow(CTRL_STATIC, LoadStrByID(MID_MON), WS_VISIBLE | SS_CENTER|WS_CHILD, LB3_NO, posX3, 72, 30, 20, hWnd, 0);
        edday1 = CreateWindow(CTRL_SLEDIT, "31", WS_VISIBLE | WS_CHILD|CBS_EDITNOBORDER| CBS_AUTOFOCUS, LB4_NO, posX4, 70, 20, 20, hWnd, 0);
        CreateWindow(CTRL_STATIC, LoadStrByID(MID_DAY), WS_VISIBLE | WS_CHILD|SS_CENTER|CBS_EDITNOBORDER, LB5_NO, posX5, 72, 40, 20, hWnd, 0);
        edhour1 = CreateWindow(CTRL_SLEDIT, "31", WS_VISIBLE | WS_CHILD|CBS_EDITNOBORDER|CBS_AUTOFOCUS, LB15_NO, posX6, 70, 20, 20, hWnd, 0);
        CreateWindow(CTRL_STATIC,LoadStrByID(MID_HOUR),WS_VISIBLE|WS_CHILD|SS_CENTER|CBS_EDITNOBORDER, LB16_NO, posX7, 72, 40, 20, hWnd, 0);
        CreateWindow(CTRL_STATIC, LoadStrByID(REP_ATTTMED), WS_VISIBLE | SS_CENTER, LB8_NO, posX1, 132, 80, 20, hWnd, 0);
        edmonth2 = CreateWindow(CTRL_SLEDIT, "03", WS_VISIBLE | WS_CHILD|CBS_EDITNOBORDER|CBS_AUTOFOCUS, LB9_NO, posX2, 130, 20, 20, hWnd, 0);
        CreateWindow(CTRL_STATIC, LoadStrByID(MID_MON), WS_VISIBLE | SS_CENTER|WS_CHILD, LB10_NO, posX3, 132, 30, 20, hWnd, 0);
        edday2 = CreateWindow(CTRL_SLEDIT, "31", WS_VISIBLE | WS_CHILD|CBS_EDITNOBORDER|CBS_AUTOFOCUS, LB11_NO, posX4, 130, 20, 20, hWnd, 0);
        CreateWindow(CTRL_STATIC, LoadStrByID(MID_DAY),WS_VISIBLE|WS_CHILD|SS_CENTER|CBS_EDITNOBORDER, LB12_NO, posX5, 132, 40, 20, hWnd, 0);
        edhour2 = CreateWindow(CTRL_SLEDIT, "31", WS_VISIBLE | WS_CHILD|CBS_EDITNOBORDER|CBS_AUTOFOCUS, LB17_NO, posX6, 130, 20, 20, hWnd, 0);
        CreateWindow(CTRL_STATIC, LoadStrByID(MID_HOUR),WS_VISIBLE|WS_CHILD|SS_CENTER|CBS_EDITNOBORDER, LB18_NO, posX7, 132, 40, 20, hWnd, 0);



	btnsaves = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_SAVE), WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_LOGSAVE,10,180,85,25,hWnd,0);
	btnexits = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_EXIT), WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_LOGEXIT,220+gOptions.ControlOffset,180,85,25,hWnd,0);

	SendMessage(editacno, EM_LIMITTEXT, gOptions.PIN2Width, 0L);
	//SetWindowText(editacno, LoadStrByID(MID_ALL));	//del by jazzy 2008.11.26 blank default 

	if (fromRight==1)
		GetWindowText(editacno,edtacno_cmp,PINLIMITLEN);

	SendMessage(editacno,EM_SETCARETPOS,0,2);
	SendMessage(edmonth1,EM_LIMITTEXT,2,0L);
	SendMessage(edday1,EM_LIMITTEXT,2,0L);
	SendMessage(edhour1,EM_LIMITTEXT,2,0L);
	SendMessage(edmonth2,EM_LIMITTEXT,2,0L);
	SendMessage(edday2,EM_LIMITTEXT,2,0L);
	SendMessage(edhour2,EM_LIMITTEXT,2,0L);
	inittimespin();
}

static int FindSetwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	static char keyupFlag=0;
	switch (message)	
	{
		case MSG_CREATE:
			//LoadBitmap(HDC_SCREEN,&querysetbmp,GetBmpPath("submenubg.jpg"));
			LoadBitmap(HDC_SCREEN,&querysetbmp1,GetBmpPath("findfg.gif"));

			InitWindow(hWnd);		
			UpdateWindow(hWnd,TRUE);
			SetFocusChild(editacno);
			break;

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

                    	FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
                    	FillBoxWithBitmap (hdc, 220+gOptions.ControlOffset+10, 70, 0, 0, &querysetbmp1);


                    	if (fGetDC)
                        	ReleaseDC (hdc);
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
                        if (gOptions.KeyPadBeep) ExKeyBeep();
			
			Edtmp=GetFocusChild(hWnd);
			
			if (LOWORD(wParam)==SCANCODE_ESCAPE)		
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
			{
				SelectNext(wParam);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_ENTER)
			{
				if(Edtmp!=btnexits && Edtmp!=btnsaves)
				{
					ProcessSetFind(hWnd);
					return 0;
				}
			}
			
			if(LOWORD(wParam)==SCANCODE_F10)
			{
				if(Edtmp!=btnexits)
					ProcessSetFind(hWnd);
				else
					PostMessage(hWnd, MSG_COMMAND, ID_LOGEXIT, 0L);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_MENU)
			{
				ProcessSetFind(hWnd);
				return 0;
			}

                        if ((LOWORD(wParam)==SCANCODE_BACKSPACE )&&(GetFocusChild(hWnd)==editacno))
                        {
                                char tmpacnobuf[PINLIMITLEN+1];
                                GetWindowText(editacno, tmpacnobuf, PINLIMITLEN);
				if ((fromRight==1)&&(tmpacnobuf&&(strcmp(tmpacnobuf,edtacno_cmp)==0)))
					SetWindowText(editacno,"");
                                else if (tmpacnobuf && (strcmp(tmpacnobuf, LoadStrByID(MID_ALL))==0))
                                {
                                        SetWindowText(editacno,"");
                                }
                        }

			break;

		case MSG_CLOSE:
			//UnloadBitmap(&querysetbmp);
			UnloadBitmap(&querysetbmp1);
			DestroyMainWindow(hWnd);
			//PostQuitMessage(hWnd);
			return 0;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if (nc==EN_CHANGE)
			{
				if (id != ID_ACNO)
					processinputvalid(id);
			}
		        switch (id)
			{
			        case ID_LOGSAVE:
					ProcessSetFind(hWnd);
					break;
				case ID_LOGEXIT:
					resst = 0;
					resed = 0;
				        PostMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;
			}
			break;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateFindLogSetWindow(HWND hOwner,char *acno,time_t *st1,time_t *ed1)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	resacno1 = acno;
	
	hOwner = GetMainWindowHandle (hOwner);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_ATTQUERY);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = FindSetwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
        CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);
	
	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hMainWnd);

	if (resst && resed)
	{
		*st1 = resst;
		*ed1 = resed;
		return 1;
	}
	else

		return 0;

}

void ShowLogSetWindow(HWND hOwner)
{
        char finduseracno[24];
        time_t sttm1=0;
        time_t edtm1=0;

        sttm1=0;
        edtm1=0;
        memset(finduseracno, 0, 24);
	CreateFindLogSetWindow(hOwner, finduseracno, &sttm1, &edtm1);
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

