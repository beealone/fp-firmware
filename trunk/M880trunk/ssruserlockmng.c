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
#include <minigui/tftmullan.h>

#define LB_STATIC1 50
#define LB_STATIC2 51
#define LB_STATIC3 52
#define LB_STATIC4 53
#define LB_STATIC5 54
#define LB_STATIC6 55
#define LB_STATIC7 56
#define LB_STATIC8 57
#define LB_STATIC9 58

#define ID_GROUP	60
#define CB_VERIFY	61
#define CB_TIMEZONE	62
#define ID_TZ1		63
#define ID_TZ2		64
#define ID_TZ3		65
#define ID_DURFP	66
#define ID_SAVE 	70
#define ID_EXIT 	71

HWND EdGroup,CbVerify,CbTimeZone,tzWnd[3];
HWND StFpCount,stzWnd[3];
HWND BtFpMng,BtSave,BtExit;
HWND Edtmp,focuswnd;

//BITMAP userlockbk;
char tmpstr[50]={0};

static int userPIN=0;
TUser tuser;
TExtUser textuser;
TGroup tgroup;
TTimeZone ttz;

int bNewExtUser=0;

int gselVerify=0;
int gselTimeZone=0;	
int bchanged=0;		//


const char* myVerifyStyle[] = 
{
	"FP/PW/RF",
	"FP",
	"PIN", 
	"PW", 
	"RF",
	"FP/PW",
	"FP/RF",
	"PW/RF", 
	"PIN&FP", 
	"FP&PW", 
	"FP&RF", 
	"PW&RF", 
	"FP&PW&RF",
	"PIN&FP&PW",
	"FP&RF/PIN"
};

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int  SelectNext(HWND hWnd,WPARAM wParam)
{
	Edtmp  = GetFocusChild(hWnd);
	memset(tmpstr,0,sizeof(tmpstr));

	if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if(Edtmp==EdGroup)
		{
			GetWindowText(EdGroup,tmpstr,3);
			if(strlen(tmpstr)>0)
			{
				if(gOptions.UserExtendFormat)	
					focuswnd=CbVerify;
				else
					focuswnd=CbTimeZone;
			}
		}
		else if(Edtmp==CbVerify)
			focuswnd=CbTimeZone;
		else if(Edtmp==CbTimeZone)
		{
			if(SendMessage(CbTimeZone,CB_GETCURSEL,0,0)==1)
				focuswnd=tzWnd[0];
			else
			{
				if(!gOptions.IsOnlyRFMachine && gOptions.LockFunOn)
					focuswnd=BtFpMng;
				else
					focuswnd=BtSave;
			}
		}
		else if(Edtmp==tzWnd[0])
		{
			GetWindowText(tzWnd[0],tmpstr,2);
			if(strlen(tmpstr)>0)
				focuswnd=tzWnd[1];
		}
		else if(Edtmp==tzWnd[1])
		{
			GetWindowText(tzWnd[1],tmpstr,2);
			if(strlen(tmpstr)>0)
				focuswnd=tzWnd[2];
		}
		else if(Edtmp==tzWnd[2])
		{
			GetWindowText(tzWnd[2],tmpstr,2);
			if(strlen(tmpstr)>0)
			{
				if(!gOptions.IsOnlyRFMachine && gOptions.LockFunOn)
					focuswnd=BtFpMng;
				else
					focuswnd=BtSave;
			}
		}
		else if(Edtmp==BtFpMng)
			focuswnd=BtSave;
		else if(Edtmp==BtSave)
			focuswnd=BtExit;
		else if(Edtmp==BtExit)
			focuswnd=EdGroup;
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if(Edtmp==EdGroup)
		{
			GetWindowText(EdGroup,tmpstr,3);
			if(strlen(tmpstr)>0)
				focuswnd=BtExit;
		}
		else if(Edtmp==BtExit)
			focuswnd=BtSave;
		else if(Edtmp==BtSave)
		{
			if(!gOptions.IsOnlyRFMachine && gOptions.LockFunOn)
				focuswnd=BtFpMng;
			else
			{
				if(SendMessage(CbTimeZone,CB_GETCURSEL,0,0)==1)
					focuswnd=tzWnd[2];
				else
					focuswnd=CbTimeZone;
			}
		}
		else if(Edtmp==BtFpMng)
		{
			if(SendMessage(CbTimeZone,CB_GETCURSEL,0,0)==1)
				focuswnd=tzWnd[2];
			else
				focuswnd=CbTimeZone;
		}
		else if(Edtmp==tzWnd[2])
		{
			GetWindowText(tzWnd[2],tmpstr,2);
			if(strlen(tmpstr)>0)
				focuswnd=tzWnd[1];
		}
		else if(Edtmp==tzWnd[1])
		{
			GetWindowText(tzWnd[1],tmpstr,2);
			if(strlen(tmpstr)>0)
				focuswnd=tzWnd[0];
		}
		else if(Edtmp==tzWnd[0])
		{
			GetWindowText(tzWnd[0],tmpstr,2);
			if(strlen(tmpstr)>0)
				focuswnd=CbTimeZone;
		}
		else if(Edtmp==CbTimeZone)
		{
			if(gOptions.UserExtendFormat)
				focuswnd=CbVerify;
			else
				focuswnd=EdGroup;
		}
		else if(Edtmp==CbVerify)
			focuswnd=EdGroup;
	}

	SetFocusChild(focuswnd);
	return 1;
}

