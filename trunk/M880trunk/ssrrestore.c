/*
	ssrrestore.c 恢复设置界面
	包括恢复出厂参数设置，恢复键盘定义设置,恢复闹铃设置,恢复门禁设置
*/

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h> 
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <string.h>
#include "ssrpub.h"
#include "ssrcommon.h"
#include "flashdb.h"
#include "tftmsgbox.h"
#include "finger.h"

static DLGTEMPLATE RestoreDlgBox =
{
    WS_BORDER | WS_CAPTION, 
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    4, NULL,
    0
};
static DLGTEMPLATE RestoreLockDlgBox =
{
    WS_BORDER | WS_CAPTION, 
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    5, NULL,
    0
};

#define IDC_RESTOREPARA   		4110    
#define IDC_RESTORESHORTKEY 		4120
#define IDC_RESTOREALARM   		4130
#define IDC_RESTORELOCKFUN   		4140
#define IDC_RESTOREOTHER   		4150

extern int fdUser;

static CTRLDATA RestoreCtrl [] =
{
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        40, 25, 240, 25,
        IDC_RESTOREPARA,
        "",
        0
    },
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		40, 70, 240, 25,
		IDC_RESTORESHORTKEY,
		"",
		0
	},
	{
                CTRL_BUTTON,
                WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
                40, 115, 240, 25,
                IDC_RESTOREALARM,
                "",
                0

	},
	{
                CTRL_BUTTON,
                WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
                40, 160, 240, 25,
                IDC_RESTOREOTHER,
                "",
                0

	}
};


static CTRLDATA RestoreLockCtrl [] =
{
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        40, 25, 240, 25,
        IDC_RESTOREPARA,
        "",
        0
    },
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		40, 60, 240, 25,
		IDC_RESTORESHORTKEY,
		"",
		0
	},
	{
                CTRL_BUTTON,
                WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
                40, 95, 240, 25,
                IDC_RESTOREALARM,
                "",
                0

	},
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        40, 130, 240, 25,
        IDC_RESTOREOTHER,
        "",
        0
    },
	{
                CTRL_BUTTON,
                WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
                40, 165, 240, 25,
                IDC_RESTORELOCKFUN,
                "",
                0

	}
};

static HWND RestoreItemWnd[7];
static int RestoreItem;
//static BITMAP Restorebk;

static int RestoreConfirm(HWND hWnd,int Item)
{
		if(MessageBox1(hWnd,LoadStrByID(HIT_SYSTEM2INFO),LoadStrByID(HIT_RUN),MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)!=IDOK)
			return 0;
		else
			return 1;
}

void resetwifi(void)
{
        SaveStr("SSID", "", 1);
        SaveStr("NetworkType", "Infra", 1);
        SaveStr("AuthMode","OPEN", 1);
        SaveStr("EncrypType","NONE", 1);
        SaveStr("wifiip", "", 1);
        SaveStr("wifimask", "", 1);
        SaveStr("wifigateway", "", 1);
        SaveStr("DefaultKeyID", "", 1);                      //seletct whic  pass str
        SaveStr("Key1Type", "", 1);
        SaveStr("Key1Str", "", 1);
        SaveStr("Key2Type", "", 1);
        SaveStr("Key2Str", "", 1);
        SaveStr("Key3Type", "", 1);
        SaveStr("Key3Str", "", 1);
        SaveStr("Key4Type", "", 1);
        SaveStr("Key4Str", "", 1);
        SaveStr("WPAPSK", "",1);
        SaveInteger("wifidhcp",0);
}

void resetgprs(void)
{
	SaveStr("APN", "", 1);
	SaveStr("ModemUserName", "", 1);
	SaveStr("ModemPassword", "", 1);
	SaveStr("ModemDialNumber", "", 1);
	SaveStr("HeartBeatServer", "", 1);

	//gOptions.ModemEnable=0;		
	//SaveOptions(&gOptions);
}

