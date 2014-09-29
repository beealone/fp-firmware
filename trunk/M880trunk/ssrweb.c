/* 
 * Web 连接设置 主入口头文件
 * 设计：CSK        2008.3.25
 * 原始版本:1.0.0   
 * 修改记录:
 *
 * 编译环境:mipsel-gcc
 */

#include <minigui/common.h>
#include "ssrcommon.h"
#include "ssrpub.h"
#include "main.h"
#include "pushapi.h"
#include "t9.h"

#define IDC_WEB_ADDRESS		90210
#define IDC_WEB_ADDRESS0	90211
#define IDC_WEB_ADDRESS1	90212
#define IDC_WEB_ADDRESS2	90213
#define IDC_WEB_ADDRESS3	90214
#define IDC_WEB_PORT		90215
#define IDC_WEB_MODEL		90216
#define IDC_WEBADDRESS_COLON	90217

#define IDC_PROXY_ADDRESS	90220
#define IDC_PROXY_ADDRESS0	90221
#define IDC_PROXY_ADDRESS1	90222
#define IDC_PROXY_ADDRESS2	90223
#define IDC_PROXY_ADDRESS3	90224
#define IDC_PROXY_PORT		90225
#define IDC_PROXY_ENABLED	90226
#define IDC_PROXY_COLON		90227


#define BASEHEIGHT	20
#define ROWSPACING	35
#define SELECTED	1
#define NOTSELECTED	0

typedef struct  _MGUICTRL{
	CTRLDATA Ctrl;
	HWND ItemWnd;
	char IsSelectFlag;
	char IsHideFlag;
	char IsEditFlag;
}TMGUICTRL, *PMGUICTRL;

typedef struct TMGUICTRLNode *PMGUICTRLNode;
struct TMGUICTRLNode{
	TMGUICTRL MGUICtrl;
	PMGUICTRLNode Previous;
	PMGUICTRLNode Next;
};

static CTRLDATA WebCtrl [] =
{ 
	{
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		10, BASEHEIGHT, 135, 20, 
		0x101, 
		"Web Server",
		0
	},
	{
		CTRL_COMBOBOX,
		WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
		145, BASEHEIGHT, 100, 20,
		IDC_WEB_MODEL,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		30, BASEHEIGHT+ROWSPACING, 155, 20,
		IDC_WEB_ADDRESS,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		30, BASEHEIGHT+ROWSPACING, 35, 20,
		IDC_WEB_ADDRESS0,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		70, BASEHEIGHT+ROWSPACING, 35, 20,
		IDC_WEB_ADDRESS1,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		110, BASEHEIGHT+ROWSPACING, 35, 20,
		IDC_WEB_ADDRESS2,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		150, BASEHEIGHT+ROWSPACING, 35, 20,
		IDC_WEB_ADDRESS3,
		"",
		0
	},
	{
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		186, BASEHEIGHT+ROWSPACING, 5, 20,
		IDC_WEBADDRESS_COLON,
		":",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		195, BASEHEIGHT+ROWSPACING, 45, 20,
		IDC_WEB_PORT,
		"80",
		0
	},
	{
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		10, BASEHEIGHT+ROWSPACING*2, 180, 20,
		0x102,
		"Proxy",
		0
	},
	{
		CTRL_COMBOBOX,
		WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
		150, BASEHEIGHT+ROWSPACING*2, 80, 25,
		IDC_PROXY_ENABLED,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		30, BASEHEIGHT+ROWSPACING*3, 155, 20,
		IDC_PROXY_ADDRESS,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		30, BASEHEIGHT+ROWSPACING*3, 35, 20,
		IDC_PROXY_ADDRESS0,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		70, BASEHEIGHT+ROWSPACING*3, 35, 20,
		IDC_PROXY_ADDRESS1,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		110, BASEHEIGHT+ROWSPACING*3, 35, 20,
		IDC_PROXY_ADDRESS2,
		"",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		150, BASEHEIGHT+ROWSPACING*3, 35, 20,
		IDC_PROXY_ADDRESS3,
		"",
		0
	},
	{
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		186, BASEHEIGHT+ROWSPACING*3, 5, 20,
		IDC_PROXY_COLON,
		":",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
		195, BASEHEIGHT+ROWSPACING*3, 45, 20,
		IDC_PROXY_PORT,
		"808",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
		10, 180, 85, 25,
		IDOK, 
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
	}
};

