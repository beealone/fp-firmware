#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <minigui/tftmullan.h>

#include <string.h>
#include "ssrcommon.h"
#include "options.h"
#include "flashdb.h"
#include "ssrpub.h"
#include "exfun.h"
#include "main.h"
#include "libdlcl.h"


#ifdef _TTS_
#include "tts/tts.h"
#endif


#define IDC_DTFMT    320
#define IDC_FZ       321
#define IDC_FZ11     322
#define IDC_NUMBER   323
#define IDC_INPUTID  324
#define IDC_KEYSND   325
#define IDC_SND      326
#define IDC_MIXIR    327
#define IDC_MGP	     328
#define IDC_MANREC   330
#define IDC_ATTREC   331
#define IDC_RETIME   332
#define IDC_IR_BL_S  333
#define IDC_FPVERSION    334

static HWND System1ItemWnd[14];
static int System1Item;
//static BITMAP system1bk;
int tmpparam[11];
char* System1Str[25];

extern char *DtFmtStr[];

extern int CreateLicenseWindow(HWND hWnd);
static int getheight(int rows, int offset)
{
	return (10+30*rows-offset);
}

void LoadSystem1Str()
{
	System1Str[0]=LoadStrByID(HIT_SYSTEM1SET1);
	System1Str[1]=LoadStrByID(HIT_SYSTEM1SET2);
	System1Str[4]=LoadStrByID(HIT_SYSTEM1SET5);
	System1Str[5]=LoadStrByID(HIT_SYSTEM1SET6);
	System1Str[6]=LoadStrByID(HIT_SYSTEM1SET7);
	System1Str[7]=LoadStrByID(MID_USER_GP);
	System1Str[8]=LoadStrByID(HIT_SYSTEM3ITEM6);
	System1Str[9]=LoadStrByID(HIT_SYSTEM3ITEM7);
}

