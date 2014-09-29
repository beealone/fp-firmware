#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h> 
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <minigui/tftmullan.h>

#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "tftmsgbox.h"
#include "ssrcommon.h"

#define LB_FACE	100
#define ID_FACE 200
#define ID_SAVE 300
#define ID_EXIT 400

//BITMAP facesetbmp;
static HWND faceWnd[8];
static char *faceset[8];
static int faceItem,origValue[6];

static void LoadWinHint(void)
{
	faceset[0]=LoadStrByID(MID_FACE_VTH);
	faceset[1]=LoadStrByID(MID_FACE_MTH);
	faceset[2]=LoadStrByID(MID_FACE_REGMODE);
	faceset[3]=LoadStrByID(FACE_EXPOSOURE);
	faceset[4]=LoadStrByID(FACE_GLOBALGAIN);
	faceset[5]=LoadStrByID(MID_VIDEO_QULITY);
	faceset[6]=LoadStrByID(MID_FACE_FINGER);
	faceset[7]=LoadStrByID(MID_FACE);
}

static int IsChanged(void)
{
	char buf[16];
	int i;

	for(i=0;i<6;i++)
	{
		if(i==2)
		{
			if(SendMessage(faceWnd[i],CB_GETCURSEL,0,0) != origValue[i])
				return 1;
		}
		else
		{
			memset(buf,0,sizeof(buf));
			GetWindowText(faceWnd[i],buf,sizeof(buf));
			if(atoi(buf) != origValue[i])
				return 1;
		}
	}
	return 0;
}

static int SetFaceParamOK(HWND hWnd, HWND Item[])
{
	char buf[16];
	int err=0;
	int index=-1;

	//1:1
	memset(buf,0,sizeof(buf));
	GetWindowText(Item[0],buf,sizeof(buf));
	if(CheckText2(buf,55,120))
		gOptions.FaceVThreshold=atoi(buf);
	else
	{
		index=0;
		err++;
	}

	//1:G
	memset(buf,0,sizeof(buf));
	GetWindowText(Item[1],buf,sizeof(buf));
	if(CheckText2(buf,65,120))
		gOptions.FaceMThreshold=atoi(buf);
	else
	{
		index=1;
		err++;
	}
	// register  mode
	gOptions.FaceRegMode=SendMessage(Item[2], CB_GETCURSEL, 0, 0);

	//exposure
	memset(buf,0,sizeof(buf));
	GetWindowText(Item[3],buf,sizeof(buf));
	if(CheckText2(buf,80,0xFFFF))
		gOptions.FaceExposoure=atoi(buf);
	else
	{
		index=3;
		err++;
	}

	//gain
	memset(buf,0,sizeof(buf));
	GetWindowText(Item[4],buf,sizeof(buf));
	if(CheckText2(buf,30,0xFF))
		gOptions.VideoGain=atoi(buf);
	else
	{
		index=4;
		err++;
	}

	//quality
	memset(buf,0,sizeof(buf));
	GetWindowText(Item[5],buf,sizeof(buf));
	if(CheckText2(buf,30,0xFF))
		gOptions.VideQuality=atoi(buf);
	else
	{
		index=5;
		err++;
	}

	if(err)
	{
		LoadOptions(&gOptions);
		MessageBox1(hWnd, LoadStrByID(HIT_ERROR0), LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
		SetFocusChild(Item[index]);
		return 0;
	}
	else
	{
		SaveOptions(&gOptions);
		//MessageBox1 (hWnd, HIT_RIGHT, HIT_RUN, MB_OK| MB_ICONINFORMATION);
		SetFaceExtactParams();
	}
	return 1;
}

static void InitWindow (HWND hWnd)
{
	int i;
	int posX1,posX2;

	if (fromRight==1)
	{
		posX1=210;
		posX2=20;
	}
	else
	{
		posX1=5;
		posX2=110;
	}

	for(i=0;i<6;i++)
	{
		CreateWindow(CTRL_STATIC,faceset[i], WS_VISIBLE | SS_CENTER, LB_FACE+i, posX1, 23+28*i,290, 23, hWnd, 0);
		if(i==2)
		{
			faceWnd[i]=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | \
					CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,ID_FACE+i,posX2,20+28*i,180,23,hWnd,0);
			SendMessage(faceWnd[i],CB_ADDSTRING,0,(LPARAM)faceset[7]);
			SendMessage(faceWnd[i],CB_ADDSTRING,0,(LPARAM)faceset[6]);
		}
		else
		{
			faceWnd[i]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE | WS_BORDER | ES_AUTOSELECT,ID_FACE+i,\
					posX2,20+28*i,180,23,hWnd,0);
		}
		if(i==0 || i==1 || i==5)
			SendMessage(faceWnd[i],EM_LIMITTEXT,3,0L);
		if(i==3 || i==4)
			SendMessage(faceWnd[i],EM_LIMITTEXT,3,0L);
	}
	faceWnd[6]=CreateWindow(CTRL_BUTTON,LoadStrByID(MID_SAVE),WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,\
			ID_SAVE,10,190,85,23,hWnd,0);
	faceWnd[7]=CreateWindow(CTRL_BUTTON,LoadStrByID(MID_EXIT),WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,\
			ID_EXIT,220,190,85,23,hWnd,0);
}

