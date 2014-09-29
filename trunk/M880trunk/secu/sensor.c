/*************************************************
                                           
 ZEM 200                                          
                                                    
 sensor.c
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include "arca.h"
#include "options.h"
#include "sensor.h"
#include "zkfp.h"
#include "lcm.h"
#include "exfun.h"
#include "utils.h"
#include "hv7131.h"
#include "cim.h"
#include "main.h"



//secugen width =260 height =300
int ImageWidth=CMOS_WIDTH; 			//当前采集区域的宽度
int ImageHeight=CMOS_HEIGHT; 		//当前采集区域的高度

#define CORRECT_NONE    	0
#define CORRECT_REVERSE 	1
#define CORRECT_ROTATION 	2
#define MARGIN 			    8	

unsigned int img_bpp = 8; /* 8/16/32 */

static int fd_sensor=-1;

//init sensor jz_secu
void InitSensor(int LeftLine, int TopLine, int Width, int Height, int FPReaderOpt)
{      

	long x,y;
		
	gOptions.OImageWidth  = Width;
	gOptions.OImageHeight = Height;
	gOptions.ZF_WIDTH  = Width;
	gOptions.ZF_HEIGHT = Height;
	gOptions.CPY[0] = 0;
	gOptions.CPX[2] = 0; 

	init_secu(&x,&y);		//get width & height; ledon ;set brighrness ;set gain
	printf("x = %ld, y = %ld\n", x, y);
	fd_sensor=7;
	/*
	fd_sensor=sensor_open(LeftLine, TopLine, Width, Height, FPReaderOpt);
        cim_open(Width, Height, img_bpp);	
        */
 
}

void FreeSensor(void)
{     

	close_secu();
/*
	cim_close();
        sensor_close();
        */
}

void FlushSensorBuffer(void)
{
	//nothing to do
}

extern int gsecu_match_score;
int CaptureSensor(char *Buffer, BOOL Sign, PSensorBufInfo SensorBufInfo)
{
	U32 framesize=ImageWidth*ImageHeight;
	U32 len;
	static int finger_present=0;	
	int score;

	//DBPRINTF("READING IMAGE...\n");
		
	score = getimagefromsecu(Buffer);


	if(score<=gsecu_match_score)
	{
		finger_present=0;
	}
	else if(finger_present==0)
	{
		ExBeep(1);	//ccc
		finger_present=1;
		len = framesize;
		if(SensorBufInfo)
		{
			//image buffer info
			SensorBufInfo->SensorNo=255;
			SensorBufInfo->RawImgLen=len;
			SensorBufInfo->RawImgPtr=Buffer;
			SensorBufInfo->DewarpedImgLen=len;
			SensorBufInfo->DewarpedImgPtr=Buffer;
		}
		return TRUE;
	}
	return FALSE;
}

#if 0
int CaptureSensor_t(char *Buffer, BOOL Sign, PSensorBufInfo SensorBufInfo)
{
	U32 framesize=ImageWidth*ImageHeight;
	U32 len;
	//static int i=0;
	//char filename[128];

	//DBPRINTF("READING IMAGE...\n");





	
	len=cim_read(Buffer, framesize);
	if(len>0)
	{
		FilterRGB(Buffer, ImageWidth, ImageHeight);
		if(SensorBufInfo)
		{
			//image buffer info
			SensorBufInfo->SensorNo=255;
			SensorBufInfo->RawImgLen=len;
			SensorBufInfo->RawImgPtr=Buffer;
			SensorBufInfo->DewarpedImgLen=len;
			SensorBufInfo->DewarpedImgPtr=Buffer;
		}
		
		/*if(i<500)
		{
			sprintf(filename, "/mnt/removable/f_%d.bmp", i++);
			if((i%10)==0)
			{
				write_bitmap(filename, Buffer, ImageWidth, ImageHeight);
				printf("%s\n", filename);
			}
		}*/

		return TRUE;
	}
	else
		return FALSE;
}
#endif

int CalcThreshold(int NewT)
{
	if(NewT<10) return NewT<0?0:NewT*3;
	else return NewT>50? 70: NewT+20;
}

