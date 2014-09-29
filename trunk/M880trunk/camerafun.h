#ifndef _CAMERAFUN_H
#define _CAMERAFUN_H

#include <stdio.h>
#include <string.h>
#include <minigui/common.h>

#define PICTURE_FILENAME "/mnt/ramdisk/picture.jpg"
int cameraflag;

int GrabPicture (char* filename);
int CameraOpen(int size);
void SetCameraParameter(int bright, int contrast, int quality, int scene, int size);
void CameraClose(void);

void ShowCapturePic(HDC hdc, int left, int top, int width, int height);
void InitCameraLogicParam(void);

#endif
