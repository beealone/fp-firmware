/*************************************************
                                           
 ZEM 200                                          
                                                    
 msg.c message process and gather functions                               
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
 
 $Log: msg.c,v $
 Revision 5.20  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.19  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.18  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.17  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.16  2005/08/18 07:16:56  david
 Fixed firmware update flash error

 Revision 5.15  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.14  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk

 Revision 5.13  2005/07/07 08:09:02  david
 Fixed AuthServer&Add remote register

 Revision 5.12  2005/06/29 20:21:43  david
 Add MultiAuthServer Support

 Revision 5.11  2005/06/16 23:27:51  david
 Add AuthServer function

 Revision 5.10  2005/06/10 17:11:01  david
 support tcp connection

 Revision 5.9  2005/06/02 20:11:12  david
 Fixed SMS bugs and Add Power Button Control function

 Revision 5.8  2005/05/20 23:41:04  david
 Add USB support for SMS

 Revision 5.7  2005/05/13 23:19:32  david
 Fixed some minor bugs

 Revision 5.6  2005/04/27 00:15:37  david
 Fixed Some Bugs
l
 Revision 5.5  2005/04/24 11:11:26  david
 Add advanced access control function

 Revision 5.4  2005/04/21 16:46:44  david
 Modify for HID Cardusernumber++;

 Revision 5.3  2005/04/07 17:01:45  david

 Modify to support A&C and 2 row LCD

*************************************************/
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "msg.h"
#include "exfun.h"
#include "options.h"
#include "kb.h"
#include "main.h"
#include "sensor.h"
#include "commu.h"
#include "rs232comm.h"
#include "serial.h"
#include "net.h"
#include "zlg500b.h"
#include "iclsrw.h"
#undef _ZKWEBFUNC
#include "webinterface/webinterface.h"
#ifdef ZEM500
#include "exfunio.h"
#endif
#include "utils.h"
#include "wiegand.h"
#include "rs_485reader.h"

#define IDLE_LOOP_TIME (10*1000*1000)

//#define FPTEST             //auto test parameter
extern int WaitAdminRemainCnt;
extern int C2connect;
static U32 TimerCount=0;
static U32 EnabledMsg=0;

static PMsgProc *MessageProcs=NULL;
static int MsgProcCount=0;
extern int msgkey;
extern char keybuffer[10];
char currentkey;
extern char MCUALARMBuffer[9];

int FPEnrollMode;   //是否在线登记指纹


TSensorBufInfo SensorBufInfo;

int HasInputControl(void)
{
	return MsgProcCount>1;
}

int TestEnabledMsg(int MsgType)
{
	return EnabledMsg & MsgType;
}

void EnableMsgType(int MsgType, int Enabled)
{
	if(Enabled)
	{
		EnabledMsg=EnabledMsg|MsgType;
		switch(MsgType)
		{
		case MSG_TYPE_TIMER: 
			TimerCount=0;
			break;
		}
	}
	else
	{
		EnabledMsg=(EnabledMsg & ~MsgType);
	}			
}

int GetMsg(PMsg msg)
{
	int m;
	while((m=GatherMsgs(msg))==0);
	return m;
}

extern int KeyBufferIndex;
extern TFPCardOP FPData;
extern int CommSessionCount;
extern int gLocalCorrectionImage;	
extern int gEthOpened;
extern serial_driver_t *gSlave232;
extern int gUSB232Connected;
int gFPDirectProc=0;
char *testImageBuffer;
extern int menuflag1;

#define CHECK_MIFARE	0
#define CHECK_ICLASS	1
static int CheckCardInput(U8 *buf, int type)
{
	static int Cardflag = 1;
	int ret=0;

	switch(type)
	{
		case CHECK_MIFARE:
			ret = MFCheckCard(buf);
			break;
		case CHECK_ICLASS:
			ret = iCLSCheckCard(buf);
			break;
	}

	if (ret)
	{
		if (Cardflag)
		{
			Cardflag = 0;
			return ret;
		}
	}
	else
		Cardflag = 1;

	return 0;
}

