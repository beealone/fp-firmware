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
#include "main.h"
#include <minigui/tftmullan.h>

#define LB_STATIC1 101
#define LB_STATIC2 102
#define LB_STATIC3 103
#define LB_STATIC4 104
#define LB_STATIC5 105
#define LB_STATIC6 106
#define LB_STATIC7 107
#define LB_STATIC8 108
#define LB_STATIC9 109
#define LB_STATIC10 110
#define LB_STATIC11 111
#define LB_STATIC12 112		//:

#define CB_FUN	113
#define ID_CODE 114
#define ID_NAME 115
#define CB_AUTO	116

#define ID_H1 117
#define ID_H2 118
#define ID_H3 119
#define ID_H4 120
#define ID_H5 121
#define ID_H6 122
#define ID_H7 123
#define ID_M1 124 
#define ID_M2 125
#define ID_M3 126
#define ID_M4 127
#define ID_M5 128
#define ID_M6 129
#define ID_M7 130

#define ID_SAVE 131 
#define ID_EXIT 132

HWND EdCode,EdName;
HWND EdH[7],EdM[7];
HWND CbFun,CbAuto;
HWND BtSave,BtExit;

HWND Edtmp,focuswnd;

//BITMAP smsmngbk;

TSHORTKEY mystkey;
int mhour[7]={0},mmin[7]={0};
static int bChanged = 0;
int funcount=0;
int tmpselkey;

#ifdef IKIOSK
char* funstr[40];
int funid[40] = {0, 1, -1, -1, -1, -1, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,-1, -1, -1, -1, -1, -1, -1,  -1, -1, -1};
#else
#ifndef FACE
int funid[6]={0,1,-1,-1,-1,-1};
#else
int funid[13]={0,1,-1,-1,-1, -1,-1,-1,-1,-1, -1,-1,-1};
#endif
#endif
char* stateName[6];
int tmpname;
extern int ifUseWorkCode;
extern HWND hIMEWnd;
extern int stkeyflag;        //Enter键是否被定义为快捷键标志

static void  SelectNext(HWND hWnd,WPARAM wParam)	
{
	int i;

	Edtmp  = GetFocusChild(hWnd);
	if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if(Edtmp==CbFun)
		{
			if(SendMessage(CbFun,CB_GETCURSEL,0,0)==1)
				focuswnd = EdCode;
			else
				focuswnd = BtSave;
		}
		else if(Edtmp==EdCode)
		{
			focuswnd = EdName;
		}
		else if(Edtmp==EdName)
		{
			if(gOptions.IMEFunOn)
			{		
				if(hIMEWnd != HWND_INVALID)
				{
					SendMessage(hIMEWnd,MSG_CLOSE,0,0);
					hIMEWnd = HWND_INVALID;
				}
			}
			//Liaozz 20081009
			if (gOptions.MustChoiceInOut)
				focuswnd = BtSave;
			else //Liaozz end
				focuswnd = CbAuto;

		}
		else if(Edtmp==CbAuto)
		{
			if(SendMessage(CbAuto,CB_GETCURSEL,0,0)==1)
				focuswnd = EdH[0];
			else
				focuswnd = BtSave;
		}
		else if (Edtmp == BtSave)
		{
			focuswnd = BtExit;
		}
		else if(Edtmp == BtExit)
		{
			focuswnd = CbFun;
		}
		else
		{
			for(i=0;i<7;i++)
			{
				if(Edtmp==EdH[i])
				{
					focuswnd = EdM[i];
					break;
				}
				else if(Edtmp==EdM[i])
				{
					focuswnd = (i<6)? EdH[i+1]:BtSave;
					break;
				}
			}
		}
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == CbFun)
		{
			focuswnd = BtExit;
		}
		else if (Edtmp == BtExit)
		{
			focuswnd = BtSave;
		}
		else if (Edtmp == BtSave)
		{
			if(tmpselkey==1)
			{
				if (!gOptions.MustChoiceInOut) { //Liaozz 20081009 fix bug 1008:1
					if(SendMessage(CbAuto,CB_GETCURSEL,0,0)==1)
						focuswnd = EdM[6];
					else
						focuswnd = CbAuto;
					//Liaozz 20081009 fix bug 1008:1
				} else {
					focuswnd = EdName;
				}//Liaozz end
			}
			else
				focuswnd = CbFun;
		}
		else if(Edtmp == CbAuto)
		{
			focuswnd = EdName;
		}
		else if(Edtmp == EdName)
		{
			if(gOptions.IMEFunOn==1)
			{
				if(hIMEWnd != HWND_INVALID)
				{
					SendMessage(hIMEWnd,MSG_CLOSE,0,0);
					hIMEWnd = HWND_INVALID;
				}
			}
			focuswnd = EdCode;
		}
		else if(Edtmp == EdCode)
		{
			focuswnd = CbFun;
		}
		else
		{
			for(i=0;i<7;i++)
			{
				if(Edtmp==EdH[i])
				{
					focuswnd = (i>0)?EdM[i-1]:CbAuto;
					break;
				}
				else if(Edtmp==EdM[i])
				{
					focuswnd = EdH[i];
					break;
				}
			}
		}
	}

	SetFocusChild(focuswnd);
}

static void timeEncode(int dHour, int dMin, int *destime)
{
	*destime = dHour*100 + dMin;
}

static void timeDecode(int restime, int *dHour, int *dMin)
{
	*dHour = restime/100;	
	*dMin = restime-((*dHour)*100);
}

static void LoadStateName(void)
{
	stateName[0]=LoadStrByID(HIT_SYSTEM5KEY1);      //上班签到
	stateName[1]=LoadStrByID(HIT_SYSTEM5KEY2);      //下班签退
	stateName[2]=LoadStrByID(HIT_SYSTEM5KEY3);      //加班签到
	stateName[3]=LoadStrByID(HIT_SYSTEM5KEY4);      //加班签退
	stateName[4]=LoadStrByID(HIT_SYSTEM5KEY5);      //外出
	stateName[5]=LoadStrByID(HIT_SYSTEM5KEY6);      //外出返回
}