int CalcNewThreshold(int Thr)
{
	if(Thr<30) return Thr<0?0:Thr/3;
	else return Thr>70? 50: Thr-20;
}

#define P_O_WIDTH(s) s[0]
#define P_O_HEIGHT(s) s[1]
#define P_CP0_X(s) ((s[2]))
#define P_CP0_Y(s) ((s[3]))
#define P_CP1_X(s) ((s[4]))
#define P_CP1_Y(s) ((s[5]))
#define P_CP2_X(s) ((s[6]))
#define P_CP2_Y(s) ((s[7]))
#define P_CP3_X(s) ((s[8]))
#define P_CP3_Y(s) ((s[9]))
#define P_TP0_X(s) s[10]
#define P_TP0_Y(s) s[11]
#define P_TP1_X(s) s[12]
#define P_TP1_Y(s) s[13]
#define P_TP2_X(s) s[14]
#define P_TP2_Y(s) s[15]
#define P_TP3_X(s) s[16]
#define P_TP3_Y(s) s[17]
#define P_TP4_X(s) s[18]
#define P_TP4_Y(s) s[19]
#define FP_WIDTH(s) s[20]
#define FP_HEIGHT(s) s[21]

short sizes[22];

//init sensor
int FPBaseInit(char *FingerCacheBuf)
{
	//short sizes[22];
	BYTE params[8];
	
	P_O_WIDTH(sizes)=gOptions.OImageWidth;
	P_O_HEIGHT(sizes)=gOptions.OImageHeight;
	FP_WIDTH(sizes)=gOptions.ZF_WIDTH;
	FP_HEIGHT(sizes)=gOptions.ZF_HEIGHT;
	P_CP2_X(sizes)=(short)gOptions.CPX[0];
	P_CP0_Y(sizes)=(short)gOptions.CPY[0];
	P_CP3_X(sizes)=(short)gOptions.CPX[1];
	P_CP1_Y(sizes)=(short)gOptions.CPY[1];
	P_CP0_X(sizes)=(short)gOptions.CPX[2];
	P_CP2_Y(sizes)=(short)gOptions.CPY[2];
	P_CP1_X(sizes)=(short)gOptions.CPX[3];
	P_CP3_Y(sizes)=(short)gOptions.CPY[3];
	params[0]=1;
	params[1]=50;
	params[2]=65;
	params[3]=110;
	params[4]=0;
	params[5]=255;
#ifdef TESTIMAGE
	P_TP0_X(sizes)=8;
	P_TP0_Y(sizes)=8;
#else
	P_TP0_X(sizes)=180;
	P_TP0_Y(sizes)=138;
#endif	
	if((FingerCacheBuf==NULL) && fhdl) 
		FingerCacheBuf=(char*)fhdl; 
	else if(FingerCacheBuf==NULL) 
		FingerCacheBuf=(char*)MALLOC(1024*1024*2);
	fhdl=BIOKEY_INIT(fd_sensor, (WORD*)sizes, params, (BYTE*)FingerCacheBuf, 0x80);	
	BIOKEY_MATCHINGPARAM(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
	BIOKEY_SETNOISETHRESHOLD(fhdl, gOptions.MaxNoiseThr, gOptions.MinMinutiae, gOptions.MaxTempLen, 500);	
	return (int)fhdl;
}

#define gOHeight gOptions.OImageHeight
#define gOWidth gOptions.OImageWidth
#define gOSize gOptions.OImageWidth*gOptions.OImageHeight
#define NewThr
#ifdef NewThr
#define MIN_THR 30
#define TOP_THR 80
#define INC_THR 20
#else
#define MIN_THR 50
#define TOP_THR 80
#define INC_THR 0
#endif
int LastIndex=0;
int ShowVar=1;
int LastUserID=0;

U32 FingerCount=0;

int GainAutoAdjust(int m1)
{
	if(gOptions.NewFPReader & CORRECT_REVERSE)
	{
		if(m1<150)	//AGC-Auto Gain Control
			IncCMOSGain();
		else if(m1>230)
			DecCMOSGain();
		else
			return TRUE;
	}
	else
	{
		if(m1>230)	//AGC-Auto Gain Control
			DecCMOSGain();
		else if(m1<150)
			IncCMOSGain();
		else
			return TRUE;
	}
	return FALSE;
}

extern TSensorBufInfo SensorBufInfo;

U32 FPTest(char *ImgBuf)
{
    static int LastFingerValue=0, AvgValue=400, AvgDiff=5, TopFingerThreshold=120, HasFingerThreshold=500;
    static int LastHasIndex=0;
    static int LeaveFinger=FALSE;
    int Result=FALSE,v1,v2,v,size=gOSize,m1,m2,NoFingerThreshold, TopFingerThresholdDiff;
    WORD buffer[20];
    BYTE *finger=(BYTE*)gImageBuffer,*prev=(BYTE*)gImageBuffer+size, *pure=(BYTE*)gImageBuffer+2*size,
        *back=(BYTE*)gImageBuffer+4*size,
        *diff=(BYTE*)gImageBuffer+3*size;

    for(v=0;v<size;v++)
    {
        if(gOptions.NewFPReader & CORRECT_REVERSE)
            m1=255-((int)*finger-(int)prev[v]);
        else
            m1=255-((int)prev[v]-(int)*finger);
        if(m1<0) m1=0; else if(m1>255) m1=255;
        diff[v]=(BYTE)m1;
        finger++;
    }
    finger=(BYTE*)gImageBuffer;
    if(gOptions.IsTestMachine)
    {
        if(ShowVar)
        {
            CalcVar(finger, gOWidth, gOHeight, &v1, &m1, 40);
            if(LastIndex>v1)
            {
                if(abs(LastIndex-AvgValue)+10<abs(LastIndex-v1)) ExBeep(1);
            }else
                LastIndex=v1;
            sprintf((char*)buffer,"%4d-%4d",LastIndex,v1);
            LCDWriteStr(0,16-9,(char*)buffer,0);
            AvgValue=v1;
        }
        return 0;
    }

    CalcVar(finger, gOWidth, gOHeight, &v1, &m1, 32);
    GainAutoAdjust(m1);
    CalcVar(diff, gOWidth, gOHeight, &v2, &m2, 32);
    
    if(v1>2000 && v2>400)
    {
        DelayMS(1000*3);
        InitSensor(gOptions.OLeftLine,gOptions.OTopLine,gOptions.OImageWidth,gOptions.OImageHeight, gOptions.NewFPReader);
        DelayMS(3);
        CaptureSensor((char*)prev,ONLY_LOCAL,&SensorBufInfo);
    }
    {
        if(FingerCount++<6) AvgValue=v1;
        if((LastUserID<=0) && (0==LastFingerValue))
        {
            TopFingerThreshold=((AvgValue+gOptions.TopThr)+TopFingerThreshold)/2;
            if(TopFingerThreshold<AvgValue*5/4) TopFingerThreshold=AvgValue*5/4;
            if(TopFingerThreshold>60*AvgValue) TopFingerThreshold=AvgValue*60;
            if(TopFingerThreshold<gOptions.MinThr) TopFingerThreshold= gOptions.MinThr;
            HasFingerThreshold=((AvgValue*9/8+gOptions.TopThr/2)+HasFingerThreshold*3)/4;
            if(HasFingerThreshold<gOptions.MinThr) HasFingerThreshold= gOptions.MinThr;
            if(HasFingerThreshold>TopFingerThreshold) HasFingerThreshold=TopFingerThreshold;
        }
        NoFingerThreshold=(AvgValue*5+HasFingerThreshold*3)/8;
        TopFingerThresholdDiff=AvgDiff*5+100;
        if(TopFingerThresholdDiff<300) TopFingerThresholdDiff=300;
    }
//	printf("v=%08d avg=%4d top=%4d has=%4d\n", v1*10000+v2, AvgValue, TopFingerThreshold, HasFingerThreshold);
    buffer[0]=10;
    buffer[1]=v1; buffer[2]=v2;
    buffer[3]=TopFingerThreshold; buffer[4]=AvgValue;
    buffer[5]=HasFingerThreshold; buffer[6]=NoFingerThreshold;
    buffer[7]=LastFingerValue;
        if((v1<NoFingerThreshold) && !LeaveFinger)
    {
        LeaveFinger=TRUE;
        LastFingerValue=0;
        AvgValue=v1;
        buffer[0]=0;
    }
    else if(LeaveFinger||(LastUserID>0))
        {
        if(((v2>=35+AvgDiff*8) && (v1>TopFingerThreshold || v2>TopFingerThresholdDiff))||
            ((LastUserID<=0) && (v1>TopFingerThreshold*3/2)))
        {
            LeaveFinger=FALSE;
            Result=TRUE;
            AvgValue=(AvgValue*15+v1)/16;
            AvgDiff=(AvgDiff*15+v2)/16;
            v=0;
            if(v1<TopFingerThreshold+50+3*gOptions.IncThr)
            while(v++<gOptions.DelayCount)
            {
                CaptureSensor((char*)diff,ONLY_LOCAL,&SensorBufInfo);
                CalcVar(diff, gOWidth, gOHeight, &v2, &m1, 32);
                if(v2>v1)
                    memcpy(finger, diff, gOptions.OImageWidth*gOptions.OImageHeight);
                if(v2<=v1+gOptions.IncThr/2)
                {
                    break;
                }
                if(v2>v1) v1=v2;
            }
            buffer[0]=512+v;
            LastHasIndex=FingerCount;
        }
        else
        {
            if((LastHasIndex+10<FingerCount) && (LastFingerValue>HasFingerThreshold))
            {
                buffer[0]=2;
                LastFingerValue=0;
            }
            if(((v1<=LastFingerValue*64/64) ||
                (v1<(AvgValue+LastFingerValue*3)/4)) && (LastFingerValue>HasFingerThreshold))
            {
                buffer[0]=256;
                Result=TRUE;
                LeaveFinger=TRUE;
                memcpy(finger,pure,size);
                TopFingerThreshold=LastFingerValue+10;
            }
            else
            {
                if((v1>HasFingerThreshold) || ((v1>LastFingerValue) && (LastFingerValue>0)))
                {
                    buffer[0]=3;
                    memcpy(pure,finger,size);
                    LastFingerValue=v1;
                    LastHasIndex=FingerCount;
                }
                if((v1<LastFingerValue*2))
                {
                    buffer[0]=4;
                    if(AvgValue>v1)
                    {
                        AvgValue=(AvgValue+v1*3)/4;
                        AvgDiff=(AvgDiff+v2*3)/4;
                    }
                    else
                    {
                        AvgValue=(AvgValue*3+v1)/4;
                        AvgDiff=(AvgDiff*3+v2)/4;
                    }
                    if(LastHasIndex!=FingerCount)
                        buffer[0]=5;
                }
                else
                {
                    AvgValue=(AvgValue+v1*3)/4;
                    AvgDiff=(AvgDiff+v2*3)/4;
                }

            }
        }
            if(Result){
                ExBeep(1);
            memcpy(back, finger, size);
                LastFingerValue=0;
                if(AvgValue>TopFingerThreshold) AvgValue=TopFingerThreshold;
        }
    }
        else
        {
            AvgValue=(AvgValue*3+v1)/4;
        AvgDiff=(AvgDiff*3+v2)/4;
    }
        memcpy(prev, finger, size);
    LastUserID=0;
    return Result;
}


int AutoGainImage(int i)
{
	int m1,v1;
	
	while(i--)
	{
		if(CaptureSensor(gImageBuffer,ONLY_LOCAL,&SensorBufInfo))
		{
			CalcVar((BYTE*)gImageBuffer, gOWidth, gOHeight, &v1, &m1, 32);
			if(m1==0) return FALSE;
			if(GainAutoAdjust(m1))
				return TRUE;
		}

	}
	return FALSE;
}

//U32 FPTest (char *gImageBuffer,int gOWidth, int gOHeight)
/*
#define NEWFPREADER 3
U32 FPTest(char *ImgBuf)
{
    static int LastFingerValue=0, AvgValue=400, AvgDiff=5, TopFingerThreshold=120, HasFingerThreshold=500;
    static int LastHasIndex=0;
    static int LeaveFinger=FALSE;
    int i, Result=FALSE,v1,v2,v,size=gOSize,m1,m2,NoFingerThreshold, TopFingerThresholdDiff;
    WORD buffer[20];
    BYTE *finger=(BYTE*)gImageBuffer,*prev=(BYTE*)gImageBuffer+size, *pure=(BYTE*)gImageBuffer+2*size,  *back=(BYTE*)gImageBuffer+4*size,*diff=(BYTE*)gImageBuffer+3*size;

    for(v=0;v<size;v++)
    {
        if(NEWFPREADER & CORRECT_REVERSE)
            m1=255-((int)*finger-(int)prev[v]);
        else
            m1=255-((int)prev[v]-(int)*finger);
        if(m1<0) m1=0; else if(m1>255) m1=255;
        diff[v]=(BYTE)m1;
        finger++;
    }
    finger=(BYTE*)gImageBuffer;

    CalcVar(finger, gOWidth, gOHeight, &v1, &m1, 32);
    GainAutoAdjust(m1);
    CalcVar(diff, gOWidth, gOHeight, &v2, &m2, 32);
    if(v1>2000 && v2>400)
    {
        //DelayMS(1000*3);
        CaptureSensor((char*)diff,ONLY_LOCAL,&SensorBufInfo);
        //CaptureSensor((char*)prev);
    }
    {
        if(FingerCount++<6) AvgValue=v1;
        if((LastUserID<=0) && (0==LastFingerValue))
        {
            TopFingerThreshold=((AvgValue+TOP_THR)+TopFingerThreshold)/2;
            if(TopFingerThreshold<AvgValue*5/4) TopFingerThreshold=AvgValue*5/4;
            if(TopFingerThreshold>60*AvgValue) TopFingerThreshold=AvgValue*60;
            if(TopFingerThreshold<MIN_THR) TopFingerThreshold= MIN_THR;
            HasFingerThreshold=((AvgValue*9/8+TOP_THR/2)+HasFingerThreshold*3)/4;
            if(HasFingerThreshold<MIN_THR) HasFingerThreshold= MIN_THR;
            if(HasFingerThreshold>TopFingerThreshold) HasFingerThreshold=TopFingerThreshold;
        }
        NoFingerThreshold=(AvgValue*5+HasFingerThreshold*3)/8;
        TopFingerThresholdDiff=AvgDiff*5+100;
        if(TopFingerThresholdDiff<300) TopFingerThresholdDiff=300;
    }
	printf("v=%08d avg=%4d top=%4d has=%4d\n", v1*10000+v2, AvgValue, TopFingerThreshold, HasFingerThreshold);
    buffer[0]=10;
    buffer[1]=v1; buffer[2]=v2;
    buffer[3]=TopFingerThreshold; buffer[4]=AvgValue;
    buffer[5]=HasFingerThreshold; buffer[6]=NoFingerThreshold;
    buffer[7]=LastFingerValue;
    if((v1<NoFingerThreshold) && !LeaveFinger)
    {
        LeaveFinger=TRUE;
        LastFingerValue=0;
        AvgValue=v1;
        buffer[0]=0;
    }
    else if(LeaveFinger||(LastUserID>0))
        {
        if(((v2>=35+AvgDiff*8) && (v1>TopFingerThreshold || v2>TopFingerThresholdDiff))||
            ((LastUserID<=0) && (v1>TopFingerThreshold*3/2)))
        {
            LeaveFinger=FALSE;
            Result=TRUE;
            AvgValue=(AvgValue*15+v1)/16;
            AvgDiff=(AvgDiff*15+v2)/16;
            v=0;
            if(v1<TopFingerThreshold+50+3*INC_THR)
            while(v++<3)
            {
               // CaptureSensor((char*)diff);
		printf("capture\n");
        	CaptureSensor((char*)diff,ONLY_LOCAL,&SensorBufInfo);
                CalcVar(diff, gOWidth, gOHeight, &v2, &m1, 32);
                if(v2>v1)
                    memcpy(finger, diff, gOSize);
                if(v2<=v1+INC_THR/2)
                {
                    break;
                }
                if(v2>v1) v1=v2;
            }
            buffer[0]=512+v;
            LastHasIndex=FingerCount;
        }
        else
        {
            if((LastHasIndex+10<FingerCount) && (LastFingerValue>HasFingerThreshold))
            {
                buffer[0]=2;
                LastFingerValue=0;
            }
            if(((v1<=LastFingerValue*64/64) ||
                (v1<(AvgValue+LastFingerValue*3)/4)) && (LastFingerValue>HasFingerThreshold))
            {
                buffer[0]=256;
                Result=TRUE;
                LeaveFinger=TRUE;
                memcpy(finger,pure,size);
		printf("LastFigerValue1: %d\n",LastFingerValue);
                TopFingerThreshold=LastFingerValue+10;
            }
            else
            {
		printf("LastFigerValue: %d\n",LastFingerValue);
                if((v1>HasFingerThreshold) || ((v1>LastFingerValue) && (LastFingerValue>0)))
                {
                    buffer[0]=3;
                    memcpy(pure,finger,size);
                    LastFingerValue=v1;
                    LastHasIndex=FingerCount;
                }
                if((v1<LastFingerValue*2))
                {
                    buffer[0]=4;
                    if(AvgValue>v1)
                    {
                        AvgValue=(AvgValue+v1*3)/4;
                        AvgDiff=(AvgDiff+v2*3)/4;
                    }
                    else
                    {
                        AvgValue=(AvgValue*3+v1)/4;
                        AvgDiff=(AvgDiff*3+v2)/4;
                    }
                    if(LastHasIndex!=FingerCount)
                        buffer[0]=5;
                }
                else
                {
                    AvgValue=(AvgValue+v1*3)/4;
                    AvgDiff=(AvgDiff+v2*3)/4;
                }

            }
        }
            if(Result){
                memcpy(back, finger, size);
                LastFingerValue=0;
                if(AvgValue>TopFingerThreshold) AvgValue=TopFingerThreshold;
        }
    }
        else
        {
            AvgValue=(AvgValue*3+v1)/4;
        AvgDiff=(AvgDiff*3+v2)/4;
    }
        memcpy(prev, finger, size);
    LastUserID=0;
    return Result;
}

*/
#define BSIZE 16

int CalcVar(BYTE *img, int width, int height, int *var, int *mean, int FrameWidth)
{
	int msum, vsum, i, j, bc, sum, m, n, v,t, bsize;
	BYTE *p;
	bsize=BSIZE*BSIZE;
	msum=0;bc=0;vsum=0;
	width-=FrameWidth*2;
	height-=FrameWidth*2;
	for(i=0;i<height/BSIZE;i++)
	for(j=0;j<width/BSIZE;j++)
	{
		sum=0;
		for(m=i*BSIZE;m<i*BSIZE+BSIZE;m++)
		{
			p=img+FrameWidth+(m+FrameWidth)*(width+FrameWidth*2)+j*BSIZE;
			for(n=0;n<BSIZE;n++)
				sum+=(int)*p++;
		}
		sum=(sum+bsize)/bsize;
		msum+=sum;
		v=0;
		for(m=i*BSIZE;m<i*BSIZE+BSIZE;m++)
		{
			p=img+FrameWidth+(m+FrameWidth)*(width+FrameWidth*2)+j*BSIZE;
			for(n=0;n<BSIZE;n++)
			{
				t=(int)*p++-sum;
				t=t*t;
				v+=t;
			}
		}
		v=(v+bsize)/bsize;
		vsum+=v;
		bc++;
	}
	*var=(vsum+bc/2)/bc;
	if(gOptions.NewFPReader & CORRECT_REVERSE)
		*mean=255-(msum+bc/2)/bc;
	else
		*mean=(msum+bc/2)/bc;
	
	return 1;
}

