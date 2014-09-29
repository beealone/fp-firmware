/************************************************

  ZEM 300 iClock-888

  main.c Main source file

  Copyright (C) 2006-2010, ZKSoftware Inc.
 *************************************************/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/vfs.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>

#include "pushapi.h"

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
#include "sensor.h"
#include "kb.h"
#include "rs232comm.h"
#include "convert.h"
#include "flash.h"

#include "sensor.h"
#include "wiegand.h"
#include "truncdate.h"
#include "finger.h"
#include "rs_485reader.h"
#include "srbfun.h"

#ifdef FACE
#include "facedb.h"
#endif

#ifdef WEBSERVER
#include "webserver.h"  /******** Add For Web Server ********/
#endif

#include "ssrpub.h"
#include "main.h"
#include "ssrcommon.h"
#include "updatepic.h"
#include "ssracc.h"
#include "libcam.h"


#include "fpreader.h"
# define IDC_INNO       0x10
# define IDC_INPWD      0x11
# define IDC_VF_TIMER  0xff02
# define IDC_VF_TIMER2 0xff03

#define ANTIPASSBACK_OUT        1
#define ANTIPASSBACK_IN         2
#define ANTIPASSBACK_INOUT      3
#define STATE_IN                0
#define STATE_OUT               1

#define TIMEOUT_INPUT_PIN               (gOptions.TimeOutPin)   //等待用户输入验证考勤号码的时间
#define TIMEOUT_WAIT_ADMIN_VERIFY       30                      //等待管理员验证身份的超时时间
#define MaxAdminVerify                  10
#define DELAYNEXTUSER                   30                      //多人验证时等待下一个用户验证的时间
#define WAIT_VERIFY_IDLE_TIME           10                      //等待验证超时时间

#define COUNT_RETRY_USER                3                       //重新验证身份的重试次数
#define SHOW_MAIN_WINDOW_DELAY          5                       //验证结束返回主界面时间

extern int curWorkCode;				//当前的工作号码
extern char ownwkcd[MAX_WORKCODE_LEN+1];	//自定义工作号码
extern int ledbool;
extern int WaitAdminRemainCnt;       				//等待进入管理功能需要验证的管理员数
extern char firstkeyValue[2];
extern int gLocalCorrectionImage;
extern int WaitDuressAlarm;          				//胁迫报警发生后延迟产生报警信号的时间
extern int DelayTriggerDuress;       				//按键触发胁迫报警的有效时间
extern BOOL RTCTimeValidSign;
extern PFilterRec gFilterBuf;
extern PAlarmRec CurAlarmRec;
extern int gErrTimes; //press finger 5 times trigger alarm
extern int gErrTimesCLearDelayCnt;
extern int CommSessionCount;
extern int gMFOpened;
extern int giCLSRWOpened;
extern time_t LastTime;
extern int LastUID;
extern int menuflag;
extern int VerifiedPIN;		//验证成功的用户ID
extern int menuflag;
extern int gLockForceAction;         //锁强制动作
extern int gLockForceState;          //当前锁状态
extern int gFPDirectProc;
extern int LastLogsCount;

extern BITMAP mbmpok;
extern BITMAP mbmpfail;
extern BITMAP mbmplock;
extern BITMAP mbmpcard;
extern BITMAP mbmppwd;
extern BITMAP mbmpbk;
extern BITMAP mbmphint;

FILE *fp;
PSensorBufInfo SensorInfo1;

int LastUserID;
U32 gcard = 0;
int g_usverify = 0;
int repeat = 0;
int retryflag = 0;
int WaitVerifyRetryCnt = 3;
int WaitAdmVerifyRetryCnt = 0;
char KeyBuffer[24] = {0};						//当前输入的考勤号码——（密码验证）
int DelayNextUser = 0;						//等待下一用户验证时间
int WaitAdmins[MaxAdminVerify] = {0};
int MFCardPIN = 0, bMFCardInfo = 0;
int exceedflag = 0;       					//记录溢出标志（0:无溢出1:溢出警告2:溢出）

int printstate = 0;
int WaitAdminVerifySecond = 0; 				//等待管理功能超时时间
static int FlashRedLED = 0;
PUser AdminUser;						//当前进入管理功能的用户
TUser gAdminUser;	//dsl 2012.4.17
extern time_t cardtime1;
extern time_t cardtime2;
//Filter 1:H 1:G
U32 gInputPin = 0;
U32 g1ToG = FALSE;

extern int i353;

//Authentication Modes(Verify Type)

BYTE FingerTemplate[10240*2];
TFPCardOP FPData = {0, 0, {0, 0, 0, 0}, FingerTemplate, OP_READ};
TFPResult AuthFPData = {0, "", ""};
TVSStatus VSStatus = {FALSE, FALSE, FALSE, FALSE};

BYTE gSensorNo = 0;
int adminvf = 0;//1 invalid 2, accessdeny

extern HWND hMainWindowWnd;

HWND acnownd, pwdnownd;
int WaitVerifyDelayTime = 0;	//验证超时时间
int ShowMainWindowDelay = 0;	//显示主界面时间

int curVerifyStyle = 0;		//当前验证方式
int curVerifyState = 0;		//当前验证状态
int bsupperpwd = 0;		//超级密码方式
int groupVerify = 0;		//组合验证标志
int bnoworkstate = 0;		//未选择工作状态
BOOL IsANDOperator = FALSE;	//多重验证标志

int admflag = 0;			//administrator verify flag
TUser vfUser;			//用户信息
TUser pwduser;

PLOGFONT infofont;	//display font
PLOGFONT logfont;	//input font

BITMAP 	mbmpfp;
BITMAP 	tmpbmp;

//解决显示用户照片后在验证出现花屏问题
//BITMAP  phbmp;
//int sphoto = 0;

int issavecapture = 0;
time_t capturetime = 0;


enum _VerifyState
{
	VF_READY = 0,                   //ready
	VF_FAILD,                       //fail
	VF_SUCCESS,                     //success
	VF_NEXT,			//wait next user
};

enum _VFStyle
{
	VFS_FP = 0,                     //Finger verify
	VFS_CARD,                       //card verify
	VFS_PWD,                        //password verify
	VFS_PIN,                        //pin verify
};

int CheckUserPIN(char *pin, PUser user);
U16 GetUserPIN_16(U32 pin);

static int SaveAttLog(U16 pin, int VerifiedMethod, HDC hdc, HWND hWnd);

BYTE GetExtUserVSByPIN(U32 pin, int msgtype);

//int IsOpenDoorAction = 0;

RECT verify_info_rc = {0, 0, 170, 210};		//信息显示区域
int VerifyLogState = 0;
HWND hMainWindow;

#ifdef IKIOSK
extern int funkeyadmin;
extern int funkeyflag;
extern void procFunctionKey(HWND hWnd, int keyfun);
#endif
extern int ifUseWorkCode;
extern int WaitProcessFingerFlag;


int ie_score, ie_quality;

int FillBoxWithBitmapFP(HDC hdc, int x, int y, int w, int h,
		BITMAP *bmp)
{
	if (bmp->bmBits)
	{
		//printf("%s() - line=%d,FillBox\n", __FUNCTION__, __LINE__);
		FillBoxWithBitmap(hdc, x, y, w, h, bmp);
		UnloadBitmap(bmp);
		bmp->bmBits = NULL;
	}
	else
		printf("%s() - line=%d,bmp->bmBits is NULL\n", __FUNCTION__, __LINE__);
	return TRUE;
}
int getCurrentState(int VerifyMethod)
{
	int state = 0;
	if (gOptions.AntiPassbackFunOn && gOptions.AntiPassbackOn && gOptions.MasterState >= 0)
	{
		//printf("gOptions.MasterState:%d, VerifyMethod:%d\n",gOptions.MasterState, VerifyMethod);
		/*MasterState: 0--OUT, 1--IN, -1--None*/
		if (gOptions.MasterState == 0) //control out door. 
			if (VerifyMethod == TYPE_SLAVE_ID)
				state = STATE_IN;
			else
				state = STATE_OUT;
		else if (gOptions.MasterState == 1) //control in door
			if (VerifyMethod == TYPE_SLAVE_ID)
				state = STATE_OUT;
			else
				state = STATE_IN;
		else
			state = gOptions.AttState;
	}
	else
	{
		state = gOptions.AttState;
	}

	//printf("current state=%d, VerifyMethod=%d\n", state, VerifyMethod);
	return state;
}

//显示图片
static void ShowBitmap(HWND hWnd, BITMAP bmp)
{
	static RECT verify_type_rc = {160, 0, 310, 210};		//图片显示区域
	static RECT verify_type_rc_Three = {230, 0, 380, 210};		//图片显示区域
	int tmpvalue = 0;
	HDC hdc = GetClientDC(hWnd);
	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
	SetTextColor(hdc, PIXEL_lightwhite);
	if (gOptions.MainVerifyOffset)
	{
		InvalidateRect(hWnd, &verify_type_rc_Three, TRUE);
	}
	else
	{
		InvalidateRect(hWnd, &verify_type_rc, TRUE);
	}

	FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &bmp);
	ReleaseDC(hdc);
}

//显示验证信息
static void ShowVerifyInfo(HWND hWnd, char* str, int line)
{
	static RECT verify_info_rc = {0, 70, 170, 170};
	HDC hdc = GetClientDC(hWnd);
	int tmpvalue = 0;
	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
	SetTextColor(hdc, PIXEL_lightwhite);
	if (line == 0)
	{
		InvalidateRect(hWnd, &verify_info_rc, TRUE);
	}
	TextOut(hdc, verify_info_rc.left + 10, verify_info_rc.top + 20*line, str);
	ReleaseDC(hdc);
}

//显示操作提示信息
static void ShowVerifyHint(HWND hWnd, char* str, int icon)
{
	static RECT verify_hint_rc = {0, 180, 170, 210};
	HDC hdc = GetClientDC(hWnd);
	int tmpvalue = 0;
	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
	SetTextColor(hdc, PIXEL_lightwhite);
	InvalidateRect(hWnd, &verify_hint_rc, TRUE);
	if (icon)
		FillBoxWithBitmap(hdc, 0, 180, 0, 0, &mbmphint);
	TextOut(hdc, verify_hint_rc.left + 35, verify_hint_rc.top + 5, str);
	ReleaseDC(hdc);
}

#define DISP_START_X	160
#define DISP_START_Y	15
#define DISP_WIDTH	140
#define DISP_HEIGHT	185

static void StrechPhoto(int* width, int* height, int* x, int* y)
{
	int tw, th;
	static float r;			//缩放比例
	tw = *width;
	th = *height;

	//宽大于显示区域,高小于显示区域
	if (tw >= DISP_WIDTH && th <= DISP_HEIGHT)
	{
		r = (float)DISP_WIDTH / tw;
	}
	//宽小于显示区域,高大于显示区域
	else if (tw <= DISP_WIDTH && th >= DISP_HEIGHT)
	{
		r = (float)DISP_HEIGHT / tw;
	}
	//长宽都小于显示区域
	else if (tw <= DISP_WIDTH && th <= DISP_HEIGHT)
	{
		r = (float)DISP_WIDTH / tw;
		if (th*r > DISP_HEIGHT)
			r = (float)DISP_HEIGHT / th;
	}
	//长宽都大于显示区域
	else if (tw >= DISP_WIDTH && th >= DISP_HEIGHT)
	{
		r = (float)DISP_WIDTH / tw;
		if (th*r > DISP_HEIGHT)
			r = (float)DISP_HEIGHT / th;
	}

	//printf("strech rate:%f\n", r);
	tw = (int)tw * r;
	th = (int)th * r;
	*x = DISP_START_X + (int)((DISP_WIDTH - tw) / 2);
	*y = DISP_START_Y + (int)((DISP_HEIGHT - th) / 2);
	*width = tw;
	*height = th;
}

//显示用户照片
static int ShowPhotoProc(HDC hdc, char* pin2)
{
	BITMAP usrbmp;
	BITMAP bkbmp;
	BLOCKHEAP my_cliprc_heap;
	CLIPRGN my_cliprgn1;
	CLIPRGN my_cliprgn2;
	char photoname[50];
	int mount = -1;
	int ret = 0;

	memset(photoname, 0, sizeof(photoname));

	sprintf(photoname, "%s%s.jpg", GetPhotoPath(""), pin2);
	//printf("user photo path:%s\n", photoname);
	if (!LoadBitmap(hdc, &usrbmp, photoname))
	{
		int width, height;
		int sx = DISP_START_X;
		int sy = DISP_START_Y;

		width = usrbmp.bmWidth;
		height = usrbmp.bmHeight;
		//		printf("source width:%d, source height:%d, old point:(%d,%d)\n", width, height, sx, sy);
		StrechPhoto(&width, &height, &sx, &sy);
		//		printf("strech width:%d, strech height:%d, new point:(%d,%d)\n", width, height, sx, sy);

		if (LoadBitmap(hdc, &bkbmp, GetBmpPath("photo.jpg")))   //读取像框失败
		{
			InitFreeClipRectList(&my_cliprc_heap, 100);
			InitClipRgn(&my_cliprgn1, &my_cliprc_heap);
			InitClipRgn(&my_cliprgn2, &my_cliprc_heap);
			InitEllipseRegion(&my_cliprgn2, 85, 110, 68, 90);

			SubtractRegion(&my_cliprgn1, &my_cliprgn1, &my_cliprgn2);
			SelectClipRegion(hdc, &my_cliprgn1);
			OffsetRegion(&my_cliprgn2, 150, 0);

			XorRegion(&my_cliprgn1, &my_cliprgn1, &my_cliprgn2);
			SelectClipRegion(hdc, &my_cliprgn1);

			FillBoxWithBitmap(hdc, sx, sy, width, height, &usrbmp);
			UnloadBitmap(&usrbmp);
			EmptyClipRgn(&my_cliprgn1);
			EmptyClipRgn(&my_cliprgn2);
			DestroyFreeClipRectList(&my_cliprc_heap);
		}
		else
		{
			//GetBitmapFromDC(hdc, 148+gOptions.MainVerifyOffset, 3, 164, 209, &phbmp );
			//sphoto = 1;

			FillBoxWithBitmap(hdc, 148 + gOptions.MainVerifyOffset, 3, 164, 209, &bkbmp);
			if (gOptions.isRotateCamera)
			{
				if (usrbmp.bmWidth == gOptions.LCDWidth && usrbmp.bmHeight == gOptions.LCDHeight)
				{
					if (gOptions.EuropeDevice)
					{
						RotateScaledBitmapVFlip(hdc, &usrbmp, 70, -12, -90*64, gOptions.LCDWidth*0.59, 249*0.59);
					}
					else if (gOptions.RotateDev)
					{
						RotateScaledBitmapVFlip(hdc, &usrbmp, 70, -12, 270*64, gOptions.LCDWidth*0.59, 249*0.59);
					}
					else
					{
						RotateScaledBitmapVFlip(hdc, &usrbmp, 70, -12, 90*64, gOptions.LCDWidth*0.59, 249*0.59);
					}
				}
				else
				{
					FillBoxWithBitmap(hdc, sx + gOptions.MainVerifyOffset, sy, width, height, &usrbmp);
				}
			}
			else
			{
				FillBoxWithBitmap(hdc, sx + gOptions.MainVerifyOffset, sy, width, height, &usrbmp);
			}
			UnloadBitmap(&bkbmp);
		}
		UnloadBitmap(&usrbmp);
		ret = 1;
	}
	else
		ret = 0;

	if (mount == 0) DoUmountUdisk();
	return ret;
}

//处理抓拍照片
int captureexceedflag = 0;		//1. 警告  2. 溢出
int gCaptureErrTimes = 0;			//vf failled 3 times trigger capture
int gErrCaptureCLearDelayCnt = 0;
static BITMAP capture1;
static BITMAP capture2;
const char fn1[50] = {"/mnt/ramdisk/picture.jpg"};
extern int GrabPicture(char *filename);

extern void RotatePhotoPicture(HDC hdc, int left, int top, int width, int height, BITMAP* videomap);
int Processcapture(HWND hWnd)
{
	//zsliu test dddddddddddddddd
	//return 0;
	static HDC hdc;
	int ret;

	hdc = GetClientDC(hWnd);
#ifndef ZEM600
        KillTimer(hWnd, IDC_VF_TIMER);
#endif
        KillTimer(hWnd, IDC_VF_TIMER2);

	ret = GrabPicture((char*)&fn1);
	//printf("-------after grab picture ret:%d\n", ret);
	if (ret != 1)
	{
        	GetBitmapFromDC(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, &capture1);
        	ExPlayOtherVoice(VOICE_CAPTURE);
	        LoadBitmap(hdc, &capture2, (char *)&fn1);
		if (i353 ==2) {
				RotateScaledBitmapVFlip(hdc, &capture2, -90, -17, 270*64, gOptions.LCDWidth*0.59, 249*0.59);
		}else {			
			if (!gOptions.isRotateCamera)
			{
			    	FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, &capture2);
			}
			else
			{
				BITMAP bkbmp;
				if(!LoadBitmap(hdc, &bkbmp, GetBmpPath("photo.jpg")))
				{
					FillBoxWithBitmap(hdc, 0, 3, 154, 209, &bkbmp);
					UnloadBitmap(&bkbmp);
				}
			//	RotateScaledBitmapVFlip(hdc, &capture2, -82, -12, 90*64, gOptions.LCDWidth*0.59, 249*0.59);
				if(gOptions.EuropeDevice)
				{
					RotateScaledBitmapVFlip(hdc, &capture2, -82, -12, -90*64, gOptions.LCDWidth*0.59, 249*0.59);
				}
				else if (gOptions.RotateDev)
				{
					RotateScaledBitmapVFlip(hdc, &capture2, -82, -12, 270*64, gOptions.LCDWidth*0.59, 249*0.59);

				}
				else
				{
					RotateScaledBitmapVFlip(hdc, &capture2, -82, -12, 90*64, gOptions.LCDWidth*0.59, 249*0.59);
				}
			}
		}		
	        UnloadBitmap(&capture2);
	        //DelayMS(1500);
		DelayMS(300);
	        FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, &capture1);
	        UnloadBitmap(&capture1);
	}
#ifndef ZEM600
        SetTimer(hWnd, IDC_VF_TIMER, 100);
        SetTimer(hWnd, IDC_VF_TIMER2, 5);
#else
        SetTimer(hWnd, IDC_VF_TIMER2, 2);
#endif
	ReleaseDC(hdc);
	return ret;
}

//计算剩余存储空间
long Diskspaceused(char *path)
{
	long blocks_used = 0;
	long blocks_percent_used = 0;
	struct statfs s;

	if ((statfs(path, &s) == 0))
	{
		if ((s.f_blocks > 0))
		{
			blocks_used = s.f_blocks - s.f_bfree;
			blocks_percent_used = 0;
			if (blocks_used + s.f_bavail)
			{
				blocks_percent_used = (((long long)blocks_used) * 100 + (blocks_used + s.f_bavail) / 2) / (blocks_used + s.f_bavail);
			}
		}
	}
	//    DBPRINTF("disk used: %ld\n",blocks_percent_used);
	return blocks_percent_used;
}

int GetIndexNodeID(int sid)
{
	int i = sid;
	while (FDB_GetPhotoIndex(i, NULL) != NULL)
	{
		i++;
	}
	return i;
}

extern int gCurCaptureCount; //current capture picture count, for count capture
static int check_picture_capacity(void)
{
	int cnt = 0;
	long captureused = 0;

	cnt = gOptions.DelPictureCnt;
	if (gOptions.DelPictureCnt >= gOptions.maxCaptureCount) {
		cnt = 20;
	}

	if (gCurCaptureCount >= gOptions.maxCaptureCount) {
		captureexceedflag = 2;
		char buffer[60]={0};
		int oldcnt = gCurCaptureCount;
		if (cnt > 0 && (FDB_DelOldAttPicture(cnt)> 0)) {
			gCurCaptureCount-=cnt;
			captureexceedflag = 0;
			sprintf(buffer, "del pic-ori%d: new%d, %d, ok", oldcnt, FDB_CntPhotoCount(), gCurCaptureCount);
			write_tracemsg(buffer);
		} else {
			sprintf(buffer, "del pic-ori%d: new%d, %d, err", oldcnt, FDB_CntPhotoCount(), gCurCaptureCount);
			write_tracemsg(buffer);
			return FALSE;
		}
	}

	captureexceedflag = 0;
	captureused = Diskspaceused(GetCapturePath(""));

	//printf("limit valued:%d\n", gOptions.CaptureAlarm);
	if (captureused >= (100 - gOptions.CaptureAlarm)) {
		captureexceedflag = 1;
		if (gOptions.DelPictureCnt > 0 && (FDB_DelOldAttPicture(cnt)> 0)) {
			gCurCaptureCount-=cnt;
			captureexceedflag = 0;
		} else if (captureused >= ((100-gOptions.CaptureAlarm)+5)) {
			captureexceedflag=2;
			return FALSE;
		}
	}
	return TRUE;
}