static void InitWindow (HWND hWnd)
{
	int i;
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=220+gOptions.ControlOffset;
		posX2=200+gOptions.ControlOffset;
		posX3=115+gOptions.ControlOffset;
		posX4=80+gOptions.ControlOffset;
		posX5=110+gOptions.ControlOffset;
		posX6=10+gOptions.ControlOffset;
		posX7=30+gOptions.ControlOffset;
	}
	else
	{
		posX1=10+gOptions.ControlOffset;
		posX2=20+gOptions.ControlOffset;
		posX3=100+gOptions.ControlOffset;
		posX4=100+gOptions.ControlOffset;
		posX5=170+gOptions.ControlOffset;
		posX6=210+gOptions.ControlOffset;
		posX7=250+gOptions.ControlOffset;
	}

	//user id	
	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"%s  %s",LoadStrByID(MID_USER_ID),tuser.PIN2);
	CreateWindow(CTRL_STATIC,tmpstr, WS_VISIBLE | SS_LEFT, LB_STATIC1,posX1,10,150,23,hWnd,0);

	//group
	CreateWindow(CTRL_STATIC,LoadStrByID(MID_USER_GP), WS_VISIBLE | SS_LEFT, LB_STATIC2,posX5,10,75,23,hWnd,0);
	EdGroup = CreateWindow(CTRL_SLEDIT,"", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
			ID_GROUP,posX7,6,60,23,hWnd,0);
	SendMessage(EdGroup,EM_LIMITTEXT,2,0L);

	//verify style
	if(gOptions.UserExtendFormat)
	{
		CreateWindow(CTRL_STATIC,LoadStrByID(MID_USER_VRTP), WS_VISIBLE | SS_LEFT, LB_STATIC3,posX1,35,80,23,hWnd,0);
		CbVerify = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
				CB_VERIFY,posX4,31,130,23,hWnd,0);
		SendMessage(CbVerify,CB_ADDSTRING,0,(LPARAM)LoadStrByID(MID_USER_GVRTP));
		for(i=0; i<VS_NUM; i++)
		{
			SendMessage(CbVerify,CB_ADDSTRING,0,(LPARAM)myVerifyStyle[i]);
		}

		//time zone
		CreateWindow(CTRL_STATIC,LoadStrByID(MID_USER_USETZ), WS_VISIBLE | SS_LEFT, LB_STATIC4,posX1,60,85,23,hWnd,0);
		CbTimeZone = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | 
				CBS_AUTOFOCUS, CB_TIMEZONE,posX4,56,130,23,hWnd,0);
		SendMessage(CbTimeZone,CB_ADDSTRING,0,(LPARAM)LoadStrByID(MID_USER_GTZ));	
		SendMessage(CbTimeZone,CB_ADDSTRING,0,(LPARAM)LoadStrByID(MID_USER_DTZ));
		for(i=0;i<3;i++)
		{
			memset(tmpstr,0,sizeof(tmpstr));
			sprintf(tmpstr,"%s%d",LoadStrByID(MID_USER_TZ),i+1);
			stzWnd[i] = CreateWindow(CTRL_STATIC,tmpstr, SS_LEFT, LB_STATIC5+i,posX2,(85+((23+2)*i)),85,23,hWnd,0);
			tzWnd[i] = CreateWindow(CTRL_SLEDIT, "", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
					ID_TZ1+i,posX4,(81+((23+2)*i)),45,23,hWnd,0);
			SendMessage(tzWnd[i],EM_LIMITTEXT,2,0L);
		}

		//duress fp
		if(!gOptions.IsOnlyRFMachine && gOptions.LockFunOn)
		{
			CreateWindow(CTRL_STATIC,LoadStrByID(MID_USER_DURFP), WS_VISIBLE | SS_LEFT, LB_STATIC8,posX1,163,75,23,hWnd,0);
			BtFpMng = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_USER_DURFPM), WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
					ID_DURFP, posX3,159,103,23,hWnd,0);
			StFpCount = CreateWindow(CTRL_STATIC,"", WS_VISIBLE | SS_LEFT, LB_STATIC8,posX6,163,100,23,hWnd,0);
		}
	}
	else
	{
		//time zone
		CreateWindow(CTRL_STATIC,LoadStrByID(MID_USER_USETZ), WS_VISIBLE | SS_LEFT, LB_STATIC4,posX1,35,85,23,hWnd,0);
		CbTimeZone = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT |
				CBS_AUTOFOCUS, CB_TIMEZONE,posX4,31,130,23,hWnd,0);
		SendMessage(CbTimeZone,CB_ADDSTRING,0,(LPARAM)LoadStrByID(MID_USER_GTZ));
		SendMessage(CbTimeZone,CB_ADDSTRING,0,(LPARAM)LoadStrByID(MID_USER_DTZ));
		for(i=0;i<3;i++)
		{
			memset(tmpstr,0,sizeof(tmpstr));
			sprintf(tmpstr,"%s%d",LoadStrByID(MID_USER_TZ),i+1);
			stzWnd[i] = CreateWindow(CTRL_STATIC,tmpstr, SS_LEFT, LB_STATIC5+i,posX2,(60+((23+2)*i)),85,23,hWnd,0);
			tzWnd[i] = CreateWindow(CTRL_SLEDIT, "", WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
					ID_TZ1+i,posX4,(56+((23+2)*i)),45,23,hWnd,0);
			SendMessage(tzWnd[i],EM_LIMITTEXT,2,0L);
		}

		//duress fp
		if(!gOptions.IsOnlyRFMachine && gOptions.LockFunOn)
		{
			CreateWindow(CTRL_STATIC,LoadStrByID(MID_USER_DURFP), WS_VISIBLE | SS_LEFT, LB_STATIC8,posX1,138,75,23,hWnd,0);
			BtFpMng = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_USER_DURFPM), WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
					ID_DURFP, posX3,134,103,23,hWnd,0);
			StFpCount = CreateWindow(CTRL_STATIC,"",WS_VISIBLE|SS_LEFT,LB_STATIC8,posX6+5,138,100,23,hWnd,0);
		}
	}

	BtSave=CreateWindow(CTRL_BUTTON,LoadStrByID(MID_SAVE),WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_SAVE,10,190,85,23,hWnd,0);
	BtExit=CreateWindow(CTRL_BUTTON,LoadStrByID(MID_EXIT),WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_EXIT,220+gOptions.ControlOffset,190,85,23,hWnd,0);
}

