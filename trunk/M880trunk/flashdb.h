/*************************************************
                                           
 ZEM 200                                          
                                                    
 flashdb.h define all functions for database mangement of flash                             
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _FLASHDB_H_
#define _FLASHDB_H_

#include <time.h>
#include "arca.h"
#include "ccc.h"
#include "zkfp.h"

#define SSRDB

#define GRIDVIEWROWCNT	8	//gridview show rows count add by zxz 2012-10-19
#define DIRECTINON_FLAG_FORWARD 0
#define DIRECTINON_FLAG_BACKWARD 1

#define WG_IN_MODE	0
#define WG_OUT_MODE	1

#define MAX_USER_FINGER_COUNT 10	/* Max Finger Count for a User */
#define FCT_EMPTY 	(U8)0xFF
#define FCT_ATTLOG 	(U8)1
#define FCT_FINGERTMP 	(U8)2
/*用户照片主动上传，add by yangxiaolong,2011-6-24,start*/
#define FCT_USERPIC	(U8)3
/*用户照片主动上传，add by yangxiaolong,2011-6-24,end*/
#define FCT_OPLOG 	(U8)4
#define FCT_USER 	(U8)5
#define FCT_OldUSER 	(U8)25
#define FCT_SMS 	(U8)6
#define FCT_UDATA 	(U8)7
#define FCT_SYSOPTIONS 	(U8)0x0a
#define FCT_SYSTEM	(U8)0x0b
#define FCT_SYSTEM_NONE (U8)0xF0
#define FCT_WEBPAGES 	(U8)0x0c

#define FCT_ALL	-1
#define FCT_EXTUSER	(U8) 8
#define FCT_DEPT    	(U8)9
#define FCT_SCH     	(U8)10
#define FCT_ATTRULE 	(U8)15
#define FCT_ALARM	(U8)16
#define FCT_BELL	(U8)30
#define FCT_WORKCODE	(U8)17
#define FCT_SHORTKEY	(U8)18
/*异地考勤,add by yangxiaolong,2011-06-14,start*/
//异地考勤时，保存用户下载时间
#define FCT_USER_DL_TIME	(U8)24
/*异地考勤,add by yangxiaolong,2011-06-14,start*/

//htm
#define FCT_LOGHTM  	(U8)11
#define FCT_ABNORHTM	(U8)12
#define FCT_CALCUHTM	(U8)13
#define FCT_ABNORHTM2	(U8)14
#define FCT_SERIALCARDNO	(U8)31

#define PIN_WIDTH 	0
#define PIN_WIDTH2 	5
#define MAX_PIN 	((U16)0xFFFE)
#define MAX_PIN2 	((U32)0x7FFFFFFE)

#define FDB_OK 			0		/* Success */
#define FDB_ERROR_NOTINIT	-1  		/* Database Not initialized */
#define FDB_ERROR_OP 		-2		/* Wrong Operation */
#define FDB_ERROR_IO 		-3		/* Flash I/O Error */
#define FDB_ERROR_NODATA 	-4		/* No (matched) Data */
#define FDB_ERROR_NOSPACE 	-5		/* No more space */
#define FDB_ERROR_DATA		-6		/* Data is not correct */

#define FDB_OVER_FLIMIT 	1
#define FDB_OVER_UFLIMIT 	2
#define FDB_OVER_ULIMIT		3
#define FDB_FID_EXISTS 		4

#define STAT_COUNT              0
#define STAT_VALIDLEN           1
#define STAT_CNTADMINUSER       2        
#define STAT_CNTADMIN           3
#define STAT_CNTPWDUSER         4
#define STAT_CNTTEMPLATE        5
//ssr deptusercount
#define STAT_CNTDEPTUSER  6
//#define STAT_CNTTEMPCOUNT	7

//database initialization, return Fingertemplate count
int FDB_InitDBs(BOOL OpenSign);
void FDB_FreeDBs(void);
int FDB_GetSizes(char* Sizes);
//Clear all data
int FDB_ClearData(int ContentType);

//考勤流水数据					25字节
typedef struct AttData{
	U8	CardSerial[10];			//卡应用序列号
	U8	Card[4];			//卡物理序列号
	U8	AttTime[7];				//打卡时间
	int AttSerialNum;			//打卡序号
}GCC_PACKED TAttData,*PAttData;


//data structure for file data searching 
typedef struct _SearchHandle{
        int ContentType;
        char *buffer;
        int bufferlen; 
        int datalen;
        int fd; 
}TSearchHandle, *PSearchHandle;

int SelectFDFromConentType(int ContentType);
void SearchFirst(PSearchHandle sh);
BOOL SearchNext(PSearchHandle sh);
void FDB_dataLast(int ContentType);

#define PRI_VALIDUSER 	0
#define PRI_INVALIDUSER 1
#define PRI_VALIDBIT	1
#define PRI_ENROLL	2
#define PRI_OPTIONS 	4
#define PRI_SUPERVISOR 	8
#define ISADMIN(p)	(((p) & ~PRI_VALIDBIT)!=0)

#define PRIVILLEGE0 PRI_VALIDUSER
#define PRIVILLEGE1 (PRI_ENROLL)
#define PRIVILLEGE2 (PRI_ENROLL|PRI_OPTIONS)
#define PRIVILLEGE3 (PRI_ENROLL|PRI_OPTIONS|PRI_SUPERVISOR)