#ifdef IKIOSK
static void LoadWinStr(void)
{
	funstr[0] = LoadStrByID(HIT_IKIOSK_KEYFUN0);
	funstr[1] = LoadStrByID(HIT_IKIOSK_KEYFUN1);
	funstr[2] = LoadStrByID(HIT_IKIOSK_KEYFUN2);
	funstr[3] = LoadStrByID(HIT_IKIOSK_KEYFUN3);
	funstr[4] = LoadStrByID(HIT_IKIOSK_KEYFUN4);
	funstr[5] = LoadStrByID(HIT_IKIOSK_KEYFUN5);
	funstr[6] = LoadStrByID(HIT_IKIOSK_KEYFUN6);
	funstr[7] = LoadStrByID(HIT_IKIOSK_KEYFUN7);
	funstr[8] = LoadStrByID(HIT_IKIOSK_KEYFUN8);
	funstr[9] = LoadStrByID(HIT_IKIOSK_KEYFUN9);
	funstr[10] = LoadStrByID(HIT_IKIOSK_KEYFUN10);
	funstr[11] = LoadStrByID(HIT_IKIOSK_KEYFUN11);
	funstr[12] = LoadStrByID(HIT_IKIOSK_KEYFUN12);
	funstr[13] = LoadStrByID(HIT_IKIOSK_KEYFUN13);
	funstr[14] = LoadStrByID(HIT_IKIOSK_KEYFUN14);
	funstr[15] = LoadStrByID(HIT_IKIOSK_KEYFUN15);
	funstr[16] = LoadStrByID(HIT_IKIOSK_KEYFUN16);
	funstr[17] = LoadStrByID(HIT_IKIOSK_KEYFUN17);
	funstr[18] = LoadStrByID(HIT_IKIOSK_KEYFUN18);
	funstr[19] = LoadStrByID(HIT_IKIOSK_KEYFUN19);
	funstr[20] = LoadStrByID(HIT_IKIOSK_KEYFUN20);
	funstr[21] = LoadStrByID(HIT_IKIOSK_KEYFUN21);
	funstr[22] = LoadStrByID(HIT_IKIOSK_KEYFUN22);
	funstr[23] = LoadStrByID(HIT_IKIOSK_KEYFUN23);
	funstr[24] = LoadStrByID(HIT_IKIOSK_KEYFUN24);
	funstr[25] = LoadStrByID(HIT_IKIOSK_KEYFUN25);
	funstr[26] = LoadStrByID(HIT_IKIOSK_KEYFUN26);
	funstr[27] = LoadStrByID(HIT_IKIOSK_KEYFUN27);
	funstr[28] = LoadStrByID(HIT_IKIOSK_KEYFUN28);
	funstr[29] = LoadStrByID(HIT_IKIOSK_KEYFUN29);
	funstr[30] = LoadStrByID(HIT_IKIOSK_KEYFUN30);
	funstr[31] = LoadStrByID(HIT_IKIOSK_KEYFUN31);
	funstr[32] = LoadStrByID(HIT_IKIOSK_KEYFUN32);
	funstr[33] = LoadStrByID(HIT_IKIOSK_KEYFUN33);
	funstr[34] = LoadStrByID(HIT_IKIOSK_KEYFUN34);
	funstr[35] = LoadStrByID(HIT_IKIOSK_KEYFUN35);
	funstr[36] = LoadStrByID(HIT_IKIOSK_KEYFUN36);
	funstr[37] = LoadStrByID(HIT_IKIOSK_KEYFUN37);
	funstr[38] = LoadStrByID(HIT_IKIOSK_KEYFUN38);
	funstr[39] = LoadStrByID(HIT_IKIOSK_KEYFUN39);
}
#endif

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int InfoErr(HWND hWnd) //ok
{
	int i,j;
	TSHORTKEY tsk;

	int thour, tmin;
	int tmptimevalue;
	int tmpcode;
	char tstr[STATE_NAME_LEN+2];
	int timevalue[7];
	int curfun;

	memset(&tsk,0,sizeof(TSHORTKEY));	//zsliu

	curfun = funid[tmpselkey];
	if(curfun==1)	//state key
	{
		memset(tstr, 0, sizeof(tstr));
		GetWindowText(EdCode,tstr,3);
		tmpcode=atoi(tstr);
		if(strlen(tstr)==0 || tmpcode<0 || tmpcode>254)
		{
			MessageBox1(hWnd, LoadStrByID(MID_STATECODEERROR), LoadStrByID(MID_APPNAME), 
					MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
			if(!ismenutimeout) SetFocusChild(EdCode);
			return 1;	
		}

		if (gOptions.IMEFunOn==1)
		{
			memset(tstr, 0, STATE_NAME_LEN+1);
			GetWindowText(EdName, tstr, STATE_NAME_LEN);
		}

		for(i=0;i<STKEY_COUNT;i++)
		{
			memset(&tsk,0,sizeof(TSHORTKEY));
			if(FDB_GetShortKey(i+1, &tsk) != NULL)
			{
				//printf("tsk.keyID:%d,mystkey.keyID:%d\n",tsk.keyID,mystkey.keyID);
				if(tsk.keyID != mystkey.keyID)
				{
					//stateCode
					if(tsk.stateCode==tmpcode)
					{
						MessageBox1(hWnd,LoadStrByID(MID_STATECODEREPEAT), LoadStrByID(MID_APPNAME),
								MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
						if(!ismenutimeout) SetFocusChild(EdCode);			
						return 1;
					}

					char strStateName[STATE_NAME_LEN+2]={0};
					memset(strStateName, 0, sizeof(strStateName));
					Str2UTF8(tftlocaleid, (unsigned char*)tsk.stateName, (unsigned char*)strStateName);

					if(gOptions.IMEFunOn==1)
					{
						//stateName
						if(tstr && strlen(tstr)>0 && (strncmp(tstr,strStateName,STATE_NAME_LEN)==0))
						{
							MessageBox1(hWnd,LoadStrByID(MID_STATENAMEREPEAT), LoadStrByID(MID_APPNAME),
									MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
							if(!ismenutimeout) SetFocusChild(EdName);
							return 1;
						}
					}
					else
					{
						if(strlen(tstr)==strlen(stateName[tmpname]) &&
								strncmp(stateName[tmpname], tstr, strlen(stateName[tmpname]))==0)
						{
							MessageBox1(hWnd,LoadStrByID(MID_STATENAMEREPEAT), LoadStrByID(MID_APPNAME),
									MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
							if(!ismenutimeout) SetFocusChild(EdName);
							return 1;
						}
					}
					//change time.	
					if(SendMessage(CbAuto,CB_GETCURSEL,0,0)==1 && tsk.autochange)	//auto change
					{
						timevalue[0]=tsk.Time1;
						timevalue[1]=tsk.Time2;
						timevalue[2]=tsk.Time3;
						timevalue[3]=tsk.Time4;
						timevalue[4]=tsk.Time5;
						timevalue[5]=tsk.Time6;
						timevalue[6]=tsk.Time7;
						for(j=0;j<7;j++)
						{
							memset(tstr,0,4);
							GetWindowText(EdH[j],tstr,2);
							thour = atoi(tstr);
							memset(tstr,0,4);
							GetWindowText(EdM[j],tstr,2);
							tmin = atoi(tstr);
							timeEncode(thour,tmin,&tmptimevalue);
							if(tmptimevalue==timevalue[j])
							{
								MessageBox1(hWnd,LoadStrByID(MID_WKCDACTIMEHINT), LoadStrByID(MID_APPNAME),
										MB_OK|MB_ICONINFORMATION|MB_BASEDONPARENT);
								if(!ismenutimeout) SetFocusChild(EdH[j]);
								return 1;
							}
						}
					}
				}
			}
		}
	}
	else	//function key
	{
		if(curfun != 0)
		{
			for(i=0; i<STKEY_COUNT; i++)
			{
				memset(&tsk, 0, sizeof(TSHORTKEY));
				if(FDB_GetShortKey(i+1, &tsk) != NULL)
				{
					if(tsk.keyID != mystkey.keyID && tsk.keyFun == curfun)
					{
						MessageBox1(hWnd,LoadStrByID(MID_STKEYFUNREPEAT), LoadStrByID(MID_APPNAME),
								MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
						if(!ismenutimeout) SetFocusChild(CbFun);			
						return 1;
					}
				}
			}
		}	
	}
	return 0;
}

static void InitWindow (HWND hWnd)
{
	int i;
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9,posX10,posX11,posX12;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=260+gOptions.ControlOffset/2;
		posX2=250+gOptions.ControlOffset/2;
		posX3=215+gOptions.ControlOffset/2;
		posX4=200+gOptions.ControlOffset/2;
		posX5=165+gOptions.ControlOffset/2;
		posX6=25+gOptions.ControlOffset/2;
		posX7=70+gOptions.ControlOffset/2;
		posX8=30+gOptions.ControlOffset/2;
		posX9=120+gOptions.ControlOffset/2;
		posX10=100+gOptions.ControlOffset/2;
		posX11=120+gOptions.ControlOffset/2;
		posX12=240+gOptions.ControlOffset/2;
	}
	else
	{
		posX1=10+gOptions.ControlOffset/2;
		posX2=20+gOptions.ControlOffset/2;
		posX3=60+gOptions.ControlOffset/2;
		posX4=80+gOptions.ControlOffset/2;
		posX5=120+gOptions.ControlOffset/2;
		posX6=170+gOptions.ControlOffset/2;
		posX7=230+gOptions.ControlOffset/2;
		posX8=270+gOptions.ControlOffset/2;
		posX9=80+gOptions.ControlOffset/2;
		posX10=120+gOptions.ControlOffset/2;
		posX11=170+gOptions.ControlOffset/2;
		posX12=10+gOptions.ControlOffset/2;
	}

	CreateWindow(CTRL_STATIC,LoadStrByID(MID_SHORTKEYFUN), WS_VISIBLE | SS_LEFT, LB_STATIC1,posX1,10,70,23,hWnd,0);
	CbFun = CreateWindow(CTRL_COMBOBOX,"", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
			CB_FUN,posX9+10,6,120,23,hWnd,0);

	SendMessage(CbFun,CB_ADDSTRING,0,(WPARAM)LoadStrByID(MID_STKEYFUN1));			//未定义
	SendMessage(CbFun,CB_ADDSTRING,0,(WPARAM)LoadStrByID(MID_STKEYFUN2));			//状态键
	funcount=2;

	if(ifUseWorkCode)
	{
		SendMessage(CbFun,CB_ADDSTRING,0,(WPARAM)LoadStrByID(MID_STKEYFUN3));		//工作状态
		funid[funcount]=2;
		funcount++;
	}
	if(gOptions.IsSupportSMS)
	{
		SendMessage(CbFun,CB_ADDSTRING,0,(WPARAM)LoadStrByID(MID_STKEYFUN4));		//查看短消息
		funid[funcount]=3;
		funcount++;
	}
	if(gOptions.LockFunOn)
	{
		SendMessage(CbFun,CB_ADDSTRING,0,(WPARAM)LoadStrByID(MID_STKEYFUN5));		//求助键
		funid[funcount]=4;
		funcount++;
	}

#ifdef FACE
	if(gOptions.FaceFunOn)
	{
		char *tmp[8];
		tmp[0]=LoadStrByID(MID_STKEYFUN6);
		tmp[1]=LoadStrByID(MID_ONE_FACE);
		tmp[2]=LoadStrByID(MID_G_FACE);
		tmp[3]=LoadStrByID(MID_STKEYFUN10);
		tmp[4]=LoadStrByID(MID_STKEYFUN11);
		tmp[5]=LoadStrByID(MID_STKEYFUN12);
		tmp[6]=LoadStrByID(MID_STKEYFUN13);
		tmp[7]=LoadStrByID(MID_STKEYFUN14);
		for(i=0;i<8;i++)
		{
			SendMessage(CbFun,CB_ADDSTRING,0,(WPARAM)tmp[i]);
			funid[funcount]=STK_FUN+i;
			funcount++;
		}
	}
#endif

#ifdef IKIOSK
	for (i=0; i<16; i++)
	{
		SendMessage(CbFun, CB_ADDSTRING, 0, (WPARAM)funstr[i+5]);
		funid[funcount]=i+5;
		funcount++;
	}

	if (gOptions.AutoAlarmFunOn)
	{
		SendMessage(CbFun, CB_ADDSTRING, 0, (WPARAM)funstr[21]);
		funid[funcount]=21;
		funcount++;
	}

	for (i=0; i<12; i++)
	{
		SendMessage(CbFun, CB_ADDSTRING, 0, (WPARAM)funstr[i+22]);
		funid[funcount]=i+22;
		funcount++;
	}

	if (gOptions.LockFunOn)
	{
		for (i=0; i<6; i++)
		{
			SendMessage(CbFun, CB_ADDSTRING, 0, (WPARAM)funstr[i+34]);
			funid[funcount]=i+34;
			funcount++;
		}
	}
#endif

	CreateWindow(CTRL_STATIC,LoadStrByID(MID_WKCDCODE), SS_LEFT, LB_STATIC2,posX1,34,45,23,hWnd,0);
	EdCode = CreateWindow(CTRL_SLEDIT, "", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_CODE,posX3,30,45,23,hWnd,0);
	SendMessage(EdCode,EM_LIMITTEXT,3,0L);

	CreateWindow(CTRL_STATIC,LoadStrByID(MID_WKCDNAME), SS_LEFT, LB_STATIC3,posX5,34,45,23,hWnd,0);
	//IF4机型不能修改工作状态名称
	if(gOptions.IMEFunOn==1)
	{
		EdName = CreateWindow(CTRL_SLEDIT, "", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_NAME,posX6-5,30,145,23,hWnd,0);
		SendMessage(EdName,EM_LIMITTEXT,STATE_NAME_LEN,0L);
		SendMessage(EdName, MSG_ENABLE, 1, 0);
	}
	else
	{
		LoadStateName();
		EdName = CreateWindow(CTRL_COMBOBOX,"", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,ID_NAME,posX6-5,30,145,23,hWnd,0);
		for(i=0;i<6;i++)
		{
			SendMessage(EdName,CB_ADDSTRING,0,(WPARAM)stateName[i]);
		}
	}

	CreateWindow(CTRL_STATIC,LoadStrByID(MID_WKCDCHGTIME), SS_LEFT, LB_STATIC4,posX12,58,90,23,hWnd,0);
	CbAuto = CreateWindow(CTRL_COMBOBOX,"", CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
			CB_AUTO,posX10,54,120,23,hWnd,0);
	SendMessage(CbAuto,CB_ADDSTRING,0,(WPARAM)LoadStrByID(MID_WKCDDOOROFF));
	SendMessage(CbAuto,CB_ADDSTRING,0,(WPARAM)LoadStrByID(MID_WKCDDOORON));

	CreateWindow(CTRL_STATIC,LoadStrByID(HID_DAY0), SS_LEFT, LB_STATIC5,posX2,82,60,23,hWnd,0);
	EdH[0]=CreateWindow(CTRL_SLEDIT,"", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_H1,posX4,78,35,23,hWnd,0);
	EdM[0]=CreateWindow(CTRL_SLEDIT,"",WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_M1,posX5,78,35,23,hWnd,0);

	CreateWindow(CTRL_STATIC,LoadStrByID(HID_DAY1), SS_LEFT, LB_STATIC6,posX11,82,60,23,hWnd,0);
	EdH[1]=CreateWindow(CTRL_SLEDIT,"",WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_H2,posX7,78,35,23,hWnd,0);
	EdM[1]=CreateWindow(CTRL_SLEDIT,"",WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_M2,posX8,78,35,23,hWnd,0);

	CreateWindow(CTRL_STATIC,LoadStrByID(HID_DAY2),SS_LEFT, LB_STATIC7,posX2,106,60,23,hWnd,0);
	EdH[2]=CreateWindow(CTRL_SLEDIT,"", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_H3,posX4,102,35,23,hWnd,0);
	EdM[2]=CreateWindow(CTRL_SLEDIT,"", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_M3,posX5,102,35,23,hWnd,0);

	CreateWindow(CTRL_STATIC,LoadStrByID(HID_DAY3), SS_LEFT, LB_STATIC8,posX11,106,60,23,hWnd,0);
	EdH[3]=CreateWindow(CTRL_SLEDIT,"", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_H4,posX7,102,35,23,hWnd,0);
	EdM[3]=CreateWindow(CTRL_SLEDIT,"", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_M4,posX8,102,35,23,hWnd,0);

	CreateWindow(CTRL_STATIC,LoadStrByID(HID_DAY4),SS_LEFT, LB_STATIC9,posX2,130,60,23,hWnd,0);
	EdH[4]=CreateWindow(CTRL_SLEDIT,"",WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_H5,posX4,126,35,23,hWnd,0);
	EdM[4]=CreateWindow(CTRL_SLEDIT,"",WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_M5,posX5,126,35,23,hWnd,0);

	CreateWindow(CTRL_STATIC,LoadStrByID(HID_DAY5),SS_LEFT, LB_STATIC10,posX11,130,60,23,hWnd,0);
	EdH[5]=CreateWindow(CTRL_SLEDIT,"", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_H6,posX7,126,35,23,hWnd,0);
	EdM[5]=CreateWindow(CTRL_SLEDIT,"",WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_M6,posX8,126,35,23,hWnd,0);

	CreateWindow(CTRL_STATIC,LoadStrByID(HID_DAY6),SS_LEFT, LB_STATIC11,posX2,154,60,23,hWnd,0);
	EdH[6]=CreateWindow(CTRL_SLEDIT,"", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_H7,posX4,150,35,23,hWnd,0);
	EdM[6]=CreateWindow(CTRL_SLEDIT,"", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER, ID_M7,posX5,150,35,23,hWnd,0);

	for(i=0;i<7;i++)
	{
		SendMessage(EdH[i],EM_LIMITTEXT,2,0L);
		SendMessage(EdM[i],EM_LIMITTEXT,2,0L);
	}

	BtSave = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_SAVE), WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, ID_SAVE,10,190,85,23,hWnd,0);
	BtExit = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_EXIT), WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, ID_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);

}

