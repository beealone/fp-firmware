/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录:
* 编译环境:mipsel-gcc
*/


#ifndef __CWX_GUI_COMM1
#define __CWX_GUI_COMM1

#include "ssrcommon.h"
#include "ssrpub.h"
#include <minigui/tftmullan.h>

static int gCommFag;
#define BASEHEIGHT	15
#define ROWSPACING	30

#define IDC_DHCP 209
#define IDC_IPADDRESS0 210
#define IDC_IPADDRESS1 211
#define IDC_IPADDRESS2 212
#define IDC_IPADDRESS3 213

#define IDC_SUBMASK0   220     
#define IDC_SUBMASK1   221
#define IDC_SUBMASK2   222
#define IDC_SUBMASK3   223

#define IDC_GREATWAY0  230
#define IDC_GREATWAY1  231
#define IDC_GREATWAY2  232
#define IDC_GREATWAY3  233

#define IDC_DNS0       240
#define IDC_DNS1       241
#define IDC_DNS2       242
#define IDC_DNS3       243

#define IDC_SPEED      250
//add by zxz 2012-11-03
static char iparray[4][5];
static char maskarray[4][5];
static char gatearray[4][5];
static char dnsarray[4][5];

static CTRLDATA Comm1Ctrl [] =
{ 	
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, BASEHEIGHT, 80, 20, 
        0x101, 
      	"",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        100, BASEHEIGHT, 35, 20,
        IDC_IPADDRESS0,
        "192",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        140, BASEHEIGHT, 35, 20,
        IDC_IPADDRESS1,
        "168",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        180, BASEHEIGHT, 35, 20,
        IDC_IPADDRESS2,
        "0",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        220, BASEHEIGHT, 35, 20,
        IDC_IPADDRESS3,
        "1",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, BASEHEIGHT+ROWSPACING, 80, 20,
        0x102,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        100, BASEHEIGHT+ROWSPACING, 35, 20,
        IDC_SUBMASK0,
        "255",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        140, BASEHEIGHT+ROWSPACING, 35, 20,
        IDC_SUBMASK1,
        "255",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        180, BASEHEIGHT+ROWSPACING, 35, 20,
        IDC_SUBMASK2,
        "255",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        220, BASEHEIGHT+ROWSPACING, 35, 20,
        IDC_SUBMASK3,
        "0",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, BASEHEIGHT+ROWSPACING*2, 80, 20,
        0x103,
        "",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        100, BASEHEIGHT+ROWSPACING*2, 35, 20,
        IDC_GREATWAY0,
        "0",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        140, BASEHEIGHT+ROWSPACING*2, 35, 20,
        IDC_GREATWAY1,
        "0",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        180, BASEHEIGHT+ROWSPACING*2, 35, 20,
        IDC_GREATWAY2,
        "0",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        220, BASEHEIGHT+ROWSPACING*2, 35, 20,
        IDC_GREATWAY3,
        "0",
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, BASEHEIGHT+ROWSPACING*3, 80, 20, 
        0x105, 
      	"",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        100, BASEHEIGHT+ROWSPACING*3, 35, 20,
        IDC_DNS0,
        "0",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        140, BASEHEIGHT+ROWSPACING*3, 35, 20,
        IDC_DNS1,
        "0",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        180, BASEHEIGHT+ROWSPACING*3, 35, 20,
        IDC_DNS2,
        "0",
        0
    },
    {
        CTRL_SLEDIT,
        WS_VISIBLE | WS_TABSTOP | ES_BASELINE | ES_AUTOSELECT,
        220, BASEHEIGHT+ROWSPACING*3, 35, 20,
        IDC_DNS3,
        "0",
        0
    },

    {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, BASEHEIGHT+ROWSPACING*4, 80, 20,
        0x104,
        "",
        0
    },
    {
        CTRL_COMBOBOX,
        WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
        100, BASEHEIGHT+ROWSPACING*4, 100, 20,
        IDC_SPEED,
        "",
        0
    },
      {
        CTRL_STATIC,
        WS_VISIBLE | SS_LEFT,
        10, BASEHEIGHT+ROWSPACING*5, 80, 20,
        0x106,
        "",
        0
    },
    {
        CTRL_COMBOBOX,
        WS_TABSTOP | WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
        100, BASEHEIGHT+ROWSPACING*5, 100, 20,
        IDC_DHCP,
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        225, BASEHEIGHT+ROWSPACING*6, 85, 20,
        IDCANCEL,
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        10, BASEHEIGHT+ROWSPACING*6, 85, 20,
        IDOK, 
        "",
        0
    }
};