static void ShowDuressFp(void)
{
	int i, dtpcount=0;

	//计算胁迫指纹数量
	for(i=0;i<10;i++)
	{
		if(FDB_IsDuressTmp(userPIN,i))
			dtpcount++;
	}
	memset(tmpstr,0,sizeof(tmpstr));
	sprintf(tmpstr,"%s:%d",LoadStrByID(MID_USER_FPS),dtpcount);
	SetWindowText(StFpCount,tmpstr);
}

int GetUserVerifyType(PExtUser pu, int* vtype)
{
	BYTE tmpvs;
	tmpvs=pu->VerifyStyle;
	//	printf("tmpvs:%d\n",tmpvs);
	if(tmpvs&0x80)
	{
		*vtype = tmpvs&0x7F;
		return 1;		//use own verify style.
	}
	else
		*vtype = 0;
	return 0;	//use group verify style.
}

static int FillUserLockInfo(void)
{
	int i;
	int vstyle=0;

	//group
	memset(tmpstr,0,sizeof(tmpstr));
	//	printf("----tuser.group ddd:%d\n",tuser.Group);
	//Liaozz 20080923 fix bug:
	//while the user group is 0, user can verify.
	//actually the system use group 1 to work .just display on the UI is 0.
	if (tuser.Group == 0)
		tuser.Group = 1;
	//Liaozz end
	sprintf(tmpstr,"%d",tuser.Group);
	SetWindowText(EdGroup,tmpstr);

	//verify style
	if(gOptions.UserExtendFormat)
	{
		memset(&textuser,0,sizeof(TExtUser));
		textuser.PIN = userPIN;
		//		printf("textuser.PIN:%d\n",textuser.PIN);

		if(FDB_GetExtUser(userPIN,&textuser)!=NULL)
		{
			bNewExtUser=0;
			if(GetUserVerifyType(&textuser, &vstyle))
				gselVerify = vstyle+1;
			else
				gselVerify = 0;
		}
		else
		{
			bNewExtUser=1;
			gselVerify=0;
		}
		SendMessage(CbVerify,CB_SETCURSEL,gselVerify,0);
	}

	//time zone
	gselTimeZone = tuser.TimeZones[0];
	SendMessage(CbTimeZone,CB_SETCURSEL,gselTimeZone,0);
	if(gselTimeZone)
	{
		for(i=0;i<3;i++)
		{
			memset(tmpstr,0,sizeof(tmpstr));
			sprintf(tmpstr,"%d",tuser.TimeZones[i+1]);
			SetWindowText(tzWnd[i],tmpstr);
		}
	}

	//duress fp
	if(!gOptions.IsOnlyRFMachine && gOptions.LockFunOn)
		ShowDuressFp();
	return 1;

}

