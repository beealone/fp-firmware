/*************************************************
                                           
 ZEM 200                                          
                                                    
 commu.h communication for PC                             
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _COMMU_H_
#define _COMMU_H_

#include "arca.h"
#include "ccc.h"
#include "utils.h"

//command for data management
#define CMD_DB_RRQ 		7			/* read request */
#define CMD_USER_WRQ 		8			/* write request */
#define CMD_USERTEMP_RRQ 	9			/* read request */
#define CMD_USERTEMP_WRQ 	10			/* write request */
#define CMD_OPTIONS_RRQ		11			/* read request */
#define CMD_OPTIONS_WRQ 	12			/* write request */
#define CMD_ATTLOG_RRQ 		13			/* read request */

#define CMD_CLEAR_DATA		14			/* command */
#define CMD_CLEAR_ATTLOG	15

#define CMD_APPEND_USER		16
#define CMD_APPEND_USERTEMP 	17
#define CMD_DELETE_USER		18
#define CMD_DELETE_USERTEMP	19
#define CMD_CLEAR_ADMIN	20

//门禁相关命令
#define CMD_USERGRP_RRQ		21				//读用户分组
#define CMD_USERGRP_WRQ		22				//设置用户分组
#define CMD_USERTZ_RRQ		23				//读用户时间段设置
#define CMD_USERTZ_WRQ		24				//写用户时间段设置
#define CMD_GRPTZ_RRQ		25				//读组时间段设置
#define CMD_GRPTZ_WRQ		26				//写组时间段设置
#define CMD_TZ_RRQ		27				//读时间段设置
#define CMD_TZ_WRQ		28				//写时段设置
#define CMD_ULG_RRQ		29				//读开锁组合
#define CMD_ULG_WRQ		30				//写开锁组合
#define CMD_UNLOCK		31				//打开锁
#define CMD_CLEAR_ACC		32				//清除门禁设置
#define CMD_HTZ_RRQ		91
#define CMD_HTZ_WRQ		92

#define CMD_CLEAR_OPLOG		33
#define CMD_OPLOG_RRQ 		34			/* read request */

#define CMD_RULE_RRQ		35
#define CMD_DEPT_RRQ		36
#define CMD_SCH_RRQ  		37


#define CMD_TEMPDB_CLEAR	48
#define CMD_TEMPDB_ADD		49

#define CMD_GET_FREE_SIZES  	50
#define CMD_GET_DATA_LAYOUT 	51
#define CMD_UPDATE_USERS	52			/* update all users data if it's empty */
#define CMD_UPDATE_TEMP		53		/* update all users template data if it's empty */
#define CMD_USER_RRQ		54
#define CMD_FREEID_RRQ		55
#define CMD_SENSOROPT_WRQ	56
#define CMD_ENABLE_CLOCK	57
#define CMD_SENSOROPT_RRQ       58
#define CMD_RTLOG_RRQ           90


//模块相关命令
#define CMD_STARTVERIFY		60
#define	CMD_STARTENROLL		61
#define ERR_OK		0
#define ERR_STATE	8
#define ERR_PARAM	1
#define ERR_ENROLLED	2
#define ERR_SAVE	3
#define ERR_FAIL	4
#define ERR_REPEAT	5
#define ERR_CANCEL	6
#define ERR_NOTENROLLED	7

#define	CMD_CANCELCAPTURE	62
#define CMD_TRANSSTATE		63
#define CMD_STATE_RRQ		64
#define CMD_LASTTEMP_RRQ	65
#define CMD_WRITE_LCD		66
#define CMD_CLEAR_LCD		67
#define CMD_LCDSIZE_RRQ		68
#define CMD_GET_PINWIDTH 	69

//SMS command
#define CMD_SMS_WRQ 		70
#define CMD_SMS_RRQ 		71
#define CMD_DELETE_SMS 		72
#define CMD_UDATA_WRQ   	73
#define CMD_DELETE_UDATA 	74

