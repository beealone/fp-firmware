#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"
#include "exfun.h"
//#include "t9.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

#define CGP_STATIC 	0x90
#define CGP_ID		0xA0
#define CGP_MB		0xA1

#define ID_SAVE		0xA8
#define ID_EXIT		0xA9

HWND EdCGPID,EdMB[5];
HWND BtSave,BtExit;
HWND Edtmp,focuswnd;

//BITMAP cgpmngbk;
int cgpOptFlag;
static int bcgpchged=0;
int cgroupID;
TCGroup mytcgp;
char cgptmpstr[40]={0};
int focusidx=0;

static void SelectNext(WPARAM wParam)
{
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if(Edtmp == EdCGPID)
		{
			focusidx = 0;
			focuswnd = EdMB[focusidx];
		}
		else if(Edtmp == BtSave)
			focuswnd = BtExit;
		else if(Edtmp == BtExit)
		{
			if(cgpOptFlag)
			{
				focusidx = 0;
				focuswnd = EdMB[focusidx];
			}
			else
				focuswnd = EdCGPID;
		}
		else
		{
			if(++focusidx > 4)
				focuswnd = BtSave;
			else
				focuswnd = EdMB[focusidx];
		}
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == EdCGPID)
			focuswnd = BtExit;
		else if (Edtmp == BtExit)
			focuswnd = BtSave;
		else if (Edtmp == BtSave)
		{
			focusidx = 4;
			focuswnd = EdMB[focusidx];
		}
		else
		{
			if(--focusidx < 0)
			{
				if(cgpOptFlag)
					focuswnd = BtExit;
				else
					focuswnd = EdCGPID;
			}
			else
				focuswnd = EdMB[focusidx];
		}
	}

	SetFocusChild(focuswnd);
}

static void InitWindow (HWND hWnd)
{
	int i;
	char* cgpmngstr[5];
	int posX1,posX2,posX3;

	posX1=30;
	posX2=80;
	posX3=40;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		if(0 == gOptions.supportLCDType)
		{
			posX1=200+gOptions.ControlOffset;
			posX2=150+gOptions.ControlOffset;
			posX3=-120+gOptions.ControlOffset;
		}
		else if(1 == gOptions.supportLCDType)
		{
			posX1=250;
			posX2=200;
			posX3=-40;
		}	
	}
	else
	{
		if(0 == gOptions.supportLCDType)
		{
			posX1=10+gOptions.ControlOffset;
			posX2=60+gOptions.ControlOffset;
			posX3=-40+gOptions.ControlOffset;
		}
		else if(1 == gOptions.supportLCDType)
		{
			posX1=30;
			posX2=80;
			posX3=40;
		}
	}

	cgpmngstr[0]=LoadStrByID(MID_LOCK_GP);
	cgpmngstr[1]=LoadStrByID(MID_GROUP);
	cgpmngstr[2]=LoadStrByID(MID_MEMBER);
	cgpmngstr[3]=LoadStrByID(MID_SAVE);
	cgpmngstr[4]=LoadStrByID(MID_EXIT);
	
	
	CreateWindow(CTRL_STATIC, cgpmngstr[0],WS_VISIBLE | SS_LEFT,CGP_STATIC, posX1, 10, 60, 23, hWnd, 0);
	if(cgpOptFlag)
	{
		memset(cgptmpstr,0,sizeof(cgptmpstr));
		sprintf(cgptmpstr,"%02d",mytcgp.ID);
		CreateWindow(CTRL_STATIC, cgptmpstr, WS_VISIBLE | SS_LEFT, CGP_STATIC+1, posX2, 10, 90, 23, hWnd, 0);
	}
	else
	{
		EdCGPID = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				CGP_ID, posX2, 6, 45, 23, hWnd, 0);
		SendMessage(EdCGPID,EM_LIMITTEXT,2,0L);
	}

	CreateWindow(CTRL_STATIC, cgpmngstr[2],WS_VISIBLE | SS_LEFT,CGP_STATIC+2, posX1, 70, 60, 23, hWnd, 0);
	for(i=0;i<5;i++)
	{
		memset(cgptmpstr,0,sizeof(cgptmpstr));
		sprintf(cgptmpstr,"%s%d",cgpmngstr[1],i+1);
		CreateWindow(CTRL_STATIC, cgptmpstr, WS_VISIBLE | SS_CENTER, CGP_STATIC+3+i, posX2+8+posX3*i, 40, 40, 23, hWnd, 0);
		EdMB[i] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
                                CGP_MB, posX2+posX3*i, 66, 40, 23, hWnd, 0);
		SendMessage(EdMB[i],EM_LIMITTEXT,2,0L);
	}

	BtSave = CreateWindow(CTRL_BUTTON, cgpmngstr[3],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,ID_SAVE, 10, 190, 90, 23, hWnd, 0);
	BtExit = CreateWindow(CTRL_BUTTON, cgpmngstr[4],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,ID_EXIT, 220+gOptions.ControlOffset, 190, 90, 23, hWnd, 0);

	focusidx = 0;
}

