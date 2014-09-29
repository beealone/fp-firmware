#ifndef _DATAMAP_H_
#define _DATAMAP_H_

#include <time.h>
#include "arca.h"
#include "ccc.h"
#include "flashdb.h"
#include "exfun.h"

//用户
typedef struct _userlib
{
	U16	Index;
	U16	PIN;
	char	PIN2[24];
	U8	Card[4];
	U8	Privilege;		//bit7=0:无密码，=1:有密码
	struct 	_userlib* pre;
	struct 	_userlib* next;
}TUserLib, *PUserLib;
PUserLib MAP_GetAllUser(void);

//短消息
typedef struct _smslib
{
	U16	Index;
	U16	ID;
	U32	StartTime;
        BYTE    Tag;
	U16	ValidMinutes;
	struct 	_smslib* pre;
	struct 	_smslib* next;
}TSmsLib, *PSmsLib;
PSmsLib MAP_GetAllSms(void);

//短消息分配表
typedef struct _udatalib
{
	U16	Index;
	U16	PIN;
	U16	SmsID;
	struct	_udatalib* pre;
	struct 	_udatalib* next;
}TUDataLib, *PUDataLib;
PUDataLib MAP_GetAllUData(void);

//定时响铃
typedef struct _alarmlib
{
	U16	Index;
	U16	AlarmIDX;
	U8	AlarmHour;
	U8	AlarmMin;
	U8	AlarmStatus;
	struct	_alarmlib* pre;
	struct	_alarmlib* next;
}TAlarmLib, *PAlarmLib;
PAlarmLib MAP_GetAllAlarm(void);

typedef struct _belllib
{
	U16	Index;
	U16	BellID;
	U8	SchInfo;
	U8	BellTime[2];
	struct	_belllib* pre;
	struct	_belllib* next;
}TBellLib, *PBellLib;
PBellLib MAP_GetAllBell(void);

//快捷键
typedef struct _shortkeylib
{
	U8	Index;
	U8	keyID;
	U8	keyFun;
	U8	stateCode;
	U16	Time[7];
	U8	autochange;
	char stateName[STATE_NAME_LEN+2]; 
	struct	_shortkeylib* pre;
	struct	_shortkeylib* next;
}TShortKeyLib, *PShortKeyLib;
PShortKeyLib MAP_GetAllShortKey(void);

//workcode
typedef struct _workcodelib
{
	U16	Index;
	U16	PIN;
	char	Code[8];
	struct	_workcodelib* pre;
	struct	_workcodelib* next;
}TWorkCodeLib, *PWorkCodeLib;
PWorkCodeLib MAP_GetAllWorkCode(void);

//部门
typedef struct _deptlib
{
	U8	Index;
	U8	Deptid;
	struct	_deptlib* pre;
	struct	_deptlib* next;
}TDeptLib, *PDeptLib;
PDeptLib MAP_GetAllDept(void);

//排班
typedef struct _timeclasslib
{
	U16	Index;
	U16	schClassid;
	time_t	StartTime;
	time_t	EndTime;
	struct	_timeclasslib* pre;
	struct	_timeclasslib* next;
}TTimeClassLib, *PTimeClassLib;
PTimeClassLib MAP_GetAllTimeClass(void);

//时间段
typedef struct _timezonelib
{
	U16	Index;
	U16	ID;
	U8	ITime[7][2][2];
	struct	_timezonelib* pre;
	struct	_timezonelib* next;
}TTimeZoneLib, *PTimeZoneLib;
PTimeZoneLib MAP_GetAllTimeZone(void);

//节假日
typedef struct _holidaylib
{ 
	U16	Index;
	U16	ID;
	U8	HDate[2][2];
	struct	_holidaylib* pre;
	struct	_holidaylib* next;
}THolidayLib, *PHolidayLib;
PHolidayLib MAP_GetAllHoliday(void);

//扩展用户
typedef struct _extuserlib
{
	U16	Index;
	U16	PIN;
	struct	_extuserlib* pre;
	struct	_extuserlib* next;
}TExtuserLib, *PExtuserLib;
PExtuserLib MAP_GetAllExtuser(void);

//组
typedef struct _grouplib
{
	U8	Index;
	U8	ID;
	struct	_grouplib* pre;
	struct	_grouplib* next;
}TGroupLib, *PGroupLib;
PGroupLib MAP_GetAllGroup(void);

//开锁组合
typedef struct _unlockgrouplib
{
	U8	Index;
	U8	ID;
	U8	GroupID[5];
	U8	MemberCount;
	struct	_unlockgrouplib* pre;
	struct	_unlockgrouplib* next;
}TUnlockGroupLib, *PUnlockGroupLib;
PUnlockGroupLib MAP_GetAllUnlockGroup(void);

//指纹模板
typedef struct _templatelib
{
	U32	Index;
	U16	Size;
	U16	PIN;
	BYTE	FingerID;
	BYTE	Valid;
	struct	_templatelib* pre;
	struct	_templatelib* next;
}TTemplateLib, *PTemplateLib;
PTemplateLib MAP_GetAllTemplate(void);

//PhotoIndex()

#define MAX_DATA_FILE_COUNT	100		//文件数量最大值
#define READ_FILE_DATA		0		// 0= 读文件数据
#define NOT_READ_DATA		1		//  1＝不读数据，
#define READ_NODE_DATA	2		//   2＝读当前节点数据

char NotReadDataFlag[MAX_DATA_FILE_COUNT];// 0= 读文件数据，1＝不读数据，2＝读当前节点数据


PUserLib pMapUser, pCurUser;
PSmsLib pMapSms, pCurSms;
PUDataLib pMapUData, pCurUData;
PAlarmLib pMapAlarm, pCurAlarm;
PBellLib pMapBell, pCurBell;
PShortKeyLib pMapShortKey, pCurShortKey;
PWorkCodeLib pMapWorkCode, pCurWorkCode;
PDeptLib pMapDept, pCurDept;
PTimeClassLib pMapTimeClass, pCurTimeClass;
PTimeZoneLib pMapTimeZone, pCurTimeZone;
PHolidayLib pMapHoliday, pCurHoliday;
PExtuserLib pMapExtuser, pCurExtuser;
PGroupLib pMapGroup, pCurGroup;
PUnlockGroupLib pMapCGroup, pCurCGroup;
PTemplateLib pMapTemplate, pCurTemplate;

void MAP_GetAllDataToMap(void);
void MAP_ResetMapList(int ContentType);
void MapSearchFirst(int ContentType);
BOOL MapSearchNext(int ContentType);
int GetRecordSize(int ContentType, int index);
int MAP_GetDataFromFile(int pos, PSearchHandle sh);
void MAP_ModifyMapList(int ContentType, char *buffer);
int MAP_GetDataInfo(int ContentType, int StatType, int Value);

#endif
