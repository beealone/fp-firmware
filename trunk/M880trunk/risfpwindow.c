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

#define RisFphint 2
#define IDC_TIMER4 9997

typedef struct _RemoteEnrollSession_{
        int Index;
        int Enrolled;
        U32 pin2;
}TRemoteEnrollSession, *PRemoteEnrollSession;

TRemoteEnrollSession es1;
int g_rishintid=HID_PLACEFINGER;

static RECT Hintrisrec={35,190,210,230};
static RECT Hintrisrec2={35,180,190,230};



BITMAP bmp_risbkgnd2;
BITMAP bmp_risok;
BITMAP bmp_risfail;
BITMAP bmp_riswarn;
PSensorBufInfo SensorInforisreg;
int startx1=10,starty1=15,width1=160,high1=20;

BOOL SendImageToRegister(char *buffer, int size, U32 pin2, U8 Index, U16 *result);
void write_bitmap(char *imgout, unsigned char *buffer, int nWidth, int nHeight);

int DrawRisProcessBorder(HDC hdc,HWND hWnd)
{
        Draw3DBorderEx(hdc, hWnd, startx1-2,starty1-2,startx1+width1+1,starty1+high1+2);
        Draw3DBorderEx(hdc, hWnd, startx1, starty1,startx1+width1/4-1,starty1+high1);
        Draw3DBorderEx(hdc, hWnd, startx1+width1/4, starty1,startx1+width1*2/4-1,starty1+high1);
        Draw3DBorderEx(hdc, hWnd, startx1+(width1*2)/4,starty1,startx1+width1-1,starty1+high1);
        Draw3DBorderEx(hdc, hWnd, startx1+(width1*3)/4,starty1,startx1+width1-1,starty1+high1);
	return 0;
}

int DrawRisProcessBar(HDC hdc,HWND hWnd,int times)
{
        SetBrushColor(hdc,COLOR_green);

        switch(times)
        {
		case 0:
                        SetPenColor(hdc,COLOR_darkgray);
                        FillBox(hdc,startx1+1,starty1+1,width1/4-4,high1-3);
                        FillBox(hdc,startx1+width1/4+1,starty1+1,width1/4-4,high1-3);
                        FillBox(hdc,startx1+(width1*2)/4+1,starty1+1,width1/4-4,high1-3);
                        FillBox(hdc,startx1+(width1*3)/4+1,starty1+1,width1/4-4,high1-3);
			break;
                case 1:
                        SetPenColor(hdc,COLOR_darkgray);
                        FillBox(hdc,startx1+1,starty1+1,width1/4-4,high1-3);
                        break;
                case 2:
                        FillBox(hdc,startx1+width1/4+1,starty1+1,width1/4-4,high1-3);
                        break;
                case 3:
                        FillBox(hdc,startx1+(width1*2)/4+1,starty1+1,width1/4-4,high1-3);
                        break;
                case 4:
                        FillBox(hdc,startx1+(width1*3)/4+1,starty1+1,width1/4-4,high1-3);
                        break;
        }
        //EndPaint(hWnd,hdc);
        return 0;
}

static void ShowRisFpHint(HDC dc1, RECT rc, int id, int type)
{
	int tmpvalue = 0;
        tmpvalue = SetBkMode(dc1,BM_TRANSPARENT);
        SetTextColor(dc1,PIXEL_lightwhite);
	if (type==RisFphint)
	        DrawText(dc1,LoadStrByID(g_rishintid),-1,&rc,DT_LEFT);
}

