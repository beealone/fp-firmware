#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <minigui/common.h>
#include <minigui/gdi.h>

#include "camerafun.h"
#include "options.h"
#include "libcam.h"
#include "exfun.h"
#include "flashdb.h"

#define video_width gOptions.VideoWidth
#define video_height gOptions.VideoHeight
#define VIDEODEVICE "/dev/video0"
#define VIDEOSIZE	1024
#define VIDEO_PALETTE_JPEG  21

char *videoIn=NULL;

int GrabPicture (char* filename)
{
	int i=0;

	if (filename==NULL) {
		return 1;
	}

	if (cameraflag < 0) {
		cameraflag = CameraOpen(0);
		if (cameraflag < 0){
			return 1;
		}
	}
#if 0
	unsigned int s_msec, e_msec;	
	s_msec=GetTickCount();
	printf("begin grab:%d\n", s_msec);
#endif
#ifndef ZEM600
	int ret;
	while((ret = grab(videoIn, filename)) <= 0)
	{
		if(++i>30)
		{
			ret = 1;
			break;
		}
	}
	sync();
#else
	i=grab_current((void*)videoIn,filename);
#endif

#if 0
	e_msec=GetTickCount();
	printf(">>>grab time:%d\n", (e_msec-s_msec));
#endif
	return i;
}

int CameraOpen(int size)
{
	int i;

	cameraflag = -1;
	if (videoIn==NULL) {
		videoIn = malloc(VIDEOSIZE);
		if (videoIn==NULL) {
			return -1;
		}
		memset((void*)videoIn, 0, VIDEOSIZE);
	}
	printf("begin init camera\n");
#ifdef ZEM600
	cameraflag = InitCamera((void*)videoIn, VIDEODEVICE, video_width, video_height, 0, 1, VIDEOSIZE);
#else
	cameraflag = InitCamera((void*)videoIn, VIDEODEVICE, video_width, video_height, VIDEO_PALETTE_JPEG, 1);
#endif
	printf("init camera  ret %d, %d, %d\n", cameraflag, video_width, video_height);
	write_tracemsg("open camera");

	if (cameraflag >= 0) {
		write_tracemsg("opened camera");
		cameraflag=1;
#ifdef FACE
		if(gOptions.FaceFunOn){
			spcaSetAutoExpo(videoIn, 0);
		}
#else
		spcaSetAutoExpo(videoIn, 1);                           //曝光模式
#endif
		for(i=0; i<4; i++){
			GrabPicture(PICTURE_FILENAME);
		}
#ifdef FACE
		if (gOptions.FaceFunOn==1){
			SetCameraLed(videoIn,0,0);
			DelayMS(50);
			SetCameraLed(videoIn,1,0);
			DelayMS(10);
			SetCameraLed(videoIn,2,1);
		}
#endif
	} else if (videoIn){
		free(videoIn);
		videoIn=NULL;
	}
	return cameraflag;
}

void SetCameraParameter(int bright, int contrast, int quality, int scene, int size)
{
	if (cameraflag < 0) {
		cameraflag = CameraOpen(0);
		if (cameraflag < 0) {
			return;
		}
	}

#ifdef ZEM600
	if (bright > 0){
		SetBrightness((void*)videoIn, (__u8)(bright*1.5));                  //亮度
	}
	if (contrast > 0){
		SetContrast((void*)videoIn, (__u8)(contrast*1.5));                  //对比度
	}
	if (quality < 3 && quality > 0){
		Setquality((void*)videoIn,(__u8)(5-quality*2));
	}
#else
	if (bright > 0){
		SetBrightness((void*)videoIn, (__u8)bright);                  //亮度
	}
	if (contrast > 0){
		SetContrast((void*)videoIn, (__u8)contrast);                  //对比度
	}
	if (quality < 3 && quality >= 0){
		if (size == 0){
			Setquality((void*)videoIn, (__u8)(5-quality*2));      //图像质量
		} else {
			Setquality((void*)videoIn, (__u8)(4-quality));
		}
	}
#endif
	if (scene < 1) {
		spcaSetLightMode((void*)videoIn, (__u8)scene);                //环境
	}
}

