#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <minigui/tftmullan.h>

#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"
#include "exfun.h"
#include "t9.h"

#define WIFI_STATIC	1000
#define WIFI_SSID	1010
#define WIFI_NETMODE	1011
#define WIFI_AUTH	1012
#define WIFI_ENCRYP	1013
#define WIFI_PWD	1014
#define WIFI_IPMODE	1015
#define WIFI_DHCPIP	1016
#define WIFI_SAVE	1040
#define WIFI_EXIT	1041

HWND dhcpip;

HWND WifiWnd[9];
int curFocusWnd;
//BITMAP wifibkg;
char* WifiStr[23];
int pwdtype1=0;				//密码类型
int AuthCount=0;
int pwdflag=1, ipflag=1;		//密码和IP设置可操作标志

char oldip[3][18];
char newip[3][18];			//保存IP设置
char oldpwdid[2];
char  newpwdid[2];

WifiPWD oldpwd[4];
WifiPWD newpwd[4];

char oldwpapsk[255];
char newwpapsk[255];
extern char DHCPIPAdress[18];

int loadflag=1;
int netmode, authmode, encrytype;
char myssid[256];
char *modetype[14]={
	"Infra",
	"Adhoc",
	"OPEN",
	"SHARED",
	"WEPAUTO",
	"WPAPSK",
	"WPA2PSK",
	"WPANONE",
	"NONE",
	"WEP",
	"TKIP",
	"AES",
	"WEP",
	"WPA"
};
int OptMod_wifi;

static void SetAuthMode(int NetWorkMode)
{
	SendMessage(WifiWnd[2], CB_RESETCONTENT, 0, 0);
	SendMessage(WifiWnd[2], CB_ADDSTRING, 0, (LPARAM)WifiStr[5]);
	SendMessage(WifiWnd[2], CB_ADDSTRING, 0, (LPARAM)WifiStr[6]);
	SendMessage(WifiWnd[2], CB_ADDSTRING, 0, (LPARAM)WifiStr[7]);
	switch(NetWorkMode)
	{
		case 0:		//Infra
			SendMessage(WifiWnd[2], CB_ADDSTRING, 0, (LPARAM)WifiStr[8]);
			SendMessage(WifiWnd[2], CB_ADDSTRING, 0, (LPARAM)WifiStr[9]);
			AuthCount=5;
			//AuthCount=4;
			break;
		case 1:		//Adhoc
			SendMessage(WifiWnd[2], CB_ADDSTRING, 0, (LPARAM)WifiStr[10]);
			AuthCount=4;
			break;
	}
}

static void SetEncrypMode(char* AuthMode)
{
	char tmpauth[10];
	sprintf(tmpauth, "%s", AuthMode);
	SendMessage(WifiWnd[3], CB_RESETCONTENT, 0, 0);

	if((strncmp(tmpauth, "OPEN", 4)==0) || (strncmp(tmpauth, "SHARED", 6)==0) || (strncmp(tmpauth, "WEPAUTO", 7)==0))
	{
		pwdtype1=0;	//WEP
		if(strncmp(tmpauth, "SHARED", 6)==0)
		{
			SendMessage(WifiWnd[3], CB_ADDSTRING, 0, (LPARAM)WifiStr[13]);
		}
		else
		{
			SendMessage(WifiWnd[3], CB_ADDSTRING, 0, (LPARAM)WifiStr[12]);
			SendMessage(WifiWnd[3], CB_ADDSTRING, 0, (LPARAM)WifiStr[13]);
		}
	}
	else if((strncmp(tmpauth, "WPAPSK", 6)==0) || (strncmp(tmpauth, "WPA2PSK", 7)==0) || (strncmp(tmpauth, "WPANONE", 7)==0))
	{
		pwdtype1=1;	//WPA
		SendMessage(WifiWnd[3], CB_ADDSTRING, 0, (LPARAM)WifiStr[14]);
		SendMessage(WifiWnd[3], CB_ADDSTRING, 0, (LPARAM)WifiStr[15]);
	}
}