static DLGTEMPLATE WebDlgBox =
{
	WS_BORDER | WS_CAPTION, 
	WS_EX_NONE,
	1, 1, 319, 239, 
	"",
	0, 0,
	sizeof(WebCtrl)/sizeof(WebCtrl[0]), NULL,
	0
};

//static BITMAP webbk;

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);

static PMGUICTRLNode MGUICTRLList = NULL;
static PMGUICTRLNode MGUICTRLHeader = NULL;
static PMGUICTRLNode MGUICTRLCurrent = NULL;
int gMGUICTRLListCount = 0;
int IsSelectCtrl(void *CtrlFlag, int ID, int ItemFlag)
{
	return 1;
}

void InitWebControl()
{
	MGUICTRLList = MALLOC(sizeof(struct TMGUICTRLNode));
	MGUICTRLList->Previous = NULL;
	MGUICTRLList->Next = NULL;
	MGUICTRLHeader = MGUICTRLList;
	MGUICTRLHeader->MGUICtrl.IsSelectFlag = 0;
	MGUICTRLHeader->MGUICtrl.IsHideFlag = 0;
	MGUICTRLHeader->MGUICtrl.IsEditFlag = 0;
	gMGUICTRLListCount = 1;

	return;
}

void AppendWebControl(CTRLDATA *Ctrl, char IsSelectFlag)
{
	PMGUICTRLNode TmpCell;
        //DebugOutput2("AppendRTLog %d, %d",EventType,Len);
        TmpCell=(PMGUICTRLNode)MALLOC(sizeof(struct TMGUICTRLNode));
        memcpy((void *)&(TmpCell->MGUICtrl.Ctrl), (void *)Ctrl, sizeof(CTRLDATA));
	TmpCell->MGUICtrl.IsSelectFlag = IsSelectFlag;
	TmpCell->MGUICtrl.IsHideFlag = 0;
	TmpCell->MGUICtrl.IsEditFlag = 0;
	TmpCell->Previous = MGUICTRLList;
        TmpCell->Next = MGUICTRLHeader;

	MGUICTRLHeader->Previous = TmpCell;
        MGUICTRLList->Next = TmpCell;
        MGUICTRLList = TmpCell;
        gMGUICTRLListCount++;

	return;
}

void FreeWebControl(PMGUICTRLNode WebMGUICTRLNode)
{
	if(!WebMGUICTRLNode)
		return;
	PMGUICTRLNode tmp;
	PMGUICTRLNode WebMGUICTRLNodeback;
	WebMGUICTRLNodeback = WebMGUICTRLNode;
	while(1)
	{
		if(WebMGUICTRLNode->Next && WebMGUICTRLNode->Next != WebMGUICTRLNodeback)
		{
			tmp = WebMGUICTRLNode->Next;
			FREE(WebMGUICTRLNode);
			WebMGUICTRLNode = NULL;
			WebMGUICTRLNode = tmp;
		}
		else
		{
			FREE(WebMGUICTRLNode);
			break;
		}
	}
	WebMGUICTRLNode = NULL;
	return;
}

