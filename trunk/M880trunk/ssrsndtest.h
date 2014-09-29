/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0
* 修改记录:
* 编译环境:mipsel-gcc
*/


#ifndef __CWX_GUI_SNDTEST

#define __CWX_GUI_SNDTEST

#include "ssrcommon.h"

static int voicenum;
static RECT Snd_rc={20,40,300,60};
static PLOGFONT sndlogfont;

static DLGTEMPLATE SndTestDlgBox =
{
    WS_VISIBLE | WS_CAPTION,
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    1, NULL,
    0
};

static CTRLDATA SndTestCtrl [] =
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

void SndShowTestInfo(HDC hdc,int item)
{
	int tmpvalue = 0;
	char buf[256];
	int showitem;
	PLOGFONT oldlf;

	SetBrushColor(hdc,GetWindowElementColor(BKC_DIALOG));

	SetTextColor(hdc,0x00990000);
        tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	if(item>=0)
	{
		FillBox(hdc,20,40,230,40);
		if (sndlogfont!=NULL)
		{
			oldlf=SelectFont(hdc,sndlogfont);
		}
		else
		{
			oldlf=gfont1;
		}

		showitem = item;
		if (item > 7)
		{
			showitem = item - 1;
		}
		sprintf(buf, "%s (%c_%d.wav)....", LoadStrByID(HIT_SNDINFO), gOptions.Language, showitem);
//        	sprintf(buf, "%s (%c_%d.wav)....",LoadStrByID(HIT_SNDINFO),gOptions.Language,item);

		TabbedTextOut(hdc,20,40,buf);
//printf("%s\n",__FUNCTION__);
        	ExPlayVoice(item);
		SelectFont(hdc,oldlf);
	}
	else
		TabbedTextOut(hdc,20,20,LoadStrByID(HIT_AUTO2));
	TabbedTextOut(hdc,5,190,LoadStrByID(HIT_AUTOTESTINFO));
}

static int SndTestDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//char info[256];
	static char keyupFlag=0;
	switch (message) {
		case MSG_INITDIALOG:
			if (gOptions.RFCardFunOn)
                		voicenum=10;
			else
	                	voicenum=10;

			if (gfont1==NULL)
				sndlogfont=CreateLogFont (NULL, "fixed", "GB2312",FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,16, 0);

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
			if(3 == gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
                        if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
                        {
				if(TestItem<voicenum)
					++TestItem;
				else
				{
					TestItem=0;
					if (sndlogfont!=NULL)
						DestroyLogFont(sndlogfont);
                                	EndDialog (hDlg, IDCANCEL);
					return 1;
				}
				if(TestItem==7) TestItem++;
				InvalidateRect(hDlg,&Snd_rc,FALSE);	
				return 1;	
                        }
			else
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				TestFlage=-1;
				if (sndlogfont!=NULL)
					DestroyLogFont(sndlogfont);
				EndDialog (hDlg, IDCANCEL);
				return 1;
			}
                        break;
		case MSG_PAINT:
                	hdc=BeginPaint(hDlg);
#if 0
		//纯射频卡机器不测试指纹语音提示音(liming)
			if(gOptions.IsOnlyRFMachine && (TestItem==4 || TestItem==8))
				++TestItem;
#endif
			SndShowTestInfo(hdc,TestItem);
        	        EndPaint(hDlg,hdc);
	                return 0;

		case MSG_CLOSE:
			if (sndlogfont!=NULL)
				DestroyLogFont(sndlogfont);
		        EndDialog (hDlg, IDCANCEL);
		        break;
        
		case MSG_COMMAND:
		        break;
	}
    
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_AUTO_SNDTEST(HWND hWnd)
{
	SndTestDlgBox.w = gOptions.LCDWidth;
	SndTestDlgBox.h = gOptions.LCDHeight;

	SndTestDlgBox.caption = LoadStrByID(HIT_AUTO2);
	SndTestDlgBox.controls = SndTestCtrl;
	DialogBoxIndirectParam (&SndTestDlgBox, hWnd, SndTestDialogBoxProc, 0L);
}


#endif
