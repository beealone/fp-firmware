/* 
 * SSR 2.0 Self Service Record 主入口头文件
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0   
 * 修改记录:
 * 编译环境:mipsel-gcc
 */
#include <minigui/common.h>

#include "ssrkeytest.h"
#include "ssrsortest.h"
#include "ssrrtctest.h"
#include "ssrsndtest.h"
#include "ssrpub.h"
#include "ssrauto.h"

static DLGTEMPLATE AutoDlgBox_FP = {WS_BORDER | WS_CAPTION, WS_EX_NONE, 1, 1, 319, 239, "", 0, 0, 7, NULL, 0};
static DLGTEMPLATE AutoDlgBox_RF = {WS_BORDER | WS_CAPTION, WS_EX_NONE, 1, 1, 319, 239, "", 0, 0, 6, NULL, 0};

//dsl 2011.9.30
#define IDC_EXTBELL      606

#define IDC_ALL      605
#define IDC_TFT      600
#define IDC_AUDIO    601
#define IDC_SENSOR   602
#define IDC_KEY      603
#define IDC_CLOCK    604

#ifdef FACE
#define IDC_FACE_TEST   606
#endif

static CTRLDATA AutoCtrl_FP [] =
{
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 25, 120, 20, IDC_ALL, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 50, 120, 20, IDC_TFT, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 75, 120, 20, IDC_AUDIO, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 100, 120, 20, IDC_KEY, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 125, 120, 20, IDC_SENSOR, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 150, 120, 20, IDC_CLOCK, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 175, 120, 20, IDC_EXTBELL, "", 0}
};
static CTRLDATA AutoCtrl_RF [] =
{
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 25, 120, 20, IDC_ALL, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 50, 120, 20, IDC_TFT, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 75, 120, 20, IDC_AUDIO, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 100, 120, 20, IDC_KEY, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 125, 120, 20, IDC_CLOCK, "", 0},
	{CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 150, 120, 20, IDC_EXTBELL, "", 0}
};

static HWND AutoItemWnd[8];//dsl 2011.9.30.7->8
static int AutoItem;
static BITMAP autoinfo;
//static BITMAP autobk;

//dsl 20111.9.30
static void AutoTextExtBell(void)
{
	if (gOptions.IsSupportExtBell)
	{
		ExAlarmBellOnByMCU(120);
	}
}

extern void SSR_MENU_AUTO_TFTTEST(HWND hWnd);
static int AutoOK(HWND hWnd,HWND Item[],int pItem)
{
	if(pItem==0)
	{
		SSR_MENU_AUTO_TFTTEST(hWnd);
		if (ismenutimeout) return 0;
		if(TestFlage==-1) return 0;
		SSR_MENU_AUTO_SNDTEST(hWnd);
		if (ismenutimeout) return 0;
		if(TestFlage==-1) return 0;
		SSR_MENU_AUTO_KEYTEST(hWnd);
		if (ismenutimeout) return 0;
		if(TestFlage==-1) return 0;
		if(!gOptions.IsOnlyRFMachine)
		{
			SSR_MENU_AUTO_SORTEST(hWnd);
			if (ismenutimeout) return 0;
			if(TestFlage==-1) return 0;
		}
#ifdef FACE
		if(gOptions.FaceFunOn) // add by caona for face
		{
			CreateFaceTestWindow(hWnd);
		}
#endif
		SSR_MENU_AUTO_RTCTEST(hWnd);
		if (ismenutimeout) return 0;
		if(TestFlage==-1) return 0;

		//dsl 2011.9.30		
		AutoTextExtBell();
		if (ismenutimeout) return 0;
		if(TestFlage==-1) return 0;
	}
	else
	{
		if(pItem==1)
		{
			SSR_MENU_AUTO_TFTTEST(hWnd);
			if (ismenutimeout) return 0;
			if(TestFlage==-1) return 0;
		}
		if(pItem==2)
		{
			SSR_MENU_AUTO_SNDTEST(hWnd);
			if (ismenutimeout) return 0;
			if(TestFlage==-1) return 0;
		}
		if(pItem==3)
		{
			SSR_MENU_AUTO_KEYTEST(hWnd);
			if (ismenutimeout) return 0;
			if(TestFlage==-1) return 0;
		}
		if(!gOptions.IsOnlyRFMachine && pItem==4)
		{
			SSR_MENU_AUTO_SORTEST(hWnd);
			if (ismenutimeout) return 0;
			if(TestFlage==-1) return 0;
		}
		if((!gOptions.IsOnlyRFMachine && pItem==5) || (gOptions.IsOnlyRFMachine && pItem==4))
		{
			SSR_MENU_AUTO_RTCTEST(hWnd);
			if (ismenutimeout) return 0;
			if(TestFlage==-1) return 0;
		}
#ifdef FACE
		if(gOptions.FaceFunOn)
		{
			if((!gOptions.IsOnlyRFMachine && pItem==6) || (gOptions.IsOnlyRFMachine && pItem==5))
			{
				CreateFaceTestWindow(hWnd);
				if (ismenutimeout) return 0;
				if(TestFlage==-1) return 0;
			}
		}
#endif
		//dsl 2011.9.30
		if (gOptions.IsSupportExtBell)
		{
			if((!gOptions.IsOnlyRFMachine && pItem==7) || (!gOptions.IsOnlyRFMachine && pItem==6) || (gOptions.IsOnlyRFMachine && pItem==5))
			{
				AutoTextExtBell();
				if (ismenutimeout) return 0;
				if(TestFlage==-1) return 0;
			}
		}
	}
	return 0;	
}