void InitWebWindows(HWND hDlg)
{
	int WebItem = 0;;
	char buffer[256];
	char WebIP0[4],WebIP1[4],WebIP2[4],WebIP3[4];
	char ProxyIP0[4],ProxyIP1[4],ProxyIP2[4],ProxyIP3[4];
	memset(buffer, 0, sizeof(buffer));
	InitWebControl();

	LoadStr("WebServerIP",buffer);
	if(!ParseIP(buffer, WebIP0, WebIP1, WebIP2, WebIP3))
	{
		memset(WebIP0, 0, 4);
		memset(WebIP1, 0, 4);
		memset(WebIP2, 0, 4);
		memset(WebIP3, 0, 4);
	}
	memset(buffer, 0, sizeof(buffer));
	LoadStr("ProxyServerIP",buffer);
	if(!ParseIP(buffer, ProxyIP0, ProxyIP1, ProxyIP2, ProxyIP3))
	{
		memset(ProxyIP0, 0, 4);
		memset(ProxyIP1, 0, 4);
		memset(ProxyIP2, 0, 4);
		memset(ProxyIP3, 0, 4);
	}

	//ID=0x101,Web Server
	AppendWebControl(&WebCtrl[WebItem++], (char)NOTSELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, LoadStrByIDDef(MID_WENSERVER, "Web Server"));

	//ID=IDC_WEB_MODEL
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SendDlgItemMessage(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id, CB_ADDSTRING, 0, (LPARAM)LoadStrByIDDef(MID_IP_MODEL, "IP Model"));
	SendDlgItemMessage(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id, CB_ADDSTRING, 0, (LPARAM)LoadStrByIDDef(MID_URL_MODEL, "URL Model"));
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, CB_SETCURSEL, gOptions.WebServerURLModel, 0);
	SetFocus(MGUICTRLList->MGUICtrl.ItemWnd);
	MGUICTRLCurrent = MGUICTRLList;

	//ID=IDC_WEB_ADDRESS
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	memset(buffer, 0, sizeof(buffer));
	LoadStr("ICLOCKSVRURL", buffer);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, buffer);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 30, 0);
	if(!gOptions.WebServerURLModel)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_WEB_ADDRESS0
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, WebIP0);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 3, 0);
	if(gOptions.WebServerURLModel)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_WEB_ADDRESS1
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, WebIP1);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 3, 0);
	if(gOptions.WebServerURLModel)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_WEB_ADDRESS2
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, WebIP2);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 3, 0);
	if(gOptions.WebServerURLModel)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_WEB_ADDRESS3
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, WebIP3);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 3, 0);
	if(gOptions.WebServerURLModel)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_WEBADDRESS_COLON
	AppendWebControl(&WebCtrl[WebItem++], (char)NOTSELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, ":");

	//ID=IDC_WEB_PORT
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	memset(buffer, 0, sizeof(buffer));
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	gOptions.WebServerPort = LoadInteger("WebServerPort", 6000);
	printf("[%s] WebServerPort:%d\n", __FUNCTION__,gOptions.WebServerPort);
	sprintf(buffer, "%d", gOptions.WebServerPort);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, buffer);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 5, 0);

	//ID=0x102, Proxy
	AppendWebControl(&WebCtrl[WebItem++], (char)NOTSELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, LoadStrByIDDef(MID_WENSERVER_PROXY, "Proxy"));

	//ID=IDC_PROXY_ENABLED
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SendDlgItemMessage(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id, CB_ADDSTRING, 0, (LPARAM)SwitchStr[0]);
	SendDlgItemMessage(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id, CB_ADDSTRING, 0, (LPARAM)SwitchStr[1]);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, CB_SETCURSEL, gOptions.EnableProxyServer, 0);

	//ID=IDC_PROXY_ADDRESS
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	memset(buffer, 0, sizeof(buffer));
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	LoadStr("ProxyServerIP", buffer);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, buffer);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 30, 0);
	//if (!gOptions.EnableProxyServer || (gOptions.EnableProxyServer && !gOptions.WebServerURLModel))
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_PROXY_ADDRESS0
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	memset(buffer, 0, sizeof(buffer));
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, ProxyIP0);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 3, 0);
	//if (!gOptions.EnableProxyServer || (gOptions.EnableProxyServer && gOptions.WebServerURLModel))
	if (!gOptions.EnableProxyServer)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_PROXY_ADDRESS1
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	memset(buffer, 0, sizeof(buffer));
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, ProxyIP1);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 3, 0);
	//if (!gOptions.EnableProxyServer || (gOptions.EnableProxyServer && gOptions.WebServerURLModel))
	if (!gOptions.EnableProxyServer)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_PROXY_ADDRESS2
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	memset(buffer, 0, sizeof(buffer));
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, ProxyIP2);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 3, 0);
	//if (!gOptions.EnableProxyServer || (gOptions.EnableProxyServer && gOptions.WebServerURLModel))
	if (!gOptions.EnableProxyServer)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_PROXY_ADDRESS3
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	memset(buffer, 0, sizeof(buffer));
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, ProxyIP3);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 3, 0);
	//if (!gOptions.EnableProxyServer || (gOptions.EnableProxyServer && gOptions.WebServerURLModel))
	if (!gOptions.EnableProxyServer)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_PROXY_COLON
	AppendWebControl(&WebCtrl[WebItem++], (char)NOTSELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, ":");
	if (!gOptions.EnableProxyServer)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDC_PROXY_PORT
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	memset(buffer, 0, sizeof(buffer));
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	sprintf(buffer, "%d", gOptions.ProxyServerPort);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, buffer);
	SendMessage(MGUICTRLList->MGUICtrl.ItemWnd, EM_LIMITTEXT, 5, 0);
	if (!gOptions.EnableProxyServer)
	{
		ShowWindow(MGUICTRLList->MGUICtrl.ItemWnd, SW_HIDE);
		MGUICTRLList->MGUICtrl.IsHideFlag = 1;
	}

	//ID=IDOK
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, LoadStrByID(HIT_OK));

	//ID=IDCANCEL
	AppendWebControl(&WebCtrl[WebItem++], (char)SELECTED);
	MGUICTRLList->MGUICtrl.ItemWnd = GetDlgItem(hDlg, MGUICTRLList->MGUICtrl.Ctrl.id);
	SetWindowText(MGUICTRLList->MGUICtrl.ItemWnd, LoadStrByID(HIT_CANCEL));

	return;
}