static void InitSys1Window(HWND hWnd)
{
	int rows = 0;
	HWND tmpstatic[2];
	int i;
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9,posX10,posX11,posX12,posX13;
	//char temp[4];

	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=240+gOptions.GridWidth*2;
		posX2=180;
		posX3=60;
		posX4=200;
		posX5=110;
		posX6=80;
		posX7=5;
		posX8=50;
		posX9=70;
		posX10=130;
		posX11=120;
		posX12=10;
		posX13=10;
	}
	else
	{
		posX1=10;
		posX2=80;
		posX3=120;
		posX4=135;
		posX5=145;
		posX6=165;
		posX7=185;
		posX8=210;
		posX9=90;
		posX10=10;
		posX11=10;
		posX12=112;
		posX13=250;
	}

	//printf("Create Window\n");
	tmpstatic[0] = CreateWindow(CTRL_STATIC, System1Str[0], SS_LEFT, 0x11010, posX1, getheight(rows, 0), 110, 23, hWnd, 0);
	System1ItemWnd[0] = CreateWindow(CTRL_SLEDIT, "", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_FZ, 
			posX4, getheight(rows, 4), 30, 23, hWnd, 0);
	SendMessage(System1ItemWnd[0], EM_LIMITTEXT, 2, 0);
	tmpstatic[1] = CreateWindow(CTRL_STATIC, "(1:N)", SS_LEFT, IDC_STATIC, posX6, getheight(rows, 0), 60, 23, hWnd, 0);
	System1ItemWnd[1] = CreateWindow(CTRL_SLEDIT, "", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE, IDC_FZ11,
			posX8, getheight(rows, 4), 30, 23, hWnd, 0);
	SendMessage(System1ItemWnd[1], EM_LIMITTEXT, 2, 0);
	if (!gOptions.IsOnlyRFMachine)
	{	
		for (i=0; i<2; i++)
		{
			ShowWindow(tmpstatic[i], SW_SHOW);
			ShowWindow(System1ItemWnd[i], SW_SHOW);
		}
		rows++;
	}

	CreateWindow(CTRL_STATIC, System1Str[1], WS_VISIBLE | SS_LEFT, 0x11011, posX1, getheight(rows, 0), 90, 23, hWnd, 0);
	System1ItemWnd[2] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST |
			CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, IDC_DTFMT, posX9+20, getheight(rows, 4), 140, 23, hWnd, 0);
	rows++;

	CreateWindow(CTRL_STATIC, System1Str[4], WS_VISIBLE | SS_LEFT, 0x11012, posX1, getheight(rows, 0), 70, 23, hWnd, 0);
	System1ItemWnd[3] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | 
			CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, IDC_KEYSND, posX2, getheight(rows, 4), 60, 23, hWnd, 0);

	if(!gOptions.IsOnlyRFMachine)
	{
		CreateWindow(CTRL_STATIC,LoadStrByID(MID_FINGER_SENSITIVITY),WS_VISIBLE | SS_LEFT,
				0x11020,posX6-10,getheight(rows, 0),110,23,hWnd,0);
		System1ItemWnd[4] = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,IDC_IR_BL_S,posX13,getheight(rows, 4),60,23,hWnd,0);

		SendMessage(System1ItemWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_VIDEO_QLOW));
		SendMessage(System1ItemWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_VIDEO_QMID));
		SendMessage(System1ItemWnd[4], CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_VIDEO_QHIGHT));

		//		printf("end %s \n", __FUNCTION__);
	}
	rows++;

	CreateWindow(CTRL_STATIC, System1Str[5], WS_VISIBLE | SS_LEFT, 0x11013, posX1, getheight(rows, 0), 70, 23, hWnd, 0);
	System1ItemWnd[5] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | 
			CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, IDC_SND, posX2, getheight(rows, 4), 60, 23, hWnd, 0);
	CreateWindow(CTRL_STATIC, System1Str[6], WS_VISIBLE | SS_LEFT, 0x11014, posX5, getheight(rows, 0), 80, 23, hWnd, 0);
	System1ItemWnd[6] = CreateWindow(CTRL_COMBOBOX, "", WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | 
			CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS, IDC_MIXIR, posX7+40, getheight(rows, 4), 60, 23, hWnd, 0);
	rows++;

	tmpstatic[0] = CreateWindow(CTRL_STATIC, System1Str[7], SS_LEFT, 0x11017, posX1, getheight(rows, 0), 110, 23, hWnd, 0);
	System1ItemWnd[7] = CreateWindow(CTRL_SLEDIT, "", WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, IDC_MGP, 
			posX3, getheight(rows, 4), 60, 23, hWnd, 0);
	SendMessage(System1ItemWnd[7], EM_LIMITTEXT, 2, 0);
	if (IDTFlag)
	{
		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(System1ItemWnd[7], SW_SHOW);
		rows++;
	}

	//zsliu use mul algver
	//if(!gOptions.IsOnlyRFMachine)
	if(!gOptions.IsOnlyRFMachine && gOptions.MulAlgVer==1)
	{
		char buff[30], fpver[50];
		CreateWindow(CTRL_STATIC, LoadStrByID(HIT_INFO7),WS_VISIBLE | SS_LEFT,
				0x11021,posX1,getheight(rows, 0),90,23,hWnd,0);

		System1ItemWnd[8] = CreateWindow(CTRL_COMBOBOX,"", WS_VISIBLE |CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS, IDC_FPVERSION, posX9+20, getheight(rows, 0), 145, 23,hWnd,0);

		memset(buff, 0, sizeof(buff));
		memset(fpver, 0, sizeof(fpver));
		LoadStr("~AlgVer",buff);
		sprintf(fpver, "%s%s", buff, "9.0");
		SendMessage(System1ItemWnd[8], CB_ADDSTRING, 0, (LPARAM)fpver);
		memset(fpver, 0, sizeof(fpver));
		sprintf(fpver, "%s%s", buff, "10.0");
		SendMessage(System1ItemWnd[8], CB_ADDSTRING, 0, (LPARAM)fpver);

		rows++;
	}

	CreateWindow(CTRL_STATIC, System1Str[9], WS_VISIBLE | SS_LEFT, 0x11016, posX10, getheight(rows, 0), 140, 23, hWnd, 0);
	System1ItemWnd[9] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, IDC_RETIME, 
			posX3+30, getheight(rows, 4), 30, 23, hWnd, 0);
	SendMessage(System1ItemWnd[9], EM_LIMITTEXT, 3, 0);
	rows++;

	CreateWindow(CTRL_STATIC, System1Str[8], WS_VISIBLE | SS_LEFT, 0x11015, posX10, getheight(rows, 0), 120, 23, hWnd, 0);
	System1ItemWnd[10] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, IDC_ATTREC, 
			posX3+10, getheight(rows, 4), 30, 23, hWnd, 0);
	SendMessage(System1ItemWnd[10], EM_LIMITTEXT, 2, 0);




	System1ItemWnd[11] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_OK), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDOK, 
			225+gOptions.ControlOffset, 160, 85, 23, hWnd, 0);
	System1ItemWnd[12] = CreateWindow(CTRL_BUTTON, LoadStrByID(HIT_CANCEL), WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, IDCANCEL, 
			225+gOptions.ControlOffset, 190, 85, 23, hWnd, 0);
}