extern void rotate_photo(char *input_file);
//存储抓拍照片
int savecapturepic(char *pin2)
{
#ifdef ZEM600
	static int count = 0;	//for save count
#endif
	char picbuf[128] = {0};
	char ymd[20];
	TPhotoIndex tmpnode;

	if (check_picture_capacity()==0) {
		write_tracemsg("picture capacity full");
		return 0;
	}

	memset(picbuf, 0, sizeof(picbuf));
	memset((void*)&tmpnode, 0, sizeof(TPhotoIndex));

	picid++;
	tmpnode.index = picid;

	GetTime(&gCurTime);
	//zsliu add
	if (gOptions.isUseHejiraCalendar) {
		ChrisToHejiraTTime(&gCurTime); //公历转伊朗日历
	}
	//zsliu add end ... ...
	
	sprintf(ymd, "%4d%02d%02d%02d%02d%02d", gCurTime.tm_year + 1900, gCurTime.tm_mon + 1, gCurTime.tm_mday,
			gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);
	if (pin2 == NULL) {
		sprintf(picbuf, "%s/bad/%s.jpg", GetCapturePath("capture"), ymd);
		tmpnode.pictype = 1;
	} else {
		sprintf(picbuf, "%s/pass/%s-%s.jpg", GetCapturePath("capture"), ymd, pin2);
		memcpy(tmpnode.userid, pin2, gOptions.PIN2Width);
		tmpnode.pictype = 0;
	}
	if (gOptions.isUseHejiraCalendar) {
		HejiraToChrisTTime(&gCurTime);  //伊朗日历转公历
	}
	tmpnode.createtime = OldEncodeTime(&gCurTime);
	capturetime = tmpnode.createtime;	//zsliu add

	int ret = fileCp(picbuf, fn1);
#ifdef ZEM600
	if (gOptions.IsRotatePhoto) {
		rotate_photo(picbuf);
	}
	if (++count >= 300) {
		/*clear system caches*/
		clearSystemCaches();
		count = 0;
	}
#endif
	//添加索引信息
	if (ret != FILECP_OK) {
		write_tracemsg("copy picture failure");
	}
	if (ret == FILECP_OK && (FDB_AddPhotoIndex(&tmpnode) == FDB_OK)) {
		gCurCaptureCount++;
	} else {
		picid--;
		write_tracemsg("write picture node  failure");
	}
	issavecapture = 1;

	return TRUE;
}


void InitParam(int mod)
{
	issavecapture=0;
	if (mod)
	{
		FPData.TempSize = 0;
		MFCardPIN = 0;
		memset(FingerTemplate, 0, sizeof(FingerTemplate));
		memset((void*)&AuthFPData, 0, sizeof(TFPResult));
		EnableMsgType(MSG_TYPE_FINGER, 1);
		EnableMsgType(MSG_TYPE_HID, 1);
		EnableMsgType(MSG_TYPE_MF, 1);
		EnableMsgType(MSG_TYPE_ICLASS, 1);
	}

	FPData.PIN = 0;
	memset((void*)&VSStatus, 0, sizeof(TVSStatus));
	memset(KeyBuffer, 0, sizeof(KeyBuffer));
	memset((void*)&vfUser, 0, sizeof(TUser));

	VerifyLogState = 0;
	bMFCardInfo = 0;
	retryflag = 0;
	groupVerify = 0;		//组合验证标志
	IsANDOperator = FALSE;	//多重验证标志
	WaitVerifyRetryCnt = gOptions.FPRetry;
	WaitVerifyDelayTime = (admflag) ? TIMEOUT_WAIT_ADMIN_VERIFY : WAIT_VERIFY_IDLE_TIME;

	ie_score = 0;
	ie_quality = 0;
	//printf("%s,score %d quality %d\n",__FUNCTION__,ie_score,ie_quality);
}

static void ShowVerifyState(HWND hWnd, int curVFStyle, int curVFState, int ismultVF)
{
	static HDC hdc;
	char hintstr[70];
	int tmpvalue = 0;
	static RECT rc = {0, 50, 320, 200};
	rc.right = gOptions.LCDWidth;
	rc.bottom = gOptions.LCDHeight;

	//printf(" lyy %s \n", __FUNCTION__);
	hdc = GetClientDC(hWnd);
	SelectFont(hdc, infofont);
	SetTextColor(hdc, PIXEL_lightwhite);
	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);

	if (curVFStyle == VFS_FP && curVFState != VF_READY && curVFState != VF_NEXT)
		InvalidateRect(hWnd, &verify_info_rc, TRUE);
	else
		InvalidateRect(hWnd, NULL, TRUE);

	switch (curVFState)
	{
		//等待开锁组合下一用户验证
		case VF_NEXT:
			ShowMainWindowDelay = 0;
			ShowWindow(acnownd, SW_HIDE);
			ShowWindow(pwdnownd, SW_HIDE);
			TextOut(hdc, rc.left + 10, rc.top + 40, LoadStrByID(HID_MUSER_OPEN1));
			break;

			//验证就绪状态
		case VF_READY:
			ShowMainWindowDelay = 0;
			WaitVerifyDelayTime = (admflag) ? TIMEOUT_WAIT_ADMIN_VERIFY : WAIT_VERIFY_IDLE_TIME;

			switch (curVFStyle)
			{
				//指纹验证
				case VFS_FP:
					ShowWindow(pwdnownd, SW_HIDE);

					if (!FPData.PIN && KeyBuffer[0] && !MFCardPIN)		//按键输入
					{
						TextOut(hdc, 2, 10, LoadStrByID(MID_ACNO));
						ShowWindow(acnownd, SW_SHOW);
						SetWindowText(acnownd, KeyBuffer);
					}
					else
					{
						ShowWindow(acnownd, SW_HIDE);
						if (admflag)
						{
							sprintf(hintstr, "%s %d", LoadStrByID(HID_VADMIN), WaitAdminRemainCnt);
							TextOut(hdc, 2, 10, hintstr);
							FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmplock);
						}
						if (FPData.PIN || MFCardPIN)
						{
							//char uinfo[40];
							char cpin2[30];
							TUser ts;

							if (MFCardPIN)
							{
								sprintf(cpin2, "%d", MFCardPIN);
								sprintf(hintstr, "%s: %s", LoadStrByID(MID_ACNO), cpin2);
								TextOut(hdc, rc.left + 10, rc.top + 20, hintstr);
								if (FDB_GetUserByCharPIN2(cpin2, &ts) != NULL)
								{
									char mynamename[100];		//modify by jazzy 2008.07.26
									memset(mynamename, 0, sizeof(mynamename));
									Str2UTF8(tftlocaleid, (unsigned char*)ts.Name, (unsigned char*)mynamename);
									sprintf(hintstr, "%s: %s", LoadStrByID(MID_NAME), mynamename);
									TextOut(hdc, rc.left + 10, rc.top + 40, hintstr);
								}
							}
							else
							{
								if (FDB_GetUser(FPData.PIN, &ts) != NULL)
								{
									sprintf(hintstr, "%s: %s", LoadStrByID(MID_ACNO), ts.PIN2);
									TextOut(hdc, rc.left + 10, rc.top + 20, hintstr);

									char mynamename[100];		//modify by jazzy 2008.07.26
									memset(mynamename, 0, sizeof(mynamename));
									Str2UTF8(tftlocaleid, (unsigned char*)ts.Name, (unsigned char*)mynamename);
									sprintf(hintstr, "%s: %s", LoadStrByID(MID_NAME), mynamename);
									TextOut(hdc, rc.left + 10, rc.top + 40, hintstr);
								}
							}
						}
					}
					sprintf(hintstr, "%s", LoadStrByID(HID_PLACEFINGER));
					EnableMsgType(MSG_TYPE_FINGER, 1);
					EnableMsgType(MSG_TYPE_HID, 0);
					EnableMsgType(MSG_TYPE_MF, 0);
					EnableMsgType(MSG_TYPE_ICLASS, 0);
					break;

				case VFS_CARD:
					ShowWindow(pwdnownd, SW_HIDE);
					ShowWindow(acnownd, SW_HIDE);

					if (!FPData.PIN && KeyBuffer[0])
					{
						TextOut(hdc, 2, 10, LoadStrByID(MID_ACNO));
						ShowWindow(acnownd, SW_SHOW);
						SetWindowText(acnownd, KeyBuffer);
					}
					else if (FPData.PIN)
					{
						TUser ts;
						if (FDB_GetUser(FPData.PIN, &ts) != NULL)
						{
							sprintf(hintstr, "%s: %s", LoadStrByID(MID_ACNO), ts.PIN2);
							TextOut(hdc, rc.left + 10, rc.top + 20, hintstr);

							char mynamename[100];		//modify by jazzy 2008.07.26
							memset(mynamename, 0, sizeof(mynamename));
							Str2UTF8(tftlocaleid, (unsigned char*)ts.Name, (unsigned char*)mynamename);
							sprintf(hintstr, "%s: %s", LoadStrByID(MID_NAME), mynamename);
							TextOut(hdc, rc.left + 10, rc.top + 40, hintstr);
						}
					}

					if (admflag)
					{
						sprintf(hintstr, "%s %d", LoadStrByID(HID_VADMIN), WaitAdminRemainCnt);
						TextOut(hdc, 2, 10, hintstr);
						FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmplock);
					}
					else
					{
						FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpcard);
					}

					sprintf(hintstr, "%s", LoadStrByID(HID_SHOWCARD));
					EnableMsgType(MSG_TYPE_HID, 1);
					EnableMsgType(MSG_TYPE_MF, 1);
					EnableMsgType(MSG_TYPE_ICLASS, 1);
					EnableMsgType(MSG_TYPE_FINGER, 0);
					break;

				case VFS_PWD:
					TextOut(hdc, 2, 10, LoadStrByID(MID_ACNO));
					ShowWindow(acnownd, SW_SHOW);
					SetWindowText(acnownd, KeyBuffer);

					TextOut(hdc, 2, 80, LoadStrByID(MID_PWD));
					ShowWindow(pwdnownd, SW_SHOW);
					SetWindowText(pwdnownd, "");
					SetFocusChild(pwdnownd);

					if (admflag)
						FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmplock);
					else
						FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmppwd);

					if (!retryflag)
						sprintf(hintstr, "%s", LoadStrByID(HID_INPUTPWD));
					else
						sprintf(hintstr, "%s", LoadStrByID(MID_RETRYPWD));
					EnableMsgType(MSG_TYPE_HID, 0);
					EnableMsgType(MSG_TYPE_FINGER, 0);
					EnableMsgType(MSG_TYPE_MF, 0);
					EnableMsgType(MSG_TYPE_ICLASS, 0);

					break;

				case VFS_PIN:
					TextOut(hdc, 2, 10, LoadStrByID(MID_ACNO));
					ShowWindow(acnownd, SW_SHOW);
					ShowWindow(pwdnownd, SW_HIDE);
					SetWindowText(acnownd, KeyBuffer);
					SetFocusChild(acnownd);

					if (admflag)
						FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmplock);

					if (!gOptions.IsOnlyRFMachine)
					{
						//Liaozz comment 20080925 fix bug second:12
						sprintf(hintstr, "%s", LoadStrByID(MID_NOACNOHINT));//Update Liaozz 20081014 "" to Load
						//Liaozz end
						EnableMsgType(MSG_TYPE_FINGER, 1);
					}
					else
					{
						sprintf(hintstr, "%s", LoadStrByID(HID_SHOWCARD));
					}
					EnableMsgType(MSG_TYPE_HID, 0);
					EnableMsgType(MSG_TYPE_MF, 0);
					EnableMsgType(MSG_TYPE_ICLASS, 0);

					break;
			}

			if (ismultVF)
			{
				TextOut(hdc, 10, 160, LoadStrByID(MID_GP_VF));
			}

			FillBoxWithBitmap(hdc, 0, 180, 0, 0, &mbmphint);
			TextOut(hdc, 35, 185, hintstr);
			break;

		case VF_FAILD:
			if (curVFStyle != VFS_PIN) ShowWindow(acnownd, SW_HIDE);
			ShowWindow(pwdnownd, SW_HIDE);
			switch (curVFStyle)
			{
				case VFS_FP:
					FillBoxWithBitmapFP(hdc, 160 + gOptions.MainVerifyOffset, 0, 0, 0, &mbmpfp);
					UnloadBitmap(&mbmpfp);

					if (admflag) {
						sprintf(hintstr, "%s  %d", LoadStrByID(HID_VADMIN), WaitAdminRemainCnt);
						TextOut(hdc, 2, 8, hintstr);
					}

					if (KeyBuffer[0]) {//1:1
						printf("0----------\n");
						sprintf(hintstr, "%s: %s", LoadStrByID(MID_ACNO), KeyBuffer);
						TextOut(hdc, rc.left + 10, rc.top + 20, hintstr);
						sprintf(hintstr, "%s: %s ", LoadStrByID(HID_SVERIFY), LoadStrByID(HID_1TO1));
						//printf("strleng:%d\n",strlen(hintstr));
						TextOut(hdc, rc.left + 10, rc.top + 40, hintstr);
						//						printf("%d, %d, %s, %d\n", ismultVF, FPData.PIN, KeyBuffer, WaitVerifyRetryCnt);
						if ((MFCardPIN || ismultVF || (!FPData.PIN && KeyBuffer[0]))
								&& WaitVerifyRetryCnt == 1) {
							FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpfail);
							TextOut(hdc, rc.left + 10, rc.top + 60, LoadStrByID(MID_VF_FAILED));
						} else {
							TextOut(hdc, rc.left + 10, rc.top + 60, LoadStrByID(HID_VFFAIL));
						}
					} else {			//1:N
						printf("1----------\n");
						//						printf(" failed here\n");
						sprintf(hintstr, "%s: %s", LoadStrByID(HID_SVERIFY), LoadStrByID(MID_FINGER));
						TextOut(hdc, rc.left + 10, rc.top + 20, hintstr);

						if (WaitVerifyRetryCnt == 1) {
							FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpfail);
							TextOut(hdc, rc.left + 10, rc.top + 40, LoadStrByID(MID_VF_FAILED));
						} else {
							TextOut(hdc, rc.left + 10, rc.top + 40, LoadStrByID(HID_VFFAIL));
						}
					}
					break;

				case VFS_CARD:
					FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpfail);
					if (((gOptions.IsSupportMF && !gOptions.MifareAsIDCard) ||
								(gOptions.IsSupportiCLSRW && !gOptions.iCLASSAsIDCard)) &&
							!gOptions.OnlyPINCard)
					{
						sprintf(hintstr, "%s: %s", LoadStrByID(HID_SVERIFY), LoadStrByID(MID_FPCARD_VF));
						TextOut(hdc, rc.left + 10, rc.top + 40, hintstr);

						if ((ismultVF || (!FPData.PIN && KeyBuffer[0])) && WaitVerifyRetryCnt == 1)
							TextOut(hdc, rc.left + 10, rc.top + 60, LoadStrByID(MID_VF_FAILED));
						else
							TextOut(hdc, rc.left + 10, rc.top + 60, LoadStrByID(HID_INVALIDCARD));
					}
					else
					{
						if (gcard)
						{	
							unsigned int aaa = ntohl(gcard);
							sprintf(hintstr, "%s: %10u", LoadStrByID(MID_CARD_CODE), aaa);
							TextOut(hdc, rc.left + 10, rc.top + 20, hintstr);
						}
						sprintf(hintstr, "%s: %s", LoadStrByID(HID_SVERIFY), LoadStrByID(HID_CARD));
						TextOut(hdc, rc.left + 10, rc.top + 40, hintstr);

						if ((ismultVF || (!FPData.PIN && KeyBuffer[0])) && WaitVerifyRetryCnt == 1)
							TextOut(hdc, rc.left + 10, rc.top + 60, LoadStrByID(MID_VF_FAILED));
						else
							TextOut(hdc, rc.left + 10, rc.top + 60, LoadStrByID(HID_CARD_NOTENROLLED));
					}
					if (!gOptions.VoiceOn)
					{
						ExBeep(1);
					}
					break;

				case VFS_PWD:
					SetWindowText(pwdnownd, "");
					FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpfail);
					sprintf(hintstr, "%s: %s", LoadStrByID(MID_ACNO), pwduser.PIN2);
					TextOut(hdc, rc.left + 10, rc.top + 20, hintstr);
					sprintf(hintstr, "%s: %s", LoadStrByID(HID_SVERIFY), LoadStrByID(MID_PWD));
					TextOut(hdc, rc.left + 10, rc.top + 40, hintstr);

					if ((ismultVF || (!FPData.PIN && KeyBuffer[0])) && WaitVerifyRetryCnt == 1)
						TextOut(hdc, rc.left + 10, rc.top + 60, LoadStrByID(MID_VF_FAILED));
					else
						TextOut(hdc, rc.left + 10, rc.top + 60, LoadStrByID(HID_VPFAIL));
					break;

				case VFS_PIN:
					FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpfail);
					TextOut(hdc, 2, 10, LoadStrByID(MID_ACNO));
					ShowVerifyHint(hWnd, LoadStrByID(HID_ERRORPIN), 1);
					break;
			}
			break;

		case VF_SUCCESS:
			ShowWindow(acnownd, SW_HIDE);
			ShowWindow(pwdnownd, SW_HIDE);

			if (admflag)
			{
				FillBoxWithBitmapFP(hdc, 160 + gOptions.MainVerifyOffset, 0, 0, 0, &mbmpfp);
				UnloadBitmap(&mbmpfp);

				if (adminvf)
				{
					if (gOptions.VoiceOn)
						ExPlayVoice(VOICE_INVALID_ADMIN);
					else
						ExBeep(1);

					sprintf(hintstr, "%s  %d", LoadStrByID(HID_VADMIN), WaitAdminRemainCnt);
					TextOut(hdc, 2, 8, hintstr);
					if (adminvf == 1)
						TextOut(hdc, 10, 30, LoadStrByID(HID_PRI_INVALID));
					if (adminvf == 2)
						TextOut(hdc, 10, 30, LoadStrByID(HID_ACCESSDENY));
					adminvf = 0;
					DelayMS(1000);
				}
			}
			else
			{
				switch (curVerifyStyle)
				{
					case VFS_FP:
						FillBoxWithBitmapFP(hdc, 160 + gOptions.MainVerifyOffset, 0, 0, 0, &mbmpfp);
						UnloadBitmap(&mbmpfp);
						sprintf(hintstr, "%s: %s", LoadStrByID(HID_SVERIFY), LoadStrByID(MID_FINGER));
						break;
					case VFS_CARD:
						sprintf(hintstr, "%s: %s", LoadStrByID(HID_SVERIFY), LoadStrByID(MID_CARD));
						break;
					case VFS_PWD:
						sprintf(hintstr, "%s: %s", LoadStrByID(HID_SVERIFY), LoadStrByID(MID_PWD));
						break;
					case VFS_PIN:
						sprintf(hintstr, "%s: %s", LoadStrByID(HID_SVERIFY), LoadStrByID(MID_ACNO));
						break;
				}
				TextOut(hdc, rc.left + 10, rc.top + 60, hintstr);
				sprintf(hintstr, "%s: %s", LoadStrByID(MID_ACNO), vfUser.PIN2);
				TextOut(hdc, rc.left + 10, rc.top + 20, hintstr);

				char mynamename[100];		//modify by jazzy 2008.07.26
				memset(mynamename, 0, sizeof(mynamename));
				Str2UTF8(tftlocaleid, (unsigned char*)vfUser.Name, (unsigned char*)mynamename);
				sprintf(hintstr, "%s: %s", LoadStrByID(MID_NAME), mynamename);
				TextOut(hdc, rc.left + 10, rc.top + 40, hintstr);
				if (VerifyLogState&LOG_REPEAT)
					TextOut(hdc, rc.left + 10, rc.top + 80, LoadStrByID(HID_ALREADY));
				else if (VerifyLogState&LOG_INVALIDUSER)
					TextOut(hdc, rc.left + 10, rc.top + 80, LoadStrByID(HID_PRI_INVALID));
				else if (VerifyLogState&LOG_INVALIDTIME)
					TextOut(hdc, rc.left + 10, rc.top + 80, LoadStrByID(HID_INVALIDTIME));
				else if (VerifyLogState&LOG_INVALIDCOMBO)
					TextOut(hdc, rc.left + 10, rc.top + 80, LoadStrByID(HID_MUSER_OPEN2));
				else if (VerifyLogState&LOG_ANTIPASSBACK)	//非法出入提示反潜
					TextOut(hdc, rc.left + 10, rc.top + 80, LoadStrByID(MID_ANTIPASSBACK));
				else
				{
					if (bnoworkstate)
						TextOut(hdc, rc.left + 10, rc.top + 80, LoadStrByID(HID_MUSTINOUT));
					else
					{	
						if((exceedflag == 1) && (gOptions.DelRecord == 0))
							TextOut(hdc, rc.left + 10, rc.top + 80, "认证失败!");
						else
							TextOut(hdc, rc.left + 10, rc.top + 80, LoadStrByID(HID_VSUCCESS));
					}
				}

#ifdef _TTS_
				if (strlen(vfUser.Name))
				{
					if (gOptions.TTS_KEY)
					{
						TTS_Say(vfUser.Name);
						TTS_Wait();
					}
				}
#endif
				if (gOptions.VoiceOn)
				{
					if (exceedflag != 1)
					{
						if (VerifyLogState <= LOG_REPEAT)
						{
							if (VerifyLogState&LOG_REPEAT)
								ExPlayVoice(VOICE_ALREADY_LOG);
							else
								ExPlayVoice(VOICE_THANK);
						}
					}
				}
				else
				{
					ExBeep(1);
				}
				if (exceedflag == 1)
				{
#ifdef ZEM600
					//sleep(3);
					//DelayMS(1500);
#endif
					TextOut(hdc, rc.left + 10, rc.top + 120, LoadStrByID(HID_EXCEED));
					if (gOptions.VoiceOn){
						ExPlayVoice(VOICE_NO_LOG_SPACE);
					}
					//if (gOptions.VoiceOn)
						//ExPlayVoice(VOICE_NO_LOG_RECSPACE);
					else
						ExBeep(1);
				}
				else if (exceedflag == 2)
				{
					TextOut(hdc, rc.left + 10, rc.top + 120, LoadStrByID(HID_LEFTSPACE));
					sprintf(hintstr, "%d", gOptions.MaxAttLogCount*10000 - FDB_CntAttLog());
					TextOut(hdc, rc.left + 20, rc.top + 140, hintstr);
				}

				//添加照片显示功能
				if (gOptions.IsSupportPhoto && gOptions.ShowPhoto)
				{
					/*后台比对显示用户照片，add by yangxiaolong,2011-7-11,start*/
					char FileName[30];
					char strFilePath[80];
					char strCmd[100];

					sprintf(FileName, "%s_Svr",vfUser.PIN2);
					if (!ShowPhotoProc(hdc, FileName))
					{
						if (!ShowPhotoProc(hdc, vfUser.PIN2))
						{
							FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpok);
						}
					}
					else
					{
						//显示后，删除该照片
						sprintf(strFilePath, "%s.jpg", GetPhotoPath(FileName));
						sprintf(strCmd, "rm %s && sync", strFilePath);
						systemEx(strCmd);
						sync();
					}
					/*后台比对显示用户照片，add by yangxiaolong,2011-7-11,end*/
				}
				else
				{
					if((exceedflag == 1) && (gOptions.DelRecord == 0))
					{
						FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpfail);
					}
					//					if(curVerifyStyle != VFS_FP)	//如果不是指纹验证，则显示验证成功图片
					else
					FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpok);
				}
				// 	UnloadBitmap(&mbmpok);
				EnableMsgType(MSG_TYPE_FINGER, TRUE);
				EnableMsgType(MSG_TYPE_HID, TRUE);
				EnableMsgType(MSG_TYPE_MF, TRUE);
				EnableMsgType(MSG_TYPE_ICLASS, TRUE);
			}
			break;
	}

	//抓拍照片空间警告
	if (gOptions.CameraOpen && !admflag && (curVFState == VF_FAILD || curVFState == VF_SUCCESS))
	{
		if (captureexceedflag == 1)//warning
			TextOut(hdc, rc.left + 10, rc.top + 160, LoadStrByID(MID_CAMERA_WARN1));
		else if (captureexceedflag == 2)//Full disk
		{
			TextOut(hdc, rc.left + 10, rc.top + 160, LoadStrByID(MID_CAMERA_WARN2));
		}
	}

	//刷卡后清空串口缓存区
	if (curVerifyStyle == VFS_CARD)
		ttl232.flush_input();

	ReleaseDC(hdc);
}