static void ShowRisFpImage(HWND handle)
{
	HDC dc1;
	BITMAP mybmp;
	BLOCKHEAP my_cliprc_heap;
	BOOL ch_inited = FALSE;

	CLIPRGN my_cliprgn1;
	CLIPRGN my_cliprgn2;
	CLIPRGN my_cliprgn3;
	char *tmpbuf;
	int i;

#ifdef URU
	write_bitmap(TMPBMPFILE, SensorInforisreg->DewarpedImgPtr, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);
#else
	char fingerbuf[150*1024];
        memcpy(fingerbuf,SensorInforisreg->DewarpedImgPtr,SensorInforisreg->DewarpedImgLen);
	tmpbuf=MALLOC(gOptions.ZF_WIDTH);
        for(i=0;i<gOptions.ZF_HEIGHT/2;i++)
        {
                memcpy(tmpbuf, fingerbuf+i*gOptions.ZF_WIDTH, gOptions.ZF_WIDTH);
                memcpy(fingerbuf+i*gOptions.ZF_WIDTH,
                       fingerbuf+(gOptions.ZF_HEIGHT-i-1)*gOptions.ZF_WIDTH,
    gOptions.ZF_WIDTH);
                memcpy(fingerbuf+(gOptions.ZF_HEIGHT-i-1)*gOptions.ZF_WIDTH,
                       tmpbuf, gOptions.ZF_WIDTH);
        }
        FREE(tmpbuf);

        write_bitmap(TMPBMPFILE,(unsigned char *) fingerbuf, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);
#endif
	sync();
	LoadBitmap(HDC_SCREEN,&mybmp, TMPBMPFILE);
        dc1=GetClientDC(handle);
	if(!ch_inited)
	{
	        InitFreeClipRectList (&my_cliprc_heap, 100);
	        ch_inited = TRUE;
	}

	InitClipRgn (&my_cliprgn1, &my_cliprc_heap);
	InitClipRgn (&my_cliprgn2, &my_cliprc_heap);
	InitClipRgn (&my_cliprgn3, &my_cliprc_heap);

	InitEllipseRegion (&my_cliprgn2, 85, 110, 68, 90);

	SubtractRegion (&my_cliprgn1, &my_cliprgn1, &my_cliprgn2);
	SelectClipRegion (dc1, &my_cliprgn1);
	OffsetRegion (&my_cliprgn2, 150+gOptions.MainVerifyOffset, 0);
	XorRegion (&my_cliprgn1, &my_cliprgn1, &my_cliprgn2);
	SelectClipRegion (dc1, &my_cliprgn1);

#ifdef URU
	FillBoxWithBitmap(dc1, 150+gOptions.MainVerifyOffset, 0, 180, 200,&mybmp);
#else
#ifdef SECU
    FillBoxWithBitmap(dc1, 150+gOptions.MainVerifyOffset, 0, 180, 200,&mybmp);
#else
	//modified by lyy
//	RotateScaledBitmapVFlip(dc1,&mybmp,90,-80,1,180,200);
    if(gOptions.FpSelect==0 || gOptions.FpSelect==2)
	{
		FillBoxWithBitmap(dc1, 150+gOptions.MainVerifyOffset, 10, 180, 200,&mybmp);
	}
    else
    {
        BITMAP finger;
        LoadBitmapFromFile(HDC_SCREEN,&finger, GetBmpPath("pic3.gif"));
        FillBoxWithBitmap(HDC_SCREEN, 160+gOptions.MainVerifyOffset, 35, 0, 0, &finger);
        UnloadBitmap(&finger);
    }
#endif
#endif

	UnloadBitmap(&mybmp);
        EmptyClipRgn(&my_cliprgn1);
        EmptyClipRgn(&my_cliprgn2);
        EmptyClipRgn(&my_cliprgn3);
        DestroyFreeClipRectList(&my_cliprc_heap);
	ReleaseDC(dc1);

}

static void ProcessRisEnroll(HWND handle)
{
	HDC regdc;
        U16 result;
        char *fingerbuf=NULL;
        int ptrlen;
        char *imgptr;
        U16 zfwidth,zfheight;

        regdc=GetClientDC(handle);
        ptrlen = SensorInforisreg->DewarpedImgLen;
        imgptr = (char *) SensorInforisreg->DewarpedImgPtr;
        fingerbuf=MALLOC(150*1024);
        zfwidth = gOptions.ZF_WIDTH;
        zfheight = gOptions.ZF_HEIGHT;
        ptrlen=gOptions.ZF_WIDTH*gOptions.ZF_HEIGHT;
        ptrlen += 4;
        memcpy(fingerbuf,&zfwidth,2);
        memcpy(fingerbuf+2,&zfheight,2);
        memcpy((BYTE*)fingerbuf+4,SensorInforisreg->DewarpedImgPtr,SensorInforisreg->DewarpedImgLen);
        imgptr = fingerbuf;
	ShowRisFpImage(handle);
        if(SendImageToRegister(imgptr, ptrlen, es1.pin2, es1.Index, &result)&&((result&0xFF)==0))
        {
		es1.Index++;
                if((result>>8)<=1)
                {
                                es1.Enrolled=result>>8;
				if (!es1.Enrolled)
				{
					//enroll success
					g_rishintid=MID_FPOK;
					SendMessage(handle,MSG_PAINT,0,0);
					EnableMsgType(MSG_TYPE_FINGER, 0);
					DelayMS(1000);
					SendMessage (handle, MSG_CLOSE, 0, 0L);
				}
				else
				{
        				es1.Index=1;
				        es1.Enrolled=1;
					g_rishintid=HID_INPUTAGAIN;
					SendMessage(handle,MSG_PAINT,0,0);
					DelayMS(1000);
					g_rishintid=HID_PLACEFINGER;
					SendMessage(handle,MSG_PAINT,0,0);
					DrawRisProcessBorder(regdc,handle);

				}

                }
                else if(es1.Index==2)
		{
			g_rishintid=HID_PLACEFINGER2;
			InvalidateRect(handle,&Hintrisrec,FALSE);
		}
                else if(es1.Index==3)
		{
			g_rishintid=HID_PLACEFINGER3;
			InvalidateRect(handle,&Hintrisrec,FALSE);
		}
                else if(es1.Index==4)
		{
			g_rishintid=HID_PLACEFINGER4;
			InvalidateRect(handle,&Hintrisrec,FALSE);
		}
                else
		{
			g_rishintid=HID_PLACEFINGER;
			InvalidateRect(handle,&Hintrisrec,FALSE);
		}
	}
	else
        {
		DrawRisProcessBorder(regdc,handle);
		g_rishintid=HID_VFFAIL;
		InvalidateRect(handle,&Hintrisrec,FALSE);
                FillBoxWithBitmap(regdc,170+gOptions.MainVerifyOffset,50,0,0,&bmp_risfail);
        }
        if ((es1.Index-1))
        	DrawRisProcessBar(regdc,handle,(es1.Index-1));
        if (fingerbuf)
        	FREE(fingerbuf);
	ReleaseDC(regdc);
}