#define ISINVALIDUSER(user) (((user).Privilege & PRI_VALIDBIT)!=PRI_VALIDUSER)
#define ISADMINUSER(user) ISADMIN((user).Privilege)

#define MAXTEMPLATESIZE 602
#define MAXVALIDTMPSIZE 598 //for 4 bytes aligned

//zsliu mv form flashdb2.h
//typedef int (*ForARecFun)(void *rec, int count, void *param);
extern char PRIVALUES[];

#define MAXNAMELENGTH  24
typedef struct _OldUser_{
        U16 PIN;
        U8 Privilege;
        char Password[5];
        char Name[8];
        U8 Card[5];             //卡号码，用于存储对应的ID卡的号码
        U8 Group;               //用户所属的分组
        U16 TimeZones;          //用户可用的时间段，位标志
        U32 PIN2;               //32位宽的用户第二识别号
}GCC_PACKED TOldUser, *POldUser;

#define MAXPIN2LEN 24
//72 byte
typedef struct _User_{
	U16 PIN;		//用户ID
	U8 Privilege;		//用户权限
	char Password[8];	//密码
	char Name[24];		//姓名
	U8 Card[4];		//卡号码，用于存储对应的ID卡的号码
	U8 Group;		//用户所属的分组
	U16 TimeZones[4];	//用户可用的时间段(TimeZones[0]:0使用组时间段，1使用自定义时间段)
	char PIN2[MAXPIN2LEN];		//用户工号
}GCC_PACKED TUser, *PUser;

//user lb
typedef struct _Userlb{
	TUser userlb;
	struct _Userlb *next;	
}GCC_PACKED TUserlb,*PUserlb;
U32 CurAttLogCount;

PUser FDB_CreateUser(PUser user, U16 pin, char *name, char *passwd, int privillege);
int FDB_AddUser(PUser user);
int FDB_ChgUser(PUser user);
int FDB_ChgUserByPIN(U16 PIN, U8 Privilege);
int FDB_DelUser(U16 PIN);
PUser FDB_GetUser(U16 PIN, PUser user);
PUser FDB_GetUserByPIN2(U32 pin2, PUser user);
PUser FDB_GetUserByCharPIN2(char *pin2, PUser user);
PUser FDB_GetUserByCard(BYTE *card, PUser user);
PUser FDB_GetUserByNameAndAcno(char *name, char *pin2,PUser user);
PUser FDB_GetUserByName(char *name,PUser user);

int FDB_ClrUser(void);
int FDB_ClrAdmin(void);	//this function to clear all user's privillege
int FDB_CntAdmin(int Privillege);
int FDB_CntUser(void);
int FDB_CntAdminUser(void);
int FDB_CntPwdUser(void);
//ssr 2006.06
PUserlb  SortUserlb(PUserlb head);
PUserlb GetUsermestolb(int deptid);
PUserlb GetOneUsertolb(int pin);
//指纹
typedef struct _Template_{
        U16 Size;                       //指纹大小
        U16 PIN;                        //用户ID
        BYTE FingerID;                  //指纹编号
        BYTE Valid;                     //标志
        BYTE Template[MAXTEMPLATESIZE]; //maximize template length
}GCC_PACKED TTemplate, *PTemplate;

//指纹
typedef struct _ZKFPTemplate_{
	U16 Size;                       //....
	U16 PIN;                        //..ID
	BYTE FingerID;                  //....
	BYTE Valid;                     //..
	BYTE Template[ZKFPV10_MAX_FP_LEN]; //maximize template length
}GCC_PACKED TZKFPTemplate, *PZKFPTemplate;

#define DURESSFINGERTAG 2
#define ISDURESSFP(tmp) (0!=((tmp)->Valid & DURESSFINGERTAG))

int FDB_PackData(int ContentType);

typedef struct _OldAttLog_{
	U16 PIN;
	BYTE verified;
	time_t time_second;
	BYTE status;
}TOldAttLog, *POldAttLog;

//40byte
typedef struct _AttLog_{
        U16 PIN;		
        U8 PIN2[24];
        BYTE verified;		//验证类型
        time_t time_second;
        BYTE status;		//工作状态
        U32 workcode;		//工作代码
        BYTE reserved[4];
}GCC_PACKED TAttLog,*PAttLog;

typedef struct _AlarmRec_{
	  U8 State;
        U16 PIN;
        U32 LastTime;
}TAlarmRec, *PAlarmRec;

typedef struct _ExtendAttLog_{
	U32 PIN;
	time_t time_second;
	BYTE status;
	BYTE verified;
	BYTE reserved[2];
	U32 workcode;
}GCC_PACKED TExtendAttLog, *PExtendAttLog;

int FDB_AddAttLog(U16 PIN, time_t t, char verified, char status, char *pin2, U32 workcode, U8 SensorNo);
int FDB_ClrAttLog(void);
int FDB_CntAttLog(void);
void UpdateAttLog(int logtype);
void UpdateUserDb(void);
//二分法查找指定用户是否存在
int SearchInLastLogs(PAlarmRec lastlogs, U16 PIN, int count,int *found);
//添加记录到Lastlogs，如果已存在该用户记录且当前时间大于已存在时间则更新
int AddToOrderedLastLogs(PAlarmRec lastlogs, PAttLog log, int count);
int SearchInLogs(PAttLog logs, time_t t, int count);
int FDB_GetAttLog(U16 pin, time_t StartTime, time_t EndTime,PAttLog logs, int MaxCount);