static int ismodified(int OptMod)
{
	BYTE tvalue;
	int i;

	for(i=0;i<5;i++)
	{
		memset(cgptmpstr,0,sizeof(cgptmpstr));
		GetWindowText(EdMB[i],cgptmpstr,2);
		tvalue = (unsigned char)atoi(cgptmpstr);
		if(OptMod)	//edit
		{
			if(mytcgp.GroupID[i] != tvalue)
				return 1;
		}
		else		//add
		{
			if(tvalue!=0)
				return 1;
		}
		
	}
	return 0;

}

static int SaveCGroupInfo(HWND hWnd)
{
	int i;
	int tmvalue;
	int mbemptyflag;
	char mystr[80]={0};
	TGroup tgp;

	memset(&mytcgp,0,sizeof(TCGroup));
	if(cgpOptFlag)
		mytcgp.ID = cgroupID;
	else
	{
		memset(cgptmpstr,0,sizeof(cgptmpstr));
		GetWindowText(EdCGPID,cgptmpstr,2);
		if(cgptmpstr && atoi(cgptmpstr)>0)
		{
			cgroupID = atoi(cgptmpstr);
			if(FDB_GetCGroup(cgroupID,&mytcgp)!=NULL)
			{
				MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT12),LoadStrByID(MID_APPNAME),MB_OK|MB_ICONQUESTION|MB_BASEDONPARENT);
				if(!ismenutimeout) SetFocusChild(EdCGPID);
				return 0;
			}
			else
				mytcgp.ID = cgroupID;
		}
		else
		{
			MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT13), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONSTOP | MB_BASEDONPARENT);
			if(!ismenutimeout) SetFocusChild(EdCGPID);
			return 0;
		}
	}

	mbemptyflag=1;
	for(i=0;i<5;i++)
	{
		memset(cgptmpstr,0,sizeof(cgptmpstr));
		GetWindowText(EdMB[i],cgptmpstr,2);
		tmvalue = (unsigned char)atoi(cgptmpstr);
		if(tmvalue)
		{
			mbemptyflag=0;
			memset(&tgp,0,sizeof(TGroup));
			if(FDB_GetGroup(tmvalue,&tgp)==NULL)
			{
				memset(mystr,0,sizeof(mystr));
				sprintf(mystr,"%s%d%s",LoadStrByID(MID_GROUP),tmvalue,LoadStrByID(MID_SAVE_HINT15));
				if(MessageBox1(hWnd,mystr, LoadStrByID(MID_APPNAME), MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
					CreateGroupOptWindow(hWnd,0,&tmvalue);
				return 0;
			}
			mytcgp.MemberCount++;
		}
		mytcgp.GroupID[i] = tmvalue;
	}
	if(mbemptyflag)
	{
		MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT14), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONSTOP | MB_BASEDONPARENT);
		if(!ismenutimeout) SetFocusChild(EdMB[0]);
		return 0;
	}

	if(cgpOptFlag)
	{
		if(FDB_ChgCGroup(&mytcgp)==FDB_OK)
		{
			sync();
			return 1;
		}
	}
	else
	{
		if(FDB_AddCGroup(&mytcgp)==FDB_OK)
		{
			sync();
			return 1;
		}
	}

	return 0;

}

static void FillCGroupData(int OptMod)
{
	int i;

	if(OptMod)	//edit
	{
		for(i=0;i<5;i++)
		{
			memset(cgptmpstr,0,sizeof(cgptmpstr));
			sprintf(cgptmpstr,"%d",mytcgp.GroupID[i]);
			SetWindowText(EdMB[i],cgptmpstr);
		}

	}
	else		//add
	{
		memset(cgptmpstr,0,sizeof(cgptmpstr));
		sprintf(cgptmpstr,"%d",cgroupID);
		SetWindowText(EdCGPID,cgptmpstr);
		for(i=0;i<5;i++)
			SetWindowText(EdMB[i],"0");
	}
}

static U16 GetFreeCGroupID(U16 id)
{
        int i;

        for(i=id;i<=CGP_MAX;i++)
        {
                memset(&mytcgp,0,sizeof(TCGroup));
                if(FDB_GetCGroup(i,&mytcgp)==NULL)
                        return i;
        }
        return 0;

}

