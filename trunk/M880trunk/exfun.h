/*************************************************
                                           
 ZEM 200                                          
                                                    
 exfun.h define times and voice function                              
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _EXFUN_H_
#define _EXFUN_H_

#include <time.h>
#include "arca.h"

//---------------For ZEM500-----------------

#define BCD2HEX(a) (((a) & 0x0F) + 10*((a)>>4))
#define HEX2BCD(a) (((a)/10)<<4 | ((a)%10))
#define TICKS_PER_SECOND 2000000	//ZEM100 3686400
#define TICKS_PER_MS	2000						//ZEM100 3686

#define VOICE_THANK		0
#define VOICE_INVALID_PWD	1
#define VOICE_INVALID_ADMIN	2
#define VOICE_INVALID_ID	3
#define VOICE_RETRY_FP		4
#define VOICE_REPEAT_ID		5
#define VOICE_NO_LOG_RECSPACE	6
#define VOICE_NO_ADM_SPACE	7
#define VOICE_REPEAT_FP		8
#define VOICE_ALREADY_LOG	9
#define VOICE_NOENROLLED_CARD	10
#define VOICE_DOOR_OPEN		11
#define VOICE_OK		12 //reserved
#define VOICE_RING		13
#define VOICE_CAMERACLICK	14
#define VOICE_REMOVE_FP		15
#define VOICE_MENU_ENTER	16
#define VOICE_LOGIN		17
#define VOICE_LOGOUT		18
#define VOICE_CAPTURE           19

//For face
#define VOICE_FACE_BEGEIN       20
#define VOICE_FACE_START        20
#define VOICE_FACE_FRONT        21
#define VOICE_FACE_SCREEN       22
#define VOICE_FACE_LEFT         23
#define VOICE_FACE_RIGHT        24
#define VOICE_FACE_CAMERA       25
#define VOICE_FACE_END          25
//end face
#define VOICE_NO_LOG_SPACE  29
#define LED_GREEN 0
#define LED_RED 1

//Transfer Data structure
#define BUFSZ	2048
typedef struct _mybuf_{
	int len;
	unsigned char buf[BUFSZ];
}TMyBuf, *PMyBuf;

//Data transfer buffer and CheckSum
PMyBuf bget(int io);
unsigned short in_chksum(unsigned char *p, int len);
void FreeCommuCache(void);
void ExAuxOut(int AuxOnTime, int OpenDoorDelay);

int ClockEnabled;
int ShowMainLCDEnabled;
//add by jazzy 2008.12.18 for battery infomatioin
int BatteryMod;
int BatteryStatus;
int IsDefaultMAC(void);

//int DoSetAlarmOff(int index);	
void ExBeep(int delay);
void ExKeyBeep(void);
void ExOpenRF(void);
int ExPlayVoice(int VoiceIndex);
int ExPlayOtherVoice(int VoiceIndex);
void ExPlayVoiceFrom(int VoiceStart, int VoiceEnd);
void ExLightLED(int LEDIndex, int Light);
BOOL ExPowerOff(int Cmd);   
int ExPlayAlarm(int AlarmIndex, int volume);

void DoAuxOut(int AuxOnTime, int OpenDoorDelay);
void ExSetAuxOutDelay(int AuxOnTime, int OpenDoorDelay, int DoorSensorMode);
int ExAlarm(int Index, int Delay);
int DoAlarmOff(int Index);

BOOL CheckDOOR(char *buffer);
int ExAlarmOff(int Index);
void ExBell(U32 DelayTime);
int TT232Check(char *buffer);
int _TT232Check_(char *buffer);

typedef struct tm TTime;

time_t mktime_1(struct tm *tmbuf);
		
void GetTime(TTime *t);
int SetTime(TTime *t);

TTime *DecodeTime(time_t t, TTime *ts);
time_t EncodeTime(TTime *t);
int TimeDiffSec(TTime t1, TTime t2);
time_t OldEncodeTime(TTime *t);
TTime * OldDecodeTime(time_t t, TTime *ts);
int GetLastDayofmonth(int y,int m);
/*考勤数据自动校对功能，add by yangxiaolong,2011-6-23,start*/
/*
*Function:
*Description:将标准时间字符串格式解析为时间戳格式
*注:标准时间字符串格式必须为:2011-06-23 10:11:13
*Input Prama:strISOTime--标准时间字符串格式
*Return:时间戳
*/
time_t StrTimeToStamp(const char *strISOTime);
/*考勤数据自动校对功能，add by yangxiaolong,2011-6-23,end*/
void GoToSleepStatus(void);
void WakeUpFromSleepStatus(void);

BOOL HIDCheckCard(char *buffer);
void ExCloseRF(void);
void ExOpenWiegand(void);
void ExCloseWiegand(void);
void ExEnableClock(int Enable);
BOOL WiegandSend(U32 deviceID, U32 CardNum, U32 DuressID);
void ExSetExtWGIn(int BitsCount, int PulseWidth, int PulseInterval);
void ExSetTftHIDFun();
void ExSetTftKeyflag(int nKeyStyle);
int ExCommand(char cmd, char *data, int DataLen, int Delay);

