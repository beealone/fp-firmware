/*************************************************
                                           
 ZEM 200                                          
                                                    
 sensor.h 
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef	_SENSOR_H_
#define	_SENSOR_H_

#include "arca.h"
#include "libfpsensor.h"

#define CMOS_WIDTH 	640
#define CMOS_HEIGHT 	480

//define waiting time 
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

int InitSensor(int *Width, int *Height, int *ImageBufferSize);
void FreeSensor(void);
//stream mode or detecting mode
int CaptureSensor(char *Buffer, int CaptureMode, PSensorBufInfo SensorBufInfo);
void FlushSensorBuffer(void);
#endif

