#include <stdlib.h>
#include "net.h"
#include "cim.h"
#include "ov7660.h"
#include "config.h"
#include "exfun.h"
#include "options.h"

void camera_open(void)
{
/*	cim_open(IMG_WIDTH, IMG_HEIGHT, IMG_BPP);
	sensor_open(IMG_WIDTH, IMG_HEIGHT);*/
	gCameraBuf=MALLOC(FRAMESIZE);
}

void camera_close(void)
{
/*	sensor_close();
	cim_close();*/
	FREE(gCameraBuf);
}

void CaptureImgAndProcess(U32 pin, U8 method)
{
	if(gOptions.CameraFunOn)
	{
		if(gOptions.VoiceOn)
			ExPlayVoice(VOICE_CAMERACLICK);
		else
			ExBeep(2);						
	}
	
	//Take a photo by camera
	if(cim_read(gCameraBuf, FRAMESIZE)>0)
	{
		switch(method)
		{
		case METHOD_AUTHSERVER:
			SendPhotoToAuthServer(pin, gCameraBuf, IMG_WIDTH, IMG_HEIGHT, IMG_BPP, 1);
			break;
		case METHOD_SAVETODISK:
			break;
		}
	}
}

