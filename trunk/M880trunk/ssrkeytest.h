/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录:
* 编译环境:mipsel-gcc
*/


#ifndef __CWX_GUI_KEYTEST

#define __CWX_GUI_KEYTEST

#include "ssrcommon.h"
#include "ssrpub.h"

PLOGFONT autotestfont;

static DLGTEMPLATE KeyTestDlgBox =
{
    WS_VISIBLE | WS_CAPTION,
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    1, NULL,
    0
};

static CTRLDATA KeyTestCtrl [] =
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

static BITMAP keybmp_bkgnd0,keybmp_bkgnd1;


int keycount[5]={27,30,30,14,27};
#define KEYTYPE gOptions.TFTKeyLayout
#define MAXKEYCOUNT keycount[KEYTYPE]
extern int TestItem,TestFlage;

struct _tag_keybtn
{
	int x,y;
	char name[8];
}
keybtn[5][30]=
{
	{
		{10,10,"F1"},{50,10,"F2"},{90,10,"F3"},{130,10,"F4"},
		{10,45,"F5"},{50,45,"F6"},{90,45,"F7"},{130,45,"F8"},
		{10,80,"1"},{50,80,"2"},{90,80,"3"},
		{10,115,"4"},{50,115,"5"},{90,115,"6"},
		{10,150,"7"},{50,150,"8"},{90,150,"9"},
		{130,115,"Menu"},{130,150,"<-"},
		{10,185,"*"},{50,185,"0"},{90,185,"#"},{130,185,"PWR"},
		{190,45,"Left"},{225,10,"UP"},{260,45,"Right"},{225,80,"Down"}
	},
	{
	        {2,10,"F1"},{42,10,"F2"},{82,10,"F3"},{122,10,"F4"},
	        {2,45,"F5"},{42,45,"F6"},{82,45,"F7"},{122,45,"F8"},
	        {2,80,"1"},{42,80,"2"},{82,80,"3"},
	        {2,115,"4"},{42,115,"5"},{82,115,"6"},
	        {2,150,"7"},{42,150,"8"},{82,150,"9"},
	        {122,115,"Menu"},{282,10,"<-"},
		{122,150,"PgUp"},{42,185,"0"},{122,185,"PgDn"}, {2,185,"Ring"},
	        {162,45,"Left"},{202,10,"UP"},{242,45,"Right"},{202,45,"Down"},
		{242,10,"OK"},{122,80,"Space"},{82,185,"Tab"}
	},
	{
	        {10,10,"F1"},{50,10,"F2"},{90,10,"F3"},{130,10,"F4"},
	        {10,45,"F5"},{50,45,"F6"},{90,45,"F7"},{130,45,"F8"},
	        {10,80,"1"},{50,80,"2"},{90,80,"3"},
	        {10,115,"4"},{50,115,"5"},{90,115,"6"},
	        {10,150,"7"},{50,150,"8"},{90,150,"9"},
	        {130,115,"Menu"},{225,10,"<-"},
		{130,150,"PgUp"},{50,185,"0"},{130,185,"PgDn"},{10,185,"Ring"},
		{190,80,"Left"},{190,45,"UP"},{225,80,"Right"},{225,45,"Down"},
	        {190,115,"OK"},{130,80,"Space"},{90,185,"Tab"}
	},
	{	
		{20,20,"1"},{60,20,"2"},{100,20,"3"},
		{20,60,"4"},{60,60,"5"},{100,60,"6"},
		{20,100,"7"},{60,100,"8"},{100,100,"9"},
		{140,60,"UP"},{140,100,"Down"},
		{20,140,"<-"},{60,140,"0"},{100,140,"Right"}
	},
	{
	        {10,10,"F1"},{50,10,"F2"},{90,10,"F3"},{130,10,"F4"},
	        {10,45,"F5"},{50,45,"F6"},{90,45,"F7"},{130,45,"F8"},
	        {10,80,"1"},{50,80,"2"},{10,115,"3"},
	        {50,115,"4"},{10,150,"5"},{50,150,"6"},
		{10,185,"7"},{50,185,"8"},{90,80,"9"},
		{90,150,"Menu"},{90,185,"<-"},
		{90,115,"*"},{130,80,"0"},{130,115,"#"},{130,185,"PWR"},
		{170, 45, "Left"},{210,10,"UP"},{250, 45, "Right"},{210,80,"Down"}
	}
};

void KeyShowTestInfo(HDC hdc,int item)
{
	int i, tmpvalue;
	PLOGFONT old;
	char hintstr[40];

	if (gfont1==NULL)
		old=SelectFont(hdc,autotestfont);
	else 	old=SelectFont(hdc,gfont1);
        tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,0x0000ffff);
	
	for(i=0;i<MAXKEYCOUNT;i++)
	{
		FillBoxWithBitmap(hdc,keybtn[KEYTYPE][i].x,keybtn[KEYTYPE][i].y,35,30,&keybmp_bkgnd0);
		TextOut(hdc,keybtn[KEYTYPE][i].x+5,keybtn[KEYTYPE][i].y+5,keybtn[KEYTYPE][i].name);
	}
	SelectFont(hdc,old);
	SetTextColor(hdc,0x00990000);
