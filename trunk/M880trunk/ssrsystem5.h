/* 
 * SSR 2.0 Self Service Record 主入口头文件
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0   
 * 修改记录:
 * 编译环境:mipsel-gcc
 */


#ifndef __CWX_GUI_SYSTEM5
#define __CWX_GUI_SYSTEM5

#include "ssrcommon.h"
#include "flashdb.h"
#include <minigui/tftmullan.h>

static DLGTEMPLATE System5DlgBox = {WS_BORDER | WS_CAPTION, WS_EX_NONE, 1, 1, 319, 239, "", 0, 0, 14, NULL, 0};

#define IDC_STATE1   550
#define IDC_STATE2   551
#define IDC_STATE3   552
#define IDC_STATE4   553
#define IDC_STATE5   554
#define IDC_STATE6   555

#define IDC_KEY1 556
#define IDC_KEY2 557
#define IDC_KEY3 558
#define IDC_KEY4 559
#define IDC_KEY5 560
#define IDC_KEY6 561

char* stateName[6];
int oldkey[6];
static int keytable[14] = {0,1,2,3,4,5,6,7,8,11,12,13,14,15};	//＊键和＃键为功能键，不能作为状态键

static CTRLDATA System5Ctrl [] =
{
	{ CTRL_STATIC, WS_VISIBLE | SS_LEFT, 10, 10, 100, 25, 0x11020, "", 0 },
	{ CTRL_COMBOBOX, WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
		130, 10, 180, 25, IDC_KEY1, "", 0 },
	{
		//CTRL_COMBOBOX,
		//WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT,
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		10, 35, 100, 25,
		0x11021,
		"",
		0
	},
	{
		CTRL_COMBOBOX,
		WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
		130, 35, 180, 25,
		IDC_KEY2,
		"",
		0
	},
	{
		//CTRL_COMBOBOX,
		//WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT,
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		10, 60, 100, 25,
		0x11022,
		"",
		0
	},
	{
		CTRL_COMBOBOX,
		WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
		130, 60, 180, 25,
		IDC_KEY3,
		"",
		0
	},
	{
		//CTRL_COMBOBOX,
		//WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT,
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		10, 85, 100, 25,
		0x11023,
		"",
		0
	},
	{
		CTRL_COMBOBOX,
		WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
		130, 85, 180, 25,
		IDC_KEY4,
		"",
		0
	},
	{
		//CTRL_COMBOBOX,
		//WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT,
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		10, 110, 100, 25,
		0x11024,
		"",
		0
	},
	{
		CTRL_COMBOBOX,
		WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
		130, 110, 180, 25,
		IDC_KEY5,
		"",
		0
	},
	{
		//CTRL_COMBOBOX,
		//WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT,
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		10, 135, 100, 25,
		0x11025,
		"",
		0
	},
	{
		CTRL_COMBOBOX,
		WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
		130, 135, 180, 25,
		IDC_KEY6,
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
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
		10, 180, 80, 25,
		IDOK, 
		"",
		0
	}
};

static HWND System5ItemWnd[8];
static int System5Item;
//static BITMAP system5bk;