int FDB_GetSectorLayout(char *p);
char* FDB_ReadAttLogBlock(int *size);
char* FDB_ReadOPLogBlock(int *size);
char* FDB_ReadUserBlock(int *size);

typedef struct _OPLog_{
	U16 Admin;		//2
	BYTE OP;		//2
	time_t time_second;	//4
	U16 Users[4];		//2*4
}TOPLog, *POPLog;

#define OP_POWER_ON    	0    //开机
#define OP_POWER_OFF 	1    //关机
#define OP_ALARM_VERIFY 2    //验证失败
#define OP_ALARM 	3    //报警
#define OP_MENU 	4    //进入菜单
#define OP_CHG_OPTION 	5    //更改设置, Users[3]定义为具体的修改项目
#define OP_ENROLL_FP   	6    //登记指纹
#define OP_ENROLL_PWD  	7    //登记密码
#define OP_ENROLL_RFCARD 	8//登记HID卡
#define OP_DEL_USER   	9    //删除用户
#define OP_DEL_FP      	10   //删除指纹
#define OP_DEL_PWD     	11   //删除密码
#define OP_DEL_RFCARD  	12   //删除射频卡
#define OP_CLEAR_DATA  	13   //清除数据
#define OP_MF_CREATE   	14	//创建MF卡
#define OP_MF_ENROLL   	15	//登记MF卡
#define OP_MF_REG      	16	//注册MF卡
#define OP_MF_UNREG    	17	//删除MF卡注册
#define OP_MF_CLEAR    	18	//清除MF卡内容
#define OP_MF_MOVE     	19	//把登记数据移到卡中
#define OP_MF_DUMP     	20	//把卡中的数据复制到机器中
#define OP_SET_TIME    	21   //设置时间
#define OP_RES_OPTION  	22   //恢复出厂设置
#define OP_CLEAR_LOG   	23   //删除进出记录
#define OP_CLEAR_ADMIN 	24   //清除管理员权限}
#define OP_ACC_GRP     	25   //修改门禁组设置
#define OP_ACC_USER    	26   //修改用户门禁设置
#define OP_ACC_TZ      	27   //修改门禁时间段
#define OP_ACC         	28   //修改开锁组合设置
#define OP_UNLOCK      	29   //开锁
#define OP_ENROLL_USER 	30   //登记新用户
#define OP_CHG_FP       31   //更改指纹属性
#define OP_DURESS       32   //胁迫报警
#define OP_ANTIPASSBACK 34
#define OP_DEL_ATTPHOTO 35      //删除考勤照片
#define OP_MODIFY_USER  36      //修改用户其他信息

/*用户照片主动上传，add by yangxiaolong,2011-6-24,start*/
#define	OP_ENROLL_USERPIC	68	//注册用户照片
#define	OP_CHG_USERPIC	69	//修改用户照片
/*用户照片主动上传，add by yangxiaolong,2011-6-24,end*/
#define OP_CHG_USER_NAME	70
#define OP_CHG_USER_PRIVILEGE	71	//修改用户权限
#define OP_SENSOR_REBOOT	72	//指纹头停止工作后，重启机器. dsl 2012.3.23

int FDB_AddOPLog(U16 Admin, BYTE OP, U16 Objs1, U16 Objs2, U16 Objs3, U16 Objts4);
int FDB_ClrOPLog(void);
int FDB_CntOPLog(void);

#define MAX_OPLOG_COUNT (4*64*1024/sizeof(TOPLog))

extern BYTE  *UserIDMap;

#define WG_TYPE_CNT	5	//modify by zxz 2012-11-30

int IsFreePIN(char *PIN);
int IsUsedPIN(char *PIN);
int IsFreePIN2(char *PIN);
int IsUsedPIN2(char *PIN);

U16 GetNextPIN(int From, int Free);
//update the userid map for test if a userid is used
int FDB_UpdateUserIDMap(void);
//search a pin in sorted pins buf(create by FDB_GetSortedPINs). 
//return 1 if found, else return 0. the ">=" position is stored in pos.
int FDB_SearchInPINs(char *buf, U16 pin, int start, int end, int *pos);

char* FDB_ReadBlock(int *size, int ContentType);

int FDB_GetFreeFingerID(U16 PIN, BYTE *FID);

int FDB_CheckUpdata(void);	//进行数据库升级检查

int FDB_CheckIntegrate(void);	//进行数据库正确性、完整性检查

BOOL FDB_Download(int ContentType, char *dstFileName);

//int AppendUserTemp(int pin, char *name, int fingerid, char *temp, int tmplen);
int AppendUserTemp(int pin, char *name, int fingerid, char *temp, int tmplen,int valid);
int AppendUser(int pin, char *name, char *password, int privillege);
int AppendFullUser(PUser user);

#define MAX_SMS_CONTENT_SIZE 	160 	//短消息长度
#define MAX_SMS_COUNT 		1024	//最大短消息条数
#define MAX_ALARM_HINT_SIZE	7	//用做闹铃提示，暂未使用	liming