#ifdef FPTEST
static void copy_testing_fptemplate(void)
{
	if(!testImageBuffer)
	{
		testImageBuffer=MALLOC(gOptions.OImageWidth*gOptions.OImageHeight);
		memcpy(testImageBuffer,gImageBuffer,gOptions.OImageWidth*gOptions.OImageHeight);
	}
	else
	{
		memcpy(gImageBuffer,testImageBuffer,gOptions.OImageWidth*gOptions.OImageHeight);
	}
	return;
}
#endif

extern int pushsdk_is_running(void);

extern int vfwndflag;
extern int menuflag;
extern int workingbool;
extern int powerkey;
int GatherMsgs(PMsg msg)
{        
	U8 buffer[10];
        int status, HIDVerifyType;
	int i;// e;
	unsigned int value=0;
	HIDVerifyType = 0;
	//dsl 2012.3.23
	//if((gOptions.RS232On || gOptions.RS485On)&&(!gOptions.IsSupportModem || !gOptions.IsConnectModem)
	if((gOptions.RS232On || gOptions.RS485On)&&(!gOptions.IsSupportModem || !gOptions.ModemEnable || !(gOptions.ModemModule == 1))
		&& (!(gOptions.Reader485FunOn && gOptions.Reader485On)))
	{
		RS232Check(&ff232);
	}

	if(gOptions.USB232On)
	{
		if(gUSB232Connected)
		{
			RS232Check(&usb232);
		}
		else
		{
			gUSB232Connected = usb232.init(gOptions.RS232BaudRate, V10_8BIT, V10_NONE, 0)==0;
		}
	}

	//check ethernet data
	if(gEthOpened && (vfwndflag == 0) && ((menuflag1==0) || (FPEnrollMode == ENROLLMODE_ONLINE)))
	{ 
		//if (gOptions.IsSupportAuthServer)
		/*后台比对，modify by yangxiaolong,2011-07-25,start*/
	        if ((gOptions.IsSupportAuthServer) && (gOptions.AuthServerCheckMode != AUTH_PUSH))
		{
			GetTime(&gAuthServerTime); //zsliu change
			AuthServerREQ(3,gOptions.AuthServerCheckMode);
			EthAuthServerCheck(gOptions.AuthServerCheckMode);
                }
		EthCommTCPCheck(); 
		EthCommCheck();	
		EthBoradcastCheck();
	}

	//后台验证模式
	status = gOptions.AuthServerEnabled;
	
	if(!gOptions.IsSupportAuthServer || (!CheckAllAuthServerRuning() && (status!=ONLY_NETWORK) ))
	{
		status=ONLY_LOCAL;
	}
		
	/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,start*/
	if ((gOptions.IsSupportAuthServer == 1) && (gOptions.AuthServerCheckMode == AUTH_PUSH))
	{
		if (pushsdk_is_running())
		{
			status = gOptions.AuthServerEnabled;
		}
	}

	/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,end*/

#ifdef ZEM600
	//timer event
	if(EnabledMsg & MSG_TYPE_TIMER)
	{
		TTime t;
		BYTE TimerType;
		
		GetTime(&t);
		if(t.tm_sec!=gCurTime.tm_sec)
		{
			TimerType=(gCurTime.tm_min==t.tm_min?Timer_Second:Timer_Minute);
			if ((TimerType==Timer_Minute)&&(gCurTime.tm_hour!=t.tm_hour))
			{
				TimerType=Timer_Hour;
			}
			ConstructMSG(msg, MSG_TYPE_TIMER,  msg->Param1, TimerType);
			memcpy(&gCurTime, &t, sizeof(TTime));
			if(TimerType==Timer_Hour)
			{
				printf("t=%d:%d:%d, gCurTime=%d:%d:%d\n",t.tm_hour,t.tm_min,t.tm_sec,gCurTime.tm_hour,gCurTime.tm_min,gCurTime.tm_sec);
			}

			CheckSessionTimeOut();
			//if (gOptions.IsSupportAuthServer)
			if ((gOptions.IsSupportAuthServer) && (gOptions.AuthServerCheckMode != AUTH_PUSH) && gEthOpened && (vfwndflag == 0) )
			{
				CheckAuthServerSessionTimeOut(gOptions.AuthServerTimeOut, 0);
			}
			return MSG_TYPE_TIMER;
		}
	}
#endif

	//dsl 2012.3.24 support the extend rs485 reader
	if(gOptions.Reader485On && Is485ReaderMaster() && (NULL != shared_Inbio_Comm_stuff)
		&& (1==shared_Inbio_Comm_stuff->CreateCommCmd))
	{
		shared_Inbio_Comm_stuff->CreateCommCmd = 0;
		if( (EnabledMsg & MSG_TYPE_FINGER) && (shared_Inbio_Comm_stuff->NewsType == SUBCMD_VERIFY_FP))
		{
			printf("inbio finger msg!\n");
			ConstructMSG(msg, MSG_TYPE_FINGER,  shared_Inbio_Comm_stuff->Cmd, (int)shared_Inbio_Comm_stuff);
			return MSG_TYPE_FINGER;
		}
		else if( ((EnabledMsg & MSG_TYPE_HID)||(EnabledMsg & MSG_TYPE_MF)) 
			&& (SUBCMD_VERIFY_RF == shared_Inbio_Comm_stuff->NewsType))
		{
			U32 cardNO = 0;
			memcpy(&cardNO,shared_Inbio_Comm_stuff->SendData,sizeof(cardNO));

			printf("inbio verfy card msg!\n");
			ConstructMSG(msg, MSG_TYPE_HID,  shared_Inbio_Comm_stuff->Cmd, cardNO);
			return MSG_TYPE_HID;
		}
	}

#ifdef HTTPD_SDK
	void sdkApiCheck(int idle);
	for(i=0;i<2;i++) sdkApiCheck(IDLE_LOOP_TIME/1000/1000);
#endif
	
#ifdef ZEM500
	//scan MCU event
	memset(buffer, 0, sizeof(buffer));
	/*add by zxz 2012-12-01*/
	/*解决读取mcu版本时,报警信息被清除问题*/
	if(MCUALARMBuffer[8] == 1 && gOptions.LockFunOn) {
		memcpy(buffer, MCUALARMBuffer, 8);
		memset(MCUALARMBuffer, 0, sizeof(MCUALARMBuffer));
		if(buffer[0]==DOOR_SENSOR_BREAK)		//门磁报警(门被意外打开)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_BREAK);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_SENSOR_OPEN)		//门磁:正常开门
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_OPEN);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_SENSOR_CLOSE)		//门磁：正常关门
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_CLOSE);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_BUTTON)			//出门开关
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Button, buffer[1]);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_BREAK)			//拆机报警
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Alarm_Strip, buffer[1]);
			return MSG_TYPE_CMD;
		}
	}

	i=MCU_CMD_NONE;

	/*
	屏蔽poll，立即读取出MCUCMDBuffer消息
	在_TT232Check_函数已有poll判断
	modify by zxz 2013-03-26
	*/
	//e=ttl232.poll();
	//if(e>0) {
		///*dsl 2012.6.11*/
		//i=_TT232Check_((char*)buffer);
		//拆机报警问题需要使用MCUCMDBuffer chng by zxz 2012-11-03
		i=TT232Check((char *)buffer);
	//}

	if (i==MCU_CMD_KEY && (EnabledMsg&MSG_TYPE_BUTTON))
	{
		buffer[0] = changeKeyValue(buffer[0], 1);
		if(buffer[0]==67 && powerkey == 73 && gOptions.TFTKeyLayout==3
			&& (!vfwndflag && !menuflag && !workingbool))
		{
			buffer[0] = powerkey;
		}
		msgkey=1;
		keybuffer[0] = buffer[0];
		//printf("key value:%d\n", keybuffer[0]);
		currentkey = buffer[0];
		return 0;
	}
	else if ((i==MCU_CMD_HID || i==MCU_CMD_HID_AUTO) && (EnabledMsg & MSG_TYPE_HID) && gOptions.RFCardFunOn && ((value=CheckCardEvent(i, buffer))>0))
	{
		if(value)
		{
			CheckSessionSend(EF_HIDNUM,(char *)buffer,5);
			ConstructMSG(msg, MSG_TYPE_HID, status, value);
			return MSG_TYPE_HID;
		}
	}
	else if (i==MCU_CMD_DOOR && gOptions.LockFunOn)
	{
		if(buffer[0]==DOOR_SENSOR_BREAK)		//门磁报警(门被意外打开)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_BREAK);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_SENSOR_OPEN)		//门磁:正常开门
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_OPEN);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_SENSOR_CLOSE)		//门磁：正常关门
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Sensor, DOOR_SENSOR_CLOSE);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_BUTTON)			//出门开关
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Door_Button, buffer[1]);
			return MSG_TYPE_CMD;
		}
		else if(buffer[0]==DOOR_BREAK)			//拆机报警
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Alarm_Strip, buffer[1]);
			return MSG_TYPE_CMD;
		}
	}

	else if (i==MCU_CMD_INTWGIN && gOptions.ExtWGInFunOn && ((value=CheckCardEvent(i, buffer))>0))
	{
		DBPRINTF("got a internal wiegand in\n");

		if (value)
		{
			CheckSessionSend(EF_HIDNUM,(char *)&value,5);
			ConstructMSG(msg, MSG_TYPE_HID, status, value);
			return MSG_TYPE_HID;
		}
	}

	else if (((i==MCU_CMD_EXTWGIN) || (i==MCU_CMD_EXT_EXTWGIN)) && gOptions.ExtWGInFunOn && ((value=CheckCardEvent(i, buffer))>0))
	{
		DBPRINTF("got a external wiegand in\n");

		if(value)
		{
			if(gOptions.AntiPassbackFunOn==1 || (gOptions.AntiPassbackFunOn==2 && gOptions.AntiPassbackOn))
			{
				status = TYPE_SLAVE_ID;
			}

			HIDVerifyType = status;

			CheckSessionSend(EF_HIDNUM,(char *)&value,5);
			ConstructMSG(msg, MSG_TYPE_HID, HIDVerifyType, value);
			return MSG_TYPE_HID;
		}
	}

	else if ((i==MCU_CMD_OPTIONS) && (buffer[0]==RESET_OPTIONS))
	{
		ConstructMSG(msg, MSG_TYPE_CMD, News_Reset_Options, buffer[1]);
		return MSG_TYPE_CMD;
	}
	
	//add by jazzy for battery infomation
	else if ((i==MCU_CMD_BATTERY_MOD||i==MCU_CMD_BATTERY) && gOptions.BatteryInfo)
	{
		//printf("powwer model...BatteryMod:%d buffer[0]=%d  CMD=%d \n",BatteryMod,buffer[0],i);
		if(buffer[0] != BatteryMod)
		{
			BatteryMod=buffer[0];
			battery_state=0x11;
			ExCommand( MCU_CMD_BATTERY_MOD,(char *)buffer,10,5);//cn 2009-03-02 解决模式状态丢失问题
		}
	 	ExCommand( MCU_CMD_BATTERY_STU,(char *)buffer,10,5); // cn 2009-03-02 解决状态丢失问题
		return 0;
	}
	else if (i==MCU_CMD_BATTERY_STU && gOptions.BatteryInfo)
	{
		//printf("battery status...BatteryStatus:%d  buffer[0]=%d, CMD=%d\n",BatteryStatus,buffer[0],i);
		if(buffer[0] != BatteryStatus)
		{
			BatteryStatus=buffer[0];
			battery_state=0x21;
	 		ExCommand( MCU_CMD_BATTERY_STU,(char *)buffer,10,5); // cn 2009-03-02 解决状态丢失问题
		}
		return 0;
	}
	
