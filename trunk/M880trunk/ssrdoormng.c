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
#include "tftmsgbox.h"

#define DM_STATIC 	0x20
#define DM_ID		0x30
#define ID_SAVE		0x40
#define ID_EXIT		0x41
 
HWND EdDM[11];
HWND Edtmp,focuswnd;

//static BITMAP gpmngbk;
int actindex;
int oldvalue[9];
int parameter[9];

//the control offset in array
int idx485Reader=-1;
int idxHoliday=-1;	
int idxNCTimeZone=-1; 	//time zone of the door still close
int idxNOTimeZone=-1; 	//time zone of the door still open

extern int bDoorBreak;                                               //门被意外打开标志
extern int bDoorOpen;                                                //门正常打开标志
extern int bDoorClose;                                               //门已关闭标志

static int GetEditCounts()
{
	int ret;
	if (gOptions.Reader485FunOn){
		ret = (gOptions.LockFunOn==1)?2:9;
	} else {
		ret = (gOptions.LockFunOn==1)?1:8;
	}
	return ret;
}

static void InitWindow (HWND hWnd)
{
	char tmpstr[20];
	char *hintstr[20];	
	int posX1,posX2,posX3,posX4,posX5;
	int index=0;

	idx485Reader = -1;
	idxHoliday = -1;
	idxNCTimeZone = -1;
	idxNCTimeZone = -1;

	//Set parameter value
        parameter[index++]=gOptions.LockOn;			//锁驱动时长(1-10秒)
	printf("gOptions.LockOn=%d, %d, %d\n", gOptions.LockOn, index, parameter[0]);

	if(!(gOptions.LockFunOn ==1)){
		parameter[index++]=gOptions.OpenDoorDelay;		//门磁延时
		parameter[index++]=gOptions.DoorSensorMode;		//门磁开关（0：常开，1：常闭，其他：无）
		if (gOptions.Reader485FunOn) {
			parameter[index++]=gOptions.Reader485On;//dsl 2012.3.27
		}
		parameter[index++]=gOptions.DoorSensorTimeout;	//门磁报警延时
        	parameter[index++]=gOptions.ErrTimes;			//按键报警次数
        	parameter[index++]=gOptions.DoorCloseTimeZone;	//常闭时间段
        	parameter[index++]=gOptions.DoorOpenTimeZone;		//常开时间段
        	parameter[index++]=gOptions.IsHolidayValid;		//节假日是否有效
	} else if (gOptions.Reader485FunOn) {
			parameter[index++]=gOptions.Reader485On;//dsl 2012.3.27
	}

	hintstr[0]=LoadStrByID(MID_OS_LOCK);
	hintstr[1]=LoadStrByID(MID_AC_DSD);
	hintstr[2]=LoadStrByID(MID_AC_DSM);
	hintstr[3]=LoadStrByID(HIT_NULLKEY);			//null
	hintstr[4]=LoadStrByID(MID_CLOSE_TZ);	//close
	hintstr[5]=LoadStrByID(MID_OPEN_TZ);	//open
	hintstr[6]=LoadStrByID(MID_ALARM_DELAY);
	hintstr[7]=LoadStrByID(MID_ALARM_COUNT);
	hintstr[8]=LoadStrByID(MID_USER_TZ);
	hintstr[9]=LoadStrByID(MID_HTZ_VALID);
	hintstr[10]=LoadStrByID(MID_HD_INVALID);
	hintstr[11]=LoadStrByID(MID_HD_VALID);
	hintstr[12]=LoadStrByID(MID_SAVE);
	hintstr[13]=LoadStrByID(MID_EXIT);
	hintstr[14]=LoadStrByID(MID_SECOND);
	hintstr[15]=LoadStrByID(MID_TIMES);
	hintstr[16]=LoadStrByID(HID_485READER);
	hintstr[17]=LoadStrByID(HIT_SWITCH2);
        hintstr[18]=LoadStrByID(HIT_SWITCH1);

	posX1=10+gOptions.ControlOffset;
	posX2=110+gOptions.ControlOffset;
	posX3=160+gOptions.ControlOffset;
	posX4=110+gOptions.ControlOffset;
	posX5=110+gOptions.ControlOffset;

	index=0;
	CreateWindow(CTRL_STATIC, hintstr[0], WS_VISIBLE | SS_LEFT,DM_STATIC, 10, 10, 120, 23, hWnd, 0);
	EdDM[index] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
			DM_ID, 110, 6, 45, 23, hWnd, 0);
	sprintf(tmpstr,"%s%s","(1-10)",hintstr[14]);
	CreateWindow(CTRL_STATIC, tmpstr, WS_VISIBLE | SS_LEFT, DM_STATIC+10,160,10,80,23, hWnd, 0); 
	SendMessage(EdDM[index], EM_LIMITTEXT,2,0);

	if(!(gOptions.LockFunOn ==1))
	{	
		index++;
		CreateWindow(CTRL_STATIC, hintstr[1], WS_VISIBLE | SS_LEFT,DM_STATIC+1, 10, 35, 100, 23, hWnd, 0);
		EdDM[index] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				DM_ID+index, 110, 31, 45, 23, hWnd, 0);
		sprintf(tmpstr,"%s%s","(1-99)",hintstr[14]);
		CreateWindow(CTRL_STATIC, tmpstr, WS_VISIBLE | SS_LEFT, DM_STATIC+11,160,35,80,23, hWnd, 0); 
		SendMessage(EdDM[index], EM_LIMITTEXT,2,0);

		index++;
		CreateWindow(CTRL_STATIC, hintstr[2], WS_VISIBLE | SS_LEFT,DM_STATIC+2, 10, 60, 120, 23, hWnd, 0);
		EdDM[index] = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
				DM_ID+index, 100, 56, 70, 23, hWnd, 0); 
		SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[5]);               //
		SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[4]);               //
		SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[3]);               //

		//dsl 2012.3.27
		if (gOptions.Reader485FunOn)
		{
			index++;
			CreateWindow(CTRL_STATIC, hintstr[16], WS_VISIBLE | SS_LEFT,DM_STATIC+12,  170, 60, 120, 23, hWnd, 0);
			EdDM[index] = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
					DM_ID+index, 260, 56, 55, 23, hWnd, 0);
			
			printf("%s, %s\n", hintstr[17], hintstr[18]);
			SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[17]);
			SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[18]);
			idx485Reader = index;
		}

		index++;
		CreateWindow(CTRL_STATIC, hintstr[6], WS_VISIBLE | SS_LEFT,DM_STATIC+3, 10, 85, 120, 23, hWnd, 0);
		EdDM[index] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				DM_ID+index, 110, 81, 45, 23, hWnd, 0);
		SendMessage(EdDM[index], EM_LIMITTEXT,2,0);
		sprintf(tmpstr,"%s%s","(1-99)",hintstr[14]);
		CreateWindow(CTRL_STATIC, tmpstr, WS_VISIBLE | SS_LEFT,DM_STATIC+4, 160, 85, 80, 23, hWnd, 0);

		index++;
		CreateWindow(CTRL_STATIC, hintstr[7], WS_VISIBLE | SS_LEFT,DM_STATIC+5, 10, 110, 120, 23, hWnd, 0);
		EdDM[index] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				DM_ID+index, 110, 106, 45, 23, hWnd, 0);
		SendMessage(EdDM[index], EM_LIMITTEXT,1,0);
		sprintf(tmpstr,"%s%s","(1-9)",hintstr[15]);
		CreateWindow(CTRL_STATIC, tmpstr, WS_VISIBLE | SS_LEFT,DM_STATIC+6, 160, 110, 80, 23, hWnd, 0);

		index++;
		memset(tmpstr,0,20);
		sprintf(tmpstr,"%s%s",hintstr[4],hintstr[8]);
		CreateWindow(CTRL_STATIC, tmpstr, WS_VISIBLE | SS_LEFT,DM_STATIC+7, 10, 135, 120, 23, hWnd, 0);
		EdDM[index] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				DM_ID+index, 110, 131, 45, 23, hWnd, 0);
		SendMessage(EdDM[index], EM_LIMITTEXT,2,0);
		idxNCTimeZone=index;

		index++;
		memset(tmpstr,0,20);
		sprintf(tmpstr,"%s%s",hintstr[5],hintstr[8]);
		CreateWindow(CTRL_STATIC, tmpstr, WS_VISIBLE | SS_LEFT, DM_STATIC+8, 165, 135, 120, 23, hWnd, 0);
		EdDM[index] = CreateWindow(CTRL_SLEDIT, "", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
				DM_ID+index, 255, 131, 45, 23, hWnd, 0);
		SendMessage(EdDM[index], EM_LIMITTEXT,2,0);
		idxNOTimeZone=index;

		index++;
		CreateWindow(CTRL_STATIC, hintstr[9], WS_VISIBLE | SS_LEFT,DM_STATIC+9, 10, 160, 120, 23, hWnd, 0);
		EdDM[index] = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
				DM_ID+index, 130, 156, 90, 23, hWnd, 0);
		SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[10]);      //无效
		SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[11]);      //有效
		idxHoliday = index;
	} else if (gOptions.Reader485FunOn){
		index++;
		CreateWindow(CTRL_STATIC, hintstr[16], WS_VISIBLE | SS_LEFT,DM_STATIC+12,  10, 35, 100, 23, hWnd, 0);
		EdDM[index] = CreateWindow(CTRL_COMBOBOX,"",WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,
				DM_ID+index, 110, 31, 55, 23, hWnd, 0);
		SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[17]);
		SendMessage(EdDM[index],CB_ADDSTRING, 0, (LPARAM)hintstr[18]);
		idx485Reader = index;
	}

	if( !(gOptions.LockFunOn ==1))
	{
		index++;
		EdDM[index]=CreateWindow(CTRL_BUTTON,hintstr[12],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_SAVE,10,190,90,23,hWnd,0);
		index++;
		EdDM[index]=CreateWindow(CTRL_BUTTON,hintstr[13],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_EXIT,220+gOptions.ControlOffset,190,90,23,hWnd,0);
	}
	else
	{
		index++;
		EdDM[index]=CreateWindow(CTRL_BUTTON,hintstr[12],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_SAVE,10,190,90,23,hWnd,0);
		index++;
		EdDM[index]=CreateWindow(CTRL_BUTTON,hintstr[13],WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER,ID_EXIT,220+gOptions.ControlOffset,190,90,23,hWnd,0);
	}
}