//数据内容标识
#define UDATA_TAG_USERSMS 	0xFE    //表示用户短消息
#define UDATA_TAG_ALL  		0xFD 	 //表示整体通知短消息
#define UDATA_TAG_TEMP 		0xFF    //表示系统保留的临时项

#define STARTTIME_UNLIMITED 	0xFFFFFFFF
#define VALIDMINUTE_UNLIMITED 	0

typedef struct _SMS_
{
	BYTE Tag;		//类别
	U16 ID; 		//短消息ID，0表示记录已经无效
	U16 ValidMinutes; 	//有效分钟数   ０＝永久
	U16 Reserved;		//保留
	U32 StartTime; 		//起始时间
	BYTE Content[MAX_SMS_CONTENT_SIZE*2+1];   //短消息内容
}GCC_PACKED TSms, *PSms;    //512 Bytes

//user->sms
typedef struct _UData_{
	U16 PIN;        	//用户ID，0表示无效记录
	U16 SmsID;		//短消息ID
}GCC_PACKED TUData, *PUData;  //4Bytes

typedef struct LSMSIDX{
        U16 smsPIN;
        int ID;
        struct LSMSIDX *next;
}GCC_PACKED TSMSIDX,*pSmsIdx;

PSms FDB_CreateSms(PSms sms, BYTE tag, U16 id, BYTE *content, U16 validminutes, time_t start);
int FDB_AddSms(PSms sms);
int FDB_ChgSms(PSms sms);
int FDB_DelSms(U16 id);
PSms FDB_GetSms(U16 id, PSms sms);

int FDB_InitSmsPoint(void);
int FDB_ReadSms(PSms psms);
int FDB_InitUDataPoint(void);
int FDB_ReadUData(PUData pudata, int smsID);

//int FDB_ReadSms(PUser psms);
int FDB_ClrSms(void);
int FDB_CntSms(void);
void FDB_CheckSmsByStamp(U32 CurTime);
int FDB_PackSms(void);

//临时存放分发信息链表liming
typedef struct UDLNode{
	U16 PIN;
	int bSelect;
	struct UDLNode* nextnode;
}TUDataLink, *PUDataLink;

PUData FDB_CreateUData(PUData udata,U16 pin, U16 smsid);
int FDB_AddUData(PUData udata);
PUData FDB_GetUData(U16 id, PUData udata);
//int FDB_GetPINBySMSID(U16 smsid, U16 pin[], int optmod); //liming根据短消息ID查找用户PIN
int FDB_ChgUData(PUData udata);
int FDB_GetUDataBySmsID(U16 smsid,PUData udata);

int FDB_DelUData(U16 PIN, U16 smsID);
PUData FDB_GetUDataByPINSMSID(U16 pin, U16 id, PUData udata);

//BYTE *FDB_ReadUserSms(U16 *smsid, BYTE *content);
#define ICLOCK
#ifndef ICLOCK
int FDB_ReadBoardSms(BYTE *content);
#else
int FDB_ReadBoardSms(BYTE *content,int flag);
int FDB_GetBoardNum(void);
#endif
BOOL CheckBoardSMS(void);
void DisplayBoardSMS(void);
BOOL CheckUserSMS(U16 pin, BYTE *smsContent);

#define BS_TRANS_ATT_FILE		"/mnt/mtdblock/BsAtt.dat"


//alarm
typedef struct _ALARM
{
	U16 AlarmIDX;	// 2
	U8 AlarmHour;	// 1
	U8 AlarmMin;	// 1
	U8 AlarmWaveIDX;	//1
	U8 AlarmAudioVol;
	U8 AlarmStatus;
	U8 AlarmTimes;
//	BYTE AlarmHint[MAX_ALARM_HINT_SIZE+1];// 预留空间，可做闹铃提示
}GCC_PACKED ALARM, *PAlarm; // 8 bytes

int FDB_CntAlarm(void);
PAlarm FDB_CreateAlarm(PAlarm alarm, U16 id, U8 t_hour, U8 t_min, U8 fileindex, U8 audiovol, U8 status, U8 alarmtimes);//BYTE *hint);
int FDB_AddAlarm(PAlarm alarm);
PAlarm FDB_GetAlarm(U16 id, PAlarm alarm);
int FDB_ChgAlarm(PAlarm alarm);
//==================================New Bell Start
#define BellIsTurnOn(bell) ((((bell).SchInfo>>7)&1)==1)		// TBellNew bell
#define BellIsValid(bell,day) ((((bell).SchInfo>>day) & 1)==1)
typedef struct _Bell_New
{
	U16 BellID;			// Bell index
	U8  SchInfo;			// Bell schedule as status.	bit7: ON/OFF, bit0-bit6: work day of week
	U8  BellTime[2];		// Bell time (hh:mm)
	U8  BellWaveIndex;		// Bell wave file name
	U8  BellVolume;			// Bell volume 
	U8  BellTimes;			// Bell times
}GCC_PACKED TBellNew, *PBellNew;	// 8bytes
int FDB_CntBell(void);
int FDB_AddBell(PBellNew bell);
int FDB_ChgBell(PBellNew bell);
int FDB_DelBell(U16 id);
PBellNew FDB_CreateBell(PBellNew bell, U8 schinfo, U8 hour, U8 min, U8 wave, U8 volume, U8 status, U8 times);
PBellNew FDB_GetBell(U16 id, PBellNew bell);
PBellNew FDB_GetBellByTime(PBellNew bell, int hour, int min, int day);
int FDB_BellSettingErr(int id, int hour, int min, BYTE SchInfo);
//===================================New Bell End
//Short key   <liming>
#define STKEY_COUNT 15
//#define STATE_NAME_LEN 16 //modiby by jazzy 2008.11.25 for mul language
#define STATE_NAME_LEN 24 //zsliu change
typedef struct _ShortKey_{
	U8 keyID;
	U8 keyFun;
	U8 stateCode;
	char stateName[STATE_NAME_LEN+2]; //6个中文字
	U16 Time1;
	U16 Time2;
	U16 Time3;
	U16 Time4;
	U16 Time5;
	U16 Time6;
	U16 Time7;		// 14byte(星期一～星期日)
	U8 autochange;
}GCC_PACKED TSHORTKEY,*PShortKey; //32Byte.

