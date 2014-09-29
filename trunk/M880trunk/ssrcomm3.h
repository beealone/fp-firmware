/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录:
* 编译环境:mipsel-gcc
*/


#ifndef __CWX_GUI_COMM3 

#define __CWX_GUI_COMM3

#include "ssrcommon.h"
#include "options.h"
#include "ssrpub.h"
#include <minigui/tftmullan.h>

static DLGTEMPLATE Comm3DlgBox =
{
    WS_BORDER | WS_CAPTION, 
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    17, NULL,
    0
};

#define IDC_DEVICEID 250
#define IDC_LINK     251
#define IDC_RIS     252
#define LB_RIS     254
#define LB_RISIP     255
#define IDC_RISIP0     256
#define IDC_RISIP1     257
#define IDC_RISIP2     258
#define IDC_RISIP3     259
#define IDC_PRINT	260

static CTRLDATA Comm3Ctrl [] =
{ 
	{ CTRL_STATIC, WS_VISIBLE | SS_LEFT, 10, 20, 80, 23, 0x006, "", 0 },
	{ CTRL_SLEDIT, WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, 95, 16, 50, 23, IDC_DEVICEID, "0", 0 },
	{ CTRL_STATIC, WS_VISIBLE | SS_LEFT, 150, 20, 120, 23, 0x007, "", 0 },

	{ CTRL_STATIC, WS_VISIBLE | SS_LEFT, 10, 50, 80, 23, 0x008, "", 0 },
#ifdef HITFLAG
	{ CTRL_SLEDIT, WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT | ES_PASSWORD, 95, 46, 50, 23, IDC_LINK, "0", 0 },
#else
	{ CTRL_SLEDIT, WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, 95, 46, 50, 23, IDC_LINK, "0", 0 },
#endif
	{ CTRL_STATIC, WS_VISIBLE | SS_LEFT, 150, 50, 200, 23, 0x009, "", 0 },

        { CTRL_STATIC, SS_LEFT, 10, 80, 80, 23, 0x010, "", 0},
        { CTRL_COMBOBOX, WS_TABSTOP|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS, 85, 76, 70, 23, IDC_PRINT, "", 0},

	{ CTRL_STATIC, SS_LEFT, 10, 110, 80, 23, LB_RIS, "", 0 },
	{ CTRL_COMBOBOX, WS_TABSTOP|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS, 85, 106, 70, 23, IDC_RIS, "", 0 },
	{ CTRL_STATIC, SS_LEFT, 10, 140, 80, 23, LB_RISIP, "", 0 },
	{ CTRL_SLEDIT, WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, 105, 136, 35, 23, IDC_RISIP0, "0", 0 },
	{ CTRL_SLEDIT, WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, 145, 136, 35, 23, IDC_RISIP1, "0", 0 },
	{ CTRL_SLEDIT, WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, 185, 136, 35, 23, IDC_RISIP2, "0", 0 },
	{ CTRL_SLEDIT, WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT, 225, 136, 35, 23, IDC_RISIP3, "0", 0 },

	{ CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 225, 180, 85, 25, IDCANCEL, "", 0 },
	{ CTRL_BUTTON, WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 180, 85, 25, IDOK, "", 0 }
};

static HWND Comm3ItemWnd[10];
static int Comm3Item;
//static BITMAP comm3bk;