static int infoError(void)
{
	int i;
	if(gselTimeZone==1)
	{
		for(i=0;i<3;i++)
		{
			memset(tmpstr,0,5);
			GetWindowText(tzWnd[i],tmpstr,2);
			if(atoi(tmpstr)>0)
				return 0;
		}
		return 1;
	}
	return 0;
}

static int ismodified(void)
{
	int tmpverify;
	int i;
	char ttc[4];

	memset(tmpstr,0,5);
	GetWindowText(EdGroup,tmpstr,3);
	if(atoi(tmpstr)!=tuser.Group) return 1;

	if(gOptions.UserExtendFormat)
	{
		if(bNewExtUser)
		{
			if(SendMessage(CbVerify,CB_GETCURSEL,0,0)!=0)
				return 1;
		}
		else
		{
			tmpverify=SendMessage(CbVerify,CB_GETCURSEL,0,0);
			if(((textuser.VerifyStyle & 0x80) && ((textuser.VerifyStyle & 0x7F) != tmpverify-1))
					|| ((!(textuser.VerifyStyle & 0x80)) && (SendMessage(CbVerify,CB_GETCURSEL,0,0)!=0)))
				return 1;
		}
	}

	if(gselTimeZone==tuser.TimeZones[0])
	{
		if(tuser.TimeZones[0]!=0)
		{
			for(i=0;i<3;i++)
			{
				memset(ttc, 0, 4);
				GetWindowText(tzWnd[i],ttc,2);
				if(atoi(ttc)!=tuser.TimeZones[i+1])
					return 1;
			}			
		}
	}
	else
		return 1;

	return 0;
}

