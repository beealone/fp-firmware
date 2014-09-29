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
#include "convert.h"
#include "tftmsgbox.h"
#include "rs_485reader.h"

#ifdef UPEK
#include "upek/upek.h"
#endif

#ifdef _TTS_
#include "tts/tts.h"
#endif

#define Enrollhint 1
#define Fphint 2
#define IDC_TIMER3 999

typedef struct _EnrollSession_{
        BYTE *(Tmps[3]);
        BYTE *GTmp;
        int Index;
        int len;
}TEnrollSession, *PEnrollSession;

TEnrollSession es;
BYTE tmps[3][4096];
char buff[256];
char buf1[24];
int g_hintid=HID_PLACEFINGER;
int enrollfingerid=0;
int isenrollid=0;
static int bakreg=0;
static RECT Hintrec={35,190,210,230};
static RECT Hintrec2={35,180,192,230};

HWND btnsave1,btnexit1,Edtmp,focuswnd;
BITMAP bmp_bkgnd2;
BITMAP bmp_ok;
BITMAP bmp_fail;
BITMAP bmp_warn;
PSensorBufInfo SensorInforeg;
int gpin=0;
static int isenrolling=0;
int newregcount=0;
int totalregcount=0;
int startx=10,starty=15,width=150,high=20;
int Qstartx=10, Qstarty=40, Qwidth=30, Qheight=100;

static RECT rectQualityValue={55, 55, 100, 95};
static RECT rectQualityText={50, 145, 120, 185};

extern int FPEnrollMode;	//是否在线登记
HWND FPEnrollWnd = (HWND)HWND_INVALID;	//zsliu change

int quality[3]={0,0,0};
static int mQuality=0;
#ifdef CHINESE_ONLY
int EnrollFpOKTimer = 0;
#endif

//zsliu add
int enrollfp=0;  //charge the user is enroll fp or press esc out

int DrawProcessBorder(HDC hdc,HWND hWnd)
{
	Draw3DBorderEx(hdc, hWnd, startx-2,starty-2,startx+width+1,starty+high+2);
	Draw3DBorderEx(hdc, hWnd, startx, starty,startx+width/3-1,starty+high);
    Draw3DBorderEx(hdc, hWnd, startx+width/3, starty,startx+width*2/3-1,starty+high);
    Draw3DBorderEx(hdc, hWnd, startx+(width*2)/3,starty,startx+width-1,starty+high);
	return 1;
}

//this function only use for ZKFPV10
int DelFingerFromCache(HANDLE Handle)
{
	if(Handle && gOptions.ZKFPVersion == ZKFPV10)
      		BIOKEY_DB_CLEAR(Handle);

	return 1;
}

static void ShowQualityBorder(HDC hdc, HWND hWnd, int qualityValue)
{
	int i=0,nStep=0;
	int x1,y1,w=Qwidth,h=Qheight/10;
	char tempbuf[32]={0};
	int tmpvalue=0;

 	nStep = qualityValue/10;
	if(nStep==0)
		nStep = 2;

	SetPenColor(hdc,COLOR_darkgray);
	x1 = Qstartx;
	for(i=0;i<nStep;i++)
	{	if(i<3)
			SetBrushColor(hdc,COLOR_red);
		else if(i<6)
			SetBrushColor(hdc,COLOR_yellow);
		else
			SetBrushColor(hdc,COLOR_green);

		y1 = Qstarty + (Qheight - h*i) + 16 ;
		FillBox(hdc, x1, y1, w, h-4);
	}

	SetBrushColor(hdc,COLOR_darkgray);
	SetPenColor(hdc,COLOR_darkgray);
	for(i=nStep;i<10;i++)
	{
		y1 = Qstarty + (Qheight - h*i) + 16 ;
		FillBox(hdc, x1,y1,w, h-4);
	}

	sprintf(tempbuf, "%d", qualityValue);
    	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
    	SetTextColor(hdc, PIXEL_lightwhite);
	DrawText(hdc, tempbuf, -1, &rectQualityValue, DT_LEFT);

	sprintf(tempbuf, "%s", LoadStrByID(HID_QUALITY));
	DrawText(hdc,tempbuf,-1,&rectQualityText,DT_LEFT);
}