#endif
	else if (gMFOpened && (EnabledMsg&MSG_TYPE_MF) && (i=CheckCardInput(buffer, CHECK_MIFARE))) //(i=MFCheckCard(buffer)))	
	{
		memcpy(&value, buffer, 4);
		int i;
		for(i=0;i<4;i++)
			printf("%02x ", buffer[i]);
		DBPRINTF("\nMF Card Id: %u\n",value);

		if (gOptions.MifareAsIDCard==1 || (gOptions.MifareAsIDCard==2 && gMachineState==STA_MENU))
		{
			if (gOptions.RFCardFunOn && (EnabledMsg&MSG_TYPE_HID) && value)
			{
				ConstructMSG(msg, MSG_TYPE_HID, status, value);
				if(msg->Param2)
				{
					CheckSessionSend(EF_HIDNUM, (char*)&(msg->Param2), sizeof(msg->Param2));
				}
				return MSG_TYPE_HID;			
			}
		}
		else
		{
			ConstructMSG(msg, MSG_TYPE_MF, i, value);	//将卡序列号发给主程序,用于判断是否为有效卡
			return MSG_TYPE_MF;
		}
	}
	else if (giCLSRWOpened && (EnabledMsg&MSG_TYPE_ICLASS) && (i=CheckCardInput(buffer, CHECK_ICLASS))) //(i=iCLSCheckCard(buffer)))
	{
		memcpy(&value, buffer, 4);
		if (gOptions.iCLASSAsIDCard==1)
		{
			if (gOptions.RFCardFunOn && (EnabledMsg&MSG_TYPE_HID) && value)
			{
				ConstructMSG(msg, MSG_TYPE_HID, status, value);
				//ConstructMSG(msg, MSG_TYPE_HID, HIDVerifyType, value);
				if(msg->Param2)
				{
					CheckSessionSend(EF_HIDNUM, (char*)&(msg->Param2), sizeof(msg->Param2));
				}
				return MSG_TYPE_HID;
			}
		}
		else
		{
			ConstructMSG(msg, MSG_TYPE_ICLASS, i, value);	//将卡序列号发给主程序,用于判断是否为有效卡
			return MSG_TYPE_ICLASS;
		}
        }
	else if ((EnabledMsg&MSG_TYPE_FINGER) && ((gMachineState!=STA_IDLE) || WaitAdminRemainCnt) && (!gOptions.IsOnlyRFMachine))
	{
		if(gLocalCorrectionImage) status=ONLY_LOCAL;
#ifdef FPTEST
		static int fptestDelay=0;
		if(((gFPDirectProc) ||
			((!gFPDirectProc)&&CaptureSensor(gImageBuffer, SENSOR_CAPTURE_MODE_AUTO, &SensorBufInfo))) 				
			||(gMachineState!=STA_ENROLLING && fptestDelay++>3 && testImageBuffer))
#else
#ifdef IR_SENSOR
                if(((!gOptions.IR_SensorOn)&&gFPDirectProc)||
                        ((!gFPDirectProc)&&(gOptions.IR_SensorOn==0 ||(gOptions.IR_SensorOn&&IRSensor_FPCheck()))
                        &&CaptureSensor(gImageBuffer, status, &SensorBufInfo)))
#else
		if(CaptureSensor(gImageBuffer, SENSOR_CAPTURE_MODE_AUTO, &SensorBufInfo) > 0)/*change 2013-01-14*/
#endif
#endif
		{		
#ifdef FPTEST
			fptestDelay=0;
			copy_testing_fptemplate();
#endif
/*
			//dsl 2012.3.23. When the fp sensor dont work, restart the device.
			if (fpFlag < 0)
			{
				FDB_AddOPLog(0, OP_SENSOR_REBOOT, 0, 0, 0, 0);
				gOptions.IsOnlyRFMachine = 1;
				SaveOptions(&gOptions);
				RebootMachine();
				return 0;
			}
*/
			ConstructMSG(msg, MSG_TYPE_FINGER, status, (int)&SensorBufInfo);
			CheckSessionSend(EF_FINGER, NULL, 0);			
			return MSG_TYPE_FINGER;
		}
	}
	
