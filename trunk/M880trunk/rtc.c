/*************************************************
                                           
 ZEM 200                                          
                                                    
 rtc.c Setup system time and RTC clock 
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include "arca.h"
#include "exfun.h"
#include "main.h"
#ifdef ZEM500
#include "exfunio.h"
#endif

BOOL SetRTCClock(TTime *tm)
{
#ifdef ZEM500
	//printf("SetRTC\n");
	SetRTCTimeByMCU(tm);
#else
    	int fd, retval;
    
    	fd = open ("/dev/rtc", O_RDONLY); 
    	if (fd == -1) return FALSE;
       
    	retval = ioctl(fd, RTC_SET_TIME, tm);
    	close(fd);
    	if (retval == -1) return FALSE;
#endif    
    	return TRUE;
}

BOOL ReadRTCClockToSyncSys(TTime *tm)
{
    int retval=0;
    time_t newtime;
    struct timeval tv;
    struct timezone tz;	
#ifdef ZEM500
    extern int gHaveRTC;

		GetRTCTimeByMCU(tm);
		if(!gHaveRTC)
		{
			DBPRINTF("no rtc,so return\n");
			return TRUE;
		}
#else		

    fd=open("/dev/rtc", O_RDONLY); 
    if (fd==-1) return FALSE;
    retval=ioctl(fd, RTC_RD_TIME, tm);
    DBPRINTF("RetVal=%d Year=%d Month=%d Day=%d Hour=%d Min=%d Sec=%d\n", 
	     retval, tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);    
    if(((tm->tm_year<=70)||(tm->tm_year>=137))||
       ((tm->tm_mon>=12)||(tm->tm_mon<0))||
       ((tm->tm_mday<=0)||(tm->tm_mday>=32))||
       ((tm->tm_hour<0)||(tm->tm_hour>=24))||
       ((tm->tm_min<0)||(tm->tm_min>=60))||
       ((tm->tm_sec<0)||(tm->tm_sec>=60)))
	    retval=-1;
    //Fixed RTC time 
    if((tm->tm_year<=70)||(tm->tm_year>=137)) tm->tm_year=100;
    if((tm->tm_mon>=12)||(tm->tm_mon<0)) tm->tm_mon=0;
    if((tm->tm_mday<=0)||(tm->tm_mday>=32)) tm->tm_mday=1;
    if((tm->tm_hour<0)||(tm->tm_hour>=24)) tm->tm_hour=0;
    if((tm->tm_min<0)||(tm->tm_min>=60)) tm->tm_min=0;
    if((tm->tm_sec<0)||(tm->tm_sec>=60)) tm->tm_sec=0;
    if(retval==-1)
    {
	retval=ioctl(fd, RTC_SET_TIME, tm);
    }
    close(fd);
#endif
    tm->tm_isdst=-1;          /* don't know whether it's dst */
    //EncodeTime标准函数mktime
    //mktime可以修正tm_wday, tm_yday的值位于正确范围
    newtime=EncodeTime(tm);
    
    gettimeofday(&tv, &tz);
    
    tv.tv_sec = newtime+1;
    tv.tv_usec = 0;
    
    settimeofday(&tv, &tz);
   
    return (retval==0);

}