static void SelectNext()
{
	while(1)
	{
		if(MGUICTRLCurrent && MGUICTRLCurrent->Next && MGUICTRLCurrent->Next->MGUICtrl.IsSelectFlag && !MGUICTRLCurrent->Next->MGUICtrl.IsHideFlag)
		{
			MGUICTRLCurrent = MGUICTRLCurrent->Next;
			SetFocus(MGUICTRLCurrent->MGUICtrl.ItemWnd);
			return;
		}
		MGUICTRLCurrent = MGUICTRLCurrent->Next;
	}
	return;
}

static void SelectPrevious()
{
	while(1)
	{
		if(MGUICTRLCurrent && MGUICTRLCurrent->Previous && MGUICTRLCurrent->Previous->MGUICtrl.IsSelectFlag && !MGUICTRLCurrent->Previous->MGUICtrl.IsHideFlag)
		{
			MGUICTRLCurrent = MGUICTRLCurrent->Previous;
			SetFocus(MGUICTRLCurrent->MGUICtrl.ItemWnd);
			return;
		}
		MGUICTRLCurrent = MGUICTRLCurrent->Previous;
	}
	return;
}

static int WebOK(HWND hWnd, PMGUICTRLNode WebCtrl)
{
	char str[254];
	char WebIP0[4],WebIP1[4],WebIP2[4],WebIP3[4];
	char ProxyIP0[4],ProxyIP1[4],ProxyIP2[4],ProxyIP3[4];
	int err = 0;
	PMGUICTRLNode WebOKCtrl = WebCtrl;
	PMGUICTRLNode WebCtrlBack = WebCtrl;
	while(1)
	{
		if(WebOKCtrl && WebOKCtrl->MGUICtrl.IsSelectFlag && !WebOKCtrl->MGUICtrl.IsHideFlag)
		{
			switch(WebOKCtrl->MGUICtrl.Ctrl.id)
			{
				case IDC_WEB_ADDRESS:
				{
					memset(str, 0, sizeof(str));
					GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, str, sizeof(str));
					SaveStr("ICLOCKSVRURL", str, 1);
					break;
				}
				case IDC_WEB_MODEL:
				{
					gOptions.WebServerURLModel = SendMessage(WebOKCtrl->MGUICtrl.ItemWnd, CB_GETCURSEL, 0, 0);
					SaveInteger("WebServerURLModel", gOptions.WebServerURLModel);
					break;
				}
				case IDC_WEB_ADDRESS0:
				{
					memset(WebIP0, 0, sizeof(WebIP0));
					GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, WebIP0, sizeof(WebIP0));
					break;
				}
				case IDC_WEB_ADDRESS1:
				{
					memset(WebIP1, 0, sizeof(WebIP1));
					GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, WebIP1, sizeof(WebIP1));
					break;
				}
				case IDC_WEB_ADDRESS2:
				{
					memset(WebIP2, 0, sizeof(WebIP2));
					GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, WebIP2, sizeof(WebIP2));
					break;
				}
				case IDC_WEB_ADDRESS3:
				{
					memset(WebIP3, 0, sizeof(WebIP3));
					GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, WebIP3, sizeof(WebIP3));
					break;
				}
				case IDC_WEB_PORT:
				{
					memset(str, 0, sizeof(str));
					GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, str, sizeof(str));
					gOptions.WebServerPort = atoi(str);
					SaveInteger("WebServerPort", atoi(str));
					break;
				}
				case IDC_PROXY_ENABLED:
				{
					gOptions.EnableProxyServer = SendMessage(WebOKCtrl->MGUICtrl.ItemWnd, CB_GETCURSEL, 0, 0);
					SaveInteger("EnableProxyServer", gOptions.EnableProxyServer);
					break;
				}
				case IDC_PROXY_ADDRESS:
				{
					/*printf("xsen test edit IDC_PROXY_ADDRESS==, gOptions.EnableProxyServer=%d\n",gOptions.EnableProxyServer);
					if(gOptions.EnableProxyServer)
					{
						memset(str, 0, sizeof(str));
						GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, str, sizeof(str));
						if(ParseIP(str, c1, c2, c3, c4) && CheckIP(c1, c2, c3, c4, 0))
						{
							gOptions.ProxyServerIP[0] = atoi(c1);
							gOptions.ProxyServerIP[1] = atoi(c2);
							gOptions.ProxyServerIP[2] = atoi(c3);
							gOptions.ProxyServerIP[3] = atoi(c4);
							sprintf(str, "%s.%s.%s.%s", c1, c2, c3, c4);
							SaveStr("ProxyServerIP", str, 1);
						}
						else
							err = 2;
					}*/
					break;
				}
				case IDC_PROXY_ADDRESS0:
				{
					if(gOptions.EnableProxyServer)
					{
						memset(ProxyIP0, 0, sizeof(ProxyIP0));
						GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, ProxyIP0, sizeof(ProxyIP0));
					}
					break;
				}
				case IDC_PROXY_ADDRESS1:
				{
					if(gOptions.EnableProxyServer)
					{
						memset(ProxyIP1, 0, sizeof(ProxyIP1));
						GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, ProxyIP1, sizeof(ProxyIP1));
					}
					break;
				}
				case IDC_PROXY_ADDRESS2:
				{
					if(gOptions.EnableProxyServer)
					{
						memset(ProxyIP2, 0, sizeof(ProxyIP2));
						GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, ProxyIP2, sizeof(ProxyIP2));
					}
					break;
				}
				case IDC_PROXY_ADDRESS3:
				{
					if(gOptions.EnableProxyServer)
					{
						memset(ProxyIP3, 0, sizeof(ProxyIP3));
						GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, ProxyIP3, sizeof(ProxyIP3));
					}
					break;
				}
				case IDC_PROXY_PORT:
				{
					if(gOptions.EnableProxyServer)
					{
						memset(str, 0, sizeof(str));
						GetWindowText(WebOKCtrl->MGUICtrl.ItemWnd, str, sizeof(str));
						gOptions.ProxyServerPort = atoi(str);
						SaveInteger("ProxyServerPort", atoi(str));
					}
					break;
				}
			}
		}
		if(WebOKCtrl->Next == WebCtrlBack)
		{
			break;
		}
		WebOKCtrl = WebOKCtrl->Next;
	}
	if(!gOptions.WebServerURLModel)
	{
		if(CheckIP(WebIP0, WebIP1, WebIP2, WebIP3, 1))
		{
			gOptions.WebServerIP[0] = atoi(WebIP0);
			gOptions.WebServerIP[1] = atoi(WebIP1);
			gOptions.WebServerIP[2] = atoi(WebIP2);
			gOptions.WebServerIP[3] = atoi(WebIP3);
			memset(str, 0, sizeof(str));
			sprintf(str,"%s.%s.%s.%s", WebIP0, WebIP1, WebIP2, WebIP3);
			SaveStr("WebServerIP",str,1);
		}
		else
			err = 1;
	}
	if(gOptions.EnableProxyServer)
	{
		if(CheckIP(ProxyIP0, ProxyIP1, ProxyIP2, ProxyIP3, 0))
		{
			gOptions.ProxyServerIP[0]=atoi(ProxyIP0);
			gOptions.ProxyServerIP[1]=atoi(ProxyIP1);
			gOptions.ProxyServerIP[2]=atoi(ProxyIP2);
			gOptions.ProxyServerIP[3]=atoi(ProxyIP3);
			memset(str, 0, sizeof(str));
			sprintf(str,"%s.%s.%s.%s", ProxyIP0, ProxyIP1, ProxyIP2, ProxyIP3);
			SaveStr("ProxyServerIP",str,1);
		}
		else
			err = 2;
	}
	if (err)
	{
		switch(err)
		{
			case 1:
				MessageBox1(hWnd, LoadStrByID(HIT_NETERROR1), LoadStrByID(HIT_ERR), MB_OK | MB_ICONINFORMATION);
				break;
			case 2:
				MessageBox1(hWnd, LoadStrByID(HIT_NETERROR1), LoadStrByID(HIT_ERR), MB_OK | MB_ICONINFORMATION);
				break;
			case 3:
				MessageBox1(hWnd, LoadStrByID(HIT_NETERROR3), LoadStrByID(HIT_ERR), MB_OK | MB_ICONINFORMATION);
				break;
			default:
				MessageBox1(hWnd, LoadStrByID(HIT_ERROR0), LoadStrByID(HIT_ERR), MB_OK | MB_ICONINFORMATION);
				break;
		}
		return 0;
	}
	else
	{
		SaveOptions(&gOptions);
		MessageBox1(hWnd, LoadStrByID(HIT_RIGHT), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
	}
	if (pushsdk_is_running()) {
		if (!gOptions.WebServerURLModel) {
			snprintf((char *)gOptions.IclockSvrURL, sizeof(gOptions.IclockSvrURL),"%d.%d.%d.%d", \
			gOptions.WebServerIP[0], gOptions.WebServerIP[1], gOptions.WebServerIP[2], gOptions.WebServerIP[3]);
			SaveStr("ICLOCKSVRURL", str, 1);
		}
		pushsdk_init_cfg();
	}
	return 1;
}


