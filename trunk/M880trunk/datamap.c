#include <string.h>
#include <stdlib.h>
#include "datamap.h"
#include "options.h"

extern int fdTransaction;
extern int fdFingerTmp;
extern int fdUser;
extern int fdExtUser;
extern int fdSms;
extern int fdUData;
extern int fdAlarm;
extern int fdWkcd;
extern int fdSTKey;
extern int fdtz;
extern int fdhtz;
extern int fdgroup;
extern int fdcgroup;
extern int fdpicidx;
extern int fddept;
extern int fdTimeclass;

extern int SelectFDFromConentType(int ContentType);
extern void SearchFirst(PSearchHandle sh);
 

BOOL mySearchNext(PSearchHandle sh)
{
        BOOL eof = TRUE;

	sh->bufferlen = 0;
	sh->datalen = 0;
	switch(sh->ContentType)
	{
		case FCT_USER:
			if (read(sh->fd, sh->buffer, sizeof(TUser))==sizeof(TUser))
				eof = FALSE;
			break;
		case FCT_FINGERTMP:
			if (read(sh->fd, sh->buffer, sizeof(TTemplate))==sizeof(TTemplate))
				eof = FALSE;
			break;
		case FCT_SMS:
			if (read(sh->fd, sh->buffer, sizeof(TSms))==sizeof(TSms))
				eof = FALSE;
			break;
		case FCT_ALARM:
			if (read(sh->fd, sh->buffer, sizeof(ALARM))==sizeof(ALARM))
				eof = FALSE;
			break;
		case FCT_BELL:
			if (read(sh->fd, sh->buffer, sizeof(TBellNew))==sizeof(TBellNew))
				eof=FALSE;
			break;
		case FCT_WORKCODE:
			if(read(sh->fd, sh->buffer, sizeof(TWORKCODE))==sizeof(TWORKCODE))
				eof = FALSE;
	                break;
	        case FCT_SHORTKEY:
        	        if (read(sh->fd, sh->buffer, sizeof(TSHORTKEY))==sizeof(TSHORTKEY))
	                        eof = FALSE;
	                break;
	        case FCT_UDATA:
        	        if (read(sh->fd, sh->buffer, sizeof(TUData))==sizeof(TUData))
	                        eof = FALSE;
	                break;
        	case FCT_EXTUSER:
                	if (read(sh->fd, sh->buffer, sizeof(TExtUser))==sizeof(TExtUser))
	                        eof = FALSE;
                	break;
	        case FCT_DEPT:
        	        if (read(sh->fd, sh->buffer, sizeof(TDept))==sizeof(TDept))
                	        eof = FALSE;
	                break;
        	case FCT_SCH:
	                if (read(sh->fd,sh->buffer,sizeof(TTimeClass)) == sizeof(TTimeClass))
        	                eof = FALSE;
                	break;
	        case FCT_TZ:
        	        if(read(sh->fd, sh->buffer, sizeof(TTimeZone))==sizeof(TTimeZone))
                	        eof = FALSE;
	                break;
        	case FCT_HTZ:
	                if(read(sh->fd, sh->buffer, sizeof(THTimeZone))==sizeof(THTimeZone))
        	                eof = FALSE;
                	break;
	        case FCT_GROUP:
        	        if(read(sh->fd, sh->buffer, sizeof(TGroup))==sizeof(TGroup))
                	        eof = FALSE;
	                break;
        	case FCT_CGROUP:
	                if(read(sh->fd, sh->buffer, sizeof(TCGroup))==sizeof(TCGroup))
        	                eof = FALSE;
                	break;
	        case FCT_PHOTOINDEX:
        	        if(read(sh->fd, sh->buffer, sizeof(TPhotoIndex))==sizeof(TPhotoIndex))
                	        eof = FALSE;
	                break;
	}
        return eof;
}

//用户
static TUser mapuser;
PUserLib MAP_GetAllUser(void)
{
	TSearchHandle sh;
	PUserLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

	//创建头节点
	pdb=(PUserLib)MALLOC(sizeof(TUserLib));
	if (!pdb) return NULL;
	memset(pdb, 0, sizeof(TUserLib));
	pdb->next=pdb;
	pdb->pre=pdb;
	pdf=pdb;

	//将所有数据记录添加到映射表
	sh.ContentType = FCT_USER;
	sh.buffer = (char*)&mapuser;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PUserLib)MALLOC(sizeof(TUserLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb != pdb->next) && (pdb != pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TUserLib));
		pnode->Index=i++;
		pnode->PIN=((PUser)sh.buffer)->PIN;
		memcpy(pnode->PIN2, ((PUser)sh.buffer)->PIN2, gOptions.PIN2Width);
		memcpy(pnode->Card, ((PUser)sh.buffer)->Card, 4);
		pnode->Privilege=((PUser)sh.buffer)->Privilege;
		if (((PUser)sh.buffer)->Password[0]) pnode->Privilege|=0x80;		//是否有密码
		if (((PUser)sh.buffer)->Name[0]) pnode->Privilege|=0x40;		//是否有姓名
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//短消息
static TSms mapsms;
PSmsLib MAP_GetAllSms(void)
{
	TSearchHandle sh;
	PSmsLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PSmsLib)MALLOC(sizeof(TSmsLib));
	if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TSmsLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

	sh.ContentType = FCT_SMS;
	sh.buffer = (char*)&mapsms;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PSmsLib)MALLOC(sizeof(TSmsLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb != pdb->next) && (pdb != pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TSmsLib));
		pnode->Index=i++;
		pnode->ID=((PSms)sh.buffer)->ID;
		pnode->StartTime=((PSms)sh.buffer)->StartTime;
		pnode->Tag=((PSms)sh.buffer)->Tag;
		pnode->ValidMinutes=((PSms)sh.buffer)->ValidMinutes;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//短消息分配表
static TUData mapudata;
PUDataLib MAP_GetAllUData(void)
{
	TSearchHandle sh;
	PUDataLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PUDataLib)MALLOC(sizeof(TUDataLib));
	if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TUDataLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

	sh.ContentType = FCT_UDATA;
	sh.buffer = (char*)&mapudata;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PUDataLib)MALLOC(sizeof(TUDataLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TUDataLib));
		pnode->Index=i++;
		pnode->PIN=((PUData)sh.buffer)->PIN;
		pnode->SmsID=((PUData)sh.buffer)->SmsID;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

static ALARM mapalarm;
PAlarmLib MAP_GetAllAlarm(void)
{
	TSearchHandle sh;
	PAlarmLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PAlarmLib)MALLOC(sizeof(TAlarmLib));
	if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TAlarmLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

	sh.ContentType = FCT_ALARM;
	sh.buffer = (char*)&mapalarm;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PAlarmLib)MALLOC(sizeof(TAlarmLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TAlarmLib));
		pnode->Index=i++;
		pnode->AlarmIDX=((PAlarm)sh.buffer)->AlarmIDX;
		pnode->AlarmHour=((PAlarm)sh.buffer)->AlarmHour;
		pnode->AlarmMin=((PAlarm)sh.buffer)->AlarmMin;
		pnode->AlarmStatus=((PAlarm)sh.buffer)->AlarmStatus;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

