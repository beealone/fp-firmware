/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0
* 修改记录:
* 编译环境:mipsel-gcc
*/


#ifndef __CWX_GUI_COMM2 
#define __CWX_GUI_COMM2

#include "ssrcommon.h"
#include "ssrpub.h"
#include <minigui/tftmullan.h>

static DLGTEMPLATE Comm2DlgBox ={ WS_BORDER | WS_CAPTION, WS_EX_NONE, 1, 1, 319, 239, "", 0, 0, 11, NULL, 0};

#define IDC_BAUTE    260
#define IDC_RS232    261
#define IDC_RS485    262
#define IDC_USB	     263
#define IDC_SUSB     264
#define IDC_SRS232    265
#define IDC_SRS485    266
#define IDC_SBPS		267

#define COMM2BASEHEIGHT1	25
#define COMM2BASEHEIGHT2        20
#define COMM2ROWSPACING		30

	
static CTRLDATA Comm2Ctrl [] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, COMM2BASEHEIGHT1, 60, 20, 
        0x005, 
      	"",
        0
    },
    {
	CTRL_COMBOBOX,
        WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
        80, COMM2BASEHEIGHT2, 80, 25,
        IDC_BAUTE,
        "",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        170, COMM2BASEHEIGHT1, 30, 20,
       // IDC_STATIC,
       IDC_SBPS,
        "BPS",
        0
    },
    {
        CTRL_STATIC,
//      WS_VISIBLE | 
        SS_LEFT,
        10, COMM2BASEHEIGHT1+COMM2ROWSPACING, 60, 20,
        IDC_SRS232,
        "RS232",
        0
    },
    {
	CTRL_COMBOBOX,
//      WS_VISIBLE |
	WS_TABSTOP | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
        80, COMM2BASEHEIGHT2+COMM2ROWSPACING, 80, 25,
        IDC_RS232,
        "",
        0
    },
    {
        CTRL_STATIC,
//      WS_VISIBLE | 
	SS_LEFT,
        10, COMM2BASEHEIGHT1+COMM2ROWSPACING*2, 60, 20,
        IDC_SRS485,
        "RS485",
        0
    },
    {
        CTRL_COMBOBOX,
//      WS_VISIBLE |
        WS_TABSTOP | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
        80, COMM2BASEHEIGHT2+COMM2ROWSPACING*2, 80, 25,
        IDC_RS485,
        "",
        0
    },
    {
	CTRL_STATIC,
//	WS_VISIBLE | 
	SS_LEFT,
	10,COMM2BASEHEIGHT1+COMM2ROWSPACING*3,60,20,
	IDC_SUSB,
	"USB",
	0
    },
    {
        CTRL_COMBOBOX,
//	WS_VISIBLE |
	WS_TABSTOP | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
        80, COMM2BASEHEIGHT2+COMM2ROWSPACING*3, 80, 25,
        IDC_USB,
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        225, 180, 85, 25,
        IDCANCEL,
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        10, 180, 85, 25,
        IDOK, 
        "",
        0
    }
};

static HWND Comm2ItemWnd[7];
static int Comm2Item;
//static BITMAP comm2bk;
extern char *DtFmtStr[];
extern char *BauteStr[];

