/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录:
* 编译环境:mipsel-gcc
*/ 


#ifndef __CWX_GUI_DATETIME

#define __CWX_GUI_DATETIME

#include "ssrcommon.h"
#include "rtc.h"
#include "exfun.h"
#include "ssrpub.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"
#include "truncdate.h"

//zsliu add 
extern int checkHejiraDate(int *year, int *mon, int *day, int *err);
extern int isHejiraLeap(int nYear);
//zsliu add end ... ...


static int dtFlage=0;

static DLGTEMPLATE DateTimeDlgBox =
{
    WS_BORDER | WS_CAPTION, 
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    36, NULL,
    0
};

#define IDC_DATEYEAR 400
#define IDC_DATEMON  401     
#define IDC_DATEDAY  402
#define IDC_TIMEHOUR 403
#define IDC_TIMEMIN  404
#define IDC_TIMESEC  405

#define IDC_DL  406
#define IDC_DLM  407
#define IDC_DLD  408
#define IDC_DLH  409
#define IDC_DLMIN  410
#define IDC_DLM2  411
#define IDC_DLD2  412
#define IDC_DLH2  413
#define IDC_DLMIN2  414
#define IDC_TIMERDATE    415


static CTRLDATA DateTimeCtrl [] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, 20, 40, 20, 
        0x030, 
        "",
        0
    },
    {
	CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        55, 20, 50, 20,
        IDC_DATEYEAR,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_CENTER,
        110, 20, 20, 20,
        0x031,
        "",
        0
    },
    {
	CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        135, 20, 40, 20,
        IDC_DATEMON,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_CENTER,
        180, 20, 20, 20,
        0x032,
        "",
        0
    },
    {
	CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        205, 20, 40, 20,
        IDC_DATEDAY,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_CENTER,
        250, 20, 20, 20,
        0x033,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_CENTER,
        10, 60, 40, 20,
        0x034,
        "",
        0
    },
    {
	CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        55, 60, 30, 20,
        IDC_TIMEHOUR,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_CENTER,
        90, 60, 20, 20,
        0x035,
        "",
        0
    },
    {
	CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        115, 60, 30, 20,
        IDC_TIMEMIN,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_CENTER,
        150, 60, 20, 20,
        0x036,
        "",
        0
    },
    {
	CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        175, 60, 30, 20,
        IDC_TIMESEC,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_CENTER,
        210, 60, 20, 20,
        0x037,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, 90, 100, 20,
        0x038,
        "",
        0
    },
    {
      CTRL_COMBOBOX,
        WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT |CBS_AUTOFOCUS,
        105, 90, 80, 23,
        IDC_DL,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, 120, 120, 20,
        0x039,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        105, 120, 20, 20,
        IDC_DLM,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        135, 120, 20, 20,
        0x040,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        160, 120, 20, 20,
        IDC_DLD,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        190, 120, 20, 20,
        0x041,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        215, 120, 20, 20,
        IDC_DLH,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        245, 120, 20, 20,
        0x042,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        270, 120, 20, 20,
        IDC_DLMIN,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        300, 120, 20, 20,
        0x043,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, 150, 100, 20,
        0x044,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        105, 150, 20, 20,
        IDC_DLM2,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        135, 150, 20, 20,
        0x045,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        160, 150, 20, 20,
        IDC_DLD2,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        190, 150, 20, 20,
        0x046,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        215, 150,20, 20,
        IDC_DLH2,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        245, 150, 20, 20,
        0x047,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        270, 150, 20, 20,
        IDC_DLMIN2,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        300, 150, 20, 20,
        0x048,
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        225, 180, 85, 25,
        IDCANCEL,
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        10, 180, 85, 25,
        IDOK, 
        "",
        0
    }
};

static HWND DateItemWnd[18];
static int DateItem;
//static BITMAP datebk;

void disabledlbox(int flag)
{
        int i;
        for (i=7;i<15;i++)
                SendMessage(DateItemWnd[i],MSG_ENABLE,flag,0);
}

static int CheckDateItem(int item)
{
        char cc[256];
        int mm;
        GetWindowText(DateItemWnd[item],cc,256);
        mm=atol(cc);
        switch(item)
        {
                case 0:
                {

                        //zsliu change
                        if(gOptions.isUseHejiraCalendar)
                        {
                                if((mm >= 1380) && (mm <= 1408))
                                        return 1;
                        }
                        else
                        {
                                 if(mm>=2000&&mm<=2037)
                                        return 1;
                        }
                        //zsliu change end ... ...
                }
			break;
		case 1:
                case 7:
                case 11:
                        if(mm>0&&mm<13) return 1;
                        break;
                case 2:
                case 8:
                case 12:
                        if(mm>0&&mm<32) return 1;
                        break;
                case 3:
                case 9:
                case 13:
                        if(mm>=0&&mm<24) return 1;
                        break;
                case 4:
                case 10:
                case 14:
                        if(mm>=0&&mm<60) return 1;
                        break;
                case 5:
                        if(mm>=0&&mm<60) return 1;
                        break;
        }
        return 0;
}

