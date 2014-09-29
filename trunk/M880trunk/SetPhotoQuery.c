#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

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
#include "exfun.h"
#include "ssrcommon.h"
#include "main.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"
#include "truncdate.h"

#define ID_PHOTO_SAVE 	432
#define ID_PHOTO_EXIT 	433

#define LB_PHOTO	95
#define ID_PHOTO	105

//BITMAP querybkg;
HWND QueryWnd[12];
HWND tmpstatic[12];
int querymode;
int curoptidx;
char resacno1[30];		//用户工号

extern int GetMonDayfromyear (int year, int month);

static void ProcInputValid(void)
{
	char editcontent[6];
	int midvalue=0;

	memset(editcontent,0,sizeof(editcontent));
	GetWindowText(QueryWnd[curoptidx], editcontent, 5);
	if (strlen(editcontent))
	{
		midvalue = atoi(editcontent);
		//zsliu change
                //if (((curoptidx == 2 || curoptidx == 6) && (midvalue >= 2038 || midvalue <= 1970)) ||
                 if(((curoptidx == 3 || curoptidx == 7) && (midvalue > 12 || midvalue < 0)) ||
                        ((curoptidx == 4 || curoptidx == 8) && (midvalue > 31 || midvalue < 0)) ||
                        ((curoptidx == 5 || curoptidx == 9) && (midvalue > 24 || midvalue < 0)))
		{
			if (curoptidx == 5)
				SetWindowText(QueryWnd[curoptidx], "0");
			else if (curoptidx == 9)
				SetWindowText(QueryWnd[curoptidx], "24");
			else
				SetWindowText(QueryWnd[curoptidx], "");
			SetFocusChild(QueryWnd[curoptidx]);
		}
	}
}

extern int isValidateDate(char* datename, time_t stime, time_t etime);
static int ProcDeletePhoto(HWND hWnd, char* userID, time_t resst, time_t resed, int deltype)
{

        HWND statushWnd;
	PPhotoList plist=NULL;
	PPhotoList ptmp=NULL;
	int lcount;
	char pPath[100];

        statushWnd = createStatusWin1(hWnd, 250, 50, LoadStrByID(MID_APPNAME), LoadStrByID(MID_PHOTO_HINT1));
        busyflag = 1;
        plist = FDB_GetPhotoToList(resst, resed, userID, 1, &lcount, deltype);
        SetMenuTimeOut(time(NULL));

	ptmp = plist->next;
	while(ptmp != plist)
	{
		memset(pPath, 0, 100);
		if(deltype==0)
		{
			sprintf(pPath, "rm -rf %s/%s && sync", GetCapturePath("capture/pass"), ptmp->PhotoFile);
		}
		else
		{
			sprintf(pPath, "rm -rf %s/%s && sync", GetCapturePath("capture/bad"), ptmp->PhotoFile);
		}
		systemEx(pPath);
		plist->next = ptmp->next;
		ptmp->next->pre = plist;
		FREE(ptmp);
		ptmp = plist->next;
	}
	FREE(plist);
	plist = NULL;
	ptmp = NULL;
        destroyStatusWin1(statushWnd);
//	picid = 1;		//仄1?7?1?7始查找照片索引ID

	MessageBox1(hWnd, LoadStrByID(HIT_SYSTEM2DATA5), LoadStrByID(HIT_RUN),MB_OK | MB_ICONINFORMATION);
	if(!ismenutimeout)
	{
		PostMessage(hWnd, MSG_CLOSE, 0, 0);
	}
	return 1;
}