PShortKey FDB_GetShortKeyByState(U8 code, PShortKey stkey);
PShortKey FDB_GetShortKey(U8 id,PShortKey stkey);
int FDB_CntShortKey(void);
PShortKey FDB_CreateShortKey(PShortKey stkey,U8 id,U8 fun,U8 sCode, char* sName,U16 t1,U16 t2,U16 t3,U16 t4,U16 t5,U16 t6,U16 t7,U8 bchg);
int FDB_AddShortKey(PShortKey stkey);
int FDB_ChgShortKey(PShortKey stkey);


//Work code
#define MAX_WORKCODE_LEN 8
#define MAX_WORKCODE_NAME_LEN 24
typedef struct _Workcode_{
	U16 PIN;				//索引值，为0时表示无效 2byte
	char Code[MAX_WORKCODE_LEN+1]; 		// 9byte
	char Name[MAX_WORKCODE_NAME_LEN+1];	// 24byte
}GCC_PACKED TWORKCODE,*PWorkCode; 

int FDB_InitWkcdPoint(void);
int FDB_ReadWorkCode(PWorkCode pworkcode);
int FDB_CntWorkCode(void);
PWorkCode FDB_CreateWorkCode(PWorkCode workcode,U16 pin,char *code,char *name);
int FDB_AddWorkCode(PWorkCode workcode);
PWorkCode FDB_GetWorkCode(U16 id, PWorkCode workcode);
int FDB_ChgWorkCode(PWorkCode workcode);
int FDB_DelWorkCode(U16 pin);

PWorkCode FDB_GetWorkCodeByCode(char* code, PWorkCode wkcd);
PWorkCode FDB_RecreateWorkCode(PWorkCode wkcd);

//Update user dara 
typedef struct _UserS_{
	U8 OpSign;
	TUser User;
}GCC_PACKED TUserS, *PUserS;

typedef struct _FingerS_{
	U8 OpSign;
	U16 PIN;
	U8 FingerID;
	U32 OffSet;
}GCC_PACKED TFingerS, *PFingerS;

void BatchOPUserData(BYTE *buffer);

typedef struct _FilterRec_{
	U16 PIN;
	U32 PIN2;
}TFilterRec, *PFilterRec;

void GetFilterGroupInfo(int inputpin, PFilterRec filterbuf);
void GetFilterHeadInfo(int inputpin, PFilterRec filterbuf);

//2006.06 ssr db define
//(dept.dat) 36 byte
typedef struct _DEPT
{       //部门id
        U8 Deptid;
        //保留
        U8 Reserve[3];
        //部门名称
        char DeptName[24];
        //4个班次  默认0
        U16 Schid[4];
}GCC_PACKED TDept,*Pdept;

int GetDeptmes(char *dept);
int GetDeptID(char *deptname);
int SaveDept(Pdept ppdept);
int GetDeptByid(int deptid,Pdept ppdept);
U32 FDB_Getcntdeptuser(U8 deptid);
int FDB_ChgDept(Pdept ppdept);
Pdept FDB_GetDept(U8 deptid,Pdept dept);

//Schclass 时间段信息表SchclassDB.dat 20byte
typedef struct _TimeClass
{       //时间段id
        U16 schClassid;
        //时间段名称
        char schName[10];
        //上班时间
        time_t  StartTime;
        //下班时间
        time_t  EndTime;
}GCC_PACKED TTimeClass,*PTimeClass;

typedef struct _SchClasslb
{
	TTimeClass  schclasslb;
	struct _SchClasslb *next;

}GCC_PACKED TSchClasslb,*PSchClasslb;

PSchClasslb GetSchmes(void);
int Generateschid(void);
int IsSchused(int contenttype,U16 schid);
PTimeClass FDB_GetSch(U16 schid,PTimeClass outschclass);
int FDB_SearchSch(PTimeClass inschclass);
int FDB_AddSch(PTimeClass addschclass);
int FDB_ChgSch(PTimeClass chgschclass);
int SaveSchclass(int act,PTimeClass outschclass);
int GetSchByid(int schid,PTimeClass schclass);
int UpdateUserschbydep(int deptid,int schindex,int schid);
int FDB_DelSch(U16 schid);
int IsValidUserSch(U16 pin);
//output report
int OutputAttLogRep(char *cpname);
int OutputAttLogRep2(char *cpname,PUser AdminUser);
int GetDayAttLog(U16 pin,time_t Sttm,time_t endtm,char *content);
int GetDayAttLog2(U16 pin,time_t Sttm,time_t endtm,char *content);
int OpenHtmFd(int fdtype,char *dstfilename);
int WriteHtml(int fdtype,char *htmcontent);
int CloseHtmFd(int fdtype);
extern char *gssrlogbuffer;
void SetUserInfoDbFirst();
int ReadUserInfoDB(PUser user);
int FDB_HaveAttLog(U16 pin, time_t StartTime, time_t EndTime);