int SuperPassword(char str[8])
{
	int password;
	password = 9999 - atoi(str);
	password = password * password;
	return password;

}

void DoAlarmByErrTime(void)
{
	//	printf("gOptoins.ErrTimes:%d\n",gOptions.ErrTimes);
	if (gOptions.ErrTimes)
	{
		gErrTimesCLearDelayCnt = gOptions.ErrTimes * 15; //seconds=times*15
		gErrTimes++;
		if (gErrTimes >= gOptions.ErrTimes)
		{
			ExAlarm(0, 24*60*60);
		}
	}
}

unsigned char *fingerbuf2 = NULL;
static void ProcFpImage(HWND handle, PSensorBufInfo mysensor)
{
	HDC dc1;
	BITMAP mybmp;
	BLOCKHEAP my_cliprc_heap;
	int i;
	char *tmpbuf;
	char fingerbuf[150*1024];

	CLIPRGN my_cliprgn1;
	CLIPRGN my_cliprgn2;

	memcpy(fingerbuf, mysensor->DewarpedImgPtr, mysensor->DewarpedImgLen);
	tmpbuf = MALLOC(gOptions.ZF_WIDTH);
	for (i = 0;i < gOptions.ZF_HEIGHT / 2;i++)
	{
		memcpy(tmpbuf, fingerbuf + i*gOptions.ZF_WIDTH, gOptions.ZF_WIDTH);
		memcpy(fingerbuf + i*gOptions.ZF_WIDTH, fingerbuf + (gOptions.ZF_HEIGHT - i - 1)*gOptions.ZF_WIDTH, gOptions.ZF_WIDTH);
		memcpy(fingerbuf + (gOptions.ZF_HEIGHT - i - 1)*gOptions.ZF_WIDTH, tmpbuf, gOptions.ZF_WIDTH);
	}
	FREE(tmpbuf);

	write_bitmap(TMPBMPFILE, (unsigned char *)fingerbuf, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);

	RECT fp_rc = {140, 0, 320, 210};         //指纹图像显示区域
	fp_rc.right = gOptions.LCDWidth;
	fp_rc.bottom = gOptions.LCDHeight;

	InvalidateRect(handle, &fp_rc, TRUE);

	LoadBitmap(HDC_SCREEN, &mybmp, TMPBMPFILE);
	dc1 = GetClientDC(handle);

	InitFreeClipRectList(&my_cliprc_heap, 100);
	InitClipRgn(&my_cliprgn1, &my_cliprc_heap);
	InitClipRgn(&my_cliprgn2, &my_cliprc_heap);
	InitEllipseRegion(&my_cliprgn2, 85, 110, 68, 90);

	SubtractRegion(&my_cliprgn1, &my_cliprgn1, &my_cliprgn2);
	SelectClipRegion(dc1, &my_cliprgn1);

	OffsetRegion(&my_cliprgn2, 150 + gOptions.MainVerifyOffset, 0);

	XorRegion(&my_cliprgn1, &my_cliprgn1, &my_cliprgn2);
	SelectClipRegion(dc1, &my_cliprgn1);
	if (gOptions.FpSelect == 1 || gOptions.FpSelect == 2)
		FillBoxWithBitmap(dc1, 150 + gOptions.MainVerifyOffset, gOptions.sensorOffsetY, 180, 200, &mybmp);
	else
	{
		BITMAP finger;
		LoadBitmapFromFile(HDC_SCREEN, &finger, GetBmpPath("pic3.gif"));
		FillBoxWithBitmap(HDC_SCREEN, 160 + gOptions.MainVerifyOffset, 10, 0, 0, &finger);
		UnloadBitmap(&finger);
	}

	GetBitmapFromDC(dc1, 160 + gOptions.MainVerifyOffset, 0, 180, 200, &mbmpfp);		//截取指纹图像保存到内存

	UnloadBitmap(&mybmp);
	EmptyClipRgn(&my_cliprgn1);
	EmptyClipRgn(&my_cliprgn2);
	DestroyFreeClipRectList(&my_cliprc_heap);
	ReleaseDC(dc1);

}

BYTE LastTemplate[1024*3];

static void InitVerifyWindow(HWND hWnd)
{
	static HDC hdc;
	char hintstr[50];
	int tmpvalue = 0;
	if (gOptions.Language == 83)
	{
		logfont = CreateLogFont(NULL, "fixed", "GB2312", FONT_WEIGHT_BOLD, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
				FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 14, 0);
		infofont = CreateLogFont(NULL, "fixed", "GB2312", FONT_WEIGHT_BOLD, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
				FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 14, 0);
	}
	else
	{
		if (gfont1 == NULL)
		{
			logfont = CreateLogFont(NULL, "Times", "ISO8859-1", FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
					FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 16, 0);
			infofont = CreateLogFont(NULL, "Times", "ISO8859-1", FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
					FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 16, 0);
		}
		else
		{
			logfont = gfont1;
			infofont = gfont1;
		}
	}

	acnownd = CreateWindow(CTRL_SLEDIT, "", WS_TABSTOP | ES_BASELINE | WS_BORDER, IDC_INNO, 2, 30, 140, 30, hWnd, 0);
	SetWindowFont(acnownd, logfont);
	SendMessage(acnownd, EM_LIMITTEXT, gOptions.PIN2Width, 0L);

	pwdnownd = CreateWindow(CTRL_SLEDIT, "", WS_CHILD | WS_BORDER | ES_PASSWORD, IDC_INPWD, 2, 100, 140, 30, hWnd, 0);
	SetWindowFont(pwdnownd, logfont);
	SendMessage(pwdnownd, EM_LIMITTEXT, PWDLIMITLEN, 0L);

	hdc = GetClientDC(hWnd);
	SelectFont(hdc, infofont);
	SetTextColor(hdc, PIXEL_lightwhite);
	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);

	if (admflag)
	{
		FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmplock);
		sprintf(hintstr, "%s  %d", LoadStrByID(HID_VADMIN), WaitAdminRemainCnt);
		TextOut(hdc, 2, 8, hintstr);
	}
	else
	{
		switch (curVerifyStyle)
		{
			case VFS_PIN:
				TextOut(hdc, 2, 10, LoadStrByID(MID_ACNO));
				ShowWindow(acnownd, SW_SHOW);
				ShowWindow(pwdnownd, SW_HIDE);
				SetWindowText(acnownd, firstkeyValue);
				SetFocusChild(acnownd);
				break;

			case VFS_FP:
				//FillBoxWithBitmapFP(hdc, 160, 0, 0, 0, &mbmpfp);
				//UnloadBitmap(&mbmpfp);
				break;

			case VFS_CARD:
				FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmpcard);
				break;

			case VFS_PWD:
				FillBoxWithBitmap(hdc, 170 + gOptions.MainVerifyOffset, 50, 0, 0, &mbmppwd);
				break;
		}
		WaitVerifyDelayTime = (admflag) ? TIMEOUT_WAIT_ADMIN_VERIFY : WAIT_VERIFY_IDLE_TIME;

		if (curWorkCode || ownwkcd[0])
		{
			char ophintstr[50];
			char wkcdhint[256];

			if (curWorkCode)
			{
				TWORKCODE vfwkcd;
				memset(&vfwkcd, 0, sizeof(TWORKCODE));
				if (FDB_GetWorkCode(curWorkCode, &vfwkcd) != NULL)
				{
					sprintf(wkcdhint, "%s:%s", LoadStrByID(MID_WORKCODE), vfwkcd.Code);
				}
			}
			else if (ownwkcd[0])
			{
				sprintf(wkcdhint, "%s:%s", LoadStrByID(MID_WORKCODE), ownwkcd);
			}
			TextOut(hdc, 10, 160, wkcdhint);

			if (curVerifyStyle == VFS_FP)
				sprintf(ophintstr, "%s", LoadStrByID(HID_PLACEFINGER));
			else if (curVerifyStyle == VFS_CARD)
				sprintf(ophintstr, "%s", LoadStrByID(HID_SHOWCARD));

			FillBoxWithBitmap(hdc, 0, 180, 0, 0, &mbmphint);
			TextOut(hdc, 35, 185, ophintstr);
		}
	}
	ReleaseDC(hdc);
}

static int GetCurWorkCode(void)
{
	TWORKCODE tpwkcd;

	if (curWorkCode > 0)
	{
		memset(&tpwkcd, 0, sizeof(TWORKCODE));
		if (FDB_GetWorkCode(curWorkCode, &tpwkcd) != NULL)
		{
			return atoi(tpwkcd.Code);
		}
	}
	else if (strlen(ownwkcd) > 0)
		return atoi(ownwkcd);

	return 0;
}

extern int wkcdwinparam;
extern void ShowSelectStateHint(HWND hWnd);
extern long SSR_MENU_MAINMENU(HWND);
extern int iCLSRead(PFPCardOP fpdata, int OnlyPINCard);
int IdentifyFinger(char *InputPin, U32 PIN, BYTE *Temp, BYTE* Image, int RS485FPFlag);
int GetUserPIN2(U32 pin, char* buf);
extern void SwitchMsgType(int Enabled);
void OutputPrinterFmt1(int pin);
void OutputPrinterFmt2(int pin);
void OutputPrinterFmt3(int pin);
void OutputPrinterFmt4(int pin);
void OutputPrinterFmt5(int pin);
void OutputPrinterFmt6(int pin);
void OutputPrinterFmt7(int pin);
extern int myMessageBox1(HWND hwnd, DWORD dwStyle, char * title, char * text, ...);
BOOL CheckNextVerifyType(HWND hWnd, TVSStatus VSStatus, TVSStatus CurVSStatus, BOOL IsANDOperator);
void DisplayUserInfo(HWND hWnd, PFPResult risuser);
extern int FDB_DelOldAttLog(int delCount);
extern int CheckFaceStkey(HWND hWnd, int index , int flag);

/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,start*/
/*
 *Function:后台比对
 *Input Prama:PIN2--用户工号(1:N时，PIN2=0)
 pInfo--指纹或脸部信息
 *			InfoLen--pInfo信息长度
 *			MsgType--消息类型(指纹或者人脸)
 *Return:0 --认证失败
 *		1--认证成功
 */
TUser g_SvrAuthUsrInfo;	//pushsdk 后台验证，服务器返回用户信息
int IdentifyByServer(char *PIN2, void * pInfo, int InfoLen, int MsgType, TUser * pAuthUsrInfo)
{
	//int i;
	int u32AuthType;
	char *pcAuthInfo;
	//int wPrama;

	if (gOptions.iClockServerStatus != 0) //pushsdk不在线
	{
		return 0;
	}

	switch (MsgType)
	{
		case MSG_TYPE_FINGER:
			u32AuthType = AUTH_TYPE_FP;
			InfoLen = BIOKEY_EXTRACT(fhdl, (BYTE *)pInfo, LastTemplate,  EXTRACT_FOR_IDENTIFICATION);
			pcAuthInfo = (char*)LastTemplate;
			break;

		case MSG_TYPE_HID:
			u32AuthType = AUTH_TYPE_CARD;
			pcAuthInfo = pInfo;
			break;

		default:
			return 0;

	}

	if (InfoLen <= 0)
	{
		return 0;
	}

	return AuthFromHttpServer(PIN2, pcAuthInfo, InfoLen, pAuthUsrInfo, u32AuthType);
}
/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,end*/

static void play_voice_by_msg(WPARAM wParam, LPARAM lParam)
{
	if (wParam == News_FailedByMFCard || wParam == News_FailedByiCLASSCard)
	{
		if (gOptions.VoiceOn && WaitVerifyRetryCnt > 1)
			ExPlayVoice(VOICE_NOENROLLED_CARD);
		else
			ExBeep(1);
		FPData.TempSize = 0;
		memset(FingerTemplate, 0, sizeof(FingerTemplate));
	}
	else if (wParam == News_FailedByPIN)
	{
		if (gOptions.VoiceOn)
		{
			DelayMS(50);
			ExPlayVoice(VOICE_INVALID_ID);
		}
		else
			ExBeep(1);
	}
	else if (wParam == News_FailedByIDCard)
	{
		printf(">>>News_FailedByIDCard\n");
		if (gOptions.VoiceOn && WaitVerifyRetryCnt > 1)
			ExPlayVoice(VOICE_NOENROLLED_CARD);
		else
			ExBeep(1);

		if (gOptions.RS232Fun == 2)//简单输出ID号
		{
			char buf[32];
			memset(buf, 0, sizeof(buf));
			sprintf((char*)buf, "-C%010u\r\n", (unsigned int)lParam);
			SerialOutputString(&ff232, (char*)buf);
		}
	}
	else if (wParam == News_FailedByFP)
	{
		if (gOptions.VoiceOn && WaitVerifyRetryCnt > 1)
			ExPlayVoice(VOICE_RETRY_FP);
		else
			ExBeep(1);
	}
	else if (wParam == News_FailedByPwd)
	{
		if (gOptions.VoiceOn && WaitVerifyRetryCnt > 1)
			ExPlayVoice(VOICE_INVALID_PWD);
		else
			ExBeep(1);
	}
	return;
}

static void printer_print_log(HWND hWnd, int pin)
{
	if (gOptions.PrinterFunOn && !(VerifyLogState&LOG_REPEAT) && !bnoworkstate)
	{
		if (gOptions.PrinterOn == 1)
			OutputPrinterFmt1(pin);
		else if (gOptions.PrinterOn == 2)
			OutputPrinterFmt2(pin);
		else if (gOptions.PrinterOn == 3)
			OutputPrinterFmt3(pin);
		else if (gOptions.PrinterOn == 4)
			OutputPrinterFmt4(pin);
		else if (gOptions.PrinterOn == 5)
		{
			int dnu = DelayNextUser;
			DelayNextUser = 0;
			WaitVerifyDelayTime = 0;
			KillTimer(hWnd, IDC_VF_TIMER2);
			GetBitmapFromDC(HDC_SCREEN, 10, 60, 350, 120, &tmpbmp);
			if (myMessageBox1(hWnd, MB_OKCANCEL | MB_ICONQUESTION, LoadStrByID(MID_APPNAME), LoadStrByID(MID_PT_HINT)) == IDOK)
			{
				OutputPrinterFmt5(pin);
			}
			FillBoxWithBitmap(HDC_SCREEN, 10, 60, 0, 0, &tmpbmp);
#ifndef ZEM600
			SetTimer(hWnd, IDC_VF_TIMER2, 5);
#else
			SetTimer(hWnd, IDC_VF_TIMER2, 2);
#endif
			DelayNextUser = dnu;
			UnloadBitmap(&tmpbmp);
		}
		else if (gOptions.PrinterOn == 6)
			OutputPrinterFmt6(pin);
		else if (gOptions.PrinterOn == 7)
			OutputPrinterFmt7(pin);
	}

}

static void save_picture_by_pass(HWND hWnd, PUser user)
{
	char buf[128]={0};
	if (!gOptions.CameraOpen) {
		return;
	}

	TExtUser myextuser;
	memset(&myextuser, 0, sizeof(TExtUser));
	if (FDB_GetExtUser(user->PIN, &myextuser) == NULL || myextuser.reserved[0] == 0)
	{
		if ((gOptions.CapturePic == CAMERA_TAKEPHOTO) || (gOptions.CapturePic == CAMERA_TAKEPHOTO_AND_SAVE))
		{
			if (Processcapture(hWnd) != 1) {
				if (gOptions.CapturePic == CAMERA_TAKEPHOTO_AND_SAVE) {
					savecapturepic(user->PIN2);
				}
			} else {
				sprintf(buf, "%s: capture picture failure", user->PIN2);
				write_tracemsg(buf);
			}
		}
	}
	else if (myextuser.reserved[0] > 1)
	{
		if (Processcapture(hWnd) != 1) {
			if (myextuser.reserved[0] == 3) {
				savecapturepic(user->PIN2);
			}
		} else {
			sprintf(buf, "%s: capture picture failure", user->PIN2);
			write_tracemsg(buf);
		}
	}
}

