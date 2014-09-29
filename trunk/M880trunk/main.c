/************************************************

  ZEM 300 iClock-888

  main.c Main source file

  Copyright (C) 2006-2010, ZKSoftware Inc.
 *************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "arca.h"
#include "serial.h"
#include "exfun.h"
#include "msg.h"
#include "flashdb.h"
#include "finger.h"
#include "zkfp.h"
#include "net.h"
#include "netspeed.h"
#include "utils.h"
#include "options.h"
#include "zlg500b.h"
#include "commu.h"
#include "tempdb.h"
#include "rtc.h"
#include "kb.h"
#include "rs232comm.h"
#include "convert.h"
#include "flash.h"
#include "sensor.h"
#include "wiegand.h"
#include "usb_helper.h"
#include "iclsrw.h"
#include "rs_485reader.h"
#include "camerafun.h"

#define _ZKWEBFUNC
#include "webinterface/webinterface.h"
#ifdef ZEM600
#define _HAVE_TYPE_BYTE
#define _HAVE_TYPE_DWORD
#define _HAVE_TYPE_WORD
#endif
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <minigui/tftmullan.h>

#include "pushapi.h"
#include "ssrpub.h"
#include "main.h"
#include "ssrcommon.h"
#include "updatepic.h"
#include "libcam.h"
#include "tftmsgbox.h"
#include "truncdate.h"
#include "rs.h"
#include "locale.h"
#include "wav.h"

#include "modem/modem.h"
#include "modem/ssrmodem.h"
#include "wifi/ssrwireless.h"

#ifdef UPEK
#include "upek/upek.h"
#endif

#ifdef _TTS_
#include "tts/tts.h"
#include "tts/ttsid.h"
#endif

#ifdef MEM_DEBUG 
void *pmalloc;
U32 malloci = 0;
U32 realloci = 0;
#endif

int iclockState=-1;
int i353 = 0;//add zxz 2012-09-14

/*dsl 2012.5.5*/
static BITMAP gSubMenuBGPic;

//BITMAP statusbmp;
//static BITMAP wfinfobmp[6];

//仅仅支持中文的时候需要包含此文件，这个针对国内不支持多国语言的机器
#ifdef CHINESE_ONLY
#include "zk_chineseLng.h"
#endif

RS g_ralist;
extern int fdTransaction;

#ifdef FACE
#include "facedb.h"
#endif

int gCurCaptureCount=0;	//current capture picture count, for count capture

/*minigui use*/
static struct _tag_Desktop
{
	PLOGFONT logfont;
	Clock clock;
	BITMAP bmp_bkgnd;
	BITMAP bmp_bar;
	BITMAP bmp_logo;
	char logostr[10][16];
	unsigned short logo;
	unsigned short waitsec;
	unsigned long flage;
	char KeyState[32];	//工作状态名称
	char Weather[256];
}gDesktop;

struct _tag_FpInput
{
	FpImage Image;
	long WaitSec;
	long IndexEnroll;
}
gFpInput;

struct _tag_KeyInput
{
	char Buffer[MAXKEYSIZE];
	unsigned long WORKCODE;
	unsigned long PIN;
}
gKeyInput;

int curWorkCode=0;			//当前的工作号码
char ownwkcd[MAX_WORKCODE_LEN+1];	//自定义工作号码
int bStateChgTime=0;			//手动切换工作状态后的持续时间(分钟)
int wkcdwinparam = 0;			//liming

time_t LastTime=-1;			//最后一次验证通过时间
int LastUID=-1;				//最后一次验证通过的用户ID

//workcode.
int alarmvol=0;
int alarmtimes=0;

//show sms
int bsmscount=0;				//当前有效公共短消息数量
int asmsflag=1;				//公共短消息标志
int smsindex=0;				//公共短消息序号
int alarmflag=0,alarmindex=0;

//static RECT gDesktop_State_rc={5,218,95,235};
static RECT gDesktop_State_rc={5,218,125,235};
static RECT gDesktop_Clock_rc={110,218,315,235};
static RECT gDesktop_SMS_rc={5,40,315,210};
char firstkeyValue[2];			//按键值

int gsecu_match_score;  //it's for secu sensor


#define ALARMSTRIPTAG 0x10000
int gAlarmStrip=0;                                              //拆机报警已经起动>=ALARMSTRIPTAG
int gAlarmDelayIndex=0;
int gAlarmDelay=0;
int gExtAlarmDelay=0;						//外接响铃延时
int gAuxOutDelay=0;
int gDoorSensorDelay=0;						//门打开超时报警计数
int gCloseDoorDelay=0;						//
int WaitDuressAlarm=0;						//胁迫报警发生后延迟产生报警信号的时间

int gErrTimes=0; //press finger 5 times trigger alarm
int gErrTimesCLearDelayCnt=0;

int bDoorBreak=0;						//门被意外打开标志
int bDoorOpen=0;						//门正常打开标志
int bDoorClose=0;						//门已关闭标志
int resetDoorflag=0;						//清除状态显示计时

int gLockForceAction=0; 	//锁强制动作
int gLockForceState=0;          //当前锁状态

PSensorBufInfo SensorInfo1;
int gLocalCorrectionImage=FALSE;

BITMAP mbmpok;
BITMAP mbmpfail;
BITMAP mbmplock;
BITMAP mbmpcard;
BITMAP mbmppwd;
BITMAP mbmpbk;
BITMAP mbmphint;
BITMAP mbmpusb;

static BITMAP smsicon;
static BITMAP mybmp8;
static BITMAP onebmp;
static BITMAP twobmp;
static BITMAP threebmp;
static BITMAP shutdownbmp;
static BITMAP arrowbmp;
static BITMAP pcbmp;
static BITMAP devbmp;
static BITMAP myalarm;
//add by jazzy 2008.12.17 for battery infomation
static BITMAP bat0,bat1,bat2,bat3,bat4,bat5,bat6;

int VerifiedPIN;		//通过验证的用户

BOOL shutdownbool=TRUE;
BOOL workingbool;
int shutflag;
int ledbool=TRUE;

unsigned char DevNumber[11];	//机具编号
int LastLogsCount=0;
PRTLogNode rtloglist;
PRTLogNode rtlogheader;
//#define MAXRTLOGSCACHE 32       		//max count for real time logs
#define MAXRTLOGSCACHE 25       		//max count for real time logs
#define TIMEOUT_POWER_OFF               3       //电源按钮按下后等待延迟的时间

int DelayTriggerDuress=0;                       //按键触发胁迫报警的有效时间
int gRTLogListCount=0;
time_t cardtime1 = 0;
time_t cardtime2 = 0;
int testetherret=0;		//测试网线连接断开状态
int testethertime=0;	//测试网络连接时间间隔，多少时间测试一次
int etherenable=0;		//断线之后的发呆机制，延迟3秒钟恢复界面
int enabledevicetime=0;		//设备显示通讯中倒计时。每次发送数据都会刷新这个时间
int grepdata = 0;
/*minigui user*/
int gMachineState=STA_IDLE;
PFilterRec gFilterBuf=NULL;
#define TIMEOUT_CLOCKBOX 		(gOptions.ClockTime)
#define TIMEOUT_SHOWSTATE               (gOptions.TimeOutState) //显示临时考勤状态时间

#define DAYLIGHTSAVINGTIME              1                       //夏令时
#define STANDARDTIME    2                                       //非夏令时
extern long SSR_MENU_MAINMENU(HWND);
extern void GPIO_TFT_BACK_LIGHT(BOOL, int);
extern int GPIO_NETWORK_STATE(void);
extern void pushsdk_clean_resource(void);
int gUSB232Connected=0;	//liming
void ShowMainLCD(HWND hWnd);
int InitProc(void);
int EndProc(void);

int gEthOpened=FALSE;			//netware open
int gMFOpened=FALSE;			//mifare card open
int giCLSRWOpened=FALSE;		//iclass card open
int gHIDiClassOpened=FALSE;
int giCLSCardType=CT_16K_16;

int ShowMainLCDDelay=0;			//等待延迟显示主界面时间
int ShowIdleLogoDelay=0;		//等待显示待机屏时间

int WaitInitSensorCount=0;	//Wait n seconds and then init sensor
int WaitSleepCount=0;		//Wait n seconds and then sleep
int sleepflag=0;

int gSetGatewayWaitCount=0;

int WaitShowState=0;
int WaitPowerOff=0;			//按了电源键后等待关机的时间
int PowerSuspend=FALSE;
int FlashRedLED=0;

int AuthServerDNSListInterval=0; // seconds
int AuthServerUploadAttlogInterval=0; //seconds
int AuthServerRefreshUserData=0; //seconds


extern int gExtWGInBitsCount;  	//wiegand bits count
extern int gExtWGInPulseWidth; 	//External wiegand in width of pulse
extern int gExtWGInPulseInterval;

BOOL RTCTimeValidSign=TRUE;

PAlarmRec CurAlarmRec=NULL;

int menuflag=0;		//开启主菜单标志
int menuflag1=0;
int vfwndflag=0;	//开启验证窗口标志
int smsflag=0;		//display sms
int WaitAdminRemainCnt=0;

int enterkeyflag=0;	//长按Enter键标志
int enterkeydown=0;
extern int keykeep;
extern int tftkeylayout;
extern int powerkey;
extern PLOGFONT tftmulfont;
HWND hMainWindowWnd=(HWND)0;
extern HWND hMainMenuWnd;

int showusbmap = 0;
int stkeyflag=0;        //Enter键是否被定义为快捷键标志
int FPEnrollMode = 0;	//是否在线登记指纹

char DHCPIPAdress[18];	//动态分配的IP地址
static void EnableDestopstate(void);
static void ResetLEDDisplay(void);
static int processstarbar(HWND hWnd);
static int procmainstatekey(HWND hWnd,int index1);
void InitRTLoglist();
void POWEROFFSTART(void);
void ProcWorking(HWND hWnd);
int check_battery_voltage(int *voltage);
void connect_wifi(void);
time_t GetLastByAttLog(RS *r,int fd);
int init_rs(RS *r,unsigned long size);
void GetPowerMod();
int CheckLockForceAction(TTime t);
unsigned char  *tftloadfontfile(int lid);
TAttLog *ref_attlog_queue(RS *r);
int isEmpty(RS *r);
extern int ShowMainWindowDelay;
extern void GPIO__USB_Host(BOOL Switch);

TTime gBellTime;



#ifdef IKIOSK
int funkeyadmin=0;
int funkeyflag=0;
#define IDC_TIMER3      0xff02          //用于IKIOSK定制功能，处理快捷进入功能界面的超时
#endif

int ifUseWorkCode=0;
static long LogoCnt=0;

int ImageBufferLength = 0;

#ifdef _TTS_
TTime AlarmSyncTTSpoint;
#endif
//extern PLOGFONT tftmulfont;
int WaitProcessFingerFlag = 0;
extern int TestFlage;

int network_ready();
void resumedisabletime(void)
{
	enabledevicetime=20;
	grepdata=3;
	return;
}