int FDB_AttLogTruncAndSave(char *Data,unsigned long Size);
extern PUser AdminUser;
#define ADMINPIN ((AdminUser==NULL)?0:(AdminUser->PIN))

#define ISHACKER (HackerNumber==2)
//#define TESTPRIVILLEGE(priv) ((HackerNumber==1) || ISHACKER || (AdminUser==NULL) || ((AdminUser->Privilege & priv)!=0))
#define TESTPRIVILLEGE(priv) ((AdminUser!=NULL) && ((AdminUser->Privilege & priv)!=0))
#define TEST_ONLY_ENROLL_PRIVILLEGE (TESTPRIVILLEGE(PRI_ENROLL) && !TESTPRIVILLEGE(PRI_SUPERVISOR) && !TESTPRIVILLEGE(PRI_OPTIONS))

/*
0 - Read out all data now
-1 - Not read out any data
other - offset(in flash) of last read out
*/
#define LOG_READ_ALL 0
#define LOG_READ_NONE  -1
void FDB_SetAttLogReadAddr(U32 addr);
void FDB_SetOpLogReadAddr(U32 addr);
char* FDB_ReadNewAttLogBlock(int *size);
char* FDB_ReadNewOpLogBlock(int *size);

#define RTLOGDATASIZE   36
//#define RTLOGDATASIZE   14
typedef struct _RTLog_{
        U16 EventType;
        char Data[RTLOGDATASIZE];
} TRTLog, *PRTLog;


struct TRTLogNode;
typedef struct TRTLogNode *PRTLogNode;
struct TRTLogNode{
        TRTLog log;
        PRTLogNode Next;
};

//门禁设置相关数据结构<liming>
//14种验证方式
enum __VERIFY_STYLE
{
        VS_FP_OR_PW_OR_RF=0,
        VS_FP,
        VS_PIN,
        VS_PW,
        VS_RF,
        VS_FP_OR_PW,
        VS_FP_OR_RF,
        VS_PW_OR_RF,
        VS_PIN_AND_FP,
        VS_FP_AND_PW,
        VS_FP_AND_RF,
        VS_PW_AND_RF,
        VS_FP_AND_PW_AND_RF,
        VS_PIN_AND_FP_AND_PW,
        VS_FP_AND_RF_OR_PIN,
        VS_NUM=15
};

#define FCT_TZ		20
#define FCT_HTZ		21
#define FCT_GROUP	22
#define FCT_CGROUP	23

//时间段
#define TZ_MAX  50
#define TZ_START 0
#define TZ_END 1
#define TZ_HOUR 0
#define TZ_MINUTE 1

#define TZ_SUN 0
#define TZ_MON 1
#define TZ_TUE 2
#define TZ_WED 3
#define TZ_THU 4
#define TZ_FRI 5
#define TZ_STA 6
typedef struct _TZ_{
	U16 ID;			//时间段编号
        U8 ITime[7][2][2];	//时间段
	char Reserve[2];	//保留
}GCC_PACKED TTimeZone, *PTimeZone;		//32Byte
int FDB_AddTimeZone(PTimeZone timezone);
int FDB_ChgTimeZone(PTimeZone timezone);
int FDB_DelTimeZone(U16 id);
int FDB_CntTimeZone(void);
PTimeZone FDB_GetTimeZone(U16 id, PTimeZone timezone);

//节假日
#define HTZ_MAX 24		//最多24个节日
#define HTZ_START 0
#define HTZ_END 1
#define HTZ_MONTH 0
#define HTZ_DAY 1

typedef struct _HTZ_{
	U16 ID;			//节假日编号
	U8 HDate[2][2];		//起始日期
	U16 TZID;		//使用时间段编号
}GCC_PACKED THTimeZone, *PHTimeZone;	//8Byte

int FDB_CntHTimeZone(void);
int FDB_AddHTimeZone(PHTimeZone htimezone);
int FDB_ChgHTimeZone(PHTimeZone htimezone);
int FDB_DelHTimeZone(U16 id);
PHTimeZone FDB_GetHTimeZone(U16 id, PHTimeZone htimezone);

//用户验证方式
typedef struct _ExtUser_{
        U16 PIN;		//用户ID
        U8 VerifyStyle;		//用户验证方式
	U8 reserved[21];	//保留
}GCC_PACKED TExtUser, *PExtUser;		//24Byte

PExtUser FDB_CreateExtUser(PExtUser extuser, U16 pin, U8 verifystyle);
int FDB_AddExtUser(PExtUser extuser);
int FDB_ChgExtUser(PExtUser extuser);
int FDB_DelExtUser(U16 pin);
PExtUser FDB_GetExtUser(U16 pin, PExtUser extuser);