static void LoadWindowStr(void)
{
	WifiStr[0]=LoadStrByID(MID_WIFI_ID);            //网络识别ID
	WifiStr[1]=LoadStrByID(MID_WIFI_MODE);          //网络模式
	WifiStr[2]=LoadStrByID(MID_WIFI_INFRA);         //Infra
	WifiStr[3]=LoadStrByID(MID_WIFI_ADHOC);         //Adhoc
	WifiStr[4]=LoadStrByID(MID_WIFI_AUTH);          //认证类型
	WifiStr[5]=LoadStrByID(MID_WIFI_OPEN);          //OPEN
	WifiStr[6]=LoadStrByID(MID_WIFI_SHARED);        //SHARED
	WifiStr[7]=LoadStrByID(MID_WIFI_WEPAUTO);       //WEPAUTO
	WifiStr[8]=LoadStrByID(MID_WIFI_WPAPSK);        //WPAPSK
	WifiStr[9]=LoadStrByID(MID_WIFI_WPA2PSK);        //WPA2PSK
	WifiStr[10]=LoadStrByID(MID_WIFI_WPANONE);      //WPANONE
	WifiStr[11]=LoadStrByID(MID_WIFI_ENCRYP);       //加密方式
	WifiStr[12]=LoadStrByID(MID_WIFI_NONE);         //NONE
	WifiStr[13]=LoadStrByID(MID_WIFI_WEP);          //WEP
	WifiStr[14]=LoadStrByID(MID_WIFI_TKIP);         //TKIP
	WifiStr[15]=LoadStrByID(MID_WIFI_AES);          //AES
	WifiStr[16]=LoadStrByID(MID_WIFI_PWDSET);       //设置密码
	WifiStr[17]=LoadStrByID(MID_LOCAL_IP);                   //本机IP地址
	WifiStr[18]=LoadStrByID(MID_WIFI_MANUIP);       //手动分配                      
	WifiStr[19]=LoadStrByID(MID_NET_DHCP);          //DHCP
	if(OptMod_wifi==1)
		WifiStr[20]=LoadStrByID(MID_NET_IP);
	else
		WifiStr[20]=LoadStrByID(MID_WIFI_IP);		//指定IP
	WifiStr[21]=LoadStrByID(MID_SAVE);              //保存
	WifiStr[22]=LoadStrByID(MID_EXIT);              //退出
}

static void changeNetMode(HWND hWnd, int id, int nc, DWORD add_data)
{
	if(nc==CBN_EDITCHANGE)
	{
		int tt=SendMessage(WifiWnd[1], CB_GETCURSEL, 0, 0);
		SetAuthMode(tt);
		if(loadflag)
			SendMessage(WifiWnd[2], CB_SETCURSEL, authmode, 0);
		else
			SendMessage(WifiWnd[2], CB_SETCURSEL, 0, 0);
	}
}

static void changeAuthType(HWND hWnd, int id, int nc, DWORD add_data)
{
	unsigned char idx;
	if(nc==CBN_EDITCHANGE)
	{
		char authchar[10];
		//GetWindowText(WifiWnd[2], authchar, 10);
		idx=SendMessage(WifiWnd[2],CB_GETCURSEL,0,0);
		sprintf(authchar,"%s",modetype[2+idx]);
		SetEncrypMode(authchar);
		if(loadflag)
		{
			SendMessage(WifiWnd[3], CB_SETCURSEL, encrytype, 0);
		}
		else
		{
			if(strncmp(authchar, "OPEN", 4)!=0)
				SendMessage(WifiWnd[3], CB_SETCURSEL, 1, 0);
			else
				SendMessage(WifiWnd[3], CB_SETCURSEL, 0, 0);
		}
	}
}

static void changeEncryp(HWND hWnd, int id, int nc, DWORD add_data)
{
	if(nc==CBN_EDITCHANGE)
	{
		char pwdtype[10];
		int idx=0;
		idx=SendMessage(WifiWnd[3],CB_GETCURSEL,0,0);

		char buf[20]={0};
		memset(buf, 0, sizeof(buf));
		int index=SendMessage(WifiWnd[2],CB_GETCURSEL,0,0);
		sprintf(buf,"%s",modetype[2+index]);
		//if((strncmp(buf, "WPAPSK", 6) == 0) || (strncmp(buf, "SHARED", 6) == 0))
		if((strncmp(buf, "WPAPSK", 6) == 0) || (strncmp(buf, "WPA2PSK", 7) == 0) || (strncmp(buf, "SHARED", 6) == 0))
		{
			pwdflag=1;
			SendMessage(WifiWnd[4], MSG_ENABLE, pwdflag, 0);
			return;
		}

		sprintf(pwdtype,"%s",modetype[8+idx]);
		//GetWindowText(WifiWnd[3], pwdtype, 10);
		if(strncmp(pwdtype, "NONE", 4)==0)
			pwdflag=0;
		else
			pwdflag=1;

		SendMessage(WifiWnd[4], MSG_ENABLE, pwdflag, 0);
	}
}