static int mywinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	//HDC mydc;
	int tmpvalue = 0;
	//int j;
	//MSG msg;
	//TMsg mymsg;
	static char keyupFlag=0;

	switch (message)
	{
		case MSG_CREATE:
	                LoadBitmap (HDC_SCREEN, &bmp_risbkgnd2, GetBmpPath("fp.jpg"));
	                LoadBitmap (HDC_SCREEN, &bmp_risok, GetBmpPath("ok.gif"));
	                LoadBitmap (HDC_SCREEN, &bmp_risfail, GetBmpPath("fail.gif"));
	                LoadBitmap (HDC_SCREEN, &bmp_riswarn, GetBmpPath("warning.gif"));
			UpdateWindow(hWnd,TRUE);
			 SetTimer(hWnd,IDC_TIMER4,1);

			return 0;
		case MSG_PAINT:
			SetMenuTimeOut(time(NULL));
			hdc=BeginPaint(hWnd);
        		tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	                FillBoxWithBitmap(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight,&bmp_risbkgnd2);
	                FillBoxWithBitmap(hdc,0,180,0,0,&bmp_riswarn);
			DrawRisProcessBorder(hdc,hWnd);
			if (g_rishintid==HID_REFINGER)
			{
	                	FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmp_risfail);
			}
			if (g_rishintid==HID_INPUTAGAIN)
			{
	                	FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmp_risfail);
			}
			if ((g_rishintid==MID_FPOK) || (g_rishintid==MID_FPFULL)||
				(g_rishintid==HID_EXCEED))
			{
	                	FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmp_risok);
				DrawRisProcessBar(hdc,hWnd,0);
				ShowRisFpHint(hdc,Hintrisrec2,HID_PLACEFINGER,RisFphint);
			}
			else
				ShowRisFpHint(hdc,Hintrisrec,HID_PLACEFINGER,RisFphint);
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_TYPE_FINGER:
			SensorInforisreg=(PSensorBufInfo)lParam;
			ProcessRisEnroll(hWnd);
			break ;

                case MSG_TIMER:
                        if(wParam==IDC_TIMER4)
                        {
                                if (m)
                                {
                                        SendMessage(hWnd,Fwmsg.Message,Fwmsg.Param1,Fwmsg.Param2);
                                        m=0;
                                }

                        }
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
			if (wParam==SCANCODE_ESCAPE)
			{
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			}
			if (wParam==SCANCODE_ENTER || (LOWORD(wParam)==SCANCODE_F10))
			{
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			}
			if (wParam==SCANCODE_MENU)
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			break;
		case MSG_CLOSE:
                        KillTimer(hWnd,IDC_TIMER4);
			UnloadBitmap(&bmp_risok);
			UnloadBitmap(&bmp_risfail);
			UnloadBitmap(&bmp_risbkgnd2);
			UnloadBitmap(&bmp_riswarn);
			DestroyMainWindow(hWnd);
			return 0;

		case MSG_COMMAND:
			break;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}


int CreateRisFpEnrollWindow(HWND hOwner,int pin2,int isnewreg)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	char buf[100];
	char buf1[100];


        es1.Index=1;
        es1.Enrolled=1;
        es1.pin2=pin2;

	g_rishintid=HID_PLACEFINGER;

	memset(buf,0,100);
	memset(buf1,0,24);
        //Pad0Str(buf1,gOptions.PIN2Width,pin2);
	sprintf(buf1,"%d",pin2);
	if (isnewreg)
        	sprintf(buf,"%s(%s)", LoadStrByID(MID_DATA_EU_FINGER), buf1);
	else
        	sprintf(buf,"%s(%s)", LoadStrByID(HID_ENROLLBACKUP), buf1);

	hOwner = GetMainWindowHandle (hOwner);
	CreateInfo.dwStyle = WS_VISIBLE| WS_CAPTION|WS_BORDER;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = buf;
	CreateInfo.hMenu = 0;
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

        FlushSensorBuffer(); //Clear sensor buffer
	EnableMsgType(MSG_TYPE_FINGER, 1);

	while(GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hMainWnd);
	EnableMsgType(MSG_TYPE_FINGER, 0);
	return 0;
}

