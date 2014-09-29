/* 
 * SSR 2.0 Self Service Record 主入口头文件
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0   
 * 修改记录:
 * 编译环境:mipsel-gcc
 */

#include <minigui/common.h>
#include "ssrcommon.h"
#include "ssrpub.h"
#include "ssrsystem2pic.h"
#include "tftmsgbox.h"

static HWND dPicItemWnd[3];
static BITMAP ddup,ddown;
static int dpicitem=0;
static int dPicItem=0;

extern RECT gMainMenu_rc;

#define IDC_DELPIC 9090
#define IDC_DELALLPIC 9999

static DLGTEMPLATE DPicDlgDlgBox =

{
	WS_BORDER,
	WS_EX_NONE,
	1, 25, 319, 210, 
	"",
	0, 0,
	4, NULL,
	0
};

static CTRLDATA DPicDlgCtrl [] =
{ 
	{
		CTRL_STATIC,
		WS_TABSTOP | WS_VISIBLE, 
		1, 1, 70, 25,
		0x11040, 
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		5, 180, 80, 25,
		IDC_DELPIC,
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		95, 180, 80, 25,
		IDC_DELALLPIC,
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		230, 180, 80, 25,
		IDCANCEL,
		"",
		0
	},

};

void DelPic(HWND hWnd,int pp)
{
	char sys[256];
	HDC hdc;
	int i, tmpvalue = 0;//,j;
	//BITMAP upicbk;
	hdc=GetClientDC(hWnd);

	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,PIXEL_lightwhite);

	//FillBoxWithBitmap(hdc,40,60,240,40,&upicbk);

	TabbedTextOut(hdc,80,70,LoadStrByID(HIT_DPIC4));
	ReleaseDC(hdc);

	if(pp>=0) {
		sprintf(sys,"%s/%s",GetPicPath("adpic"),uPicList[pp]);
		memset(uPicList[pp],0,16);
	} else {
		for(i=0;i<uPic;i++)
		{
			sprintf(sys,"%s/%s",GetPicPath("adpic"),uPicList[i]);
			remove(sys);
		}
		return;
	}
	remove(sys);
	for(i=pp;i<uPic-1;i++)
	{
		strcpy(uPicList[i],uPicList[i+1]);
	}
	uPic--;
}

long DPicDlgShowInfo(HWND hDlg,HDC hdc,int item)
{
	char str[255];
	int tmpvalue = 0;
	BITMAP mybmp;

	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,PIXEL_lightwhite);

	TabbedTextOut(hdc,5,120,LoadStrByID(HIT_UPIC4));
	TabbedTextOut(hdc,5,145,LoadStrByID(HIT_UPIC5));

	FillBoxWithBitmap(hdc,55,120,15,15,&ddup);
	FillBoxWithBitmap(hdc,55,145,15,15,&ddown);

	sprintf(str,LoadStrByID(HIT_UPIC2),item+1,uPic);
	//TabbedTextOut(hdc,5,60,str);
	TabbedTextOut(hdc,3,60,str);

	if(item==-1){
		TabbedTextOut(hdc,140,90,LoadStrByID(HIT_DPICINFO));	
	} else	{
		TabbedTextOut(hdc,120,165,uPicList[item]);
		sprintf(str,"%s/%s",GetPicPath("adpic"),uPicList[item]);

		if(LoadBitmap(HDC_SCREEN,&mybmp,str)){
			return 0;
		}
		FillBoxWithBitmap(hdc,90+gOptions.ControlOffset,5,220,160,&mybmp);
		UnloadBitmap(&mybmp);
	}
	return 1;
}