static void changeIPMode(HWND hWnd, int id, int nc, DWORD add_data)
{
	if(nc==CBN_EDITCHANGE)
	{
		if(SendMessage(WifiWnd[5], CB_GETCURSEL, 0, 0)==0)
		{
			ipflag=1;
			ShowWindow(dhcpip, SW_HIDE);
		}
		else
		{
			ipflag=0;
			ShowWindow(dhcpip, SW_SHOW);
		}
		SendMessage(WifiWnd[6], MSG_ENABLE, ipflag, 0);
	}
}

static void InitWindow (HWND hWnd)
{
	int posX1,posX2,posX3,posX4,posX5;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=220+gOptions.GridWidth*2;
		posX2=220+gOptions.GridWidth*2;
		posX3=110+gOptions.GridWidth*2;
		posX4=10+gOptions.GridWidth*2;
		posX5=10;
	}
	else
	{
		posX1=10;
		posX2=20;
		posX3=101;
		posX4=220;
		posX5=110;
	}
	//网络识别ID	
	CreateWindow(CTRL_STATIC,WifiStr[0],WS_VISIBLE|SS_LEFT,WIFI_STATIC,posX1,10,100,23,hWnd,0);
	//网络模式
	CreateWindow(CTRL_STATIC,WifiStr[1],WS_VISIBLE|SS_LEFT,WIFI_STATIC+1,posX1,40,100,23,hWnd,0);
	//认证类型	
	CreateWindow(CTRL_STATIC,WifiStr[4],WS_VISIBLE|SS_LEFT,WIFI_STATIC+2,posX1,70,100,23,hWnd,0);
	//加密方式
	CreateWindow(CTRL_STATIC,WifiStr[11],WS_VISIBLE|SS_LEFT,WIFI_STATIC+3,posX1,100,100,23,hWnd,0);
	//网络设置
	CreateWindow(CTRL_STATIC,WifiStr[17],WS_VISIBLE|SS_LEFT,WIFI_STATIC+4,posX1,130,100,23,hWnd,0);
	dhcpip=CreateWindow(CTRL_STATIC, "", SS_LEFT, WIFI_DHCPIP, posX2, 156, 200, 23, hWnd, 0);
	WifiWnd[6]=CreateWindow(CTRL_BUTTON,WifiStr[20],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,WIFI_IPMODE,posX4,126,85,23,hWnd,0);	

	if(OptMod_wifi==1)
	{
		WifiWnd[0]= CreateWindow(CTRL_STATIC, "",WS_VISIBLE |  WS_BORDER | WS_THICKFRAME | SS_LEFT,WIFI_SSID, posX5,6,200,23,hWnd,0);
		WifiWnd[1]= CreateWindow(CTRL_STATIC, "",WS_VISIBLE |  WS_BORDER | WS_THICKFRAME | SS_LEFT,WIFI_NETMODE,posX3,36,100,23,hWnd,0);	
		WifiWnd[2]= CreateWindow(CTRL_STATIC, "",WS_VISIBLE |  WS_BORDER | WS_THICKFRAME | SS_LEFT,WIFI_AUTH,posX3,66,100,23,hWnd,0);
		WifiWnd[3]= CreateWindow(CTRL_STATIC, "",WS_VISIBLE |  WS_BORDER | WS_THICKFRAME | SS_LEFT,WIFI_ENCRYP,posX3,96,100,23,hWnd,0);
		WifiWnd[5]= CreateWindow(CTRL_STATIC, "",WS_VISIBLE |  WS_BORDER | WS_THICKFRAME | SS_LEFT,WIFI_IPMODE,posX3,126,100,23,hWnd,0);
	}
	else
	{
		WifiWnd[0]=CreateWindow(CTRL_SLEDIT,"",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,WIFI_SSID,posX5,6,200,23,hWnd,0);
		WifiWnd[1]=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS|CBS_NOTIFY,
				WIFI_NETMODE,posX3,36,100,23,hWnd,0);
		SendMessage(WifiWnd[1], CB_ADDSTRING, 0, (LPARAM)WifiStr[2]);
		SendMessage(WifiWnd[1], CB_ADDSTRING, 0, (LPARAM)WifiStr[3]);	

		WifiWnd[2]=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS|CBS_NOTIFY,
				WIFI_AUTH,posX3,66,100,23,hWnd,0);
		WifiWnd[3]=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS|CBS_NOTIFY,
				WIFI_ENCRYP,posX3,96,100,23,hWnd,0);
		WifiWnd[4]=CreateWindow(CTRL_BUTTON,WifiStr[16],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,WIFI_PWD, posX4,96,85,23,hWnd,0);

		WifiWnd[5]=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS|CBS_NOTIFY,
				WIFI_IPMODE,posX3,126,100,23,hWnd,0);
		SendMessage(WifiWnd[5], CB_ADDSTRING, 0, (LPARAM)WifiStr[18]);
		SendMessage(WifiWnd[5], CB_ADDSTRING, 0, (LPARAM)WifiStr[19]);

		SetNotificationCallback(WifiWnd[1], changeNetMode);
		SetNotificationCallback(WifiWnd[2], changeAuthType);
		SetNotificationCallback(WifiWnd[3], changeEncryp);
		SetNotificationCallback(WifiWnd[5], changeIPMode);

		WifiWnd[7]=CreateWindow(CTRL_BUTTON,WifiStr[21],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,WIFI_SAVE,10,190,85,23,hWnd,0);
		WifiWnd[8]=CreateWindow(CTRL_BUTTON,WifiStr[22],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,WIFI_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);	
	}		
}

