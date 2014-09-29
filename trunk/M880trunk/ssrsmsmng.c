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
#include <minigui/tftmullan.h>

#define LD_TEXT 50
#define LB_TIME 51
#define LB_MIN 	52
#define LB_TYPE 53
#define LD_USER 54
#define LB_1	55
#define LB_2	56
#define LB_3	57

#define ID_YEAR 	58
#define ID_MONTH 	59
#define ID_DAY 		60
#define ID_HOUR 	61
#define ID_MIN 		62
#define ID_MINLEN 	63
#define ID_SEND 	64
#define ID_SAVE 	65
#define ID_EXIT 	66

#define LB_YEAR 	67
#define LB_MON 		68
#define LB_DAY 		69
#define LB_HOUR 	70
#define LB_MINUTE 	71
#define LB_MINLEN 	72

#define CB_TYPE 	75

HWND EdText,LbTime,LbMin,LbType,LtUser;
HWND EdYear,EdMonth,EdDay,EdHour,EdMin,EdMinlen,CbType;
HWND BtSend,BtSave,BtExit;
HWND Edtmp,focuswnd;

PUDataLink pudlk=NULL;
int bsended = 0;	//if sended
int sendchgflag = 0;

TSms tmpsms_mng;
TTime tmptime;
int tmpminlen;
int tmptype;

int bChanged_sms;
int OptMod_sms;
int smstmppin1=0;			//sms id
int g_seltype;
//BITMAP smsmngbk;
int personalFlag= 0;//Liaozz 20081006 fix bug second 49

extern HWND hIMEWnd;

char* smsHint1[12];
static void LoadHint(void)
{
	smsHint1[0]=LoadStrByID(MID_YEAR);
	smsHint1[1]=LoadStrByID(MID_MONTH);
	smsHint1[2]=LoadStrByID(MID_DAY1);
	smsHint1[3]=LoadStrByID(MID_HOUR1);
	smsHint1[4]=LoadStrByID(MID_MINUTE);
	smsHint1[5]=LoadStrByID(MID_SMSMINUTE);
	smsHint1[6]=LoadStrByID(MID_SMSSEND);
	smsHint1[7]=LoadStrByID(MID_SAVE);
	smsHint1[8]=LoadStrByID(MID_EXIT);
	smsHint1[9]=LoadStrByID(MID_SMSPRIVATE);
	smsHint1[10]=LoadStrByID(MID_SMSCOMMON);
	smsHint1[11]=LoadStrByID(MID_SMSRESERVED);

}

static int  SelectNext(HWND hWnd,WPARAM wParam)
{
	Edtmp  = GetFocusChild(hWnd);
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == EdText)
		{
			if(hIMEWnd!=HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd=HWND_INVALID;
			}
			focuswnd = EdYear;
		}
		else if (Edtmp == EdYear)
			focuswnd = EdMonth;
		else if (Edtmp == EdMonth)
			focuswnd = EdDay;
		else if (Edtmp == EdDay)
			focuswnd = EdHour;
		else if (Edtmp == EdHour)
			focuswnd = EdMin;
		else if (Edtmp == EdMin)
			focuswnd = EdMinlen;
		else if (Edtmp == EdMinlen)
			focuswnd = CbType;
		else if (Edtmp == CbType)
			focuswnd = BtSave;
		else if(Edtmp == BtSave)
			if(SendMessage(CbType,CB_GETCURSEL,0,0)==0)
				focuswnd = BtSend;
			else
				focuswnd = BtExit;
		else if(Edtmp == BtSend)
			focuswnd = BtExit;
		else if(Edtmp == BtExit)
			focuswnd = EdText;
	}

	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == EdText)
		{
			if(hIMEWnd != HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd=HWND_INVALID;
			}
			focuswnd = BtExit;
		}
		else if (Edtmp == BtExit)
			if(SendMessage(CbType,CB_GETCURSEL,0,0)==0)
				focuswnd = BtSend;
			else	
				focuswnd = BtSave;
		else if (Edtmp == BtSave)
			focuswnd = CbType;
		else if (Edtmp == BtSend)
			focuswnd = BtSave;
		else if (Edtmp == CbType)
			focuswnd = EdMinlen;
		else if (Edtmp == EdMinlen)
			focuswnd = EdMin;
		else if (Edtmp == EdMin)
			focuswnd = EdHour;
		else if (Edtmp == EdHour)
			focuswnd = EdDay;
		else if(Edtmp == EdDay)
			focuswnd = EdMonth;
		else if(Edtmp == EdMonth)
			focuswnd = EdYear;
		else if(Edtmp == EdYear)
			focuswnd = EdText;
	}

	SetFocusChild(focuswnd);
	return 1;
}

extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
extern TTime * HejiraToChrisTTime(TTime *hejiratm);
static void InitWindow (HWND hWnd)
{
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9,posX10,posX11,posX12,posX13,posX14,posX15,posX16;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=5;
		posX2=260;
		posX3=180;
		posX4=180;
		posX5=145;
		posX6=132;
		posX7=105;
		posX8=95;
		posX9=65;
		posX10=50;
		posX11=20;
		posX12=10;
		posX13=120;
		posX14=32;
		posX15=20;
		posX16=30;
	}
	else
	{
		posX1=5;
		posX2=10;
		posX3=80;
		posX4=122;
		posX5=139;
		posX6=166;
		posX7=183;
		posX8=210;
		posX9=227;
		posX10=254;
		posX11=271;
		posX12=298;
		posX13=142;
		//posX14=10;
		posX14=3;
		posX15=0;
		posX16=0;
	}

	if(gOptions.TFTKeyLayout==3)
	{
		EdText=CreateWindow (CTRL_MLEDIT, "", WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_AUTOWRAP,
				LD_TEXT, 5, 5, 385, 100, hWnd, 0);
	}
	else
	{
		EdText=CreateWindow (CTRL_MLEDIT, "", WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_AUTOWRAP,
				LD_TEXT, 5, 5, 308, 100, hWnd, 0);
	}
	SendMessage(EdText,EM_LIMITTEXT,MAX_SMS_CONTENT_SIZE*2,0L);

	CreateWindow(CTRL_STATIC, LoadStrByID(MID_SMSSTARTTIME),WS_VISIBLE | SS_LEFT,LB_1, posX2, 112, 75, 23, hWnd, 0);
	CreateWindow(CTRL_STATIC, LoadStrByID(MID_SMSTIMELEN),WS_VISIBLE | SS_LEFT,LB_2, posX2, 138, 75, 23, hWnd, 0);
	CreateWindow(CTRL_STATIC, LoadStrByID(MID_SMSTYPE1),WS_VISIBLE | SS_LEFT,LB_3, posX2, 164, 75, 23, hWnd, 0);

	switch(OptMod_sms)
	{
		case 0:
			LbTime=CreateWindow(CTRL_STATIC, "",WS_VISIBLE | SS_LEFT,LB_TIME, posX3-posX16, 112, 160, 23, hWnd, 0);
			LbMin=CreateWindow(CTRL_STATIC, "",WS_VISIBLE | SS_LEFT,LB_MIN, posX3-posX16, 138, 75, 23, hWnd, 0);
			LbType=CreateWindow(CTRL_STATIC, "",WS_VISIBLE | SS_LEFT,LB_TYPE, posX3-posX16, 164, 80, 23, hWnd, 0);
			break;

		case 1:
		case 2:
			EdYear=CreateWindow(CTRL_SLEDIT, "",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
					ID_YEAR, posX3+posX15, 108, 40, 23, hWnd, 0);
			CreateWindow(CTRL_STATIC, smsHint1[0],WS_VISIBLE | SS_LEFT,LB_YEAR, posX4, 112, 15, 23, hWnd, 0);

			EdMonth=CreateWindow(CTRL_SLEDIT, "",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
					ID_MONTH, posX5, 108, 25, 23, hWnd, 0);
			CreateWindow(CTRL_STATIC, smsHint1[1],WS_VISIBLE | SS_LEFT,LB_MON, posX6, 112, 15, 23, hWnd, 0);

			EdDay=CreateWindow(CTRL_SLEDIT, "",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
					ID_DAY, posX7, 108, 25, 23, hWnd, 0);
			CreateWindow(CTRL_STATIC, smsHint1[2],WS_VISIBLE | SS_LEFT,LB_DAY, posX8, 112, 15, 23, hWnd, 0);

			EdHour=CreateWindow(CTRL_SLEDIT, "",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
					ID_HOUR, posX9, 108, 25, 23, hWnd, 0);
			CreateWindow(CTRL_STATIC, smsHint1[3],WS_VISIBLE | SS_LEFT,
					LB_HOUR, posX10, 112, 15, 23, hWnd, 0);

			EdMin=CreateWindow(CTRL_SLEDIT, "",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
					ID_MIN, posX11, 108, 25, 23, hWnd, 0);
			CreateWindow(CTRL_STATIC, smsHint1[4],WS_VISIBLE | SS_LEFT,LB_MINUTE, posX12, 112, 15, 23, hWnd, 0);

			EdMinlen=CreateWindow(CTRL_SLEDIT, "",WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,
					ID_MINLEN, posX3, 134, 60, 23, hWnd, 0);
			CreateWindow(CTRL_STATIC, smsHint1[5], WS_VISIBLE | SS_LEFT, LB_MINLEN, posX13, 138, 60, 23, hWnd, 0);	

			if(gOptions.TFTKeyLayout == 3)
			{
				CbType=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
						CB_TYPE, posX3-posX15, 160, 100, 23, hWnd, 0);
			}
			else				
			{
				CbType=CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
						CB_TYPE, posX3-posX14, 160, 100, 23, hWnd, 0);
			}

			BtSend= CreateWindow(CTRL_BUTTON, smsHint1[6],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
					ID_SEND,115,190,85,23,hWnd,0);
			BtSave = CreateWindow(CTRL_BUTTON, smsHint1[7],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
					ID_SAVE,10,190,85,23,hWnd,0);
			BtExit = CreateWindow(CTRL_BUTTON, smsHint1[8],WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
					ID_EXIT,220,190,85,23,hWnd,0);

			SendMessage(EdYear,EM_LIMITTEXT,4,0L);
			SendMessage(EdMonth,EM_LIMITTEXT,2,0L);
			SendMessage(EdDay,EM_LIMITTEXT,2,0L);
			SendMessage(EdHour,EM_LIMITTEXT,2,0L);
			SendMessage(EdMin,EM_LIMITTEXT,2,0L);
			SendMessage(EdMinlen,EM_LIMITTEXT,5,0L);
			break;
	}

}