static void refreshWindow(HWND hWnd)
{
	int i;

	if(tmpselkey==1)
	{
		ShowWindow(GetDlgItem(hWnd,LB_STATIC2),SW_SHOW);
		ShowWindow(EdCode,SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC3),SW_SHOW);
		ShowWindow(EdName,SW_SHOW);
		//Liaozz 20081009 fix bug 1008:1
		if (!gOptions.MustChoiceInOut) {
			ShowWindow(GetDlgItem(hWnd,LB_STATIC4),SW_SHOW);
			ShowWindow(CbAuto,SW_SHOW);
		}
		//Liaozz end
		if(SendMessage(CbAuto,CB_GETCURSEL,0,0)==1)
		{
			ShowWindow(GetDlgItem(hWnd,LB_STATIC5),SW_SHOW);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC6),SW_SHOW);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC7),SW_SHOW);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC8),SW_SHOW);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC9),SW_SHOW);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC10),SW_SHOW);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC11),SW_SHOW);
			for(i=0;i<7;i++)
			{
				ShowWindow(EdH[i],SW_SHOW);
				ShowWindow(EdM[i],SW_SHOW);
			}
		}
		else
		{
			ShowWindow(GetDlgItem(hWnd,LB_STATIC5),SW_HIDE);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC6),SW_HIDE);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC7),SW_HIDE);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC8),SW_HIDE);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC9),SW_HIDE);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC10),SW_HIDE);
			ShowWindow(GetDlgItem(hWnd,LB_STATIC11),SW_HIDE);
			for(i=0;i<7;i++)
			{
				ShowWindow(EdH[i],SW_HIDE);
				ShowWindow(EdM[i],SW_HIDE);
			}
		}		
	}
	else
	{
		ShowWindow(GetDlgItem(hWnd,LB_STATIC2),SW_HIDE);
		ShowWindow(EdCode,SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC3),SW_HIDE);
		ShowWindow(EdName,SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC4),SW_HIDE);
		ShowWindow(CbAuto,SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC5),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC6),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC7),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC8),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC9),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC10),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,LB_STATIC11),SW_HIDE);
		for(i=0;i<7;i++)
		{
			ShowWindow(EdH[i],SW_HIDE);
			ShowWindow(EdM[i],SW_HIDE);
		}
	}
}