extern int UTF8toLocal(int lid,const unsigned char *utf8, unsigned char *str);
static int System5OK(HWND hWnd,HWND Item[],int Size)
{
	int key;
	int err=0;
	int i,j,keys[6];
	char tmpstr[40];
	TSHORTKEY tsskey;

	for(i=0;i<6;i++)
	{
		keys[i]=SendMessage(Item[i],CB_GETCURSEL,0,0);
	}

	err=0;
	for(i=0;i<5;i++)
	{
		for(j=i+1;j<6;j++)
			if(keys[i]==keys[j] && keys[i]!=0) 
			{
				err++;
			}
	}	

	if(err)
	{
		MessageBox1 (hWnd ,LoadStrByID(HIT_KEYINFO) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	else
	{
		for(i=0;i<6;i++)
		{
			key=SendMessage(Item[i],CB_GETCURSEL,0,0);	//get current select key
			if(key!=oldkey[i])
			{
				//clear old key record
				if(keytable[oldkey[i]])
				{
					memset(&tsskey,0,sizeof(TSHORTKEY));
					if(FDB_GetShortKey(keytable[oldkey[i]],&tsskey)!=NULL)
					{
						tsskey.keyFun=0;
						tsskey.stateCode=0;
						memset(tsskey.stateName,0,STATE_NAME_LEN+2);
						FDB_ChgShortKey(&tsskey);
					}
				}
				if(keytable[key])
				{
					memset(&tsskey,0,sizeof(TSHORTKEY));
					if(FDB_GetShortKey(keytable[key],&tsskey)!=NULL)
					{
						tsskey.keyFun=1;
						tsskey.stateCode=i+1;
						memset(tsskey.stateName,0,STATE_NAME_LEN+2);
						memset(tmpstr,0,40);
						memcpy(tmpstr,stateName[i],strlen(stateName[i]));
						UTF8toLocal(tftlocaleid,(unsigned char*)tmpstr, (unsigned char*)tsskey.stateName);//modified by lyy 2009.07.16

						FDB_ChgShortKey(&tsskey);
					}
				}
			}							
			sync();
		}

		MessageBox1 (hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
		return 1;
	}
}

static int getkeyindex(int keyid)
{
	int i;
	for(i=0;i<14;i++)
	{
		if(keytable[i]==keyid)
			return i;
	}
	return 0;
}

static int InitWindow(HWND hDlg)
{
	int i,j;
	TSHORTKEY tstkey;
	HWND stWnd[6];
	char* keyName[14];
	char tmpstr[50];

	stWnd[0]=GetDlgItem(hDlg,0x11020);
	stWnd[1]=GetDlgItem(hDlg,0x11021);
	stWnd[2]=GetDlgItem(hDlg,0x11022);
	stWnd[3]=GetDlgItem(hDlg,0x11023);
	stWnd[4]=GetDlgItem(hDlg,0x11024);
	stWnd[5]=GetDlgItem(hDlg,0x11025);

	System5ItemWnd[0]=GetDlgItem(hDlg,IDC_KEY1);
	System5ItemWnd[1]=GetDlgItem(hDlg,IDC_KEY2);
	System5ItemWnd[2]=GetDlgItem(hDlg,IDC_KEY3);
	System5ItemWnd[3]=GetDlgItem(hDlg,IDC_KEY4);
	System5ItemWnd[4]=GetDlgItem(hDlg,IDC_KEY5);
	System5ItemWnd[5]=GetDlgItem(hDlg,IDC_KEY6);
	System5ItemWnd[6]=GetDlgItem(hDlg,IDOK);
	System5ItemWnd[7]=GetDlgItem(hDlg,IDCANCEL);

	stateName[0]=LoadStrByID(HIT_SYSTEM5KEY1);      //上班签到
	stateName[1]=LoadStrByID(HIT_SYSTEM5KEY2);      //下班签退
	stateName[2]=LoadStrByID(HIT_SYSTEM5KEY3);      //加班签到
	stateName[3]=LoadStrByID(HIT_SYSTEM5KEY4);      //加班签退
	stateName[4]=LoadStrByID(HIT_SYSTEM5KEY5);      //外出
	stateName[5]=LoadStrByID(HIT_SYSTEM5KEY6);      //外出返回

	keyName[0]=LoadStrByID(HID_NOTHING);
	if(gOptions.TFTKeyLayout!=3)
	{
		for(i=1;i<14;i++)
		{
			keyName[i]=LoadStrByID(MID_SHORTKEY01+keytable[i]-1);
		}
	}
	else
	{
		keyName[1]=LoadStrByID(MID_SHORTKEY11);        //back  
		keyName[2]=LoadStrByID(MID_SHORTKEY15);       //right
		keyName[3]=LoadStrByID(MID_WKCDKEY73);       //esc
		keyName[4]=LoadStrByID(MID_SHORTKEY12);        //up
		keyName[5]=LoadStrByID(MID_WKCDKEY74);       //enter
		keyName[6]=LoadStrByID(MID_SHORTKEY13);        //down
		keyName[7]=LoadStrByID(MID_WKCDKEY75);       //0
	}

	for(i=0;i<6;i++)
	{
		SetWindowText(stWnd[i],stateName[i]);
		oldkey[i]=0;

		if(gOptions.TFTKeyLayout!=3)
		{
			for(j=0;j<14;j++)
			{
				SendMessage(System5ItemWnd[i],CB_ADDSTRING ,0,(LPARAM)keyName[j]);
			}
		}
		else
		{
			for(j=0;j<8;j++)
			{
				SendMessage(System5ItemWnd[i],CB_ADDSTRING ,0,(LPARAM)keyName[j]);
			}
		}
	}

	for(i=0;i<13;i++)
	{
		memset(&tstkey,0,sizeof(TSHORTKEY));
		if(FDB_GetShortKey(keytable[i+1],&tstkey)!=NULL)
		{
			if(tstkey.keyFun==1)
			{
				for(j=0;j<6;j++)
				{
					memset(tmpstr,0,sizeof(tmpstr)); //modified by lyy 2009.07.16
					Str2UTF8(tftlocaleid,(unsigned char*)tstkey.stateName,(unsigned char*)tmpstr);
					if(strlen(tmpstr)==strlen(stateName[j]) && 
							strncmp(tmpstr,stateName[j],strlen(stateName[j]))==0)
					{
						oldkey[j] = getkeyindex(tstkey.keyID);
					}
				}
			}
		}
	}

	for(i=0;i<6;i++)
	{
		SendMessage(System5ItemWnd[i],CB_SETCURSEL,oldkey[i],0);
	}

	SetWindowText(GetDlgItem(hDlg,IDCANCEL),LoadStrByID(HIT_CANCEL));
	SetWindowText(GetDlgItem(hDlg,IDOK),LoadStrByID(HIT_OK));
	SetFocus(System5ItemWnd[0]);
	System5Item=0;
	return 1;
}

static int System5DialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	//HDC hdc;
	//int sel;
	//int i;
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_INITDIALOG:
			InitWindow(hDlg);
			UpdateWindow(hDlg,TRUE);
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
				break;
			}

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
				ExKeyBeep();

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
			{
				if(++System5Item>7) System5Item=0;
				SetFocus(System5ItemWnd[System5Item]);
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				if(--System5Item<0) System5Item=7;
				SetFocus(System5ItemWnd[System5Item]);
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT))
			{
				switch(System5Item)
				{
					case 0:
						SendDlgItemMessage(hDlg,IDC_KEY1,CB_SPIN,0,0);
						break;
					case 1:
						SendDlgItemMessage(hDlg,IDC_KEY2,CB_SPIN,0,0);
						break;
					case 2:
						SendDlgItemMessage(hDlg,IDC_KEY3,CB_SPIN,0,0);
						break;
					case 3:
						SendDlgItemMessage(hDlg,IDC_KEY4,CB_SPIN,0,0);
						break;
					case 4:
						SendDlgItemMessage(hDlg,IDC_KEY5,CB_SPIN,0,0);
						break;
					case 5:
						SendDlgItemMessage(hDlg,IDC_KEY6,CB_SPIN,0,0);
						break;
				}
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				switch(System5Item)
				{
					case 0:
						SendDlgItemMessage(hDlg,IDC_KEY1,CB_SPIN,1,0);
						break;
					case 1:
						SendDlgItemMessage(hDlg,IDC_KEY2,CB_SPIN,1,0);
						break;
					case 2:
						SendDlgItemMessage(hDlg,IDC_KEY3,CB_SPIN,1,0);
						break;
					case 3:
						SendDlgItemMessage(hDlg,IDC_KEY4,CB_SPIN,1,0);
						break;
					case 4:
						SendDlgItemMessage(hDlg,IDC_KEY5,CB_SPIN,1,0);
						break;
					case 5:
						SendDlgItemMessage(hDlg,IDC_KEY6,CB_SPIN,1,0);
						break;
				}
			}
			else if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_MENU)||(LOWORD(wParam)==SCANCODE_F10))
			{
				if(System5Item<=6)
				{
					if(System5OK(hDlg,System5ItemWnd,8) && !ismenutimeout)
						SendMessage(hDlg,MSG_CLOSE,0,0);
				}
				else if(System5Item==7)
					SendMessage(hDlg,MSG_CLOSE,0,0);
			}
			else if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				SendMessage(hDlg,MSG_CLOSE,0,0);
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&system5bk);
			EndDialog (hDlg, IDCANCEL);
			break;

		default:
			return DefaultDialogProc (hDlg, message, wParam, lParam);

	}
	return (0);
}

