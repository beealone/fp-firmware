#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include <sys/ioctl.h>
#include<stdio.h>

#include "opp03.h"

static char *filedev = "/dev/opp";
static int handle = -1;
static int hdl = -1;

int opendev(void)
{
	if(handle>0) close(handle);
	handle = open(filedev, O_RDWR);
	return handle;
}

void closedev(void)
{
        close(handle);
}

HRESULT
OPP_FPD_Initialize (FPD_Para Reg)
{
  printf ("FPD initializing...\n");
  if (hdl <= 0)
	hdl = opendev();
  if (ioctl (hdl, IO_OPP_FPD_Initialize, &Reg) < 0)
    {
      printf ("IO_OPP_FPD_Initialize error\n");
      return OPP_ERROR;
    }
  return OPP_OK;
}

#ifdef OSP04
HRESULT
OSP04_Initialize (UINT8 brightness)
{
  printf ("OSP_Initialize start\n");
//  int hdl = getdev (1);
  if (hdl <= 0)
        hdl = opendev();
  unsigned int val = brightness;
  if (ioctl(hdl, IO_OSP_SetBrightness, &val) < 0)
    {
      printf ("IO_OPP_Initialize error\n");
      return OPP_ERROR;
    }
  printf("finished Initialize\n");
  return OPP_OK;
}
#else
HRESULT
OPP_Initialize (UINT16 brightness, UINT8 gain, UINT8 contrast, UINT8 nTimeOut, UINT8 nQuality, UINT8 nDetectInterval)
{
  printf ("OPP_Initialize start\n");
  if (hdl <= 0)
	hdl = opendev();

  OPP_Initialize_Para reg = {.brightness = 0,.gain = 0,.contrast = 0 };
  reg.brightness = brightness;
  reg.gain = gain;
  reg.contrast = contrast;
  if (ioctl(hdl, IO_OPP_Initialize, &reg) < 0)
    {
      printf ("IO_OPP_Initialize error\n");
      return OPP_ERROR;
    }

  OPP_LiveImage_Para Reg = {.nTimeOut = 0,.nQuality = 0, .nDetectInterval = 0 };
  Reg.nTimeOut = nTimeOut;
  Reg.nQuality = nQuality;
  Reg.nDetectInterval = nDetectInterval;
  if(ioctl(hdl, IO_OPP_Set, &Reg) < 0)
  {
      printf ("IO_OPP_Set error\n");
      return OPP_ERROR;
  }
  printf("finished Initialize\n");
  return OPP_OK;
}
#endif

HRESULT
OPP_GetImage (UINT8 * pBuff)
{
  if (hdl <= 0)
	hdl = opendev();

  if (read (hdl, pBuff, 400 * 300) < 0)
    return OPP_ERROR;

  return OPP_OK;
}

HRESULT
OPP_GetLiveImage (UINT8 *pBuff, UINT32 Len)
{
  int rel;
  if (hdl <= 0)
	hdl = opendev();
  rel = read(hdl, pBuff, Len);
  if(rel==Len)
  {
//	printf("Get live image successful!\n");
  	return OPP_OK;
  }
  else
  {
//	printf("Get live image failed!\n");
	return OPP_ERROR;
  }		
}