/*change by zxz 2012-12-11 for zem510 push*/
#ifndef ZEM600
	if (gOptions.IclockSvrFun == 0) {
	        if ((EnabledMsg & MSG_WEBINTERFACE) && ZKWeb(THIS)->Event()) {
	                ConstructMSG(msg, MSG_WEBINTERFACE, 0, 0);
	                return MSG_WEBINTERFACE;
	        }
	}
#else
	if ((EnabledMsg & MSG_WEBINTERFACE) && ZKWeb(THIS)->Event()) {
                ConstructMSG(msg, MSG_WEBINTERFACE, 0, 0);
                return MSG_WEBINTERFACE;
        }
#endif
	if(CommSessionCount==0)
	{
#ifdef ZEM600
		DelayNS(IDLE_LOOP_TIME);
#else
		msleep(10);
#endif
	}
	return 0;
}

int TranslateMsg(int MsgType, PMsg msg)
{
	return 1;
}

U32 SelectNewMsgMask(U32 newmsk)
{
	U32 oldmsk=EnabledMsg;
	EnabledMsg=newmsk;
	return oldmsk;
}

int ProcessMsg(PMsg msg)
{
	int i=MsgProcCount;
	while(i>0)
	{
		i--;
		if (MessageProcs[i](msg)) break;
	}
	return i>=0;
}