#if 0
//用户指纹信息
typedef struct _UserTempInfo_{
	U16 PIN;		
	U16	FingerCount;	//指纹数
}GCC_PACKED TUserTempInfo, *PUserTempInfo;
PUserTempInfo FDB_CreateUserTempInfo(PUserTempInfo usertemp, U16 pin, U16 fingercount);
int FDB_AddUserTempInfo(PUserTempInfo usertemp);
int FDB_ChgUserTempInfo(PUserTempInfo usertemp);
int FDB_DelUserTempInfo(U16 pin);
PUserTempInfo FDB_GetUserTempInfo(U16 pin, PUserTempInfo usertemp);
int FDB_CntUserTempInfo(void);
#endif

//组
#define GP_MAX	99		//最多允许99个组
typedef struct _GROUP_{
	U8 ID;			//组编号
	U16 TZID[3];		//默认时间段编号
	U8 VerifyStyle;		//组验证方式	bit7为节假日是否有效标志(0;无效，1：有效)
}GCC_PACKED TGroup, *PGroup;			//8Byte
#define HTZVALID(VerifyStyle) ((VerifyStyle>>7) & 1)		//节假日是否有效
#define SETHTZVALID(VerifyStyle) (VerifyStyle |= 0x80)		//设置节假日有效
#define CLRHTZVALID(VerifyStyle) (VerifyStyle &= 0x7F)		//设置节假日无效
 
int FDB_CntGroup(void);
int FDB_AddGroup(PGroup group);
int FDB_ChgGroup(PGroup group);
int FDB_DelGroup(U8 id);
PGroup FDB_GetGroup(U8 id, PGroup group);
int beUsedGroup(U8 id);
//开锁组合
#define CGP_MAX 10		//最多允许10种开锁组合
#define CGP_MEMBER_MAX 5	//开锁组合成员个数
typedef struct _CGROUP_{
	U8 ID;				//开锁组合编号
	U8 GroupID[CGP_MEMBER_MAX];	//组合成员编号
	U8 MemberCount;			//成员个数
	char Reserve[1];		//保留
}GCC_PACKED TCGroup, *PCGroup;			//8Byte
int FDB_CntCGroup(void);
int FDB_AddCGroup(PCGroup cgroup);
int FDB_ChgCGroup(PCGroup cgroup);
int FDB_DelCGroup(U8 id);
PCGroup FDB_GetCGroup(U8 id, PCGroup cgroup);


//PTemplate FDB_CreateTemplate(PTemplate tmp, U16 PIN, char FingerID, char *Template, int TmpLen);

PZKFPTemplate FDB_CreateTemplate(char* tmp, U16 pin, char FingerID, char *Template, int TmpLen,int Valid);
//int FDB_AddTmp(PTemplate tmp);
int FDB_AddTmp(char *tmp);
int FDB_DelTmp(U16 PIN, char FingerID);
int FDB_DeleteTmps(U16 PIN);
U32 FDB_GetTmp(U16 PIN, char FingerID, char* tmp);
//ssr
U32 FDB_GetTmpCount(U16 UID);
int FDB_ClrTmp(void);
int FDB_CntTmp(void);
int FDB_LoadTmp(void *Handle);
U32 FDB_GetTmpCnt(U16 UID);

int FDB_ChgTmpValidTag(PTemplate tmp, BYTE SetTag, BYTE ClearTag);
int FDB_ClrDuressTagTmpAll(void);
int FDB_IsDuressTmp(U16 PIN, char FingerID);

#define DURESSFINGERTAG 2
#define ISDURESSFP(tmp) (0!=((tmp)->Valid & DURESSFINGERTAG))
#define FDB_SetDuressTmp(TmpAddress) FDB_ChgTmpValidTag(TmpAddress, DURESSFINGERTAG, 0)
#define FDB_ClearDuressTmp(TmpAddress) FDB_ChgTmpValidTag(TmpAddress, 0, DURESSFINGERTAG)

//初始化闹铃，组，时间段，开锁组合，工作状态
void FDB_InitShortKey(void);
void FDB_InitAlarm(void);
void FDB_InitDefaultDoor(void);

//=================照片索引文件处理========================

#define FCT_PHOTOINDEX (U8) 19
#define FCT_PHOTOID (U8) 28
typedef struct PhotoIndexNode
{
	U32 index;				//索引序号(为0表示此记录无效,相应照片已被删除)		4byte
	time_t createtime;			//图片文件创建时间					4byte
	char userid[26];			//用户工号						26byte
	U8 pictype;				//图片类型(0为通过照片, 1为黑名单照片)			1byte
	char reserve[1];			//保留字节						1byte
}GCC_PACKED TPhotoIndex, *PhotoIndex;		//36Bytes

typedef struct _Photolb
{
        TPhotoIndex  photoindex;
        struct _Photolb *next;

}GCC_PACKED TPhotolb,*PPhotolb;

typedef struct PhotoIDIndex //存储实时上传成功抓拍照片的索引
{
        U32 index;//4byte
}GCC_PACKED TPhotoID, *PPhotoID;//4Bytes
typedef struct PhotoListNode
{
	U32 index;				//索引表中的序号
        char PhotoFile[40];			//文件名(不包括文件扩展名)
        struct PhotoListNode *pre;
        struct PhotoListNode *next;
}TPhotoList, *PPhotoList;

