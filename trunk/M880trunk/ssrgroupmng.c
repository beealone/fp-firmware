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
#include "t9.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

#define GP_STATIC 	0x50
#define GP_ID		0x60
#define GP_VR		0x61
#define GP_HV		0x62
#define GP_TZ		0x63
#define ID_SAVE		0x68
#define ID_EXIT		0x69

HWND LbID,LbHDValid;
HWND EdGpID,CbGpVerify,CbHDValid;
HWND EdGpTZ[3];
static HWND BtSave,BtExit;
static HWND Edtmp,focuswnd;

//BITMAP gpmngbmpbk;
int gpOptFlag;
int bgpchged;
int groupID;
TGroup mytgp;
char gptmpstr[20];
int gvsel=0;
int ghdsel=0;

extern char* myVerifyStyle[];
static int  SelectNext(WPARAM wParam)
{
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == EdGpID)
		{
			if(gOptions.UserExtendFormat)
				focuswnd = CbGpVerify;
			else
				focuswnd = CbHDValid;
		}
		else if (Edtmp == CbGpVerify)
			focuswnd = CbHDValid;
		else if (Edtmp == CbHDValid)
			focuswnd = EdGpTZ[0];
		else if (Edtmp == EdGpTZ[0])
			focuswnd = EdGpTZ[1];
		else if (Edtmp == EdGpTZ[1])
			focuswnd = EdGpTZ[2];
		else if (Edtmp == EdGpTZ[2])
			focuswnd = BtSave;
		else if (Edtmp == BtSave)
			focuswnd = BtExit;
		else if (Edtmp == BtExit)
		{
			if(gpOptFlag)
			{
				if(gOptions.UserExtendFormat)
					focuswnd = CbGpVerify;
				else
					focuswnd = CbHDValid;
			}
			else
				focuswnd = EdGpID;
		}
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == EdGpID)
			focuswnd = BtExit;
		else if (Edtmp == BtExit)
			focuswnd = BtSave;
		else if (Edtmp == BtSave)
			focuswnd = EdGpTZ[2];
		else if (Edtmp == EdGpTZ[2])
			focuswnd = EdGpTZ[1];
		else if (Edtmp == EdGpTZ[1])
			focuswnd = EdGpTZ[0];
		else if (Edtmp == EdGpTZ[0])
			focuswnd = CbHDValid;
		else if (Edtmp == CbHDValid)
		{
			if(gOptions.UserExtendFormat)
				focuswnd = CbGpVerify;
			else
				focuswnd = (gpOptFlag)?BtExit:EdGpID;
		}
		else if (Edtmp == CbGpVerify)
			focuswnd = (gpOptFlag)?BtExit:EdGpID;
	}

	SetFocusChild(focuswnd);
	return 1;
}

