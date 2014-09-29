/*************************************************

  ZEM 200                                          

  exfun.c time and voice and access control function                              

  Copyright (C) 2003-2005, ZKSoftware Inc.      		

  $Log: exfun.c,v $
  Revision 5.14  2006/03/04 17:30:09  david
  Add multi-language function

  Revision 5.13  2005/12/22 08:54:23  david
  Add workcode and PIN2 support

  Revision 5.12  2005/08/15 13:00:22  david
  Fixed some Minor Bugs

  Revision 5.11  2005/08/13 13:26:14  david
  Fixed some minor bugs and Modify schedule bell

  Revision 5.10  2005/08/07 08:13:15  david
  Modfiy Red&Green LED and Beep

  Revision 5.9  2005/08/04 15:42:53  david
  Add Wiegand 26 Output&Fixed some minor bug

  Revision 5.8  2005/08/02 16:07:51  david
  Add Mifare function&Duress function

  Revision 5.7  2005/07/14 16:59:53  david
  Add update firmware by SDK and U-disk

  Revision 5.6  2005/06/10 17:11:01  david
  support tcp connection

  Revision 5.5  2005/05/13 23:19:32  david
  Fixed some minor bugs

  Revision 5.4  2005/04/27 00:15:37  david
  Fixed Some Bugs

  Revision 5.3  2005/04/24 11:11:26  david
  Add advanced access control function

 *************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

#include "arca.h"
#include "exfun.h"
#include "options.h"
#include "msg.h"
#include "main.h"
#include "rtc.h"
#include "sensor.h"
#include "usb_helper.h"
#include "wiegand.h"
#include "serial.h"
#include "exfunio.h"
#include "utils.h"
#include "flashdb.h"
#include "wav.h"
#include "gpio.h"

#include <errno.h>
#ifdef _TTS_
#include "tts/tts.h"
#endif

#define DUMMY_PULSE_WIDTH       0x10
#define DUMMY_PULSE_INTERVAL    0x20
#define DUMMY_OUTPUT_TYPE       0x30
#define WIEGAND_PULSE_WIDTH	0x40
#define WIEGAND_PULSE_INTERVAL	0x50
#define WIEGAND_FORMAT		0x60

extern int errno;
static TMyBuf *buff_in=NULL, *buff_out1=NULL, *buff_out2=NULL;
static char WavFilePath[80]="NONE";
static int MCUVersion=0;
static int fd_wiegand=-1;
static int fd_dummy=-1;

int getWavFileDelay(char *fileName);
void GPIO_I2S_Mute(BOOL Switch, int delay);
//计算指定月份的天数
static int IsLeapYear (int year)
{
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) return 1;
	else return 0;
}

int GetMonDay (int year, int month)
{
	int mon_len;

	if (month < 1 || month > 12){
		return -1;
	}

	if ((month <= 7 && month % 2 == 1) || (month >= 8 && month % 2 == 0)){
		mon_len = 31;
	}
	else if (month == 2){
		if (IsLeapYear (year)){
			mon_len = 29;
		}
		else {
			mon_len = 28;
		}
	}
	else
	{
		mon_len = 30;
	}
	return mon_len;
}



int ExPlayVoice(int VoiceIndex)
{
#ifdef _TTS_
	char *pString=NULL;

	TTS_Wait();
	TTS_Stop();
	TTS_SetVol(AdjTTSVol(gOptions.AudioVol));
	/*获取VoiceIndex 相对应的字符串*/
	pString=GetTTSID(VoiceIndex);
	/*根据字符串播报语音*/
	if(pString)
		TTS_Say(pString);

	return 1;
#else	
	char buffer[128];

	if (strcmp(WavFilePath, "NONE")==0)
		if (!LoadStr("WAVFILEPATH", WavFilePath)) memset(WavFilePath, 0, 80);  

	//2007.07.23 解决英文状态下面播放中文语音问题
	gOptions.Language = tftnewlng;

	sprintf(buffer, "%s%c_%d.wav", WavFilePath, gOptions.Language, VoiceIndex);

	return PlayWavFileAsync(gOptions.AudioVol, buffer);

#endif
	return 0;
}

static char AlarmFilePath[80]="NONE";
int ExPlayAlarm(int AlarmIndex, int volume)
{
	char buffer[128];
	if (strcmp(AlarmFilePath, "NONE")==0)
	{
		if (!LoadStr("WAVFILEPATH", AlarmFilePath))
			memset(AlarmFilePath, 0, 80);
	}
	sprintf(buffer, "%sbell%02d.wav", AlarmFilePath, AlarmIndex+1);

	return PlayWavFileAsync(volume,  buffer);
}

//获取图片文件完整路径
char BmpFilePath[80];
char* GetBmpPath(char *filename)
{
	static char pathbuf[80];
	if(!BmpFilePath[0])
	{
		if(!LoadStr("BMPFILEPATH", BmpFilePath))
			memset(BmpFilePath, 0, 80);
	}
	sprintf(pathbuf,"%s%s",BmpFilePath,filename);
	return (char*)pathbuf;
}

//获取用户照片文件完整路径
char PhotoFilePath[80];
char* GetPhotoPath(char *filename)
{
	static char pathbuf1[80];
	if(!PhotoFilePath[0])
	{
		if(!LoadStr("PHOTOFILEPATH", PhotoFilePath))
			memset(PhotoFilePath, 0, 80);
	}
	sprintf(pathbuf1,"%s%s",PhotoFilePath, filename);
	return (char*)pathbuf1;
}

char CapturePhotoPath[80]={0};
char* GetCapturePath(char *filename)
{
	static char pathbuf2[80]={0};
	if(!CapturePhotoPath[0])
	{
		if(!LoadStr("CAPTUREPHOTOPATH", CapturePhotoPath))
			memset(CapturePhotoPath, 0, sizeof(CapturePhotoPath));
	}
	sprintf(pathbuf2, "%s%s", CapturePhotoPath, filename);
	return (char*)pathbuf2;
}

int ExPlayOtherVoice(int VoiceIndex)
{
	char buffer[128];

	memset(buffer,0,sizeof(buffer));
	if (strcmp(WavFilePath, "NONE")==0)
		if (!LoadStr("WAVFILEPATH", WavFilePath)) memset(WavFilePath, 0, sizeof(WavFilePath));
	if (VoiceIndex==VOICE_LOGIN)
		sprintf(buffer, "%slogin.wav", WavFilePath);
	else if (VoiceIndex==VOICE_LOGOUT)
		sprintf(buffer, "%slogout.wav", WavFilePath);
	else if (VoiceIndex==VOICE_MENU_ENTER)
		sprintf(buffer, "%smenu.wav", WavFilePath);
	else if (VoiceIndex==VOICE_CAPTURE)
		sprintf(buffer, "%scapture.wav", WavFilePath);
	else
		sprintf(buffer, "%sbeep.wav", WavFilePath);

	return PlayWavFileAsync(gOptions.AudioVol, buffer);
}
void ExPlayVoiceFrom(int VoiceStart, int VoiceEnd)
{
	int i;

	for(i=VoiceStart; i<=VoiceEnd; i++) ExPlayVoice(i);
}

