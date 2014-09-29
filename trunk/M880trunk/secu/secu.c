/*************************************************************
 *
 * Author :      SecuGen Corporation
 * Description : FPLibTest main.c source code module
 * Copyright(c): 2007 SecuGen Corporation, All rights reserved
 * History : 
 * date        person   comments
 * ======================================================
 *
 *
 *************************************************************/

#include "fplib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>


int image_quality=0;

int init_secu(int *x, int *y)
{
   long err;
   DeviceInfoParam2 deviceInfo;
   
   printf(  "SecuGen init\n");

   ///////////////////////////////////////////////
   // Initialize SecuGen Library
   err = FPMInitLibrary(CN_FDU02);
   if (err != ERROR_NONE)
   {
   	printf("ERROR - Unable to Initlalize SecuGen Library\n");
   	return 0;
   }
   printf("\nFPMInitLibrary returned: %ld\n",err);

   ///////////////////////////////////////////////
   // OpenDevice()
   printf("\nCall FPMOpenDevice(AUTO_DETECT)\n");
   err = FPMOpenDevice(AUTO_DETECT);
   printf("FPMOpenDevice returned : [%ld]\n\n",err);
  if (err == ERROR_NONE)
  {
	    // setBrightness()
        printf("Call FPMsetBrightness(90)\n");
        err = FPMSetBrightness(90);
        printf("FPMSetBrightness returned : [%ld]\n\n",err);
       

        ///////////////////////////////////////////////
        // setGain()
       // printf("Call FPMSetGain(1)\n");
        err = FPMSetGain(1);
       // printf("FPMSetGain returned : [%ld]\n\n",err);        


        ///////////////////////////////////////////////
        // getDeviceInfo()
       // printf("Call FPMGetDeviceInfo()\n");
		err = FPMGetDeviceInfoEx(SGST_DEV_INFO2,(void*) &deviceInfo);
       // printf("FPMGetDeviceInfoEx returned : [%ld]\n",err);
		if (err == ERROR_NONE)
		{
			printf("\tdeviceInfo.DeviceID   : %ld\n", deviceInfo.DeviceID);
			printf("\tdeviceInfo.DeviceSN   : %s\n",  deviceInfo.DeviceSN);
			printf("\tdeviceInfo.ComPort    : %ld\n", deviceInfo.ComPort);
			printf("\tdeviceInfo.ComSpeed   : %ld\n", deviceInfo.ComSpeed);
			printf("\tdeviceInfo.ImageWidth : %ld\n", deviceInfo.ImageWidth);
			printf("\tdeviceInfo.ImageHeight: %ld\n", deviceInfo.ImageHeight);
			printf("\tdeviceInfo.Contrast   : %ld\n", deviceInfo.Contrast);
			printf("\tdeviceInfo.Brightness : %ld\n", deviceInfo.Brightness);
			printf("\tdeviceInfo.Gain       : %ld\n", deviceInfo.Gain);
			printf("\tdeviceInfo.ImageDPI   : %ld\n", deviceInfo.ImageDPI);
			printf("\tdeviceInfo.FWVersion  : %04X\n",deviceInfo.FWVersion);
		}
		*x= deviceInfo.ImageWidth;
		*y= deviceInfo.ImageHeight;
		printf("\n");
	     // getImage() - Fingerprint Capture
        //printf("\nCall FPMSetLedOn(true)\n");
        err = FPMSetLedOn(TRUE);
	}
}

void close_secu(void)
{
    long err;
	err = FPMExitLibrary();
	if (err != ERROR_NONE)
	{
		printf("ERROR - Unable to Close SecuGen Library\n");
	}
	printf("\nFPMExitLibrary returned: %ld\n",err);
}




// 0=fail 1=ok

/*
int getimagefromsecu(char * buffer)
{
	long err;
	static int   quality=0;
	pid_t    pid =-1;
    static	int  status=0;


	if(status ==2)
	{
		status =0;
		printf("2222------\n");
		return quality;
	}

	if(status ==1)
	{
		printf("1111----\n");
		return 0;
	}
	

	quality = 0;
	pid = fork();
	if(pid==0)	//child
	{    
		status =1;		//reading 
		
		err = FPMGetImage(buffer);
		printf("FPMGetImage returned : [%ld]\n\n",err);
		if(err == ERROR_NONE)
		{
		    printf("\nCall FPMGetImageQuality()\n");
		    err = FPMGetImageQuality(260, 300, buffer, &quality);
			if(err == ERROR_NONE)
			{
		   	   	printf("FPMGetImageQuality returned : [%ld]\n",err);
			    printf("Image quality : [%ld]\n\n",quality);
			}
		}
		
	
		printf("11111111---22222\n");
		status=2;		//reading over
		exit(1);
	}
	return 0;
}
*/


int getimagefromsecu(char * buffer)
{
	long err,x,y;
	 int   quality=0;
	
		err = FPMGetImage(buffer);
	//	printf("FPMGetImage returned : [%ld]\n\n",err);
		if(err == ERROR_NONE)
		{
		   // printf("\nCall FPMGetImageQuality()\n");
		    err = FPMGetImageQuality(260, 300, buffer, &quality);
	//		if(err == ERROR_NONE)
	//		{
	//	   	    printf("FPMGetImageQuality returned : [%ld]\n",err);
	//		    printf("Image quality : [%ld]\n\n",quality);
	//		}
			return quality;
		}
		else
		{
			close_secu();
			init_secu(&x,&y);
			return (int)err;
		}
		return 0;
	
}