int DrawProcessBar(HDC hdc,HWND hWnd,int times)
{
        SetBrushColor(hdc,COLOR_green);

        switch(times)
        {
		case 0:
                        SetPenColor(hdc,COLOR_darkgray);
                        FillBox(hdc,startx+1,starty+1,width/3-4,high-3);
                        FillBox(hdc,startx+width/3+1,starty+1,width/3-4,high-3);
                        FillBox(hdc,startx+(width*2)/3+1,starty+1,width/3-4,high-3);
			break;
                case 1:
                        SetPenColor(hdc,COLOR_darkgray);
                        FillBox(hdc,startx+1,starty+1,width/3-4,high-3);
                        break;
                case 2:
                        FillBox(hdc,startx+width/3+1,starty+1,width/3-4,high-3);
                        break;
                case 3:
                        FillBox(hdc,startx+(width*2)/3+1,starty+1,width/3-4,high-3);
                        break;
        }
        return 0;
}

static void ShowFpHint(HDC dc1, RECT rc, int id, int type)
{
	int tmpvalue=0;
	tmpvalue = SetBkMode(dc1,BM_TRANSPARENT);
    SetTextColor(dc1,PIXEL_lightwhite);
	if (type==Fphint)
	    DrawText(dc1,LoadStrByID(g_hintid),-1,&rc,DT_LEFT | DT_WORDBREAK);
}

static void ShowFpImage(HWND handle)
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

	char fingerbuf[150*1024];
	memcpy(fingerbuf,SensorInforeg->DewarpedImgPtr,SensorInforeg->DewarpedImgLen);

	tmpbuf=MALLOC(gOptions.ZF_WIDTH);
	for(i=0;i<gOptions.ZF_HEIGHT/2;i++)
	{
		memcpy(tmpbuf, fingerbuf+i*gOptions.ZF_WIDTH, gOptions.ZF_WIDTH);
		memcpy(fingerbuf+i*gOptions.ZF_WIDTH,
		fingerbuf+(gOptions.ZF_HEIGHT-i-1)*gOptions.ZF_WIDTH,gOptions.ZF_WIDTH);
		memcpy(fingerbuf+(gOptions.ZF_HEIGHT-i-1)*gOptions.ZF_WIDTH, tmpbuf, gOptions.ZF_WIDTH);
	}
	FREE(tmpbuf);

	write_bitmap(TMPBMPFILE, (unsigned char *)fingerbuf, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);

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

	UnloadBitmap(&mybmp);
	EmptyClipRgn(&my_cliprgn1);
	EmptyClipRgn(&my_cliprgn2);
	EmptyClipRgn(&my_cliprgn3);
	DestroyFreeClipRectList(&my_cliprc_heap);
	ReleaseDC(dc1);
}

