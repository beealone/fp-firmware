/* 
 * SSR 2.0 Self Service Record 主入口头文件
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0
 * 修改记录:
 * 编译环境:mipsel-gcc
 */

#include <minigui/common.h>

#include "ssrcommon.h"
#include "flashdb.h"
#include "ssrsystem2pic.h"
#include "ssrpub.h"
#include "ssrsystem2.h"
#include "updatepic.h"
#include "tftmsgbox.h"
#include "utils.h"


static DLGTEMPLATE System2DlgBox =
{
	WS_BORDER | WS_CAPTION, 
	WS_EX_NONE,
	1, 1, 319, 239, 
	"",
	0, 0,
	6, NULL,
	0
};

#define IDC_CLEARREC   		3110    
#define IDC_CLEARDATA  		3120
#define IDC_CLEARMAN   		3130
#define IDC_CLEARPIC   		3140
#define IDC_CLEARATTPIC		3150
#define IDC_CLEARHACKPIC	3160

static CTRLDATA System2Ctrl [] =
{
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		60, 25, 200, 25,
		IDC_CLEARREC,
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		60, 55, 200, 25,
		IDC_CLEARATTPIC,
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		60, 85, 200, 25,
		IDC_CLEARHACKPIC,
		"",
		0

	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		60, 115, 200, 25,
		IDC_CLEARDATA,
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		60, 145, 200, 25,
		IDC_CLEARMAN,
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		60, 175, 200, 25,
		IDC_CLEARPIC,
		"",
		0
	},

};

static HWND System2ItemWnd[6];
static int System2Item;
//static BITMAP system2bk;
extern int gCurCaptureCount; //current capture picture count, for count capture

static int System2Confirm(HWND hWnd,int Item)
{
	if(Item!=5 && Item!=1) {
		if(MessageBox1(hWnd,LoadStrByID(HIT_SYSTEM2INFO),LoadStrByID(HIT_RUN),MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)!=IDOK) {
			return 0;
		} else {
			return 1;
		}
	} else {
		return 1;
	}
}

extern int SSR_MENU_ATTPHOTO(HWND hWnd);
static int System2OK(HWND hWnd, HWND Item[5], int Items)
{
	char commandstr[100];
	int i=0;

	memset(commandstr,0,sizeof(commandstr));
	switch(Items)
	{
		case 0:
			i=FDB_ClearData(FCT_ATTLOG);
			FDB_AddOPLog(ADMINPIN, OP_CLEAR_LOG, 0, i, 0, 0);
			break;

		case 1:	//删除考勤照片
			SSR_MENU_ATTPHOTO(hWnd);
/*
			//zsliu add 2009-10-14
			//删除考勤照片时直接删除文件夹，避免删除3000以上照片时，提示Arugument list too long的错误，并且无法删除。
			sprintf(commandstr, "rm -rf %s/ && sync", GetCapturePath("capture/pass"));
			printf("delete pass document:%s\n",commandstr);
			system(commandstr);

			//再创建 pass 文件夹。
			sprintf(commandstr,"mkdir %s && sync",GetCapturePath("capture/pass"));
			printf("mkdir pass document:%s\n",commandstr);
			system(commandstr);

			FDB_ClearIndex(0);
			sync();
*/
			gCurCaptureCount = FDB_CntPhotoCount();
			//add end

			break;
		case 2: //删除黑名单图片
			//zsliu add 2009-10-14
			//删除考勤照片时直接删除文件夹，避免删除3000以上照片时，提示Arugument list too long的错误，并且无法删除。
			sprintf(commandstr, "rm -rf %s/ && sync", GetCapturePath("capture/bad"));
			printf("delete bad document:%s\n",commandstr);
			systemEx(commandstr);

			//再创建 bad 文件夹。
			sprintf(commandstr,"mkdir %s && sync",GetCapturePath("capture/bad"));
			printf("mkdir bad document:%s\n",commandstr);
			systemEx(commandstr);

			FDB_ClearIndex(1);
			sync();

			gCurCaptureCount = FDB_CntPhotoCount();
			//add end
			break;
		case 3:
			i=FDB_ClearData(FCT_ALL);
			FDB_AddOPLog(ADMINPIN, OP_CLEAR_DATA, 0, i, 0, 0);

			//初始化快捷键，闹铃，默认门禁设置
			if (gOptions.CameraOpen)
			{
				//zsliu add 2009-10-14
				//删除考勤照片时直接删除文件夹，避免删除3000以上照片时，提示Arugument list too long的错误，并且无法删除。
				sprintf(commandstr, "rm -rf %s/ && sync", GetCapturePath("capture/pass"));
				systemEx(commandstr);
				sprintf(commandstr,"mkdir %s && sync",GetCapturePath("capture/pass"));//再创建 pass 文件夹。
				systemEx(commandstr);
				FDB_ClearIndex(0);
				sync();

				memset(commandstr, 0, sizeof(commandstr));

				sprintf(commandstr, "rm -rf %s/ && sync", GetCapturePath("capture/bad"));
				systemEx(commandstr);
				sprintf(commandstr,"mkdir %s && sync",GetCapturePath("capture/bad"));//再创建 bad 文件夹。
				systemEx(commandstr);
				FDB_ClearIndex(1);
				sync();
				//add end
			}
			//clear webserver index file
			unlink("./dpm.dat");
			unlink("./dpmidx.dat");
			systemEx("rm /mnt/mtdblock/*.tmp -rf");   //add by jazzy 2010.01.25 for clear download error data
			/*add 2012-11-16 zxz*/
			if(!gOptions.IsOnlyRFMachine) {
				MessageBox1(hWnd, LoadStrByID(HID_RESTART), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
				RebootMachine();
			}
			gCurCaptureCount = 0;
			//if (gOptions.IclockSvrFun > 0) {
				//add by luoxw: power off machine when clear all data			
				//MessageBox1(hWnd, LoadStrByID(HID_RESTART), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
				//RebootMachine();
			//}		
			break;
		case 4:
			i=FDB_ClrAdmin();
			FDB_AddOPLog(ADMINPIN, OP_CLEAR_ADMIN, 0, i, 0, 0);
			break;
		case 5:
			for(uPic=0;uPic<16;uPic++)
				memset(uPicList[uPic],0,16);
			uPic=0;
			uPic=UpdatePic(GetPicPath("adpic"),"ad_",uPicList);

			if(uPic)
				SSR_MENU_DPICDLG(hWnd);
			else
				MessageBox1(hWnd,LoadStrByID(HIT_UINFO16),LoadStrByID(HIT_RUN),MB_OK | MB_ICONINFORMATION);
			break;
	}

	if(ismenutimeout)
	{
		return 0;
	}
	if(Items == 1)
	{
		return 1;
	}

	if(Items <= 3)
		MessageBox1 (hWnd ,LoadStrByID(HIT_SYSTEM2DATA5) ,LoadStrByID(HIT_RUN),MB_OK | MB_ICONINFORMATION);
	else if(Items==4)
		MessageBox1 (hWnd ,LoadStrByID(HIT_SYSTEM2DATA6) ,LoadStrByID(HIT_RUN),MB_OK | MB_ICONINFORMATION);
	return 1;
}

static int System2DialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_INITDIALOG:
			//if (LoadBitmap(HDC_SCREEN,&system2bk,GetBmpPath("submenubg.jpg"))) {
			//	return 0;
			//}

			SetWindowText(GetDlgItem(hDlg,IDC_CLEARREC),LoadStrByID(HIT_SYSTEM2DATA2));
			SetWindowText(GetDlgItem(hDlg,IDC_CLEARDATA),LoadStrByID(HIT_SYSTEM2DATA3));
			SetWindowText(GetDlgItem(hDlg,IDC_CLEARMAN),LoadStrByID(HIT_SYSTEM2DATA4));
			SetWindowText(GetDlgItem(hDlg,IDC_CLEARPIC),LoadStrByID(HIT_SYSTEM2DATA7));
			SetWindowText(GetDlgItem(hDlg, IDC_CLEARATTPIC), LoadStrByID(PID_PHOTO_MNG));
			SetWindowText(GetDlgItem(hDlg, IDC_CLEARHACKPIC), LoadStrByID(MID_QUERY_SET3));

			System2ItemWnd[0] = GetDlgItem (hDlg, IDC_CLEARREC);
			System2ItemWnd[1] = GetDlgItem (hDlg, IDC_CLEARATTPIC);
			System2ItemWnd[2] = GetDlgItem (hDlg, IDC_CLEARHACKPIC);
			System2ItemWnd[3] = GetDlgItem (hDlg, IDC_CLEARDATA);
			System2ItemWnd[4] = GetDlgItem (hDlg, IDC_CLEARMAN);
			System2ItemWnd[5] = GetDlgItem (hDlg, IDC_CLEARPIC);
			if (!gOptions.CameraOpen)
			{
				SendMessage(System2ItemWnd[1], MSG_ENABLE, 0, 0);
				SendMessage(System2ItemWnd[2], MSG_ENABLE, 0, 0);
			}
			System2Item=0;

#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				char buffer[32];

				GetWindowText(System2ItemWnd[System2Item],buffer,32);
				TTS_Say(buffer);
			}