static void save_picture_by_failure(HWND hWnd)
{
	if (!(gOptions.CameraOpen && gOptions.CapturePic && !admflag)) {
		return;
	}

	if (Processcapture(hWnd) != 1) {
		if (gOptions.CapturePic == CAMERA_TAKEPHOTO_AND_SAVE) {
			savecapturepic(NULL);
		} else if (gOptions.CapturePic == CAMERA_TAKEPHOTO_BY_FAILURE && (gOptions.CapturevfTimes > 0)) {
			gErrCaptureCLearDelayCnt = gOptions.CapturevfTimes * 5;
			gCaptureErrTimes++;
			if (gCaptureErrTimes >= gOptions.CapturevfTimes) {
				savecapturepic(NULL);
				gCaptureErrTimes = 0;
			}
		}
	} else {
		write_tracemsg("capture picture failure");
	}
	return ;

}

static int process_workcode(HWND hWnd)
{
	if (ifUseWorkCode && !curWorkCode && !ownwkcd[0] && gOptions.MustChoiceWorkCode)
	{
		SwitchMsgType(0);
		WaitVerifyDelayTime = 0;
		ShowMainWindowDelay = 0;
		wkcdwinparam = 1;
		memset(ownwkcd, 0, MAX_WORKCODE_LEN + 1);
		curWorkCode = CreateWorkCodeManageWindow(hWnd);
		wkcdwinparam = 0;
		SwitchMsgType(1);

		//如果没有选择WorkCode，则取消此次验证
		if (!curWorkCode && !ownwkcd[0])
		{
			//PostMessage(hWnd, MSG_CLOSE, 0, 0);
			return 0;
		}
	}
	return 1;
}

static int get_user_verification_type(int UserVS)
{
	if (!gOptions.UserExtendFormat)
	{
		//如果不使用extend模式的，默认原来
		//DBPRINTF("UserVS: %d\n",UserVS);
		switch (UserVS)
		{
			case VS_PW:
			case VS_FP_OR_PW_OR_RF:
				//UserVS=2;
				UserVS = 0;
				//UserVS=1;
				break;
			case VS_FP:
				UserVS = 1;
				//UserVS=0;
				break;
			case VS_RF:
				//UserVS=3;
				UserVS = 2;
				break;
			case VS_FP_AND_RF:
				UserVS = 1;
				break;
		}
	}
	return UserVS;
}

static int check_attlog_capacity(void)
{
	int flag = 0;
	int flag2 = 0;
	if (gOptions.AlarmAttLog || (gOptions.LoopWriteRecordOn < 0)) {
		if (CurAttLogCount >= gOptions.MaxAttLogCount*10000){
			flag = 1;
		} else if (CurAttLogCount + gOptions.AlarmAttLog >= gOptions.MaxAttLogCount*10000) {
			flag = 2;
		}

		if ((flag == 1) && gOptions.DelRecord > 0) {
			FDB_DelOldAttLog(gOptions.DelRecord);
			if (flag == 1)	{
				flag = 0;
			}
			flag2=1;
		}

		if (pushsdk_is_running() && flag2==1 ) {
			pushsdk_data_reset(FCT_ATTLOG);
		}
	} else {
		if (CurAttLogCount >= gOptions.MaxAttLogCount*10000){
			flag = 1;
			if ((flag == 1) && gOptions.DelRecord > 0) {
				FDB_DelOldAttLog(gOptions.DelRecord);
				if (flag == 1)	{
					flag = 0;
				}
				flag2=1;
			}
		} 
	}
	return flag;
}

extern void ShowMainLCD(HWND hWnd);
static int VerifyWinProc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	int i;
	BYTE *pImagePtr = NULL;
	U32 iImageLen = 0;
	static BYTE UserVS = VS_FP_OR_PW_OR_RF;
	static TVSStatus CurVSStatus = {FALSE, FALSE, FALSE, FALSE};
	U16 pin;
	PSensorBufInfo SensorInfo;
	int bSign = 0;
	U32 pin2 = 0;

	//printf("--%s-- message %d\n", __FUNCTION__, message);
	static char keyupFlag = 1;
	switch (message)
	{
		case MSG_CREATE:
			InitParam(1);
			InitVerifyWindow(hWnd);
#ifndef ZEM600
			SetTimer(hWnd, IDC_VF_TIMER, 100);  /* 设置定时器,以秒为单位,这里不要修改*/
			SetTimer(hWnd, IDC_VF_TIMER2, 1); /* 设置定时器,以50ms为单位,这里不要修改*/
#else
			SetTimer(hWnd, IDC_VF_TIMER2, 2); /* 设置定时器,以50ms为单位,这里不要修改*/
#endif
			return 0;

		case MSG_ERASEBKGND:
			{
				hdc = (HDC)wParam;
				const RECT* clip = (const RECT*)lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;
				if (hdc == 0)
				{
					hdc = GetClientDC(hWnd);
					fGetDC = TRUE;
				}
				if (clip)
				{
					rcTemp = *clip;
					ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient(hWnd, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect(hdc, &rcTemp);
				}

				if (gOptions.IsOnlyRFMachine)
					FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight - 30, &mbmpbk);
				else
					FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, &mbmpbk);

				if (fGetDC) ReleaseDC(hdc);
				break;
			}

		case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			EndPaint(hWnd, hdc);
			return 0;

		case MSG_KEYUP:
			if (gOptions.TFTKeyLayout == 3)
			{
				keyupFlag = 1;
			}
			break;
		case MSG_KEYDOWN:
			if (gOptions.TFTKeyLayout == 3) {
				if (keyupFlag == 1) {
					keyupFlag = 0;
				} else {
					break;
				}
			}
			if (gOptions.KeyPadBeep) {
				ExKeyBeep();
			}

			if (LOWORD(wParam) == SCANCODE_ESCAPE) {
				if (gOptions.TFTKeyLayout == 3) {
					keyupFlag = 1;
				}
				PostMessage(hWnd, MSG_CLOSE, 0, 0);
			}

			if ((LOWORD(wParam) >= SCANCODE_1 && LOWORD(wParam) <= SCANCODE_9) || LOWORD(wParam) == SCANCODE_0)
			{
#ifdef _TTS_
				if ((GetFocusChild(hWnd) != pwdnownd))
				{
					char Number[2];

					if (gOptions.TTS_KEY)
					{
						if (LOWORD(wParam) == SCANCODE_0)
							Number[0] = '0';
						else
							Number[0] = LOWORD(wParam) + 47;
						Number[1] = 0;
						TTS_Wait();
						TTS_Say(Number);
					}
				}
#endif
				//Liaozz 20081013 ShowSelectStateHint
				if (gOptions.MustChoiceInOut && gOptions.AttState < 0 && !admflag)
				{
					ShowSelectStateHint(hMainWindowWnd);
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				//Liaozz end
				//				printf("keycode:%d\n",wParam);
				if (curVerifyState != VF_READY || (curVerifyStyle != VFS_PIN && curVerifyStyle != VFS_PWD)) {
					InitParam(1);		//重新验证
					curVerifyStyle = VFS_PIN;
					curVerifyState = VF_READY;
					ShowMainWindowDelay = 0;
					DelayNextUser = 0;
					ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
				} else {
					WaitVerifyDelayTime = (admflag) ? TIMEOUT_WAIT_ADMIN_VERIFY : WAIT_VERIFY_IDLE_TIME;
				}

				break;
			}

			if ((wParam == SCANCODE_ENTER) || (LOWORD(wParam) == SCANCODE_F10))
			{
				WaitVerifyDelayTime = (admflag) ? TIMEOUT_WAIT_ADMIN_VERIFY : WAIT_VERIFY_IDLE_TIME;
				if (((curVerifyStyle == VFS_PIN && GetFocusChild(hWnd) == acnownd) ||
							(curVerifyStyle == VFS_FP && !FPData.PIN && !MFCardPIN)) && KeyBuffer[0])
				{
					char str[10];
					//					printf("KeyBuffer:%s\n",KeyBuffer);
					TUser user;
					i = CheckUserPIN(KeyBuffer, &user);
					if (i > 0)
					{
						GetWindowText(acnownd, str, gOptions.PIN2Width);
						if (admflag && atoi(str) == 8888)	//超级密码
						{
							if (!(ISADMIN(user.Privilege)))
							{
								bsupperpwd = 1;
								//进入密码验证方式
								curVerifyStyle = VFS_PWD;
								curVerifyState = VF_READY;
								ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
							}
							else
								SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByPIN, i);
						}
						else
						{
							SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByPIN, i);
						}
					}
					else
					{
						GetWindowText(acnownd, str, gOptions.PIN2Width);
						if (admflag && atoi(str) == 8888)	//超级密码
						{
							bsupperpwd = 1;
							//进入密码验证方式
							curVerifyStyle = VFS_PWD;
							curVerifyState = VF_READY;
							ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
						}
						else
						{
							/*异地考勤,add by yangxiaolong,2011-06-14,start*/
							if ((gOptions.RemoteAttFunOn == 1) && (KeyBuffer[0] != 0))
							{
								//本地不存在该用户，向服务器请求下载该用户信息
								if (DownLoadUser(KeyBuffer) == 1)
								{
									//保存用户下载时间戳,返回用户pin号
									i = SaveUserDLStamp(KeyBuffer);
									if (i > 0)
									{
										SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByPIN, i);
										break;
									}
								}
							}
							/*异地考勤,add by yangxiaolong,2011-06-14,end*/
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPIN, 0);
							memset(KeyBuffer, 0, sizeof(KeyBuffer));
							SetWindowText(acnownd, "");
							break;
						}
					}

				}
				else if (curVerifyStyle == VFS_PWD && (GetFocusChild(hWnd) == pwdnownd))
				{
					char str[20];
					char tpwdstr[8];
					TUser tmpuser;
					Clock mclock;
					int OldState = gMachineState;  //changed by cxp at 2010-04-22

					GetWindowText(pwdnownd, tpwdstr, 8);
					GetWindowText(acnownd, str, gOptions.PIN2Width);

					if (bsupperpwd && admflag)		//超级密码
					{
						GetClockDT(&mclock);
						memset(str, 0, sizeof(str));
						sprintf(str, "%02d%02d", mclock.Hour, mclock.Min);
						if (atoi(tpwdstr) == SuperPassword(str))
						{
							//管理员验证成功,关闭验证窗
							menuflag = 1;
							KillTimer(hWnd, IDC_VF_TIMER2);
#ifndef ZEM600
							KillTimer(hWnd, IDC_VF_TIMER);
							KillTimer(hMainWindowWnd, IDC_TIMER);
#else
							EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
							gMachineState = STA_MENU;  //changed by cxp at 2010-04-22
							EnableMsgType(MSG_TYPE_FINGER, 0);//changed by luoxw 20120307
							SSR_MENU_MAINMENU(hWnd);
							EnableMsgType(MSG_TYPE_FINGER, 1);//changed by luoxw 20120307
							gMachineState = OldState;  //changed by cxp at 2010-04-22
							menuflag = 0;
#ifndef ZEM600
							SetTimer(hMainWindowWnd, IDC_TIMER, 100);
#else
							EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
							PostMessage(hWnd, MSG_CLOSE, 0, 0);
						}
						else
						{
							//密码验证失败
							adminvf = 2;
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPwd, 0);
						}
					}
					else
					{
						memset(&tmpuser, 0, sizeof(TUser));
						if (FDB_GetUserByCharPIN2(str, &tmpuser) != NULL)
						{
							if (tmpuser.Password[0] && nstrcmp(tpwdstr, tmpuser.Password, 8) == 0)
								SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByPwd, tmpuser.PIN);
							else
								SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPwd, 0);
						}
						else
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPIN, 0);
					}
				}
				break;
			}

			//查看临时考勤记录
			if (wParam == SCANCODE_MENU)
			{
				TTime ttlog;
				if (curVerifyState == VF_SUCCESS || curVerifyState == VF_NEXT)// || curVerifyState==VF_DOOROPEN)
				{
					GetTime(&ttlog);
					time_t t = OldEncodeTime(&ttlog);
					if (gOptions.ViewAttlogFunOn && ((t - LastTime) < 10) && (t >= LastTime))
					{
						time_t logst, loged;
						TUser logquser;

						ttlog.tm_hour = 0;
						ttlog.tm_min = 0;
						ttlog.tm_sec = 0;
						ttlog.tm_isdst = -1;
						logst = OldEncodeTime(&ttlog);
						loged = logst + OneDay - OneSecond;
						memset(&logquser, 0, sizeof(TUser));
						SwitchMsgType(0);
#ifndef ZEM600
						KillTimer(hWnd, IDC_VF_TIMER);
#endif
						KillTimer(hWnd, IDC_VF_TIMER2);
						FDB_GetUser(LastUID, &logquser);
						ShowMainWindowDelay = 0;
						DelayNextUser = 0;
						WaitVerifyDelayTime = 0;
						if (!CreateAttLogWindow(hWnd, logquser.PIN2, logst, loged, 1))
						{
							;
						}
						LastTime = -1;
						LastUID = -1;
						SwitchMsgType(1);
						if (curVerifyState == VF_NEXT)
						{
							DelayNextUser = DELAYNEXTUSER;
							ShowVerifyState(hWnd, 0, curVerifyState, 0);
#ifndef ZEM600
							SetTimer(hWnd, IDC_VF_TIMER, 100);
							SetTimer(hWnd, IDC_VF_TIMER2, 5);
#else
							SetTimer(hWnd, IDC_VF_TIMER2, 2);
#endif
						}
						else
							PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
				}
				break;
			}
			//回退键
			if (wParam == SCANCODE_BACKSPACE)
			{
				if (curVerifyStyle == VFS_PIN && !retryflag)
				{
					char str[30];
					memset(str, 0, sizeof(str));
					GetWindowText(acnownd, str, 30);
					if (strlen(str) == 1)
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
				break;
			}
#ifdef FACE
			int stkyidx = isShortKey(LOWORD(wParam));
			if (stkyidx)
			{
				if (CheckFaceStkey(hWnd, stkyidx, 1))
				{
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
#endif
			break;

		case MSG_COMMAND:
			if (LOWORD(wParam) == IDC_INNO && HIWORD(wParam) == EN_CHANGE)
			{
				memset(KeyBuffer, 0, sizeof(KeyBuffer));
				GetWindowText(acnownd, KeyBuffer, 24);
				if (strlen(KeyBuffer))
				{
					if (KeyBuffer[0] == '0')
					{
						SetWindowText(acnownd, "");
						break;
					}
				}
				if (KeyBuffer && strlen(KeyBuffer) >= 1)
					SendMessage(acnownd, EM_SETCARETPOS, 0, strlen(KeyBuffer));
			}
			break;

		case MSG_TYPE_HID:
			{
				TUser user;
				U8 card[4];
				HDC hdc_hid;
				int tmpPin;  //changed by cxp
				int recardtime;
				recardtime = LoadInteger("ReadCardInterval", 3);
				cardtime2 = time(NULL);
				printf("[%s] inteval=%d, recardtime = %d\n",__FUNCTION__, (unsigned int)(cardtime2 -cardtime1), recardtime);
				if((unsigned int)(cardtime2 -cardtime1) < recardtime){
					printf("[%s] inteval=%d\n",__FUNCTION__, (unsigned int)(cardtime2 -cardtime1));
					msleep(1000*(recardtime - (int)(cardtime2 -cardtime1)));
					break;
				}
				cardtime1 = cardtime2;
				ExLightLED(LED_RED, FALSE);
				ExLightLED(LED_GREEN, FALSE);

				memset(card, 0, sizeof(card));
				memcpy(card, (void*)&lParam, 4);
				gcard = lParam;

				memset(FingerTemplate, 0, sizeof(FingerTemplate));
				FPData.TempSize = 0;

				bSign = FALSE;

				if (wParam == TYPE_SLAVE_ID) {
					char slavepin2[30];

					memset(slavepin2, 0, sizeof(slavepin2));
					sprintf(slavepin2, "%d", (int)lParam);
					memset(&vfUser, 0, sizeof(TUser));

					if (gOptions.WiegandInType) {
						ShowBitmap(hWnd, mbmpcard);
					}

					if ((!gOptions.WiegandInType && FDB_GetUserByCharPIN2(slavepin2, &vfUser) != NULL)
							|| (gOptions.WiegandInType && FDB_GetUserByCard(card, &vfUser) != NULL))
					{
						FPData.PIN = vfUser.PIN;
						hdc_hid = GetClientDC(hWnd);

						ledbool = FALSE;
						if (!admflag) {
							led_greenlight_on(gOptions.IsFlashLed);
						}

						int result=SaveAttLog(FPData.PIN, TYPE_SLAVE_ID, hdc_hid, hWnd);
						ReleaseDC(hdc_hid);

						curVerifyStyle = (gOptions.WiegandInType) ? VFS_CARD : VFS_PIN;
						if (result==FDB_OK) {
							curVerifyState = VF_SUCCESS;
						} else {
							curVerifyState = VF_FAILD;
						}
						ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
						InitParam(1);
						ShowMainWindowDelay = 5;
					} else {
						/*dsl 2011.5.30.Fix When Wingand input type is ID.No, display Card information*/
						curVerifyStyle = (gOptions.WiegandInType > 0) ? VFS_CARD : VFS_PIN;
						SendMessage(hWnd, MSG_TYPE_CMD, 
							(gOptions.WiegandInType) ? News_FailedByIDCard : News_FailedByPIN, 0);
					}
					break;
				}

				curVerifyStyle = VFS_CARD;
				/*restore default value of g_iSlaveID*/
				set_verification_type(wParam); 
				if ((wParam == ONLY_LOCAL) || (wParam == LOCAL_NETWORK) 
					|| (gOptions.Reader485On && Is485ReaderMaster()	&& MSG_INBIO_COMM_CMD == wParam))
				{
					memset(&user, 0, sizeof(TUser));
					if (FDB_GetUserByCard(card, &user) != NULL) {
						tmpPin = atoi(user.PIN2); //changed by cxp at 2010-04-22
						CheckSessionSend(EF_VERIFY, (char*)(&tmpPin), 4); //changed by cxp at 2010-04-21

						//dsl 2012.3.26
						/*verify the card from the slave device */
						set_verification_type(wParam);
						set_fpreader_msg_flag(wParam);

						if ((!FPData.PIN) || (user.PIN == FPData.PIN)) {//组合验证
							SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByIDCard, user.PIN);
							break;
						} else if (wParam == ONLY_LOCAL) {
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 0);
							break;
						}
					} else if (wParam == ONLY_LOCAL) {
						SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 0);
						break;
					}
				}
				if (((wParam == LOCAL_NETWORK) ) || (wParam == ONLY_NETWORK) || (wParam == NETWORK_LOCAL))
				{
					if (KeyBuffer[0] != 0)
					{
						if (wParam != LOCAL_NETWORK)
						{
							bSign = 3; //刷卡1:1方式验证不支持后台不对，转为本地验证
						}
					}
					else if(gOptions.AuthServerCheckMode == AUTH_PUSH)
					{
						char strCard[26];
						memset(strCard, 0, sizeof(strCard));
						sprintf(strCard, "%u", ntohl(gcard));
						if (IdentifyByServer(KeyBuffer, strCard, strlen(strCard), MSG_TYPE_HID, &g_SvrAuthUsrInfo))
						{
							bSign = 2;
							gSensorNo = 2;

							AuthFPData.PIN = atoi(g_SvrAuthUsrInfo.PIN2);
							strcpy(AuthFPData.Name, g_SvrAuthUsrInfo.Name);
							SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByIDCardRIS, AuthFPData.PIN);
							break;
						}
					}

					if (gOptions.AuthServerCheckMode != AUTH_PUSH)
					{
						/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,end*/
						if (UploadIDCardToAuthServer(gcard, &AuthFPData) && AuthFPData.PIN)
						{
							//		printf("upload card:%d name:%s\n",AuthFPData.PIN,AuthFPData.Name);
							bSign = 2;
							SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByIDCardRIS, AuthFPData.PIN);
						}
						break;
					}
					if ((wParam == NETWORK_LOCAL) || (bSign == 3))
					{
						memset(&user, 0, sizeof(TUser));
						if (FDB_GetUserByCard(card, &user) != NULL)
						{
							bSign = 1;
							tmpPin = atoi(user.PIN2); //changed by cxp at 2010-04-22
							CheckSessionSend(EF_VERIFY, (char*)(&tmpPin), 4); //changed by cxp at 2010-04-21
							//CheckSessionSend(EF_VERIFY,(char*)user.PIN2,5);  //changed by cxp at 2010-04-21

							if ((!FPData.PIN) || (user.PIN == FPData.PIN)) //组合验证
							{
								SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByIDCard, user.PIN);
							}
							else
							{
								SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 0);
							}
							break;
						}
						else
						{
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 0);
							break;
						}
					}
				}

				if (!bSign){
					SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 0);
				}
				break;
			}

		case MSG_TYPE_ICLASS:
			{
				U32 fpdata_pin;
				char userpin2[32];
				int iclspin;
				BYTE card[4];
				if (FPData.TempSize > 0 && gOptions.ShowName) break;
				curVerifyStyle = VFS_CARD;
				ShowBitmap(hWnd, mbmpcard);		//显示卡图标
				ExLightLED(LED_RED, FALSE);
				ExLightLED(LED_GREEN, FALSE);

				//验证是否为有效卡（登记卡时将卡ID写入用户Card中）
				memcpy(card, (void*)&lParam, 4);
				if (gOptions.MustEnroll && FDB_GetUserByCard(card, NULL) == NULL)
				{
					gcard = lParam;
					SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByiCLASSCard, 0);
					return 0;
				}

				ShowVerifyInfo(hWnd, LoadStrByID(HID_WORKING), 0);
				memset(FingerTemplate, 0, sizeof(FingerTemplate));
				fpdata_pin = FPData.PIN;
				FPData.Templates = FingerTemplate;
				FPData.OP = OP_READ;
				FPData.TempSize = iCLSRead(&FPData, gOptions.iCLASSAsIDCard);
				MFCardPIN = FPData.PIN;
				FPData.PIN = fpdata_pin;
				if (FPData.TempSize < 0)
				{
					gcard = 0;
					SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByMFCard, 0);
					return 0;
				}

				sprintf(userpin2, "%d", MFCardPIN);		//读取指纹卡ID

				iclspin = CheckUserPIN(userpin2, NULL);
				if (iclspin != fpdata_pin)		//非同一用户，重新开始验证
					InitParam(0);

				if (gOptions.OnlyPINCard)
				{
					memset(&vfUser, 0, sizeof(TUser));
					gcard = MFCardPIN;
					if (FDB_GetUserByCharPIN2(userpin2, &vfUser))
					{
						gcard = MFCardPIN;
						ExBeep(1);
						SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByIDCard, vfUser.PIN);
					}
					else
					{
						gcard = 0;
						memset(FingerTemplate, 0, sizeof(FingerTemplate));
						MFCardPIN = 0;
						SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 0);
					}
				}
				else
				{
					//				printf("FPData.TempSize:%d\n",FPData.TempSize);
					if ((FPData.TempSize >= 0) && gOptions.MustEnroll && iclspin != -1)
					{
						ExBeep(1);
						SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByiCLASSCard, iclspin);
					}
					else if ((FPData.TempSize >= 0) && !gOptions.MustEnroll)
					{
						//					printf("UserPIN:%d\n",CheckUserPIN(userpin2));
						ExBeep(1);
						if (iclspin != -1)
							SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByiCLASSCard, iclspin);
						else
						{
							curVerifyStyle = VFS_FP;
							curVerifyState = VF_READY;
							memset(KeyBuffer, 0, sizeof(KeyBuffer));
							sprintf(KeyBuffer, "%d", MFCardPIN);
							ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
						}
					}
					else
					{
						if (FPData.TempSize == MFCARD_ERROR_UNKNOWN) ExBeep(1);
						memset(FingerTemplate, 0, sizeof(FingerTemplate));
						MFCardPIN = 0;
						SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByiCLASSCard, 0);
					}
				}
				break;
			}

		case MSG_TYPE_MF:
			{
				U32 fpdata_pin = 0;
				int mfupin = 0;
				char userpin2[32];
				BYTE card[4];

				if (FPData.TempSize > 0 && gOptions.ShowName) break;
				curVerifyStyle = VFS_CARD;
				ShowBitmap(hWnd, mbmpcard);		//显示卡图标
				ExLightLED(LED_RED, FALSE);
				ExLightLED(LED_GREEN, FALSE);

				//验证是否为有效卡（登记卡时将卡ID写入用户Card中）
				memcpy(card, (void*)&lParam, 4);
				if (gOptions.MustEnroll && FDB_GetUserByCard(card, NULL) == NULL)
				{
					gcard = lParam;
					SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByMFCard, 0);
					return 0;
				}

				memset(FingerTemplate, 0, sizeof(FingerTemplate));
				fpdata_pin = FPData.PIN;		//暂存用户PIN值
				FPData.Templates = FingerTemplate;
				FPData.OP = OP_READ;
				FPData.TempSize = MFRead(&FPData, gOptions.OnlyPINCard);

				//保存读到的用户ID
				MFCardPIN = FPData.PIN;
				//恢复FPData.PIN值
				FPData.PIN = fpdata_pin;
				if (FPData.TempSize < 0)
				{
					gcard = 0;
					SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByMFCard, 0);
					return 0;
				}

				//根据ID查找用户
				sprintf(userpin2, "%d", MFCardPIN);

				mfupin = CheckUserPIN(userpin2, NULL);
				if (mfupin != fpdata_pin)				//非同一用户，重新开始验证
					InitParam(0);

				if (gOptions.OnlyPINCard)
				{
					memset(&vfUser, 0, sizeof(TUser));
					if (FDB_GetUserByCharPIN2(userpin2, &vfUser))
					{
						MFFinishCard();
						gcard = MFCardPIN;
						ExBeep(1);
						SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByIDCard, vfUser.PIN);
					}
					else
					{
						memset(FingerTemplate, 0, sizeof(FingerTemplate));
						MFCardPIN = 0;
						SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 0);
					}
				}
				else
				{
					if ((FPData.TempSize >= 0) && gOptions.MustEnroll && (mfupin != -1))
					{
						MFFinishCard();
						gcard = MFCardPIN;
						ExBeep(1);
						SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByMFCard, mfupin);
					}
					else if ((FPData.TempSize >= 0) && !gOptions.MustEnroll)
					{
						MFFinishCard();
						gcard = MFCardPIN;
						ExBeep(1);
						if (mfupin != -1)
						{
							SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByMFCard, mfupin);
						}
						else
						{
							curVerifyStyle = VFS_FP;
							curVerifyState = VF_READY;
							memset(KeyBuffer, 0, sizeof(KeyBuffer));
							sprintf(KeyBuffer, "%d", MFCardPIN);
							ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
						}
					}
					else
					{
						if (FPData.TempSize == MFCARD_ERROR_UNKNOWN) ExBeep(1);
						memset(FingerTemplate, 0, sizeof(FingerTemplate));
						MFCardPIN = 0;
						SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByMFCard, 0);
					}
				}
				break;
			}

		case MSG_TYPE_FINGER:
			{
				/*restore the default value of g_iSlaveID*/
				set_verification_type(wParam);

				//必须1:1比对指纹
				//printf("get finger in MSG_TYPE_FINGER\n");
				WaitProcessFingerFlag = 0; 
				ExBeep(1);//dsl 2011.4.7
				if (KeyBuffer && !KeyBuffer[0] && !MFCardPIN && gOptions.Must1To1 && (FPData.PIN == 0))
				{
					curVerifyStyle = VFS_PIN;
					curVerifyState = VF_READY;
					ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, 0);
					FlushSensorBuffer();
					break;
				}

				curVerifyStyle = VFS_FP;

				SensorInfo = (PSensorBufInfo)lParam;
				ExLightLED(TRUE, FALSE);
				ExLightLED(LED_GREEN, FALSE);
				
				//dsl 2012.3.26.
				int oldtype=set_fppic_display_type(3, wParam);
				ProcFpImage(hWnd, SensorInfo);
				//dsl 2012.3.26
				set_fppic_display_type(oldtype, wParam);

				//1:1指纹验证前先判断输入的是否为有效ID<07.09.15>
				//if(KeyBuffer[0] && !MFCardPIN)
				//support RIS 07.11.02
				if (((wParam == ONLY_LOCAL) || (wParam == LOCAL_NETWORK)) && KeyBuffer[0] && !MFCardPIN)
				{
					if (FDB_GetUserByCharPIN2(KeyBuffer, NULL) == NULL)
					{
						if (wParam == ONLY_LOCAL)
						{
							memset(KeyBuffer, 0, sizeof(KeyBuffer));
							curVerifyStyle = VFS_PIN;
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPIN, 0);
							return 0;
						}
						else if (wParam == LOCAL_NETWORK)
						{
							bSign = 1;
							goto Verify_LN;
						}
					}
				}

				i = 0;
				if ((wParam == ONLY_LOCAL) || (wParam == LOCAL_NETWORK)) //dewarped image
				{
					if (!KeyBuffer[0])
					{
						FPData.PIN = 0;
					}

					i = IdentifyFinger(KeyBuffer, FPData.PIN, FingerTemplate, SensorInfo->DewarpedImgPtr, 0);
					if (i == -100) {
						ShowMainLCD(hWnd);
						break;
					}

					if (bMFCardInfo)		//指纹卡1：1比对成功(机器内无该用户信息)
					{
						char myPIN2[24];

						memset(&vfUser, 0, sizeof(TUser));
						vfUser.PIN = MFCardPIN;
						sprintf(myPIN2, "%d", MFCardPIN);
						memcpy(vfUser.PIN2, myPIN2, gOptions.PIN2Width);

						//SaveAttLog
						//printf("PIN:%d,time:%d, VerifyType:%d, AttState:%d, PIN2:%s, WorkCode:%d, SensorNo:%d\n",0,OldEncodeTime(&gCurTime),3,gOptions.AttState,myPIN2,GetCurWorkCode(),gSensorNo);
						int ret=FDB_AddAttLog(0, OldEncodeTime(&gCurTime), 3, gOptions.AttState, myPIN2, 
								GetCurWorkCode(), gSensorNo);
						curWorkCode = 0;
						memset(ownwkcd, 0, sizeof(ownwkcd));

						//open door?
						if (gOptions.LockFunOn && gLockForceAction == FORCENONE 
								&& gOptions.LockOn && ret==FDB_OK) { /*dsl 2012.4.23*/
							DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
						}
						/*dsl 2012.4.23*/
						if (ret==FDB_OK) {
							curVerifyState = VF_SUCCESS;
						} else {
							curVerifyState = VF_FAILD;
						}
						ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
						InitParam(1);
						VerifiedPIN = 0;	//pin;              //记录验证成功的用户PIN
						ShowMainWindowDelay = 5;
						break;
					} else if (i > 0) {
						gSensorNo = SensorInfo->SensorNo;
						SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByFP, i);
						break;
					} else {
						bSign = 1;
					}
				}