int SSR_MENU_SYSTEM5(HWND hWnd)
{
	int posX1,posX2;
	System5DlgBox.w = gOptions.LCDWidth;
	System5DlgBox.h = gOptions.LCDHeight;


	if (fromRight==1)	//add by jazzy 2008.07.24
	{
		posX1=210+gOptions.ControlOffset;
		posX2=20+gOptions.ControlOffset;

		System5Ctrl[0].x=posX1;
		System5Ctrl[1].x=posX2;
		System5Ctrl[2].x=posX1;
		System5Ctrl[3].x=posX2;
		System5Ctrl[4].x=posX1;
		System5Ctrl[5].x=posX2;
		System5Ctrl[6].x=posX1;
		System5Ctrl[7].x=posX2;
		System5Ctrl[8].x=posX1;
		System5Ctrl[9].x=posX2;
		System5Ctrl[10].x=posX1;
		System5Ctrl[11].x=posX2;
	}
	else
	{
		System5Ctrl [12].x = 230 + gOptions.ControlOffset;
	}

	//if(LoadBitmap(HDC_SCREEN,&system5bk,GetBmpPath("submenubg.jpg")))
	//{
	//	return 0;
	//}

	System5DlgBox.caption = LoadStrByID(HIT_SYSTEM5KEY);
	System5DlgBox.controls = System5Ctrl;
	DialogBoxIndirectParam (&System5DlgBox, hWnd, System5DialogBoxProc, 0L);
	return 1;
}


#endif