#define CMD_GET_IOSTATUS	75

//Mifare card command
#define CMD_WRITE_MIFARE        76
#define CMD_READ_MIFARE         77
#define CMD_EMPTY_MIFARE        78

//EXTUSER
#define CMD_EXTUSER_WRQ		79
#define CMD_EXTUSER_RRQ		80
#define CMD_DELETE_EXTUSER	81

//Workcode command
#define CMD_WorkCode_WRQ        82
#define CMD_WorkCode_RRQ        83
#define CMD_DELETE_WorkCode     84

//ShortKey command
#define CMD_STKEY_WRQ	85
#define CMD_STKEY_RRQ	86

//template
#define CMD_USERTEMP_EX_WRQ	87
#define CMD_USERTEMP_EX_RRQ	88
//command for fireware update
#define CMD_UPDATE_FIREWARE	110		/* update firmware */
#define UPDATE_BOOTLOADER	0
#define UPDATE_FIRMWARE		1
#define UPDATE_CFIRMWARE	2
#define UPDATE_FONT		3
#define UPDATE_OPTIONS		4
#define UPDATE_USERS		5
#define UPDATE_TEMPS		6
#define UPDATE_FLASH		7		/* WRITE FLASH FROM AN ADDRESS */
#define UPDATE_SOUND		8		/* update wav files */
#define UPDATE_ALGORITHM	9	       /* update algorithm files */
#define UPDATE_LANGUAGE		10	       /* update language files*/
#define UPDATE_AUTOSHELL	11	       /* update AUTO.SH files */
#define UPDATE_BATCHUSERDATA	12
#define UPDATE_UDISKUPGRADEPACKAGE	13
#define UPDATE_RUN		100

#define CMD_QUERY_FIRWARE	111		/*  */
#define CMD_CPU_REG			112
#define CMD_CMOS_GAIN		113
#define CMD_UADATA_RRQ		114		/*  */
#define CMD_UADATA_WRQ		115		/*  */
#define CMD_APPEND_INFO		116
#define CMD_USER_INFO		117
#define CMD_MTHRESHOLD		118
#define CMD_HASH_DATA		119
#define CMD_READ_NEW		120
#define CMD_UPDATE_READALL	121

//2008.09.02 解决上传数据乱
#define CMD_SSRUSERTEMP_WRQ     132
#define CMD_SSRDELETE_USER      133
#define CMD_SSRDELETE_USERTEMP  134

//2008.12.15 发送命令，从U盘升级固件,LOGO图片,options.cfg和MAC地址
#define CMD_UPDATEFROMUDISK	135

//command for device options
#define CMD_GET_TIME 		201		/* write request */
#define CMD_SET_TIME 		202
#define CMD_GET_COUNT 		203

//command for register a event notification
#define CMD_REG_EVENT		500	

// command for connect and device control
#define CMD_CONNECT 		1000	/* connect request */
#define CMD_EXIT 		1001	/* disconnect request */
#define CMD_ENABLEDEVICE 	1002	/* enabled device operation */
#define CMD_DISABLEDEVICE	1003	/* disable device opreation */
#define CMD_RESTART		1004	/* restart device */
#define CMD_POWEROFF		1005	/* power off device */
#define CMD_SLEEP		1006    /* sleep device */
#define CMD_RESUME		1007	/* Resume sleeping device */