static int getfunindex(int fun)
{
	int i;
#ifdef IKIOSK
	int mc = 40;
#else
	int mc = 5;
#endif

#ifdef FACE
	if(gOptions.FaceFunOn)
		mc+=8;
#endif

	for(i=0; i<mc; i++)
	{
		if(funid[i]==fun)
			return i;
	}
	return -1;
}

static int getnameindex(char *name)
{
	int i;

	for (i=0;i<6;i++)
	{
		if((strlen(name)==strlen(stateName[i]))&&strncmp(name,stateName[i],strlen(name))==0)
		{
			return i;
		}
	}
	return 0;
}

static int FillShortKeyData(HWND hWnd)
{
	char tmpstr[40];
	int i;

	tmpselkey=getfunindex(mystkey.keyFun);
	if(tmpselkey>=0)
		SendMessage(CbFun, CB_SETCURSEL, tmpselkey, 0);

	if(mystkey.keyFun==1)
	{
		memset(tmpstr,0,sizeof(tmpstr));
		sprintf(tmpstr,"%d",mystkey.stateCode);
		SetWindowText(EdCode,tmpstr);
		memset(tmpstr,0,sizeof(tmpstr));
		Str2UTF8(tftlocaleid,(unsigned char*)mystkey.stateName,(unsigned char*)tmpstr);
		//memcpy(tmpstr,mystkey.stateName,STATE_NAME_LEN);
		if (gOptions.IMEFunOn==1)       //lyy 20090721
		{
			SetWindowText(EdName,tmpstr);
		}
		else
		{
			tmpname = getnameindex(tmpstr);
			SendMessage(EdName, CB_SETCURSEL, tmpname, 0);
		}
		if (!gOptions.MustChoiceInOut) {//Liaozz 20081009 fix bug 1008:1
			SendMessage(CbAuto,CB_SETCURSEL,mystkey.autochange,0);
			if(mystkey.autochange)
			{
				for(i=0;i<7;i++)
				{
					memset(tmpstr,0,sizeof(tmpstr));
					sprintf(tmpstr,"%02d",mhour[i]);
					SetWindowText(EdH[i],tmpstr);
					memset(tmpstr,0,sizeof(tmpstr));
					sprintf(tmpstr,"%02d",mmin[i]);
					SetWindowText(EdM[i],tmpstr);
				}
			}
			else
			{
				for(i=0;i<7;i++)
				{
					SetWindowText(EdH[i],"00");
					SetWindowText(EdM[i],"00");
				}
			}
			//Liaozz 20081009 fix bug 1008:1
		} else
			SendMessage(CbAuto,CB_SETCURSEL,0,0);
		//Liaozz end
	}
	else
	{
		SetWindowText(EdCode,"");
		if (gOptions.IMEFunOn==1)
		{
			SetWindowText(EdName,"");
		}
		else
		{
			SendMessage(EdName, CB_SETCURSEL, 0, 0);
		}
		for(i=0;i<7;i++)
		{
			SetWindowText(EdH[i],"00");
			SetWindowText(EdM[i],"00");
		}
		SendMessage(CbAuto,CB_SETCURSEL,0,0);
	}
	refreshWindow(hWnd);
	return 1;

}