static char *AuthNames[]={"LO","NL", "NO", "LN"};
static int Comm3OK(HWND hWnd,HWND Item[],int Size)
{
        char c1[16],c2[16],c3[16],c4[16];
        int sel;
        int err=0;
	int changeflag = 0;

        GetWindowText (Item[0],c1,16);
        GetWindowText (Item[1],c2,16);

        if(CheckText2(c1,1,254))
        {
		if(gOptions.DeviceID != atoi(c1)) {
	                SaveInteger("DeviceID",atoi(c1));
			gOptions.DeviceID=atoi(c1);
			changeflag = 1;
		}
        }
        else ++err;

	if(CheckText2(c2,0,999999))
        {
		if(gOptions.ComKey != atoi(c2)) {
	                SaveInteger("COMKey",atoi(c2));
			gOptions.ComKey=atoi(c2);
			SetRootPwd(gOptions.ComKey);
			changeflag = 1;
		}
        }
        else ++err;

	if (gOptions.PrinterFunOn)
	{
		if(gOptions.PrinterOn != SendMessage(Item[2], CB_GETCURSEL, 0, 0)) {
			gOptions.PrinterOn=SendMessage(Item[2], CB_GETCURSEL, 0, 0);
			changeflag = 1;
		}
	}

	if (gOptions.IsSupportAuthServer && gOptions.IsSupportModem<=1)
	{
	        GetWindowText (Item[4],c1,16);
        	GetWindowText (Item[5],c2,16);
	        GetWindowText (Item[6],c3,16);
        	GetWindowText (Item[7],c4,16);
		/*此处删除判断2012-11-21*/
	        //if(CheckIP(c1,c2,c3,c4,1))
        	{
			if(gOptions.AuthServerIP[0] != atoi(c1) ||gOptions.AuthServerIP[1] != atoi(c2) ||
				 gOptions.AuthServerIP[2] != atoi(c3) ||gOptions.AuthServerIP[3] != atoi(c4)) {
	                	gOptions.AuthServerIP[0]=atoi(c1);
		                gOptions.AuthServerIP[1]=atoi(c2);
	        	        gOptions.AuthServerIP[2]=atoi(c3);
	                	gOptions.AuthServerIP[3]=atoi(c4);
				changeflag = 1;
			}
        	}
	        //else err=1;

	        sel=SendMessage(Item[3],CB_GETCURSEL,0,0);
		if(gOptions.AuthServerEnabled != sel) {
			gOptions.AuthServerEnabled=sel;
			changeflag = 1;
		}
	}

        if (err)
        {
                MessageBox1 (hWnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                return 0;
        }
        else
	{
		if(changeflag) {
			SaveOptions(&gOptions);
	                MessageBox1 (hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
		}
	}
        return 1;
}

static int Check3Item(int item)
{
        char cc[256];
        int mm;
        GetWindowText(Comm3ItemWnd[item],cc,256);
        mm=atol(cc);
        if(Comm3Item>=4&&Comm3Item<=7)
        {
                if(strlen(cc) && (mm>=0&&mm<=255)) return 1;
                else return 0;
        }
	return 0;
}

static void comm3_notif_proc(HWND hwnd, int id, int nc, DWORD add_data)
{
    	unsigned int len;
    	if (nc == EN_CHANGE) 
    	{
    		if(id>=IDC_RISIP0&&id<=IDC_RISIP3)
    		{
    			len=GetWindowTextLength(hwnd);
    			if(len>=3)
    			{
    				if(Check3Item(Comm3Item))
    				{
    					if(Comm3Item>=4&&Comm3Item<=7)
    						++Comm3Item;
    					SetFocus(Comm3ItemWnd[Comm3Item]);
    				} else {
    					MessageBox1 (hwnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
    				}
    			}
    		}
    	}
}

static int Comm3DialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//int sel;
	char c1[16],c2[16],c3[16],c4[17];
	char buffer[256];
	char tmpstr[20];
	int i, tmpvalue = 0;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_INITDIALOG:
	
			//if (LoadBitmap(HDC_SCREEN,&comm3bk,GetBmpPath("submenubg.jpg")))
                        //	return 0;

			SetWindowText(GetDlgItem(hDlg,0x006),LoadStrByID(HIT_COMM3LINK1));
		        SetWindowText(GetDlgItem(hDlg,0x007),LoadStrByID(HIT_COMM3LINK3));
			SetWindowText(GetDlgItem(hDlg,0x008),LoadStrByID(HIT_COMM3LINK2));
		        SetWindowText(GetDlgItem(hDlg,0x009),LoadStrByID(HIT_COMM3LINK4));

			memset(tmpstr, 0, sizeof(tmpstr));
			sprintf(tmpstr, "%s%s", LoadStrByID(MID_PRINTER), LoadStrByID(MID_PT_MODE));
			SetWindowText(GetDlgItem(hDlg,0x010), tmpstr);					//printer mode

			SetWindowText(GetDlgItem(hDlg,LB_RIS),LoadStrByID(MID_AUTHSERVER));		//AuthServer functions
			SetWindowText(GetDlgItem(hDlg,LB_RISIP),LoadStrByID(MID_AUTHSERVER_IP));	//AuthServer IP

		        SetWindowText(GetDlgItem(hDlg,IDCANCEL),LoadStrByID(HIT_CANCEL));
		        SetWindowText(GetDlgItem(hDlg,IDOK),LoadStrByID(HIT_OK));

		        Comm3ItemWnd[0] = GetDlgItem (hDlg, IDC_DEVICEID);
			Comm3ItemWnd[1] = GetDlgItem (hDlg, IDC_LINK);
			Comm3ItemWnd[2] = GetDlgItem (hDlg, IDC_PRINT);
			Comm3ItemWnd[3] = GetDlgItem (hDlg, IDC_RIS);
			Comm3ItemWnd[4] = GetDlgItem (hDlg, IDC_RISIP0);
			Comm3ItemWnd[5] = GetDlgItem (hDlg, IDC_RISIP1);
			Comm3ItemWnd[6] = GetDlgItem (hDlg, IDC_RISIP2);
			Comm3ItemWnd[7] = GetDlgItem (hDlg, IDC_RISIP3);
			Comm3ItemWnd[8] = GetDlgItem (hDlg, IDOK);
		        Comm3ItemWnd[9] = GetDlgItem (hDlg, IDCANCEL);

			if(gOptions.PrinterFunOn)
			{
				ShowWindow(GetDlgItem(hDlg, 0x010), SW_SHOW);
				ShowWindow(Comm3ItemWnd[2], SW_SHOW);
                                SendMessage(Comm3ItemWnd[2], CB_ADDSTRING, 0,(int) LoadStrByID(HIT_NULLKEY));        //无
                                for(i=0;i<7;i++)
                                {
                                        memset(tmpstr, 0, sizeof(tmpstr));
                                        sprintf(tmpstr, "%s%d", LoadStrByID(MID_PT_MODE), i+1);
                                        SendMessage(Comm3ItemWnd[2], CB_ADDSTRING, 0,(int ) tmpstr);
                                }
                                SendMessage(Comm3ItemWnd[2], CB_SETCURSEL, gOptions.PrinterOn, 0);
			}

			if(gOptions.IsSupportAuthServer && gOptions.IsSupportModem<=1)
			{
				ShowWindow(GetDlgItem(hDlg, LB_RIS), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, LB_RISIP), SW_SHOW);
				ShowWindow(Comm3ItemWnd[3], SW_SHOW);
				for(i=4;i<8;i++)
				{
					ShowWindow(Comm3ItemWnd[i], SW_SHOW);
					SendMessage(Comm3ItemWnd[i], EM_LIMITTEXT, 3, 0);
					SendDlgItemMessage(hDlg, IDC_RIS, CB_ADDSTRING, 0, (LPARAM)AuthNames[i-4]);
				}
				SendMessage(Comm3ItemWnd[3],CB_SETCURSEL,gOptions.AuthServerEnabled,0);

				LoadStr("AuthServerIP",buffer);
			        if(ParseIP(buffer,c1,c2,c3,c4))
		        	{
					SetWindowText(Comm3ItemWnd[4],c1);
			                SetWindowText(Comm3ItemWnd[5],c2);
		        	        SetWindowText(Comm3ItemWnd[6],c3);
		                	SetWindowText(Comm3ItemWnd[7],c4);
		        	}
				SetNotificationCallback(Comm3ItemWnd[4],comm3_notif_proc);
				SetNotificationCallback(Comm3ItemWnd[5],comm3_notif_proc);
				SetNotificationCallback(Comm3ItemWnd[6],comm3_notif_proc);
				SetNotificationCallback(Comm3ItemWnd[7],comm3_notif_proc);
			}

			SendMessage(Comm3ItemWnd[0],EM_LIMITTEXT,3,0);
			SendMessage(Comm3ItemWnd[1],EM_LIMITTEXT,6,0);
	
			SetFocus(Comm3ItemWnd[0]);
			Comm3Item=0;

			sprintf(c1,"%d",LoadInteger("DeviceID",0));
			SetWindowText(Comm3ItemWnd[0],c1);
		        sprintf(c2,"%d",LoadInteger("COMKey",0));
			SetWindowText(Comm3ItemWnd[1],c2);
			return 1;

	 case MSG_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            const RECT* clip = (const RECT*) lParam;
            BOOL fGetDC = FALSE;
            RECT rcTemp;

            if (hdc == 0) {
                hdc = GetClientDC (hDlg);
                fGetDC = TRUE;
            }

            if (clip) {
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
			if (gOptions.IsSupportAuthServer && gOptions.IsSupportModem<=1)
			{
				int pos1, pos2, pos3;
				if(fromRight==1)
				{
					pos1=78+gOptions.GridWidth*2;
					pos2=118+gOptions.GridWidth*2;
					pos3=158+gOptions.GridWidth*2;
				}
				else
				{
					pos1=128+gOptions.GridWidth*2;
					pos2=168+gOptions.GridWidth*2;
					pos3=208+gOptions.GridWidth*2;
				}
		     tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	            SetTextColor(hdc,PIXEL_lightwhite);
        	    TextOut(hdc,pos1,141,".");
                TextOut(hdc,pos2,141,".");
                TextOut(hdc,pos3,141,".");
			}
                        EndPaint(hDlg,hdc);
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
			if(gOptions.KeyPadBeep)
                                ExKeyBeep();

			if ((LOWORD(wParam)==SCANCODE_TAB))
                                return 0;
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
                        {
				if(gOptions.IsSupportAuthServer && Comm3Item>=4 && Comm3Item <=7)
				{
					if(!Check3Item(Comm3Item))
						return 1;
				}
				if(++Comm3Item>9) Comm3Item=0;
				if(!gOptions.PrinterFunOn && Comm3Item==2)
					Comm3Item++;
				if((!gOptions.IsSupportAuthServer || gOptions.IsSupportModem>1) && (Comm3Item>2 && Comm3Item<8))
					Comm3Item=8;
                                SetFocus(Comm3ItemWnd[Comm3Item]);
				return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
                        {
				if(gOptions.IsSupportAuthServer && Comm3Item>=4 && Comm3Item <=7)
				{
					if(!Check3Item(Comm3Item))
						return 1;
				}
				if(--Comm3Item<0) Comm3Item=9;
				if((!gOptions.IsSupportAuthServer || gOptions.IsSupportModem>1) && (Comm3Item>2 && Comm3Item<8))
					Comm3Item=2;
				if(!gOptions.PrinterFunOn && Comm3Item==2)
					Comm3Item--;
                                SetFocus(Comm3ItemWnd[Comm3Item]);
				return 1;
                        }
			else
                        if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_MENU)||(LOWORD(wParam)==SCANCODE_F10))                                       {
				if(Comm3Item<=8)
				{
					if(Comm3OK(hDlg,Comm3ItemWnd,3) && !ismenutimeout)
                                        	SendMessage(hDlg,MSG_CLOSE,0,0);
				}
				else
				if(Comm3Item==9)
					SendMessage(hDlg,MSG_CLOSE,0,0);
                                return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
                        {
				if(Comm3Item==2)
				{
					int g_sel=SendMessage(Comm3ItemWnd[2], CB_GETCURSEL, 0, 0);
					if(++g_sel>7) g_sel=0;
					SendMessage(Comm3ItemWnd[2], CB_SETCURSEL, g_sel, 0);
					return 1;
				}
                                if(Comm3Item==3)
                                {
                                        int g_sel=SendMessage(Comm3ItemWnd[3], CB_GETCURSEL, 0, 0);
                                        if(++g_sel>3) g_sel=0;
                                        SendMessage(Comm3ItemWnd[3], CB_SETCURSEL, g_sel, 0);
                                        return 1;
                                }

			}
			else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
                        {
				if(Comm3Item==2)
				{
					int g_sel=SendMessage(Comm3ItemWnd[2], CB_GETCURSEL, 0, 0);
					if(--g_sel<0) g_sel=7;
					SendMessage(Comm3ItemWnd[2], CB_SETCURSEL, g_sel, 0);
					return 1;
				}
                                if(Comm3Item==3)
                                {
                                        int g_sel=SendMessage(Comm3ItemWnd[3],CB_GETCURSEL,0,0);
                                        if(--g_sel<0) g_sel=3;
                                        SendMessage(Comm3ItemWnd[3],CB_SETCURSEL,g_sel,0);
					return 1;
				}
			}
			else
                        if ((LOWORD(wParam)==SCANCODE_ESCAPE))
                        {
                                SendMessage(hDlg,MSG_CLOSE,0,0);
                                return 1;
                        }
			break;
    case MSG_CLOSE:
	//UnloadBitmap(&comm3bk);
        EndDialog (hDlg, IDCANCEL);
        break;
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_COMM3(HWND hWnd)
{
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8;

	Comm3DlgBox.w=gOptions.LCDWidth;
	Comm3DlgBox.h=gOptions.LCDHeight;

	if (fromRight==1)		//add by jazzy 2008.07.14
	{
		posX1=230+gOptions.GridWidth*3;
		posX2=165+gOptions.GridWidth*2;
		posX3=40+gOptions.GridWidth*2;
		posX4=125+gOptions.GridWidth*2;
		posX5=85+gOptions.GridWidth*2;
		posX6=45+gOptions.GridWidth*2;
		posX7=10;
		posX8=225+gOptions.ControlOffset;
	}
	else
	{
		posX1=10;
		posX2=95+gOptions.GridWidth*2;
		posX3=150+gOptions.GridWidth*2;
		posX4=135+gOptions.GridWidth*2;
		posX5=175+gOptions.GridWidth*2;
		posX6=215+gOptions.GridWidth*2;
		posX7=225+gOptions.ControlOffset;
		posX8=10;
	}
	

		Comm3Ctrl[0].x=posX1;
		Comm3Ctrl[1].x=posX2;
		Comm3Ctrl[2].x=posX3;
		Comm3Ctrl[3].x=posX1;
		Comm3Ctrl[4].x=posX2;
		Comm3Ctrl[5].x=posX3;
		Comm3Ctrl[6].x=posX1;
		Comm3Ctrl[7].x=posX4;
		Comm3Ctrl[8].x=posX1;
		Comm3Ctrl[9].x=posX4;
		Comm3Ctrl[10].x=posX1;
		Comm3Ctrl[11].x=posX2;
		Comm3Ctrl[12].x=posX4;
		Comm3Ctrl[13].x=posX5;
		Comm3Ctrl[14].x=posX6;
		Comm3Ctrl[15].x=posX7;
		Comm3Ctrl[16].x=posX8;


    Comm3DlgBox.caption=LoadStrByID(HIT_COMM3);
    Comm3DlgBox.controls = Comm3Ctrl;
    DialogBoxIndirectParam (&Comm3DlgBox, hWnd, Comm3DialogBoxProc, 0L);
}


#endif