static void InitWindowText(HWND hWnd)
{
	int i;
	char tmpchar[10];
	int EditCounts=GetEditCounts();

	printf("EditCounts=%d\n", EditCounts);
	for(i=0;i<EditCounts;i++)
	{
        	oldvalue[i] = parameter[i];
		printf("old value[%d]=%d, ori=%d\n", i, oldvalue[i], parameter[i]);

		if (i==2 || (gOptions.Reader485FunOn && i==idx485Reader) || i==idxHoliday) {
			SendMessage(EdDM[i], CB_SETCURSEL, oldvalue[i], 0);
		} else {
			memset(tmpchar, 0, 10);
			sprintf(tmpchar,"%d",oldvalue[i]);
			SetWindowText(EdDM[i], tmpchar);
		}
		//printf("1------old value[%d]=%d, ori=%d\n", i, oldvalue[i], parameter[i]);
	}
}

static int ismodified(void)
{
	int i;
	char tmpchar[10];
	int EditCounts=GetEditCounts();

	for(i=0;i<EditCounts;i++)
	{
		if(i==2 || i==idxHoliday || (gOptions.Reader485FunOn && i==idx485Reader)){
			if(oldvalue[i] != SendMessage(EdDM[i], CB_GETCURSEL, 0, 0)){
				return 1;
			}
		} else {
			memset(tmpchar,0,10);
			GetWindowText(EdDM[i],tmpchar,10);
			if(oldvalue[i] != atoi(tmpchar)){
				return 1;
			}
		}
	}
	return 0;
}