static int ismodified(void)
{
	char tmpstr[STATE_NAME_LEN+2];
	int i;

	//Fun
	if(funid[tmpselkey]!=mystkey.keyFun) return 1;

	if(mystkey.keyFun==1)
	{
		//state code
		memset(tmpstr,0,sizeof(tmpstr));
		GetWindowText(EdCode,tmpstr,3);
		if(atoi(tmpstr)!=mystkey.stateCode) return 1;

		if(gOptions.IMEFunOn==1)
		{
			memset(tmpstr,0,sizeof(tmpstr));
			GetWindowText(EdName,tmpstr,STATE_NAME_LEN);
			if (strlen(tmpstr)!=strlen(mystkey.stateName) || strncmp(tmpstr, mystkey.stateName, strlen(tmpstr))!=0) return 1;
		}
		else
		{
			memset(tmpstr,0,sizeof(tmpstr));
			Str2UTF8(tftlocaleid, (unsigned char*)mystkey.stateName, (unsigned char*)tmpstr);    //modify by lyy 2009.0724
			if(strlen(tmpstr)!=strlen(stateName[tmpname]) || strncmp(tmpstr, stateName[tmpname],strlen(tmpstr))!=0) return 1;
		}
		if (!gOptions.MustChoiceInOut) { //Liaozz 20081009 fix bug 1008:1
			if(SendMessage(CbAuto,CB_GETCURSEL,0,0)!=mystkey.autochange) return 1;
			if(mystkey.autochange)
			{
				//time
				for(i=0;i<7;i++)
				{
					memset(tmpstr,0,sizeof(tmpstr));
					GetWindowText(EdH[i],tmpstr,2);
					if((atoi(tmpstr)) != mhour[i]) return 1;
					memset(tmpstr,0,sizeof(tmpstr));
					GetWindowText(EdM[i],tmpstr,2);
					if((atoi(tmpstr)) != mmin[i]) return 1;
				}
			}
		}//Liaozz end
	}
	return 0;
}

