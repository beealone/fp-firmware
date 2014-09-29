#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "ssrpub.h"
#include "options.h"

//#define IDC_TIMER	111
#define RESPATH 	"res/"

static int hr,min,sec;

static void gettime(void)
{
	time_t local_time ;
	struct tm * now;
	local_time = time(0);
	now = localtime(&local_time);
	hr=now->tm_hour;
	min=now->tm_min;
	sec=now->tm_sec;
}

static int ShowClock(HDC hdc,int index)
{
	BITMAP Bitmap1;
	int angle=0;
	float tmp=0.0;

	if(index==1)
	{
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath("res/180.bmp")) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
	}

	if(index==2)
	{
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath("res/clock3.bmp")) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
	}

	if(index==1)
		FillBoxWithBitmap(hdc, 7, 20, 178 ,180,&Bitmap1);
	else
		FillBoxWithBitmap(hdc, 5, 20, 180 ,180,&Bitmap1);

	UnloadBitmap(&Bitmap1);

	//tmp=(90+30*(12-(float)hr)-(float)min*5/60.*6);
	tmp=(90+30*(12-(float)hr)-((float)min)*5/((float)60.)*6);
	if(tmp<0)
		tmp+=360;
	angle=(int)(tmp*64); //angle of hour
	if(index==1)
	{
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath("res/hour.bmp")) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
	}
	if(index==2)
	{
		if(LoadBitmap(hdc, &Bitmap1,GetBmpPath("res/hour3.bmp")) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
	}

	PivotBitmap(hdc,(const BITMAP *)&Bitmap1,95,110,8,6,360*64-angle);

	UnloadBitmap(&Bitmap1);

	tmp=(90-(float)min*6);
	if(tmp<0)
		tmp+=360;
	angle=(int)(tmp*64); //angle of min
	if(index==1)
	{
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath("res/min.bmp")) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
	}
	if(index==2)
	{
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath("res/min3.bmp")) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
	}

	PivotBitmap(hdc,(const BITMAP *)&Bitmap1,95,110,8,5,360*64-angle);
	UnloadBitmap(&Bitmap1);

	tmp=90+6*(60-(float)sec);
	if(tmp<0)
		tmp+=360;
	angle=(int)(tmp*64); //angle of sec
	if(index==1)
	{
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath("res/sec.bmp")) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
	}
	if(index==2)
	{
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath("res/sec3.bmp")) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
	}
	PivotBitmap(hdc,(const BITMAP *)&Bitmap1,95,110,5,2,360*64-angle);
	UnloadBitmap(&Bitmap1);

	return 0;
}

static int ShowDigit(HDC hdc,int flag)
{
	BITMAP Bitmap1;
	char buff[50];
	char bmpfile[14][20]={"000.bmp","001.bmp","002.bmp","003.bmp","004.bmp","005.bmp","006.bmp"\
	,"007.bmp","008.bmp","009.bmp","00a.bmp","00b.bmp","am0.bmp","pm0.bmp"};

	int index;

	{
		index=hr/10;
		memset(buff,0,50);
		sprintf(buff,"%s%s",RESPATH,(char *)bmpfile[index]);
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath(buff)) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}

		FillBoxWithBitmap(hdc, 195,80,26,40,&Bitmap1);
		UnloadBitmap(&Bitmap1);
	}
	index=hr%10;
	memset(buff,0,sizeof(buff));
	sprintf(buff,"%s%s",RESPATH,bmpfile[index]);
	if(LoadBitmap(hdc, &Bitmap1, GetBmpPath(buff)) < 0 )
	{
		fprintf(stderr,"Load Bitmap %s Error\n","");
		return -1;
	}
	FillBoxWithBitmap(hdc, 195+26,80,26,40,&Bitmap1);
	UnloadBitmap(&Bitmap1);
	if(flag==0)
	{
		index=10;
		memset(buff,0,sizeof(buff));
		sprintf(buff,"%s%s",RESPATH,bmpfile[index]);
		if(LoadBitmap(hdc, &Bitmap1, GetBmpPath(buff)) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
		FillBoxWithBitmap(hdc, 195+2*26,80,15,40,&Bitmap1);
		UnloadBitmap(&Bitmap1);
	}
	else
	{
		index=11;
		memset(buff,0,sizeof(buff));
		sprintf(buff,"%s%s",RESPATH,bmpfile[index]);
		if(LoadBitmap(hdc, &Bitmap1,GetBmpPath(buff)) < 0 )
		{
			fprintf(stderr,"Load Bitmap %s Error\n","");
			return -1;
		}
		FillBoxWithBitmap(hdc, 195+2*26,80,15,40,&Bitmap1);
		UnloadBitmap(&Bitmap1);

	}
	index=min/10;
	memset(buff,0,sizeof(buff));
	sprintf(buff,"%s%s",RESPATH,bmpfile[index]);
	if(LoadBitmap(hdc, &Bitmap1, GetBmpPath(buff)) < 0 )
	{
		fprintf(stderr,"Load Bitmap %s Error\n","");
		return -1;
	}
	FillBoxWithBitmap(hdc, 195+2*26+15,80,26,40,&Bitmap1);
	UnloadBitmap(&Bitmap1);
	index=min%10;
	memset(buff,0,sizeof(buff));
	sprintf(buff,"%s%s",RESPATH,bmpfile[index]);
	if(LoadBitmap(hdc, &Bitmap1, GetBmpPath(buff)) < 0 )
	{
		fprintf(stderr,"Load Bitmap %s Error\n","");
		return -1;
	}
	FillBoxWithBitmap(hdc, 195+3*26+15,80,26,40,&Bitmap1);
	UnloadBitmap(&Bitmap1);
	return 0;
}
int ShowTime(HDC hdc)
{
	gettime();

	ShowClock(hdc,gOptions.InterStyle);
	ShowDigit(hdc,0);

	return 0;
}