static int AutoDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	int i;
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_INITDIALOG:
			//if (LoadBitmap(HDC_SCREEN,&autobk,GetBmpPath("submenubg.jpg")))
			//	return 0;

			if (LoadBitmap(HDC_SCREEN,&autoinfo,GetBmpPath("auto.gif")))
				return 0;

			SetWindowText(GetDlgItem(hDlg,IDC_ALL),LoadStrByID(HIT_AUTO0));
			SetWindowText(GetDlgItem(hDlg,IDC_TFT),LoadStrByID(HIT_AUTO1));
			SetWindowText(GetDlgItem(hDlg,IDC_AUDIO),LoadStrByID(HIT_AUTO2));
			SetWindowText(GetDlgItem(hDlg,IDC_KEY),LoadStrByID(HIT_AUTO4));
			if(!gOptions.IsOnlyRFMachine)
				SetWindowText(GetDlgItem(hDlg,IDC_SENSOR),LoadStrByID(HIT_AUTO3));
			SetWindowText(GetDlgItem(hDlg,IDC_CLOCK),LoadStrByID(HIT_AUTO5));

			i=0;
			AutoItemWnd[i++] = GetDlgItem (hDlg, IDC_ALL);
			AutoItemWnd[i++] = GetDlgItem (hDlg, IDC_TFT);
			AutoItemWnd[i++] = GetDlgItem (hDlg, IDC_AUDIO);
			AutoItemWnd[i++] = GetDlgItem (hDlg, IDC_KEY);
			if(!gOptions.IsOnlyRFMachine)
				AutoItemWnd[i++] = GetDlgItem (hDlg, IDC_SENSOR);
			AutoItemWnd[i++] = GetDlgItem (hDlg, IDC_CLOCK);

#ifdef FACE
			if(gOptions.FaceFunOn) //add  by caona for face
			{
				int y=175;
				if(gOptions.IsOnlyRFMachine)
					y=150;
				AutoItemWnd[i++]=CreateWindow(CTRL_BUTTON,LoadStrByID(MID_FACE_TEST),WS_VISIBLE | WS_BORDER,\
						IDC_FACE_TEST,50,y,120,20,hDlg,0);
			}

#endif
			//dsl 2011.9.30. for the delay bell 
			if (gOptions.IsSupportExtBell)
			{
				SetWindowText(GetDlgItem(hDlg,IDC_EXTBELL), LoadStrByID(MID_BELL_EXT));
				AutoItemWnd[i++] = GetDlgItem (hDlg, IDC_EXTBELL);
			}
			else
			{
				ShowWindow(GetDlgItem (hDlg, IDC_EXTBELL), SW_HIDE);
			}

			AutoItem=0;
			SetFocus(AutoItemWnd[AutoItem]);
			TestFlage=0;
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				char buffer[32];

				GetWindowText(AutoItemWnd[AutoItem],buffer,32);
				TTS_Say(buffer);
			}
