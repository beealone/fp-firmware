#ifndef LIBCAM_H
#define LIBCAM_H

#include <stdio.h>
#include <string.h>
#include <linux/types.h>

#ifdef ZEM600
	int InitCamera(void *vd,const char *device,int width,int height,int format,int grabmethod,const int size);
#else
	int InitCamera(void *vd,const char *device,int width,int height,int format,int grabmethod);
#endif

/*capture a new picture*/
int grab_current(void *vd, const char *filename);

/*capture a picture for video setting*/
int grab_continue(void *vd, const char *filename);

char *get_version_info(void);
char *get_hardware_info(void);

void CloseCamera(void *vd);

void spcaSetAutoExpo(void *vd, __u8);
void spcaSetLightMode(void *vd, __u8 lightmode);
void SetBrightness(void *vd, __u8 bright);
void SetContrast(void *vd, __u8 contrast);
void Setquality(void *vd, __u8 quality);
int grab(void *vd, const char *filename);

#endif
