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

#define DAM_STATIC 	0x60
#define DAM_ID		0x70
#define ID_SAVE		0x80
#define ID_EXIT		0x81

HWND EdDAM[5];
HWND Edtmp,focuswnd;

//static BITMAP gpmngbk1;
int actindex;
int oldvalue[5];
int parameter[5];

static void InitWindow (HWND hWnd)
{
	int i, j;//Liaozz 20081008 fix bug third 6, add j to count Y coordinate of controls in order to process only rf machine
	char tmpstr[20];
	char* hintstr[10];
	int posX1,posX2, posX3;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=200+gOptions.ControlOffset;
		posX2=100+gOptions.ControlOffset;
		posX3=20+gOptions.ControlOffset;
	}
	else
	{
		posX1=20+gOptions.ControlOffset;
		posX2=129+gOptions.ControlOffset;
		posX3=225+gOptions.ControlOffset;
	}

        hintstr[0]=LoadStrByID(MID_AD_DURESSHELP);    //按键求助
        hintstr[1]=LoadStrByID(MID_AD_DURESS11);    //1:1验证报警
        hintstr[2]=LoadStrByID(MID_AD_DURESS1N);    //1:N验证报警
        hintstr[3]=LoadStrByID(MID_AD_DURESSPWD);    //密码验证报警
        hintstr[4]=LoadStrByID(MID_AD_DURESSAD);    //报警延时
        hintstr[5]=LoadStrByID(HID_YES);     //是
        hintstr[6]=LoadStrByID(HID_NO);     //否
        hintstr[7]=LoadStrByID(MID_SAVE);
        hintstr[8]=LoadStrByID(MID_EXIT);
        hintstr[9]=LoadStrByID(MID_SECOND);

	//Set parameter value 
        parameter[0]=gOptions.DuressHelpKeyOn;
        parameter[1]=gOptions.Duress1To1;
        parameter[2]=gOptions.Duress1ToN;
        parameter[3]=gOptions.DuressPwd;
        parameter[4]=gOptions.DuressAlarmDelay;

	for(i=0;i<5;i++)
	{
		//Liaozz 20081008 fix bug third 6,
		//if only rf machine , will not create the control for duress 1:1 and duress 1:N
		//and will use j to count Y coordinate to create the other control
		//e.g.: 20 + (30*j)
		j = i;
		if (gOptions.IsOnlyRFMachine) {
			if (i > 2)
				j = i-2;
			if(i == 1 || i == 2)
				continue;
		}
		//Liaozz end
		CreateWindow(CTRL_STATIC, hintstr[i], WS_VISIBLE | SS_LEFT, DAM_STATIC+i, posX1, 20 + (30*j), 120, 23, hWnd, 0);
		if(i==4)
		{
			EdDAM[i] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
                                DAM_ID+i, posX2, 20+(30*j)-4, 72, 23, hWnd, 0);
			sprintf(tmpstr,"(0-255)%s",hintstr[9]);
			CreateWindow(CTRL_STATIC, tmpstr, WS_VISIBLE | SS_LEFT, DAM_STATIC+i+1, posX3, 20+(30*j), 90, 23, hWnd, 0);
			SendMessage(EdDAM[i], EM_LIMITTEXT, 3, 0);
		}
		else
		{
			EdDAM[i] = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
                                DAM_ID+i, posX2-9, 20+(30*j)-4, 90, 23, hWnd, 0);
			SendMessage(EdDAM[i], CB_ADDSTRING, 0, (LPARAM)hintstr[6]);
			SendMessage(EdDAM[i], CB_ADDSTRING, 0, (LPARAM)hintstr[5]);
		}
	}

	EdDAM[5] = CreateWindow(CTRL_BUTTON,hintstr[7], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
				ID_SAVE, 10, 190, 90, 23, hWnd, 0);
	EdDAM[6] = CreateWindow(CTRL_BUTTON, hintstr[8], WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
				ID_EXIT, 220+gOptions.ControlOffset, 190, 90, 23, hWnd, 0);

}

static void InitWindowText(HWND hWnd)
{
	int i;
	char tmpchar[10];

	for(i=0;i<5;i++)
	{
		//Liaozz 20081008 fix bug third 6
		if (gOptions.IsOnlyRFMachine && (i == 1 || i == 2))
			continue;
		//Liaozz end
        	oldvalue[i] = parameter[i];
		if(i!=4)
		{
			SendMessage(EdDAM[i], CB_SETCURSEL, oldvalue[i], 0);
		}
		else
		{
			memset(tmpchar, 0, 10);
			sprintf(tmpchar,"%d",oldvalue[i]);
			SetWindowText(EdDAM[i], tmpchar);
		}
	}
	
}

static int ismodified(void)
{
	int i;
	char tmpchar[10];

	for(i=0;i<5;i++)
	{
		//Liaozz 20081008 fix bug third 6
		if (gOptions.IsOnlyRFMachine && (i == 1 || i == 2))
			continue;
		//Liaozz end
		if(i!=4)
		{
			if(oldvalue[i] != SendMessage(EdDAM[i], CB_GETCURSEL, 0, 0))
				return 1;
		}
		else
		{
			memset(tmpchar,0,10);
			GetWindowText(EdDAM[i],tmpchar,10);
			if(oldvalue[i] != atoi(tmpchar))
				return 1;
		}
	}
	return 0;
}