extern int UTF8toLocal(int lid,const unsigned char *utf8, unsigned char *str);
static int SaveShortKeyInfo(HWND hWnd)
{
	int tmptime[7];
	int i;
	char tmpstr1[4];
	char tmpstr2[4];
	char tmpstr[STATE_NAME_LEN+2];

	mystkey.keyFun = funid[tmpselkey];

#ifdef IKIOSK	
	if (mystkey.keyFun==STK_UPDTHEME)
	{
		MessageBox1(hWnd, LoadStrByID(MID_IKIOSK_FUNLIMIT), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONINFORMATION);
		return 0;
	}
#endif

	if(mystkey.keyFun==1)	//state key.
	{
		char tmpstcode[STATE_NAME_LEN+2];
		memset(tmpstr,0,sizeof(tmpstr));
		GetWindowText(EdCode,tmpstr,4);
		mystkey.stateCode = atoi(tmpstr);

		memset(tmpstr,0,sizeof(tmpstr));
		memset(mystkey.stateName,0,STATE_NAME_LEN+2);
		GetWindowText(EdName,tmpstr,STATE_NAME_LEN);
		memset(tmpstcode,0,sizeof(tmpstcode));
		UTF8toLocal(tftlocaleid,(unsigned char*)tmpstr,(unsigned char*)tmpstcode);
		memcpy(mystkey.stateName,tmpstcode,STATE_NAME_LEN);

		if (!gOptions.MustChoiceInOut) {//Liaozz 20081009 fix bug 1008:1
			mystkey.autochange = SendMessage(CbAuto,CB_GETCURSEL,0,0);
			if(mystkey.autochange)
			{
				for(i=0; i<7; i++)
				{
					memset(tmpstr1,0,4);
					memset(tmpstr2,0,4);
					GetWindowText(EdH[i],tmpstr1,4);
					GetWindowText(EdM[i],tmpstr2,4);
					timeEncode(atoi(tmpstr1),atoi(tmpstr2),&tmptime[i]);
				}	
				mystkey.Time1 = tmptime[0];
				mystkey.Time2 = tmptime[1];
				mystkey.Time3 = tmptime[2];
				mystkey.Time4 = tmptime[3];
				mystkey.Time5 = tmptime[4];
				mystkey.Time6 = tmptime[5];
				mystkey.Time7 = tmptime[6];
			}
			//Liaozz 20081009 fix bug 1008:1
		} else {
			mystkey.Time1 = 0;
			mystkey.Time2 = 0;
			mystkey.Time3 = 0;
			mystkey.Time4 = 0;
			mystkey.Time5 = 0;
			mystkey.Time6 = 0;
			mystkey.Time7 = 0;
			mystkey.autochange = 0;
		}
		//Liaozz end
	}
	else
	{
		mystkey.stateCode=-1;
		memset(mystkey.stateName,0,STATE_NAME_LEN+2);
#ifdef FACE
		if(gOptions.FaceFunOn)
		{
			int index=-1;
			char buf[64];
			memset(buf,0,sizeof(buf));
			index=SendMessage(CbFun,CB_GETCURSEL,0,0);
			if(index>=0)
				SendMessage(CbFun,CB_GETLBTEXT,index,buf);

			UTF8toLocal(tftlocaleid,(unsigned char*)buf,(unsigned char*)mystkey.stateName);
		}
#endif

		mystkey.Time1=0;
		mystkey.Time2=0;
		mystkey.Time3=0;
		mystkey.Time4=0;
		mystkey.Time5=0;
		mystkey.Time6=0;
		mystkey.Time7=0;
		mystkey.autochange=0;
	}

	return (FDB_ChgShortKey(&mystkey)==FDB_OK);
}