extern int UTF8toLocal(int lid,const unsigned char *utf8, unsigned char *str);
static int ismodified()
{
	char tmpstr[MAX_SMS_CONTENT_SIZE*2+1];
	char tmpstcode[MAX_SMS_CONTENT_SIZE*2+1];
	memset(tmpstcode,0,MAX_SMS_CONTENT_SIZE*2+1);         //modify by jazzy 2008.12.15 for sms content name

	memset(&tmptime,0,sizeof(TTime));
	OldDecodeTime(tmpsms_mng.StartTime, &tmptime);

	memset(tmpstr,0,MAX_SMS_CONTENT_SIZE*2+1);
	//GetWindowText(EdText,tmpstr,MAX_SMS_CONTENT_SIZE*2+1);
	GetWindowText(EdText,tmpstcode,MAX_SMS_CONTENT_SIZE*2+1);
	UTF8toLocal(tftlocaleid,(unsigned char*)tmpstcode,(unsigned char*)tmpstr);

	if((OptMod_sms==2 && strlen(tmpstr)>0) || (OptMod_sms==1 && strncmp(tmpstr,(char*)tmpsms_mng.Content,MAX_SMS_CONTENT_SIZE*2)!=0)) {
		return 1;
	}

	if(OptMod_sms==1)
	{
		memset(tmpstr,0,10);
		GetWindowText(EdYear,tmpstr,4);
		if((atoi(tmpstr)-1900) != tmptime.tm_year) {
			return 1;
		}

		memset(tmpstr,0,10);
		GetWindowText(EdMonth,tmpstr,2);
		if((atoi(tmpstr)-1) != tmptime.tm_mon) {
			return 1;
		}

		memset(tmpstr,0,10);
		GetWindowText(EdDay,tmpstr,2);
		if(atoi(tmpstr) != tmptime.tm_mday) {
			return 1;
		}

		memset(tmpstr,0,10);
		GetWindowText(EdHour,tmpstr,2);
		if(atoi(tmpstr) != tmptime.tm_hour) {
			return 1;
		}

		memset(tmpstr,0,10);
		GetWindowText(EdMin,tmpstr,2);
		if(atoi(tmpstr) != tmptime.tm_min) {
			return 1;
		}

		memset(tmpstr,0,10);
		GetWindowText(EdMinlen,tmpstr,10);
		if(atoi(tmpstr) != tmpsms_mng.ValidMinutes) {
			return 1;
		}

		if(SendMessage(CbType,CB_GETCURSEL,0,0)!=tmptype) {
			return 1;
		}
		if(sendchgflag) {
			//printf("______%s%d\n", __FILE__, __LINE__);
			return 1;
		}
	}
	return 0;
}

