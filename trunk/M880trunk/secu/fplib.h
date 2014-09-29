/*************************************************************
 *
 * Author :      SecuGen Corporation
 * Description : fplib.h Source Code Module
 * Copyright(c): 2004 SecuGen Corporation, All rights reserved
 * History : 
 * date        person   comments
 * ======================================================
 * 2004-10-14  driley   Added support for multiple devices and 
 *                      Optimouse III
 *
 *
 *************************************************************/

#ifndef FPLIB_H
#define FPLIB_H

#include "misc4unix.h" // jacob 2001.07.27
#ifdef __cplusplus
extern "C" {
#endif
   
#define  CN_FDS01       (0)  // FDS01 code name, serial port type
#define  CN_FDP01       (1)  // FDP01 code name, parallel port type
#define  CN_FDU01       (2)  // FDU01 code name, USB type
#define  CN_FDC01       (3)  // FDC01 code name, card type
#define  CN_FDA01       (4)  // FDA01 code name, built-in CPU
#define  CN_FDA10       (5)  // FDA10 code name, built-in CPU
#define  CN_FDP02       (6)  // FDP02 code name, parallel port type, 2nd generation
#define  CN_FDU02       (7)  // FDU02 code name, USB type 2nd generation //RILEY 2004.10.02
   
enum ErrorCode {
   // General error
   ERROR_NONE = 0,
   ERROR_CREATION_FAILED,
   ERROR_FUNCTION_FAILED,
   ERROR_INVALID_PARAM,
   ERROR_NOT_USED,
   
   // Device error
   ERROR_SYSLOAD_FAILED = 51, // system file load fail
   ERROR_INITIALIZE_FAILED = 52,   // chip initialize fail
   ERROR_LINE_DROPPED = 53,        // image data drop
   ERROR_TIME_OUT = 54,            // getliveimage timeout error
   ERROR_DEVICE_NOT_FOUND = 55,    // device not found
   ERROR_DLLLOAD_FAILED = 56,      // dll file load fail
   ERROR_WRONG_IMAGE = 57,         // wrong image
   ERROR_LACK_OF_BANDWIDTH = 58,   // USB Bandwidth Lack Error //RILEY 2004.10.02
   ERROR_DEV_ALREADY_OPEN = 59,    // Device Exclusive access Error //RILEY 2004.10.02
   ERROR_GETSN_FAILED = 60,        // Fail to get Device Serial Number //RILEY 2004.10.02
   
   // Extract&verification error
   ERROR_UNLOCK_FAIL = 101,  
   ERROR_DLL_LOCKED,          // dll locked
   ERROR_REGIST_FAIL,         // register fail
   ERROR_VERIFY_FAIL,         // verification fail
   ERROR_IMAGE_LOCKED,        // image locked
   ERROR_EXTRACT_FAIL,        // extraction fail
   ERROR_VERIFY_FAKE,         // verified to fake
   ERROR_NO_SEARCH_DATA,
   ERROR_INVALID_EXTRACTION_LIBRARY,
   ERROR_INVALID_VERIFICATION_LIBRARY

};

enum SecurityLevel {
   LOWEST=1,
      LOWER,
      LOW,
      BELOW_NORMAL,   
      NORMAL,
      ABOVE_NORMAL,
      HIGH,
      HIGHER,
      HIGHEST,
};

enum ComPort {
   AUTO_DETECT = 0,
   COM1        = 0x3f8,       // default
   COM2	      = 0x2f8,
   COM3	      = 0x3e8,
   COM4	      = 0x2e8,
   LPT1        = 0x378,
   LPT2        = 0x278,
   LPT3        = 0x3BC,
   USB,
};

//RILEY 2004.10.02
#define USB_AUTO_DETECT (0x3BC+1)   // same as ComPort:USB				//RILEY 2004.10.02
#define PARALLEL_AUTO_DETECT (0)   // same as ComPort:AUTO_DETECT //RILEY 2004.10.02

// DeviceInfo Param
typedef struct tagDeviceInfoParam
{
   DWORD ComPort;
   DWORD ComSpeed;
   DWORD ImageWidth;
   DWORD ImageHeight;
   DWORD Contrast; 	// 0~100
   DWORD Brightness; // 0~100
   DWORD BlackCalibration;
   DWORD AutoExposure;
   DWORD AutoGain;
   DWORD	Gain;
}DeviceInfoParam;
typedef DeviceInfoParam* LPDeviceInfoParam;

// 2000.2.23 
enum {
      MIN_MODE_400 = 400,
      MIN_MODE_256 = 256
};   

// 2000.3.6
enum {
      SETINFO_MINUTIAE_SIZE = 0,
};

/////////////////////////////////////////////////////
//RILEY 2004.10.02
// Used at EnumerateDevice()
//Used at GetDeviceInfo2(), EnumerateDevice()
#define SGST_DEV_INFO1		1    // Structure Type1
#define SGST_DEV_INFO2		2    // Structure Type2
#define SGDEV_SN_LEN		15   // Device Serial Number Length
/////////////////////////////////////////////////////

typedef struct tagDeviceList
{
  DWORD DeviceID;
  BYTE DeviceSN[SGDEV_SN_LEN+1];
} DeviceList;
typedef DeviceList *LPDeviceList;

// Used at GetDeviceInfo2()
typedef struct tagDeviceInfoParam2
{
   DWORD DeviceID;	          // 0 - 9
   BYTE DeviceSN[SGDEV_SN_LEN+1]; //Device Serial Number, Length of SN = 15
   DWORD ComPort;
   DWORD ComSpeed;
   DWORD ImageWidth;
   DWORD ImageHeight;
   DWORD Contrast; 	// 0~100
   DWORD Brightness; // 0~100
   DWORD Gain;
   DWORD ImageDPI;
   DWORD FWVersion;
}DeviceInfoParam2;
typedef DeviceInfoParam2* LPDeviceInfoParam2;
/////////////////////////////////////////////////////
   
#ifdef WIN32
#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif  // DLLEXPORT
#else   // WIN32
#define DLLEXPORT       // for Linux
#endif  // WIN32
   
#ifdef __cplusplus

   struct FPM
   {
      virtual ~FPM(){};
   
      // Image sensor API
      virtual DWORD WINAPI  OpenDevice(DWORD port) = 0;
      virtual DWORD WINAPI  CloseDevice() = 0;
      virtual DWORD WINAPI  GetImage(BYTE* buffer) = 0;
      virtual DWORD WINAPI  GetError()= 0;
      virtual DWORD WINAPI  GetDeviceInfo(DeviceInfoParam* pInfo) = 0;
      virtual DWORD WINAPI  Configure(HWND hwnd) = 0;
      virtual DWORD WINAPI  ResetDevice() = 0;
      virtual DWORD WINAPI  SetBrightness(DWORD brightness) = 0;
      virtual DWORD WINAPI  SetContrast(DWORD contrast) = 0;
      virtual DWORD WINAPI  SetBlackCalibration(DWORD calibration) = 0;
      virtual DWORD WINAPI  SetLedOn(bool on) = 0;
   
      // Extraction API
      virtual DWORD WINAPI  GetMinutiae(BYTE *image, BYTE* minutiae) = 0;
   
      // Matching API
      virtual DWORD  WINAPI MatchForRegister(BYTE *min1, BYTE *min2, DWORD secuLevel) = 0;
      virtual DWORD  WINAPI MatchForVerification(BYTE *regMin1, BYTE *inMin, DWORD secuLevel) = 0;
      virtual DWORD  WINAPI MatchForVerificationEx(BYTE *regMin1, BYTE *regMin2, BYTE *inMin, DWORD secuLevel) = 0;
   
      // 99.3.9 Additional API
      virtual DWORD  WINAPI GetLiveImage(BYTE* buffer, DWORD time = 0, HWND dispWnd = NULL) = 0;
      virtual DWORD  WINAPI GetLiveImageEx(BYTE* buffer, DWORD time = 0, HWND dispWnd = NULL, DWORD quality = 50) = 0;

      // 99.9.3 Additional API
      virtual DWORD  WINAPI SetGain(DWORD gain) = 0;
      virtual DWORD  WINAPI GetImageQuality(DWORD width, DWORD height, BYTE* imgBuf, DWORD* quality) = 0;

      // 2000.3.6 Additional API
      virtual DWORD  WINAPI SetDeviceInfo(DWORD dwType, DWORD dwValue, LPVOID pReserved = NULL) = 0;

      // 2001.1.12 Additional API. Added by Bastian Choi.
      virtual DWORD  WINAPI AutoTuning() = 0;

      // 2001.7.20 Additional API. Added by Josephin
      virtual DWORD  WINAPI WriteData(unsigned char index, unsigned char data) = 0;
////////////////////////////////////////////////////
//RILEy 2004.10.02
      virtual DWORD  WINAPI ReadData (unsigned char index, unsigned char* data) = 0;
      virtual DWORD  WINAPI SetCallBackFunction(DWORD selector, DWORD (WINAPI*)(void* pUserData, void* pCallBackData), void* pUserData) = 0;
      virtual DWORD  WINAPI EnumerateDevice(DWORD* ndevs, DeviceList** devList) = 0;
      virtual DWORD  WINAPI GetDeviceInfoEx(DWORD structureType, void* info) = 0;
      virtual DWORD  WINAPI ResetExtractionModule(DWORD imageWidth, DWORD imageHeight, DWORD dpi) = 0;
      virtual DWORD  WINAPI GetMatchingScore(BYTE* min1, BYTE* min2, DWORD* score) = 0;
      virtual DWORD  WINAPI GetmatchingScore(BYTE* min2, BYTE* min2, DWORD* score) = 0; //RILEY - Bug

////////////////////////////////////////////////////



////////////////////////////////////////////////////
//RILEY 2004.10.02
//Following functions not used in Collection 7 Windows 
      virtual DWORD WINAPI  ResetMatchingEngine(void) = 0;
 
      // 2000.3.6 Additional API
      virtual DWORD  WINAPI ConvertMinData(unsigned char *InMinData, DWORD InMinType, unsigned char *OutMinData, DWORD OutMinType) = 0;

      // 2001.1.12 Additional API. Added by Bastian Choi.
      virtual DWORD  WINAPI AutoTunning() = 0;
      virtual DWORD  WINAPI Identify(unsigned char *inMin, int secuLevel, unsigned long nFinger, unsigned long nCountPerFinger, unsigned char *minArray, unsigned long* matchedIndex) = 0;
////////////////////////////////////////////////////

   };

   typedef FPM  FAR* LPFPM;
   typedef DWORD (WINAPI* FPM_CreateFunc)(DWORD codeName, LPFPM* ppFPM);
   typedef DWORD (WINAPI* FPM_DestroyFunc)(FPM* pFPM);
   DLLEXPORT DWORD WINAPI  CreateFPMObject(DWORD codeName, LPFPM* ppFPM = (LPFPM*) NULL);
   DLLEXPORT DWORD WINAPI  DestroyFPMObject(FPM* pFPM = (FPM*) NULL);

#endif //__cplusplus


#ifndef _FPM_MULTI_PORT_SUPPORT
   //------------------------------------------------------------------------------
   // c style exported function
   // only one port support

   // Library Initialization
   DLLEXPORT DWORD WINAPI  FPMInitLibrary(DWORD codeName);
   DLLEXPORT DWORD WINAPI  FPMExitLibrary();
   
   // Image sensor API
   DLLEXPORT DWORD WINAPI  FPMOpenDevice(DWORD port);
   DLLEXPORT DWORD WINAPI  FPMGetImage(BYTE* buffer);
   DLLEXPORT DWORD WINAPI  FPMGetError();
   DLLEXPORT DWORD WINAPI  FPMGetDeviceInfo(DeviceInfoParam* pInfo);
   DLLEXPORT DWORD WINAPI  FPMConfigure(HWND hwnd);
   DLLEXPORT DWORD WINAPI  FPMResetDevice();
   DLLEXPORT DWORD WINAPI  FPMSetBrightness(DWORD brightness);
   DLLEXPORT DWORD WINAPI  FPMSetContrast(DWORD contrast);
   DLLEXPORT DWORD WINAPI  FPMSetBlackCalibration(DWORD calibration);
   DLLEXPORT DWORD WINAPI  FPMSetLedOn(BOOL on);
   DLLEXPORT DWORD WINAPI  FPMSetGain(DWORD gain);
   
   // Extraction API
   DLLEXPORT DWORD  WINAPI FPMGetMinutiae(BYTE *image, BYTE* minutiae);
   
   // Matching API
   DLLEXPORT DWORD  WINAPI FPMMatchForRegister(BYTE *min1, BYTE *min2, DWORD secuLevel);
   DLLEXPORT DWORD  WINAPI FPMMatchForVerification(BYTE *regMin1, BYTE *inMin, DWORD secuLevel);
   DLLEXPORT DWORD  WINAPI FPMMatchForVerificationEx(BYTE *regMin1, BYTE *regMin2, BYTE *inMin, DWORD secuLevel);
   
   // 99.3.9
   DLLEXPORT DWORD  WINAPI FPMGetLiveImage(BYTE* buffer, DWORD time, HWND dispWnd);
   DLLEXPORT DWORD  WINAPI FPMAutoTunning();
   DLLEXPORT DWORD  WINAPI FPMGetLiveImageEx(BYTE* buffer, DWORD time, HWND dispWnd, DWORD quality);
   
   // 99.4.16, 1:Many matching
   DLLEXPORT DWORD  WINAPI FPMIdentify(unsigned char *inMin, int secuLevel, unsigned long nFinger, unsigned long nCountPerFinger, unsigned char *minArray, unsigned long* matchedIndex);
   
   // 99.9.6
   DLLEXPORT DWORD  WINAPI FPMGetImageQuality(DWORD width, DWORD height, BYTE* imgBuf, DWORD* quality);
   DLLEXPORT DWORD  WINAPI FPMResetMatchingEngine(void);

   // 2000.3.6
   DLLEXPORT DWORD  WINAPI FPMSetDeviceInfo(DWORD dwType, DWORD dwValue, void* pReserved);
   DLLEXPORT DWORD  WINAPI FPMConvertMinData(unsigned char *InMinData, DWORD InMinType, unsigned char *OutMinData, DWORD OutMinType);

   // 2001.1.12 Additional API. Added by Bastian Choi.
   DLLEXPORT DWORD  WINAPI FPMAutoTuning();
   DLLEXPORT DWORD  WINAPI FPMCloseDevice();

   // 2001.7.20 Additional API. Added by Josephin
   DLLEXPORT DWORD  WINAPI FPMWriteData(unsigned char index, unsigned char data);
////////////////////////////////////////////////////
//RILEy 2004.10.02
   // 2002.2.25
   DLLEXPORT DWORD  WINAPI FPMReadData(unsigned char index, unsigned char* data);
   DLLEXPORT DWORD  WINAPI FPMSetCallBackFunction(DWORD selector, DWORD (WINAPI*)(void* pUserData, void* pCallBackData), void* pUserData);

   DLLEXPORT DWORD  WINAPI FPMEnumerateDevice(DWORD* ndevs, DeviceList** devList);
   DLLEXPORT DWORD  WINAPI FPMGetDeviceInfoEx(DWORD structureType, void* info);
   DLLEXPORT DWORD  WINAPI FPMResetExtractionModule(DWORD imageWidth, DWORD imageHeight, DWORD dpi);
      
   // 2004.7.16, Josephin Kang
   DLLEXPORT DWORD WINAPI FPMGetMatchingScore(BYTE* min1, BYTE* min2, DWORD* score);
   DLLEXPORT DWORD WINAPI FPMGetmatchingScore(BYTE* min1, BYTE* min2, DWORD* score);
////////////////////////////////////////////////////

#else // ifdef _FPM_MULTI_PORT_SUPPORT

   //------------------------------------------------------------------------------
   // c style exported function 2.
   // Multiport support version.

   typedef void*  HFPM;


   // Library Initialization
   DLLEXPORT DWORD WINAPI  FPMInitLibrary2(HFPM* ppFpmHandle, DWORD codeName);
   DLLEXPORT DWORD WINAPI  FPMExitLibrary2(HFPM pFpmHandle);
   
   // Image sensor API
   DLLEXPORT DWORD WINAPI  FPMOpenDevice2(HFPM pFpmHandle, DWORD port);
   DLLEXPORT DWORD WINAPI  FPMGetImage2(HFPM pFpmHandle, BYTE* buffer);
   DLLEXPORT DWORD WINAPI  FPMGetError2(HFPM pFpmHandle);
   DLLEXPORT DWORD WINAPI  FPMGetDeviceInfo2(HFPM pFpmHandle, DeviceInfoParam* pInfo);
   DLLEXPORT DWORD WINAPI  FPMConfigure2(HFPM pFpmHandle, HWND hwnd);
   DLLEXPORT DWORD WINAPI  FPMResetDevice2(HFPM pFpmHandle);
   DLLEXPORT DWORD WINAPI  FPMSetBrightness2(HFPM pFpmHandle, DWORD brightness);
   DLLEXPORT DWORD WINAPI  FPMSetContrast2(HFPM pFpmHandle, DWORD contrast);
   DLLEXPORT DWORD WINAPI  FPMSetBlackCalibration2(HFPM pFpmHandle, DWORD calibration);
   DLLEXPORT DWORD WINAPI  FPMSetLedOn2(HFPM pFpmHandle, bool on);
   DLLEXPORT DWORD WINAPI  FPMSetGain2(HFPM pFpmHandle, DWORD gain);
   
   // Extraction API
   DLLEXPORT DWORD  WINAPI FPMGetMinutiae2(HFPM pFpmHandle, BYTE *image, BYTE* minutiae);
   
   // Matching API
   DLLEXPORT DWORD  WINAPI FPMMatchForRegister2(HFPM pFpmHandle, BYTE *min1, BYTE *min2, DWORD secuLevel);
   DLLEXPORT DWORD  WINAPI FPMMatchForVerification2(HFPM pFpmHandle, BYTE *regMin1, BYTE *inMin, DWORD secuLevel);
   DLLEXPORT DWORD  WINAPI FPMMatchForVerificationEx2(HFPM pFpmHandle, BYTE *regMin1, BYTE *regMin2, BYTE *inMin, DWORD secuLevel);

   DLLEXPORT DWORD  WINAPI FPMGetLiveImage2(HFPM pFpmHandle, BYTE* buffer, DWORD time, HWND dispWnd);
   DLLEXPORT DWORD  WINAPI FPMAutoTunning2(HFPM pFpmHandle);
   DLLEXPORT DWORD  WINAPI FPMGetLiveImageEx2(HFPM pFpmHandle, BYTE* buffer, DWORD time, HWND dispWnd, DWORD quality);
   
   DLLEXPORT DWORD  WINAPI FPMIdentify2(HFPM pFpmHandle, unsigned char *inMin, int secuLevel, unsigned long nFinger, unsigned long nCountPerFinger, unsigned char *minArray, unsigned long* matchedIndex);

   // 99.9.6
   DLLEXPORT DWORD  WINAPI FPMGetImageQuality2(HFPM pFpmHandle, DWORD width, DWORD height, BYTE* imgBuf, DWORD* quality);
   DLLEXPORT DWORD  WINAPI FPMResetMatchingEngine2(HFPM pFpmHandle);

   // 2000.3.6
   DLLEXPORT DWORD  WINAPI FPMSetDeviceInfo2(HFPM pFpmHandle, DWORD dwType, DWORD dwValue, LPVOID pReserved);
   DLLEXPORT DWORD  WINAPI FPMConvertMinData2(HFPM pFpmHandle, unsigned char *InMinData, DWORD InMinType, unsigned char *OutMinData, DWORD OutMinType);

   // 2001.1.12 Additional API. Added by Bastian Choi.
   //DLLEXPORT DWORD  WINAPI FPMAutoTuning2();
   //DLLEXPORT DWORD  WINAPI FPMCloseDevice2();

   // Modifed by josephin
   DLLEXPORT DWORD  WINAPI FPMAutoTuning2(HFPM pFpmHandle);
   DLLEXPORT DWORD  WINAPI FPMCloseDevice2(HFPM pFpmHandle);

   // 2001.7.20 Additional API. Added by Josephin
   DLLEXPORT DWORD  WINAPI FPMWriteData2(HFPM pFpmHandle, unsigned char index, unsigned char data);
////////////////////////////////////////////////////
//RILEY 2004.10.02
   DLLEXPORT DWORD  WINAPI FPMReadData2(HFPM pFpmHandle, unsigned char index, BYTE* data);
   DLLEXPORT DWORD  WINAPI FPMSetCallBackFunction2(HFPM pFpmHandle, DWORD selector, DWORD (WINAPI*)(void* pUserData, void* pCallBackData), void* pUserData);
   DLLEXPORT DWORD  WINAPI FPMEnumerateDevice2(HFPM pFpmHandle, DWORD* ndevs, DeviceList** devList);
   DLLEXPORT DWORD  WINAPI FPMGetDeviceInfoEx2(HFPM pFpmHandle, DWORD structureType, void* info);
   DLLEXPORT DWORD  WINAPI FPMResetExtractionModule2(HFPM pFpmHandle, DWORD imageWidth, DWORD imageHeight, DWORD dpi);
   DLLEXPORT DWORD WINAPI FPMGetmatchingScore2(HFPM pFpmHandle, BYTE* min1, BYTE* min2, DWORD* score);
////////////////////////////////////////////////////


#endif//_FPM_MULTI_PORT_SUPPORT

#ifdef __cplusplus
};
#endif


#endif
