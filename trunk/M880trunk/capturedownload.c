#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include "usb_helper.h"
#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "ssrcommon.h"

static DLGTEMPLATE CaptureDownBox =
{
	WS_BORDER | WS_CAPTION, 
	WS_EX_NONE,
	0, 0, 320, 240, 
	"",
	0, 0,
	4, NULL,
	0
};

#define IDC_DOWNALL		900
#define IDC_DOWNCAPTURE		901
#define IDC_DOWNBADCAPTURE	902     
#define IDC_DOWNDELETE		903

static CTRLDATA CaptureDataCtrl [] =
{ 
	{
        	CTRL_BUTTON,
	        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        	60, 20, 200, 25,
		IDC_DOWNALL,
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        	60, 60, 200, 25,
	        IDC_DOWNCAPTURE,
		"",
		0
	},
	{
        	CTRL_BUTTON,
	        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        	60, 100, 200, 25,
	        IDC_DOWNBADCAPTURE,
	        "",
	        0
	},
	{
	        CTRL_COMBOBOX,
        	WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
	        61, 140, 198, 25,
	        IDC_DOWNDELETE,
		"",
	        0
	}
};

extern void SSR_MENU_DATADLG(HWND hWnd, int i);
static HWND CaptureItemWnd[4];
static int CaptureDataItem;
//static BITMAP capturedatabk;
int downclearflag;
static void PicDown(HWND hWnd, int item)
{
	int ItemList[] = {8, 9, 10};
	downclearflag = SendMessage(CaptureItemWnd[3], CB_GETCURSEL, 0, 0);
	SSR_MENU_DATADLG(hWnd, ItemList[item]);
}

static int CaptureDownBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_INITDIALOG:
			//if (LoadBitmap(HDC_SCREEN,&capturedatabk, GetBmpPath("submenubg.jpg")))
	                //        return 0;

			SetWindowText(GetDlgItem(hDlg, IDC_DOWNALL), LoadStrByID(MID_DOWNALL_PIC));
			SetWindowText(GetDlgItem(hDlg,IDC_DOWNCAPTURE), LoadStrByID(MID_DOWN_ATTPIC));
			SetWindowText(GetDlgItem(hDlg,IDC_DOWNBADCAPTURE),LoadStrByID(MID_DOWN_BADPIC));
	
	        	SendDlgItemMessage(hDlg, IDC_DOWNDELETE,CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_NOCLEAR_DOWN));
		        SendDlgItemMessage(hDlg, IDC_DOWNDELETE,CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_CLEAR_DOWN));
	
			CaptureItemWnd[0] = GetDlgItem(hDlg, IDC_DOWNALL);
	                CaptureItemWnd[1] = GetDlgItem(hDlg, IDC_DOWNCAPTURE);
		        CaptureItemWnd[2] = GetDlgItem(hDlg, IDC_DOWNBADCAPTURE);
			CaptureItemWnd[3] = GetDlgItem(hDlg, IDC_DOWNDELETE);
	
		        SendMessage(CaptureItemWnd[3], CB_SETCURSEL, 0, 0);
       			CaptureDataItem=0;
			SetFocusChild(CaptureItemWnd[CaptureDataItem]);

	        	break;

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
					break;
			}
			if(gOptions.KeyPadBeep)
                                ExKeyBeep();

                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
                        {
				if (++CaptureDataItem > 3)
					CaptureDataItem=0;
                                SetFocus(CaptureItemWnd[CaptureDataItem]);
                                return 0;
                        }
                        else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
                        {
				if (--CaptureDataItem < 0)
					CaptureDataItem=3;
                                SetFocus(CaptureItemWnd[CaptureDataItem]);
                                return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				if (CaptureDataItem == 3)
				{
					int sel;
					sel=SendMessage(CaptureItemWnd[3],CB_GETCURSEL,0,0);
					if (sel==0)
						sel=SendMessage(CaptureItemWnd[3],CB_SETCURSEL,1,0);
					else
						sel=SendMessage(CaptureItemWnd[3],CB_SETCURSEL,0,0);
				}
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_MENU))
                        {
				if (CaptureDataItem < 3)
				{
					PicDown(hDlg, CaptureDataItem);
				}
                                return 0;
                        }
                        else if (LOWORD(wParam)==SCANCODE_ESCAPE)
                        {
				PostMessage(hDlg, MSG_CLOSE, 0, 0);
                                return 0;
                        }
                        break;

		case MSG_CLOSE:
			//UnloadBitmap(&capturedatabk);
		        EndDialog (hDlg, IDCANCEL);
       			return 0;
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int CaptureDownLoadWindow(HWND hWnd)
{
	//zsliu add
	CaptureDataCtrl[0].x=60+gOptions.GridWidth*2;
	CaptureDataCtrl[1].x=60+gOptions.GridWidth*2;
	CaptureDataCtrl[2].x=60+gOptions.GridWidth*2;
	CaptureDataCtrl[3].x=60+gOptions.GridWidth*2;

	CaptureDownBox.w = gOptions.LCDWidth;
	CaptureDownBox.h = gOptions.LCDHeight;

    CaptureDownBox.caption = LoadStrByID(MID_DOWN_ATTPIC);
    CaptureDownBox.controls = CaptureDataCtrl;
    DialogBoxIndirectParam (&CaptureDownBox, hWnd, CaptureDownBoxProc, 0L);
    return 0;
}
