#ifndef __PUSHAPI_H__
#define __PUSHAPI_H__
#include <stdio.h>
#include <sys/stat.h>
#include <minigui/common.h>
#undef _USE_MINIGUIENTRY
#include <minigui/minigui.h>
#undef ROP_XOR	//add by zxz 2012-09-20
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "pushsdk_lib_api.h"
#include <flashdb.h>



#ifndef  BOOL 
#define BOOL int 
#endif

#ifndef  U32
#define U32 unsigned int 
#endif

#ifndef  U16 
#define U16 unsigned short 
#endif

#ifndef  BYTE 
#define BYTE unsigned char
#endif

#ifndef  U8
#define U8 unsigned char 
#endif

#define ERR_OK		0
#define ERR_STATE	8
#define ERR_PARAM	1
#define ERR_ENROLLED	2
#define ERR_SAVE	3
#define ERR_FAIL	4
#define ERR_REPEAT	5
#define ERR_CANCEL	6
#define ERR_NOTENROLLED	7

extern int GPRS_Started;
extern int gAlarmDelay, gErrTimes;
extern int newiclockState;
extern int bsmscount;

/*extern fw api*/
#ifdef RADIO
extern int  restart_gsm_stack();
extern int stop_pppd();
#endif
extern int base64_encode(const unsigned char *in,  unsigned long len,unsigned char *out);
extern int base64_decode(const unsigned char *in, unsigned char *out);
extern int hex_decode(unsigned char *in, unsigned char *out);
extern int IsUserDataNeedUpload(TUser *user);
extern int IsUserDataNeedDownload(char *pin, TUser *user);
extern char *fmtPinLog(TAttLog *log);
extern char *fmtPin(TUser *user);
int FDB_CntUserByMap(void);
void register_push_fun(void);
int extractValueSample(char* s, char *key, char sp, char *value, int maxlen);
int CheckUserPIN(char *pin, PUser user);

#define SERVER_AUTH_MODE_OLD		1	//后台验证模式，OLD SDK方式验证
#define SERVER_AUTH_MODE_PUSH	2	//后台验证模式，PUSH SDK方式验证

#define  AUTH_TYPE_PWD				0	//用户校验方式(密码)
#define AUTH_TYPE_CARD 			1	//用户校验方式(刷卡)
#define  AUTH_TYPE_FP				2	//用户校验方式(指纹)
#define  AUTH_TYPE_FACE				3	//用户校验方式(人脸)
#define HTTP_MAXBUF_LEN 22*1024
/*fow fw use api*/
void DelRmAttUser(void);
void  AppendRTLogToFile(int EventFlag, unsigned char *Data, int Len);
void adjust_file_offset(int fd,int file_offset);
BOOL init_push();
int SaveUserDLStamp(char *pin2);
void iClock_Invoke(void);
int pushsdk_is_running(void);
void push_child_end(pid_t pid);
void pushsdk_data_new(int ContentType);
void pushsdk_data_reset(int ContentType);
void Draw_Iclock_Status(HDC hdc, int status);
/*for push sdk lib used api*/
int ServerAuthUpdate(FILE *pfileRes,const char *pszHttpInfoHeader, void *pUserInfo);
int FDB_ForAllData(int ContentType, ForARecFun fun, int MaxRecCount, void *param);
int FDB_ForDataFromCur(int ContentType, ForARecFun fun, int MaxRecCount, void *param);
int  mainEnrollCmd(const char *cmd);
void ReLoadOptions(void);
void reset_alarm();
int  GetPushSDKVersion(char * version, int buflen);
int main_update_data(int type,int action,void * data,void *addinfo,unsigned int info_len);
int  PushSDKVersion(char * version, int buflen);
int get_all_net_link_status(void);

#define OP_ENROLL_FACE 			65
#define OP_CHG_FACE 			66
#define FACE_CACHE_NUM  		5
#define PUSHSDK_MAX_BUFSIZ 	8092
#define PUSHSDK_MIN_BUFSIZ 	128
typedef struct _user_fp_tmp_{
	U16 Size;                      
	U16 PIN;                        
	BYTE FingerID;                
	BYTE Valid;                    
	BYTE Template[4*1024]; 
}GCC_PACKED user_fp_tmp_t;
typedef struct _pushsdk_fp_tmp_{
	char pin2[MAXPIN2LEN];
	user_fp_tmp_t fp_tmp;
}GCC_PACKED pushsdk_fp_tmp_t;


#ifdef SUPPORT_FACE
#include "facedb.h"
typedef  struct _pushsdk_face_tmp_ {
	char pin2[MAXPIN2LEN];
	TFaceTmp facetmp;
}pushsdk_face_tmp_t;
#endif

typedef struct _fp_index_ {
	int fp_count;
	int *fp_index;
}fp_index_t;

void show_iclock_status_icon(HWND hWnd, int LCDWidth);
//考勤名单下发data中head部分	22字节
typedef struct DownloadUserHead{
	U8	DevNum[11];				//机具编号
	U8	OperationType;			//操作标示		0x00:机具名单异常，需擦除名单，从版本0开始请求  0x01:机具名单正常
	short int UserNum;			//名单个数
	int UserRemain;				//剩余个数
	int ListVer;				//名单版本
}GCC_PACKED TUserHead, *PUserHead;


//名单		58字节
typedef struct DownloadUserData{
	U8	Card[4];				//物理卡号
	U8	CardSerial[10];			//卡应用序列号
	char Name[20];				//姓名,
	char PIN2[16];				//工号(学号)
	U8	CardStatus;				//卡状态标志
	char Password[6];			//密码
	U8	Privilege;				//人员标识(权限)
}GCC_PACKED TUserData,*PUserData;

typedef struct belltime{
	U8	time[2];
}GCC_PACKED TBell;

/**********************************
考勤机具参数下
***************************************/
typedef struct update_param{
	U8	DevNum[11];
	U8	VFInterval;
	TBell	BellTime[16];
	U8	CardMode;
	unsigned int	ParamVer;
}GCC_PACKED TUpParam, *PUpParam;

#endif