extern int i353;
void CameraClose(void)
{
	if ((cameraflag == 1) && videoIn) {
		CloseCamera((void*)videoIn);
		free(videoIn);
		videoIn=NULL;
		cameraflag = 0;
	}
}

void ShowCapturePic(HDC hdc, int left, int top, int width, int height)
{
	BITMAP myvideo;

	if (cameraflag < 0) {
		if (CameraOpen(0) < 0){
			return;
		}
	}
#ifdef	ZEM600
	if (grab_continue(videoIn, PICTURE_FILENAME) > 1) {
		LoadBitmap(hdc, &myvideo, PICTURE_FILENAME);
		if (i353 == 2){
			RotateScaledBitmapVFlip(hdc, &myvideo, -10, 10, 270*64, gOptions.LCDWidth*0.66, gOptions.LCDHeight*0.62);
		}else {
			if (!gOptions.isRotateCamera)
			{
				FillBoxWithBitmap(hdc, left, top, width, height, &myvideo);
			}
			else
			{
				if(gOptions.EuropeDevice)
				{
					RotateScaledBitmapVFlip(hdc, &myvideo, -10, 10, -90*64, gOptions.LCDWidth*0.66, gOptions.LCDHeight*0.66);
				}
				else if (gOptions.RotateDev)
				{
					RotateScaledBitmapVFlip(hdc, &myvideo, -10, 10, 270*64, gOptions.LCDWidth*0.66, gOptions.LCDHeight*0.66);

				}
				else
				{
					RotateScaledBitmapVFlip(hdc, &myvideo, -10, 10, 90*64, gOptions.LCDWidth*0.66, gOptions.LCDHeight*0.62);
				}
			}
		}
		UnloadBitmap(&myvideo);
	}
#else
	if (GrabPicture(PICTURE_FILENAME) != 1)
	{
		LoadBitmap(hdc, &myvideo, PICTURE_FILENAME);
		if (!gOptions.isRotateCamera)
		{
			FillBoxWithBitmap(hdc, left, top, width, height, &myvideo);
		}
		else
		{
			if(gOptions.EuropeDevice)
			{
				RotateScaledBitmapVFlip(hdc, &myvideo, -10, 10, -90*64, gOptions.LCDWidth*0.66, gOptions.LCDHeight*0.66);
			}
			else if (gOptions.RotateDev)
			{
				RotateScaledBitmapVFlip(hdc, &myvideo, -10, 10, 270*64, gOptions.LCDWidth*0.66, gOptions.LCDHeight*0.66);

			}
			else
			{
				RotateScaledBitmapVFlip(hdc, &myvideo, -10, 10, 90*64, gOptions.LCDWidth*0.66, gOptions.LCDHeight*0.66);
			}
		}
		UnloadBitmap(&myvideo);
	}
#endif	
}

void InitCameraLogicParam(void)
{
	if (gOptions.CameraOpen) {

#ifdef	ZEM600		
		printf("Camera Ver: %s\n", get_version_info());
		printf("Camera Hardware Info: %s\n", get_hardware_info());
#else
		if (CameraOpen(0) > 0){
			CameraClose();
		}
#endif
		if (CameraOpen(0) > 0) {
			SetCameraParameter(gOptions.CameraBright, gOptions.CameraContrast, gOptions.PicQuality, gOptions.CameraScene, 0);
		}
	}

	if(gOptions.CameraOpen || (gOptions.LockFunOn&LOCKFUN_ADV) || gOptions.AttUseTZ){
		gOptions.FaceFunOn=0;
	}
#ifdef FACE
	if(gOptions.FaceFunOn) {
		CameraOpen(0);
	}
#endif


}