//	TabbedTextOut(hdc,170,150,HIT_AUTOTESTINFO);
//	printf("KEYTYPE:%d\n",KEYTYPE);
	if(KEYTYPE==0 || KEYTYPE==3 || KEYTYPE==4)
		sprintf(hintstr,"OK %s",LoadStrByID(MID_KEYTESTHINT1));
	else
		sprintf(hintstr,"Enter %s",LoadStrByID(MID_KEYTESTHINT1));

	TextOut(hdc,200,180,hintstr);
	TextOut(hdc,200,200,LoadStrByID(MID_KEYTESTHINT2));
}

static int KeyTestDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	int tmpvalue = 0;
	switch (message)
	{
		case MSG_INITDIALOG:
			if (LoadBitmap(HDC_SCREEN,&keybmp_bkgnd0,GetBmpPath("btnup.bmp")))
                	        return 0;
			if (LoadBitmap(HDC_SCREEN,&keybmp_bkgnd1,GetBmpPath("btndown.bmp")))
        	                return 0;

			if (gfont1==NULL)
				autotestfont=CreateLogFont(NULL,"fixed","GB2312",FONT_WEIGHT_LIGHT,FONT_SLANT_ROMAN,FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL,FONT_UNDERLINE_NONE,FONT_STRUCKOUT_NONE,12,0);

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
			if ((LOWORD(wParam)==SCANCODE_ENTER))
                        {
				EndDialog (hDlg, IDCANCEL);
				return 1;	
                        }
			else
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				TestFlage=-1;
				EndDialog (hDlg, IDCANCEL);
				return 1;
			}
			else
			{
				if(KEYTYPE!=3)
				{
					if(LOWORD(wParam)>=SCANCODE_F1 && LOWORD(wParam)<=SCANCODE_F8)
						TestItem=0+(LOWORD(wParam)-SCANCODE_F1);
					if(LOWORD(wParam)>=SCANCODE_1&&LOWORD(wParam)<=SCANCODE_9)
						TestItem=8+(LOWORD(wParam)-SCANCODE_1);

					if(LOWORD(wParam)==SCANCODE_MENU)
						TestItem=17;
					if(LOWORD(wParam)==SCANCODE_BACKSPACE)
						TestItem=18;
					if(LOWORD(wParam)==SCANCODE_F11)
						TestItem=19;
					if(LOWORD(wParam)==SCANCODE_0)
						TestItem=20;
					if(LOWORD(wParam)==SCANCODE_F12)
						TestItem=21;
					if(LOWORD(wParam)==SCANCODE_TAB)//power
						TestItem=22;
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
						TestItem=23;
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
						TestItem=24;
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
						TestItem=25;
	                                if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
						TestItem=26;
					if(LOWORD(wParam)==SCANCODE_F10)
						TestItem=27;
					if(LOWORD(wParam)==SCANCODE_SPACE)
						TestItem=28;
					if(LOWORD(wParam)==SCANCODE_F9)
						TestItem=29;
				}
				else
				{
					if(LOWORD(wParam)>=SCANCODE_1 && LOWORD(wParam)<=SCANCODE_9)
						TestItem=0+(LOWORD(wParam)-SCANCODE_1);
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
						TestItem=9;
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
						TestItem=10;
					if(LOWORD(wParam)==SCANCODE_BACKSPACE)
						TestItem=11;
					if(LOWORD(wParam)==SCANCODE_0)
						TestItem=12;
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
						TestItem=13;
				}
				hdc=GetClientDC(hDlg);
				if (gfont1==NULL)
					SelectFont(hdc,autotestfont);
				else	SelectFont(hdc,gfont1);
				tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,0x0000ffff);
				FillBoxWithBitmap(hdc,keybtn[KEYTYPE][TestItem].x,keybtn[KEYTYPE][TestItem].y,35,30,&keybmp_bkgnd1);
				TabbedTextOut(hdc,keybtn[KEYTYPE][TestItem].x+5,keybtn[KEYTYPE][TestItem].y+5,keybtn[KEYTYPE][TestItem].name);
				Delay(3);
				FillBoxWithBitmap(hdc,keybtn[KEYTYPE][TestItem].x,keybtn[KEYTYPE][TestItem].y,35,30,&keybmp_bkgnd0);
				TabbedTextOut(hdc,keybtn[KEYTYPE][TestItem].x+5,keybtn[KEYTYPE][TestItem].y+5,keybtn[KEYTYPE][TestItem].name);
				ReleaseDC(hdc);
				return 1;
			}
                        break;
		case MSG_PAINT:
	                hdc=BeginPaint(hDlg);
			KeyShowTestInfo(hdc,TestItem);
                	EndPaint(hDlg,hdc);
	                return 0;
	
		case MSG_CLOSE:
			UnloadBitmap(&keybmp_bkgnd0);
			UnloadBitmap(&keybmp_bkgnd1);
			if (gfont1==NULL)
				DestroyLogFont(autotestfont);
		        EndDialog (hDlg, IDCANCEL);
		        return 0;
        
	}
    
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_AUTO_KEYTEST(HWND hWnd)
{
	KeyTestDlgBox.w = gOptions.LCDWidth;
	KeyTestDlgBox.h = gOptions.LCDHeight;

	KeyTestDlgBox.caption = LoadStrByID(HIT_AUTO4);
	KeyTestDlgBox.controls = KeyTestCtrl;
	DialogBoxIndirectParam (&KeyTestDlgBox, hWnd, KeyTestDialogBoxProc, 0L);
}


#endif
