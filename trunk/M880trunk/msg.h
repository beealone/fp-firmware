/*************************************************
                                           
 ZEM 200                                          
                                                    
 msg.h                               
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _MSG_H_
#define _MSG_H_

#include <stdlib.h>
#include <stdio.h>
#include "arca.h"

#define MSG_TYPE_CMD		1
#define MSG_TYPE_FF232 		(1 << 1)
#define MSG_TYPE_RS485 		(1 << 2)
#define MSG_TYPE_FINGER 	(1 << 3)
#define MSG_TYPE_USB 		(1 << 4)
#define MSG_TYPE_ETHERNET 	(1 << 5)
#define MSG_TYPE_TIMER 		(1 << 6)
#define MSG_TYPE_BUTTON		(1 << 7)
#define MSG_TYPE_MF		(1 << 8)
#define MSG_TYPE_HID		(1 << 9)
#define MSG_TYPE_DOOR		(1 << 10)
#define MSG_TYPE_ICLASS		(1 << 14)

#define News_Exit_Program 		1
#define News_Exit_Input 		3

#define News_VerifiedByPwd		10
#define News_VerifiedByFP		11
#define News_VerifiedByIDCard		12
#define News_VerifiedByPIN		13
#define News_VerifiedByMFCard		14
#define News_VerifiedByIDCardRIS	15
#define News_VerifiedByFPRIS		16
#define News_VerifiedByiCLASSCard  	17 //iCLSRW

#define News_FailedByPwd		20
#define News_FailedByFP			21
#define News_FailedByIDCard		22
#define News_FailedByPIN		23
#define News_FailedByMFCard		24
#define News_FailedByiCLASSCard 	25 //iCLSRW

#define News_Door_Button 	53
#define News_Door_Sensor 	54
#define News_Alarm_Strip 	55
#define News_Reset_Options	56

#define News_CancelInput 	100
#define News_CommitInput 	101
#define News_ErrorInput		102
#define News_NextInput 		103
#define News_PrevInput 		104
#define News_TimeOut		105

#define Timer_None	0
#define Timer_Second	1
#define Timer_Minute	2
#define Timer_Hour	3

typedef struct _TAG_MSG
{
	void *Object;
	int Message;
	int Param1,Param2;
} TMsg, *PMsg;

//a message process handle will process a interested event and 
//return 1, if the msg isn't interested, return is 0
typedef int (*PMsgProc)(PMsg msg);

void EnableMsgType(int MsgType, int Enabled);

//wait a event and get it, return the message
int GetMsg(PMsg msg);
//test if have a event, return the message or 0
int GatherMsgs(PMsg msg);
//call all registered message handle, if there is no handle
//process it return 0, else return 1. Only the top handle can 
//process the interested messages.
int ProcessMsg(PMsg msg);
//translate a message
int TranslateMsg(int MsgType, PMsg msg);
//waiting and process events util ExitCommand occurred
int DoMsgProcess(void *Obj, int ExitCommand);

U32 SelectNewMsgMask(U32 newmsk);

//Register a custom message process function
int RegMsgProc(PMsgProc MsgProc);
int RegMsgProcBottom(PMsgProc MsgProc);
int UnRegMsgProc(int index);
int TestEnabledMsg(int MsgType);
int HasInputControl(void);

#define ConstructMSG(msg,MSG,P1,P2) \
do\
{\
	msg->Message=MSG;\
	msg->Param1=P1;\
	msg->Param2=P2;\
}while(0)

extern int gMFOpened;
extern int giCLSRWOpened; //iCLSRW
//对特殊键盘做的映射表转换。这个需要特殊处理
//int changeKeyValue(char keyvalue);
int changeKeyValue(char keyvalue, int Enable);
#endif