static void InitWindow (HWND hWnd)
{
	int i;
	char* gpmngstr[8];
	int posX1,posX2,posX3;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=180+gOptions.ControlOffset;
		posX2=80+gOptions.ControlOffset;
		posX3=25+gOptions.ControlOffset;
	}
	else
	{
		posX1=10+gOptions.ControlOffset;
		posX2=90+gOptions.ControlOffset;
		posX3=0+gOptions.ControlOffset;
	}
	gpmngstr[0]=LoadStrByID(MID_GP_INDEX);
	gpmngstr[1]=LoadStrByID(MID_USER_VRTP);
	gpmngstr[2]=LoadStrByID(MID_GP_HD);
	gpmngstr[3]=LoadStrByID(MID_USER_TZ);
	gpmngstr[4]=LoadStrByID(MID_HD_INVALID);
	gpmngstr[5]=LoadStrByID(MID_HD_VALID);
	gpmngstr[6]=LoadStrByID(MID_SAVE);
	gpmngstr[7]=LoadStrByID(MID_EXIT);
	
	CreateWindow(CTRL_STATIC, gpmngstr[0],WS_VISIBLE | SS_LEFT,GP_STATIC, posX1, 10, 90, 23, hWnd, 0);
	if(gpOptFlag)
	{
		memset(gptmpstr,0,20);
		sprintf(gptmpstr,"%02d",mytgp.ID);
		CreateWindow(CTRL_STATIC, gptmpstr, WS_VISIBLE | SS_LEFT, GP_STATIC+1, posX2, 10, 90, 23, hWnd, 0);
	}
	else
	{
		EdGpID = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				GP_ID, posX2+posX3, 6, 45, 23, hWnd, 0);
		SendMessage(EdGpID,EM_LIMITTEXT,2,0L);
	}

	if(gOptions.UserExtendFormat)
	{
		CreateWindow(CTRL_STATIC, gpmngstr[1],WS_VISIBLE | SS_LEFT,GP_STATIC+2, posX1, 40, 90, 23, hWnd, 0);
		CbGpVerify=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
	                                GP_VR, posX2-10, 36, 220, 23, hWnd, 0);
	        for(i=0; i<VS_NUM; i++)
        	{
	                SendMessage(CbGpVerify,CB_ADDSTRING,0,(LPARAM)myVerifyStyle[i]);
        	}

		CreateWindow(CTRL_STATIC, gpmngstr[2],WS_VISIBLE | SS_LEFT,GP_STATIC+3, posX1, 70, 90, 23, hWnd, 0);
		CbHDValid=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
                          	      GP_HV, posX2-10, 66, 90, 23, hWnd, 0);
		SendMessage(CbHDValid,CB_ADDSTRING,0,(LPARAM)gpmngstr[4]);
        	SendMessage(CbHDValid,CB_ADDSTRING,0,(LPARAM)gpmngstr[5]);

		for(i=0;i<3;i++)
		{
			memset(gptmpstr,0,20);
			sprintf(gptmpstr,"%s%d",gpmngstr[3],i+1);
			CreateWindow(CTRL_STATIC, gptmpstr, WS_VISIBLE | SS_LEFT, GP_STATIC+3+i, posX1, 100+30*i, 90, 23, hWnd, 0);
			EdGpTZ[i] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
					GP_TZ+i, posX2+posX3, 100+30*i-4, 45, 23, hWnd, 0);
			SendMessage(EdGpTZ[i],EM_LIMITTEXT,2,0L);
		}
	}
	else
	{
                CreateWindow(CTRL_STATIC, gpmngstr[2],WS_VISIBLE | SS_LEFT,GP_STATIC+3, posX1, 40, 90, 23, hWnd, 0);
                CbHDValid=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
                                      GP_HV, posX2-10, 36, 90, 23, hWnd, 0);
                SendMessage(CbHDValid,CB_ADDSTRING,0,(LPARAM)gpmngstr[4]);
                SendMessage(CbHDValid,CB_ADDSTRING,0,(LPARAM)gpmngstr[5]);

                for(i=0;i<3;i++)
                {
                        memset(gptmpstr,0,20);
                        sprintf(gptmpstr,"%s%d",gpmngstr[3],i+1);
                        CreateWindow(CTRL_STATIC, gptmpstr, WS_VISIBLE | SS_LEFT, GP_STATIC+3+i, posX1, 70+30*i, 90, 23, hWnd, 0);
                        EdGpTZ[i] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
                                        GP_TZ+i, posX2+posX3, 70+30*i-4, 45, 23, hWnd, 0);
                        SendMessage(EdGpTZ[i],EM_LIMITTEXT,2,0L);
                }
		
	}
	BtSave = CreateWindow(CTRL_BUTTON, gpmngstr[6],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
				ID_SAVE, 10, 190, 90, 23, hWnd, 0);
	BtExit = CreateWindow(CTRL_BUTTON, gpmngstr[7],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
				ID_EXIT, 220+gOptions.ControlOffset, 190, 90, 23, hWnd, 0);

}

