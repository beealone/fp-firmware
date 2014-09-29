/*
 * rs_485reader.h
 *
 *  Created on: 2011-12-26
 *      Author: hp
 */

#ifndef RS_485READER_H_
#define RS_485READER_H_

#define	RS485_MASTER 1

//定义紧凑型结构及其字节对齐指针
#ifndef GCC_PACKED
#define GCC_PACKED __attribute__((packed))
#endif

#ifndef GCC_ALIGN0
#define GCC_ALIGN0 __attribute__((aligned(1)))
#endif

#define MAX_INBIO_SEND_DATA_LEN    1024*2
#define MAX_INBIO_REC_DATA_LEN     128

#ifndef U32
#define	U32   	unsigned int
#endif

#ifndef BYTE
#define BYTE  	unsigned char
#endif

typedef struct _inbiocommdata_
{
	int CommSendMark;//如果被inbiocomm进程修改，就改为1，如果main读取完了，就改为0
	int MainSendMark;//如果被main进程修改，就改为1，如果inbiocomm读取完了，就改为0
	int Cmd;
	int NewsType;
	int MainCmdRet;
	int CommCmdRet;
	int CreateCommCmd;
	int CreateMainCmd;
    U32 CardNo;
	BYTE ReaderID;
	int RebootTime;
	int RecLen;
	char SendData[MAX_INBIO_SEND_DATA_LEN];
	char RecData[MAX_INBIO_REC_DATA_LEN];
}GCC_PACKED TInbioCommData, *PInbioCommData;





//add by yuanfat for F8+SR200
//无权限开门
#define	ReturnToRS485ReaderNoPower()	\
do{\
	if(1 == gOptions.Reader485FunOn)\
	{\
		printf("Info:verify failed.[%s,%d]\n", __FILE__,__LINE__);\
		shared_Inbio_Comm_stuff->RecData[0] = 0xff;\
		shared_Inbio_Comm_stuff->RecLen = 1;\
		shared_Inbio_Comm_stuff->CommCmdRet = 0;\
	}\
}while(0);

#define	RS485_READER_RETURN_SUCCESS()	\
do{\
	if(1 == gOptions.Reader485FunOn)\
	{\
		printf("Info:verify success.[%s,%d]\n", __FILE__,__LINE__);\
		shared_Inbio_Comm_stuff->RecData[0] = 0;\
		shared_Inbio_Comm_stuff->RecLen = 1;\
		shared_Inbio_Comm_stuff->CommCmdRet = 0;\
	}\
}while(0);

#define	ReturnToRS485ReaderFailed()	\
do{\
	if(1 == gOptions.Reader485FunOn)\
	{\
		printf("Info:verify failed.[%s,%d]\n", __FILE__,__LINE__);\
		shared_Inbio_Comm_stuff->RecData[0] = 0xfe;\
		shared_Inbio_Comm_stuff->RecLen = 1;\
		shared_Inbio_Comm_stuff->CommCmdRet = 0;\
	}\
}while(0);

#define IsAntiPassbackOn()		(gOptions.AntiPassbackOn)
#define	IsAdvancedLockFunOn()	(gOptions.LockFunOn & LOCKFUN_ADV)
#define	Is485ReaderMaster()		(gOptions.Reader485FunOn)
//#define	IsInbiocommMsg()		(MSG_INBIO_COMM_CMD == msg->Param1)
#define	IsAthorByCardAndFP()	(VS_FP_AND_RF == verifyWay)

//add by yuanfat for F8+SR200
extern int g_iSlaveID;
extern int g_iVerifyCnt;
#define MAX_ATHOR_WAY_COUNT		2
#define IsSlaveDevice()		(g_iSlaveID)
#define SaveSlaveDevID(SlaveID)	\
do{\
	g_iSlaveID	= SlaveID;\
}while(0);

#define AddVerifyCount()	\
do{\
	g_iVerifyCnt++;\
}while(0);

#define	GetVerifyCount()	(g_iVerifyCnt)

#define ClearVerifyCount()	\
do{\
	g_iVerifyCnt=0;\
}while(0);



//此宏当函数应用
#define InitRS485Param()	\
do{\
	gOptions.RS232BaudRate = 115200;\
	SaveInteger("RS232BaudRate", 115200);\
\
	gOptions.RS485On = 1;\
	SaveInteger("RS485On", 1);\
\
	gOptions.RS485Port = 0;\
	SaveInteger("~RS485Port", 0);\
\
	gOptions.RS232On = 0;\
	SaveInteger("RS232On", 0);\
\
	gOptions.InBIOComType = 3;\
	SaveInteger("InBIOComType", 3);\
\
	gOptions.ZKFPVersion = ZKFPV10;\
	SaveInteger("~ZKFPVersion", ZKFPV10);\
\
	gOptions.AlarmReRec = 0;\
	SaveInteger("AlarmReRec", 0);\
\
	gOptions.FreeTime = 0;\
	SaveInteger("FreeTime", 0);\
}while(0);
//gOptions.ExtWGInFunOn = 0;
//SaveInteger("~ExtWGInFunOn", 0);

#define ExtractFingerTemplate()	\
do{\
	memcpy(LastTemplate,Temp,MAX_INBIO_SEND_DATA_LEN-4);\
	LastTempLen = MAX_INBIO_SEND_DATA_LEN-4;\
	Temp[0] = 0;\
}while(0);


#define ALARM_STRIP_TAG 	0x10000
extern int gAlarmStrip;
#define HandleStripAlarmTimeOut(MSG_VALUE)	\
do{	\
	if (gAlarmStrip >= ALARM_STRIP_TAG)	\
	{\
		printf("gAlarmStrip=0x%x,[%s,%d]\n", gAlarmStrip, __FILE__, __LINE__);\
		if (++gAlarmStrip > ALARM_STRIP_TAG+60)\
			gAlarmStrip = 0;\
		if(MSG_VALUE == MSG_TYPE_BUTTON)\
		{\
			gAlarmStrip = 0;\
		}\
	}\
}while(0);




//放于msg.h
#define MSG_INBIO_COMM_CMD  (1 << 13) //从inbio comm过来的指令。//add by yuanfat for F8+SR200

//add by yuanfat for F8+SR200
#define SUBCMD_VERIFY_FP  			29  //inbio comm进程发送main 的指纹比对指令
#define SUBCMD_VERIFY_FP_AND_RF  	30  //inbio comm进程发送main 的指纹加卡比对指令
#define SUBCMD_VERIFY_RF  			31  //inbio comm进程发送main 的仅卡比对指令
#define SUBCMD_VERIFY_FP_OR_RF  	32  //inbio comm进程发送main 的指纹或卡比对指令


//放于langres.h
//add by yuanfat for F8+SR100
#define MID_OC_485READER 		(1519)	//458读头

#define GET_MCU_DOOR_SENSOR_STATUS_CMD		223
#define DOOR_SENOR_IS_OPEN					0

extern PInbioCommData shared_Inbio_Comm_stuff;
extern char g_doorSensorOpen;

extern int InitReaderMem(void);
extern void DoSetReader485();


#endif /* RS_485READER_H_ */