void ExBeep(int delay)
{
#ifdef _TTS_
	TTS_PlayWav(GetWavPath("beep.wav"));
#else	
	char buffer[128];

	if (strcmp(WavFilePath, "NONE")==0) {
		if (!LoadStr("WAVFILEPATH", WavFilePath)) {
			memset(WavFilePath, 0, 80);
		}
	}
	sprintf(buffer, "%sbeep.wav", WavFilePath);

	PlayWavFileAsync(gOptions.AudioVol, buffer);
#endif
}
void ExKeyBeep(void)
{
	char buffer[128];

	if (strcmp(WavFilePath, "NONE")==0)
		if (!LoadStr("WAVFILEPATH", WavFilePath)) memset(WavFilePath, 0, 80);
	sprintf(buffer, "%skey.wav", WavFilePath);

	PlayWavFileAsync(gOptions.AudioVol, buffer);
}

void ExSetExtWGTip(int red_led_delay, int green_led_delay, int buzzer_delay)
{
	U8 buf[2];

	if(red_led_delay)
	{
		buf[0] = EXT_WG_OUT_RLED;
		buf[1] = red_led_delay;
		ExCommand(CMD_SWITCH_OUT,(char *)buf,2,10);
	}

	if(green_led_delay)
	{
		buf[0] = EXT_WG_OUT_GLED;
		buf[1] = green_led_delay;
		ExCommand(CMD_SWITCH_OUT,(char *)buf,2,10);
	}

	if(buzzer_delay)
	{
		buf[0] = EXT_WG_OUT_BUZZER;
		buf[1] = buzzer_delay;
		ExCommand(CMD_SWITCH_OUT,(char *)buf,2,10);		
	}
}


extern struct vdIn videoIn;
void ExLightLED(int LEDIndex, int Light)
{
	if (LEDIndex==LED_RED)
	{    
		GPIOSetLevel(IO_FUN_RED_LED, Light);
#ifdef FACE
		if(gOptions.FaceFunOn)
			SetCameraLed(&videoIn, 0, Light);
#endif
	}
	else
	{
		GPIOSetLevel(IO_FUN_GREEN_LED, Light);
#ifdef FACE
		if(gOptions.FaceFunOn)
			SetCameraLed(&videoIn, 1, Light);
#endif				
	}
}

BOOL ExPowerOff(int Cmd)
{
	int Suspend=Cmd;
	char buf[3]={0x5a,0xa5,0};
	buf[2]=Suspend;
	ExCommand(30, buf, 3, 0);
	buf[2]=Suspend;
	ExCommand(30, buf, 3, 0);
	buf[2]=Suspend;
	ExCommand(30, buf, 3, 0);
	return TRUE;
}

void GetTime(TTime *t)
{
	time_t tt;

	tt = time(NULL);
	memcpy(t, localtime(&tt), sizeof(TTime));    	 
}

int SetTime(TTime *t)
{	
	int ret1;
	int ret2;
	time_t tt;

	//fix tm_wday tm_yday
	tt = EncodeTime(t);

	//setup RTC CLOCK 
	ret1= SetRTCClock(t);
	DelayUS(100*1000);

	//synochronize system time from RTC
	ret2 = ReadRTCClockToSyncSys(t);
	if(FALSE == ret1)
		return ret1;
	else
		return ret2;
}

time_t EncodeTime(TTime *t)
{	
	time_t tt;

	//夏令时 = 没有信息
	t->tm_isdst = -1; 
	tt = mktime_1(t);
	return tt;
}

time_t OldEncodeTime(TTime *t)
{
	time_t tt;

	tt=((t->tm_year-100)*12*31+((t->tm_mon)*31)+t->tm_mday-1)*(24*60*60)+
		(t->tm_hour*60+t->tm_min)*60+t->tm_sec;
	return tt;
}

TTime * OldDecodeTime(time_t t, TTime *ts)
{
	ts->tm_sec=t % 60;
	t/=60;
	ts->tm_min=t % 60;
	t/=60;
	ts->tm_hour=t % 24;
	t/=24;
	ts->tm_mday=t % 31+1;
	t/=31;
	ts->tm_mon=t % 12;
	t/=12;
	ts->tm_year=t+100;
	return ts;
}

TTime * DecodeTime(time_t t, TTime *ts)
{
	memcpy(ts, localtime(&t), sizeof(TTime));
	return ts;
}

int TimeDiffSec(TTime t1, TTime t2)
{
	return (EncodeTime(&t1) - EncodeTime(&t2)); 
}

int GetLastDayofmonth(int y,int m)
{
	int f,n;
	n=0;
	f=(y%4==0&&y%100!=0)||(y%400==0);
	if (m>7 && m%2==0)
		n=31;
	else if (m>7)
		n=30;
	else if (m%2==0 && m!=2)
		n=30;
	else if (m!=2)
		n=31;
	else
		n=28+f;
	return n;

}

void GoToSleepStatus(void)
{
	FreeSensor();
	GPIO_HY7131_Power(FALSE);
	GPIO_LCD_USB0_Power(FALSE);
	//disabled mute
	GPIO_AC97_Mute(TRUE);
	//whether display clock ":" or not
	ClockEnabled = FALSE;		
	//whether display main windows or not
	ShowMainLCDEnabled = FALSE;
}

/*
 * Very simple buffer management.
 *
 * io = 0 : request for input buffer
 * io = 1 : request for output buffer
 *
 * set buffer length to 0 to free the buffer.
 *
 */
PMyBuf bget(int io)
{
	if(buff_in==NULL)
	{
		buff_out2=(TMyBuf*)MALLOC(sizeof(TMyBuf));
		buff_out2->len=0;
		buff_out1=(TMyBuf*)MALLOC(sizeof(TMyBuf));
		buff_out1->len=0;
		buff_in=(TMyBuf*)MALLOC(sizeof(TMyBuf));
		buff_in->len=0;
	}
	if( io == 1) {
		if(buff_out1->len == 0) return buff_out1;
		else return buff_out2;
	}else return buff_in;

	//      DBPRINTF("%s:can't get buffer\n",__FUNCTION__);
	return 0;
}

void FreeCommuCache(void)
{
	if(buff_in != NULL) FREE(buff_in);
	if(buff_out1 != NULL) FREE(buff_out1);
	if(buff_out2 != NULL) FREE(buff_out2);
}