static int Comm2OK(HWND hWnd,HWND Item[],int Size)
{
        int sel;
	int err;
	int selvalue[3];
	int i;
	int changeflag = 0;

	err=0;
	for(i=0;i<3;i++)
		selvalue[i] = SendMessage(Item[i+1],CB_GETCURSEL,0,0);
	
	if((selvalue[0]==1 && selvalue[1]==1) ||
	   (selvalue[0]==1 && selvalue[2]==1) ||
	   (selvalue[1]==1 && selvalue[2]==1) ||
	   (selvalue[0]==1 && selvalue[1]==1 && selvalue[2]==1))
		err=1;

	if(err==0)
	{
		if (gOptions.RS232FunOn || gOptions.RS485FunOn)
		{
			sel=SendMessage(Item[0],CB_GETCURSEL,0,0);
        		//SaveInteger("RS232BaudRate",sel);
        		if(gOptions.RS232BaudRate != atoi(BauteStr[sel])) {
				gOptions.RS232BaudRate=atoi(BauteStr[sel]);
				changeflag = 1;
        		}
		}
        	
		if (gOptions.RS232FunOn)
		{
			sel=SendMessage(Item[1],CB_GETCURSEL,0,0);
		}
		else
		{
			sel = 0;
		}
        	//SaveInteger("RS232On",sel)
        	if(gOptions.RS232On != sel) {
			gOptions.RS232On=sel;
			changeflag = 1;
        	}

		if(sel)
		{
			gOptions.RS485On=0;
			gOptions.USB232On=0;
			if(gOptions.NetworkOn && (gOptions.RS232On || gOptions.RS485On) && LoadInteger(IsNetSwitch, 0))
        		{
                		gOptions.NetworkOn=0;
       			}
			//gOptions.RS485On = !gOptions.RS232On;
        		if(gOptions.IsSupportModem&&gOptions.RS485On)
        		{
                		gOptions.ModemEnable=0;
        		}
		}
	
		if (gOptions.RS485FunOn)
		{
			sel=SendMessage(Item[2],CB_GETCURSEL,0,0);
		}
		else
		{
			sel = 0;
		}
        	//SaveInteger("RS485On",sel);
        	if(gOptions.RS485On != sel) {
			gOptions.RS485On=sel;
			changeflag = 1;
        	}
		if(sel)
		{
			gOptions.RS232On=0;
			gOptions.USB232On=0;
			if(gOptions.NetworkOn && (gOptions.RS232On || gOptions.RS485On) && LoadInteger(IsNetSwitch, 0))
        		{
                		gOptions.NetworkOn=0;
        		}
        		//gOptions.RS232On =!gOptions.RS485On;
        		if(gOptions.IsSupportModem&&gOptions.RS485On)
        		{
                		gOptions.ModemEnable=0;
			}
		}
		
		if (gOptions.USB232FunOn)
		{
			sel=SendMessage(Item[3],CB_GETCURSEL,0,0);
		}
		else
		{
			sel = 0;
		}
		if(gOptions.USB232On != sel) {
			gOptions.USB232On=sel;
			changeflag = 1;
		}
		if(sel)
		{
			gOptions.RS232On=0;
			gOptions.RS485On=0;
			if(gOptions.NetworkOn && (gOptions.RS232On || gOptions.RS485On) && LoadInteger(IsNetSwitch, 0))
        		{
                		gOptions.NetworkOn=0;
        		}
        		//gOptions.RS232On =!gOptions.RS485On;
        		if(gOptions.IsSupportModem&&gOptions.RS485On)
        		{
                		gOptions.ModemEnable=0;
			}
		}
		/*如果不支持485/232功能，则不打开232/485功能，add by yangxiaolong,2011-9-5,start*/
		printf("RS485FunOn=%d,RS232FunOn=%d\n",gOptions.RS485FunOn,gOptions.RS232FunOn);
		printf("RS485On=%d,RS232On=%d\n",gOptions.RS485On,gOptions.RS232On);
		if (!gOptions.RS485FunOn)
		{
			gOptions.RS485On = 0;
		}
		if (!gOptions.RS232FunOn)
		{
			gOptions.RS232On = 0;
		}
		printf("RS485On=%d,RS232On=%d\n",gOptions.RS485On,gOptions.RS232On);
		/*如果不支持485/232功能，则不打开232/485功能，add by yangxiaolong,2011-9-5,end*/
		if(changeflag) {
			SaveOptions(&gOptions);
	        	//MessageBox1(hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
				MessageBox1(hWnd, LoadStrByID(HID_RESTART), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
		}
        	return 1;
	}
	else
	{
		MessageBox1(hWnd ,LoadStrByID(HIT_RSERROR) ,LoadStrByID(HIT_ERR),MB_OK| MB_ICONINFORMATION);
		return 0;
	}
}

static int Comm2DialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    int sel;
	static char keyupFlag=0;
    switch (message) {
    case MSG_INITDIALOG:
	//if (LoadBitmap(HDC_SCREEN,&comm2bk,GetBmpPath("submenubg.jpg")))
        //               return 0;

	SendDlgItemMessage(hDlg, IDC_RS232,CB_ADDSTRING ,0,(LPARAM)SwitchStr[0]);
	SendDlgItemMessage(hDlg, IDC_RS232,CB_ADDSTRING ,0,(LPARAM)SwitchStr[1]);

	SendDlgItemMessage(hDlg, IDC_RS485,CB_ADDSTRING ,0,(LPARAM)SwitchStr[0]);
        SendDlgItemMessage(hDlg, IDC_RS485,CB_ADDSTRING ,0,(LPARAM)SwitchStr[1]);

	SendDlgItemMessage(hDlg, IDC_USB,CB_ADDSTRING ,0,(LPARAM)SwitchStr[0]);
        SendDlgItemMessage(hDlg, IDC_USB,CB_ADDSTRING ,0,(LPARAM)SwitchStr[1]);

        SendDlgItemMessage(hDlg, IDC_BAUTE,CB_ADDSTRING ,0,(LPARAM)BauteStr[0]);
        SendDlgItemMessage(hDlg, IDC_BAUTE,CB_ADDSTRING ,0,(LPARAM)BauteStr[1]);
	SendDlgItemMessage(hDlg, IDC_BAUTE,CB_ADDSTRING ,0,(LPARAM)BauteStr[2]);
        SendDlgItemMessage(hDlg, IDC_BAUTE,CB_ADDSTRING ,0,(LPARAM)BauteStr[3]);
	SendDlgItemMessage(hDlg, IDC_BAUTE,CB_ADDSTRING ,0,(LPARAM)BauteStr[4]);

	if (gOptions.RS485FunOn || gOptions.RS232FunOn)
	{
		SetWindowText(GetDlgItem(hDlg,0x005),LoadStrByID(HIT_COMM2SER1));
	}
	
        SetWindowText(GetDlgItem(hDlg,IDCANCEL),LoadStrByID(HIT_CANCEL));
        SetWindowText(GetDlgItem(hDlg,IDOK),LoadStrByID(HIT_OK));

        Comm2ItemWnd[0] = GetDlgItem (hDlg, IDC_BAUTE);
	Comm2ItemWnd[1] = GetDlgItem (hDlg, IDC_RS232);
	Comm2ItemWnd[2] = GetDlgItem (hDlg, IDC_RS485);
	Comm2ItemWnd[3] = GetDlgItem (hDlg, IDC_USB);
	Comm2ItemWnd[4] = GetDlgItem (hDlg, IDOK);
        Comm2ItemWnd[5] = GetDlgItem (hDlg, IDCANCEL);
         Comm2ItemWnd[6] = GetDlgItem (hDlg, IDC_SBPS);
	
	if(gOptions.RS232FunOn)
	{
		ShowWindow(GetDlgItem(hDlg, IDC_SRS232), SW_SHOW);
		ShowWindow(Comm2ItemWnd[1], SW_SHOW);
	}

	if(gOptions.RS485FunOn)
	{
		ShowWindow(GetDlgItem(hDlg, IDC_SRS485), SW_SHOW);
		ShowWindow(Comm2ItemWnd[2], SW_SHOW);
	}

	//支持USB Client功能则显示设置项
	if(gOptions.USB232FunOn)
	{
		ShowWindow(GetDlgItem(hDlg, IDC_SUSB), SW_SHOW);
		ShowWindow(Comm2ItemWnd[3], SW_SHOW);
	}

	

	if (gOptions.RS485FunOn || gOptions.RS232FunOn)
	{
		SetFocus(Comm2ItemWnd[0]);
		Comm2Item=0;
		 sel=LoadInteger("RS232BaudRate",0);
		if(sel==atol(BauteStr[0]))
			SendMessage(Comm2ItemWnd[0],CB_SETCURSEL,0,0);
		else if(sel==atol(BauteStr[1]))
	                SendMessage(Comm2ItemWnd[0],CB_SETCURSEL,1,0);
	       else if(sel==atol(BauteStr[2]))
	                SendMessage(Comm2ItemWnd[0],CB_SETCURSEL,2,0);
	       else if(sel==atol(BauteStr[3]))
	                SendMessage(Comm2ItemWnd[0],CB_SETCURSEL,3,0);
	       else if(sel==atol(BauteStr[4]))
	                SendMessage(Comm2ItemWnd[0],CB_SETCURSEL,4,0);
	 }
	 else
	{
		ShowWindow(Comm2ItemWnd[0], SW_HIDE);
		ShowWindow(Comm2ItemWnd[6], SW_HIDE);
	}
	
	 if(gOptions.RS232FunOn)
	 {
		sel=LoadInteger("RS232On",0);
		SendMessage(Comm2ItemWnd[1],CB_SETCURSEL,sel,0);
	 }

	 if(gOptions.RS485FunOn)
	{
		sel=LoadInteger("RS485On",0);
		SendMessage(Comm2ItemWnd[2],CB_SETCURSEL,sel,0);
	}
	if(gOptions.USB232FunOn)
	{
		sel=LoadInteger("USB232On",0);
		SendMessage(Comm2ItemWnd[3],CB_SETCURSEL,sel,0);
	}

	
       
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
				if(Comm2Item<5)
                                        Comm2Item++;
                                else
                                        Comm2Item=0;
				if(!gOptions.RS232FunOn && Comm2Item==1)
					Comm2Item++;
				if(!gOptions.RS485FunOn && Comm2Item==2)
					Comm2Item++;
				if(!gOptions.USB232FunOn && Comm2Item==3)
					Comm2Item++;

                                SetFocus(Comm2ItemWnd[Comm2Item]);
				return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
                        {
				if(Comm2Item>0)
                                        Comm2Item--;
                                else
                                        Comm2Item=5;
				if(!gOptions.USB232FunOn && Comm2Item==3)
					Comm2Item--;
				if(!gOptions.RS485FunOn && Comm2Item==2)
					Comm2Item--;
				if(!gOptions.RS232FunOn && Comm2Item==1)
					Comm2Item--;

                                SetFocus(Comm2ItemWnd[Comm2Item]);
				return 1;
                        }
			else
			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
                        {
				if(Comm2Item==0)
                                {
					if(SendMessage(Comm2ItemWnd[0],CB_GETCURSEL,0,0)==4)
                                                SendMessage(Comm2ItemWnd[0],CB_SETCURSEL,0,0);
                                        else
						SendDlgItemMessage(hDlg,IDC_BAUTE,CB_SPIN,1,0);
                                        return 1;
                                }
				else
				if(Comm2Item==1)
                                {
					if(SendMessage(Comm2ItemWnd[1],CB_GETCURSEL,0,0)==1)
                                                SendMessage(Comm2ItemWnd[1],CB_SETCURSEL,0,0);
                                        else
						SendDlgItemMessage(hDlg,IDC_RS232,CB_SPIN,1,0);
                                        return 1;
                                }
				else
				if(Comm2Item==2)
                                {
					if(SendMessage(Comm2ItemWnd[2],CB_GETCURSEL,0,0)==1)
                                                SendMessage(Comm2ItemWnd[2],CB_SETCURSEL,0,0);
                                        else
						SendDlgItemMessage(hDlg,IDC_RS485,CB_SPIN,1,0);
                                        return 1;
                                }
				else
				if(Comm2Item==3)
				{
					if(SendMessage(Comm2ItemWnd[3],CB_GETCURSEL,0,0)==1)
                                                SendMessage(Comm2ItemWnd[3],CB_SETCURSEL,0,0);
                                        else
						SendDlgItemMessage(hDlg,IDC_USB,CB_SPIN,1,0);
                                        return 1;
                                }
				
                                return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
                        {
				if(Comm2Item==0)
				{
					if(SendMessage(Comm2ItemWnd[0],CB_GETCURSEL,0,0)==0)
                                                SendMessage(Comm2ItemWnd[0],CB_SETCURSEL,4,0);
                                        else
						SendDlgItemMessage(hDlg,IDC_BAUTE,CB_SPIN,0,0);	
					return 1;
				}
				else
                                if(Comm2Item==1)
                                {
					if(SendMessage(Comm2ItemWnd[1],CB_GETCURSEL,0,0)==0)
                                                SendMessage(Comm2ItemWnd[1],CB_SETCURSEL,1,0);
                                        else
                                        	SendDlgItemMessage(hDlg,IDC_RS232,CB_SPIN,0,0);
                                        return 1;
                                }
                                else
                                if(Comm2Item==2)
                                {
					if(SendMessage(Comm2ItemWnd[2],CB_GETCURSEL,0,0)==0)
                                                SendMessage(Comm2ItemWnd[2],CB_SETCURSEL,1,0);
                                        else
                                        	SendDlgItemMessage(hDlg,IDC_RS485,CB_SPIN,0,0);
                                        return 1;
                                }
                                if(Comm2Item==3)
                                {
					if(SendMessage(Comm2ItemWnd[3],CB_GETCURSEL,0,0)==0)
                                                SendMessage(Comm2ItemWnd[3],CB_SETCURSEL,1,0);
                                        else
                                        	SendDlgItemMessage(hDlg,IDC_USB,CB_SPIN,0,0);
                                        return 1;
                                }
                                return 1;
                        }
			else
                        if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_MENU)||(LOWORD(wParam)==SCANCODE_F10))
                        {
				if(Comm2Item<=4)
				{
                                	if(Comm2OK(hDlg,Comm2ItemWnd,5) && !ismenutimeout)	//?
                                        	SendMessage(hDlg,MSG_CLOSE,0,0);
				}
				else
				if(Comm2Item==5)
					SendMessage(hDlg,MSG_CLOSE,0,0);
                                return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_ESCAPE))
                        {
                                SendMessage(hDlg,MSG_CLOSE,0,0);
                                return 1;
                        }
			break;
    case MSG_CLOSE:
    	
	//UnloadBitmap(&comm2bk);
        EndDialog (hDlg, IDCANCEL);
        break;
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_COMM2(HWND hWnd)
{
    int posX1,posX2,posX3,posX4, posX5;

	Comm2DlgBox.w=gOptions.LCDWidth;
	Comm2DlgBox.h=gOptions.LCDHeight;

    if (fromRight==1)	//add by jazzy 2008.07.14
    {
		posX1=210+gOptions.GridWidth*2;
		posX2=110+gOptions.GridWidth*2;
		posX3=70;
		posX4=225+gOptions.ControlOffset;
		posX5=10;
	}
	else
	{
		posX1=10;
		posX2=80+gOptions.GridWidth*2;
		posX3=170+gOptions.GridWidth*2;
		posX4=10;
		posX5=225+gOptions.ControlOffset;
	}

	Comm2Ctrl[0].x=posX1;
	Comm2Ctrl[1].x=posX2;
	Comm2Ctrl[2].x=posX3;
	Comm2Ctrl[3].x=posX1;
	Comm2Ctrl[4].x=posX2;
	Comm2Ctrl[5].x=posX1;
	Comm2Ctrl[6].x=posX2;
	Comm2Ctrl[7].x=posX1;
	Comm2Ctrl[8].x=posX2;
	Comm2Ctrl[9].x=posX5;
	Comm2Ctrl[10].x=posX4;
	int Item=0;
	if (!(gOptions.RS232FunOn || gOptions.RS485FunOn) )
	{
		Item = -1;
	}
	if(gOptions.RS232FunOn)
	{
		Item++;
		Comm2Ctrl[3].y=COMM2BASEHEIGHT1+COMM2ROWSPACING*Item;
		Comm2Ctrl[4].y=COMM2BASEHEIGHT2+COMM2ROWSPACING*Item;
	}
	if(gOptions.RS485FunOn)
	{
		Item++;
		Comm2Ctrl[5].y=COMM2BASEHEIGHT1+COMM2ROWSPACING*Item;
		Comm2Ctrl[6].y=COMM2BASEHEIGHT2+COMM2ROWSPACING*Item;

	}
	if(gOptions.USB232FunOn)
	{
		Item++;
		Comm2Ctrl[7].y=COMM2BASEHEIGHT1+COMM2ROWSPACING*Item;
		Comm2Ctrl[8].y=COMM2BASEHEIGHT2+COMM2ROWSPACING*Item;
	}

    Comm2DlgBox.caption=LoadStrByID(HIT_COMM2);
    Comm2DlgBox.controls = Comm2Ctrl;
    DialogBoxIndirectParam (&Comm2DlgBox, hWnd, Comm2DialogBoxProc, 0L);
}


#endif