#define CONTROLNRCOUNT	sizeof(Comm1Ctrl)/sizeof(Comm1Ctrl[0])
static DLGTEMPLATE Comm1DlgBox =
{
    WS_BORDER | WS_CAPTION, 
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    CONTROLNRCOUNT, NULL,
    0
};
static HWND Comm1DNSStaticWnd;
static HWND Comm1DHCPStaticWnd;
static HWND Comm1ItemWnd[20];
static int Comm1Item;
//static BITMAP comm1bk;

static void dhcpswitch(int value)
{
	int i;
	for(i=0; i<16; i++)
	{
		if(value){
			SetWindowBkColor(Comm1ItemWnd[i], PIXEL_lightgray);
		}
		else {
			SetWindowBkColor(Comm1ItemWnd[i], PIXEL_lightwhite);
		}
	}
}

static int Comm1OK(HWND hWnd,HWND Item[],int Size)
{
	char str[255];
	char c1[16] = {0}, c2[16] = {0}, c3[16] = {0}, c4[16] = {0};
	int err=0;
	int changeflag = 0;

	GetWindowText (Item[0],c1,16);
	GetWindowText (Item[1],c2,16);
	GetWindowText (Item[2],c3,16);
	GetWindowText (Item[3],c4,16);
	gOptions.DHCP = SendMessage(Item[17], CB_GETCURSEL, 0, 0);
	if(gOptions.DHCP)
	{
#ifdef FORFACTORY
		setMACAddress();
#endif
		systemEx("./udhcpc -q -n");
		UpdateNetworkInfoByDHCP("dhcp.txt");
		SetNetworkIP_MASK((BYTE *)gOptions.IPAddress, gOptions.NetMask);
		SetGateway("add", gOptions.GATEIPAddress);
		ExportProxySetting();
		//SaveStr("IPAddress",gOptions.,1);
	}
	printf("[%s]gOptions.DHCP=%d\n", __FUNCTION__, gOptions.DHCP);
	SaveInteger("DHCP", gOptions.DHCP);
	
	if(CheckIP(c1,c2,c3,c4,1))
	{
		if(strncmp(iparray[0], c1, sizeof(iparray[0])) ||strncmp(iparray[1], c2, sizeof(iparray[1])) || 
			strncmp(iparray[2], c3, sizeof(iparray[2])) || strncmp(iparray[3], c4, sizeof(iparray[3]))) {
			gOptions.IPAddress[0]=atoi(c1);
			gOptions.IPAddress[1]=atoi(c2);
			gOptions.IPAddress[2]=atoi(c3);
			gOptions.IPAddress[3]=atoi(c4);	
			//sprintf(str,"%s.%s.%s.%s",c1,c2,c3,c4);
			sprintf(str,"%d.%d.%d.%d",gOptions.IPAddress[0],gOptions.IPAddress[1],gOptions.IPAddress[2],gOptions.IPAddress[3]);
			SaveStr("IPAddress",str,1);
			changeflag = 1;
		} 
	}
	else err=1;

	GetWindowText (Item[4],c1,16);
	GetWindowText (Item[5],c2,16);
	GetWindowText (Item[6],c3,16);
	GetWindowText (Item[7],c4,16);

	if(CheckIP(c1,c2,c3,c4,0))
        {
		if(strncmp(maskarray[0], c1, sizeof(maskarray[0])) ||strncmp(maskarray[1], c2, sizeof(maskarray[1])) || 
			strncmp(maskarray[2], c3, sizeof(maskarray[2])) || strncmp(maskarray[3], c4, sizeof(maskarray[3]))) {
			gOptions.NetMask[0]=atoi(c1);
			gOptions.NetMask[1]=atoi(c2);
			gOptions.NetMask[2]=atoi(c3);
			gOptions.NetMask[3]=atoi(c4);
	               	//sprintf(str,"%s.%s.%s.%s",c1,c2,c3,c4);
	               	sprintf(str,"%d.%d.%d.%d",gOptions.NetMask[0],gOptions.NetMask[1],gOptions.NetMask[2],gOptions.NetMask[3]);
	               	SaveStr("NetMask",str,1);
			changeflag = 1;
		}
	}
	else err=2;

	GetWindowText (Item[8],c1,16);
	GetWindowText (Item[9],c2,16);
	GetWindowText (Item[10],c3,16);
	GetWindowText (Item[11],c4,16);

	if(CheckIP(c1,c2,c3,c4,0))
        {
		if(strncmp(gatearray[0], c1, sizeof(gatearray[0])) ||strncmp(gatearray[1], c2, sizeof(gatearray[1])) || 
			strncmp(gatearray[2], c3, sizeof(gatearray[2])) || strncmp(gatearray[3], c4, sizeof(gatearray[3]))) {
			gOptions.GATEIPAddress[0]=atoi(c1);
			gOptions.GATEIPAddress[1]=atoi(c2);
			gOptions.GATEIPAddress[2]=atoi(c3);
			gOptions.GATEIPAddress[3]=atoi(c4);
	               	//sprintf(str,"%s.%s.%s.%s",c1,c2,c3,c4);
	               	sprintf(str,"%d.%d.%d.%d",gOptions.GATEIPAddress[0],gOptions.GATEIPAddress[1],gOptions.GATEIPAddress[2],gOptions.GATEIPAddress[3]);
	        	SaveStr("GATEIPAddress",str,1);
			changeflag = 1;
		}
	}
	else err=3;
	
	GetWindowText (Item[12],c1,16);
	GetWindowText (Item[13],c2,16);
	GetWindowText (Item[14],c3,16);
	GetWindowText (Item[15],c4,16);

	if(gOptions.IsSupportDNS)
	{
		if(CheckIP(c1,c2,c3,c4,0))
		{
			if(strncmp(dnsarray[0], c1, sizeof(dnsarray[0])) ||strncmp(dnsarray[1], c2, sizeof(dnsarray[1])) || 
				strncmp(dnsarray[2], c3, sizeof(dnsarray[2])) || strncmp(dnsarray[3], c4, sizeof(dnsarray[3]))) {
				gOptions.DNS[0]=atoi(c1);
				gOptions.DNS[1]=atoi(c2);
				gOptions.DNS[2]=atoi(c3);
				gOptions.DNS[3]=atoi(c4);
				//sprintf(str,"%s.%s.%s.%s",c1,c2,c3,c4);
				sprintf(str,"%d.%d.%d.%d",gOptions.DNS[0],gOptions.DNS[1],gOptions.DNS[2],gOptions.DNS[3]);
				SaveStr("DNS",str,1);
				changeflag = 1;
			}
		}
		else err=4;
	}

	gOptions.HiSpeedNet = SendMessage(Item[16], CB_GETCURSEL, 0, 0);
	if (err)
	{
		switch(err)
		{
			case 1:
				MessageBox1(hWnd ,LoadStrByID(HIT_NETERROR1) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
				break;
			case 2:
				MessageBox1(hWnd ,LoadStrByID(HIT_NETERROR2) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                                break;
			case 3:
				MessageBox1(hWnd ,LoadStrByID(HIT_NETERROR3) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                                break;
			case 4://xsen test for applyfor language
				MessageBox1(hWnd ,LoadStrByID(MID_DNS_ERROR) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                                break;
			default:
				MessageBox1(hWnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
                                break;
		}
		LoadOptions(&gOptions);
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

static int CheckItem(int item)
{
        char cc[4];
        int mm,flag=0;
	memset((void*)cc, 0, sizeof(cc));
        GetWindowText(Comm1ItemWnd[item],cc,4);
        mm=atol(cc);

	if (Comm1Item==0 || Comm1Item==8 || Comm1Item==12){
		if(strlen(cc) && (mm>=0 && mm < 224)){
			flag=1;
		}
		return flag;
	}
	else
        if(Comm1Item>=1&&Comm1Item<=15)
        {
                if(strlen(cc) && (mm>=0&&mm<=255)) return 1;
                else return 0;
        }
	return 0;
}

static void comm1_notif_proc(HWND hwnd, int id, int nc, DWORD add_data)
{
    unsigned int len;
    

    if (nc == EN_CHANGE) 
    {
	if(id>=IDC_IPADDRESS0&&id<=IDC_DNS3)
	{
       		len=GetWindowTextLength(hwnd);
		if(len>=3)
		{
			if(CheckItem(Comm1Item))
			{
				if(Comm1Item>=0&&Comm1Item<=3)
                                	++Comm1Item;
                        	else
                        	if(Comm1Item>=4&&Comm1Item<=7)
                                	++Comm1Item;
                        	else
                        	if(Comm1Item>=8&&Comm1Item<=11)
                                	++Comm1Item;
                        	else
                        	if(Comm1Item>12&&Comm1Item<=15)
                                	++Comm1Item;
				SetFocus(Comm1ItemWnd[Comm1Item]);
			}
		}
	}
    }
}

static int Comm1DialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char c1[16],c2[16],c3[16],c4[16],buffer[256];
	int posX=0, tmpvalue = 0;
	static char keyupFlag=0;
	switch (message) {
	case MSG_INITDIALOG:

	gCommFag=0;

	//if (LoadBitmap(HDC_SCREEN,&comm1bk,GetBmpPath("submenubg.jpg")))
        //                return 0;
        SendDlgItemMessage(hDlg, IDC_DHCP,CB_ADDSTRING ,0,(LPARAM)SwitchStr[0]);
	SendDlgItemMessage(hDlg, IDC_DHCP,CB_ADDSTRING ,0,(LPARAM)SwitchStr[1]);

	SendDlgItemMessage(hDlg, IDC_SPEED,CB_ADDSTRING ,0,(LPARAM)SpeedStr[2]);
	SendDlgItemMessage(hDlg, IDC_SPEED,CB_ADDSTRING ,0,(LPARAM)SpeedStr[1]);
	SendDlgItemMessage(hDlg, IDC_SPEED,CB_ADDSTRING ,0,(LPARAM)SpeedStr[0]);

	SetWindowText(GetDlgItem(hDlg,0x101),LoadStrByID(MID_LOCAL_IP));
	SetWindowText(GetDlgItem(hDlg,0x102),LoadStrByID(HIT_COMM1NET2));
	SetWindowText(GetDlgItem(hDlg,0x103),LoadStrByID(HIT_COMM1NET3));
	SetWindowText(GetDlgItem(hDlg,0x104),LoadStrByID(HIT_COMM1NET4));
	SetWindowText(GetDlgItem(hDlg,0x105),LoadStrByID(MID_DNS));//xsen test for applyfor language
	
	SetWindowText(GetDlgItem(hDlg,0x106),"DHCP");
	SetWindowText(GetDlgItem(hDlg,IDCANCEL),LoadStrByID(HIT_CANCEL));
	SetWindowText(GetDlgItem(hDlg,IDOK),LoadStrByID(HIT_OK));


        Comm1ItemWnd[0] = GetDlgItem (hDlg, IDC_IPADDRESS0);
	Comm1ItemWnd[1] = GetDlgItem (hDlg, IDC_IPADDRESS1);
	Comm1ItemWnd[2] = GetDlgItem (hDlg, IDC_IPADDRESS2);
	Comm1ItemWnd[3] = GetDlgItem (hDlg, IDC_IPADDRESS3);


	Comm1ItemWnd[4] = GetDlgItem (hDlg, IDC_SUBMASK0);
	Comm1ItemWnd[5] = GetDlgItem (hDlg, IDC_SUBMASK1);
	Comm1ItemWnd[6] = GetDlgItem (hDlg, IDC_SUBMASK2);
	Comm1ItemWnd[7] = GetDlgItem (hDlg, IDC_SUBMASK3);

	Comm1ItemWnd[8] = GetDlgItem (hDlg, IDC_GREATWAY0);
	Comm1ItemWnd[9] = GetDlgItem (hDlg, IDC_GREATWAY1);
	Comm1ItemWnd[10] = GetDlgItem (hDlg, IDC_GREATWAY2);
	Comm1ItemWnd[11] = GetDlgItem (hDlg, IDC_GREATWAY3);

	Comm1ItemWnd[12] = GetDlgItem (hDlg, IDC_DNS0);
	Comm1ItemWnd[13] = GetDlgItem (hDlg, IDC_DNS1);
	Comm1ItemWnd[14] = GetDlgItem (hDlg, IDC_DNS2);
	Comm1ItemWnd[15] = GetDlgItem (hDlg, IDC_DNS3);
	Comm1DNSStaticWnd= GetDlgItem (hDlg, 0x105);

	Comm1ItemWnd[16] = GetDlgItem (hDlg, IDC_SPEED);
	Comm1DHCPStaticWnd=GetDlgItem (hDlg, 0x106);
	Comm1ItemWnd[17] = GetDlgItem (hDlg, IDC_DHCP);
	
	Comm1ItemWnd[18] = GetDlgItem (hDlg, IDOK);
	Comm1ItemWnd[19] = GetDlgItem (hDlg, IDCANCEL);

	if(!gOptions.IsSupportDNS)
	{
		ShowWindow(Comm1DNSStaticWnd,SW_HIDE);
		ShowWindow(Comm1ItemWnd[12],SW_HIDE);
		ShowWindow(Comm1ItemWnd[13],SW_HIDE);
		ShowWindow(Comm1ItemWnd[14],SW_HIDE);
		ShowWindow(Comm1ItemWnd[15],SW_HIDE);
	}
	SetFocus(Comm1ItemWnd[0]);
	Comm1Item=0;

	LoadStr("IPAddress",buffer);
        if(ParseIP(buffer,c1,c2,c3,c4))
	{
		memset(iparray, 0, sizeof(iparray));
		strncpy(iparray[0], c1, sizeof(iparray[0]));
		strncpy(iparray[1], c2, sizeof(iparray[1]));
		strncpy(iparray[2], c3, sizeof(iparray[2]));
		strncpy(iparray[3], c4, sizeof(iparray[3]));
		SetWindowText(Comm1ItemWnd[0],c1);
		SetWindowText(Comm1ItemWnd[1],c2);
		SetWindowText(Comm1ItemWnd[2],c3);
		SetWindowText(Comm1ItemWnd[3],c4);
	}
	
	LoadStr("NetMask",buffer);
        if(ParseIP(buffer,c1,c2,c3,c4))
        {
		memset(maskarray, 0, sizeof(maskarray));
		strncpy(maskarray[0], c1, sizeof(maskarray[0]));
		strncpy(maskarray[1], c2, sizeof(maskarray[1]));
		strncpy(maskarray[2], c3, sizeof(maskarray[2]));
		strncpy(maskarray[3], c4, sizeof(maskarray[3]));
                SetWindowText(Comm1ItemWnd[4],c1);
                SetWindowText(Comm1ItemWnd[5],c2);
                SetWindowText(Comm1ItemWnd[6],c3);
                SetWindowText(Comm1ItemWnd[7],c4);
        }

	LoadStr("GATEIPAddress",buffer);
        if(ParseIP(buffer,c1,c2,c3,c4))
        {
		memset(gatearray, 0, sizeof(gatearray));
		strncpy(gatearray[0], c1, sizeof(gatearray[0]));
		strncpy(gatearray[1], c2, sizeof(gatearray[1]));
		strncpy(gatearray[2], c3, sizeof(gatearray[2]));
		strncpy(gatearray[3], c4, sizeof(gatearray[3]));
                SetWindowText(Comm1ItemWnd[8],c1);
                SetWindowText(Comm1ItemWnd[9],c2);
                SetWindowText(Comm1ItemWnd[10],c3);
                SetWindowText(Comm1ItemWnd[11],c4);
        }

	if(gOptions.IsSupportDNS)
	{
		LoadStr("DNS",buffer);
		if(ParseIP(buffer,c1,c2,c3,c4))
		{
			memset(dnsarray, 0, sizeof(dnsarray));
			strncpy(dnsarray[0], c1, sizeof(dnsarray[0]));
			strncpy(dnsarray[1], c2, sizeof(dnsarray[1]));
			strncpy(dnsarray[2], c3, sizeof(dnsarray[2]));
			strncpy(dnsarray[3], c4, sizeof(dnsarray[3]));
			SetWindowText(Comm1ItemWnd[12],c1);
			SetWindowText(Comm1ItemWnd[13],c2);
			SetWindowText(Comm1ItemWnd[14],c3);
			SetWindowText(Comm1ItemWnd[15],c4);
		}
	}

	SendMessage(Comm1ItemWnd[16], CB_SETCURSEL, gOptions.HiSpeedNet, 0);
	SendMessage(Comm1ItemWnd[17], CB_SETCURSEL, gOptions.DHCP, 0);
	
	SendMessage(Comm1ItemWnd[0],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[1],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[2],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[3],EM_LIMITTEXT,3,0);

	SendMessage(Comm1ItemWnd[4],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[5],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[6],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[7],EM_LIMITTEXT,3,0);

	SendMessage(Comm1ItemWnd[8],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[9],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[10],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[11],EM_LIMITTEXT,3,0);

	SendMessage(Comm1ItemWnd[12],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[13],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[14],EM_LIMITTEXT,3,0);
	SendMessage(Comm1ItemWnd[15],EM_LIMITTEXT,3,0);

	SetNotificationCallback(Comm1ItemWnd[0],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[1],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[2],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[3],comm1_notif_proc);

	SetNotificationCallback(Comm1ItemWnd[4],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[5],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[6],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[7],comm1_notif_proc);

	SetNotificationCallback(Comm1ItemWnd[8],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[9],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[10],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[11],comm1_notif_proc);

	SetNotificationCallback(Comm1ItemWnd[12],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[13],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[14],comm1_notif_proc);
	SetNotificationCallback(Comm1ItemWnd[15],comm1_notif_proc);

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
			tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
        		SetTextColor(hdc,PIXEL_lightwhite);
			if (fromRight==1)
				posX=46;
			TextOut(hdc,133-posX+gOptions.GridWidth*2,20,".");	
			TextOut(hdc,173-posX+gOptions.GridWidth*2,20,".");
			TextOut(hdc,213-posX+gOptions.GridWidth*2,20,".");
			TextOut(hdc,133-posX+gOptions.GridWidth*2,55,".");
                        TextOut(hdc,173-posX+gOptions.GridWidth*2,55,".");
                        TextOut(hdc,213-posX+gOptions.GridWidth*2,55,".");
			TextOut(hdc,133-posX+gOptions.GridWidth*2,90,".");
                        TextOut(hdc,173-posX+gOptions.GridWidth*2,90,".");
                        TextOut(hdc,213-posX+gOptions.GridWidth*2,90,".");
			if(gOptions.IsSupportDNS)
			{
				TextOut(hdc,133-posX+gOptions.GridWidth*2,125,".");
				TextOut(hdc,173-posX+gOptions.GridWidth*2,125,".");
				TextOut(hdc,213-posX+gOptions.GridWidth*2,125,".");
			}
                        EndPaint(hDlg,hdc);
			break;
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
                                if(Comm1Item>=0&&Comm1Item<=3)
                                        Comm1Item=4;
                                else
				if(Comm1Item>=4&&Comm1Item<=7)
                                        Comm1Item=8;
				else
				if(Comm1Item>=8&&Comm1Item<=11)
				{
					if(gOptions.IsSupportDNS)
						Comm1Item=12;
					else
						Comm1Item=16;
				}
				else
				if(Comm1Item>=12&&Comm1Item<=15)
					Comm1Item=16;
				else
				if(Comm1Item==16)
					Comm1Item=17;
				else
                                if(Comm1Item==17)
                                        Comm1Item=18;
				else
                                if(Comm1Item==18)
                                        Comm1Item=19;
				else
				if(Comm1Item==19)
					Comm1Item=0;
                                SetFocus(Comm1ItemWnd[Comm1Item]);
				return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
                        {
				if(Comm1Item>=0&&Comm1Item<=3)
                                        Comm1Item=19;
                                else
                                if(Comm1Item>=4&&Comm1Item<=7)
                                        Comm1Item=0;
                                else
                                if(Comm1Item>=8&&Comm1Item<=11)
                                        Comm1Item=4;
                                else
                                if(Comm1Item>=12&&Comm1Item<=15)
                                        Comm1Item=8;
                                else
				if(Comm1Item==16)
				{
					if(gOptions.IsSupportDNS)
						Comm1Item=12;
					else
						Comm1Item=8;
				}
				else
				if(Comm1Item==17)
                                        Comm1Item=16;
				else
				if(Comm1Item==18)
                                        Comm1Item=17;
				else
				if(Comm1Item==19)
					Comm1Item=18;
                                SetFocus(Comm1ItemWnd[Comm1Item]);
				return 1;
                        }
			else
			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
                        {
				if(Comm1Item>=0&&Comm1Item<=15)
				{
					if(!CheckItem(Comm1Item))
						return 1;	
				}
				if(Comm1Item==16)
                                {
					int g_sel=SendMessage(Comm1ItemWnd[16],CB_GETCURSEL,0,0);
					if(++g_sel>2) g_sel=0;
					SendMessage(Comm1ItemWnd[16],CB_SETCURSEL,g_sel,0);
#if 0
					if(SendMessage(Comm1ItemWnd[12],CB_GETCURSEL,0,0)==2)
						SendMessage(Comm1ItemWnd[12],CB_SETCURSEL,0,0);
					else
	                                        SendDlgItemMessage(hDlg,IDC_SPEED,CB_SPIN,1,0);
#endif
                                        return 1;
                                }
				if(Comm1Item == 17)
				{
					int g_dhcp=SendMessage(Comm1ItemWnd[17],CB_GETCURSEL,0,0);
					if(--g_dhcp<0) g_dhcp=1;
					SendMessage(Comm1ItemWnd[17],CB_SETCURSEL,g_dhcp,0);
					return 1;
				}
                                else
                                if(Comm1Item<15)
				{
					if(!gOptions.IsSupportDNS && Comm1Item>=11)
						Comm1Item=0;
					else
						Comm1Item++;
				}
                                else
                                        Comm1Item=0;
                                SetFocus(Comm1ItemWnd[Comm1Item]);
                                return 1;
                        }
                        else
                        if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
                        {
				if(Comm1Item>=0&&Comm1Item<=15)
                                {
                                        if(!CheckItem(Comm1Item))
                                                return 1;
                                }
				if(Comm1Item==16)
				{
					int g_sel=SendMessage(Comm1ItemWnd[16],CB_GETCURSEL,0,0);
					if(--g_sel<0) g_sel=2;
					SendMessage(Comm1ItemWnd[16],CB_SETCURSEL,g_sel,0);
#if 0
					if(SendMessage(Comm1ItemWnd[12],CB_GETCURSEL,0,0)==0)
                                                SendMessage(Comm1ItemWnd[12],CB_SETCURSEL,2,0);
                                        else
						SendDlgItemMessage(hDlg,IDC_SPEED,CB_SPIN,0,0);	
#endif
					return 1;
				}
				if(Comm1Item == 17)
				{
					int g_dhcp=SendMessage(Comm1ItemWnd[17],CB_GETCURSEL,0,0);
					if(--g_dhcp<0) g_dhcp=1;
					SendMessage(Comm1ItemWnd[17],CB_SETCURSEL,g_dhcp,0);
					return 1;
				}
					
				else
                                if(Comm1Item>0)
                                {
                                        Comm1Item--;
                                }
                                else
				{
					if(!gOptions.IsSupportDNS)
						Comm1Item=11;
					else
						Comm1Item=15;
				}
                                SetFocus(Comm1ItemWnd[Comm1Item]);
                                return 1;
                        }
			else
			if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_MENU)||(LOWORD(wParam)==SCANCODE_F10))
			{
//				if(gCommFag==0)
//					SendMessage(hDlg,MSG_CLOSE,0,0);
//				else
				if(Comm1Item <= 18)
				{
					if(Comm1OK(hDlg,Comm1ItemWnd,18))
						SendMessage(hDlg,MSG_CLOSE,0,0);	
				}
				else
				if(Comm1Item == 19)
					SendMessage(hDlg,MSG_CLOSE,0,0);
				return 1;
			}
			else
                        if ((LOWORD(wParam)==SCANCODE_ESCAPE))
                        {
                                SendMessage(hDlg,MSG_CLOSE,0,0);
				return 1;
                        }
			else
				gCommFag=1;
			break;
	case MSG_CLOSE:
		//UnloadBitmap(&comm1bk);
		EndDialog (hDlg, IDCANCEL);
		break;
        
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_COMM1(HWND hWnd)
{
	int posX1,posX2,posX3,posX4;
	Comm1DlgBox.w=gOptions.LCDWidth;
	Comm1DlgBox.h=gOptions.LCDHeight;

	if (fromRight==1)	//add by jazzy 2008.07.14
	{
		posX1=230+gOptions.GridWidth*2;
		posX2=50+gOptions.GridWidth*2;
		posX3=225+gOptions.ControlOffset;
		posX4=10;
	}
	else
	{
		posX1=10;
		posX2=100+gOptions.GridWidth*2;
		posX3=10;
		posX4=225+gOptions.ControlOffset;
	}

	Comm1Ctrl[0].x=posX1;
	Comm1Ctrl[1].x=posX2;
	Comm1Ctrl[2].x=posX2+40;
	Comm1Ctrl[3].x=posX2+80;
	Comm1Ctrl[4].x=posX2+120;
	Comm1Ctrl[5].x=posX1;
	Comm1Ctrl[6].x=posX2;
	Comm1Ctrl[7].x=posX2+40;
	Comm1Ctrl[8].x=posX2+80;
	Comm1Ctrl[9].x=posX2+120;
	Comm1Ctrl[10].x=posX1;
	Comm1Ctrl[11].x=posX2;
	Comm1Ctrl[12].x=posX2+40;
	Comm1Ctrl[13].x=posX2+80;
	Comm1Ctrl[14].x=posX2+120;
	Comm1Ctrl[15].x=posX1;
	Comm1Ctrl[16].x=posX2;
	Comm1Ctrl[17].x=posX2+40;
	Comm1Ctrl[18].x=posX2+80;
	Comm1Ctrl[19].x=posX2+120;

	Comm1Ctrl[20].x=posX1;
	Comm1Ctrl[21].x=posX2+40;
	
	Comm1Ctrl[22].x=posX1;
	Comm1Ctrl[23].x=posX2+40;


	Comm1Ctrl[24].x=posX4;
	Comm1Ctrl[25].x=posX3;

	if(!gOptions.IsSupportDNS)
	{
		Comm1Ctrl[20].y=BASEHEIGHT+ROWSPACING*3;
		Comm1Ctrl[21].y=BASEHEIGHT+ROWSPACING*3;
	}
	Comm1DlgBox.caption=LoadStrByID(HIT_COMM1);
	Comm1DlgBox.controls = Comm1Ctrl;

	DialogBoxIndirectParam (&Comm1DlgBox, hWnd, Comm1DialogBoxProc, 0L);
}


#endif