unsigned short in_chksum(unsigned char *p, int len)
{
	unsigned long sum=0;

	while(len > 1) {
		sum += *((unsigned short*)p); p+=2;
		if( sum & 0x80000000 )
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}

	if(len)
		sum += (unsigned short) *(unsigned char*) p;

	while(sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum;
}

void ExOpenRF(void)
{
	char buf[4]={1,1,1,1};
	ExCommand(RCMD_RF_ENABLED, buf, 1, 5);
	ExCommand(RCMD_RF_ENABLED, buf, 1, 5);
}

void ExCloseRF(void)
{
	if (fd_wiegand>=0) close(fd_wiegand);
}

BOOL iClassReadSN(int *in_data)
{
	const int MaxChars=32;
	int rdindex = 0;
	U8 data[MaxChars];
	int rdcnt = 200;

	if(st232.poll()) //data arrived
	{
		//printf("data arrived\n");
		while(rdcnt&&rdindex<MaxChars)
		{
			if(st232.poll())
			{
				data[rdindex++] = st232.read();
				// printf("iclass %d\n", data[rdindex-1]);
			}
			else
			{
				DelayUS(1000);
				rdcnt--;
			}
		}
		st232.flush_input();
		if(rdindex>=16)
		{
			*in_data=(int)((data[rdindex-2]*256+data[rdindex-1])/2);
			return TRUE;
		}
	}
	return FALSE;
}


BOOL HIDCheckCard(char *buffer)
{	
	int in_data=0;
	extern int gHIDiClassOpened;
	if(gHIDiClassOpened&&iClassReadSN(&in_data))
	{
		memset(buffer, 0, 5);
		memcpy(buffer, &in_data, 4);
		return TRUE;
	}
	else if ((fd_wiegand>=0)&&(read(fd_wiegand, &in_data, 4)==4))
	{
		memset(buffer, 0, 5);
		memcpy(buffer, &in_data, 4);
		//DBPRINTF("wiegand = %d \n", in_data);	
		return TRUE;
	}
	else
		return FALSE; 
}

int isNewWGMCU=-1;

int ExSetIntWGIn(int BitsCount, int PulseWidth, int PulseInterval);
void ExOpenWiegand(void)
{
	wiegandDefineInit(WIEGAND_ALL);
	// Wiegand Settings
	TWiegandDefine *wiegandOut = wiegandGetDefine(WIEGAND_OUT);
	TWiegandDefine *IntwiegandIn = wiegandGetDefine(WIEGAND_IN_INTERNAL);
	TWiegandDefine *ExtwiegandIn = wiegandGetDefine(WIEGAND_IN_EXTERNAL);

	// Set wiegand output parameters.
	if (fd_dummy>=0) {
		close(fd_dummy);
	}
	fd_dummy = -1;
	fd_dummy = open("/dev/dummy", O_WRONLY);
	if (fd_dummy==-1) {
		DBPRINTF("/dev/dummy/ open failed\n");
	} else {		
		ioctl(fd_dummy, DUMMY_PULSE_WIDTH, &wiegandOut->pulseWidth);
		ioctl(fd_dummy, DUMMY_PULSE_INTERVAL, &wiegandOut->pulseInterval);
		ioctl(fd_dummy, DUMMY_OUTPUT_TYPE, &gWGOutputType);
	}

	// Set internal wiegand in parameters.
	int mcuver = GetMCUVersion();
	if ((mcuver >= 13) && (mcuver < 20) && IntwiegandIn->format) { 
		/*When the mcu version more than 20, mcu support auto read the card bits.*/
		ExSetIntWGIn(IntwiegandIn->format->count, IntwiegandIn->pulseWidth, IntwiegandIn->pulseInterval);
	}

	if (gOptions.ExtWGInFunOn && ExtwiegandIn->format) {
		ExSetExtWGIn(ExtwiegandIn->format->count, ExtwiegandIn->pulseWidth, ExtwiegandIn->pulseInterval);
	}
}

int str2wg(char* buf,char* wg_value)
{
	int i,j;
	char wg_buf[10];
	char *p_wg;

	i=0;	
	if(buf!=NULL)
	{
		memset(wg_buf,0,10);

		p_wg = wg_buf;

		for(i=0,j=0; buf[i] !='\0';i++)
		{

			if( (buf[i] != '1') && (buf[i]!='0'))
			{
				DBPRINTF(">>>mcdebug:\tinvalid wiegand str value\n");
				break;
			}

			if(j==8) 
			{
				j=0;
				p_wg++;
			}

			if(buf[i] == '1')
			{
				*p_wg |= (1<<j) & 0xff; 
			}
			else//f(buf[i] == '0')
			{
				*p_wg &= (~(1<<j)) & 0xff;
			}
			j++;
		}

		if(wg_value != NULL)
		{
			memcpy(wg_value,wg_buf,p_wg-wg_buf+1);
		}
	}

	return i;
}


void ExCloseWiegand(void)
{
	FreeWiegandFmt();
	if (fd_dummy>=0) {
		close(fd_dummy);
	}
}

BOOL WiegandSendData(U8 *buf)
{
	if (fd_dummy>=0) {
		write(fd_dummy, buf, strlen((char *)buf));
		return TRUE;
	} else {
		printf("Error of Wiegand Output for dummy\n");
	}
	return FALSE;
}

BOOL WiegandSend(U32 deviceID, U32 CardNum, U32 DuressID)
{
	U8 data[MAX_WG_BITS];
	if (wiegandEncode(WIEGAND_OUT, (char *)data, gWGOEMCode, gWGFacilityCode, deviceID, CardNum)==ERROR_OK) {
		return WiegandSendData(data);
	} else {
		DBPRINTF("Error to encode wiegand output data\n");
	}
	return FALSE;
}

int GetMCUVersion(void)
{
	int i=20;
	char buf[20]={0};
	while(i--)
	{
		buf[0]=0;
		ExCommand(MCU_CMD_VERSION, NULL, 0, 2);
		if (BT232Wait(RCMD_VERSION, 100, buf)) return buf[0];
	}
	return 0;
}

int ExSetIntWGIn(int BitsCount, int PulseWidth, int PulseInterval)
{
	char buf[5];
	DBPRINTF("IntBitsCount=%d,IntPulseWidth=%d,IntPulseInterval=%d\n",BitsCount,PulseWidth,PulseInterval);
	buf[0]=BitsCount&0xFF;
	*(unsigned short*)(buf+1)=PulseWidth&0xffff;
	*(unsigned short*)(buf+3)=PulseInterval&0xffff;
#if 0
	if (0) {
		printf("MCU Version Number:%d\n", GetMCUVersion());
		return (GetMCUVersion()>=13);
	} else {
		return 0;
	}
#endif
	ExCommand(RCMD_INTWG_OPTIONS, buf, 5, 20);
	return 1;
}

extern int gAuxOutDelay;
void ExAuxOut(int AuxOnTime, int OpenDoorDelay)
{
	U8 AuxTime[2];
	if(gOptions.LockFunOn&8)
	{
		if(AuxOnTime>0 && OpenDoorDelay>0)
			gAuxOutDelay=(int)((AuxOnTime*20)/1000)+2;
	}
	else
		gAuxOutDelay=(int)((AuxOnTime*20)/1000)+2;

	if(AuxOnTime==0xFF) {
		AuxTime[0] = AuxOnTime;
	} else {
		if(MCUVersion != 29) {
			//AuxTime[0] = AuxOnTime*15;
			AuxTime[0] = AuxOnTime*25;//changed by zhongjr 2013-03-09,25*40ms=1s,单片机以40ms为单位
		} else {
			/*change by zxz 2013-01-15*/
			AuxTime[0] = AuxOnTime;
		}
	}

	AuxTime[1]=OpenDoorDelay;
	//   printf("AuxTime[0]:%d,AuxTime[1]:%d\n",AuxTime[0],AuxTime[1]);
	ExCommand(4, (char*)AuxTime, 2, 10);
}

void DoAuxOut(int AuxOnTime, int OpenDoorDelay)
{
	//printf("______%s-%d, AuxOnTime = %d, OpenDoorDelay = %d\n", __FILE__, __LINE__, AuxOnTime, OpenDoorDelay);
	ExAuxOut(AuxOnTime, OpenDoorDelay);
}

void ExSetAuxOutDelay(int AuxOnTime, int OpenDoorDelay, int DoorSensorMode)
{
	U8 AuxTime[3];
	printf("____%s-%d,AuxOnTime=%d,OpenDoorDelay=%d,DoorSensorMode=%d\n", __FILE__, __LINE__, AuxOnTime,OpenDoorDelay,DoorSensorMode);
	AuxOnTime-=2;
	if(AuxOnTime<=0) AuxOnTime=1;

	AuxTime[0]=AuxOnTime*50;
	AuxTime[1]=OpenDoorDelay;
	AuxTime[2]=DoorSensorMode;
	ExCommand(RMCU_CMD_AUXOUT_LEN, (char*)AuxTime, 3, 10);
}

extern int gAlarmDelay;
int ExAlarmOnByMCU(int Delay)
{
	char buf[4]={0,0,0,0};
	if(Delay>254) buf[0]=255;
	return ExCommand(RMCU_CMD_ALARM, buf, 1, 5);
}

int ExAlarm(int Index, int Delay)
{
	if(Index==0)
	{
		ExAlarmOnByMCU(Delay);
		gAlarmDelay=Delay;
	}
	return TRUE;
}

//Bell
int ExAlarmBellOnByMCU(int Delay)
{
	printf("send command RCMD_OUT_BELL\n");
	char buf[4]={0,0,0,0};
	if(Delay>254) {
		/*the relay still close.(or the bell will still play a sound)*/
		buf[0]=255;
	} else {
		/*Play a sound for x seconds.
		 * for example, the Delay value is 200, so the mcu will play a sound for 8 seconds
		 * the calculation method is 200*40/1000=8, the value 40 is inner value of mcu*/
		buf[0] = Delay;
	}
	if(!gOptions.LockFunOn)
	{
		ExCommand(DOOR_SENSOR_OPEN, buf, 1, 5);
	}
	ExCommand(RMCU_CMD_ALARM, buf, 1, 5);
	return ExCommand(RCMD_OUT_BELL, buf, 1, 5);
}

int ExAlarmBell(int Index, int Delay)
{
	if(Index==0)
	{
		ExAlarmBellOnByMCU(Delay);
		gAlarmDelay=Delay;
	}
	return TRUE;
}

int ExAlarmBellOffByMCU(void)
{
	char buf[4]={0,0,0,0};
	if(!gOptions.LockFunOn)
	{
		ExCommand(DOOR_SENSOR_OPEN, buf, 1, 5);
	}
	ExCommand(RMCU_CMD_ALARM, buf, 1, 5);
	return ExCommand(RCMD_OUT_BELL, buf, 1, 5);
}

int ExAlarmBellOff(int Index)
{
	//Output a LOW pulse
	ExAlarmBellOffByMCU();
	return TRUE;
}

//end
int ExAlarmOffByMCU(void)
{
	char buf[4]={0,0,0,0};
	return ExCommand(RMCU_CMD_ALARM, buf, 1, 5);
}

int ExAlarmOff(int Index)
{
	//Output a LOW pulse
	ExAlarmOffByMCU();
	return TRUE;
}         

int DoAlarmOff(int Index)
{
	int ret=0;
	ret= ExAlarmOff(Index);
	return ret;
}

BYTE ExCheckStrip(void)
{
	int i=200,c=0;
	while(--i)
	{
		if(!GPIOGetLevel(IO_FUN_SENSOR))
		{	
			if(++c>20) 
			{
				return (gOptions.DoorSensorMode?DOOR_SENSOR_OPEN:DOOR_SENSOR_CLOSE);
			}
		}
		DelayUS(5);
	}
	return (gOptions.DoorSensorMode?DOOR_SENSOR_CLOSE:DOOR_SENSOR_OPEN);
}
int GetDoor_Sensor(void)
{
	int i=20;
	char buf[20];
	memset(buf,0,20);
	while(i--)
	{
		buf[0]=0;
		ExCommand(PCMD_DOOR_SENSOR, NULL, 0, 2);
		if(BT232Wait(PCMD_DOOR_SENSOR, 100, buf))
		{
			return buf[0];
		}
	}
	return 2;
}

BYTE ExGetIOStatus(void)
{
	/*
	   BYTE ret=0;
	//sensor status
	ret|=ExCheckStrip()-DOOR_SENSOR_OPEN;

	printf("dddd  exgetio status ret=%d   ",ret);
	//button
	ret|=ExCheckGPI(IO_FUN_BUTTON)<<1;
	printf("%d   ",ret);
	//alarm
	ret|=ExCheckGPI(IO_FUN_WOD0)<<2;
	printf("%d   ",ret);
	//alarm strip
	ret|=ExCheckGPI(IO_FUN_ALARMSTRIP)<<3;
	printf("%d   ",ret);
	//Lock
	ret|=ExCheckGPI(IO_FUN_LOCK)<<4;
	printf("%d   \n",ret);
	return ret;
	*/
	int ret=0;
	int DoorSensor=0;//checkDoorState();
	int mcuVersion=GetMCUVersion();
	printf("\n================ mcuVersion=%d\n\n",mcuVersion);
	if(mcuVersion < 11 )
		return -1;
	DoorSensor=GetDoor_Sensor();
	if (DoorSensor==2)
	{
		DoorSensor=0;
	}
	DoorSensor = !DoorSensor;
	DoorSensor=DoorSensor&0x1;
	ret=DoorSensor;
	return ret;
}

extern int gDoorSensorDelay;
extern int gAlarmStrip;

BOOL CheckDOOR(char *buffer)
{
	static BYTE DoorSensorStatus=DOOR_UNKNOWN;
	static BYTE DoorOpenSign=DOOR_UNKNOWN;
	BYTE status;	

	if(!gOptions.IsSupportC2)
	{
		if((gOptions.LockFunOn&LOCKFUN_DOORSENSOR)&&(gOptions.DoorSensorMode<2))
		{
			//Door sensor
			status=ExCheckStrip();
			if (status!=DoorSensorStatus)
			{
				DoorSensorStatus=status;
				if(status==DOOR_SENSOR_OPEN)
				{
					if(gAuxOutDelay)
					{
						buffer[0]=DOOR_SENSOR_OPEN;
						gDoorSensorDelay=gOptions.OpenDoorDelay;
						DoorOpenSign=DOOR_SENSOR_OPEN;							
					}
					else	
					{
						buffer[0]=DOOR_SENSOR_BREAK;
						DoorOpenSign=DOOR_SENSOR_BREAK;
					}
					return TRUE;
				}
				if(status==DOOR_SENSOR_CLOSE)
				{
					gDoorSensorDelay=0;
					buffer[0]=DOOR_SENSOR_CLOSE;
					DoorOpenSign=DOOR_SENSOR_CLOSE;
					return TRUE;			
				}
			}
		}
		else
		{
			gDoorSensorDelay=0;
			DoorSensorStatus=DOOR_UNKNOWN;
			DoorOpenSign=DOOR_UNKNOWN;
		}
		//Door button
		if((gAuxOutDelay==0)&&ExCheckGPI(IO_FUN_BUTTON))
		{
			buffer[0]=DOOR_BUTTON;
			buffer[1]=0;
			return TRUE;	
		}
	}
	else
	{
		//nothing to do
	}
	//Door strip
	if((gAlarmStrip==0)&&ExCheckGPI(IO_FUN_ALARMSTRIP))
	{
		buffer[0]=DOOR_BREAK;
		buffer[1]=0;
		return TRUE;		
	}
	return FALSE;
}

void Switch_mode(U32 RS232Mode)
{
	//Output a LOW pulse
	//Switch RJ45/RS232
	GPIOSetLevel(IO_FUN_BUTTON, RS232Mode); 
}

#if 1 

#define MCUBUFFERCNT    8
static char MCUCMDBuffer[MCUBUFFERCNT][8];
static int OutCmdBuf=0;
static int InCmdBuf=0;
/*add by zxz 2012-12-01*/
/*解决拆机时机器开机后无拆机报警提示问题*/
char MCUALARMBuffer[9] = {0};
int _BT232Check_(char *buffer)
{
	int i,cc;
	U32 cmd=0, ch,sum;
	if(bt232.poll()==0) return MCU_CMD_NONE;
	i=0;sum=0;
	while(1)
	{
		cc=0;
		while(bt232.poll()==0)
		{
			DelayUS(50);
			cc+=1;
			if(cc>=2000) 
			{
				printf("mcu time out\n");
				return MCU_CMD_ERROR;
			}
		}
		ch=bt232.read();
		switch(i){
			case 0:
				if(ch==0xA5) return MCU_CMD_REPONSE;
			case 1:
				if(ch==0x53) i++; else i=0;
				break;
			case 2:
				cmd=ch;
				i++;
				break;
			case 3:
				if(ch+cmd==0xFF)
					i++;
				else
					i=0;
				break;
			case 11:
			case 12:
				if((cmd!=216 && cmd!=215) || i==12)
				{
					if((sum & 0xFF)==ch)
					{
						if(cmd==20)
							MCUVersion=buffer[0];
						return cmd;
					}
					else
						i=0;
					//DBPRINTF("sum%d&0xff==ch%d,i=%d\n",sum,ch,i);
					break;
				}

			default:
				buffer[i-4]=ch;
				sum+=ch;
				i++;
		}
		if(i==0) break;
	}
	return MCU_CMD_ERROR;
}

int BT232Check(char *buffer)
{
	int i;
	if(OutCmdBuf!=InCmdBuf)
	{
		memcpy(buffer, MCUCMDBuffer[OutCmdBuf], 8);
		i=OutCmdBuf;
		OutCmdBuf++;
		if(OutCmdBuf>=MCUBUFFERCNT) OutCmdBuf=0;
		return MCUCMDBuffer[i][7];
	}
	else
		return _BT232Check_(buffer);
}

int _TT232Check_(char *buffer)
{
	int i,cc;
	U32 cmd=0, ch,sum;
	if(ttl232.poll()==0) return MCU_CMD_NONE;
	i=0;sum=0;
	while(1)
	{
		cc=0;
		while(ttl232.poll()==0)
		{
			DelayUS(50);
			cc+=1;
			if(cc>=2000) 
			{
				printf("mcu time out\n");
				return MCU_CMD_ERROR;
			}
		}
		ch=ttl232.read();

		switch(i){
			case 0:
				if(ch==0xA5) 
				{
				    return MCU_CMD_REPONSE;
				}
			case 1:
				if(ch==0x53) i++; else i=0;
				break;
			case 2:
				cmd=ch;
				i++;
				break;
			case 3:
				if(ch+cmd==0xFF)
					i++;
				else
					i=0;
				break;
			case 11:
			case 12:
				if((cmd!=216 && cmd!=215) || i==12)
				{
					if((sum & 0xFF)==ch)
					{
						if(cmd==20)
							MCUVersion=buffer[0];
						return cmd;
					}
					else
						i=0;
					//DBPRINTF("sum%d&0xff==ch%d,i=%d\n",sum,ch,i);
					break;
				}

			default:
				buffer[i-4]=ch;
				sum+=ch;
				i++;
		}
		if(i==0) break;
	}
	return MCU_CMD_ERROR;

}

int TT232Check(char *buffer)
{
	int i;
	if(OutCmdBuf!=InCmdBuf) {
		memcpy(buffer, MCUCMDBuffer[OutCmdBuf], 8);
		i=OutCmdBuf;
		OutCmdBuf++;
		if(OutCmdBuf>=MCUBUFFERCNT) {
			OutCmdBuf=0;
		}
		return (U8)MCUCMDBuffer[i][7];
	} else {
		return _TT232Check_(buffer);
	}
}
#ifdef ZEM500
#define CMDLEN 22

int CheckTT232(void)
{
	int i,ret=_TT232Check_(MCUCMDBuffer[InCmdBuf]);
	if(ret==MCU_CMD_NONE) return ret;
	if(ret==MCU_CMD_REPONSE) return ret;
	if(ret==MCU_CMD_ERROR) return ret;
	if(ret == MCU_CMD_HID) {
		return ret;
	}
	MCUCMDBuffer[InCmdBuf][7]=ret;
	i=InCmdBuf+1;
	if(i>=MCUBUFFERCNT) i=0;
	if(i!=OutCmdBuf)
		InCmdBuf=i;
	return ret;
}

int TT232GetResponseMS(U32 timeout)
{
	U32 startTime;
	int cmd = 0;
	startTime=GetUS();
	while(1)
	{

		//if(CheckTT232()==MCU_CMD_REPONSE) return TRUE;
		
	    cmd = CheckTT232();
	    
	    if(cmd == MCU_CMD_REPONSE || (cmd != MCU_CMD_ERROR && cmd != MCU_CMD_NONE))
	    {
            return TRUE;
	    }

		if((GetUS() - startTime)>timeout * TICKS_PER_MS)
		{
			break;
	    }
	}
	return FALSE;
}

int ExCommand(char cmd, char *data, int DataLen, int Delay)
{
	U8 buf[CMDLEN+16];
	U32 s,i,len;
	if(gOptions.IsModule) return FALSE;
	if(MCUVersion>7)
		while(CheckTT232()!=MCU_CMD_NONE);
	buf[0]=buf[1]=0x53;
	buf[2]=cmd;  buf[3]=0xFF-buf[2];
	s=0;
	len=CMDLEN;
	if(len<DataLen) len+=16;
	for(i=4; i<len-1; i++)
		if(i-4<DataLen)
		{
			buf[i]=data[i-4];
			s+=buf[i];
		}
		else
			buf[i]=0;
	buf[len-1]=s & 0xFF;
	//DBPRINTF("MCU_VERSION:%d\n",MCUVersion);
	s=MCUVersion>7?10:1;

	while(s--)
	{
		//DBPRINTF("ExCommand begin write:%d\n",GetTickCount()-etime);
		ttl232.write_buf(buf,len);
		//DBPRINTF("ExCommand end write(cmd=%d,DataLen=%d):%d\n",cmd,DataLen,GetTickCount()-etime);
		if((MCUVersion>7) && cmd!=MCU_CMD_RTC && cmd!=MCU_CMD_BATTERY_MOD && cmd!=MCU_CMD_BATTERY_STU)
		{
			if(TT232GetResponseMS(200))
			{
				//DBPRINTF("ExCommand BT232GetResponseMS:%d\n",GetTickCount()-etime);
				return TRUE;
			}
		}
		else
		{
			DBPRINTF("ExCommand delay:%d\n",Delay);
			DelayMS(Delay);
			return TRUE;
		}
	}
	return FALSE;
}

TTime MachineBaseTime;
int gHaveRTC=-1;	//-1 - Unknown, 0 - None, 1 - Have RTC

int ReadSysTime(void)
{
	int res=Timer_None;
	TTime t=MachineBaseTime;
	time_t tt;
	U32 ct=GetUS(), sec=ct/TICKS_PER_SECOND;
	t.tm_year-=1900;
	t.tm_mon-=1;
	tt=mktime(&t)+sec;
	t=*localtime(&tt);
	t.tm_year+=1900;
	t.tm_mon+=1;
	if(t.tm_sec!=gCurTime.tm_sec) res=Timer_Second;
	if(t.tm_min!=gCurTime.tm_min) res=Timer_Minute;
	gCurTime=t;
	if(ct>0x7f000000)
	{
		MachineBaseTime=t;
	}
	return res;
}

TTime *CalcDays(TTime *t)
{
	int y,c;
	y=t->tm_year;
	c=1+(y-1)/4;
	c=c+365*(y-1);  //Total days count
	t->tm_yday=0;
	for(y=1;y<t->tm_mon;y++)
	{
		if(y==2)
		{
			if(t->tm_year%4==0) t->tm_yday+=29; else t->tm_yday+=28;
		}
		else if(y==4 || y==6 || y==9 || y==11)
			t->tm_yday+=30;
		else
			t->tm_yday+=31;
	}
	t->tm_yday+=t->tm_mday;
	c+=t->tm_yday;
	t->tm_wday=(c%7+6)%7;
	//2000-1-1==6
	return t;
}

void SetRTCTimeByMCU(TTime *tt)
{
	char buf[10];
	int year;
	if(gHaveRTC==1)
	{
		CalcDays(tt);
		buf[0]=HEX2BCD(tt->tm_sec);
		buf[1]=HEX2BCD(tt->tm_min);
		buf[2]=HEX2BCD(tt->tm_hour);
		buf[3]=HEX2BCD(tt->tm_mday);
		buf[4]=HEX2BCD(tt->tm_mon+1);	//diff from ZEM100
		buf[5]=(tt->tm_wday+6)%7+1;
		year=tt->tm_year%100;
		buf[6]=HEX2BCD(year);
		DBPRINTF("Settime:%d-%d-%d %d:%d:%d\n",year,tt->tm_mon,tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec);
		ExCommand(21,buf,7,5);
		DelayMS(1000); //DelayMS(2500);
	}
	MachineBaseTime=*tt;
}

int BT232Wait(int CmdType, int TimeOut, char *Buffer)
{
	int c;
	U32 startTime;
	startTime=GetUS();
	while(1)
	{
		c=TT232Check(Buffer);
		/*add by zxz 2012-12-01*/
		/*解决读取mcu版本时,报警信息被清除问题*/
		if(Buffer[7] == MCU_CMD_DOOR) {
			memcpy(MCUALARMBuffer, Buffer, 8);
			MCUALARMBuffer[8] = 1;
			printf("________%s%d, Door event:%d\n", __FILE__, __LINE__, Buffer[0]);
		}
		if(c==CmdType)	return TRUE;
		if((GetUS() - startTime) >
				TimeOut * (TICKS_PER_SECOND/1000))
			return FALSE;
	}
}

int Buff2Time(char *buffer, TTime *Value)
{
	int v;
	TTime t;
	v=BCD2HEX(buffer[0]);
	if((v<0) || (v>59)) return 0;
	t.tm_sec=v;
	v=BCD2HEX(buffer[1]);
	if((v<0) || (v>59)) return 0;
	t.tm_min=v;
	v=BCD2HEX(buffer[2]);
	if((v<0) || (v>23)) return 0;
	t.tm_hour=v;
	v=BCD2HEX(buffer[3]);
	if((v<0) || (v>31)) return 0;
	t.tm_mday=v;
	v=BCD2HEX(buffer[4]);
	if((v<0) || (v>12)) return 0;
	t.tm_mon=v;
	t.tm_year=BCD2HEX(buffer[6])%100;
	//DBPRINTF("Buff2Time:%d-%d-%d %d:%d:%d\n",t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
	CalcDays(&t);
	t.tm_mon-=1;
	t.tm_year+=100;
	*Value=t;
	return 1;
}


void GetRTCTimeByMCU(TTime *t)
{
	if(gOptions.IsModule && gHaveRTC==-1)
		gHaveRTC=0;
	if(gHaveRTC!=0)
	{
		char buffer[10];
		int i=4;
		int TimeOK=FALSE;
		while(i-->0)
		{
			//read RTC
			ExCommand(22,NULL,0,5);
			if(BT232Wait(MCU_CMD_RTC,1000,buffer+1))
			{
				buffer[0]=0;
				if(0==*(int*)buffer && 0==((int*)buffer)[1])//全锟斤拷
				{
					gHaveRTC=0;
				}
				else if(Buff2Time(buffer+1, &gCurTime))//RTC 
				{
					TimeOK=TRUE;
					gHaveRTC=1;
				}
				else
				{
					TimeOK=FALSE;
					gHaveRTC=1;
				}
				break;
			}
			DelayUS(1000);
		}
		if((!TimeOK) && gHaveRTC==1)	//RTC 锟斤拷锟揭ｏ拷锟斤拷写
		{
			;
		}
		else if(gHaveRTC!=1)
			ReadSysTime();
	}
	else
	{
		return ;
		//DBPRINTF("readSysTime,hour=%d,",gCurTime.tm_hour);
		ReadSysTime();
		//DBPRINTF("hour2=%d\n",gCurTime.tm_hour);
	}
	if(t)
	{
		memcpy(t, &gCurTime, sizeof(TTime));
		memcpy(&gMinTime, &gCurTime, sizeof(TTime));
	}
}
#endif

void ExSetPowerOnTime(int Hour, int Minute)
{
	char buf[2];
	buf[0]=HEX2BCD(Hour & 0xFF);
	buf[1]=HEX2BCD(Minute & 0xFF);
	ExCommand(31, buf, 2, 0);
}

void ExPowerRestart(void)
{
	char buf[3]={0x5a,0xa5,0};
	ExCommand(32, buf, 3, 0);
}

extern int powerkey;
void ExEnableClock(int Enable)
{
	DelayUS(10*10);
	if(Enable) Enable = powerkey;//65;		//指定关机键liming
	ClockEnabled=Enable;
	if(Enable)
	{
		ExCommand(23, (char*)&Enable, 1, 5);
		Enable=0;
	}
#ifdef FAMILYPLAN
	Enable=0;
#endif
	ExCommand(23, (char*)&Enable, 1, 8);
}

#endif

void ExSetTftKeyflag(int nKeyStyle)
{
	// ExCommand(MCU_CMD_KEY_TYPE, NULL, 0, 0);
	ExCommand(MCU_CMD_KEY_TYPE, (char* )&nKeyStyle, 1, 5);

}

void ExSetTftHIDFun()
{
	ExCommand(239, NULL, 0, 0);
}

void ExSetExtWGIn(int BitsCount, int PulseWidth, int PulseInterval)
{
	char buf[5];
	memset(buf, 0, sizeof(buf));
	DBPRINTF("External WG Fmt>>>BitsCount=%d,PulseWidth=%d,PulseInterval=%d\n",BitsCount,PulseWidth,PulseInterval);
	buf[0]=BitsCount&0xFF;
	*(unsigned short*)(buf+1)=PulseWidth&0xffff;
	*(unsigned short*)(buf+3)=PulseInterval&0xffff;
	ExCommand(RCMD_WG_OPTIONS,buf,5,20);
}

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
//add by jazzy for set MAC or not 2009.01.06
int IsDefaultMAC(void)
{
	/**//* get mac */
	struct ifreq ifreq;
	int sock;
	char mac[32];

	if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("error");
		return 1;
	}
	strcpy(ifreq.ifr_name,"eth0");
	if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0)
	{
		perror("error:");
		return 1;
	}

	sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",
			(unsigned char)ifreq.ifr_hwaddr.sa_data[0],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

	//    printf("MAC: %s ", mac);

	if (strcmp(mac,"00:17:61:00:00:01")==0)
		return 1;
	else        return 0;
}