static int ismodified(int OptMod)
{
	BYTE tmpvalue;
	int i;

	if(OptMod)	//edit
	{
		tmpvalue=mytgp.VerifyStyle;
		if(gOptions.UserExtendFormat && gvsel!=(tmpvalue & 0x7F))
			return 1;
		if(ghdsel!=((tmpvalue>>7) & 0x01))
			return 1;

		for(i=0;i<3;i++)
		{
			memset(gptmpstr,0,5);
			GetWindowText(EdGpTZ[i],gptmpstr,2);
			if(atoi(gptmpstr)!=mytgp.TZID[i])
				return 1;
		}
	}
	else
	{
		//Liaozz 20080926 fix bug second 37 is modify
		if(gOptions.UserExtendFormat && gvsel) {
			if (gOptions.IsOnlyRFMachine && gvsel == 2)
				return 0;
			else
				return 1;
		}
		//Liaozz end
		if(ghdsel) return 1;

		for(i=0;i<3;i++)
		{
			memset(gptmpstr,0,5);
			GetWindowText(EdGpTZ[i],gptmpstr,2);
			if(atoi(gptmpstr)!=0)
				return 1;
		}
	}
	return 0;

}

static int SaveGroupInfo(HWND hWnd)
{
	int i;
	int tmpvalue;
	int tzemptyflag;

	memset(&mytgp,0,sizeof(TGroup));
	if(gpOptFlag)
		mytgp.ID = groupID;
	else
	{
		memset(gptmpstr,0,5);
		GetWindowText(EdGpID,gptmpstr,2);
		if(gptmpstr && atoi(gptmpstr)>0 && atoi(gptmpstr)<=GP_MAX) 
		{
			groupID = atoi(gptmpstr);
			if(FDB_GetGroup(groupID,&mytgp)!=NULL)
			{
				MessageBox1(hWnd, LoadStrByID(MID_SAVE_HINT8), LoadStrByID(MID_APPNAME),
						MB_OK | MB_ICONQUESTION | MB_BASEDONPARENT);
				if(!ismenutimeout) SetFocusChild(EdGpID);
				return 0;
			}
			else
				mytgp.ID = groupID;
		}
		else
		{
			MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT7), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONSTOP | MB_BASEDONPARENT);
			if(!ismenutimeout) SetFocusChild(EdGpID);
			return 0;
		}
	}

	//Liaozz 20080926 fix bug second 37 SaveGroupInfo
//	tmpvalue = (gOptions.UserExtendFormat)?gvsel:0;
	if (gOptions.UserExtendFormat)
		tmpvalue = gvsel;
	else {
		if (gOptions.IsOnlyRFMachine)
			tmpvalue = 2;
		else
			tmpvalue = 0;
	}
//Liaozz end
	if(ghdsel)	//节假日有效
		tmpvalue |= 0x80;
	mytgp.VerifyStyle = tmpvalue;
//	printf("VerifyStyle:%d\n",tmpvalue);
	
	tzemptyflag=1;
	for(i=0;i<3;i++)
	{
		memset(gptmpstr,0,5);
		GetWindowText(EdGpTZ[i],gptmpstr,2);
		tmpvalue=atoi(gptmpstr);
		if(strlen(gptmpstr)>0 && tmpvalue>=0 && tmpvalue<=TZ_MAX)
		{
			if(tmpvalue && FDB_GetTimeZone(tmpvalue,NULL)==NULL)
			{
				if(MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT2), LoadStrByID(MID_APPNAME),
						MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
					TimeZoneMngWindow(hWnd,tmpvalue,1);               //add new time zone
				return 0;
			}
			else
			{
				mytgp.TZID[i]=tmpvalue;
				if(tmpvalue>0) tzemptyflag=0;
			}
		}
		else
		{
			MessageBox1(hWnd, LoadStrByID(HIT_ERROR0),LoadStrByID(MID_APPNAME),MB_OK|MB_ICONSTOP|MB_BASEDONPARENT);
			if(!ismenutimeout) SetFocusChild(EdGpTZ[i]);			
			return 0;
		}
	}

	if(tzemptyflag)
	{
		MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT9), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONSTOP | MB_BASEDONPARENT);		
		if(!ismenutimeout) SetFocusChild(EdGpTZ[0]);
		return 0;
	}

	if(gpOptFlag)
	{
		if(FDB_ChgGroup(&mytgp)==FDB_OK)
		{
			sync();
			return 1;
		}
	}
	else
	{
		if(FDB_AddGroup(&mytgp)==FDB_OK)
		{
			sync();
			return 1;
		}
	}

	return 0;

}