static int cgroupmngwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	char sstrc[50]={0};
	int i;
	int tmpvalue;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&cgpmngbk,GetBmpPath("submenubg.jpg")))
			//	return 0;

			InitWindow(hWnd);		//add controls
			FillCGroupData(cgpOptFlag);
			UpdateWindow(hWnd,TRUE);
			if(cgpOptFlag)
				SetFocusChild(EdMB[0]);
			else
				SetFocusChild(EdCGPID);
			break;

		case MSG_ERASEBKGND:
		{
	        HDC hdc = (HDC)wParam;
			const RECT* clip = (const RECT*)lParam;
			BOOL fGetDC = FALSE;
			RECT rcTemp;
			if(hdc == 0)
			{
				hdc = GetClientDC(hWnd);
				fGetDC = TRUE;
			}

			if(clip)
			{
				rcTemp = *clip;
				ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
				ScreenToClient(hWnd,&rcTemp.right, &rcTemp.bottom);
				IncludeClipRect(hdc, &rcTemp);
			}

			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
			if(fGetDC) ReleaseDC (hdc);
			return 0;
		}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);	
			EndPaint(hWnd,hdc);
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
			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			Edtmp = GetFocusChild(hWnd);
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
				PostMessage(hWnd, MSG_COMMAND, ID_EXIT, 0L);

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN || LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if(Edtmp!=BtExit && Edtmp!=BtSave)
				{
					memset(cgptmpstr, 0, sizeof(cgptmpstr));
					GetWindowText(Edtmp, cgptmpstr, 2);
					if(strlen(cgptmpstr)==0)
						break;
				}
				SelectNext(wParam);
				return 0;
			}
			if(((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) 
				|| (gOptions.TFTKeyLayout==3&&LOWORD(wParam)==SCANCODE_BACKSPACE)) 
				|| LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if(Edtmp==BtSave)
					SetFocusChild(BtExit);
				else if(Edtmp==BtExit)
					SetFocusChild(BtSave);
				break;	
			}

			if(LOWORD(wParam)==SCANCODE_ENTER)
			{
				if(Edtmp!=BtExit && Edtmp!=BtSave)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0);
			}

			if(LOWORD(wParam)==SCANCODE_F10)	
			{
				if(Edtmp!=BtExit)
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0);
				else
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_EXIT, 0);
			}

			if(LOWORD(wParam)==SCANCODE_MENU)
				PostMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);

			break;
	
		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(nc==EN_CHANGE)
			{
				if(id == CGP_ID)
				{
					memset(sstrc,0,sizeof(sstrc));
					GetWindowText(EdCGPID,sstrc,2);
					tmpvalue = atoi(sstrc);
					if(tmpvalue>CGP_MAX)
					{
						memset(sstrc,0,sizeof(sstrc));
						sprintf(sstrc,"%d",GetFreeCGroupID(1));
						SetWindowText(EdCGPID,sstrc);
						SendMessage(EdCGPID,EM_SETCARETPOS,0,strlen(sstrc));
					}
				}
				else
				{
					for(i=0;i<5;i++)
					{
						if(id==CGP_MB+i)
						{
							memset(sstrc,0,sizeof(sstrc));
							GetWindowText(EdMB[i],sstrc,2);
							tmpvalue = atoi(sstrc);
							if(tmpvalue<0 || tmpvalue>GP_MAX)
							{
								SetWindowText(EdMB[i],"0");
								SendMessage(EdMB[i],EM_SETCARETPOS,0,1);
							}
						}
					}
				}
					
			}

			switch(id)
			{
				case ID_SAVE:	
					if(SaveCGroupInfo(hWnd))	
					{
						bcgpchged = 1;
						PostMessage(hWnd,MSG_CLOSE,0,0);
					}
					break;

				case ID_EXIT:				
	                if(ismodified(cgpOptFlag) && 
						MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
                                    MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
                        PostMessage(hWnd, MSG_COMMAND, ID_SAVE, 0);
       	            else
					{
						if(!ismenutimeout)
							PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
	                break;
			}			

			break;

		case MSG_CLOSE:
			//UnloadBitmap(&cgpmngbk);
			DestroyMainWindow(hWnd);
			return 0;
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateCGroupOptWindow(HWND hWnd, int optmod, int *cgpid)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	char sstr[50]={0};

	cgpOptFlag = optmod;
	bcgpchged = 0;
	memset(sstr,0,sizeof(sstr));
	switch(cgpOptFlag)
	{
		case 0:	//add group
			cgroupID = (*cgpid) ? *cgpid:GetFreeCGroupID(1);
			sprintf(sstr,"%s",LoadStrByID(MID_LOCKGP_ADD));
			break;
		case 1:	//edit group
			cgroupID = *cgpid;
			memset(&mytcgp,0,sizeof(TCGroup));
			if(FDB_GetCGroup(cgroupID,&mytcgp)==NULL) 
				return 0;
			sprintf(sstr,"%s",LoadStrByID(MID_LOCKGP_EDIT));
			break;
	}
	
	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = sstr;
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = cgroupmngwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
        CreateInfo.by = gOptions.LCDHeight;
	//CreateInfo.iBkColor = COLOR_lightgray;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);
	
	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	*cgpid=cgroupID;
	MainWindowThreadCleanup(hMainWnd);
	MiniGUIExtCleanUp ();
	return bcgpchged;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