#define CMD_AUXCOMMAND		1008    /* Aux Board Command */
#define CMD_CAPTUREFINGER	1009    /* Capture a fingerprint image */
#define CMD_ENROLL		1010	/* Capture a fingerprint to enroll template */
#define CMD_TEST_TEMP		1011
#define CMD_CAPTUREIMAGE	1012    /* Capture a full fingerprint image */
#define CMD_REFRESHDATA		1013	/* Reload data from rom to cache */
#define CMD_REFRESHOPTION	1014	/* Refresh Options */
#define CMD_CALC_FINGER		1015	
#define CMD_RUN_PRG		1016
#define CMD_TESTVOICE		1017
#define CMD_GET_FLASHID		1018
#define CMD_GET_MCU_VERSION	1019
#define CMD_MCU_COMMAND		1020
#define CMD_SETFPDIRECT         1021    //Direct capture finger and not detect`
#define CMD_GET_VERSION		1100
#define CMD_CHANGE_SPEED	1101
#define CMD_AUTH		1102	/* 授权 */
#define CMD_ALARM_TRIGER        1104
#define CMD_WIEGAND 		1105

#define CMD_SERURITY_KEY_RRQ	1200
#define CMD_SERURITY_DATA_RRQ	1201
#define CMD_SERURITY_KEY_WRQ	1202

#define CMD_PREPARE_DATA	1500	/* Prepare data transfer */
#define CMD_DATA		1501	/* Send a data packet */
#define CMD_FREE_DATA		1502	/* Free the transfered data */
#define	CMD_QUERY_DATA		1503
#define CMD_READ_DATA		1504
//upldate file like userphoto,logo
#define CMD_UPDATEFILE          1700
#define CMD_READFILE            1702 //zhc 2008.11.24 read file

#define CMD_CHECKUDISKUPDATEPACKPAGE	1709
#define CMD_OPTIONS_DECIPHERING	1710
#define CMD_OPTIONS_ENCRYPT	1711

//response from device
#define CMD_ACK_OK 		2000	/* Return OK to execute */
#define CMD_ACK_ERROR 		2001	/* Return Fail to execute command */
#define CMD_ACK_DATA		2002	/* Return Data */
#define CMD_ACK_RETRY		2003	/* Regstered event occorred */
#define CMD_ACK_REPEAT		2004
#define CMD_ACK_UNAUTH		2005	/* 连接尚未授权 */
#define CMD_ACK_UNKNOWN		0xffff	/* Return Unknown Command */

//define sdk communication command
#define CMD_GET_PHOTO_COUNT 		2013
#define CMD_GET_PHOTO_BYNAME    	2014
#define CMD_CLEAR_PHOTO_BY_TIME 	2015
#define CMD_GET_PHOTONAMES_BY_TIME   	2016

#define CMD_READ_LARGE_TMP		2017 /*读取5万枚10算法指纹模板*/
#define CMD_DATA_LARGE_TMP		2018/*脱机sdk读取大数据时分块读取*/
#define CMD_FREE_LARGE_TMP		2019/*读取完数据之后释放内存资源*/

#define CMD_LARGE_FINGER_MACHINE	2221     /*判断是否是支持大容量指纹传输*/
#define CMD_LARGE_FINGER_UPDATEFLAG	2222     /*高速上传50000指纹标识*/

typedef int (*SendDataProc)(void *buf, int size, void *param);
typedef int (*CloseSessionProc)(void *param);

#define EF_ATTLOG	1
#define EF_FINGER	(1<<1)
#define EF_ENROLLUSER	(1<<2)
#define EF_ENROLLFINGER	(1<<3)
#define EF_BUTTON	(1<<4)
#define EF_UNLOCK	(1<<5)		//开锁
#define EF_STARTUP	(1<<6)		//起动系统
#define EF_VERIFY	(1<<7)		//验证指纹
#define EF_FPFTR	(1<<8)		//提取指纹特征点
#define EF_ALARM 	(1<<9)          //报警信号
#define EF_HIDNUM	(1<<10)		//射频卡号码
#define EF_WRITECARD    (1<<11)         //写卡成功
#define EF_EMPTYCARD    (1<<12)         //清除卡成功
#define EF_DELTEMPLATE  (1<<13)    
#define EF_ALARMOFF	(1<<14)		//报警被解除