static TBellNew mapbell;
PBellLib MAP_GetAllBell(void)
{
	TSearchHandle sh;
	PBellLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PBellLib)MALLOC(sizeof(TBellLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TBellLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_BELL;
	sh.buffer = (char*)&mapbell;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PBellLib)MALLOC(sizeof(TBellLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TBellLib));
		pnode->Index=i++;
		pnode->BellID=((PBellNew)sh.buffer)->BellID;
		pnode->SchInfo=((PBellNew)sh.buffer)->SchInfo;
		pnode->BellTime[0]=((PBellNew)sh.buffer)->BellTime[0];
		pnode->BellTime[1]=((PBellNew)sh.buffer)->BellTime[1];
		
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//快捷键
static TSHORTKEY mapstkey; 
PShortKeyLib MAP_GetAllShortKey(void)
{
	TSearchHandle sh;
	PShortKeyLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PShortKeyLib)MALLOC(sizeof(TShortKeyLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TShortKeyLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表

	sh.ContentType = FCT_SHORTKEY;
	sh.buffer = (char*)&mapstkey;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PShortKeyLib)MALLOC(sizeof(TShortKeyLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TShortKeyLib));
		pnode->Index=i++;
		pnode->keyID=((PShortKey)sh.buffer)->keyID;
		pnode->keyFun=((PShortKey)sh.buffer)->keyFun;
		pnode->stateCode=((PShortKey)sh.buffer)->stateCode;
		pnode->Time[0]=((PShortKey)sh.buffer)->Time1;
		pnode->Time[1]=((PShortKey)sh.buffer)->Time2;
		pnode->Time[2]=((PShortKey)sh.buffer)->Time3;
		pnode->Time[3]=((PShortKey)sh.buffer)->Time4;
		pnode->Time[4]=((PShortKey)sh.buffer)->Time5;
		pnode->Time[5]=((PShortKey)sh.buffer)->Time6;
		pnode->Time[6]=((PShortKey)sh.buffer)->Time7;
		pnode->autochange=((PShortKey)sh.buffer)->autochange;
		memcpy(pnode->stateName, ((PShortKey)sh.buffer)->stateName, STATE_NAME_LEN);		
		
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//workcode
static TWORKCODE mapwkcd;
PWorkCodeLib MAP_GetAllWorkCode(void)
{
	TSearchHandle sh;
	PWorkCodeLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PWorkCodeLib)MALLOC(sizeof(TWorkCodeLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TWorkCodeLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_WORKCODE;
	sh.buffer = (char*)&mapwkcd;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PWorkCodeLib)MALLOC(sizeof(TWorkCodeLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TWorkCodeLib));
		pnode->Index=i++;
		pnode->PIN=((PWorkCode)sh.buffer)->PIN;
		memcpy(pnode->Code, ((PWorkCode)sh.buffer)->Code, 8);
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//部门
static TDept mapdept;
PDeptLib MAP_GetAllDept(void)
{
	TSearchHandle sh;
	PDeptLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PDeptLib)MALLOC(sizeof(TDeptLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TDeptLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_DEPT;
	sh.buffer = (char*)&mapdept;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PDeptLib)MALLOC(sizeof(TDeptLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TDeptLib));
		pnode->Index=i++;
		pnode->Deptid=((Pdept)sh.buffer)->Deptid;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//排班
static TTimeClass maptimeclass;
PTimeClassLib MAP_GetAllTimeClass(void)
{
	TSearchHandle sh;
	PTimeClassLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PTimeClassLib)MALLOC(sizeof(TTimeClassLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TTimeClassLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_SCH;
	sh.buffer = (char*)&maptimeclass;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PTimeClassLib)MALLOC(sizeof(TTimeClassLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TTimeClassLib));
		pnode->Index=i++;
		pnode->schClassid=((PTimeClass)sh.buffer)->schClassid;
		pnode->StartTime=((PTimeClass)sh.buffer)->StartTime;
		pnode->EndTime=((PTimeClass)sh.buffer)->EndTime;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//时间段
static TTimeZone maptimezone;
PTimeZoneLib MAP_GetAllTimeZone(void)
{
	TSearchHandle sh;
	PTimeZoneLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0, j;

        //创建头节点
        pdb=(PTimeZoneLib)MALLOC(sizeof(TTimeZoneLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TTimeZoneLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_TZ;
	sh.buffer = (char*)&maptimezone;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PTimeZoneLib)MALLOC(sizeof(TTimeZoneLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TTimeZoneLib));
		pnode->Index=i++;
		pnode->ID=((PTimeZone)sh.buffer)->ID;
		for(j=0; j<7; j++)
		{
			pnode->ITime[j][0][0] = ((PTimeZone)sh.buffer)->ITime[j][0][0];
			pnode->ITime[j][0][1] = ((PTimeZone)sh.buffer)->ITime[j][0][1];
			pnode->ITime[j][1][0] = ((PTimeZone)sh.buffer)->ITime[j][1][0];
			pnode->ITime[j][1][1] = ((PTimeZone)sh.buffer)->ITime[j][1][1];
		}		
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//节假日
static THTimeZone maphtimezone;
PHolidayLib MAP_GetAllHoliday(void)
{
	TSearchHandle sh;
	PHolidayLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PHolidayLib)MALLOC(sizeof(THolidayLib));
       // pdb=(PHolidayLib)malloc(20);
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(THolidayLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;
        //将所有数据记录添加到映射表
	sh.ContentType = FCT_HTZ;
	sh.buffer = (char*)&maphtimezone;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PHolidayLib)MALLOC(sizeof(THolidayLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(THolidayLib));
		pnode->Index=i++;
		pnode->ID=((PHTimeZone)sh.buffer)->ID;
		pnode->HDate[0][0]=((PHTimeZone)sh.buffer)->HDate[0][0];
		pnode->HDate[0][1]=((PHTimeZone)sh.buffer)->HDate[0][1];
		pnode->HDate[1][0]=((PHTimeZone)sh.buffer)->HDate[1][0];
		pnode->HDate[1][1]=((PHTimeZone)sh.buffer)->HDate[1][1];
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//扩展用户
static TExtUser mapextuser;
PExtuserLib MAP_GetAllExtuser(void)
{
	TSearchHandle sh;
	PExtuserLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PExtuserLib)MALLOC(sizeof(TExtuserLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TExtuserLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_EXTUSER;
	sh.buffer = (char*)&mapextuser;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PExtuserLib)MALLOC(sizeof(TExtuserLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TExtuserLib));
		pnode->Index=i++;
		pnode->PIN=((PExtUser)sh.buffer)->PIN;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//组
//static TGroup mapgroup;
PGroupLib MAP_GetAllGroup(void)
{
	TSearchHandle sh;
	PGroupLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PGroupLib)MALLOC(sizeof(TGroupLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TGroupLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_GROUP;
	sh.buffer = (char*)&mapudata;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PGroupLib)MALLOC(sizeof(TGroupLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TGroupLib));
		pnode->Index=i++;
		pnode->ID=((PGroup)sh.buffer)->ID;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//开锁组合
static TCGroup mapcgroup;
PUnlockGroupLib MAP_GetAllUnlockGroup(void)
{
	TSearchHandle sh;
	PUnlockGroupLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0, j;

        //创建头节点
        pdb=(PUnlockGroupLib)MALLOC(sizeof(TUnlockGroupLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TUnlockGroupLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_CGROUP;
	sh.buffer = (char*)&mapcgroup;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PUnlockGroupLib)MALLOC(sizeof(TUnlockGroupLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TUnlockGroupLib));
		pnode->Index=i++;
		pnode->ID=((PCGroup)sh.buffer)->ID;
		for(j=0; j<CGP_MEMBER_MAX; j++)
			pnode->GroupID[j]=((PCGroup)sh.buffer)->GroupID[j];
		pnode->MemberCount = ((PCGroup)sh.buffer)->MemberCount;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//指纹模板
static TTemplate maptemplate;
PTemplateLib MAP_GetAllTemplate(void)
{
	TSearchHandle sh;
	PTemplateLib pdb=NULL, pdf=NULL, pnode=NULL;
	int i=0;

        //创建头节点
        pdb=(PTemplateLib)MALLOC(sizeof(TTemplateLib));
        if (!pdb) return NULL;
        memset(pdb, 0, sizeof(TTemplateLib));
        pdb->next=pdb;
        pdb->pre=pdb;
        pdf=pdb;

        //将所有数据记录添加到映射表
	sh.ContentType = FCT_FINGERTMP;
	sh.buffer = (char*)&maptemplate;
	SearchFirst(&sh);
	while(!mySearchNext(&sh))
	{
		pnode = (PTemplateLib)MALLOC(sizeof(TTemplateLib));
		if (!pnode)
		{
			//内存分配失败,释放已创建的节点后退出
			while((pdb!=pdb->next) && (pdb!=pdb->pre))
			{
				pdf=pdb->next;
				pdb->pre->next=pdf;
				pdf->pre=pdb->pre;
				pdb->next=NULL;
				pdb->pre=NULL;
				FREE(pdb);
				pdb=pdf;
			}
			FREE(pdb);
			return NULL;
		}

		memset(pnode, 0, sizeof(TTemplateLib));
		pnode->Index=i++;
		pnode->Size=((PTemplate)sh.buffer)->Size;
		pnode->PIN=((PTemplate)sh.buffer)->PIN;
		pnode->FingerID=((PTemplate)sh.buffer)->FingerID;
		pnode->Valid=((PTemplate)sh.buffer)->Valid;
		pnode->pre=pnode;
		pnode->next=pnode;

		pdf->next=pnode;
		pnode->next=pdb;
		pdb->pre=pnode;
		pnode->pre=pdf;
		pdf=pnode;
	}
	return pdb;
}

//PhotoIndex()
//==================================================================================================
//建立索引表
void MAP_GetAllDataToMap(void)
{
	pMapUser=NULL;
	pMapSms=NULL;
	pMapUData=NULL;
	pMapAlarm=NULL;
	pMapBell=NULL;
	pMapShortKey=NULL;
	pMapWorkCode=NULL;
	pMapDept=NULL;
	pMapTimeClass=NULL;
	pMapTimeZone=NULL;
	pMapHoliday=NULL;
	pMapExtuser=NULL;
	pMapGroup=NULL;
	pMapCGroup=NULL;
	pMapTemplate=NULL;

	pMapUser=MAP_GetAllUser();
	pMapSms=MAP_GetAllSms();
	pMapUData= MAP_GetAllUData();
	pMapBell=MAP_GetAllBell();
	pMapAlarm= MAP_GetAllAlarm();
	pMapShortKey= MAP_GetAllShortKey();

	pMapWorkCode= MAP_GetAllWorkCode();

	pMapDept= MAP_GetAllDept();
	pMapTimeClass=MAP_GetAllTimeClass();

	pMapTimeZone= MAP_GetAllTimeZone();
	pMapHoliday= MAP_GetAllHoliday();
	pMapGroup= MAP_GetAllGroup();
	pMapCGroup= MAP_GetAllUnlockGroup();

	pMapExtuser= MAP_GetAllExtuser();

	pMapTemplate= MAP_GetAllTemplate();

	pCurUser=pMapUser;
	pCurSms=pMapSms;
	pCurUData=pMapUData;
	pCurAlarm=pMapAlarm;
	pCurBell=pMapBell;
	pCurShortKey=pMapShortKey;
	pCurWorkCode=pMapWorkCode;
	pCurDept=pMapDept;
	pCurTimeClass=pMapTimeClass;
	pCurTimeZone=pMapTimeZone;
	pCurHoliday=pMapHoliday;
	pCurExtuser=pMapExtuser;
	pCurGroup=pMapGroup;
	pCurCGroup=pMapCGroup;
	pCurTemplate=pMapTemplate;

	memset(NotReadDataFlag, 0, MAX_DATA_FILE_COUNT);
}

//更新索引表
void MAP_ResetMapList(int ContentType)
{
	PUserLib pdfUser=NULL;
	PSmsLib pdfSms=NULL;
	PUDataLib pdfUData=NULL;
	PAlarmLib pdfAlarm=NULL;
	PBellLib pdfBell=NULL;
	PShortKeyLib pdfShortKey=NULL;
	PWorkCodeLib pdfWorkCode=NULL;
	PDeptLib pdfDept=NULL;
	PTimeClassLib pdfTimeClass=NULL;
	PTimeZoneLib pdfTimeZone=NULL;
	PHolidayLib pdfHoliday=NULL;
	PExtuserLib pdfExtuser=NULL;
	PGroupLib pdfGroup=NULL;
	PUnlockGroupLib pdfCGroup=NULL;
	PTemplateLib pdfTemplate=NULL;

	if (ContentType==FCT_ALL || ContentType==FCT_USER)
	{
		if (pMapUser)
		{
		while((pMapUser!=pMapUser->next)&&(pMapUser!=pMapUser->pre))
		{
			pdfUser=pMapUser->next;
			pMapUser->pre->next=pdfUser;
			pdfUser->pre=pMapUser->pre;
			pMapUser->next=NULL;
			pMapUser->pre=NULL;
			FREE(pMapUser);
                	pMapUser=pdfUser;
		}
		FREE(pMapUser);
		}
		pMapUser=MAP_GetAllUser();
		pCurUser=pMapUser;
	}
	if ((ContentType==FCT_ALL && !gOptions.IsOnlyRFMachine) || ContentType==FCT_FINGERTMP)
	{
		if (pMapTemplate)
		{
		while((pMapTemplate!=pMapTemplate->next)&&(pMapTemplate!=pMapTemplate->pre))
		{
			pdfTemplate=pMapTemplate->next;
			pMapTemplate->pre->next=pdfTemplate;
			pdfTemplate->pre=pMapTemplate->pre;
			pMapTemplate->next=NULL;
			pMapTemplate->pre=NULL;
			FREE(pMapTemplate);
			pMapTemplate=pdfTemplate;
		}
		FREE(pMapTemplate);
		}
		pMapTemplate=MAP_GetAllTemplate();
		pCurTemplate=pMapTemplate;
	}
	
	if ((ContentType==FCT_ALL) || ContentType==FCT_SMS)
	{
		if (pMapSms)
		{
		while((pMapSms!=pMapSms->next)&&(pMapSms!=pMapSms->pre))
		{
			pdfSms=pMapSms->next;
			pMapSms->pre->next=pdfSms;
			pdfSms->pre=pMapSms->pre;
			pMapSms->next=NULL;
			pMapSms->pre=NULL;
			FREE(pMapSms);
        		pMapSms=pdfSms;
		}
		FREE(pMapSms);
		}
		pMapSms=MAP_GetAllSms();
		pCurSms=pMapSms;
	}
	
	if ((ContentType==FCT_ALL) || ContentType==FCT_ALARM)
	{
		if (pMapAlarm)
		{
		while((pMapAlarm!=pMapAlarm->next)&&(pMapAlarm!=pMapAlarm->pre))
		{
			pdfAlarm=pMapAlarm->next;
			pMapAlarm->pre->next=pdfAlarm;
			pdfAlarm->pre=pMapAlarm->pre;
			pMapAlarm->next=NULL;
			pMapAlarm->pre=NULL;
			FREE(pMapAlarm);
                        pMapAlarm=pdfAlarm;
                }
		FREE(pMapAlarm);
		}
		pMapAlarm=MAP_GetAllAlarm();
		pCurAlarm=pMapAlarm;
	}

	if ((ContentType==FCT_ALL) || ContentType==FCT_BELL)
	{
		if (pMapBell)
		{
		while((pMapBell!=pMapBell->next)&&(pMapBell!=pMapBell->pre))
		{
			pdfBell=pMapBell->next;
			pMapBell->pre->next=pdfBell;
			pdfBell->pre=pMapBell->pre;
			pMapBell->next=NULL;
			pMapBell->pre=NULL;
			FREE(pMapBell);
			pMapBell=pdfBell;
		}
		FREE(pMapBell);
		}
		pMapBell=MAP_GetAllBell();
		pCurBell=pMapBell;
	}

	if ((ContentType==FCT_ALL) || ContentType==FCT_WORKCODE)
	{
		if (pMapWorkCode)
		{
		while((pMapWorkCode!=pMapWorkCode->next)&&(pMapWorkCode!=pMapWorkCode->pre))
		{
			pdfWorkCode=pMapWorkCode->next;
			pMapWorkCode->pre->next=pdfWorkCode;
			pdfWorkCode->pre=pMapWorkCode->pre;
			pMapWorkCode->next=NULL;
			pMapWorkCode->pre=NULL;
			FREE(pMapWorkCode);
                	pMapWorkCode=pdfWorkCode;
		}
		FREE(pMapWorkCode);
		}
		pMapWorkCode=MAP_GetAllWorkCode();
		pCurWorkCode=pMapWorkCode;
	}

	if (ContentType==FCT_ALL || ContentType==FCT_SHORTKEY)
	{
		if (pMapShortKey)
		{
		while((pMapShortKey!=pMapShortKey->next)&&(pMapShortKey!=pMapShortKey->pre))
		{
			pdfShortKey=pMapShortKey->next;
			pMapShortKey->pre->next=pdfShortKey;
			pdfShortKey->pre=pMapShortKey->pre;
			pMapShortKey->next=NULL;
			pMapShortKey->pre=NULL;
			FREE(pMapShortKey);
                        pMapShortKey=pdfShortKey;
        	}
		FREE(pMapShortKey);
		}
		pMapShortKey=MAP_GetAllShortKey();
		pCurShortKey=pMapShortKey;
	}

	if (ContentType==FCT_ALL || ContentType==FCT_UDATA)
	{
		if (pMapUData)
		{
		while((pMapUData!=pMapUData->next)&&(pMapUData!=pMapUData->pre))
		{
			pdfUData=pMapUData->next;
			pMapUData->pre->next=pdfUData;
			pdfUData->pre=pMapUData->pre;
			pMapUData->next=NULL;
			pMapUData->pre=NULL;
			FREE(pMapUData);
			pMapUData=pdfUData;
		}
		FREE(pMapUData);
		}
		pMapUData=MAP_GetAllUData();
		pCurUData=pMapUData;
	}

	if (ContentType==FCT_ALL || ContentType==FCT_EXTUSER)
	{
		if (pMapExtuser)
		{
		while((pMapExtuser!=pMapExtuser->next)&&(pMapExtuser!=pMapExtuser->pre))
		{
			pdfExtuser=pMapExtuser->next;
			pMapExtuser->pre->next=pdfExtuser;
			pdfExtuser->pre=pMapExtuser->pre;
			pMapExtuser->next=NULL;
			pMapExtuser->pre=NULL;
			FREE(pMapExtuser);
			pMapExtuser=pdfExtuser;
		}
		FREE(pMapExtuser);
		}
		pMapExtuser=MAP_GetAllExtuser();
		pCurExtuser=pMapExtuser;
	}

	if ((ContentType==FCT_ALL) || ContentType==FCT_DEPT)
	{
		if (pMapDept)
		{
		while((pMapDept!=pMapDept->next)&&(pMapDept!=pMapDept->pre))
		{
			pdfDept=pMapDept->next;
			pMapDept->pre->next=pdfDept;
			pdfDept->pre=pMapDept->pre;
			pMapDept->next=NULL;
			pMapDept->pre=NULL;
			FREE(pMapDept);
			pMapDept=pdfDept;
		}
		FREE(pMapDept);
		}
		pMapDept=MAP_GetAllDept();
		pCurDept=pMapDept;
	}

	if ((ContentType==FCT_ALL) || ContentType==FCT_SCH)
	{
		if (pMapTimeClass)
		{
		while((pMapTimeClass!=pMapTimeClass->next)&&(pMapTimeClass!=pMapTimeClass->pre))
		{
			pdfTimeClass=pMapTimeClass->next;
			pMapTimeClass->pre->next=pdfTimeClass;
			pdfTimeClass->pre=pMapTimeClass->pre;
			pMapTimeClass->next=NULL;
			pMapTimeClass->pre=NULL;
			FREE(pMapTimeClass);
			pMapTimeClass=pdfTimeClass;
		}
		FREE(pMapTimeClass);
		}
		pMapTimeClass=MAP_GetAllTimeClass();
		pCurTimeClass=pMapTimeClass;
	}

	if ((ContentType==FCT_ALL) || ContentType==FCT_TZ)
	{
		if (pMapTimeZone)
		{
		while((pMapTimeZone!=pMapTimeZone->next)&&(pMapTimeZone!=pMapTimeZone->pre))
		{
			pdfTimeZone=pMapTimeZone->next;
			pMapTimeZone->pre->next=pdfTimeZone;
			pdfTimeZone->pre=pMapTimeZone->pre;
			pMapTimeZone->next=NULL;
			pMapTimeZone->pre=NULL;
			FREE(pMapTimeZone);
			pMapTimeZone=pdfTimeZone;
		}
		FREE(pMapTimeZone);
		}
		pMapTimeZone=MAP_GetAllTimeZone();
		pCurTimeZone=pMapTimeZone;
	}

	if ((ContentType==FCT_ALL) || ContentType==FCT_HTZ)
	{
		if (pMapHoliday)
		{
		while((pMapHoliday!=pMapHoliday->next)&&(pMapHoliday!=pMapHoliday->pre))
		{
			pdfHoliday=pMapHoliday->next;
			pMapHoliday->pre->next=pdfHoliday;
			pdfHoliday->pre=pMapHoliday->pre;
			pMapHoliday->next=NULL;
			pMapHoliday->pre=NULL;
			FREE(pMapHoliday);
			pMapHoliday=pdfHoliday;
		}
		FREE(pMapHoliday);
		}
		pMapHoliday=MAP_GetAllHoliday();
		pCurHoliday=pMapHoliday;
	}

	if (ContentType==FCT_ALL || ContentType==FCT_GROUP)
	{
		if(pMapGroup)
		{
		while((pMapGroup!=pMapGroup->next)&&(pMapGroup!=pMapGroup->pre))
		{
			pdfGroup=pMapGroup->next;
			pMapGroup->pre->next=pdfGroup;
			pdfGroup->pre=pMapGroup->pre;
			pMapGroup->next=NULL;
			pMapGroup->pre=NULL;
			FREE(pMapGroup);
			pMapGroup=pdfGroup;
		}
		FREE(pMapGroup);
		}
		pMapGroup=MAP_GetAllGroup();
		pCurGroup=pMapGroup;
	}

	if ((ContentType==FCT_ALL) || ContentType==FCT_CGROUP)
	{
		if(pMapCGroup)
		{
		while((pMapCGroup!=pMapCGroup->next)&&(pMapCGroup!=pMapCGroup->pre))
		{
			pdfCGroup=pMapCGroup->next;
			pMapCGroup->pre->next=pdfCGroup;
			pdfCGroup->pre=pMapCGroup->pre;
			pMapCGroup->next=NULL;
			pMapCGroup->pre=NULL;
			FREE(pMapCGroup);
			pMapCGroup=pdfCGroup;
		}
		FREE(pMapCGroup);
		}
		pMapCGroup=MAP_GetAllUnlockGroup();
		pCurCGroup=pMapCGroup;
	}

	if (ContentType==FCT_ALL || ContentType==FCT_PHOTOINDEX)
	{
		;
	}
}

//增加、修改索引表结点
//删除记录时只修改结点内容，不需删除结点
void MAP_ModifyMapList(int ContentType, char *buffer)
{
        PUserLib pdfUser=NULL;
        PSmsLib pdfSms=NULL;
        PUDataLib pdfUData=NULL;
        PAlarmLib pdfAlarm=NULL;
        PBellLib pdfBell=NULL;
        PShortKeyLib pdfShortKey=NULL;
        PWorkCodeLib pdfWorkCode=NULL;
        PDeptLib pdfDept=NULL;
        PTimeClassLib pdfTimeClass=NULL;
        PTimeZoneLib pdfTimeZone=NULL;
        PHolidayLib pdfHoliday=NULL;
        PExtuserLib pdfExtuser=NULL;
        PGroupLib pdfGroup=NULL;
        PUnlockGroupLib pdfCGroup=NULL;
        PTemplateLib pdfTemplate=NULL;

	int i;

	switch(ContentType)
	{
		case FCT_USER:
			if (pCurUser==pMapUser)		//添加新节点
			{
				pdfUser=(PUserLib)MALLOC(sizeof(TUserLib));
				if (pdfUser)
				{
					memset(pdfUser, 0, sizeof(TUserLib));

					pdfUser->Index=(pCurUser->pre==pMapUser)?0:pCurUser->pre->Index+1;
					pdfUser->PIN = ((PUser)buffer)->PIN;
					memcpy(pdfUser->PIN2, ((PUser)buffer)->PIN2, gOptions.PIN2Width);
					memcpy(pdfUser->Card, ((PUser)buffer)->Card, 4);
					pdfUser->Privilege = ((PUser)buffer)->Privilege;
					if (((PUser)buffer)->Password[0])
						pdfUser->Privilege|=0x80;
					if (((PUser)buffer)->Name[0])
						pdfUser->Privilege|=0x40;
					
					pCurUser->pre->next=pdfUser;
					pdfUser->pre=pCurUser->pre;
					pCurUser->pre=pdfUser;
					pdfUser->next=pCurUser;
				}
			}
			else				//修改节点
			{
				pCurUser->PIN = ((PUser)buffer)->PIN;
				memcpy(pCurUser->PIN2, ((PUser)buffer)->PIN2, gOptions.PIN2Width);
				memcpy(pCurUser->Card, ((PUser)buffer)->Card, 4);
				pCurUser->Privilege = ((PUser)buffer)->Privilege;
				if (((PUser)buffer)->Password[0])
					pCurUser->Privilege|=0x80;              
				if (((PUser)buffer)->Name[0])              
					pCurUser->Privilege |= 0x40;
			}
			break;
		case FCT_FINGERTMP:
			if (pCurTemplate==pMapTemplate)
			{
				pdfTemplate=(PTemplateLib)MALLOC(sizeof(TTemplateLib));
				if (pdfTemplate)
				{
					//printf("dddd chage tmp list add .. \n");
					memset(pdfTemplate, 0, sizeof(TTemplateLib));

					pdfTemplate->Index=(pCurTemplate->pre==pMapTemplate)?0:pCurTemplate->pre->Index+1;
					pdfTemplate->Size=((PTemplate)buffer)->Size;
					pdfTemplate->PIN=((PTemplate)buffer)->PIN;
					pdfTemplate->FingerID=((PTemplate)buffer)->FingerID;
					pdfTemplate->Valid=((PTemplate)buffer)->Valid;

					pCurTemplate->pre->next=pdfTemplate;
					pdfTemplate->pre=pCurTemplate->pre;
					pCurTemplate->pre=pdfTemplate;
					pdfTemplate->next=pCurTemplate;
				}
			}
			else
			{
				pCurTemplate->Size=((PTemplate)buffer)->Size;
				pCurTemplate->PIN=((PTemplate)buffer)->PIN;
				pCurTemplate->FingerID=((PTemplate)buffer)->FingerID;
			}
			break;
		case FCT_SMS:
			if (pCurSms==pMapSms)
			{
				pdfSms=(PSmsLib)MALLOC(sizeof(TSmsLib));
				if (pdfSms)
				{
					memset(pdfSms, 0, sizeof(TSmsLib));
		
					pdfSms->Index=(pCurSms->pre==pMapSms)?0:pCurSms->pre->Index+1;
					pdfSms->ID=((PSms)buffer)->ID;
					pdfSms->StartTime=((PSms)buffer)->StartTime;
					pdfSms->Tag=((PSms)buffer)->Tag;
					pdfSms->ValidMinutes=((PSms)buffer)->ValidMinutes;

					pCurSms->pre->next=pdfSms;
					pdfSms->pre=pCurSms->pre;
					pCurSms->pre=pdfSms;
					pdfSms->next=pCurSms;
				}
			}
			else
			{
				pCurSms->ID=((PSms)buffer)->ID;
				pCurSms->StartTime=((PSms)buffer)->StartTime;
				pCurSms->Tag=((PSms)buffer)->Tag;
			}
			break;
		case FCT_ALARM:
			if (pCurAlarm==pMapAlarm)
			{
				pdfAlarm=(PAlarmLib)MALLOC(sizeof(TAlarmLib));
				if (pdfAlarm)
				{
					memset(pdfAlarm, 0, sizeof(TAlarmLib));
					
					pdfAlarm->Index=(pCurAlarm->pre==pMapAlarm)?0:pCurAlarm->pre->Index+1;
					pdfAlarm->AlarmIDX=((PAlarm)buffer)->AlarmIDX;
					pdfAlarm->AlarmHour=((PAlarm)buffer)->AlarmHour;
					pdfAlarm->AlarmMin=((PAlarm)buffer)->AlarmMin;
					pdfAlarm->AlarmStatus=((PAlarm)buffer)->AlarmStatus;

					pCurAlarm->pre->next=pdfAlarm;
					pdfAlarm->pre=pCurAlarm->pre;
					pCurAlarm->pre=pdfAlarm;
					pdfAlarm->next=pCurAlarm;
				}
			}
			else
			{
				pCurAlarm->AlarmIDX=((PAlarm)buffer)->AlarmIDX;
				pCurAlarm->AlarmHour=((PAlarm)buffer)->AlarmHour;
				pCurAlarm->AlarmMin=((PAlarm)buffer)->AlarmMin;
				pCurAlarm->AlarmStatus=((PAlarm)buffer)->AlarmStatus;
			}
			break;
		case FCT_BELL:
			if (pCurBell==pMapBell)
			{
				pdfBell=(PBellLib)MALLOC(sizeof(TBellLib));
				if (pdfBell)
				{
					memset(pdfBell, 0, sizeof(TBellLib));
				
					pdfBell->Index=(pCurBell->pre==pMapBell)?0:pCurBell->pre->Index+1;
					pdfBell->BellID=((PBellNew)buffer)->BellID;
					pdfBell->SchInfo=((PBellNew)buffer)->SchInfo;
					pdfBell->BellTime[0]=((PBellNew)buffer)->BellTime[0];
					pdfBell->BellTime[1]=((PBellNew)buffer)->BellTime[1];

					pCurBell->pre->next=pdfBell;
					pdfBell->pre=pCurBell->pre;
					pCurBell->pre=pdfBell;
					pdfBell->next=pCurBell;
				}
			}
			else
			{
				pCurBell->BellID=((PBellNew)buffer)->BellID;
				pCurBell->SchInfo=((PBellNew)buffer)->SchInfo;
				pCurBell->BellTime[0]=((PBellNew)buffer)->BellTime[0];
				pCurBell->BellTime[1]=((PBellNew)buffer)->BellTime[1];
			}
			break;
		case FCT_WORKCODE:
			if (pCurWorkCode==pMapWorkCode)
			{
				pdfWorkCode=(PWorkCodeLib)MALLOC(sizeof(TWorkCodeLib));
				if (pdfWorkCode)
				{
					memset(pdfWorkCode, 0, sizeof(TWorkCodeLib));

					pdfWorkCode->Index=(pCurWorkCode->pre==pMapWorkCode)?0:pCurWorkCode->pre->Index+1;
					pdfWorkCode->PIN=((PWorkCode)buffer)->PIN;
					memcpy(pdfWorkCode->Code, ((PWorkCode)buffer)->Code, 8);

					pCurWorkCode->pre->next=pdfWorkCode;
					pdfWorkCode->pre=pCurWorkCode->pre;
					pCurWorkCode->pre=pdfWorkCode;
					pdfWorkCode->next=pCurWorkCode;
				}
			}
			else
			{
				pCurWorkCode->PIN=((PWorkCode)buffer)->PIN;
				memcpy(pCurWorkCode->Code, ((PWorkCode)buffer)->Code, 8);
			}
	                break;
	        case FCT_SHORTKEY:
			if (pCurShortKey==pMapShortKey)
			{
				pdfShortKey=(PShortKeyLib)MALLOC(sizeof(TShortKeyLib));
				if (pdfShortKey)
				{
					memset(pdfShortKey, 0, sizeof(TShortKeyLib));

					pdfShortKey->Index=(pCurShortKey->pre==pMapShortKey)?0:pCurShortKey->pre->Index+1;
					pdfShortKey->keyID=((PShortKey)buffer)->keyID;
					pdfShortKey->keyFun=((PShortKey)buffer)->keyFun;
					pdfShortKey->stateCode=((PShortKey)buffer)->stateCode;
					pdfShortKey->Time[0]=((PShortKey)buffer)->Time1;
					pdfShortKey->Time[1]=((PShortKey)buffer)->Time2;
					pdfShortKey->Time[2]=((PShortKey)buffer)->Time3;
					pdfShortKey->Time[3]=((PShortKey)buffer)->Time4;
					pdfShortKey->Time[4]=((PShortKey)buffer)->Time5;
					pdfShortKey->Time[5]=((PShortKey)buffer)->Time6;
					pdfShortKey->Time[6]=((PShortKey)buffer)->Time7;
					pdfShortKey->autochange=((PShortKey)buffer)->autochange;
					memcpy(pdfShortKey->stateName, ((PShortKey)buffer)->stateName, STATE_NAME_LEN);		

					pCurShortKey->pre->next=pdfShortKey;
					pdfShortKey->pre=pCurShortKey->pre;
					pCurShortKey->pre=pdfShortKey;
					pdfShortKey->next=pCurShortKey;
				}
			}
			else
			{
				pCurShortKey->keyID=((PShortKey)buffer)->keyID;
				pCurShortKey->keyFun=((PShortKey)buffer)->keyFun;
				pCurShortKey->stateCode=((PShortKey)buffer)->stateCode;
				pCurShortKey->Time[0]=((PShortKey)buffer)->Time1;
				pCurShortKey->Time[1]=((PShortKey)buffer)->Time2;
				pCurShortKey->Time[2]=((PShortKey)buffer)->Time3;
				pCurShortKey->Time[3]=((PShortKey)buffer)->Time4;
				pCurShortKey->Time[4]=((PShortKey)buffer)->Time5;
				pCurShortKey->Time[5]=((PShortKey)buffer)->Time6;
				pCurShortKey->Time[6]=((PShortKey)buffer)->Time7;
				pCurShortKey->autochange=((PShortKey)buffer)->autochange;
				memcpy(pCurShortKey->stateName, ((PShortKey)buffer)->stateName, STATE_NAME_LEN);	
			}
	                break;
	        case FCT_UDATA:
			if (pCurUData==pMapUData)
			{
				pdfUData=(PUDataLib)MALLOC(sizeof(TUDataLib));
				if (pdfUData)
				{
					memset(pdfUData, 0, sizeof(TUDataLib));

					pdfUData->Index=(pCurUData->pre==pMapUData)?0:pCurUData->pre->Index+1;
					pdfUData->PIN=((PUData)buffer)->PIN;
					pdfUData->SmsID=((PUData)buffer)->SmsID;
					
					pCurUData->pre->next=pdfUData;
					pdfUData->pre=pCurUData->pre;
					pCurUData->pre=pdfUData;
					pdfUData->next=pCurUData;
				}
			}
			else
			{
				pCurUData->PIN=((PUData)buffer)->PIN;
				pCurUData->SmsID=((PUData)buffer)->SmsID;
			}
	                break;
        	case FCT_EXTUSER:
			if (pCurExtuser==pMapExtuser)
			{
				pdfExtuser=(PExtuserLib)MALLOC(sizeof(TExtuserLib));
				if (pdfExtuser)
				{
					memset(pdfExtuser, 0, sizeof(TExtuserLib));
					
					pdfExtuser->Index=(pCurExtuser->pre==pMapExtuser)?0:pCurExtuser->pre->Index+1;
					pdfExtuser->PIN=((PExtUser)buffer)->PIN;

					pCurExtuser->pre->next=pdfExtuser;
					pdfExtuser->pre=pCurExtuser->pre;
					pCurExtuser->pre=pdfExtuser;
					pdfExtuser->next=pCurExtuser;
				}
			}
			else
			{
				pCurExtuser->PIN=((PExtUser)buffer)->PIN;
			}
                	break;
	        case FCT_DEPT:
			if (pCurDept==pMapDept)
			{
				pdfDept=(PDeptLib)MALLOC(sizeof(TDeptLib));
				if (pdfDept)
				{
					memset(pdfDept, 0, sizeof(TDeptLib));

					pdfDept->Index=(pCurDept->pre==pMapDept)?0:pCurDept->pre->Index+1;
					pdfDept->Deptid=((Pdept)buffer)->Deptid;

					pCurDept->pre->next=pdfDept;
					pdfDept->pre=pCurDept->pre;
					pCurDept->pre=pdfDept;
					pdfDept->next=pCurDept;
				}
			}
			else
			{
				pCurDept->Deptid=((Pdept)buffer)->Deptid;
			}
	                break;
        	case FCT_SCH:
			if (pCurTimeClass==pMapTimeClass)
			{
				pdfTimeClass=(PTimeClassLib)MALLOC(sizeof(TTimeClassLib));
				if (pdfTimeClass)
				{
					memset(pdfTimeClass, 0, sizeof(TTimeClassLib));
					
					pdfTimeClass->Index=(pCurTimeClass->pre==pMapTimeClass)?0:pCurTimeClass->pre->Index+1;
					pdfTimeClass->schClassid=((PTimeClass)buffer)->schClassid;
					pdfTimeClass->StartTime=((PTimeClass)buffer)->StartTime;
					pdfTimeClass->EndTime=((PTimeClass)buffer)->EndTime;

					pCurTimeClass->pre->next=pdfTimeClass;
					pdfTimeClass->pre=pCurTimeClass->pre;
					pCurTimeClass->pre=pdfTimeClass;
					pdfTimeClass->next=pCurTimeClass;
				}
			}
			else
			{
				pCurTimeClass->schClassid=((PTimeClass)buffer)->schClassid;
				pCurTimeClass->StartTime=((PTimeClass)buffer)->StartTime;
				pCurTimeClass->EndTime=((PTimeClass)buffer)->EndTime;
			}
                	break;
	        case FCT_TZ:
			if (pCurTimeZone==pMapTimeZone)
			{
				pdfTimeZone=(PTimeZoneLib)MALLOC(sizeof(TTimeZoneLib));
				if (pdfTimeZone)
				{
					memset(pdfTimeZone, 0, sizeof(TTimeZoneLib));

					pdfTimeZone->Index=(pCurTimeZone->pre==pMapTimeZone)?0:pCurTimeZone->pre->Index+1;
					pdfTimeZone->ID=((PTimeZone)buffer)->ID;
					for (i=0; i<7; i++)
					{
						pdfTimeZone->ITime[i][0][0]=((PTimeZone)buffer)->ITime[i][0][0];
						pdfTimeZone->ITime[i][0][1]=((PTimeZone)buffer)->ITime[i][0][1];
						pdfTimeZone->ITime[i][1][0]=((PTimeZone)buffer)->ITime[i][1][0];
						pdfTimeZone->ITime[i][1][1]=((PTimeZone)buffer)->ITime[i][1][1];
					}
					pCurTimeZone->pre->next=pdfTimeZone;
					pdfTimeZone->pre=pCurTimeZone->pre;
					pCurTimeZone->pre=pdfTimeZone;
					pdfTimeZone->next=pCurTimeZone;
				}
			}
			else
			{
				pCurTimeZone->ID=((PTimeZone)buffer)->ID;
				for (i=0; i<7; i++)
				{
					pCurTimeZone->ITime[i][0][0]=((PTimeZone)buffer)->ITime[i][0][0];
					pCurTimeZone->ITime[i][0][1]=((PTimeZone)buffer)->ITime[i][0][1];
					pCurTimeZone->ITime[i][1][0]=((PTimeZone)buffer)->ITime[i][1][0];
					pCurTimeZone->ITime[i][1][1]=((PTimeZone)buffer)->ITime[i][1][1];
				}
			}
	                break;
        	case FCT_HTZ:
			if (pCurHoliday==pMapHoliday)
			{
				pdfHoliday=(PHolidayLib)MALLOC(sizeof(THolidayLib));
				if (pdfHoliday)
				{
					memset(pdfHoliday, 0, sizeof(THolidayLib));

					pdfHoliday->Index=(pCurHoliday->pre==pMapHoliday)?0:pCurHoliday->pre->Index+1;
					pdfHoliday->ID=((PHTimeZone)buffer)->ID;
					pdfHoliday->HDate[0][0]=((PHTimeZone)buffer)->HDate[0][0];
					pdfHoliday->HDate[0][1]=((PHTimeZone)buffer)->HDate[0][1];
					pdfHoliday->HDate[1][0]=((PHTimeZone)buffer)->HDate[1][0];
					pdfHoliday->HDate[1][1]=((PHTimeZone)buffer)->HDate[1][1];

					pCurHoliday->pre->next=pdfHoliday;
					pdfHoliday->pre=pCurHoliday->pre;
					pCurHoliday->pre=pdfHoliday;
					pdfHoliday->next=pCurHoliday;
				}
			}
			else
			{
				pCurHoliday->ID=((PHTimeZone)buffer)->ID;
				pCurHoliday->HDate[0][0]=((PHTimeZone)buffer)->HDate[0][0];
				pCurHoliday->HDate[0][1]=((PHTimeZone)buffer)->HDate[0][1];
				pCurHoliday->HDate[1][0]=((PHTimeZone)buffer)->HDate[1][0];
				pCurHoliday->HDate[1][1]=((PHTimeZone)buffer)->HDate[1][1];
			}
                	break;
	        case FCT_GROUP:
			if (pCurGroup==pMapGroup)
			{
				pdfGroup=(PGroupLib)MALLOC(sizeof(TGroupLib));
				if (pdfGroup)
				{
					memset(pdfGroup, 0, sizeof(TGroupLib));

					pdfGroup->Index=(pCurGroup->pre==pMapGroup)?0:pCurGroup->pre->Index+1;
					pdfGroup->ID=((PGroup)buffer)->ID;

					pCurGroup->pre->next=pdfGroup;
					pdfGroup->pre=pCurGroup->pre;
					pCurGroup->pre=pdfGroup;
					pdfGroup->next=pCurGroup;
				}
			}
			else
			{
				pCurGroup->ID=((PGroup)buffer)->ID;
			}
	                break;
        	case FCT_CGROUP:
			if (pCurCGroup==pMapCGroup)
			{
				pdfCGroup=(PUnlockGroupLib)MALLOC(sizeof(TUnlockGroupLib));
				if (pdfCGroup)
				{
					memset(pdfCGroup, 0, sizeof(TUnlockGroupLib));

					pdfCGroup->Index=(pCurCGroup->pre==pMapCGroup)?0:pCurCGroup->pre->Index+1;
					pdfCGroup->ID=((PCGroup)buffer)->ID;
					for(i=0; i<CGP_MEMBER_MAX; i++)
						pdfCGroup->GroupID[i]=((PCGroup)buffer)->GroupID[i];
					pdfCGroup->MemberCount=((PCGroup)buffer)->MemberCount;
				
					pCurCGroup->pre->next=pdfCGroup;
					pdfCGroup->pre=pCurCGroup->pre;
					pCurCGroup->pre=pdfCGroup;
					pdfCGroup->next=pCurCGroup;	
				}
			}
			else
			{
				pCurCGroup->ID=((PCGroup)buffer)->ID;
				for(i=0; i<CGP_MEMBER_MAX; i++)
					pCurCGroup->GroupID[i]=((PCGroup)buffer)->GroupID[i];
				pCurCGroup->MemberCount=((PCGroup)buffer)->MemberCount;
			}
                	break;
	        case FCT_PHOTOINDEX:
	                break;
	}

	NotReadDataFlag[ContentType]=READ_FILE_DATA;            //读取数据

}

//索引表当前指针指向首结点
void MapSearchFirst(int ContentType)
{
//printf("ddd  fine header pointer =%d\n",pMapUser);
	switch(ContentType)
	{
		case FCT_USER:
			pCurUser=pMapUser;
			break;
		case FCT_FINGERTMP:
			pCurTemplate=pMapTemplate;
			break;
		case FCT_SMS:
			pCurSms=pMapSms;
			break;
		case FCT_ALARM:
			pCurAlarm=pMapAlarm;
			break;
		case FCT_BELL:
			pCurBell=pMapBell;
			break;
		case FCT_WORKCODE:
			pCurWorkCode=pMapWorkCode;
	                break;
	        case FCT_SHORTKEY:
			pCurShortKey=pMapShortKey;
	                break;
	        case FCT_UDATA:
			pCurUData=pMapUData;
	                break;
        	case FCT_EXTUSER:
			pCurExtuser=pMapExtuser;
                	break;
	        case FCT_DEPT:
			pCurDept=pMapDept;
	                break;
        	case FCT_SCH:
			pCurTimeClass=pMapTimeClass;
                	break;
	        case FCT_TZ:
			pCurTimeZone=pMapTimeZone;
	                break;
        	case FCT_HTZ:
			pCurHoliday=pMapHoliday;
                	break;
	        case FCT_GROUP:
			pCurGroup=pMapGroup;
	                break;
        	case FCT_CGROUP:
			pCurCGroup=pMapCGroup;
                	break;
	        case FCT_PHOTOINDEX:
	                break;
	}
}

BOOL MapSearchNext(int ContentType)
{
	BOOL eof = TRUE;

	switch(ContentType)
	{
		case FCT_USER:
			pCurUser=pCurUser->next;
			if (pCurUser!=pMapUser)
				eof = FALSE;
			break;
		case FCT_FINGERTMP:
			pCurTemplate=pCurTemplate->next;
			if (pCurTemplate!=pMapTemplate)
				eof = FALSE;
			break;
		case FCT_SMS:
			pCurSms=pCurSms->next;
			if (pCurSms!=pMapSms)
				eof = FALSE;
			break;
		case FCT_ALARM:
			pCurAlarm=pCurAlarm->next;
			if (pCurAlarm!=pMapAlarm)
				eof = FALSE;
			break;
		case FCT_BELL:
			pCurBell=pCurBell->next;
			if (pCurBell!=pMapBell)
				eof=FALSE;
			break;
		case FCT_WORKCODE:
			pCurWorkCode=pCurWorkCode->next;
			if (pCurWorkCode!=pMapWorkCode)
				eof = FALSE;
	                break;
	        case FCT_SHORTKEY:
			pCurShortKey=pCurShortKey->next;
			if (pCurShortKey!=pMapShortKey)
	                        eof = FALSE;
	                break;
	        case FCT_UDATA:
			pCurUData=pCurUData->next;
			if (pCurUData!=pMapUData)
	                        eof = FALSE;
	                break;
        	case FCT_EXTUSER:
			pCurExtuser=pCurExtuser->next;
			if (pCurExtuser!=pMapExtuser)
	                        eof = FALSE;
                	break;
	        case FCT_DEPT:
			pCurDept=pCurDept->next;
			if (pCurDept!=pMapDept)
                	        eof = FALSE;
	                break;
        	case FCT_SCH:
			pCurTimeClass=pCurTimeClass->next;
			if (pCurTimeClass!=pMapTimeClass)
        	                eof = FALSE;
                	break;
	        case FCT_TZ:
			pCurTimeZone=pCurTimeZone->next;
			if (pCurTimeZone!=pMapTimeZone)
                	        eof = FALSE;
	                break;
        	case FCT_HTZ:
			pCurHoliday=pCurHoliday->next;
			if (pCurHoliday!=pMapHoliday)
        	                eof = FALSE;
                	break;
	        case FCT_GROUP:
			pCurGroup=pCurGroup->next;
			if (pCurGroup!=pMapGroup)
                	        eof = FALSE;
	                break;
        	case FCT_CGROUP:
			pCurCGroup=pCurCGroup->next;
			if (pCurCGroup!=pMapCGroup)
        	                eof = FALSE;
                	break;
	        case FCT_PHOTOINDEX:
                	        eof = FALSE;
	                break;
	}
	return eof;
}

//获取各表中每条记录的长度
int GetRecordSize(int ContentType, int index)
{
	int rsize=0;

	if(NotReadDataFlag[ContentType]!=READ_FILE_DATA)
		index+=1;
	
	switch(ContentType)
	{
		case FCT_USER:
			rsize=sizeof(TUser)*index;
			break;
		case FCT_FINGERTMP:
			rsize=sizeof(TTemplate)*index;
			break;
		case FCT_SMS:
			rsize=sizeof(TSms)*index;
			break;
		case FCT_ALARM:
			rsize=sizeof(ALARM)*index;
			break;
		case FCT_BELL:
			rsize=sizeof(TBellNew)*index;
			break;
		case FCT_WORKCODE:
			rsize=sizeof(TWORKCODE)*index;
	                break;
	        case FCT_SHORTKEY:
        	        rsize=sizeof(TSHORTKEY)*index;
	                break;
	        case FCT_UDATA:
        	        rsize=sizeof(TUData)*index;
	                break;
        	case FCT_EXTUSER:
                	rsize=sizeof(TExtUser)*index;
                	break;
	        case FCT_DEPT:
        	        rsize=sizeof(TDept)*index;
	                break;
        	case FCT_SCH:
	                rsize=sizeof(TTimeClass)*index;
                	break;
	        case FCT_TZ:
        	        rsize=sizeof(TTimeZone)*index;
	                break;
        	case FCT_HTZ:
	                rsize=sizeof(THTimeZone)*index;
                	break;
	        case FCT_GROUP:
        	        rsize=sizeof(TGroup)*index;
	                break;
        	case FCT_CGROUP:
	                rsize=sizeof(TCGroup)*index;
                	break;
	        case FCT_PHOTOINDEX:
        	        rsize=sizeof(TPhotoIndex)*index;
	                break;
	}
	return rsize;
}

int MAP_GetDataFromFile(int pos, PSearchHandle sh)
{
	int ret = 0;
	int i;

	sh->bufferlen = 0;
	sh->datalen = 0;
	lseek(sh->fd, pos, SEEK_SET);

	if ((NotReadDataFlag[sh->ContentType]==NOT_READ_DATA))            //不读取数据
		return 1;
	
	switch(sh->ContentType)
	{
		case FCT_USER:
			if (NotReadDataFlag[FCT_USER]==READ_NODE_DATA)  	 //读取节点数据
			{
				((PUser)sh->buffer)->PIN=pCurUser->PIN;
				memcpy(((PUser)sh->buffer)->PIN2, pCurUser->PIN2, gOptions.PIN2Width);	
				memcpy(((PUser)sh->buffer)->Card, pCurUser->Card, 4);	
				//((PUser)sh->buffer)->Privilege=pCurUser->Privilege;
				((PUser)sh->buffer)->Privilege=pCurUser->Privilege&0x0f;
				ret = 1;				
			}
			else
			{
				if (read(sh->fd, sh->buffer, sizeof(TUser))==sizeof(TUser))
					ret = 1;	
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TUser);
				sh->datalen=sh->bufferlen;
			}
			break;
		case FCT_FINGERTMP:
			if (NotReadDataFlag[FCT_FINGERTMP]==READ_NODE_DATA)  	 //读取节点数据
			{
				((PTemplate)sh->buffer)->Size=pCurTemplate->Size;
				((PTemplate)sh->buffer)->PIN=pCurTemplate->PIN;
				((PTemplate)sh->buffer)->FingerID=pCurTemplate->FingerID;
				((PTemplate)sh->buffer)->Valid=pCurTemplate->Valid;

				sh->bufferlen=sizeof(TTemplate);
				sh->datalen=pCurTemplate->Size;
				ret  = 1;				
			}
			else
			{
				if (read(sh->fd, sh->buffer, sizeof(TTemplate))==sizeof(TTemplate))
				{
					sh->bufferlen=sizeof(TTemplate);
					if (((PTemplate)sh->buffer)->Valid)
						sh->datalen=((PTemplate)sh->buffer)->Size;
					ret  = 1;
				}
			}
			break;
		case FCT_SMS:
			if (NotReadDataFlag[FCT_SMS]==READ_NODE_DATA)            //读取节点数据
			{
				((PSms)sh->buffer)->StartTime=pCurSms->StartTime;
				((PSms)sh->buffer)->ValidMinutes=pCurSms->ValidMinutes;
				((PSms)sh->buffer)->Tag=pCurSms->Tag;
				((PSms)sh->buffer)->ID=pCurSms->ID;
				ret  = 1;
			}
			else
			{
				if (read(sh->fd, sh->buffer, sizeof(TSms))==sizeof(TSms))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TSms);
				sh->datalen=sh->bufferlen;
			}			
			else 
			{	
				if (read(sh->fd, sh->buffer, sizeof(TSms))==sizeof(TSms))
					ret  = 1;
			}
			break;
		case FCT_ALARM:
			if (NotReadDataFlag[FCT_ALARM]==READ_NODE_DATA)           //读取节点数据
			{
				((PAlarm)sh->buffer)->AlarmIDX=pCurAlarm->AlarmIDX;
				((PAlarm)sh->buffer)->AlarmHour=pCurAlarm->AlarmHour;
				((PAlarm)sh->buffer)->AlarmMin=pCurAlarm->AlarmMin;
				((PAlarm)sh->buffer)->AlarmStatus=pCurAlarm->AlarmStatus;
				ret  = 1;				
			}
			else
			{
				if (read(sh->fd, sh->buffer, sizeof(ALARM))==sizeof(ALARM))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(ALARM);
				sh->datalen=sh->bufferlen;
			}
			break;
		case FCT_BELL:
			if (NotReadDataFlag[FCT_BELL]==READ_NODE_DATA)             //读取节点数据
			{
				((PBellNew)sh->buffer)->BellID=pCurBell->BellID;
				((PBellNew)sh->buffer)->SchInfo=pCurBell->SchInfo;
				((PBellNew)sh->buffer)->BellTime[0]=pCurBell->BellTime[0];
				((PBellNew)sh->buffer)->BellTime[1]=pCurBell->BellTime[1];
				ret  = 1;			
			}
			else
			{
				if (read(sh->fd, sh->buffer, sizeof(TBellNew))==sizeof(TBellNew))
					ret  = 1;

			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TBellNew);
				sh->datalen=sh->bufferlen;
			}			
			break;
		case FCT_WORKCODE:
			if (NotReadDataFlag[FCT_WORKCODE]==READ_NODE_DATA)             //读取节点数据
			{
				((PWorkCode)sh->buffer)->PIN=pCurWorkCode->PIN;
				memcpy(((PWorkCode)sh->buffer)->Code, pCurWorkCode->Code, 8);	
				ret  = 1;						
			}
			else
			{
				if(read(sh->fd, sh->buffer, sizeof(TWORKCODE))==sizeof(TWORKCODE))
					ret  = 1;

			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TWORKCODE);
				sh->datalen=sh->bufferlen;
			}			
	                break;
	        case FCT_SHORTKEY:
			if (NotReadDataFlag[FCT_SHORTKEY]==READ_NODE_DATA)             //读取节点数据
			{
				((PShortKey)sh->buffer)->keyID=pCurShortKey->keyID;
				((PShortKey)sh->buffer)->keyFun=pCurShortKey->keyFun;
				((PShortKey)sh->buffer)->stateCode=pCurShortKey->stateCode;
				((PShortKey)sh->buffer)->autochange=pCurShortKey->autochange;
				((PShortKey)sh->buffer)->Time1=pCurShortKey->Time[0];
				((PShortKey)sh->buffer)->Time2=pCurShortKey->Time[1];
				((PShortKey)sh->buffer)->Time3=pCurShortKey->Time[2];
				((PShortKey)sh->buffer)->Time4=pCurShortKey->Time[3];
				((PShortKey)sh->buffer)->Time5=pCurShortKey->Time[4];
				((PShortKey)sh->buffer)->Time6=pCurShortKey->Time[5];
				((PShortKey)sh->buffer)->Time7=pCurShortKey->Time[6];
				memcpy((char *)((PShortKey)sh->buffer)->stateName, pCurShortKey->stateName, STATE_NAME_LEN);
				ret  = 1;					
			}
			else
			{
	        	        if (read(sh->fd, sh->buffer, sizeof(TSHORTKEY))==sizeof(TSHORTKEY))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TSHORTKEY);
				sh->datalen=sh->bufferlen;
			}				
	              break;
	        case FCT_UDATA:
			if (NotReadDataFlag[FCT_UDATA]==READ_NODE_DATA)             //读取节点数据
			{
				((PUData)sh->buffer)->PIN=pCurUData->PIN;
				((PUData)sh->buffer)->SmsID=pCurUData->SmsID;
				ret  = 1;						
			}
			else
			{
	        	        if (read(sh->fd, sh->buffer, sizeof(TUData))==sizeof(TUData))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TUData);
				sh->datalen=sh->bufferlen;
			}				
	                break;
        	case FCT_EXTUSER:
			if (NotReadDataFlag[FCT_EXTUSER]==READ_NODE_DATA)             //读取节点数据
			{
				((PExtUser)sh->buffer)->PIN=pCurExtuser->PIN;
				ret  = 1;						
			}
			else
			{
	                	if (read(sh->fd, sh->buffer, sizeof(TExtUser))==sizeof(TExtUser))
					ret  = 1;

			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TExtUser);
				sh->datalen=sh->bufferlen;
			}				
                	break;
	        case FCT_DEPT:
			if (NotReadDataFlag[FCT_DEPT]==READ_NODE_DATA)             //读取节点数据
			{
				((Pdept)sh->buffer)->Deptid=pCurDept->Deptid;
				ret  = 1;						
			}
			else
			{
	        	        if (read(sh->fd, sh->buffer, sizeof(TDept))==sizeof(TDept))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TDept);
				sh->datalen=sh->bufferlen;
			}	
	                break;
        	case FCT_SCH:
			if (NotReadDataFlag[FCT_SCH]==READ_NODE_DATA)             //读取节点数据
			{
				((PTimeClass)sh->buffer)->schClassid=pCurTimeClass->schClassid;
				((PTimeClass)sh->buffer)->StartTime=pCurTimeClass->StartTime;
				((PTimeClass)sh->buffer)->EndTime=pCurTimeClass->EndTime;
				ret  = 1;					
			}
			else
			{
		                if (read(sh->fd,sh->buffer,sizeof(TTimeClass)) == sizeof(TTimeClass))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TTimeClass);
				sh->datalen=sh->bufferlen;
			}
	                break;
	        case FCT_TZ:
			if (NotReadDataFlag[FCT_TZ]==READ_NODE_DATA)             //读取节点数据
			{
				((PTimeZone)sh->buffer)->ID=pCurTimeZone->ID;
				for (i=0; i<7; i++)
				{
					((PTimeZone)sh->buffer)->ITime[i][0][0]=pCurTimeZone->ITime[i][0][0];
					((PTimeZone)sh->buffer)->ITime[i][0][1]=pCurTimeZone->ITime[i][0][1];
					((PTimeZone)sh->buffer)->ITime[i][1][0]=pCurTimeZone->ITime[i][1][0];
					((PTimeZone)sh->buffer)->ITime[i][1][1]=pCurTimeZone->ITime[i][1][1];
				}
				ret  = 1;					
			}
			else
			{
	        	        if(read(sh->fd, sh->buffer, sizeof(TTimeZone))==sizeof(TTimeZone))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TTimeZone);
				sh->datalen=sh->bufferlen;
			}				
	                break;
        	case FCT_HTZ:
			if (NotReadDataFlag[FCT_HTZ]==READ_NODE_DATA)             //读取节点数据
			{
				((PHTimeZone)sh->buffer)->ID=pCurHoliday->ID;
				((PHTimeZone)sh->buffer)->HDate[0][0]=pCurHoliday->HDate[0][0];
				((PHTimeZone)sh->buffer)->HDate[0][1]=pCurHoliday->HDate[0][1];
				((PHTimeZone)sh->buffer)->HDate[1][0]=pCurHoliday->HDate[1][0];
				((PHTimeZone)sh->buffer)->HDate[1][1]=pCurHoliday->HDate[1][1];
				ret  = 1;						
			}
			else
			{
		                if(read(sh->fd, sh->buffer, sizeof(THTimeZone))==sizeof(THTimeZone))
					ret  = 1;

			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(THTimeZone);
				sh->datalen=sh->bufferlen;
			}					
                	break;
	        case FCT_GROUP:
			if (NotReadDataFlag[FCT_GROUP]==READ_NODE_DATA)             //读取节点数据
			{
				((PGroup)sh->buffer)->ID=pCurGroup->ID;
				ret  = 1;						
			}
			else
			{
	        	        if(read(sh->fd, sh->buffer, sizeof(TGroup))==sizeof(TGroup))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TGroup);
				sh->datalen=sh->bufferlen;
			}				
	                break;
        	case FCT_CGROUP:
			if (NotReadDataFlag[FCT_CGROUP]==READ_NODE_DATA)             //读取节点数据
			{
				((PCGroup)sh->buffer)->ID=pCurCGroup->ID;
				((PCGroup)sh->buffer)->MemberCount=pCurCGroup->MemberCount;
				for (i=0; i<CGP_MEMBER_MAX; i++)
					((PCGroup)sh->buffer)->GroupID[i]=pCurCGroup->GroupID[i];
				ret = 1;						
			}
			else
			{
		                if(read(sh->fd, sh->buffer, sizeof(TCGroup))==sizeof(TCGroup))
					ret  = 1;
			}
			if(ret==1)
			{
				sh->bufferlen=sizeof(TCGroup);
				sh->datalen=sh->bufferlen;
			}				
                	break;
	        case FCT_PHOTOINDEX:
        	        if(read(sh->fd, sh->buffer, sizeof(TPhotoIndex))==sizeof(TPhotoIndex))
				ret  = 1;
	                break;
		default:
			break;
	}
	return ret;
}

static int MAP_IsValidData(int ContentType)
{
	int vflag=0;

	switch(ContentType)
	{
        case FCT_USER:
		if (pCurUser->PIN) vflag=1;
                break;
        case FCT_FINGERTMP:
		if (pCurTemplate->Valid && pCurTemplate->Size) vflag=1;
                break;
        case FCT_SMS:
		if (pCurSms->ID) vflag=1;
                break;
        case FCT_ALARM:
		if (pCurAlarm->AlarmIDX) vflag=1;
                break;
        case FCT_BELL:
		if (pCurBell->BellID) vflag=1;
                break;
        case FCT_WORKCODE:
		if (pCurWorkCode->PIN) vflag=1;
                break;
        case FCT_SHORTKEY:
		if (pCurShortKey->keyID) vflag=1;
                break;
        case FCT_UDATA:
		if (pCurUData->PIN) vflag=1;
                break;
        case FCT_EXTUSER:
		if (pCurExtuser->PIN) vflag=1;
                break;
        case FCT_DEPT:
		if (pCurDept->Deptid) vflag=1;
                break;
        case FCT_SCH:
		if (pCurTimeClass->schClassid) vflag=1;
                break;
        case FCT_TZ:
		if (pCurTimeZone->ID) vflag=1;
                break;
        case FCT_HTZ:
		if (pCurHoliday->ID) vflag=1;
                break;
        case FCT_GROUP:
		if (pCurGroup->ID) vflag=1;
                break;
        case FCT_CGROUP:
		if (pCurCGroup->ID) vflag=1;
                break;
//	case FCT_PHOTOINDEX:
        }
        return vflag;
}

int MAP_GetDataInfo(int ContentType, int StatType, int Value)
{
	int tmp=0;
	MapSearchFirst(ContentType);
	while (!MapSearchNext(ContentType))
	{
		//是否为有效数据
		if (MAP_IsValidData(ContentType))
		{
			switch(StatType)
			{
				case STAT_COUNT:
                        	        tmp++;
	                                break;
        	                case STAT_VALIDLEN:
					if (ContentType==FCT_FINGERTMP)
						tmp+=pCurTemplate->Size;
					else
		                                tmp+=GetRecordSize(ContentType, 1);
                        	        break;
	                        case STAT_CNTADMINUSER:
        	                        if (ISADMIN((pCurUser->Privilege)&0x3F)) tmp++;
                	                break;
                        	case STAT_CNTADMIN:
	                                if (Value & ((pCurUser->Privilege)&0x3F)) tmp++;
        	                        break;
                	        case STAT_CNTPWDUSER:
                        	        if ((pCurUser->Privilege)&0x80) tmp++;
                                	break;
	                        case STAT_CNTTEMPLATE:
        	                        if (pCurTemplate->PIN==Value) tmp++;
                	                break;
			}
                }
        }
	return tmp;
}