char *GetBmpPath(char* filename);     //liming
char* GetPhotoPath(char *filename);	//liming
char* GetCapturePath(char *filename);

char* GetWavPath(char *filename);


enum GPIO_PIN_CTL{
      IO_FUN_GREEN_LED		= 0x07, //output is 0 padr pin7
      IO_FUN_RED_LED 		= 0x06,
      IO_FUN_LOCK 		= 0x05,
      IO_FUN_ALARMSTRIP 	= 0x14,
      IO_FUN_WOD1 		= 0x03, //wiegand output 
      IO_FUN_WOD0		= 0x02, //wiegand output and alarm
      IO_FUN_BUTTON		= 0x31, //input or output(A8)
      IO_FUN_SENSOR 		= 0x10, //input is 1 padr pin0
};

BYTE ExGetIOStatus(void);

void Switch_mode(U32 RS232Mode);

int BT232Check(char *buffer);


#define RCMD_WG_OUT_SET                 181
#define RCMD_WG_OUT_DATA                182
#define CMD_SWITCH_OUT          185
#define EXT_WG_OUT_BUZZER               1       
#define EXT_WG_OUT_RLED         2
#define EXT_WG_OUT_GLED         4
#define DEF_EXT_WG_RLED_DELAY 2 // 4*20ms
#define DEF_EXT_WG_GLED_DELAY 2
#define DEF_EXT_WG_BUZZER_DELAY 2

extern int isNewWGMCU;

#define MCU_CMD_NONE 0
#define MCU_CMD_KEY 1
#define MCU_CMD_VERSION 20
#define MCU_CMD_RTC 22
#define MCU_CMD_LIC 25
#define RCMD_LCD_SIZE   33
#define CMD_LCD_STR             34
#define CMD_LCD_BRIGHT  35

#define MCU_CMD_HID 50

#define MCU_CMD_DOOR    110
#define DOOR_UNKNOWN		0     
#define DOOR_SENSOR_BREAK 	1     //门被意外打开
#define DOOR_BUTTON 		2     //出门开关
#define DOOR_BREAK 		3     //拆机
#define DOOR_SENSOR_OPEN 	4     //正常开门
#define DOOR_SENSOR_CLOSE 	5     //正常关门

#define FORCENONE       0
#define FORCEOPEN       1
#define FORCECLOSE      2

#define MCU_CMD_BATTERY 111
#define BATTERY_None    2
#define BATTERY_Internal 1
#define BATTERY_External 0

//add by jazzy 2008.12.18 for battery 
#define MCU_CMD_BATTERY_MOD 112		//0：外部供电；1：内部电池；2：没有使用电池（保留未用)
#define MCU_CMD_BATTERY_STU 113		//0：充电状态；1：非充电状态；

#define MCU_CMD_ERROR -1
#define MCU_CMD_REPONSE 0xA5
#define MCU_CMD_OPTIONS 120

#define MCU_CMD_KEY_TYPE 241
#define AUX_ON  0xFF
#define AUX_OFF 0

#define LOCKFUN_BASE 1
#define LOCKFUN_ADV  2
#define LOCKFUN_DOORSENSOR 4

//Wiegand 设置参数
#define MAX_FAILD_ID            65535
#define MAX_SITE_CODE           256
#define MAX_PULSEWIDTH          1000
#define MAX_PULSEINTERVAL       10000

#define DEFAULTPULSEWIDTH       100             //默认脉冲宽度
#define DEFAULTPULSEINTERVAL    1000            //默认脉冲间隔

#define PCMD_DOOR_SENSOR        223
#ifdef _TTS_
int TTS_PlayWavFileAsync(int argc, char **command, int delay, int volume);
int TTS_ExPlayAlarm(int AlarmIndex, int volume);
int TTS_ExPlayWav(char *fileName);
#endif

void ExModemPowerByMcu(int power);

//dsl 2012.3.26 
#define CAMERA_DONT_TAKEPHOTO		0
#define CAMERA_TAKEPHOTO		1
#define CAMERA_TAKEPHOTO_AND_SAVE	2
#define CAMERA_TAKEPHOTO_BY_FAILURE	3 //when verify fail, take a photo and save this picture.

//dsl 2012.4.13
void led_redlight_on(int type);
void led_greenlight_on(int type);

int ExAlarmBellOnByMCU(int Delay);
int GetMCUVersion(void);
int led_flash(int RTCTimeValidSign, int OFF);
int ExAlarmBell(int Index, int Delay);
int ExAlarmBellOff(int Index);
BOOL CheckVoiceIdle(void);
unsigned short in_chksum(unsigned char *p, int len);
void ExSetPowerOnTime(int Hour, int Minute);
void ExPowerRestart(void);
BOOL WiegandSendData(U8 *buf);

#endif

