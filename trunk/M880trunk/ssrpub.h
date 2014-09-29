#ifndef _SSRPUB_H_
#define _SSRPUB_H_

#include <minigui/gdi.h>

#include "flashdb.h"
#include "exfun.h"
#include "msg.h"

int CreateFpEnrollWindow(HWND hOwner,char *tmp, int *len,int fingerid,char *pin2,int isnewreg);
int CreateRisFpEnrollWindow(HWND hOwner,int pin2,int isnewreg);
int CreatePwdWindow(HWND hOwner,char *pwd, int isnewreg);
int CreateUserBrowseWindow(HWND hOwner);
int CreateAddUserWindow(HWND hOwner,int opmode, int *pin);
int CreateNewUserWindow(HWND hOwner);
int CreateDelUserWindow(HWND hOwner,char *pin2, int pin);
int CreateFindUserWindow(HWND hOwner,char *acno);
int CreateFindLogWindow(HWND hOwner);
int CreateFindLogSetWindow(HWND hOwner,char *acno,time_t *st1,time_t *ed1);
int CreateShowUserWindow(HWND hOwner,int pin); 
void ShowLogSetWindow(HWND hOwner);
int CreateAttLogWindow(HWND hOwner,char *acno, time_t st1, time_t ed1, int realquery);
int CreateOneLogWindow(HWND hOwner,int pin,time_t st1,time_t ed1);
void SSR_DATA_UPLOAD(HWND hWnd);
void SSR_DATA_DOWNLOAD(HWND hWnd);

//add by liming
int CreateArlarmManageWindow(HWND hOwner);			//闹铃管理
int CreateAlarmSetingWindow(HWND hWnd, int *selPIN);		//闹铃设置
int CreateBellEditWindow(HWND hWnd, int* bellID);
int CreateSmsManageWindow(HWND hOwner);				//短消息管理
int CreateSmsOptWindow(HWND hWnd, int optmod, int *SMSPIN);	//短消息编辑
int CreateSmsSendWindow(HWND hWnd, int optmod, int *SMSPIN, int *bchanged, PUDataLink pudlk);	//发送短消息
int CreateSMSBoard(HWND hWnd, int smscount);							//显示公共短消息
int CreateWCodeManageWindow(HWND hWnd, int optmod, int *wkcdPIN); 				//工作代码管理
int CreateWorkCodeManageWindow(HWND hOwner);							//工作代码管理
int CreateShortKeyWindow(HWND hOwner);								//快捷键管理
int CreateShortKeyMngWindow(HWND hWnd, int keyID);						//快捷键编辑

//liming 2007.08
int CreateLockWindow(HWND hWnd, int *userid);
int DuressFpWindow(HWND hWnd,int usrPIN);
int LockSettingWindow(HWND hWnd);
int TimeZoneMngWindow(HWND hWnd, int tzid, int flag);
int CreateGroupOptWindow(HWND hWnd, int optmod, int *gpid);
int HolidayMngWindow(HWND hWnd);
int CreateHolidayOptWindow(HWND hWnd, int optmod, int *gpid);
int CGroupMngWindow(HWND hWnd);
int GroupMngWindow(HWND hWnd);
int CreateCGroupOptWindow(HWND hWnd, int optmod , int *cgpid);
int DoorParameterWindow(HWND hWnd);
int DuressParameterWindow(HWND hWnd);
int CardMngWindow(HWND hWnd);
int CreateRFCardWindow(HWND hOwner,char *tmpstr, int *value,int Isnewreg);	//普通ID卡登记
int CreateMFCardWindow(HWND hOwner, int opMode);
int CreateMFManageWindow(HWND hOwner);
int OTSetWindow(HWND hWnd);

int VerifyWindow(HWND hOwner, int vfstyle, int adm, PMsg pmsg);
int CreateWiFiWindow(HWND hWnd, char* selssid);
int CreateWiFiIPWindow(HWND hWnd);
int CreateWiFiPWDWindow(HWND hWnd, int ptype);
int CreateWiFiInfoWindow(HWND hWnd);
/*dsl 2012.4.19*/
int CreateNewWiFiWindow(HWND hWnd, char* selssid);


int CreateModemWindow(HWND hWnd);
int CreateModemInfoWindow(HWND hWnd);

int CreateServerWindow(HWND hWnd);

int CreateWiegandWindow(HWND hWnd);
int CreateWiegandInWindow(HWND hWnd);
int CreateWiegandOutWindow(HWND hWnd, int InOutMode);

int CreateSystem6Window(HWND hWnd);
int SSR_MENU_SYSTEM1(HWND hWnd);

int CreateRecordWindow(HWND hWnd);
int ViewPhotoWindow(HWND hWnd, char* userPIN, time_t sttime, time_t edtime, int mode);
int CreateFindPhotoSetWindow(HWND hWnd, int mode, char* userPIN2);
int CreateVideoAdjustWindow(HWND hWnd, int* v_brightness, int* v_contrast, int* v_quality, int* v_scene, int flag);
int CreateSystemOtherWindow(HWND hWnd);

typedef struct _wifiPwd{
	unsigned char pwdtype[2];
	unsigned char pwdchar[30];
}WifiPWD;


int CaptureDownLoadWindow(HWND hWnd);


int GetMonDay (int year, int month);

char *FormatStatus(BYTE status);
char *FormatVerified(BYTE verified);
int ShowTime(HDC hdc);
int processscrollview(HWND listwnd, int down,int incseed);
char * GetPicPath(char *name);
int IsLeapYear (int year);
int GetMonDayformyear (int year, int month);
//夏令时处理函数
int IsDaylightSavingTimeMenu(void);
int CreateMainStateWindow(HWND hOwner,int key);
int SSR_MENU_RESTORE(HWND hWnd);

//cache ten finger template
char usertmp[10][2048];
//cache ten finger template length
int usertmplen[10];
//whether finger[userisenroll] Enroll finger, total 10 finger
int userisenroll[10];
int isfpdbload;

int ismenutimeout;	//菜单超时标志

/***********************for mul language add by jazzy 2008.07.24**********/
PLOGFONT gfont;
PLOGFONT gfont1;
PLOGFONT lvfont;
int Str2UTF8(int lid,unsigned char *str, unsigned char *utf8);

/*dsl 2012.5.5*/
PBITMAP get_submenubg_jgp(void);
#endif