void InitTFTFont()
{
	//中英文，西欧语系不使用ttf
	//所有语言内码使用UTF_8 格式
	//printf("code value:%d--%d-%d-%d\n",'\t','\n','#','\\');
	switch (tftlocaleid)
	{
		case LID_GB2312:
		case LID_ENGLISH:
			{
				gfont=NULL;
				break;
			}
		case LID_CP1250://捷克
		case LID_ISO8859_15://法文
		case LID_CP1251://俄语
			{
				gfont = CreateLogFont ("ttf","arab","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,14, 0);
				gfont1 = CreateLogFont ("ttf","arab","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
				lvfont = CreateLogFont ("ttf","arab","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
				tftmulfont=gfont;
				break;
			}
		case LID_CP1254://土尔其
			{
				gfont = CreateLogFont ("ttf","Verdana","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
				gfont1 = CreateLogFont ("ttf","Verdana","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,10, 0);
				lvfont = CreateLogFont ("ttf","Verdana","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,10, 0);
				tftmulfont=gfont;
				break;
			}
		case LID_BIG5:
			{
				printf("Init BIG5 Font\n");
				gfont = CreateLogFont ("ttf","mingliu","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,15, 0);
				gfont1 = CreateLogFont ("ttf","mingliu","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,13, 0);
				lvfont = CreateLogFont ("ttf","mingliu","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,13, 0);
				tftmulfont=gfont;
				tftloadfontfile(LID_BIG5);
				break;

			}
		case LID_CP1256://阿拉伯
		case LID_CP1255://希伯来文
		case LID_UTF8:
			{
				gfont = CreateLogFont ("ttf","arab","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,14, 0);
				gfont1 = CreateLogFont ("ttf","arab","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,11, 0);
				lvfont = CreateLogFont ("ttf","arab","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,11, 0);
				tftmulfont=gfont;
				break;
			}
		case LID_CP1258: //Vietnamese language 
		case LID_CP874://泰文
			{
				gfont = CreateLogFont ("ttf","thai","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
				gfont1 = CreateLogFont ("ttf","thai","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,10, 0);
				lvfont = CreateLogFont ("ttf","thai","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,10, 0);
				tftmulfont=gfont;
				break;

			}
		case LID_SJIS: //日文
			{
				gfont = CreateLogFont ("ttf","japan","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,15, 0);
				gfont1 = CreateLogFont ("ttf","japan","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,13, 0);
				lvfont = CreateLogFont ("ttf","japan","UTF-8",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_FLIP_NIL,FONT_OTHER_NIL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,13, 0);
				tftmulfont=gfont;
				tftloadfontfile(LID_SJIS);
				break;

			}
		default:
			{
				gfont=NULL;
				break;
			}
	}

}

unsigned long GetLastAttLog(TAttLog **OutBuffer,time_t dt)
{
	TAttLog *Buffer=NULL;
	unsigned long idx=0;
	TAttLog catt;
	TAttLog *att;
	TAttLog *offset;
	TAttLog *queue;

	dt=dt?dt:g_ralist.lasttime;

	if(isEmpty(&g_ralist))
		offset=NULL;
	else
	{
		queue=(TAttLog *)g_ralist.InBuffer;
		offset=&queue[g_ralist.first];
	}

	if((offset && dt<offset->time_second) || dt<g_ralist.lasttime)
	{
		//                printf("----------is database\n");
		Buffer=(TAttLog*)MALLOC(g_ralist.total*sizeof(TAttLog));
		if(Buffer)
		{
			idx=0;
			lseek(fdTransaction,0,SEEK_SET);
			while(read(fdTransaction,&catt,sizeof(TAttLog))==sizeof(TAttLog))
			{
				if(idx>=g_ralist.total)
					break;
				if(catt.time_second>dt)
				{
					memcpy(&Buffer[idx],&catt,sizeof(TAttLog));
					++idx;
				}
			}
			*OutBuffer=Buffer;
			return idx;
		}
	}
	else
	{
		//              printf("---------------is cache\n");
		Buffer=(TAttLog*)MALLOC(g_ralist.total*sizeof(TAttLog));
		if(Buffer)
		{
			idx=0;
			while((att=ref_attlog_queue(&g_ralist))!=NULL)
			{
				if(att->time_second>dt)
				{
					memcpy(&Buffer[idx],att,sizeof(TAttLog));
					++idx;
				}
			}
		}
		*OutBuffer=Buffer;
	}
	return idx;
}

void ClearLastAttLog(void)
{
	memset(g_ralist.InBuffer,0,g_ralist.total*sizeof(TAttLog));
	g_ralist.first=g_ralist.last=0;
}

TTime getMonthDayTime(int MonthDayTime)
{
	TTime tt={0,0,0,1,1,2004,4,0,0};
	tt.tm_year=gCurTime.tm_year;
	tt.tm_min=MonthDayTime & 0xFF;
	tt.tm_hour=MonthDayTime>>8&0xFF;
	tt.tm_mday=MonthDayTime>>16&0xFF;
	tt.tm_mon=(MonthDayTime>>24&0xFF) -1;
	return tt;
}

TTime getConstTime1(void)
{
	TTime tt={0,0,0,1,1,2004,4,0,0};
	tt.tm_year= gCurTime.tm_year;
	tt.tm_sec = 59;
	tt.tm_min = 59;
	tt.tm_hour= 23;
	tt.tm_mday= 31;
	tt.tm_mon = 11;//month(1=0, 2=1, 3=2...)
	return tt;
}
TTime getConstTime2(void)
{
	TTime tt={0,0,0,1,1,2004,4,0,0};
	tt.tm_year= gCurTime.tm_year;
	tt.tm_min = 0;
	tt.tm_hour= 0;
	tt.tm_mday= 1;
	tt.tm_mon = 0;//month(1=0, 2=1, 3=2...)
	return tt;
}
//夏令时处理
int IsDaylightSavingTimeMenu(void)
{
	//dsl 2007.7.14
	TTime DaylightSavingTime,StandardTime, tempTime2, tempTime3;
	int StartMonth = (gOptions.DaylightSavingTime>>24&0xFF)-1;
	int EndMonth = (gOptions.StandardTime>>24&0xFF)-1;
	int iResult = 0;

	DaylightSavingTime=getMonthDayTime(gOptions.DaylightSavingTime);
	StandardTime=getMonthDayTime(gOptions.StandardTime);

	if (StartMonth > EndMonth)
	{
		tempTime2 = getConstTime1();
		tempTime3 = getConstTime2();
		if(TimeDiffSec(gCurTime, DaylightSavingTime)>=0 && TimeDiffSec(gCurTime, tempTime2)<=0)
		{
			iResult = 1;
		}
		else if(TimeDiffSec(gCurTime, tempTime3)>=0 && TimeDiffSec(gCurTime, StandardTime)<0)
		{
			iResult = 1;
		}
	}
	else if(TimeDiffSec(gCurTime, DaylightSavingTime)>=0 && TimeDiffSec(gCurTime, StandardTime)<0)
	{
		iResult = 1;
	}
	return iResult;
}
int IsDaylightSavingTime()
{
	//如果当前时间在夏令开始和结束(即非夏令开始时间)的区间内，则为夏令，否则不是。
	TTime DaylightSavingTime,StandardTime, tempTime, tempTime2, tempTime3;
	int StartMonth = (gOptions.DaylightSavingTime>>24&0xFF)-1;
	int EndMonth = (gOptions.StandardTime>>24&0xFF)-1;
	int iResult = 0, iYear1 = 2004, iYear2 = 2004;

	DaylightSavingTime=getMonthDayTime(gOptions.DaylightSavingTime);
	StandardTime=getMonthDayTime(gOptions.StandardTime);

	if (StartMonth > EndMonth)
	{
		tempTime2 = getConstTime1();
		tempTime3 = getConstTime2();
		if(TimeDiffSec(gCurTime, DaylightSavingTime)>=0 && TimeDiffSec(gCurTime, tempTime2)<=0)
		{
			iResult = 1;
		}
		else if(TimeDiffSec(gCurTime, tempTime3)>=0 && TimeDiffSec(gCurTime, StandardTime)<0)
		{
			iResult = 1;
		}
		if (iResult)
		{
			if (gOptions.CurTimeMode == STANDARDTIME)
			{
				GetTime(&tempTime);
				tempTime.tm_hour+=1;
				iYear1 = tempTime.tm_year;
				time_t tt = EncodeTime(&tempTime);
				DecodeTime(tt, &tempTime);
				iYear2 = tempTime.tm_year;
				if (iYear1 != iYear2)
				{
					StandardTime.tm_year+=1;
				}
				if (TimeDiffSec(tempTime, StandardTime) > 0 && TimeDiffSec(tempTime, StandardTime) <= 3600)
				{
					iResult = 0;
				}
			}
		}
	}
	else if(TimeDiffSec(gCurTime, DaylightSavingTime)>=0 && TimeDiffSec(gCurTime, StandardTime)<0)
	{
		//modify by dsl 2007.5.30
		iResult = 1;
		if (gOptions.CurTimeMode == STANDARDTIME)
		{
			GetTime(&tempTime);
			tempTime.tm_hour+=1;
			if (TimeDiffSec(tempTime, StandardTime) > 0 && TimeDiffSec(tempTime, StandardTime) <= 3600)
			{
				iResult = 0;
			}
		}
	}
	//DBPRINTF("dsl_iResult=%d\n", iResult);
	return iResult;
}

//for logopic
char PicFilePath[80];
char* GetPicPath(char *name)
{
	static char temp[180];
	memset(temp,0,sizeof(temp));
	if (!PicFilePath[0])
		if (!LoadStr("PICFILEPATH", PicFilePath)) memset(PicFilePath, 0, 80);
	sprintf(temp,"%s%s",PicFilePath,name);
	return (char*)temp;
}

int GetNearState(PShortKey pstkey)
{
	int i;
	TSHORTKEY tstkey;
	int autochgtime[7];
	int tmpcurtime;
	int timedistance = 65535;
	int tmpPIN;

	tmpcurtime = gCurTime.tm_hour * 100 + gCurTime.tm_min;
	tmpPIN = 0;
	for(i=0;i<STKEY_COUNT;i++)
	{
		memset(&tstkey,0,sizeof(TSHORTKEY));
		if(FDB_GetShortKey(i+1, &tstkey) != NULL)
		{
			if(tstkey.keyFun==1 && tstkey.autochange)
			{
				autochgtime[0]=tstkey.Time1;
				autochgtime[1]=tstkey.Time2;
				autochgtime[2]=tstkey.Time3;
				autochgtime[3]=tstkey.Time4;
				autochgtime[4]=tstkey.Time5;
				autochgtime[5]=tstkey.Time6;
				autochgtime[6]=tstkey.Time7;
				if(tmpcurtime >= autochgtime[gCurTime.tm_wday])
				{
					if(timedistance > (tmpcurtime-autochgtime[gCurTime.tm_wday]))
					{
						tmpPIN = tstkey.keyID;
						timedistance = tmpcurtime - autochgtime[gCurTime.tm_wday];
					}
				}

			}
		}
	}

	//没有符合条件的状态则默认取第一个
	if (!tmpPIN)
	{
		for(i=0; i<STKEY_COUNT; i++)
		{
			memset(&tstkey, 0, sizeof(TSHORTKEY));
			if(FDB_GetShortKey(i+1, &tstkey)!=NULL && tstkey.keyFun==1)
			{
				tmpPIN = tstkey.keyID;
				break;
			}
		}
	}

	if (tmpPIN)
		FDB_GetShortKey(tmpPIN, pstkey);

	return tmpPIN;
}

void GetNowState(void)
{
	TSHORTKEY curstkey;                     //当前的工作状态

	memset((void*)&curstkey, 0, sizeof(TSHORTKEY));
	memset(gDesktop.KeyState, 0, STATE_NAME_LEN+1);
	if (gOptions.ShowState)
	{
		if (!gOptions.MustChoiceInOut && GetNearState(&curstkey))
		{
			char mynamename[100];           //modify by jazzy 2008.12.02
			memset(mynamename,0,100);
			Str2UTF8(tftlocaleid,(unsigned char *)curstkey.stateName,(unsigned char *) mynamename);

			gOptions.AttState = curstkey.stateCode;
			nmemcpy((BYTE *)gDesktop.KeyState,(BYTE *)mynamename, strlen(mynamename));
		}
		else
		{
			gOptions.AttState = -1;
			sprintf(gDesktop.KeyState, "%s", LoadStrByID(HID_WELCOME));
		}
	}
}

void ProcessAlarm(int index)
{
	FillBoxWithBitmap(HDC_SCREEN,68,12,0,0,&myalarm);
#ifdef _TTS_
	TTS_ExPlayAlarm(index,alarmvol);
#else
	ExPlayAlarm(index,alarmvol);
#endif

//#ifdef ZEM600
	if (index==9)
		DelayMS(4000);
	else if (index%2==1 || index==8)
		DelayMS(2000);
	else
		DelayMS(1000);
//#endif
}

void CloseAlarm(HWND hWnd)
{
	alarmflag=0;
	ShowMainLCD(hWnd);
}

//flag=0 BoradSMS flag=1 UserSMS
static int ProcessSMS(HWND hWnd, U16 flag)
{
	HDC hdc;
	BYTE smsBuffer[512];
	char str[64];
	memset(smsBuffer, 0, sizeof(smsBuffer));

	//zsliu
	gDesktop_SMS_rc.right = gOptions.LCDWidth-5;
	gDesktop_SMS_rc.bottom = gOptions.LCDHeight-30;

	if (gOptions.IsSupportSMS)
	{
		hdc = GetClientDC(hWnd);
		if(!flag)
		{
			FDB_ReadBoardSms(smsBuffer,smsindex);
			if(smsBuffer[0])
			{
				SetPenColor(hdc,COLOR_black);
				Rectangle(hdc,4,4,316,206);
				SetBrushColor(hdc,COLOR_lightwhite);
				FillBox(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight-30);
				memset(str,0,sizeof(str));
				snprintf(str,sizeof(str), "%s", LoadStrByID(842));
				SelectFont(hdc,gDesktop.logfont);
				TextOut(hdc,128,10,str);	//title use text,not picture will.modify by jazzy 2009.02.23

				SelectFont(hdc,gDesktop.logfont);
				SetBkColor(hdc,COLOR_lightwhite);//liming
				SetTextColor(hdc,COLOR_black);
				if(gOptions.TFTKeyLayout == 3)
				{
					Rectangle(hdc,4,4,396,206);
				}
				else
					Rectangle(hdc,4,4,316,206);

				memset(str,0,sizeof(str));
				sprintf(str,"(%d/%d)",smsindex+1,bsmscount);
				TextOut(hdc,220,14,str);
				/*dsl 2012.5.9 fix Traditional Chinese language can not correct display*/
				if (gOptions.Language == 84) {
					char buf[512]={0};
					Str2UTF8(tftlocaleid, smsBuffer, (unsigned char*)buf);
					DrawText(hdc,buf,-1,&gDesktop_SMS_rc,DT_LEFT | DT_WORDBREAK);
				} else {
					DrawText(hdc,(char *)smsBuffer,-1,&gDesktop_SMS_rc,DT_LEFT | DT_WORDBREAK);
				}
				smsflag=1;
			}
			else
			{
				smsflag=0;
				smsindex=0;
			}
		}
		else
		{
			CheckUserSMS((U16)flag, smsBuffer);
			if(smsBuffer[0])
			{
				SetPenColor(hdc,COLOR_black);
				SetBrushColor(hdc,COLOR_lightwhite);
				FillBox(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight-30);
				memset(str,0,sizeof(str));
				snprintf(str,sizeof(str), "%s", LoadStrByID(842));
				SelectFont(hdc,gDesktop.logfont);
				TextOut(hdc,128,10,str);        //title use text,not picture will.modify by jazzy 2009.02.23

				//zsliu change end ... ...
				SelectFont(hdc,gDesktop.logfont);
				SetTextColor(hdc,COLOR_black);
				if(gOptions.TFTKeyLayout == 3)
				{
					Rectangle(hdc,4,4,396,206);
				}
				else
					Rectangle(hdc,4,4,316,206);
				DrawText(hdc,(char *)smsBuffer,-1,&gDesktop_SMS_rc,DT_LEFT | DT_WORDBREAK);
				smsflag=1;
			}
			else
				smsflag=0;
		}
		ReleaseDC(hdc);
	}
	return smsflag;

}
static int ShowUsbState(HWND  hWnd)
{	
	//printf("begin ShowUsbState >>\n");
	static int usbcheckcount = 0;
	static int haveshow = 0;
	int ret = 0;
	if ( usbcheckcount == gOptions.USBCheckTime)
	{	
		ret =  CheckUsbState();
		//printf("CheckUsbState = %d 0000\n",ret);
		if (ret > 0)
		{
			showusbmap = 1;
			if(!haveshow)
			{
				processstarbar(hWnd);
				haveshow = 1;
			}
		}
		else
		{
			showusbmap = 0;
			if(haveshow)
			{
				processstarbar(hWnd);
				haveshow = 0;
			}			
		}
		usbcheckcount = 0;
		//FillBoxWithBitmap(hdc,80,218,0,0,&mbmpusb);	
	}
	else
	{
		usbcheckcount++;
	}
	return ret;

}


/************************
将配置文件中的机具编码转换为下面的形式
"1122334455667788990011"
unsigned char DevNum[11]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0x11};
*************************/
static int Str2Byte(char *strsrc, unsigned char bytedest[])
{
	int i=0;
	char *p=strsrc;
	char tmpstr[2];
	int num=0;
	unsigned char c=0;
	
	if(strsrc==NULL || bytedest==NULL || (strlen(strsrc)!=22))
	{
		return -1;
	}
	for(i=0;i<11;i++)
	{
		strncpy(tmpstr,p,2);
		num=atoi(tmpstr);
		c=(num/10 << 4 | num%10);
	//	memcpy(bytedest[i],c,1);
		bytedest[i]=c;
		p+=2;	
	}
	return 0;
}
static void CloseSMS(HWND hWnd)
{
	asmsflag=1;
	smsflag=0;
	ShowMainLCD(hWnd);
}

void SwitchMsgType(int Enabled)
{
	EnableMsgType(MSG_TYPE_FINGER, Enabled);
	if(gOptions.IsSupportMF && gMFOpened) EnableMsgType(MSG_TYPE_MF,Enabled);
	if(gOptions.IsSupportiCLSRW && giCLSRWOpened) EnableMsgType(MSG_TYPE_ICLASS, Enabled);
	if(gOptions.RFCardFunOn) EnableMsgType(MSG_TYPE_HID,Enabled);
}

extern int iclockProcCheck(pid_t p);
extern int iclockstateflag;
void child_end(int signum)
{
	int child_status;
	pid_t pid;

	while ((pid = waitpid(-1, &child_status, WNOHANG)) > 0) {
		;
	}
}

int GetNoNcState(void)
{
	if(gOptions.LockFunOn&8)
	{
		gLockForceAction=CheckLockForceAction(gCurTime);
		//		DBPRINTF("dsl_ForceAction=%d,gLockForceState=%d\n", gLockForceAction,gLockForceState);
		if(gLockForceAction==FORCEOPEN && gLockForceState!=FORCEOPEN)
		{
			//			DBPRINTF("Force Open\n");
			DoAuxOut(0xFF, 0);
			gLockForceState=FORCEOPEN;
		}
		else if(gLockForceAction==FORCECLOSE && gLockForceState!=FORCECLOSE)
		{
			//			DBPRINTF("Force Close\n");
			DoAuxOut(0,0);
			gLockForceState=FORCECLOSE;
		}
		else if(gLockForceAction==FORCENONE && gLockForceState!=FORCENONE)
		{
			//			DBPRINTF("None Force\n");
			DoAuxOut(0,0);
			gLockForceState=FORCENONE;
		}
	}
	return gLockForceState;
}

//在线登记指纹
extern HWND FPEnrollWnd;
void EnrollAFinger(char *tmp, int *len, int fingerid, char* PIN2, int enrollflag)
{
	//modify by jazzy 2009.01.08
	//彩屏登记在线登记BUG修改.如：用户编号1，指纹索引0，登记成功后，再备份登记指纹索引1，
	//但按指纹时，还是按索引0登记过的指纹，这时提示失败，再修改指纹索引为2，用没有登记过的指纹登记后，在界面一直上显示登记状态条
	int refreshflag = 1;
#ifndef ZEM600
	KillTimer(hMainWindowWnd,IDC_TIMER);
#else
	EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
	SwitchMsgType(0);
	gDesktop.flage = STATE_NORMAL;
	asmsflag=1;
	smsindex=0;
	menuflag=1;
	FPEnrollMode = ENROLLMODE_ONLINE;
	if (FPEnrollWnd!=HWND_INVALID)
	{
		refreshflag = 0;
		SendMessage(FPEnrollWnd, MSG_CLOSE, 0, 0);
	}
	CreateFpEnrollWindow(hMainWindowWnd, tmp, len, fingerid, PIN2, enrollflag);
	FPEnrollMode = ENROLLMODE_STANDALONE;
	menuflag=0;
	if (refreshflag)
	{
#ifndef ZEM600
		SetTimer(hMainWindowWnd,IDC_TIMER,100);
#else
		EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
		menuflag=0;
		FPEnrollMode = ENROLLMODE_STANDALONE;
		SwitchMsgType(1);
		ShowMainLCD(hMainWindowWnd);
		//zsliu
		SendMessage(HWND_DESKTOP,MSG_PAINT,0,0);
		ShowMainLCDDelay=1;

	}
	FlushSensorBuffer();
}

extern HWND MFCardWnd;
int EmptyCard()
{//add by cxp at 2010-04-20
	int refreshflag = 1;
	int ret;
#ifndef ZEM600
	KillTimer(hMainWindowWnd,IDC_TIMER);
#else
	EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
	SwitchMsgType(0);
	gDesktop.flage = STATE_NORMAL;
	asmsflag=1;
	smsindex=0;
	menuflag=1;

	if (MFCardWnd!=HWND_INVALID)
	{
		refreshflag = 0;
		SendMessage(MFCardWnd, MSG_CLOSE, 0, 0);
	}
	ret=CreateMFCardWindow(hMainWindowWnd, 2);
	menuflag=0;
	if (refreshflag)
	{
#ifndef ZEM600
		SetTimer(hMainWindowWnd,IDC_TIMER,100);
#else
		EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
		menuflag=0;
		SwitchMsgType(1);
		ShowMainLCD(hMainWindowWnd);
		//zsliu
		SendMessage(HWND_DESKTOP,MSG_PAINT,0,0);
		ShowMainLCDDelay=1;

	}
	return (ret>1)?1 :0;
}


extern TFPCardOP tmpFPCard;
extern U8 TMPBUF[10240];
//extern  int PackTemplate(U8 *Temp, U8 *Templates[], int TempCount, int ResSize);
extern int PackTemplate(U8 *Temp, U8 *Templates[], int TempCount, int ResSize);
int WriteCard(char *buffer,U32 PIN, int writelen)
{//add by cxp at 2010-04-20
	int refreshflag = 1,fc=0,ret;
	int pin,fingerindex,tmplen,i,newsize;
	char *tmp,tmplate[1024];
	TZKFPTemplate temp;
	U8 tmps[4][1024];

	TBuffer *Buffer = NULL;
	Buffer=(TBuffer *)(buffer);
	tmp=(char*)Buffer->buffer;
#ifndef ZEM600
	KillTimer(hMainWindowWnd,IDC_TIMER);
#else
	EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
	SwitchMsgType(0);
	gDesktop.flage = STATE_NORMAL;
	asmsflag=1;
	smsindex=0;
	menuflag=1;
	if (gOptions.PIN2Width<=5)
	{
		pin=*(WORD*)(Buffer->buffer);
		tmp=tmp+2;

	}
	else
	{
		pin=*(int*)(Buffer->buffer);
		tmp=tmp+4;
	}

	tmpFPCard.Templates=TMPBUF;
	tmpFPCard.OP=OP_WRITE;

	tmpFPCard.PIN=pin;
	memset(tmpFPCard.Finger, 0xFF, 4);
	newsize=3*4;
	for (i=0;i<4;i++)
	{
		tmplen=*(WORD*)(tmp+3*i);
		if (tmplen>0)
		{
			fingerindex=tmp[2+3*i];
			memcpy(tmplate,tmp+newsize,tmplen);
			FDB_CreateTemplate((char*)&temp, (U16)pin, (char)fingerindex, tmplate, tmplen,1);
			memcpy((char *)tmps[i], temp.Template, tmplen);
			tmpFPCard.Finger[fc]=fingerindex;
			fc++;
			newsize+=tmplen;
			//if(fc>=gOptions.RFCardFPC) 
			if(fc > gOptions.RFCardFPC) //add 2012-09-17 
				break;
		}
	}

	if(i>0)
	{
		U8 * t[4];
		t[0]=tmps[0];
		t[1]=tmps[1];
		t[2]=tmps[2];
		t[3]=tmps[3];
		tmpFPCard.TempSize=PackTemplate((U8 *)tmpFPCard.Templates, t, fc, MFGetResSize()-8);
	}
	else
	{
		return 0;
	}

	if (MFCardWnd!=HWND_INVALID)
	{
		refreshflag = 0;
		SendMessage(MFCardWnd, MSG_CLOSE, 0, 0);
	}
	ret=CreateMFCardWindow(hMainWindowWnd, 4);
	menuflag=0;
	if (refreshflag)
	{
#ifndef ZEM600
		SetTimer(hMainWindowWnd,IDC_TIMER,100);
#else
		EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
		menuflag=0;
		SwitchMsgType(1);
		ShowMainLCD(hMainWindowWnd);
		//zsliu
		SendMessage(HWND_DESKTOP,MSG_PAINT,0,0);
		ShowMainLCDDelay=1;

	}
	return (ret>1)?1 :0;
}

void CancelEnrollFinger(void)
{
	if (FPEnrollWnd)
	{
		PostMessage(FPEnrollWnd, MSG_CLOSE, 0, 0);
	}
}

static void webserver_init(void)
{
	WebInterfaceInit();
	ZKWebDebug("Initializing WebInterface....");
	ZKWeb(THIS)->Load("/lib/libweb.so");
	int ret = ZKWeb(THIS)->GetFWCB(NULL);
	if(!ret){
		ZKWeb(THIS)->Load("/lib/libwebserver_a.so");
		ret = ZKWeb(THIS)->GetFWCB(NULL);
	}

	if(ret)	{
		ZKWebDebug("Get CallBack Table....");
		ZKWEBCALLBACK->SaveStr=SaveStr;
		ZKWEBCALLBACK->ExAuxOut=ExAuxOut;
		ZKWEBCALLBACK->ExPowerOff=ExPowerOff;
		ZKWEBCALLBACK->FDB_InitDBs=FDB_InitDBs;
		ZKWEBCALLBACK->FDB_ClrUser=FDB_ClrUser;
		ZKWEBCALLBACK->FDB_ClrAdmin=FDB_ClrAdmin;
		ZKWEBCALLBACK->FDB_ClrAttLog=FDB_ClrAttLog;
		ZKWEBCALLBACK->FDB_ClrTmp=FDB_ClrTmp;
		ZKWEBCALLBACK->FDB_CntAdmin=FDB_CntAdmin;
		ZKWEBCALLBACK->FDB_CntUser=FDB_CntUser;
		ZKWEBCALLBACK->FDB_CntAdminUser=FDB_CntAdminUser;
		ZKWEBCALLBACK->FDB_CntPwdUser=FDB_CntPwdUser;
		ZKWEBCALLBACK->FDB_CntTmp=FDB_CntTmp;
		ZKWEBCALLBACK->FDB_CntAttLog=FDB_CntAttLog;
		ZKWEBCALLBACK->FDB_CntOPLog=FDB_CntOPLog;
		ZKWEBCALLBACK->FDB_GetTmpCnt=FDB_GetTmpCnt;
		ZKWEBCALLBACK->FDB_DeleteTmps=FDB_DeleteTmps;
		ZKWEBCALLBACK->FDB_DelUser=FDB_DelUser;
		ZKWEBCALLBACK->FDB_AddUser=FDB_AddUser;
		ZKWEBCALLBACK->FDB_ChgUser=FDB_ChgUser;
		ZKWEBCALLBACK->FDB_GetUser=FDB_GetUser;
		ZKWEBCALLBACK->FDB_CreateTemplate=FDB_CreateTemplate;
		ZKWEBCALLBACK->FDB_AddTmp=FDB_AddTmp;
		ZKWEBCALLBACK->FDB_GetTmp=FDB_GetTmp;
		ZKWEBCALLBACK->FPDBInit=FPDBInit;
		ZKWEBCALLBACK->OldDecodeTime=OldDecodeTime;
		ZKWEBCALLBACK->RebootMachine=RebootMachine;
		ZKWEBCALLBACK->OldEncodeTime=OldEncodeTime;
		ZKWEBCALLBACK->GetNextPIN=GetNextPIN;
		ZKWEBCALLBACK->GetLastAttLog=GetLastAttLog;
		ZKWEBCALLBACK->ClearLastAttLog=ClearLastAttLog;

		//新增加一个回调入口
		ZKWEBCALLBACK->AppendUserTemp= AppendUserTemp;
		ZKWEBCALLBACK->SetRTCClock=SetRTCClock;
		ZKWEBCALLBACK->ReadRTCClockToSyncSys=ReadRTCClockToSyncSys;
		//增加完成
	}
	ZKWebDebug("Start webinterface server..");
	ZKWeb(THIS)->Start();
}

static void wifi_init_and_connect(void)
{
	int ret;
	if(gOptions.IsSupportWIFI == 0){
		return;
	}

	//ret=system("ifconfig rausb0 up"); //open rausb0
	//set_wifi_rausb0_flag(ret); /*dsl 2011.5.4. Tell work state of rausb0 for connect_wifi function */
	//if (ret != EXIT_SUCCESS) {
	//	printf("_____%s__%d\n",__FILE__, __LINE__);
	//	write_tracemsg("ifconfig rausb0 fail");
	//	return;
	//}

	if (gOptions.WifiModule==0) {
		setwifipara();
		ret=systemEx("ifconfig rausb0 up"); //open rausb0
		printf(">>>ifconfig rausb0 up, return:%d\n", ret);
		connect_wifi();
	} else {
		//printf("_____%s__%d\n",__FILE__, __LINE__);
		//ret=systemEx("ifconfig rausb0 up"); //open rausb0
		printf(">>>ifconfig rausb0 up, return:%d\n", ret);
		wireless_load_configuration();
		if(wireless_thread_create() < 0) {
			printf ("create wifi thread sucess\n");
		}
	}
}

static void tts_init(void)
{
#ifdef _TTS_
	//打开TTS，需要关闭WAV语音设备,关闭TTS语音设备

	InitTTSID();
	LoadTTSID("/mnt/mtdblock/tts.cfg");
	printf("----Loading TTS Resource Data.........\n");

	TTS_Init("/mnt/mtdblock/Resource.dat");
	printf("----Load TTS Data finish\n");

	gOptions.TTS_S=LoadInteger("TTS_S",5);
	gOptions.TTS_VERIFY=LoadInteger("TTS_VERIFY",0);
	gOptions.TTS_SMS=LoadInteger("TTS_SMS",0);
	gOptions.TTS_LOGO=LoadInteger("TTS_LOGO",0);
	gOptions.TTS_ENROLL=LoadInteger("TTS_ENROLL",0);
	gOptions.TTS_REC=LoadInteger("TTS_REC",0);
	gOptions.TTS_KEY=LoadInteger("TTS_KEY",0);
	gOptions.TTS_STATE=LoadInteger("TTS_STATE",0);
	gOptions.TTS_TIME=LoadInteger("TTS_TIME",0);
	gOptions.TTS_MENU=LoadInteger("TTS_MENU",0);

	gOptions.TTS_STATE=0;

	TTS_SetS(gOptions.TTS_S);
	TTS_SetChinese();
	TTS_SetVol(AdjTTSVol(gOptions.AudioVol));

	if(gOptions.TTS_LOGO)
	{
		printf("play wav tts logo!!!!!!!!!!\n");
		TTS_ExPlayWav(GetWavPath("logo.wav"));
		TTS_Wait();
		ExPlayVoice(TTS_LOGO_VOICE);

		if(gOptions.VoiceOn)	//解决开机后播放tts logo时按键死机
		{
			sleep(3);
		}
	}
	fprintf(stderr,"TTS start complate\n");
#endif
}

extern int GetIndexNodeID(int sid);
int InitProc(void)
{
	int NewLng;
	HWND hStatusWnd;

	if(!GPIO_IO_Init()) {
		DBPRINTF("GPIO OR FLASH Initialize Error!\n");
	}


	
	gImageBuffer=NULL;	//指纹图像缓冲区
	fhdl=0;			//指纹算法内存句柄
	ClockEnabled = TRUE;	//是否显示时钟":"
	CalibrationTimeBaseValue();
	ShowMainLCDEnabled = TRUE;	 //是否显示主界面FlowControl
	
	//仅仅支持中文的时候需要，这个针对国内不支持多国语言的机器
#ifdef CHINESE_ONLY
#ifdef ZEM600
	//针对zem600的初始化部分
	zk_initARM_RCS();
#else
	//针对zem510的初始化部分
	zk_initMipsel_RCS();
#endif
#endif

	//printf("%x %x %x\n",(U32)gFlash16,GetFlashStartAddress(FLASH_SECTOR_COUNT-2),GetFlashStartAddress(FLASH_SECTOR_COUNT-2)-(U32)gFlash16);
	InitOptions();		//初始化全局参数

//	if(gOptions.VoiceOn) {
		audio_thread_init();
//	}
	//dsl 2012.3.24
	if(gOptions.Reader485On && Is485ReaderMaster())	{
		InitRS485Param();
		InitReaderMem();
	}

	if(gOptions.UserExtendFormat && (gOptions.IsSupportMF || gOptions.IsSupportiCLSRW)) {
		gOptions.OnlyPINCard=0;
	}

	gOptions.TFTKeyLayout = tftkeylayout;	//Select Keyboard layout (0:iclock200/300,1:HIT-1-2,2:iclock400/500,3:iMF4)
	//printf("lyy===== gOptions.TFTKeyLayout %d powerkey %d \n", gOptions.TFTKeyLayout, powerkey);

	//定制功能标志
	IDTFlag = LoadInteger("IDT", 0);	//西班牙定制
	SRLFlag = LoadInteger("SRL", 0);	//罗马尼亚
#ifdef IKIOSK
	ifUseWorkCode = LoadInteger("UseWorkCode", 0);
#else
	ifUseWorkCode = gOptions.WorkCode;
#endif
	//是否使用SDCard
	useSDCardFlag = LoadInteger("USESDCard", 0);

	//Initial COM0 and check connect Mifare reader or not
	if(gOptions.IsSupportMF)
	{
		if((gOptions.IsSupportMF==1)&&(st232.init(115200, V10_8BIT, V10_NONE, FALSE)==0))
		{
			gMFOpened=MFInit(&st232);
		}
		if((gOptions.IsSupportMF==2)&&(ttl232.init(115200, V10_8BIT, V10_NONE, FALSE)==0))
		{
			gMFOpened=MFInit(&ttl232);
		}

		if((gOptions.IsSupportMF==3)&&(ct232.init(115200, V10_8BIT, V10_NONE, FALSE)==0))
		{
			gMFOpened=MFInit(&ct232);
		}

		printf (">>>gMFOpened=%d\n",gMFOpened);
		if (gMFOpened <= 0) {
			write_tracemsg("Mifare open fail");
		}
	}

	//iCLSRW: iCLASS card read and write
	if(gOptions.IsSupportiCLSRW)
	{
		if(gOptions.iCLASSCardType ==0){
			giCLSCardType =CT_16K_16;
		}
		else if(gOptions.iCLASSCardType ==1){
			giCLSCardType =CT_16K_2;
		}
		else if(gOptions.iCLASSCardType ==2){
			giCLSCardType =CT_2K;
		}

		if((gOptions.IsSupportiCLSRW==COM1)&&(st232.init(57600, V10_8BIT, V10_EVEN, FALSE)==0))
		{
			giCLSRWOpened=iCLSInit(&st232);
		}
		if((gOptions.IsSupportiCLSRW==COM2)&&(ttl232.init(57600, V10_8BIT, V10_EVEN, FALSE)==0))
		{
			giCLSRWOpened=iCLSInit(&ttl232);
		}
		if((gOptions.IsSupportiCLSRW==COM3)&&(ct232.init(57600, V10_8BIT, V10_EVEN, FALSE)==0))
		{
			giCLSRWOpened=iCLSInit(&ct232);
		}
		printf("giCLSRWOpened:%d\n",giCLSRWOpened);
		if (giCLSRWOpened <= 0) {
			write_tracemsg("iclass open fail");
		}
	}

	//iClass Reader
	/*dsl 2012.6.13, the parameter "IsSupportHID(~HID)" already abolish, HID card read will parameter "~RFCardOn"*/
#if 0
	if(gOptions.IsSupportHID && (st232.init(57600, V10_8BIT, V10_EVEN, FALSE)==0)) {
		gHIDiClassOpened = TRUE;
		printf("HID card Opened\n");
	}
#endif

	ttl232.init(115200, V10_8BIT, V10_NONE, 0);
	EnableMsgType(MSG_TYPE_BUTTON,1);
	ExSetAuxOutDelay(gOptions.LockOn,gOptions.OpenDoorDelay, gOptions.DoorSensorMode);

	//setting font and language
	printf("Starting ... FONT\n");
#ifdef CHINESE_ONLY
	//仅仅支持中文的时候需要，这个针对国内不支持多国语言的机器
	NewLng=83;
	gOptions.Language=83;
	SaveInteger("Language", gOptions.Language);
	SaveInteger("NewLng", gOptions.Language);
#else	
	NewLng=LoadInteger("NewLng", gOptions.Language);
	if(NewLng!=gOptions.Language)
	{
		if(!NewLng){
			NewLng=gOptions.Language;
		} else {
			gOptions.Language=NewLng;
		}
		SaveInteger("Language", gOptions.Language);
		SaveInteger("NewLng", gOptions.Language);
	}
#endif
	SelectLanguage(gOptions.Language);

	//2007.07.23 修正在英文播放中文语音问题
	tftnewlng = NewLng;
#ifdef CHINESE_ONLY
	tftlocaleid=LID_GB2312;
#else
	tftlocaleid=GetDefaultLocaleID();
#endif
	//NewLng=tftlocaleid;
	//初始化字体
	InitTFTFont();
	if ((gOptions.ShowStyle)|| (tftlocaleid==LID_CP1256))	//modify by jazzy 2008.08.02 if arabic language must transfer
	{
		fromRight=1;   //1－从右向左显示文字
		//printf("show arabic language\n");
	}
	printf("load tft FONT all is successful\n");

	//初始化显示界面的参数，区分是3寸屏或者是3.5寸屏
	initLCDTypeOptions();

	hStatusWnd = createStatusWin1(HWND_DESKTOP , 250 , 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(MID_WAIT));

	//打开背光(解决联机调试时休眠状态下重启程序无法开启显示的问题liming)
	brightness = gOptions.Brightness;
	GPIO_TFT_BACK_LIGHT(TRUE, gOptions.LcdMode);

	//Switch RJ45/RS232
	if(LoadInteger(IsNetSwitch, 0))	{
		Switch_mode(!gOptions.NetworkOn);
	}

	//处理继电器拨动
	if (LoadInteger("Is232Switch",0)==1) {
		//printf("Switch COM Port!\n");
		if(gOptions.RS232On) {
			char mode=0;
			ExCommand(58,&mode,1,10);
		} else if(gOptions.RS485On) {
			char mode=0xff;
			ExCommand(58,&mode,1,10);
		}
	}

	//Setup signal
	if(signal(SIGCHLD, child_end)==SIG_ERR){
		DBPRINTF("REGISTER SIGNAL SIGCHLD FAILED!\n");
	}

	//初始化TCP/IP UDP
	gEthOpened=FALSE;
	if(gOptions.NetworkFunOn)
	{
		gEthOpened = (EthInit()==0);
		printf("gEthOpened:%d, GPIO_NETWORK_STATE():%d\n", gEthOpened, GPIO_NETWORK_STATE());
		
		/*Liaozz 20081007 comment " && (GPIO_NETWORK_STATE() || !gOptions.IsSupportWIFI)" condition
		 *let the network can init whenever if gEthOpened.
		 */
		if (gEthOpened/* && (GPIO_NETWORK_STATE() || !gOptions.IsSupportWIFI)*/)
		{
			//DHCP
			if(gOptions.DHCP)
			{
				//LCDWriteCenterStrID(1, HID_NET_DHCPHINT);
				/*dsl 2011.5.6. To init the device for the factory*/
#ifdef FORFACTORY
				setMACAddress();
#endif
				systemEx("./udhcpc -q -n");
				UpdateNetworkInfoByDHCP("dhcp.txt");
			}

			//SET IP ADDRESS, NETMASK , SPEED
			SetNetworkIP_MASK((BYTE *)gOptions.IPAddress, gOptions.NetMask);
			set_network_speed((BYTE *)NET_DEVICE_NAME, gOptions.HiSpeedNet);
			// DBPRINTF("Setup network speed OK!\n");
			//SET GATEWAY
			SetGateway("add", gOptions.GATEIPAddress);
			ExportProxySetting();
		}
	}

	//初始化SENSOR
	if (!gOptions.IsOnlyRFMachine)	{
		printf("Init  Sensor...\n");
#ifdef UPEK
		char *match_score;
		match_score = LoadStrOld("secu_score");
		if(match_score) {
			gsecu_match_score = atoi(match_score);
		} else {
			gsecu_match_score = 35;//default we set 40
		}
		gOptions.OLeftLine = 0;
		gOptions.OTopLine = 0;
		gOptions.OImageWidth = 260;
		gOptions.OImageHeight = 300;    //ccc

		InitSensor1(gOptions.OLeftLine,gOptions.OTopLine,gOptions.OImageWidth,gOptions.OImageHeight,gOptions.NewFPReader);

		STSetChipState(NOMINAL_MODE);
		gImageBuffer=(char *)malloc(5*gOptions.OImageWidth*gOptions.OImageHeight);
#else
		int imageBufferSize = 0;
		InitSensor(&gOptions.ZF_WIDTH,&gOptions.ZF_HEIGHT,&imageBufferSize);
		printf("w=%d,h=%d, imageBufferSize=%u\n",gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT, imageBufferSize);
		if(imageBufferSize > 0) {
			gImageBuffer=(char *)malloc(imageBufferSize);
			ImageBufferLength = imageBufferSize;
		}
#endif
		SaveOptions(&gOptions);
	}
	InitRTLoglist();
	DBPRINTF("FP Engine initializing....\n");
	//初始化指纹系统和模版
	
	if (gOptions.CameraOpen) {
		DIR *dir;
		char path[100];
		char commandstr[100];

		memset(path,0,100);
		sprintf(path,"%s",GetCapturePath("capture"));
		dir=opendir(path);
		if (!dir){
			sprintf(commandstr,"cd %s/ && mkdir capture && sync", GetCapturePath(""));
			systemEx(commandstr);
			sprintf(commandstr,"cd %s/ && mkdir pass && sync && mkdir bad && sync", path);
			systemEx(commandstr);
			sync();
		} else {
			closedir(dir);
		}
	}
	FDB_InitDBs(TRUE);

	//指纹超过限定的数量时强制为1:1验证方式  2006.11.30
	if (gOptions.ZKFPVersion!=ZKFPV10 && (FDB_CntTmp()>gOptions.LimitFpCount) && (!gOptions.I1ToG))	{
		SaveInteger("Must1To1", 1);
		gOptions.Must1To1 = 1;
	}
#ifdef IKIOSK
	gOptions.UseNewBell=1;
#endif
	//初始化闹铃信息
	if (gOptions.AutoAlarmFunOn){
		if (!gOptions.UseNewBell && FDB_CntAlarm()==0) {
			FDB_InitAlarm();
		} else if (gOptions.UseNewBell && FDB_CntBell()==0) {
			FDB_InitBell();
		}
	}
	//初始化工作状态
	if(gOptions.ShowState && FDB_CntShortKey()==0) {
		FDB_InitShortKey();
	}

	//初始化默认时间段、组及开锁组合信息
	if(gOptions.LockFunOn || gOptions.AttUseTZ) {
		FDB_InitDefaultDoor();
	}

	if (LoadInteger("ChangeLng",0)){
		FDB_ClearData(FCT_SHORTKEY);
		FDB_InitShortKey();
		SaveInteger("ChangeLng", 0);
	}
	//初始化工作状态
	if(gOptions.ShowState && FDB_CntShortKey()==0) {
		FDB_InitShortKey();
	}


	//初始化工作号码
	memset(ownwkcd,0,MAX_WORKCODE_LEN+1);
	//判断ENTER键是否被定义为快捷键(liming)
	if(tftkeylayout==3){
		stkeyflag = EnterIsSTkey(LOWORD(SCANCODE_ENTER));
	}


	if (!gOptions.IsOnlyRFMachine) {
		int ret=FPInit(NULL);
		printf("FPInit result=%d\n", ret);
		if (ret <= 0) {
			write_tracemsg("fingerprint open fail\n");
		}
	} else {
		fhdl=0; //add by caona for IsOnlyRFMachine=1
	}
	//	printf("init zkfp finish\n");
#ifdef UPEK
	//临时添加用以初始化UPEK指纹头
	printf("Calibrate UPEK Sensor, wait...\n");
	int upinit = calibrate();
	printf("UPEK Init:%d\n", upinit);
#endif

	if(gOptions.IsSupportModem>1) {
		gOptions.AuthServerEnabled=ONLY_LOCAL;
	}

	//if(gOptions.IsSupportModem==0){
	//当且仅当gprs时，关闭232和485功能，3G不能关闭232和485功能
	if((gOptions.RS232On || gOptions.RS485On)&&(!gOptions.IsSupportModem || !gOptions.ModemEnable || !(gOptions.ModemModule == 1)))
	{
		if (ff232.init(gOptions.RS232BaudRate, V10_8BIT, V10_NONE, 0)!=0){
			printf("ff232 init fail\n");
		}

		if (gOptions.RS485On){
			RS485_setmode(FALSE);
		}
	}
	//open USB232 function <liming>
	if(gOptions.USB232FunOn && gOptions.USB232On){
		gUSB232Connected=(usb232.init(gOptions.RS232BaudRate, V10_8BIT, V10_NONE, 0)==0);
	} else {
		gOptions.USB232On=0;
	}

	if ((gOptions.LockFunOn & LOCKFUN_DOORSENSOR) || gOptions.IsACWiegand || gOptions.ExtWGInFunOn || gOptions.RFCardFunOn)
	{
		ExOpenWiegand();
	}
	if (gOptions.RFCardFunOn) {
		ExOpenRF();
	}
	ExEnableClock(1);
	ExSetTftKeyflag(0);
	if(powerkey == 67 && gOptions.TFTKeyLayout==3){
		powerkey = 73;	
	}
	if (LoadInteger("~IncardType",0) ||LoadInteger("~Outcardtype",0)) {
		ExSetTftHIDFun();
	}

	ExLightLED(LED_GREEN, FALSE);
	ExLightLED(LED_RED, FALSE);

	//自动关机检测
	if(gOptions.PowerMngFunOn){
		WaitSleepCount=gOptions.IdleMinute*60;
	}

	//空闲时间到后, 关机 = 87 或者 休眠 = 88
	PowerSuspend=(gOptions.IdlePower==HID_SUSPEND);
#ifndef ZEM600
	EnableMsgType(MSG_TYPE_TIMER, 0);
#else
	EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
	EnableMsgType(MSG_TYPE_BUTTON, 1);
	EnableMsgType(MSG_TYPE_FINGER, 1);
	EnableMsgType(MSG_TYPE_MF, 1);
	EnableMsgType(MSG_TYPE_HID, 1);
	EnableMsgType(MSG_TYPE_ICLASS,1);//iCLSRW
	EnableMsgType(MSG_TYPE_DOOR, 1);
	EnableMsgType(MSG_WEBINTERFACE, 1);  /******** Add For Web Server ********/

	//Synchronize Linux system time from RTC clock
	RTCTimeValidSign=ReadRTCClockToSyncSys(&gCurTime);
	GetTime(&gCurTime);
	GetTime(&gMinTime);
	GetTime(&gBellTime);		//只用于读取当前闹铃

	//Check Sms is valid by the valid date and pack SMS table
	if(gOptions.IMEFunOn != 1 || gOptions.TFTKeyLayout==3) {
		//开启自动清除过期短消息功能<liming>
		FDB_CheckSmsByStamp(EncodeTime(&gCurTime));
	}

	//获取公共短消息数量
	if(gOptions.IsSupportSMS){
		bsmscount=FDB_GetBoardNum();
	}

	DoAlarmOff(0);
	GetNoNcState();		//门磁状态
	//无网络连接时关闭后台功能
	if(!gEthOpened){
		gOptions.IsSupportAuthServer = 0;
		gOptions.AutoSyncTimeFunOn = 0;
	}

	if(gOptions.IsSupportAuthServer){
	       	InitAuthServer();
	}

	AuthServerUploadAttlogInterval = 3;
	AuthServerDNSListInterval = gOptions.DNSCheckInterval*60;
	CurAlarmRec=(PAlarmRec)MALLOC(gOptions.MaxUserCount*100*sizeof(TAlarmRec));
	memset(CurAlarmRec, 0, gOptions.MaxUserCount*100*sizeof(TAlarmRec));

	//Filter buf
	if (gOptions.I1ToH || gOptions.I1ToG) {
		gFilterBuf=MALLOC(gOptions.MaxUserCount*100*sizeof(TFilterRec));
		memset(gFilterBuf, 0, gOptions.MaxUserCount*100*sizeof(TFilterRec));
	}

#ifndef ZEM510
#ifndef ZEM600
	UpdateDBforWeb();
#endif
#endif
	//update compressed log data if the extend log format is true
	UpdateUserDb();

	//从考勤记录中获取用户最近进出记录
	LastLogsCount = FetchLastLogs(CurAlarmRec);

	if(gOptions.ShowState || gOptions.ShowCheckIn){
		GetNowState();
	}

	if(gOptions.IsModule){
		gMachineState=STA_IDLE;
	} else {
		gMachineState=STA_VERIFYING;
	}

	//dsl 2012.4.26
	InitCameraLogicParam();
	if (gOptions.CameraOpen){
#ifdef	ZEM600		
		 if (strstr((const char*)get_hardware_info(), "301PLH") > 0) {
                                i353 = 1;
                } else if (strstr((const char*)get_hardware_info(), "VC353") > 0) {
                        i353 = 2;
                }
#endif	
		picid = FDB_GetMaxPhotoIndex();
	}

	gOptions.RSize=LoadInteger("RSize",512);
	init_rs(&g_ralist,gOptions.RSize);
	GetLastByAttLog(&g_ralist,fdTransaction);

	//web server 
#ifndef ZEM600
	if (gOptions.IclockSvrFun == 0) {
		webserver_init();
	}
#else
	webserver_init();
#endif

	//初始化WIFI并创建连接
	wifi_init_and_connect();

	char tmpValue;
	switch(gOptions.keyboardStyle)
	{
		case 0:
			tmpValue=0;
			break;
		case 1:
			tmpValue=1;
			break;
		default:
			tmpValue=0;
			break;
	}
	ExCommand(220, &tmpValue, 1, 10);

	//关掉蜂鸣器
	tmpValue=0;
	ExCommand(56, &tmpValue, 1, 10);

	tts_init();

#ifdef ZEM510
	if(gOptions.USB232FunOn==0 && gOptions.supportLCDType == 0){
		GPIO__USB_Host(1);
	}
#endif
	gCurCaptureCount = FDB_CntPhotoCount();	

#ifdef FACE
	if(gOptions.FaceFunOn) {
		DBPRINTF("Face DB init start....\n");
		FaceDBInit();
	}
#endif
	//add by liq
	char buffer[30];
	memset(buffer, 0x0, 30);
	LoadStr("DevNumber", buffer);
	Str2Byte(buffer,DevNumber); 
	//
	if (gOptions.IclockSvrFun == 1) {
		init_push();
	} 
	
	memset((void*)&gSubMenuBGPic, 0, sizeof(BITMAP));
	if (LoadBitmap(HDC_SCREEN,&gSubMenuBGPic,GetBmpPath("submenubg.jpg"))) {
		/*load submenubg.jpg error*/
		printf("load submenubg.jpg error, exit mainmenu function\n");
	}
	
	printf("MCU Version:%d\n", GetMCUVersion());
	/*
	将检查电池信息放置获取MCU版本之后
	change by zxz 2013-03-26
	*/
	if (gOptions.BatteryInfo) {
		BatteryMod=3;
		BatteryStatus=3;
		GetPowerMod();
	}

/*	
	int i, j;
	TUser u;
	TAttLog log;
	time_t t;
	GetTime(&gCurTime);
	t=OldEncodeTime(&gCurTime);
		for(i=0;i<20000;i++){
			memset(&u, 0, sizeof(TUser));
			u.PIN = i;
			sprintf(u.PIN2, "%05d", i);
			sprintf(u.Name, "测试用户%d", i);
			adduser(&u);
		}

		for(i=0;i<20000;i++){
			memset(&log, 0, sizeof(TAttLog));
			log.PIN = 1;
			log.time_second = t+i ;
			log.verified=(char)101;
			memcpy(log.reserved, &i, 4);
			sprintf(log.PIN2, "%05d", 1);
			log.status=0x01;
			addlog(&log);
		}
*/

	DBPRINTF("Finish InitProc.....\n");
	
	FDB_AddOPLog(0, OP_POWER_ON, 0,0,0,0);
	cardtime1 = time(NULL);
	destroyStatusWin1 (hStatusWnd);
	
	return 0;
}

int EndProc(void)
{
#ifdef _TTS_
	TTS_Stop();
	FreeTTSID();
	TTS_Free();
#endif

	if (gOptions.IsSupportModem > 1 && gOptions.ModemEnable) {
		modem_thread_exit();
	}

	if(gOptions.CameraOpen)	{
		CameraClose();
	}

	//add for push sdk
	if(pushsdk_is_running()) {
		pushsdk_clean_resource();
	}

	if (CurAlarmRec) {
		free(CurAlarmRec);
	}

	if ((gOptions.I1ToH || gOptions.I1ToG) && gFilterBuf) {
		free(gFilterBuf);
	}

	//关闭RS232
	ff232.free();
	if(gOptions.IsSupportMF || gOptions.IsSupportiCLSRW) {
		st232.free();
	}

	//关闭UDP
	EthFree();
	FDB_AddOPLog(0, OP_POWER_OFF, 0,0,0,0);
	FDB_FreeDBs();

	if (!gOptions.IsOnlyRFMachine) {
		FPFree();
		FREE(gImageBuffer);
		FreeSensor();
	}


	if(gOptions.IsSupportAuthServer) {
		FreeAuthServer();
	}

	ExCloseRF();
	ExCloseWiegand();
	GPIO_IO_Free();
	sync();

	ExPowerOff(FALSE);
	DBPRINTF("Finish EndProc....");
	return 0;
}

void GetClockDT(Clock *clock)
{
	time_t gTime;
	struct tm *gDate;

	gTime=time(0);
	gDate=localtime(&gTime);
	clock->Year=gDate->tm_year+1900;
	clock->Mon=gDate->tm_mon+1;
	clock->Day=gDate->tm_mday;
	clock->YDay=gDate->tm_yday + 1;         //zsliu add
	clock->Week=gDate->tm_wday;
	clock->Hour=gDate->tm_hour;
	clock->Min=gDate->tm_min;
	clock->Sec=gDate->tm_sec;
}

char* GetWeekStr(int n)
{
	static char WeekStr[64];
	switch(n)
	{
		case 1:strcpy(WeekStr,LoadStrByID(HID_DAY1));break;
		case 2:strcpy(WeekStr,LoadStrByID(HID_DAY2));break;
		case 3:strcpy(WeekStr,LoadStrByID(HID_DAY3));break;
		case 4:strcpy(WeekStr,LoadStrByID(HID_DAY4));break;
		case 5:strcpy(WeekStr,LoadStrByID(HID_DAY5));break;
		case 6:strcpy(WeekStr,LoadStrByID(HID_DAY6));break;
		case 0:strcpy(WeekStr,LoadStrByID(HID_DAY0));break;
	}
	return WeekStr;
}

static long Desktop_Init(void)
{
	SetStateStr();
	SetKeyStr();
	SetSpeedStr();
	SetSwitchStr();
	SetFreeSetStr();

	*gDesktop.Weather=0;
	GetClockDT(&gDesktop.clock);
	if(LoadBitmap(HDC_SCREEN, &gDesktop.bmp_bkgnd, GetBmpPath("desktop.jpg"))) return 1;
	if(LoadBitmap(HDC_SCREEN, &gDesktop.bmp_bar, GetBmpPath("bar.bmp"))) return 1;
	LoadBitmap(HDC_SCREEN,&shutdownbmp,GetBmpPath("shutdown.jpg"));
	LoadBitmap(HDC_SCREEN,&onebmp,GetBmpPath("one.gif"));
	LoadBitmap(HDC_SCREEN,&twobmp,GetBmpPath("two.gif"));
	LoadBitmap(HDC_SCREEN,&threebmp,GetBmpPath("three.gif"));
	LoadBitmap(HDC_SCREEN,&pcbmp,GetBmpPath("mi2.gif"));
	LoadBitmap(HDC_SCREEN,&devbmp,GetBmpPath("device.gif"));
	LoadBitmap(HDC_SCREEN,&arrowbmp,GetBmpPath("arrow.gif"));
	LoadBitmap(HDC_SCREEN, &mbmpok, GetBmpPath("ok.gif"));
	LoadBitmap(HDC_SCREEN, &mbmpfail, GetBmpPath("fail.gif"));
	LoadBitmap(HDC_SCREEN, &mbmplock, GetBmpPath("lock.gif"));
	LoadBitmap(HDC_SCREEN, &mbmpcard, GetBmpPath("card.gif"));
	LoadBitmap(HDC_SCREEN, &mbmppwd, GetBmpPath("pwdfg.gif"));
	LoadBitmap(HDC_SCREEN, &mbmpusb, GetBmpPath("usbcheck.gif"));

	if(gOptions.IsOnlyRFMachine)
		LoadBitmap(HDC_SCREEN,&mbmpbk, GetBmpPath("card_bkg.jpg"));
	else
		LoadBitmap(HDC_SCREEN,&mbmpbk, GetBmpPath("fp.jpg"));
	LoadBitmap(HDC_SCREEN,&mbmphint,GetBmpPath("warning.gif"));
	LoadBitmap(HDC_SCREEN,&smsicon,GetBmpPath("smshint.gif"));
	LoadBitmap(HDC_SCREEN,&myalarm,GetBmpPath("alarmnew.gif"));
	if (gOptions.BatteryInfo)	//add by jazzy 2008.12.17 for battery infomation
	{
		LoadBitmap(HDC_SCREEN,&bat0,GetBmpPath("bat0.gif"));
		LoadBitmap(HDC_SCREEN,&bat1,GetBmpPath("bat1.gif"));
		LoadBitmap(HDC_SCREEN,&bat2,GetBmpPath("bat2.gif"));
		LoadBitmap(HDC_SCREEN,&bat3,GetBmpPath("bat3.gif"));
		LoadBitmap(HDC_SCREEN,&bat4,GetBmpPath("bat4.gif"));
		LoadBitmap(HDC_SCREEN,&bat5,GetBmpPath("bat5.gif"));
		LoadBitmap(HDC_SCREEN,&bat6,GetBmpPath("bat6.gif"));
	}
	if (gfont!=NULL)
		gDesktop.logfont=gfont;
	//	循环显示的LOGO图片,目前可支持的格式为BMP,GIF,PCX
	gDesktop.logo=GetPIC(GetPicPath("adpic"),"ad_",gDesktop.logostr);

	//	设置初始桌面状态
	EnableDestopstate();

	gFpInput.WaitSec=0;
	gFpInput.IndexEnroll=0;
	gFpInput.Image.Width=gFpInput.Image.Height=0;

	memset(gFpInput.Image.BitMap,0,MAXFPIMGSIZE);
	memset(gKeyInput.Buffer,0,MAXKEYSIZE);

	gKeyInput.PIN=0;
	gKeyInput.WORKCODE=0;

	workingbool=FALSE;
	vfwndflag=0;
	menuflag=0;
	keykeep=0;
	VerifiedPIN=0;
	shutflag=0;
	return 0;
}

static void Desktop_Free(void)
{
	UnloadBitmap(&gDesktop.bmp_bkgnd);
	UnloadBitmap(&gDesktop.bmp_bar);
	UnloadBitmap(&mybmp8);
//	UnloadBitmap(&statusbmp);
	UnloadBitmap(&shutdownbmp);
	UnloadBitmap(&devbmp);
	UnloadBitmap(&pcbmp);
	UnloadBitmap(&arrowbmp);

	UnloadBitmap(&mbmpok);
	UnloadBitmap(&mbmpfail);
	UnloadBitmap(&mbmplock);
	UnloadBitmap(&mbmpcard);
	UnloadBitmap(&mbmppwd);
	UnloadBitmap(&mbmpbk);
	UnloadBitmap(&mbmphint);
	UnloadBitmap(&smsicon);
	UnloadBitmap(&myalarm);
	UnloadBitmap(&mbmpusb);

	/*dsl 2012.6.11*/
	UnloadBitmap(&gSubMenuBGPic);

	if (gOptions.BatteryInfo)	//add by jazzy 2008.12.17 for battery infomation
	{
		UnloadBitmap(&bat0);
		UnloadBitmap(&bat1);
		UnloadBitmap(&bat2);
		UnloadBitmap(&bat3);
		UnloadBitmap(&bat4);
		UnloadBitmap(&bat5);
		UnloadBitmap(&bat6);
	}
	DestroyLogFont(gDesktop.logfont);
}

static void ShowDoorStatus(HWND hWnd)
{
	char doorhint[50];
	int tmpvalue=0;

	HDC hdc = GetClientDC(hWnd);
	if(!gOptions.ClockTime || ShowIdleLogoDelay)
	{
		SetBrushColor(hdc, COLOR_black);
		FillBox(hdc,0,0,120,18);
	}
	else
	{
		FillBoxWithBitmap(hdc, 0,0,0,0,&mybmp8);
	}

	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
	SetTextColor(hdc,PIXEL_green);
	memset(doorhint, 0, 50);
	if(gAlarmStrip)
	{
		sprintf(doorhint, "%s",LoadStrByID(HID_ALARM_STRIPE));
	}
	else
	{
		if(bDoorBreak)
			sprintf(doorhint,"%s",LoadStrByID(HID_ALARM_DOOR));
		else if(bDoorOpen)
			sprintf(doorhint,"%s", LoadStrByID(HID_DOOR_OPEN));
		else if(bDoorClose)
			sprintf(doorhint,"%s", LoadStrByID(HID_DOOR_CLOSE));
	}
	TextOut(hdc, 2, 2, doorhint);
	ReleaseDC(hdc);
}

static void ClearDoorStatus(HWND hWnd)
{
	HDC hdc = GetClientDC(hWnd);
	if(!gOptions.ClockTime || ShowIdleLogoDelay)
	{
		SetBrushColor(hdc,COLOR_black);
		FillBox(hdc,0,0,140,18);
	}
	else
	{
		FillBoxWithBitmap(hdc, 0, 0, 0, 0, &mybmp8);
	}

	ReleaseDC(hdc);
}

static RECT battery_rc={278,4,317,20};
extern unsigned char free_battery;
//显示电池电量状态
//TimeFlag:1= 为每秒的刷新处理 0=其他时间的刷新处理
static void ShowBatteryStatus(HWND hWnd,int TimeFlag)
{
	int i,y=5;
	int vbat;
	static unsigned int  bat_alarm=0;
	static unsigned char ShowBat=0;
	HDC hdc = GetClientDC(hWnd);

	vbat=check_battery_voltage(&i);

	if(BatteryMod==BATTERY_Internal)
	{
		//	vbat=check_battery_voltage(&i);
		if (vbat==0)
		{
			if(TimeFlag)
			{
				bat_alarm++;
				ExBeep(1);
				if(bat_alarm%2)
				{
					FillBoxWithBitmap(hdc,280,y,36,12,&bat0);
					ShowBat=1;
				}
				else
				{
					if(ShowBat)
						InvalidateRect(hWnd,&battery_rc,FALSE);
					ShowBat=0;
				}

				if(bat_alarm>180)  // 3min
				{
					POWEROFFSTART();
					shutdownbool=FALSE;
					bat_alarm=0;
				}
			}
			ReleaseDC(hdc);
			return ;
		}
		else
			bat_alarm=0;

		switch (vbat)
		{
			case 0: FillBoxWithBitmap(hdc,280,y,36,12,&bat0);       break;
			case 1: FillBoxWithBitmap(hdc,280,y,36,12,&bat1);       break;
			case 2: FillBoxWithBitmap(hdc,280,y,36,12,&bat2);       break;
			case 3: FillBoxWithBitmap(hdc,280,y,36,12,&bat3);       break;
			case 4: FillBoxWithBitmap(hdc,280,y,36,12,&bat4);       break;
			default:
				{

					FillBoxWithBitmap(hdc,280,5,36,12,&bat4);
					break;
				}
		}
		ShowBat=1;
	}
	else if(BatteryMod==BATTERY_External)
	{
		free_battery=0;
		//printf(" BATTERY_External   BatteryStatus %d  vbat %d \n", BatteryStatus, vbat);
		if(BatteryStatus==1 && vbat>=0)  //充电饱和状态
		{
			FillBoxWithBitmap(hdc,280,5,36,12,&bat6);
			ShowBat=1;
		}
		else if((BatteryStatus==0) && (vbat>=0) && (vbat <5))  //正在充电状态
		{
			FillBoxWithBitmap(hdc,280,5,36,12,&bat5);
			ShowBat=1;
		}
		else
		{
			//其他为没有电池或者电池坏，不显示电池图标
			if(ShowBat)
				InvalidateRect(hWnd,&battery_rc,FALSE);
			ShowBat=0;
		}
	}
	else
	{
		//add cn 2009-03-02
		if(battery_fd>0)
		{
			close(battery_fd);
			battery_fd=-1;
		}
	}
	ReleaseDC(hdc);
}

static long Desktop_ShowLogo(HWND hWnd, char *name)
{
	char bmpfile[256];
	HDC hdc=GetClientDC(hWnd);

	if(strlen(name)){
		sprintf(bmpfile,"%s%s",GetPicPath("adpic/"),name);
	} else {
		sprintf(bmpfile,"%s",GetBmpPath("desktop.jpg"));
	}

	if(LoadBitmap(hdc, &gDesktop.bmp_logo, bmpfile)){
		ShowDoorStatus(hWnd);
		//处理永远显示状态图
		if (gOptions.ShowState && gOptions.AlwaysShowState){
			procmainstatekey(hWnd,-1);
		}
	} else {
		FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight-30, &gDesktop.bmp_logo);
		UnloadBitmap(&gDesktop.bmp_logo);
		GetBitmapFromDC(hdc, 0, 0, 130, 18, &mybmp8);
		ShowDoorStatus(hWnd);
		{
			//GetBitmapFromDC(hdc, 140, 1, 180, 24, &statusbmp);
		}
		//处理永远显示状态图
		if ((bStateChgTime != 0) || (gOptions.ShowState && gOptions.AlwaysShowState)){
			procmainstatekey(hWnd,-1);
		}
	}

	ReleaseDC(hdc);
	//add by jazzy 2008.12.17 for battery infomation
	if (gOptions.BatteryInfo) {
		ShowBatteryStatus(hWnd,0);
	}

	if (gOptions.IsSupportModem > 1 && gOptions.ModemEnable) {
		show_modem_status_icon(hWnd, gOptions.LCDWidth);
	}


	if (pushsdk_is_running()) {
		show_iclock_status_icon(hWnd, gOptions.LCDWidth);
	}

	if (gOptions.IsSupportWIFI && (gOptions.WifiModule > 0)) {
		show_wireless_status_icon(hWnd, gOptions.LCDWidth);
	}

	return 1;
}

static void EnableDestopstate(void)
{
	gDesktop.waitsec=MAXLOGOSHOWSEC;
	if(gOptions.ClockTime)
		gDesktop.flage = STATE_ENABLELOGO;
	else
		gDesktop.flage = STATE_ENABLECLOCK;

}

static void ShowWkcdSelect(HWND hWnd)
{
	//select workcode
	SwitchMsgType(0);
#ifndef ZEM600
	KillTimer(hWnd, IDC_TIMER);
#else
	EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
	asmsflag=1;
	smsindex=0;
	menuflag = 1;
	if(tftkeylayout==3 && gOptions.ShowState){
		keykeep=1;
	}

	wkcdwinparam = 1;
	memset(ownwkcd,0,MAX_WORKCODE_LEN+1);
	curWorkCode = CreateWorkCodeManageWindow(hWnd);
	wkcdwinparam = 0;
	SwitchMsgType(1);
	menuflag = 0;
	keykeep=0;
#ifndef ZEM600
	SetTimer(hWnd,IDC_TIMER,100);
#else
	EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
	if (curWorkCode || ownwkcd[0])          //选择了WorkCode
	{
		VerifiedPIN = 0;
		vfwndflag=1;
		if (!gOptions.IsOnlyRFMachine)
			VerifyWindow(hWnd, 0, 0, &Fwmsg);
		else
			VerifyWindow(hWnd, 1, 0, &Fwmsg);
		vfwndflag = 0;
		FlushSensorBuffer();
		ResetLEDDisplay();
	}

	//处理胁迫验证信号

	if(DelayTriggerDuress && VerifiedPIN)
		TriggerDuress((U16)VerifiedPIN, TRUE);
	else
		DelayTriggerDuress=0;
}

extern int issavecapture;
static void ShowMainMenu(HWND hWnd)
{
	int i;
	int OldState=gMachineState;  //changed by cxp at 2010-04-22
	menuflag1=1;
#ifdef _TTS_
	if(gOptions.TTS_KEY)
	{
		//TTS_PlayWav(GetWavPath("menu.wav"));
		TTS_ExPlayWav(GetWavPath("menu.wav"));
	}
#endif

#ifndef ZEM600
	KillTimer(hWnd,IDC_TIMER);
#else
	EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
	SwitchMsgType(0);
	gDesktop.flage = STATE_NORMAL;
	asmsflag=1;
	smsindex=0;
	menuflag=1;
	if(tftkeylayout==3 && gOptions.ShowState)
		keykeep=1;
	gMachineState=STA_MENU;  //changed by cxp at 2010-04-22
	SSR_MENU_MAINMENU(hWnd);
	gMachineState=OldState;  //changed by cxp at 2010-04-22
#ifndef ZEM600
	SetTimer(hWnd,IDC_TIMER,100);
#else
	EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
	menuflag=0;
	menuflag1=0;
	keykeep=0;
	FlushSensorBuffer();

	if (gOptions.CameraOpen) issavecapture=0;
	if(gOptions.IsSupportSMS) bsmscount=FDB_GetBoardNum();

	for(i=0;i<10;i++)
		memset(gDesktop.logostr[i],0,16);
	gDesktop.logo=GetPIC(GetPicPath("adpic"),"ad_",gDesktop.logostr);

	if(!bStateChgTime && gOptions.AutoStateFunOn)
		GetNowState();

	//processstarbar(hWnd);	//zsliu add
	SwitchMsgType(1);
	sync();
	ShowMainLCD(hWnd);
}

static int ShowTimeBox(HWND hWnd)
{
	HDC hdc1,mem_dc;

	hdc1=GetClientDC(hWnd);
	mem_dc = CreateMemDC (gOptions.LCDWidth, gOptions.LCDHeight-30, 16, MEMDC_FLAG_HWSURFACE | MEMDC_FLAG_SRCALPHA, 0x0000F000, 0x00000F00, 0x000000F0, 0x0000000F);
	ShowTime(mem_dc);
	if (alarmflag)
		FillBoxWithBitmap(mem_dc,68,12,0,0,&myalarm);
	BitBlt(mem_dc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight-30, hdc1, 0 ,0, 0);
	//GetBitmapFromDC(hdc1, 140, 1, 180, 24, &statusbmp);
	DeleteMemDC(mem_dc);
	ReleaseDC(hdc1);
	return 0;
}

static int processstarbar(HWND hWnd)
{
	HDC hdc;
	char strbar[100];
	char tmp1[20];
	int tmpvalue = 0;
	PLOGFONT statefont=NULL	;
	int cWidth, cnum;
	hdc=GetClientDC(hWnd);
	FillBoxWithBitmap(hdc,0,gOptions.LCDHeight-30, gOptions.LCDWidth, 0, &gDesktop.bmp_bar);
	if(bsmscount>0)
		FillBoxWithBitmap(hdc,110,218,0,0,&smsicon);
	if(showusbmap > 0)
		FillBoxWithBitmap(hdc,80,218,0,0,&mbmpusb);	

	SelectFont(hdc,gDesktop.logfont);
	SetTextColor(hdc,PIXEL_black);
	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);

	GetClockDT(&gDesktop.clock);
	//zsliu add
	if(gOptions.isUseHejiraCalendar)
	{
		ChrisToHejira(&gDesktop.clock); //公历转伊朗日历
	}
	//zsliu add end ... ...

	FormatDate(tmp1, gOptions.DateFormat, gDesktop.clock.Year ,gDesktop.clock.Mon,gDesktop.clock.Day);
	sprintf(strbar,"%s %02d:%02d %s",tmp1,gDesktop.clock.Hour, gDesktop.clock.Min,GetWeekStr(gDesktop.clock.Week));

	//zsliu
	gDesktop_Clock_rc.right = gOptions.LCDWidth-5;
	gDesktop_Clock_rc.bottom = gOptions.LCDHeight-5;

	DrawText(hdc,strbar,-1,&gDesktop_Clock_rc,DT_RIGHT | DT_VCENTER);

	//	printf("gOptions.AttState:%d\n",gOptions.AttState);
	cnum = strlen(gDesktop.KeyState);
	if(cnum>0)
		cWidth = (90/cnum)*2-1;
	else
		cWidth = 14;

	if(cWidth>=14) cWidth=14;

	if(gOptions.ShowState)
	{
		if ((gOptions.AlwaysShowState) && (gDesktop.flage&STATE_ENABLELOGO) && (!workingbool))
			procmainstatekey(hWnd,-1);
		if (gfont==NULL)
			statefont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
					FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,cWidth, 0);
		else 	statefont =gfont;
		SelectFont(hdc,statefont);
	}

	DrawText(hdc, gDesktop.KeyState,-1,&gDesktop_State_rc,DT_LEFT|DT_VCENTER );
	SelectFont(hdc,gDesktop.logfont);
	ReleaseDC(hdc);
	if (gOptions.ShowState && gfont==NULL)
		DestroyLogFont(statefont);
	return 0;
}

void RefreshStarBar(HWND hWnd)
{
	gOptions.AttState=-1;
	sprintf(gDesktop.KeyState, "%s", LoadStrByID(HID_WELCOME));
	processstarbar(hWnd);
}

void ShowSelectStateHint(HWND hWnd)
{
	MessageBox1(hWnd, LoadStrByID(MID_MUSTCHOICESTATE), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
	UpdateWindow(hWnd, TRUE);
}

//处理LED闪烁状态
static void ResetLEDDisplay(void)
{
	FlashRedLED=0;
	ledbool=TRUE;
	ExLightLED(LED_GREEN, TRUE);
	ExLightLED(LED_RED, FALSE);
}

//处理门磁信号显示
static void processDoorStatus(HWND hWnd)
{
	if(!ShowIdleLogoDelay && !ShowMainLCDDelay && !workingbool && !vfwndflag && !menuflag)
	{
		ShowDoorStatus(hWnd);
	}
}

//处理验证后的显示
static void ProcessDisplayAfterVerify(HWND hWnd, int userID)
{
	if(!ProcessSMS(hWnd, userID))	//不存在个人短消息
	{
		if (!gOptions.AlwaysShowState)
		{
			gDesktop.flage = STATE_ENABLECLOCK;
			ShowIdleLogoDelay=(TIMEOUT_CLOCKBOX)?TIMEOUT_CLOCKBOX:30;
			SetBrushColor(HDC_SCREEN,COLOR_black);
			FillBox(HDC_SCREEN,0,0,gOptions.LCDWidth, gOptions.LCDHeight-30);
			ShowTimeBox(hWnd);
			ShowMainLCDDelay=0;
		}
		else
		{
			FillBoxWithBitmap(HDC_SCREEN,0,0,gOptions.LCDWidth, gOptions.LCDHeight-30,&gDesktop.bmp_bkgnd);
			procmainstatekey(hWnd,-1);
			gDesktop.flage=STATE_ENABLELOGO;
			ShowMainLCDDelay=0;
			ShowIdleLogoDelay=MAXLOGOSHOWSEC;
		}
	}
	else
	{
		gDesktop.flage=STATE_NORMAL;
#ifdef IKIOSK
		ShowMainLCDDelay=gOptions.SMSTime;		//短消息显示30秒
#else
		ShowMainLCDDelay=30;
#endif
		ShowIdleLogoDelay=0;
	}
}

void TriggerDuress(U16 pin, int verified)
{
	U32 d[3];
	WaitDuressAlarm=gOptions.DuressAlarmDelay +1;
	d[0]=0xFFFFFFFF;
	d[1]=OP_DURESS | (pin<<16);
	d[2]=verified;
	CheckSessionSend(EF_ALARM, (void*)d, 12);
	FDB_AddOPLog(pin, OP_DURESS, verified, 0, 0, 0);
}

static void ResetLEDDisplayAfterVerify(void)
{
        FlashRedLED=0;
        ledbool=TRUE;
        ExLightLED(LED_RED, FALSE);
}


static void processVerify(HWND hWnd, int style, int flag)
{
	VerifiedPIN=0;
	gDesktop.flage = STATE_NORMAL;
	asmsflag=1;
	smsindex=0;
	smsflag=0;
	menuflag=1;
	if(tftkeylayout==3 && gOptions.ShowState)
		keykeep=1;
	vfwndflag=1;
	ShowMainLCDDelay=0;
	ShowIdleLogoDelay=0;
	int oldWaitPowerOff=0;
	oldWaitPowerOff=WaitPowerOff;
	WaitPowerOff=0;

	if (!(!flag && gOptions.MustChoiceInOut && gOptions.AttState<0))
		VerifyWindow(hWnd, style, flag, &Fwmsg);
	else
		ShowSelectStateHint(hWnd);

	//Liaozz 20080925 fix bug second 14
	extern int bsupperpwd;
	bsupperpwd = 0;
	//Liaozz end
	menuflag=0;
	keykeep=0;
	FlushSensorBuffer();
	vfwndflag=0;
	WaitPowerOff=oldWaitPowerOff;
	ShowMainLCD(hWnd);
	ResetLEDDisplayAfterVerify();

	if(gOptions.ShowState && gOptions.MustChoiceInOut)
	{
		GetNowState();
		processstarbar(hWnd);
	}

	//处理胁迫验证信号
	//zsliu change
	if(DelayTriggerDuress && VerifiedPIN)
		TriggerDuress((U16)VerifiedPIN, TRUE);
	else
		DelayTriggerDuress=0;
}

int EnterIsSTkey(int wParam)
{
	TSHORTKEY estkey;
	int stkeyid;

	stkeyid=isShortKey(wParam);
	if(stkeyid)
	{
		memset(&estkey, 0, sizeof(TSHORTKEY));
		if(FDB_GetShortKey(stkeyid, &estkey)!=NULL)
			return estkey.keyFun;
	}
	return 0;
}

int isShortKey(int wParam)
{
	int i;
	int STKey[15];
	if(tftkeylayout!=3)
	{
		STKey[0]=SCANCODE_F1;
		STKey[1]=SCANCODE_F2;
		STKey[2]=SCANCODE_F3;
		STKey[3]=SCANCODE_F4;
		STKey[4]=SCANCODE_F5;
		STKey[5]=SCANCODE_F6;
		STKey[6]=SCANCODE_F7;
		STKey[7]=SCANCODE_F8;
		STKey[8]=SCANCODE_F11;	//pageup
		STKey[9]=SCANCODE_F12;	//pagedown
		STKey[10]=SCANCODE_BACKSPACE;
		STKey[11]=SCANCODE_CURSORBLOCKUP;
		STKey[12]=SCANCODE_CURSORBLOCKDOWN;
		STKey[13]=SCANCODE_CURSORBLOCKLEFT;
		STKey[14]=SCANCODE_CURSORBLOCKRIGHT;
	}
	else
	{
		STKey[0]=SCANCODE_BACKSPACE;
		STKey[1]=SCANCODE_CURSORBLOCKRIGHT;
		STKey[2]=SCANCODE_ESCAPE;
		STKey[3]=SCANCODE_CURSORBLOCKUP;
		STKey[4]=SCANCODE_ENTER;
		STKey[5]=SCANCODE_CURSORBLOCKDOWN;
		if(powerkey == 73)
		{
			STKey[6]=SCANCODE_F1;
		}
		else
		{
			STKey[6]=SCANCODE_0;
		}
	}

	for(i=0;i<STKEY_COUNT;i++)
	{
		//printf("lyy ===== STKey[%d] %d  wParam %d \n", i ,STKey[i], wParam);
		if(STKey[i]==wParam)
			return (i+1);
	}
	return 0;
}

static int procmainstatekey(HWND hWnd,int index1)
{
	int i,j;
	BITMAP bmp;
	int x,y;
	HDC hdc;

	unsigned char temp[30];

	char *winStr[2];
	int num=0;
	int curindex=0;
	int tmpvalue = 0;


	//永远显示时钟不支持状态图永远显示
	if (gOptions.ClockTime==0 ||WaitPowerOff)
		return 0;
	memset(temp, 0, sizeof(temp));
	winStr[0]=LoadStrByID(MID_STKEYFUN3);
	winStr[1]=LoadStrByID(MID_STKEYFUN4);
	curindex=index1;
	//由goptions.Attstate 去找index 只处理F1-F8
	if (index1==-1)
	{
		TSHORTKEY mystkey;

		memset(&mystkey,0,sizeof(TSHORTKEY));
		if (FDB_GetShortKeyByState(gOptions.AttState, &mystkey)==NULL)
		{
			return 0;
		}
		else
		{
			curindex=mystkey.keyID;
			if (curindex==0 || curindex>8)
			{
				return 0;
			}
			curindex-=1;
		}
	}

	hdc=GetClientDC(hWnd);
	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	for(i=0;i<8;i++)
	{
		TSHORTKEY mystkey;
		memset(&mystkey,0,sizeof(TSHORTKEY));

		if(i<4)
		{
			x=0;
			y=20+i*50;
		}
		else
		{
			x=190+gOptions.ControlOffset;
			y=20+(i-4)*50;
		}
		if(FDB_GetShortKey(i+1, &mystkey)!=NULL)
		{
			int nRet = -1;
			if(i==curindex)
				nRet = LoadBitmap(hdc,&bmp,GetBmpPath("status2.gif"));
			else
				nRet = LoadBitmap(hdc,&bmp,GetBmpPath("status.gif"));
			if((mystkey.keyFun) && (nRet == 0))
				FillBoxWithBitmap(hdc,x,y,130,30,&bmp);
			if(nRet == 0)
				UnloadBitmap(&bmp);

			if (fromRight==1)		//add by jazzy 2008.11.26 for arbic
				x+=15;
			else if (tftlocaleid==LID_CP874)
			{
				x += 15;
			}
			//zsliu change end ... ...
			memset(temp,0,sizeof(temp));
			if (mystkey.keyFun==1)  //moify by jazzy for menu error 2008.08.14
			{
				char mynamename[100]={0};           //modify by jazzy 2008.12.02
				memset(mynamename,0,sizeof(mynamename));
				Str2UTF8(tftlocaleid,(unsigned char*)mystkey.stateName,(unsigned char*) mynamename);

				//strncpy(temp,mystkey.stateName,30);
				strncpy((char *)temp,mynamename,STATE_NAME_LEN-4);

				for(j=0;j<STATE_NAME_LEN;j++)
				{
					if(temp[j]==0x0)
						break;
					if(temp[j]<0xa0)
						num++;
				}
				//if((num%2)&&(strlen((char *)temp)==20)&&temp[19]>=0xa0)
				//	temp[19]=0x0;
				if((num%2)&&(strlen((char *)mystkey.stateName)==STATE_NAME_LEN)&&temp[STATE_NAME_LEN-1]>=0xa0)
					temp[STATE_NAME_LEN-1]=0x0;

				if((strlen((char *)mystkey.stateName)*8)>130)
				{
					TextOut(hdc,x,y+8,(char *)temp);

				}
				else
				{
					TextOut(hdc,x+(130-strlen((char *)mystkey.stateName)*8)/2,y+8,(char *)temp);
				}

			}
			else if (mystkey.keyFun==2)
			{
				if (tftlocaleid == LID_CP874)
					TextOut(hdc, x+15, y+8, winStr[0]);
				else
				{
					if((strlen(winStr[0])*8)>130)
					{	
						if(fromRight == 1)
							TextOut(hdc, x+(130-(strlen(winStr[1])/2)*8)/2, y+8, winStr[0]);
						else
							TextOut(hdc,x,y+8,winStr[0]);
					}
					else
					{
						TextOut(hdc,x+(130-strlen(winStr[0])*8)/2,y+8,winStr[0]);
					}
				}
			}
			else if (mystkey.keyFun==3)
			{
				if (tftlocaleid==LID_CP874)
					TextOut(hdc, x+35, y+8, winStr[1]);
				else
				{
					if((strlen(winStr[1])*8)>130)
					{
						if(fromRight ==1)
							TextOut(hdc,x+(130-(strlen(winStr[1])/2)*8)/2,y+8,winStr[1]);
						else
							TextOut(hdc,x, y+8,winStr[1]);
					}
					else
					{
						TextOut(hdc,x+(130-strlen(winStr[1])*8)/2,y+8,winStr[1]);
					}
				}
			}
#ifdef FACE
			else if(gOptions.FaceFunOn && mystkey.keyFun>= STK_FUN && mystkey.keyFun <= STK_GROUP5)
			{
				if ((strlen(mystkey.stateName)*8)>130)
				{
					TextOut(hdc, x, y+8, mystkey.stateName);
				}
				else
				{
					TextOut(hdc,x+(130-strlen(mystkey.stateName)*8)/2,y+8,mystkey.stateName);
				}
			}
#endif
#ifdef IKIOSK
			else if (mystkey.keyFun>0)
			{
				if((strlen(LoadStrByID(MID_IKIOSK+mystkey.keyFun))*8)>130)
				{
					TextOut(hdc,x, y+8,LoadStrByID(MID_IKIOSK+mystkey.keyFun));
				}
				else
				{
					TextOut(hdc,x+(130-strlen(LoadStrByID(MID_IKIOSK+mystkey.keyFun))*8)/2,y+8,LoadStrByID(MID_IKIOSK+mystkey.keyFun));
				}

			}

#endif
			num=0;
		}

	}
	ReleaseDC(hdc);
	return 0;
}

static void freesub(HWND hWnd)
{
	HWND hChild;
	while ((hChild = GetFirstHosted(hWnd)) != 0)
	{
		freesub(hChild);
		if (IsDialog(hChild))
			EndDialog(hChild, IDCANCEL);
		else
			SendMessage(hChild, MSG_CLOSE, 0, 0);
	}
}

#ifdef IKIOSK
extern HWND hIMEWnd;
static void procSubWindowTimeOut(hWnd)
{
	timer_t ctimer = time(NULL);
	if (ctimer>=gMenuTimeOut+60 && !busyflag)
	{
		ismenutimeout=1;
		if(hIMEWnd != HWND_INVALID)
		{
			SendMessage(hIMEWnd, MSG_CLOSE, 0, 0);
			hIMEWnd = HWND_INVALID;
		}
		freesub(hWnd);
		if (isfpdbload) FPDBInit();
	}
}

static void BeforeEnterSubWindow(HWND hWnd)
{
	SwitchMsgType(0);
	ShowMainLCDDelay=0;
	ShowIdleLogoDelay=0;
	gDesktop.flage=STATE_NORMAL;
	asmsflag=1;
	smsindex=0;
	smsflag=0;
	menuflag = 1;
	SetMenuTimeOut(0);
#ifndef ZEM600
	KillTimer(hWnd, IDC_TIMER);
#else
	EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
	SetTimer(hWnd, IDC_TIMER3, 100);
}

static void AfterExitSubWindow(HWND hWnd)
{
	int i;

	FlushSensorBuffer();

	if (gOptions.IsSupportSMS) bsmscount=FDB_GetBoardNum();

	for(i=0;i<10;i++)
		memset(gDesktop.logostr[i],0,16);
	gDesktop.logo=GetPIC(GetPicPath("adpic"),"ad_",gDesktop.logostr);

	if(!bStateChgTime && gOptions.AutoStateFunOn)
		GetNowState();

	keykeep=0;
	menuflag=0;
	ResetLEDDisplay();
	EnableDestopstate();
	UpdateWindow(hWnd, TRUE);
	SwitchMsgType(1);
	KillTimer(hWnd, IDC_TIMER3);
#ifndef ZEM600
	SetTimer(hWnd, IDC_TIMER, 100);
#else
	EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
}

int procFunctionKey(HWND hWnd, int keyfun)
{
	BeforeEnterSubWindow(hWnd);
	gLocalCorrectionImage=FALSE;

	switch(keyfun)
	{
		case STK_UPDTHEME:
			break;
		case STK_USERMGMT:			//6
			SSR_MENU_USER(hWnd);
			break;
		case STK_NEWUSER:			//7
			CreateNewUserWindow(hWnd);
			break;
		case STK_MANAGEUSER:
			CreateUserBrowseWindow(hWnd);
			break;
		case STK_COMMMGMT:
			SSR_MENU_COMM(hWnd);
			break;
		case STK_NETWORK:
			SSR_MENU_COMM1(hWnd);
			break;
		case STK_RS232:
			SSR_MENU_COMM2(hWnd);
			break;
		case STK_SECURITY:
			SSR_MENU_COMM3(hWnd);
			break;
		case STK_SYSTEM:
			SSR_MENU_SYSTEM(hWnd);
			break;
		case STK_SYSTEMSET:
			SSR_MENU_SYSTEM1(hWnd);
			break;
		case STK_DATA:
			SSR_MENU_SYSTEM2(hWnd);
			break;
		case STK_UPDATE:
			SSR_MENU_UPDATE(hWnd);
			break;
		case STK_KEYBOARD:
			if (gOptions.ShowState)
			{
				if (gOptions.IMEFunOn==1)
					CreateShortKeyWindow(hWnd);
				else
					SSR_MENU_SYSTEM5(hWnd);
			}
			break;
		case STK_DISPLAY:
			CreateSystem6Window(hWnd);
			break;
		case STK_POWERMGMT:
			CreateSystemOtherWindow(hWnd);
			break;
		case STK_RESET:
			SSR_MENU_RESTORE(hWnd);
			break;
		case STK_BELL:
			CreateArlarmManageWindow(hWnd);
			break;
		case STK_DATETIME:
			SSR_MENU_DATE(hWnd);
			break;
		case STK_PENDRIVE:
			SSR_MENU_DATA(hWnd);
			break;
		case STK_DWNRECORD:
			SSR_MENU_DATADLG(hWnd, 0);
			break;
		case STK_DWNUSER:
			SSR_MENU_DATADLG(hWnd, 1);
			break;
		case STK_UPLUSER:
			SSR_MENU_DATADLG(hWnd, 4);
			break;
		case STK_UPLPICTURE:
			SSR_MENU_DATADLG(hWnd, 7);
			break;
		case STK_DWNEMS:
			SSR_MENU_DATADLG(hWnd, 2);
			break;
		case STK_UPLEMS:
			SSR_MENU_DATADLG(hWnd, 5);
			break;
		case STK_UPLTHEME:
			SSR_MENU_DATADLG(hWnd, 11);
			break;
		case STK_AUTOTEST:
			SSR_MENU_AUTO(hWnd);
			break;
		case STK_RECORD:
			if (gOptions.CameraOpen)
				CreateRecordWindow(hWnd);
			else
				ShowLogSetWindow(hWnd);
			break;
		case STK_SYSINFO:
			SSR_MENU_INFO(hWnd);
			break;
		case STK_TIMEZONE:
			TimeZoneMngWindow(hWnd, 0, 0);
			break;
		case STK_HOLIDAY:
			HolidayMngWindow(hWnd);
			break;
		case STK_GROUP:
			GroupMngWindow(hWnd);
			break;
		case STK_UNLOCKGROUP:
			CGroupMngWindow(hWnd);
			break;
		case STK_ACCESS:
			DoorParameterWindow(hWnd);
			break;
		case STK_DURESS:
			DuressParameterWindow(hWnd);
			break;
	}
	AfterExitSubWindow(hWnd);
	return 1;
}
#endif

static int procShortKey(HWND hWnd, int index)
{
	TSHORTKEY mystkey;
	if(!index) return 0;

	memset(&mystkey,0,sizeof(TSHORTKEY));
	if(FDB_GetShortKey(index, &mystkey)!=NULL)
	{
		//只处理F1-F8快捷键的显示
		if (index <= 8)
		{
			if (SRLFlag && gOptions.VoiceOn)
				ExPlayVoice(19+index);

			if (gOptions.ShowState)
				procmainstatekey(hWnd,index-1);
		}

		if (mystkey.keyFun==1)
		{
			//切换工作状态
			if(gOptions.ShowState && gOptions.AttState!=mystkey.stateCode)
			{
				char mynamename[100];           //modify by jazzy 2008.12.02
				memset(mynamename,0,100);
				Str2UTF8(tftlocaleid,(unsigned char*)mystkey.stateName, (unsigned char*)mynamename);

				gOptions.AttState = mystkey.stateCode;
				memset(gDesktop.KeyState,0,STATE_NAME_LEN+1);
				nmemcpy((BYTE*)gDesktop.KeyState,(BYTE*)mynamename,strlen(mynamename));
				processstarbar(hWnd);
				if(bStateChgTime==0)
				{
					bStateChgTime = TIMEOUT_SHOWSTATE;   //modified by lyy 2009.06.23
				}
				return 1;
			}
		}
		else if (mystkey.keyFun==2)
		{
			//输入工作号码
			if(ifUseWorkCode)
			{
				ShowMainLCDDelay=0;
				ShowIdleLogoDelay=0;
				gDesktop.flage = STATE_NORMAL;
				asmsflag=1;
				smsindex=0;
				smsflag=0;
				menuflag=1;
				if(tftkeylayout==3 && gOptions.ShowState)
					keykeep=1;
				ShowWkcdSelect(hWnd);
				menuflag=0;
				keykeep=0;
				FlushSensorBuffer();
				ShowMainLCD(hWnd);
				return 1;
			}
		}
		else if (mystkey.keyFun==3)
		{
			//查看公共短消息
			if(gOptions.IsSupportSMS && bsmscount)
			{
#ifndef ZEM600
				KillTimer(hWnd,IDC_TIMER);
#else
				EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
				SwitchMsgType(0);
				ShowMainLCDDelay=0;
				ShowIdleLogoDelay=0;
				gDesktop.flage=STATE_NORMAL;
				asmsflag=1;
				smsindex=0;
				smsflag=0;
				menuflag = 1;
				if(tftkeylayout==3 && gOptions.ShowState)
					keykeep=1;
				CreateSMSBoard(hWnd,bsmscount);
				menuflag = 0;
				keykeep=0;
				FlushSensorBuffer();
				ShowMainLCD(hWnd);
#ifndef ZEM600
				SetTimer(hWnd,IDC_TIMER,100);
#else
				EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
				SwitchMsgType(1);
				return 1;
			}
		}
		else if (mystkey.keyFun==4)
		{
			//按键求助
			if((gOptions.LockFunOn&LOCKFUN_ADV) && gOptions.DuressHelpKeyOn)
			{
				DelayTriggerDuress=10;
				return 1;
			}
		}
#ifdef FACE
		else if(gOptions.FaceFunOn && mystkey.keyFun>= STK_FUN && mystkey.keyFun <= STK_GROUP5)
		{
			ProcFaceShortKey(hWnd, mystkey.keyFun,1);
		}
#endif
#ifdef IKIOSK
		else
		{
			if (mystkey.keyFun)
			{
				gLocalCorrectionImage = TRUE;
				WaitAdminRemainCnt = FDB_CntAdminUser();
				if (WaitAdminRemainCnt==0)
					procFunctionKey(hWnd, mystkey.keyFun);
				else
				{
					funkeyadmin=0;
					funkeyflag=1;

					VerifiedPIN=0;
					gDesktop.flage = STATE_NORMAL;
					asmsflag=1;
					smsindex=0;
					smsflag=0;
					menuflag=1;
					if(tftkeylayout==3 && gOptions.ShowState)
						keykeep=1;
					vfwndflag=1;
					ShowMainLCDDelay=0;
					ShowIdleLogoDelay=0;

					VerifyWindow(hWnd, 0, 1, &Fwmsg);
					FlushSensorBuffer();
					vfwndflag=0;
					ResetLEDDisplay();

					funkeyflag=0;
					if (funkeyadmin)
						procFunctionKey(hWnd, mystkey.keyFun);
				}
				return 1;
			}
		}
#endif
	}
	return 0;
}

void ProcPowerOff(HWND hWnd)
{
	//add cn 2009-03-02
	if(battery_fd>0)
	{
		close(battery_fd);
		battery_fd=-1;
	}
	ExPlayOtherVoice(VOICE_LOGOUT);
	DelayMS(2000);
	Desktop_Free();
#ifndef ZEM600
	KillTimer(hWnd,IDC_TIMER);
#else
	EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
	KillTimer(hWnd, IDC_TIMER_EVENT_MSG);
	DestroyMainWindow(hWnd);
	PostQuitMessage(hWnd);
}

//处理闹铃
static void ProcessAlarmByTime(HWND hWnd)
{
	if (gOptions.AutoAlarmFunOn)
	{
		TTime t;
		GetTime(&t);
#ifdef _TTS_
		if(gOptions.ttsIntegralPointOpen)
			memcpy(&AlarmSyncTTSpoint,&t,sizeof(TTime));
#endif

		if (t.tm_min != gBellTime.tm_min)
		{
			int HaveValidBell=0;

			memcpy(&gBellTime, &t, sizeof(TTime));
			if (gOptions.UseNewBell)
			{
				TBellNew bell;
				memset(&bell, 0, sizeof(TBellNew));
				if (FDB_GetBellByTime(&bell, t.tm_hour, t.tm_min, t.tm_wday))
				{
					HaveValidBell=1;
					alarmindex = bell.BellWaveIndex;
					alarmvol = bell.BellVolume;
					alarmtimes = bell.BellTimes;
				}
			}
			else
			{
				ALARM taa;
				int i;
				for (i=1; i<gOptions.AlarmMaxCount+1; i++)
				{
					if (FDB_GetAlarm(i, &taa) && taa.AlarmHour==t.tm_hour && taa.AlarmMin==t.tm_min && taa.AlarmStatus)
					{
						HaveValidBell=1;
						memset(&taa,0,sizeof(ALARM));
						FDB_GetAlarm(i, &taa);
						alarmindex = taa.AlarmWaveIDX;
						alarmvol = taa.AlarmAudioVol;
						alarmtimes = taa.AlarmTimes;
						break;
					}
				}
			}
			if (HaveValidBell)
			{
				if (!vfwndflag && !menuflag && !workingbool)
				{
					alarmflag = 1;
					smsflag=0;
					ShowMainLCDDelay = 0;		//关闭短消息显示
					ShowIdleLogoDelay = 0;		//关闭时钟显示
					UpdateWindow(hWnd, TRUE);
					printf("bell 1\n");
					ProcessAlarm(alarmindex);
				}
				else
				{
					printf("bell 2\n");
				}

				if (!gOptions.LockFunOn && gOptions.isSupportAlarmExt)
				{
					gExtAlarmDelay = gOptions.ExAlarmDelay;

					int mcuExBellDelay = 20*gExtAlarmDelay;
					ExAlarmBell(0, mcuExBellDelay);

					gAlarmDelay=0;
				}

				HaveValidBell=0;
			}
		}
	}
	return;
}

static int poweroff_of_press_key(HWND hWnd)
{
	//关机事件具有最高优先级
	HDC shutdowndc;
	int tmpvalue = 0;
	if(WaitPowerOff <= 0) return FALSE;

	shutdowndc=BeginPaint(hWnd);
	gDesktop.flage=STATE_NORMAL;
	if (WaitPowerOff<5)
	{
		FillBoxWithBitmap(shutdowndc,0,0,gOptions.LCDWidth,gOptions.LCDHeight,&shutdownbmp);
		tmpvalue = SetBkMode(shutdowndc,BM_TRANSPARENT);
		SelectFont(shutdowndc,gDesktop.logfont);
		SetTextColor(shutdowndc,PIXEL_lightwhite);
		TextOut(shutdowndc,10,10,LoadStrByID(HID_PREPOWEROFF));
	}
	if (WaitPowerOff==4)
	{
		ExBeep(1);
		FillBoxWithBitmap(shutdowndc,100+gOptions.ControlOffset,50,0,0,&threebmp);
		shutflag=1;
	}
	if (WaitPowerOff==3)
	{
		ExBeep(1);
		if(gOptions.ClockTime) gDesktop.flage=0;
		FillBoxWithBitmap(shutdowndc,100+gOptions.ControlOffset,50,0,0,&twobmp);
	}
	if (WaitPowerOff==2)
	{
		ExBeep(1);
		FillBoxWithBitmap(shutdowndc,100+gOptions.ControlOffset,50,0,0,&onebmp);
	}
	EndPaint(hWnd,shutdowndc);
	if(!--WaitPowerOff)
	{
		if(gOptions.KeyType == 1)
		{
			//B3, it tell the MCU powkeyvalue
			int Enable = 53;
			Enable = changeKeyValue(powerkey, 0);
			ExCommand(23, (char*)&Enable, 1, 5);
		}
		printf("send msg_close\n");
		SendMessage(hWnd,MSG_CLOSE,0,0);
	}
	return TRUE;
}

static void trunoff_relay_bell(void)
{
	if (gExtAlarmDelay)
	{
		if (!--gExtAlarmDelay)
		{
			ExAlarmBellOff(0);
		}
	}
	return;
}

static int process_enterkey_flag(HWND hWnd)
{
	if(enterkeyflag)
	{
		if(enterkeyflag==1)
		{
			enterkeyflag=0;
			enterkeydown=0;
			SendMessage(hWnd, MSG_KEYDOWN, SCANCODE_MENU, 0);
			return FALSE;
		}
		else
		{
			--enterkeyflag;
		}
	}
	return TRUE;
}

//zsliu 添加同步时间功能，以前的每小时同步一次，导致整点闹铃和夏令时会失效。
static int gSyncTimeFlag=0;	   //时钟是否同步的标志，1为同步，0为不同步
static int gSyncTimeSec=0;	 //时钟秒数计数器。

static void delay_sync_time(void)
{
	if(gSyncTimeFlag == 1)
	{
		//printf("gSyncTimeSec=%d\n", gSyncTimeSec);
		if(gSyncTimeSec >= 30)
		{
			RTCTimeValidSign=ReadRTCClockToSyncSys(&gCurTime);
			GetTime(&gCurTime);
			memcpy(&gMinTime, &gCurTime, sizeof(TTime));

			//初始化成默认不同步状态
			gSyncTimeFlag=0;
			gSyncTimeSec=0;
			printf("sync system datetime successfull\n");
		}
	}
	return;
}

static void adjust_daylight_saving(void)
{
	if(!gOptions.DaylightSavingTimeOn) return;

	if(gOptions.CurTimeMode==DAYLIGHTSAVINGTIME)
	{
		if(!IsDaylightSavingTime())
		{
			gCurTime.tm_hour-=1;
			SetTime(&gCurTime);
			gOptions.CurTimeMode=STANDARDTIME;
			SaveOptions(&gOptions);
			ReadRTCClockToSyncSys(&gCurTime);
		}
	}
	else if(gOptions.CurTimeMode==STANDARDTIME)
	{
		if(IsDaylightSavingTime())
		{
			gCurTime.tm_hour+=1;
			SetTime(&gCurTime);
			gOptions.CurTimeMode=DAYLIGHTSAVINGTIME;
			SaveOptions(&gOptions);
			ReadRTCClockToSyncSys(&gCurTime);
		}
	}
	else if(gOptions.CurTimeMode==0)
	{
		if(IsDaylightSavingTime())
			gOptions.CurTimeMode=DAYLIGHTSAVINGTIME;
		else
			gOptions.CurTimeMode=STANDARDTIME;
		SaveOptions(&gOptions);
	}
	return;
}

static void check_power_supply_status(HWND hWnd)
{
	static int countSec=60;

	if (!gOptions.BatteryInfo) return;

	countSec--;
	if(countSec==0)	{
		ExCommand( MCU_CMD_BATTERY_MOD,NULL,0,5);
		ExCommand( MCU_CMD_BATTERY_STU,NULL,0,5);
		countSec=60;
	}

	ShowBatteryStatus(hWnd,1);
	// add cn 2009-03-02
	if(battery_state){
		char bchar=battery_state&0x0f;
		if((bchar)>0) battery_state--;

		switch(battery_state&0xf0) {
			case 0x10:  // battery  mode change
				if(bchar){
					//cn 2009-03-02 解决模式状态丢失问题
					ExCommand( MCU_CMD_BATTERY_MOD,NULL,0,5);
				}
				else {
					battery_state=0;
				}
				break;
			case 0x20: // battery state change
				if(bchar){
					// cn 2009-03-02 解决状态丢失问题
					ExCommand( MCU_CMD_BATTERY_STU,NULL,0,5);
				}
				else {
					battery_state=0;
				}
				break;
			default:
				battery_state=0;
				break;
		}
	}
	return;
}

static int play_schedule_bell(HWND hWnd)
{
	if(!(gOptions.AutoAlarmFunOn && alarmflag)) return FALSE;

	static int alarmSec = 0;

	printf("--------0, alarmflag=%d, %d %d\n", alarmflag, alarmtimes, alarmvol);
	if(alarmflag < alarmtimes)
	{
#ifdef _TTS_
		TTS_ExPlayAlarm(alarmindex,alarmvol);
#else
//#ifdef ZEM600
		//int i = gCurTime.tm_sec%2;

		if ((++alarmSec) % 5 == 0)
		{
			ExPlayAlarm(alarmindex,alarmvol);
			alarmflag++;
		}
//#else
//		ExPlayAlarm(alarmindex, alarmvol);
//#endif
#endif //_TTS_	

	}
	else
	{
		alarmSec = 0;
		CloseAlarm(hWnd);
	}
	return TRUE;
}

//static void check_authserver_online(void)
//{
//	/*后台比对，modify by yangxiaolong,2011-07-25,start*/
//	if ((gOptions.IsSupportAuthServer) && (gOptions.AuthServerCheckMode != AUTH_PUSH))
//		/*后台比对，modify by yangxiaolong,2011-07-25,end*/
//	{
//		CheckAuthServerSessionTimeOut(gOptions.AuthServerTimeOut, 0);
//		if(AuthServerDNSListInterval)
//		{
//			if(!--AuthServerDNSListInterval)
//			{
//				RefreshAuthServerListFromAuthServer();
//				AuthServerDNSListInterval=gOptions.DNSCheckInterval*60;
//			}
//		}
//	}
//	return;
//}

static void shcedule_sleep(void)
{
	if(workingbool) WaitSleepCount=0;

	if(gOptions.FreeTime && (++WaitSleepCount>=gOptions.FreeTime*60))
	{
		GPIO_TFT_BACK_LIGHT(FALSE, gOptions.LcdMode);
		WaitSleepCount=0;
		sleepflag=1;
	}		
	return;
}

static int display_public_sms(HWND hWnd)
{
	if(bsmscount && gOptions.IsSupportSMS && !asmsflag && !vfwndflag && !menuflag && !workingbool &&!gDesktop.waitsec)
	{
		//printf("Process public SMS display\n");
		if(!ProcessSMS(hWnd, 0))
		{
			CloseSMS(hWnd);
		}
		else 
		{
			asmsflag=1;
			gDesktop.flage=STATE_NORMAL;
			ShowMainLCDDelay=0;
			ShowIdleLogoDelay=MAXLOGOSHOWSEC;
			if(++smsindex > bsmscount-1) smsindex=0;
		}
		return TRUE;
	}
	return FALSE;
}

static void ready_standby_scene(void)
{
	gDesktop.flage = STATE_ENABLECLOCK;
	ShowIdleLogoDelay = ((TIMEOUT_CLOCKBOX > 0) ? TIMEOUT_CLOCKBOX : 30);
	SetBrushColor(HDC_SCREEN,COLOR_black);
	FillBox(HDC_SCREEN,0,0,gOptions.LCDWidth, gOptions.LCDHeight-30);
}

static void stop_showing_sms(void)
{
	if(ShowMainLCDDelay)
	{
		if(!--ShowMainLCDDelay)
		{
			ready_standby_scene();
			smsflag=0;
		}
	}
	return;
}

static void immediately_stop_sms(HWND hWnd)
{
	ready_standby_scene();
	smsflag=0;
	ShowTimeBox(hWnd);
	return;
}

static void show_adpic_or_analog_clock(HWND hWnd)
{
	if(ShowIdleLogoDelay)
	{
		if(!--ShowIdleLogoDelay)
		{
			smsflag=0;
			if(gOptions.ClockTime)
			{
				gDesktop.flage=STATE_ENABLELOGO;
			}
			else
			{
				gDesktop.flage=STATE_ENABLECLOCK;
				UpdateWindow(hWnd,TRUE);
			}
			gDesktop.waitsec=0;
		}
	}

	//显示
	if(!workingbool && !shutflag && !vfwndflag && !menuflag)
	{
		if(gDesktop.flage&STATE_ENABLECLOCK)
		{
			ShowTimeBox(hWnd);
		}
		else if(gDesktop.flage&STATE_ENABLELOGO)
		{
			if(!gDesktop.waitsec && LogoCnt>=0)//&&i!=WaitVerifyRetryCnt)
			{
				Desktop_ShowLogo(hWnd, gDesktop.logostr[LogoCnt]);
				if (++LogoCnt > gDesktop.logo-1)
				{
					LogoCnt = 0;
				}

				asmsflag++;
				if((gDesktop.logo>0 && asmsflag>gDesktop.logo) || (gDesktop.logo<=0 && asmsflag>1))
				{
					asmsflag=0;
				}
				gDesktop.waitsec=MAXLOGOSHOWSEC;
			}
			else
			{
			 	 --gDesktop.waitsec;
			}
		}
	}
	return;
}

static void process_door_relation_action(HWND hWnd)
{
	if(gDoorSensorDelay)
	{
		//if(!--gDoorSensorDelay)
		//{
		//	//开始倒数计时60秒，逾期还不关门将产生报警信号
		//	gCloseDoorDelay=gOptions.DoorSensorTimeout;
		//}
		/*change by zxz 2013-01-16*/
		/*单片机已经处理延时,固件无需再延时*/
		gDoorSensorDelay = 0;
		gCloseDoorDelay=gOptions.DoorSensorTimeout;
	}

	if(gCloseDoorDelay)
	{
		if((gCloseDoorDelay%5)==0 || (gCloseDoorDelay==gOptions.OpenDoorDelay))
		{
			bDoorOpen=1;
			bDoorClose=0;
			bDoorBreak=0;
			processDoorStatus(hWnd);
			ExPlayVoice(VOICE_DOOR_OPEN);
		}
		if(!--gCloseDoorDelay)
		{
			SendMessage(hWnd, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_BREAK);
		}
	}
	return;
}

static void clear_broken_msg(HWND hWnd)
{
	if(gAlarmStrip>=ALARMSTRIPTAG)
	{
		if(++gAlarmStrip>ALARMSTRIPTAG+60) //计时满60秒则重置标志
		{
			gAlarmStrip=0;
			if(!ShowMainLCDDelay && !workingbool && !vfwndflag && !menuflag && !smsflag)
			{
				ClearDoorStatus(hWnd);
			}
		}
	}
	return;
}

static void reset_door_statuflag(HWND hWnd)
{
	if(resetDoorflag)
	{
		if(!--resetDoorflag)
		{
			bDoorOpen=0;
			bDoorClose=0;
			bDoorBreak=0;
			if(!ShowMainLCDDelay && !workingbool && !vfwndflag && !menuflag && !smsflag)
			{
				ClearDoorStatus(hWnd);
			}
		}
	}
	return;
}

static void schedule_turnoff_alarm_relay(void)
{
	if(gAlarmDelay)
	{
		//printf("gAlarmDelay:%d\n",gAlarmDelay);
		if(!--gAlarmDelay)
		{
			gDoorSensorDelay=0;
			gErrTimes=0;
			ExAlarmOff(0);
		}
	}
	return;
}

static void schedule_duress_alarm(void)
{
	if(WaitDuressAlarm && !vfwndflag && !menuflag)	
	{
		if(!--WaitDuressAlarm)
		{
			//printf("warning: DuressAlarm\n");
			ExAlarm(0, 24*60*60);
		}
	}
	return;
}

static void schedule_clear_errpush_counter(void)
{
	if(gErrTimesCLearDelayCnt && !vfwndflag && !menuflag)
	{
		if(!--gErrTimesCLearDelayCnt)
		{
			gErrTimes=0;
		}
	}
	return;
}

static void flash_led_light(void)
{
	if (ledbool && !vfwndflag){
		FlashRedLED = led_flash(RTCTimeValidSign, FlashRedLED);
	}
}

static void back_to_prev_attstate(HWND hWnd)
{
	if(bStateChgTime && gOptions.ShowState && gOptions.AutoStateFunOn && !vfwndflag && !menuflag)
	{
		if(!--bStateChgTime)
		{
			int tmpstate = gOptions.AttState;
			GetNowState();
			if(gOptions.AttState!=tmpstate && !workingbool)
			{
				processstarbar(hWnd);
			}
			if(gDesktop.flage&STATE_ENABLELOGO)
			{
				Desktop_ShowLogo(hWnd, gDesktop.logostr[LogoCnt]);
				if (++LogoCnt > gDesktop.logo-1)
				{
					LogoCnt = 0;
				}

				asmsflag++;
				if((gDesktop.logo>0 && asmsflag>gDesktop.logo) || (gDesktop.logo<=0 && asmsflag>1))
				{
					asmsflag=0;
				}
				gDesktop.waitsec=MAXLOGOSHOWSEC;
			}
		}
	}
	return;
}

static void check_hour_change(TTime *tt)
{
	/*TTime t;
	if (!tt){
		GetTime(&t);
	}
	else {
		memcpy((void*)&t, (void*)tt, sizeof(TTime));
	}*/

	//zsliu 添加同步时间功能，以前的每小时同步一次，导致整点闹铃和夏令时会失效。
	if ((gMinTime.tm_hour != tt->tm_hour) && (gSyncTimeFlag==0))//sync time by every hour
	{
		//printf("after 30 sec will sync time\n");
		gSyncTimeFlag=1;
#ifdef _TTS_
		if(gOptions.ttsIntegralPointOpen && (tt->tm_sec==0 && tt->tm_min==0))
		{
			char info[256];
			sprintf(info,"%s,[n2]%d%s",GetTTSID(121),tt->tm_hour,GetTTSID(122));
			TTS_Wait();
			TTS_PlayWav(GetWavPath("notify.wav"));
			//TTS_ExPlayWav(GetWavPath("notify.wav"));
			TTS_Wait();
			if(strlen(info)) TTS_Say(info);
			TTS_Wait();
			DelayMS(TTS_WAV_INTERVAL_TIME);
		}
#endif				
	}
	return;
}

static void check_minute_change(HWND hWnd, TTime *tt)
{
	/*TTime t;
	if (!tt){
		GetTime(&t);
	}
	else {
		memcpy((void*)&t, (void*)tt, sizeof(TTime));
	}*/

	if (gMinTime.tm_min != tt->tm_min)
	{
		memcpy(&gCurTime, tt, sizeof(TTime));
		memcpy(&gMinTime, tt, sizeof(TTime));
		//夏令时处理
		adjust_daylight_saving();

		//获取当前工作状态
		if(gOptions.ShowState && !bStateChgTime && gOptions.AutoStateFunOn && !vfwndflag)
		{
			GetNowState();
		}

		GetNoNcState();		//强制开关门
		if(gOptions.IsSupportSMS) bsmscount = FDB_GetBoardNum();
		processstarbar(hWnd);

		//处理公共短消息
		if(gOptions.IsSupportSMS && !gOptions.ClockTime && !vfwndflag && !menuflag && !workingbool)
		{
			//printf("Process public sms\n");
			if(asmsflag==1) asmsflag=0;
		}
	}
	return;
}

static void check_iclocksvr_online_status(HWND hWnd)
{
	if (pushsdk_is_running()) {
		iClock_Invoke();	//先屏蔽掉push状态检测
	//	printf("workingbool=%d vfwndflag=%d menuflag=%d  smsflag=%d\n",workingbool,	\
	//		vfwndflag,menuflag,smsflag);
		if (!workingbool && !vfwndflag && (!menuflag)&&(!smsflag))
		{
			show_iclock_status_icon(hWnd, gOptions.LCDWidth);
		}
	}
	return;
}

static void process_access_control_signal(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
		//门磁信号
		case News_Door_Sensor:
			if((gOptions.LockFunOn&LOCKFUN_DOORSENSOR)&&(gOptions.DoorSensorMode<2))
			{
				int param[2];
				resetDoorflag=0;
				if(lParam==DOOR_SENSOR_BREAK)
				{
					bDoorOpen=0;
					bDoorBreak=1;
					bDoorClose=0;
					processDoorStatus(hWnd);
					ExAlarm(0, 24*60*60);
					FDB_AddOPLog(0, OP_ALARM, wParam, lParam, 0,0);
				}
				else if(lParam==DOOR_SENSOR_OPEN)
				{
					bDoorOpen=1;
					bDoorBreak=0;
					bDoorClose=0;
					processDoorStatus(hWnd);
					gDoorSensorDelay=gOptions.OpenDoorDelay;
				}
				else if(lParam==DOOR_SENSOR_CLOSE)
				{
					gCloseDoorDelay=0;
					gDoorSensorDelay=0;
					bDoorOpen=0;
					bDoorBreak=0;
					bDoorClose=1;
					processDoorStatus(hWnd);
					resetDoorflag=10;
				}
				param[0]=wParam;
				param[1]=lParam;
				CheckSessionSend(EF_ALARM, (void*)&param, 8);
			}
			break;

		//出门开关信号
		case News_Door_Button:
			CheckSessionSend(EF_ALARM, (void*)&wParam, 4);
			FDB_AddOPLog(0, OP_ALARM, wParam, lParam, 0,0);
			DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
			break;

		//拆机信号
		case News_Alarm_Strip:
			CheckSessionSend(EF_ALARM, (void*)&wParam, 4);
			FDB_AddOPLog(0, OP_ALARM, wParam, lParam, 0,0);
			ExAlarm(0, 24*60*60);
			gAlarmStrip=0x10000;
			//printf ("gAlarmStrip %d\n",gAlarmStrip);
			processDoorStatus(hWnd);
			break;
		default:
			break;
	}
	return;
}

static int show_attlog_by_menu(HWND hWnd)
{
	TTime ttlog;

	if (!gOptions.ViewAttlogFunOn) return FALSE;

	GetTime(&ttlog);
	time_t t=OldEncodeTime(&ttlog);
	if ((t < LastTime) || (t-LastTime)>10) return FALSE;
	//if(gOptions.ViewAttlogFunOn && ((t-LastTime)<10)&&(t>=LastTime))
	
	{
		time_t logst,loged;
		TUser logquser;
		ttlog.tm_hour = 0;
		ttlog.tm_min = 0;
		ttlog.tm_sec = 0;
		ttlog.tm_isdst = -1;
		logst = OldEncodeTime(&ttlog);
		loged = logst+OneDay-OneSecond;
		memset(&logquser,0,sizeof(TUser));
		SwitchMsgType(0);
#ifndef ZEM600
		KillTimer(hWnd, IDC_TIMER);
#else
		EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
		ShowMainLCDDelay=0;
		smsflag=0;
		ShowIdleLogoDelay=0;
		gDesktop.flage=STATE_NORMAL;
		FDB_GetUser(LastUID,&logquser);
		if (!CreateAttLogWindow(hWnd,logquser.PIN2,logst,loged, 1))
		{
			myMessageBox1(hWnd, MB_OK | MB_ICONINFORMATION, LoadStrByID(MID_APPNAME),LoadStrByID(HID_NOATTLOG));
		}
		EnableDestopstate();
		LastTime=-1;
		LastUID=-1;
		SwitchMsgType(1);
#ifndef ZEM600
		SetTimer(hWnd, IDC_TIMER, 100);
#else
		EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
	}
	return TRUE;
}

static void play_number_by_tts(WPARAM wParam)
{
#ifdef _TTS_
	char Number[2];

	if(gOptions.TTS_KEY)
	{
		Number[0]=LOWORD(wParam)+47;
		Number[1]=0;
		TTS_Wait();
		TTS_Say(Number);
	}
#endif				
	return;
}

static void pause_or_resume_some_fun_bymsg(int msg)
{
	if (0==msg) return;

	//Liaozz 20081013 fix alarm
	if (msg != MSG_WEBINTERFACE)
	{
#ifdef ZEM600
		if(msg != MSG_TYPE_TIMER)
#endif
		{
			if(sleepflag)
			{
				GPIO_TFT_BACK_LIGHT(TRUE, gOptions.LcdMode);
				sleepflag=0;
			}
			asmsflag=1;
			smsindex=0;
			alarmflag=0;
			WaitSleepCount=0;
		}
	}
	//Liaozz end

	return;
}

/*
static void show_dont_poweroff_msg(HWND hWnd)
{
	char doorhint[50];

	HDC hdc = GetClientDC(hWnd);
	if(!gOptions.ClockTime || ShowIdleLogoDelay)
	{
		SetBrushColor(hdc, COLOR_black);
		FillBox(hdc,0,0,120,18);
	}
	else
	{
		FillBoxWithBitmap(hdc, 0,0,0,0,&mybmp8);
	}

	SetBkMode(hdc, BM_TRANSPARENT);
	SetTextColor(hdc,PIXEL_green);
	memset(doorhint, 0, 50);
	sprintf(doorhint, "%s", LoadStrByID(HID_NOTOFF));
	TextOut(hdc, 2, 2, doorhint);
	ReleaseDC(hdc);
}
*/

extern char currentkey;		//按键原始值
int pwkeyflag = 0;
extern int iclockCheck();
static int MainWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	switch (message)
	{
		case MSG_CREATE:
			Desktop_Init();
#ifndef ZEM600
			SetTimer(hWnd,IDC_TIMER,100);  /* 设置定时器,以秒为单位,这里不要修改 del by jazzy 2009.06.06*/
			SetTimer(hWnd,IDC_TIMER_EVENT_MSG,5);  /* 设置定时器,以50ms为单位,这里不要修改*/
#else
			SetTimer(hWnd,IDC_TIMER_EVENT_MSG,2);  /* 设置定时器,以50ms为单位,这里不要修改*/
#endif
			UpdateWindow(hWnd,TRUE);
			return 0;

		case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			if(workingbool)
			{
				ProcWorking(hWnd);
			}
			else
			{
				if(!vfwndflag && !menuflag)
				{
					if(VerifiedPIN)
					{
						ProcessDisplayAfterVerify(hWnd, VerifiedPIN);
						VerifiedPIN=0;
					}
					else
					{
						if(gOptions.ClockTime==0)
						{
							SetBrushColor(hdc,COLOR_black);
							FillBox(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight-30);
							ShowTimeBox(hWnd);
						}
						else
						{
							gDesktop.flage=STATE_ENABLELOGO;
							FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, &gDesktop.bmp_bkgnd);
							GetBitmapFromDC(hdc, 0, 0, 130, 18, &mybmp8);
						}
					}
				}
			}

			if(bDoorOpen || bDoorBreak || bDoorClose || gAlarmStrip)
			{
				processDoorStatus(hWnd);
			}
			processstarbar(hWnd);
			EndPaint(hWnd,hdc);
			return 0;

		case MSG_ERASEBKGND:
			return 0;

		case MSG_KEYDOWN:
			WaitSleepCount=0;
			//kenny
			if(sleepflag)
			{
				GPIO_TFT_BACK_LIGHT(TRUE, gOptions.LcdMode);
				sleepflag=0;
				break;
			}

			//close alaram
			if(alarmflag)
			{
				CloseAlarm(hWnd);
				break;
			}
			AuthServerUploadAttlogInterval=gOptions.AutoUploadAttlog;

			//通讯时按键无效
			if (workingbool) break;

			//清除短消息显示
			if(ShowMainLCDDelay)
			{
				immediately_stop_sms(hWnd);
				ShowMainLCDDelay=0;
				break;
			}
			if(ShowIdleLogoDelay && gOptions.ClockTime)
			{
				ShowIdleLogoDelay=0;
				ShowMainLCD(hWnd);
			}
			//解决不显示考勤状态时，menu按键不能使用的问题
			//if(tftkeylayout==3 && gOptions.ShowState && wParam==SCANCODE_ENTER)
			if(tftkeylayout==3 && wParam==SCANCODE_ENTER)
			{
				if(stkeyflag)		//ENTER key is shortcut key
				{
					if(!enterkeydown)
					{
						enterkeydown=1;
						enterkeyflag=2;		//长按2秒
					}
				}
				else
				{
					enterkeydown=3;
					PostMessage(hWnd, MSG_KEYDOWN, SCANCODE_MENU, 0);
				}
				break;
			}
			//处理快捷键<liming>
			{
				int stkyidx = isShortKey(LOWORD(wParam));
				if (stkyidx && procShortKey(hWnd, stkyidx))
					return 0;
			}

			if (gOptions.KeyPadBeep && shutdownbool && !enterkeydown &&
					!(currentkey==powerkey && gOptions.LockPowerKey))	//锁定关机键后按该键不发声
			{
				ExKeyBeep();
			}
#ifdef FORFACTORY
			/*dsl 2011.5.7.To display the sensor testing window for no lience.*/
			if(wParam==SCANCODE_ESCAPE)
			{
				SSR_MENU_AUTO_SORTEST(hMainWindowWnd);
				return 0;
			}
#endif
			if(wParam==SCANCODE_MENU)
			{
				if  (!show_attlog_by_menu(hWnd))
				{
					gLocalCorrectionImage=TRUE;
					WaitAdminRemainCnt=FDB_CntAdminUser();
					if(WaitAdminRemainCnt==0 || ((gAlarmStrip>ALARMSTRIPTAG+30) && (gAlarmStrip<ALARMSTRIPTAG+60)))
					{
						gLocalCorrectionImage=FALSE;
						ShowMainMenu(hWnd);
						/*Modify by zxz 2013-1-4,此处如不加延时，不停进入退出菜单机器会死机*/
#ifdef ZEM600
						DelayMS(1000);	//for ESC to main window error by jazzy 2008.10.15
#else
						DelayMS(300);
#endif
					}
					else
					{
						processVerify(hWnd,0,1);
					}
				}
				break;
			}
			//1:1验证
			if ((LOWORD(wParam)>=SCANCODE_1 && LOWORD(wParam)<=SCANCODE_9))
			{
				play_number_by_tts(wParam);

				memset(firstkeyValue,0,2);
				firstkeyValue[0]=LOWORD(wParam)+47;
				processVerify(hWnd, 3, 0);
				break;
			}
			//当TAB为电源键时不能用做状态快捷键.
			//printf("currentkey=%d, powerkey=%d\n", currentkey, powerkey);
			if (currentkey==powerkey && !gOptions.LockPowerKey)
			{
				pwkeyflag = 1;
				if (shutdownbool)
				{
					POWEROFFSTART();
					shutdownbool=FALSE;
				}
			}
			break;

		case MSG_KEYUP:
#ifdef _TTS_
			TTS_Stop();
#endif
			if (workingbool) break;

			if (pwkeyflag == 1)
			{
				pwkeyflag = 0;
				WaitPowerOff=0;
				shutdownbool=TRUE;
				if(shutflag)
				{
					shutflag=0;
					ShowMainLCD(hWnd);
				}
			}

			if (wParam==SCANCODE_ENTER && stkeyflag)
			{
				enterkeydown=0;
				if(enterkeyflag)
				{
					enterkeyflag=0;
					procShortKey(hWnd, isShortKey(LOWORD(wParam)));
				}
			}
			break;

		case MSG_WEBINTERFACE:
			ZKWeb(THIS)->Run();
			break;
		case MSG_TYPE_HID:
		case MSG_TYPE_MF:
		case MSG_TYPE_ICLASS:
			if(workingbool) break;
			processVerify(hWnd,1,0);
			break;

		case MSG_TYPE_FINGER:
			if(m==MSG_TYPE_FINGER) m = 0;

			if(workingbool) break;
			processVerify(hWnd,0,0);
			break;

		case MSG_TYPE_CMD:
			process_access_control_signal(hWnd, wParam, lParam);
			break;

		case MSG_TYPE_TIMER://dsl 2012.3.23
		case MSG_TIMER:
			if(wParam==IDC_TIMER_EVENT_MSG)
			{
				//if(m ==MSG_TYPE_FINGER && TestFlage ==-1 && WaitProcessFingerFlag)
				//{
				//	return 0;
				//}
				/*change by zxz 2012-11-19*/
				if(m == MSG_TYPE_FINGER) {
					return 0;
				}
				
				Fwmsg.Message = 0;
				Fwmsg.Param1 = 0;
				Fwmsg.Param2 = 0;
				m=GatherMsgs(&Fwmsg);
				if (m)
				{
					if(MSG_TYPE_TIMER==m) second_msg = m;
					pause_or_resume_some_fun_bymsg(m);

					if((menuflag || vfwndflag) && (Fwmsg.Message==MSG_TYPE_FINGER || Fwmsg.Message==MSG_TYPE_HID 
						|| Fwmsg.Message==MSG_TYPE_MF || Fwmsg.Message==MSG_TYPE_ICLASS
#ifdef ZEM600
						|| Fwmsg.Message==MSG_TYPE_TIMER
#endif
						))
					{
#ifdef ZEM600
						if(Fwmsg.Message==MSG_TYPE_TIMER)
						{
							SendMessage(hWnd,Fwmsg.Message,Fwmsg.Param1,Fwmsg.Param2);
						}
#endif
						return 1;
					}
					else
					{
						if ((Fwmsg.Message==MSG_TYPE_FINGER || Fwmsg.Message==MSG_TYPE_HID ||
							Fwmsg.Message==MSG_TYPE_MF || Fwmsg.Message==MSG_TYPE_ICLASS))
						{
							AuthServerUploadAttlogInterval=gOptions.AutoUploadAttlog;
						}

						SendMessage(hWnd,Fwmsg.Message,Fwmsg.Param1,Fwmsg.Param2);
						m=0;
						second_msg=0;
					}
				}
				//dsl 2012.3.23
				return 0;
			}
#ifdef IKIOSK
			if (wParam==IDC_TIMER3)
			{
				procSubWindowTimeOut(hWnd);
			}
#endif
		
			if(wParam==IDC_TIMER || message==MSG_TYPE_TIMER)//dsl 2012.3.23
			{
				TTime t;

				//同步时间标识，触发时间同步后，计满30秒后同步
				if(gSyncTimeFlag == 1)
				{
					++gSyncTimeSec;
				}

				//处理时钟显示
				show_adpic_or_analog_clock(hWnd);

				check_iclocksvr_online_status(hWnd);

				//关机事件具有最高优先级
				if (poweroff_of_press_key(hWnd))break;

				//处理外接响铃延时
				trunoff_relay_bell();

				if (!alarmflag) ProcessAlarmByTime(hWnd);
				if (!process_enterkey_flag(hWnd)) return 0;
				GetTime(&t);
#ifdef _TTS_
				if(alarmflag && gOptions.ttsIntegralPointOpen)
				{
					memcpy(&t,&AlarmSyncTTSpoint,sizeof(TTime));
				}
#endif
				check_hour_change(&t);
				check_minute_change(hWnd, &t);

				//手动改变工作状态后自动恢复
				back_to_prev_attstate(hWnd);
				//显示电池状态
				check_power_supply_status(hWnd);
				//播放闹铃
				if (play_schedule_bell(hWnd)) break;

				CheckSessionTimeOut();
				/*后台比对，modify by yangxiaolong,2011-07-25,start*/
				//check_authserver_online();
				
				CheckSessionSendMsg();
				//通讯时不计休眠
				shcedule_sleep();

				//处理公共短消息
				if (display_public_sms(hWnd)) break;	
				if (!bsmscount && !asmsflag) asmsflag = 1;
				//处理个人短消息显示延时
				stop_showing_sms();

				process_door_relation_action(hWnd);

				//拆机报警已经起动，则开始计时
				clear_broken_msg(hWnd);
				reset_door_statuflag(hWnd);
				schedule_turnoff_alarm_relay();
				schedule_duress_alarm();
				if(DelayTriggerDuress) DelayTriggerDuress--;
				schedule_clear_errpush_counter();
				//process LED
				flash_led_light();
				if (gOptions.USBCheckOn)
				{
					ShowUsbState(hWnd);
				}

				if (gOptions.IsSupportModem > 1 && gOptions.ModemEnable && (smsflag == 0)) {
					show_modem_status_icon(hWnd, gOptions.LCDWidth);
				}


				/*异地考勤,add by yangxiaolong,2011-06-14,start*/
				if (gOptions.RemoteAttFunOn == 1)
				{
					DelRmAttUser();	//异地考勤用户信息在考勤机内保存RmUserSaveTime天后，被删除
				}

				if (gOptions.IsSupportWIFI && (gOptions.WifiModule > 0) && (smsflag == 0)) {
					show_wireless_status_icon(hWnd, gOptions.LCDWidth);
				}

				if (pushsdk_is_running() && (smsflag == 0)) {
					show_iclock_status_icon(hWnd, gOptions.LCDWidth);
				}
			}
			return 0;

		case MSG_IDLE:
		{
			delay_sync_time();
			break;
		}

		case MSG_CLOSE:
			//printf ("**%s**MSG_CLOSE\n",__FUNCTION__);
			ProcPowerOff(hWnd);
			return 0;
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int MiniGUIMain(int argc, const char* argv[])
{
	MSG msg;
	MAINWINCREATE CreateInfo;

	CreateInfo.dwStyle = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "Self Service Reader 2.0";
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = MainWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = COLOR_black;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = HWND_DESKTOP;

	if(!InitVectorialFonts())
	{
		printf("Init Vectorialfonts err\n");
		return 1;
	}

	sync();
	InitProc();

	if (gOptions.IsSupportModem > 1 && gOptions.ModemEnable) {
		reload_modem_configuration();
		modem_thread_init();
	}

	hMainWindowWnd = CreateMainWindow(&CreateInfo);
	if (hMainWindowWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWindowWnd, SW_SHOWNORMAL);

	while (1)
	{
		if  (GetMessage(&msg,hMainWindowWnd))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			break;
	}

	MainWindowThreadCleanup(hMainWindowWnd);
	EndProc();
	return 0;

}

void ExSetPowerSleepTime(int IdleMinute)
{
	if(gOptions.PowerMngFunOn)
		WaitSleepCount=IdleMinute*60;
}

//kenny
void ProcWorking(HWND hWnd)
{
	HDC wkdc;
	int tmpvalue = 0 ;
	wkdc=GetClientDC(hWnd);
	FillBoxWithBitmap(wkdc,0,0,gOptions.LCDWidth,gOptions.LCDHeight,&shutdownbmp);
	FillBoxWithBitmap(wkdc,20,90,0,0,&pcbmp);
	FillBoxWithBitmap(wkdc,230,70,0,0,&devbmp);
	FillBoxWithBitmap(wkdc,90,100,0,0,&arrowbmp);
	tmpvalue = SetBkMode(wkdc,BM_TRANSPARENT);
	SelectFont(wkdc,gDesktop.logfont);
	SetTextColor(wkdc,PIXEL_red);
	TextOut(wkdc,110,130,LoadStrByID(HID_WORKING));
	ReleaseDC(wkdc);
}

void EnableDevice(int Enabled)
{
	static int DeviceEnabled=TRUE;

	//zsliu change
	//if (!vfwndflag && !menuflag)
	GPIO_TFT_BACK_LIGHT(TRUE, gOptions.LcdMode);
	if(!menuflag1)
	{
		//printf(" %s  %d \n", __FUNCTION__, Enabled);
		if(DeviceEnabled!=Enabled)	//状态改变
		{
			EnableMsgType(MSG_TYPE_FINGER,Enabled);
			EnableMsgType(MSG_TYPE_MF,Enabled);
			EnableMsgType(MSG_TYPE_ICLASS,Enabled);
			EnableMsgType(MSG_TYPE_HID,Enabled);
			EnableMsgType(MSG_TYPE_BUTTON,Enabled);
			DeviceEnabled=Enabled;
			FlushSensorBuffer();
			if(!Enabled)
			{
				workingbool=TRUE;
				alarmflag=0;
				ShowMainLCDDelay=0;             //关闭短消息显示
				smsflag=0;
				ShowIdleLogoDelay=0;            //关闭时钟显示
				asmsflag=1;			//清除公共短消息显示
			}
			else
			{
#ifdef FACE
				if( gOptions.FaceFunOn && FaceDBChg)
				{
					FDB_LoadAllFaceTmp();
					FaceDBChg=0;
				}
#endif
				workingbool=FALSE;
			}

			//SendMessage(HWND_DESKTOP, MSG_PAINT, 0, 0);
			SendMessage(hMainWindowWnd, MSG_PAINT, 0, 0);
		}
		else
		{
			SendMessage(hMainWindowWnd, MSG_PAINT, 0, 0);
		}
	}
}

//--------------------------Show Main LCD Screen--------------------------
void GetNumberChar(char *line1, char *line2, int Number)
{
	*line1=(char)254;
	line1[1]=(Number*2+161);
	*line2=(char)254;
	line2[1]=(Number*2+162);
}

int FirstLine=0;
void ShowMainLCD(HWND hWnd)
{
	EnableDestopstate();
	SendMessage(hWnd, MSG_PAINT, 0, 0);
}

int SumNum(int i)
{
	int ret=0;
	while(i)
	{
		ret+=i % 10;
		i/=10;
	}
	return ret;
}

static int OldAttState=0;

BOOL ProcStateKey(int i)
{
	if(WaitShowState==0) OldAttState=gOptions.AttState;
	if(i>=IKeyIn)
	{
		int MaxState=i-IKeyIn;
		if(gOptions.AttState!=MaxState)
		{
			gOptions.AttState=MaxState;
			if(gOptions.MustChoiceInOut) WaitShowState=TIMEOUT_SHOWSTATE;
			//			ShowMainLCD();
			return TRUE;
		}
	}
	else
	{
		gOptions.AttState=2+i-IKeyOTIn;
		WaitShowState=TIMEOUT_SHOWSTATE;
		return TRUE;
	}
	//	DBPRINTF("ATTSTATE=%d\n", gOptions.AttState);
	return FALSE;
}

//kenny
void POWEROFFSTART(void)
{
#ifdef _TTS_
	ExPlayVoice(TTS_SHUTDOWN_VOICE);
#endif
	//ExBeep(1);
	WaitPowerOff=4;
}

void WakeUpFromSleepStatus(void)
{
	if (!ShowMainLCDEnabled)
	{
		//enabled mute
		GPIO_AC97_Mute(FALSE);
		//whether display clock ":" or not
		ClockEnabled = TRUE;
		//whether display main windows or not
		ShowMainLCDEnabled = TRUE;
		GPIO_HY7131_Power(TRUE);
		GPIO_LCD_USB0_Power(TRUE);
		WaitInitSensorCount=LOADDRIVERTIME;
	}
}

void InitRTLoglist()
{
	rtloglist = MALLOC(sizeof(struct TRTLogNode));
	rtloglist->Next = NULL;
	rtlogheader=rtloglist;
	gRTLogListCount=0;
}

void AppendRTLog(int EventType,char *Data, int Len)
{
	PRTLogNode TmpCell=NULL;
	//DebugOutput2("AppendRTLog %d, %d",EventType,Len);
	if(Len>RTLOGDATASIZE)
	{	
		Len=RTLOGDATASIZE;
	}

	if(gRTLogListCount>MAXRTLOGSCACHE)
	{
		TmpCell=rtlogheader->Next;
		rtlogheader->Next=TmpCell->Next;
		FREE(TmpCell);
		gRTLogListCount--;
	}
	TmpCell=(PRTLogNode)MALLOC(sizeof(struct TRTLogNode));
	if(TmpCell)
	{
		TmpCell->log.EventType=EventType;
		memcpy((char*)(TmpCell->log.Data),Data,Len);
		TmpCell->Next = NULL;
		rtloglist->Next = TmpCell;
		rtloglist=TmpCell;
		gRTLogListCount++;
	}
}

int network_ready()
{
	if (gEthOpened) {
		return TRUE;
	}

	return FALSE;
}

PBITMAP get_submenubg_jgp(void)
{
	return &gSubMenuBGPic;
}