static void InitWindowValue(void)
{
	int i;
	char buf[16];

	origValue[0]=gOptions.FaceVThreshold;
	origValue[1]=gOptions.FaceMThreshold;
	origValue[2]=gOptions.FaceRegMode;
	origValue[3]=gOptions.FaceExposoure;
	origValue[4]=gOptions.VideoGain;
	origValue[5]=gOptions.VideQuality;

	for(i=0;i<6;i++)
	{
		if(i==2)
			SendMessage(faceWnd[i],CB_SETCURSEL,origValue[i],0);
		else
		{
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%d",origValue[i]);
			SetWindowText(faceWnd[i],buf);
		}
	}
}

static int FaceSettingWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//	int id,nc;

	switch (message)	
	{
		case MSG_CREATE:
			//LoadBitmap(HDC_SCREEN,&facesetbmp,GetBmpPath("submenubg.jpg"));

			InitWindow(hWnd);
			InitWindowValue();
			UpdateWindow(hWnd,TRUE);			
			SetFocusChild(faceWnd[0]);
			faceItem=0;
			break;
		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;

				if (hdc == 0) 
				{
					hdc = GetClientDC (hWnd);
					fGetDC = TRUE;
				}
				if (clip) {
					rcTemp = *clip;
					ScreenToClient (hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient (hWnd, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect (hdc, &rcTemp);
				}
				FillBoxWithBitmap (hdc, 0, 0, 0, 0, get_submenubg_jgp());
				if (fGetDC)
					ReleaseDC (hdc);
				return 0;
			}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);		
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_KEYDOWN:
			{
				SetMenuTimeOut(time(NULL));
				if (gOptions.KeyPadBeep) ExKeyBeep();

				if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
				{
					if(++faceItem>7)
						faceItem=0;
					SetFocus(faceWnd[faceItem]);
				}
				else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
				{
					if(--faceItem<0)
						faceItem=7;
					SetFocus(faceWnd[faceItem]);
				}
				else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
				{
					int sel_mode;
					if(faceItem==2)
					{
						sel_mode=SendMessage(faceWnd[2],CB_GETCURSEL,0,0);
						if(sel_mode==0)
							sel_mode=1;
						else
							sel_mode=0;
						SendMessage(faceWnd[2],CB_SETCURSEL,sel_mode,0);
					}
				}
				else if((LOWORD(wParam)==SCANCODE_ESCAPE))
				{
					if(IsChanged())
					{
						if(MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),\
									MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
						{
							if(SetFaceParamOK(hWnd, faceWnd) && !ismenutimeout)
								PostMessage(hWnd, MSG_CLOSE,0,0);
						}
						else
						{
							if(!ismenutimeout) PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
					}
					else
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				else if((LOWORD(wParam)==SCANCODE_MENU) || (LOWORD(wParam)==SCANCODE_ENTER))
				{
					if(faceItem<7 && IsChanged())
					{
						if(SetFaceParamOK(hWnd, faceWnd) && !ismenutimeout)
							PostMessage(hWnd, MSG_CLOSE, 0, 0);
						else
							return 0;
					}
					else
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				return 0;
			}
		case MSG_CLOSE:
			//UnloadBitmap(&facesetbmp);
			//MainWindowCleanup (hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateFaceSettingWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	LoadWinHint();

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(FACE_FACESET);
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = FaceSettingWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hMainWnd);
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