static void SaveDuressParameter(HWND hWnd)
{
	int i;
	int tmpvalue[5];
	char tmpchar[10];

	for(i=0;i<5;i++)
	{
		//Liaozz 20081008 fix bug third 6
		if (gOptions.IsOnlyRFMachine && (i == 1 || i == 2))
			continue;
		//Liaozz end
		if(i!=4)
		{
			tmpvalue[i] = SendMessage(EdDAM[i], CB_GETCURSEL, 0, 0);
		}
		else
		{
			memset(tmpchar,0,10);
			GetWindowText(EdDAM[i],tmpchar,10);
			tmpvalue[i] = atoi(tmpchar);
		}
	}
        gOptions.DuressHelpKeyOn=tmpvalue[0];
	//Liaozz 20081008 fix bug third 6
	if (!gOptions.IsOnlyRFMachine) {
        gOptions.Duress1To1=tmpvalue[1];
        gOptions.Duress1ToN=tmpvalue[2];
	}
	//Liaozz end
        gOptions.DuressPwd=tmpvalue[3];
        gOptions.DuressAlarmDelay=tmpvalue[4];

	SaveOptions(&gOptions);
	LoadOptions(&gOptions);
}

static int duressparawinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	char sstr[10];
	int tmpvalue;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&gpmngbk1,GetBmpPath("submenubg.jpg")))
	                //        return 0;

			InitWindow(hWnd);		//add controls
			InitWindowText(hWnd);
			UpdateWindow(hWnd,TRUE);
			actindex=0;
			SetFocusChild(EdDAM[actindex]);
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

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				PostMessage(hWnd, MSG_COMMAND, ID_EXIT, 0L);
			}

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if(actindex==4)
				{
					memset(sstr,0,10);
					GetWindowText(EdDAM[actindex],sstr,3);
					if(strlen(sstr)==0)
						return 0;
				}
				if(++actindex > 6) actindex = 0;
				//Liaozz 20081008 fix bug third 6
				if (gOptions.IsOnlyRFMachine) {
					if (actindex == 1 || actindex == 2)
						actindex = 3;
				}
				//Liaozz end
				SetFocusChild(EdDAM[actindex]);
				return 0;
			}

			if( LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if(actindex==4)
				{
					memset(sstr,0,10);
					GetWindowText(EdDAM[actindex],sstr,3);
					if(strlen(sstr)==0)
						return 0;
				}
				if(--actindex < 0) actindex = 6;
				//Liaozz 20081008 fix bug third 6
				if (gOptions.IsOnlyRFMachine) {
					if (actindex == 1 || actindex == 2)
						actindex = 0;
				}
				//Liaozz end
				SetFocusChild(EdDAM[actindex]);
				return 0;
			}

			if(LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT || (gOptions.TFTKeyLayout == 3 && LOWORD(wParam) == SCANCODE_BACKSPACE))
			{
				if(actindex<4)
				{
					tmpvalue=SendMessage(EdDAM[actindex], CB_GETCURSEL, 0, 0);
					if(--tmpvalue < 0) tmpvalue = 1;
					SendMessage(EdDAM[actindex], CB_SETCURSEL, tmpvalue, 0);
					return 0;
				}
				else if(actindex>4)
				{
					if(--actindex < 0) actindex = 6;
					SetFocusChild(EdDAM[actindex]);
					return 0;
				}
			}

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if(actindex<4)
				{
					tmpvalue=SendMessage(EdDAM[actindex], CB_GETCURSEL, 0, 0);
					if(++tmpvalue > 1) tmpvalue = 0;
					SendMessage(EdDAM[actindex], CB_SETCURSEL, tmpvalue, 0);
					return 0;
				}
				else if(actindex>4)
				{
					if(++actindex > 6) actindex = 0;
					SetFocusChild(EdDAM[actindex]);
					return 0;
				}
			}

			if(LOWORD(wParam)==SCANCODE_ENTER)
			{
				if(actindex<5)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0);
			}

			if(LOWORD(wParam)==SCANCODE_F10)
			{
				if(actindex!=6)
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
				else
					PostMessage(hWnd, MSG_COMMAND, (WPARAM)ID_EXIT, 0L);
			}

			if(LOWORD(wParam)==SCANCODE_MENU)
				PostMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);

			break;
	
		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(nc==EN_CHANGE)
			{
				if(id == DAM_ID+4)
				{
					memset(sstr,0,5);
					GetWindowText(EdDAM[actindex],sstr,3);
					tmpvalue = atoi(sstr);
					if(tmpvalue>255)
					{
						SetWindowText(EdDAM[actindex],"0");
						SendMessage(EdDAM[actindex],EM_SETCARETPOS,0,1);
					}
				}
			}

			switch(id)
			{
				case ID_SAVE:
					SaveDuressParameter(hWnd);	
			                LoadOptions(&gOptions);
			                MessageBox1(hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
					if(!ismenutimeout) PostMessage(hWnd,MSG_CLOSE,0,0);
					break;

				case ID_EXIT:				
	                                if(ismodified() && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
                                                	MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
                                                SendMessage(hWnd, MSG_COMMAND, ID_SAVE, 0);
					if(!ismenutimeout) PostMessage(hWnd,MSG_CLOSE,0,0L);
	                                break;
			}			

			break;

		case MSG_CLOSE:
			//UnloadBitmap(&gpmngbk1);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int DuressParameterWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_LOCK_OP6);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = duressparawinproc;
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

	MainWindowThreadCleanup(hMainWnd);
	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

