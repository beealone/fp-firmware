/* 
 * SSR 2.0 Self Service Record 主入口头文件
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0   
 * 修改记录:
 * 编译环境:mipsel-gcc
 */
#include <minigui/common.h>
#include "ssrcommon.h"
#include "options.h"
#include "flashdb.h"
#include "usb_helper.h"
#include "exfun.h"
#include "ssrsystem4.h"
#include "ssrpub.h"
#include "tftmsgbox.h"

extern int TestItem,TestFlage;

//BITMAP updatebk;
BITMAP upbmp;

static DLGTEMPLATE UpdateDlgBox =
{
	WS_BORDER, WS_EX_NONE,
	20, 70, 279, 90, 
	"",
	0, 0, 1, NULL, 0
};

static CTRLDATA UpdateCtrl [] =
{ 
	{
		CTRL_STATIC,
		WS_TABSTOP | WS_VISIBLE, 
		1, 1, 259, 60,
		IDC_STATIC, 
		"",
		0
	},
};

extern int UpdateOptions(char *filename);
int UpdateShowInfo(HWND hWnd, HDC hdc)
{
	int mount,err=0;
	int sing=0, tmpvalue = 0;
	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,PIXEL_lightwhite);

	mount=DoMountUdisk();
	if (mount==0)
	{
		FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
		FillBoxWithBitmap(hdc,5,5,35,35,&upbmp);
		TabbedTextOut(hdc,10,45,LoadStrByID(HIT_UINFO9));

		if(UpdateFirmware("emfw.cfg"))
		{
			err=1;
		}
		else
		{
			err=0;
		}

		sing=UpdateOptions("option2.cfg");


		DoUmountUdisk();
		if((err)&&(sing))
		{
			MessageBox1(hWnd,LoadStrByID(MID_UPDATE_OPTFIR),LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION); // all success
		}
		else if(err)
		{
			MessageBox1(hWnd,LoadStrByID(HIT_UINFO13),LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION); // main success
		}
		else if (sing)
		{
			MessageBox1(hWnd,LoadStrByID(MID_UPDATE_OPT),LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION); // options success
		}
		else
		{
			char buf[256] = {0};
			sprintf(buf, "%s\n[%d]", LoadStrByID(MID_UPFAILD_OPTFW), GetUpdatingFirmeareErrorNo());

			MessageBox1(hWnd,buf,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION); // all failed
		}

	}
	else
	{
		MessageBox1(hWnd,LoadStrByID(HIT_UINFO7),LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
	}

	if(!ismenutimeout) 
		PostMessage(hWnd,MSG_CLOSE,0,0);
	return 0;
}

static int UpdateDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message) 
	{
		case MSG_INITDIALOG:
			//LoadBitmap(HDC_SCREEN,&updatebk,GetBmpPath("submenubg.jpg"));
			LoadBitmap(HDC_SCREEN,&upbmp,GetBmpPath("info.gif"));
			TestItem=-1;
			break;

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
				ExBeep(1);

			if((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_ESCAPE) ||(LOWORD(wParam)==SCANCODE_F10))
				SendMessage(hDlg,MSG_CLOSE,0,0);

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

				if (clip)
				{
					rcTemp = *clip;
					ScreenToClient (hDlg, &rcTemp.left, &rcTemp.top);
					ScreenToClient (hDlg, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect (hdc, &rcTemp);
				}

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());

				if (fGetDC)
					ReleaseDC (hdc);
				return 0;
			}
		case MSG_PAINT:
			hdc=BeginPaint(hDlg);
			UpdateShowInfo(hDlg,hdc);
			EndPaint(hDlg,hdc);
			return 0;

		case MSG_CLOSE:
			//UnloadBitmap(&updatebk);
			UnloadBitmap(&upbmp);
			EndDialog(hDlg, IDCANCEL);
			return 0;

	}

	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_UPDATE(HWND hWnd)
{
	UpdateDlgBox.x = 0;
	UpdateDlgBox.y = 0;
	UpdateDlgBox.w = gOptions.LCDWidth;
	UpdateDlgBox.h = gOptions.LCDHeight;

	UpdateDlgBox.controls = UpdateCtrl;
	DialogBoxIndirectParam (&UpdateDlgBox, hWnd, UpdateDialogBoxProc, 0L);
}