static int DPicDlgDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message) {
		case MSG_INITDIALOG:
			//if (LoadBitmap(HDC_SCREEN,&dpicbk,GetBmpPath("submenubg.jpg"))){
			//	return 0;
			//}

			if (LoadBitmap(HDC_SCREEN,&ddup,GetBmpPath("up.gif"))){
				return 0;
			}

			if (LoadBitmap(HDC_SCREEN,&ddown,GetBmpPath("down.gif"))){
				return 0;
			}

			SetWindowText(GetDlgItem(hDlg,0x11040),LoadStrByID(HIT_UPIC1));
			SetWindowText(GetDlgItem(hDlg,IDC_DELPIC),LoadStrByID(HIT_DPIC1));
			SetWindowText(GetDlgItem(hDlg,IDC_DELALLPIC),LoadStrByID(HIT_DPIC2));
			SetWindowText(GetDlgItem(hDlg,IDCANCEL),LoadStrByID(HIT_CANCEL));

			dPicItemWnd[0] = GetDlgItem (hDlg, IDC_DELPIC);
			dPicItemWnd[1] = GetDlgItem (hDlg, IDC_DELALLPIC);
			dPicItemWnd[2] = GetDlgItem (hDlg, IDCANCEL);
			SetFocus(dPicItemWnd[0]);
			dPicItem=0;
			dpicitem=0;
			return 1;
		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout) {
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout) {
				if(1==keyupFlag){
					keyupFlag=0;
				} else {
					break;
				}
			}
			if(gOptions.KeyPadBeep){
				ExKeyBeep();
			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))	{
				if(uPic==0) {
					dpicitem=-1;
				} else if(dpicitem<uPic-1) {
					++dpicitem;
				} else {
					dpicitem=0;
				}
				InvalidateRect(hDlg,&gMainMenu_rc,TRUE);
				SetFocus(dPicItemWnd[0]);
				return 1;
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)) {
				if(uPic==0) {
					dpicitem=-1;
				} else if(dpicitem>0){
					--dpicitem;
				} else {
					dpicitem=uPic-1;
				}
				InvalidateRect(hDlg,&gMainMenu_rc,TRUE);
				SetFocus(dPicItemWnd[0]);
				return 1;
			} else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)) {	
				if(dPicItem<2){
					++dPicItem;
				} else {
					dPicItem=0;
				}
				SetFocus(dPicItemWnd[dPicItem]);
				return 1;
			} else if ((gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)) {
				if(dPicItem>0) {
					--dPicItem;
				} else {
					dPicItem=2;
				}
				SetFocus(dPicItemWnd[dPicItem]);
				return 1;
			} else if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10)) {
				SetFocus(dPicItemWnd[0]);
				if(dPicItem==0)	{
					if(dpicitem==-1) {
					       	return 0;
					}
					DelPic(hDlg,dpicitem);
					MessageBox1(hDlg,LoadStrByID(HIT_DPIC3),LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
					if(!ismenutimeout) {
						if(3 == gOptions.TFTKeyLayout){
							SendMessage(hDlg,MSG_KEYUP,0,0);
						}
						SendMessage(hDlg,MSG_KEYDOWN,SCANCODE_CURSORBLOCKDOWN,0);
					}
				}
				else if(dPicItem==1) {
					if(dpicitem==-1) {
						return 0;
					}
					DelPic(hDlg,-1);
					MessageBox1(hDlg,LoadStrByID(HIT_DPIC3),LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
					if(!ismenutimeout) SendMessage(hDlg,MSG_CLOSE,0,0);
				} else if(dPicItem==2) {
					SendMessage(hDlg,MSG_CLOSE,0,0);
				}
				return 1;	
			} else if ((LOWORD(wParam)==SCANCODE_ESCAPE)) {
				SendMessage(hDlg,MSG_CLOSE,0,0);
				return 1;
			}
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

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());

				if (fGetDC){
					ReleaseDC (hdc);
				}
				return 0;
			}

		case MSG_PAINT:
			hdc=BeginPaint(hDlg);
			DPicDlgShowInfo(hDlg,hdc,dpicitem);
			EndPaint(hDlg,hdc);
			return 0;

		case MSG_CLOSE:
			//UnloadBitmap(&dpicbk);
			UnloadBitmap(&ddup);
			UnloadBitmap(&ddown);
			EndDialog (hDlg, IDCANCEL);
			return 0;

		case MSG_COMMAND:
			break;
	}

	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_DPICDLG(HWND hWnd)
{
	gMainMenu_rc.right = gOptions.LCDWidth-1;
	gMainMenu_rc.bottom = gOptions.LCDHeight-1;
	DPicDlgDlgBox.w = gOptions.LCDWidth-1;
	DPicDlgDlgBox.h = gOptions.LCDHeight-1;
	DPicDlgDlgBox.controls = DPicDlgCtrl;

	DialogBoxIndirectParam (&DPicDlgDlgBox, hWnd, DPicDlgDialogBoxProc, 0L);
}
