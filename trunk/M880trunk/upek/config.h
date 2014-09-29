#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "arca.h"

/*
 * Select one of the sensors
 */
#define CONFIG_OV7660       1

/*
 * Define the image size
 */

#define IMG_BPP		16

int IMG_WIDTH;          //320
int IMG_HEIGHT;         //240

int FRAMESIZE; 		//IMG_WIDTH*IMG_HEIGHT*IMG_BPP/8

#define METHOD_AUTHSERVER	1
#define METHOD_SAVETODISK	2

char *gCameraBuf; 

void camera_open(void);
void camera_close(void);
void CaptureImgAndProcess(U32 pin, U8 method);

#endif /* __CONFIG_H__ */

