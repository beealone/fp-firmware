/*
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0
* 修改记录:
* 编译环境:mipsel-gcc
*/


#ifndef __CWX_GUI_TFTTEST

#define __CWX_GUI_TFTTEST

#include "arca.h"
#include "ssrcommon.h"

static DLGTEMPLATE TftTestDlgBox =
{
    WS_VISIBLE | WS_CAPTION,
    WS_EX_NONE,
    1, 1, 319, 239,
    "",
    0, 0,
    1, NULL,
    0
};

static CTRLDATA TftTestCtrl [] =
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

extern int TestItem,TestFlage;


void TftShowTestInfo(HDC hdc,int item)
{
	int tmpvalue = 0;
	tmpvalue =SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,0x00000000);
	if(item==0)
	{
		SetBrushColor(hdc,RGB2Pixel(hdc,0xff,0x00,0x00));
                FillBox(hdc,1,1,gOptions.LCDWidth-1,30);
                SetBrushColor(hdc,RGB2Pixel(hdc,0x00,0xff,0x00));
                FillBox(hdc,1,30,gOptions.LCDWidth-1,60);
                SetBrushColor(hdc,RGB2Pixel(hdc,0x00,0x00,0xff));
                FillBox(hdc,1,60,gOptions.LCDWidth-1,90);
                SetBrushColor(hdc,RGB2Pixel(hdc,0xff,0xff,0x00));
                FillBox(hdc,1,90,gOptions.LCDWidth-1,120);
                SetBrushColor(hdc,RGB2Pixel(hdc,0x00,0xff,0xff));
                FillBox(hdc,1,120,gOptions.LCDWidth-1,150);
                SetBrushColor(hdc,RGB2Pixel(hdc,0xff,0x00,0xff));
                FillBox(hdc,1,150,gOptions.LCDWidth-1,180);
                SetBrushColor(hdc,RGB2Pixel(hdc,0xff,0xff,0xff));
                FillBox(hdc,1,180,gOptions.LCDWidth-1,210);
	}
	else if(item==1)
	{
		SetBrushColor(hdc,RGB2Pixel(hdc,0xff,0xff,0xff));
                FillBox(hdc,1,1,gOptions.LCDWidth-1,gOptions.LCDHeight-1);
	}
	else if(item==2)
	{
		SetTextColor(hdc,RGB2Pixel(hdc,0xff,0xff,0xff));
		SetBrushColor(hdc,RGB2Pixel(hdc,0x00,0x00,0x00));
                FillBox(hdc,1,1,gOptions.LCDWidth-1, gOptions.LCDHeight-1);
	}
        TabbedTextOut(hdc,5,190,LoadStrByID(HIT_AUTOTESTINFO));
}

static RECT gTestScreen_rc={0,0,320,240};

static int TftTestDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message) {
		case MSG_INITDIALOG:
			TestFlage=0;
			TestItem=0;
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
                        if ((LOWORD(wParam)==SCANCODE_ENTER)  || (LOWORD(wParam)==SCANCODE_F10))
                        {
                         	if(TestItem<2) ++TestItem;
				else
				{
					TestItem=0;
					EndDialog (hDlg, IDCANCEL);
				}

                		gTestScreen_rc.right = gOptions.LCDWidth;
                		gTestScreen_rc.bottom = gOptions.LCDHeight;
				InvalidateRect(hDlg,&gTestScreen_rc,FALSE);
				return 1;
                        }
			else
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				TestFlage=-1;
				EndDialog (hDlg, IDCANCEL);
				return 1;
			}
                        break;
		case MSG_PAINT:
                	hdc=BeginPaint(hDlg);
			TftShowTestInfo(hdc,TestItem);
        	        EndPaint(hDlg,hdc);
	                return 0;

		case MSG_CLOSE:
			EndDialog (hDlg, IDCANCEL);
			break;

		case MSG_COMMAND:
		        break;
	}

	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_AUTO_TFTTEST(HWND hWnd)
{
    TftTestDlgBox.w = gOptions.LCDWidth-1;
    TftTestDlgBox.h = gOptions.LCDHeight-1;

    TftTestCtrl[0].w=gOptions.LCDWidth-1;
    TftTestCtrl[0].h=gOptions.LCDHeight-1;

    TftTestDlgBox.caption = LoadStrByID(HIT_AUTO1);
    TftTestDlgBox.controls = TftTestCtrl;
    DialogBoxIndirectParam (&TftTestDlgBox, hWnd, TftTestDialogBoxProc, 0L);
}


#endif