static void ProcessEnroll(HWND handle)
{
	/*dsl 2011.4.28*/
	ExBeep(1);

	HDC regdc;
	int qlt=0;
	int PurposeMode=EXTRACT_FOR_IDENTIFICATION;
	PEnrollSession es1=&es;
	int tmpextract;

	regdc=GetClientDC(handle);
	ShowFpImage(handle);
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		PurposeMode= PurposeMode | (enrollfingerid<<16);
	}

	tmpextract=BIOKEY_EXTRACT(fhdl, SensorInforeg->DewarpedImgPtr,es1->Tmps[es1->Index], EXTRACT_FOR_IDENTIFICATION);

	if (BIOKEY_GETLASTQUALITYEx)
	{
  		mQuality = BIOKEY_GETLASTQUALITYEx(fhdl);
		//quality is show or not
		ShowQualityBorder(regdc,handle, mQuality);
		InvalidateRect(handle,&rectQualityValue,FALSE);
	}

	if(tmpextract)
	{
		qlt=100;

		if(++(es1->Index)>=gOptions.EnrollCount)
		{
			g_hintid=MID_ENROLLING;
			SendMessage(handle,MSG_PAINT,0,0);
			es1->len=BIOKEY_GENTEMPLATE(fhdl, es1->Tmps, gOptions.EnrollCount, es1->GTmp);

			if(es1->len>0)
			{
				int result, score=55;

				g_hintid=MID_ENROLLING;
				SendMessage(handle,MSG_PAINT,0,0);

				if(BIOKEY_IDENTIFYTEMP(fhdl, (BYTE*)es1->GTmp, &result, &score))
				{
					extern int FPEnrollMode;
					if(gOptions.VoiceOn) 
					{
#ifdef _TTS_
						TTS_Wait();
#endif						
						ExPlayVoice(VOICE_REPEAT_FP);
					}
					es1->len = 0;
					es1->Index = 0;
					g_hintid=HID_REFINGER;
					SendMessage(handle,MSG_PAINT,0,0);
					DelayMS(1000);
					g_hintid=HID_PLACEFINGER;
					mQuality = 0;
					SendMessage(handle,MSG_PAINT,0,0);
					DrawProcessBorder(regdc,handle);
					qlt=0;
					if(FPEnrollMode == ENROLLMODE_ONLINE)
                                        	SendMessage(handle,MSG_CLOSE,0,0);
				}
				else
				{
					//enroll success
					EnableMsgType(MSG_TYPE_FINGER, 0);
					usertmplen[enrollfingerid]=es.len;
					userisenroll[enrollfingerid]=1;
					//printf("BIOKEY_ADD,TID=%d\n",gpin|(enrollfingerid<<16));
					BIOKEY_DB_ADD(fhdl, gpin|(enrollfingerid<<16), es.len-6, (BYTE*)es1->GTmp);

					enrollfp=1;	//enroll is sucessful, will save user fp
					if (BIOKEY_GETLASTQUALITYEx)
						mQuality = BIOKEY_GETLASTQUALITYEx(fhdl);

					isfpdbload=1;

#ifdef _TTS_
					if(gOptions.TTS_KEY)
					{
						TTS_Wait();
						ExPlayVoice(TTS_ENROLLOK_VOICE);
					}
#endif			
					while (userisenroll[++enrollfingerid] && (enrollfingerid <gOptions.MaxUserFingerCount))
						;

					if (enrollfingerid>(gOptions.MaxUserFingerCount-1))
					{
						g_hintid=MID_FPFULL;
						SendMessage(handle,MSG_PAINT,0,0);
					}
					else
					{
						newregcount++;
						if ((newregcount+totalregcount)>=(gOptions.MaxFingerCount*100))
						{
							g_hintid=HID_EXCEED;
							SendMessage(handle,MSG_PAINT,0,0);
						}
						else
						{
							isenrolling=1;
							g_hintid=MID_FPOK;
#ifdef CHINESE_ONLY
#ifdef ZEM600
							EnrollFpOKTimer = 60;
#else
							EnrollFpOKTimer = 30;
#endif
#endif
							SendMessage(handle,MSG_PAINT,0,0);
						}
					}
				}
			}
			else
			{
				if(gOptions.VoiceOn)
				{
#ifdef _TTS_
					TTS_Wait();
#endif	
					ExPlayVoice(4);
				}		
				es1->len = 0;
				es1->Index = 0;

				g_hintid=HID_INPUTAGAIN;
				SendMessage(handle,MSG_PAINT,0,0);
				DelayMS(1000);
				g_hintid=HID_PLACEFINGER;
				SendMessage(handle,MSG_PAINT,0,0);
				DrawProcessBorder(regdc,handle);
				qlt=0;

				if(FPEnrollMode == ENROLLMODE_ONLINE)
				{
					SendMessage(handle,MSG_CLOSE,0,0);
				}
			}

		}
		else if(es1->Index==1)
		{
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				TTS_Wait();
				ExPlayVoice(TTS_PRESSFP1_VOICE);
			}
#endif						
			g_hintid=HID_PLACEFINGER2;
			InvalidateRect(handle,&Hintrec,FALSE);
		}
		else if(es1->Index==2)
		{
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				TTS_Wait();
				ExPlayVoice(TTS_PRESSFP2_VOICE);
			}
#endif			
			g_hintid=HID_PLACEFINGER3;
			InvalidateRect(handle,&Hintrec,FALSE);
		}
		else
		{
			g_hintid=HID_PLACEFINGER;
			InvalidateRect(handle,&Hintrec,FALSE);
		}
		if (es1->Index)
		{
			DrawProcessBar(regdc,handle,es1->Index);
		}
	}
	else
	{
		DrawProcessBorder(regdc,handle);
		g_hintid=HID_VFFAIL;
		InvalidateRect(handle,&Hintrec,FALSE);
		FillBoxWithBitmap(regdc,170+gOptions.MainVerifyOffset,50,0,0,&bmp_fail);
	}

	CheckSessionSend(EF_FPFTR, (void*)&qlt, 1);
	ReleaseDC(regdc);
}