//addd by jazzy 2008.12.23 for get power model
void GetPowerMod()
{
	//printf("get power model\n");
	ExCommand(MCU_CMD_BATTERY_MOD,NULL,0,5);
}


char* GetWavPath(char *filename)
{
	static char pathbuf2[80];
	if (strcmp(WavFilePath, "NONE")==0)
	{
		if (!LoadStr("TTSWAVFILEPATH", WavFilePath))
			memset(WavFilePath, 0, 80);
	}
	memset(pathbuf2, 0, sizeof(pathbuf2));
	sprintf(pathbuf2, "%s%s", WavFilePath, filename);
	return (char*)pathbuf2;
}


//zsliu add



void testCheckMCUSensor()
{
	//int ExCommand(char cmd, char *data, int DataLen, int Delay)
	char buf[5]={0};

	int ret = ExCommand(MCU_CMD_DOOR,(char* )buf,0,1);
	printf("zsliu test ret = %d, buf[0]=%d\n", ret, buf[0]);

}

int checkDoorState()
{
	char buf[5]={0};
	int ret = ExCommand(110,(char* )buf,0,4);
	ret = buf[0];
	printf("checkDoorState sensor=%d, %d, %d, %d\n", buf[0], buf[1], buf[2], buf[3]);
	return ret;


}


