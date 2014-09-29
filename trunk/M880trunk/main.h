#ifndef	_MAIN_H_
#define	_MAIN_H_

#include <stdlib.h>
#include "arca.h"
#include "exfun.h"
#include "ccc.h"
#include "msg.h"
#include "finger.h"

char *gImageBuffer;
TTime gCurTime;
TTime gMinTime;

TTime gAuthServerTime;	//zsliu add 

typedef struct _VSStatus_{
	BOOL PIN;
	BOOL FP;
	BOOL Password;
	BOOL Card;
}GCC_PACKED TVSStatus, *PVSStatus;

#define ZEM500
//#define UPEK

#define RAMPATH "/mnt/ramdisk/"
#define TMPBMPFILE "/mnt/ramdisk/finger.bmp"

#define NOLICENSE		0
#define LICENSEPASS		1
#define LICENSEFAIL		2

#define MAXFPWAITSEC    4      /*  验证指纹朄1?7大等待秒敄1?7 */
#define MAXEROLLCNT     3      /* 朄1?7大登记次敄1?7 */

#define MAXFPIMGSIZE    (150*1024)    /* 指纹图象大小 BYTE */
#define MAXKEYSIZE      32
#define MAXLOGOSHOWSEC  (gOptions.LogoTime)

#define STATE_NORMAL         0x0000
#define STATE_ENROLLFP       0x0001
#define STATE_ENABLECLOCK    0x0002
#define STATE_ENABLEKEYINPUT 0x0004
#define STATE_VERIFY1TON     0x0008
#define STATE_ENABLELOGO     0x0010

#define KEY_MENU       0x0001

#define IDC_TIMER  0xff00
//#define IDC_TIMER2  0xff01
#define IDC_TIMER_EVENT_MSG  0xff01

#define MaxDate 1924790400 //2030-12-30
#define OneHour (60*60)
#define OneSecond  1//1/(24*60*60)
#define OneMinute  (1*60)//1/(24*60)
#define OneDay (24*60*60)

typedef struct _tag_Clock
{
        short Year;
        short Mon;
        short Day;
        short Hour;
        short Min;
        short Sec;
	short Week;
	short YDay;     // days since Jan 1, 1-366 //add by zsliu for Hejira 080730
}
Clock;

typedef struct _tag_FpImage
{
        unsigned char BitMap[MAXFPIMGSIZE];
        unsigned long Width,Height;
}
FpImage;
#if 0
static struct _tag_FpInput
{
        FpImage Image;
        long WaitSec;
        long IndexEnroll;
}
gFpInput;

static struct _tag_KeyInput
{
        char Buffer[MAXKEYSIZE];
        unsigned long WORKCODE;
        unsigned long PIN;
}
gKeyInput;
#endif
/*
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
}
gDesktop;
*/

/*
#define CLOCKSPERSEC 1000000l

inline unsigned long DelaySecssr(unsigned long sec)
{
        clock_t start;
        volatile unsigned long base=0;
        start=clock();
        while(base<CLOCKSPERSEC*sec)
        {
                base=clock()-start;
        }
}

*/


//void ShowMainLCD(HWND hWnd);
void EnableDevice(int Enabled);
void ExSetPowerSleepTime(int IdleMinute);
int ShowFPAnimate(int x, int y);
int isShortKey(int wParam);
void GetClockDT(Clock *clock);
void child_end(int signum);
void TriggerDuress(U16 pin, int verified);
int GetNoNcState(void);

int EnterIsSTkey(int wParam);

//int gFPDirectProc;

int brightness;
TMsg Fwmsg;
int m;
int cameraflag;
U32 picid;
int second_msg; //2012-3-9 for exit verification window

int useSDCardFlag;	//是否使用SDCard

//定制功能标志
int IDTFlag;	//西班牙定刄1?7
int SRLFlag;	//罗马尼亚定制


//-------  PUSH_SDK -----------
void iclock_statechanged(void);
void iClock_Invoke();

extern int exceedflag;
extern int ifUseWorkCode;
extern int curWorkCode;
extern int wkcdwinparam;
extern int VerifiedPIN;
extern int vfwndflag;
extern int ImageBufferLength;
#define MaxAdminVerify 10
#define TTS_WAV_INTERVAL_TIME 500
#endif