Verify_LN:
				if (((wParam == LOCAL_NETWORK) && (bSign == 1)) || (wParam == ONLY_NETWORK) || (wParam == NETWORK_LOCAL))
				{
					char *fingerbuf = NULL;
					ShowMainWindowDelay = 0;
					if (gOptions.ASDewarpedImage)
					{
						U16 zfwidth, zfheight;
						zfwidth = gOptions.ZF_WIDTH;
						zfheight = gOptions.ZF_HEIGHT;
						iImageLen = gOptions.ZF_WIDTH * gOptions.ZF_HEIGHT;
						iImageLen += 4;
						//iImageLen=280*330;
						//DBPRINTF("testDewarpedImage: RawImgLen: %d\tdewlen:%d\n",SensorInfo->RawImgLen,iImageLen);
						fingerbuf = MALLOC(150 * 1024);
						memcpy(fingerbuf, &zfwidth, 2);
						memcpy(fingerbuf + 2, &zfheight, 2);
						memcpy(fingerbuf + 4, SensorInfo->RawImgPtr, SensorInfo->DewarpedImgLen);
						pImagePtr = (unsigned char*)fingerbuf;
					}
					else
					{
						iImageLen = SensorInfo->RawImgLen;
						pImagePtr = SensorInfo->RawImgPtr;
					}

					memset(&AuthFPData, 0, sizeof(TFPResult));
					//for RIS 1:1
					//printf("RIS KeyBuffer:%s\n",KeyBuffer);
					if (KeyBuffer[0])
					{
						pin2 = atoi(KeyBuffer);
					}

					if (KeyBuffer[0] == 0)
					{
						if (wParam != LOCAL_NETWORK)
						{
							bSign = 3;
						}
					}
					else  if (gOptions.AuthServerCheckMode == AUTH_PUSH)
					{
						if (IdentifyByServer(KeyBuffer, pImagePtr, iImageLen, MSG_TYPE_FINGER, &g_SvrAuthUsrInfo))
						{
							bSign = 2;
							gSensorNo = 2;

							AuthFPData.PIN = atoi(g_SvrAuthUsrInfo.PIN2);
							strcpy(AuthFPData.Name, g_SvrAuthUsrInfo.Name);

							SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByFPRIS, AuthFPData.PIN);
							break;
						}
					}
					if (gOptions.AuthServerCheckMode != AUTH_PUSH)
					{
						/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,end*/
						if (SendImageAndIdentify((char*)pImagePtr, iImageLen, pin2, &AuthFPData) && AuthFPData.PIN)
						{
							gSensorNo = 2;
							//printf("AuthFpData.PIN:%d\n",AuthFPData.PIN);
							SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByFPRIS, AuthFPData.PIN);
							if (fingerbuf)
								FREE(fingerbuf);
							break;
						} else {
							if (wParam == NETWORK_LOCAL)
							{
								i = IdentifyFinger(KeyBuffer, FPData.PIN, FingerTemplate, SensorInfo->DewarpedImgPtr, 0);
								if (i > 0)
								{
									gSensorNo = SensorInfo->SensorNo;
									SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByFP, i);
									if (fingerbuf)
										FREE(fingerbuf);
									break;
								}
								else
									bSign = 2;
							}
							if (fingerbuf) {
								FREE(fingerbuf);
								fingerbuf = NULL;
							}
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByFP, 0);
							break;
						}
					}
				}
	
				if(Is485ReaderMaster() && is_slavedevice_verification())// 判断是否是 485 读头采集到的指纹信号
				{
					//dsl 2012.3.26. for rs485 fp reader
					if ((i=identify_finger_by_template(KeyBuffer, FPData.PIN, wParam, lParam)) > 0)
					{
						/*verify the FP from the slave device */
						set_verification_type(wParam);
						SendMessage(hWnd, MSG_TYPE_CMD, News_VerifiedByFP, i);
						break;
					}else{
						SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByFP, 0);	
						break;
					}
				}
				else {
					bSign = 3;
				}
				//verification failed
				if (bSign)
				{
					SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByFP, 0);
				}
				//			printf("MSG_TYPE_FINGER\n");
				break;
			}

		case MSG_TYPE_CMD:
			switch (wParam)
			{
				case News_VerifiedByPIN:
				case News_VerifiedByPwd:
				case News_VerifiedByFP:
				case News_VerifiedByIDCard:
				case News_VerifiedByMFCard:
				case News_VerifiedByIDCardRIS:
				case News_VerifiedByFPRIS:
				case News_VerifiedByiCLASSCard:
					if (FPData.PIN && (FPData.PIN != lParam) && !retryflag) //PIN must be same
					{
						if ((wParam == News_VerifiedByFP) || (wParam == News_VerifiedByFPRIS))
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByFP, 1);
						else if ((wParam == News_VerifiedByIDCard) || (wParam == News_VerifiedByIDCardRIS))
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 1);
						else if (wParam == News_VerifiedByMFCard)
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByMFCard, 1);
						else if (wParam == News_VerifiedByiCLASSCard)
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByiCLASSCard, 1);
						else if (wParam == News_VerifiedByPIN)
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPIN, 1);
						else if (wParam == News_VerifiedByPwd)
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPwd, 1);
						break;
					}

					if (FPData.PIN == 0)// || retryflag) //first verify then get the value of VS
					{
						//printf(">>> PIN=%u\n", lParam);
						UserVS = GetExtUserVSByPIN(lParam, wParam);
						//printf("UserVs: %d\n",UserVS);
						memset(&CurVSStatus, 0, sizeof(TVSStatus));
						IsANDOperator = FALSE;
						//Check verification types
						switch (UserVS)
						{
							case VS_FP_OR_PW_OR_RF:
								CurVSStatus.FP = TRUE;
								if ((wParam != News_VerifiedByMFCard) && (wParam != News_VerifiedByiCLASSCard))
								{
									CurVSStatus.Password = TRUE;
									CurVSStatus.Card = TRUE;
								}
								break;
							case VS_FP:
								CurVSStatus.FP = TRUE;
								break;
							case VS_PIN:
								CurVSStatus.PIN = TRUE;
								break;
							case VS_PW:
								CurVSStatus.Password = TRUE;
								break;
							case VS_RF:
								CurVSStatus.Card = TRUE;
								break;
							case VS_FP_OR_PW:
								CurVSStatus.FP = TRUE;
								CurVSStatus.Password = TRUE;
								break;
							case VS_FP_OR_RF:
								CurVSStatus.FP = TRUE;
								CurVSStatus.Card = TRUE;
								break;
							case VS_PW_OR_RF:
								CurVSStatus.Password = TRUE;
								CurVSStatus.Card = TRUE;
								break;
							case VS_PIN_AND_FP:
								CurVSStatus.FP = TRUE;
								CurVSStatus.PIN = TRUE;
								IsANDOperator = TRUE;
								break;
							case VS_FP_AND_PW:
								CurVSStatus.FP = TRUE;
								CurVSStatus.Password = TRUE;
								IsANDOperator = TRUE;
								break;
							case VS_FP_AND_RF:
								CurVSStatus.FP = TRUE;
								CurVSStatus.Card = TRUE;
								IsANDOperator = TRUE;
								break;
							case VS_PW_AND_RF:
								CurVSStatus.Password = TRUE;
								CurVSStatus.Card = TRUE;
								IsANDOperator = TRUE;
								break;
							case VS_FP_AND_PW_AND_RF:
								CurVSStatus.FP = TRUE;
								CurVSStatus.Password = TRUE;
								CurVSStatus.Card = TRUE;
								IsANDOperator = TRUE;
								break;
							case VS_PIN_AND_FP_AND_PW:
								CurVSStatus.FP = TRUE;
								CurVSStatus.Password = TRUE;
								CurVSStatus.PIN = TRUE;
								IsANDOperator = TRUE;
								break;
							case VS_FP_AND_RF_OR_PIN:
								IsANDOperator = TRUE;
								if ((wParam == News_VerifiedByFP) ||
										(wParam == News_VerifiedByFPRIS))
								{
									CurVSStatus.FP = TRUE;
									CurVSStatus.Card = TRUE;
								}
								else if ((wParam == News_VerifiedByIDCard) ||
										(wParam == News_VerifiedByIDCardRIS) ||
										(wParam == News_VerifiedByMFCard) ||
										(wParam == News_VerifiedByiCLASSCard))
								{
									CurVSStatus.Card = TRUE;
									CurVSStatus.FP = TRUE;
								}
								else if (wParam == News_VerifiedByPIN)
								{
									CurVSStatus.FP = TRUE;
									CurVSStatus.PIN = TRUE;
								}
								break;
						}
					}

					//precheck verification types
					if ((wParam == News_VerifiedByFP) || (wParam == News_VerifiedByFPRIS))
					{
						if ((!CurVSStatus.FP) || (CurVSStatus.PIN && !VSStatus.PIN && IsANDOperator))
						{
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByFP, 2);
							break;
						}

					}
					else if ((wParam == News_VerifiedByIDCard) || (wParam == News_VerifiedByIDCardRIS))
					{
						if (!CurVSStatus.Card && !CurVSStatus.FP && !CurVSStatus.Password)
						{
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByIDCard, 2);
							break;
						}
					}
					else if (wParam == News_VerifiedByMFCard || wParam == News_VerifiedByiCLASSCard)
					{
						if (!CurVSStatus.Card && !CurVSStatus.FP && !CurVSStatus.Password)
						{
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByMFCard, 2);
							break;
						}
					}
					else if (wParam == News_VerifiedByPIN)
					{
						if (!CurVSStatus.PIN && !CurVSStatus.FP && !CurVSStatus.Password)
						{
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPIN, 2);
							break;
						}
					}
					else if (wParam == News_VerifiedByPwd)
					{
						if (!CurVSStatus.Password)
						{
							SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPwd, 2);
							break;
						}
					}

					//dsl 2012.3.26
					return_msg_to_fpreader(TRUE);

					//verification check
					if ((wParam == News_VerifiedByFP) || (wParam == News_VerifiedByFPRIS))
					{
						VSStatus.FP = TRUE;
					}
					else if ((wParam == News_VerifiedByIDCard) || (wParam == News_VerifiedByMFCard) ||
							(wParam == News_VerifiedByIDCardRIS) || (wParam == News_VerifiedByiCLASSCard))
					{
						VSStatus.Card = TRUE;
					}
					else if (wParam == News_VerifiedByPIN)
					{
						VSStatus.PIN = TRUE;
					}
					else if (wParam == News_VerifiedByPwd)
					{
						VSStatus.Password = TRUE;
						if (gOptions.LockFunOn && (DelayTriggerDuress || gOptions.DuressPwd))
						{
							DelayTriggerDuress = 10;
						}
					}

					if (FPData.PIN == 0)
					{
						FPData.PIN = lParam;
						if (!KeyBuffer[0])      //第一次验证，保留PIN
						{
							GetUserPIN2(FPData.PIN, (char *)KeyBuffer);
						}
					}

					ledbool = FALSE;
					if (!admflag) {
						led_greenlight_on(gOptions.IsFlashLed);
					}

					//	printf("FPData.PIN:%d,KeyBuffer:%s\n",FPData.PIN,KeyBuffer);
					gErrTimes = 0;
					if (retryflag) retryflag = 0;	//清除重试标志