#ifdef _TTS_
int TTS_PlayWavFileAsync(int argc, char **command, int delay, int volume)
{ 
	int wavdelay=10;	//50ms
	argc= volume;

	//加入TTS 播放语音功能
	TTS_Wait();

	//TTS关语音设备
	TTS_CloseSound();
	DelayMS(wavdelay);

	//Flash开语音设备
	//OpenSoundDevice();
	DelayMS(wavdelay);

	//PlayWavFile(argc, command);
	DelayMS(delay);

	//Flash关语音设备
	//CloseSoundDevice();
	DelayMS(wavdelay);

	//TTS开语音设备
	TTS_OpenSound();
	DelayMS(wavdelay);

	//调试打印播放语音的语音文件名称
	return 0;
}

int TTS_ExPlayAlarm(int AlarmIndex, int volume)
{
	int delay=0;
	char *VoiceWavCmd[3] = {"main", "", NULL};
	char buffer[80];  
	struct stat statTemp;	//for read the file size

	if (strcmp(AlarmFilePath, "NONE")==0)
	{
		if (!LoadStr("WAVFILEPATH", AlarmFilePath))
		{
			memset(AlarmFilePath, 0, 80);
		}
	}
	sprintf(buffer, "%sbell%02d.wav", AlarmFilePath, AlarmIndex+1);

	int ret = 0;
	ret = stat(buffer, &statTemp);
	delay = ((statTemp.st_size*1.0)/8192)*1000 - 300;   //(1024*8=8192) 300ms is wav head will use time 

	//printf("file size=%d,delay=%d\n", statTemp.st_size, delay);

	VoiceWavCmd[1] = buffer;
	//gOptions.AudioVol = volume;

	return (TTS_PlayWavFileAsync(2, VoiceWavCmd, delay, volume));
}