static int DateOK(HWND hWnd,HWND Item[],int Size)
{
	int year,mon,day,hour,min,sec;
	struct tm tb;
        time_t tt;
	int err=0;
	char c1[5];
	int mon1 = 0, day1 = 0, hour1 = 0, min1 = 0;
	int mon2 = 0, day2 = 0, hour2 = 0, min2 = 0;
	int sel;

	memset((void*)c1, 0, sizeof(c1));
	if(dtFlage)
	{
		if(GetWindowTextLength(Item[0])==0) err++;
		GetWindowText(Item[0],c1,4);
		year=atol(c1);
		
		if(GetWindowTextLength(Item[1])==0) err++;
		GetWindowText(Item[1],c1,4);
                mon=atol(c1);

		if(GetWindowTextLength(Item[2])==0) err++;
		GetWindowText(Item[2],c1,4);
                day=atol(c1);

		if(GetWindowTextLength(Item[3])==0) err++;
		GetWindowText(Item[3],c1,4);
                hour=atol(c1);

		if(GetWindowTextLength(Item[4])==0) err++;
		GetWindowText(Item[4],c1,4);
                min=atol(c1);

		if(GetWindowTextLength(Item[5])==0) err++;
		GetWindowText(Item[5],c1,4);
                sec=atol(c1);


                //zsliu change
		if(gOptions.isUseHejiraCalendar)
		{
			checkHejiraDate(&year, &mon, &day, &err);
		}
                else
                {
                        if((year >= 2038) || (year < 2000))
                                err++;
                        if(mon<=0||mon>=13) err++;
                        if(day<=0||day>=32) err++;
                }
                //zsliu change end ... ...

		if(hour<0||hour>=24) err++;
		if(min<0||min>=60||sec<0||sec>=60) err++;
		if (day>GetMonDay(year,mon)) err++;

                //夏令时
                sel=SendMessage(Item[6],CB_GETCURSEL,0,0);
                gOptions.DaylightSavingTimeOn=sel;
                if (gOptions.DaylightSavingTimeFun && gOptions.DaylightSavingTimeOn)
                {
			struct tm tb1;
	        	time_t tt1,tt2;

                        if(GetWindowTextLength(Item[7])==0) err++;
                        GetWindowText(Item[7],c1,4);
                        mon1=atol(c1);

                        if(GetWindowTextLength(Item[8])==0) err++;
                        GetWindowText(Item[8],c1,4);
                        day1=atol(c1);

                        if(GetWindowTextLength(Item[9])==0) err++;
                        GetWindowText(Item[9],c1,4);
                        hour1=atol(c1);

                        if(GetWindowTextLength(Item[10])==0) err++;
                        GetWindowText(Item[10],c1,4);
                        min1=atol(c1);

                        //zsliu add 
			int year1 = 0;
			if(gOptions.isUseHejiraCalendar)
			{
				//get current year;
				if(GetWindowTextLength(Item[0])==0){ 
					err++;
				}
				GetWindowText(Item[0],c1,4);
				year1 = atol(c1);
				checkHejiraDate(&year1, &mon1, &day1, &err);
			}
			else 
			{
				if(mon1<=0||mon1>=13) err++;
				if(day1<=0||day1>=32) err++;
			}
			//zsliu add end ... ...
						
                        if(hour1<0||hour1>=24) err++;
                        if(min1<0||min1>=60) err++;
			//zsliu change 
			if(gOptions.isUseHejiraCalendar)
			{
				if (day1>GetMonDay(year1,mon1)) err++;
			}
			else
			{
				if (day1>GetMonDay(year,mon1)) err++;
			}
			//zsliu change end ... ...
//                        if (day1>GetMonDay(year,mon1)) err++;
			
			if(gOptions.isUseHejiraCalendar)
                                tb1.tm_year = year1;
                        else
                                tb1.tm_year = year;
			//zsliu change end ... ...
        		tb1.tm_mon = mon1;
        		tb1.tm_mday = day1;
        		tb1.tm_hour = hour1;
        		tb1.tm_min = min1;
        		tb1.tm_sec = sec;
        		tb1.tm_year -= 1900;
        		tb1.tm_mon -= 1;
        		tb1.tm_isdst = -1;
        		tt1 = EncodeTime(&tb1);

                        if(GetWindowTextLength(Item[11])==0) err++;
                        GetWindowText(Item[11],c1,4);
                        mon2=atol(c1);
                        if(GetWindowTextLength(Item[12])==0) err++;
                        GetWindowText(Item[12],c1,4);
                        day2=atol(c1);

                        if(GetWindowTextLength(Item[13])==0) err++;
                        GetWindowText(Item[13],c1,4);
                        hour2=atol(c1);

                        if(GetWindowTextLength(Item[14])==0) err++;
                        GetWindowText(Item[14],c1,4);
                        min2=atol(c1);

                        //zsliu add
			int year2 = 0; 
			if(gOptions.isUseHejiraCalendar)
			{
				//get current year;
				if(GetWindowTextLength(Item[0])==0) err++;
				GetWindowText(Item[0],c1,4);
				year2 = atol(c1);
		
				checkHejiraDate(&year2, &mon2, &day2, &err);
			}
			else 
			{
				if(mon2<=0||mon2>=13) err++;
				if(day2<=0||day2>=32) err++;
			}
			//zsliu add end ... ...
                        if(hour2<0||hour2>=24) err++;
                        if(min2<0||min2>=60) err++;
			//zsliu change 
                        if(gOptions.isUseHejiraCalendar)
			{
				if (day2>GetMonDay(year2,mon2)) err++;
			}
                        else    
			{
                                if (day2>GetMonDay(year,mon2)) err++;
			}
                        //zsliu change end ... ...

                        //if (mon2<mon1)  err++;

			if(gOptions.isUseHejiraCalendar)
				tb1.tm_year = year2;
			else
        			tb1.tm_year = year;
        		tb1.tm_mon = mon2;
        		tb1.tm_mday = day2;
        		tb1.tm_hour = hour2;
        		tb1.tm_min = min2;
        		tb1.tm_sec = sec;
        		tb1.tm_year -= 1900;
        		tb1.tm_mon -= 1;
        		tb1.tm_isdst = -1;
        		tt2 = EncodeTime(&tb1);

			if (tt2>tt1 && (tt2-tt1)<=3600) err++;
			if (tt2<tt1 && (mon1-mon2)<1) err++;

                }


		if(err)
		{
			MessageBox1 (hWnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                	return 0;
		}
		else
		{
        		tb.tm_year = year;
        		tb.tm_mon = mon;
        		tb.tm_mday = day;
        		tb.tm_hour = hour;
        		tb.tm_min = min;
        		tb.tm_sec = sec;
        		tb.tm_year -= 1900;
        		tb.tm_mon -= 1;
        		tb.tm_isdst = -1;
        		tt = EncodeTime(&tb);
        		SetRTCClock(&tb);
        		DelayUS(100*1000);
        		ReadRTCClockToSyncSys(&tb);
			FDB_AddOPLog(ADMINPIN, OP_SET_TIME, tb.tm_year, tb.tm_mon, tb.tm_mday, tb.tm_hour*tb.tm_min);

                        //夏令时处理
                        if (gOptions.DaylightSavingTimeOn)
                        {
                                gOptions.DaylightSavingTime= (mon1&0xFF)<<24|(day1&0xFF)<<16|(hour1&0xFF)<<8|(min1&0xFF);
                                gOptions.StandardTime= (mon2&0xFF)<<24|(day2&0xFF)<<16|(hour2&0xFF)<<8|(min2&0xFF);
                                if (IsDaylightSavingTimeMenu())
                                {
                                        gOptions.CurTimeMode = 1;
                                }
                                else
                                {
                                        gOptions.CurTimeMode = 2;
                                }

                        }
                        else
			{
				gOptions.CurTimeMode=0;
			}
                        SaveOptions(&gOptions);

			SetMenuTimeOut(time(NULL));
        		MessageBox1 (hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
		}
	}
        return 1;
}


int dlsel;
static int DateTimeDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	struct tm *gDate;
	time_t gTime;
	char info[256];
	//zsliu add 
	int year=0;
	//zsliu add end ... ...
	static char keyupFlag=0;
	switch (message) {

	case MSG_INITDIALOG:
	dtFlage=0;
	//if (LoadBitmap(HDC_SCREEN,&datebk,GetBmpPath("submenubg.jpg"))){
        //                return 0;
	//}
	SetWindowText(GetDlgItem(hDlg,0x030),LoadStrByID(HIT_DATE1));
	SetWindowText(GetDlgItem(hDlg,0x031),LoadStrByID(MID_YEAR));
	SetWindowText(GetDlgItem(hDlg,0x032),LoadStrByID(MID_MONTH));
	SetWindowText(GetDlgItem(hDlg,0x033),LoadStrByID(MID_DAY1));
	SetWindowText(GetDlgItem(hDlg,0x034),LoadStrByID(MID_TIME));
	SetWindowText(GetDlgItem(hDlg,0x035),LoadStrByID(MID_HOUR1));
	SetWindowText(GetDlgItem(hDlg,0x036),LoadStrByID(MID_MINUTE));
	SetWindowText(GetDlgItem(hDlg,0x037),LoadStrByID(HIT_DATE8));
        SetWindowText(GetDlgItem(hDlg,0x038),LoadStrByID(MID_DAYLIGHTSAVINGTIMEON));
        SetWindowText(GetDlgItem(hDlg,0x039),LoadStrByID(MID_DAYLIGHTSAVINGTIME));
        SetWindowText(GetDlgItem(hDlg,0x040),LoadStrByID(MID_MONTH));
        SetWindowText(GetDlgItem(hDlg,0x041),LoadStrByID(MID_DAY1));
        SetWindowText(GetDlgItem(hDlg,0x042),LoadStrByID(MID_HOUR1));
        SetWindowText(GetDlgItem(hDlg,0x043),LoadStrByID(MID_MINUTE));

        SetWindowText(GetDlgItem(hDlg,0x044),LoadStrByID(MID_STANDARDTIME));
        SetWindowText(GetDlgItem(hDlg,0x045),LoadStrByID(MID_MONTH));
        SetWindowText(GetDlgItem(hDlg,0x046),LoadStrByID(MID_DAY1));
        SetWindowText(GetDlgItem(hDlg,0x047),LoadStrByID(MID_HOUR1));
        SetWindowText(GetDlgItem(hDlg,0x048),LoadStrByID(MID_MINUTE));

	SetWindowText(GetDlgItem(hDlg,IDCANCEL),LoadStrByID(HIT_CANCEL));                        
	SetWindowText(GetDlgItem(hDlg,IDOK),LoadStrByID(HIT_OK));

	gTime=time(0);
        gDate=localtime(&gTime);

        SendDlgItemMessage(hDlg, IDC_DATEYEAR, CB_SETSPINFORMAT, 0, (LPARAM)"%04d");

	//zsliu add
        if(gOptions.isUseHejiraCalendar)
        {
		year = gDate->tm_year;
                SendDlgItemMessage(hDlg, IDC_DATEYEAR, CB_SETSPINRANGE, 1380, 1408);
                ChrisToHejiraTTime(gDate);      //公历转伊朗日历

                SendDlgItemMessage(hDlg, IDC_DATEYEAR, CB_SETSPINVALUE, gDate->tm_year+1900, 0);
                //gDate->tm_mon = gDate->tm_mon - 1;
        }
        else
        {
                SendDlgItemMessage(hDlg, IDC_DATEYEAR, CB_SETSPINRANGE, 2000, 2037);
                SendDlgItemMessage(hDlg, IDC_DATEYEAR, CB_SETSPINVALUE, gDate->tm_year+1900, 0);
        }
        //zsliu change end ... ...

        SendDlgItemMessage(hDlg, IDC_DATEYEAR, CB_SETSPINPACE, 1, 1);

        SendDlgItemMessage(hDlg, IDC_DATEMON, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DATEMON, CB_SETSPINRANGE, 1, 12);
        SendDlgItemMessage(hDlg, IDC_DATEMON, CB_SETSPINVALUE, gDate->tm_mon+1, 0);
        SendDlgItemMessage(hDlg, IDC_DATEMON, CB_SETSPINPACE, 1, 1);


        SendDlgItemMessage(hDlg, IDC_DLM, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DLM, CB_SETSPINRANGE, 1, 12);
        SendDlgItemMessage(hDlg, IDC_DLM, CB_SETSPINVALUE, gDate->tm_mon+1, 0);
        SendDlgItemMessage(hDlg, IDC_DLM, CB_SETSPINPACE, 1, 1);

        SendDlgItemMessage(hDlg, IDC_DLM2, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DLM2, CB_SETSPINRANGE, 1, 12);
        SendDlgItemMessage(hDlg, IDC_DLM2, CB_SETSPINVALUE, gDate->tm_mon+1, 0);
        SendDlgItemMessage(hDlg, IDC_DLM2, CB_SETSPINPACE, 1, 1);


        SendDlgItemMessage(hDlg, IDC_DATEDAY, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DATEDAY, CB_SETSPINRANGE, 1, 31);
        SendDlgItemMessage(hDlg, IDC_DATEDAY, CB_SETSPINVALUE, gDate->tm_mday, 0);
        SendDlgItemMessage(hDlg, IDC_DATEDAY, CB_SETSPINPACE, 1, 1);


        SendDlgItemMessage(hDlg, IDC_DLD, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DLD, CB_SETSPINRANGE, 1, 31);
        SendDlgItemMessage(hDlg, IDC_DLD, CB_SETSPINVALUE, gDate->tm_mday, 0);
        SendDlgItemMessage(hDlg, IDC_DLD, CB_SETSPINPACE, 1, 1);

        SendDlgItemMessage(hDlg, IDC_DLD2, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DLD2, CB_SETSPINRANGE, 1, 31);
        SendDlgItemMessage(hDlg, IDC_DLD2, CB_SETSPINVALUE, gDate->tm_mday, 0);
        SendDlgItemMessage(hDlg, IDC_DLD2, CB_SETSPINPACE, 1, 1);
        SendDlgItemMessage(hDlg, IDC_TIMEHOUR, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_TIMEHOUR, CB_SETSPINRANGE, 0, 23);
        SendDlgItemMessage(hDlg, IDC_TIMEHOUR, CB_SETSPINVALUE, gDate->tm_hour, 0);
        SendDlgItemMessage(hDlg, IDC_TIMEHOUR, CB_SETSPINPACE, 1, 1);


        SendDlgItemMessage(hDlg, IDC_DLH, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DLH, CB_SETSPINRANGE, 0, 23);
        SendDlgItemMessage(hDlg, IDC_DLH, CB_SETSPINVALUE, gDate->tm_hour, 0);
        SendDlgItemMessage(hDlg, IDC_DLH, CB_SETSPINPACE, 1, 1);

        SendDlgItemMessage(hDlg, IDC_DLH2, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DLH2, CB_SETSPINRANGE, 0, 23);
        SendDlgItemMessage(hDlg, IDC_DLH2, CB_SETSPINVALUE, gDate->tm_hour, 0);
        SendDlgItemMessage(hDlg, IDC_DLH2, CB_SETSPINPACE, 1, 1);


        SendDlgItemMessage(hDlg, IDC_TIMEMIN, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_TIMEMIN, CB_SETSPINRANGE, 0, 59);
        SendDlgItemMessage(hDlg, IDC_TIMEMIN, CB_SETSPINVALUE, gDate->tm_min, 0);
        SendDlgItemMessage(hDlg, IDC_TIMEMIN, CB_SETSPINPACE, 1, 1);


        SendDlgItemMessage(hDlg, IDC_DLMIN, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DLMIN, CB_SETSPINRANGE, 0, 59);
        SendDlgItemMessage(hDlg, IDC_DLMIN, CB_SETSPINVALUE, gDate->tm_min, 0);
        SendDlgItemMessage(hDlg, IDC_DLMIN, CB_SETSPINPACE, 1, 1);
        SendDlgItemMessage(hDlg, IDC_DLMIN2, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_DLMIN2, CB_SETSPINRANGE, 0, 59);
        SendDlgItemMessage(hDlg, IDC_DLMIN2, CB_SETSPINVALUE, gDate->tm_min, 0);
        SendDlgItemMessage(hDlg, IDC_DLMIN2, CB_SETSPINPACE, 1, 1);

        SendDlgItemMessage(hDlg, IDC_TIMESEC, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_TIMESEC, CB_SETSPINRANGE, 0, 59);
        SendDlgItemMessage(hDlg, IDC_TIMESEC, CB_SETSPINVALUE, gDate->tm_sec, 0);
        SendDlgItemMessage(hDlg, IDC_TIMESEC, CB_SETSPINPACE, 1, 1);


        SendDlgItemMessage(hDlg, IDC_DL,CB_ADDSTRING ,0,(LPARAM)SwitchStr[0]);
        SendDlgItemMessage(hDlg, IDC_DL,CB_ADDSTRING ,0,(LPARAM)SwitchStr[1]);


        DateItemWnd[0] = GetDlgItem (hDlg, IDC_DATEYEAR);
	DateItemWnd[1] = GetDlgItem (hDlg, IDC_DATEMON);
	DateItemWnd[2] = GetDlgItem (hDlg, IDC_DATEDAY);
	DateItemWnd[3] = GetDlgItem (hDlg, IDC_TIMEHOUR);
	DateItemWnd[4] = GetDlgItem (hDlg, IDC_TIMEMIN);
	DateItemWnd[5] = GetDlgItem (hDlg, IDC_TIMESEC);

        DateItemWnd[6] = GetDlgItem (hDlg, IDC_DL);
        DateItemWnd[7] = GetDlgItem (hDlg, IDC_DLM);
        DateItemWnd[8] = GetDlgItem (hDlg, IDC_DLD);
        DateItemWnd[9] = GetDlgItem (hDlg, IDC_DLH);
        DateItemWnd[10] = GetDlgItem (hDlg, IDC_DLMIN);
        DateItemWnd[11] = GetDlgItem (hDlg, IDC_DLM2);
        DateItemWnd[12] = GetDlgItem (hDlg, IDC_DLD2);
        DateItemWnd[13] = GetDlgItem (hDlg, IDC_DLH2);
        DateItemWnd[14] = GetDlgItem (hDlg, IDC_DLMIN2);

        DateItemWnd[15] = GetDlgItem (hDlg, IDOK);
        DateItemWnd[16] = GetDlgItem (hDlg, IDCANCEL);

		
	//zsliu add
	TTime DateStart;
	TTime DateEnd;
	
	if(gOptions.isUseHejiraCalendar)
	{
		time_t Time;
	
		Time = time(0);
		OldDecodeTime(Time, &DateStart);
		OldDecodeTime(Time, &DateEnd);
		
		DateStart.tm_year = year;
		DateStart.tm_mon = ((gOptions.DaylightSavingTime)>>24&0xFF) - 1; //7
		DateStart.tm_mday = (gOptions.DaylightSavingTime)>>16&0xFF;  //8
		
		DateEnd.tm_year = year;
		DateEnd.tm_mon = ((gOptions.StandardTime)>>24&0xFF) - 1; //11
		DateEnd.tm_mday = (gOptions.StandardTime)>>16&0xFF;  //12
		
//		DBPRINTF("----------- DateStart 1 = %d/%d/%d,   %d\n", DateStart.tm_year, DateStart.tm_mon, DateStart.tm_mday, &DateStart);
//		DBPRINTF("----------- DateEnd 1 = %d/%d/%d,    %d\n", DateEnd.tm_year, DateEnd.tm_mon, DateEnd.tm_mday, &DateEnd);
			
		ChrisToHejiraTTime(&DateStart);      //公历转伊朗日历
		ChrisToHejiraTTime(&DateEnd);      //公历转伊朗日历
		
// 		DBPRINTF("----------- DateStart 2 = %d/%d/%d,   %d\n", DateStart.tm_year, DateStart.tm_mon, DateStart.tm_mday, &DateStart);
// 		DBPRINTF("----------- DateEnd 2 = %d/%d/%d,    %d\n", DateEnd.tm_year, DateEnd.tm_mon, DateEnd.tm_mday, &DateEnd);
	}
		
        sprintf(info,"%.4d",gDate->tm_year+1900);
        SetWindowText(DateItemWnd[0],info);
        sprintf(info,"%.2d",gDate->tm_mon+1);
        SetWindowText(DateItemWnd[1],info);
		
	//zsliu change 
	if (gOptions.DaylightSavingTimeOn)
	{
		if(gOptions.isUseHejiraCalendar)
			sprintf(info,"%.2d",DateStart.tm_mon + 1);
		else
			sprintf(info,"%.2d",(gOptions.DaylightSavingTime)>>24&0xFF);
	}
		
	SetWindowText(DateItemWnd[7],info);
	if (gOptions.DaylightSavingTimeOn)
	{
		if(gOptions.isUseHejiraCalendar)
			sprintf(info,"%.2d",DateEnd.tm_mon + 1);
		else
			sprintf(info,"%.2d",(gOptions.StandardTime)>>24&0xFF);
	}
		
	SetWindowText(DateItemWnd[11],info);
        
	if (gOptions.DaylightSavingTimeOn)
	{
		if(gOptions.isUseHejiraCalendar)
			sprintf(info,"%.2d",DateStart.tm_mday);
		else
			sprintf(info,"%.2d",(gOptions.DaylightSavingTime)>>16&0xFF);
	}
		
	SetWindowText(DateItemWnd[8],info);
		
	if (gOptions.DaylightSavingTimeOn)
	{
		if(gOptions.isUseHejiraCalendar)
			sprintf(info,"%.2d",DateEnd.tm_mday);
		else
			sprintf(info,"%.2d",(gOptions.StandardTime)>>16&0xFF);
	}
	
	SetWindowText(DateItemWnd[12],info);
	//zsliu change end ... ...
		
	sprintf(info,"%.2d",gDate->tm_mday);
	SetWindowText(DateItemWnd[2],info);
		
        sprintf(info,"%.2d",gDate->tm_hour);
        SetWindowText(DateItemWnd[3],info);
        if (gOptions.DaylightSavingTimeOn)
                sprintf(info,"%.2d",(gOptions.DaylightSavingTime)>>8&0xFF);
        SetWindowText(DateItemWnd[9],info);
        if (gOptions.DaylightSavingTimeOn)
                sprintf(info,"%.2d",(gOptions.StandardTime)>>8&0xFF);
        SetWindowText(DateItemWnd[13],info);
        sprintf(info,"%.2d",gDate->tm_min);
        SetWindowText(DateItemWnd[4],info);
        if (gOptions.DaylightSavingTimeOn)
                sprintf(info,"%.2d",(gOptions.DaylightSavingTime)&0xFF);
        SetWindowText(DateItemWnd[10],info);
        if (gOptions.DaylightSavingTimeOn)
                sprintf(info,"%.2d",(gOptions.StandardTime)&0xFF);
        SetWindowText(DateItemWnd[14],info);
        sprintf(info,"%.2d",gDate->tm_sec);
        SetWindowText(DateItemWnd[5],info);

	SendMessage(DateItemWnd[0],EM_LIMITTEXT,4,0);
	SendMessage(DateItemWnd[1],EM_LIMITTEXT,2,0);
	SendMessage(DateItemWnd[2],EM_LIMITTEXT,2,0);
	SendMessage(DateItemWnd[3],EM_LIMITTEXT,2,0);
	SendMessage(DateItemWnd[4],EM_LIMITTEXT,2,0);
	SendMessage(DateItemWnd[5],EM_LIMITTEXT,2,0);
        SendMessage(DateItemWnd[7],EM_LIMITTEXT,2,0);
        SendMessage(DateItemWnd[8],EM_LIMITTEXT,2,0);
        SendMessage(DateItemWnd[9],EM_LIMITTEXT,2,0);
        SendMessage(DateItemWnd[10],EM_LIMITTEXT,2,0);
        SendMessage(DateItemWnd[11],EM_LIMITTEXT,2,0);
        SendMessage(DateItemWnd[12],EM_LIMITTEXT,2,0);
        SendMessage(DateItemWnd[13],EM_LIMITTEXT,2,0);
        SendMessage(DateItemWnd[14],EM_LIMITTEXT,2,0);

        dlsel=gOptions.DaylightSavingTimeOn;
	SendMessage(DateItemWnd[6],CB_SETCURSEL,gOptions.DaylightSavingTimeOn,0);
        if (!gOptions.DaylightSavingTimeOn)
                disabledlbox(0);
        SetFocus(DateItemWnd[0]);
	DateItem=0;
        return 1;
	case MSG_ERASEBKGND:
        {
		HDC hdc = (HDC)wParam;
		const RECT* clip = (const RECT*) lParam;
		BOOL fGetDC = FALSE;
		RECT rcTemp;

		if (hdc == 0) {
			hdc = GetClientDC (hDlg);
			fGetDC = TRUE;
		}

		if (clip) {
			rcTemp = *clip;
			ScreenToClient (hDlg, &rcTemp.left, &rcTemp.top);
			ScreenToClient (hDlg, &rcTemp.right, &rcTemp.bottom);
			IncludeClipRect (hdc, &rcTemp);
		}

		FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

		if (fGetDC)
			ReleaseDC (hdc);
		return 0;
	}

	case MSG_PAINT:
                        hdc=BeginPaint(hDlg);
			//FillBoxWithBitmap (hdc, 220, 60, 0, 0, &dtinfo);
                        EndPaint(hDlg,hdc);
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
					return 0;
			}
			if(gOptions.KeyPadBeep)
                                ExKeyBeep();

			if ((LOWORD(wParam)==SCANCODE_TAB))
                                return 0;
			if ((LOWORD(wParam)>=SCANCODE_1)&&(LOWORD(wParam)<=SCANCODE_0))
				dtFlage++;
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
                        {

				if(DateItem>=0&&DateItem<=14)
                                {
                                	if( (DateItem != 6) && !CheckDateItem(DateItem))
                                                return 1;
                                }
				if (dlsel==0)
				{
					if (DateItem==6)
						DateItem=14;
				}	
                                if(DateItem<16)
                                        DateItem++;
                                else
                                        DateItem=0;
                                SetFocus(DateItemWnd[DateItem]);
				return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
                        {
				if(DateItem>=0&&DateItem<=14)
                                {
                                	if( (DateItem != 6) && !CheckDateItem(DateItem))
                                                return 1;
                                }
				if (dlsel==0)
				{
					if (DateItem==15)
						DateItem=7;
				}	
                                if(DateItem>0)
                                {
                                        DateItem--;
                                }
                                else
                                        DateItem=16;
                                 SetFocus(DateItemWnd[DateItem]);
				return 1;
                        }
			else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
                        {
                                if (DateItem==6)
                                {
	                                dlsel= SendMessage(DateItemWnd[6],CB_GETCURSEL,0,0);
                                        if(dlsel==1)
                                        {
                                                SendMessage(DateItemWnd[6],CB_SETCURSEL,0,0);
                                                disabledlbox(0);
						dlsel=0;
                                                if (gOptions.DaylightSavingTimeOn != 0)
                                                        dtFlage++;
                                        }
                                        else
                                        {
                                                SendDlgItemMessage(hDlg,IDC_DL,CB_SPIN,1,0);
                                                disabledlbox(1);
						dlsel=1;
                                                if (gOptions.DaylightSavingTimeOn != 1)
                                                        dtFlage++;
                                        }
                                        return 1;
                                }
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
							|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
                        {
                                if (DateItem==6)
                                {
	                                dlsel= SendMessage(DateItemWnd[6],CB_GETCURSEL,0,0);
                                        if(dlsel==0)
                                        {
                                                SendMessage(DateItemWnd[6],CB_SETCURSEL,1,0);
                                                disabledlbox(1);
						dlsel=1;
                                                if (gOptions.DaylightSavingTimeOn != 1)
                                                        dtFlage++;

                                        }
                                        else
                                        {
                                                SendDlgItemMessage(hDlg,IDC_DL,CB_SPIN,0,0);
                                                disabledlbox(0);
						dlsel=0;
                                                if (gOptions.DaylightSavingTimeOn != 0)
                                                        dtFlage++;
                                        }
                                        return 1;
                                }
                        }
			else
                        if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_MENU)||(LOWORD(wParam)==SCANCODE_F10))
                        {
				if(DateItem<=15)
				{
                                	if(DateOK(hDlg,DateItemWnd,6) && !ismenutimeout)
					{
						GetNoNcState();         //强制开关门
                                        	SendMessage(hDlg,MSG_CLOSE,0,0);
					}
				}
				else
				if(DateItem==16)
					SendMessage(hDlg,MSG_CLOSE,0,0);
                                return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_ESCAPE))
                        {
                                SendMessage(hDlg,MSG_CLOSE,0,0);
                                return 1;
                        }
			break;
	case MSG_CLOSE:
		//UnloadBitmap(&datebk);
		EndDialog (hDlg, IDCANCEL);
		return 0;
	}
    
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_DATE(HWND hWnd)
{
    int posX1,posX2,posX3,posX4;

    if (fromRight==1)	//add by jazzy 2008.07.14
    {
    	posX1=220;
		posX2=170;
		posX3=123;
		posX4=65;

	DateTimeCtrl[0].x=posX1+30;
	DateTimeCtrl[1].x=posX2+15;
	DateTimeCtrl[2].x=posX2;
	DateTimeCtrl[3].x=posX3;
	DateTimeCtrl[4].x=posX3-17;
	DateTimeCtrl[5].x=posX4-10;
	DateTimeCtrl[6].x=posX4-30;
	DateTimeCtrl[7].x=posX1+30;
	DateTimeCtrl[8].x=posX2+35;
	DateTimeCtrl[9].x=posX2;
	DateTimeCtrl[10].x=posX3+10;
	DateTimeCtrl[11].x=posX3-10;
	DateTimeCtrl[12].x=posX4;
	DateTimeCtrl[13].x=posX4-30;
	DateTimeCtrl[14].x=posX1+20;
	DateTimeCtrl[15].x=posX3;
	DateTimeCtrl[16].x=posX1;
	DateTimeCtrl[17].x=posX2;
	DateTimeCtrl[18].x=posX2-15;
	DateTimeCtrl[19].x=posX3;
	DateTimeCtrl[20].x=posX3-15;
	DateTimeCtrl[21].x=posX4;
	DateTimeCtrl[22].x=posX4-15;
	DateTimeCtrl[23].x=posX4-50;
	DateTimeCtrl[24].x=posX4-65;
	DateTimeCtrl[25].x=posX1;
	DateTimeCtrl[26].x=posX2;
	DateTimeCtrl[27].x=posX2-15;
	DateTimeCtrl[28].x=posX3;
	DateTimeCtrl[29].x=posX3-15;
	DateTimeCtrl[30].x=posX4;
	DateTimeCtrl[31].x=posX4-15;
	DateTimeCtrl[32].x=posX4-50;
	DateTimeCtrl[33].x=posX4-65;
    }

    DateTimeCtrl[34].x=225+gOptions.ControlOffset;

    DateTimeDlgBox.w = gOptions.LCDWidth;
    DateTimeDlgBox.h = gOptions.LCDHeight;

    DateTimeDlgBox.caption = LoadStrByID(HIT_DATE);
    DateTimeDlgBox.controls = DateTimeCtrl;
    DialogBoxIndirectParam (&DateTimeDlgBox, hWnd, DateTimeDialogBoxProc, 0L);
}