extern HWND createStatusWin1 (HWND hParentWnd, int width, int height,char* title, char* text, ...);
extern void destroyStatusWin1(HWND hwnd);
void resetlockfunon(void)
{
	//初始化默认门禁设置
	FDB_ClearData(FCT_TZ);
	FDB_ClearData(FCT_HTZ);
	FDB_ClearData(FCT_GROUP);
	FDB_ClearData(FCT_CGROUP);
        FDB_InitDefaultDoor();
	//将所有用户默认为1组
	TUser tmpuser;
	int i;
	TZKFPTemplate template;
	lseek(fdUser,0,SEEK_SET);
	while(read(fdUser, &tmpuser, sizeof(TUser)) == sizeof(TUser))
	{
		if(tmpuser.PIN <= 0) {
			continue;
		}
		tmpuser.Group = 1;
		lseek(fdUser,-1*sizeof(TUser),SEEK_CUR);
		write(fdUser,&tmpuser, sizeof(TUser));
		if(!gOptions.IsOnlyRFMachine && ((gOptions.ZKFPVersion == ZKFPV10 && fhdl) 
			|| gOptions.ZKFPVersion != ZKFPV10))
		{
			for(i=0;i<MAX_USER_FINGER_COUNT;i++)
			{
		                memset(&template,0,sizeof(TZKFPTemplate));
    	        		if(FDB_GetTmp(tmpuser.PIN, (char)i, (char *)&template)>0)
				{
					if(gOptions.ZKFPVersion == ZKFPV10)
					{
						template.Valid = template.Valid & (~DURESSFINGERTAG);
						//FDB_AddTmp(&template);
						FDB_SetDuressTemp(tmpuser.PIN,i,template.Valid);
					}		
					else
					{
						FDB_ChgTmpValidTag((PTemplate)&template, 1, 0xFE);
					}
				}
			}
		}
	}

	
	//恢复默认参数
	gOptions.LockOn=10;
	gOptions.OpenDoorDelay=10;
	gOptions.DoorSensorMode=2;
	gOptions.DoorSensorTimeout=30;
	gOptions.ErrTimes=3;
	gOptions.DoorCloseTimeZone=0;
	gOptions.DoorOpenTimeZone=0;
	gOptions.IsHolidayValid=0;
        gOptions.DuressHelpKeyOn=0;
        gOptions.Duress1To1=0;
        gOptions.Duress1ToN=0;
        gOptions.DuressPwd=0;
        gOptions.DuressAlarmDelay=10;
	gOptions.AntiPassbackOn=0;
	gOptions.MasterState=0;
	SaveOptions(&gOptions);
	sync();
}

static int RestoreOK(HWND hWnd, HWND Item[5], int Items)
{
	HWND stahwnd;

        stahwnd = createStatusWin1(hWnd , 250 , 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(MID_WAIT));
        busyflag = 1;
	switch(Items)
	{
		//恢复全部出厂设置
		case 0:
	        	SaveOptions(GetDefaultOptions(&gOptions));
	        	ClearOptionItem("NONE");        
                        if (gOptions.IsSupportWIFI){
                                resetwifi();
			}

                        if (gOptions.IsSupportModem){
                                resetgprs();
			}

                        //初始化快捷键
			FDB_ClearData(FCT_SHORTKEY);
                        FDB_InitShortKey();
                        //init Alarm
			if (!gOptions.UseNewBell && FDB_CntAlarm()>0)
			{
				FDB_ClearData(FCT_ALARM);
                        	FDB_InitAlarm();
			}
			else if (gOptions.UseNewBell && FDB_CntBell()>0)
			{			
				//init Bell
				FDB_ClearData(FCT_BELL);
				FDB_InitBell();
			}

			if(gOptions.LockFunOn || gOptions.AttUseTZ)
				resetlockfunon();
			sync();
			break;
		case 1:
                        //初始化快捷键
			FDB_ClearData(FCT_SHORTKEY);
			sync();
			printf(" end FDB_ClearData \n");
                        FDB_InitShortKey();
			break;
		case 2:
                        //初始化闹铃
	                if (!gOptions.UseNewBell && FDB_CntAlarm()>0)
			{
				FDB_ClearData(FCT_ALARM);
        	                FDB_InitAlarm();
			}
	                else if (gOptions.UseNewBell && FDB_CntBell()>0)
			{
				FDB_ClearData(FCT_BELL);
        	                FDB_InitBell();
			}
			sync();
			break;
		case 3:
	        	SaveOptions(GetDefaultOptions(&gOptions));
	        	ClearOptionItem("NONE");        
                        if (gOptions.IsSupportWIFI){
                                resetwifi();
			}

			if(gOptions.IsSupportModem){
                                resetgprs();
			}

			sync();
			break;
		case 4:
			resetlockfunon();
			break;
	}
	destroyStatusWin1 (stahwnd);
	SetMenuTimeOut(time(NULL));
    FDB_AddOPLog(ADMINPIN, OP_RES_OPTION, 0, 0, 0, 0);
	if(ismenutimeout) return 0;
        MessageBox1 (hWnd ,LoadStrByID(HIT_UPDATEINFO),LoadStrByID(HIT_RUN),MB_OK | MB_ICONINFORMATION);
        return 1;
}