static int SaveSmsInfo(HWND hWnd)
{
	TSms tsms;
	char tmpc[5];
	TTime tt;
	time_t tmptime1;
	char contentbuf[MAX_SMS_CONTENT_SIZE*2+1];
	int minutelen;
	int tag =0;
	char tmpstcode[MAX_SMS_CONTENT_SIZE*2+1];
	memset(tmpstcode,0,MAX_SMS_CONTENT_SIZE*2+1);         //modify by jazzy 2008.12.15 for sms content name

	memset(contentbuf,0,MAX_SMS_CONTENT_SIZE*2+1);
	GetWindowText(EdText,contentbuf,MAX_SMS_CONTENT_SIZE*2);
	UTF8toLocal(tftlocaleid,(unsigned char *)contentbuf,(unsigned char *)tmpstcode);
	if(strlen(contentbuf)==0)
	{
		MessageBox1(hWnd,LoadStrByID(MID_SMSCONTENTEMPTY), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
		if(!ismenutimeout) SetFocusChild(EdText);
		return 0;
	}

	memset(tmpc,0,5);
	GetWindowText(EdMinlen,tmpc,5);
	minutelen = atoi(tmpc);
	if(minutelen<0)
	{
		MessageBox1(hWnd,LoadStrByID(MID_SMSTIMEHINT), LoadStrByID(MID_APPNAME),MB_OK | MB_ICONQUESTION | MB_BASEDONPARENT);
		return 0;
	}

	memset(tmpc,0,4);
	GetWindowText(EdYear,tmpc,4);

	//zsliu add 
	if(gOptions.isUseHejiraCalendar)
	{
		//HejiraToChrisTTime(&gCurTime);  //ä¼Šæœ—æ—¥åŽ†è½¬å…¬åŽ†
		//gCurTime.tm_year -= 1900;
		//gCurTime.tm_mon -= 1;
		if(atoi(tmpc) < 1380 || atoi(tmpc) > 1408)
		{
			MessageBox1 (hWnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
			return 0;
		}

	}
	else
	{
		if(atoi(tmpc)<2000 || atoi(tmpc)>2037)
		{
			MessageBox1 (hWnd ,LoadStrByID(HIT_ERROR0) ,LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
			return 0;
		}

	}
	//zsliu add end ... ...

	GetTime(&tt);
	tt.tm_year=atoi(tmpc)-1900;
	memset(tmpc,0,2);
	GetWindowText(EdMonth,tmpc,2);
	tt.tm_mon=atoi(tmpc)-1;
	memset(tmpc,0,2);
	GetWindowText(EdDay,tmpc,2);
	tt.tm_mday=atoi(tmpc);
	memset(tmpc,0,2);
	GetWindowText(EdHour,tmpc,2);
	tt.tm_hour=atoi(tmpc);
	memset(tmpc,0,2);
	GetWindowText(EdMin,tmpc,2);
	tt.tm_min=atoi(tmpc);
	tt.tm_sec=0;

	//zsliu add 
	if(gOptions.isUseHejiraCalendar)
	{
		//DBPRINTF("SaveSMSInfo() - 1 date: %d-%d-%d\n", tt.tm_year, tt.tm_mon, tt.tm_mday);
		//tt.tm_mon += 1;
		HejiraToChrisTTime(&tt);  //ä¼Šæœ—æ—¥åŽ†è½¬å…¬åŽ†
		//tt.tm_year -= 1900;
		//tt.tm_mon -= 1;
		//DBPRINTF("SaveSMSInfo() - 2 date: %d-%d-%d\n", tt.tm_year, tt.tm_mon, tt.tm_mday);
	}
	//zsliu add end ... ...

	tmptime1 = OldEncodeTime(&tt);

	if(SendMessage(CbType,CB_GETCURSEL,0,0)==0) tag=UDATA_TAG_USERSMS;
	else if(SendMessage(CbType,CB_GETCURSEL,0,0)==1) tag=UDATA_TAG_ALL;
	else if(SendMessage(CbType,CB_GETCURSEL,0,0)==2) tag=UDATA_TAG_TEMP;

	if(tag==UDATA_TAG_ALL || tag==UDATA_TAG_TEMP)
	{
		if(FDB_DelUData(0,smstmppin1)!=FDB_OK)
			return 0;
	}
	//Liaozz 20081006 fix bug second 49
	//	if (tag==UDATA_TAG_USERSMS && bsended==0)
	if (tag==UDATA_TAG_USERSMS && personalFlag == 1)
		//Liaozz end
	{
		MessageBox1(hWnd, LoadStrByID(HIT_ERR_SELUSER), LoadStrByID(HIT_ERR),MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	if(FDB_CreateSms(&tsms, tag, smstmppin1, (unsigned char*)tmpstcode, minutelen, tmptime1)!=NULL)
	{
		if(OptMod_sms==1)
		{
			if(FDB_ChgSms(&tsms)!=FDB_OK) return 0;
		}
		else if(OptMod_sms==2)
		{
			if(FDB_AddSms(&tsms)!=FDB_OK) return 0;
		}	
	}
	else
		return 0;

	if(pudlk!=NULL && tag==UDATA_TAG_USERSMS)
	{
		U16 usridx;
		TUData udata;
		PUDataLink tmpnode=pudlk;

		while(tmpnode->nextnode!=NULL)
		{
			tmpnode=tmpnode->nextnode;
			//			printf("tmpnode->PIN:%d\n",tmpnode->PIN);
			usridx=tmpnode->PIN;
			if(tmpnode->bSelect==1)
			{
				if(FDB_GetUData(usridx,&udata)==NULL)
				{
					FDB_CreateUData(&udata,usridx,smstmppin1);
					//					printf("FDB_CreateUData PIN:%d,SMSID:%d\n",usridx,smstmppin1);
					if(FDB_AddUData(&udata)!=FDB_OK) return 0;
				}
			}
			else
			{
				if(FDB_GetUData(usridx,&udata)!=NULL)
				{
					if(FDB_DelUData(udata.PIN,udata.SmsID)!=FDB_OK)
						return 0;
				}
			}
		}
	}
	sync();
	return 1;
}

extern TTime * ChrisToHejiraTTime(TTime *christm);
static void FillSmsData(int OptMod)
{
	char tmpstr[MAX_SMS_CONTENT_SIZE*2+1];
	//U16 usrid[100];	
	//int i,usrcount;

	time_t local_time ;
	struct tm * now;

	switch(OptMod)
	{
		case 0:		//check
			memset(tmpstr,0,MAX_SMS_CONTENT_SIZE*2+1);

			Str2UTF8(tftlocaleid,tmpsms_mng.Content, (unsigned char*)tmpstr);	//modify by jazzy 2008.07.26
			//memcpy(tmpstr,tmpsms_mng.Content,strlen(tmpsms_mng.Content));

			SetWindowText(EdText,tmpstr);

			memset(tmpstr,0,20);
			memset(&tmptime,0,sizeof(TTime));
			OldDecodeTime(tmpsms_mng.StartTime, &tmptime);

			//zsliu add 
			if(gOptions.isUseHejiraCalendar)
			{
				//DBPRINTF("FillSmsData() - 1 date: %d-%d-%d\n", tmptime.tm_year, tmptime.tm_mon, tmptime.tm_mday);
				ChrisToHejiraTTime(&tmptime); //å…¬åŽ†è½¬ä¼Šæœ—æ—¥åŽ†
				//tmptime.tm_mon -= 1;
				//DBPRINTF("FillSmsData() - 2 date: %d-%d-%d\n", tmptime.tm_year, tmptime.tm_mon, tmptime.tm_mday);
			}
			//zsliu add end ... ...

			sprintf(tmpstr,"%d.%02d.%02d %02d:%02d",tmptime.tm_year+1900,tmptime.tm_mon+1,tmptime.tm_mday,tmptime.tm_hour,tmptime.tm_min);

			SetWindowText(LbTime,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%d%s",tmpsms_mng.ValidMinutes,smsHint1[5]);
			SetWindowText(LbMin,tmpstr);

			memset(tmpstr,0,10);
			if(tmpsms_mng.Tag==UDATA_TAG_USERSMS)
				sprintf(tmpstr,"%s",smsHint1[9]);
			else if(tmpsms_mng.Tag==UDATA_TAG_ALL)
				sprintf(tmpstr,"%s",smsHint1[10]);
			else if(tmpsms_mng.Tag==UDATA_TAG_TEMP)
				sprintf(tmpstr,"%s",smsHint1[11]);
			SetWindowText(LbType,tmpstr);
			SendMessage(EdText,EM_SETREADONLY,1,0);						
			break;

		case 1:
			memset(tmpstr,0,MAX_SMS_CONTENT_SIZE*2+1);

			Str2UTF8(tftlocaleid,tmpsms_mng.Content, (unsigned char*)tmpstr);	//modify by jazzy 2008.07.26
			//memcpy(tmpstr,tmpsms_mng.Content,strlen(tmpsms_mng.Content));

			SetWindowText(EdText,tmpstr);
			if(gOptions.TFTKeyLayout==3)
			{
				SendMessage (EdText, EM_SELECTALL, 0, 0);
			}
			else
			{
				SendMessage(EdText,EM_SETCARETPOS,0,strlen(tmpstr));
			}

			memset(&tmptime,0,sizeof(TTime));
			OldDecodeTime(tmpsms_mng.StartTime, &tmptime);
			//zsliu add 
			if(gOptions.isUseHejiraCalendar)
			{
				//DBPRINTF("FillSmsData() - 3 date: %d-%d-%d\n", tmptime.tm_year, tmptime.tm_mon, tmptime.tm_mday);
				ChrisToHejiraTTime(&tmptime); //å…¬åŽ†è½¬ä¼Šæœ—æ—¥åŽ†
				//tmptime.tm_mon -= 1;
				//DBPRINTF("FillSmsData() - 4 date: %d-%d-%d\n", tmptime.tm_year, tmptime.tm_mon, tmptime.tm_mday);
			}
			//zsliu add end ... ...

			memset(tmpstr,0,10);
			sprintf(tmpstr,"%d",tmptime.tm_year+1900);
			SetWindowText(EdYear,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%02d",tmptime.tm_mon+1);
			SetWindowText(EdMonth,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%02d",tmptime.tm_mday);
			SetWindowText(EdDay,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%02d",tmptime.tm_hour);
			SetWindowText(EdHour,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%02d",tmptime.tm_min);
			SetWindowText(EdMin,tmpstr);

			memset(tmpstr,0,10);
			sprintf(tmpstr,"%d",tmpsms_mng.ValidMinutes);
			SetWindowText(EdMinlen,tmpstr);

			SendMessage(CbType,CB_ADDSTRING,0,(LPARAM)smsHint1[9]);
			SendMessage(CbType,CB_ADDSTRING,0,(LPARAM)smsHint1[10]);
			SendMessage(CbType,CB_ADDSTRING,0,(LPARAM)smsHint1[11]);
			if(tmpsms_mng.Tag==UDATA_TAG_USERSMS)
			{
				tmptype = 0;
				//personalFlag = 1;//Liaozz 20081006 fix bug second 49
				personalFlag = 0;
				SendMessage(BtSend,MSG_ENABLE,1,0);
				bsended=1;
			}
			else if(tmpsms_mng.Tag==UDATA_TAG_ALL)
			{
				tmptype = 1;
				SendMessage(BtSend,MSG_ENABLE,0,0);
			}
			else if(tmpsms_mng.Tag==UDATA_TAG_TEMP)
			{
				tmptype = 2;
				SendMessage(BtSend,MSG_ENABLE,0,0);
			}
			SendMessage(CbType,CB_SETCURSEL,tmptype,0);
			SendMessage(EdText,EM_SETREADONLY,0,0);						
			break;

		case 2:
			SetWindowText(EdText,"");
			local_time = time(0);
			now = localtime(&local_time);
			//zsliu add 
			if(gOptions.isUseHejiraCalendar)
			{
				//DBPRINTF("FillSmsData() - 5 date: %d-%d-%d\n", now->tm_year, now->tm_mon, now->tm_mday);
				ChrisToHejiraTTime(now); //å…¬åŽ†è½¬ä¼Šæœ—æ—¥åŽ†
				//now->tm_mon -= 1;
				//DBPRINTF("FillSmsData() - 6 date: %d-%d-%d\n", now->tm_year, now->tm_mon, now->tm_mday);
			}
			//zsliu add end ... ...

			memset(tmpstr,0,10);
			sprintf(tmpstr,"%d",now->tm_year+1900);
			SetWindowText(EdYear,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%02d",now->tm_mon+1);
			SetWindowText(EdMonth,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%02d",now->tm_mday);
			SetWindowText(EdDay,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%02d",now->tm_hour);
			SetWindowText(EdHour,tmpstr);
			memset(tmpstr,0,10);
			sprintf(tmpstr,"%02d",now->tm_min);
			SetWindowText(EdMin,tmpstr);

			memset(tmpstr,0,10);
			sprintf(tmpstr,"%d",60);
			SetWindowText(EdMinlen,tmpstr);

			SendMessage(CbType,CB_ADDSTRING,0,(LPARAM)smsHint1[9]);
			SendMessage(CbType,CB_ADDSTRING,0,(LPARAM)smsHint1[10]);
			SendMessage(CbType,CB_ADDSTRING,0,(LPARAM)smsHint1[11]);
			g_seltype=2;
			SendMessage(CbType,CB_SETCURSEL,g_seltype,0);
			SendMessage(BtSend,MSG_ENABLE,0,0);
			SendMessage(EdText,EM_SETREADONLY,0,0);	

			break;
	}
}

static void freeLinkListBuf()
{
	PUDataLink tmplink;
	if(pudlk!=NULL)
	{
		tmplink = pudlk;
		while (tmplink != NULL)
		{
			pudlk = tmplink;
			tmplink=tmplink->nextnode;
			FREE(pudlk);
		}
		pudlk=NULL;
	}
}

static int smswinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	char sstr[20];
	int tmpvalue;
	char tmpchar1[MAX_SMS_CONTENT_SIZE*2+1];
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			InitWindow(hWnd);		//add controls
			FillSmsData(OptMod_sms);
			UpdateWindow(hWnd,TRUE);
			SetFocusChild(EdText);
			Edtmp = GetFocusChild(hWnd);
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

				FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
				if(fGetDC) ReleaseDC (hdc);
				break;
			}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);	
			if(tmpsms_mng.Tag==UDATA_TAG_USERSMS && OptMod_sms==0)
			{
				SetTextColor(hdc,PIXEL_yellow);
				tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
				TextOut(hdc,160,195,LoadStrByID(MID_SMSCHECKHINT));
			}
			EndPaint(hWnd,hdc);
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
			if (gOptions.KeyPadBeep) ExKeyBeep();

			Edtmp = GetFocusChild(hWnd);
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				if(OptMod_sms!=0)
					SendMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0L);
				else 
					SendMessage(hWnd,MSG_CLOSE,0,0L);
				return 0;				
			}

			if(LOWORD(wParam)==SCANCODE_F11)
			{
				if(Edtmp==EdText && OptMod_sms && gOptions.IMEFunOn && 
						(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4))
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
				else
					SendMessage(Edtmp,MSG_KEYDOWN,SCANCODE_PAGEUP,lParam);
				break;
			}

			if(LOWORD(wParam)==SCANCODE_F9 || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				if((Edtmp==EdText) && (OptMod_sms!=0) && (gOptions.IMEFunOn==1))
				{
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
					break;
				}
			}

#if 0
			if (LOWORD(wParam) == gOptions.IMESwitchKey)
			{
				if(LOWORD(wParam)==SCANCODE_F11)
				{
					if((Edtmp==EdText) && (OptMod_sms!=0) && (gOptions.IMEFunOn==1) && (gOptions.TFTKeyLayout==0))
						T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
					else	
						SendMessage(Edtmp,MSG_KEYDOWN,SCANCODE_PAGEUP,lParam);
				}
				else if(LOWORD(wParam)==SCANCODE_F9)
				{
					if((Edtmp==EdText) && (OptMod_sms!=0) && (gOptions.IMEFunOn==1))
						T9IMEWindow(hWnd, 0, 200, gOptions.LCDWidth, gOptions.LCDHeight, gOptions.HzImeOn);
				}
				break;
			}
#endif
			if(LOWORD(wParam)==SCANCODE_F12)
			{
				SendMessage(Edtmp,MSG_KEYDOWN,SCANCODE_PAGEDOWN,lParam);
				break;	
			}

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				if(OptMod_sms!=0)
					SelectNext(hWnd,wParam);
				/*SMS£¬½â¾ö¶ÌÏûÏ¢²é¿´Ê±£¬²»ÄÜÉÏÏÂ·­Ò³ÎÊÌâ¡£add by yangxiaolong,start*/
				else
				{
					if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
					{
						SendMessage(Edtmp,MSG_KEYDOWN,SCANCODE_PAGEDOWN,lParam);
					}
					else
					{
						SendMessage(Edtmp,MSG_KEYDOWN,SCANCODE_PAGEUP,lParam);
					}
				}
				/*SMS£¬½â¾ö¶ÌÏûÏ¢²é¿´Ê±£¬²»ÄÜÉÏÏÂ·­Ò³ÎÊÌâ¡£add by yangxiaolong,start*/

				if((gOptions.TFTKeyLayout==3) && (EdText==focuswnd) && (OptMod_sms==1))
					SendMessage (EdText, EM_SELECTALL, 0, 0);
				break;
			}

			if((LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam) == SCANCODE_CURSORBLOCKRIGHT)||
					(gOptions.TFTKeyLayout == 3 && LOWORD(wParam) == SCANCODE_BACKSPACE))
			{
				if(Edtmp==BtSave && LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
				{
					if(SendMessage(CbType,CB_GETCURSEL,0,0)==0)
						SetFocusChild(BtSend);
					else
						SetFocusChild(BtExit);
				}
				else if(Edtmp==BtSend)
				{
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
						SetFocusChild(BtExit);
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || (gOptions.TFTKeyLayout==3&&LOWORD(wParam)==SCANCODE_BACKSPACE))
						SetFocusChild(BtSave);
				}
				else if(Edtmp==BtExit && (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT ||(gOptions.TFTKeyLayout==3&&LOWORD(wParam)==SCANCODE_BACKSPACE)))
				{
					if(SendMessage(CbType,CB_GETCURSEL,0,0)==0)
						SetFocusChild(BtSend);
					else
						SetFocusChild(BtSave);
					break;
				}
				else if(Edtmp==CbType)
				{
					g_seltype = SendMessage(CbType,CB_GETCURSEL,0,0);
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT ||(gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
					{					
						if(--g_seltype<0) g_seltype=2;
					}
					else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						if(++g_seltype>2) g_seltype=0;
					}
					SendMessage(CbType,CB_SETCURSEL,g_seltype ,0);

					if(g_seltype==0) {
						personalFlag = 1;//Liaozz 20081006 fix bug second 49
						SendMessage(BtSend,MSG_ENABLE,1,0);
					} else
						SendMessage(BtSend,MSG_ENABLE,0,0);
					break;
				}
				else
					return DefaultMainWinProc(hWnd,message,wParam,lParam);
			}

			if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10)) 
			{
				if(Edtmp == BtSave)
					SendMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0L);
				else if (Edtmp == BtExit)
					SendMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0L);
				else if(Edtmp ==BtSend)
					SendMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SEND,0L);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_MENU)
			{	
				if(OptMod_sms==0)
				{
					if(tmpsms_mng.Tag==UDATA_TAG_USERSMS)
					{
						CreateSmsSendWindow(hWnd,OptMod_sms,&smstmppin1,NULL,NULL);
						if(ismenutimeout) return 0;
					}
				}
				else 
					SendMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
				return 0;
			}

			if(LOWORD(wParam)==SCANCODE_F8 && SendMessage(CbType,CB_GETCURSEL,0,0)==0)
			{
				SendMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SEND,0L);
				return 0;
			}
			break;

		case MSG_CHAR:
			if(GetFocusChild(hWnd)==EdText)
			{
				memset(tmpchar1,0,MAX_SMS_CONTENT_SIZE*2+1);
				GetWindowText(EdText,tmpchar1,MAX_SMS_CONTENT_SIZE*2);
				if(strlen(tmpchar1)==(MAX_SMS_CONTENT_SIZE*2-1) && wParam>0xa0)
					return 0;
				else if(wParam==0x0d)
					return 0;
				else
					return DefaultMainWinProc(hWnd,message,wParam,lParam);
			}
			else
				return DefaultMainWinProc(hWnd,message,wParam,lParam);
			break;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(nc==EN_CHANGE)
			{
				switch(id)
				{
					case ID_YEAR:
						memset(sstr,0,20);
						GetWindowText(EdYear,sstr,4);
						if(strlen(sstr)>0)
						{
							tmpvalue = atoi(sstr);

							//zsliu change
							//DBPRINTF("smswinproc() - year = %d, tmptime.tm_year = %d\n", tmpvalue, tmptime.tm_year);
							if(gOptions.isUseHejiraCalendar)
							{
								if(tmpvalue < 0 || tmpvalue > 1408)
								{
									memset(sstr,0,20);
									sprintf(sstr,"%d",tmptime.tm_year + 1900);
									SetWindowText(EdYear,sstr);
								}
							}
							else
							{
								if(tmpvalue<0 || tmpvalue>2037)
								{
									memset(sstr,0,20);
									sprintf(sstr,"%d",tmptime.tm_year + 1900);
									SetWindowText(EdYear,sstr);
									SendMessage(EdYear,EM_SETCARETPOS,0,strlen(sstr));//Liaozz 20081008 fix bug third 11
								}
							}

							//zsliu change end ... ...
						}
						break;
					case ID_MONTH:
						memset(sstr,0,20);
						GetWindowText(EdMonth,sstr,4);
						if(strlen(sstr)>0)
						{
							tmpvalue = atoi(sstr);
							if(tmpvalue<0 || tmpvalue>12)
							{
								memset(sstr,0,20);
								sprintf(sstr,"%02d",tmptime.tm_mon+1);
								SetWindowText(EdMonth,sstr);
								SendMessage(EdMonth,EM_SETCARETPOS,0,strlen(sstr));//Liaozz 20081008 fix bug third 11
							}
						}
						break;
					case ID_DAY:
						memset(sstr,0,20);
						GetWindowText(EdDay,sstr,4);
						if(strlen(sstr)>0)
						{
							tmpvalue = atoi(sstr);
							if(tmpvalue<0 || tmpvalue>31)
							{
								memset(sstr,0,20);
								sprintf(sstr,"%02d",tmptime.tm_mday);
								SetWindowText(EdDay,sstr);
								SendMessage(EdDay,EM_SETCARETPOS,0,strlen(sstr));//Liaozz 20081008 fix bug third 11
							}
						}
						break;
					case ID_HOUR:
						memset(sstr,0,20);
						GetWindowText(EdHour,sstr,4);
						if(strlen(sstr)>0)
						{
							tmpvalue = atoi(sstr);
							if(tmpvalue<0 || tmpvalue>23)
							{
								memset(sstr,0,20);
								sprintf(sstr,"%02d",tmptime.tm_hour);
								SetWindowText(EdHour,sstr);
								SendMessage(EdHour,EM_SETCARETPOS,0,strlen(sstr));//Liaozz 20081008 fix bug third 11
							}
						}
						break;
					case ID_MIN:
						memset(sstr,0,20);
						GetWindowText(EdMin,sstr,4);
						if(strlen(sstr)>0)
						{
							tmpvalue = atoi(sstr);
							if(tmpvalue<0 || tmpvalue>60)
							{
								memset(sstr,0,20);
								sprintf(sstr,"%02d",tmptime.tm_min);
								SetWindowText(EdMin,sstr);
							}
						}
						break;
					case ID_MINLEN:
						memset(sstr,0,20);
						GetWindowText(EdMinlen,sstr,10);
						if(strlen(sstr)>0)
						{
							tmpvalue = atoi(sstr);
							if(tmpvalue<0 || tmpvalue>65535)
							{
								memset(sstr,0,20);
								sprintf(sstr,"%d",tmpminlen);
								SetWindowText(EdMinlen,sstr);
							}
						}
						break;

				}
			}

			/*			if(nc==CBN_SELCHANGE && id==CB_TYPE)
						{
						g_seltype = SendMessage(CbType,CB_GETCURSEL,0,0);
						if(g_seltype==0) 
						SendMessage(BtSend,MSG_ENABLE,1,0);
						else
						SendMessage(BtSend,MSG_ENABLE,0,0);
						}
						*/
			switch(id)
			{
				case ID_SEND:
					if (pudlk) freeLinkListBuf();
					pudlk =(PUDataLink) MALLOC(sizeof(TUDataLink));
					memset(pudlk,0,sizeof(TUDataLink));
					sendchgflag=0;
					bsended = CreateSmsSendWindow(hWnd,OptMod_sms,&smstmppin1,&sendchgflag,pudlk);
					//printf("_______%s%d,bsended = %d\n", __FILE__, __LINE__, bsended);
					if(ismenutimeout) break;
					g_seltype = (bsended==1)?0:2;
					SendMessage(CbType,CB_SETCURSEL,g_seltype,0);
					if(g_seltype==0) {
						personalFlag = 0;//Liaozz 20081006 fix bug second 49
						SendMessage(BtSend,MSG_ENABLE,1,0);
					} else
					{
						SendMessage(BtSend,MSG_ENABLE,0,0);
						SetFocusChild(BtSave);
					}
					UpdateWindow(hWnd,TRUE);
					break;

				case ID_SAVE:
					if(SaveSmsInfo(hWnd))	
					{
						bChanged_sms=1;
						SendMessage(hWnd,MSG_CLOSE,0,0L);
						return 0;
					}
					break;

				case ID_EXIT:
					if(ismodified()==1)
					{
						if(MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
									MB_OKCANCEL | MB_ICONQUESTION |	MB_BASEDONPARENT)==IDOK)
						{
							if(SaveSmsInfo(hWnd))
								bChanged_sms = 1;
						}
					}
					if(!ismenutimeout) PostMessage(hWnd,MSG_CLOSE,0,0L);
					return 0;
			}			

			break;

		case MSG_CLOSE:
			if(pudlk)
			{
				freeLinkListBuf();
				//				printf("freeLinkListBuf OK!\n");
			}

			if(hIMEWnd!=HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
				hIMEWnd=HWND_INVALID;
			}

			//UnloadBitmap(&smsmngbk);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

		default:
			return DefaultMainWinProc(hWnd,message,wParam,lParam);
	}
	return 0;

}

