#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ccc.h"
#include "flashdb.h"
#include "flashdb2.h"
#include "utils2.h"
#include "options.h"

//给出一个用户的字符串格式的考勤号码(用于显示和数据交换)
//不同的TUser数据格式请修改该函数
char *fmtPin(TUser *user)
{
	static char pin[40];
	memset(pin,0,sizeof(pin));
#ifdef SSRDB
//	printf("user PIN=%d, PIN2=%s\n", user->PIN, user->PIN2);
	if(user->PIN2[0])
	{
		memcpy(pin, user->PIN2, sizeof(user->PIN2));
	}
#else
//	printf("user PIN=%d, PIN2=%d\n", user->PIN, user->PIN2);
	if(user->PIN2 && gOptions.PIN2Width>5)
	{
		sprintf(pin, "%d", user->PIN2);
	}
#endif
	else
		sprintf(pin,"%d", user->PIN);
	return pin;
}

//给出一个用户的字符串格式的考勤号码(用于显示和数据交换)
//不同的TLog数据格式请修改该函数
char *fmtPinLog(TAttLog *log)
{
	//U32 upin;
	static char pin[40];
	memset(pin,0,sizeof(pin));
#ifdef SSRDB
	//printf("log PIN=%d, PIN2=%s\n", log->PIN, log->PIN2);
	if(log->PIN2[0])
	{
		int i;
		memcpy(pin, log->PIN2, sizeof(log->PIN2));
		for(i=0;i<sizeof(pin);i++)
		{
			if(pin[i]==0) break;
			if(pin[i]<' ') pin[i]='*';
		}
	}
	else
		sprintf(pin,"%d", log->PIN);
#else
	upin=log->PIN;
	if(gOptions.PIN2Width>5)
	{
		TUser user;
		if(FDB_GetUser(log->PIN, &user))
		{
//			printf("log PIN=%d, PIN2=%d\n", log->PIN, user.PIN2);
			if(user.PIN2) upin=user.PIN2;
		}
	}
	sprintf(pin, "%d", upin);
#endif
	return pin;
}

typedef struct{
	int startNum;
	int curNum;
	TBuffer *buffer;
}TAttLogFilter;

int filterAttLog(void *data, int count, void *param)
{
	TAttLog *l=(TAttLog*)data;
	TAttLogFilter *f=(TAttLogFilter *)param;
	f->curNum++;
	if(f->curNum>f->startNum)
	{//如果考勤记录不被删除，则把它缓存起来
		if(FDB_IsOKLog(l))
		{
			writeBufferAuto(f->buffer, (char *)l,
#ifdef SSRDB
				sizeof(TAttLog)
#else
				gOptions.AttLogExtendFormat?sizeof(TExtendAttLog):sizeof(TAttLog)
#endif
				);
			return 0;
		}
	}
	return 1;
}

#if 0
//del by zengrr 2010-9-15 for not using this mothod
//删除旧的考勤记录
//delCount 指定删除多少记录
int FDB_DelOldAttLog(int delCount)
{
	int c;
	//TAttLog *log;
	TAttLogFilter filter;
	printf("Delete %d old att logs, count=%d\n",delCount, FDB_CntAttLog());
	if(delCount<0) return 0;

	memset(&filter, 0, sizeof(filter));
	filter.startNum=delCount;
	filter.curNum=0;
	filter.buffer=createRamBuffer(10*1024);
	c=FDB_ForAllData(FCT_ATTLOG, filterAttLog, NO_LIMIT, &filter);
	if(c>0)//真有记录被删除
	{
		printf("delete count=%d\n", c);
		FDB_AttLogTruncAndSave((char *)filter.buffer->buffer, bufferSize(filter.buffer));

	}
	freeBuffer(filter.buffer);
	printf(" %d old att logs reserved\n",FDB_CntAttLog());
	return c;
}
#endif

int IsUserDataNeedUpload(TUser *user)
{
	if(!ISADMINUSER(*user))
		return TRUE;
//	if(gOptions.UploadAdminData) 
		return TRUE;
//	return FALSE;
}

int IsUserDataNeedDownload(char *pin, TUser *user)
{
//	if(gOptions.DownloadAdminData) return TRUE;
//	if(user)
//		return !ISADMINUSER(*user);
	return TRUE;
}


