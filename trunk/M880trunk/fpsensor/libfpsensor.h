/****************************************************
	author	: jazzy 
	date	: 2009.05.22
                                                      
 Copyright (C) 2009-2009, ZKSoftware Inc.               
                                                      
*************************************************/
#ifndef __LIBFPSENSOR_H__
#define __LIBFPSENSOR_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility("default")))
    #define DLL_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif

//错误信息:
#define SENSOR_PARAM_ERROR_INVALID_CODE	-1
#define SENSOR_PARAM_ERROR_READONLY	-2
#define SENSOR_PARAM_ERROR_NOT_SUPPORT	-3
		
#define SENSOR_PARAM_VALUE_TRUE		1
#define SENSOR_PARAM_VALUE_FALSE	0

//图像宽度
#define SENSOR_PARAM_CODE_WIDTH		1
//图像高度
#define SENSOR_PARAM_CODE_HEIGHT	2
//图像像素值
#define SENSOR_PARAM_CODE_DPI		3
//图像颜色
#define SENSOR_PARAM_CODE_IMAGE_FMT	4
#define SENSOR_IMAGE_FMT_GRAY256	256
#define SENSOR_IMAGE_FMT_GRAY16		16
#define SENSOR_IMAGE_FMT_GRAY8		8
#define SENSOR_IMAGE_FMT_BINARY		2
//取像模式
#define SENSOR_PARAM_CODE_CAPTURE_MODE	5
#define SENSOR_CAPTURE_MODE_AUTO	0	//capture image if finger present
#define SENSOR_CAPTURE_MODE_STREAM  	1	//capture image not care about finger present or not
//取像区域
#define SENSOR_PARAM_CODE_IMAGE_AREA	6
#define SENSOR_IMAGE_AREA_VALID		0
#define SENSOR_IMAGE_AREA_FULL		1
//图像模式
#define SENSOR_PARAM_CODE_IMAGE_PROC	7
#define	SENSOR_IMAGE_PROC_STD		0
#define SENSOR_IMAGE_PROC_RAW   	1
//FP detect alg
#define SENSOR_DETECT_FP_ALG		8
//fingerprint sensitivity
#define SENSOR_FP_SENSITIVITY		9
#define SENSOR_IMAGE_PROC_ROTATION	10
//背光控制
#define SENSOR_PARAM_CODE_CTRL_BACK_LIGHT	101
//绿灯控制
#define SENSOR_PARAM_CODE_CTRL_GREEN_LED	102
//红灯控制
#define SENSOR_PARAM_CODE_CTRL_RED_LED		103
//提示音控制
#define SENSOR_PARAM_CODE_CTRL_BEEP	    	104
//图像采集状态
#define SENSOR_PARAM_CODE_CTRL_FP_PRESENT	105
//指纹采集器图像空间大小
#define SENSOR_PARAM_CODE_IMAGE_BUFFER_SIZE 	106 //get - image buffer size should be malloced when call sensorCapture;
//图像空间分配模式
#define SENSOR_PARAM_CODE_IMAGE_BUFFER_MOD 	107 
#define SENSOR_PARAM_CODE_IMAGE_BUFFER_AUTO  	0 //auto count malloc memory
#define SENSOR_PARAM_CODE_IMAGE_BUFFER_USER  	1  //user malloc memory
//获取指纹采集库名称
#define SENSOR_PARAM_CODE_LIB_NAME 	200      //get the full library file name
//获取授权信息
#define SENSOR_PARAM_CODE_LIB_LICENSE   201 //get the license code
//自定义参数起始值
#define SENSOR_PARAM_CODE_USER_START    1000

#define SENSOR_PARAM_CODE_NEWFP_TYPE	SENSOR_PARAM_CODE_USER_START+1

#define SENSOR_ERROR_NONE		0
#define SENSOR_ERROR_FAILED		-1
#define SENSOR_ERROR_NULL		-2
#define SENSOR_ERROR_OUT_OF_RANGE	-3
#define SENSOR_ERROR_STATE		-4
#define SENSOR_ERROR_NOT_IMPLEMENTED	-5
#define SENSOR_ERROR_NOT_SUPPORTED	-5

#define SENSOR_ERROR_EEPROM		-55

#define SENSOR_ERROR_STATE_LEAVE_FINGER	-101
#define SENSOR_ERROR_STATE_TOUCH_FINGER	-102
#define SENSOR_ERROR_STATE_FAKE_FINGER	-103

#define FPSENSOR_DLOPEN

#ifdef FPSENSOR_DLOPEN  
typedef int (*T_sensorInit)();
typedef int (*T_sensorFree)();
typedef int (*T_sensorGetCount)();
typedef const char * (*T_sensorGetName)(int index);
typedef void * (*T_sensorOpen)(int index);
typedef int (*T_sensorCapture)(void *handle, unsigned char *imageBuffer, int imageBufferSize);
typedef int (*T_sensorClose)(void *handle);
typedef int (*T_sensorGetParameter)(void *handle, int paramCode);
typedef int (*T_sensorSetParameter)(void *handle, int paramCode, int paramValue);
typedef int (*T_sensorReset)(void *handle);
typedef int (*T_sensorPause)(void *handle);
typedef int (*T_sensorResume)(void *handle);
typedef const char *(*T_sensorGetAPIVersion)();
typedef const char *(*T_sensorGetLibVersion)();

T_sensorInit sensorInit;
T_sensorFree sensorFree;
T_sensorGetCount sensorGetCount;
T_sensorGetName sensorGetName;
T_sensorOpen sensorOpen;
T_sensorCapture sensorCapture;
T_sensorClose sensorClose;
T_sensorGetParameter sensorGetParameter;
T_sensorSetParameter sensorSetParameter;
T_sensorReset sensorReset;
T_sensorPause sensorPause;
T_sensorResume sensorResume;
T_sensorGetAPIVersion sensorGetAPIVersion;
T_sensorGetLibVersion sensorGetLibVersion;

#else
int DLL_PUBLIC sensorInit();
int DLL_PUBLIC sensorFree();
int DLL_PUBLIC sensorGetCount();
DLL_PUBLIC const char * sensorGetName(int index);
DLL_PUBLIC void * sensorOpen(int index);
int DLL_PUBLIC sensorCapture(void *handle, unsigned char *imageBuffer, int imageBufferSize);
int DLL_PUBLIC sensorClose(void *handle);
int DLL_PUBLIC sensorGetParameter(void *handle, int paramCode);
int DLL_PUBLIC sensorSetParameter(void *handle, int paramCode, int paramValue);
int DLL_PUBLIC sensorReset(void *handle);
int DLL_PUBLIC sensorPause(void *handle);
//唤醒指定采集器
int DLL_PUBLIC sensorResume(void *handle);
//获取指纹采集库API版本信息
DLL_PUBLIC const char *sensorGetAPIVersion();
//获取指纹采集库版本信息
DLL_PUBLIC const char *sensorGetLibVersion();

#endif
#endif