static void FillGroupData(int OptMod)
{
	int i;
	int tmpvalue;

	if(OptMod)	//edit
	{
		tmpvalue = mytgp.VerifyStyle;
		gvsel = tmpvalue & 0x7F;
		ghdsel = (tmpvalue>>7) & 0x01;
		
		for(i=0;i<3;i++)
		{
			memset(gptmpstr,0,5);
			sprintf(gptmpstr,"%02d",mytgp.TZID[i]);
			SetWindowText(EdGpTZ[i],gptmpstr);
		}
	}
	else		//add
	{
		memset(gptmpstr,0,5);
		sprintf(gptmpstr,"%d",groupID);
		SetWindowText(EdGpID,gptmpstr);
		gvsel=0;
		//Liaozz 20080926 fix bug second 37 init the combobox default
		if (gOptions.IsOnlyRFMachine)
			gvsel = 2;
		//Liaozz end
		ghdsel=0;
		for(i=0;i<3;i++)
			SetWindowText(EdGpTZ[i],"00");
	}

	if(gOptions.UserExtendFormat)
		SendMessage(CbGpVerify, CB_SETCURSEL, gvsel, 0);
	SendMessage(CbHDValid, CB_SETCURSEL, ghdsel, 0);

}

static U16 GetFreeGroupID(U16 id)
{
        int i;

        for(i=id;i<=GP_MAX;i++)
        {
                memset(&mytgp,0,sizeof(TGroup));
                if(FDB_GetGroup(i,&mytgp)==NULL)
                        return i;
        }
        return 0;

}

static int groupmngwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	//char sstr[20];
	//int i;
	//int tmpvalue;
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&gpmngbmpbk,GetBmpPath("submenubg.jpg")))
	                 //       return 0;

			InitWindow(hWnd);		//add controls
			FillGroupData(gpOptFlag);
			UpdateWindow(hWnd,TRUE);
			if(gpOptFlag)
			{
				if(gOptions.UserExtendFormat)
					SetFocusChild(CbGpVerify);
				else
					SetFocusChild(CbHDValid);
			}
			else
				SetFocusChild(EdGpID);
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
				SelectNext(wParam);
				return 0;
			}
			
			if(LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT || (gOptions.TFTKeyLayout == 3 && LOWORD(wParam) == SCANCODE_BACKSPACE))
			{
                                if(Edtmp==BtExit)
                                        SetFocusChild(BtSave);
                                else if(Edtmp==CbGpVerify)
                                {
                                        if(--gvsel<0) gvsel=VS_NUM-1;
                                        //Liaozz 20080926 fix bug second 37 group mng left
       					if (gOptions.IsOnlyRFMachine) {
       						if (gvsel == 14 || gvsel == 13 || gvsel == 12)
       							gvsel = 11;
       						if (gvsel == 10 || gvsel == 9 || gvsel == 8)
       							gvsel = 7;
       						if (gvsel == 6 || gvsel == 5)
       							gvsel = 4;
       						if (gvsel == 1 || gvsel == 0)
       							gvsel = 11;
       					}
       					//Liaozz end
                                        SendMessage(CbGpVerify, CB_SETCURSEL, gvsel ,0);
					return 0;
                                }
                                else if(Edtmp==CbHDValid)
                                {
                                        if(ghdsel==1) ghdsel=0;
					else ghdsel=1;
                                        SendMessage(CbHDValid, CB_SETCURSEL, ghdsel, 0);
					return 0;
                                }
				break;
			}

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if(Edtmp==BtSave)
					SetFocusChild(BtExit);
				else if(Edtmp==CbGpVerify)
				{
					if(++gvsel>VS_NUM-1) gvsel=0;
					//Liaozz 20080926 fix bug second 37 group mng right
					if (gOptions.IsOnlyRFMachine) {
						if (gvsel == 0 || gvsel == 1)
							gvsel = 2;
						if (gvsel == 5 || gvsel == 6)
							gvsel = 7;
						if (gvsel == 8 || gvsel == 9 || gvsel == 10)
							gvsel = 11;
						if (gvsel == 12 || gvsel == 13 || gvsel == 14)
							gvsel = 2;
					}
					//Liaozz end
					SendMessage(CbGpVerify, CB_SETCURSEL, gvsel ,0);
					return 0;
				}
				else if(Edtmp==CbHDValid)
				{
					if(ghdsel==1) ghdsel=0;
					else ghdsel=1;
					SendMessage(CbHDValid, CB_SETCURSEL, ghdsel, 0);
					return 0;
				}
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
			{
				PostMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);	
			}

			break;
	
		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