static int stkeymngproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	int tmpvalue;

	char tmpchar[STATE_NAME_LEN+2];//
	char tmpstr[20];
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitWindow(hWnd);		//add controls
			FillShortKeyData(hWnd);
			UpdateWindow(hWnd, TRUE);
			SetFocusChild(CbFun);
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
			hdc = BeginPaint(hWnd);	
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

			Edtmp = GetFocusChild(hWnd);
			if (LOWORD(wParam)==SCANCODE_ESCAPE)
				PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0L);

			//			else if (LOWORD(wParam) == gOptions.IMESwitchKey)
			if (LOWORD(wParam)==SCANCODE_F9 || 
					((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && LOWORD(wParam)==SCANCODE_F11))
			{
				if(Edtmp==EdName && (gOptions.IMEFunOn==1))
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
				return 0;
			}

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				SelectNext(hWnd,wParam);
				return 0;
			}

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				if(Edtmp==CbFun)
				{
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
					{
						if(--tmpselkey<0) tmpselkey=funcount-1;
#ifdef FACE
						if(!gOptions.FaceFunOn)
#endif
						{
							if (tmpselkey==STK_UPDTHEME) tmpselkey--;
						}
					}
					else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						if(++tmpselkey>funcount-1) tmpselkey=0;
#ifdef FACE
						if(!gOptions.FaceFunOn)
#endif
						{
							if (tmpselkey==STK_UPDTHEME) tmpselkey++;
						}
					}
					SendMessage(CbFun,CB_SETCURSEL,tmpselkey,0);
					refreshWindow(hWnd);
				}
				else if(Edtmp==EdName)
				{
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) //left
					{
						if(gOptions.IMEFunOn==0)
						{
							if(--tmpname<0)
								tmpname=5;
							SendMessage(EdName,CB_SETCURSEL,tmpname,0);
							refreshWindow(hWnd);
						}
					}
					else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT) //for 3.0 and 3.5
					{
						if(gOptions.IMEFunOn==1)
						{
							if( gOptions.TFTKeyLayout==3)
							{
								T9IMEWindow(hWnd,0,200,gOptions.LCDWidth,gOptions.LCDHeight,gOptions.HzImeOn);
								return 0;
							}
						}
						else
						{
							if(++tmpname>5) 
								tmpname=0;
							SendMessage(EdName,CB_SETCURSEL,tmpname,0);
							refreshWindow(hWnd);
						}
					}
				}
				else if(Edtmp==CbAuto)
				{
					if(SendMessage(CbAuto,CB_GETCURSEL,0,0)==0)
						SendMessage(CbAuto,CB_SETCURSEL,1,0);
					else
						SendMessage(CbAuto,CB_SETCURSEL,0,0);
					refreshWindow(hWnd);
				}
				else if(Edtmp==BtSave)
				{
					SetFocusChild(BtExit);
				}
				else if(Edtmp==BtExit)
				{
					SetFocusChild(BtSave);
				}
				return 0;
			}

			if ((LOWORD(wParam)==SCANCODE_BACKSPACE) && gOptions.TFTKeyLayout==3)
			{
				if(Edtmp==CbFun)
				{
					if(--tmpselkey<0) tmpselkey=funcount-1;
					SendMessage(CbFun,CB_SETCURSEL,tmpselkey,0);
					refreshWindow(hWnd);
				}
				else if(Edtmp==EdName && gOptions.IMEFunOn==0)
				{
					if(--tmpname<0) tmpname=5;
					SendMessage(EdName,CB_SETCURSEL,tmpname,0);
					refreshWindow(hWnd);
				}
				else if(Edtmp==CbAuto)
				{
					if(SendMessage(CbAuto,CB_GETCURSEL,0,0)==0)
						SendMessage(CbAuto,CB_SETCURSEL,1,0);
					else
						SendMessage(CbAuto,CB_SETCURSEL,0,0);
					refreshWindow(hWnd);
				}
				else if(Edtmp==BtSave)
				{
					SetFocusChild(BtExit);
				}
				else if(Edtmp==BtExit)
				{
					SetFocusChild(BtSave);
				}
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_ENTER) 
			{
				if (Edtmp!=BtExit)
				{
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0L);
					return 0;
				}
			}

			if(LOWORD(wParam)==SCANCODE_F10)
			{
				if(Edtmp!=BtExit)
					SendMessage(hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
				else
					SendMessage(hWnd, MSG_COMMAND, (WPARAM)ID_EXIT, 0L);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_MENU)
			{	
				SendMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
				return 0;
			}
			break;

		case MSG_CHAR:
			Edtmp = GetFocusChild(hWnd);
			if(Edtmp==EdName && gOptions.IMEFunOn==1)
			{
				memset(tmpchar,0,sizeof(tmpchar));
				GetWindowText(EdName,tmpchar,STATE_NAME_LEN);
				//modify by jazzy 2008.12.02 for limit name low 14 char
				if ((tmpchar[0]&0xFF)>0x80)     //if arabic or other mul language
				{
					if (gOptions.Language!=83)
						SetWindowText(EdName,"");
				}
				else if((strlen(tmpchar)==STATE_NAME_LEN) && (wParam>=0x20)&&(wParam<0x7f))
				{
					SetWindowText(EdName,tmpchar);
					SendMessage(EdName,EM_SETCARETPOS,0,strlen(tmpchar));
					//return 0;
				}
			}
			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);

			if(nc==EN_CHANGE)
			{
				int i;

				//允许工作代码设为0
				if(id == ID_CODE)
				{
					char tmpss[MAX_WORKCODE_LEN+1];
					int codelen, tmpcode, tvalue;

					memset(tmpss, 0, sizeof(tmpss));
					GetWindowText(EdCode, tmpss, MAX_WORKCODE_LEN);
					codelen = strlen(tmpss);

					if (codelen > 1)
					{
						tmpcode = atoi(tmpss);
						tvalue = 1;
						for(i=1; i<codelen; i++)
							tvalue*=10;

						if(tmpcode < tvalue)
						{
							for(i=0; i<=codelen; i++)
							{
								tmpss[i] = tmpss[i+1];
							}
							SetWindowText(EdCode,tmpss);
							SendMessage(EdCode, EM_SETCARETPOS, 0,strlen(tmpss));
						}
					}
				}

				for(i=0; i<7; i++)
				{
					if (id==(ID_H1+i))
					{
						memset(tmpstr,0,sizeof(tmpstr));
						GetWindowText(EdH[i],tmpstr,2);
						tmpvalue = atoi(tmpstr);
						if(tmpvalue<0 || tmpvalue>23)
						{
							memset(tmpstr,0,sizeof(tmpstr));
							//sprintf(tmpstr,"%02d",mhour[i]); //zsliu change
							sprintf(tmpstr,"00");
							SetWindowText(EdH[i],tmpstr);
							SendMessage(EdH[i],EM_SETCARETPOS,0,strlen(tmpstr));
						}
					}
					else if (id==(ID_M1+i))
					{
						memset(tmpstr,0,4);
						GetWindowText(EdM[i],tmpstr,2);
						tmpvalue = atoi(tmpstr);
						if(tmpvalue<0 || tmpvalue>59)
						{
							memset(tmpstr,0,4);
							//sprintf(tmpstr,"%02d",mmin[i]); //zsliu change
							sprintf(tmpstr,"00");
							SetWindowText(EdM[i],tmpstr);
							SendMessage(EdM[i],EM_SETCARETPOS,0,strlen(tmpstr));
						}
					}
				}	
				break;
			}

			switch(id)
			{
				case ID_SAVE:			//保存
					if(!InfoErr(hWnd))
					{
						if(SaveShortKeyInfo(hWnd))
						{
							bChanged = 1;
							PostMessage(hWnd,MSG_CLOSE,0,0L);
						}
						printf("Save ShortKey Success!\n");
					}
					break;

				case ID_EXIT:				//返回
					if (ismodified() && MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA),LoadStrByID(MID_APPNAME),MB_OKCANCEL | MB_ICONQUESTION |	MB_BASEDONPARENT)==IDOK)
						PostMessage(hWnd,MSG_COMMAND,ID_SAVE,0L);
					else
					{
						if(!ismenutimeout)
						{
							bChanged = 0;
							PostMessage(hWnd,MSG_CLOSE,0,0L);
						}
					}
					break;
			}			

			break;

		case MSG_CLOSE:
			if(hIMEWnd!=HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd = HWND_INVALID;
			}
			//UnloadBitmap(&smsmngbk);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateShortKeyMngWindow(HWND hWnd, int keyID)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	char* wcapstr;
	char* tmpkeyname[7];

