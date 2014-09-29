/*************************************************
                                           
 ZEM 200                                          
                                                    
 sensor.h 
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef	_SENSOR_H_
#define	_SENSOR_H_

#include "arca.h"
#include "options.h"
#include "zkfp.h"
#include "flashdb.h"

#ifdef OSP04
#define CMOS_WIDTH 	256
#define CMOS_HEIGHT 	336 
#else
#define CMOS_WIDTH 	260
#define CMOS_HEIGHT 	300
#endif

#ifdef URU
#define LOADDRIVERTIME  5
#else
#define LOADDRIVERTIME  2
#endif

enum USER_VERIFY_METHOD {
        ONLY_LOCAL    = 0,
        NETWORK_LOCAL = 1,
        ONLY_NETWORK  = 2,
        LOCAL_NETWORK = 3
};

typedef struct _SensorBufInfo_{
   U32 RawImgLen;
   U32 DewarpedImgLen;
   BYTE *RawImgPtr;
   BYTE *DewarpedImgPtr;
   BYTE SensorNo;
} TSensorBufInfo, *PSensorBufInfo;

#define MSPEED_AUTO 	2
#define IDENTIFYSPEED 	(gOptions.MSpeed<MSPEED_AUTO?gOptions.MSpeed:(FDB_CntTmp()>400?SPEED_HIGH:SPEED_LOW))

HANDLE fhdl;

extern int LastIndex;
extern int ShowVar;
extern int LastUserID;

void InitSensor(int LeftLine, int TopLine, int Width, int Height, int FPReaderOpt);
void FreeSensor(void);
int CaptureSensor(char *Buffer, BOOL Sign, PSensorBufInfo SensorBufInfo);
void FlushSensorBuffer(void);
int CalcThreshold(int NewT);
int CalcNewThreshold(int Thr);
int FPBaseInit(char *FingerBuf);

U32 FPTest(char *ImgBuf);

void CalcFCenter(char *Img, int IWidth, int IHeight, int CWidth, int CHeight, int *HCenter, int *VCenter);
int CalcFingerCenter(int MaxWidth, int MaxHeight);
int CalcImageDPI(char *Img, int IWidth, int IHeight, int *XDPI, int *YDPI);
int CalcVar(BYTE *img, int width, int height, int *var, int *mean, int FrameWidth);

int AutoGainImage(int i);
void CorrectFingerLinear_1(BYTE* PixelsBuffer, BYTE* Target, WORD *Sizes, BYTE Flag);

#endif