int DoMsgProcess(void *Obj, int ExitCommand)
{
	TMsg msg;
	int i;
	msg.Object=Obj;
	do
	{
		msg.Message=0;
		i=GetMsg(&msg);
		if(TranslateMsg(i, &msg))
		{
			while(msg.Message && !((msg.Message==MSG_TYPE_CMD) && (msg.Param1==ExitCommand)))
			{
				if(!ProcessMsg(&msg)) break;
			}
		}		
	}while(!((msg.Message==MSG_TYPE_CMD) && (msg.Param1==ExitCommand)));
	return msg.Param2;
}

int RegMsgProc(PMsgProc MsgProc)
{
	if(MessageProcs==NULL)
		MessageProcs=(PMsgProc*)MALLOC(sizeof(PMsgProc)*200);
	MessageProcs[MsgProcCount++]=MsgProc;
	return MsgProcCount-1;
}

int RegMsgProcBottom(PMsgProc MsgProc)
{
	int i=MsgProcCount-1;
	if(MessageProcs==NULL)
		MessageProcs=(PMsgProc*)MALLOC(sizeof(PMsgProc)*200);
	while(i)
		MessageProcs[i+1]=MessageProcs[i];
	MessageProcs[0]=MsgProc;
	MsgProcCount++;
	return 0;
}