static int SaveUserLockInfo(HWND hWnd)
{
	int tmpval;
	int i;
	char mystr[50];

	//get group info
	memset(&tgroup,0,sizeof(TGroup));
	memset(tmpstr,0,5);
	GetWindowText(EdGroup,tmpstr,3);
	if(strlen(tmpstr)>0)
	{
		tmpval = atoi(tmpstr);
		if(tmpval)
		{
			if(FDB_GetGroup(tmpval,&tgroup)==NULL)
			{
				memset(mystr,0,50);
				sprintf(mystr,"%s%d%s",LoadStrByID(MID_GROUP),tmpval,LoadStrByID(MID_SAVE_HINT15));
				if(MessageBox1(hWnd,mystr, LoadStrByID(MID_APPNAME),MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK)
					CreateGroupOptWindow(hWnd,0,&tmpval);
				return 0;
			}
			tuser.Group=tmpval;
		}
		else
		{
			MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT7), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONSTOP | MB_BASEDONPARENT);
			if(!ismenutimeout) SetFocusChild(EdGroup);
			return 0;
		}
	}
	else
		return 0;

	if(gOptions.UserExtendFormat)
	{
		if(gselVerify==0)		//use group verify style.
			textuser.VerifyStyle = tgroup.VerifyStyle & 0x7F;
		else
			textuser.VerifyStyle = (gselVerify-1) | 0x80;
	}
	//	printf("textuser.VerifyStyle:%d\n",textuser.VerifyStyle);

	if(gselTimeZone==0)	//group time zone
		memset(tuser.TimeZones,0,8);
	else		//user define time zone
	{
		tuser.TimeZones[0]=1;
		for(i=0;i<3;i++)
		{
			memset(tmpstr,0,5);
			GetWindowText(tzWnd[i],tmpstr,3);
			tmpval=atoi(tmpstr);
			if (tmpval==0 || FDB_GetTimeZone(tmpval, NULL)!=NULL)
				tuser.TimeZones[i+1]=tmpval;
			else	//time zone not exsit.
			{
				if(MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT2), LoadStrByID(MID_APPNAME),
							MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
					TimeZoneMngWindow(hWnd,tmpval,1);		//add new time zone
				return 0;
			}
		}

	}

	if(FDB_ChgUser(&tuser)==FDB_OK)
	{
		if(gOptions.UserExtendFormat)
		{
			if(bNewExtUser)
			{
				if(FDB_AddExtUser(&textuser)!=FDB_OK)
					return 0;
			}
			else
			{
				if(FDB_ChgExtUser(&textuser)!=FDB_OK)
					return 0;
			}
		}
		sync();
	}
	else
		return 0;

	return 1;
}

static void freshTZBox(void)
{
	int i;
	for(i=0;i<3;i++)
	{
		//		printf("tuser.TimeZones[i+1]:%d\n",tuser.TimeZones[i+1]);
		memset(tmpstr,0,sizeof(tmpstr));
		sprintf(tmpstr,"%d",tuser.TimeZones[i+1]);
		SetWindowText(tzWnd[i],tmpstr);
		ShowWindow(tzWnd[i],(gselTimeZone)?SW_SHOW:SW_HIDE);
		ShowWindow(stzWnd[i],(gselTimeZone)?SW_SHOW:SW_HIDE);
	}
}