static int isChanged1(void)
{
	char ctmp[16];
	int i;

	for (i=0; i<11; i++)
	{
		if (i>1 && i<9 && i!=7)
		{
			if(i != 8 || gOptions.IsOnlyRFMachine || gOptions.MulAlgVer==1)
			{
				if (SendMessage(System1ItemWnd[i], CB_GETCURSEL, 0, 0) != tmpparam[i])
					return 1;
			}
		}
		else
		{
			memset(ctmp, 0, sizeof(ctmp));
			GetWindowText(System1ItemWnd[i], ctmp, 16);
			if (atoi(ctmp) != tmpparam[i])
			{
				if (gOptions.IsOnlyRFMachine && (i==0 || i==1))	continue;
				if (!IDTFlag && i==7) continue;

				return 1;
			}
		}
	}
	return 0;
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
extern int CalcThreshold(int NewT);
extern int CalcNewThreshold(int Thr);
static int System1OK(HWND hWnd, HWND Item[], int Size)
{
	char cc[16];
	int NewFpVersion;
	int err=0;

	if (!gOptions.IsOnlyRFMachine)
	{
		memset(cc, 0, sizeof(cc));
		GetWindowText(Item[0], cc, 16);
		if (CheckText2(cc, 5, 50))
		{
			gOptions.VThreshold = CalcThreshold(atoi(cc));
		}
		else
			err++;

		memset(cc, 0, sizeof(cc));
		GetWindowText(Item[1], cc, 16);
		if (CheckText2(cc, 15, 50))  
		{
			gOptions.MThreshold = CalcThreshold(atoi(cc));
		}
		else
			err++;

		gOptions.FingerSensitivity = SendMessage(Item[4],CB_GETCURSEL,0,0);

		if(gOptions.MulAlgVer==1 && (SendMessage(System1ItemWnd[8], CB_GETCURSEL, 0, 0) != tmpparam[8])) 
		{
			NewFpVersion=(SendMessage(System1ItemWnd[8], CB_GETCURSEL, 0, 0)==0)?9:10;
			SaveInteger("ChangeVersion", NewFpVersion);
		}
	}

	gOptions.DateFormat = SendMessage(Item[2], CB_GETCURSEL, 0, 0);
	gOptions.KeyPadBeep = SendMessage(Item[3], CB_GETCURSEL, 0, 0);
	gOptions.TTS_KEY=gOptions.KeyPadBeep;
	SaveInteger("TTS_KEY",gOptions.TTS_KEY);

	gOptions.VoiceOn = SendMessage(Item[5], CB_GETCURSEL, 0, 0);
	gOptions.TTS_LOGO=gOptions.VoiceOn;
	SaveInteger("TTS_LOGO",gOptions.TTS_LOGO);

	if(gOptions.VoiceOn==0)
	{
		gOptions.TTS_KEY=gOptions.VoiceOn;
		SaveInteger("TTS_KEY",gOptions.TTS_KEY);
	}

#ifndef ZEM600
	gOptions.AudioVol = SendMessage(Item[6], CB_GETCURSEL, 0, 0);
#else
	//gOptions.AudioVol = (SendMessage(Item[6], CB_GETCURSEL, 0, 0)*8+14)*3;	//modify by jazzy 2009.06.10
	int value = SendMessage(Item[6], CB_GETCURSEL, 0, 0);
	if(value==0)
	{
		gOptions.AudioVol = VOICE_LOW;
	}
	else if(value==1)
	{
		gOptions.AudioVol = VOICE_MIDDLE;
	}
	else
	{
		gOptions.AudioVol = VOICE_HEIGH;
	}
#endif
	if (IDTFlag)
	{
		memset(cc, 0, sizeof(cc));
		GetWindowText(Item[7], cc, 16);
		if (cc[0] && atoi(cc)>0) 
			gOptions.MachineGroup = atoi(cc);
		else
			err++;
	}

	memset(cc, 0, sizeof(cc));
	GetWindowText(Item[10], cc, 16);
	if(CheckText2(cc, 0, 99))
		gOptions.AlarmAttLog = atoi(cc);
	else
		err++;

	memset(cc, 0, sizeof(cc));
	GetWindowText(Item[9], cc, 16);
	if(CheckText2(cc, 0, 255))
		gOptions.AlarmReRec = atoi(cc);
	else
		err++;

	if (err)
	{
		LoadOptions(&gOptions);
		MessageBox1 (hWnd, LoadStrByID(HIT_ERROR0), LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	else 
	{
#ifdef _TTS_
		TTS_SetVol(AdjTTSVol(gOptions.AudioVol));
#endif		
		SaveOptions(&gOptions);
		MessageBox1 (hWnd, LoadStrByID(HIT_RIGHT),LoadStrByID(HIT_RUN), MB_OK| MB_ICONINFORMATION);
		if(SendMessage(System1ItemWnd[8], CB_GETCURSEL, 0, 0) != tmpparam[8]) 
			MessageBox1(hWnd, LoadStrByID(HID_RESTART), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
	}
	return 1;
}

static void InitWindowValue(void)
{
	int i;
	char cc[16];

	for(i=0; i<10; i++)
	{
		SendMessage(System1ItemWnd[2], CB_ADDSTRING, 0, (LPARAM)DtFmtStr[i]);
	}
	//printf("%s\n",__FUNCTION__);
	SendMessage(System1ItemWnd[3], CB_ADDSTRING, 0, (LPARAM)SwitchStr[0]);
	SendMessage(System1ItemWnd[3], CB_ADDSTRING, 0, (LPARAM)SwitchStr[1]);
	SendMessage(System1ItemWnd[5], CB_ADDSTRING, 0, (LPARAM)SwitchStr[0]);
	SendMessage(System1ItemWnd[5], CB_ADDSTRING, 0, (LPARAM)SwitchStr[1]);
#ifdef ZEM600
	for(i=0;i<3;i++)
	{
		SendMessage(System1ItemWnd[6],CB_ADDSTRING ,0,(LPARAM)LoadStrByID(1265+i));
	}
#else	
	for(i=0; i<=100; i++)
	{
		memset(cc, 0, sizeof(cc));
		sprintf(cc, "%d%%", i);
		SendMessage(System1ItemWnd[6], CB_ADDSTRING, 0, (LPARAM)cc);
	}
#endif
	if (!gOptions.IsOnlyRFMachine)
	{
		tmpparam[0] = CalcNewThreshold(gOptions.VThreshold);
		tmpparam[1] = CalcNewThreshold(gOptions.MThreshold);

		tmpparam[4]=gOptions.FingerSensitivity;
		if(gOptions.ZKFPVersion==ZKFPV09)
		{
			tmpparam[8]=0;
		}
		else if(gOptions.ZKFPVersion==ZKFPV10)
		{
			tmpparam[8]=1;
		}
	}
	tmpparam[2] = gOptions.DateFormat;
	tmpparam[3] = gOptions.KeyPadBeep;
	tmpparam[5] = gOptions.VoiceOn;
#ifndef ZEM600
	tmpparam[6] = gOptions.AudioVol;
#else
	if(gOptions.AudioVol<=VOICE_LOW)
	{
		gOptions.AudioVol=VOICE_LOW;
		tmpparam[6]=0;
	}
	else if(gOptions.AudioVol>VOICE_LOW && gOptions.AudioVol<=VOICE_MIDDLE)
	{
		gOptions.AudioVol=VOICE_MIDDLE;
		tmpparam[6]=1;
	}
	else
	{
		gOptions.AudioVol=VOICE_HEIGH;
		tmpparam[6]=2;
	}
	//tmpparam[6] = ((gOptions.AudioVol<42?42:gOptions.AudioVol)/3-14)/8;
#endif
	tmpparam[7] = gOptions.MachineGroup;
	tmpparam[10] = gOptions.AlarmAttLog;
	tmpparam[9] = gOptions.AlarmReRec;

	for(i=0; i<11; i++)
	{
		if ((i>1 && i<9) && i!=7)
		{
			SendMessage(System1ItemWnd[i], CB_SETCURSEL, tmpparam[i], 0);
		}
		else
		{
			if (gOptions.IsOnlyRFMachine && (i<=1 || i==4)) continue;
			if (!IDTFlag && i==7) continue;
			memset(cc, 0, sizeof(cc));
			sprintf(cc, "%d", tmpparam[i]);
			SetWindowText(System1ItemWnd[i], cc);
		}
	}
}

static int System1DialogBoxProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitSys1Window(hWnd);
			UpdateWindow(hWnd, TRUE);
			InitWindowValue();
			SetFocusChild(System1ItemWnd[0]);
			if(!gOptions.IsOnlyRFMachine)
				System1Item=0;
			else
				System1Item=2;
			break;

		case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			EndPaint(hWnd, hdc);
			return 0;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;

				if (hdc == 0) {
					hdc = GetClientDC(hWnd);
					fGetDC = TRUE;
				}

				if (clip) {
					rcTemp = *clip;
					ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient(hWnd, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect(hdc, &rcTemp);
				}

				FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

				if (fGetDC)
					ReleaseDC(hdc);
				return 0;
			}
		case MSG_KEYUP:
			{
				if(gOptions.TFTKeyLayout==3)
				{
					keyupFlag=1;
				}
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(gOptions.TFTKeyLayout==3)
			{
				if(keyupFlag==1)
				{
					keyupFlag=0;
				}
				else
					break;
			}
			if(gOptions.KeyPadBeep)
				ExKeyBeep();

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if (++System1Item>12)// change by cn
				{
					System1Item = (gOptions.IsOnlyRFMachine) ? 2:0;
				}

				if ((!IDTFlag && System1Item==7)||(gOptions.IsOnlyRFMachine && (System1Item==4 || System1Item==8)))
				{
					System1Item++;
				}
				//zsliu change
				if((System1Item == 8) && (gOptions.IsOnlyRFMachine==0) && (gOptions.MulAlgVer==0))
				{
					System1Item++;
				}

				SetFocus(System1ItemWnd[System1Item]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if (--System1Item < ((gOptions.IsOnlyRFMachine)?2:0))
				{
					System1Item=12; // change by cn
				}
				if ((!IDTFlag && System1Item==7)||(gOptions.IsOnlyRFMachine && (System1Item==4 || System1Item==8)))
				{
					System1Item--;
				}
				//zsliu change
				if((System1Item == 8) && (gOptions.IsOnlyRFMachine==0) && (gOptions.MulAlgVer==0))
				{
					System1Item-=2;
				}

				SetFocus(System1ItemWnd[System1Item]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if(System1Item==2)
				{
					if (SendMessage(System1ItemWnd[System1Item], CB_GETCURSEL, 0, 0)==9)
						SendMessage(System1ItemWnd[System1Item],CB_SETCURSEL, 0, 0);
					else
						SendMessage(System1ItemWnd[System1Item], CB_SPIN, 1, 0);
					return 0;
				}
				else if(System1Item==3 || System1Item==5 || System1Item==8)
				{
					if(SendMessage(System1ItemWnd[System1Item], CB_GETCURSEL, 0, 0)==1)
						SendMessage(System1ItemWnd[System1Item], CB_SETCURSEL, 0, 0);
					else
						SendMessage(System1ItemWnd[System1Item], CB_SETCURSEL, 1, 0);
					return 0;
				}
				else if(System1Item==4)
				{
					int ret = SendMessage(System1ItemWnd[System1Item],CB_GETCURSEL,0,0);
					int newValue=0;
					if(ret ==0)
						newValue = 1;
					else if(ret == 1)
						newValue = 2;
					else if(ret == 2)
						newValue = 0;
					SendMessage(System1ItemWnd[System1Item],CB_SETCURSEL,newValue,0);	

				}
				else if(System1Item==6)
				{
#ifdef ZEM600
					if(SendMessage(System1ItemWnd[System1Item], CB_GETCURSEL, 0, 0)==2)
#else
						if(SendMessage(System1ItemWnd[System1Item], CB_GETCURSEL, 0, 0)==100)
#endif
							SendMessage(System1ItemWnd[System1Item], CB_SETCURSEL, 0, 0);
						else
							SendMessage(System1ItemWnd[System1Item], CB_SPIN, 1, 0);
					return 0;
				}
			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if(System1Item==2)
				{
					if (SendMessage(System1ItemWnd[System1Item], CB_GETCURSEL, 0, 0)==0)
						SendMessage(System1ItemWnd[System1Item], CB_SETCURSEL, 9, 0);
					else
						SendMessage(System1ItemWnd[System1Item], CB_SPIN, 0, 0);
					return 0;
				}
				else if(System1Item==3 || System1Item==5 || System1Item==8)
				{
					if(SendMessage(System1ItemWnd[System1Item],CB_GETCURSEL, 0, 0)==0)
						SendMessage(System1ItemWnd[System1Item], CB_SETCURSEL, 1, 0);
					else
						SendMessage(System1ItemWnd[System1Item], CB_SETCURSEL, 0, 0);
					return 0;
				}
				else if(System1Item==4)
				{
					int ret = SendMessage(System1ItemWnd[System1Item],CB_GETCURSEL,0,0);
					int newValue=0;
					if(ret ==0)
						newValue = 2;
					else if(ret == 1)
						newValue = 0;
					else if(ret == 2)
						newValue = 1;
					SendMessage(System1ItemWnd[System1Item],CB_SETCURSEL,newValue,0);	
				}

				else if(System1Item==6)
				{
					if(SendMessage(System1ItemWnd[System1Item], CB_GETCURSEL, 0, 0)==0)
#ifdef ZEM600
						SendMessage(System1ItemWnd[System1Item], CB_SETCURSEL, 2, 0);
#else
					SendMessage(System1ItemWnd[System1Item], CB_SETCURSEL, 100, 0);
#endif
					else
						SendMessage(System1ItemWnd[System1Item], CB_SPIN, 0, 0);
					return 0;
				}
			}

			if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_MENU)||(LOWORD(wParam)==SCANCODE_F10))
			{
				if (System1Item<12 && isChanged1())
				{
					if (System1OK(hWnd, System1ItemWnd, 8) && !ismenutimeout)
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					else
						//Liaozz 20080926 fix bug second 42
						return 0;
					//Liaozz end
				}
				else
					PostMessage(hWnd, MSG_CLOSE, 0, 0);

				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				if(isChanged1())
				{
					if(MessageBox1(hWnd, LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
								MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
					{
						if(System1OK(hWnd, System1ItemWnd, 8) && !ismenutimeout)
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
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&system1bk);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int SSR_MENU_SYSTEM1(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(HIT_SYSTEMSET);
	CreateInfo.hMenu = 0;
	//        CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = System1DialogBoxProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	LoadSystem1Str();
	//if (LoadBitmap(HDC_SCREEN,&system1bk,GetBmpPath("submenubg.jpg")))
	//         return 0;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg, hMainWnd))
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