int UnRegMsgProc(int index)
{
	int i=index;
	while(i<MsgProcCount-1)
	{
		MessageProcs[i]=MessageProcs[i+1];
	}
	MsgProcCount--;
	return MsgProcCount;
}
int changeKeyValue(char keyvalue, int Enable)
{
	if((gOptions.KeyType==0) || (keyvalue == 0))
	{
		return keyvalue;	//不需要转换。直接返回。
	}
	//-------------- 处理 3 寸屏部分的键盘问题 ----------------------
	//单片机的默认 default keyboard
	unsigned char KeyValIndex3_01[]=
	{
		'*', '0', '#', 'D',
		'7', '8', '9', 'C',
		'4', '5', '6', 'B',
		'1', '2', '3', 'A'
	};
	
	//单片机3寸 B3 机器的键盘，特殊处理
	unsigned char KeyValIndex3_02[]=
	{
		'*', '8', '5', '1',
		'7', 'C', '#', '2',
		'4', 'D', '9', 'B',
		'0', 'A', '6', '3'
	};
		
	//-------------- 处理 3.5 寸屏部分的键盘问题 ----------------------
	//针对 3.5寸屏的键盘配置，单片机默认键盘配置
	unsigned char KeyValIndex3_5_01[]=
	{
		'*', '7', '4', '1',
		'0', '8', '5', '2',
		'#', '9', '6', '3',
		'D', 'C', 'B', 'A',
		'E', 'F', 'G', 'H', 
		'I', 'J', 'K', 'L',
		'e', 'f', 'g', 'h', 
		'i', 'j', 'k', 'l'
	};
		
	//3.5 寸 ICS01 机器的键盘，特殊处理
	unsigned char KeyValIndex3_5_02[]=
	{
		'3', 'A', '6', '2',
		'1', '4', '5', 'B',
		'*', '0', '#', 'D',
		'7', '8', '9', 'C',
		'F', 'E', 'L', 'e',
		'h', 'i', 'g', 'f',
		'H', 'G', 'I', 'J',
		'K', 'j', 'k', 'l'
	};
	int count=0;
	//-------------- 处理 3 寸屏部分的键盘问题 ----------------------
	if(gOptions.TFTKeyLayout == 3)	//3寸键盘配置
	{
		for(count=0; count<16; count++)
		{
			switch(gOptions.KeyType)
			{
				case 1:	//3寸 B3 机器的键盘，特殊处理
				{
					if(Enable)
					{
						if(KeyValIndex3_01[count] == keyvalue)
						{
							keyvalue = KeyValIndex3_02[count];
							return keyvalue;
						}//if
					}
					else
					{
						if(KeyValIndex3_02[count] == keyvalue)
						{
							keyvalue = KeyValIndex3_01[count];
							return keyvalue;
						}
					}
				}//case 1
				break;
				default:
				{
					printf("Warning: 3.0 The KeyType %d doesn't have!!\n", gOptions.KeyType);
					return keyvalue;	//转换异常。不处理直接返回。
				}//default	
			}//switch				
		}//for			
	}//if
	//-------------- 处理 3.5 寸屏部分的键盘问题 ----------------------
	else if(gOptions.TFTKeyLayout == 0)	//3.5 寸键盘配置
	{
		
		for(count=0; count<32; count++)
		{
			switch(gOptions.KeyType)
			{
				case 1:
				{
					printf("Warning: 3.5 The KeyType %d doesn't have!!\n", gOptions.KeyType);
					return keyvalue;	//转换异常。不处理直接返回。
				}
				case 2:	//3.5 寸 ICS01 机器的键盘，特殊处理
				{
					if(Enable)
					{
						if(KeyValIndex3_5_01[count] == keyvalue)
						{
							keyvalue = KeyValIndex3_5_02[count];
							return keyvalue;
						}//if
					}
					else
					{
						if(KeyValIndex3_5_02[count] == keyvalue)
						{
							keyvalue = KeyValIndex3_5_01[count];
							return keyvalue;
						}
					}
				}//case 1
				break;
				default:
				{
					printf("Warning: 3.5 The KeyType %d doesn't have!!\n", gOptions.KeyType);
					return keyvalue;	//转换异常。不处理直接返回。
				}//default	
			}//switch				
		}//for			
	}//if
	else
	{
		printf("Warning: not 3.0 and 3.5, the TFTKeyLayout Value is %d\n", gOptions.TFTKeyLayout);
		return keyvalue;
	}
	
	return keyvalue;
}

