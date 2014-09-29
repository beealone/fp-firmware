/*
   tftmsgbox.c 用于多国语言的系统提示 与消息框
   2008.04 zksoftware
   */
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/mgext.h>
#include <minigui/control.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "ssrpub.h"
#include "options.h"
#include "tftmsgbox.h"
//BITMAP msgbmp;

/*statuswin================================================*/
static int StatusWinProc1 (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	int tmpvalue = 0;
	switch (message) {
		case MSG_CREATE:
			//LoadBitmap(HDC_SCREEN,&msgbmp,GetPicPath("msgbox.jpg"));
			break;
		case MSG_PAINT:
			{ 
				HDC hdc;
				char* text;
				RECT rc;

				hdc = BeginPaint (hWnd);
				//FillBoxWithBitmap(hdc,0,0,125,52,&msgbmp);
				text = (char*) GetWindowAdditionalData (hWnd);
				if (text) {
					GetClientRect (hWnd, &rc);
					SelectFont(hdc,gfont);
					tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
					//SetBkColor (hdc, COLOR_lightgray);
					TextOut (hdc, 5, (rc.bottom - GetSysCharHeight())>>1, text);
				}

				EndPaint (hWnd, hdc);
				return 0;
			}
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}
HWND createStatusWin1 (HWND hParentWnd, int width, int height,
		char* title, char* text, ...)
{
	HWND hwnd;
	char* buf = NULL;
	MAINWINCREATE CreateInfo;

	if (strchr (text, '%')) {
		va_list args;
		int size = 0;
		int i = 0;

		va_start(args, text);
		do {
			size += 1000;
			if (buf) FREE(buf);
			buf = MALLOC(size);
			i = vsnprintf(buf, size, text, args);
		} while (i == size);
		va_end(args);
	}

	width = ClientWidthToWindowWidth (WS_CAPTION | WS_BORDER, width);
	height= ClientHeightToWindowHeight (WS_CAPTION | WS_BORDER, height, FALSE);

	CreateInfo.dwStyle = WS_ABSSCRPOS | WS_CAPTION | WS_BORDER | WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NOCLOSEBOX;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor (IDC_WAIT);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = StatusWinProc1;
	CreateInfo.lx = (GetGDCapability (HDC_SCREEN, GDCAP_MAXX) - width) >> 1;
	CreateInfo.ty = (GetGDCapability (HDC_SCREEN, GDCAP_MAXY) - height) >> 1;
	CreateInfo.rx = CreateInfo.lx + width;
	CreateInfo.by = CreateInfo.ty + height;
	CreateInfo.iBkColor = COLOR_lightgray;
	CreateInfo.dwAddData = buf ? (DWORD) buf : (DWORD) text;
	CreateInfo.hHosting = hParentWnd;

	hwnd = CreateMainWindow (&CreateInfo);
	if (hwnd == HWND_INVALID){
		return hwnd;
	}
	SetWindowFont(hwnd,gfont);
	SetWindowAdditionalData2 (hwnd, buf ? (DWORD) buf : 0);
	UpdateWindow (hwnd, TRUE);

	return hwnd;
}

void destroyStatusWin1(HWND hwnd)
{
	char* buf=NULL;
	//dsl 2012.4.17
	if (hwnd == HWND_INVALID) {
		return;
	}

	DestroyMainWindow (hwnd);
	MainWindowThreadCleanup (hwnd);

	buf = (char*)GetWindowAdditionalData2 (hwnd);
	if (buf){
		free(buf);
		buf=NULL;
	}
}
/*=====================msgbox===========================================*/
DWORD mytftboxstyle;
char *infochar;
#define MB_MARGIN    10
#define MB_BUTTONW   80
#define MB_BUTTONH   26
#define MB_TEXTW     300

static int MsgBoxProc1 (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	//	HWND myboxwnd;
	//HDC hdc;
	switch (message) {
		case MSG_INITDIALOG:
			{
				/*
				   switch (mytftboxstyle & MB_TYPEMASK) {
				   case MB_OK:
				   {
				   myboxwnd=GetDlgItem(hWnd,IDOK);
				   SetWindowFont(myboxwnd,gfont);
				   SetWindowText(myboxwnd,LoadStrByID(MID_REMOVEHINT));
				   break;
				   }
				   case MB_OKCANCEL:
				   {
				   myboxwnd=GetDlgItem(hWnd,IDOK);
				   SetWindowFont(myboxwnd,gfont);
				   SetWindowText(myboxwnd,LoadStrByID(MID_REMOVEHINT));
				   myboxwnd=GetDlgItem(hWnd,IDCANCEL);
				   SetWindowFont(myboxwnd,gfont);
				   SetWindowText(myboxwnd,LoadStrByID(HID_NO));
				   }

				   }
				   */

				HWND hFocus = GetDlgDefPushButton (hWnd);
				if (hFocus)
					SetFocus (hFocus);

				SetWindowAdditionalData (hWnd, (DWORD)lParam);
				return 0;
			}

#if 0
		case MSG_PAINT:
			{
				hdc=BeginPaint(hWnd);
				SelectFont (hdc, gfont);
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,COLOR_black);
				TextOut(hdc,50, GetSysCharHeight(),infochar);
				EndPaint(hWnd,hdc);
				return 0;

			}		
#endif

#if 0
		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;
				RECT rcText1;

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

				rcText1.left = 0;
				rcText1.top  = 0;
				rcText1.right = btnright1 + (MB_MARGIN << 1);
				rcText1.right = MAX (rcText1.right, MB_TEXTW);
				rcText1.bottom = GetSysCharHeight ();
				SelectFont (hdc, gfont);
				DrawText (hdc, LoadStrByID(MID_OK), -1, &rcText1,
						DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);
				if (fGetDC)
					ReleaseDC (hdc);
				return 0;
			}
#endif
		case MSG_KEYDOWN:
			if(LOWORD(wParam) == SCANCODE_BACKSPACE && gOptions.TFTKeyLayout ==3)
			{
				SendMessage(hWnd,MSG_KEYDOWN,SCANCODE_CURSORBLOCKLEFT,0L);
			}
			break;
		case MSG_COMMAND:
			{
				switch (wParam) {
					case IDOK:
					case IDCANCEL:
					case IDABORT:
					case IDRETRY:
					case IDIGNORE:
					case IDYES:
					case IDNO:
						if (GetDlgItem (hWnd, wParam))
							EndDialog (hWnd, wParam);
						break;
				}
				break;
			}
		case MSG_CLOSE:
			if (GetDlgItem (hWnd, IDCANCEL)) {
				EndDialog (hWnd, IDCANCEL);
			}
			else if (GetDlgItem (hWnd, IDIGNORE)) {
				EndDialog (hWnd, IDIGNORE);
			}
			else if (GetDlgItem (hWnd, IDNO)) {
				EndDialog (hWnd, IDNO);
			}
			else if (GetDlgItem (hWnd, IDOK)) {
				EndDialog (hWnd, IDOK);
			}
			break;

		default:
			break;
	}

	return DefaultDialogProc (hWnd, message, wParam, lParam);
}