static int ShowInput(HWND hWnd, PMGUICTRLNode WebCtrl, int index)
{
	HDC hdc = GetClientDC(hWnd);
	PMGUICTRLNode ShowWebCtrl = WebCtrl;
	char ProxyHideFlag = 0;
	char ProxyShowFlag = 0;
	char IPAddressHideFlag = 0;
	char IPAddressShowFlag = 0;
	if (index)
	{
		while(1)
		{
			if(WebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ENABLED)
			{
				/*if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					ProxyHideFlag |= 1;
				}*/
				if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS0)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					ProxyHideFlag |= 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS1)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					ProxyHideFlag |= 1 << 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS2)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					ProxyHideFlag |= 1 << 2;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS3)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					ProxyHideFlag |= 1 << 3;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_COLON)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					ProxyHideFlag |= 1 << 4;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_PORT)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					ProxyHideFlag |= 1 << 5;
				}
			}
			else if(WebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_MODEL)
			{
				if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					IPAddressHideFlag |= 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS0)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					IPAddressShowFlag |= 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS1)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					IPAddressShowFlag |= 1 << 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS2)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					IPAddressShowFlag |= 1 << 2;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS3)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					IPAddressShowFlag |= 1 << 3;
				}
			}
			if(!(~ProxyHideFlag & 63))
				break;
			if(!(~IPAddressHideFlag & 1) && !(~IPAddressShowFlag & 15))
				break;
			ShowWebCtrl = ShowWebCtrl->Next;
		}
	}
	else
	{
		while(1)
		{
			if(WebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ENABLED)
			{
				/*if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					ProxyShowFlag |= 1;
				}*/
				if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS0)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					ProxyShowFlag |= 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS1)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					ProxyShowFlag |= 1 << 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS2)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					ProxyShowFlag |= 1 << 2;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_ADDRESS3)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					ProxyShowFlag |= 1 << 3;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_COLON)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					ProxyShowFlag |= 1 << 4;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_PROXY_PORT)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					ProxyShowFlag |= 1 << 5;
				}
			}
			else if(WebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_MODEL)
			{
				if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_SHOW);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 0;
					IPAddressShowFlag |= 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS0)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					IPAddressHideFlag |= 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS1)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					IPAddressHideFlag |= 1 << 1;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS2)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					IPAddressHideFlag |= 1 << 2;
				}
				else if(ShowWebCtrl->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS3)
				{
					ShowWindow(ShowWebCtrl->MGUICtrl.ItemWnd, SW_HIDE);
					ShowWebCtrl->MGUICtrl.IsHideFlag = 1;
					IPAddressHideFlag |= 1 << 3;
				}
			}
			if(!(~ProxyShowFlag & 63))
				break;
			if(!(~IPAddressShowFlag & 1) && !(~IPAddressHideFlag & 15))
				break;
			ShowWebCtrl = ShowWebCtrl->Next;
		}
	}
	ReleaseDC(hdc);
	return 1;
}