static int userlockwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	int i;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			ShowWindow(hWnd,FALSE);
			//if (LoadBitmap(HDC_SCREEN,&userlockbk,GetBmpPath("submenubg.jpg")))
			//         return 0;

			InitWindow(hWnd);		//add controls
			FillUserLockInfo();
			freshTZBox();
			UpdateWindow(hWnd,TRUE);
			SetFocusChild(EdGroup);

			ShowWindow(hWnd,TRUE);
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

			Edtmp = GetFocusChild(hWnd);

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
				PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0L);
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				SelectNext(hWnd,wParam);
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT)
			{
				if(Edtmp==CbVerify)
				{
					if(--gselVerify<0) gselVerify=15;
					//Liaozz 20080926 fix bug second 37 user lock mng left
					//					printf("gselverify:%d====before=======\n", gselVerify);
					if (gOptions.IsOnlyRFMachine) {
						if (gselVerify == 15 || gselVerify == 14 || gselVerify == 13)
							gselVerify = 12;
						if (gselVerify == 11 || gselVerify == 10 || gselVerify == 9)
							gselVerify = 8;
						if (gselVerify == 7 || gselVerify == 6)
							gselVerify = 5;
						if (gselVerify == 2 || gselVerify == 1)
							gselVerify = 0;
					}
					//					printf("gselverify:%d====after=======\n", gselVerify);
					//Liaozz end
					SendMessage(CbVerify,CB_SETCURSEL,gselVerify,0);
					return 0;
				}
				else if(Edtmp==CbTimeZone)
				{
					if(--gselTimeZone<0) gselTimeZone=1;
					SendMessage(CbTimeZone,CB_SETCURSEL,gselTimeZone,0);
					freshTZBox();
					return 0;
				}
				else if(Edtmp==BtExit)
				{
					SetFocusChild(BtSave);
					return 0;
				}
			}
			else if (LOWORD(wParam) == SCANCODE_CURSORBLOCKRIGHT || (LOWORD(wParam) == SCANCODE_BACKSPACE && gOptions.TFTKeyLayout == 3))
			{
				if(Edtmp==CbVerify)
				{
					if(++gselVerify>15) gselVerify=0;
					//Liaozz 20080926 fix bug second 37 user lock mng right
					//						printf("gselverify:%d====before====right===\n", gselVerify);
					if (gOptions.IsOnlyRFMachine) {
						if (gselVerify == 1 || gselVerify == 2)
							gselVerify = 3;
						if (gselVerify == 6 || gselVerify == 7)
							gselVerify = 8;
						if (gselVerify == 9 || gselVerify == 10 || gselVerify == 11)
							gselVerify = 12;
						if (gselVerify == 13 || gselVerify == 14 || gselVerify == 15)
							gselVerify = 0;
					}
					//						printf("gselverify:%d====after=====right==\n", gselVerify);
					//Liaozz end
					SendMessage(CbVerify,CB_SETCURSEL,gselVerify,0);
					return 0;
				}
				else if(Edtmp==CbTimeZone)
				{ 
					if(++gselTimeZone>1) gselTimeZone=0;
					SendMessage(CbTimeZone,CB_SETCURSEL,gselTimeZone,0);
					freshTZBox();
					return 0;
				}
				else if(Edtmp==BtSave)
				{
					SetFocusChild(BtExit);
					return 0;
				}
			}
			else if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10)) 
			{
				if(Edtmp==BtExit)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0L);
				else if(Edtmp==BtFpMng)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_DURFP,0L);
				else if(Edtmp!=BtSave)
					PostMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0L);
			}
			else if(LOWORD(wParam)==SCANCODE_MENU)
			{
				PostMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
			}

			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(nc==EN_CHANGE)
			{
				for(i=0;i<3;i++)
				{
					if(tzWnd[i]==id)
					{
						memset(tmpstr,0,5);
						GetWindowText(tzWnd[i],tmpstr,2);
						if(atoi(tmpstr)>TZ_MAX)
						{
							memset(tmpstr,0,5);
							sprintf(tmpstr,"%d",tuser.TimeZones[i+1]);
							SetWindowText(tzWnd[i],tmpstr);
							SendMessage(tzWnd[i],EM_SETCARETPOS,0,strlen(tmpstr));
						}
					}
				}
			}

			if(id==ID_SAVE)
			{
				if(infoError())
				{
					if(MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT6), LoadStrByID(MID_APPNAME),
								MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
					{
						gselTimeZone=0;
						SendMessage(CbTimeZone,CB_SETCURSEL,gselTimeZone,0);
						freshTZBox();
					}
					else
						break;
				}
				if(SaveUserLockInfo(hWnd))
				{
					PostMessage(hWnd,MSG_CLOSE,0,0L);
				}
			}
			else if(id==ID_EXIT)
			{
				if(ismodified() && MessageBox1 (hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
							MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT)==IDOK)
					PostMessage(hWnd,MSG_COMMAND,ID_SAVE,0L);

				if(!ismenutimeout) PostMessage(hWnd,MSG_CLOSE,0,0L);
			}
			else if(id==ID_DURFP)
			{
				DuressFpWindow(hWnd, userPIN);
				if(ismenutimeout) return 0;
				ShowDuressFp();	
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&userlockbk);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);

}

int CreateLockWindow(HWND hWnd, int *userid)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	userPIN = *userid;
	//	printf("userPIN:%d\n",userPIN);

	memset(&tuser,0,sizeof(TUser));
	if(FDB_GetUser(userPIN,&tuser)==NULL) return 0;

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_USER_DOOR);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = userlockwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
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

