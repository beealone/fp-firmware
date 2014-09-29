#ifndef EXFUNIO_H_
#define EXFUNIO_H_
//---------------For ZEM500-----------------
#define BCD2HEX(a) (((a) & 0x0F) + 10*((a)>>4))
#define HEX2BCD(a) (((a)/10)<<4 | ((a)%10))

#define LCD_WRAP 		1
#define LCD_BOTTOM_LINE 	2
#define LCD_TOP_LINE		4
#define LCD_HIGH_LIGHT		8
#define LCD_LEFT_LINE		16
#define LCD_RIGHT_LINE		32

#define MCU_CMD_NONE 		0
#define MCU_CMD_KEY 		1
#define MCU_CMD_VERSION 	20
#define MCU_CMD_RTC 		22
#define MCU_CMD_LIC 		25
#define RCMD_LCD_SIZE 		33
#define CMD_LCD_STR		34
#define CMD_LCD_BRIGHT		35
#define MCU_CMD_HID 		50
#define MCU_CMD_DOOR		110
#define DOOR_SENSOR_BREAK 	1	//�ű�����???
#define DOOR_BUTTON		2	//���ſ���
#define DOOR_BREAK		3	//��???
#define DOOR_SENSOR_OPEN 	4	//����
#define DOOR_SENSOR_CLOSE 	5	//����???
#define MCU_CMD_BATTERY 	111
#define BATTERY_None		2
#define BATTERY_Internal 	1
#define BATTERY_External 	0
#define MCU_CMD_ERROR 		-1
#define MCU_CMD_REPONSE 	0xA5
#define MCU_CMD_OPTIONS 	120
#define RESET_OPTIONS		1	//�ָ�������
#define AUX_ON			0xFF
#define AUX_OFF 		0

//Check the data from MCU. return
int BT232Check(char *buffer);
int ExCommand(char cmd, char *data, int DataLen, int Delay);
int BT232Wait(int CmdType, int TimeOut, char *Buffer);
int BT232GetChar(U32 timeout);
void ExAuxOut(int AuxOnTime, int OpenDoorDelay);
//void ExSetAuxOutDelay(int AuxOnTime, int OpenDoorDelay, int DoorSensorMode);	//zsliu
extern int gAuxOutDelay;
void ExLightLED(int LEDIndex, int Light);
void ExEnableClock(int Enable);
void ExPowerOffStart(void);
void ExSetPowerOnTime(int Hour, int Minute);
void ExPlayVoiceFrom(int VoiceStart, int VoiceEnd);
int GetMCUVersion(void);
void ExPowerRestart(void);
void ExShowLogo(int Language);
void LoadVoiceParam(void);
int ExAlarm(int Index, int Delay);
void ExGetLCDSize(int *Width, int *Height, int *Data);
int ExAlarmOff(int Index);
int ExBellOff(void);
int ExFeedWatchDog(int i);

int ExIsFPReaderReady(void);
void ExPlayVoiceByMCU(int VoiceIndex);

extern int gAlarmDelay,gAlarmDelayIndex,gBellDelay;

int ExGPIOBeep(int freq, int intensity, int delay);

int FlashInputs(void);

#define RCMD_VERSION		20
#define RCMD_RF_ENABLED		51
#define RCMD_LIC_FP 		101
#define RCMD_LIC_RECFP 		102
#define RCMD_LIC_RECLOG 	103
#define RCMD_LIC_LOCK 		104
#define RCMD_LIC_NET 		105
#define RMCU_CMD_AUXOUT_LEN	106
#define RMCU_CMD_ALARM		107
#define RMCU_CMD_WATCHDOG 	108

#define	RCMD_OUT_BELL		211
#define RCMD_LCD_BACK_LIGHT 	212
#define RCMD_BEEP_ENABLE  	56

#define RCMD_INTWG_OPTIONS 	243
#define RCMD_WG_OPTIONS 	214
#define MCU_CMD_EXTWGIN		215
#define MCU_CMD_INTWGIN		216
#define PCMD_GPRS_POWER 	224 //add by jazzy for GPRS power

int ExGetLicNum(BYTE Lic);
int ExGetLicFP(void);
int CheckLic_1(void);

int ExInit(void);
int FileSystemInit(void);
void DrawImage(char *image, int width, int height, int WhiteThreshold);

void ResetBaseTime(void);

void SetRTCTimeByMCU(TTime *tt);
void GetRTCTimeByMCU(TTime *t);

TTime *DecodeTime(time_t t, TTime *ts);
time_t EncodeTime(TTime *t);
int Buff2Time(char *buffer, TTime *t);
int TimeDiffSec(TTime t1, TTime t2);
int TimeAddSec(TTime *t, int Sec);
TTime *CalcDays(TTime *t);

void TestExFun(void);

void FinalSystem(void);

extern int ClockEnabled;
extern TTime gCurTime;
extern int gPowerState,WaitPowerOff;
extern int gLCDWidth, gLCDHeight, gLCDData;

#define RESETEXCMD
#define IO_FUN_WID0			20	//Wiegand Input Data 0
#define IO_FUN_WID1			21	//Wiegand Input Data 1
#define IO_FUN_STRIP		22	//Strip Detection
#define IO_FUN_CLEAR		23	//Clear Database Button
#define IO_FUN_VERIFY		24	//Start Verify Button
#define IO_FUN_ENROLL		25	//Start Enroll Button
#define IO_FUN_CANCEL		26	//Cancel Button
#define IO_FUN_OPEN_DET		27	//Door Open Detect
#define IO_FUN_USB_INT		28	//USB Int
#define IO_FUN_FPREADER_OK	29	//Fingerprint reader ready

#define INVALID_IO_PIN 'z'
extern BYTE GPIO_PIN[40];
int ExIsLockAndBellSame(void);
int GPIOSetup(BYTE *GPIOFunDef);

void RS485_setmode(U32 SendMode);
void Switch_mode(U32 RS485Mode);
void EnableDevice(int Enabled);

int getgmtime(void *timep, time_t t);
time_t getmktime(TTime *t);

void CheckRTCAdjust(void);  //����Ƿ���ҪУ׼ʱ�???
int CalcRTCAdjust(TTime *NewTime, int MinInterval);  //����У׼����
int AdjustRTC(TTime *t);	//����趨����ʱ��У??

void ExBeepN(int count);

void FinalSystem(void);

extern TTime MachineBaseTime;
extern int gHaveRTC;
extern BYTE *GetFontBuffer(int Size);

BYTE *LoadFile(char *FileName, int *FileSize);

//zzz
#define TICKS_PER_SECOND 2000000	//ZEM100 3686400
#define TICKS_PER_MS	2000						//ZEM100 3686
//

void ExPlayVoiceFromByMCU(int VoiceStart, int VoiceEnd);
void ExPlayVoiceByMCU(int VoiceIndex);
void ExBeepByMCU(int delay);
void ExSetExtWGIn(int BitsCount, int PulseWidth, int PulseInterval);
int GetMCUVersion(void);
#endif /*EXFUNIO_H_*/