static int GetAuthIndex(char* authname)
{
	char tmpauth[10];
	sprintf(tmpauth, "%s", authname);
	if(strncmp(tmpauth, "OPEN", 4)==0)
		return 0;
	else if(strncmp(tmpauth, "SHARED", 6)==0)
		return 1;
	else if(strncmp(tmpauth, "WEPAUTO", 7)==0)
		return 2;
	else if((strncmp(tmpauth, "WPAPSK", 6)==0) || (strncmp(tmpauth, "WPANONE", 7)==0))
		return 3;
	//jell
	else if(strncmp(tmpauth, "WPA2PSK", 7)==0)
		return 4;

	return 0;	
}

static int GetEncrypIndex(char* encrypname)
{
	char tmpencryp[10];
	sprintf(tmpencryp, "%s", encrypname);

	//if((strncmp(tmpencryp, "NONE", 4)==0) || (strncmp(tmpencryp, "TKIP", 4)==0))
	if((strncmp(tmpencryp, "NONE", 4)==0) || (strncmp(tmpencryp, "TKIP", 4)==0))
	{
		return 0;
	}
	else if((strncmp(tmpencryp, "WEP", 3)==0) || (strncmp(tmpencryp, "AES", 3)==0) )
	{
		return 1;
	}

	return 0;
}

static void InitParameters(void)
{
	char cfg[256];

	if (myssid[0])
	{
		SetWindowText(WifiWnd[0], myssid);
	}
	else
	{
		memset(cfg, 0x00, sizeof(cfg));
		if (LoadStr("SSID", cfg))
			SetWindowText(WifiWnd[0], cfg);
	}

	if (OptMod_wifi==1)
	{
		memset(cfg, 0x00, sizeof(cfg));
		if (LoadStr("NetworkType", cfg))
			SetWindowText(WifiWnd[1], cfg);

		memset(cfg, 0x00, sizeof(cfg));
		if (LoadStr("AuthMode", cfg))
			SetWindowText(WifiWnd[2], cfg);

		memset(cfg, 0x00, sizeof(cfg));
		if (LoadStr("EncrypType", cfg))
			SetWindowText(WifiWnd[3], cfg);
		if(gOptions.wifidhcpfunon)
			SetWindowText(WifiWnd[5], "DHCP");
		else
			SetWindowText(WifiWnd[5], LoadStrByID(MID_WIFI_MANUIP));

		SendMessage(WifiWnd[6], MSG_ENABLE, (!gOptions.wifidhcpfunon), 0);
	}
	else
	{
		memset(cfg, 0x00, sizeof(cfg));
		netmode = 0;
		if (LoadStr("NetworkType", cfg))
		{
			if(strncmp(cfg, "Infra", 5)==0)
				netmode=0;
			else if(strncmp(cfg, "Adhoc", 5)==0)
				netmode=1;
		}
		SendMessage(WifiWnd[1], CB_SETCURSEL, netmode, 0);

		memset(cfg, 0x00, sizeof(cfg));
		LoadStr("AuthMode",cfg);
		authmode=GetAuthIndex(cfg);

		memset(cfg, 0x00, sizeof(cfg));
		LoadStr("EncrypType",cfg);
		encrytype=GetEncrypIndex(cfg);

		SendMessage(WifiWnd[5], CB_SETCURSEL, gOptions.wifidhcpfunon, 0);
	}
	if(gOptions.wifidhcpfunon)
	{
		if(DHCPIPAdress[0]!='\0')
			SetWindowText(dhcpip, DHCPIPAdress);
		else
			SetWindowText(dhcpip, LoadStrByID(MID_WIFI_DHCPERR));

		ShowWindow(dhcpip, SW_SHOW);
	}

	LoadStr("wifiip", (char*)oldip[0]);
	LoadStr("wifimask", (char*)oldip[1]);
	LoadStr("wifigateway", (char*)oldip[2]);
	LoadStr("DefaultKeyID", (char*)oldpwdid);			//seletct which  pass str
	LoadStr("Key1Type", (char*)oldpwd[0].pwdtype);
	LoadStr("Key1Str", (char*)oldpwd[0].pwdchar);
	LoadStr("Key2Type", (char*)oldpwd[1].pwdtype); 
	LoadStr("Key2Str", (char*)oldpwd[1].pwdchar);
	LoadStr("Key3Type", (char*)oldpwd[2].pwdtype);
	LoadStr("Key3Str", (char*)oldpwd[2].pwdchar);
	LoadStr("Key4Type", (char*)oldpwd[3].pwdtype);
	LoadStr("Key4Str", (char*)oldpwd[3].pwdchar);
	LoadStr("WPAPSK", oldwpapsk);
	memcpy(newip, oldip, 54);
	memcpy(newpwd, oldpwd, 4*sizeof(WifiPWD));
	memcpy(newpwdid, oldpwdid, 2);
	memcpy(newwpapsk, oldwpapsk, 255);
}


