#ifndef __TRUNCDATE_H__
#define __TRUNCDATE_H__


#include "exfun.h"
#include <stdlib.h>
#include "main.h"


Clock* ChrisToHejira(Clock *chrisClock);  //公历转伊朗日历
Clock* HejiraToChris(Clock *hejiraClock);  //伊朗日历转公历

TTime * ChrisToHejiraTTime(TTime *christm);  //公历转伊朗日历
TTime * HejiraToChrisTTime(TTime *hejiratm);  //伊朗日历转公历


#endif