static int RestoreDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_INITDIALOG:
			//if (LoadBitmap(HDC_SCREEN,&Restorebk,GetBmpPath("submenubg.jpg")))
			//	return 0;

			SetWindowText(GetDlgItem(hDlg,IDC_RESTOREPARA),LoadStrByID(MID_RESTOREPARA));
			SetWindowText(GetDlgItem(hDlg,IDC_RESTORESHORTKEY),LoadStrByID(MID_RESTORESHORTKEY));
			SetWindowText(GetDlgItem(hDlg,IDC_RESTOREALARM),LoadStrByID(MID_RESTOREALARM));
			SetWindowText(GetDlgItem(hDlg,IDC_RESTOREOTHER),LoadStrByID(MID_RESTOREOTHER));
			if (gOptions.LockFunOn ||gOptions.AttUseTZ)
			{
				SetWindowText(GetDlgItem(hDlg,IDC_RESTORELOCKFUN),LoadStrByID(MID_RESTORELOCKFUN));
			}

			RestoreItemWnd[0] = GetDlgItem (hDlg, IDC_RESTOREPARA);
			RestoreItemWnd[1] = GetDlgItem (hDlg, IDC_RESTORESHORTKEY);
			RestoreItemWnd[2] = GetDlgItem (hDlg, IDC_RESTOREALARM);
			RestoreItemWnd[3] = GetDlgItem (hDlg, IDC_RESTOREOTHER);
			if (gOptions.LockFunOn || gOptions.AttUseTZ)
			{
				RestoreItemWnd[4] = GetDlgItem (hDlg, IDC_RESTORELOCKFUN);
			}

			if (!gOptions.AutoAlarmFunOn)
			{
				SendMessage(RestoreItemWnd[2], MSG_ENABLE, 0, 0);
			}

	                RestoreItem=0;
			SetFocus(RestoreItemWnd[0]);
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				char buffer[32];

				GetWindowText(RestoreItemWnd[RestoreItem],buffer,32);
				TTS_Say(buffer);
			}
#endif			
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

			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

			if (fGetDC)
				ReleaseDC (hdc);
			return 0;
		}

		case MSG_PAINT:
    			hdc=BeginPaint(hDlg);
                        EndPaint(hDlg,hdc);
			return 0;
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

                        if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
                        {
				if(++RestoreItem > ((gOptions.LockFunOn ||gOptions.AttUseTZ)?4:3))
					RestoreItem = 0;
				if(!gOptions.AutoAlarmFunOn && RestoreItem==2)
				{
					RestoreItem++;
				}
                                SetFocus(RestoreItemWnd[RestoreItem]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];

					GetWindowText(RestoreItemWnd[RestoreItem],buffer,32);
					TTS_Say(buffer);
				}
#endif                                
                                return 0;
                        }
                        else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
                        {
				if(--RestoreItem < 0)
					RestoreItem = ((gOptions.LockFunOn ||gOptions.AttUseTZ)?4:3);
				if(!gOptions.AutoAlarmFunOn && RestoreItem==2)
				{
					RestoreItem--;
				}
                                SetFocus(RestoreItemWnd[RestoreItem]);
#ifdef _TTS_
	                if(gOptions.TTS_KEY)
			{
				char buffer[32];

				GetWindowText(RestoreItemWnd[RestoreItem],buffer,32);
				TTS_Say(buffer);
			}
#endif                                
                                return 0;
                        }
			else if (LOWORD(wParam)==SCANCODE_ENTER || LOWORD(wParam)==SCANCODE_MENU || LOWORD(wParam)==SCANCODE_F10)
                        {
				if(RestoreConfirm(hDlg, RestoreItem))
                                	RestoreOK(hDlg, RestoreItemWnd, RestoreItem);
                                return 0;
                        }
                        else if ((LOWORD(wParam)==SCANCODE_ESCAPE))
                        {
				PostMessage(hDlg, MSG_CLOSE, 0, 0);
                                return 0;
                        }
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
				return 0;
                        break;

		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
				TTS_Stop();
#endif			
			//UnloadBitmap(&Restorebk);
			EndDialog(hDlg, IDCANCEL);
			return 0;
	}
    
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int  SSR_MENU_RESTORE(HWND hWnd)
{
    if (gOptions.LockFunOn || gOptions.AttUseTZ)
    {	
		RestoreLockDlgBox.w=gOptions.LCDWidth-1;
		RestoreLockDlgBox.h=gOptions.LCDHeight-1;

		RestoreLockCtrl[0].x=40+gOptions.GridWidth*2;
		RestoreLockCtrl[1].x=40+gOptions.GridWidth*2;
		RestoreLockCtrl[2].x=40+gOptions.GridWidth*2;
		RestoreLockCtrl[3].x=40+gOptions.GridWidth*2;
		RestoreLockCtrl[4].x=40+gOptions.GridWidth*2;

    		RestoreLockDlgBox.caption=LoadStrByID(HIT_SYSTEM8);
	    	RestoreLockDlgBox.controls = RestoreLockCtrl;
    		DialogBoxIndirectParam (&RestoreLockDlgBox, hWnd, RestoreDialogBoxProc, 0L);
    }
    else
    {
		RestoreDlgBox.w=gOptions.LCDWidth-1;
		RestoreDlgBox.h=gOptions.LCDHeight-1;

		RestoreCtrl[0].x=40+gOptions.GridWidth*2;
		RestoreCtrl[1].x=40+gOptions.GridWidth*2;
		RestoreCtrl[2].x=40+gOptions.GridWidth*2;
		RestoreCtrl[3].x=40+gOptions.GridWidth*2;

    		RestoreDlgBox.caption=LoadStrByID(HIT_SYSTEM8);
	    	RestoreDlgBox.controls = RestoreCtrl;
    		DialogBoxIndirectParam (&RestoreDlgBox, hWnd, RestoreDialogBoxProc, 0L);
    }
    return 0;
}