static int isModified(void)
{
	char cfg[256];
	char tmpstring[256];
	int idx=0;

	memset(cfg, 0x00, sizeof(cfg));
	memset(tmpstring, 0, sizeof(tmpstring));
	LoadStr("SSID", cfg);
	GetWindowText(WifiWnd[0], tmpstring, 256);
	if(strncmp(cfg, tmpstring, 256)!=0)
		return 1;

	memset(cfg, 0x00, sizeof(cfg));
	memset(tmpstring, 0, sizeof(tmpstring));
	//LoadStr("NetworkType", cfg);
	if(!LoadStr("NetworkType", cfg))
	{
		if(OptMod_wifi==0)
			sprintf(cfg,"Infra");
	}
	//GetWindowText(WifiWnd[1], tmpstring, 256);
	idx=SendMessage(WifiWnd[1],CB_GETCURSEL,0,0);
	sprintf(tmpstring,"%s",modetype[idx]);
	if(strncmp(cfg, tmpstring, 256)!=0)
		return 1;

	memset(cfg, 0x00, sizeof(cfg));
	memset(tmpstring, 0, sizeof(tmpstring));
	//LoadStr("AuthMode", cfg);
	if(!LoadStr("AuthMode", cfg))
	{
		if(OptMod_wifi==0)
			sprintf(cfg,"OPEN");

	}
	idx=SendMessage(WifiWnd[2],CB_GETCURSEL,0,0);
	sprintf(tmpstring,"%s",modetype[2+idx]);

	char buf[10]={0};
	LoadStr("NetworkType", buf);	
	if((strncmp(buf, "Adhoc", 5) == 0) && (strncmp(tmpstring, "WPAPSK", 6) == 0))
	{
		idx+=2;
	}	
	sprintf(tmpstring,"%s",modetype[2+idx]); 
	//GetWindowText(WifiWnd[2], tmpstring, 256);
	if(strncmp(cfg, tmpstring, 256)!=0)
		return 1;

	memset(cfg, 0x00, sizeof(cfg));
	memset(tmpstring, 0, sizeof(tmpstring));
	//LoadStr("EncrypType", cfg);
	if(!LoadStr("EncrypType", cfg))
	{
		if(OptMod_wifi==0)
			sprintf(cfg,"NONE");
	}
	idx=SendMessage(WifiWnd[3],CB_GETCURSEL,0,0);	

	int index=SendMessage(WifiWnd[2],CB_GETCURSEL,0,0);
	sprintf(tmpstring,"%s",modetype[2+index]);
	//if(strncmp(tmpstring, "WPAPSK", 6) == 0)
	if((strncmp(tmpstring, "WPAPSK", 6) == 0) || (strncmp(tmpstring, "WPA2PSK", 7) == 0) || (strncmp(tmpstring, "WPANONE", 7) == 0))
	{
		idx += 2;	
	}
	else if(strncmp(tmpstring, "SHARED", 6) == 0)
	{
		idx = 1;	
	}
	memset(tmpstring, 0, sizeof(tmpstring));

	sprintf(tmpstring,"%s",modetype[8+idx]);
	//GetWindowText(WifiWnd[3], tmpstring, 256);
	if(strncmp(cfg, tmpstring, 256)!=0)
		return 1;

	memset(tmpstring, 0, sizeof(tmpstring));
	if(SendMessage(WifiWnd[5], CB_GETCURSEL, 0, 0)!=gOptions.wifidhcpfunon)
		return 1;

	if(strncmp(oldip[0], newip[0], 18)!=0)
		return 1;
	if(strncmp(oldip[1], newip[1], 18)!=0)
		return 1;
	if(strncmp(oldip[2], newip[2], 18)!=0)
		return 1;

	if(strncmp((char*)newpwd, (char*)oldpwd, 4*sizeof(WifiPWD))!=0)
		return 1;

	if(strncmp(newpwdid, oldpwdid, 2)!=0)
		return 1;

	if(strncmp(newwpapsk, oldwpapsk, 255)!=0)
		return 1;

	return 0;

}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int SaveWiFiSetting(HWND hWnd)
{
	char tmpstring[256];
	int idx=0;
	//Liaozz 20081009 fix bug 1008:3
	if (pwdflag && ((pwdtype1 == 1 && strcmp(newwpapsk, "") == 0) ||
				(pwdtype1 == 0 && strcmp((char*)newpwd[0].pwdchar, "") == 0 &&
				 strcmp((char*)newpwd[1].pwdchar, "") == 0 &&
				 strcmp((char*)newpwd[2].pwdchar, "") == 0 &&
				 strcmp((char*)newpwd[3].pwdchar, "") == 0))) {
		MessageBox1(hWnd, LoadStrByID(MID_WIFI_ERR4), LoadStrByID(HIT_RUN), MB_OK | MB_ICONSTOP);
		if(!ismenutimeout) SetFocusChild(WifiWnd[4]);
		curFocusWnd = 4;
		return 0;
	}
	//Liaozz 20081011 fix bug 1010:1
	if (ipflag == 1 && strcmp(newip[0],"") == 0) {
		MessageBox1(hWnd, LoadStrByID(MID_WIFI_ERR5), LoadStrByID(HIT_RUN), MB_OK | MB_ICONSTOP);
		if(!ismenutimeout) SetFocusChild(WifiWnd[6]);
		curFocusWnd = 6;
		return 0;
	}
	//Liaozz end
	memset(tmpstring, 0, sizeof(tmpstring));
	GetWindowText(WifiWnd[0], tmpstring, 256);
	if(tmpstring && strlen(tmpstring)>0)
		SaveStr("SSID", tmpstring, 1);
	else
	{
		MessageBox1(hWnd, LoadStrByID(MID_WIFI_ERR1), LoadStrByID(HIT_RUN), MB_OK | MB_ICONSTOP);
		if(!ismenutimeout) SetFocusChild(WifiWnd[0]);	
		return 0;
	}

	memset(tmpstring, 0, sizeof(tmpstring));
	//GetWindowText(WifiWnd[1], tmpstring, 256);
	idx=SendMessage(WifiWnd[1],CB_GETCURSEL,0,0);
	sprintf(tmpstring,"%s",modetype[idx]);
	SaveStr("NetworkType", tmpstring, 1);

	memset(tmpstring, 0, sizeof(tmpstring));
	idx=SendMessage(WifiWnd[2],CB_GETCURSEL,0,0);
	sprintf(tmpstring,"%s",modetype[2+idx]);
	char buf[10]={0};
	LoadStr("NetworkType", buf);	
	if((strncmp(buf, "Adhoc", 5) == 0) && (strncmp(tmpstring, "WPAPSK", 6) == 0))
	{
		idx+=2;
	}	
	sprintf(tmpstring,"%s",modetype[2+idx]);
	//GetWindowText(WifiWnd[2], tmpstring, 256);
	SaveStr("AuthMode",tmpstring, 1);

	memset(tmpstring, 0, sizeof(tmpstring));
	idx=SendMessage(WifiWnd[3],CB_GETCURSEL,0,0);

	//char buf[10]={0};
	memset(buf, 0, sizeof(buf));
	LoadStr("AuthMode", buf);
	if((strncmp(buf, "WPAPSK", 6) == 0) || (strncmp(buf, "WPA2PSK", 7) == 0) || (strncmp(buf, "WPANONE", 7) == 0))
	{
		idx+=2;
	}	
	else if(strncmp(buf, "SHARED", 6) == 0) //永远为wep加密模式
	{
		idx=1;
	}

	sprintf(tmpstring,"%s",modetype[8+idx]);
	//GetWindowText(WifiWnd[3], tmpstring, 256);
	SaveStr("EncrypType",tmpstring, 1);

	gOptions.wifidhcpfunon=SendMessage(WifiWnd[5], CB_GETCURSEL, 0, 0);
	SaveOptions(&gOptions);

	SaveStr("wifiip", (char*)newip[0], 1);
	SaveStr("wifimask", (char*)newip[1], 1);
	SaveStr("wifigateway", (char*)newip[2], 1);
	SaveStr("DefaultKeyID", newpwdid, 1);                      //select which password str
	SaveStr("Key1Type", (char*)newpwd[0].pwdtype, 1);
	SaveStr("Key1Str", (char*)newpwd[0].pwdchar, 1);
	SaveStr("Key2Type", (char*)newpwd[1].pwdtype, 1);
	SaveStr("Key2Str", (char*)newpwd[1].pwdchar, 1);
	SaveStr("Key3Type", (char*)newpwd[2].pwdtype, 1);
	SaveStr("Key3Str", (char*)newpwd[2].pwdchar, 1);
	SaveStr("Key4Type", (char*)newpwd[3].pwdtype, 1);
	SaveStr("Key4Str", (char*)newpwd[3].pwdchar, 1);
	SaveStr("WPAPSK", newwpapsk,1);
	return 1;
}

