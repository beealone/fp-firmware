#ifndef _FULIYE_H
#define _FULIYE_H


#ifdef __cplusplus
extern "C"{
#endif

#define LICENSEVERSION 0x01		//当前协议版本0x01
#define MAX_DATA_LEGHT 1024

#define MAX_POST_ATT_COUNT	1		//一个包上传考勤流水数量，最大设置为35
#define BCDTODEC(bcd) ((bcd) = ((bcd) & 0x0f) + ((bcd)>>4) * 10)

#ifndef GCC_PACKED
#define GCC_PACKED __attribute__((packed))
#endif

typedef enum servicetype{
	INVALIDTYPE=0x0000,		//无效的业务代码	
						
	DEVREGISTER=0x0001,		//设备注册
	DEVOPEN=0x0002,		//设备开通
	UPDATECOMMKEY=0x0003,		//更新通信密钥
	UPDATESERVADD=0x0004,		//更新服务器地址
	DOWNCOMMPARAM=0x0005,		//下发通讯参数
	HEARTBEAT=0x0006,		//心跳
	WARNINTINFO = 0x0007, //警告信息
	ERRORINFO = 0x8000, //警告信息

	DOWNPARAM=0x0301,		//下发机具参数
	DOWNUSERLIST=0x0302,		//考勤名单下发
	UPLOADATTDATA=0x0303,		//考勤流水上传
	UPLOADATTPIC=0x0304,		//考勤照片上传

	DOWNACCESSPARAM=0x0401,	//门禁参数下发
	DOWNACCESSUSER=0x0402,		//门禁人员下发
	DOWNWEEKPLAN=0x0403,		//周计划下发
	DOWNHOLIDAYPLAN=0x0404,	//假日计划下发
	UPLOADACCESSSLOG=0x0405,	//门禁记录上传
}ServiceType;

/**************************************
注册设备data部分
**************************************/


//设备注册请求data部分结构体	39字节
typedef struct dev_register{
	U8 DevNum[11];				//机具编号  
	U8 PSAM_num[6];				//PSAM卡号
	unsigned short int DevType;			//机具类型
	U8 CommType;				//通信类型
	U8 DevVersion[8];			//机具版本
	U8 DevTime[7];				//机具时间
	U8 CustomID[4];			//客户ID
}GCC_PACKED TRegister,*PRegister;

//设备注册，返回结构体	28字节
typedef struct dev_register_ret{
	U8	DevNum[11];			//机具编号
	unsigned int OpenKey[4];			//开通密钥
	unsigned char Result;		//结果	0x01成功，其他失败
}GCC_PACKED TRegisterRet,*PRegisterRet;

/**************************************
开通设备data部分
**************************************/


//设备开通请求data部分结构体	15字节
typedef struct dev_open{
	U8 DevNum[11];				//机具编号
	U8 CustomID[4];			//客户ID
}GCC_PACKED TOpenDev,*POpenDev;

//设备开通，返回结构体  	64字节
typedef struct dev_open_ret{
	U8 DevNum[11];				//机具编号
	U8 ServerTime[7];			//服务器时间
	unsigned int CommKey[4];			//通信密钥，新的通信密钥使用3DES进行加密，加密所用密钥为开通密钥
	U8 ServerIP[4];			//机具接入服务器IP
	unsigned short int ServerPort;			//机具接入服务器端口
	unsigned int CPU_AID[4];				//CPU卡AID
	unsigned int PowerPwd[2];			//开机密码
}GCC_PACKED TOpenDevRet,*POpenDevRet;


/**************************************
下发通信参数部分
**************************************/

typedef struct  comm_param{
	U8 DevNum[11];				//机具编号
	U8 UploadMode;				//上传模式
	U8 TransInterval[8];			//上传时间间隔
	unsigned short int UpChachelogCnt;		//上传缓存记录数
	U8 Sessioninterval;			//会话间隔
	unsigned short int delay;				//心跳
	unsigned short int ErrorDelay;			//超时未响应重发阀值
	U8 ReTransCnt;				//重发次数阀值
}GCC_PACKED TCommParam, *PCommParam;

/**************************************
更新通信秘钥部分
**************************************/

typedef struct update_commkey{
	U8 DevNum[11];				//机具编号
	U8 ServerTime[7];			//服务器时间
	unsigned int MainCommKey[4];		//主密钥使用3DES进行加密，加密所用密钥为旧的通信密钥
}GCC_PACKED TUpdateCommKey,*PUpdateCommKey;

/**************************************
更新服务器地址部分
**************************************/

typedef struct update_srvaddress{
	U8 DevNum[11];				//机具编号
	U8 ServerTime[7];			//服务器时间
	U8 ServerIP[4];			//机具接入服务器IP
	unsigned short int ServerPort;			//机具接入服务器端口
}GCC_PACKED TUpdateSrvAddr,*PUpdateSrvAaddr;

/*********************************************
机具应答报文   (更新通信秘钥  更新通信地址  
**********************************************/
typedef struct normal_answer{
	U8 DevNum[11];
	U8 Result;
}GCC_PACKED TNomalAns, *PNomalAns;

/***************************************
机具应答报文   (下发通信参数机具应答)
***************************************/
typedef struct commpram_answer{
	U8	DevNum[11];
	unsigned short int DevType;			//机具类型
}GCC_PACKED TCommParamAns, *PCommParamAns;

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

/**********************************
机具应答报文  考勤机具参数下发机具应答
***************************************/
typedef struct param_answer{
	U8	DevNum[11];
	unsigned int	ParamVer;
}GCC_PACKED TParamAns, *PParamAns;

/*****************
心跳包data部分
**************************************/

//心跳包		36字节
typedef struct heartbeat{
	U8 DevNum[11];				//机具编号 
	unsigned int CommVer;				//机具通信版本
	U8 ListType;				//名单类型
	unsigned int ListVer;				//名单版本号
	unsigned int Reserved[4];			//保留字段	
}GCC_PACKED THeartbeat,*PHeartbeat;

//心跳包返回
typedef struct heartbeatret{
	U8 DevNum[11];				//机具编号
	U8 ServerTime[7];			//服务器时间
	unsigned short int CommandType;		//指令类别
	unsigned int Reserverd[4];			//保留字段
}GCC_PACKED THeartbeatRet,*PHeartbeatRet;




/************************************
考勤名单下发data部分
data=datahead+名单1+名单2+名单3....
data= 22字节+58字节+58字节+58字节....
*************************************/

//考勤名单下发data中head部分	22字节
typedef struct DownloadUserHead{
	U8	DevNum[11];				//机具编号
	U8	OperationType;			//操作标示		0x00:机具名单异常，需擦除名单，从版本0开始请求  0x01:机具名单正常
	unsigned short int UserNum;			//名单个数
	unsigned int UserRemain;				//剩余个数
	unsigned int ListVer;				//名单版本
}GCC_PACKED TUserHead, *PUserHead;

//名单		58字节
typedef struct DownloadUserData{
	U8	Card[4];				//物理卡号
	U8	CardSerial[10];			//卡应用序列号
	unsigned char Name[20];				//姓名,
	unsigned char PIN2[16];				//工号(学号)
	U8	CardStatus;				//卡状态标志
	unsigned char Password[6];			//密码
	U8	Privilege;				//人员标识(权限)
}GCC_PACKED TUserData,*PUserData;

//考勤名单下发返回	23字节
typedef struct DownloadUserRet{
	U8	DevNum[11];				//机具编号
	unsigned int	ListVer;				//名单版本
	U8	CardSerialNumber[10]; 		//最后一条用户数据的应用序列号
	unsigned int	UserCount;				//当前机具中名单数量
	unsigned int	UserRemain;				//当前机具中剩余名单空间
}GCC_PACKED TUserRet, *PUserRet;

/****************************************************************
考勤记录上传data
data=datahead + 流水1 + 流水2 + 流水3 +....+ 流水N + 剩余流水数量
data= 12字节+25字节+25字节+25字节+....+25字节+4字节
*****************************************************************/


//考勤记录上传data中head部分	12字节
typedef struct UploadAttHead{
	U8	DevNum[11];				//机具编号
	U8	AttlogCount;			//流水数量
}GCC_PACKED TUploadAttHead,*PUploadAttHead;

//考勤流水数据					25字节
typedef struct AttData{
	U8	CardSerial[10];			//卡应用序列号
	U8	Card[4];			//卡物理序列号
	U8	AttTime[7];				//打卡时间
	unsigned int AttSerialNum;			//打卡序号
}GCC_PACKED TAttData,*PAttData;

//考勤流水平台应答
typedef struct UploadAttRet{
	U8	DevNum[11];				//机具编号
	unsigned int	LastAttSerNum;			//最后一条流水序号
}GCC_PACKED TUploadAttRet,*PUploadAttRet;

typedef struct _postattlog{
	U8  attbuffer[1024];	//上传流水记录的buffer		每次最多39条
	unsigned int maxrec;					//当前考勤记录最大流水号
	unsigned int currec;					//当前考勤上传流水号
	unsigned int reclenght;				//读取attlog记录到attbuffer的长度
	unsigned int lastrec;					//服务器应答最后一条流水序号
	unsigned int readattflag;			//读取数据标志，1表示已经读取数据，可以进行发送
	off_t	curfp;		//当前ssrattlog.dat读取的指针
}GCC_PACKED TPostAttLog,*PPostAttLog;


/****************************************
门禁报文: 门禁人员名单下发
		 门禁事件记录上传
		 周计划下发
		 假日计划下发
		 门禁参数下发
****************************************/
//门禁人员下发  数据头24bit
typedef struct  DownloadAccessHead{
	U8	DevNum[11];				//机具编号
	U8	OperationType;			//操作标示		0x00:机具名单异常，需擦除名单，从版本0开始请求  0x01:机具名单正常
	unsigned short int UserNo;			//名单个数
	unsigned int Remain;				//剩余个数
	unsigned int ListVer;				//名单版本
	U8	DoorNumber;					//门号
}GCC_PACKED TAccessHead, *PAcessHead;

//门禁人员下发数据部分15bit*n
typedef struct DownloadAccessData{
	U8	CardSerial[10];			//卡应用序列号			AccessStatu:	按位定义，最低位为 bit0，最高位为 bit7 。
	U8	Card[4];				//物理卡号								Bit3~ bit0 表示周计划编号(1~15)；
	U8	AccessStatus;			//状态位									Bit7~ bit4 表示假日划编号(1~15)
}GCC_PACKED TAccessUser, *PAccessUser;								//	Bit7~ bit4 为 0 表示不启用假日计划 
																	//	此字节为0表示受禁名单			
//门禁下发机具应答报文 16bit
typedef struct AccessUserRet{
	U8	DevNum[11];	
	U8	DoorNumber;
	unsigned int Listver;
}GCC_PACKED TAccessUserRet, *PAccessUserRet;

/*******************************************
	时间段处理
*******************************************/
typedef struct TimeZone{
	U8	StartTime[2];
	U8	EndTime[2];
	U8	status;
}GCC_PACKED AccessTZ;

//日时间段5*5bit
typedef struct DayTimeZone{
	AccessTZ	DayTime[5];			//每天5 个时间段
}GCC_PACKED DayTime;
																	
typedef struct HolidayTimeZone{
	U8	HolidayTime[3];
	AccessTZ DayTimeZone[5];
}GCC_PACKED HolidayTZ;

/*********************************************/
																	
//周计划下发192bit 
typedef struct DownloadWeekPlan{					//一周分 7 天， 每天分 5 个时间段， 每个时间段使用 5 个字节表示。
	U8	DevNum[11];									//时分＋时分＋状态				
	U8	DoorNumber;									//状态域意义:0=刷卡开门3=常开（效果同刷卡开门）2=常闭（刷卡不开门
	U8	WeekPlanNumber;
	DayTime	WeekTime[7];
	unsigned int Listver;
}GCC_PACKED TWeekPlan, *PWeekPlan;

//周计划应答报文17bit
typedef struct DownloadWeekPlanRet{
	U8	DevNum[11];
	U8	DoorNumber;
	U8	WeekPlanNumber;
	unsigned int Listver;
}GCC_PACKED TWeekPlanRet, *PWeekPlanRet;


//假日计划下发327bit
typedef struct DownloadHolidayPlan{
	U8	DevNum[11];	
	U8	DoorNumber;	
	U8	HolidayNumber;
	HolidayTZ HolidayPlan[10];
	unsigned int Listver;
}GCC_PACKED THolidayPlan, *PHolidayPlan;

//假日计划应答报文17bit
typedef struct DownloadHolidayPlanRet{
	U8	DevNum[11];	
	U8	DoorNumber;	
	U8	HolidayNumber;
	unsigned int Listver;
}GCC_PACKED THolidayPlanRet, *PHolidayPlanRet;


/**************************************
通信协议:
Head + data + checksum

head:		协议版本+总长度+帧号+业务代码	
			1字节+3字节+2字节+2字节
data:		变长
checksum:	4字节
**************************************/


//YMIP 报文头	8字节
typedef struct YMIP_head{
	unsigned char LisenceVer;			//协议版本
	unsigned char Lenght[3];	//总长度
	unsigned short int FrameNum;			//帧号
	unsigned short int ServiceCode;		//业务代码
}GCC_PACKED TYMIP_HEAD, *PYMIP_HEAD;

//YMIP 数据
typedef struct TMIP_data{
	char *Data;					//数据长度，一般不超过1024
}TYMIP_DATA, *PYMIP_DATA;

//协议发送的总体格式
typedef struct Transinfo{
	PYMIP_HEAD Head;
	PYMIP_DATA Data;
	unsigned int CheckCode;				//4字节校验码
}GCC_PACKED TTransinfo, *PTransinfo;

/********************************************
	错误应答报文         警告报文  8000
********************************************/
typedef struct error_response{
	U8	DevNum[11];
	unsigned short int	ErrorCode;
	U8	ErrorDscription[50];
}GCC_PACKED TErrorRet, *PErrorRet;

typedef struct warning_respose{
	U8 DevNum[11];				//机具编号  
	U8 PSAM_num[6];				//PSAM卡号
	unsigned short int DevType;			//机具类型
	U8 CommType;				//通信类型
	U8 DevVersion[8];			//机具版本
	U8 DevTime[7];				//机具时间
	U8 WarningType;
	U8 WarningLevel;
	unsigned int WarningCode;
	U8 WarningInfo[16];
}GCC_PACKED TWarning, *PWarning;

typedef struct warning_return{
	U8	DevNum[11];
	U8 	DevTime[7];				//机具时间
	U8	WaningResult;		//0x01 success
}GCC_PACKED TWarningRet, *PWarningRet;


extern unsigned char DevNumber[11];



int incFN(void);

int IsRequest(PYMIP_HEAD head);
int IsReturn(PYMIP_HEAD head);

int time2byte(unsigned char devtime[]);



PRegister ymip_data_reg_format(PRegister data,U8 *DevNum,U8 *PSAM_num,short int DevType,U8 CommType,U8 *DevVersion,U8 *DevTime,U8 *CustomID);
PYMIP_HEAD ymip_head_format(PYMIP_HEAD head,char lisensever,unsigned char lenght[],short int framenum,short int servicecode);
PTransinfo ymip_transinfo_format(PTransinfo transinfo,PYMIP_HEAD head,PYMIP_DATA data,unsigned int datalen,int checkcode);

#ifdef __cplusplus
}
#endif

#endif
