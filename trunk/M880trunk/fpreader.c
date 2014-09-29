/************************************************

  ZEM 300 iClock-888

  main.c Main source file

  Copyright (C) 2006-2010, ZKSoftware Inc.
 *************************************************/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/vfs.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>

#include "arca.h"
#include "exfun.h"
#include "msg.h"
#include "flashdb.h"
#include "utils.h"
#include "options.h"
#include "rs_485reader.h" 
#include "fpreader.h"


void set_verification_type(WPARAM wParam)
{
	if((gOptions.Reader485FunOn > 0) && (gOptions.Reader485On > 0) &&  (MSG_INBIO_COMM_CMD == wParam))
	{
		SaveSlaveDevID(TYPE_SLAVE_ID);
	}
	else
	{
		SaveSlaveDevID(0);
	}
}

int is_slavedevice_verification(void)
{
	return ((gOptions.Reader485FunOn > 0) && (gOptions.Reader485On > 0) && (g_iSlaveID > 0)) ? 1:0;
}

int set_fppic_display_type(int type, WPARAM wParam)
{
	int ori_type = gOptions.FpSelect;

	if(gOptions.Reader485On && Is485ReaderMaster())
	{
		/*inbiocomm 进程只是传递指纹模板，而没有图像，所以在界面上显示不了*/
		if(MSG_INBIO_COMM_CMD == wParam)
		{
			gOptions.FpSelect = type;//登记，比对不显示
		}
	}
	return ori_type;
}

extern BYTE FingerTemplate[10240*2];
int IdentifyFinger(char *InputPin, U32 PIN, BYTE *Temp, BYTE* Image, int RS485FPFlag);
int identify_finger_by_template(char *InputPin, unsigned int PIN, WPARAM wParam, LPARAM lParam)
{
	if(!gOptions.Reader485On && !Is485ReaderMaster() && (MSG_INBIO_COMM_CMD == wParam))
	{
		return -1;
	}

	PInbioCommData SensorInfo = (PInbioCommData)lParam;
	memcpy(FingerTemplate , SensorInfo->SendData+sizeof(SensorInfo->CardNo),MAX_INBIO_SEND_DATA_LEN-sizeof(SensorInfo->CardNo));
	
	set_fpreader_msg_flag(wParam);
	int i = IdentifyFinger(InputPin, PIN, FingerTemplate,NULL, 1);
	if(i>0)
	{
		//printf("Info:finger success.[%s,%d]\n", __FILE__,__LINE__);
		return i;
	}
	//printf("Info:finger failed.[%s,%d]\n", __FILE__,__LINE__);
	return 0;
}

int fpread_msg_flag=0;
int set_fpreader_msg_flag(WPARAM wParam)
{
	if(gOptions.Reader485On && Is485ReaderMaster() && (MSG_INBIO_COMM_CMD == wParam))
	{
		fpread_msg_flag=TRUE;
	}
	else
	{
		fpread_msg_flag=FALSE;
	}
	return fpread_msg_flag;
}

void return_msg_to_fpreader(int type)
{
	if(!fpread_msg_flag) return;

	printf(">>>>>>>>>>>>>>>>>>return_msg_to_fpreader\n");
	if (type){
		RS485_READER_RETURN_SUCCESS();
	}
	else {
		ReturnToRS485ReaderFailed();
	}
	fpread_msg_flag = FALSE;
	return;
}


#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

