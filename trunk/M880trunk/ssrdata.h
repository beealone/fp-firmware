/*
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0
* 修改记录:
* 编译环境:mipsel-gcc
*/

#ifndef __CWX_GUI_DATA
#define __CWX_GUI_DATA

#include "ssrcommon.h"
#include "ssrdatadlg.h"
//#include "ssrpub.h"

static DLGTEMPLATE DataDlgBox = {WS_BORDER | WS_CAPTION, WS_EX_NONE, 0, 0, 320, 240, "", 0, 0, 3, NULL, 0};

#define IDC_DOWNLOAD	500
#define IDC_UPLOAD	501
#define IDC_EXIT	502

static CTRLDATA DataCtrl [] =
{ 
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 90, 40, 135, 25, IDC_DOWNLOAD, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 90, 80, 135, 25, IDC_UPLOAD, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 90, 120, 135, 25, IDC_EXIT, "", 0},
};

HWND USBWnd[3];
int USBItem;
//BITMAP USBbkg;
static int DataDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_INITDIALOG:
			//if (LoadBitmap(HDC_SCREEN, &USBbkg, GetBmpPath("submenubg.jpg"))){
                        //	return 0;
			//}
		
			SetWindowText(GetDlgItem(hDlg, IDC_DOWNLOAD), LoadStrByID(MID_USB_DOWNLOAD));
			SetWindowText(GetDlgItem(hDlg, IDC_UPLOAD), LoadStrByID(MID_USB_UPLOAD));
			SetWindowText(GetDlgItem(hDlg, IDC_EXIT), LoadStrByID(MID_EXIT));
	
			USBWnd[0] = GetDlgItem (hDlg, IDC_DOWNLOAD);
        	        USBWnd[1] = GetDlgItem (hDlg, IDC_UPLOAD);
			USBWnd[2] = GetDlgItem (hDlg, IDC_EXIT);

	                USBItem=0;
			if (TEST_ONLY_ENROLL_PRIVILLEGE) {
				ShowWindow(USBWnd[0], SW_HIDE);
				USBItem = 1;
			}
			SetFocusChild(USBWnd[USBItem]);
#ifdef _TTS_
			if(gOptions.TTS_KEY) {
				char buffer[32];
				GetWindowText(USBWnd[USBItem],buffer,32);
				TTS_Say(buffer);
			}
#endif			
        		break;

	        case MSG_ERASEBKGND:
        	{
			HDC hdc = (HDC)wParam;
			const RECT* clip = (const RECT*) lParam;
			BOOL fGetDC = FALSE;
			RECT rcTemp;

			if (hdc == 0) 
			{
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

			if (fGetDC) {
		                ReleaseDC (hdc);
			}
			return 0;
		}

		case MSG_PAINT:
	                hdc=BeginPaint(hDlg);
        	        EndPaint(hDlg,hdc);
	                return 0;
		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout) {
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				TTS_PlayWav(GetWavPath("item.wav"));
			}
#endif			
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag) {
					keyupFlag=0;
				} else {
					break;
				}
			}
			if(gOptions.KeyPadBeep) {
                                ExKeyBeep();
			}

                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
                        {
				int idx=0;
				if (TEST_ONLY_ENROLL_PRIVILLEGE) {
					idx = 1;
				}

				if (++USBItem > 2) {
					USBItem = idx;
				}
                                SetFocus(USBWnd[USBItem]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];
					GetWindowText(USBWnd[USBItem],buffer,32);
					TTS_Say(buffer);
				}
#endif  
                                return 0;
                        }
                        else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
                        {
				int idx = 0;
				if (TEST_ONLY_ENROLL_PRIVILLEGE) {
					idx = 1;
				}
                                if(--USBItem < idx) {
					USBItem = 2;
				}
                                SetFocus(USBWnd[USBItem]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];
					GetWindowText(USBWnd[USBItem],buffer,32);
					TTS_Say(buffer);
				}
#endif
                                return 0;
                        }
			else if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
                        {
				if (USBItem==0) {
					SSR_DATA_DOWNLOAD(hDlg);
				} else if (USBItem == 1) {
					SSR_DATA_UPLOAD(hDlg);
				} else {
					PostMessage(hDlg, MSG_CLOSE, 0, 0);
				}
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_ESCAPE))
                        {
				PostMessage(hDlg, MSG_CLOSE, 0, 0);
                                return 0;
                        }
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT){
				return 0;
			}
                        break;

		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY) {
				TTS_Stop();
			}
#endif			
			//UnloadBitmap(&USBbkg);
		        EndDialog(hDlg, IDCANCEL);
			return 0;
	}
    
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_DATA(HWND hWnd)
{
	DataCtrl[0].x=90+gOptions.GridWidth*2;
	DataCtrl[1].x=90+gOptions.GridWidth*2;
	DataCtrl[2].x=90+gOptions.GridWidth*2;

	DataDlgBox.w = gOptions.LCDWidth;
	DataDlgBox.h = gOptions.LCDHeight;

	DataDlgBox.caption = LoadStrByID(HIT_UDATA);
	DataDlgBox.controls = DataCtrl;
	DialogBoxIndirectParam (&DataDlgBox, hWnd, DataDialogBoxProc, 0L);
}


#endif