extern HWND hIMEWnd;
static int WebDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//int id = 0, nc = 0;
	static char keyupFlag = 0;
	switch (message) 
	{
		case MSG_INITDIALOG:
		{
			//if (LoadBitmap(HDC_SCREEN, &webbk, GetBmpPath("submenubg.jpg")))
			//	return 0;
			InitWebWindows(hDlg);
			return 1;
		}

		case MSG_ERASEBKGND:
		{
			HDC hdc = (HDC)wParam;
			const RECT* clip = (const RECT*) lParam;
			BOOL fGetDC = FALSE;
			RECT rcTemp;

			if (hdc == 0) {
				hdc = GetClientDC(hDlg);
				fGetDC = TRUE;
			}

			if (clip) {
				rcTemp = *clip;
				ScreenToClient(hDlg, &rcTemp.left, &rcTemp.top);
				ScreenToClient(hDlg, &rcTemp.right, &rcTemp.bottom);
				IncludeClipRect(hdc, &rcTemp);
			}

			FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

			if (fGetDC)
				ReleaseDC (hdc);
			return 0;
		}

		case MSG_PAINT:
		{
			hdc = BeginPaint(hDlg);
			EndPaint(hDlg,hdc);
			break;
		}
		case MSG_KEYUP:
		{
			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag = 1;
			}
			break;
		}
		case MSG_KEYDOWN:
		{
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout)
			{
				if(1 == keyupFlag)
					keyupFlag = 0;
				else
					break;
			}
			if(gOptions.KeyPadBeep)
				ExKeyBeep();

			if ((LOWORD(wParam) == SCANCODE_TAB))
				return 0;
			if ((LOWORD(wParam) == SCANCODE_CURSORBLOCKDOWN))
			{
				if(gOptions.IMEFunOn == 1 && MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS)
				{
					if(hIMEWnd != HWND_INVALID)
					{
						SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
						hIMEWnd=HWND_INVALID;						
					}
				}
				SelectNext();
				return 1;
			}
			else if ((LOWORD(wParam) == SCANCODE_CURSORBLOCKUP))
			{
				if(gOptions.IMEFunOn == 1 && MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS)
				{
					if(hIMEWnd != HWND_INVALID)
					{
						SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
						hIMEWnd=HWND_INVALID;						
					}
				}
				SelectPrevious();
				return 1;
			}
			else if ((LOWORD(wParam) == SCANCODE_F9) ||((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && (LOWORD(wParam)==SCANCODE_F11)))
			{
				if(gOptions.IMEFunOn == 1 && gOptions.TFTKeyLayout != 3 && MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS)
				{
					T9IMEWindow(hDlg, 0, 200, gOptions.LCDWidth, gOptions.LCDHeight, gOptions.HzImeOn);
				}
				return 1;
			}
			else if ((LOWORD(wParam) == SCANCODE_CURSORBLOCKRIGHT))
			{
				if(gOptions.IMEFunOn == 1 && gOptions.TFTKeyLayout == 3 && MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDC_WEB_ADDRESS)
				{
					T9IMEWindow(hDlg, 0, 200, gOptions.LCDWidth, gOptions.LCDHeight, gOptions.HzImeOn);
				}
				else if(MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDC_PROXY_ENABLED)
				{
					int index = SendMessage(MGUICTRLCurrent->MGUICtrl.ItemWnd, CB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hDlg, IDC_PROXY_ENABLED, CB_SPIN, 1-index, 0);
					ShowInput(hDlg, MGUICTRLCurrent, index);
				}
				else if(MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDC_WEB_MODEL)
				{
					int index = SendMessage(MGUICTRLCurrent->MGUICtrl.ItemWnd, CB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hDlg, IDC_WEB_MODEL, CB_SPIN, 1-index, 0);
					ShowInput(hDlg, MGUICTRLCurrent, index);
				}
				else
				{
					SelectNext();
				}
				return 1;
			}
			else if ((LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT) || (gOptions.TFTKeyLayout == 3 && LOWORD(wParam) == SCANCODE_BACKSPACE))
			{
				if(MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDC_PROXY_ENABLED)
				{
					int index = SendMessage(MGUICTRLCurrent->MGUICtrl.ItemWnd, CB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hDlg, IDC_PROXY_ENABLED, CB_SPIN, 1-index, 0);
					ShowInput(hDlg, MGUICTRLCurrent, index);
				}
				else if(MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDC_WEB_MODEL)
				{
					int index = SendMessage(MGUICTRLCurrent->MGUICtrl.ItemWnd, CB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hDlg, IDC_WEB_MODEL, CB_SPIN, 1-index, 0);
					ShowInput(hDlg, MGUICTRLCurrent, index);
				}
				else
				{
					SelectPrevious();
				}
				return 1;
			}
			else if ((LOWORD(wParam) == SCANCODE_ENTER) || (LOWORD(wParam) == SCANCODE_MENU) || (LOWORD(wParam) == SCANCODE_F10))
			{
				if(MGUICTRLCurrent->MGUICtrl.Ctrl.id == IDCANCEL)
				{
					SendMessage(hDlg, MSG_CLOSE, 0, 0);
				}
				else
				{
					if(WebOK(hDlg, MGUICTRLHeader))
						SendMessage(hDlg, MSG_CLOSE, 0, 0);
				}
				return 1;
			}
			else if ((LOWORD(wParam) == SCANCODE_ESCAPE))
			{
				SendMessage(hDlg, MSG_CLOSE, 0, 0);
				return 1;
			}
			break;
		}
		case MSG_CLOSE:
		{
			if(hIMEWnd != HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd=HWND_INVALID;
			}
			FreeWebControl(MGUICTRLHeader);
			//UnloadBitmap(&webbk);
			EndDialog (hDlg, IDCANCEL);
			break;
		}

	}

	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void CreateWebWindow(HWND hWnd)
{

	WebDlgBox.w = gOptions.LCDWidth;
	WebDlgBox.h = gOptions.LCDHeight;

	int i, Count = sizeof(WebCtrl)/sizeof(WebCtrl[0]), posX4 = 225 + gOptions.ControlOffset;
	for(i = Count; i > 0; i--)
	{
		if(WebCtrl[i].id == IDCANCEL)
		{
			WebCtrl[i].x = posX4;
			break;
		}
	}
	WebDlgBox.caption = LoadStrByIDDef(MID_WENSERVER_SETUP, "Web Setup");
	WebDlgBox.controls = WebCtrl;
	DialogBoxIndirectParam (&WebDlgBox, hWnd, WebDialogBoxProc, 0L);
}


//#endif