static int ProcQueryResult(HWND hWnd)
{
	char editcontent[6];
	int startvalue[5], endvalue[5];
	int i;
	time_t resst=0;
	time_t resed=0;
        int dayerror=0;
	TTime mytime;

	int procall = (SendMessage(QueryWnd[1], CB_GETCURSEL, 0, 0)==1)?0:1;

	memset(resacno1, 0, sizeof(resacno1));
	if (querymode <= 1)	//只有查询考勤图片时设置工叄1?7
	{
		GetWindowText(QueryWnd[0], resacno1, PINLIMITLEN);
		/*
		if (strcmp(resacno1, LoadStrByID(MID_ALL)) == 0)
		{
			memset(resacno1, 0, sizeof(resacno1));
		}*/
	}

	memset(&mytime, 0, sizeof(TTime));
	if (!procall)
	{
		//读取日期及时闄1?7
		for (i = 0; i < 4; i++)
		{
			memset(editcontent, 0, sizeof(editcontent));
			GetWindowText(QueryWnd[i+2], editcontent, 5);
			startvalue[i] = atoi(editcontent);

			memset(editcontent, 0, 5);
			GetWindowText(QueryWnd[i+6], editcontent, 5);
			endvalue[i] = atoi(editcontent);
		}
		
		//zsliu add

                if(gOptions.isUseHejiraCalendar)
                {
                        if( (startvalue[0] > 1408) ||  (startvalue[0] < 1380) || (endvalue[0] > 1408 || (endvalue[0] < 1380)))
                        {
                                myMessageBox1(hWnd, MB_OK | MB_ICONSTOP, LoadStrByID(MID_APPNAME), LoadStrByID(MID_ERRORTIME));
                                return 0;
                        }
                }
                //zsliu add end ... ...

		
	        if (startvalue[2] > GetMonDayfromyear(startvalue[0], startvalue[1]) || 
			endvalue[2] > GetMonDayfromyear(endvalue[0], endvalue[1]))
		{
        	        dayerror=1;
		}

		if (startvalue[3] == 24)
			startvalue[3] = 23;

		if (endvalue[3] == 24)
			endvalue[3] = 23;

		if (startvalue[0] == 0 || startvalue[1] == 0 || startvalue[2] == 0 || 
			endvalue[0] == 0 || endvalue[1] == 0 || endvalue[2] == 0 || dayerror)
		{
			myMessageBox1(hWnd, MB_OK | MB_ICONSTOP, LoadStrByID(MID_APPNAME), LoadStrByID(MID_ERRORTIME));
			if(!ismenutimeout)
			{
				if (startvalue[0] == 0)
					SetFocusChild(QueryWnd[2]);
				if (startvalue[1] == 0)
					SetFocusChild(QueryWnd[3]);
				if (startvalue[2] == 0)
					SetFocusChild(QueryWnd[4]);
				if (endvalue[0]==0)
					SetFocusChild(QueryWnd[6]);
				if (endvalue[1]==0)
					SetFocusChild(QueryWnd[7]);
				if (endvalue[2]==0)
					SetFocusChild(QueryWnd[8]);
			}
			return 0;
		}

		mytime.tm_year = startvalue[0] - 1900;
        	mytime.tm_mon = startvalue[1] - 1;
	        mytime.tm_mday = startvalue[2];
	        mytime.tm_hour = startvalue[3];
	        mytime.tm_min = 0;
	        mytime.tm_sec = 0;
	        mytime.tm_isdst = -1;
		
		//zsliu add 
                if(gOptions.isUseHejiraCalendar)
                {
                        HejiraToChrisTTime(&mytime); //伊朗日历转公厄1?7
                        //mytime.tm_year -= 1900;
                }
                //zsliu add end ... ...

		
	        if (endvalue[1] < startvalue[1])
        	        mytime.tm_year -= 1;
	        resst = OldEncodeTime(&mytime);

	        mytime.tm_year = endvalue[0] - 1900;
        	mytime.tm_mon = endvalue[1] - 1;
	        mytime.tm_mday = endvalue[2];
	        mytime.tm_hour = endvalue[3];
        	mytime.tm_min =  59;
	        mytime.tm_sec =  59;
	        mytime.tm_isdst = -1;
		//zsliu add
                if(gOptions.isUseHejiraCalendar)
                {
                        HejiraToChrisTTime(&mytime);//伊朗日历转公厄1?7
                        //mytime.tm_year -= 1900;
                }
                //zsliu add end ... ...

        	resed = OldEncodeTime(&mytime);

		if (resed < resst)
		{
			myMessageBox1(hWnd, MB_OK | MB_ICONSTOP, LoadStrByID(MID_APPNAME), LoadStrByID(MID_ERRORTIME));
			if(!ismenutimeout) SetFocusChild(QueryWnd[1]);
			return 0;
		}
	}

	if ((resacno1 && strlen(resacno1)) && (FDB_GetUserByCharPIN2(resacno1, NULL)==NULL))
	{
		myMessageBox1(hWnd, MB_OK | MB_ICONSTOP, LoadStrByID(MID_APPNAME), LoadStrByID(HID_NOTENROLLED));
		if(!ismenutimeout) SetFocusChild(QueryWnd[0]);
		return 0;
	}

	switch(querymode)
	{
		case 0:
			if(ViewPhotoWindow(hWnd, resacno1, resst, resed, 0)==0)		//浏览考勤照片
			{
				myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,
				LoadStrByID(MID_APPNAME),LoadStrByID(HID_VERDELETE));
			}   //lyy
			break;
		case 1:
			ProcDeletePhoto(hWnd, resacno1, resst, resed, 0);		//删除考勤照片
			break;
		case 2:
			if(ViewPhotoWindow(hWnd, resacno1, resst, resed, 1)==0)		//浏览黑名单照牄1?7
			{
				myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,
				LoadStrByID(MID_APPNAME),LoadStrByID(HID_VERDELETE));
			}   //lyy
			break;
		case 3:
			ProcDeletePhoto(hWnd, resacno1, resst, resed, 1);		//删除黑名单照牄1?7
			break;
	}
	return 1;

}