static U16 GetFreeSmsID(U16 id)
{
	TSms testsms;
	U16 testid;

	memset(&testsms,0,sizeof(TSms));
	testid=id;
	while(FDB_GetSms(testid,&testsms)!=NULL)
	{
		testid++;
	}
	return testid;

}

int CreateSmsOptWindow(HWND hWnd, int optmod, int *SMSPIN)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	char sstr[20];
	sendchgflag=0;
	OptMod_sms = optmod;
	memset(sstr,0,20);
	bChanged_sms=0;	
	switch(OptMod_sms)
	{
		case 0:	
			smstmppin1 = *SMSPIN;
			sprintf(sstr,"%s%s",LoadStrByID(MID_SMSCHECK),LoadStrByID(MID_SMSEDITCAPTION1));
			if(FDB_GetSms(smstmppin1,&tmpsms_mng)==NULL) return 0;
			break;

		case 1:	
			smstmppin1 = *SMSPIN;
			sprintf(sstr,"%s%s",LoadStrByID(MID_SMSEDIT),LoadStrByID(MID_SMSEDITCAPTION1));
			if(FDB_GetSms(smstmppin1,&tmpsms_mng)==NULL) return 0;
			break;

		case 2:	
			smstmppin1 = GetFreeSmsID(1);
			sprintf(sstr,"%s%s",LoadStrByID(MID_SMSADD),LoadStrByID(MID_SMSEDITCAPTION1));
			break;
	}

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = sstr;
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = smswinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	//CreateInfo.iBkColor = COLOR_lightgray;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	//if (LoadBitmap(HDC_SCREEN,&smsmngbk,GetBmpPath("submenubg.jpg")))
	//        return 0;
	LoadHint();

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return 0;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	*SMSPIN=smstmppin1;
	MainWindowThreadCleanup(hMainWnd);
	return bChanged_sms;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