#if 1
					if (CheckNextVerifyType(hWnd, VSStatus, CurVSStatus, IsANDOperator))
					{
						PUser pu=NULL;
						TUser user;
						int OldState = gMachineState;  //changed by cxp at 2010-04-22
						if (gOptions.IsSupportAuthServer && ((gOptions.AuthServerCheckMode == 0) 
							|| (gOptions.AuthServerCheckMode == 2)) && AuthFPData.PIN)
						{
							pin = 0;
						} else {
							//dsl 2012.3.26
							//pin = GetUserPIN_16(lParam);
							memset((void*)&user, 0, sizeof(TUser));
							pu=FDB_GetUser((U16)lParam, &user);
							pin=user.PIN;
						}
						curVerifyState = VF_SUCCESS;	//验证成功
						//处理管理员验证结果
						if (WaitAdminRemainCnt && admflag)
						{
							//pu = FDB_GetUser((U16)pin, NULL);
							if (pu && ISADMIN(pu->Privilege))
							{
								if (ISINVALIDUSER(*pu))		//非法管理员
								{
									adminvf = 1;
									ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
									if (WaitAdmVerifyRetryCnt > 0)  //重新验证管理员
									{
										--WaitAdmVerifyRetryCnt;
										InitParam(1);
										/*
										delete by zxz 2013-02-19
										if (!gOptions.IsOnlyRFMachine) {
											curVerifyStyle = VFS_FP;
										} else {
											curVerifyStyle = VFS_CARD;
										}
										*/
										curVerifyState = VF_READY;
										retryflag = 1;
										DelayMS(1000);
										ShowVerifyState(hWnd,curVerifyStyle,curVerifyState,IsANDOperator);
										//dsl 2011.4.12, back to main screen quickly.
										if (!WaitAdmVerifyRetryCnt) {
											ShowMainWindowDelay = 1;
										}
									} else {
										//dsl 2011.4.12, back to main screen quickly.
										ShowMainWindowDelay = 1;//=3;
									}
								} else if (WaitAdminRemainCnt >= 1) {	//
									//dsl 2012.4.17
									memcpy(&gAdminUser, pu, sizeof(TUser));

									WaitAdminRemainCnt = 0;
									gLocalCorrectionImage = FALSE;
									ledbool = TRUE;
#ifdef IKIOSK
									if (funkeyflag) {
										funkeyadmin = 1;
									}
									else
#endif
									{
										menuflag = 1;
										KillTimer(hWnd, IDC_VF_TIMER2);
#ifndef ZEM600
										KillTimer(hWnd, IDC_VF_TIMER);
										KillTimer(hMainWindowWnd, IDC_TIMER);
#else
										EnableMsgType(MSG_TYPE_TIMER, 0);
#endif
										gMachineState = STA_MENU;  //changed by cxp at 2010-04-22
										EnableMsgType(MSG_TYPE_FINGER, 0);//changed by luoxw 20120307
										SSR_MENU_MAINMENU(hWnd);
										EnableMsgType(MSG_TYPE_FINGER, 1);//changed by luoxw 20120307
										gMachineState = OldState;  //changed by cxp at 2010-04-22
										menuflag = 0;
#ifndef ZEM600
										SetTimer(hMainWindowWnd, IDC_TIMER, 100);
#else
										EnableMsgType(MSG_TYPE_TIMER, 1);
#endif
									}
									PostMessage(hWnd, MSG_CLOSE, 0, 0);
								} else if (WaitAdminRemainCnt > 0) {
									for (i = 0;i < MaxAdminVerify;i++) {
										if (WaitAdmins[i] == pu->PIN) {
											break;
										}
									}
									if (i == MaxAdminVerify) {
										WaitAdminRemainCnt--;
									}
								}
							} else {//else if(pu && ISADMIN(pu->Privilege))
								adminvf = 2;
								ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
								if (WaitAdmVerifyRetryCnt > 0) {	//重新验证管理员
									--WaitAdmVerifyRetryCnt;
									InitParam(1);
									
									/*
									//delete by zxz 2013-02-19
									if (!gOptions.IsOnlyRFMachine) {
										curVerifyStyle = VFS_FP;
									} else {
										curVerifyStyle = VFS_CARD;
									}
									*/
									curVerifyState = VF_READY;
									retryflag = 1;
									DelayMS(1000);
									ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
									//dsl 2011.4.12, back to main screen quickly.
									if (!WaitAdmVerifyRetryCnt) {
										ShowMainWindowDelay = 1;
									}
								} else {
									//dsl 2011.4.12, back to main screen quickly.
									ShowMainWindowDelay = 1;//3;
								}
							}
						}
						else //else if(WaitAdminRemainCnt && admflag)
						{
							gCaptureErrTimes = 0;
							//处理胁迫验证信号
							if (gOptions.LockFunOn && DelayTriggerDuress)
							{
								TriggerDuress((U16)lParam, TRUE);
							}
							//user not exist
							if (!pin && (AuthFPData.PIN || (!gOptions.MustEnroll && VSStatus.Card)))
							{
								DisplayUserInfo(hWnd, &AuthFPData);
							} else {
								//验证方式, 2012.3.26
								UserVS=get_user_verification_type(UserVS);
								
								memset(&vfUser, 0, sizeof(TUser));
								FDB_GetUser(FPData.PIN, &vfUser);

								//dsl 2012.3.26
								exceedflag = check_attlog_capacity();

								//select WorkCode
								if (!process_workcode(hWnd)) {//dsl 2012.3.26
									PostMessage(hWnd, MSG_CLOSE, 0, 0);
									return 0;
								}
								
								//拍照处理
								save_picture_by_pass(hWnd, &vfUser); //dsl 2012.3.26

								if (is_slavedevice_verification()) {
									UserVS = TYPE_SLAVE_ID;
								}

								//写考勤记录
								int ret=0;
								if ((ret=SaveAttLog(pin, UserVS, hdc, hWnd)) == FDB_OK){
									curVerifyState = VF_SUCCESS;
								} else {
									curVerifyState = VF_FAILD;
									memset((void*)KeyBuffer, 0, sizeof(KeyBuffer));

									ExBeep(1);
									ledbool = FALSE;
									led_redlight_on(gOptions.IsFlashLed);
								}
								ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);

								//串口打印(liming)
								if (ret==FDB_OK) {
									printer_print_log(hWnd, pin); //dsl 2012.3.26
								}
							}
							InitParam(1);
							if (bnoworkstate) {
								ShowMainWindowDelay = 3;
							} else if (DelayNextUser) {
								DelayMS(1000);
								DelayNextUser = DELAYNEXTUSER;
								curVerifyState = VF_NEXT;
								ShowVerifyState(hWnd, 0, curVerifyState, 0);
							} else {
								ShowMainWindowDelay = 5;
							}
							bnoworkstate = 0;
						}
					}
#endif
					break;

				case News_FailedByPIN:
				case News_FailedByPwd:
				case News_FailedByFP:
				case News_FailedByIDCard:
				case News_FailedByMFCard:
				case News_FailedByiCLASSCard:
					{
						//dsl 2012.3.26
						return_msg_to_fpreader(FALSE);

						ledbool = FALSE;
						led_redlight_on(gOptions.IsFlashLed);

						//dsl 2012.3.26
						save_picture_by_failure(hWnd);

						if (gOptions.CameraOpen)
							issavecapture = 0;

						if (gOptions.LockFunOn) DoAlarmByErrTime();

						//dsl 2012.3.26
						play_voice_by_msg(wParam, lParam);

						curVerifyState = VF_FAILD;
						ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
						if (!retryflag && (FPData.PIN || admflag))
							retryflag = 1;

						VerifiedPIN = 0;
						if (KeyBuffer[0] || MFCardPIN || admflag)	//1:1
						{
							if (WaitVerifyRetryCnt)
							{
								if (!--WaitVerifyRetryCnt)
								{
									VSStatus.Card = FALSE;
									VSStatus.Password= FALSE;
									VSStatus.PIN= FALSE;
									VSStatus.FP= FALSE;
									FPData.PIN = 0;

									if (gWGFailedID >= 0)
										WiegandSend(gWGSiteCode < 0 ? gOptions.DeviceID : gWGSiteCode, gWGFailedID, 0);
									ShowMainWindowDelay = 3;
									if (KeyBuffer[0])
									{
										KeyBuffer[0] = 0;
										WaitVerifyRetryCnt = gOptions.FPRetry;
									}
								}
								else
								{
									DelayMS(1000);
									if (FPData.PIN)
										CheckNextVerifyType(hWnd, VSStatus, CurVSStatus, IsANDOperator);
									else
									{
										curVerifyState = VF_READY;
										ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
									}
									//								if(!KeyBuffer[0]) FPData.PIN=0;
								}
							}
							else
							{
								//未注册卡延时500ms
								if (wParam == News_FailedByMFCard || wParam == News_FailedByiCLASSCard)
									DelayMS(500);
								ShowMainWindowDelay = 5;
							}
						}
						else	//1:N验证
						{
							if (gOptions.ExtWGInFunOn == 1)//zsliu add
							{
								if (gWGFailedID >= 0)
									WiegandSend(gWGSiteCode < 0 ? gOptions.DeviceID : gWGSiteCode, gWGFailedID, 0);
							}

							//未注册卡延时500ms
							if (wParam == News_FailedByMFCard || wParam == News_FailedByiCLASSCard)
								DelayMS(500);
							ShowMainWindowDelay = 5;
						}
						break;
					}
			}

			break;

		case MSG_TYPE_TIMER:
		case MSG_TIMER:
			if (wParam == IDC_VF_TIMER2)
			{
				if (m)
				{
					m = 0;
					second_msg=0;
					//printf("set m and second_msg 0\n");
					if (Fwmsg.Message != MSG_TYPE_TIMER)
					{
						DelayNextUser = 0;
						ShowMainWindowDelay = 0;
						WaitVerifyDelayTime = 0; //(admflag) ? TIMEOUT_WAIT_ADMIN_VERIFY : WAIT_VERIFY_IDLE_TIME;
					}
					if (gOptions.MustChoiceInOut && gOptions.AttState < 0 &&
							(Fwmsg.Message == MSG_TYPE_FINGER ||
							 Fwmsg.Message == MSG_TYPE_HID ||
							 Fwmsg.Message == MSG_TYPE_MF ||
							 Fwmsg.Message == MSG_TYPE_ICLASS) && !admflag)//add "&& !admflag" condition Liaozz 20080926
					{
						SendMessage(hWnd, MSG_CLOSE, 0, 0);
						ShowSelectStateHint(hMainWindowWnd);
					}
					else
					{
						SendMessage(hWnd, Fwmsg.Message, Fwmsg.Param1, Fwmsg.Param2);
					}
				}
				else if (second_msg==MSG_TYPE_TIMER)
				{
					Fwmsg.Message=MSG_TYPE_TIMER;
					Fwmsg.Param1=0;
					Fwmsg.Param2=0;
					second_msg=0;
					SendMessage(hWnd, Fwmsg.Message, Fwmsg.Param1, Fwmsg.Param2);
				}
				return 0;
			}
			else if (wParam == IDC_VF_TIMER || message== MSG_TYPE_TIMER)
			{
				if (ShowMainWindowDelay) {
					if (!--ShowMainWindowDelay) {
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
				}
				if (WaitVerifyDelayTime) {
					if (!--WaitVerifyDelayTime) {
						ExBeep(1);
						InitParam(1);
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
				}
				if (DelayNextUser) {
					if (!--DelayNextUser) {
						ExBeep(1);
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					}
				}
				if (gErrCaptureCLearDelayCnt) {
					if (!--gErrCaptureCLearDelayCnt) {
						gCaptureErrTimes = 0;
					}
				}
				if (WaitDuressAlarm) {
					if (!--WaitDuressAlarm)	{
						ExAlarm(0, 24*60*60);
					}
				}
				if (DelayTriggerDuress) {
					DelayTriggerDuress--;
				}
				if (gErrTimesCLearDelayCnt) {
					if (!--gErrTimesCLearDelayCnt) {
						gErrTimes = 0;
					}
				}
				if (ledbool) {
					FlashRedLED = led_flash(RTCTimeValidSign, FlashRedLED);
				}
				return 0;
			}
			break;

		case MSG_CLOSE:
#ifndef ZEM600
			KillTimer(hWnd, IDC_VF_TIMER);
#endif
			KillTimer(hWnd, IDC_VF_TIMER2);
			if (gfont1 == NULL)
			{
				DestroyLogFont(infofont);
				DestroyLogFont(logfont);
			}
			EnableMsgType(MSG_TYPE_FINGER, 1);
			EnableMsgType(MSG_TYPE_HID, 1);
			EnableMsgType(MSG_TYPE_MF, 1);
			EnableMsgType(MSG_TYPE_ICLASS, 1);

			ClearMemberRecord();		//清除组合开锁记录

			//clear workcode
			ShowMainWindowDelay = 0;
			curWorkCode = 0;
			memset(ownwkcd, 0, sizeof(ownwkcd));
			if (mbmpfp.bmBits) {
				UnloadBitmap(&mbmpfp);
				mbmpfp.bmBits = NULL;
			}
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			break;

		default:
			return DefaultMainWinProc(hWnd, message, wParam, lParam);

	}
	return (0);

}

int Filter_Group_Run(int TID)
{
	int i, count = gOptions.MaxUserCount * 100;
	int pin = TID & 0xFFFF;

	for (i = 0;i < count;i++)
	{
		if (gFilterBuf[i].PIN == pin) return TRUE;
		if (gFilterBuf[i].PIN == 0) break;
	}
	return FALSE;
}

int Filter_Head_Run(int TID)
{
	int i, count = gOptions.MaxUserCount * 100;
	U32 pin = TID & 0xFFFF;
	U32 ipin = gInputPin, res = 1;

	for (i = 0;i < count;i++)
	{
		if (gFilterBuf[i].PIN == pin)
		{
			if (gOptions.PIN2Width > PIN_WIDTH)
				strtou32((void*)gFilterBuf[i].PIN2, &pin);
			if (pin == 0) break;
			while (ipin <= pin)
			{
				U32 ipn = ipin * 10;
				if (ipn > pin) break;
				if (ipin > (U32)0xFFFFFFFD / 10) break;
				ipin = ipn;
				res *= 10;
			}
			if (ipin <= pin)
			{
				if (res > pin - ipin) return TRUE;
			}
			break;
		}
		if (gFilterBuf[i].PIN == 0) break;
	}
	return FALSE;
}

int GetUserPIN2(U32 pin, char* buf)
{
	PUser u;
	u = FDB_GetUser(pin, NULL);
	if (u)
	{
		nmemcpy((unsigned char*)buf, (unsigned char*)u->PIN2, gOptions.PIN2Width);
		return 1;
	}
	return 0;
}

U16 GetUserPIN_16(U32 pin)
{
	PUser u;
	u = FDB_GetUser(pin, NULL);
	if (u)
	{
		return u->PIN;
	}
	else
	{
		return 0;
	}
}