static void processgoonreg(HWND hWnd)
{
	char buf[100];

	if (isenrolling)
	{
        es.Index=0;
		es.len=0;
		es.GTmp=(BYTE*)usertmp[enrollfingerid];
		isenrolling=0;
        FlushSensorBuffer(); //Clear sensor buffer
		EnableMsgType(MSG_TYPE_FINGER, 1);
		g_hintid=HID_PLACEFINGER;
		if (bakreg)
        		sprintf(buf,"%s(%s-%d)", LoadStrByID(MID_DATA_EU_FINGER),buf1, enrollfingerid);
		else
        		sprintf(buf,"%s(%s-%d)", LoadStrByID(HID_ENROLLBACKUP),buf1, enrollfingerid);
		SetWindowCaption(hWnd,buf);
		mQuality = 0;
		SendMessage(hWnd,MSG_PAINT,0,0);
	}
}

static int mywinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	static char keyupFlag=0;
	int tmpvalue = 0;
	switch (message)
	{
		case MSG_CREATE:
	        LoadBitmap (HDC_SCREEN, &bmp_bkgnd2, GetBmpPath("fp.jpg"));
	        LoadBitmap (HDC_SCREEN, &bmp_ok, GetBmpPath("ok.gif"));
	        LoadBitmap (HDC_SCREEN, &bmp_fail, GetBmpPath("fail.gif"));
	        LoadBitmap (HDC_SCREEN, &bmp_warn, GetBmpPath("warning.gif"));
			UpdateWindow(hWnd,TRUE);
			SetTimer(hWnd,IDC_TIMER3,1);
#ifdef _TTS_
			if(gOptions.TTS_ENROLL)
			{
				TTS_Wait();
				ExPlayVoice(TTS_PRESSFP_VOICE);
			}
#endif			
			return 0;

		case MSG_PAINT:
			SetMenuTimeOut(time(NULL));
			hdc=BeginPaint(hWnd);
        		tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	        	FillBoxWithBitmap(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight,&bmp_bkgnd2);
	        	FillBoxWithBitmap(hdc,0,180+gOptions.MainVerifyOffset,0,0,&bmp_warn);
			DrawProcessBorder(hdc, hWnd);
			if(BIOKEY_GETLASTQUALITYEx)
			{
				ShowQualityBorder(hdc, hWnd, mQuality);
			}
			if (g_hintid==HID_REFINGER)
			{
	            		FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmp_fail);
			}
			if (g_hintid==HID_INPUTAGAIN)
			{
	            		FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmp_fail);
			}
			if ((g_hintid==MID_FPOK) || (g_hintid==MID_FPFULL)||(g_hintid==HID_EXCEED))
			{
	            		FillBoxWithBitmap(hdc,170+gOptions.MainVerifyOffset,50,0,0,&bmp_ok);
				DrawProcessBar(hdc,hWnd,0);
#ifdef CHINESE_ONLY
				if(g_hintid != MID_FPOK)
#endif
					ShowFpHint(hdc,Hintrec2,HID_PLACEFINGER,Fphint);
				if(g_hintid==MID_FPOK)
				{
				}
				if (FPEnrollMode==1)
				{
					EndPaint(hWnd, hdc);
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
					return 0;
				}
			}
			else
				ShowFpHint(hdc,Hintrec,HID_PLACEFINGER,Fphint);
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_TYPE_FINGER:
#ifdef _TTS_
			if(gOptions.TTS_ENROLL)
			{
				TTS_Wait();
				ExPlayVoice(TTS_LEAVEFP_VOICE);
			}
