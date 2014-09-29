/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录:
* 编译环境:mipsel-gcc
*/


#ifndef __CWX_GUI_RTCTEST

#define __CWX_GUI_RTCTEST

#include "ssrcommon.h"
#include "truncdate.h"

static unsigned int rtcsec=0,rtcmm=0;
static PLOGFONT rtclogfont;
static RECT Rtc_rc={20,20,180,80};

static int Flage=0;

static DLGTEMPLATE RtcTestDlgBox =
{
    WS_VISIBLE | WS_CAPTION,
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    1, NULL,
    0
};

static CTRLDATA RtcTestCtrl [] =
{ 
    {
        CTRL_STATIC,
        WS_TABSTOP | WS_VISIBLE, 
        1, 1, 319, 239,
        IDC_TESTINFO, 
        "",
        0
    },
};

void RtcShowTestInfo(HDC hdc,int item)
{
	char info[256];
	int tmpvalue = 0;
        PLOGFONT oldlf;

	time_t timer=time(NULL);
        struct tm *tb;
        char dt[256];//,*str;
        tb=localtime(&timer);

	//zsliu add
        if(gOptions.isUseHejiraCalendar)
        {
                ChrisToHejiraTTime(tb); //公历转伊朗日历
                //tb->tm_mon = tb->tm_mon - 1;
        }
        //zsliu add end ... ...

        sprintf(dt,"%.4d-%.2d-%.2d %.2d:%.2d",tb->tm_year+1900,tb->tm_mon+1,tb->tm_mday,tb->tm_hour,tb->tm_min);

	SetBrushColor(hdc,GetWindowElementColor(BKC_DIALOG));	
	SetTextColor(hdc,0x00990000);
        tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);

	
	FillBox(hdc,20,20,180,80);
	if (gfont1==NULL)
		oldlf=SelectFont(hdc,rtclogfont);
	else	oldlf=SelectFont(hdc,gfont1);
	TabbedTextOut(hdc,20,20,dt);

	sprintf(info,"%.2d(s) : %.2d(ms)",rtcsec,rtcmm);
        TabbedTextOut(hdc,20,60,info);
	SelectFont(hdc,oldlf);
	//str=HIT_AUTOTESTINFO;
	//printf("leng=%d\n",strlen(str));
	TabbedTextOut(hdc,5,190,LoadStrByID(HIT_AUTOTESTINFO));
}

static int RtcTestDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message) 
	{
	case MSG_INITDIALOG:
		if (gfont1==NULL)
		{
			rtclogfont=CreateLogFont (NULL, "fixed", "ISO8859-1",FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, 
				FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,30, 0);
		}

		TestItem=0;
		TestFlage=0;
        	return 1;
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
		if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
		{
			if(Flage==0)
			{
				rtcsec=0;
                               	rtcmm=0;
#ifdef ZEM600
				if (gOptions.ZEM800Platform) {
					printf("set timer for zem800\n");
					/*dsl 2011.4.28, let the timer slowly run, the reason is that timer precision is incorrect*/
					SetTimer(hDlg,0x80180,5*2);
				} else {
					SetTimer(hDlg,0x80180,5);
				}
#else
                               	SetTimer(hDlg,0x80180,10);
#endif
				Flage=1;
			}
			else if(Flage==1)
			{
				Flage=0;
                               	KillTimer(hDlg,0x80180);
			}
			return 1;	
		}
		else if ((LOWORD(wParam)==SCANCODE_ESCAPE))
		{
			TestFlage=-1;
			KillTimer(hDlg,0x80180);
			if (rtclogfont!=NULL)
			{
				DestroyLogFont(rtclogfont);
			}
			EndDialog (hDlg, IDCANCEL);
			return 1;
		}
		break;
	case MSG_TIMER:
		if(wParam==0x80180)
		{
			if(rtcmm>=10)
			{
				rtcmm=0;
				if(rtcsec>=60)
				{
					rtcsec=0;
				}
				else
				{	
					++rtcsec;
				}
			}
			else
			{
				++rtcmm;
			}
			InvalidateRect(hDlg,&Rtc_rc,FALSE);
		}
		break;
	case MSG_PAINT:
                hdc=BeginPaint(hDlg);
		RtcShowTestInfo(hdc,TestItem);
                EndPaint(hDlg,hdc);
                return 0;
	case MSG_CLOSE:
		if(rtclogfont!=NULL)
			DestroyLogFont(rtclogfont);
		EndDialog (hDlg, IDCANCEL);
		break;
        
	case MSG_COMMAND:
		break;
	}
    
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_AUTO_RTCTEST(HWND hWnd)
{
	/*dsl 2011.4.28. the second and millisecond reset*/
	rtcsec=0;
	rtcmm=0;
	Flage = 0; //add by zxz 2012-09-24
	RtcTestDlgBox.w = gOptions.LCDWidth;
	RtcTestDlgBox.h = gOptions.LCDHeight;

	RtcTestDlgBox.caption = LoadStrByID(HIT_AUTO5);
	RtcTestDlgBox.controls = RtcTestCtrl;
	DialogBoxIndirectParam (&RtcTestDlgBox, hWnd, RtcTestDialogBoxProc, 0L);
}


#endif