#define LOWQLT -100
int IdentifyFinger(char *InputPin, U32 PIN, BYTE *Temp, BYTE* Image, int RS485FPFlag)
{
	int result = -1, score = 0, i, FingerID = 0;
	char Buffer[32];
	int LastTempLen = 0;
	TUser ttuser;

	//	printf("InputPin:%s,PIN:%lu,Temp:%s\n",InputPin,PIN,Temp);
	//	unsigned int s_msec, e_msec;
	//	s_msec=GetTickCount1();
	
	if (RS485FPFlag) {
		ExtractFingerTemplate();
	} else {
		LastTempLen = BIOKEY_EXTRACT(fhdl, Image, LastTemplate,  EXTRACT_FOR_IDENTIFICATION);
	}
	//printf(" end EXTRACT*****************\n");

	//	e_msec=GetTickCount1();
	//	DBPRINTF("EXTRACT FINISHED! time=%d tmplen=%d\n", e_msec-s_msec, LastTempLen);

	if (LastTempLen > 0)
	{
		if (!InputPin[0] && !Temp[0] && !PIN)  //1:many
		{
			//s_msec=GetTickCount1();
			{
				score = gOptions.MThreshold;
				BIOKEY_DB_FILTERID_ALL(fhdl);
				if (!BIOKEY_IDENTIFYTEMP(fhdl, (BYTE*)LastTemplate, &result, &score))
				{
					result = -1;
				}
				else
				{
					FingerID = (result >> 16) & 0xf;
					result &= 0xFFFF;
				}
				printf("FingerID=%d, result=%d\n", FingerID, result);
			}
			//e_msec=GetTickCount1();

		}
		else if (Temp[0]) //1:1 fingerprint - MIFARE
		{
			char tmppin2[24];
			TUser pur;

			sprintf(tmppin2, "%d", MFCardPIN);

			BIOKEY_MATCHINGPARAM1(fhdl, SPEED_LOW, gOptions.VThreshold); //use low speed matching for verification
			result = -1;
			i = 0;
			bMFCardInfo = 0;
			while (i < gOptions.RFCardFPC)	//指纹数量
			{
				int TmpLen;
				score = BIOKEY_VERIFY(fhdl, Temp, LastTemplate);
				if (gOptions.VThreshold <= score)
				{
					memset(&pur, 0, sizeof(TUser));
					//					printf("tmppin2:%s\n",tmppin2);
					if (FDB_GetUserByCharPIN2(tmppin2, &pur))
					{
						result = pur.PIN;
						bMFCardInfo = 0;
					}
					else
					{
						bMFCardInfo = (gOptions.MustEnroll) ? 0 : 1;
					}
					break;
				}
				TmpLen = BIOKEY_TEMPLATELEN(Temp);
				if (TmpLen > 0)
					Temp += TmpLen;
				else
					break;
			}
			BIOKEY_MATCHINGPARAM1(fhdl, IDENTIFYSPEED, gOptions.MThreshold); //restore identification speed
		}
		else if (PIN && !retryflag)  //1:1 fingerprint - PIN
		{
			result = GetUserPIN_16(PIN);
			if (result)
			{
				TZKFPTemplate tmp;
				//use low speed matching for verification
				BIOKEY_MATCHINGPARAM1(fhdl, SPEED_LOW, gOptions.VThreshold);
				for (i = 0;i < gOptions.MaxUserFingerCount;i++)
				{
					if (FDB_GetTmp((U16)result, (char)i, (char *)&tmp))
					{
						score = BIOKEY_VERIFY(fhdl, (BYTE *)tmp.Template, LastTemplate);
						if (gOptions.VThreshold <= score)
							break;
					}
				}
				//restore identification speed
				BIOKEY_MATCHINGPARAM1(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
				if (i >= gOptions.MaxUserFingerCount)
				{
					if (!admflag)
						WaitVerifyDelayTime = TIMEOUT_INPUT_PIN;
					else
						WaitVerifyDelayTime = 0;

					result = -1;
				}
				else
					FingerID = i;
			}
			else
				result = -2;
		}
		else //1:1 1:G 1:H user
		{
			//	printf("1:1 1:G 1:H \n");
			//if (gOptions.PIN2Width > PIN_WIDTH)
			if (gOptions.I1ToH || g1ToG)
			{
				strtou32(InputPin, (U32*)&gInputPin);
			}
			else
			{
				if (FDB_GetUserByCharPIN2(InputPin, &ttuser))
					gInputPin = ttuser.PIN;
			}
			//printf("gInputPin %d\n",gInputPin);
			if (gInputPin > 0)
			{
				if (gInputPin)
				{
					if (gOptions.I1ToH || g1ToG)
					{
						int fcount = 0;
						if (g1ToG)
						{
							GetFilterGroupInfo(gInputPin, gFilterBuf);
							fcount = BIOKEY_DB_FILTERID(fhdl, Filter_Group_Run);
						}
						else
						{
							//9位以内号码才支持1:H
							if (gOptions.PIN2Width <= 9)
							{
								GetFilterHeadInfo(gInputPin, gFilterBuf);
								fcount = BIOKEY_DB_FILTERID(fhdl, Filter_Head_Run);
							}
						}
						if (fcount >= 1)
						{
							score = fcount < 10 ? gOptions.VThreshold : gOptions.MThreshold;
							//use low speed matching for verification
							BIOKEY_MATCHINGPARAM1(fhdl, IDENTIFYSPEED, score);
							if (!BIOKEY_IDENTIFYTEMP(fhdl, LastTemplate, &result, &score))
								result = -1;
							else
							{
								//高字节是FingerID;
								FingerID = (result >> 16) & 0xf;
								result &= 0xFFFF;
							}
							//restore identification speed
							BIOKEY_MATCHINGPARAM1(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
						}
						else
							result = -2;
					}
					else
					{
						result = gInputPin;
						if (result)
						{
							//use low speed matching for verification
							TZKFPTemplate tmp;
							BIOKEY_MATCHINGPARAM1(fhdl, SPEED_LOW, gOptions.VThreshold);
							for (i = 0;i < gOptions.MaxUserFingerCount;i++)
							{
								if (FDB_GetTmp((U16)result, (char)i, (char *)&tmp))
								{
									score = BIOKEY_VERIFY(fhdl, (BYTE *)tmp.Template, LastTemplate);
									if (gOptions.VThreshold <= score)
										break;
								}
							}
							//restore identification speed
							BIOKEY_MATCHINGPARAM1(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
							if (i >= gOptions.MaxUserFingerCount)
							{
								if (!admflag)
									WaitVerifyDelayTime = TIMEOUT_INPUT_PIN;
								else
									WaitVerifyDelayTime = 0;
								result = -1;
							}
							else
								FingerID = i;
						}
						else
							result = -2;
					}
				}
				else
					result = -3;
			}
			else
				result = -3;
		}
		if (result > 0 && gOptions.LockFunOn)
		{
			if (DelayTriggerDuress || FDB_IsDuressTmp(result, FingerID) ||
					((!InputPin[0] && !Temp[0]) && gOptions.Duress1ToN) ||	//1:N
					((InputPin[0] || Temp[0]) && gOptions.Duress1To1))	//1:1
			{
				DelayTriggerDuress = 10;
			}
		}
#if 0
		if (gFPDirectProc == -2)
			gFPDirectProc = 0;
		if (gFPDirectProc == -3 && result > 0)
			gFPDirectProc = 0;
		else if (gFPDirectProc > 0)
			gFPDirectProc--;
#endif
	}
	else //else if(LastTempLen>0)
	{
		result = LOWQLT;
		*Buffer = 0;
		//if (gFPDirectProc == 0) CheckSessionSend(EF_FPFTR, Buffer, 1);
		CheckSessionSend(EF_FPFTR, Buffer, 1);
		return result;
	}

	memcpy(Buffer, &result, 4);
	Buffer[4] = 1;
	CheckSessionSend(EF_VERIFY, Buffer, 5);

	if (gOptions.RS232Fun == 2)//简单输出ID号
	{
		if (result <= 0)
		{
			if (PIN)
				sprintf((char*)Buffer, "-F%d\r\n", PIN);
			else if (InputPin)
				sprintf((char*)Buffer, "-F%s\r\n", InputPin);
			else
				sprintf((char*)Buffer, "-F\r\n");
			SerialOutputString(&ff232, (char*)Buffer);
		}
	}
	LastUserID = result;
	//printf(" --%s-- result %d\n", __FUNCTION__, result);
	return result;
}

#if 1
char *FormatPIN(U16 PIN)
{
	static char buffer[MAX_CHAR_WIDTH];
	char format[20];//="%0.5d";
	U32 uid;

	PUser pu = FDB_GetUser(PIN, NULL);
	if (pu && pu->PIN2)
	{
		if (strncmp(buffer, pu->PIN2, 24) != 0)
		{
			uid = atoi(pu->PIN2);
			sprintf(format, "%%0.%dd", gOptions.PIN2Width);
			sprintf(buffer, format, uid);
		}
	}
	return buffer;
}

extern void SerialOutputData(serial_driver_t *serial, char *s, int  _size_);
void OutputPrinterFmt1(int pin)
{
	char buf1[30];
	char strStatus[2];

	sprintf(buf1, "%s %3d ", FormatPIN(pin), gOptions.DeviceID);
	FormatDate(buf1 + (gOptions.PIN2Width + 5), gOptions.DateFormat, gCurTime.tm_year + 1900, gCurTime.tm_mon + 1, gCurTime.tm_mday);
	sprintf(strStatus, FormatStatus(printstate), sizeof(strStatus));
	if (gOptions.SwitchAttStateByTimeZone)
	{
		int res = printstate;
		switch (res)
		{
			case 2:
				{
					res = 4;
					break;
				}
			case 3:
				{
					res = 5;
					break;
				}
			case 4:
				{
					res = 2;
					break;
				}
			case 5:
				{
					res = 3;
					break;
				}
		}
		memset(strStatus, 0, sizeof(strStatus));
		sprintf(strStatus, FormatStatus(res), sizeof(strStatus));
	}
	sprintf(buf1 + (gOptions.PIN2Width + 13), " %02d:%02d:%02d %s\r\n", gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec, strStatus);
	SerialOutputString(&ff232, buf1);
	//        printf("%s",buf1);
}

void OutputPrinterFmt2(int pin)
{
	char buf1[30] = {0x1B, 0x40, 0x1B, 0x61, 0x01, 0x1C, 0x70, 0x01, 0x00, 0x1B, 0x61, 0x00, 0x1B, 0x21, 0x8};
	char buf2[30] = {0x1B, 0x21, 0x00};
	char buf3[10] = {0x0A, 0x0B, 0x0A, 0x0B, 0x1B, 0x21, 0x08};
	char buf4[10] = {0x1B, 0x21, 0x00, 0x0A, 0x0B};
	char buf5[20] = {0x0A, 0x0B, 0x0A, 0x0B, 0x0A, 0x0B, 0x0A, 0x0B, 0x0A, 0x0B, 0x0A, 0x0B, 0x0A, 0x0B, 0x1B, 0x69};

	char buf6[30] = {0};    //"Enroll Number"
	char buf7[30] = {0};    //Enroll number
	char buf8[30] = {0};    //"Date time    CheckIn"
	char buf9[60] = {0};    //Date time

	int res = printstate;
	if (gOptions.SwitchAttStateByTimeZone)
	{
		switch (res)
		{
			case 4:
				{
					res = 2;
					break;
				}
			case 5:
				{
					res = 3;
					break;
				}
			case 2:
				{
					res = 4;
					break;
				}
			case 3:
				{
					res = 5;
					break;
				}
		}
	}
	sprintf(buf6, "%s%s", LoadStrByID(MID_USER_ID), ":  ");
	sprintf(buf7, "%s", FormatPIN(pin));

	/*dsl 2012.5.5, fix bug from FAE*/
	PShortKey pk = FDB_GetShortKeyByState(printstate, NULL);
	if (pk && pk->keyID)
	{
		if (pk->stateName[0])
			sprintf(buf8, "%s      %s", LoadStrByID(HIT_DATE1), pk->stateName);
		else
			sprintf(buf8, "%s      %d", LoadStrByID(HIT_DATE1), pk->stateCode);
	}

#if 0
	if (gOptions.IMEFunOn)
	{
		PShortKey pk = FDB_GetShortKeyByState(printstate, NULL);
		if (pk && pk->keyID)
		{
			if (pk->stateName[0])
				sprintf(buf8, "%s      %s", LoadStrByID(HIT_DATE1), pk->stateName);
			else
				sprintf(buf8, "%s      %d", LoadStrByID(HIT_DATE1), pk->stateCode);
		}
	}
	else
	{
		if (res >= 0 && res <= 5)
			sprintf(buf8, "%s      %s", LoadStrByID(HIT_DATE1), LoadStrByID(HIT_SYSTEM5KEY1 + res));
		else
			sprintf(buf8, "%s      %s", LoadStrByID(HIT_DATE1), "");
	}
#endif

	FormatDate(buf9, gOptions.DateFormat, gCurTime.tm_year + 1900, gCurTime.tm_mon + 1, gCurTime.tm_mday);
	sprintf(buf9 + 8, "        %02d:%02d:%02d", gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);

	SerialOutputData(&ff232, buf1, 15);
	SerialOutputString(&ff232, buf6);
	SerialOutputData(&ff232, buf2, 3);
	SerialOutputString(&ff232, buf7);
	SerialOutputData(&ff232, buf3, 7);
	SerialOutputString(&ff232, buf8);
	SerialOutputData(&ff232, buf4, 5);
	SerialOutputString(&ff232, buf9);
	SerialOutputData(&ff232, buf5, 16);
#if 0
	printf(buf1);
	printf("%s", buf6);
	printf(buf2);
	printf("%s", buf7);
	printf(buf3);
	printf("%s", buf8);
	printf(buf4);
	printf("%s", buf9);
	printf(buf5);
#endif
}

void OutputPrinterFmt3(int pin)
{
	char printbuf[50];
	char Datebuf[10];
	char Namebuf[40];
	char Timebuf[10];

	PUser pu = FDB_GetUser(pin, NULL);
	FormatDate(Datebuf, gOptions.DateFormat, gCurTime.tm_year + 1900, gCurTime.tm_mon + 1, gCurTime.tm_mday);    //日期
	sprintf(Timebuf, "%02d:%02d:%02d", gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);                 //时间
	sprintf(Namebuf, "%s", pu->Name[0] ? pu->Name : FormatPIN(pin));                                        //姓名或工号
	sprintf(printbuf, "%-3d %s %s %s %s\r\n", gOptions.DeviceID, FormatPIN(pin), Namebuf, Datebuf, Timebuf);
	SerialOutputData(&ff232, (char*)printbuf, strlen(printbuf));
	// printf("PrintOutput: %s",printbuf);
}

void OutputPrinterFmt4(int pin)
{
	char statbuf[20] = {0};
	char buf1[2] = {0x1b, 64};
	char buf2[30] = "  Control Asistencia\n\r";
	char buf3[30] = "  ==================\n\r\n\r";
	char buf4[30] = {0};
	char buf5[40] = {0};
	char buf6[30] = {0};
	char buf7[30] = {0};
	char buf8[30] = {0};
	char buf9[10] = {0};
	char Timebuf[10] = {0};
	char Datebuf[12] = {0};

	if (gOptions.IMEFunOn)
	{
		PShortKey pk = FDB_GetShortKeyByState(printstate, NULL);
		if (pk && pk->keyID)
		{
			if (pk->stateName[0])
				sprintf(statbuf, "%s", pk->stateName);
			else
				sprintf(statbuf, "%d", pk->stateCode);
		}
	}
	else
	{
		switch (printstate)
		{
			case 0:
				strcpy(statbuf, "Entrada");
				break;
			case 1:
				strcpy(statbuf, "Salida");
				break;
			case 5:
				strcpy(statbuf, "Colacion IN");
				break;
			case 4:
				strcpy(statbuf, "Colacion OUT");
				break;
		}
	}

	sprintf(Timebuf, "%02d:%02d", gCurTime.tm_hour, gCurTime.tm_min);
	sprintf(Datebuf, "%02d/%02d/%04d", gCurTime.tm_mday, gCurTime.tm_mon + 1, gCurTime.tm_year + 1900);

	strcpy(buf4, statbuf);
	strcat(buf4, "\n\r");
	sprintf(buf5, "%s.....%s\n\r", Timebuf, Datebuf);
	sprintf(buf6, "Codigo.....%s\n\r", FormatPIN(pin));
	sprintf(buf7, "===== Gracias =====\n\r");
	sprintf(buf8, "\n\r\n\r\n\r\n\r\n\r");
	sprintf(buf9, "\x1b\n\r");

	SerialOutputData(&ff232, buf1, 2);
	SerialOutputString(&ff232, buf2);
	SerialOutputString(&ff232, buf3);
	SerialOutputString(&ff232, buf4);
	SerialOutputString(&ff232, buf5);
	SerialOutputString(&ff232, buf6);
	SerialOutputString(&ff232, buf7);
	SerialOutputString(&ff232, buf8);
	SerialOutputString(&ff232, buf9);
#if 0
	printf(buf1);
	printf("%s", buf2);
	printf("%s", buf3);
	printf("%s", buf4);
	printf("%s", buf5);
	printf("%s", buf6);
	printf("%s", buf7);
	printf("%s", buf8);
	printf("%s", buf9);
#endif
}
//punto seguro
void OutputPrinterFmt5(int pin)
{
	char buf[56] = {0};
	char buf1[4] = {0};

	buf1[0] = '\n';
	buf1[1] = '\r';
	buf1[2] = '\n';
	SerialOutputData(&ff232, buf1, 3);

	sprintf((char *)buf, "  %s   ", FormatPIN(pin));
	FormatDate(buf + (gOptions.PIN2Width + 5), gOptions.DateFormat, gCurTime.tm_year + 1900, gCurTime.tm_mon + 1, gCurTime.tm_mday);

	if (gOptions.IMEFunOn)
	{
		PShortKey pk = FDB_GetShortKeyByState(printstate, NULL);
		if (pk && pk->keyID)
		{
			if (pk->stateName[0])
				sprintf(buf + (gOptions.PIN2Width + 5 + 8), "   %02d:%02d   %s\r\n\n      ", gCurTime.tm_hour, gCurTime.tm_min, pk->stateName);
			else
				sprintf(buf + (gOptions.PIN2Width + 5 + 8), "   %02d:%02d   %d\r\n\n      ", gCurTime.tm_hour, gCurTime.tm_min, pk->stateCode);
		}
	}
	else
	{
		int res = printstate;
		if (res >= 0 && res <= 5)
			sprintf(buf + 18, "   %02d:%02d   %s\r\n\n      ", gCurTime.tm_hour, gCurTime.tm_min, LoadStrByID(HIT_SYSTEM5KEY1 + res));
		else
			sprintf(buf + 18, "   %02d:%02d   %s\r\n\n      ", gCurTime.tm_hour, gCurTime.tm_min, "");
	}
	SerialOutputData(&ff232, buf, sizeof(buf));

	buf1[0] = 0x1d;
	buf1[1] = 0x56;
	buf1[2] = 0x30;	//切纸命令
	SerialOutputString(&ff232, buf1);

	buf1[0] = '\n';
	buf1[1] = '\r';
	buf1[2] = '\n';
	SerialOutputData(&ff232, buf1, 1);
}

void OutputPrinterFmt6(int pin)
{
	//        char buf1[30] = {0x1B,0x40,0x1B,0x61,0x01,0x1C,0x70,0x01,0x00,0x1B,0x61,0x00,0x1B,0x21,0x8};
	char buf2[30] = {0x1B, 0x21, 0x00};
	//      char buf3[10] = {0x0A,0x0B,0x0A,0x0B,0x1B,0x21,0x08};
	char buf4[10] = {0x1B, 0x21, 0x00, 0x0A, 0x0B};
	//    char buf5[20] = {0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x1B,0x69};
	char buf6[30] = {0};			//"Enroll Number"
	char buf7[30] = {0};			//Enroll number
	char buf8[30] = {0};			//"Date time    CheckIn"
	char buf9[60] = {0};			//Date time
	char buf10[4] = {0x1d, 0x56, 0x30, 0};	//切纸命令

	sprintf(buf6, "%s", "ID USR:  ");
	sprintf(buf7, "%s", FormatPIN(pin));

	if (gOptions.IMEFunOn)
	{
		PShortKey pk = FDB_GetShortKeyByState(printstate, NULL);
		if (pk && pk->keyID)
		{
			if (pk->stateName[0])
				sprintf(buf8, "%s      %s", LoadStrByID(HIT_DATE1), pk->stateName);
			else
				sprintf(buf8, "%s      %d", LoadStrByID(HIT_DATE1), pk->stateCode);
		}
	}
	else
	{
		int res = printstate;
		if (res >= 0 && res <= 5)
			sprintf(buf8, "%s      %s", LoadStrByID(HIT_DATE1), LoadStrByID(HIT_SYSTEM5KEY1 + res));
		else
			sprintf(buf8, "%s      %s", LoadStrByID(HIT_DATE1), "");
	}

	FormatDate(buf9, gOptions.DateFormat, gCurTime.tm_year + 1900, gCurTime.tm_mon + 1, gCurTime.tm_mday);
	sprintf(buf9 + 8, "        %02d:%02d:%02d", gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);

	SerialOutputData(&ff232, " \n\r", 3);
	SerialOutputData(&ff232, buf6, 9);
	SerialOutputData(&ff232, buf2, 3);
	SerialOutputString(&ff232, buf7);
	SerialOutputData(&ff232, "\n\r\n\r", 4);
	SerialOutputString(&ff232, buf8);
	SerialOutputData(&ff232, buf4, 5);
	SerialOutputString(&ff232, buf9);
	SerialOutputString(&ff232, "\n\r\n\r");
	SerialOutputString(&ff232, buf10);
	SerialOutputString(&ff232, " \n\r");
}
//silicontech
void OutputPrinterFmt7(int pin)
{
	char buf1[30] = {0};
	char Datebuf[12];
	char Timebuf[10];

	sprintf(buf1, "\n");
	SerialOutputString(&ff232, buf1);
	sprintf(buf1, " Usr.ID:%s\n", FormatPIN(pin));
	SerialOutputString(&ff232, buf1);

	PUser pu = FDB_GetUser(pin, NULL);
	if (pu && pu->Name[0])
		sprintf(buf1, " Nombre:%s\n", pu->Name);//"Enrique");
	else
		sprintf(buf1, " Nombre:%s\n", "");
	SerialOutputString(&ff232, buf1);

	if (gOptions.IMEFunOn)
	{
		PShortKey pk = FDB_GetShortKeyByState(printstate, NULL);
		if (pk && pk->keyID)
		{
			if (pk->stateName[0])
				sprintf(buf1, " Fecha/Hora:%s\n", pk->stateName);
			else
				sprintf(buf1, " Fecha/Hora:%d\n", pk->stateCode);
		}
	}
	else
	{
		int res = printstate;
		if (res >= 0)
			sprintf(buf1, " Fecha/Hora:%s\n", LoadStrByID((int)(HIT_SYSTEM5KEY1 + res)));
		else
			sprintf(buf1, " Fecha/Hora:%s\n", " ");
	}
	SerialOutputString(&ff232, buf1);

	FormatDate(Datebuf, gOptions.DateFormat, gCurTime.tm_year + 1900, gCurTime.tm_mon + 1, gCurTime.tm_mday);	//日期
	//	sprintf(buf2, "%02d-%02d-%02d", gCurTime.tm_mday, gCurTime.tm_mon+1, gCurTime.tm_year%100);
	sprintf(Timebuf, "%02d:%02d:%02d", gCurTime.tm_hour, gCurTime.tm_min, gCurTime.tm_sec);			//时间
	sprintf(buf1, " %s %s\n", Datebuf, Timebuf);
	SerialOutputString(&ff232, buf1);
}

#endif
extern void RefreshStarBar(HWND hWnd);

static int check_anti_passback(int CurPos, int LogState, int VerifiedMethod, int pin)
{
	if (gOptions.AntiPassbackFunOn < 1 || gOptions.AntiPassbackOn < 1 
			|| (LogState != LOG_VALID) || CurPos < 0) {
		return LogState;
	}
	//printf("______%s, %d,gOptions.MasterState = %d\n", __FILE__, __LINE__, gOptions.MasterState);
	/*add by zxz 2012-11-01*/
	if(gOptions.MasterState < 0) {
		return LogState;
	}
	//printf("ANTIPASSBACK_OUT(%d), ANTIPASSBACK_IN(%d)\n", ANTIPASSBACK_OUT, ANTIPASSBACK_IN);
	//printf("CurPos=%d, last state=%d, pin=%d, AntiPassbackOn=%d\n", CurPos, CurAlarmRec[CurPos].State, pin, gOptions.AntiPassbackOn);
	switch (gOptions.AntiPassbackOn)
	{
		/*Anti pass back rules...*/

		/*The master device state of door is Out.
		 *You can In or Out the door with everyone when first time. 
		 *If you want to enter the door, you must have a record of Out state, else anti pass back.
		 *you can Out the door in anytimes
		 */
		case ANTIPASSBACK_IN:		//入反潜.
			if (CurAlarmRec[CurPos].LastTime &&
					(CurAlarmRec[CurPos].State == STATE_IN && (getCurrentState(VerifiedMethod) == STATE_IN))){
				LogState = LOG_ANTIPASSBACK;
			}
			break;

		/*The master device state of door is In.
		 *You can In or Out the door with every one when first times.
		 *If you want to Out the door, you must have a record of In state, else anti pass back.
		 *you can In the door in anytimes*/
		case ANTIPASSBACK_OUT:		//出反潜
			if (CurAlarmRec[CurPos].LastTime &&
					(CurAlarmRec[CurPos].State == STATE_OUT && (getCurrentState(VerifiedMethod) == STATE_OUT))){
				LogState = LOG_ANTIPASSBACK;
			}
			break;
		case ANTIPASSBACK_INOUT:	//出入反潜
			if (CurAlarmRec[CurPos].LastTime){
				if ((CurAlarmRec[CurPos].State == STATE_IN && (getCurrentState(VerifiedMethod) == STATE_IN)) ||
						(CurAlarmRec[CurPos].State == STATE_OUT && (getCurrentState(VerifiedMethod) == STATE_OUT))){
					LogState = LOG_ANTIPASSBACK;
				}
			}
			break;
	}

	//printf("AntiPassback LogState:%d\n",LogState);
	if (LogState == LOG_ANTIPASSBACK)
	{
		VerifyLogState |= LOG_ANTIPASSBACK;		//保存验证结果（liming）
		U32 d[3];
		d[0] = 0xFFFFFFFF;
		d[1] = OP_ANTIPASSBACK | (pin << 16);
		d[2] = TRUE;
		CheckSessionSend(EF_ALARM, (void*)d, 16);
		FDB_AddOPLog(pin, OP_ANTIPASSBACK, TRUE, 0, 0, 0);
		ExAlarm(0, 24*60*60);
	}
	return LogState;
}

static int try_to_open_door(int pin, TTime t, int VerifyMethod)
{
	int ret=0;
	if (gOptions.MustChoiceInOut && (gOptions.AttState < 0)){
		return ret;
	}
	
	DelayNextUser = 0;
	if (gLockForceAction == FORCENONE){
		int c = TestOpenLock(pin, t, VerifyMethod);
		if (c > 0){
			groupVerify = 0;			//清除组合验证标志
			if (gOptions.LockOn){
				DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
			}
		} else 	if (c < 0) {		//合法组合成员
			groupVerify = 1;
			ShowMainWindowDelay = 0;
			DelayNextUser = DELAYNEXTUSER;
		} else {		//非法组合成员
			ret = LOG_INVALIDCOMBO;
			VerifyLogState |= LOG_INVALIDCOMBO;
			ClearMemberRecord();            //清除组合开锁记录
		}
	}
	return ret;
}

static void send_wiegand_sign(PUser u)
{
	U32 pin2=0;
	if ((u==NULL) || (gOptions.ExtWGInFunOn==0)) {
		return;
	}

	if (gOptions.IsWGSRBFunOn && gOptions.SRBOn)
	{
		WiegandSRBOutput(gOptions.SRBType, (unsigned char)gOptions.LockOn*25);//change 2013-01-23 40ms
		return;
	}

	if (strlen(u->PIN2) > 10) {
		return;
	}

	if (gOptions.WiegandOutType == 0) {	//输出工号
		pin2 = atoi(u->PIN2);
		WiegandSend((gWGSiteCode < 0) ? 0 : gWGSiteCode, pin2, 0);
	} else {					//输出卡号
		memcpy(&pin2, (void*)(u->Card), 4);
		if (pin2 > 0) {
			WiegandSend((gWGSiteCode < 0) ? 0 : gWGSiteCode, pin2, 0);
		} else {
		}
	}
}

static void send_userid_to_serial(PUser u)
{
	char buf[50];
	char format[32];
	if (gOptions.RS232Fun != 2 || u==NULL) {
		return;
	}

	memset(buf, 0, sizeof(buf));
	memset(format, 0, sizeof(format));

	sprintf(format, "+%%0%dd\r\n", gOptions.PIN2Width);
	if (gOptions.PIN2Width > PIN_WIDTH){
		strcpy(buf, u->PIN2);
	} else {
		sprintf((char*)buf, format, u->PIN);
	}
	SerialOutputString(&ff232, (char*)buf);
}

static void send_realevent(int VerifiedMethod, int LogState, int workcode, PUser u)
{
	char buf[40];

	if (u==NULL) {
		return;
	}

	memset(buf, 0, sizeof(buf));
	memcpy(buf, u->PIN2, 24);
	buf[24] = VerifiedMethod;
	buf[25] = gOptions.AttState | (char)((LogState <= LOG_REPEAT) ? LOG_VALID : (1 << 7));
	buf[26] = gCurTime.tm_year - 100;
	buf[27] = gCurTime.tm_mon + 1;
	buf[28] = gCurTime.tm_mday;
	buf[29] = gCurTime.tm_hour;
	buf[30] = gCurTime.tm_min;
	buf[31] = gCurTime.tm_sec;
	memcpy(buf + 32, &workcode, 4);
	CheckSessionSend(EF_ATTLOG, buf, 36);
}

static int SaveAttLog(U16 pin, int VerifiedMethod, HDC hdc, HWND hWnd)
{
	char buf[50];
	int LogState = LOG_VALID;
	PUser u = NULL;
	time_t t;
	int c = 0;
	int workcode = 0;
	int CurPos;
	char CurAttState;
	U32 pin2;
	int group = 0;
	int SaveResult=0;

	if (issavecapture) {//保存拍照保存时间与记录时间一致
		issavecapture = 0;
		t = capturetime;
	} else {
		GetTime(&gCurTime);
		t = OldEncodeTime(&gCurTime);
	}

	for (CurPos = 0; CurPos < gOptions.MaxUserCount*100; CurPos++) {
		if (CurAlarmRec[CurPos].PIN == pin) {
			CurPos = GetUserLastLog(CurAlarmRec, pin, LastLogsCount);
			//if (gOptions.AlarmReRec && (((int)(t - CurAlarmRec[CurPos].LastTime)) < (gOptions.AlarmReRec*60)))
			if (gOptions.AlarmReRec && (((int)(t - CurAlarmRec[CurPos].LastTime)) < (gOptions.AlarmReRec)))
			{
				LogState |= LOG_REPEAT;
				VerifyLogState |= LOG_REPEAT;
			}
			break;
		} else if (CurAlarmRec[CurPos].PIN == 0) {
			break;
		}
	}

	if ((u=FDB_GetUser(pin, NULL)) != NULL) {
		group = u->Group & 0x0F;
	}

	if (!gOptions.MustEnroll || u)
	{
		if (gOptions.MustEnroll && u) {
			if (ISINVALIDUSER(*u) || (gOptions.DisableAdminUser && ISADMIN(u->Privilege)) 
				|| (gOptions.DisableNormalUser && !ISADMIN(u->Privilege))) {
				LogState |= LOG_INVALIDUSER;
				VerifyLogState |= LOG_INVALIDUSER;
			}
		}

		if (!(LogState&LOG_INVALIDUSER)) {
			//检查时间段
			if ((gOptions.LockFunOn&LOCKFUN_ADV) || (gOptions.AttUseTZ)) {
				if (!TestUserTZ(pin, gCurTime)) {
					LogState |= LOG_INVALIDTIME;
					VerifyLogState |= LOG_INVALIDTIME;
					//时间段外继绝打卡
					if (gOptions.AttUseTZ) {
						ledbool = FALSE;
						led_redlight_on(gOptions.IsFlashLed);
						ExBeep(1);
						return 0;
					}
				}
			}

			//检查反潜
			LogState = check_anti_passback(CurPos, LogState, VerifiedMethod, pin);

			/*门禁功能<liming>*/
			if (gOptions.LockFunOn&8 && gLockForceAction == FORCECLOSE){
				LogState = LOG_INVALIDTIME;
				VerifyLogState |= LOG_INVALIDTIME;
			}

			if ((LogState <= LOG_REPEAT) && gOptions.LockFunOn){
				if ((c=try_to_open_door(pin, gCurTime, VerifiedMethod)) > 0) {
					LogState |= c;
				}
			}
		}

		if (LogState > LOG_REPEAT){
			ledbool = FALSE;
			led_redlight_on(gOptions.IsFlashLed);
			ExBeep(1);

		} else if (gOptions.MustChoiceInOut && (gOptions.AttState < 0)){
			bnoworkstate = 1;
			ExBeep(1);
			return 0;
		}

		if (!(LogState&LOG_INVALIDUSER))
		{
			if (exceedflag == 1){       //记录溢出
				return 0;
			}
			pin2 = (u==NULL ? 0:atoi(u->PIN2));
			if (gOptions.SaveAttLog && (!LogState&LOG_REPEAT))
			{
				if (gOptions.AntiPassbackFunOn && gOptions.AntiPassbackOn)
					CurAttState = (char)getCurrentState(VerifiedMethod);	//进出状态值0为进，1为出
				else if ((LogState == LOG_VALID) && !(gOptions.MustChoiceInOut && (gOptions.AttState < 0)))
					CurAttState = (char)gOptions.AttState;
				else
					CurAttState = (char) - 1;

				if (gOptions.PrinterFunOn){
					printstate = gOptions.AttState;
				}

				//Get current workcode
				workcode = GetCurWorkCode();

				printf("VerifiedMethod===%d\n", VerifiedMethod);
				if ((u->PIN2[0] && strlen(u->PIN2) > 0) &&
					((SaveResult=FDB_AddAttLog((U16)pin, t, (char)VerifiedMethod, 
						CurAttState, u->PIN2, workcode, gSensorNo)) == FDB_OK))	{
					//liming
					TAttLog log;
					log.status = CurAttState;
					log.time_second = t;
					log.PIN = pin;
					log.verified = (char)VerifiedMethod;
					if (CurPos < gOptions.MaxUserCount*100)	{
						if (AddToOrderedLastLogs(CurAlarmRec, &log, LastLogsCount))
							LastLogsCount++;
					}
					VerifiedPIN = pin;		//记录验证成功的用户PIN
					//clear current workcode.
					curWorkCode = 0;
					memset(ownwkcd, 0, sizeof(ownwkcd));
				}
			}

			/*dsl 2012.4.25*/
			if (SaveResult == FDB_OK) {
				//修改后的Wiegand发送方式(liming)
				send_wiegand_sign(u);

				//简单输出ID号
				send_userid_to_serial(u);
				send_realevent(VerifiedMethod, LogState, workcode, u);
			}

			LastTime = t;
			LastUID = pin;
		}
		
	} else {
		ExBeep(1);
		ledbool = FALSE;
		led_redlight_on(gOptions.IsFlashLed);
		if (gOptions.RS232Fun == 2) {//简单输出ID号
			sprintf((char*)buf, "-%d\r\n", pin);
			SerialOutputString(&ff232, (char*)buf);
		}
	}

	if (gOptions.MustChoiceInOut) {
		RefreshStarBar(hMainWindowWnd);
	}

	return SaveResult;
}

int CheckUserPIN(char *pin, PUser user)
{
	U32 p = 0;
	TUser u;
	memset(&u, 0, sizeof(TUser));
	if (gOptions.PIN2Width == PIN_WIDTH)
	{
		if (strtou32(pin, &p) == 0 && FDB_GetUser(p, &u))
		{
			p = u.PIN;
			if (user != NULL)
			{
				memset(user, 0, sizeof(TUser));
				memcpy(user, &u, sizeof(TUser));
			}
		}
	}
	else
	{
		if (FDB_GetUserByCharPIN2(pin, &u) != NULL)
		{
			p = u.PIN;
			if (user != NULL)
			{
				memset(user, 0, sizeof(TUser));
				memcpy(user, &u, sizeof(TUser));
			}
		}
	}
	if (p)
		return p;
	else
		return -1;
}

BYTE GetExtUserVSByPIN(U32 pin, int msgtype)
{
	U16 p=0;
	PExtUser extuser = NULL;
	BYTE vs;
	TGroup tgp;
	TUser tu;

	memset((void*)&tu, 0, sizeof(TUser));
	if (FDB_GetUser(pin, &tu)) {
		p = tu.PIN;
	}

	if (gOptions.UserExtendFormat && pin) {
		extuser = FDB_GetExtUser(pin, NULL);
		/*use own verify style*/
		if (extuser && (extuser->VerifyStyle&0x80)) {
			//printf("use own verify style:%d\n",extuser->VerifyStyle&0x7F);
			return extuser->VerifyStyle&0x7F;
		}

		/*use Group Verify Type*/
		memset(&tgp, 0, sizeof(TGroup));
		if (FDB_GetGroup(tu.Group, &tgp)){
			return tgp.VerifyStyle&0x7F;
		}
	}

	//default value
	vs = VS_FP_OR_PW_OR_RF;
	if (!gOptions.OnlyPINCard && (((msgtype == News_VerifiedByMFCard) && !gOptions.MifareAsIDCard) 
			||((msgtype == News_VerifiedByiCLASSCard) && !gOptions.iCLASSAsIDCard)) ) {
		vs = VS_FP_AND_RF;
		return vs;
	}

	//如果不使用exten模式的，默认原来 2006.08.24
	switch (msgtype)
	{
		case News_VerifiedByPwd:
			vs = VS_PW;
			break;
		case News_VerifiedByFP:
		case News_VerifiedByFPRIS:
			vs = VS_FP;
			break;
		case News_VerifiedByIDCard:
		case News_VerifiedByIDCardRIS:
		case News_VerifiedByMFCard:
		case News_VerifiedByiCLASSCard:
			vs = VS_RF;
			break;
	}
	return vs;
}

BOOL CheckNextVerifyType(HWND hWnd, TVSStatus VSStatus, TVSStatus CurVSStatus, BOOL IsANDOperator)
{
	BOOL result = FALSE;
	TUser uu;

	memset(&uu, 0, sizeof(TUser));
	//PIN CARD PWD FP
	//	printf("verify temp value:%s\n",KeyBuffer);
	if (IsANDOperator || !((CurVSStatus.PIN && VSStatus.PIN) || (CurVSStatus.Card && VSStatus.Card) ||
				(CurVSStatus.Password && VSStatus.Password) || (CurVSStatus.FP && VSStatus.FP)))
	{
		//		printf("verify temp value:%s\n",KeyBuffer);
		if (CurVSStatus.PIN && !VSStatus.PIN)
		{
			if (!retryflag)
			{
				WaitVerifyRetryCnt = gOptions.FPRetry;
			}

			curVerifyStyle = VFS_PIN;
			curVerifyState = VF_READY;
			ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
		}
		else if (CurVSStatus.Password && !VSStatus.Password &&
				(IsANDOperator || (FDB_GetUser(GetUserPIN_16(FPData.PIN), NULL)->Password[0])))
		{
			if (!retryflag)
			{
				WaitVerifyRetryCnt = gOptions.PwdRetry;
			}

			memset(&pwduser, 0, sizeof(TUser));
			if (FPData.PIN && FDB_GetUser(FPData.PIN, &pwduser))
			{
				if (pwduser.Password[0])
				{
					curVerifyStyle = VFS_PWD;
					curVerifyState = VF_READY;
					ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
				}
				else
				{
					//Liaozz 20080926 fix bug second 20
					curVerifyStyle = VFS_PWD;
					//Liaozz end
					WaitVerifyRetryCnt = 1;
					SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPIN, 0);
				}
			}
		}
		else if (CurVSStatus.FP && !VSStatus.FP && !gOptions.IsOnlyRFMachine)
		{
			if (!retryflag)
				WaitVerifyRetryCnt = gOptions.FPRetry;

			if (KeyBuffer[0])
			{
				curVerifyStyle = VFS_FP;
				curVerifyState = VF_READY;
				ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
				EnableMsgType(MSG_TYPE_FINGER, 1);
				WaitProcessFingerFlag = 1;
			}
			//ExBeep(1);
		}
		else if (CurVSStatus.Card && !VSStatus.Card && !gOptions.IsOnlyRFMachine)
		{
			if (!retryflag)
				WaitVerifyRetryCnt = gOptions.FPRetry;

			curVerifyStyle = VFS_CARD;
			curVerifyState = VF_READY;
			ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
			EnableMsgType(MSG_TYPE_FINGER, 0);
			EnableMsgType(MSG_TYPE_HID, 1);
			EnableMsgType(MSG_TYPE_MF, 1);
			EnableMsgType(MSG_TYPE_ICLASS, 1);
		}
		else
		{
			if (CurVSStatus.Password && !VSStatus.Password)
			{
				memset(&pwduser, 0, sizeof(TUser));
				if (FPData.PIN && FDB_GetUser(FPData.PIN, &pwduser))
				{
					if (pwduser.Password[0])
					{
						curVerifyStyle = VFS_PWD;
						curVerifyState = VF_READY;
						ShowVerifyState(hWnd, curVerifyStyle, curVerifyState, IsANDOperator);
					}
					else
					{
						WaitVerifyRetryCnt = 1;
						SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPIN, 0);
					}
				}
			}
#if 0
			{
				//LCDWriteLineStrID(0, HID_VERINPUTPWD);
				WaitVerifyRetryCnt = gOptions.PwdRetry;//COUNT_RETRY_USER;
				SendMessage(hWnd, MSG_TYPE_CMD, News_FailedByPwd, 0);
			}
#endif
			else
			{
				result = TRUE;
			}
		}
	}
	else //else if(IsANDOperator....)
	{
		result = TRUE;
	}
	return result;
}

void DisplayUserInfo(HWND hWnd, PFPResult risuser)
{
	if (gOptions.LockFunOn && gOptions.LockOn) {
		DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
	}

	memset((void*)&vfUser, 0, sizeof(TUser));
	sprintf(vfUser.PIN2, "%u", risuser->PIN);
	snprintf(vfUser.Name, sizeof(vfUser.Name), "%s", risuser->Name);
	//printf("____%s%d vfuser.pin2 = %s , vfUser.Name = %s\n", __FILE__, __LINE__, vfUser.PIN2, vfUser.Name);
	ShowVerifyState(hWnd, VFS_FP, VF_SUCCESS, 0);
	memset(KeyBuffer, 0, 24);
	AuthFPData.PIN = 0;
	ShowMainWindowDelay = 5;
}

int VerifyWindow(HWND hOwner, int vfstyle, int adm, PMsg pmsg)
{
	HWND hMainWnd;
	MSG msg;
	MAINWINCREATE CreateInfo;

	curVerifyStyle = vfstyle;
	curVerifyState = VF_READY;
	admflag = adm;
	WaitAdmVerifyRetryCnt = 3;

	hMainWindow = GetMainWindowHandle(hOwner);
	CreateInfo.dwStyle = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = VerifyWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom - 30;
	CreateInfo.iBkColor = COLOR_lightwhite;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hMainWindow;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID) {
		return -1;
	}

	/*dsl 2012.4.17*/
	memset((void*)&gAdminUser, 0, sizeof(TUser));
	AdminUser = &gAdminUser;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	if (pmsg) {
		SendMessage(hMainWnd, pmsg->Message, pmsg->Param1, pmsg->Param2);
	}
	while (GetMessage(&msg, hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hMainWnd);
	
	/*dsl 2012.4.17*/
	AdminUser = NULL;

	return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

