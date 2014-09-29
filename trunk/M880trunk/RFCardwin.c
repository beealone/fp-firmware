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
#include "options.h"
#include "msg.h"
#include "sensor.h"
#include "commu.h"
#include "exfun.h"
#include "ssrcommon.h"
#include "main.h"
#include "serial.h"
#include "rs_485reader.h"

#define IDC_TIMER_CARD 900

char tmpcard[20];
U32 tmpcardvalue;
U32 CardValue;

BITMAP bmpcard_bkg;
BITMAP bmpcard_ok;
BITMAP bmpcard_card;
BITMAP bmpcard_fail;
BITMAP bmpcard_warn;

#define REG_READY	0
#define REG_FAILD	1
#define REG_SUCCESS	2
#define REG_REPEAT	3

int state=REG_READY;
int exitflag=0;

static int Isregisted(BYTE* card)
{
	TUser tmpuser;
	memset(&tmpuser,0,sizeof(TUser));
	tmpuser.PIN = 1;
	if(FDB_GetUserByCard(card,&tmpuser)!=NULL)
		return 1;
	return 0;
}

static int mywinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char tmpstr[50];
	static char keyupFlag=0;
	int tmpvalue = 0;
	switch (message)
	{
		case MSG_CREATE:
		        if(gOptions.IsOnlyRFMachine)
		                LoadBitmap(HDC_SCREEN,&bmpcard_bkg, GetBmpPath("card_bkg.jpg"));
		        else
		                LoadBitmap(HDC_SCREEN,&bmpcard_bkg, GetBmpPath("fp.jpg"));
	                LoadBitmap (HDC_SCREEN, &bmpcard_ok, GetBmpPath("ok.gif"));
	                LoadBitmap (HDC_SCREEN, &bmpcard_fail, GetBmpPath("fail.gif"));
	                LoadBitmap (HDC_SCREEN, &bmpcard_card, GetBmpPath("card.gif"));
	                LoadBitmap (HDC_SCREEN, &bmpcard_warn, GetBmpPath("warning.gif"));

			memset(tmpcard,0,sizeof(tmpcard));
			tmpcardvalue=0;
			CardValue=0;
			state=REG_READY;

			UpdateWindow(hWnd,TRUE);
			SetTimer(hWnd,IDC_TIMER_CARD,1);
			return 0;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);

			EnableMsgType(MSG_TYPE_HID,0);		//显示前停止读卡
			EnableMsgType(MSG_TYPE_ICLASS,0);
			EnableMsgType(MSG_TYPE_MF,0);

			if(gOptions.IsOnlyRFMachine)
		                FillBoxWithBitmap(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight-20,&bmpcard_bkg);
			else
		                FillBoxWithBitmap(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight,&bmpcard_bkg);
	                FillBoxWithBitmap(hdc,0,180,0,0,&bmpcard_warn);

                        //show card number
                        tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
			SetTextColor(hdc,PIXEL_lightwhite);

                        memset(tmpstr,0,sizeof(tmpstr));

                        switch(state)
                        {
				case REG_READY:
					FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmpcard_card);
					TextOut(hdc,35,180,LoadStrByID(MID_PLACE_CARD));
					if(CardValue)
					{	
	                                        sprintf(tmpstr,"%s%010u",LoadStrByID(MID_CARD_NUM),ntohl(CardValue));
        	                                TextOut(hdc,10,40,tmpstr);
						TextOut(hdc,35,196,LoadStrByID(MID_CARD_OPT));
					}
					break;

                                case REG_FAILD:
                                        FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmpcard_fail);
                                        TextOut(hdc,35,180,LoadStrByID(MID_ERROR_CARD));

					DelayMS(1000);
					state = REG_READY;
					InvalidateRect(hWnd,NULL,FALSE);
                                        break;

                                case REG_SUCCESS:
                                        FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmpcard_ok);
                                        sprintf(tmpstr,"%s%010u",LoadStrByID(MID_CARD_NUM),ntohl(CardValue));
                                        TextOut(hdc,10,40,tmpstr);
                                        TextOut(hdc,35,180,LoadStrByID(MID_CARD_SUCC));
					TextOut(hdc,35,196,LoadStrByID(MID_CARD_OPT));
					ttl232.flush_input();