static void InitPhotoQueryWindow(HWND hWnd)
{
	int rows=0;
        TTime curtime;
        char yearstr[5], monstr[3], daystr[3];
	int posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9,posX10,posX11;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX1=240+gOptions.ControlOffset;
		posX2=170+gOptions.ControlOffset;

		posX3=220+gOptions.ControlOffset;
		posX4=200+gOptions.ControlOffset;
		posX5=160+gOptions.ControlOffset;
		posX6=130+gOptions.ControlOffset;
		posX7=90+gOptions.ControlOffset;
		posX8=75+gOptions.ControlOffset;
		posX9=30+gOptions.ControlOffset;
		posX10=10+gOptions.ControlOffset;
		posX11=130+gOptions.ControlOffset;
	}
	else
	{
		posX1=10+gOptions.ControlOffset;
		posX2=90+gOptions.ControlOffset;
		posX3=55+gOptions.ControlOffset;
		posX4=103+gOptions.ControlOffset;
		posX5=130+gOptions.ControlOffset;
		posX6=163+gOptions.ControlOffset;
		posX7=190+gOptions.ControlOffset;
		posX8=223+gOptions.ControlOffset;
		posX9=250+gOptions.ControlOffset;
		posX10=283+gOptions.ControlOffset;
		posX11=0+gOptions.ControlOffset;
	}

        GetTime(&curtime);

	//zsliu add 
        if(gOptions.isUseHejiraCalendar)
        {
                ChrisToHejiraTTime(&curtime); //公历转伊朗日厄1?7
                //curtime.tm_mon -= 1;
        }
        //zsliu add end ... ...

	memset(yearstr,0,sizeof(yearstr));
	sprintf(yearstr, "%04d", curtime.tm_year+1900);
	memset(monstr,0,sizeof(monstr));
        sprintf(monstr, "%02d", curtime.tm_mon+1);
	memset(daystr,0,sizeof(daystr));
        sprintf(daystr, "%02d", curtime.tm_mday);

	tmpstatic[0] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_ACNO), SS_CENTER, LB_PHOTO, posX1, 20+rows*30, 80, 23, hWnd, 0);
	//QueryWnd[0] = CreateWindow(CTRL_SLEDIT, LoadStrByID(MID_ALL), WS_BORDER, ID_PHOTO, posX2-posX11, 16+rows*30, 180, 23, hWnd, 0);
	QueryWnd[0] = CreateWindow(CTRL_SLEDIT, "", WS_BORDER, ID_PHOTO, posX2-posX11, 16+rows*30, 180, 23, hWnd, 0);
	if (querymode <= 1)
	{
		ShowWindow(tmpstatic[0], SW_SHOW);
		ShowWindow(QueryWnd[0], SW_SHOW);
		SendMessage(QueryWnd[0], EM_LIMITTEXT, gOptions.PIN2Width, 0L);
		SendMessage(QueryWnd[0], EM_SETCARETPOS, 0, 2);
		rows++;
	}

	CreateWindow(CTRL_STATIC, LoadStrByID(MID_PHOTO_DEL), WS_VISIBLE | SS_CENTER, LB_PHOTO+1, posX1, 20+rows*30, 80, 23, hWnd, 0);
	QueryWnd[1] = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE | CBS_READONLY | CBS_SPINLIST | CBS_SPINARROW_LEFTRIGHT | CBS_AUTOFOCUS,
                                ID_PHOTO+1, posX2, 16+rows*30, 60, 23, hWnd, 0);
	SendMessage(QueryWnd[1], CB_ADDSTRING, 0, (int)LoadStrByID(HID_YES));
	SendMessage(QueryWnd[1], CB_ADDSTRING, 0, (int)LoadStrByID(HID_NO));
	SendMessage(QueryWnd[1], CB_SETCURSEL, 0, 0);
	rows++;

        tmpstatic[1] = CreateWindow(CTRL_STATIC, LoadStrByID(REP_ATTTMST), SS_CENTER, LB_PHOTO+2, posX1, 20+rows*30, 80, 23, hWnd, 0);
	//year1
	QueryWnd[2] = CreateWindow(CTRL_SLEDIT, yearstr, WS_CHILD | CBS_EDITNOBORDER | CBS_AUTOFOCUS, ID_PHOTO+2,
						posX3, 16+rows*30, 45, 23, hWnd, 0);
	SendMessage(QueryWnd[2], EM_LIMITTEXT, 4, 0L);
	tmpstatic[2] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_YEAR), SS_CENTER | WS_CHILD, LB_PHOTO+3, posX4, 20+rows*30, 25, 23, hWnd, 0);	
	//month1
	QueryWnd[3] = CreateWindow(CTRL_SLEDIT, monstr, WS_CHILD | CBS_EDITNOBORDER | CBS_AUTOFOCUS, ID_PHOTO+3,
						posX5, 16+rows*30, 30, 23, hWnd, 0);
        SendMessage(QueryWnd[3], EM_LIMITTEXT, 2, 0L);
        tmpstatic[3] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_MON), SS_CENTER | WS_CHILD, LB_PHOTO+4, posX6, 20+rows*30, 25, 23, hWnd, 0);

	//day1
	QueryWnd[4] = CreateWindow(CTRL_SLEDIT, daystr, WS_CHILD | CBS_EDITNOBORDER | CBS_AUTOFOCUS, ID_PHOTO+4,
						posX7, 16+rows*30, 30, 23, hWnd, 0);
        SendMessage(QueryWnd[4], EM_LIMITTEXT, 2, 0L);
        tmpstatic[4] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_DAY), SS_CENTER | WS_CHILD, LB_PHOTO+5, posX8, 20+rows*30, 25, 23, hWnd, 0);

	//hour1
	QueryWnd[5] = CreateWindow(CTRL_SLEDIT, "0", WS_CHILD | CBS_EDITNOBORDER | CBS_AUTOFOCUS, ID_PHOTO+5, 
						posX9, 16+rows*30, 30, 23, hWnd, 0);
        SendMessage(QueryWnd[5], EM_LIMITTEXT, 2, 0L);
        tmpstatic[5] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_HOUR), SS_CENTER | WS_CHILD, LB_PHOTO+6, posX10, 20+rows*30, 25, 23, hWnd, 0);
	rows++;

        tmpstatic[6] = CreateWindow(CTRL_STATIC, LoadStrByID(REP_ATTTMED), SS_CENTER, LB_PHOTO+7, posX1, 20+rows*30, 80, 23, hWnd, 0);
	//year2
        QueryWnd[6] = CreateWindow(CTRL_SLEDIT, yearstr, WS_CHILD | CBS_EDITNOBORDER | CBS_AUTOFOCUS, ID_PHOTO+6,
                                                posX3, 16+rows*30, 45, 23, hWnd, 0);
        SendMessage(QueryWnd[6], EM_LIMITTEXT, 4, 0L);
        tmpstatic[7] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_YEAR), SS_CENTER | WS_CHILD, LB_PHOTO+8, posX4, 20+rows*30, 25, 23, hWnd, 0);
	
	//month2
        QueryWnd[7] = CreateWindow(CTRL_SLEDIT, monstr, WS_CHILD | CBS_EDITNOBORDER | CBS_AUTOFOCUS, ID_PHOTO+7,
                                                posX5, 16+rows*30, 30, 23, hWnd, 0);
        SendMessage(QueryWnd[7], EM_LIMITTEXT, 2, 0L);
        tmpstatic[8] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_MON), SS_CENTER | WS_CHILD, LB_PHOTO+9, posX6, 20+rows*30, 25, 23, hWnd, 0);

	//day2
        QueryWnd[8] = CreateWindow(CTRL_SLEDIT, daystr, WS_CHILD | CBS_EDITNOBORDER | CBS_AUTOFOCUS, ID_PHOTO+8, 
                                                posX7, 16+rows*30, 30, 23, hWnd, 0);
        SendMessage(QueryWnd[8], EM_LIMITTEXT, 2, 0L);
        tmpstatic[9] = CreateWindow(CTRL_STATIC, LoadStrByID(MID_DAY), SS_CENTER | WS_CHILD, LB_PHOTO+10, posX8, 20+rows*30, 25, 23, hWnd, 0);

	//hour2
        QueryWnd[9] = CreateWindow(CTRL_SLEDIT, "24", WS_CHILD | CBS_EDITNOBORDER | CBS_AUTOFOCUS, ID_PHOTO+9,
                                                posX9, 16+rows*30, 30, 23, hWnd, 0);
        SendMessage(QueryWnd[9], EM_LIMITTEXT, 2, 0L);
        tmpstatic[10] = CreateWindow(CTRL_STATIC,LoadStrByID(MID_HOUR), SS_CENTER|WS_CHILD, LB_PHOTO+11, posX10, 20+rows*30, 25, 23, hWnd, 0);

        rows++;

	QueryWnd[10] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_SAVE), WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER, 
						ID_PHOTO_SAVE, 10, 180, 85, 25, hWnd, 0);
	QueryWnd[11] = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_EXIT), WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
						ID_PHOTO_EXIT, 220+gOptions.ControlOffset, 180, 85, 25, hWnd, 0);
}

