/*************************************************
 sensor.c
                                                      
 Copyright (C) 2003-2009, ZKSoftware Inc.      		

 Common interface code for fingerprint reader library libfpsensor.so

 $Log $
 Revision 1.01  2009/08/05 13:49:09  Zhang Honggen
 Spupport libfpsensor.so,remove fingerprint algorithm function

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "sensor.h"
#include "options.h"
#include "dlfcn.h"
#include "finger.h"

typedef struct _sensor_{
        void *fpHandle;
	char name[128];
        int width;
        int height;
	int size;
} TSensor;

TSensor sensors[8];
void* m_FPHandle;
static int m_SensorCount =0;
static int m_CaptureMode= SENSOR_CAPTURE_MODE_AUTO;
static int m_width=0,m_height=0,m_size=0,m_ImageSize=0;
extern void ExBeep(int delay);

int loadSymbol(void *handle)
{
	sensorInit = dlsym(handle,"sensorInit");
	sensorFree = dlsym(handle,"sensorFree");
	sensorOpen = dlsym(handle,"sensorOpen");
	sensorGetCount = dlsym(handle,"sensorGetCount");
	sensorGetName = dlsym(handle,"sensorGetName");
	sensorCapture = dlsym(handle,"sensorCapture");
	sensorClose = dlsym(handle,"sensorClose");
	sensorGetParameter = dlsym(handle,"sensorGetParameter");
	sensorSetParameter = dlsym(handle,"sensorSetParameter");
	sensorReset = dlsym(handle,"sensorReset");
	sensorPause = dlsym(handle,"sensorPause");
	sensorResume = dlsym(handle,"sensorResume");
	sensorGetAPIVersion = dlsym(handle,"sensorGetAPIVersion");
	sensorGetLibVersion = dlsym(handle,"sensorGetLibVersion");
	return 0;
}

int InitSensor(int *Width, int *Height, int *ImageBufferSize)
{
	int i=0,largerImageSize = 0;
	//int ret;
	m_FPHandle = dlopen("libfpsensor.so",RTLD_LAZY);
	if(m_FPHandle == NULL)
	{
		printf("Cannot open library: %s\n",dlerror());
		return FALSE;
	}	
	loadSymbol(m_FPHandle);
	printf("libSensorApi version:%s,lib version:%s\n",sensorGetAPIVersion(),sensorGetLibVersion());
	printf("Sensor Init %d\n",sensorInit());	
	m_SensorCount = sensorGetCount();
	printf("Sensor Count=%d\n",m_SensorCount);	
	if(m_SensorCount<0)
		return FALSE;
	*ImageBufferSize = 0;
	*Width = 0;
	*Height = 0;
	for(i=0;i<m_SensorCount;i++)
	{
		int imageSize=0,itemp;
		sprintf(sensors[i].name,"%s",sensorGetName(i));
		printf("sensors[%d].name=%s\n",i,sensors[i].name);
       		sensors[i].fpHandle = sensorOpen(i);	//zzz
		if(!sensors[i].fpHandle)
		{
			printf("SensorOpen %d failed\n",i);
			return FALSE;
		}
		if(strncmp(sensors[i].name,"CIM",3)==0)
		{
			sensorSetParameter(sensors[i].fpHandle,SENSOR_FP_SENSITIVITY,gOptions.FingerSensitivity);
			sensorSetParameter(sensors[i].fpHandle,SENSOR_DETECT_FP_ALG,gOptions.DetectFpAlg);
			if(gOptions.FPRotation == 180)
				sensorSetParameter(sensors[i].fpHandle,SENSOR_IMAGE_PROC_ROTATION,gOptions.FPRotation);
		}
		m_CaptureMode= SENSOR_CAPTURE_MODE_AUTO;
		sensors[i].width = sensorGetParameter(sensors[i].fpHandle, SENSOR_PARAM_CODE_WIDTH);
		sensors[i].height = sensorGetParameter(sensors[i].fpHandle,SENSOR_PARAM_CODE_HEIGHT); 
		imageSize = sensorGetParameter(sensors[i].fpHandle,SENSOR_PARAM_CODE_IMAGE_BUFFER_SIZE);
		//select larger buffer 
		if(imageSize>*ImageBufferSize)
			*ImageBufferSize = imageSize;
		//select small image to use
		itemp =sensors[i].width*sensors[i].height;
		if(itemp > largerImageSize)
			largerImageSize = itemp;
		if(((*Width**Height)==0) || (itemp>0 && itemp<(*Width**Height))) 
		{
			*Width = sensors[i].width;
			*Height = sensors[i].height;
		}
		sensors[i].size = sensors[i].width * sensors[i].height;
		
	}

	//All sensors must output same size image
	for(i=0;i<m_SensorCount;i++)
	{
		if(sensors[i].width != *Width || sensors[i].height != *Height)
		{
			sensorSetParameter(sensors[i].fpHandle,SENSOR_PARAM_CODE_WIDTH,*Width);
			sensorSetParameter(sensors[i].fpHandle,SENSOR_PARAM_CODE_HEIGHT,*Height);
			
			sensors[i].width = sensorGetParameter(sensors[i].fpHandle, SENSOR_PARAM_CODE_WIDTH);
			sensors[i].height = sensorGetParameter(sensors[i].fpHandle,SENSOR_PARAM_CODE_HEIGHT); 
		}
	}
	m_width = *Width;
	m_height = *Height;
	m_size = m_width*m_height;
	if(m_SensorCount>1 && *ImageBufferSize < largerImageSize*2)
		*ImageBufferSize = largerImageSize * 2;	//two sensor need large buffer for function convertImage 
	m_ImageSize = *ImageBufferSize;
	printf("sensor init complete,width=%d,height=%d,ImageSize=%d\n",*Width,*Height,m_ImageSize);
	return TRUE;
}

void FreeSensor(void)
{     
	int i;
	for(i=0;i<m_SensorCount;i++)
		sensorClose(sensors[i].fpHandle);
	dlclose(m_FPHandle);
}

void FlushSensorBuffer(void)
{
	//nothing to do
}

static void SetCaptureMode(int mode)
{
	int i;
	for(i=0;i<m_SensorCount;i++)
	    sensorSetParameter(sensors[i].fpHandle, SENSOR_PARAM_CODE_CAPTURE_MODE, mode);
}

int CaptureSensor(char *Buffer, int CaptureMode, PSensorBufInfo SensorBufInfo)
{
	int i,len;
	if(m_SensorCount <=0)
		return FALSE;
	if(fhdl<=0 && CaptureMode!= SENSOR_CAPTURE_MODE_STREAM)
	        return FALSE;
	
	if(CaptureMode!=m_CaptureMode)
	{
		SetCaptureMode(CaptureMode);
		m_CaptureMode = CaptureMode;
	}
	for(i=0;i<m_SensorCount;i++)
	{
		len = sensorCapture(sensors[i].fpHandle,(BYTE*)Buffer,m_ImageSize);
		if(len>0)
		{
			/*dont need beep when test the fp sensor in menu,dsl 2011.4.7*/
#if 0
			if(m_CaptureMode == SENSOR_CAPTURE_MODE_AUTO){
	        		ExBeep(1);
				printf("beep in CaptureSensor\n");
			}
#endif
			if(SensorBufInfo)
			{
				//image buffer info
				SensorBufInfo->SensorNo=255;
				SensorBufInfo->RawImgLen=len;
				SensorBufInfo->RawImgPtr=(BYTE*)Buffer;
				SensorBufInfo->DewarpedImgLen=len;
				SensorBufInfo->DewarpedImgPtr=(BYTE*)Buffer;
			}
			return TRUE;
		}
		else if (len < 0)
		{
			return -1;
		}
	}
	
	return FALSE;
}