static void get_box_xy1 (HWND hParentWnd, DWORD dwStyle, DLGTEMPLATE* MsgBoxData)
{
	RECT rcTemp;

	if (dwStyle & MB_BASEDONPARENT) {
		GetWindowRect (hParentWnd, &rcTemp);
	}
	else {
		rcTemp = g_rcDesktop;
	}

	switch (dwStyle & MB_ALIGNMASK) {
		case MB_ALIGNCENTER:
			MsgBoxData->x = rcTemp.left + (RECTW(rcTemp) - MsgBoxData->w)/2;
			MsgBoxData->y = rcTemp.top + (RECTH(rcTemp) - MsgBoxData->h)/2;
			break;

		case MB_ALIGNTOPLEFT:
			MsgBoxData->x = rcTemp.left;
			MsgBoxData->y = rcTemp.top;
			break;

		case MB_ALIGNBTMLEFT:
			MsgBoxData->x = rcTemp.left;
			MsgBoxData->y = rcTemp.bottom - MsgBoxData->h;
			break;

		case MB_ALIGNTOPRIGHT:
			MsgBoxData->x = rcTemp.right - MsgBoxData->w;
			MsgBoxData->y = rcTemp.top;
			break;
		case MB_ALIGNBTMRIGHT:
			MsgBoxData->x = rcTemp.right - MsgBoxData->w;
			MsgBoxData->y = rcTemp.bottom - MsgBoxData->h;
			break;
	}

	if ((MsgBoxData->x + MsgBoxData->w) > g_rcDesktop.right) {
		MsgBoxData->x = g_rcDesktop.right - MsgBoxData->w;
	}

	if ((MsgBoxData->y + MsgBoxData->h) > g_rcDesktop.bottom) {
		MsgBoxData->y = g_rcDesktop.bottom - MsgBoxData->h;
	}
}