//根据时间/用户查找照片加入链表
U32 FDB_GetMaxPhotoIndex(void);
PPhotoList FDB_GetPhotoToList(time_t sttime, time_t edtime, char* userid, int bhour, int *pcount, int type);
PhotoIndex FDB_GetPhotoIndex(int index, PhotoIndex pidx);
int FDB_DelPhotoIndex(int index);
int FDB_AddPhotoIndex(PhotoIndex pidx);
int FDB_ClearIndex(int type);
//seiya dlphoto
PPhotolb FDB_GetPhototolb(U32 pictime);
PPhotoID FDB_GetPhotoID(int index, PPhotoID picid);
//seiya dlphoto end
//======================end==================================

void ResolvedUserDataErr(int min, int max);

//反潜
int GetUserLastLog(PAlarmRec lastlogs,U16 PIN,int LastLogsCount);
int FetchLastLogs(PAlarmRec CurAlarmLog);
//结束

int FDB_IsOKLog(TAttLog *log);

typedef struct codeconvert
{
	U16 code;
	U16 value;
}TCodeConvert, *PCodeConvert;

void addcode(U16 code,U16 value);
void UpdateDBforWeb(void);
void FDB_InitBell(void);
int GetFreePIN2FromRam(char *pin2);
int GetFreePIN2FromRamEx(char *pin2);


int ProcessGetFingerprintCount(int pin);
//BOOL ProcessDownload(char *dstFileName);
BOOL ProcessDownload(char *dstFileName,int flag);
U16* ProcessUploadUDiskUser(char *InUsersData,int InUsersDataLen, U16 *AddUserPIN, int *returnValue, int *AdminCount);
//adms
PPhotoID FDB_GetPhotoID(int index, PPhotoID picid);
int FDB_ClrPhoto(void);
int FDB_CntAttLog2(void);
int FDB_AddPhotoID(PPhotoID picid);
int FDB_CntPhoto(void);
int FDB_AddTmpV10(PZKFPTemplate tmp);

//zsliu add
int FDB_SetDuressTemp(U16 PIN, char FingerID,U16 flag);
int AppendSSRUserTemp(int pin, int isdelete, int fingerid, char *temp, int tmplen,int valid);

int FDB_UpgradeTmpV10DB();

int checkfilestatus(const char* filename);
int readfile(const char* filename,char *buf);
//clear system caches when copy
void clearSystemCaches();

int FDB_CntPhotoCount();

#define FILECP_OK 0
#define FILECP_FAIL -1
#define FILE_WIDTH gOptions.LCDWidth
#define FILE_HEIGHT gOptions.LCDHeight
int fileCp(const char *destFileName,const char *srcFileName);

/*异地考勤,add by yangxiaolong,2011-06-14,start*/
/*
*异地考勤，用户信息在设备中保存一定时间(由options参数设定)
*当当前时间-m_downLoadTime >= optionsTime(options参数设定时间)时，
*用户所有信息被删除(包括用户密码，工号，指纹)
*/
typedef struct _UserDownLoadTime
{
	U16 m_pin;					//用户ID
       time_t m_downLoadTime;		//用户下载时间戳
}GCC_PACKED TUserDLTime,*PUserDLTime; 

int FDB_AddUserDLTime(PUserDLTime  UserDLTimelog);
int FDB_DelUserDLTime(U16 pin);
int FDB_GetUserDLTime(U16 pin, PUserDLTime  PUserDLTimelog);

/*异地考勤,add by yangxiaolong,2011-06-14,end*/

/*考勤数据自动校对功能，add by yangxiaolong,2011-6-20,start*/
/*
*Function:FDB_CheckAttlog
*Description:统计某段时间内的考勤记录总数
*Input Prama:param--包含
*			startStamp--起始时间戳
*			endStamp--截止时间戳
*Output Prama:sumAttlog--考勤记录总数
*Return:0 --执行成功
		-1--参数错误
*/
 int  FDB_CheckAttlog(char *param,unsigned int *sumAttLog);
/*考勤数据自动校对功能，add by yangxiaolong,2011-6-20,end*/

void write_tracemsg(const char *hint);

int FDB_DelOldAttPicture(int num);
char *FDB_ReadAttBlock(int *size, int ContentType);
int GetDataInfo(int ContentType, int StatType, int Value);
int TruncFDAndSaveAs(int fd, char *filename, char *buffer, int size);
int FDB_DelPhotoID(int index);
void write_tracemsg(const char *hint);
int FDB_DelPhoto_By_Time(int timedot);
int FDB_DelPhoto_By_Count(int count);
PUserlb GetUsermestolb_PageForward(int deptid, int *cnt, int smsPin);
PUserlb GetUsermestolb_PageBackward(int deptid, int *cnt, int smsPin);
PUserlb GetUsermestolb_NameOrAcno(char *name, char *pin2, int *cnt);
PUserlb GetUsermestolb_SMS(int smspin);
int FDB_CntUserByMap(void);
int ProcessBatchData(void);
int GetTmpPameters(int *fingercount,int *UserIDs);
int GetMax2MV10TmpData(int *Size,int fingercount,int *currentCnt,int*UserIDs,char* Buff);
int GetTmpDataByID(int *Size,int fingercount,int *currentCnt,int*UserIDs,char* Buff);
int GetFreePIN2Fast(char *pin2);

typedef struct SerialCardNumber{
	U16	PIN;
	U8 CardSerialNumber[10];	 		//应用序列号
}GCC_PACKED TSerialCardNo, *PSerialCardNo;

#endif