#endif		
			//dsl 2012.3.27
			if(gOptions.Reader485On && Is485ReaderMaster() && (MSG_INBIO_COMM_CMD == wParam))
			{
				break;
			}

			SensorInforeg=(PSensorBufInfo)lParam;
			ProcessEnroll(hWnd);
			break ;
		case MSG_TIMER:
	    		//scanfp 10ms unit loop
			if(wParam==IDC_TIMER3)
	    		{
				if(m)
				{
					SendMessage(hWnd,Fwmsg.Message,Fwmsg.Param1,Fwmsg.Param2);
					m=0;
				}
#ifdef CHINESE_ONLY
				if(EnrollFpOKTimer)
				{
					if(!--EnrollFpOKTimer)
					{
						SendMessage (hWnd, MSG_CLOSE, 0, 0L);
					}
				}
#endif
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
				if(1 == keyupFlag)
					keyupFlag=0;
				else
					break;
			}
			if (wParam==SCANCODE_ESCAPE)	//1
			{
				//if(enrollfp==1 && fhdl)
				//{
				//	enrollfp=0;     //press ESC will exit and not save
				//	BIOKEY_DB_CLEAR(fhdl);
				//}
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			}
			if (wParam==SCANCODE_ENTER || (LOWORD(wParam)==SCANCODE_F10))
			{
				processgoonreg(hWnd);
			}
			if (wParam==SCANCODE_MENU)	//127
			{
				SendMessage (hWnd, MSG_CLOSE, 0, 0L);
			}
			break;
		case MSG_CLOSE:
			KillTimer(hWnd,IDC_TIMER3);
#ifdef CHINESE_ONLY
			EnrollFpOKTimer = 0;
#endif
			if(ismenutimeout )			
			{
				DelFingerFromCache(fhdl);
			}

			UnloadBitmap(&bmp_ok);
			UnloadBitmap(&bmp_fail);
			UnloadBitmap(&bmp_bkgnd2);
			UnloadBitmap(&bmp_warn);

			BIOKEY_MATCHINGPARAM1(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
			DestroyMainWindow(hWnd);
			//return 0;
			//printf("zsliu return enrollfp=%d\n", enrollfp);
			return enrollfp;

		case MSG_COMMAND:
			break;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateFpEnrollWindow(HWND hOwner,char *tmp, int *len,int fingerid,char *pin2,int Isnewreg)
{
	enrollfp=0; //zsliu init this flag

	MSG msg;
	MAINWINCREATE CreateInfo;

	char buf[100];
	TUser enrolluser;
	if (!fhdl) 
	{
		printf("Warning: create fp enroll window fhd1 is null\n");
		return 0;
	}

	mQuality = 0;
	newregcount=0;
	totalregcount=FDB_CntTmp();
	memset(&enrolluser,0,sizeof(TUser));
	enrolluser.PIN=1;
	FDB_GetUserByCharPIN2(pin2,&enrolluser);
	gpin=enrolluser.PIN;
	//printf("gpin %d, fingerid %d, Isnewreg %d \n",gpin,fingerid, Isnewreg);

	isenrolling=0;
	bakreg=Isnewreg;
	g_hintid=HID_PLACEFINGER;
        es.Index=0;
        es.len=0;
        es.Tmps[0]=tmps[0];es.Tmps[1]=tmps[1];es.Tmps[2]=tmps[2];
        es.GTmp=(BYTE*)tmp;
	es.GTmp=(BYTE*)usertmp[fingerid];
	enrollfingerid=fingerid;
	isenrollid=fingerid;

	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	sprintf(buf1,"%s",pin2);
	if (Isnewreg)
        	sprintf(buf,"%s(%s-%d)", LoadStrByID(MID_DATA_EU_FINGER), buf1, fingerid);
	else
        	sprintf(buf,"%s(%s-%d)", LoadStrByID(HID_ENROLLBACKUP), buf1, fingerid);

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

	FPEnrollWnd = CreateMainWindow(&CreateInfo);
	if (FPEnrollWnd == HWND_INVALID)
		return -1;
	ShowWindow(FPEnrollWnd, SW_SHOWNORMAL);
	FlushSensorBuffer(); //Clear sensor buffer
	BIOKEY_MATCHINGPARAM1(fhdl, SPEED_LOW, gOptions.EThreshold);

	EnableMsgType(MSG_TYPE_FINGER, 1);

	while(GetMessage(&msg,FPEnrollWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(FPEnrollWnd);

	EnableMsgType(MSG_TYPE_FINGER, 0);
	//zsliu add
	FPEnrollWnd = (HWND)HWND_INVALID;

	//return 0;
	return enrollfp;
}