extern int menuflag1;
int TTS_ExPlayWav(char *fileName)
{
	if((menuflag1 == 1) && (gOptions.IclockSvrFun==1))
	{
		TTS_PlayWav(fileName);		
		return 1;
	}
	int delay=0;
	int defaultDelay=2000;
	char *VoiceWavCmd[3] = {"main", "", NULL};
	struct stat statTemp;	//for read the file size
	int ret = 0;  
	ret = stat(fileName, &statTemp);
	delay = ((statTemp.st_size*1.0)/8192)*1000 - 300;   //(1024*8=8192) 300ms is wav head will use time 

	//intf("fileName=%s, file size=%d,delay=%d\n", fileName, statTemp.st_size, delay);  
	VoiceWavCmd[1] = fileName; 

	//防止过度延时
	if(delay>=defaultDelay)
	{
		delay=defaultDelay;
	}

	return (TTS_PlayWavFileAsync(2, VoiceWavCmd, delay, gOptions.AudioVol));
}

#endif	//_TTS_

void ExModemPowerByMcu(int power)
{
	/* 0xFF-power on, 0-power off */
	printf("-----------ExModemPowerByMcu\n");
	ExCommand(224, (char *)&power, 1, 10);
}

int getWavFileDelay(char *fileName)
{
	if(fileName==NULL){
		return 0;
	}

	struct stat statTemp;	//for read the file size
	int ret = 0;  
	ret = stat(fileName, &statTemp);
	int delay = ((statTemp.st_size*1.0)/8192)*1000;   //(1024*8=8192) 300ms is wav head will use time 

	//防止数据出错，无法关闭声音的情况
	if(delay<=0) {
		delay=500;	//0.5秒
	} else if(delay >= 10000) {
		delay = 10000;	//10秒
	}

	//printf("getWavFileDelay filename=%s, delay=%d\n", fileName, delay);
	return delay;
}


