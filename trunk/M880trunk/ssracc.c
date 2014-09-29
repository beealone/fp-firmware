/*************************************************

  ZEM 500                                          
  ssracc.c define all functions for access mangage                             
 
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/unaligned.h>
#include "ccc.h"
#include "flashdb.h"
#include "utils.h"
#include "options.h"
#include "main.h"
#include "sensor.h"
#include "ssracc.h"

int TestTZ(int tzid, TTime gCurTime);

//写用户时间段
int SaveUserTZ(int UserID, int *TZs)
{
        TUser u;
        int i;
	if(FDB_GetUser(UserID, &u))
	{
	        for(i=0;i<4;i++)
			u.TimeZones[i]=TZs[i];	
		if(FDB_ChgUser(&u)==FDB_OK) return TRUE;
	}
	return FALSE;
}   

//写用户分组信息
int SaveUserGrp(int UserID, int GrpID)
{
        int i;
        TUser u;
        if(FDB_GetUser(UserID, &u))
        {
                i=u.Group;//&0x0F;
                if(i==0) i=1;
        } 
        else
                i=1;
        if(i==GrpID) return TRUE; 

	u.Group=(BYTE)GrpID;	    
//        u.Group=(u.Group & 0xf0) | (GrpID &0x0f);
        if(FDB_ChgUser(&u)==FDB_OK) return TRUE;
        return FALSE;
}

//获取用户所属组，若没有指定组，则默认为第1组
int GetUserGrp(int UserID)
{
        int i;
        PUser u = FDB_GetUser(UserID, NULL);
        if(u)
        {
                i = u->Group;
                if(i>0)
                        return i;
        }
        return 1;       //default 1 group.
}

//判断是否在时间段内
static int TestTimeZone(PTimeZone pTZ, TTime t)
{
        int Start, End, i=t.tm_wday;

        Start = pTZ->ITime[i][TZ_START][TZ_HOUR]*60 + pTZ->ITime[i][TZ_START][TZ_MINUTE];
        End = pTZ->ITime[i][TZ_END][TZ_HOUR]*60 + pTZ->ITime[i][TZ_END][TZ_MINUTE];

//        printf("Start:%d:%d\n", pTZ->ITime[i][TZ_START][TZ_HOUR]*60, pTZ->ITime[i][TZ_START][TZ_MINUTE]);
//        printf("End:%d:%d\n", pTZ->ITime[i][TZ_END][TZ_HOUR]*60, pTZ->ITime[i][TZ_END][TZ_MINUTE]);
        i = t.tm_hour*60 + t.tm_min;
        return (Start<=i) && (End>=i);
}

//判断是否为节假日
int beforeday(TTime t, int m, int d)
{
	return((t.tm_mon+1<m) || ((t.tm_mon+1==m)&&(t.tm_mday<=d)));
}
int afterday(TTime t, int m, int d)
{
	return((t.tm_mon+1>m) || ((t.tm_mon+1==m)&&(t.tm_mday>=d)));
}
static int beHoliday(TTime t)
{
	THTimeZone thtz;
	int i, hdcount;
	int sm, sd, em, ed;
	int ret=0;

	hdcount = FDB_CntHTimeZone();
//	printf("hdcount:%d\n",hdcount);
	if(hdcount)
	{
		for(i=0; i<hdcount; i++)
		{
			memset(&thtz, 0, sizeof(THTimeZone));
			if(FDB_GetHTimeZone(i+1, &thtz)!=NULL)
			{
				sm = thtz.HDate[HTZ_START][HTZ_MONTH];
				sd = thtz.HDate[HTZ_START][HTZ_DAY];
				em = thtz.HDate[HTZ_END][HTZ_MONTH];
				ed = thtz.HDate[HTZ_END][HTZ_DAY];
		
//				printf("start:%d-%d, end:%d-%d\n",sm,sd,em,ed);
	
				if(sm > em || (sm==em && sd>ed))	//结束日期在开始日期之前，作为跨年度节假日
				{
//					printf("1:  t.tm_mon:%d,t.tm_mday:%d\n",t.tm_mon+1,t.tm_mday);
					if(beforeday(t,em,ed) || afterday(t,sm,sd))
					{
						ret = thtz.ID;
						break;
					}
				}
				else if(sm < em || (sm==em && sd<ed))	//结束日期在开始日期之后
				{
//					printf("2:  t.tm_mon:%d,t.tm_mday:%d\n",t.tm_mon+1,t.tm_mday);
					if(beforeday(t,em,ed) && afterday(t,sm,sd))
					{
						ret = thtz.ID;
						break;
					}
				}
				else if(sm==em && sd==ed)		//开始日期与结束日期相同，只有当天被认为是节假日
				{
//					printf("3:  t.tm_mon:%d,t.tm_mday:%d\n",t.tm_mon+1,t.tm_mday);
					if(t.tm_mon+1==sm && t.tm_mday==sd)
					{
						ret = thtz.ID;
						break;
					}
				}
			}
		}
	}
	return ret;
}

static int testUserTimeZone(PUser pu, PGroup pgp, TTime gCurTime)
{
        int i;
        int TZs[3];
        int tzflag;

        tzflag=pu->TimeZones[0];
        for(i=0;i<3;i++)
        {
                TZs[i]=(tzflag)? pu->TimeZones[i+1]: pgp->TZID[i];		//0：使用组时间段，1：使用自定义时间段
        }

//	printf("TZs:%d,%d,%d\n",TZs[0],TZs[1],TZs[2]);
	for(i=0;i<3;i++)
	{
                if(TZs[i]>0)
                {
			if(TestTZ(TZs[i], gCurTime))
				return 1;
                }
        }
        return 0;

}

int TestTZ(int tzid, TTime gCurTime)
{
	TTimeZone tz;
	memset(&tz, 0, sizeof(TTimeZone));
	if(FDB_GetTimeZone(tzid, &tz)!=NULL) {
		return TestTimeZone(&tz, gCurTime);
	}
	return 0;
}

static int testHolidayTimeZone(int tzid, TTime gCurTime)
{
	THTimeZone htz;
	memset(&htz, 0, sizeof(THTimeZone));
	if(FDB_GetHTimeZone(tzid, &htz)!=NULL)
		return TestTZ(htz.TZID, gCurTime);
	return 0;	
}

//判断是否在有效时间内
int TestUserTZ(int UserID, TTime gCurTime)
{
        int gpID;
	int hdID;
        TUser tu;
        TGroup tgp;

        memset(&tu, 0, sizeof(TUser));
        memset(&tgp, 0, sizeof(TGroup));
        if(FDB_GetUser(UserID, &tu)==NULL) {
		return 0;
        }

	//组信息
        gpID = (tu.Group > 0) ? tu.Group:1;
        if(FDB_GetGroup(gpID, &tgp)==NULL) {
		return 0;
        }

	hdID=beHoliday(gCurTime);
//	printf("hdID:%d\n",hdID);

	if(hdID) {
		//节假日设为有效，取用户时间段和节假日时间段的交界段
		if(HTZVALID(tgp.VerifyStyle)) {
			//printf("1:%d,2:%d\n",testUserTimeZone(&tu, &tgp, gCurTime) ,testHolidayTimeZone(hdID, gCurTime));
			//节假日有效时只判断是否在用户有效时间段之内
			return (testUserTimeZone(&tu, &tgp, gCurTime));// && testHolidayTimeZone(hdID, gCurTime));
		} else if(testHolidayTimeZone(hdID, gCurTime)) {
			//节假日设为无效，若当前时间在节假日时间段内，则返回0。否则返回当前时间是否在用户时间段内
			return 0;
		} else {
			return testUserTimeZone(&tu, &tgp, gCurTime);
		}
	} else {
		return testUserTimeZone(&tu, &tgp, gCurTime);
	}
}

typedef struct _VerifyRecord_
{
        U16 PIN;
        int VerifyMethod;
        TTime VerTime;   
}TVerifyRecord, *PVerifyRecord;         

#define VerRecSetCnt CGP_MEMBER_MAX
static TVerifyRecord LastVerRecSet[CGP_MEMBER_MAX];
static int VerRecCount=0;
#define UnlockInterval ((gOptions.WorkCode==0)?LoadInteger("~UnlockInterval", 8):LoadInteger("~UnlockInterval", 12))

static int TestOneGrp(PCGroup pcgp, unsigned char *TGrp, int count)
{
	int i, j;
	int gpcount=0;
	unsigned char tmpmember[CGP_MEMBER_MAX];

	for(i=0;i<CGP_MEMBER_MAX;i++)
		tmpmember[i]=(unsigned char)pcgp->GroupID[i];

	for(i=0;i<count;i++)
	{
		for(j=0;j<CGP_MEMBER_MAX;j++)
		{
//			printf("TGrp[i]:%d, pcgp->GroupID[j]:%d\n",TGrp[i],(unsigned char)pcgp->GroupID[j]);
//			if(TGrp[i]==(unsigned char)pcgp->GroupID[j])
			if(TGrp[i]==tmpmember[j])
			{
				tmpmember[j]=0;
				gpcount++;
				break;
			}
		}
	}

	if(gpcount==count)
	{
		if(pcgp->MemberCount==count) return count;	//合法开锁组合
		else return -1;					//合法组合成员
	}
	else
		return 0;					//非组合成员
}

static int TestAllGrp(unsigned char *TGrp, int count)
{
	TCGroup tcgp;
	int i;
	int last = 0;
	int ret;

	for(i=0; i<CGP_MAX; i++)
        {
		memset(&tcgp, 0, sizeof(TCGroup));
		if (FDB_GetCGroup(i+1, &tcgp)!=NULL)
		{
//			printf("tcgp.MemberCount:%d\n",tcgp.MemberCount);
			if (tcgp.MemberCount >= count)
			{
				ret = TestOneGrp(&tcgp, TGrp, count);
				if(ret>0)
				{
					return ret;
				}
				if(ret<0) last=ret;
			}
		}
	}
	return last;

}

//清除开锁成员记录
void ClearMemberRecord(void)
{
	memset((void*)&LastVerRecSet,0, sizeof(TVerifyRecord)*CGP_MEMBER_MAX);
	VerRecCount=0;
}

//判断是否开锁
int TestOpenLock(int UID, TTime t, int VerifyMethod)
{                        
        int i,j;
	unsigned char mgpID[CGP_MEMBER_MAX];
	int ret;

        //删除已超出开锁时间的旧记录
        for(i=0;i<VerRecCount;i++)
        {                       
                j = TimeDiffSec(t, LastVerRecSet[i].VerTime);
                if(j <= (UnlockInterval*VerRecCount-1))
                {       
                        if(i>0)
                        {               
                                for(j=i;j<VerRecCount;j++)
                                        LastVerRecSet[j-i]=LastVerRecSet[j];
                        }               
                        break;
                }       
        }
        VerRecCount-=i;   

        //删除同一个用户的旧记录
        for(i=0;i<VerRecCount;i++)              
        {                                               
                if(LastVerRecSet[i].PIN==UID)
                {
                        for(j=i+1;j<VerRecCount;j++)
                                LastVerRecSet[j-1]=LastVerRecSet[j];
                        VerRecCount--;
                }
        }

        //只保存最近VerRecSetCnt-1条记录
        if(VerRecCount>=VerRecSetCnt)
        {
                j=0;
                for(i=VerRecCount-VerRecSetCnt+1;i<VerRecCount;i++)
                        LastVerRecSet[j++]=LastVerRecSet[i];
                VerRecCount=VerRecSetCnt-1;
        }

        //写入当前记录 
        LastVerRecSet[VerRecCount].VerTime=t;
        LastVerRecSet[VerRecCount].VerifyMethod=VerifyMethod;
        LastVerRecSet[VerRecCount].PIN=UID;

	//判断是否满足开锁条件
        if(gOptions.LockFunOn&LOCKFUN_ADV)
        {
	        memset(mgpID, 0, sizeof(mgpID));
		//printf("VerRecCount:%d\n",VerRecCount);
       		for(i=0; i<VerRecCount+1; i++)
		{
                	mgpID[i] = (unsigned char)GetUserGrp(LastVerRecSet[i].PIN);
//			printf("mgpID[i]:%d\n",mgpID[i]);
		}

		ret = TestAllGrp((unsigned char*)&mgpID, VerRecCount+1);
		if(ret>0)	//满足开锁条件，返回组合成员个数
		{
			VerRecCount=0;
			return ret;
		}

		VerRecCount++;
		return ret;	
        }
        else //gOptions.LockFunOn==LOCKFUN_BASE
        {
                if(VerRecCount+1>=gOptions.UnlockPerson)
                {
                        VerRecCount=0;
                        return gOptions.UnlockPerson;
                }
                VerRecCount++;
                return -1;
        }
        return 0;
}

int CheckLockForceAction(TTime t)
{
	int tzID;
	int opentz, closetz;

	tzID = beHoliday(gCurTime);		//判断当前日期是否为节假日
	opentz = gOptions.DoorOpenTimeZone;	//常开时间段
	closetz = gOptions.DoorCloseTimeZone;	//常闭时间段

	if(opentz==closetz) return 0;		//常开常闭时间段冲突
//	printf("tzID:%d, isholiday:%d, testholiday:%d\n",tzID,!gOptions.IsHolidayValid,testHolidayTimeZone(tzID, t));
	if(tzID && !gOptions.IsHolidayValid && testHolidayTimeZone(tzID, t))
		return 0;
	
//	printf("opentz:%d,closetz:%d\n",opentz,closetz);
	//强制关门优先
	if(closetz && TestTZ(closetz,t)) return 2;
	else if(opentz && TestTZ(opentz,t)) return 1;	
	return 0;
}

int GetFirstTimeZone(void)
{
	int i, ret=0;
	for(i=0; i<TZ_MAX; i++)
	{
		if (FDB_GetTimeZone(i+1,NULL)!=NULL)
		{
			ret = i+1;
			break;
		}
	}
	return ret;
}

int GetFirstGroup(void)
{
	int i, ret=0;
	for(i=0; i<GP_MAX; i++)
	{
		if (FDB_GetGroup(i+1, NULL)!=NULL)
		{
			ret = i+1;
			break;
		}
	}
	return 0;
}


