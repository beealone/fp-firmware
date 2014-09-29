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

#define CMOS_WIDTH 		640
#define CMOS_HEIGHT 		480

// image sizes output by the device
#define  DEV_IMAGE_WIDTH 	384
#define  DEV_IMAGE_HEIGHT 	289

#define uru4000_width 		292
#define uru4000_height 		400

#define LOADDRIVERTIME		5

#define URU_IMAGE_SIZE 		150*1024  //RAWIMAGE OR URU4000_WIDTH*URU4000_HEIGHT

enum USER_VERIFY_METHOD {
        ONLY_LOCAL    = 0,
        NETWORK_LOCAL = 1,
        ONLY_NETWORK  = 2,
        LOCAL_NETWORK = 3,
	ONLY_CAPTURE  = 4
};

typedef struct _SensorBufInfo_{
   U32 RawImgLen;
   U32 DewarpedImgLen;
   BYTE *RawImgPtr;
   BYTE *DewarpedImgPtr;
   BYTE SensorNo;
} TSensorBufInfo, *PSensorBufInfo;

#define MSPEED_AUTO 		2

#define IDENTIFYSPEED (gOptions.MSpeed<MSPEED_AUTO?gOptions.MSpeed:(FDB_CntTmp()>400?SPEED_HIGH:SPEED_LOW))

HANDLE fhdl;

extern int LastIndex;
//extern int ShowVar;
extern int LastUserID;

#ifdef UPEK
void InitSensor1(int LeftLine, int TopLine, int Width, int Height, int FPReaderOpt);
#else
void InitSensor(int LeftLine, int TopLine, int Width, int Height, int FPReaderOpt);
#endif
void RotationImageVetical(char *buffer, int width, int height);
void dewarpImageAndStretch(unsigned char *src, unsigned char *dest);
void FreeSensor(void);
int CaptureSensor(char *Buffer, BOOL Sign, PSensorBufInfo SensorBufInfo);
void FlushSensorBuffer(void);
int CalcThreshold(int NewT);
int CalcNewThreshold(int Thr);
int FPBaseInit(char *FingerBuf);

#endif