/*

   int stat(const char *restrictpathname, struct stat *restrictbuf); 
   提供文件名字，获取文件对应属性。
   int fstat(intfiledes, struct stat *buf); 
   通过文件描述符获取文件对应的属性。
   int lstat(const char *restrictpathname, struct stat *restrictbuf); 
   连接文件描述命，获取文件属性。
   2 文件对应的属性
   struct stat {
   mode_t     st_mode;       //文件对应的模式，文件，目录等
   ino_t      st_ino;       //inode节点号
   dev_t      st_dev;        //设备号码
   dev_t      st_rdev;       //特殊设备号码
   nlink_t    st_nlink;      //文件的连接数
   uid_t      st_uid;        //文件所有者
   gid_t      st_gid;        //文件所有者对应的组
   off_t      st_size;       //普通文件，对应的文件字节数
   time_t     st_atime;      //文件最后被访问的时间
   time_t     st_mtime;      //文件内容最后被修改的时间
   time_t     st_ctime;      //文件状态改变时间
   blksize_t st_blksize;    //文件内容对应的块大小
   blkcnt_t   st_blocks;     //伟建内容对应的块数量
   }; 
   可以通过上面提供的函数，返回一个结构体，保存着文件的信息。
   */


//dsl 2012.3.23. mcu dog start
int feeddog_delay=0;
int feeddog_funflag=0;
int feeddog_time=0;
int feeddog_checktime=0;

