/*************************************************
                                         
ICLOCK 400                                          
                                                  
TruncDate.c replace function mkDatetime                            
                                                    
Copyright (C) 2003-2008, ZKSoftware Inc.      		
                                                    
*************************************************/
#include "truncdate.h"


Clock* ChrisToHejira(Clock *chrisClock)  //公历转伊朗日历
{
    int day = 0, month = 0, year = 0, section = 0, doy = 0;
    BOOL Leap = FALSE;
    int len[] = {20, 50, 79, 110, 141, 172, 203, 234, 265, 295, 325, 355};
    int mon[] = {10, 11, 12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    //DBPRINTF("test ----ChrisToHejira() 1 data = %d-%d-%d\n", chrisClock->Year, chrisClock->Mon, chrisClock->Day);
    if (((chrisClock->Year -1) % 4) == 0)
        Leap = TRUE;

    //DBPRINTF("ChrisToHejira() - Leap = %d\n", Leap);
    if (Leap) {
        len[0] = 19;
        len[1] = 49;
    }

    doy = chrisClock->YDay;
    //DBPRINTF("ChrisToHejira() - doy = %d\n", doy);

    while (doy > len[section]) {
        section = section + 1;
        if (section > 11)
            break;
    }
    month = mon[section];
    //DBPRINTF("ChrisToHejira() - month = %d\n", month);

    //DBPRINTF("ChrisToHejira() - section = %d\n", section);

    if (section >0)
        day = doy - len[section-1];
    else
        day = doy + (30 - len[0]);

    //DBPRINTF("ChrisToHejira() - day = %d\n", day);

    if (section <= 2)
        year = chrisClock->Year - 622;
    else
        year = chrisClock->Year - 621;
    //DBPRINTF("ChrisToHejira() - year = %d\n", year);


    chrisClock->Year = year;
    chrisClock->Mon = month;
    chrisClock->Day = day;

    //DBPRINTF("test ----ChrisToHejira() 2 data = %d-%d-%d\n", chrisClock->Year, chrisClock->Mon, chrisClock->Day);
    return chrisClock;
}

Clock* HejiraToChris(Clock *hejiraClock)  //伊朗日历转公历
{
    int i = 0, day = 0, month = 0, year = 0, doy = 0, section = 0, hday = 0, hmonth = 0, hyear = 0;
    int mon[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 1, 2, 3};
    int len[] = {11, 41, 72, 102, 133, 164, 194, 225, 255, 286, 317, 345};
    int mlen[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};

    //DBPRINTF("test ----HejiraToChris() 1 data = %d-%d-%d\n", hejiraClock->Year, hejiraClock->Mon, hejiraClock->Day);
    hyear = (hejiraClock->Year % 100);
    hmonth = hejiraClock->Mon;
    hday = hejiraClock->Day;

    if ((hyear % 4) == 0)	//闰年
        mlen[11] = 30;
    if (((hyear + 1) % 4) == 0) {
        len[0] = 12;
        len[1] = 42;
        len[2] = 73;
        len[3] = 103;
        len[4] = 134;
        len[5] = 165;
        len[6] = 195;
        len[7] = 226;
        len[8] = 256;
        len[9] = 287;
        len[10] = 318;
        len[11] = 346;
    }

    if (((hyear + 2) % 4) ==0) {
        len[11] = 346;
    }

    if ((((hyear + 1) % 4) == 0) && (hmonth == 12)) {
        if (hday > mlen[11] + 1)
            return hejiraClock;
    } else if (hday >mlen[hmonth-1]) {
        return hejiraClock;
    }

    for (i=0; i< hmonth -1; i++)
        doy =doy + mlen[i];

    doy = doy + hday;

    while (doy > len[section]) {
        section = section + 1;
        if (section > 11 )
            break;
    }

    month = mon[section];
    if (section > 0)
        day = doy - len[section - 1];
    else
        day = doy + (31 -len[0]);

    if (section <= 9)
        year = hyear + 21;
    else
        year = hyear + 22;

    if (year >= 100)
        year = year -100;

    hejiraClock->Year = year + 2000;
    hejiraClock->Mon = month;
    hejiraClock->Day = day;

    //DBPRINTF("test ----HejiraToChris() 2 data = %d-%d-%d\n", hejiraClock->Year, hejiraClock->Mon, hejiraClock->Day);

    return hejiraClock;
}

/////////////////////////////////////////////////////////////////////////

//TTime * hejiratm
//if date is 2008/07/15, you must input 108/06/15, tm format.
int getTotalDay(int nYear, int nMonth, int nDay);
TTime * ChrisToHejiraTTime(TTime *christm)  //公历转伊朗日历
{
    int day = 0, month = 0, year = 0, section = 0, doy = 0;
    BOOL Leap = FALSE;
    int len[] = {20, 50, 79, 110, 141, 172, 203, 234, 265, 295, 325, 355};
    int mon[] = {10, 11, 12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    christm->tm_year += 1900;
    christm->tm_mon += 1;

    //DBPRINTF("test -ChrisToHejiraTTime() 1 data = %d-%d-%d\n", christm->tm_year, christm->tm_mon, christm->tm_mday);
    if (((christm->tm_year - 1) % 4) == 0)
        Leap = TRUE;

    if (Leap) {
        len[0] = 19;
        len[1] = 49;
    }

    doy = getTotalDay((christm->tm_year), christm->tm_mon, christm->tm_mday);

    while (doy > len[section]) {
        section = section + 1;
        if (section > 11)
            break;
    }
    month = mon[section];



    if (section >0)
        day = doy - len[section-1];
    else
        day = doy + (30 - len[0]);

    if (section <= 2)
        year = christm->tm_year - 622;
    else
        year = christm->tm_year - 621;

    christm->tm_year = year;
    christm->tm_mon = month;
    christm->tm_mday = day;

    //DBPRINTF("test -ChrisToHejiraTTime() 2 end data ------------- = %d-%d-%d\n", christm->tm_year, christm->tm_mon, christm->tm_mday);

    christm->tm_year -= 1900;
    christm->tm_mon -= 1;

    return christm;
}

//TTime * hejiratm
//if date is 1387/07/15, you must input -513/06/15, tm format.
TTime * HejiraToChrisTTime(TTime *hejiratm)  //伊朗日历转公历
{
    int i = 0, day = 0, month = 0, year = 0, doy = 0, section = 0, hday = 0, hmonth = 0, hyear = 0;
    int mon[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 1, 2, 3};
    int len[] = {11, 41, 72, 102, 133, 164, 194, 225, 255, 286, 317, 345};
    int mlen[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
	
    hejiratm->tm_year += 1900;
    hejiratm->tm_mon += 1;
    //DBPRINTF("test -HejiraToChrisTTime() data = %d-%d-%d\n", hejiratm->tm_year, hejiratm->tm_mon, hejiratm->tm_mday);

    hyear = hejiratm->tm_year % 100;
    hmonth = hejiratm->tm_mon;
    hday = hejiratm->tm_mday;

    if ((hyear % 4) == 0)
        mlen[11] = 30;
    if (((hyear + 1) % 4) == 0) {
        len[0]=12;
        len[1]=42;
        len[2]=73;
        len[3]=103;
        len[4]=134;
        len[5]=165;
        len[6]=195;
        len[7]=226;
        len[8]=256;
        len[9]=287;
        len[10]=318;
        len[11]=346;
    }

    if (((hyear + 2) % 4) ==0) {
        len[11] = 346;
    }

    if ((((hyear + 1) % 4) == 0) && (hmonth == 12)) {
        if (hday > mlen[11] + 1)
            return hejiratm;
    } else {
        if (hday >mlen[hmonth-1])
            return hejiratm;
    }

    //DBPRINTF("Section hmonth is %d\n", hmonth);
    for (i=0; i< hmonth -1; i++)
        doy =doy + mlen[i];
    doy = doy + hday;
   //DBPRINTF("Section doy is %d\n", doy);

    while (doy > len[section]) {
        section = section + 1;
        if (section > 11 )
			break;
        }

    //DBPRINTF("Section is %d\n", section);
    month = mon[section];
    if (section > 0)
        day = doy - len[section - 1];
    else
        day = doy + (31 -len[0]);

    if (section <= 9)
        year = hyear + 21;
    else
        year = hyear + 22;

    //DBPRINTF("test -HejiraToChrisTTime() year = %d\n", year);
    if (year >= 100)
        year = year -100;

   //zsliu add 
   if(year < 0)	
	hejiratm->tm_year = year + 2100;
   else
	hejiratm->tm_year = year + 2000; 
    hejiratm->tm_mon = month;
    hejiratm->tm_mday = day;

   //DBPRINTF("test -HejiraToChrisTTime() data end  ---------  = %d-%d-%d\n", hejiratm->tm_year, hejiratm->tm_mon, hejiratm->tm_mday);

    hejiratm->tm_year -= 1900;
    hejiratm->tm_mon -= 1;

    return hejiratm;
}


//charge is or not Leap Year
int isLeapYear(int nYear) {
    if(((nYear % 4 == 0) && (nYear % 100 != 0))||(nYear % 400 == 0))
        return 1; //leap year
    else
        return 0;
}

//day since Jan 1, 1~366
int getTotalDay(int nYear, int nMonth, int nDay) {
    int yDay = 0;
    int DaysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};

    int nCounter;
    DaysInMonth[1] = isLeapYear(nYear) ? 29 : 28;
    for(nCounter = 0; nCounter < nMonth - 1; ++nCounter) {
        yDay += DaysInMonth[nCounter];
    }

    return (yDay + nDay);
}