#if 0
			if(nc==EN_CHANGE)
			{
				if(id==GP_ID)
				{
					memset(sstr,0,5);
					GetWindowText(EdGpID,sstr,2);
					if(strlen(sstr)>0)
					{	
						if(atoi(sstr)>GP_MAX)
						{
							memset(sstr,0,5);
							sprintf(sstr,"%d",GetFreeGroupID(1));
							SetWindowText(EdGpID,sstr);
						}
					}
				}
				else
				{
					for(i=0;i<3;i++)
					{
						if(id==GP_TZ+i)
						{
							memset(sstr,0,5);
							GetWindowText(EdGpTZ[i],sstr,2);
							tmpvalue = atoi(sstr);
							if(tmpvalue<0 || tmpvalue>TZ_MAX)
								SetWindowText(EdGpTZ[i],"0");
						}
					}
				}
					
			}
#endif
			switch(id)
			{
				case ID_SAVE:	
					if(SaveGroupInfo(hWnd))	
					{
						bgpchged=1;
						PostMessage(hWnd,MSG_CLOSE,0,0);
					}
					break;

				case ID_EXIT:				
	                                if(ismodified(gpOptFlag) && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
                                                	MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
                                                PostMessage(hWnd, MSG_COMMAND, ID_SAVE, 0);
                                	else
						if(!ismenutimeout) PostMessage(hWnd,MSG_CLOSE,0,0L);
	                                break;
			}			

			break;

		case MSG_CLOSE:
			//UnloadBitmap(&gpmngbmpbk);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateGroupOptWindow(HWND hWnd, int optmod, int *gpid)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	//char sstr[10];
	char sstr[20];

	gpOptFlag = optmod;
	bgpchged=0;
	memset(sstr,0,sizeof(sstr));
	if(gpOptFlag==0)	//add
	{
		groupID = (*gpid) ? *gpid:GetFreeGroupID(1);
		sprintf(sstr,"%s",LoadStrByID(MID_GP_ADD));
	}
	else			//edit
	{
		groupID = *gpid;
		memset(&mytgp,0,sizeof(TGroup));
		if(FDB_GetGroup(groupID,&mytgp)==NULL) 
			return 0;
		sprintf(sstr,"%s",LoadStrByID(MID_GP_EDIT));
	}
	
	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = sstr;
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = groupmngwinproc;
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

	*gpid=groupID;
	MainWindowThreadCleanup(hMainWnd);
	return bgpchged;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