int ExFeedWatchDog(int i)
{
	return ExCommand(108, (char*)&i, 1, 1);
}

void mcudog_read_param(void)
{
	feeddog_funflag=LoadInteger("MCUDog", 1);
	feeddog_checktime=LoadInteger("MCUDogTime", 60);

	if (feeddog_checktime > 250)
	{
		feeddog_checktime=250;
	}
	if (feeddog_checktime <= 10)
	{
		feeddog_checktime=20;
	}

	printf("feeddog_checktime=%d, feeddog_funflag=%d\n", feeddog_checktime, feeddog_funflag);
	if (feeddog_funflag && feeddog_checktime)
	{
		feeddog_delay=10;
	}
}

void mcudog_time_signal(void)
{
	if (feeddog_funflag && feeddog_delay){
		if (!--feeddog_delay){
			int ret=0;
			if (!(ret=ExFeedWatchDog(feeddog_checktime))){
				ret=ExFeedWatchDog(feeddog_checktime);
			}
			feeddog_delay=10;
			if (ret)
			{
				printf("feed muddog ok\n");
			}
		}
	}
}

void mcudog_send_signal(int second)
{
	int sec=second;
	if (sec > 250) //mcu only accept 1Byte data, so the max value is 250.
	{
		sec = 250;
	}

	if (feeddog_funflag)
	{
		feeddog_delay=10;
		if (!ExFeedWatchDog(second))
		{
			ExFeedWatchDog(second);
		}
		printf("start_mcudog\n");
	}
}
//end mcu dog

/**/
void led_redlight_on(int type)
{
	switch (type)
	{
		case 0:
		case 1:
		case 2:
			ExLightLED(LED_RED, TRUE);
			ExLightLED(LED_GREEN, FALSE);
			break;
		case 3:
		case 4:
			ExLightLED(LED_RED, FALSE);
			ExLightLED(LED_GREEN, TRUE);
			break;
		default:
			ExLightLED(LED_RED, TRUE);
			ExLightLED(LED_GREEN, FALSE);
	}
}

void led_greenlight_on(int type)
{
	switch (type)
	{
		case 0:
		case 1:
		case 2:
			ExLightLED(LED_GREEN, TRUE);
			ExLightLED(LED_RED, FALSE);
			break;
		case 3:
		case 4:
			ExLightLED(LED_GREEN, FALSE);
			ExLightLED(LED_RED, TRUE);
			break;
	}
}

int led_flash(int RTCTimeValidSign, int OFF)
{
	if(gOptions.IsFlashLed==1){
		if(RTCTimeValidSign){
			ExLightLED(LED_GREEN, OFF);
		} else {
			ExLightLED(LED_RED, OFF);
		}
		OFF =! OFF;
	}else if(gOptions.IsFlashLed==2 || gOptions.IsFlashLed==3 || gOptions.IsFlashLed==4){
		if(RTCTimeValidSign){
			ExLightLED(LED_RED, OFF);
		} else {
			ExLightLED(LED_GREEN, OFF);
		}
		OFF =! OFF;
	}else if(gOptions.IsFlashLed==0){
		if(RTCTimeValidSign){
			ExLightLED(LED_GREEN, OFF);
		} else {
			ExLightLED(LED_RED, OFF);
		}
		OFF=FALSE;
	}
	return OFF;
}