#endif
			return 1;

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
				if(++System2Item > 5)
					System2Item = 0;
				if(!gOptions.CameraOpen && (System2Item==1 || System2Item==2))
					System2Item = 3;
				SetFocus(System2ItemWnd[System2Item]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];

					GetWindowText(System2ItemWnd[System2Item],buffer,32);
					TTS_Say(buffer);
				}
#endif
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if(--System2Item < 0)
					System2Item = 5;
				if(!gOptions.CameraOpen && (System2Item==1 || System2Item==2))
					System2Item = 0;
				SetFocus(System2ItemWnd[System2Item]);
#ifdef _TTS_
				if(gOptions.TTS_KEY)
				{
					char buffer[32];

					GetWindowText(System2ItemWnd[System2Item],buffer,32);
					TTS_Say(buffer);
				}
#endif                                
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_ENTER || LOWORD(wParam)==SCANCODE_MENU || LOWORD(wParam)==SCANCODE_F10)
			{
				if(System2Confirm(hDlg, System2Item)) {
					System2OK(hDlg, System2ItemWnd, System2Item);
				}
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
			//UnloadBitmap(&system2bk);
			EndDialog(hDlg, IDCANCEL);
			return 0;
	}

	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_SYSTEM2(HWND hWnd)
{
	System2DlgBox.w=gOptions.LCDWidth-1;
	System2DlgBox.h=gOptions.LCDHeight-1;

	System2Ctrl[0].x = 60+gOptions.GridWidth*2;
	System2Ctrl[1].x = 60+gOptions.GridWidth*2;
	System2Ctrl[2].x = 60+gOptions.GridWidth*2;
	System2Ctrl[3].x = 60+gOptions.GridWidth*2;
	System2Ctrl[4].x = 60+gOptions.GridWidth*2;
	System2Ctrl[5].x = 60+gOptions.GridWidth*2;

	System2DlgBox.caption=LoadStrByID(HIT_SYSTEM2);
	System2DlgBox.controls = System2Ctrl;
	DialogBoxIndirectParam (&System2DlgBox, hWnd, System2DialogBoxProc, 0L);
}