// ---------------------------------------------------------------- main() ---
int test_secu(void) 
{

   long err;
   DWORD quality;
   BYTE *imageBuffer1;
   FILE *fp = NULL;
	DeviceInfoParam2 deviceInfo;
//   DWORD score;

   printf("\n-------------------------------------\n");
   printf(  "SecuGen FPLIB Test\n");
   printf(  "-------------------------------------\n");

   ///////////////////////////////////////////////
   // Initialize SecuGen Library
   err = FPMInitLibrary(CN_FDU02);
   if (err != ERROR_NONE)
   {
   	printf("ERROR - Unable to Initlalize SecuGen Library\n");
   	return 0;
   }

   printf("\nFPMInitLibrary returned: %ld\n",err);

   ///////////////////////////////////////////////
   // OpenDevice()
   printf("\nCall FPMOpenDevice(AUTO_DETECT)\n");
   err = FPMOpenDevice(AUTO_DETECT);
   printf("FPMOpenDevice returned : [%ld]\n\n",err);
  if (err == ERROR_NONE)
  {
	
  
        ///////////////////////////////////////////////
        // setLedOn(true)
        printf("Press <Enter> to turn fingerprint sensor LEDs on >> ");
        getc(stdin);
        printf("Call FPMSetLedOn(TRUE)");
        err = FPMSetLedOn(TRUE);
        printf("FPMSetLedOn returned : [%ld]\n", err);


        ///////////////////////////////////////////////
        // setLedOn(false)
        printf("Fingerprint Sensor LEDS should now be illuminated.\n\n");
        printf("Press <Enter> to turn fingerprint sensor LEDs off >> ");
        getc(stdin);
        printf("Call FPMSetLedOn(FALSE)");
        err = FPMSetLedOn(FALSE);
        printf("FPMSetLedOn returned : [%ld]\n\n", err);


        ///////////////////////////////////////////////
        ///////////////////////////////////////////////
        printf("Fingerprint Sensor LEDS should now be off.\n");
        printf("Press <Enter> to continue >> ");
        getc(stdin);


        ///////////////////////////////////////////////
        // setBrightness()
        printf("Call FPMsetBrightness(90)\n");
        err = FPMSetBrightness(90);
        printf("FPMSetBrightness returned : [%ld]\n\n",err);
       

        ///////////////////////////////////////////////
        // setGain()
        printf("Call FPMSetGain(1)\n");
        err = FPMSetGain(1);
        printf("FPMSetGain returned : [%ld]\n\n",err);        


        ///////////////////////////////////////////////
        // getDeviceInfo()
        printf("Call FPMGetDeviceInfo()\n");
	err = FPMGetDeviceInfoEx(SGST_DEV_INFO2,(void*) &deviceInfo);
        printf("FPMGetDeviceInfoEx returned : [%ld]\n",err);
	if (err == ERROR_NONE)
	{
		printf("\tdeviceInfo.DeviceID   : %ld\n", deviceInfo.DeviceID);
		printf("\tdeviceInfo.DeviceSN   : %s\n",  deviceInfo.DeviceSN);
		printf("\tdeviceInfo.ComPort    : %ld\n", deviceInfo.ComPort);
		printf("\tdeviceInfo.ComSpeed   : %ld\n", deviceInfo.ComSpeed);
		printf("\tdeviceInfo.ImageWidth : %ld\n", deviceInfo.ImageWidth);
		printf("\tdeviceInfo.ImageHeight: %ld\n", deviceInfo.ImageHeight);
		printf("\tdeviceInfo.Contrast   : %ld\n", deviceInfo.Contrast);
		printf("\tdeviceInfo.Brightness : %ld\n", deviceInfo.Brightness);
		printf("\tdeviceInfo.Gain       : %ld\n", deviceInfo.Gain);
		printf("\tdeviceInfo.ImageDPI   : %ld\n", deviceInfo.ImageDPI);
		printf("\tdeviceInfo.FWVersion  : %04X\n", deviceInfo.FWVersion);
	}
	printf("\n");



        ///////////////////////////////////////////////
        // getImage() - Fingerprint Capture
        printf("\nCall FPMSetLedOn(true)\n");
        err = FPMSetLedOn(TRUE);
        printf("FPMSetLedOn returned : [%ld]\n",err);
        printf("Capture. Please place finger on sensor with LEDs on and press <ENTER> ");
        imageBuffer1 = (BYTE*) MALLOC(deviceInfo.ImageHeight*deviceInfo.ImageWidth); 
        getc(stdin);
        printf("\nCall FPMGetImage()\n");
        err = FPMGetImage(imageBuffer1);
        printf("FPMGetImage returned : [%ld]\n\n",err);
        if (err == ERROR_NONE)
        {
	  fp = fopen("fplibtest_c_finger.raw","wb");
	  fwrite (imageBuffer1 , sizeof (BYTE) , deviceInfo.ImageWidth*deviceInfo.ImageHeight , fp);
          fclose(fp);
	}


        ///////////////////////////////////////////////
        // getImageQuality()
        quality = 0;
        printf("\nCall FPMGetImageQuality()\n");
        err = FPMGetImageQuality(deviceInfo.ImageWidth, deviceInfo.ImageHeight, imageBuffer1, &quality);
        printf("FPMGetImageQuality returned : [%ld]\n",err);
        printf("Image quality : [%ld]\n\n",quality);

/*
*/
        ///////////////////////////////////////////////
        // closeDevice()
        printf("\nCall FPMCloseDevice()\n");
        err = FPMCloseDevice();
        printf("FPMCloseDevice returned : [%ld]\n",err);
		
        imageBuffer1 = NULL;
		
  }
        ///////////////////////////////////////////////
        // Close SecuGen Library
	err = FPMExitLibrary();
	if (err != ERROR_NONE)
	{
		printf("ERROR - Unable to Close SecuGen Library\n");
	}
	printf("\nFPMExitLibrary returned: %ld\n",err);

  
  return 0;
}

