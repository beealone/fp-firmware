/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录:
* 编译环境:mipsel-gcc
*/

#ifndef __CWX_GUI_

#define __CWX_GUI_

#define MAXFPWAITSEC    4      /*  验证指纹最大等待秒数 */
#define MAXEROLLCNT     3      /* 最大登记次数 */

#define MAXFPIMGSIZE    (150*1024)    /* 指纹图象大小 BYTE */
#define MAXKEYSIZE      32
#define MAXLOGOSHOWSEC  5

#define STATE_NORMAL         0x0000
#define STATE_ENROLLFP       0x0001
#define STATE_ENABLECLOCK    0x0002
#define STATE_ENABLEKEYINPUT 0x0004
#define STATE_VERIFY1TON     0x0008
#define STATE_ENABLELOGO     0x0010

#define KEY_MENU       0x0001

#define IDC_TIMER  0xff00

typedef struct _tag_Clock
{
        short Year;
        short Mon;
        short Day;
        short Hour;
        short Min;
        short Sec;
	short Week;
}
Clock;

typedef struct _tag_FpImage
{
        unsigned char BitMap[MAXFPIMGSIZE];
        unsigned long Width,Height;
}
FpImage;

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
}
gDesktop;

#define CLOCKSPERSEC 1000000l

inline unsigned long DelaySec(unsigned long sec)
{
	clock_t start;
	volatile unsigned long base=0;
	start=clock();
	while(base<CLOCKSPERSEC*sec)
	{
		base=clock()-start;
	}
}

#endif
