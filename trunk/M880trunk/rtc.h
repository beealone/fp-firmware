/*************************************************
                                           
 ZEM 200                                          
                                                    
 rtc.h 
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _RTC_H_
#define _RTC_H_

#include "arca.h"
#include "exfun.h"
#define ZEM500
BOOL SetRTCClock(TTime *tm);
BOOL ReadRTCClockToSyncSys(TTime *tm);

#endif