static void SaveDoorParameter(HWND hWnd)
{
	int i;
	int tmpvalue[9];
	char tmpchar[10];
	int EditCounts=GetEditCounts();

	memset(tmpvalue, 0, sizeof(tmpvalue));
	memset(tmpchar, 0, sizeof(tmpchar));
	for(i=0; i<EditCounts; i++)
	{
		if(i==2 || i==idxHoliday || (gOptions.Reader485FunOn && i==idx485Reader)) {
			tmpvalue[i] = SendMessage(EdDM[i], CB_GETCURSEL, 0, 0);
		} else {
			memset(tmpchar,0,10);
			GetWindowText(EdDM[i],tmpchar,10);
			tmpvalue[i] = atoi(tmpchar);
		}
		printf("tmpvalue[%d]=%d\n", i, tmpvalue[i]);
	}

        gOptions.LockOn=tmpvalue[0];                   //锁驱动时长(1-10秒)
	if(!(gOptions.LockFunOn == 1)){
		int idx=3;
		if(gOptions.OpenDoorDelay!=tmpvalue[1] || gOptions.DoorSensorMode!=tmpvalue[2])	{
			gOptions.OpenDoorDelay=tmpvalue[1];            //门磁延时
			gOptions.DoorSensorMode=tmpvalue[2];           //门磁开关（0：常开，1：常闭，其他：无）
			ExSetAuxOutDelay(gOptions.LockOn,gOptions.OpenDoorDelay, gOptions.DoorSensorMode);
			if(gOptions.DoorSensorMode>1){
				bDoorBreak=0;			//门被意外打开标志
				bDoorOpen=0;                    //门正常打开标志
				bDoorClose=0;                   //门已关闭标志
			}
		}
		if (gOptions.Reader485FunOn) {
			gOptions.Reader485On =  tmpvalue[idx++];
			printf("Reader485On=%d\n", gOptions.Reader485On);
		}
		gOptions.DoorSensorTimeout=tmpvalue[idx++];        //门磁报警延时
		gOptions.ErrTimes=tmpvalue[idx++];                 //按键报警次数
		gOptions.DoorCloseTimeZone=tmpvalue[idx++];        //常闭时间段
		gOptions.DoorOpenTimeZone=tmpvalue[idx++];         //常开时间段
		gOptions.IsHolidayValid=tmpvalue[idx++];             //节假日是否有效
	} else if (gOptions.Reader485FunOn) {
		/*dsl 2012.4.6*/
		gOptions.Reader485On = tmpvalue[1];
	}
	GetNoNcState();         //强制开关门
	SaveOptions(&gOptions);
	sync();
	LoadOptions(&gOptions);	
}