#ifdef IKIOSK
	LoadWinStr();
#endif
	memset(&mystkey,0,sizeof(TSHORTKEY));
	if(FDB_GetShortKey(keyID,&mystkey)!=NULL)
	{
		if(mystkey.autochange)
		{
			timeDecode(mystkey.Time1,&mhour[0],&mmin[0]);
			timeDecode(mystkey.Time2,&mhour[1],&mmin[1]);
			timeDecode(mystkey.Time3,&mhour[2],&mmin[2]);
			timeDecode(mystkey.Time4,&mhour[3],&mmin[3]);
			timeDecode(mystkey.Time5,&mhour[4],&mmin[4]);
			timeDecode(mystkey.Time6,&mhour[5],&mmin[5]);
			timeDecode(mystkey.Time7,&mhour[6],&mmin[6]);
		}
	}
	else
		return 0;

	bChanged = 0;
	if(gOptions.TFTKeyLayout!=3)
	{
		wcapstr=LoadStrByID(MID_SHORTKEY01+keyID-1);
		if(gOptions.TFTKeyLayout!=0 && gOptions.TFTKeyLayout!=4)
		{
			if(keyID==9) wcapstr=LoadStrByID(MID_WKCDKEY71);
			else if(keyID==10) wcapstr=LoadStrByID(MID_WKCDKEY72);
		}
	}
	else
	{
		tmpkeyname[0]=LoadStrByID(MID_SHORTKEY11);       //Back
		tmpkeyname[1]=LoadStrByID(MID_SHORTKEY15);       //Right
		tmpkeyname[2]=LoadStrByID(MID_WKCDKEY73);        //ESC
		tmpkeyname[3]=LoadStrByID(MID_SHORTKEY12);       //Up
		tmpkeyname[4]=LoadStrByID(MID_WKCDKEY74);        //Enter
		tmpkeyname[5]=LoadStrByID(MID_SHORTKEY13);       //Down
		tmpkeyname[6]=LoadStrByID(MID_WKCDKEY75);        //0

		wcapstr=tmpkeyname[keyID-1];
	}

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;

	CreateInfo.spCaption = wcapstr;
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = stkeymngproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//if (LoadBitmap(HDC_SCREEN,&smsmngbk,GetBmpPath("submenubg.jpg")))
	//         return 0;
	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//判断ENTER键是否被定义为快捷键(liming)
	if(gOptions.TFTKeyLayout==3)
	{
		stkeyflag = EnterIsSTkey(LOWORD(SCANCODE_ENTER));
		//                printf("stkeyflag:%d\n",stkeyflag);
	}

	MainWindowThreadCleanup(hMainWnd);
	return bChanged;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