//zsliu add 

int isHejiraLeap(int nYear)
{
	int nLeap = 0;
	if(((nYear - 1379) % 4) == 0)   // 1379 is leap year
		nLeap = 1;
	else
		nLeap = 0;

	return nLeap;
}


int checkHejiraDate(int *year, int *mon, int *day, int *err)
{
	if((*year > 1408) || (*year < 1380))
		(*err)++;
	else
	{
		if(*mon<=0||*mon>=13)
			(*err)++;
		else if(((*mon >= 1) && (*mon <= 6)) && (((*day <= 0) || (*day >= 32))))    //1~6  31 days
			(*err)++;
		else if(((*mon >= 7) && (*mon <= 11)) && (((*day <= 0) || (*day >= 31))))   //7~11  30 days
			(*err)++;
		else if(*mon == 12)
		{
					//12  Leap is 30 days a month, else 29 days a month
			int nLeap = isHejiraLeap(*year);

			if((nLeap == 1) && ((*day <= 0) || (*day >= 31)))
				(*err)++;
			else if((nLeap == 0) && ((*day <= 0) || (*day >= 30)))
				(*err)++;
		}


		time_t local_time ;
		struct tm* time0;

		local_time = time(0);
		time0 = localtime(&local_time);

		time0->tm_year = *year - 1900;
		time0->tm_mon = *mon - 1;
		time0->tm_mday = *day;
	
		HejiraToChrisTTime(time0);      //伊朗日历转公历
		*year = time0->tm_year + 1900;
		*mon = time0->tm_mon + 1;
		*day = time0->tm_mday;
	}
	return 1;
}

//zsliu add end ... ...

#endif