static int InvalidValue(HWND hWnd, int index)
{
	TTimeZone ttz;
	int tmpvalue;
	int err=0;
	char sstr[10];
	
	memset(sstr,0,10);
	GetWindowText(EdDM[index],sstr,9);
	tmpvalue = atoi(sstr);

	if (gOptions.Reader485FunOn){
		switch(index){
			case 0:
				if(tmpvalue < 1 || tmpvalue > 10)	
					err++;
				break;
			case 1:
				if(tmpvalue < 1 || tmpvalue > 99)
					err++;
				break;
			case 4:
				if(tmpvalue < 1 || tmpvalue > 99)
					err++;
				break;
			case 5:
				//1~9
				if(tmpvalue < 1 ||tmpvalue > 9)	//modify by jazzy 2009.01.15
					err++;
				break;
			case 6:
			case 7:
				memset(&ttz,0,sizeof(TTimeZone));
				if(tmpvalue && FDB_GetTimeZone(tmpvalue,&ttz)==NULL)
					err++;
				break;
		}
	} else {
		switch(index){
			case 0:
				if(tmpvalue < 1 || tmpvalue > 10)	
					err++;
				break;
			case 1:
				if(tmpvalue < 1 || tmpvalue > 99)
					err++;
				break;
			case 3:
				if(tmpvalue < 1 || tmpvalue > 99)
					err++;
				break;
			case 4:
				//1~9
				if(tmpvalue < 1 ||tmpvalue > 9)	//modify by jazzy 2009.01.15
					err++;
				break;
			case 5:
			case 6:
				memset(&ttz,0,sizeof(TTimeZone));
				if(tmpvalue && FDB_GetTimeZone(tmpvalue,&ttz)==NULL)
					err++;
				break;
		}
	}

	if(0==err){
		return 0;
	}
	if(index==idxNCTimeZone || index==idxNOTimeZone){
		if(MessageBox1(hWnd,LoadStrByID(MID_SAVE_HINT2), LoadStrByID(MID_APPNAME),MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK){
			TimeZoneMngWindow(hWnd,tmpvalue,1);               //add new time zone
		}
	} else {
		MessageBox1(hWnd,LoadStrByID(HIT_ERROR0),LoadStrByID(HIT_ERR),MB_OK|MB_ICONINFORMATION);
	}
	return 1;			
}

static int doorparawinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	char sstr[10];
	int i;
	int tmpvalue,tmptz;
	int gselmax;
	static char keyupFlag=0;
	int EditCounts=GetEditCounts();
	switch (message)
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN,&gpmngbk,GetBmpPath("submenubg.jpg"))){
	                 //       return 0;
			//}

			InitWindow(hWnd);		//add controls

			InitWindowText(hWnd);
			UpdateWindow(hWnd,TRUE);
			actindex=0;
			SetFocusChild(EdDM[actindex]);
			break;

		case MSG_ERASEBKGND:
		{
	            	HDC hdc = (HDC)wParam;
			const RECT* clip = (const RECT*)lParam;
			BOOL fGetDC = FALSE;
			RECT rcTemp;
			if (hdc == 0) {
				hdc = GetClientDC(hWnd);
				fGetDC = TRUE;
			}

			if(clip) {
				rcTemp = *clip;
				ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
				ScreenToClient(hWnd,&rcTemp.right, &rcTemp.bottom);
				IncludeClipRect(hdc, &rcTemp);
			}

			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
			if(fGetDC) ReleaseDC (hdc);
			break;
		}

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);	
			EndPaint(hWnd,hdc);
			break;

		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout){
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout) {
				if (1==keyupFlag) {
					keyupFlag=0;
				} else {
					break;
				}
			}
			if (gOptions.KeyPadBeep) {
				ExKeyBeep();
			}

			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				SendMessage(hWnd, MSG_COMMAND, ID_EXIT, 0L);
				break;
			}

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if(actindex!=2 && actindex != idx485Reader && actindex != idxHoliday)
				{
					memset(sstr,0,10);
					GetWindowText(EdDM[actindex],sstr,9);
					if(strlen(sstr)==0){
						break;
					}
				}
				if(++actindex > EditCounts+1) actindex = 0;
				SetFocusChild(EdDM[actindex]);
			}

			if( LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if(actindex!=2 && actindex != idx485Reader && actindex != idxHoliday)
				{
					memset(sstr,0,10);
					GetWindowText(EdDM[actindex],sstr,9);
					if(strlen(sstr)==0){
						break;
					}
				}
				if(--actindex < 0) actindex = EditCounts+1;
				SetFocusChild(EdDM[actindex]);
			}

			if(LOWORD(wParam) == SCANCODE_CURSORBLOCKLEFT || (gOptions.TFTKeyLayout == 3 && LOWORD(wParam) == SCANCODE_BACKSPACE))
			{
				if(actindex==2 || actindex==idx485Reader || actindex == idxHoliday)
				{
					tmpvalue=SendMessage(EdDM[actindex], CB_GETCURSEL, 0, 0);
					gselmax = (actindex==2) ? 2:1;
					if(--tmpvalue < 0) tmpvalue = gselmax;
					SendMessage(EdDM[actindex], CB_SETCURSEL, tmpvalue, 0);
				}
				else
				{
					memset(sstr,0,10);
					GetWindowText(EdDM[actindex],sstr,9);
					if(strlen(sstr)==0){
						break;
					}
					if(--actindex < 0) actindex = EditCounts+1;
					SetFocusChild(EdDM[actindex]);
				}
			}

			if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if(actindex==2 || actindex==idx485Reader || actindex == idxHoliday)
				{
					tmpvalue=SendMessage(EdDM[actindex], CB_GETCURSEL, 0, 0);
					gselmax = (actindex==2) ? 2:1;
					if(++tmpvalue > gselmax) tmpvalue = 0;
					SendMessage(EdDM[actindex], CB_SETCURSEL, tmpvalue, 0);
				}
				else
				{
					memset(sstr,0,10);
					GetWindowText(EdDM[actindex],sstr,9);
					if(strlen(sstr)==0){
						break;
					}
					if(++actindex > EditCounts+1) actindex = 0;
					SetFocusChild(EdDM[actindex]);
				}
			}

			if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
			{
				if(actindex != EditCounts+1)
					SendMessage(hWnd,MSG_COMMAND,(WPARAM)ID_SAVE,0);
				else
					SendMessage(hWnd,MSG_COMMAND,(WPARAM)ID_EXIT,0);
				break;
			}

			if(LOWORD(wParam)==SCANCODE_MENU)
			{
				SendMessage (hWnd, MSG_COMMAND, (WPARAM)ID_SAVE, 0L);
				break;
			}

			break;
	
		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if(nc==EN_CHANGE)
			{
				if(id == DM_ID+5 || id == DM_ID+6)
				{
					memset(sstr,0,5);
					GetWindowText(EdDM[actindex],sstr,2);
					tmpvalue = atoi(sstr);
					if(tmpvalue>TZ_MAX)
					{
						SetWindowText(EdDM[actindex],"0");
						SendMessage(EdDM[actindex],EM_SETCARETPOS,0,1);
					}
				}
			}

			switch(id)
			{
				case ID_SAVE:
					for(i=0;i<EditCounts;i++)
					{	
						if(i!=2 && i!=idx485Reader && i != idxHoliday){
							if (InvalidValue(hWnd, i)) {
								if (!ismenutimeout) {
									actindex=i;
									SetFocusChild(EdDM[i]);
								}
								return 0;
							}
						}
					}
					if(!(gOptions.LockFunOn ==1))
					{
						if (gOptions.Reader485FunOn) {
							memset(sstr,0,5);
							GetWindowText(EdDM[6],sstr,2);
							tmptz=atoi(sstr);
							memset(sstr,0,5);
							GetWindowText(EdDM[7],sstr,2);

						} else {
							memset(sstr,0,5);
							GetWindowText(EdDM[5],sstr,2);
							tmptz=atoi(sstr);
							memset(sstr,0,5);
							GetWindowText(EdDM[6],sstr,2);
						}
						
						if(tmptz && atoi(sstr) && tmptz==atoi(sstr))
						{
							MessageBox1(hWnd, LoadStrByID(MID_TZ_SAME), LoadStrByID(MID_APPNAME), MB_OK | MB_ICONSTOP);
							if(!ismenutimeout) SetFocusChild(EdDM[5]);
							return 0;
						}
					}
					if(ismodified())
					{	
						SaveDoorParameter(hWnd);	
			                	MessageBox1(hWnd ,LoadStrByID(HIT_RIGHT) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
						if(ismenutimeout) return 0;
					}
					SendMessage(hWnd,MSG_CLOSE,0,0);
					break;

				case ID_EXIT:				
	                                if(ismodified())
       					{
                                        	if(MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA), LoadStrByID(MID_APPNAME),
                                                	MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) == IDOK)
	                                                SendMessage(hWnd, MSG_COMMAND, ID_SAVE, 0);
						if(ismenutimeout) break;
                        	        }
					SendMessage(hWnd,MSG_CLOSE,0,0L);
	                                break;
			}			

			break;

		case MSG_CLOSE:
			//UnloadBitmap(&gpmngbk);
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			break;

		default:
			return DefaultMainWinProc(hWnd,message,wParam,lParam);

	}
	return (0);

}

int DoorParameterWindow(HWND hWnd)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	
	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_LOCK_OP5);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = doorparawinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
        CreateInfo.by = gOptions.LCDHeight;
	//CreateInfo.iBkColor = COLOR_lightgray;
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