static int PhotoQueryWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id,nc;
	static char keyupFlag=0;
	switch (message)	
	{
		case MSG_CREATE:
			//LoadBitmap(HDC_SCREEN, &querybkg, GetBmpPath("submenubg.jpg"));
			InitPhotoQueryWindow(hWnd);
			UpdateWindow(hWnd,TRUE);
	//printf ("MSG_CREATE querymode %d \n",querymode);
			if (querymode <= 1)
				curoptidx=0;
			else
				curoptidx=1;
			SetFocusChild(QueryWnd[curoptidx]);
			break;

                case MSG_ERASEBKGND:
                {
                    	HDC hdc = (HDC)wParam;
			const RECT* clip = (const RECT*) lParam;
                    	BOOL fGetDC = FALSE;
                    	RECT rcTemp;

                    	if (hdc == 0) {
                        	hdc = GetClientDC (hWnd);
                        	fGetDC = TRUE;
                   	}

                    	if (clip) {
                        	rcTemp = *clip;
                        	ScreenToClient (hWnd, &rcTemp.left, &rcTemp.top);
                        	ScreenToClient (hWnd, &rcTemp.right, &rcTemp.bottom);
                        	IncludeClipRect (hdc, &rcTemp);
                    	}

                    	FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());

                    	if (fGetDC)
                        	ReleaseDC (hdc);
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
                        if (gOptions.KeyPadBeep) ExKeyBeep();
			
			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{	
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT || LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
			{
				if (curoptidx == 1)
				{
					int i;
					if (SendMessage(QueryWnd[1], CB_GETCURSEL, 0, 0) == 0)
					{
						SendMessage(QueryWnd[1], CB_SETCURSEL, 1, 0);
						for (i=2; i<10; i++)
						{
							ShowWindow(QueryWnd[i], SW_SHOW);
						}
						for (i=1; i<11; i++)
						{
							ShowWindow(tmpstatic[i], SW_SHOW);
						}
					}
					else
					{
						SendMessage(QueryWnd[1], CB_SETCURSEL, 0, 0);
						for (i=2; i<10; i++)
						{
							ShowWindow(QueryWnd[i], SW_HIDE);
						}
						for (i=1; i<11; i++)
						{
							ShowWindow(tmpstatic[i], SW_HIDE);
						}
					}
					return 0;
				}
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				if ((querymode<=1 && --curoptidx < 0) || (querymode>1 && --curoptidx < 1))
				{
					curoptidx = 11;
				}
				if (SendMessage(QueryWnd[1], CB_GETCURSEL, 0, 0)==0)
				{
					if (curoptidx < 10 && curoptidx > 1)
						curoptidx = 1;
				}
				SetFocusChild(QueryWnd[curoptidx]);
				return 0;
			}

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				if (++curoptidx > 11)
				{
					if (querymode <= 1)
						curoptidx = 0;
					else
						curoptidx = 1;
				}
				if (SendMessage(QueryWnd[1], CB_GETCURSEL, 0, 0)==0)
				{
					if (curoptidx >1 && curoptidx < 10)
						curoptidx = 10;
				}
				SetFocusChild(QueryWnd[curoptidx]);
				return 0;
			}

			if((LOWORD(wParam)==SCANCODE_ENTER && curoptidx < 10) || LOWORD(wParam)==SCANCODE_MENU)
			{
				ProcQueryResult(hWnd);
				return 0;
			}
			
			if(LOWORD(wParam)==SCANCODE_F10)
			{
				if(curoptidx != 11)
					ProcQueryResult(hWnd);
				else
					PostMessage(hWnd, MSG_COMMAND, ID_PHOTO_EXIT, 0L);
				return 0;
			}

                        if (LOWORD(wParam)==SCANCODE_BACKSPACE && curoptidx == 0)
                        {
                                char tmpacnobuf[30];
                                GetWindowText(QueryWnd[0], tmpacnobuf, 24);
                                if (tmpacnobuf && (strcmp(tmpacnobuf, LoadStrByID(MID_ALL))==0))
                                {
                                        SetWindowText(QueryWnd[0], "");
                                }
                        }

			break;			

		case MSG_CLOSE:
			//UnloadBitmap(&querybkg);
			DestroyMainWindow(hWnd);
			return 0;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if (nc==EN_CHANGE)
			{
				if (curoptidx > 1 && curoptidx < 10)
					ProcInputValid();
			}
		        switch (id)
			{
			        case ID_PHOTO_SAVE:
					ProcQueryResult(hWnd);
					break;

				case ID_PHOTO_EXIT:
				        PostMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;
			}
			break;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateFindPhotoSetWindow(HWND hWnd, int mode, char* userPIN2) // mode:0查询考勤照片, 1删除考勤照片, 2查询黑名单照牄1?7, 3删除黑名单照牄1?7
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	querymode = mode;
	printf ("querymode %d \n",querymode);
	memset(resacno1, 0, sizeof(resacno1));
	if (userPIN2 && strlen(userPIN2)>0)
	{
		memcpy(resacno1, userPIN2, strlen(userPIN2));	//用户工号
	}

	hWnd = GetMainWindowHandle (hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_QUERY_SET+querymode);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = PhotoQueryWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
        CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

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