//					DelayMS(1000);
//					state = REG_READY;
//                                      InvalidateRect(hWnd,NULL,FALSE);
                                        break;

                                case REG_REPEAT:
                                        FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmpcard_fail);
                                        sprintf(tmpstr,"%s%010u",LoadStrByID(MID_CARD_NUM),ntohl(CardValue));
                                        TextOut(hdc,10,40,tmpstr);
                                        TextOut(hdc,35,180,LoadStrByID(MID_REGED_CARD));
					DelayMS(1000);
					state = REG_READY;
					InvalidateRect(hWnd,NULL,FALSE);
                                        break;
                        }

			DelayMS(1000);
			EnableMsgType(MSG_TYPE_HID,1);		//显示完成开启读卡
			EnableMsgType(MSG_TYPE_MF,1);
			EnableMsgType(MSG_TYPE_ICLASS,1);
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_TYPE_HID:
			SetMenuTimeOut(time(NULL));

			//dsl 2012.3.27
			if(gOptions.Reader485On && Is485ReaderMaster() && (MSG_INBIO_COMM_CMD == wParam))
			{
				break;
			}

			//get card number
			CardValue = 0;
			memset(tmpcard,0,sizeof(tmpcard));
			memcpy(tmpcard,&lParam,4);
			tmpcardvalue = lParam;

			if(tmpcardvalue)
				state = (Isregisted((BYTE *)tmpcard))?REG_REPEAT:REG_SUCCESS;
			else
				state = REG_FAILD;

			switch(state)
			{
				case REG_FAILD:
//					if(gOptions.VoiceOn)
//						ExPlayVoice(VOICE_NOENROLLED_CARD);
//					else
						ExBeep(1);
					break;
				case REG_SUCCESS:
//					if(gOptions.VoiceOn)
//						ExPlayVoice(VOICE_THANK);
//					else
						ExBeep(1);
					CardValue = tmpcardvalue;
					break;

				case REG_REPEAT:
					if(gOptions.VoiceOn)
					{
						if(gOptions.IsOnlyRFMachine)
							ExPlayVoice(VOICE_REPEAT_FP);
						else
							ExPlayVoice(VOICE_NOENROLLED_CARD);
					}
					else
						ExBeep(1);
					break;
			}
			InvalidateRect(hWnd,NULL,FALSE);
			break ;

                case MSG_TIMER:
                        //scanfp 10ms unit loop
                        if(wParam==IDC_TIMER_CARD)
                        {
				if(m && Fwmsg.Message==MSG_TYPE_HID)
                                {
                                        SendMessage(hWnd,Fwmsg.Message,Fwmsg.Param1,Fwmsg.Param2);
                                        m=0;
                                }
                        }
			break;

		case MSG_KEYUP:
			if(3==gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3==gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
			if(wParam==SCANCODE_ESCAPE)
			{
				exitflag=0;
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			}
			else if((wParam==SCANCODE_MENU || wParam==SCANCODE_ENTER || wParam==SCANCODE_F10) && CardValue)
			{
				exitflag=1;
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			}
			break;

		case MSG_CLOSE:
                        KillTimer(hWnd,IDC_TIMER_CARD);
			UnloadBitmap(&bmpcard_ok);
			UnloadBitmap(&bmpcard_fail);
			UnloadBitmap(&bmpcard_bkg);
			UnloadBitmap(&bmpcard_card);
			UnloadBitmap(&bmpcard_warn);
			DestroyMainWindow(hWnd);
			break;

		case MSG_COMMAND:
			break;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}


int CreateRFCardWindow(HWND hOwner,char *tmpstr, int *value,int Isnewreg)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	EnableMsgType(MSG_TYPE_HID,1);
	EnableMsgType(MSG_TYPE_ICLASS,1);
	EnableMsgType(MSG_TYPE_MF,1);

	hOwner = GetMainWindowHandle (hOwner);

        CreateInfo.dwStyle = WS_VISIBLE| WS_CAPTION|WS_BORDER;
        CreateInfo.dwExStyle = WS_EX_NONE;
	if(Isnewreg)
        	CreateInfo.spCaption = LoadStrByID(MID_ENROLL_CARD);
	else
        	CreateInfo.spCaption = LoadStrByID(MID_CHANGE_CARD);

        CreateInfo.hMenu = 0;
        //CreateInfo.hCursor = GetSystemCursor(0);
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = mywinproc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
        CreateInfo.iBkColor = COLOR_lightwhite;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hOwner;

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
	EnableMsgType(MSG_TYPE_HID,0);
	EnableMsgType(MSG_TYPE_ICLASS,0);
	EnableMsgType(MSG_TYPE_MF,0);
	if(exitflag)
	{
		memset(tmpstr,0,sizeof(tmpstr));
		sprintf(tmpstr,"%010u",ntohl(CardValue));
		printf("[%s] cardvalue=%u\ntmpstr=%s\n", __FUNCTION__, CardValue, tmpstr);
		*value=CardValue;
		return 1;
	}
	else
		return 0;
}