int  MessageBox1 (HWND hParentWnd, const char* pszText,
		const char* pszCaption, DWORD dwStyle)
{
	DLGTEMPLATE MsgBoxData =
	{
		WS_ABSSCRPOS | WS_CAPTION | WS_BORDER,
		WS_EX_NONE, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, 0L
	};
	CTRLDATA     CtrlData [5] =
	{
		{"button",
			BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE | WS_GROUP,
			0, 0, 0, 0, 0, NULL, 0L},
		{"button", 
			BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE,
			0, 0, 0, 0, 0, NULL, 0L},
		{"button",
			BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE,
			0, 0, 0, 0, 0, NULL, 0L}
	};

	int i, nButtons, buttonx;
	RECT rcText, rcButtons, rcIcon;
	int width, height;

	mytftboxstyle=dwStyle;	
	//infochar=pszText;
	if (pszCaption)
		MsgBoxData.caption  = pszCaption;
	else
		MsgBoxData.caption  = "MiniGUI";

	switch (dwStyle & MB_TYPEMASK) {
		case MB_OK:
			MsgBoxData.controlnr = 1;
			//CtrlData [0].caption = GetSysText (SysText[10]);
			CtrlData [0].caption = LoadStrByID(MID_REMOVEHINT);
			CtrlData [0].id      = IDOK;
			break;
		case MB_OKCANCEL:
			MsgBoxData.controlnr = 2;
			//CtrlData [0].caption = GetSysText (SysText[10]);
			CtrlData [0].caption = LoadStrByID(MID_REMOVEHINT);
			CtrlData [0].id      = IDOK;
			//CtrlData [1].caption = (dwStyle & MB_CANCELASBACK) ?
			//              GetSysText (SysText[13]) : GetSysText (SysText[12]);
			CtrlData [1].caption =LoadStrByID(HID_CANCEL);
			CtrlData [1].id      = IDCANCEL;
			break;
		case MB_YESNO:
			MsgBoxData.controlnr = 2;
			CtrlData [0].caption = GetSysText (SysText[14]);
			CtrlData [0].id      = IDYES;
			CtrlData [1].caption = GetSysText (SysText[15]);
			CtrlData [1].id      = IDNO;
			break;
		case MB_RETRYCANCEL:
			MsgBoxData.controlnr = 2;
			CtrlData [0].caption = GetSysText (SysText[17]);
			CtrlData [0].id      = IDRETRY;
			CtrlData [1].caption = (dwStyle & MB_CANCELASBACK) ?
				GetSysText (SysText[13]) : GetSysText (SysText[12]);
			CtrlData [1].id      = IDCANCEL;
			break;
		case MB_ABORTRETRYIGNORE:
			MsgBoxData.controlnr = 3;
			CtrlData [0].caption = GetSysText (SysText[16]);
			CtrlData [0].id      = IDABORT;
			CtrlData [1].caption = GetSysText (SysText[17]);
			CtrlData [1].id      = IDRETRY;
			CtrlData [2].caption = GetSysText (SysText[18]);
			CtrlData [2].id      = IDIGNORE;
			break;
		case MB_YESNOCANCEL:
			MsgBoxData.controlnr = 3;
			CtrlData [0].caption = GetSysText (SysText[14]);
			CtrlData [0].id      = IDYES;
			CtrlData [1].caption = GetSysText (SysText[15]);
			CtrlData [1].id      = IDNO;
			CtrlData [2].caption = (dwStyle & MB_CANCELASBACK) ?
				GetSysText (SysText[13]) : GetSysText (SysText[12]);
			CtrlData [2].id      = IDCANCEL;
			break;
	}

	switch (dwStyle & MB_DEFMASK) {
		case MB_DEFBUTTON1:
			CtrlData [0].dwStyle |= BS_DEFPUSHBUTTON;
			break;
		case MB_DEFBUTTON2:
			if (MsgBoxData.controlnr > 1)
				CtrlData [1].dwStyle |= BS_DEFPUSHBUTTON;
			break;
		case MB_DEFBUTTON3:
			if (MsgBoxData.controlnr > 2)
				CtrlData [2].dwStyle |= BS_DEFPUSHBUTTON;
			break;
	}

	nButtons = MsgBoxData.controlnr;
	rcButtons.left   = 0;
	rcButtons.top    = 0;
	rcButtons.bottom = MB_BUTTONH;
	rcButtons.right  = MsgBoxData.controlnr * MB_BUTTONW +
		(MsgBoxData.controlnr - 1) * (MB_MARGIN << 1);

	rcIcon.left   = 0;
	rcIcon.top    = 0;
	rcIcon.right  = 0;
	rcIcon.bottom = 0;
	if (dwStyle & MB_ICONMASK) {
		int id_icon = -1;
		i = MsgBoxData.controlnr;
		CtrlData [i].class_name= "static";
		CtrlData [i].dwStyle   = WS_VISIBLE | SS_ICON | WS_GROUP;
		CtrlData [i].x         = MB_MARGIN;
		CtrlData [i].y         = MB_MARGIN;
		CtrlData [i].w         = 32;
		CtrlData [i].h         = 32;
		CtrlData [i].id        = IDC_STATIC;
		CtrlData [i].caption   = "Hello";
		switch (dwStyle & MB_ICONMASK) {
			case MB_ICONSTOP:
				id_icon = IDI_STOP;
				break;
			case MB_ICONINFORMATION:
				id_icon = IDI_INFORMATION;
				break;
			case MB_ICONEXCLAMATION:
				id_icon = IDI_EXCLAMATION;
				break;
			case MB_ICONQUESTION:
				id_icon = IDI_QUESTION;
				break;
		}

		if (id_icon != -1) {
			CtrlData [i].dwAddData = GetLargeSystemIcon (id_icon);
			MsgBoxData.hIcon       = GetSmallSystemIcon (id_icon);
		}

		rcIcon.right  = 32;
		rcIcon.bottom = 32;
		MsgBoxData.controlnr ++;
	}

	rcText.left = 0;
	rcText.top  = 0;
	rcText.right = rcButtons.right + (MB_MARGIN << 1);
	rcText.right = MAX (rcText.right, MB_TEXTW);
	rcText.bottom = GetSysCharHeight ();
	//SelectFont (HDC_SCREEN, GetSystemFont (SYSLOGFONT_CONTROL));
	SelectFont (HDC_SCREEN, gfont);
	DrawText (HDC_SCREEN, pszText, -1, &rcText,
			DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);

	i = MsgBoxData.controlnr;
#if 1
	CtrlData [i].class_name= "static";
	CtrlData [i].dwStyle   = WS_VISIBLE | SS_LEFT | WS_GROUP;
	CtrlData [i].x         = RECTW (rcIcon) + (MB_MARGIN << 1);
	CtrlData [i].y         = MB_MARGIN;
	CtrlData [i].w         = RECTW (rcText);
	CtrlData [i].h         = RECTH (rcText);
	CtrlData [i].id        = IDC_STATIC;
	CtrlData [i].caption   = pszText;
	//CtrlData [i].caption   = "";
	CtrlData [i].dwAddData = 0;
	MsgBoxData.controlnr ++;
#endif
	width = MAX (RECTW (rcText), RECTW (rcButtons)) + RECTW (rcIcon)
		+ (MB_MARGIN << 2)
		+ (GetMainWinMetrics(MWM_BORDER) << 1);
	height = MAX (RECTH (rcText), RECTH (rcIcon)) + RECTH (rcButtons)
		+ MB_MARGIN + (MB_MARGIN << 1)
		+ (GetMainWinMetrics (MWM_BORDER) << 1)
		+ GetMainWinMetrics (MWM_CAPTIONY);

	buttonx = (width - RECTW (rcButtons)) >> 1;
	for (i = 0; i < nButtons; i++) {
		CtrlData[i].x = buttonx + i*(MB_BUTTONW + MB_MARGIN);
		CtrlData[i].y = MAX (RECTH (rcIcon), RECTH (rcText)) + (MB_MARGIN<<1);
		CtrlData[i].w = MB_BUTTONW;
		CtrlData[i].h = MB_BUTTONH;
	}

	//printf("width:%d\theight:%d\n",width,height);
	MsgBoxData.w = width;
	MsgBoxData.h = height;
	get_box_xy1 (hParentWnd, dwStyle, &MsgBoxData);

	MsgBoxData.controls = CtrlData;

	return DialogBoxIndirectParam (&MsgBoxData, hParentWnd, MsgBoxProc1,
			(LPARAM)dwStyle);
}

int myMessageBox1 (HWND hwnd, DWORD dwStyle, char* title, char* text, ...)
{
	char * buf = NULL;
	int rc;

	if (strchr (text, '%')) {
		va_list args;
		int size = 0;
		int i = 0;

		va_start(args, text);
		do {
			size += 1000;
			if (buf) FREE(buf);
			buf = MALLOC(size);
			i = vsnprintf(buf, size, text, args);
		} while (i == size);
		va_end(args);
	}

	rc = MessageBox1 (hwnd, buf ? buf : text, title, dwStyle);

	if (buf)
		free (buf);

	return rc;
}