#define SENDER_LEN	16
#define MAX_BUFFER_MSG  80           //最大缓存事件个数
#define MAX_MSGBUFFER_SIZE      1024 //事件缓存区的大小

#define ENROLLMODE_ONLINE	1
#define ENROLLMODE_STANDALONE	0

//2009.09.21 ......
#define CMD_USERFACE_RRQ	150	    /* read request */
#define CMD_USERFACE_WRQ	151	    /* write request */
#define CMD_APPEND_USERFACE     153         /*....*/
#define CMD_DELETE_USERFACE	152         /*....*/

/*
typedef struct _comm_session_{
	int SessionID;
	int StartTime;
	int LastActive;
	int LastCommand;
	int LastReplyID;
	int LastSendLen;
	char LastSendData[1032];
	void *Buffer;
	int BufferPos;
	int BufferLen;
	int Speed;
	int RegEvents;
	int Authen;
	WORD VerifyUserID;
	BYTE VerifyFingerID;
	SendDataProc Send;
	char Sender[SENDER_LEN];
	int TimeOutSec;
	CloseSessionProc Close;
	char Interface[16];
        char MsgBuffer[MAX_MSGBUFFER_SIZE]; //??????????
        int MsgLength[MAX_BUFFER_MSG];  //???????????
        int MsgStartAddress[MAX_BUFFER_MSG];//??????????????
        int MsgLast;                    //????????????????
        int MsgCount;                   //?????????????
        int MsgCached;                  //???????????
}TCommSession, *PCommSession;
*/

typedef struct _comm_session_{
	int SessionID;
	int StartTime;
	int LastActive;
	int LastCommand;
	int LastReplyID;
	int LastSendLen;
	char LastSendData[1032];
	TBuffer *Buffer;
	int Speed;
	int RegEvents;
	int Authen;
	WORD VerifyUserID;
	BYTE VerifyFingerID;
	SendDataProc Send;
	char Sender[SENDER_LEN];
	char MsgBuffer[MAX_MSGBUFFER_SIZE];			//事件内容缓存
	int MsgLength[MAX_BUFFER_MSG];	//事件内容的长度
	int MsgStartAddress[MAX_BUFFER_MSG];//事件内容的起始地址
	int MsgLast;			//正在传送的事件的序号
	int MsgCount;			//等待传送的事件个数
	int MsgCached;			//是否缓存事件数据
	int TimeOutSec;
	
	CloseSessionProc Close;
	char Interface[16];
	int fdCacheData; //dsl.2011.10.27.Batch cached file handle	
}TCommSession, *PCommSession;

/*  PC->Device 

Reply ID is a flag to identify the replayed data from device. So while send data
to device, let ReplyID++ to keep every cmd have a different id.

|--------|--------|--------|--------|--------|--------|--------|--------|---
|       CMD       |    Check Sum    |    Session ID   |    Reply ID     |Data

*/

/* Device->PC

|--------|--------|--------|--------|--------|--------|--------|--------|---
|       CMD       |    Check Sum    |      Data1      |     Reply ID    |Data2
CMD=CMD_ACK_OK,CMD_ACK_ERROR,CMD_DATA,CMD_UNKNOWN
*/
typedef struct _CmdHdr_{
	unsigned short Command, CheckSum, SessionID, ReplyID;
}GCC_PACKED TCmdHeader, *PCmdHeader;

int CheckSessionSend(int EventFlag, char *Data, int Len);
PCommSession CheckSessionVerify(int *PIN, int *FingerID);

int RunCommand(void *buf, int size, int CheckSecury);
PCommSession CreateSession(void *param);
int CheckSessionTimeOut(void);
PCommSession GetSession(int SessionID);
int FreeSession(int SessionID);
int CheckSessionSendMsg(void);
void SendLargeDataByFile(char *in_buf, PCommSession session, int offset, int size);
void SendLargeData(char *in_buf, PCommSession session, char *buf, int size);
#endif