#endif
			break;
		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout)
			{
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
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
			if(gOptions.KeyPadBeep)
				ExKeyBeep();

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
				{
					if(!gOptions.IsOnlyRFMachine)
					{
#ifdef FACE
						if(gOptions.FaceFunOn)
						{
							if (gOptions.IsSupportExtBell)
							{
								if (++AutoItem > 7)
								{
									AutoItem = 0;
								}
							}
							else if(++AutoItem > 6)
							{
								AutoItem = 0;
							}
						}
						else
#endif
							if (gOptions.IsSupportExtBell)
							{
								if(++AutoItem > 6) AutoItem = 0;
							}
							else
							{
								if(++AutoItem > 5) AutoItem = 0;
							}
					}
					else
					{
#ifdef FACE
						if(gOptions.FaceFunOn)
						{
							if (gOptions.IsSupportExtBell)
							{
								if(++AutoItem > 6)
								{
									AutoItem = 0;
								}
							}
							else if(++AutoItem > 5)
							{
								AutoItem = 0;
							}
						}
						else
#endif
							if (gOptions.IsSupportExtBell)
							{
								if(++AutoItem >5) AutoItem=0;
							}
							else
							{
								if(++AutoItem >4) AutoItem=0;
							}
					}
				}
				else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
				{
					if(--AutoItem < 0)
					{
#ifdef FACE
						if(gOptions.FaceFunOn)
						{
							if (gOptions.IsSupportExtBell)
							{
								AutoItem = (!gOptions.IsOnlyRFMachine)?7:6;
							}
							else
							{
								AutoItem = (!gOptions.IsOnlyRFMachine)?6:5;
							}
						}
						else
#endif
							if (gOptions.IsSupportExtBell)
							{
								AutoItem = (!gOptions.IsOnlyRFMachine)?6:5;
							}
							else
							{
								AutoItem = (!gOptions.IsOnlyRFMachine)?5:4;
							}
					}
				}
				SetFocus(AutoItemWnd[AutoItem]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];

					GetWindowText(AutoItemWnd[AutoItem],buffer,32);
					TTS_Say(buffer);
				}
#endif
				SendMessage(AutoItemWnd[AutoItem],BM_SETCHECK,BST_CHECKED,0);
				return 0;
			}
			else if((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_MENU) || (LOWORD(wParam)==SCANCODE_F10))
			{
				if(AutoOK(hDlg,AutoItemWnd,AutoItem))
					PostMessage(hDlg,MSG_CLOSE,0,0);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage(hDlg,MSG_CLOSE,0,0);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
				return 0;
			break;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;

				if(hdc == 0)
				{
					hdc = GetClientDC (hDlg);
					fGetDC = TRUE;
				}

				if(clip)
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
			FillBoxWithBitmap(hdc, 220, 60, 0, 0, &autoinfo);
			EndPaint(hDlg,hdc);
			return 0;

		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
				TTS_Stop();
#endif			
			//UnloadBitmap(&autobk);
			UnloadBitmap(&autoinfo);
			EndDialog(hDlg, IDCANCEL);
			return 0;
	}

	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_AUTO(HWND hWnd)
{
	if(!gOptions.IsOnlyRFMachine)
	{
		AutoDlgBox_FP.w = gOptions.LCDWidth;
		AutoDlgBox_FP.h = gOptions.LCDHeight;

		AutoDlgBox_FP.caption = LoadStrByID(HIT_AUTOS);
		AutoDlgBox_FP.controls = AutoCtrl_FP;
		DialogBoxIndirectParam (&AutoDlgBox_FP, hWnd, AutoDialogBoxProc, 0L);
	}
	else
	{
		AutoDlgBox_RF.w = gOptions.LCDWidth;
		AutoDlgBox_RF.h = gOptions.LCDHeight;

		AutoDlgBox_RF.caption = LoadStrByID(HIT_AUTOS);
		AutoDlgBox_RF.controls = AutoCtrl_RF;
		DialogBoxIndirectParam (&AutoDlgBox_RF, hWnd, AutoDialogBoxProc, 0L);
	}
}