extern HWND hIMEWnd;
static int WiFiWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	//	int i;
	static char keyupFlag=0;	
	switch (message)
	{
		case MSG_CREATE:
			loadflag=1;
			InitWindow(hWnd);		//add controls
			InitParameters();
			if (OptMod_wifi==1)
			{
				if(!gOptions.wifidhcpfunon)
					curFocusWnd=6;
			}
			else
				curFocusWnd=0;
			SetFocusChild(WifiWnd[curFocusWnd]);
			UpdateWindow(hWnd,TRUE);
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

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());
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
			loadflag=0;
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				if (OptMod_wifi==1)
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				else
					PostMessage(hWnd, MSG_COMMAND, WIFI_EXIT, 0);
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN && (OptMod_wifi!=1))
			{
				char tmpssid[256];
				if(curFocusWnd==0)
				{
					GetWindowText(WifiWnd[0], tmpssid, 256);
					if(tmpssid && strlen(tmpssid)>0)
					{
						if(hIMEWnd!=HWND_INVALID)
						{
							SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
							hIMEWnd=HWND_INVALID;
						}
						curFocusWnd++;
					}
				} else if (curFocusWnd==1) {
					if(hIMEWnd!=HWND_INVALID)
					{
						SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
						hIMEWnd=HWND_INVALID;
					}
					curFocusWnd++;
				} else {
					if(++curFocusWnd>8) curFocusWnd=0;
					if((curFocusWnd==4&&pwdflag==0)||(curFocusWnd==6&&ipflag==0))
						++curFocusWnd;
				}			
				SetFocusChild(WifiWnd[curFocusWnd]);		
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP && (OptMod_wifi!=1))
			{
				char tmpssid[256];
				if(curFocusWnd==0)
				{
					GetWindowText(WifiWnd[0], tmpssid, 256);
					if(tmpssid && strlen(tmpssid)>0)
					{
						if(hIMEWnd!=HWND_INVALID)
						{
							SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
							hIMEWnd=HWND_INVALID;
						}
						curFocusWnd--;
					}
				} else if (curFocusWnd==1) {
					if(hIMEWnd!=HWND_INVALID){
						SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
						hIMEWnd=HWND_INVALID;
					}
					curFocusWnd--;

				} else {
					if(--curFocusWnd<0) curFocusWnd=8;
					if((curFocusWnd==6&&ipflag==0)||(curFocusWnd==4&&pwdflag==0))
						--curFocusWnd;
				}
				SetFocusChild(WifiWnd[curFocusWnd]);
				return 0;
			} else if(LOWORD(wParam)==SCANCODE_F9 ||
					((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && (LOWORD(wParam)==SCANCODE_F11))) {
				if((curFocusWnd==0 || curFocusWnd==1) && (gOptions.IMEFunOn==1 && gOptions.TFTKeyLayout!=3))
				{
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
				}
			} else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) 
					|| (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) &&curFocusWnd==0 &&
						gOptions.IMEFunOn==1 && gOptions.TFTKeyLayout==3)
				{
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight, 0);     //涓嶆敮鎸佷腑鏂囪緭鍏??
					return 0;
				}
				if(curFocusWnd>=1 && curFocusWnd<=8)
				{
					if(curFocusWnd==1 || curFocusWnd==2 || curFocusWnd==3 || curFocusWnd==5)
					{
						int tmpsel=SendMessage(WifiWnd[curFocusWnd], CB_GETCURSEL, 0, 0);
						if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT
								|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
						{
							if(--tmpsel<0)
							{
								if(curFocusWnd==1 || curFocusWnd==3 || curFocusWnd==5)
									tmpsel=1;
								else
									tmpsel=AuthCount-1;
							}
						}
						else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
						{
							if(curFocusWnd==1 || curFocusWnd==3 || curFocusWnd==5)
							{
								if(++tmpsel>1)
									tmpsel=0;
							}
							else
							{
								if(++tmpsel>AuthCount-1)
									tmpsel=0;
							}
						}
						SendMessage(WifiWnd[curFocusWnd], CB_SETCURSEL, tmpsel, 0);
					}
					else
					{
						if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT
								|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
						{
							if(curFocusWnd==7)
								curFocusWnd=8;
							else if(curFocusWnd==8)
								curFocusWnd=7;
						}
						else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
						{
							if(curFocusWnd==7)
								curFocusWnd=8;
							else if(curFocusWnd==8)
								curFocusWnd=7;
						}
						SetFocusChild(WifiWnd[curFocusWnd]);
					}
					return 0;
				}
			}
			else if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10)) 
			{
				char tmpssid[256];
				switch(curFocusWnd)
				{
					case 0:
						GetWindowText(WifiWnd[0], tmpssid, 256);
						if(tmpssid && strlen(tmpssid)>0)
							SendMessage(hWnd, MSG_COMMAND, WIFI_SAVE, 0);
						else
							MessageBox1(hWnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
						break;
					case 4:		//set password
						CreateWiFiPWDWindow(hWnd, pwdtype1);
						break;
					case 6:		//set ip address
						CreateWiFiIPWindow(hWnd);
						break;	
					case 8:		//exit
						SendMessage(hWnd, MSG_COMMAND, WIFI_EXIT, 0);
						break;
					default:
						SendMessage(hWnd, MSG_COMMAND, WIFI_SAVE, 0);
				}
				return 0;
			}
			//			else if (LOWORD(wParam) == gOptions.IMESwitchKey)
			else if(((LOWORD(wParam)==SCANCODE_F9) || (LOWORD(wParam)==SCANCODE_F11 
							&& (gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)))&& (OptMod_wifi!=1))
			{
				if(curFocusWnd==0 && gOptions.IMEFunOn==1)
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight, 0);     //涓嶆敮鎸佷腑鏂囪緭鍏??
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_MENU && (OptMod_wifi!=1))
			{
				SendMessage(hWnd, MSG_COMMAND, WIFI_SAVE, 0);
				return 0;
			}

			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(id==WIFI_SAVE)
			{
				if(isModified() && SaveWiFiSetting(hWnd))
				{
					//Liaozz 20081008 fix bug third 7
					//					MessageBox(hWnd ,HIT_RIGHT ,HIT_RUN,MB_OK| MB_ICONINFORMATION);
					MessageBox1(hWnd ,LoadStrByID(HID_RESTART) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
					//Liaozz end
					if(!ismenutimeout) PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				else
				{
					if(!ismenutimeout) PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
			else if(id==WIFI_EXIT)
			{
				if(isModified()&&MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA),LoadStrByID(MID_APPNAME),
							MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK)
					PostMessage(hWnd, MSG_COMMAND, WIFI_SAVE, 0);
				else
				{
					if(!ismenutimeout) PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
			break;

		case MSG_CLOSE:
			//			printf("close window\n");
			//UnloadBitmap(&wifibkg);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateWiFiWindow(HWND hWnd, char* selssid)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	if (gOptions.IMEFunOn)
		OptMod_wifi=0;
	else
		OptMod_wifi=1;
	//	printf("%s,%d\n",__FUNCTION__,OptMod_wifi);
	//gMainMenu_rc.right = gOptions.LCDWidth-1;
	//gMainMenu_rc.bottom = gOptions.LCDHeight-1;

	LoadWindowStr();
	memset(myssid, 0, 256);
	nstrcpy(myssid, selssid, strlen(selssid));

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER|WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_WIFI_SETTING);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = WiFiWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//if (LoadBitmap(HDC_SCREEN,&wifibkg,GetBmpPath("submenubg.jpg")))
	//	return 0;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
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

