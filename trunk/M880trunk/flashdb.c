/*************************************************

  ZEM 200

  flashdb.c define all functions for database mangement of flash

  Copyright (C) 2003-2005, ZKSoftware Inc.

  Add Batch update,database upgrade automatic for ZKFPV10 by ZhangHonggen 2009.11.20

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
#include "flashdb2.h"
#include "utils.h"
#include "options.h"
#include "main.h"
#include "sensor.h"
#include "arca.h"
#define _HAVE_TYPE_BYTE
#define _HAVE_TYPE_DWORD
#define _HAVE_TYPE_WORD
#include "ssrcommon.h"
#include "exfun.h"
#include <sys/vfs.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include "Des.h"		//3DES加密算法

#ifdef FACE
#include "facedb.h"
#endif

#include "strutils.h"
#include "utils2.h"
#include "pushapi.h"
#include "flashdb.h"

#define USER_ID_MAP_SIZE ((0xFFFF>>3)+1)
#define USER_ID_MAP_BITS (USER_ID_MAP_SIZE<<3)

typedef struct _FSizes_{  //
	int Total, TotalSector,
	    SectorCnt, SectorFree,
	    UserCnt, UserFree,
	    TmpCnt, TmpFree,
	    AttLogCnt, AttLogFree,
	    OpLogCnt, OpLogFree,
	    AdminCnt, PwdCnt,
	    StdTmp, StdUser, StdLog,
	    ResTmp, ResUser, ResLog,
	    FaceTmpCnt, FaceTmpFree,StdFaceTmp;
}GCC_PACKED TFSizes, *PFSizes;

//record index
U32 recindex=0;

//transactions
int fdTransaction=-1;
int fdOldTransaction=-1;
int fdExtLog=-1;
int fdFingerTmp=-1;
int fdUser=-1;
int fdUser1=-1;
int fdOldUser=-1;
int fdExtUser=-1;
//operate log
int fdOpLog=-1;
int fdSms=-1;
int fdUData=-1;
int fdAlarm=-1;
int fdWkcd=-1;
int fdSTKey=-1;



int userdatacuroffset_s=-1;//add for gridview 2012-10-18
int userdatacuroffset_e=-1;//add for gridview 2012-10-18
PUserlb ppuserlb = NULL; //add by zxz 2012-10-19
int userrowindex = 0;	//add by zxz 2012-10-19

//门禁功能相关文件句柄<liming>
int fdtz=-1;		//时间段
int fdhtz=-1;		//节假日
int fdgroup=-1;		//组
int fdcgroup=-1;		//开锁组合

//升级使用
static int fdtz1=-1;		//时间段
static int fdhtz1=-1;		//节假日
int fdgroup1=-1;		//组
int fdcgroup1=-1;		//开锁组合

/*异地考勤,add by yangxiaolong,2011-06-14,start*/
int fdUserDLTime = -1;	//保存异地考勤用户信息下载时间
/*异地考勤,add by yangxiaolong,2011-06-14,end*/
//照片索引文件句柄<liming>
int fdpicidx=-1;
static int fdpicid=-1; //seiya china
static unsigned char gBoardSMS[1024];
static int gBoardSMSPos=0;

static int attLogReadPos=LOG_READ_NONE;
static int opLogReadPos=LOG_READ_NONE;

static int fdbig5code=-1;
//add by liq for fuliye
int fdserialcardno = -1;
int fdbatchdata=-1;  /*软件高速下发指纹和用户时，临时文件2013-01-05 zxz*/
static int* gUserPinOffSet;/*用户索引*/

//transaction storage format
//4字节短时间格式 4 byte short time format
#define AttLogSize1 4
//8字节完整时间格式 8 byte long time format
#define AttLogSize2 8
//|0      |1       |2          |3      |
//+----------------+---+--+-+----------+--------------------------------+
//    UID                             16 bits user id
//                  ST                3  bits status
//                      VT            2  bits verification type
//                         ST         1  bit  time format，1-short time type，0-long time type
//                              Time  10 bit  times, short time type mean elapsed seconds since the last long time type(0-1024)，long time format is 0
//long time format add 4 byte UNIX times, that is total seconds from 1970-1-1 00:00:00

// ExtendAttLogFormat
//|0 	|4 	|8     		|9	|12 	|16
//+------+---------+-----------------+--------------------------------+
//PIN 							//4Bytes userid or pin2
//	DATETIME						//4Bytes UNIX times total seconds from 1970-1-1 00:00:00
//		STATUS					//1Byte
//			VERI				//1Byte	verification type
//				Reserved			//2Bytes
//					WORKCODE		//4Bytes

//当前最高用户号码
//static int UserMaxID=1;
//当前考勤记录基准时间
time_t BaseTime=0;

BYTE *UserIDMap=NULL;
#define USERID_MAP_SIZE (0xFFFF)


char PRIVALUES[4]={PRIVILLEGE0, PRIVILLEGE1, PRIVILLEGE2, PRIVILLEGE3};

#define IsAttLogLongPack(value) (((value & 0x04)==0))
#define IsValidUser(buf) ((((PUser)(buf))->Privilege & PRI_VALID)==PRI_VALID)
#define EmptyData(buf) ((get_unaligned((U32*)(buf)))==0xFFFFFFFF)

int UnpackAttLog(char *buf, POldAttLog log);
int PackAttLog(char *buf, POldAttLog log);
int UnpackNewAttLog(char *buf, PAttLog log);

void SearchFirst(PSearchHandle sh);
BOOL SearchNext(PSearchHandle sh);
int GetRecordSize(int ContentType, int index);
void MAP_ResetMapList(int ContentType);
int UTF8toLocal(int lid,const unsigned char *utf8, unsigned char *str);
void SearchNewFirst(PSearchHandle sh);
int FDB_DelSerialCardNumber(U16 pin);
PSerialCardNo FDB_GetSerialCardNumber(U16 pin, PSerialCardNo serialno);
#ifdef WEBSERVER
int * pSelectFDFromConentType(int ContentType) /******** Add For Web Server ********/
{
	if (FCT_ATTLOG==ContentType){
		return (int*)&fdTransaction;
	}
	else if (FCT_FINGERTMP==ContentType){
		return (int*)&fdFingerTmp;
	}
	else if (FCT_USER==ContentType){
		return (int*)&fdUser;
	}
	else if (FCT_OPLOG==ContentType){
		return (int*)&fdOpLog;
	}
	else if (FCT_SMS==ContentType){
		return (int*)&fdSms;
	}
	else if (FCT_UDATA==ContentType){
		return (int*)&fdUData;
	}
	else if (FCT_EXTUSER==ContentType){
		return (int*)&fdExtUser;
	}	
	//For face
#ifdef FACE
	else if(FCT_FACE == ContenType){
		return (int*)&fdface;
	}
	else if(FCT_FACEPOS==ContenType){
		return (int*)&fdfacepos;
	}
#endif
	//end face

	return NULL;
}
#endif

//get file handle from File type
int SelectFDFromConentType(int ContentType)
{
	if (FCT_ATTLOG==ContentType){
		return fdTransaction;
	}
	else if (FCT_FINGERTMP==ContentType){
		return fdFingerTmp;
	}
	else if (FCT_USER==ContentType){
		return fdUser;
	}
	else if (FCT_OldUSER==ContentType){
		return fdOldUser;
	}
	else if (FCT_OPLOG==ContentType){
		return fdOpLog;
	}	
	else if (FCT_SMS==ContentType){
		return fdSms;
	}
	else if (FCT_UDATA==ContentType){
		return fdUData;
	}
	else if (FCT_EXTUSER==ContentType){
		return fdExtUser;
	}
	//alarm<liming>
	else if(FCT_ALARM==ContentType || FCT_BELL==ContentType){
		return fdAlarm;
	}
	//workcode<liming>
	else if(FCT_WORKCODE == ContentType){
		return fdWkcd;
	}
	//shortkey<liming>
	else if(FCT_SHORTKEY == ContentType){
		return fdSTKey;
	}
	//time zone<liming>
	else if(FCT_TZ == ContentType){
		return fdtz;
	}
	//holiday<liming>
	else if(FCT_HTZ == ContentType){
		return fdhtz;
	}
	//group<liming>
	else if(FCT_GROUP == ContentType){
		return fdgroup;
	}
	//clock group<liming>
	else if(FCT_CGROUP == ContentType){
		return fdcgroup;
	}
	else if(FCT_PHOTOINDEX == ContentType){
		return fdpicidx;
	}
	//seiya dlphoto
	else if(FCT_PHOTOID == ContentType){
		return fdpicid;
	}
	//seiya dlphoto end
	//For face
#ifdef FACE
	else if(FCT_FACEPOS == ContentType){
		return fdfacepos;
	}
	else if(FCT_FACE == ContentType){
		return fdface;
	}
#endif
	//end face
	/*异地考勤,add by yangxiaolong,2011-06-14,start*/
	else if (FCT_USER_DL_TIME == ContentType)
	{
		return fdUserDLTime;
	}
	/*异地考勤,add by yangxiaolong,2011-06-14,end*/
	
	else if(FCT_SERIALCARDNO == ContentType)//add by liq for card serial number
	{
		return fdserialcardno;
	}
	return -1;
}

void RefreshJFFS2Node(int ContentType, int REFRESH_JFFS2_NODE)
{
	TSearchHandle sh;
	U8 buf[1024];
	char *tmpdata;
	int datasize=0;
	int offset=0;
	int count=0;
	int fd;

	fd = SelectFDFromConentType(ContentType);

	sh.ContentType=ContentType;
	sh.buffer=(char *)buf;
	SearchFirst(&sh);

	if(CurAttLogCount&&((CurAttLogCount%REFRESH_JFFS2_NODE)==0))
	{
		while(!SearchNext(&sh))
		{
			count++;
			if((count%REFRESH_JFFS2_NODE)==1)
			{
				offset=lseek(fd, 0, SEEK_CUR)-sh.bufferlen;
			}
		}

		if(count&&((count%REFRESH_JFFS2_NODE)==0)) //??? CurAttLogCount count  % REFRESH_JFFS2_NODE
		{
			datasize=lseek(fd, 0, SEEK_CUR)-offset;
			tmpdata=MALLOC(datasize);
			lseek(fd, offset, SEEK_SET);
			if(read(fd, tmpdata, datasize)==datasize)
			{
				lseek(fd, offset, SEEK_SET);
				write(fd, tmpdata, datasize);
			}
			// DBPRINTF("count=%d datasize=%d offset=%d\n", count, datasize, offset);
			FREE(tmpdata);
			fsync(fd);
		}
	}
}

//
#define FlashBlockLimit 80
static void ReloadDataBase(int ContentType)
{
	char buf[80];
	switch(ContentType)
	{
		case FCT_FINGERTMP:
			close(fdFingerTmp);
#ifndef ZEM510
#ifndef ZEM600
			if (useSDCardFlag || ((gOptions.MaxAttLogCount*10000) > ATTLOGLIMIT))
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO);
			else
#endif
#endif
				fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO);
			break;
		case FCT_USER:
			close(fdUser);

			fdUser=open(GetEnvFilePath("USERDATAPATH", "ssruser.dat", buf), O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO);
			break;
	}
}

static int GetBlockUsed(int ContentType)
{
	struct statfs s;
	long blocks_used=0;
	long blocks_percent_used=0;
	int ret=-1;

	if (ContentType==FCT_FINGERTMP)
		ret = statfs("/mnt/mtdblock/data", &s);
	else if (ContentType==FCT_USER)
	{
		if (useSDCardFlag || ((gOptions.MaxAttLogCount*10000) > ATTLOGLIMIT))
			ret = statfs("/mnt/mtdblock/data", &s);
		else
			ret = statfs("/mnt/mtdblock", &s);
	}

	if (ret==0)
	{
		if ((s.f_blocks > 0))
		{
			blocks_used = s.f_blocks - s.f_bfree;
			blocks_percent_used = 0;
			if (blocks_used+s.f_bavail)
			{
				blocks_percent_used = (((long long)blocks_used)*100+(blocks_used+s.f_bavail)/2)/(blocks_used+s.f_bavail);
			}
		}
		//printf("disk used: %ld\n",blocks_percent_used);
		return blocks_percent_used;
	}

	return -1;

}


void RefreshDataBase(int ContentType, int MaxGroupCount)
{
	char* ptmpBuf=NULL;
	TSearchHandle sh;
	int bufsize;
	int  i, j, k;
	char* tmpCacheBuf;
	int RecordCount;
	int CurGroupRecordCnt;
	int fd=-1;

	if (GetBlockUsed(ContentType) >= FlashBlockLimit)
	{
		ReloadDataBase(ContentType);
		bufsize = GetRecordSize(ContentType, 1);
		ptmpBuf = MALLOC(bufsize);
		fd = SelectFDFromConentType(ContentType);
		RecordCount=0;

		sh.ContentType = ContentType;
		sh.buffer = (void*)ptmpBuf;
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen) RecordCount++;
		}
		SearchFirst(&sh);

		j=1;
		k=0;
		tmpCacheBuf = MALLOC(MaxGroupCount*bufsize);
		while (k < RecordCount)
		{
			CurGroupRecordCnt = MaxGroupCount;
			if ((k + MaxGroupCount) > RecordCount)
				CurGroupRecordCnt = RecordCount - k;

			read(fd, tmpCacheBuf, CurGroupRecordCnt*bufsize);
			k += CurGroupRecordCnt;
			//printf("GROUP %d Starting......\n", j);

			for (i=0; i< CurGroupRecordCnt; i++)
			{
				if (ContentType==FCT_FINGERTMP)
					sh.buffer = (char*)((PTemplate)tmpCacheBuf+i);
				else if (ContentType==FCT_USER)
					sh.buffer = (char*)((PUser)tmpCacheBuf+i);
			}

			//printf("GROUP %d Writting......\n", j);
			lseek(fd, -1*CurGroupRecordCnt*bufsize, SEEK_CUR);
			write(fd, tmpCacheBuf, CurGroupRecordCnt*bufsize);
			printf("GROUP %d Endded\n", j);
			j++;
		}
		FREE(ptmpBuf);
		FREE(tmpCacheBuf);
		fsync(fd);

		ReloadDataBase(ContentType);
		sync();
		printf("Finish Template Space Compress\n");
	}
}

//append or overwrite data to file
int SearchAndSave(int ContentType, char *buffer, U32 size)
{
	int fd;
	int rc;
	char *cbuf=NULL;

	fd = SelectFDFromConentType(ContentType);
	switch(ContentType)
	{
		case FCT_ATTLOG:
#ifndef ZEM600
#ifndef ZEM510
			RefreshJFFS2Node(ContentType, 200);
#endif
#endif
			if (gOptions.LoopWriteRecordOn && (recindex > (gOptions.MaxAttLogCount*10000))) {
				lseek(fd, 0, SEEK_SET);
				lseek(fd, (recindex % (gOptions.MaxAttLogCount*10000))*sizeof(TAttLog), SEEK_CUR);
			} else {
				lseek(fd, 0, SEEK_END);
			}
			break;
		case FCT_USER:
#ifndef ZEM600
#ifndef ZEM510
			if (gOptions.MaxUserCount*100 > USERLIMIT)
				RefreshDataBase(ContentType, 110);
#endif
#endif
			if (FDB_GetUser(0, NULL))
				lseek(fd, -1*sizeof(TUser), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_FINGERTMP:
#ifndef ZEM600
#ifndef ZEM510
			RefreshDataBase(ContentType, 200);
#endif
#endif
			if (FDB_GetTmp(0, 0, NULL))
				lseek(fd, -1*sizeof(TTemplate), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
#ifdef FACE
		case FCT_FACE:
			if (FDB_GetFaceTmp(0, 0 , NULL))
				lseek(fd, -1*sizeof(TFaceTmp), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_FACEPOS:
			if (FDB_GetFaceRecord(0, 0 , NULL))
				lseek(fd, -1*sizeof(TFacePos), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
#endif

		case FCT_OPLOG:
			lseek(fd, 0, SEEK_END);
			break;
		case FCT_SMS:
			if (FDB_GetSms(0, NULL))
				lseek(fd, -1*sizeof(TSms), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_UDATA:
			if (FDB_GetUData(0, NULL))
				lseek(fd, -1*sizeof(TUData), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_EXTUSER:
			if (FDB_GetExtUser(0, NULL))
				lseek(fd, -1*sizeof(TExtUser), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_ALARM:
			if (FDB_GetAlarm(0, NULL))
				lseek(fd, -1*sizeof(ALARM), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_BELL:
			if (FDB_GetBell(0, NULL))
				lseek(fd, -1*sizeof(TBellNew), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_WORKCODE:
			if (FDB_GetWorkCode(0, NULL))
				lseek(fd, -1*sizeof(TWORKCODE),SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_SHORTKEY:
			if (FDB_GetShortKey(0, NULL))
				lseek(fd, -1*sizeof(TSHORTKEY),SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_TZ:
			if (FDB_GetTimeZone(0, NULL)) {
				lseek(fd, -1*sizeof(TTimeZone),SEEK_CUR);
			} else {
				lseek(fd, 0, SEEK_END);
			}
			break;
		case FCT_HTZ:
			if (FDB_GetHTimeZone(0, NULL))
				lseek(fd, -1*sizeof(THTimeZone),SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
		case FCT_GROUP:
			if (FDB_GetGroup(0, NULL)) {
				lseek(fd, -1*sizeof(TGroup),SEEK_CUR);
			} else {
				lseek(fd, 0, SEEK_END);
			}
			break;
		case FCT_CGROUP:
			if (FDB_GetCGroup(0, NULL)) {
				lseek(fd, -1*sizeof(TCGroup),SEEK_CUR);
			} else {
				lseek(fd, 0, SEEK_END);
			}
			break;
		case FCT_PHOTOINDEX:
			lseek(fd, 0, SEEK_END);
			break;
			//seiya dlphoto
		case FCT_PHOTOID:
			lseek(fd, 0, SEEK_END);
			break;
			//seiya dlphoto end
			/*异地考勤,add by yangxiaolong,2011-06-14,start*/
		case FCT_USER_DL_TIME:
			if (FDB_GetUserDLTime(0, NULL) == FDB_OK) {
				lseek(fd, -1*sizeof(TUserDLTime), SEEK_CUR);
			} else {
				lseek(fd, 0, SEEK_END);
			}
			break;
			/*异地考勤,add by yangxiaolong,2011-06-14,end*/
		case FCT_SERIALCARDNO:
			if (FDB_GetSerialCardNumber(0, NULL))
				lseek(fd, -1*sizeof(TSerialCardNo), SEEK_CUR);
			else
				lseek(fd, 0, SEEK_END);
			break;
	}

	rc=((write(fd, buffer, size)==size)?FDB_OK:FDB_ERROR_IO);
	if (rc == FDB_ERROR_IO) {
		/*When write failure, write again*/
		rc=((write(fd, buffer, size)==size)?FDB_OK:FDB_ERROR_IO);
	}
	fsync(fd);
	sync();

	if (rc==FDB_OK) {
		if (ContentType==FCT_ATTLOG) {
			cbuf=malloc(size);
			lseek(fd, -1*size, SEEK_CUR);
			rc=((read(fd, cbuf, size)==size && memcmp(cbuf,buffer,size)==0) ? FDB_OK:FDB_ERROR_IO);
			if(rc==FDB_OK) {
				CurAttLogCount++;
			}
			free(cbuf);
			cbuf = NULL;
		}

		if (pushsdk_is_running()) {
		//	if(ContentType==FCT_ATTLOG || ContentType==FCT_OPLOG || ContentType==FCT_PHOTOINDEX)
			if(ContentType==FCT_ATTLOG || ContentType==FCT_PHOTOINDEX)
			{
				pushsdk_data_new(ContentType);
			}
		}
	} else {
		char buf[128];
		memset(buf, 0, sizeof(buf));
		if (ContentType==FCT_ATTLOG) {
			sprintf(buf, "Type=%d, error, User:%s, t:%ld", ContentType, ((PAttLog)buffer)->PIN2, ((PAttLog)buffer)->time_second);
		} else {
			sprintf(buf, "ContentType=%d, error", ContentType);
		}
		write_tracemsg(buf);
	}

	return rc;
}

void SearchOffSize(PSearchHandle sh, off_t curfp)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, curfp, SEEK_SET);
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}

void SearchFirst(PSearchHandle sh)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, 0, SEEK_SET);
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}

void SearchLast(PSearchHandle sh)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, 0, SEEK_END);
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}

BOOL SearchNext(PSearchHandle sh)
{
	BOOL eof;

	eof = TRUE;
	sh->bufferlen=0;
	sh->datalen=0;
	switch(sh->ContentType)
	{
		case FCT_ATTLOG:
			{
				char flag=0;
				if (read(sh->fd, sh->buffer, sizeof(TAttLog))==sizeof(TAttLog))
				{
					flag=1;
					sh->bufferlen=sizeof(TAttLog);
					sh->datalen=sh->bufferlen;
				}

				if (flag)
				{
					eof = FALSE;
				}
				else
				{
					lseek(sh->fd, 0, SEEK_END);
				}

				break;
			}
		case FCT_USER:
			{
				//off_t iCurPos=lseek(sh->fd,0,SEEK_CUR);
				if (read(sh->fd, sh->buffer, sizeof(TUser))==sizeof(TUser)){
					sh->bufferlen=sizeof(TUser);
					if (((PUser)sh->buffer)->PIN) //pin > 0
						sh->datalen=sh->bufferlen;
					eof = FALSE;
				}
				break;
			}
		case FCT_OldUSER:
			if (read(sh->fd, sh->buffer, sizeof(TOldUser))==sizeof(TOldUser)){
				sh->bufferlen=sizeof(TOldUser);
				if (((POldUser)sh->buffer)->PIN) //pin > 0
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_FINGERTMP:
			{
				//off_t iCurPos=lseek(sh->fd,0,SEEK_CUR);
				if (read(sh->fd, sh->buffer, sizeof(TTemplate))==sizeof(TTemplate)){
					sh->bufferlen=sizeof(TTemplate);
					if (((PTemplate)sh->buffer)->Valid)
						sh->datalen=((PTemplate)sh->buffer)->Size;
					eof = FALSE;
				}
				break;
			}
#ifdef FACE
		case FCT_FACE:
			if (read(sh->fd, sh->buffer, sizeof(TFaceTmp))==sizeof(TFaceTmp))
			{
				sh->bufferlen=sizeof(TFaceTmp);
				if (((PFaceTmp)sh->buffer)->PIN)
				{
					sh->datalen=((PFaceTmp)sh->buffer)->Size;
				}
				eof = FALSE;
			}
			break;
		case FCT_FACEPOS:
			if (read(sh->fd, sh->buffer, sizeof(TFacePos))==sizeof(TFacePos))
			{
				sh->bufferlen=sizeof(TFacePos);
				if (((PFacePos)sh->buffer)->PIN)
				{
					sh->datalen=sizeof(TFacePos);
				}
				eof = FALSE;
			}
			break;
#endif
		case FCT_OPLOG:
			if (read(sh->fd, sh->buffer, sizeof(TOPLog))==sizeof(TOPLog)){
				sh->bufferlen=sizeof(TOPLog);
				sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			else
			{
				lseek(sh->fd, 0, SEEK_END);
			}
			break;
		case FCT_SMS:
			if (read(sh->fd, sh->buffer, sizeof(TSms))==sizeof(TSms)){
				sh->bufferlen=sizeof(TSms);
				if (((PSms)sh->buffer)->ID)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_ALARM:
			if(read(sh->fd, sh->buffer, sizeof(ALARM))==sizeof(ALARM)){
				sh->bufferlen=sizeof(ALARM);
				if(((PAlarm)sh->buffer)->AlarmIDX)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_BELL:
			if (read(sh->fd, sh->buffer, sizeof(TBellNew))==sizeof(TBellNew))
			{
				sh->bufferlen=sizeof(TBellNew);
				if(((PBellNew)sh->buffer)->BellID)
					sh->datalen=sh->bufferlen;
				eof=FALSE;
			}
			break;
		case FCT_WORKCODE:
			if(read(sh->fd, sh->buffer, sizeof(TWORKCODE))==sizeof(TWORKCODE))
			{
				sh->bufferlen=sizeof(TWORKCODE);
				if(((PWorkCode)sh->buffer)->PIN)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_SHORTKEY:	//<2007.7.10>
			if(read(sh->fd, sh->buffer, sizeof(TSHORTKEY))==sizeof(TSHORTKEY))
			{
				sh->bufferlen=sizeof(TSHORTKEY);
				if(((PShortKey)sh->buffer)->keyID)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;

		case FCT_UDATA:
			if (read(sh->fd, sh->buffer, sizeof(TUData))==sizeof(TUData)){
				sh->bufferlen=sizeof(TUData);
				if (((PUData)sh->buffer)->PIN)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_EXTUSER:
			if (read(sh->fd, sh->buffer, sizeof(TExtUser))==sizeof(TExtUser)){
				sh->bufferlen=sizeof(TExtUser);
				if (((PExtUser)sh->buffer)->PIN) //pin > 0
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_DEPT:
			if (read(sh->fd, sh->buffer, sizeof(TDept))==sizeof(TDept)){
				sh->bufferlen=sizeof(TDept);
				if (((Pdept)sh->buffer)->Deptid) //deptid > 0
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_SCH:
			if (read(sh->fd,sh->buffer,sizeof(TTimeClass)) == sizeof(TTimeClass)){
				sh->bufferlen = sizeof(TTimeClass);
				if (((PTimeClass)sh->buffer)->schClassid)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_TZ:
			if(read(sh->fd, sh->buffer, sizeof(TTimeZone))==sizeof(TTimeZone))
			{
				sh->bufferlen=sizeof(TTimeZone);
				if(((PTimeZone)sh->buffer)->ID)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_HTZ:
			if(read(sh->fd, sh->buffer, sizeof(THTimeZone))==sizeof(THTimeZone))
			{
				sh->bufferlen=sizeof(THTimeZone);
				if(((PHTimeZone)sh->buffer)->ID)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_GROUP:
			if(read(sh->fd, sh->buffer, sizeof(TGroup))==sizeof(TGroup))
			{
				sh->bufferlen=sizeof(TGroup);
				if(((PGroup)sh->buffer)->ID)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_CGROUP:
			if(read(sh->fd, sh->buffer, sizeof(TCGroup))==sizeof(TCGroup))
			{
				sh->bufferlen=sizeof(TCGroup);
				if(((PCGroup)sh->buffer)->ID)
					sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
		case FCT_PHOTOINDEX:
			if(read(sh->fd, sh->buffer, sizeof(TPhotoIndex))==sizeof(TPhotoIndex))
			{
				sh->bufferlen = sizeof(TPhotoIndex);
				if (((PhotoIndex)sh->buffer)->index)
					sh->datalen = sh->bufferlen;
				eof = FALSE;
			}
			break;

			//seiya dlphoto
		case FCT_PHOTOID:
			if (read(sh->fd, sh->buffer, sizeof(TPhotoID))==sizeof(TPhotoID)){
				sh->bufferlen=sizeof(TPhotoID);
				sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
			//seiya dlhpoto end
			/*异地考勤,add by yangxiaolong,2011-06-14,start*/
			//用户信息保存一段时间处理
		case FCT_USER_DL_TIME:
			if (read(sh->fd, sh->buffer, sizeof(TUserDLTime))==sizeof(TUserDLTime))
			{
				sh->bufferlen=sizeof(TUserDLTime);
				sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
			break;
			/*异地考勤,add by yangxiaolong,2011-06-14,end*/
		case FCT_SERIALCARDNO:
			if (read(sh->fd, sh->buffer, sizeof(TSerialCardNo))==sizeof(TSerialCardNo))
			{
				sh->bufferlen=sizeof(TSerialCardNo);
				sh->datalen=sh->bufferlen;
				eof = FALSE;
			}
	}
	return eof;
}

BOOL FDB_IsEmpty(int ContentType)
{
	return (lseek(SelectFDFromConentType(ContentType), 0, SEEK_END)?TRUE:FALSE);
}

void ResolveLostRecord()
{
	int TotalAttLogSize=0;
	int CurAttLogSize=0;

	//解决丢记录问题 2007.11.03
	if (CurAttLogCount <= 0) {
		return;
	}

	CurAttLogSize = CurAttLogCount*(sizeof(TAttLog));
	TotalAttLogSize = lseek(fdTransaction,0,SEEK_END);
	printf("attlogsize:%d-%d\n",CurAttLogSize, TotalAttLogSize);

	//may be bad  record
	//printf("TotalAttLogSize:%d\n",TotalAttLogSize-CurAttLogSize);
	if (CurAttLogSize < TotalAttLogSize) {
		time_t t;
		TAttLog fixattlog;

		GetTime(&gCurTime);
		t=OldEncodeTime(&gCurTime);
		memset(&fixattlog,0,sizeof(TAttLog));
		fixattlog.time_second = t;
		lseek(fdTransaction,-1*(TotalAttLogSize-CurAttLogSize),SEEK_CUR);
		if (write(fdTransaction,&fixattlog,sizeof(TAttLog))==sizeof(TAttLog))
		{
			fsync(fdTransaction);
			CurAttLogCount++;
			//printf("fixok\n");
		}
	}
}

//整理用户数据表
static TUser gtUser;
void ResolvedUserDataErr(int min, int max)
{
	char buf[128];
	sprintf(buf,"%stmpuser.dat",RAMPATH);

	int fdtmp = open(buf, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	int getcount=0, lostcount=0;

	TSearchHandle sh;
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gtUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PUser)sh.buffer)->PIN > min)// && ((PUser)sh.buffer)->PIN <= max)
		{
			//printf("new PIN:%d, PIN2:%s, Name:%s\n", ((PUser)sh.buffer)->PIN, ((PUser)sh.buffer)->PIN2, ((PUser)sh.buffer)->Name);
			if (write(fdtmp, (void*)sh.buffer, sizeof(TUser))==sizeof(TUser))
			{
				sync();
				getcount++;
			}
		}
		else
			lostcount++;
	}
	close(fdtmp);
	printf("GetCount:%d, LostCount:%d\n", getcount, lostcount);
}

int FDB_InitDBs(BOOL OpenSign)
{
	char buf[1024];
	TSearchHandle sh;

	//UserMaxID=0;
	BaseTime=0;
	CurAttLogCount=0;
	memset(gBoardSMS, 0, sizeof(gBoardSMS));

	if (OpenSign)
	{
		fdUser=open(GetEnvFilePath("USERDATAPATH","ssruser.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
#ifndef ZEM510
#ifndef ZEM600
		if (useSDCardFlag || ((gOptions.MaxAttLogCount*10000) > ATTLOGLIMIT))
		{
			fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);

		}
		else
#endif
#endif
		{
			fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
#ifndef ZEM510
#ifndef ZEM600
			fdUser1=open(GetEnvFilePath("USERDATAPATH", "data/ssruser.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
#endif
#endif
		}
#ifdef FACE
		if(gOptions.FaceFunOn) //add by caona for face
		{
			fdface=open(GetEnvFilePath("USERDATAPATH", "ssrface.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
			fdfacepos=open(GetEnvFilePath("USERDATAPATH", "ssrfacepos.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		}
#endif
		fdExtLog=open(GetEnvFilePath("USERDATAPATH", "data/extlog.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);

		fdTransaction=open(GetEnvFilePath("USERDATAPATH", "data/ssrattlog.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdOldTransaction=open(GetEnvFilePath("USERDATAPATH", "data/transaction.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdOldUser=open(GetEnvFilePath("USERDATAPATH", "data/user.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdOpLog=open(GetEnvFilePath("USERDATAPATH", "data/oplog.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdSms=open(GetEnvFilePath("USERDATAPATH", "data/sms.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdAlarm=open(GetEnvFilePath("USERDATAPATH","data/alarm.dat",buf),O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdWkcd=open(GetEnvFilePath("USERDATAPATH","data/wkcd.dat",buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdSTKey=open(GetEnvFilePath("USERDATAPATH","data/stkey.dat",buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdUData=open(GetEnvFilePath("USERDATAPATH", "data/udata.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		fdExtUser=open(GetEnvFilePath("USERDATAPATH", "data/extuser.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);

		fdtz1 = open(GetEnvFilePath("USERDATAPATH", "data/timezone.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdhtz1 = open(GetEnvFilePath("USERDATAPATH", "data/htimezone.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdgroup1 = open(GetEnvFilePath("USERDATAPATH", "data/group.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdcgroup1 = open(GetEnvFilePath("USERDATAPATH", "data/lockgroup.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		//for webserver 2008.09.23
		fdtz = open(GetEnvFilePath("USERDATAPATH", "timezone.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdhtz = open(GetEnvFilePath("USERDATAPATH", "htimezone.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdgroup = open(GetEnvFilePath("USERDATAPATH", "group.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdcgroup = open(GetEnvFilePath("USERDATAPATH", "lockgroup.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);

		sprintf(buf, "%s/picindex.dat", GetCapturePath("capture"));
		fdpicidx = open(buf, O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		//seiya dlphoto
		sprintf(buf, "%s/picid.dat", GetCapturePath("capture"));
		fdpicid = open(buf, O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		//seiya dlphoto end

		/*异地考勤,add by yangxiaolong,2011-06-14,start*/
		if (gOptions.RemoteAttFunOn == 1)
		{
			fdUserDLTime = open(GetEnvFilePath("USERDATAPATH", "data/ssruserdltime.dat", buf), \
					O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		}
		/*异地考勤,add by yangxiaolong,2011-06-14,end*/
		/*add by liq for save card serial number*/
		fdserialcardno = open(GetEnvFilePath("USERDATAPATH", "serialcardnumber.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);



#ifdef WEBSERVER
		unlink("myweb/attloghtm.htm");
		unlink("myweb/attloghtm2.htm");
		unlink("myweb/yichang.htm");
		unlink("myweb/tongji.htm");
		unlink("myweb/yueyichang.htm");
		sync();
#endif

	}

	U32 idid=0;
	///get base time from transactions
	sh.ContentType=FCT_ATTLOG;
	sh.buffer=buf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PAttLog)sh.buffer)->PIN && gOptions.LoopWriteRecordOn)
		{
			memcpy(&idid, ((PAttLog)sh.buffer)->reserved, 4);
			if (ntohl(idid)>recindex){
				recindex=idid;
			}
			printf("recindex=%d\n",recindex);
		}
		//在统计考勤记录的时候进行考勤记录正确性验证。
		//if(FDB_IsOKLog((PAttLog)sh.buffer))
		CurAttLogCount++;
	}
	printf("CurAttLogCount:%u, recindex:%u\n", CurAttLogCount, recindex);

// remove by jigc 2013.7.9
//删除 ResolveLostRecord，该API不但起不到修复考勤缺失功能而且，还可能破坏考勤记录文件 
//	ResolveLostRecord();
// remove end
#if 0
	//get max user idint FDB_ClrDuressTagTmpAll(void);
	sh.ContentType=FCT_USER;
	sh.buffer=buf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		id=((PUser)sh.buffer)->PIN;
		if(id>UserMaxID) UserMaxID=id;
	}
#endif
	if(UserIDMap==NULL){
		UserIDMap = (U8*)malloc(USER_ID_MAP_SIZE);
		printf("flashdb malloc UserIDMap memery:  %d byte\n",USER_ID_MAP_SIZE);
	}
	FDB_UpdateUserIDMap();
	return FDB_OK;
}

void FDB_FreeDBs(void)
{
	close(fdTransaction);
	close(fdExtLog);
	close(fdFingerTmp);
	close(fdUser);
	close(fdUser1);
	close(fdOldTransaction);
	close(fdOldUser);
	close(fdOpLog);
	close(fdSms);
	close(fdUData);
	close(fdExtUser);
	close(fdAlarm);
	close(fdWkcd);
	close(fdSTKey);
	close(fdtz1);
	close(fdhtz1);
	close(fdgroup1);
	close(fdcgroup1);
	close(fdtz);
	close(fdhtz);
	close(fdgroup);
	close(fdcgroup);
	close(fdpicidx);
	close(fdpicid);	//seiya dlphoto
	close(fdserialcardno);//add by liq

#ifdef FACE
	if(gOptions.FaceFunOn)
	{
		close(fdface);
		close(fdfacepos);
	}
#endif

	if (UserIDMap) FREE(UserIDMap);
}

int GetDataInfo(int ContentType, int StatType, int Value)
{
	int tmp=0;
	char buf[1024];
	TSearchHandle sh;

	memset(buf,0,sizeof(buf));
	sh.ContentType=ContentType;
	sh.buffer=buf;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		switch(StatType)
		{
			case STAT_COUNT:
				if (sh.datalen>0) tmp++;
				break;
			case STAT_VALIDLEN:
				tmp+=sh.datalen;
				break;
			case STAT_CNTADMINUSER:
				if ((sh.datalen>0)&&(ISADMIN(((PUser)sh.buffer)->Privilege))) tmp++;
				break;
			case STAT_CNTADMIN:
				if ((sh.datalen>0)&&(Value & (((PUser)sh.buffer)->Privilege))) tmp++;
				break;
			case STAT_CNTPWDUSER:
				if ((sh.datalen>0)&&(((PUser)sh.buffer)->Password[0])) tmp++;
				break;
			case STAT_CNTTEMPLATE:
				if ((sh.datalen>0)&&((((PTemplate)sh.buffer)->PIN)==Value)) tmp++;
				break;
			case STAT_CNTDEPTUSER:
				if ((sh.datalen>0)&&(((PUser)sh.buffer)->Group == Value)) tmp++;
				break;
		}
	}
	return tmp;
}

int TruncFDAndSaveAs(int fd, char *filename, char *buffer, int size)
{
	if (fd > 0) close(fd);
	fd = open(filename, O_RDWR|O_CREAT|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if (fd && buffer!=NULL)
	{
		write(fd, buffer, size);
		fsync(fd);
	}
	return fd;
}

int adduser(PUser user)
{
	if (FDB_GetUser(0, NULL))
		lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
	else
		lseek(fdUser, 0, SEEK_END);
		write(fdUser, (char*)user, sizeof(TUser));
}
int addlog(PAttLog log)
{
	lseek(fdTransaction, 0, SEEK_END);
	write(fdTransaction, (char*)log, sizeof(TAttLog));
}
int FDB_AttLogTruncAndSave(char *Data,unsigned long Size)
{
	char buf[180];
	if(Data)
	{
		fdTransaction = TruncFDAndSaveAs(fdTransaction, GetEnvFilePath("USERDATAPATH", "data/ssrattlog.dat", buf), Data, Size);
		CurAttLogCount=Size/sizeof(TAttLog);
	}
	else
		CurAttLogCount=0;
	fsync(fdTransaction);
	return fdatasync(fdTransaction);
}

static int DeleteUserPhoto(char* upin2)
{
	char commandstr[256];
	memset(commandstr, 0, sizeof(commandstr));

	if (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))==0)
	{
		DoUmountUdisk();
		if (DoMountUdisk() != 0) return 0;
	}
	if (upin2)
		sprintf(commandstr, "rm %s%s.jpg -rf && sync", GetPhotoPath(""), upin2);
	else
		sprintf(commandstr, DELALL, GetPhotoPath(""),GetPhotoPath(""));

	//        printf("%s---%s\n",__FUNCTION__, commandstr);
	return (systemEx(commandstr)==EXIT_SUCCESS);
}

int FDB_AdjustAttOffset(PAttLog attlog)
{
	TSearchHandle sh;
	TAttLog log;
	int value = 0;

	memset((void*)&sh, 0, sizeof(TSearchHandle));
	memset((void*)&log, 0, sizeof(TAttLog));

	sh.ContentType=FCT_ATTLOG;
	sh.buffer = (char *)&log;

	SearchFirst(&sh);
	while(!SearchNext(&sh))	{
		if (memcmp((void*)attlog, (void*)&log, sh.bufferlen) == 0) {
			value = 1;
			break;
		}
	}
	if (value == 1) {
		off_t offset = lseek(sh.fd, 0, SEEK_CUR);
		if (offset > 0) {
			value = (offset / sizeof(TAttLog));
			if (value > 0) {
				value -= 1;
			}
		}
	}
	SaveInteger("AttOffset", value);
	return value;
}

int FDB_DelOldAttLog(int delCount)
{
	int fd = 0;
	int size = 0;
	int pin = 0;
	int offset = 0;
	TAttLog log;

	if(delCount<0) {
		return 0;
	}
/*
	//zsliu add 2009-07-14 解决打开扩展记录的时候，不能正常删除记录的问题.
	if(gOptions.AttLogExtendFormat) {
		fd = fdExtLog;
	} else {
		fd = fdTransaction;
	}
*/
	fd = fdTransaction;		//change by zxz 2012-12-28彩屏已经不需要扩展记录，考勤记录统一使用fdTransaction
	printf("Start to delete the old attendence logs\n");
#ifdef SSRDB
	size = sizeof(TAttLog);
#else
	size = ((gOptions.AttLogExtendFormat>0) ? sizeof(TExtendAttLog) : 8);
#endif
	offset = LoadInteger("AttOffset", 0);

	printf("last record offset=%d\n", offset);
	
	memset((void*)&log, 0, size);
	if (offset > 0) {
		if (offset <= delCount) {
			 SaveInteger("AttOffset", 0);
		} else {
			lseek(fd, offset*sizeof(TAttLog), SEEK_SET);
			if (read(fd, (void*)&log, size) == size) {
				pin = log.PIN;
			}
			printf("PIN=%d, PIN2=%s\n", log.PIN, log.PIN2);
		}
	}

	delFileHead(fd, delCount*size);
	CurAttLogCount=FDB_CntAttLog();
	
	if (pin > 0) {
		FDB_AdjustAttOffset(&log);
	}

	printf("FDB_DelOldAttLog() %d old att logs reserved\n--------------------------\n", CurAttLogCount);
	return CurAttLogCount;
}

extern int LastLogsCount;
extern PAlarmRec CurAlarmRec;
int FDB_ClearData_helper(int ContentType, int delTemp)
{
	char buf[80], buf1[80];

	if ((ContentType==FCT_ALL) || (ContentType==FCT_ATTLOG))
	{	
		fdTransaction = TruncFDAndSaveAs(fdTransaction, GetEnvFilePath("USERDATAPATH", "data/ssrattlog.dat", buf), NULL, 0);
		fdExtLog = TruncFDAndSaveAs(fdExtLog, GetEnvFilePath("USERDATAPATH", "data/extlog.dat", buf), NULL, 0);
		memset(CurAlarmRec, 0, gOptions.MaxUserCount*100*sizeof(TAlarmRec));
		CurAttLogCount=0;
		LastLogsCount=0;
		recindex = 0;
		BaseTime=0;
		//add by liq 
		sprintf(buf1, "rm -f %s %s", "/mnt/mtdblock/BsAtt.dat");
		systemEx(buf1);
		SaveInteger("AttOffset", 0);
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_FINGERTMP)) 
	{	
		fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp,GetEnvFilePath("USERDATAPATH","data/template.dat",buf),NULL,0);

		if(gOptions.ZKFPVersion==ZKFPV10)	
		{
			if(fhdl)
				BIOKEY_DB_CLEAREX(fhdl);	
		}
		else
		{
			sprintf(buf, "rm -f %s %s", "/mnt/mtdblock/templatev10.dat", "/mnt/mtdblock/tempinfo.dat");
			systemEx(buf);
		}
	}
#ifdef FACE
	if ((ContentType==FCT_ALL) || (ContentType==FCT_FACE))
	{
		if(gOptions.FaceFunOn) //add by caona for face
		{
			FaceDBChg=1;
			fdface = TruncFDAndSaveAs(fdface, GetEnvFilePath("USERDATAPATH", "ssrface.dat", buf), NULL, 0);
			fdfacepos = TruncFDAndSaveAs(fdfacepos, GetEnvFilePath("USERDATAPATH", "ssrfacepos.dat", buf), NULL, 0);
			FaceDBReset();
		}
	}

#endif
	if ((ContentType==FCT_ALL) || (ContentType==FCT_USER))
	{
		fdUser = TruncFDAndSaveAs(fdUser, GetEnvFilePath("USERDATAPATH", "ssruser.dat", buf), NULL, 0);
		fdExtUser = TruncFDAndSaveAs(fdExtUser, GetEnvFilePath("USERDATAPATH", "data/extuser.dat", buf), NULL, 0);

		//清除用户照片
		if (gOptions.IsSupportPhoto)
		{
			DeleteUserPhoto(NULL);
		}

		if (ContentType==FCT_USER)
		{
			if(delTemp)
			{
				fdFingerTmp = TruncFDAndSaveAs(fdFingerTmp,GetEnvFilePath("USERDATAPATH","data/template.dat",buf),NULL,0);

				if(gOptions.ZKFPVersion==ZKFPV10)
				{
					if(fhdl)
						BIOKEY_DB_CLEAREX(fhdl);
				}
				else
				{
					sprintf(buf, "rm -f %s %s", "/mnt/mtdblock/templatev10.dat", "/mnt/mtdblock/tempinfo.dat");
					systemEx(buf);
				}
			}
#ifdef FACE
			if(gOptions.FaceFunOn) //add by caona for face
			{
				FaceDBChg=1;
				fdface = TruncFDAndSaveAs(fdface, GetEnvFilePath("USERDATAPATH", "ssrface.dat", buf), NULL, 0);
				fdfacepos = TruncFDAndSaveAs(fdfacepos, GetEnvFilePath("USERDATAPATH", "ssrfacepos.dat", buf), NULL, 0);
				FaceDBReset();
			}	

#endif
		}
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_OPLOG))
		fdOpLog = TruncFDAndSaveAs(fdOpLog, GetEnvFilePath("USERDATAPATH", "data/oplog.dat", buf), NULL, 0);
	if ((ContentType==FCT_ALL) || (ContentType==FCT_SMS))
	{
		fdSms = TruncFDAndSaveAs(fdSms, GetEnvFilePath("USERDATAPATH", "data/sms.dat", buf), NULL, 0);
		fdUData = TruncFDAndSaveAs(fdUData, GetEnvFilePath("USERDATAPATH", "data/udata.dat", buf), NULL, 0);
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_UDATA))
	{
		fdUData = TruncFDAndSaveAs(fdUData, GetEnvFilePath("USERDATAPATH", "data/udata.dat", buf), NULL, 0);
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_WORKCODE))
	{
		fdWkcd = TruncFDAndSaveAs(fdWkcd, GetEnvFilePath("USERDATAPATH", "data/wkcd.dat", buf), NULL, 0);
	}
	if ((ContentType==FCT_ALL) || (ContentType==FCT_PHOTOINDEX))
	{
		sprintf(buf, "%s/picindex.dat", GetCapturePath("capture"));
		fdpicidx = TruncFDAndSaveAs(fdpicidx, buf, NULL, 0);
		//seiya dlphoto
		sprintf(buf, "%s/picid.dat", GetCapturePath("capture"));
		fdpicid = TruncFDAndSaveAs(fdpicid, buf, NULL, 0);
		//seiya dlphoto end
	}
	if ((ContentType==FCT_SHORTKEY))
	{
		fdSTKey = TruncFDAndSaveAs(fdSTKey, GetEnvFilePath("USERDATAPATH", "data/stkey.dat", buf), NULL, 0);
	}
	//liming 2007.08
	if ((ContentType==FCT_ALARM) || (ContentType==FCT_BELL))
	{
		fdAlarm = TruncFDAndSaveAs(fdAlarm, GetEnvFilePath("USERDATAPATH", "data/alarm.dat", buf), NULL, 0);
	}
	if ( (ContentType==FCT_TZ))
	{
		fdtz = TruncFDAndSaveAs(fdtz, GetEnvFilePath("USERDATAPATH", "timezone.dat", buf), NULL, 0);
	}
	if ( (ContentType==FCT_HTZ))
	{
		fdhtz = TruncFDAndSaveAs(fdhtz, GetEnvFilePath("USERDATAPATH", "htimezone.dat", buf), NULL, 0);
	}
	if ((ContentType==FCT_GROUP))
	{
		fdgroup = TruncFDAndSaveAs(fdgroup, GetEnvFilePath("USERDATAPATH", "group.dat", buf), NULL, 0);
	}
	if ((ContentType==FCT_CGROUP))
	{
		fdcgroup = TruncFDAndSaveAs(fdcgroup, GetEnvFilePath("USERDATAPATH", "lockgroup.dat", buf), NULL, 0);
	}

	if(ContentType==FCT_ALL) {
		BaseTime=0;
		//UserMaxID=0;
	}

	if(ContentType==FCT_ALL || ContentType==FCT_USER) {	
		FDB_UpdateUserIDMap();
		//UserMaxID=0;
	}

	if((ContentType==FCT_ALL || ContentType==FCT_USER || ContentType==FCT_FINGERTMP) 
		&& (!gOptions.IsOnlyRFMachine && fhdl))	{
			BIOKEY_DB_CLEAR(fhdl);
	}

	if (pushsdk_is_running()) {
		pushsdk_data_reset(ContentType);	//for ADMS
	}

	//flush the cached data to disk
	sync();

	return FDB_OK;
}

int FDB_ClearData(int ContentType)
{	
	//修改该函数。适应 dataapi 接口		zhangwei	2008-7-11
	return FDB_ClearData_helper(ContentType, 1);
}

int FDB_ClearData2(int ContentType)
{
	return FDB_ClearData_helper(ContentType, 0);
}

int FDB_GetSizes(char* Sizes)
{
	PFSizes p=(PFSizes)Sizes;

	memset((void*)p, 0, sizeof(TFSizes));
	p->UserCnt=GetDataInfo(FCT_USER, STAT_COUNT, 0);

	p->AttLogCnt=GetDataInfo(FCT_ATTLOG, STAT_COUNT, 0);
	p->TmpCnt=FDB_CntTmp();
	p->OpLogCnt=GetDataInfo(FCT_OPLOG, STAT_COUNT, 0);
	p->AdminCnt=FDB_CntAdminUser();
	p->PwdCnt=FDB_CntPwdUser();
	p->StdTmp=gOptions.MaxFingerCount*100;
	p->StdUser=gOptions.MaxUserCount*100;
	p->StdLog=gOptions.MaxAttLogCount*10000;
	p->ResTmp=p->StdTmp-p->TmpCnt;
	p->ResUser=p->StdUser-p->UserCnt;
	p->ResLog=p->StdLog-p->AttLogCnt;
#ifdef FACE
	if (gOptions.FaceFunOn)
	{
		p->FaceTmpCnt=FDB_CntFaceUser();
		p->StdFaceTmp=gOptions.MaxFaceCount*100;
	}
	else
	{
		p->FaceTmpCnt=0;
		p->StdFaceTmp=0;
	}
#endif

	return sizeof(TFSizes);
}

//shortkey <2007.7.10>
static TSHORTKEY gShortKey;
PShortKey FDB_GetShortKey(U8 id, PShortKey stkey)
{
	TSearchHandle sh;

	sh.ContentType=FCT_SHORTKEY;
	sh.buffer=(char*)&gShortKey;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PShortKey)sh.buffer)->keyID==id)
		{
			if (stkey)
			{
				memcpy(stkey, sh.buffer, sizeof(TSHORTKEY));
				return stkey;
			}
			else
				return (PShortKey)sh.buffer;
		}
	}
	return NULL;
}

PShortKey FDB_GetShortKeyByState(U8 code, PShortKey stkey)
{
	TSearchHandle sh;

	sh.ContentType=FCT_SHORTKEY;
	sh.buffer=(char*)&gShortKey;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PShortKey)sh.buffer)->stateCode==code)
		{
			if (stkey)
			{
				memcpy(stkey, sh.buffer, sizeof(TSHORTKEY));
				return stkey;
			}
			else
				return (PShortKey)sh.buffer;
		}
	}
	return NULL;
}

int FDB_CntShortKey(void)
{
	return GetDataInfo(FCT_SHORTKEY,STAT_COUNT,0);
}

PShortKey FDB_CreateShortKey(PShortKey stkey,U8 id,U8 fun,U8 sCode, char* sName,U16 t1,U16 t2,U16 t3,U16 t4,U16 t5,U16 t6,U16 t7,U8 bchg)
{
	memset((void*)stkey,0,sizeof(TSHORTKEY));

	stkey->keyID = id;
	stkey->keyFun = fun;
	stkey->stateCode = sCode;
	if(sName) nstrcpy(stkey->stateName, sName, STATE_NAME_LEN);

	stkey->Time1 = t1;
	stkey->Time2 = t2;
	stkey->Time3 = t3;
	stkey->Time4 = t4;
	stkey->Time5 = t5;
	stkey->Time6 = t6;
	stkey->Time7 = t7;
	stkey->autochange = bchg;

	return stkey;
}

int FDB_AddShortKey(PShortKey stkey)
{
	return SearchAndSave(FCT_SHORTKEY, (char*)stkey, sizeof(TSHORTKEY));
}

int FDB_ChgShortKey(PShortKey stkey)
{
	PShortKey sk;
	int ret;

	if ((sk=FDB_GetShortKey(stkey->keyID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)stkey, (void*)sk, sizeof(TSHORTKEY))) return FDB_OK;

	//overwrite
	lseek(fdSTKey, -1*sizeof(TSHORTKEY), SEEK_CUR);
	if (write(fdSTKey, (void*)stkey, sizeof(TSHORTKEY))==sizeof(TSHORTKEY))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;
	fsync(fdSTKey);

	return ret;
}

PUser FDB_CreateUser(PUser user, U16 pin, char *name, char *passwd, int privillege)
{
	memset((void*)user,0,sizeof(TUser));

	if(name) nstrcpy(user->Name, name, MAXNAMELENGTH);
	if(passwd) nstrcpy(user->Password, passwd, 8);
	user->PIN=pin;
	user->Privilege=privillege;

	//默认分配到存在的第一个组
	if(gOptions.LockFunOn)
	{
		int i=0;
		TGroup tgp;
		for(i=0;i<GP_MAX;i++)
		{
			memset(&tgp,0,sizeof(TGroup));
			if(FDB_GetGroup(i+1,&tgp)!=NULL)
			{
				user->Group=tgp.ID;
				break;
			}
		}
	}

	return user;
}

int FDB_AddUser(PUser user)
{
	if(SearchAndSave(FCT_USER, (char*)user, sizeof(TUser))==FDB_OK)
	{
		SetBit((BYTE*)UserIDMap, user->PIN);
		return FDB_OK;
	}
	else
	{
		return FDB_ERROR_IO;
	}
}
/*U盘上传人员基本信息速度优化，add by yangxiaolong,2011-10-20,start*/
/*
 *Function:FDB_GetUserWithOff
 *Description:根据传入文件偏移开始查找指定pin
 *Input Prama:offset-- 
 *			>=0--从提供offset处开始查找写入位置
 *Output Prama:offset--文件当前偏移
 *Return:1 --执行成功
 0--执行失败
 */
int  FDB_GetUserWithOff(U16 pin, PUser user, off_t offset)
{
	TSearchHandle sh;
	TUser  userbuf;

	sh.ContentType = FCT_USER;
	sh.buffer = (char*) & userbuf;

	sh.fd = SelectFDFromConentType(sh.ContentType);
	lseek(sh.fd, offset, SEEK_SET);
	while (!SearchNext(&sh))
	{
		if (userbuf.PIN == pin)
		{
			if (user)
			{
				memcpy(user, sh.buffer, sizeof(TUser));
				return 1;
			}
			else
			{
				return 1;
			}
		}
	}
	return 0;
}
/*
 *Function:FDB_AddUserFromUsb
 *Description:将U盘内用户人员数据写入数据库中，需指名开始写入偏移
 *Input Prama:offset-- 
 *			-1--直接从文件末尾开始添加数据
 *			>=0--从提供offset处开始查找写入位置
 *Output Prama:offset--文件当前偏移
 *Return:FDB_OK --执行成功
 FDB_ERROR_IO--执行失败
 */
int FDB_AddUserFromUsb(PUser user, off_t *offset)
{
	int fd;
	int rc;
	//static time_t  backStamp = 0;

	fd = SelectFDFromConentType(FCT_USER);
#ifndef ZEM600
#ifndef ZEM510
	if (gOptions.MaxUserCount*100 > USERLIMIT)
		RefreshDataBase(FCT_USER, 110);
#endif
#endif
	if ((*offset != -1) && (FDB_GetUserWithOff(0, NULL, *offset) == 1))
	{
		lseek(fd, -1*sizeof(TUser), SEEK_CUR);
	}
	else
	{
		lseek(fd, 0, SEEK_END);
	}
	printf("user pin-%d, PIN2=%s\n", user->PIN, user->PIN2);
	rc = ((write(fd, user, sizeof(TUser)) == sizeof(TUser)) ? \
			(SetBit((BYTE*)UserIDMap, user->PIN),FDB_OK) : FDB_ERROR_IO);
	fsync(fd);
	*offset = lseek(fd, 0, SEEK_CUR);

	return rc;
}
/*U盘上传人员基本信息速度优化，add by yangxiaolong,2011-10-20,end*/
int FDB_ChgUser(PUser user)
{
	PUser u;
	int ret;
	if((u=FDB_GetUser(user->PIN, NULL))==NULL) return FDB_ERROR_NODATA;
	if(0==memcmp((void*)user, (void*)u, sizeof(TUser))) return FDB_OK;
	//overwrite user
	lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
	if (write(fdUser, (void*)user, sizeof(TUser))==sizeof(TUser))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdUser);


	return ret;
}

int FDB_ChgUserByPIN(U16 PIN, U8 Privilege)
{
	PUser u;
	int ret;
	if((u=FDB_GetUser(PIN, NULL))==NULL) return FDB_ERROR_NODATA;
	if(0==memcmp((void*)(&Privilege), (void*)(&(u->Privilege)), sizeof(U8))) return FDB_OK;
	u->Privilege=Privilege;
	//overwrite user
	lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
	if (write(fdUser, (void*)u, sizeof(TUser))==sizeof(TUser))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdUser);


	return ret;
}

int FDB_DelUser(U16 pin)
{
	PUser u;
	int uid;
	char uPIN2[30];

	if ((u=FDB_GetUser(pin, NULL))==NULL) return FDB_ERROR_NODATA;

	uid = u->PIN;

	memset(uPIN2, 0, sizeof(uPIN2));
	memcpy(uPIN2, u->PIN2, gOptions.PIN2Width);

	//overwrite user
	lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
	u->PIN=0;
	if (write(fdUser, (void*)u, sizeof(TUser))==sizeof(TUser))
	{
		fsync(fdUser);
		FDB_DelSerialCardNumber((U16)uid);
		if (gOptions.UserExtendFormat) FDB_DelExtUser((U16)uid);
		if (gOptions.IsSupportPhoto) DeleteUserPhoto(uPIN2);	//删除用户照片
		ClearBit((BYTE*)UserIDMap, pin);

#ifdef FACE
		if(gOptions.FaceFunOn) //add by caona for face
			FDB_DeleteFaceTmps(uid);
#endif

		return FDB_DeleteTmps((U16)uid);
	}
	return FDB_ERROR_IO;
}

static TUser gUser;
PUser FDB_GetUser(U16 pin, PUser user)
{
	TSearchHandle sh;

	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh)) {
		if(((PUser)sh.buffer)->PIN==pin) {
			if (user) {
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			} else {
				return (PUser)sh.buffer;
			}
		}
	}
	return NULL;
}

PUser GetUserToBuf(char *userbuf)
{
	TSearchHandle sh;
	int pin=0;

	sh.ContentType=FCT_USER;
	sh.buffer=( char*)&gUser;
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		pin = ((PUser)sh.buffer)->PIN;
		if(pin>0)
		{
			if (((PUser)sh.buffer)->Name[0])
				memcpy((char*)userbuf, ((PUser)sh.buffer)->Name,24);
		}
	}
	return NULL;
}

void InsertAndSort(int *buffer, int UserNo, int count)
{
	int i = 0;
	int j = 0;
	int pin = 0;

	if (count == 0) {
		*buffer = UserNo;
		return;
	}

	j = count-1;
	if (*(buffer+j) < UserNo) {
		*(buffer+count) = UserNo;
		return;
	}

	for (i = j; i>=0; i--) {
		pin = *(buffer+i);
		if (pin < UserNo) {
			break;
		}
		*(buffer+i+1) = pin;
		*(buffer+i) =  UserNo;

	}
	return;
}

int GetFreePIN2FromRam(char *pin2)
{
	TSearchHandle sh;
	int *pCacheUser = NULL;
	int j = 0;
	int i = 0;
	int k = 0;
	int bSign = FALSE;
	int memflag = 0;

	j = FDB_CntUserByMap()+1;
	printf("ImageBufferLength=%d, cnt=%d\n", ImageBufferLength, (j*sizeof(int)) );

	if (!gOptions.IsOnlyRFMachine && ImageBufferLength >= j*sizeof(int)) {
			pCacheUser = (int*)gImageBuffer;
	} else {
		pCacheUser = malloc(j * sizeof(int));
		if (pCacheUser == NULL){
			return -1;
		}
		memflag = 1;
	}

	/*read PIN2 and sort, when insert PIN2, will filter incorrect PIN2. for example: A123 or more than 0x7FFFFFFF */
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen <= 0) {
			continue;
		}
		j = atoi(gUser.PIN2);
		if (j > 0 && j < 0x7FFFFFFF) {
			InsertAndSort(pCacheUser, j, i++);
		}
	}

	/*build a free PIN2*/

	for(k = 1; k <= i; k++) {
		bSign = FALSE;
		for (j=0; j<i; j++) {
			if (*(pCacheUser+j) == k) {
				bSign = TRUE;
				break;
			}
		}
		if(bSign == FALSE) {
			sprintf(pin2, "%d", k);
			bSign = TRUE;
			break;
		} else {
			bSign = FALSE;
		}
	}
	
	if (!bSign) {
		sprintf(pin2, "%d", (i+1));
	}

	/*free memory*/
	if (memflag) {
		free(pCacheUser);
		pCacheUser=NULL;
	}
	return bSign;
}

PUser FDB_GetUserByPIN2(U32 pin2, PUser user)
{
	TSearchHandle sh;

	//printf("%s pin2 %d\n",__FUNCTION__,pin2);
	if(pin2<=0) return NULL;
	if(pin2>MAX_PIN2) return NULL;
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if ((sh.datalen>0) && (atoi(((PUser)sh.buffer)->PIN2)==pin2))
		{
			if (user)
			{
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			}
			else
				return (PUser)sh.buffer;
		}
	}

	return NULL;
}

//2006.08.03 ssr
PUser FDB_GetUserByCharPIN2(char *pin2, PUser user)
{
	TSearchHandle sh;
	if(pin2[0]==0) return NULL;
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if ((sh.datalen>0) && (strcmp(((PUser)sh.buffer)->PIN2,pin2))==0)
		{
			if (user)
			{
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			}
			else
			{
				return (PUser)sh.buffer;
			}
		}
	}
	return NULL;
}

PUser FDB_GetUserByNameAndAcno(char *name, char *pin2,PUser user)
{
	TSearchHandle sh;

	if(pin2[0]==0) return NULL;
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if ((sh.datalen>0) && (strcmp(((PUser)sh.buffer)->PIN2,pin2))==0 && ((strcmp(((PUser)sh.buffer)->Name,name))==0))
		{
			if (user)
			{
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			}
			else
			{
				return (PUser)sh.buffer;
			}
		}
	}
	return NULL;
}

PUser FDB_GetUserByName(char *name,PUser user)
{
	TSearchHandle sh;

	if(name[0]==0) return NULL;
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if ((sh.datalen>0) && (strcmp(((PUser)sh.buffer)->Name,name))==0)
		{
			if (user)
			{
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			}
			else
			{
				return (PUser)sh.buffer;
			}
		}
	}
	return NULL;
}

PUser FDB_GetUserByCard(BYTE *card, PUser user)
{
	TSearchHandle sh;

	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if((sh.datalen>0) && (nmemcmp((((PUser)sh.buffer)->Card), card, 4)==0)) {
			if (user) {
				memcpy(user, sh.buffer, sizeof(TUser));
				return user;
			} else {
				return (PUser)sh.buffer;
			}
		}
	}
	return NULL;
}

int FDB_CntAdminUser(void)
{
	return GetDataInfo(FCT_USER, STAT_CNTADMINUSER, 0);
}

int FDB_CntAdmin(int Privilege)
{
	return GetDataInfo(FCT_USER, STAT_CNTADMIN, Privilege);
}

int FDB_CntPwdUser(void)
{
	return GetDataInfo(FCT_USER, STAT_CNTPWDUSER, 0);
}

int FDB_ClrUser(void)
{
	return FDB_ClearData(FCT_USER);
}

int FDB_CntUser(void)
{
	return GetDataInfo(FCT_USER, STAT_COUNT, 0);
}
int FDB_CntPhotoCount(void)
{
	return GetDataInfo(FCT_PHOTOINDEX, STAT_COUNT, 0);
}

void CopyTemplate(PTemplate dest, PTemplate src)
{
	dest->PIN=src->PIN;
	dest->FingerID=src->FingerID;
	memcpy(dest->Template, src->Template, MAXTEMPLATESIZE);
	dest->Valid=src->Valid;
	dest->Size=src->Size;
}

PZKFPTemplate FDB_CreateTemplate(char* tmp, U16 pin, char FingerID, char *Template, int TmpLen,int Valid)
{
	if(gOptions.ZKFPVersion==ZKFPV10)
	{
		PZKFPTemplate ptmp=(PZKFPTemplate)tmp;

		ptmp->PIN=pin;
		ptmp->FingerID=(BYTE)FingerID;
		ptmp->Valid=Valid;
		ptmp->Size=TmpLen;

		memset(ptmp->Template, 0, ZKFPV10_MAX_FP_LEN);
		memcpy(ptmp->Template, (BYTE*)Template, TmpLen);
	}
	else
	{
		PTemplate ptmp=(PTemplate)tmp;

		ptmp->PIN=pin;
		ptmp->FingerID=(BYTE)FingerID;
		ptmp->Valid=Valid;

		memset(ptmp->Template, 0, MAXTEMPLATESIZE);
		if (TmpLen>MAXVALIDTMPSIZE)
			TmpLen=BIOKEY_SETTEMPLATELEN((BYTE *)Template, MAXVALIDTMPSIZE);
		memcpy(ptmp->Template, (BYTE*)Template, TmpLen);
		//Make a 4-Byte alignment for all template data
		ptmp->Size=((TmpLen+3+6)/4)*4-6;
	}
	return (PZKFPTemplate)tmp;
}

int FDB_AddTmp(char *tmp)
{
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		return FDB_AddTmpV10((PZKFPTemplate)tmp);
	}else
	{
		TTemplate obj;
		CopyTemplate(&obj, (PTemplate)tmp);
		obj.Size+=6;
		return SearchAndSave(FCT_FINGERTMP, (char*)&obj, sizeof(TTemplate));
	}
}


int FDB_AddTmpV10(PZKFPTemplate tmp)
{
	//zsliu add
	/*
	   int ret = BIOKEY_TEMPLATELEN(tmp->Template);
	   if(ret <= 0)
	   {
	   printf("FDB_AddTmpV10: BIOKEY_TEMPLATELEN(tmp->Template) ret is %d\n", ret);
	   return FDB_ERROR_DATA;
	   }
	   */

	int Custom=1;

	if(tmp->FingerID==ZKFPV10_FINGER_INDEX)
	{
		TZKFPTemplate tmpv10[10];
		BYTE *tmps[10];
		int i,fingerCount=0,sizes[10]={0};
		for(i=0;i<10;i++)
			tmps[i] = tmpv10[i].Template;
		FDB_DeleteTmps(tmp->PIN);
		tmp->FingerID=0;
		BIOKEY_SPLIT_TEMPLATE(tmp->Template,tmps,&fingerCount,sizes);
		for(i=0;i<fingerCount;i++)
		{
			int fid = i%10;
			if(BIOKEY_DB_ADDEX(fhdl,tmp->PIN | (fid<<16),sizes[i],tmps[i]))
			{
				Custom=1;
				BIOKEY_SET_CUSTOMDATA(fhdl, tmp->PIN|fid<<16, (char *)&Custom, CUSTOM_DATA_LENGTH);
				//pushsdk_data_new(FCT_FINGERTMP);	//for ADMS
			}else
				return FDB_ERROR_IO;
		}
		return FDB_OK;
	}
	else
	{
		//printf("dddddddddd  add biokey  new  : pin=%d, fpid=%d size=%d\n",tmp->PIN,tmp->FingerID,tmp->Size);
		//BIOKEY_DB_DEL(fhdl,tmp->PIN | (tmp->FingerID<<16));
		if(BIOKEY_DB_ADDEX(fhdl, tmp->PIN|(tmp->FingerID<<16), tmp->Size, (BYTE*)tmp->Template))
		{
			Custom=tmp->Valid;
			BIOKEY_SET_CUSTOMDATA(fhdl, tmp->PIN|tmp->FingerID<<16, (char *)&Custom, CUSTOM_DATA_LENGTH);
			//pushsdk_data_new(FCT_FINGERTMP);	//for ADMS
			return FDB_OK;
		}
		else
		{
			printf("_____%s-%d____FDB_ERROR_IO\n", __FILE__, __LINE__);
			return FDB_ERROR_IO;
		}
	}
}


int DeleteFingerData(void)
{
	int size=sizeof(TTemplate);
	TTemplate deltmp;
	int ret;

	memset(&deltmp,0,sizeof(TTemplate));
	//overwrite templates set valid = 0
	lseek(fdFingerTmp, -1*size, SEEK_CUR);
	if (write(fdFingerTmp, (void*)&deltmp, sizeof(TTemplate))==sizeof(TTemplate))
	{
		fsync(fdFingerTmp);
		return FDB_OK;
	}
	else
	{
		lseek(fdFingerTmp, size, SEEK_CUR);
		ret = FDB_ERROR_IO;
	}

	fsync(fdFingerTmp);
	return ret;

}

int FDB_DelTmp(U16 UID, char FingerID)
{
	int Len ;
	if(gOptions.ZKFPVersion == ZKFPV10 )
	{
		if(FingerID==ZKFPV10_FINGER_INDEX)
			return FDB_DeleteTmps(UID);

		Len =FDB_GetTmp(UID,FingerID,NULL);
		if(Len<=0)
		{
			return FDB_OK;
		}
		//int ret= BIOKEY_DB_DEL(fhdl,UID|FingerID<<16);
		if(fhdl &&  (BIOKEY_DB_DEL(fhdl,UID|FingerID<<16)))
			return FDB_OK;
		else
			return FDB_ERROR_IO;
	}
	else
	{
		if (FDB_GetTmp(UID, FingerID, NULL)==0)
			return FDB_OK;
		else
			return DeleteFingerData();
	}
}

static U8 gTemplate[1024];
U32 FDB_GetTmps(U16 UID, char* tmp)
{
	if(fhdl)
	{
		PZKFPTemplate ptmp=(PZKFPTemplate)tmp;
		TZKFPTemplate tmpv10[10];
		BYTE *tmps[10];
		int i,j=0,tmplen=0,tmplen1=0;

		for(i=0;i<10;i++)
			tmps[i] = tmpv10[i].Template;

		memset(tmpv10,0,sizeof(tmpv10));
		for(i=0;i<gOptions.MaxUserFingerCount;i++)
		{
			if(BIOKEY_DB_GET_TEMPLATE(UID,i,tmps[i],&tmplen))
			{
				j++;
				tmplen1 = tmplen;
			}
		}
		if(j>1)
		{
			tmplen = BIOKEY_MERGE_TEMPLATE(tmps,j,ptmp->Template);
		}
		else
		{
			tmplen = tmplen1;
			memcpy(ptmp->Template,tmps[0],tmplen);
		}

		ptmp->Size = tmplen;
		ptmp->PIN = UID;
		ptmp->FingerID = ZKFPV10_FINGER_INDEX;
		ptmp->Valid=1;
		return tmplen;
	}
	return 0;
}

void FDB_dataLast(int ContentType)
{
	TSearchHandle sh;
	sh.ContentType=ContentType;
	sh.buffer=NULL;
	SearchLast(&sh);
	return;
}

//U32 FDB_GetTmp(U16 UID, char FingerID, PTemplate tmp)
U32 FDB_GetTmp(U16 UID, char FingerID, char* tmp)
{
	TSearchHandle sh;
	sh.ContentType=FCT_FINGERTMP;
	if( gOptions.ZKFPVersion == ZKFPV10)
	{
		int tmplen=0;
		int Custom=0;
		int CustomLen=CUSTOM_DATA_LENGTH;
		PZKFPTemplate ptmp=NULL;
		TZKFPTemplate tmpv10;

		if(tmp==NULL)
			ptmp=&tmpv10;
		else
			ptmp=(PZKFPTemplate)tmp;
		if(FingerID==ZKFPV10_FINGER_INDEX)
		{
			return FDB_GetTmps(UID, (char*)ptmp);
		}

		if( fhdl)
		{
			if(BIOKEY_DB_GET_TEMPLATE(UID,FingerID,ptmp->Template,&tmplen))
			{
				ptmp->Size = tmplen;
				ptmp->PIN = UID;
				ptmp->FingerID = FingerID;
				if(BIOKEY_GET_CUSTOMDATA(fhdl,UID|FingerID<<16,(char *)&Custom,&CustomLen))
					ptmp->Valid = Custom;
				else
					ptmp->Valid=1;
				return tmplen;
			}
		}
		return 0;
	}


	if (tmp==NULL)
		sh.buffer=(char *)gTemplate;
	else
		sh.buffer=(char *)tmp;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (sh.datalen==0){
			if (UID==0)
				return sh.bufferlen;
			else
				continue;
		}
		if((((PTemplate)sh.buffer)->PIN==UID) && (((PTemplate)sh.buffer)->FingerID==FingerID))
		{
			((PTemplate)sh.buffer)->Size-=6;
			return sh.datalen;
		}
	}
	return 0;
}
U32 FDB_GetTmpCountToBuf(int *tmpcountbuf)
{
	TSearchHandle sh;
	U16 tmppin;

	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		if(!fhdl) //add by caona for IsOnlyRFMachine=1
			return 0;

		int count=0, i;
		int UserIDs[gOptions.MaxUserCount*1000];
		BIOKEY_GET_PARAM(fhdl, ZKFP_PARAM_CODE_USERCOUNT, &count);
		BIOKEY_GET_PARAM(fhdl, ZKFP_PARAM_CODE_USERIDS, UserIDs);

		for(i=0; i<count; i++)
		{
			tmppin = UserIDs[i];
			tmpcountbuf[tmppin]=FDB_GetTmpCnt(UserIDs[i]);
		}
		return 1;
	}

	sh.ContentType=FCT_FINGERTMP;
	sh.buffer=(char *)gTemplate;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		tmppin=((PTemplate)sh.buffer)->PIN;
		if ((sh.datalen>0) && (tmppin>0))
			tmpcountbuf[tmppin]++;
		//printf("tmppin:%d\tcount:%d\n",tmppin,tmpcountbuf[tmppin]);
	}
	return 1;
}

//ssr 2006.06 calculate tmpcount by pin
U32 FDB_GetTmpCount(U16 UID)
{
	return GetDataInfo(FCT_USER, STAT_CNTTEMPLATE, UID);
}

int FDB_DeleteTmps(U16 UID)
{
	TSearchHandle sh;
	int i;

	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		if(!fhdl) //add by caona for IsOnlyRFMachine=1
			return FDB_ERROR_IO;
		int ret=0;
		for(i=0;i<MAX_USER_FINGER_COUNT;i++)
		{
			ret = BIOKEY_DB_DEL(fhdl,UID|i<<16);
			//printf("FDB_DeleteTmps UID=%d, ret=%d\n", UID, ret);
			//if(ret==0) zsliu change 20091210
			//	return FDB_ERROR_IO;
		}
		return FDB_OK;
	}

	sh.ContentType=FCT_FINGERTMP;
	sh.buffer=(char *)gTemplate;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (sh.datalen==0) continue;
		if (((PTemplate)sh.buffer)->PIN==UID)
		{
			if(FDB_OK!=DeleteFingerData())
				return FDB_ERROR_IO;
		}
	}

	return FDB_OK;
}

int FDB_ClrDuressTagTmpAll(void)
{
	TSearchHandle sh;
	PTemplate tmp;
	int size=sizeof(TTemplate);
	int ret = FDB_OK;

	sh.ContentType=FCT_FINGERTMP;
	sh.buffer=(char *)gTemplate;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (sh.datalen==0) continue;
		tmp=(PTemplate)sh.buffer;
		tmp->Valid &= ~DURESSFINGERTAG;

		lseek(fdFingerTmp, -1*size, SEEK_CUR);
		if (write(fdFingerTmp, (void*)tmp, size)!=size)
		{
			ret = FDB_ERROR_IO;
			break;
		}
	}
	fsync(fdFingerTmp);
	return ret;
}

U32 FDB_GetTmpCnt(U16 UID)
{
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		if(fhdl)
		{
			return  ProcessGetFingerprintCount(UID);
		}
		else
			return 0;
	}
	else
		return GetDataInfo(FCT_FINGERTMP, STAT_CNTTEMPLATE, UID);
}

int FDB_ChgTmpValidTag(PTemplate tmp, BYTE SetTag, BYTE ClearTag)
{
	U8 valid=0;
	int size=sizeof(TTemplate);
	int ret;

	if (FDB_GetTmp(tmp->PIN, tmp->FingerID, NULL)==0)
		return FDB_ERROR_NODATA;
	else
	{
		tmp->Valid |= SetTag;
		tmp->Valid &= (BYTE)~ClearTag;
		valid=tmp->Valid;
		lseek(fdFingerTmp, -1*size + 5, SEEK_CUR);
		if (write(fdFingerTmp, (void*)&valid, 1)==1)
		{
			lseek(fdFingerTmp, size - 6, SEEK_CUR);
			ret = FDB_OK;
		}
		else
		{
			lseek(fdFingerTmp, size - 5, SEEK_CUR);
			ret = FDB_ERROR_IO;
		}
		fsync(fdFingerTmp);
		return ret;
	}
}


/* ******add by caona for duress finger*******
 * set duress finger for comm.
 * fp9.0: flag=0, not duress finger;flag=1, duress finger;
 * fp10.0: flag =custom  like fp9.0
 ********************************************/
int FDB_SetDuressTemp(U16 PIN, char FingerID,U16 flag)
{
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		//int CustomLen=CUSTOM_DATA_LENGTH;
		if(!fhdl)
			return 0;

		BIOKEY_SET_CUSTOMDATA(fhdl, PIN|FingerID<<16, (char *)&flag, CUSTOM_DATA_LENGTH);
	}
	else
	{
		TTemplate template;
		memset(&template,0, sizeof(TTemplate));
		if(FDB_GetTmp(PIN, (char)FingerID, (char *)&template))
		{
			if(flag & DURESSFINGERTAG)
				FDB_ChgTmpValidTag(&template, DURESSFINGERTAG, 0);
			else
				FDB_ChgTmpValidTag(&template, 1, 0xFE);
		}
	}
	return 1;
}

/* ******add by caona for duress finger*******
 *  gey duress finger for comm.
 *  fp10.0: flag =custom
 * ********************************************/
int FDB_GetDuressTemp(U16 PIN, char FingerID,U16* flag)
{
	*flag=0;
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		int CustomLen=CUSTOM_DATA_LENGTH;
		//int Custom=0;
		if(!fhdl)
			return 0;

		if(BIOKEY_GET_CUSTOMDATA(fhdl,PIN|FingerID<<16 , (char *)flag,&CustomLen))
		{
			return 1;
		}
	}

	return 1;
}


int FDB_IsDuressTmp(U16 PIN, char FingerID)
{
	//add by caona for FPV10.0 duress finger
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		int Custom=0;
		int CustomLen=CUSTOM_DATA_LENGTH;
		if(BIOKEY_GET_CUSTOMDATA(fhdl, PIN|FingerID<<16 ,(char *)&Custom,&CustomLen))
		{
			if(Custom & DURESSFINGERTAG)
			{
				return 1;
			}
		}
		return 0;
	}
	else
	{
		TTemplate tmp;
		if(FDB_GetTmp(PIN, FingerID, (char *)&tmp))
			return ISDURESSFP(&tmp);
		else
			return FALSE;
	}
}

int g1ToNTemplates=0;

int FDB_LoadTmp(void *Handle)
{
	TSearchHandle sh;
	PTemplate tmp;
	U32 pin2;
	if(gOptions.ZKFPVersion == ZKFPV10)
		return TRUE;
	g1ToNTemplates=0;
	if(!gOptions.Must1To1 || gOptions.I1ToG || gOptions.I1ToH)
	{
		sh.ContentType=FCT_FINGERTMP;
		sh.buffer=(char *)gTemplate;

		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(((U8*)sh.buffer)[5]) //valid
			{
				tmp=(PTemplate)sh.buffer;
				//定义了1:N用户的号码范围
				if(gOptions.I1ToNTo>0)
				{
					//2006.09.01
					//if(gOptions.PIN2Width>PIN_WIDTH)
					//	pin2=(FDB_GetUser(tmp->PIN, NULL))->PIN2;
					//else
					pin2=tmp->PIN;
					if(pin2==0) continue;
					if(pin2<gOptions.I1ToNFrom) continue;
					if(pin2>gOptions.I1ToNTo) continue;
				}
				if (tmp->Size > 6)
					BIOKEY_DB_ADD(Handle, tmp->PIN|(tmp->FingerID<<16), tmp->Size-6, (BYTE*)tmp->Template);
				//DBPRINTF("PIN = %d SIZE =%d\n", tmp->PIN, tmp->Size-6);
				g1ToNTemplates++;
			}
		}
		//		printf("%s g1ToNTemplates %d\n",__FUNCTION__,g1ToNTemplates);
		return g1ToNTemplates;
	}
	else
		return 0;
}

int FDB_ClrTmp(void)
{
	return FDB_ClearData(FCT_FINGERTMP);
}

int FDB_CntTmp(void)
{
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		if(fhdl)
			return ProcessGetFingerprintCount(0);
		else
			return 0;
	}
	else
		return GetDataInfo(FCT_FINGERTMP, STAT_COUNT, 0);
}
static TAttLog gattlog;
int PackAttLog(char *buf, POldAttLog log)
{
	U32 t=log->time_second-BaseTime;

	memset(buf, 0, 4);
	memcpy(buf, &log->PIN, 2);
	buf[2]=((log->status & 0x07) << 5) | ((log->verified & 0x03) << 3);
	if(gOptions.CompressAttlog && (t>0) && (t<1024))//short time format
	{
		buf[2]=buf[2] | 4 | ((t & 0x300) >>8);
		buf[3]=t & 0xff;
		return AttLogSize1;
	}
	else
	{
		memcpy(buf+4,&log->time_second, 4);
		BaseTime=log->time_second;
		return AttLogSize2;
	}
}


int UnpackAttLog(char *buf, POldAttLog log)
{
	memcpy(&log->PIN, buf, 2);
	log->status=(buf[2] >> 5) & 7;
	log->verified=(buf[2] >> 3) & 3;
	if(buf[2] & 4)//short time format
	{
		log->time_second=(U8)buf[3]+((buf[2] & 3) << 8);
		return AttLogSize1;
	}
	else
	{
		memcpy(&log->time_second, buf+4, 4);
		return AttLogSize2;
	}
}
int UnpackNewAttLog(char *buf, PAttLog log)
{
	memcpy(&log->PIN, buf, 2);
	memcpy(&log->PIN2, buf+2, 24);
	memcpy(&log->verified, buf+26, 1);
	memcpy(&log->time_second, buf+27, 4);
	memcpy(&log->status, buf+31, 1);
	memcpy(&log->workcode, buf+32, 4);
	memcpy(&log->reserved, buf+36, 4);
	return 1;
}

int FDB_AddAttLog(U16 pin, time_t t, char verified, char status, char *pin2, U32 workcode, U8 SensorNo)
{
	TAttLog log;
	// FIX Me:if used PUSH_SDK,not save Att Log in one second
#ifndef SDK2ADMS
	if (gOptions.IclockSvrFun)
#endif
	{
		static time_t lastlogtime=0;    //save last log save time by jazzy 2010.01.13
#ifdef SDK2ADMS
		if (t<=lastlogtime)
#else
		if((t<=lastlogtime) && (lastlogtime-t<10))
#endif
		{
			t=lastlogtime+1;
			printf("%s %d %ld time add..\n",__FUNCTION__,__LINE__,lastlogtime);
		}
		lastlogtime=t;
	}

	memset(&log, 0, sizeof(TAttLog));
	log.PIN = pin;
	nmemcpy((BYTE *)log.PIN2, (BYTE *)pin2, gOptions.PIN2Width);
	log.time_second = t;
	log.status = status;
	log.verified = verified;
	log.workcode = workcode;
	if (gOptions.LoopWriteRecordOn) {
		recindex++;
		memcpy(log.reserved, &recindex, 4);
		printf("cur record index:%d\n", recindex);
	} else {
		log.reserved[0] = SensorNo;
	}

	return SearchAndSave(FCT_ATTLOG, (char*)&log, sizeof(TAttLog));
}

int FDB_IsOKLog(TAttLog *log)
{
	int i;
	TTime t;
	for(i=0;i<sizeof(log->PIN2);i++)
	{
		U8 ch=log->PIN2[i];
		if(ch==0 && i>0) break;
		if(ch<'0' || ch>'9')
		{
			printf("[attlog bad]PIN2 Bad %s, %d!\n", log->PIN2, i);
			return 0;
		}
	}
	OldDecodeTime(log->time_second, &t);
	if(t.tm_year>3008-1900 || t.tm_year<2004-1900) { printf("[attlog bad]year=%d\n", t.tm_year); return 0;}
	if(log->reserved[1] || log->reserved[2] || log->reserved[3]) {printf("[attlog bad]reserved=%d\n", log->reserved[1]);return 0;}
	//	if(log->verified>15 || log->status>10) return 0;
	return 1;
}


int FDB_ClrAttLog(void)
{
	return FDB_ClearData(FCT_ATTLOG);
}

void AddToOrderedLogs(PAttLog logs, PAttLog log, int count)
{
	int index=SearchInLogs(logs, log->time_second, count);

	int i;
	for(i=count;i>index;i--)
	{
		logs[i]=logs[i-1];
	}

	logs[index]=*log;
}

int FDB_HaveAttLog(U16 pin, time_t StartTime, time_t EndTime)
{
	TSearchHandle sh;
	int ret=0;
	TAttLog log;
	time_t d1,d2;
	sh.ContentType=FCT_ATTLOG;
	sh.buffer = (char *)&gattlog;
	d1 = StartTime;
	d2 = EndTime;
	while (d1<=d2)
	{
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			memcpy(&log,sh.buffer,sizeof(TAttLog));
			//DBPRINTF("log.pin: %d\tlog.time_second: %ld\n",log.PIN,log.time_second);
			if((pin==log.PIN) && (log.time_second>=d1) && (log.time_second <=(d1+OneDay-OneSecond)))
			{
				if (log.time_second > 0)
				{
					return 1;
				}
			}
		}
		d1 += OneDay;
	}
	return ret;
}

int FDB_GetAttLog(U16 pin, time_t StartTime, time_t EndTime, PAttLog logs, int MaxCount)
{
	TSearchHandle sh;
	int count=0;
	TAttLog log;
	//time_t lastt=0;
	sh.ContentType=FCT_ATTLOG;
	sh.buffer = (char *)&gattlog;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		memcpy(&log,sh.buffer,sizeof(TAttLog));
		//		DBPRINTF("log.pin: %d\tlog.time_second: %ld\n",log.PIN,log.time_second);
		if((pin==0 || pin==log.PIN) && (log.time_second>=StartTime) && (log.time_second <=EndTime))
		{
			if (log.time_second > 0)
				AddToOrderedLogs(logs, &log, count++);
			if(count>=MaxCount) break;
		}
	}
	return count;
}

//根据流水号来获取考勤记录
static TAttLog tmpattlog;
//int FDB_GetAttByIndex(unsigned char buffer[],int maxindex, int curindex, int maxcount,int *lenght)
int FDB_GetAttByIndex(PAttLog logs,int maxindex, int curindex, int maxcount,int *lenght,off_t *curfp)
{	
	TSearchHandle sh;
	unsigned int count=0;
	TAttLog log;
	//time_t lastt=0;
	sh.ContentType=FCT_ATTLOG;
	sh.buffer = (char *)&tmpattlog;
	U32 idid;
	if(*curfp==0){
		SearchFirst(&sh);
	}else{
		SearchOffSize(&sh,*curfp);
	}
	while(!SearchNext(&sh))
	{	
		memcpy(&log,sh.buffer,sizeof(TAttLog));
		memcpy(&idid, ((PAttLog)sh.buffer)->reserved, 4);
		//printf("[%s]_%d maxindex=%d curfp=%d idid=%d, log.PIN=%d\n", __FUNCTION__, __LINE__, maxindex, *curfp, idid, log.PIN);
		if((log.PIN) && (idid>curindex))	//必须是考勤记录pin!=0
		{	
			memcpy(&logs[count], &log, sizeof(TAttLog));
			count++;
			//printf("[%s][%d]count=%d time=%d idid=%d\n",__FUNCTION__,__LINE__,count,log.time_second, idid);
			if(count>=maxcount) break;
		}
	}
//	*curfp+=count*sizeof(TAttLog);
	*curfp=lseek(SelectFDFromConentType(FCT_ATTLOG), 0, SEEK_CUR);
	//printf("<<<<<[%s]_%d curfp=%d\n", __FUNCTION__, __LINE__, *curfp);
	return count;
}


int FDB_GetAttLog_OriginalOrder(U16 pin, time_t StartTime, time_t EndTime, PAttLog logs, int MaxCount)
{
	TSearchHandle sh;
	int count=0;
	TAttLog log;
	//time_t lastt=0;
	sh.ContentType=FCT_ATTLOG;
	sh.buffer = (char *)&gattlog;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		memcpy(&log,sh.buffer,sizeof(TAttLog));
		if((pin==0 || pin==log.PIN) && (log.time_second>=StartTime) && (log.time_second <=EndTime))
		{
			memcpy(&logs[count], &log, sizeof(TAttLog));
			count++;
			if(count>=MaxCount) break;
		}
	}
	return count;
}

//二分法查找
int SearchInLogs(PAttLog logs, time_t t, int count)
{
	int found=0, start=0, end=count-1;
	U32 tt;
	while(start<=end)
	{
		int i=(start+end)/2,j;
		j=0;
		tt=logs[i].time_second;
		if(tt>t)
			end=i-1;
		else if(tt<t)
			start=i+1;
		else
		{
			start=i;
			found=1;
			break;
		}
	}
	return start;
}

//二分法在Lastlogs中查找指定用户是否存在
int SearchInLastLogs(PAlarmRec logs, U16 PIN, int count,int *found)
{
	int start=0, end=count-1;
	U16     pin;
	*found=0;
	while(start<=end)
	{
		int i=(start+end)/2,j;
		j=0;
		pin=logs[i].PIN;
		if(pin>PIN)
			end=i-1;
		else if(pin<PIN)
			start=i+1;
		else
		{
			start=i;
			*found=1;
			break;
		}
	}
	return start;
}
//添加记录到Lastlogs，如果已存在该用户记录且当前时间大于已存在时间则更新
int AddToOrderedLastLogs(PAlarmRec lastlogs, PAttLog log, int count)
{
	int found=0;
	int index=SearchInLastLogs(lastlogs, log->PIN, count, &found);
	int i;
	if(found)
	{
		if(log->time_second > lastlogs[index].LastTime)
		{
			lastlogs[index].LastTime=log->time_second;
			lastlogs[index].State=log->status;
			//ExBeepN(1);
		}
	}
	else
	{
		for(i=count;i>index;i--)
		{
			lastlogs[i].PIN=lastlogs[i-1].PIN;
			lastlogs[i].LastTime=lastlogs[i-1].LastTime;
			lastlogs[i].State=lastlogs[i-1].State;
		}
		lastlogs[index].LastTime=log->time_second;
		lastlogs[index].State=log->status;
		lastlogs[index].PIN=log->PIN;
	}
	return !found;
}

int FDB_CntAttLog(void)
{
	return GetDataInfo(FCT_ATTLOG, STAT_COUNT, 0);
}

char *GetTmpsV10BlockData(int *Size);
char* FDB_ReadBlock(int *size, int ContentType)
{
	TSearchHandle sh;
	U8 buf[1024];
	int validLen;
	U8 *validBuf;
	U8 *p;

	if(ContentType == FCT_FINGERTMP && gOptions.ZKFPVersion == ZKFPV10)
	{
		return GetTmpsV10BlockData(size);
	}

	validLen=GetDataInfo(ContentType, STAT_VALIDLEN, 0);
	*size=validLen+4;
	if (validLen>0)
	{
		validBuf=MALLOC(validLen+4);
		memcpy(validBuf, &validLen, 4);
		p=validBuf+4;

		sh.ContentType=ContentType;
		sh.buffer=(char *)buf;
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen>0)
			{
				memcpy(p, sh.buffer, sh.datalen);
				p+=sh.datalen;
			}
		}
		return (char *)validBuf;
	}
	else
		return NULL;
}

char* FDB_ReadUserBlock(int *size)
{
	return FDB_ReadBlock(size, FCT_USER);
}

char* FDB_ReadAttLogBlock(int *size)
{
	return FDB_ReadBlock(size, FCT_ATTLOG);
}

int IsFreePIN(char *pin)
{
	U32 PIN;
	if(0==strtou32(pin, &PIN))
	{
		if(PIN>MAX_PIN) return FALSE;
		if(PIN==0) return FALSE;
		return !GetBit((BYTE*)UserIDMap, PIN);
	}
	else
		return FALSE;
}
/*
 *Function:FDB_CntUserByMap
 *Description:通过用户MAP表来统计用户数量
 *Input Prama:param--包含
 *Return:用户数
 */
int FDB_CntUserByMap(void)
{
	int i;
	int ret;

	ret = 0;
	for (i=0; i< USER_ID_MAP_BITS;  i++)
	{
		ret = ret + GetBit((BYTE*)UserIDMap, i);
	}
	return ret;
}

int IsUsedPIN(char *pin)
{
	U32 PIN;
	if(0!=strtou32(pin, &PIN))
		return FALSE;
	else
		return !IsFreePIN(pin);
}

int IsFreePIN2(char *pin)
{
	/*	U32 PIN;
		if(0==strtou32(pin, &PIN))
		{
		if(PIN>MAX_PIN2) return FALSE;
		if(PIN==0) return FALSE;
	//return (NULL==FDB_GetUserByPIN2(PIN,NULL));
	//2006.08.03 ssr
	return (NULL==FDB_GetUserByCharPIN2(PIN,NULL));
	}
	else
	return FALSE;
	*/

	//2006.08.03 ssr
	if (gOptions.PinPreAdd0)
		TrimLeft0(pin);
	else
		TrimStr(pin);
	if (pin[0]==0) return FALSE;

	return (NULL==FDB_GetUserByCharPIN2(pin,NULL));
}

int IsUsedPIN2(char *pin)
{
	return !IsFreePIN2(pin);
}


U16 GetNextPIN(int From, int Free)
{
	int i=From;
	do {
		if(Free^GetBit((BYTE*)UserIDMap, i)) {
			return i;
		}
	} while(++i <= USER_ID_MAP_BITS);/*dsl 2012.4.23*/

	return 0;
}

int FDB_UpdateUserIDMap(void)
{
	TSearchHandle sh;
	TUser user;
	int ret=0;

	if (UserIDMap == NULL) {
		return 0;
	}

	memset((char*)UserIDMap, 0, USER_ID_MAP_SIZE);
	memset((char*)&user, 0, sizeof(TUser));

	sh.ContentType=FCT_USER;
	sh.buffer=(char *)&user;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen==0) {
			continue;
		}
		SetBit((BYTE*)UserIDMap, user.PIN);

		ret++;
	}
	return ret;
}


//查找指定用户可用（未登记）的指纹编号
//若指定的指纹编号已经登记，则返回错误
//若指定的指纹编号>=gOptions.MaxUserFingerCount，则表示要查找一个未用（没有登记）的指纹编号
int FDB_GetFreeFingerID(U16 PIN, BYTE *FID)
{
	int i;
	int ret=0xffff;


	if(*FID>=gOptions.MaxUserFingerCount)
	{
		if(FDB_CntTmp()==gOptions.MaxFingerCount*100)
		{
			//return FDB_OVER_FLIMIT;
			ret=FDB_OVER_FLIMIT;
		}
		else
		{
			for(i=0;i<gOptions.MaxUserFingerCount;i++)
			{
				if(0==FDB_GetTmp(PIN, (char)i, NULL))
				{
					*FID=i;
					//return FDB_OK;
					ret=FDB_OK;
				}
			}
			if(ret!=FDB_OK)
			{
				//return FDB_OVER_UFLIMIT;
				ret=FDB_OVER_UFLIMIT;
			}
		}

	}
	else if(FDB_GetTmp(PIN, *FID, NULL))
	{
		ret=FDB_FID_EXISTS;
	}
	else
	{
		ret=FDB_OK;
	}

	return ret;
}

int FDB_AddOPLog(U16 Admin, BYTE OP, U16 Objs1, U16 Objs2, U16 Objs3, U16 Objs4)
{
	int ret;
	time_t t;
	TOPLog log;

	if(OP<32)
	{
		if((gOptions.OPLogMask1 & (1<<OP))==0) return FDB_ERROR_DATA;
	}
	else if(OP<64)
	{
		if((gOptions.OPLogMask2 & (1<<(OP-32)))==0) return FDB_ERROR_DATA;
	}
	/*用户考勤照片上传，add by yangxiaolong,2011-07-29,start*/
	else if ((OP > 64) && (OP < 80))
	{
		;
	}
	/*用户考勤照片上传，add by yangxiaolong,2011-07-29,end*/
	else
		return FDB_ERROR_DATA;

	GetTime(&gCurTime);
	t=OldEncodeTime(&gCurTime);
	// FIX Me:if used PUSH_SDK,not save Att Log in one second
#ifndef SDK2ADMS
	if (gOptions.IclockSvrFun)
#endif
	{
		static time_t lastlogtime=0;    //save last log save time by jazzy 2010.01.16
#ifdef SDK2ADMS               
		if (t<=lastlogtime)
#else
			if ((t<=lastlogtime) && (lastlogtime-t<10))
#endif
			{
				t=lastlogtime+1;
			}
		lastlogtime=t;
	}
	log.time_second=t;

	log.Admin=Admin;
	log.OP=OP;
	log.Users[0]=Objs1;log.Users[1]=Objs2;log.Users[2]=Objs3;log.Users[3]=Objs4;
	if(FDB_CntOPLog()>=MAX_OPLOG_COUNT) FDB_ClrOPLog();
	ret=SearchAndSave(FCT_OPLOG, (char*)&log, sizeof(TOPLog));

	if(FDB_CntOPLog()+gOptions.AlarmOpLog>=MAX_OPLOG_COUNT)
	{
		char buf[50];
		sprintf(buf, LoadStrByID(323), MAX_OPLOG_COUNT-FDB_CntOPLog());
	}
	return ret;
}

int FDB_ClrOPLog(void)
{
	return FDB_ClearData(FCT_OPLOG);
}

int FDB_CntOPLog(void)
{
	return GetDataInfo(FCT_OPLOG, STAT_COUNT, 0);
}

char* FDB_ReadOPLogBlock(int *size)
{
	return FDB_ReadBlock(size, FCT_OPLOG);
}

int FDB_ClrAdmin(void)
{
	char *buf;
	char tmp[80];
	int size, buflen;
	PUser usr;

	if ((buf=FDB_ReadBlock(&size, FCT_USER))==NULL) return FDB_ERROR_IO;
	buflen=size;
	usr=(PUser)(buf+4);
	while((usr->PIN<0xFFFF) && (size>=sizeof(TUser)))
	{
		usr->Privilege=usr->Privilege & 1;
		usr++;
		size-=sizeof(TUser);
	}
	fdUser=TruncFDAndSaveAs(fdUser, GetEnvFilePath("USERDATAPATH", "ssruser.dat", tmp), buf+4, buflen-4);
	//if (useSDCardFlag || ((gOptions.MaxAttLogCount*10000) > ATTLOGLIMIT))
	//	fdUser=TruncFDAndSaveAs(fdUser, GetEnvFilePath("USERDATAPATH", "ssruser.dat", tmp), buf+4, buflen-4);
	//else
	//	fdUser=TruncFDAndSaveAs(fdUser, GetEnvFilePath("USERDATAPATH", "data/ssruser.dat", tmp), buf+4, buflen-4);

	FREE(buf);


	return FDB_OK;
}

int FDB_CheckIntegrate(void)		//进行数据库正确性、完整性检查
{
	return FDB_OK;
}

#ifdef IKIOSK
//数据加密
static int Encrypt(char* cSrc, char* cDest)
{
	char   c;
	int   i,h,l,j=0;

	memset(cDest, 0, 1024);
	for (i=0; i<(int)strlen(cSrc); i++)
	{
		c=cSrc[i];
		h=(c>>4)&0xf;
		l=c&0xf;
		cDest[j]=h+'x';
		cDest[j+1]=l+'z';
		j+=2;
	}
	cDest[j]='\0';
	return 0;
}
#endif

BOOL FDB_Download(int ContentType, char *dstFileName)
{
	TSearchHandle sh;
	U8 buf[3*1024];
	char fmt[64];
	int dstHandle;

	TAttLog log;
	TTime tt;
	U32 WorkCode, PIN;
	char PIN2[24];
	BYTE status, verified;

	dstHandle=open(dstFileName, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if (dstHandle==-1) return FALSE;

	sh.buffer=(char *)buf;

#if 0
	//U盘加密================
#ifdef IKIOSK
	char destbuf[1024] ="\0";
	sprintf(sh.buffer, "\t%s\r\n", SerialNumber);
	Encrypt(sh.buffer, destbuf);
	memcpy (sh.buffer, destbuf, strlen(destbuf)+1);
	sh.bufferlen=strlen(sh.buffer);
	if (write(dstHandle, sh.buffer, sh.bufferlen)!=sh.bufferlen)
	{
		close(dstHandle);
		return FALSE;
	}
#endif
	//=======================
#endif

	sh.ContentType=ContentType;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen>0)
		{
			if (ContentType==FCT_ATTLOG)
			{
				PIN=((PAttLog)(sh.buffer))->PIN;
				memcpy(PIN2,((PAttLog)(sh.buffer))->PIN2,24);
				if(((PIN2[0]) &&(strlen(PIN2)||(strcmp(PIN2,"0")>0))))
				{
					log.time_second=((PAttLog)(sh.buffer))->time_second;
					status=((PAttLog)(sh.buffer))->status;
					verified=((PAttLog)(sh.buffer))->verified;
					WorkCode=((PAttLog)(sh.buffer))->workcode;
					OldDecodeTime(log.time_second, &tt);

					sprintf(fmt, "%%%ds%s", gOptions.PIN2Width,
							"\t%4.4d-%02d-%02d %02d:%02d:%02d\t%d\t%d\t%d\t%d\r\n");
					sprintf(sh.buffer,
							fmt,
							PIN2,
							tt.tm_year+1900,tt.tm_mon+1,tt.tm_mday,tt.tm_hour, tt.tm_min, tt.tm_sec,
							gOptions.DeviceID,
							status,
							verified,
							WorkCode
					       );
					sh.bufferlen=strlen(sh.buffer);
				}
				else
					continue;

				//只对考勤记录数据进行加密
#if 0			//3DES加密算法
				{
					char key[20];
					LoadStr("Encrypt", key);
					if (key[0] && strlen(key)>0)
					{
						printf("Encrypt key:%s\n", key);
						Encrypt_Des(sh.buffer, sh.buffer, sh.bufferlen, key, strlen(key), ENCRYPT);
					}
					//				Encrypt_Des(sh.buffer, sh.buffer, sh.bufferlen, key, strlen(key), DECRYPT);
					//				printf("After Decrypt:%s\n", sh.buffer);
				}
#endif
				//U盘加密================
#ifdef IKIOSK
				char destbuf[1024] ="\0";
				Encrypt(sh.buffer, destbuf);
				memcpy(sh.buffer, destbuf, strlen(destbuf)+1);
				sh.bufferlen=strlen(sh.buffer);
#endif
				//========================
			}

			if (write(dstHandle, sh.buffer, sh.bufferlen)!=sh.bufferlen)
			{
				close(dstHandle);
				return FALSE;
			}
		}
	}
	fsync(dstHandle);
	close(dstHandle);
	return TRUE;
}

//按指定用户号码、姓名和指纹编号新增指纹
//若指定的指纹存在，则覆盖
//若指定的指纹编号大于允许范围（gOptions.MaxUserFingerCount-1），则尝试使用一个未用的指纹编号来保存
//当name==NULL时，表示仅仅新增指纹，而且用户必须事先存在
//成功时返回 FDB_OK
int AppendUserTemp(int pin, char *name, int fingerid, char *temp, int tmplen,int valid)
{
	int ret;
	TUser user;

	if(pin>0 && FDB_GetUser(pin,&user))
	{
		BYTE fid=(char)fingerid;

		if(gOptions.ZKFPVersion!=ZKFPV10 && (ret=FDB_GetFreeFingerID(user.PIN, &fid))!=FDB_OK)
		{
			if(ret!=FDB_FID_EXISTS) return ret;
			if(name==NULL || (FDB_DelTmp(user.PIN, fid)!=FDB_OK))
			{
				return FDB_FID_EXISTS; //old 3
			}
		}
		fingerid=fid;
	}
	else if(name)
	{
		if(!FDB_CreateUser(&user, pin, name, NULL, 0))
		{
			return FDB_ERROR_OP;
		}
		if(FDB_AddUser(&user)!=FDB_OK)
		{
			return FDB_ERROR_IO;
		}
		if(gOptions.ZKFPVersion!=ZKFPV10 && fingerid>9) fingerid=0;
	}
	else
		return FDB_ERROR_NODATA;

	if(FDB_CntTmp()>=gOptions.MaxFingerCount*100)
		return FDB_OVER_FLIMIT;
	else
	{
		if(gOptions.ZKFPVersion == ZKFPV10)
		{
			TZKFPTemplate tmpv10;
			tmpv10.PIN = user.PIN;
			tmpv10.FingerID = fingerid;
			tmpv10.Size = tmplen;
			tmpv10.Valid = valid;
			memcpy(tmpv10.Template,temp,tmplen);
#if 0
#ifndef SDK2ADMS
			return FDB_AddTmp(&tmpv10);
#else
			int j=FDB_AddTmp((char *)&tmpv10);
			FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, user.PIN,j,fingerid,tmplen);
			return j;
#endif
#endif
			int j=FDB_AddTmp((char *)&tmpv10);
			FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, user.PIN,j,fingerid,tmplen);
			//IclockSvrURLLength =  IclockSvrURLLength_back;
			return j;
		}
		else
		{
			TTemplate tmp;
			if(!FDB_CreateTemplate((char*)&tmp, user.PIN, fingerid, temp, tmplen,valid))
				return FDB_ERROR_IO;
#if 0
#ifndef SDK2ADMS
			if(FDB_OK ==FDB_AddTmp(&tmp))
#else                   		
#endif
#endif	
				int j=FDB_AddTmp((char *)&tmp);
			FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, user.PIN,j,fingerid,tmplen);
			if (j==FDB_OK)
				return FDB_OK;
			else
				return FDB_ERROR_IO;
		}
	}

}

int AppendSSRUserTemp(int pin, int isdelete, int fingerid, char *temp, int tmplen,int valid)
{
	int ret;
	TUser user;

	if(FDB_GetUser(pin,&user))
	{
		BYTE fid=(char)fingerid;
		if((ret=FDB_GetFreeFingerID(user.PIN, &fid))!=FDB_OK)
		{
			if(ret!=FDB_FID_EXISTS) return ret;
			if(isdelete==0 || (FDB_DelTmp(user.PIN, fid)!=FDB_OK))
			{
				return FDB_FID_EXISTS; //old 3
			}
		}
		fingerid=fid;
	}
	else
		return FDB_ERROR_NODATA;

	if(FDB_CntTmp()>=gOptions.MaxFingerCount*100)
		return FDB_OVER_FLIMIT;
	else
	{
		TTemplate tmp;
		if(!FDB_CreateTemplate((char*)&tmp, user.PIN, fingerid, temp, tmplen,valid))
			return FDB_ERROR_IO;
		return FDB_AddTmp((char *)&tmp);
	}
}



int AppendUser(int pin, char *name, char *password, int privillege)
{
	int ret=FDB_OK;
	TUser user;

	if(FDB_GetUser((U16)pin, &user))
	{
		memcpy((BYTE*)user.Name, name, MAXNAMELENGTH);
		memcpy((BYTE*)user.Password, password, 5);
		user.Privilege=privillege;
		ret=FDB_ChgUser(&user);
	}
	else if(FDB_CntUserByMap()==gOptions.MaxUserCount*100)
		ret=FDB_OVER_ULIMIT;
	else if(FDB_CreateUser(&user, pin, name, password, privillege))
		ret=FDB_AddUser(&user);
	else
		ret=FDB_ERROR_OP;
	return ret;
}

int GetFreeUserPIN(void)
{
	/*dsl 2012.4.23*/
	return GetNextPIN(1,1);
#if 0
	TSearchHandle sh;
	U16 *pCacheUser=NULL;
	char* Buffer=NULL;
	PUser CurUser=NULL;
	int cuser= 0;//FDB_CntUser();
	U16 i=0,testpin=0,bSign=TRUE;
	//unsigned int msc=GetTickCount1();

	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;

	SearchFirst(&sh);
	cuser=lseek(sh.fd,0,SEEK_END);
	Buffer=MALLOC(cuser);
	if(Buffer == NULL)
	{
		printf("get user   malloc failed \n");
		goto SearchOneByOne;
	}
	cuser /= sizeof(TUser);
	pCacheUser = MALLOC((cuser+1)*2);
	if(pCacheUser==NULL)
	{
		printf("get free PIN  malloc failed \n");
		FREE(Buffer);
		return 0;
	}
	SearchFirst(&sh);
	memset(pCacheUser, 0, cuser*2);
	if(read(sh.fd,(void*)Buffer,cuser*sizeof(TUser)) != cuser*sizeof(TUser))
	{
		FREE(Buffer);
		goto SearchOneByOne;
	}
	else
	{
		CurUser=(PUser)Buffer;
		for(i=0;i<cuser;i++)
		{
			pCacheUser[i] = CurUser[i].PIN;
		}
	}

	FREE(Buffer);
	goto FindFreePIN;

SearchOneByOne:
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen>0)
		{
			pCacheUser[i] = ((PUser)sh.buffer)->PIN;
			i++;
		}
	}

FindFreePIN:
	do
	{
		bSign=TRUE;
		testpin++;

		for (i=0;i<cuser;i++)
		{
			if(pCacheUser[i]==testpin)
			{
				bSign=FALSE;
				break;
			}
		}

	}while(!bSign);

	FREE(pCacheUser);
	//printf("get time %d  testpin %d\n", GetTickCount1()-msc, testpin);
	return testpin;
#endif
}

int AppendFullUser(PUser user)
{
	int ret=FDB_OK;
	TUser appenduser;
	TGroup tgp;
	int i;

	if (user->PIN && user->PIN2[0])
	{
		//默认分配到存在的第一个组
		if(gOptions.LockFunOn && user->Group<=0)
		{

			for(i=0;i<GP_MAX;i++)
			{
				memset(&tgp,0,sizeof(TGroup));
				if(FDB_GetGroup(i+1,&tgp)!=NULL)
				{
					user->Group=tgp.ID;
					break;
				}
			}
		}
		memset(&appenduser,0,sizeof(TUser));
		appenduser.PIN=user->PIN;
		//if(FDB_GetUser((U16)user->PIN, NULL))

		if(FDB_GetUserByCharPIN2(user->PIN2, &appenduser))
		{
			user->PIN=appenduser.PIN;
			ret=FDB_ChgUser(user);
		}
		else
		{
			if(FDB_CntUserByMap()>=gOptions.MaxUserCount*100)
				ret=FDB_OVER_ULIMIT;
			else
			{
				user->PIN=GetFreeUserPIN();
				ret=FDB_AddUser(user);
			}
		}
	}
	else
		ret=FDB_ERROR_DATA;

	return ret;
}

PUData FDB_CreateUData(PUData udata,U16 pin, U16 smsid)
{
	memset((void *)udata,0,sizeof(TUData));
	udata->PIN = pin;
	udata->SmsID = smsid;
	return udata;
}

PSms FDB_CreateSms(PSms sms, BYTE tag, U16 id, BYTE *content, U16 validminutes, time_t start)
{
	memset((void *)sms, 0, sizeof(TSms));
	sms->Tag=tag;
	sms->ID=id;
	sms->ValidMinutes=validminutes;
	sms->StartTime=start;
	nstrcpy((char *)sms->Content, (char *)content, MAX_SMS_CONTENT_SIZE*2);
	return sms;
}

int FDB_AddSms(PSms sms)
{
	int ret=FDB_CntSms();
	if (ret>=MAX_SMS_COUNT || ret>=gOptions.SMSCount)
		return FDB_ERROR_NOSPACE;
	return SearchAndSave(FCT_SMS, (char*)sms, sizeof(TSms));
}

static TSms gSms;
PSms FDB_GetSms(U16 id, PSms sms)
{
	TSearchHandle sh;

	sh.ContentType=FCT_SMS;
	sh.buffer=(char*)&gSms;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(((PSms)sh.buffer)->ID==id)
		{
			if (sms)
				memcpy(sms, sh.buffer, sizeof(TSms));
			return (PSms)sh.buffer;
		}
	}
	return NULL;
}

int FDB_ChgSms(PSms sms)
{
	PSms s;
	int ret;

	if ((s=FDB_GetSms(sms->ID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)sms, (void*)s, sizeof(TSms))) return FDB_OK;
	//overwrite
	lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
	if (write(fdSms, (void*)sms, sizeof(TSms))==sizeof(TSms))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdSms);


	return ret;
}

int FDB_DelSms(U16 id)
{
	TSms s;
	U16 smsid;
	int ret;

	if ((FDB_GetSms(id, &s))==NULL) return FDB_ERROR_NODATA;
	//overwrite
	smsid=id;//s.ID;
	memset(&s, 0, sizeof(TSms));
	lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
	if (write(fdSms, (void*)&s, sizeof(TSms))==sizeof(TSms))
	{
		ret = FDB_DelUData(0, smsid);
	}
	else
		ret = FDB_ERROR_IO;


	fsync(fdSms);
	return ret;
}

int FDB_ClrSms(void)
{
	return FDB_ClearData(FCT_SMS);
}

int FDB_CntSms(void)
{
	return GetDataInfo(FCT_SMS, STAT_COUNT, 0);
}


//workcode
int FDB_InitWkcdPoint(void)
{
	if(fdWkcd<0) return 0;
	lseek(fdWkcd, 0, 0);
	return 1;
}

int FDB_ReadWorkCode(PWorkCode pworkcode)
{
	memset(pworkcode,0,sizeof(TWORKCODE));
	if((read(fdWkcd,pworkcode,sizeof(TWORKCODE)))==sizeof(TWORKCODE))
		return 1;
	else
		return 0;
}

int FDB_CntWorkCode(void)
{
	return GetDataInfo(FCT_WORKCODE,STAT_COUNT,0);
}


PWorkCode FDB_CreateWorkCode(PWorkCode workcode,U16 pin,char *code,char *name)
{
	memset((void *)workcode,0,sizeof(TWORKCODE));
	workcode->PIN =pin;
	nstrcpy(workcode->Code,code,MAX_WORKCODE_LEN);
	nstrcpy(workcode->Name,name,MAX_WORKCODE_NAME_LEN);

	return workcode;
}

int FDB_DelWorkCode(U16 pin)
{
	TWORKCODE s;
	int ret;
	if((FDB_GetWorkCode(pin,&s))==NULL) return FDB_ERROR_NODATA;

	memset(&s, 0, sizeof(TWORKCODE));
	lseek(fdWkcd,-1*sizeof(TWORKCODE),SEEK_CUR);
	if(write(fdWkcd, (void*)&s,sizeof(TWORKCODE))==sizeof(TWORKCODE))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdWkcd);


	return ret;
}

int FDB_AddWorkCode(PWorkCode workcode)
{
	return SearchAndSave(FCT_WORKCODE, (char*)workcode, sizeof(TWORKCODE));
}

static TWORKCODE gWorkcode;
PWorkCode FDB_GetWorkCode(U16 id, PWorkCode workcode)
{
	TSearchHandle sh;

	sh.ContentType=FCT_WORKCODE;
	sh.buffer=(char*)&gWorkcode;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PWorkCode)sh.buffer)->PIN==id)
		{
			if (workcode)
			{
				memcpy(workcode, sh.buffer, sizeof(TWORKCODE));
				return workcode;
			}
			else
				return (PWorkCode)sh.buffer;
		}
	}
	return NULL;
}

int FDB_ChgWorkCode(PWorkCode workcode)
{
	PWorkCode a;
	int ret;

	if ((a=FDB_GetWorkCode(workcode->PIN, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)workcode, (void*)a, sizeof(TWORKCODE))) return FDB_OK;
	//overwrite
	lseek(fdWkcd, -1*sizeof(TWORKCODE), SEEK_CUR);
	if (write(fdWkcd, (void*)workcode, sizeof(TWORKCODE))==sizeof(TWORKCODE))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdWkcd);


	return ret;
}

//==================================
PWorkCode FDB_GetWorkCodeByCode(char* code, PWorkCode wkcd)
{
	TSearchHandle sh;

	sh.ContentType = FCT_WORKCODE;
	sh.buffer = (char*)&gWorkcode;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PWorkCode)sh.buffer)->PIN && nstrcmp(((PWorkCode)sh.buffer)->Code, code, MAX_WORKCODE_LEN)==0)
		{
			if (wkcd)
			{
				memcpy(wkcd, sh.buffer, sizeof(TWORKCODE));
				return wkcd;
			}
			else
				return (PWorkCode)sh.buffer;
		}
	}
	return NULL;
}

PWorkCode FDB_RecreateWorkCode(PWorkCode wkcd)
{
	TWORKCODE tmpwkcd;

	memset(&tmpwkcd, 0, sizeof(TWORKCODE));
	if (FDB_GetWorkCodeByCode(wkcd->Code, &tmpwkcd))
	{

		wkcd->PIN = tmpwkcd.PIN;
		return wkcd;
	}
	else
	{
		int tmppin = 1;
		while (FDB_GetWorkCode(tmppin, NULL)!=NULL)
		{
			tmppin++;
		}
		wkcd->PIN = tmppin;
		return wkcd;
	}

	return NULL;
}

//==================================New Bell Start
/*New Bell
  typedef struct _Bell_New
  {
  U16 BellID;                     // Bell index
  U8  DayOfWeek;                  // Day of week
  U8  BellTime[2];                // Bell time (hh:mm)
  U8  BellWaveIndex;              // Bell wave file name
  U8  BellVolume;                 // Bell volume
  U8  BellTimes;                  // Bell times
  U8  BellStatus;                 // Bell status
  BYTE Reserved[3];
  }GCC_PACKED TBellNew, *PBellNew;        // 12bytes
  */

static TBellNew gBellNew;
int FDB_CntBell(void)
{
	return GetDataInfo(FCT_BELL, STAT_COUNT, 0);
}

int FDB_AddBell(PBellNew bell)
{
	return SearchAndSave(FCT_BELL, (void*)bell, sizeof(TBellNew));
}

int FDB_DelBell(U16 id)
{
	TBellNew d;
	int ret;

	if ((FDB_GetBell(id, NULL))==NULL) return FDB_ERROR_NODATA;
	//overwrite
	memset(&d, 0, sizeof(TBellNew));
	lseek(fdAlarm, -1*sizeof(TBellNew), SEEK_CUR);
	if (write(fdAlarm, (void*)&d, sizeof(TBellNew))==sizeof(TBellNew))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdAlarm);

	return ret;
}

PBellNew FDB_GetBell(U16 id, PBellNew bell)
{
	TSearchHandle sh;

	sh.ContentType=FCT_BELL;
	sh.buffer=(char*)&gBellNew;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PBellNew)sh.buffer)->BellID==id)
		{
			if (bell)
			{
				memcpy(bell, sh.buffer, sizeof(TBellNew));
				return bell;
			}
			else
				return (PBellNew)sh.buffer;
		}
	}
	return NULL;
}

int FDB_ChgBell(PBellNew bell)
{
	PBellNew a;
	int ret;
	if ((a=FDB_GetBell(bell->BellID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (memcmp((void*)bell, (void*)a, sizeof(TBellNew))==0) return FDB_OK;
	//overwrite
	lseek(fdAlarm, -1*sizeof(TBellNew), SEEK_CUR);
	if (write(fdAlarm, (void*)bell, sizeof(TBellNew))==sizeof(TBellNew))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdAlarm);

	return ret;
}

static U16 GetBellIndex()
{
	U16 i = 1;


	while (FDB_GetBell(i, NULL))
		i++;

	return i;
}

PBellNew FDB_CreateBell(PBellNew bell, U8 schinfo, U8 hour, U8 min, U8 wave, U8 volume, U8 status, U8 times)
{
	memset((void*)bell, 0, sizeof(TBellNew));
	bell->BellID = GetBellIndex();
	if (status) schinfo |= 128;
	bell->SchInfo = schinfo;
	bell->BellTime[0] = hour;
	bell->BellTime[1] = min;
	bell->BellWaveIndex = wave;
	bell->BellVolume = volume;
	bell->BellTimes = times;
	return bell;
}

PBellNew FDB_GetBellByTime(PBellNew bell, int hour, int min, int day)
{
	TSearchHandle sh;

	sh.ContentType=FCT_BELL;
	sh.buffer=(char*)&gBellNew;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PBellNew)sh.buffer)->BellID && 		//有效
				((PBellNew)sh.buffer)->BellTime[0]==hour && ((PBellNew)sh.buffer)->BellTime[1]==min &&	//时间
				(((((PBellNew)sh.buffer)->SchInfo)>>7)&1)==1 && 					//启用
				(((((PBellNew)sh.buffer)->SchInfo)>>day)&1)==1) 					//
		{
			if (bell)
			{
				memcpy(bell, sh.buffer, sizeof(TBellNew));
				return bell;
			}
			else
				return (PBellNew)sh.buffer;
		}
	}
	return NULL;

}

static int CheckSchInfo(BYTE s, BYTE d)
{
	return (((s>>7)&1) && ((s&d)&0x7F)!=0);
}

int FDB_BellSettingErr(int id, int hour, int min, BYTE SchInfo)
{
	TSearchHandle sh;

	sh.ContentType=FCT_BELL;
	sh.buffer=(char*)&gBellNew;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		//		printf("id:%d, hour:%d, min:%d, SchInfo:%d\n", ((PBellNew)sh.buffer)->BellID, ((PBellNew)sh.buffer)->BellTime[0],

		if (((PBellNew)sh.buffer)->BellID && ((PBellNew)sh.buffer)->BellID!=id &&
				((PBellNew)sh.buffer)->BellTime[0]==hour && ((PBellNew)sh.buffer)->BellTime[1]==min &&
				CheckSchInfo((BYTE)(((PBellNew)sh.buffer)->SchInfo), (BYTE)SchInfo))
			return 1;
	}
	return 0;
}

void FDB_InitBell(void)
{
	int i;
	TBellNew bell;

	printf("Init New Bell %d\n", gOptions.AlarmMaxCount);
	for (i=0; i<gOptions.AlarmMaxCount; i++)
	{
		if (FDB_CreateBell(&bell, 0, 0, 0, 0, gOptions.AudioVol, 0, 10)!=NULL)
		{
			FDB_AddBell(&bell);
		}
	}
}
//=======================================New Bell End

// old alarm
int FDB_CntAlarm(void)
{
	return GetDataInfo(FCT_ALARM,STAT_COUNT,0);
}

PAlarm FDB_CreateAlarm(PAlarm alarm, U16 id, U8 t_hour, U8 t_min, U8 fileindex, U8 audiovol, U8 status, U8 alarmtimes)//BYTE *hint)
{
	memset((void *)alarm, 0, sizeof(ALARM));
	alarm->AlarmIDX = id;
	alarm->AlarmHour = t_hour;
	alarm->AlarmMin = t_min;
	//	alarm->AlarmTime = alarmtime;
	alarm->AlarmAudioVol = audiovol;
	alarm->AlarmStatus = status;
	alarm->AlarmWaveIDX = fileindex;
	alarm->AlarmTimes = alarmtimes;
	//	nstrcpy(alarm->AlarmHint,hint,MAX_ALARM_HINT_SIZE);
	return alarm;
}

int FDB_AddAlarm(PAlarm alarm)
{
	return SearchAndSave(FCT_ALARM, (char*)alarm, sizeof(ALARM));
}

static ALARM gAlarm;
PAlarm FDB_GetAlarm(U16 id, PAlarm alarm)
{
	TSearchHandle sh;

	sh.ContentType=FCT_ALARM;
	sh.buffer=(char*)&gAlarm;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PAlarm)sh.buffer)->AlarmIDX==id)
		{
			if (alarm)
				memcpy(alarm, sh.buffer, sizeof(ALARM));
			return (PAlarm)sh.buffer;
		}
	}
	return NULL;

}

int FDB_ChgAlarm(PAlarm alarm)
{
	PAlarm a;
	int ret;

	if ((a=FDB_GetAlarm(alarm->AlarmIDX, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)alarm, (void*)a, sizeof(ALARM))) return FDB_OK;
	//overwrite
	lseek(fdAlarm, -1*sizeof(ALARM), SEEK_CUR);
	if (write(fdAlarm, (void*)alarm, sizeof(ALARM))==sizeof(ALARM))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdAlarm);

	return ret;
}
//alarm end

int FDB_AddUData(PUData udata)
{
	return SearchAndSave(FCT_UDATA, (char*)udata, sizeof(TUData));
}

static TUData gUData;

PUData FDB_GetUData(U16 id, PUData udata)
{
	TSearchHandle sh;

	sh.ContentType=FCT_UDATA;
	sh.buffer=(char*)&gUData;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PUData)sh.buffer)->PIN==id)
		{
			if (udata)
				memcpy(udata, sh.buffer, sizeof(TUData));
			return (PUData)sh.buffer;
		}
	}
	return NULL;
}

//修改短消息分别表数据2007.7.26
int FDB_ChgUData(PUData udata)
{
	PUData u;
	int ret;

	if((u=FDB_GetUData(udata->PIN,NULL))==NULL) return FDB_ERROR_NODATA;
	if(0==memcmp((void*)udata, (void*)u,sizeof(TUData))) return FDB_OK;
	//overwrite
	lseek(fdUData, -1*sizeof(TUData),SEEK_CUR);
	if(write(fdUData,(void*)udata, sizeof(TUData))==sizeof(TUData))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdUData);

	return ret;
}

int FDB_GetUDataBySmsID(U16 smsid,PUData udata)
{
	TSearchHandle sh;

	sh.ContentType=FCT_UDATA;
	sh.buffer=(char*)&gUData;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PUData)sh.buffer)->SmsID==smsid)
		{
			if (udata)
				memcpy(udata, sh.buffer, sizeof(TUData));
			return (int)sh.buffer;
		}
	}
	return 0;

}

PUData FDB_GetUDataByPINSMSID(U16 pin, U16 id, PUData udata)
{
	TSearchHandle sh;

	sh.ContentType=FCT_UDATA;
	sh.buffer=(char*)&gUData;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if ((((PUData)sh.buffer)->PIN==pin)&&(((PUData)sh.buffer)->SmsID==id))
		{
			if (udata)
				memcpy(udata, sh.buffer, sizeof(TUData));
			return (PUData)sh.buffer;
		}
	}
	return NULL;
}

//删除PIN OR SMSID指定的数据项
int FDB_DelUData(U16 PIN, U16 smsID)
{
	TUData u;
	int ret = FDB_OK;


	lseek(fdUData, 0, SEEK_SET);
	while(TRUE)
	{
		if (read(fdUData, (void *)&u, sizeof(TUData))==sizeof(TUData))
		{
			if (u.PIN&&((PIN&&smsID&&(u.SmsID==smsID)&&(u.PIN==PIN))||
						(smsID&&!PIN&&(u.SmsID==smsID))||(!smsID&&PIN&&(u.PIN==PIN))))
			{
				lseek(fdUData, -1*sizeof(TUData), SEEK_CUR);
				u.PIN=0;
				u.SmsID=0;
				if (write(fdUData, (void*)&u, sizeof(TUData))!=sizeof(TUData))
				{
					ret = FDB_ERROR_IO;
					break;
				}

			}
		}
		else
			break;
	}

	fsync(fdUData);
	return ret;
}

int IsValidTimeDuration(U32 TestTime, PSms sms)
{
	U32 StartTime, EndTime;
	TTime st;
	OldDecodeTime(sms->StartTime, &st);
	st.tm_isdst=-1;
	StartTime=mktime(&st);
	EndTime=StartTime+60*sms->ValidMinutes;
	if(TestTime<StartTime)// && sms->ValidMinutes!=0)						//未到时间
		return -1;
	else if(TestTime>=EndTime && sms->ValidMinutes!=VALIDMINUTE_UNLIMITED && sms->Tag!=0xFF)		//过期
		return 1;
	else 												//有效
		return 0;
}

void FDB_CheckSmsByStamp(U32 CurTime)
{
	TSearchHandle sh;
	U16 smsid;

	sh.ContentType=FCT_SMS;
	sh.buffer=(char*)&gSms;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(gSms.ID && (IsValidTimeDuration(CurTime, &gSms)==1))
		{
			//overwrite
			smsid=gSms.ID;
			gSms.ID=0;
			lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
			if (write(fdSms, (void*)sh.buffer, sizeof(TSms))==sizeof(TSms))
			{
				FDB_DelUData(0, smsid);
			}
		}
	}
	fsync(fdSms);
}

int FDB_PackSms(void)
{
	char *buf;
	char tmp[80];
	int size;
	if ((buf=FDB_ReadBlock(&size, FCT_SMS))==NULL) return FDB_ERROR_IO;
	fdSms=TruncFDAndSaveAs(fdSms, GetEnvFilePath("USERDATAPATH", "data/sms.dat", tmp), buf+4, size-4);
	FREE(buf);

	return FDB_OK;
}

BYTE *FDB_ReadUserSms(U16 *smsid, int smsnum, BYTE *content)
{
	PSms s;
	int i=0,j,k;
	int imefunon=0;
	imefunon=gOptions.IMEFunOn;

	memset(content, 0, MAX_SMS_CONTENT_SIZE*2+1);
	k=smsnum-1;


	while(k>=0)
	{
		if((s=FDB_GetSms(smsid[k], NULL))!=NULL)
		{
			i=IsValidTimeDuration(EncodeTime(&gCurTime), s);
			//overdate so overwrite
			if (i==1 && imefunon!=1)	//不支持输入法功能时删除过期短消息
			{
				lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
				s->ID=0;
				if (write(fdSms, (void*)s, sizeof(TSms))==sizeof(TSms))
				{
					FDB_DelUData(0, smsid[k]);
				}
				fsync(fdSms);
			}
			//show information
			else if (i==0)
			{
				j=0;
				while((j<MAX_SMS_CONTENT_SIZE*2)&&s->Content[j])
				{
					*(content+j)=s->Content[j];
					j++;
				}
				*(content+j)='\0';

				return content;
			}
		}
		k--;
	}


	return NULL;
}

#ifndef ICLOCK
int FDB_ReadBoardSms(BYTE *content)
{
	TSearchHandle sh;
	int valid,len=0,i=1;
	int smsid;

	sh.ContentType=FCT_SMS;
	sh.buffer=(unsigned char*)&gSms;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (gSms.ID==0) continue;
		valid=IsValidTimeDuration(EncodeTime(&gCurTime), &gSms);
		if((gSms.Tag==UDATA_TAG_ALL)&&(valid==0))
		{
			if(i>1)
			{
				sprintf(content+len, "%d.%s", i, gSms.Content);
				len+=strlen(gSms.Content)+2;
			}
			else
			{
				sprintf(content+len, "%s", gSms.Content);
				len+=strlen(gSms.Content);
			}
			i++;
		}
		if (i>=10) break;
	}
	//DBPRINTF("%s\n", content);
	return strlen(content);
}
#else
int FDB_ReadBoardSms(BYTE *content, int flag)
{
	TSearchHandle sh;
	int valid,len=0,i=0;
	int smsid;
	int imefunon=0;
	imefunon=gOptions.IMEFunOn;

	sh.ContentType=FCT_SMS;
	sh.buffer=(char*)&gSms;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (gSms.ID==0) continue;
		valid=IsValidTimeDuration(EncodeTime(&gCurTime), &gSms);
		if (valid==1 && imefunon!=1)	//add "imefunon!=1"<liming>
		{
			//			lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
			smsid = gSms.ID;
			gSms.ID = 0;
			if (write(fdSms, sh.buffer, sizeof(TSms))==sizeof(TSms))
			{
				FDB_DelUData(0, smsid);
			}
			fsync(fdSms);
		}
		else if ((gSms.Tag==UDATA_TAG_ALL)&&(valid==0))
		{
			i++;
			if(i==(flag+1))
			{
				sprintf((char *)(content+len), "%s", gSms.Content);
				len+=strlen((char *)gSms.Content);
				break;
			}
		}
	}
	return strlen((char *)content);
}

int FDB_GetBoardNum(void)
{
	TSearchHandle sh;
	int num=0;
	int valid;//,len=0,i=1;
	int smsid;

	int imefunon=gOptions.IMEFunOn;
	sh.ContentType=FCT_SMS;
	sh.buffer=(char*)&gSms;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (gSms.ID==0) continue;
		valid=IsValidTimeDuration(EncodeTime(&gCurTime), &gSms);
		if (valid==1 && imefunon!=1)		//add "imefunon!=1"<liming>
		{
			lseek(fdSms, -1*sizeof(TSms), SEEK_CUR);
			smsid=gSms.ID;
			memset((unsigned char*)&gSms,0,sizeof(TSms));
			if (write(fdSms, sh.buffer, sizeof(TSms))==sizeof(TSms))
			{
				FDB_DelUData(0, smsid);
			}
			fsync(fdSms);
		}
		else if (gSms.Tag==UDATA_TAG_ALL && valid==0)
		{
			num++;
		}
	}

	return num;
}
#endif

BOOL CheckBoardSMS(void)
{
	memset(gBoardSMS, 0, 1024);
	gBoardSMSPos=0;
#ifndef ICLOCK
	return (FDB_ReadBoardSms(gBoardSMS)>0);
#else
	return (FDB_ReadBoardSms(gBoardSMS,0)>0);
#endif
}
BOOL CheckUserSMS(U16 pin, BYTE *smsContent)
{
	//处理用户多条短消息
#if 0
	U16 sms[10];
	TUData u;
	int i=0;

	memset(sms, 0, 20);
	lseek(fdUData, 0, SEEK_SET);
	while(TRUE)
	{
		if (read(fdUData, (void *)&u, sizeof(TUData))==sizeof(TUData))
		{
			if (u.PIN&&(u.PIN==pin))
			{
				printf("u.SMSID:%d\n",u.SmsID);
				sms[i++]=u.SmsID;
				if (i>=10) break;
			}
		}
		else break;
	}
	return ((i>0)&&FDB_ReadUserSms(sms, i, smsContent));
# endif
	TUData u;
	TSms sms;

	memset(&u, 0, sizeof(TUData));
	if(FDB_GetUData(pin,&u)!=NULL)
	{
		memset(&sms,0,sizeof(TSms));
		if(FDB_GetSms(u.SmsID,&sms)!=NULL)
		{
			if(IsValidTimeDuration(EncodeTime(&gCurTime), &sms)==0)
			{
				memset(smsContent, 0, sizeof(smsContent));
				/*dsl 2012.5.9 fix Traditional Chinese language can not correct display*/
				if (gOptions.Language == 84) {
					sprintf((char*)smsContent, "%s", sms.Content);
				} else {
					memcpy(smsContent,sms.Content, MAX_SMS_CONTENT_SIZE*2);
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}



enum Record_mode {
	OP_DEL = 0,
	OP_ADD_ONLY = 1,
	OP_ADD_OVERWRITE = 2,
	OP_UPDATE = 3,
	OP_IGNORE = 4
};



//buffer format description
//USER    FINGER   TEMPLATE   length(bytes)
// 4        4         4
//TUSERS
//TFINGERS  offset is based on TEMPLATE
//TEMPLATE(LEN 2B + template)

#define GETVALID(fingerID) (((fingerID&0xf0)>0)?(fingerID&0xf0)>>4:1)

int BatchOpUserTmpV10(char *buffer,int lenstart)
{
	PFingerS FingerS;
	BYTE *pTemplate;
	U32 len;
	U32 records;
	int i;
	U16 tmplen;
	U16  FingerCount=0;
	TZKFPTemplate tmp;

	FingerS=(PFingerS)(buffer+12+lenstart);
	memcpy(&len, buffer+4, 4);
	if(len)
	{
		records=(U32)(len/sizeof(TFingerS));
		pTemplate=(BYTE *)FingerS+len;
		for(i=0;i<records;i++)
		{
			if((FingerS[i].OpSign==OP_DEL))
			{
				//delete template
				//DBPRINTF("DelTmp:pin=%d,fingerid=%d\n",FingerS[i].PIN,FingerS[i].FingerID);
				FDB_DelTmp(FingerS[i].PIN,FingerS[i].FingerID&0x0f);
			}
			else if(FingerS[i].PIN&&((FingerS[i].OpSign==OP_ADD_ONLY)||(FingerS[i].OpSign==OP_ADD_OVERWRITE)))
			{
				BYTE valid = GETVALID(FingerS[i].FingerID);
				FingerCount = FDB_CntTmp();
				if(FingerCount>=gOptions.MaxFingerCount*100) break;
				//printf("AddTmp:pin=%d,fingerid=%d\n",FingerS[i].PIN,FingerS[i].FingerID);
				memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
				FDB_AddTmp((char *)FDB_CreateTemplate((char*)&tmp,
							FingerS[i].PIN, FingerS[i].FingerID&0x0f,
							(char *)(pTemplate+FingerS[i].OffSet+2),
							tmplen,valid));
				FingerS[i].PIN=0;
			}
		}
	}
	return 0;
}

void BatchOPUserData(BYTE *buffer)
{
	char buf[80];
	PUserS UserS;
	PFingerS FingerS;
	BYTE *pTemplate;
	U32 len;
	U32 records;
	TSearchHandle sh;
	int i, j, k, l;
	U16 tmplen;
	U16 UserCount=0, FingerCount=0;
	BOOL AddSign;
	char *FingerCacheBuf;
	int MaxFingerCacheCnt=200;
	int CurFingerCacheCnt;
	char *UserCacheBuf=NULL;
	int CurUserCacheSize;
	PUser user;
	int usernumber;
	int MaxGroupFingerCnt=100;
	int FingerNumber;
	int CurGroupFingerCnt;
	BOOL ChangeSign;
	//user table
	memcpy(&len, buffer, 4);
	if(len)
	{
		UserS=(PUserS)(buffer+12);
		records=(U32)(len/sizeof(TUserS));
		//read data to memory
		CurUserCacheSize=lseek(fdUser, 0, SEEK_END);
		if(CurUserCacheSize)
		{
			UserCacheBuf=MALLOC(CurUserCacheSize);
			lseek(fdUser, 0, SEEK_SET);
			read(fdUser, UserCacheBuf, CurUserCacheSize);
		}
		usernumber=CurUserCacheSize/sizeof(TUser);
		user=(PUser)UserCacheBuf;
		//acquired the status for operation
		for(j=0;j<usernumber;j++)
		{
			if(user[j].PIN && user[j].PIN2[0])
			{
				UserCount++;
				for(i=0;i<records;i++)
				{
					//工号相同，为同一个用户
					if(strcmp(user[j].PIN2,UserS[i].User.PIN2)==0)
					{
						if(UserS[i].OpSign==OP_ADD_ONLY)
							UserS[i].OpSign=OP_IGNORE;
						else if(UserS[i].OpSign==OP_ADD_OVERWRITE)
							UserS[i].OpSign=OP_UPDATE;
					}
				}
			}
		}
		//execute
		AddSign=TRUE;
		for(j=0;j<usernumber;j++)
		{
			for(i=0;i<records;i++)
			{
				if(strcmp(user[j].PIN2,UserS[i].User.PIN2)==0)
				{
					if(UserS[i].OpSign==OP_DEL)
					{
						//delete user
						user[j].PIN=0;
						memset(user[j].PIN2,0,24);
						UserCount--;
					}
					else if(UserS[i].OpSign==OP_UPDATE)
					{
						//update
						memcpy((void*)&(user[j]), (void*)&(UserS[i].User), sizeof(TUser));
					}
					//there maybe have two records for one ID.
					break;
				}
			}
			//append record
			if(AddSign&&(user[j].PIN==0 && user[j].PIN2[0])&&(UserCount<gOptions.MaxUserCount*100))
			{
				AddSign=FALSE;
				for(i=0;i<records;i++)
				{
					//append record
					if(UserS[i].User.PIN && user[j].PIN2[0]&&((UserS[i].OpSign==OP_ADD_ONLY)||(UserS[i].OpSign==OP_ADD_OVERWRITE)))
					{
						memcpy((void*)&(user[j]), (void*)&(UserS[i].User), sizeof(TUser));
						UserS[i].User.PIN=0;
						UserCount++;
						AddSign=TRUE;
						break;
					}
				}
			}
		}
		//calculate all record that need append
		if(AddSign)
		{
			for(i=0;i<records;i++)
			{
				if(UserS[i].User.PIN &&((UserS[i].OpSign==OP_ADD_ONLY)||(UserS[i].OpSign==OP_ADD_OVERWRITE)))
					usernumber++;
			}
		}
		//append records
		if(j<usernumber)
		{
			UserCacheBuf =REALLOC(UserCacheBuf, usernumber*sizeof(TUser));
			user = (PUser)UserCacheBuf;
			for(i=0;i<records;i++)
			{
				//append record
				if(UserS[i].User.PIN &&((UserS[i].OpSign==OP_ADD_ONLY)||(UserS[i].OpSign==OP_ADD_OVERWRITE)))
				{
					if(UserCount>=gOptions.MaxUserCount*100) break;
					memcpy((void*)&(user[j]), (void*)&(UserS[i].User), sizeof(TUser));
					UserS[i].User.PIN=0;
					UserCount++;
					j++;
					if(j>=usernumber) break;
				}
			}
		}
		//write data with O_TRUNC AND NO O_SYNC
		if(j)
		{
			close(fdUser);
			fdUser=open(GetEnvFilePath("USERDATAPATH", "ssruser.dat", buf), O_RDWR|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			write(fdUser, UserCacheBuf, j*sizeof(TUser));
			fsync(fdUser);
			close(fdUser);
			fdUser=open(GetEnvFilePath("USERDATAPATH", "ssruser.dat", buf), O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		}
		if(UserCacheBuf) FREE(UserCacheBuf);
	}
	//template table
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		BatchOpUserTmpV10((char *)buffer,len);
		return;
	}
	FingerS=(PFingerS)(buffer+12+len);
	memcpy(&len, buffer+4, 4);
	if(len)
	{
		records=(U32)(len/sizeof(TFingerS));
		pTemplate=(BYTE *)FingerS+len;

		close(fdFingerTmp);
		/* zsliu change ,this firmware can't use in zem500 board
		   if ((gOptions.MaxAttLogCount*10000) >ATTLOGLIMIT)
		   fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
		   else
		   fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
		*/
		fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);

		sh.ContentType=FCT_FINGERTMP;
		sh.buffer=(char *)gTemplate;

		//acquired the status for operation
		FingerNumber=0;
		SearchFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen)
			{
				FingerCount++;
				for(i=0;i<records;i++)
				{
					if((((PTemplate)sh.buffer)->Valid)&&
							(((PTemplate)sh.buffer)->PIN==FingerS[i].PIN)&&
							(((PTemplate)sh.buffer)->FingerID==(FingerS[i].FingerID&0x0f)))
					{

						if(FingerS[i].OpSign==OP_ADD_ONLY)
							FingerS[i].OpSign=OP_IGNORE;
						else if(FingerS[i].OpSign==OP_ADD_OVERWRITE)
							FingerS[i].OpSign=OP_UPDATE;
					}
				}
			}
			FingerNumber++;
		}
		//execute
		SearchFirst(&sh);
		AddSign=TRUE;
		j=1;
		k=0;
		FingerCacheBuf=MALLOC(MaxGroupFingerCnt*sizeof(TTemplate));
		while(k<FingerNumber)
		{
			CurGroupFingerCnt=MaxGroupFingerCnt;
			if ((k+MaxGroupFingerCnt)>FingerNumber)
				CurGroupFingerCnt=FingerNumber-k;
			read(fdFingerTmp, FingerCacheBuf, CurGroupFingerCnt*sizeof(TTemplate));
			k+=CurGroupFingerCnt;
			DBPRINTF("GROUP %d Starting......\n", j);
			ChangeSign=FALSE;
			for(l=0;l<CurGroupFingerCnt;l++)
			{
				sh.buffer=(char*)((PTemplate)FingerCacheBuf+l);
				for(i=0;i<records;i++)
				{
					if((((PTemplate)sh.buffer)->Valid)&&
							(((PTemplate)sh.buffer)->PIN==FingerS[i].PIN))
					{
						if((FingerS[i].OpSign==OP_DEL)&&
								((FingerS[i].FingerID==0xFF)||(((PTemplate)sh.buffer)->FingerID==(FingerS[i].FingerID&0x0f))))
						{
							//delete template
							((PTemplate)sh.buffer)->Valid=0;
							FingerCount--;
							ChangeSign=TRUE;
						}
						else if((FingerS[i].OpSign==OP_UPDATE)&&
								(((PTemplate)sh.buffer)->FingerID==(FingerS[i].FingerID&0x0f)))
						{
							BYTE valid = GETVALID(FingerS[i].FingerID);
							memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
							FDB_CreateTemplate(sh.buffer,
									FingerS[i].PIN, FingerS[i].FingerID&0x0f,
									(char *)(pTemplate+FingerS[i].OffSet+2),
									tmplen,valid);
							ChangeSign=TRUE;
						}
					}
				}
				//append record
				if(AddSign&&(((PTemplate)sh.buffer)->Valid==0)&&(FingerCount<gOptions.MaxFingerCount*100))
				{
					AddSign=FALSE;
					for(i=0;i<records;i++)
					{
						//append record
						if(FingerS[i].PIN&&((FingerS[i].OpSign==OP_ADD_ONLY)||(FingerS[i].OpSign==OP_ADD_OVERWRITE)))
						{
							BYTE valid = GETVALID(FingerS[i].FingerID);
							memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
							FDB_CreateTemplate(sh.buffer,
									FingerS[i].PIN, FingerS[i].FingerID&0x0f,
									(char *)(pTemplate+FingerS[i].OffSet+2),
									tmplen,valid);
							FingerS[i].PIN=0;
							FingerCount++;
							AddSign=TRUE;
							ChangeSign=TRUE;
							break;
						}
					}
				}
			}
			if(ChangeSign)
			{
				lseek(fdFingerTmp, -1*CurGroupFingerCnt*sizeof(TTemplate), SEEK_CUR);
				write(fdFingerTmp, FingerCacheBuf, CurGroupFingerCnt*sizeof(TTemplate));
			}
			DBPRINTF("GROUP %d Endded\n", j);
			j++;
		}
		FREE(FingerCacheBuf);
		sh.buffer=(char *)gTemplate;//2009.12.11 fix upload finger count error
		if(AddSign)
		{
			//append all record that left
			lseek(fdFingerTmp, 0, SEEK_END);
			FingerCacheBuf=MALLOC(MaxFingerCacheCnt*sizeof(TTemplate));
			CurFingerCacheCnt=0;
			for(i=0;i<records;i++)
			{
				//append record
				if(FingerS[i].PIN&&((FingerS[i].OpSign==OP_ADD_ONLY)||(FingerS[i].OpSign==OP_ADD_OVERWRITE)))
				{
					BYTE valid = GETVALID(FingerS[i].FingerID);
					if(FingerCount>=gOptions.MaxFingerCount*100) break;
					memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
					FDB_CreateTemplate(sh.buffer,
							FingerS[i].PIN, FingerS[i].FingerID&0x0f,
							(char *)(pTemplate+FingerS[i].OffSet+2),
							tmplen,valid);
					memcpy(FingerCacheBuf+CurFingerCacheCnt*sizeof(TTemplate), (void*)sh.buffer, sizeof(TTemplate));
					CurFingerCacheCnt++;
					if(CurFingerCacheCnt==MaxFingerCacheCnt)
					{
						write(fdFingerTmp, FingerCacheBuf, CurFingerCacheCnt*sizeof(TTemplate));
						CurFingerCacheCnt=0;
						DBPRINTF("Write cache data to disk!\n");
					}
					//write(fdFingerTmp, (void*)sh.buffer, sizeof(TTemplate));
					FingerS[i].PIN=0;
					FingerCount++;
				}
			}
			if(CurFingerCacheCnt)
			{
				write(fdFingerTmp, FingerCacheBuf, CurFingerCacheCnt*sizeof(TTemplate));
				DBPRINTF("Write cache data to disk finished!\n");
			}
			FREE(FingerCacheBuf);
		}
		fsync(fdFingerTmp);
		close(fdFingerTmp);
		/* zsliu change ,this firmware can't use in zem500 board
		   if ((gOptions.MaxAttLogCount*10000) >ATTLOGLIMIT)
		   fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "template.dat", buf), O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		   else
		   fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		   */
		fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);

		sync();
	}
}


//liming??????????????????????
void FDB_UploadUserInfo(void)
{
	TUser tuser;
	char* oldInfoBuf=NULL;//, p;
	char buf[80];
	int fd=-1;
	int recsize, recnum;

	sprintf(buf, "%s/%s", USB_MOUNTPOINT, "user.dat");
	if ((fd=open(buf, O_RDONLY))!=-1)			//打开文件"user.dat"
	{
		recsize = sizeof(TUser);
		recnum = 0;
		lseek(fdUser, 0, SEEK_SET);
		while(read(fdUser, &tuser, sizeof(TUser))==sizeof(TUser))
		{
			if (tuser.PIN && tuser.PIN2[0])		//有效用户记录
			{
				if(oldInfoBuf==NULL)
					oldInfoBuf = MALLOC(recsize);
				else
					oldInfoBuf = REALLOC(oldInfoBuf, recsize);

				//p = oldInfoBuf + recnum*sizeof(TUser);
			}
		}
	}
}

typedef struct _PIN2Rec_{
	U16 PIN;
	U32 PIN2;
}TPIN2Rec, *PPIN2Rec;

//为了保持大小彩屏webserver的路径设置一致,必须将一些表放到/mnt/mtdblock/
void UpdateDBforWeb(void)
{
	struct stat dblog;
	char buf[1024];

	fstat(fdUser1, &dblog);
	if(dblog.st_size>0)
	{
		close(fdUser);
		close(fdUser1);
		systemEx("mv /mnt/mtdblock/data/ssruser.dat /mnt/mtdblock/  && sync");
		fdUser=open(GetEnvFilePath("USERDATAPATH", "ssruser.dat", buf), O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	}
	fstat(fdtz1, &dblog);
	if(dblog.st_size>0)
	{
		close(fdtz);
		close(fdhtz);
		close(fdgroup);
		close(fdcgroup);
		close(fdtz1);
		systemEx("mv /mnt/mtdblock/data/timezone.dat /mnt/mtdblock/ && sync");
		systemEx("mv /mnt/mtdblock/data/htimezone.dat /mnt/mtdblock/  && sync");
		systemEx("mv /mnt/mtdblock/data/group.dat /mnt/mtdblock/ && sync");
		systemEx("mv /mnt/mtdblock/data/lockgroup.dat /mnt/mtdblock/ && sync");

		fdtz = open(GetEnvFilePath("USERDATAPATH", "timezone.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdhtz = open(GetEnvFilePath("USERDATAPATH", "htimezone.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdgroup = open(GetEnvFilePath("USERDATAPATH", "group.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
		fdcgroup = open(GetEnvFilePath("USERDATAPATH", "lockgroup.dat", buf),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
	}
	systemEx("rm -rf /mnt/mtdblock/data/timezone.dat && sync");
	systemEx("rm -rf /mnt/mtdblock/data/htimezone.dat && sync");
	systemEx("rm -rf /mnt/mtdblock/data/group.dat && sync");
	systemEx("rm -rf /mnt/mtdblock/data/lockgroup.dat && sync");
	systemEx("rm -rf /mnt/mtdblock/data/ssruser.dat && sync");
	sync();
	printf("update db to /mnt/mtdblock finish.....\n");
}
void UpdateAttLog(int logtype)
{
	char buf[16];
	TOldAttLog log;
	TExtendAttLog extlog;
	time_t t=0;
	int s;
	struct stat attlog;
	PPIN2Rec pin2rec=NULL;
	TSearchHandle sh;
	U8 databuf[64];
	int I=0;
	int UserCount=0;
	U32 pin2;
	U16 pin;
	char pinbuf[24];

	memset(pinbuf,0,24);
	if (logtype==OldLog)
		fstat(fdOldTransaction, &attlog);
	else
		fstat(fdExtLog, &attlog);
	//DBPRINTF("attlog.st_size: %d\n",attlog.st_size);
	if(attlog.st_size>0)
	{
		if(gOptions.PIN2Width!=PIN_WIDTH2)
		{
			UserCount=GetDataInfo(FCT_OldUSER, STAT_COUNT, 0);
			pin2rec=(PPIN2Rec)MALLOC(UserCount*sizeof(TPIN2Rec));
			memset(pin2rec, 0, UserCount*sizeof(TPIN2Rec));
			sh.ContentType=FCT_OldUSER;
			sh.buffer=(char *)databuf;
			SearchFirst(&sh);
			while(!SearchNext(&sh))
			{
				if(sh.datalen>0)
				{
					pin2rec[I].PIN=((POldUser)(sh.buffer))->PIN;
					pin2rec[I].PIN2=((POldUser)(sh.buffer))->PIN2;
					I++;
				}
			}
		}
		if (logtype==OldLog)
			lseek(fdOldTransaction, 0, SEEK_SET);
		else
			lseek(fdExtLog,0,SEEK_SET);
		while(TRUE)
		{
			if (logtype==OldLog)
			{
				if (read(fdOldTransaction, buf, 4)==4)
				{
					if (IsAttLogLongPack(buf[2]))
						read(fdOldTransaction, buf+4, 4);
					s=UnpackAttLog(buf, &log);
					if(s==AttLogSize2)
						t=log.time_second;
					else
						log.time_second+=t;
					pin2=0;
					memset(pinbuf,0,24);
					//search pin2
					if(gOptions.PIN2Width!=PIN_WIDTH2)
					{
						for(I=0;I<UserCount;I++)
						{
							if(pin2rec[I].PIN==log.PIN)
							{
								pin2=pin2rec[I].PIN2;
								sprintf(pinbuf,"%d",pin2);
								break;
							}
						}
					}
					FDB_AddAttLog(log.PIN, log.time_second, log.verified, log.status, pinbuf, 0, 0);
					//DBPRINTF("update attlog: %d\n",log.PIN);
				}
				else
					break;
			}
			else
			{
				if (read(fdExtLog,&extlog,sizeof(TExtendAttLog))==sizeof(TExtendAttLog))
				{
					pin2=0;
					pin=0;
					memset(pinbuf,0,24);
					//search pin2
					if(gOptions.PIN2Width!=PIN_WIDTH2)
					{
						for(I=0;I<UserCount;I++)
						{
							if(pin2rec[I].PIN2==extlog.PIN)
							{
								pin2=pin2rec[I].PIN2;
								pin = pin2rec[I].PIN;
								sprintf(pinbuf,"%d",pin2);
								break;
							}
						}
					}
					FDB_AddAttLog(pin, extlog.time_second, extlog.verified, extlog.status, pinbuf, 0, 0);
					//DBPRINTF("update extlog: %s\tpin:%d\n",pinbuf,pin);
				}
				else
					break;
			}

		}
		if (logtype==OldLog)
			fdOldTransaction=TruncFDAndSaveAs(fdOldTransaction, GetEnvFilePath("USERDATAPATH", "data/transaction.dat", buf), NULL, 0);
		else
			fdExtLog=TruncFDAndSaveAs(fdExtLog, GetEnvFilePath("USERDATAPATH", "data/extlog.dat", buf), NULL, 0);
		if(pin2rec) FREE(pin2rec);
		sync();
	}
}

void UpdateUserDb(void)
{
	char buf[16];
	TOldUser olduser;
	TUser newuser;
	struct stat attuser;
	char pinbuf[24];

	memset(pinbuf,0,24);
	fstat(fdOldUser, &attuser);
	//printf("user stat: %d\n",attuser.st_size);
	lseek(fdOldUser,0,SEEK_SET);
	if(attuser.st_size>0) {
		while(TRUE) {
			if (read(fdOldUser,&olduser,sizeof(TOldUser))==sizeof(TOldUser)) {
				memset(pinbuf,0,24);
				memset(&newuser,0,sizeof(TUser));
				newuser.PIN = olduser.PIN;
				if(gOptions.PIN2Width > PIN_WIDTH2) {
					sprintf(newuser.PIN2,"%d",olduser.PIN2);
				} else {
					sprintf(newuser.PIN2,"%d",olduser.PIN);
				}
				if (olduser.Name) {
					nstrcpy(newuser.Name,olduser.Name,strlen(olduser.Name));
				}
				if (olduser.Card[0]) {
					memcpy(newuser.Card,olduser.Card,4);
				}
				if (olduser.Password) {
					nstrcpy(newuser.Password,olduser.Password,strlen(olduser.Password));
				}
				FDB_AddUser(&newuser);
			} else {
				break;
			}

		}
		fdOldUser=TruncFDAndSaveAs(fdOldUser, GetEnvFilePath("USERDATAPATH", "data/user.dat", buf), NULL, 0);
		fsync(fdOldUser);
	}
}

void GetFilterGroupInfo(int inputpin, PFilterRec filterbuf)
{
	TSearchHandle sh;
	TUser User;
	int i=0;
	memset(filterbuf, 0, gOptions.MaxUserCount*100*sizeof(TFilterRec));
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&User;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen>0)
		{
			if((((PUser)sh.buffer)->Group&0x0F)==inputpin)
				filterbuf[i++].PIN=((PUser)sh.buffer)->PIN;
		}
	}
}

void GetFilterHeadInfo(int inputpin, PFilterRec filterbuf)
{
	TSearchHandle sh;
	TUser User;
	int i=0;
	memset(filterbuf, 0, gOptions.MaxUserCount*100*sizeof(TFilterRec));
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&User;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen>0)
		{
			filterbuf[i].PIN=((PUser)sh.buffer)->PIN;
			strtou32(((PUser)sh.buffer)->PIN2,&(filterbuf[i].PIN2));
			//filterbuf[i].PIN2=((PUser)sh.buffer)->PIN2;
			i++;
		}
	}
}

//Ext User info operation functions
PExtUser FDB_CreateExtUser(PExtUser extuser, U16 pin, U8 verifystyle)
{
	memset((void*)extuser, 0, sizeof(TExtUser));
	extuser->PIN=pin;
	extuser->VerifyStyle=verifystyle;
	return extuser;
}

int FDB_AddExtUser(PExtUser extuser)
{
	return SearchAndSave(FCT_EXTUSER, (char*)extuser, sizeof(TExtUser));
}

int FDB_ChgExtUser(PExtUser extuser)
{
	PExtUser u;
	int ret;

	if((u=FDB_GetExtUser(extuser->PIN, NULL))==NULL) return FDB_ERROR_NODATA;
	if(0==memcmp((void*)extuser, (void*)u, sizeof(TExtUser))) return FDB_OK;
	//overwrite user
	lseek(fdExtUser, -1*sizeof(TExtUser), SEEK_CUR);
	if(write(fdExtUser, (void*)extuser, sizeof(TExtUser))==sizeof(TExtUser))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdExtUser);

	return ret;
}

int FDB_DelExtUser(U16 pin)
{
	TExtUser u;
	int ret;
	if((FDB_GetExtUser(pin, &u))==NULL) return FDB_ERROR_NODATA;

	//	uid=u.PIN;
	//overwrite user
	memset(&u, 0, sizeof(TExtUser));
	lseek(fdExtUser, -1*sizeof(TExtUser), SEEK_CUR);
	if(write(fdExtUser, (void*)&u, sizeof(TExtUser))==sizeof(TExtUser))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdExtUser);

	return ret;
}

static TExtUser gExtUser;

PExtUser FDB_GetExtUser(U16 pin, PExtUser extuser)
{
	TSearchHandle sh;

	sh.ContentType=FCT_EXTUSER;
	sh.buffer=(char*)&gExtUser;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(((PExtUser)sh.buffer)->PIN==pin)
		{
			if (extuser)
			{
				memcpy(extuser, sh.buffer, sizeof(TExtUser));
				return extuser;
			}
			else
				return (PExtUser)sh.buffer;
		}
	}
	return NULL;
}

PUserlb SortUserlb(PUserlb head)
{
	PUserlb h,p,q,r,s;
	h=p=(PUserlb)MALLOC(sizeof(TUserlb));
	p->next=head;

	while(p->next!=NULL)
	{
		r=p;
		q=p->next;
		while(q->next!=NULL)
		{
			//asc
			if(q->next->userlb.PIN <  r->next->userlb.PIN)
				r=q;
			q=q->next;
		}
		if(r!=p)
		{
			s=r->next;
			r->next=s->next;
			s->next=p->next;
			p->next=s;
		}
		p=p->next;
	}
	head=h->next;
	FREE(h);
	return head;
}

PUserlb GetOneUsertolb(int pin)
{
	PUserlb  head,pb,pf;
	PUser puser;
	if(pin <= 0) {
		return NULL;
	}
	head = pb = pf = NULL;
	puser = MALLOC(sizeof(TUser));
	memset(puser,0,sizeof(TUser));
	if (fdUser < 0) {
		return NULL;
	}
	lseek(fdUser,0,0);
	while (read(fdUser,puser,sizeof(TUser))==sizeof(TUser)) {
		if (puser->PIN > 0 && ((pin>0 && pin == puser->PIN))) {
			pb = (PUserlb)MALLOC(sizeof(TUserlb));
			memset(pb,0,sizeof(TUserlb));
			memcpy(&pb->userlb,puser,sizeof(TUser));
			pf=head=pb;
			pb->next = NULL;
			break;
		}
	}
	FREE(puser);
	if (head == NULL) {
		return NULL;
	} else {
		return head;
	}
}

int FDB_InitUDataPoint(void)
{
	if(fdUData<0) return 0;
	lseek(fdUData,0,0);
	return 1;
}

int FDB_ReadUData(PUData pudata,int smsID)
{
	memset(pudata,0,sizeof(TUData));
	if((read(fdUData,pudata,sizeof(TUData))==sizeof(TUData))&&(pudata->SmsID==smsID))
		return 1;
	else
		return 0;
}

int FDB_InitSmsPoint(void)
{
	if(fdSms<0) return 0;
	lseek(fdSms,0,0);
	return 1;
}

int FDB_ReadSms(PSms psms)
{
	memset(psms,0,sizeof(TSms));
	if((read(fdSms,psms,sizeof(TSms))==sizeof(TSms)))
	{
		//printf("psms->ID:%d\n",psms->ID);
		return 1;
	}
	else
		return 0;
}

PUserlb GetUsermestolb(int deptid)
{

	PUserlb  head,pb,pf;
	PUser puser;
	head = pb = pf = NULL;
	int i = 0;
	puser = MALLOC(sizeof(TUser));
	memset(puser,0,sizeof(TUser));
	if (fdUser < 0)
		return NULL;
	lseek(fdUser,0,0);
	//printf("before read\n");
	while (read(fdUser,puser,sizeof(TUser))==sizeof(TUser))
	{
		if (puser->PIN >0)
		{
			pb = (PUserlb)MALLOC(sizeof(TUserlb));
			memset(pb,0,sizeof(TUserlb));

			if (deptid == 0)
				memcpy(&pb->userlb,puser,sizeof(TUser));
			else
			{
				if ((U8)deptid == puser->Group)
					memcpy(&pb->userlb,puser,sizeof(TUser));
			}
			//printf("userid: %d\n",pb->userlb.PIN);
			//printf("name: %-5s\n",pb->userlb.Name);
			//printf("dept: %d\n",pb->userlb.Group);
			//printf("sch: %d\t%d\t%d\t%d\n",pb->userlb.TimeZones[0],pb->userlb.TimeZones[1],
			//				pb->userlb.TimeZones[2],pb->userlb.TimeZones[3]);
			//printf("i: %d\n",i);
			if (pb->userlb.PIN > 0)
			{
				if (i==0)
					pf=head=pb;
				else
					pf->next=pb;
				pb->next=NULL;
				pf=pb;
				i++;
			}
		}

	}
	FREE(puser);
	if (head == NULL)
		return NULL;
	else
	{
		//printf("finish read\n");
		//sort user asc
		if (gOptions.PIN2Width <= 5)
			head =SortUserlb(head);
		return head;
	}
}

void SetUserInfoDbFirst()
{
	lseek(fdUser,0,SEEK_SET);
}

int  ReadUserInfoDB(PUser user)
{
	return (read(fdUser,user,sizeof(TUser))==sizeof(TUser));
}


int GetDayAttLog(U16 pin,time_t Sttm,time_t endtm,char *content)
{
	int ret = 0;
	int i=0;
	char *logBuffer;
	char buf[1024]="";
	TTime tt,tt1;
	logBuffer = (char *)MALLOC(5*1024);
	PAttLog logs=(PAttLog)logBuffer;
	int c=(5*1024)/40;
	strcat(content,"<tr>\r\n");
	strcat(content,"<td width=100% height=16>");
	OldDecodeTime(Sttm,&tt);

	c=FDB_GetAttLog(pin,Sttm,endtm,logs,c);

	sprintf(buf,"%2d-%2d&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;",tt.tm_mon+1,tt.tm_mday);
	strcat(content,buf);
	if (c>0) {
		while (i>=0 && i<c)
		{
			OldDecodeTime(logs[i].time_second,&tt1);
			sprintf(buf,"%02d:%02d&nbsp;&nbsp;&nbsp;",tt1.tm_hour, tt1.tm_min);
			strcat(content,buf);
			i++;
		}
	}
	strcat(content,"</td>\r\n");
	strcat(content,"</tr>\r\n");
	FREE(logBuffer);
	return ret;
}

void FDB_SetAttLogReadAddr(U32 addr)
{
	if(attLogReadPos!=addr)	{
		attLogReadPos=addr;
	}
}

void FDB_SetOpLogReadAddr(U32 addr)
{
	if(opLogReadPos!=addr) {
		opLogReadPos=addr;
	}
}

BYTE *ReadNewDataBlock(BYTE ContentType, int *size, int LogReadPos)
{
	TSearchHandle sh;
	U8 buf[1024];
	int validLen;
	U8 *validBuf;
	U8 *p;

	validLen=GetDataInfo(ContentType, STAT_VALIDLEN, 0);
	*size=validLen+4;
	if (validLen>0)
	{
		validBuf=MALLOC(validLen+4);
		memcpy(validBuf, &validLen, 4);
		p=validBuf+4;

		sh.ContentType=(int)ContentType;
		sh.buffer=(char *)buf;
		SearchNewFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen>0)
			{
				memcpy(p, sh.buffer, sh.datalen);
				p+=sh.datalen;
			}
		}
		return validBuf;
	}
	else
		return NULL;
}

char* FDB_ReadNewAttLogBlock(int *size)
{
	return (char*)ReadNewDataBlock(FCT_ATTLOG, size, attLogReadPos);
}

char* FDB_ReadNewOpLogBlock(int *size)
{
	return (char*)ReadNewDataBlock(FCT_OPLOG, size, opLogReadPos);
}

void SearchNewFirst(PSearchHandle sh)
{
	sh->fd=SelectFDFromConentType(sh->ContentType);
	if(sh->ContentType==FCT_ATTLOG && attLogReadPos!=LOG_READ_NONE)
		lseek(sh->fd,0, attLogReadPos);
	else if(sh->ContentType==FCT_OPLOG && opLogReadPos!=LOG_READ_NONE)
		lseek(sh->fd,0, opLogReadPos);
	else
		lseek(sh->fd, 0,SEEK_SET );
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}

//liming <2007.08>
//time zone
int FDB_CntTimeZone(void)
{
	return GetDataInfo(FCT_TZ, STAT_COUNT, 0);
}

int FDB_AddTimeZone(PTimeZone timezone)
{
	if (FDB_CntTimeZone()>=TZ_MAX)
		return FDB_ERROR_NOSPACE;
	return SearchAndSave(FCT_TZ, (char*)timezone, sizeof(TTimeZone));

}

static TTimeZone gTZ;
PTimeZone FDB_GetTimeZone(U16 id, PTimeZone timezone)
{
	TSearchHandle sh;

	sh.ContentType=FCT_TZ;
	sh.buffer=(char*)&gTZ;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PTimeZone)sh.buffer)->ID==id)
		{
			if (timezone)
			{
				memcpy(timezone, sh.buffer, sizeof(TTimeZone));
				return timezone;
			}
			else
				return (PTimeZone)sh.buffer;
		}
	}
	return NULL;
}

int FDB_ChgTimeZone(PTimeZone timezone)
{
	PTimeZone s;
	int ret;

	if ((s=FDB_GetTimeZone(timezone->ID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)timezone, (void*)s, sizeof(TTimeZone))) return FDB_OK;
	//overwrite
	lseek(fdtz, -1*sizeof(TTimeZone), SEEK_CUR);
	if (write(fdtz, (void*)timezone, sizeof(TTimeZone))==sizeof(TTimeZone))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdtz);

	return ret;
}

int FDB_DelTimeZone(U16 id)
{
	TTimeZone s;
	PUser puser=NULL;
	PGroup pgp=NULL;
	PHTimeZone phtz=NULL;

	int i, bchanged;

	if ((FDB_GetTimeZone(id, &s))==NULL) return FDB_ERROR_NODATA;
	//overwrite
	memset(&s, 0, sizeof(TTimeZone));
	lseek(fdtz, -1*sizeof(TTimeZone), SEEK_CUR);
	if (write(fdtz, (void*)&s, sizeof(TTimeZone))==sizeof(TTimeZone))
	{


		fsync(fdtz);

		//clear user timezone.
		if(fdUser)
		{
			bchanged=0;
			lseek(fdUser,0,0);
			puser = (PUser)MALLOC(sizeof(TUser));
			memset(puser, 0, sizeof(TUser));
			while(read(fdUser, puser, sizeof(TUser))==sizeof(TUser))
			{
				if(puser->PIN)
				{
					if(puser->TimeZones[0]==1)	//use own time zone
					{
						for(i=1;i<4;i++)
						{
							if(puser->TimeZones[i]==id)
							{
								puser->TimeZones[i]=0;
								bchanged=1;
							}
						}
						if(bchanged)
						{
							lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
							if(write(fdUser, puser, sizeof(TUser))!=sizeof(TUser))
								break;
						}
					}
				}
			}
			fsync(fdUser);
			FREE(puser);
		}

		//clear group timezone
		if(fdgroup)
		{
			bchanged=0;
			lseek(fdgroup,0,0);
			pgp = (PGroup)MALLOC(sizeof(TGroup));
			memset(pgp, 0, sizeof(TGroup));
			while(read(fdgroup, pgp, sizeof(TGroup))==sizeof(TGroup))
			{
				if(pgp->ID)
				{
					for(i=0;i<3;i++)
					{
						if(pgp->TZID[i]==id)
						{
							pgp->TZID[i]=0;
							bchanged=1;
						}
					}
				}
				if(bchanged)
				{
					lseek(fdgroup, -1*sizeof(TGroup), SEEK_CUR);
					if(write(fdgroup, pgp, sizeof(TGroup))!=sizeof(TGroup))
						break;
				}
			}
			fsync(fdgroup);
		}

		//clear holiday timezone
		if(fdhtz)
		{
			bchanged=0;
			lseek(fdhtz,0,0);
			phtz = (PHTimeZone)MALLOC(sizeof(THTimeZone));
			memset(phtz, 0, sizeof(THTimeZone));
			while(read(fdhtz, phtz, sizeof(THTimeZone))==sizeof(THTimeZone))
			{
				if(phtz->ID && (phtz->TZID==id))
				{
					phtz->TZID=0;
					lseek(fdhtz, -1*sizeof(THTimeZone), SEEK_CUR);
					if(write(fdhtz, phtz, sizeof(THTimeZone))!=sizeof(THTimeZone))
						break;
				}
			}
			fsync(fdhtz);
		}

		//clear options.cfg parameters
		bchanged=0;
		if(gOptions.DoorCloseTimeZone==id)        //常闭时间段
		{
			gOptions.DoorCloseTimeZone=0;
			bchanged=1;
		}
		if(gOptions.DoorOpenTimeZone==id)         //常开时间段
		{
			gOptions.DoorOpenTimeZone=0;
			bchanged=1;
		}
		if(bchanged)
		{
			SaveOptions(&gOptions);
			LoadOptions(&gOptions);
		}

		return FDB_OK;
	}
	else
	{
		return FDB_ERROR_IO;
	}
}

//holiday
int FDB_CntHTimeZone(void)
{
	return GetDataInfo(FCT_HTZ, STAT_COUNT, 0);
}

int FDB_AddHTimeZone(PHTimeZone htimezone)
{
	if (FDB_CntHTimeZone()>=HTZ_MAX)
		return FDB_ERROR_NOSPACE;
	return SearchAndSave(FCT_HTZ, (char*)htimezone, sizeof(THTimeZone));

}

static THTimeZone gHTZ;
PHTimeZone FDB_GetHTimeZone(U16 id, PHTimeZone htimezone)
{
	TSearchHandle sh;

	sh.ContentType=FCT_HTZ;
	sh.buffer=(char*)&gHTZ;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PHTimeZone)sh.buffer)->ID==id)
		{
			if (htimezone)
				memcpy(htimezone, sh.buffer, sizeof(THTimeZone));
			return (PHTimeZone)sh.buffer;
		}
	}
	return NULL;
}

int FDB_ChgHTimeZone(PHTimeZone htimezone)
{
	PHTimeZone s;
	int ret;

	if ((s=FDB_GetHTimeZone(htimezone->ID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)htimezone, (void*)s, sizeof(THTimeZone))) return FDB_OK;
	//overwrite
	lseek(fdhtz, -1*sizeof(THTimeZone), SEEK_CUR);
	if (write(fdhtz, (void*)htimezone, sizeof(THTimeZone))==sizeof(THTimeZone))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdhtz);

	return ret;

}

int FDB_DelHTimeZone(U16 id)
{
	THTimeZone s;
	int ret;
	if ((FDB_GetHTimeZone(id, &s))==NULL) return FDB_ERROR_NODATA;
	//overwrite
	memset(&s, 0, sizeof(THTimeZone));
	lseek(fdhtz, -1*sizeof(THTimeZone), SEEK_CUR);
	if (write(fdhtz, (void*)&s, sizeof(THTimeZone))==sizeof(THTimeZone))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdhtz);

	return ret;
}

//group
int FDB_CntGroup(void)
{
	return GetDataInfo(FCT_GROUP, STAT_COUNT, 0);
}

int FDB_AddGroup(PGroup group)
{
	if (FDB_CntGroup()>=GP_MAX)
		return FDB_ERROR_NOSPACE;
	return SearchAndSave(FCT_GROUP, (char*)group, sizeof(TGroup));
}

static TGroup gGp;
PGroup FDB_GetGroup(U8 id, PGroup group)
{
	TSearchHandle sh;

	sh.ContentType=FCT_GROUP;
	sh.buffer=(char*)&gGp;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PGroup)sh.buffer)->ID==id)
		{
			if (group)
				memcpy(group, sh.buffer, sizeof(TGroup));
			return (PGroup)sh.buffer;
		}
	}
	return NULL;
}

int FDB_ChgGroup(PGroup group)
{
	PGroup s;
	int ret;

	if ((s=FDB_GetGroup(group->ID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)group, (void*)s, sizeof(TGroup))) return FDB_OK;
	//overwrite
	lseek(fdgroup, -1*sizeof(TGroup), SEEK_CUR);
	if (write(fdgroup, (void*)group, sizeof(TGroup))==sizeof(TGroup))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdgroup);

	return ret;
}

int beUsedGroup(U8 id)
{
	PUser puser;
	int usercount=0;

	if(fdUser)
	{
		lseek(fdUser,0,0);
		puser=(PUser)MALLOC(sizeof(TUser));
		memset(puser, 0, sizeof(TUser));
		while(read(fdUser, puser, sizeof(TUser))==sizeof(TUser))
		{
			if(puser->PIN && puser->Group==id)
				usercount++;
		}

		FREE(puser);
	}

	return usercount;
}

int FDB_DelGroup(U8 id)
{
	TGroup s;
	PUser puser;
	PCGroup pcgp;
	int i;//,idx;
	int mfg,ffg=0;
	if ((FDB_GetGroup(id, &s))==NULL) return FDB_ERROR_NODATA;

	//overwrite
	memset(&s, 0, sizeof(TGroup));
	lseek(fdgroup, -1*sizeof(TGroup), SEEK_CUR);
	if (write(fdgroup, (void*)&s, sizeof(TGroup))==sizeof(TGroup))
	{

		fsync(fdgroup);

		if(fdUser)
		{
			lseek(fdUser,0,0);
			puser = (PUser)MALLOC(sizeof(TUser));
			memset(puser, 0, sizeof(TUser));
			while(read(fdUser, puser, sizeof(TUser))==sizeof(TUser))
			{
				if(puser->PIN && puser->Group==id)
				{
					puser->Group=0;
					lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
					if(write(fdUser, puser, sizeof(TUser))!=sizeof(TUser))
						break;
				}
			}
			fsync(fdUser);
			FREE(puser);
		}


		if(fdcgroup)
		{
			lseek(fdcgroup,0,0);
			pcgp = (PCGroup)MALLOC(sizeof(TCGroup));
			memset(pcgp, 0, sizeof(TCGroup));
			while(read(fdcgroup, pcgp, sizeof(TCGroup))==sizeof(TCGroup))
			{
				ffg=0;
				mfg=0;
				for(i=0;i<CGP_MEMBER_MAX;i++)
				{
					if(pcgp->GroupID[i])
					{
						if(pcgp->GroupID[i]==id)
						{
							pcgp->GroupID[i]=0;
							pcgp->MemberCount--;
							mfg++;
						}
						else
							ffg++;
					}
				}
				if(!ffg) memset(pcgp, 0, sizeof(TCGroup));
				if(mfg)
				{
					lseek(fdcgroup, -1*sizeof(TCGroup), SEEK_CUR);
					if(write(fdcgroup, pcgp, sizeof(TCGroup))!=sizeof(TCGroup))
						break;
				}

			}
			fsync(fdcgroup);
			FREE(pcgp);
		}

		return FDB_OK;
	}
	else
		return FDB_ERROR_IO;
}

//lock group
int FDB_CntCGroup(void)
{
	return GetDataInfo(FCT_CGROUP, STAT_COUNT, 0);
}

int FDB_AddCGroup(PCGroup cgroup)
{
	if (FDB_CntCGroup()>=CGP_MAX)
		return FDB_ERROR_NOSPACE;
	return SearchAndSave(FCT_CGROUP, (char*)cgroup, sizeof(TCGroup));

}

static TCGroup gCGp;
PCGroup FDB_GetCGroup(U8 id, PCGroup cgroup)
{
	TSearchHandle sh;

	sh.ContentType=FCT_CGROUP;
	sh.buffer=(char*)&gCGp;

	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PCGroup)sh.buffer)->ID==id)
		{
			if (cgroup)
				memcpy(cgroup, sh.buffer, sizeof(TCGroup));
			return (PCGroup)sh.buffer;
		}
	}
	return NULL;
}

int FDB_ChgCGroup(PCGroup cgroup)
{
	PCGroup s;
	int ret;

	if ((s=FDB_GetCGroup(cgroup->ID, NULL))==NULL) return FDB_ERROR_NODATA;
	if (0==memcmp((void*)cgroup, (void*)s, sizeof(TCGroup))) return FDB_OK;
	//overwrite
	lseek(fdcgroup, -1*sizeof(TCGroup), SEEK_CUR);
	if (write(fdcgroup, (void*)cgroup, sizeof(TCGroup))==sizeof(TCGroup))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdcgroup);

	return ret;
}

int FDB_DelCGroup(U8 id)
{
	TCGroup s;
	int ret;
	if ((FDB_GetCGroup(id, &s))==NULL) return FDB_ERROR_NODATA;

	//overwrite
	memset(&s, 0, sizeof(TCGroup));
	lseek(fdcgroup, -1*sizeof(TCGroup), SEEK_CUR);
	if (write(fdcgroup, (void*)&s, sizeof(TCGroup))==sizeof(TCGroup))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdcgroup);


	return ret;
}

//反潜
int GetUserLastLog(PAlarmRec lastlogs,U16 PIN,int LastLogsCount)
{
	int found;
	int index=SearchInLastLogs(lastlogs, PIN, LastLogsCount, &found);
	if(found)
	{
		return index;
	}
	else
	{
		return -1;
	}
}

//取得所有最近一次用户的记录
int FetchLastLogs(PAlarmRec CurAlarmLog)
{
	TSearchHandle sh;
	int count=0;
	TAttLog log;
	U8 buf[64];

	memset(buf, 0, sizeof(buf));
	sh.ContentType=FCT_ATTLOG;
	sh.buffer=(char *)buf;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
#ifndef ZEM500
		int size=UnpackAttLog((char*)sh.buffer, &log);
		if(size==AttLogSize2) {
			lastt=log.time_second;
		} else {
			log.time_second+=lastt;
		}
#else
		memset(&log, 0, sizeof(TAttLog));
		UnpackNewAttLog((char*)sh.buffer, &log);
#endif
		if(AddToOrderedLastLogs(CurAlarmLog, &log, count)) {
			count++;
		}
	}
	//DBPRINTF("FetchLastLogs=%d\n",count);
	return count;
}

void FDB_InitShortKey(void)
{
	int i;
	char tmpstcode[8][40];
	TSHORTKEY tmpstkey;
	int incnt;
	memset(tmpstcode[0],0,40);
	//tmpstcode[0]=HIT_SYSTEM5KEY1;      //上班签到
	UTF8toLocal(tftlocaleid,(unsigned char*)LoadStrByID(HIT_SYSTEM5KEY1),(unsigned char*)tmpstcode[0]);

	//tmpstcode[1]=HIT_SYSTEM5KEY2;      //下班签退
	memset(tmpstcode[1],0,40);
	UTF8toLocal(tftlocaleid,(unsigned char*)LoadStrByID(HIT_SYSTEM5KEY2),(unsigned char*)tmpstcode[1]);

	//tmpstcode[4]=HIT_SYSTEM5KEY3;      //加班签到
	memset(tmpstcode[4],0,40);
	UTF8toLocal(tftlocaleid,(unsigned char*)LoadStrByID(HIT_SYSTEM5KEY3),(unsigned char*)tmpstcode[4]);

	//tmpstcode[5]=HIT_SYSTEM5KEY4;      //加班签退
	memset(tmpstcode[5],0,40);
	UTF8toLocal(tftlocaleid,(unsigned char*)LoadStrByID(HIT_SYSTEM5KEY4),(unsigned char*)tmpstcode[5]);

	//tmpstcode[2]=HIT_SYSTEM5KEY5;      //外出
	memset(tmpstcode[2],0,40);
	UTF8toLocal(tftlocaleid,(unsigned char*)LoadStrByID(HIT_SYSTEM5KEY5),(unsigned char*)tmpstcode[2]);

	//tmpstcode[3]=HIT_SYSTEM5KEY6;      //外出返回
	memset(tmpstcode[3],0,40);
	UTF8toLocal(tftlocaleid,(unsigned char*)LoadStrByID(HIT_SYSTEM5KEY6),(unsigned char*)tmpstcode[3]);

	//tmpstcode[6]=LoadStrByID(MID_TIMEOFFOUT);	//非上班时间外出
	memset(tmpstcode[6],0,40);
	UTF8toLocal(tftlocaleid,(unsigned char*)LoadStrByID(MID_TIMEOFFOUT),(unsigned char*)tmpstcode[6]);

	//tmpstcode[7]=LoadStrByID(MID_TIMEOFFIN);	//非上班时间进入
	memset(tmpstcode[7],0,40);
	UTF8toLocal(tftlocaleid,(unsigned char*)LoadStrByID(MID_TIMEOFFIN),(unsigned char*)tmpstcode[7]);

	if (SRLFlag)
		incnt = 8;
	else
		incnt = 6;

	if(gOptions.TFTKeyLayout!=3)
	{
		for (i=0;i<STKEY_COUNT;i++)
		{
			memset(&tmpstkey,0,sizeof(TSHORTKEY));
			if (i < incnt)
				FDB_CreateShortKey(&tmpstkey, i+1, 1, i, tmpstcode[i], 0, 0, 0, 0, 0, 0, 0, 0);   	//预设状态键
			else if(i==8 && gOptions.WorkCode)//Liaozz 20081007 when workcode is supported, then init the work short key, fix bug second 53
				FDB_CreateShortKey(&tmpstkey, i+1, 2, -1, NULL, 0, 0, 0, 0, 0, 0, 0, 0);             //预设WorkCode键
			else if(i==9)
				FDB_CreateShortKey(&tmpstkey, i+1, 3, -1, NULL, 0, 0, 0, 0, 0, 0, 0, 0);             //预设查看短消息键
			else
			{
#ifdef FACE
				if(gOptions.FaceFunOn && i==7)
				{
					FDB_CreateShortKey(&tmpstkey, i+1, STK_FUN, -1, LoadStrByID(MID_STKEYFUN6),0,0,0,0,0,0,0,0);
				}
				else
#endif
					FDB_CreateShortKey(&tmpstkey, i+1, 0, -1, NULL, 0, 0, 0, 0, 0, 0, 0, 0);             //未定义
			}

			if(FDB_GetShortKey(i+1, NULL)!=NULL)
				FDB_ChgShortKey(&tmpstkey);
			else
				FDB_AddShortKey(&tmpstkey);
		}
	}
	else
	{
		for(i=0;i<7;i++)
		{
			memset(&tmpstkey,0,sizeof(TSHORTKEY));
			if(i<6)
				FDB_CreateShortKey(&tmpstkey, i+1, 1, i, tmpstcode[i], 0, 0, 0, 0, 0, 0, 0, 0);		//预设状态键
			//zsliu change
			//else if(i==6 && gOptions.WorkCode) // by lyy 20090520
			//        FDB_CreateShortKey(&tmpstkey, i+1, 2, -1, NULL, 0, 0, 0, 0, 0, 0, 0, 0);//预设WorkCode键
			else
				FDB_CreateShortKey(&tmpstkey, i+1, 0, -1, NULL, 0, 0, 0, 0, 0, 0, 0, 0);             //未定义
			if (FDB_GetShortKey(i+1, NULL)!=NULL)
				FDB_ChgShortKey(&tmpstkey);
			else
				FDB_AddShortKey(&tmpstkey);
		}
	}
}

void FDB_InitAlarm(void)
{
	int i;
	ALARM alarm_t;

	for (i=0;i<gOptions.AlarmMaxCount;i++)
	{
		if(FDB_CreateAlarm(&alarm_t, i+1, 0, 0, 0, 60, 0, 3)!=NULL)
		{
			if (FDB_GetAlarm(i+1, NULL)!=NULL)
				FDB_ChgAlarm(&alarm_t);
			else
				FDB_AddAlarm(&alarm_t);
		}
	}
}

void FDB_InitDefaultDoor(void)
{
	int i;
	TTimeZone ttz1;
	TGroup tgp;
	TCGroup tcgp1;

	//默认第一个时间段(00:00～23:59)
	if (FDB_CntTimeZone()==0)
	{
		memset(&ttz1, 0, sizeof(TTimeZone));
		ttz1.ID=1;
		for(i=0;i<7;i++)
		{
			ttz1.ITime[i][TZ_START][TZ_HOUR]=0;
			ttz1.ITime[i][TZ_START][TZ_MINUTE]=0;
			ttz1.ITime[i][TZ_END][TZ_HOUR]=23;
			ttz1.ITime[i][TZ_END][TZ_MINUTE]=59;
		}
		FDB_AddTimeZone(&ttz1);
	}

	//默认第一个组，组时间段为1
	if(FDB_CntGroup()==0)
	{
		memset(&tgp, 0, sizeof(TGroup));
		tgp.ID=1;
		tgp.TZID[0]=1;
		FDB_AddGroup(&tgp);
	}

	if(FDB_CntCGroup()==0)
	{
		memset(&tcgp1, 0, sizeof(TCGroup));
		tcgp1.ID=1;
		tcgp1.GroupID[0]=1;
		tcgp1.MemberCount=1;
		FDB_AddCGroup(&tcgp1);
	}
}

//photo index===================================
//FCT_PHOTOINDEX, fdpicidx
static TPhotoIndex gTPI;
PPhotoList FDB_GetPhotoToList(time_t sttime, time_t edtime, char* userid, int bhour, int *pcount, int type)
{
	TSearchHandle sh;
	time_t curtime;
	TTime tmptime;
	PPhotoList mlist = NULL;
	PPhotoList ppre = NULL;
	PPhotoList pNode = NULL;
	char namestr[80];

	*pcount = 0;
	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&gTPI;
	SearchFirst(&sh);

	//	printf("start time:%d, endtime:%d, user name:%s, type:%d\n", sttime, edtime, userid, type);
	while (!SearchNext(&sh))
	{
		if (((PhotoIndex)sh.buffer)->index && ((PhotoIndex)sh.buffer)->pictype==type)		//记录有效
		{
			OldDecodeTime(((PhotoIndex)sh.buffer)->createtime, &tmptime);
			//if (!bhour) tmptime.tm_hour = 0;	//是否需要精确到小时
			//tmptime.tm_min = 0;
			//tmptime.tm_sec = 0;
			tmptime.tm_isdst = -1;
			curtime = OldEncodeTime(&tmptime);

			//不符合查询条件
			if (((sttime!=0 && curtime<sttime) || (edtime!=0 && curtime>edtime)) ||
					(type==0 && (userid && strlen(userid)!=0 &&
						     strncmp(((PhotoIndex)sh.buffer)->userid, userid, MAXNAMELENGTH)!=0)))
			{
				continue;
			}

			memset(namestr, 0, 40);
			OldDecodeTime(((PhotoIndex)sh.buffer)->createtime, &tmptime);
			if (type==0)
			{
				snprintf(namestr, sizeof(namestr), "%04d%02d%02d%02d%02d%02d-%s", tmptime.tm_year+1900, tmptime.tm_mon+1,
						tmptime.tm_mday, tmptime.tm_hour, tmptime.tm_min, tmptime.tm_sec, ((PhotoIndex)sh.buffer)->userid);
			}
			else
			{
				snprintf(namestr, sizeof(namestr), "%04d%02d%02d%02d%02d%02d", tmptime.tm_year+1900, tmptime.tm_mon+1,
						tmptime.tm_mday, tmptime.tm_hour, tmptime.tm_min, tmptime.tm_sec);
			}

			pNode = (PPhotoList)MALLOC(sizeof(TPhotoList));
			if (pNode != NULL)
			{
				memset(pNode, 0, sizeof(TPhotoList));
				pNode->index = ((PhotoIndex)sh.buffer)->index;
				memcpy(pNode->PhotoFile, namestr, strlen(namestr));
				if (mlist==NULL) mlist=pNode;
				if (ppre==NULL) ppre=pNode;
				pNode->next = mlist;
				pNode->pre = ppre;
				ppre->next = pNode;
				mlist->pre = pNode;
				ppre = pNode;
				(*pcount)++;
			}
		}
	}
	pNode = NULL;
	ppre = NULL;

	return mlist;
}

PhotoIndex FDB_GetPhotoIndex(int index, PhotoIndex pidx)
{
	TSearchHandle sh;

	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&gTPI;
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		if (((PhotoIndex)sh.buffer)->index == index)
		{
			if (pidx)
				memcpy(pidx, sh.buffer, sizeof(TPhotoIndex));
			return (PhotoIndex)sh.buffer;
		}
	}
	return NULL;
}

U32 FDB_GetMaxPhotoIndex(void)
{
	U32 maxindex=0;

	TSearchHandle sh;
	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&gTPI;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if (((PhotoIndex)sh.buffer)->index > maxindex)
			maxindex = ((PhotoIndex)sh.buffer)->index;
	}
	return maxindex;
}

int FDB_DelPhotoID(int index)
{
	TPhotoID s;
	if (FDB_GetPhotoID(index, &s) == NULL) 
	{
		return FDB_ERROR_NODATA;
	} 
	memset(&s, 0, sizeof(TPhotoID));
	lseek(fdpicid, -1*sizeof(TPhotoID), SEEK_CUR);
	if (write(fdpicid, (void*)&s, sizeof(TPhotoID)) == sizeof(TPhotoID))
	{
		fsync(fdpicid);
		return FDB_OK;
	}
	else
	{
		return FDB_ERROR_IO;
	}
}

int FDB_ClearIndex(int type)
{
	TSearchHandle sh;
	TPhotoIndex s;
	int ret = FDB_OK;

	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&gTPI;
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		if (((PhotoIndex)sh.buffer)->index && ((PhotoIndex)sh.buffer)->pictype == type)
		{
			memset(&s, 0, sizeof(TPhotoIndex));
			lseek(fdpicidx, -1*sizeof(TPhotoIndex), SEEK_CUR);
			if (write(fdpicidx, (void*)&s, sizeof(TPhotoIndex)) != sizeof(TPhotoIndex))
			{
				ret = FDB_ERROR_IO;
				break;
			}
			ret = FDB_DelPhotoID(((PhotoIndex)sh.buffer)->index);
			if (ret == FDB_ERROR_IO)
			{
				break;
			}
		}
	}

	fsync(fdpicidx);
	return ret;
}

int FDB_AddPhotoIndex(PhotoIndex pidx)
{
	return SearchAndSave(FCT_PHOTOINDEX, (char*)pidx, sizeof(TPhotoIndex));
}

int FDB_DelPhotoIndex(int index)
{
	TPhotoIndex s;
	int ret;

	if (FDB_GetPhotoIndex(index, &s)==NULL) return FDB_ERROR_NODATA;
	memset(&s, 0, sizeof(TPhotoIndex));
	lseek(fdpicidx, -1*sizeof(TPhotoIndex), SEEK_CUR);
	if (write(fdpicidx, (void*)&s, sizeof(TPhotoIndex)) == sizeof(TPhotoIndex))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;

	fsync(fdpicidx);
	return ret;
}

void addcode(U16 code,U16 value)
{
	TCodeConvert mycodevert;

	memset(&mycodevert,0,sizeof(TCodeConvert));
	mycodevert.code=code;
	mycodevert.value=value;
	lseek(fdbig5code,0,SEEK_END);
	write(fdbig5code,&mycodevert,sizeof(TCodeConvert));
	fsync(fdbig5code);
	printf("%d=%d\n",code,value);
}

int GetUserCntAndPIN(int *pCacheUser,int *UserCount)
{
	TSearchHandle sh;
	int cuser=0;

	sh.ContentType=FCT_USER;
	sh.buffer=( char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(gUser.PIN>0)
		{
			pCacheUser[cuser]=gUser.PIN;
			cuser++;
		}
	}
	(*UserCount)=cuser;
	return *UserCount;

}

BOOL ProcessDownload(char *dstFileName,int flag)
{
	int dstHandle=-1;
	int i, UserCount=0;
	int lengthTmp=0;
	int UserIDs[gOptions.MaxUserCount*1000];
	TZKFPTemplate TTmp;

	//add by caona for IsOnlyRFMachine=1
	if(!fhdl && gOptions.ZKFPVersion == ZKFPV10)
		return 0;

	dstHandle=open(dstFileName, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if (dstHandle==-1) return FALSE;

	memset(UserIDs,0,sizeof(UserIDs));
	if(flag==ZKFPV10_2_FORMAT) //FP10.01
	{
		BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERCOUNT,&UserCount);
		BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERIDS,UserIDs);
		printf("%s:get user finger count:%d\n",__FUNCTION__,UserCount);

		for (i=0;i<UserCount;i++)
		{
			memset(&TTmp,0,sizeof(TZKFPTemplate));
			lengthTmp=FDB_GetTmp(UserIDs[i]&0xffff,(UserIDs[i]>>16)&0xf,(char *)&TTmp);
			if(lengthTmp<=0)
				continue;

			//printf("dddddd  down new:  pin=%d,fpid=%d,size=%d\n",TTmp.PIN,TTmp.FingerID,TTmp.Size);
			TTmp.Size+=6;
			if (write(dstHandle, (char *)&TTmp, 6)!=6 || write(dstHandle,TTmp.Template,TTmp.Size-6)!= (TTmp.Size-6))
			{
				close(dstHandle);
				return FALSE;
			}
		}
	}
	else
	{
		GetUserCntAndPIN(UserIDs,&UserCount);
		for (i=0;i<UserCount;i++)
		{
			memset(&TTmp,0,sizeof(TZKFPTemplate));
			lengthTmp=FDB_GetTmp(UserIDs[i],ZKFPV10_FINGER_INDEX,(char *)&TTmp);
			if(lengthTmp<=0)
				continue;

			//printf("dddddd  down merge:  pin=%d,fpid=%d,size=%d\n",TTmp.PIN,TTmp.FingerID,TTmp.Size);
			TTmp.Size+=6;
			if (write(dstHandle, (char *)&TTmp, 6)!=6 || write(dstHandle,TTmp.Template,TTmp.Size-6)!= (TTmp.Size-6))
			{
				close(dstHandle);
				return FALSE;
			}
		}
	}

	fsync(dstHandle);
	close(dstHandle);
	return TRUE;
}


int ProcessGetFingerprintCount(int pin)
{
	int TmpCount=0;
	if(!fhdl) //add by caona for IsOnlyRFMachine=1
		return 0;

	if (pin == 0)
	{
		BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_FINGERCOUNT,&TmpCount);
	}
	else
	{
		U32 fingerCount=0,i;
		for(i=0;i<MAX_USER_FINGER_COUNT;i++)
		{
			fingerCount = pin | (i<<16);//[in,out]:in pin,out user finger count
			BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERFINGERCOUNT,(int*)&fingerCount);
			TmpCount +=fingerCount;
		}
	}
	return TmpCount;
}
/*
   1.scan user.dat
   2.
   */
U16* ProcessUploadUDiskUser(char *InUsersData,int InUsersDataLen, U16 *AddUserPIN, int *returnValue, int *AdminCount)
{
	PUser pInUser = NULL;
	PUser pUser = NULL;
	int inUserCount;
	int ret = 0;
	int pin = 0;
	int i = 0;
	int igroupCnt = 20;
	int iUserCnt = 0;
	TSearchHandle sh;
	U16 *pAdminBuf = NULL;
	off_t FileUserOffset = 0;

	//process the exists user from udisk
	inUserCount = InUsersDataLen/sizeof(TUser);
	pAdminBuf = (U16*)malloc(igroupCnt*sizeof(U16));
	*AdminCount=0;
	if (pAdminBuf == NULL) {
		return NULL;
	}

	iUserCnt = FDB_CntUserByMap();

	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		pUser = (PUser)sh.buffer;
		for(i=0; i<inUserCount; i++)
		{
			pInUser = (PUser)InUsersData+i;
			if(strcmp(pUser->PIN2,pInUser->PIN2)==0)
			{
				if(0==memcmp((void*)pUser, (void*)pInUser, sizeof(TUser))) {
					AddUserPIN[pInUser->PIN] = pUser->PIN;
				} else {
					if(pUser->PIN==0) {
						pUser->PIN=GetNextPIN(1, TRUE);
					}
					AddUserPIN[pInUser->PIN] = pUser->PIN;

					if(pAdminBuf && (pInUser->Privilege>0)&&!(pInUser->Password[0])&&!(pInUser->Card[0])) {
						if (*AdminCount >= igroupCnt) {
							igroupCnt+=20;
							pAdminBuf=realloc(pAdminBuf,igroupCnt*sizeof(U16));
						}
						pAdminBuf[*AdminCount]=pInUser->PIN;
						*AdminCount=*AdminCount+1;
					}
					pInUser->PIN = pUser->PIN;

					//overwrite user 
					lseek(fdUser, -1*sizeof(TUser), SEEK_CUR);
					ret =((write(fdUser, (void*)pInUser, sizeof(TUser))==sizeof(TUser))?FDB_OK:FDB_ERROR_IO);
					if (ret==FDB_OK) {
						SetBit((BYTE*)UserIDMap, pInUser->PIN);
					}
				}
				pInUser->PIN = 0; //mark as processed
				break;
			}
		}
	}


	//process the new user from udisk;
	int iMaxUserCnt=gOptions.MaxUserCount*100;
	pin = 0;
	if (iUserCnt == 0) {
		FileUserOffset = -1;
	}

	for (i=0; i<inUserCount; i++)
	{
		pInUser = (PUser)InUsersData + i;
		if (pInUser->PIN > 0) {
			if (iUserCnt >= iMaxUserCnt) {
				ret = FDB_OVER_ULIMIT;
				break;
			}
			
			 /*When the length of pin2 is more then gOptions.PIN2Width, ignoring upload!*/
			 /*2012-11-14*/
			if(pInUser->PIN2 && strlen(pInUser->PIN2) > gOptions.PIN2Width) {
			    continue;
			}


			if (iUserCnt > 0) {
				pin =  GetNextPIN(pin+1, TRUE);
			} else {
				pin++;
			}
			AddUserPIN[pInUser->PIN] = pin;
			if (pAdminBuf && (pInUser->Privilege > 0) && !(pInUser->Password[0]) && !(pInUser->Card[0])) {
				if (*AdminCount >= igroupCnt) {
					igroupCnt+=20;
					pAdminBuf=realloc(pAdminBuf,igroupCnt*sizeof(U16));
				}
				pAdminBuf[*AdminCount]=pInUser->PIN;
				*AdminCount=*AdminCount+1;
			}
			pInUser->PIN = pin;
			ret = FDB_AddUserFromUsb(pInUser, &FileUserOffset);			
			if (ret == FDB_OK) {
				iUserCnt++;
			}	            
		}
	}
	*returnValue=ret;
	return pAdminBuf;
}

//add by zhc 2008.11.24
int checkfilestatus(const char* filename)
{
	struct stat statbuf;
	char mtdblockbuf[48];
	char databuf[48];
	//int size;
	printf("filename: %s\n",filename);
	if(stat(filename,&statbuf)==-1)	{
		sprintf(mtdblockbuf,"/mnt/mtdblock/%s",filename);
		if(stat(mtdblockbuf,&statbuf)==-1) {
			sprintf(databuf,"/mnt/mtdblock/data/%s",filename);
			//DBPRINTF("no1-----error at stat file(checkfilestatus) %s\n",mtdblockbuf);
			if(stat(databuf,&statbuf)==-1) {
				return -1;
			} else {
				strcpy((char *)filename,databuf);
				DBPRINTF("checkfilestatus->databuf: %s\n",filename);
			}
		} else {
			strcpy((char *)filename,mtdblockbuf);
			DBPRINTF("checkfilestatus-> mtdblockbuf: %s\n",filename);
		}
	}

	if(S_ISREG(statbuf.st_mode)) {
		DBPRINTF("file(at checkfilestatus) size is %ld\n",statbuf.st_size);
		return statbuf.st_size;
	} else {
		return -1;
	}
}

int readfile(const char* filename,char *buf)
{
	struct stat statbuf;
	int fd,read_size;
	printf("readfile-> filename: %s\n",filename);
	fd=open(filename, O_RDONLY);
	printf("fd: %d\n",fd);
	stat(filename,&statbuf);
	read_size=read(fd,buf,statbuf.st_size);
	DBPRINTF("(readfile)read file %s success size %d\n ",filename,read_size);
	close(fd);
	return read_size;
}

//end add by zhc

TPhotoID gpicid;
PPhotoID FDB_GetPhotoID(int index, PPhotoID picid)
{
	TSearchHandle sh;

	sh.ContentType = FCT_PHOTOID;
	sh.buffer = (char*)&gpicid;
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		if ((((PPhotoID)sh.buffer)->index == index))
		{
			if (picid)
				memcpy(picid, sh.buffer, sizeof(TPhotoID));
			return (PPhotoID)sh.buffer;
		}
	}
	return NULL;
}

int FDB_ClrPhoto(void)
{
	char buf[100];


	sprintf(buf,DELALLPASSPIC,GetCapturePath(""),GetCapturePath(""));
	systemEx(buf);
	sprintf(buf,DELALLBADPIC,GetCapturePath(""),GetCapturePath(""));
	systemEx(buf);
	FDB_ClearData(FCT_PHOTOINDEX);
	sync();
	picid=0;
	return 1;
}


int FDB_CntAttLog2(void)
{
	int fd=SelectFDFromConentType(FCT_ATTLOG);
	int pos=lseek(fd, 0, SEEK_CUR);
	int lst=lseek(fd, 0, SEEK_END);
#ifdef SSRDB
	int ret=lst/sizeof(TAttLog);
#else


	int ret=lst/8;
	if(gOptions.AttLogExtendFormat)
		ret=lst/16;
#endif
	lseek(fd, pos, SEEK_SET);
	return ret;
}



int FDB_AddPhotoID(PPhotoID picid)
{
	return SearchAndSave(FCT_PHOTOID, (char*)picid, sizeof(TPhotoID));
}

int FDB_CntPhoto(void)
{
	return GetDataInfo(FCT_PHOTOID, STAT_COUNT, 0);
}

//Upgrade ZKFPV10 DB first version to second version
int FDB_UpgradeTmpV10DB()
{
	//int dstHandle=-1;
	int i, UserCount=0;
	int lengthTmp=0;
	int UserIDs[gOptions.MaxUserCount*1000];
	TZKFPTemplate tmp;

	//add by caona for IsOnlyRFMachine=1
	if(!fhdl && gOptions.ZKFPVersion == ZKFPV10)
		return 0;
	memset(UserIDs,0,sizeof(UserIDs));
	BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERCOUNT,&UserCount);
	BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERIDS,UserIDs);
	printf("%s:get user finger count:%d\n",__FUNCTION__,UserCount);
	if(UserCount<=0)
	{
		return TRUE;
	}
	for (i=0;i<UserCount;i++)
	{
		memset(&tmp,0,sizeof(TZKFPTemplate));
		lengthTmp=FDB_GetTmp(UserIDs[i]&0xffff,(UserIDs[i]>>16)&0xf,(char *)&tmp);
		if(lengthTmp<=0) 
			continue;
		tmp.FingerID = ZKFPV10_FINGER_INDEX;
		FDB_AddTmpV10(&tmp);
	}
	return TRUE;
}

char *GetTmpsV10BlockData(int *Size)
{
	//int dstHandle=-1;
	int i, fingerCount=0;
	int lengthTmp=0;
	int UserIDs[gOptions.MaxUserCount*1000];
	TZKFPTemplate tmp;
	BYTE *blockBuf;
	int validLen=0;

	//add by caona for IsOnlyRFMachine=1
	if(!fhdl && gOptions.ZKFPVersion == ZKFPV10)
		return 0;
	memset(UserIDs,0,sizeof(UserIDs));
	BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERCOUNT,&fingerCount);
	BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERIDS,UserIDs);
	//printf("get user finger count:%d\n",fingerCount);
	if(fingerCount>0)
	{
		blockBuf = (BYTE*)MALLOC(fingerCount*ZKFPV10_MAX_ONE_FP_LEN+4);
		if(!blockBuf)
		{
			return NULL;
		}
		*Size = 4;
	}else
	{
		*Size = 0;
		return NULL;
	}
	for (i=0;i<fingerCount;i++)
	{
		memset(&tmp,0,sizeof(TZKFPTemplate));
		lengthTmp=FDB_GetTmp(UserIDs[i]&0xffff,(UserIDs[i]>>16)&0xf,(char *)&tmp);
		if(lengthTmp<=0)
			continue;
		tmp.Size+=6;
		memcpy(blockBuf+*Size,(char*)&tmp,6);
		*Size +=6;
		memcpy(blockBuf+*Size,tmp.Template,tmp.Size-6);
		*Size += tmp.Size-6;
	}
	validLen = *Size -4;
	memcpy(blockBuf,&validLen,4);
	return (char*)blockBuf;
}

void clearSystemCaches()
{
#ifdef ZEM600
	//system("/bin/echo 2 >>/proc/sys/vm/drop_caches");
#endif
}

int fileCp(const char *destFileName,const char *srcFileName)
{
	int size;
	FILE *srcfp,*destfp;
	char buff[4096];
	int result=0;

	if ((srcfp=fopen(srcFileName,"r")) == NULL)
	{
		return -1;
	}
	if ((destfp=fopen(destFileName,"wb")) == NULL)
	{
		fclose(srcfp);
		return -2;
	}
	while ((size=fread(buff,sizeof(char),sizeof(buff),srcfp)) > 0)
	{
		if (fwrite(buff,sizeof(char),size,destfp) != size)
		{
			result = -3;
			break;	
		}
	}
	fsync(fileno(destfp));
	fclose(srcfp);
	fclose(destfp);

	return result;
}

/*异地考勤,add by yangxiaolong,2011-06-14,start*/
int   FDB_GetUserDLTime(U16 pin, PUserDLTime  PUserDLTimelog)
{
	TSearchHandle sh;
	TUserDLTime	UserDLTimelog;

	sh.ContentType = FCT_USER_DL_TIME;
	sh.buffer=(char*)&UserDLTimelog;

	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		if(((PUserDLTime)sh.buffer)->m_pin == pin)
		{
			if (PUserDLTimelog != NULL)
			{
				memcpy(PUserDLTimelog, sh.buffer, sizeof(TUserDLTime));
			}
			return FDB_OK;	
		}
	}
	return FDB_ERROR_NODATA;
}


int FDB_AddUserDLTime(PUserDLTime  PUserDLTimelog)
{
	if(SearchAndSave(FCT_USER_DL_TIME, (char*)PUserDLTimelog, sizeof(TUserDLTime)) == FDB_OK)
	{
		return FDB_OK;
	}
	else
	{
		return FDB_ERROR_IO;
	}
}

int FDB_DelUserDLTime(U16 pin)
{
	TUserDLTime  UserDLTimelog;

	if ((FDB_GetUserDLTime(pin, NULL)) != FDB_OK) 
	{
		return FDB_ERROR_NODATA;
	}

	memset((void *)&UserDLTimelog, 0, sizeof(TUserDLTime));

	//overwrite 
	lseek(fdUserDLTime, -1*sizeof(TUserDLTime), SEEK_CUR);
	if (write(fdUserDLTime, (void*)&UserDLTimelog, sizeof(TUserDLTime)) == sizeof(TUserDLTime))
	{
		return FDB_OK;
	}

	return FDB_ERROR_IO;
}


//dsl 2012.3.23 for download last attendence record
int SaveAttLogOffset(int offset)
{
	SaveInteger("AttOffset", offset);
	return 1;
}

int ReadAttLogOffset(void)
{
	int icnt;
	icnt=LoadInteger("AttOffset", 0);
	if (icnt > CurAttLogCount)
	{
		icnt=CurAttLogCount;
		SaveAttLogOffset(icnt);
	}
	return icnt;
}

void SearchAttFirst(PSearchHandle sh)
{
	int offset=0;
	offset=sizeof(TAttLog);
	sh->fd=SelectFDFromConentType(sh->ContentType);
	lseek(sh->fd, offset*ReadAttLogOffset(), SEEK_SET);
	sh->bufferlen=0;
	sh->datalen=0;  //valid data length
}

int GetAttDataInfo(void)
{
	int tmp=0;
	char buf[48];
	TSearchHandle sh;

	memset(buf,0,sizeof(buf));
	sh.ContentType=FCT_ATTLOG;
	sh.buffer=buf;

	SearchAttFirst(&sh);
	while(!SearchNext(&sh))
	{
		tmp+=sh.datalen;
	}
	return tmp;
}

char* FDB_ReadAttBlock(int *size, int ContentType)
{
	TSearchHandle sh;
	U8 buf[1024];
	int validLen, iOffset=0;
	U8 *validBuf;
	U8 *p;

	validLen=GetAttDataInfo();
	*size=validLen+4;
	if (validLen>0)
	{
		validBuf=malloc(validLen+4);
		memcpy(validBuf, &validLen, 4);
		p=validBuf+4;

		sh.ContentType=ContentType;
		sh.buffer=(char *)buf;
		SearchAttFirst(&sh);
		while(!SearchNext(&sh))
		{
			if(sh.datalen>0)
			{
				memcpy(p, sh.buffer, sh.datalen);
				p+=sh.datalen;
			}
			iOffset++;
		}
		if (iOffset > 0)
		{
			SaveAttLogOffset(iOffset+ReadAttLogOffset());
		}
		return (char *)validBuf;
	}
	else
	{
		return NULL;
	}
}
//end //dsl 2012.3.23 for download last attendence record

int FDB_DelOldAttPicture(int num)
{
	TSearchHandle sh;
	TPhotoIndex pic;
	TTime tmptime;
	int cnt = 0;
	int offset = 0;
	char filename0[128];
	char filename1[128];

	memset((void*)&pic, 0, sizeof(TPhotoIndex));
	memset((void*)filename0, 0, sizeof(filename0));
	memset((void*)filename1, 0, sizeof(filename1));

	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&pic;
	SearchFirst(&sh);
	while ((cnt < num) && !SearchNext(&sh)) {
		if (pic.index > 0) {
			OldDecodeTime(pic.createtime, &tmptime);
			sprintf(filename0, "%4d%02d%02d%02d%02d%02d", tmptime.tm_year + 1900, tmptime.tm_mon + 1, tmptime.tm_mday,
					tmptime.tm_hour, tmptime.tm_min, tmptime.tm_sec);
			memset((void*)filename1, 0, sizeof(filename1));
			if (pic.pictype==0) {
				sprintf(filename1, "%s/pass/%s-%s.jpg", GetCapturePath("capture"), filename0,pic.userid);
			} else {
				sprintf(filename1, "%s/bad/%s.jpg", GetCapturePath("capture"), filename0);
			}
			//printf("file:%s\n", filename1);
			remove(filename1);

			/*clear picture information in file*/
			memset((void*)&pic, 0, sizeof(TPhotoIndex));
			lseek(sh.fd, -1*sizeof(TPhotoIndex), SEEK_CUR);
			if (write(sh.fd, (void*)&pic, sizeof(TPhotoIndex)) != sizeof(TPhotoIndex)) {
				break;
			}
			cnt++;
		}
		offset+=sh.bufferlen;
	}
	fsync(sh.fd);

	if (cnt > 0) {
		if (delFileHead(sh.fd, offset) > 0) {
			return num;
		}
	}
	return 0;
}

void write_tracemsg(const char *hint)
{
	TTime t;
	char buffer[256];
	int fd=-1;

	memset(buffer, 0, sizeof(buffer));
	fd = open(GetEnvFilePath("USERDATAPATH", "data/runlog.dat", buffer),O_RDWR|O_CREAT|O_SYNC,S_IRWXU|S_IRWXG|S_IRWXO);
	if (fd < 0) {
		return;
	}

	GetTime(&t);
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d  %s\n", t.tm_year+1900,
			t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, hint);
	lseek(fd, 0, SEEK_END);
	write(fd, buffer, strlen(buffer));
	fsync(fd);
	sync();
	close(fd);
}

#if 0
void test_add_user(void)
{
	TUser user;
	int pin;
	int i;
	char name[24]={0};

	
	for (i=0; i<gOptions.MaxUserCount*100; i++) {
		pin = GetNextPIN(1, 1);
		sprintf(name, "%d", pin);
		printf("pin=%d ", pin);
		FDB_CreateUser(&user, pin, name, NULL, 0);
		FDB_AddUser(&user);
	}
}
#endif

// int PhotoFilter(int time)//检查照片是否已上传
//{
//	if(time < gPhotoStamp){
//		return 0;
//	} else {
//		return -1;
//	}
//	return 0;
//}

int FDB_DelPhotoCur(PhotoIndex photo)
{
	TPhotoIndex s;
	int ret;
	TTime timephoto;
	char photoPath[80]={0};
	OldDecodeTime(photo->createtime,&timephoto);
	if(photo->pictype == 0){
		snprintf(photoPath, sizeof(photoPath), "%s/%04d%02d%02d%02d%02d%02d-%s.jpg",GetCapturePath("capture/pass"),
			timephoto.tm_year+1900, timephoto.tm_mon+1,timephoto.tm_mday, timephoto.tm_hour, 
			timephoto.tm_min, timephoto.tm_sec, photo->userid);
	} else {
		snprintf(photoPath, sizeof(photoPath), "%s/%04d%02d%02d%02d%02d%02d.jpg",GetCapturePath("capture/bad"),
			timephoto.tm_year+1900, timephoto.tm_mon+1,timephoto.tm_mday, timephoto.tm_hour, 
			timephoto.tm_min, timephoto.tm_sec);
	}

	if (remove(photoPath) == 0)
	{
		//printf("delete photo ____%s___ success\n",photoPath);
	}
	else
	{
		printf("delete photo ____%s___ fail\n",photoPath);
	}

	memset(&s, 0, sizeof(TPhotoIndex));
	lseek(fdpicidx, -1*sizeof(TPhotoIndex), SEEK_CUR);
	if (write(fdpicidx, (void*)&s, sizeof(TPhotoIndex)) == sizeof(TPhotoIndex))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;
	//lseek(fdpicidx, 1*sizeof(TPhotoIndex), SEEK_CUR);
	fsync(fdpicidx);
	return ret;
}

int FDB_DelPhoto_By_Time(int timedot)
{
	int delcount=0;
	int photocnt;
	char capturepath[80]={0};
	TSearchHandle sh;
	photocnt = 0;
	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&gTPI;
	
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		photocnt++;
		if(((PhotoIndex)sh.buffer)->createtime <= timedot){
			FDB_DelPhotoCur((PhotoIndex)sh.buffer);
			delcount++;
		}
		else
		{
			printf("_______photo not filter____\n");
		}
        }
	if(delcount >= photocnt)
	{
		//printf("_______delete all photos____\n");
		close(fdpicidx);
		snprintf(capturepath, sizeof(capturepath), "%spicindex.dat",GetCapturePath("capture/"));
		remove(capturepath);
		fdpicidx = open(capturepath, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	}
	else if(delcount > 0){
		delFileHead(fdpicidx,delcount*sizeof(TPhotoIndex));
		pushsdk_data_reset(FCT_PHOTOINDEX);
	}
	return 0;
}

int FDB_DelPhoto_By_Count(int count)
{
	int delcount=0;
	TSearchHandle sh;
	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&gTPI;
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		FDB_DelPhotoCur((PhotoIndex)sh.buffer);
		delcount++;
		if(delcount >= count)
		{
			break;
		}
        }
	
	if(delcount > 0){
		delFileHead(fdpicidx,delcount*sizeof(TPhotoIndex));
		pushsdk_data_reset(FCT_PHOTOINDEX);
	}
	return 0;
}

PUserlb GetUsermestolb_PageForward(int deptid, int *cnt, int smsPin)
{

	PUserlb  head,pb,pf;
	PUser puser;
	TUData udata;
	int tmpindex = 0;
	head = pb = pf = NULL;
	int i = 0;
	puser = malloc(sizeof(TUser));
	memset(puser,0,sizeof(TUser));
	if (fdUser < 0)
		return NULL;
	lseek(fdUser,0, SEEK_SET);
	if(userdatacuroffset_e >= lseek(fdUser, 0, SEEK_END)){
		return NULL;
	}
	
	lseek(fdUser,userdatacuroffset_e, SEEK_SET);
	tmpindex = userdatacuroffset_s;
	userdatacuroffset_s = userdatacuroffset_e;
	//printf("_____%s,%d,userdatacuroffset_s=%d\n", __FILE__, __LINE__, userdatacuroffset_e);

	while (read(fdUser,puser,sizeof(TUser))==sizeof(TUser))
	{
		if (puser->PIN >0)
		{
			/*增加短消息处理*/
			if(smsPin > 0 ) {
				memset(&udata,0,sizeof(TUData));
				if(FDB_GetUData(puser->PIN,&udata) != NULL) {
					if(udata.SmsID != smsPin) {
						continue;
					}
				}
			}
			pb = (PUserlb)malloc(sizeof(TUserlb));
			memset(pb,0,sizeof(TUserlb));

			if (deptid == 0)
				memcpy(&pb->userlb,puser,sizeof(TUser));
			else
			{
				if ((U8)deptid == puser->Group)
					memcpy(&pb->userlb,puser,sizeof(TUser));
			}
			
			if (pb->userlb.PIN > 0)
			{
				if(i >= GRIDVIEWROWCNT){
					break;
				}
				if (i==0)
					pf=head=pb;
				else
					pf->next=pb;
				pb->next=NULL;
				pf=pb;
				i++;
				userdatacuroffset_e = lseek(fdUser, 0, SEEK_CUR);
			}
		}

	}
	//userdatacuroffset_e = lseek(fdUser, 0, SEEK_CUR);
	*cnt = i;
	if(i == 0) {
		userdatacuroffset_s = tmpindex;
	}
	free(puser);
	if (head == NULL)
		return NULL;
	else
	{
		if (gOptions.PIN2Width <= 5)
			head =SortUserlb(head);
		return head;
	}
}

PUserlb GetUsermestolb_PageBackward(int deptid, int *cnt, int smsPin)
{

	PUserlb  head,pb,pf;
	TUser puser;
	head = pb = pf = NULL;
	int i = 0;
	int offsettmp=0;
	TUData udata;
	int tmpindex = 0;
	memset(&puser,0,sizeof(TUser));
	if ((fdUser < 0) || (userdatacuroffset_s < sizeof(TUser)))
		return NULL;
	lseek(fdUser,userdatacuroffset_s, SEEK_SET);
	tmpindex = userdatacuroffset_e;
	userdatacuroffset_e = userdatacuroffset_s;
	userdatacuroffset_s = lseek(fdUser, -sizeof(TUser), SEEK_CUR);
	//printf("_____%s,%d,userdatacuroffset_s=%d\n", __FILE__, __LINE__, userdatacuroffset_s);
	
	while (read(fdUser,&puser,sizeof(TUser))==sizeof(TUser))
	{
		//printf("_____%s,%d,puser->PIN=%d,puser->PIN2=%s\n", __FILE__, __LINE__, puser.PIN, puser.PIN2);
		if (puser.PIN >0)
		{
			/*增加短消息处理*/
			if(smsPin > 0 ) {
				memset(&udata,0,sizeof(TUData));
				if(FDB_GetUData(puser.PIN,&udata) != NULL) {
					if(udata.SmsID != smsPin) {
						if(lseek(fdUser, -sizeof(TUser)*2, SEEK_CUR) < 0) {
							break;
						}
						continue;
					}
				}
			}
			//printf("puser->pin2=%s\n", puser.PIN2);
			pb = (PUserlb)malloc(sizeof(TUserlb));
			memset(pb,0,sizeof(TUserlb));

			if (deptid == 0)
				memcpy(&pb->userlb,&puser,sizeof(TUser));
			else
			{
				if ((U8)deptid == puser.Group)
					memcpy(&pb->userlb,&puser,sizeof(TUser));
			}
			
			if (pb->userlb.PIN > 0)
			{
				if (i==0)
					pf=head=pb;
				else
					pf->next=pb;
				pb->next=NULL;
				pf=pb;
				i++;
				offsettmp= lseek(fdUser, -sizeof(TUser), SEEK_CUR);
				userdatacuroffset_s = offsettmp;
				if(i >= GRIDVIEWROWCNT){
					break;
				}
		
				offsettmp= lseek(fdUser, -sizeof(TUser), SEEK_CUR);
				if(offsettmp >= 0){
					userdatacuroffset_s = offsettmp;
				}
			}
			
		}
		else {
			if(lseek(fdUser, -sizeof(TUser)*2, SEEK_CUR) < 0 ) {
				break;
			}
		}

	}
	*cnt = i;
	if(i == 0) {
		userdatacuroffset_e = tmpindex;
	}
	
	if (head == NULL)
		return NULL;
	else
	{
		if (gOptions.PIN2Width <= 5)
			head =SortUserlb(head);
		return head;
	}
}

int FindUser_NameOrAcno(char *name, char *pin2, TUser *users, int *cnt)
{
	TUser tuser;
	int i = 0;
	int offsettmp = 0;
	int findflag = 0;
	memset(&tuser,0,sizeof(TUser));
	if (fdUser < 0)
		return 0;
	offsettmp = lseek(fdUser,0, SEEK_SET);
	//printf("______%s%d\n", __FILE__, __LINE__);
	while (read(fdUser,&tuser,sizeof(TUser))==sizeof(TUser))
	{
		if (tuser.PIN >0)
		{
			if(i >= GRIDVIEWROWCNT){
				offsettmp = lseek(fdUser, 0, SEEK_CUR);
				offsettmp -= sizeof(TUser);
				i = 0;
			}
			//printf("tuser.PIN2=%s\n", tuser.PIN2);
			memset(&users[i], 0, sizeof(TUser));
			memcpy(&users[i], &tuser, sizeof(TUser));

			if(strlen(name) <= 0){
				if(strcmp(tuser.PIN2, pin2) == 0)
					findflag = 1;
				else
					findflag = 0;
			} else {
				if(strcmp(tuser.PIN2, pin2) == 0 || strcmp(tuser.Name, name) == 0)
					findflag = 1;
				else
					findflag = 0;
			}
			
			if(findflag == 1)
			{
				userrowindex = i;
				userdatacuroffset_e = lseek(fdUser, 0, SEEK_CUR);
				i++;
				for(; i < GRIDVIEWROWCNT; i++)
				{
					memset(&users[i], 0, sizeof(TUser));
					while(read(fdUser,&tuser,sizeof(TUser))==sizeof(TUser))
					{
						userdatacuroffset_e = lseek(fdUser, 0, SEEK_CUR);
						if(tuser.PIN > 0){
							memcpy(&users[i], &tuser, sizeof(TUser));
							break;
						}
					}
					
					//printf("tuser.PIN2=%s  userdatacuroffset_e=%d\n", tuser.PIN2, userdatacuroffset_e);
				}
				break;
			}
			else {
				i++;
			}

		}

	}
	if(findflag){
		*cnt = i;
		userdatacuroffset_s = offsettmp;
	}else {
		*cnt = 0;
	}
	return findflag;
}

PUserlb GetUsermestolb_NameOrAcno(char *name, char *pin2, int *cnt)
{

	PUserlb  head,pb,pf;
	TUser tuser[GRIDVIEWROWCNT];
	head = pb = pf = NULL;
	int i = 0;
	//int offsettmp=0;
	//printf("sizeof(tuser)=%d ,cnt=%d\n", sizeof(tuser), *cnt);
	memset(&tuser,0,sizeof(tuser));
	if(FindUser_NameOrAcno(name, pin2, tuser, cnt))
	{
		for(i = 0; i < *cnt; i++)
		{
			pb = (PUserlb)malloc(sizeof(TUserlb));
			memset(pb,0,sizeof(TUserlb));
			memcpy(&pb->userlb, &tuser[i],sizeof(TUser));
			//printf("%s %d PIN2=%s ,cnt=%d\n", __FILE__, __LINE__, tuser[i].PIN2, *cnt);
			if (i==0)
				pf=head=pb;
			else
				pf->next=pb;
			pb->next=NULL;
			pf=pb;
		}
	}

	if (head == NULL)
		return NULL;
	else
	{
		if (gOptions.PIN2Width <= 5)
			head =SortUserlb(head);
		ppuserlb = head;
		return head;
	}
}

PUserlb GetUsermestolb_SMS(int smspin)
{

	PUserlb  head,pb,pf;
	PUser puser;
	head = pb = pf = NULL;
	TUData udata;
	int i = 0;
	puser = MALLOC(sizeof(TUser));
	memset(puser,0,sizeof(TUser));
	if (fdUser < 0)
		return NULL;
	lseek(fdUser,0,0);
	//printf("before read\n");
	while (read(fdUser,puser,sizeof(TUser))==sizeof(TUser))
	{
		if (puser->PIN >0)
		{
			if(FDB_GetUData(puser->PIN,&udata) != NULL) {
				if(udata.SmsID == smspin) {
					pb = (PUserlb)MALLOC(sizeof(TUserlb));
					memset(pb,0,sizeof(TUserlb));
					memcpy(&pb->userlb,puser,sizeof(TUser));

					if (pb->userlb.PIN > 0)
					{
						if (i==0)
							pf=head=pb;
						else
							pf->next=pb;
						pb->next=NULL;
						pf=pb;
						i++;
					}
				}
			}
		}

	}
	FREE(puser);
	if (head == NULL)
		return NULL;
	else
	{
		//printf("finish read\n");
		//sort user asc
		if (gOptions.PIN2Width <= 5)
			head =SortUserlb(head);
		return head;
	}
}

static int FDB_AddUserQuick(PUser user)
{
	int fd,rc;

	fd = SelectFDFromConentType(FCT_USER);
	lseek(fd, 0, SEEK_END);

    	if (write(fd, (char*)user, sizeof(TUser)) == sizeof(TUser))
	{
		SetBit((BYTE*)UserIDMap, user->PIN);
		rc = FDB_OK;
	}
	else
		rc = FDB_ERROR_IO;
	fsync(fd);

	return rc;
}

static int InitPinOffSet(int * count)
{
	TUser user;
	TSearchHandle sh;
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&user;
	SearchFirst(&sh);
	gUserPinOffSet = malloc(sizeof(int) * 50005);
	if(gUserPinOffSet == NULL) {
		return -1;
	}
	printf("____________%s   %d\n",__FILE__, __LINE__);
	memset(gUserPinOffSet, -1, sizeof(int) * 50005);
	while(!SearchNext(&sh))
	{
		if(user.PIN > 0)
		{
			(*count)++;
			gUserPinOffSet[user.PIN] = lseek(sh.fd, 0, SEEK_CUR)-1*sizeof(TUser);
		}
	}
	return 0;
}

/*
static void RefreshPinOffSet(U16 pin, int offset)
{
	gUserPinOffSet[pin] = offset;
}

static void DelPinOffSet(U16 pin)
{
	gUserPinOffSet[pin] = -1;
}
*/

static int FindFreePinByOffSet(void)
{
	int i=0;
	for(i=1; i<sizeof(gUserPinOffSet);i++)
	{
		if(gUserPinOffSet[i] < 0)
		{
			return i;
		}
	}
	return -1;
}

static int ProcessBatchTmp10(int lenstart)
{
	TFingerS FingerS;
	BYTE pTemplate[1024*2]={0};
	U32 len;
	U32 records;
	int i;
	int FingerCount=0;
	U16 tmplen;
	TZKFPTemplate tmp;
	lseek(fdbatchdata,4,SEEK_SET);
	if(read(fdbatchdata, &len, sizeof(int)) != sizeof(int))
	{
		printf("____________%s   %d,_____len=%d\n",__FILE__, __LINE__, len);
	}

	if(len > 0){
		records=(U32)(len/sizeof(TFingerS));
		FingerCount = FDB_CntTmp();
		for(i=0; i<records; i++)
		{
			lseek(fdbatchdata, lenstart+12+i*sizeof(TFingerS), SEEK_SET);
			if(read(fdbatchdata, &FingerS, sizeof(TFingerS)) == sizeof(TFingerS))
			{
				BYTE valid = GETVALID(FingerS.FingerID);
				if(FingerCount >= gOptions.MaxFingerCount*100){
					FingerCount = FDB_CntTmp();
					if(FingerCount >= gOptions.MaxFingerCount*100){
						break;
					}
				}
				lseek(fdbatchdata, lenstart+12+len+FingerS.OffSet, SEEK_SET);
				read(fdbatchdata, &tmplen, 2);
				if(read(fdbatchdata, pTemplate, tmplen) != tmplen){
					break;
				}
				//printf("____________%s   %d,_____FingerS.PIN=%d,FingerS.FingerID=%d, i=%d\n",__FILE__, __LINE__, FingerS.PIN, FingerS.FingerID, i);
				FDB_CreateTemplate((char*)&tmp, FingerS.PIN, FingerS.FingerID&0x0f, (char *)pTemplate, tmplen, valid);
				if(FDB_AddTmp((char*)&tmp) == FDB_OK)
				{
					FingerCount++;
				}
			}
			else
			{
				printf("____________%s   %d,_____read tmp error\n",__FILE__, __LINE__);
			}
		}
	}

	return 0;
}

static int Process9TmpData(char* buffer, int len)
{
	char buf[80];
	PFingerS FingerS;
	BYTE *pTemplate;
	TSearchHandle sh;
        int i, j, k, l;
	U32 records;
        U16 tmplen;
        U16 FingerCount=0;
        BOOL AddSign;
        char *FingerCacheBuf;
        int MaxFingerCacheCnt=200;
        int CurFingerCacheCnt;



        int MaxGroupFingerCnt=100;
        int FingerNumber;
        int CurGroupFingerCnt;
        BOOL ChangeSign;

	FingerS=(PFingerS)buffer;
	records=(U32)(len/sizeof(TFingerS));
	pTemplate=(BYTE *)FingerS+len;

	close(fdFingerTmp);

	fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);

	sh.ContentType=FCT_FINGERTMP;
	sh.buffer=(char *)gTemplate;

	//acquired the status for operation
	FingerNumber=0;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen)
		{
			FingerCount++;
			for(i=0;i<records;i++)
			{
				if((((PTemplate)sh.buffer)->Valid)&&
						(((PTemplate)sh.buffer)->PIN==FingerS[i].PIN)&&
						(((PTemplate)sh.buffer)->FingerID==(FingerS[i].FingerID&0x0f)))
				{

					if(FingerS[i].OpSign==OP_ADD_ONLY)
						FingerS[i].OpSign=OP_IGNORE;
					else if(FingerS[i].OpSign==OP_ADD_OVERWRITE)
						FingerS[i].OpSign=OP_UPDATE;
				}
			}
		}
		FingerNumber++;
	}
	//execute
	SearchFirst(&sh);
	AddSign=TRUE;
	j=1;
	k=0;
	FingerCacheBuf=MALLOC(MaxGroupFingerCnt*sizeof(TTemplate));
	while(k<FingerNumber)
	{
		CurGroupFingerCnt=MaxGroupFingerCnt;
		if ((k+MaxGroupFingerCnt)>FingerNumber)
			CurGroupFingerCnt=FingerNumber-k;
		read(fdFingerTmp, FingerCacheBuf, CurGroupFingerCnt*sizeof(TTemplate));
		k+=CurGroupFingerCnt;
		DBPRINTF("GROUP %d Starting......\n", j);
		ChangeSign=FALSE;
		for(l=0;l<CurGroupFingerCnt;l++)
		{
			sh.buffer=(char*)((PTemplate)FingerCacheBuf+l);
			for(i=0;i<records;i++)
			{
				if((((PTemplate)sh.buffer)->Valid)&&
						(((PTemplate)sh.buffer)->PIN==FingerS[i].PIN))
				{
					if((FingerS[i].OpSign==OP_DEL)&&
							((FingerS[i].FingerID==0xFF)||(((PTemplate)sh.buffer)->FingerID==(FingerS[i].FingerID&0x0f))))
					{
						//delete template
						((PTemplate)sh.buffer)->Valid=0;
						FingerCount--;
						ChangeSign=TRUE;
					}
					else if((FingerS[i].OpSign==OP_UPDATE)&&
							(((PTemplate)sh.buffer)->FingerID==(FingerS[i].FingerID&0x0f)))
					{
						BYTE valid = GETVALID(FingerS[i].FingerID);
						memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
						FDB_CreateTemplate(sh.buffer,
								FingerS[i].PIN, FingerS[i].FingerID&0x0f,
								(char *)(pTemplate+FingerS[i].OffSet+2),
								tmplen,valid);
						ChangeSign=TRUE;
					}
				}
			}
			//append record
			if(AddSign&&(((PTemplate)sh.buffer)->Valid==0)&&(FingerCount<gOptions.MaxFingerCount*100))
			{
				AddSign=FALSE;
				for(i=0;i<records;i++)
				{
					//append record
					if(FingerS[i].PIN&&((FingerS[i].OpSign==OP_ADD_ONLY)||(FingerS[i].OpSign==OP_ADD_OVERWRITE)))
					{
						BYTE valid = GETVALID(FingerS[i].FingerID);
						memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
						FDB_CreateTemplate(sh.buffer,
								FingerS[i].PIN, FingerS[i].FingerID&0x0f,
								(char *)(pTemplate+FingerS[i].OffSet+2),
								tmplen,valid);
						FingerS[i].PIN=0;
						FingerCount++;
						AddSign=TRUE;
						ChangeSign=TRUE;
						break;
					}
				}
			}
		}
		if(ChangeSign)
		{
			lseek(fdFingerTmp, -1*CurGroupFingerCnt*sizeof(TTemplate), SEEK_CUR);
			write(fdFingerTmp, FingerCacheBuf, CurGroupFingerCnt*sizeof(TTemplate));
		}
		DBPRINTF("GROUP %d Endded\n", j);
		j++;
	}
	FREE(FingerCacheBuf);
	sh.buffer=(char *)gTemplate;//2009.12.11 fix upload finger count error
	if(AddSign)
	{
		//append all record that left
		lseek(fdFingerTmp, 0, SEEK_END);
		FingerCacheBuf=MALLOC(MaxFingerCacheCnt*sizeof(TTemplate));
		CurFingerCacheCnt=0;
		for(i=0;i<records;i++)
		{
			//append record
			if(FingerS[i].PIN&&((FingerS[i].OpSign==OP_ADD_ONLY)||(FingerS[i].OpSign==OP_ADD_OVERWRITE)))
			{
				BYTE valid = GETVALID(FingerS[i].FingerID);
				if(FingerCount>=gOptions.MaxFingerCount*100) break;
				memcpy(&tmplen, pTemplate+FingerS[i].OffSet, 2);
				FDB_CreateTemplate(sh.buffer,
						FingerS[i].PIN, FingerS[i].FingerID&0x0f,
						(char *)(pTemplate+FingerS[i].OffSet+2),
						tmplen,valid);
				memcpy(FingerCacheBuf+CurFingerCacheCnt*sizeof(TTemplate), (void*)sh.buffer, sizeof(TTemplate));
				CurFingerCacheCnt++;
				if(CurFingerCacheCnt==MaxFingerCacheCnt)
				{
					write(fdFingerTmp, FingerCacheBuf, CurFingerCacheCnt*sizeof(TTemplate));
					CurFingerCacheCnt=0;
					DBPRINTF("Write cache data to disk!\n");
				}
				//write(fdFingerTmp, (void*)sh.buffer, sizeof(TTemplate));
				FingerS[i].PIN=0;
				FingerCount++;
			}
		}
		if(CurFingerCacheCnt)
		{
			write(fdFingerTmp, FingerCacheBuf, CurFingerCacheCnt*sizeof(TTemplate));
			DBPRINTF("Write cache data to disk finished!\n");
		}
		FREE(FingerCacheBuf);
	}
	fsync(fdFingerTmp);
	close(fdFingerTmp);

	fdFingerTmp=open(GetEnvFilePath("USERDATAPATH", "data/template.dat", buf), O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);

	sync();

	return 0;
}

int ProcessBatchData(void)
{
	int size=0;
	TUserS UserS;
	int usercnt=0;
	int i;

	int curusrcnt=0;
	int quickadd=0;
	

	U32 len;
	int oplen;
	char *buffer;

	InitPinOffSet(&curusrcnt);
	lseek(fdbatchdata, 0, SEEK_SET);
	if(read(fdbatchdata, &size, sizeof(int)) != sizeof(int))
	{
		if(gUserPinOffSet != NULL) {
			free(gUserPinOffSet);
			gUserPinOffSet = NULL;
		}
		return -1;
	}
	lseek(fdbatchdata, 12, SEEK_SET);
	usercnt = size/sizeof(TUserS);
	printf("____________%s   %d, usercnt = %d\n",__FILE__, __LINE__, usercnt);
	for(i=0; i<usercnt; i++)
	{
		/*限制最大用户数为5万*/
		if(curusrcnt >= gOptions.MaxUserCount*100 || curusrcnt >= 50000) {
			break;
		}
		
		if(read(fdbatchdata, &UserS, sizeof(TUserS)) == sizeof(TUserS))
		{
			if(UserS.User.PIN > 0)
			{
				if(gUserPinOffSet[UserS.User.PIN] >= 0)
				{
					lseek(fdUser, gUserPinOffSet[UserS.User.PIN], SEEK_SET);
					write(fdUser, &UserS.User, sizeof(TUser));
				}
				else
				{
					curusrcnt++;
					if(quickadd == 0)
					{
						if (FDB_GetUser(0, NULL) == NULL)
						{
							quickadd = 1;
						}
					}
					
					if(quickadd)
					{
						FDB_AddUserQuick(&UserS.User);
					} else {
						FDB_AddUser(&UserS.User);
					}
					gUserPinOffSet[UserS.User.PIN] = lseek(fdUser, 0, SEEK_CUR)-1*sizeof(TUser);
				}
			}
			else
			{
				if(quickadd == 0)
				{
					if (FDB_GetUser(0, NULL) == NULL)
					{
						quickadd = 1;
					}
				}
				UserS.User.PIN = FindFreePinByOffSet();
				if(quickadd)
				{
					FDB_AddUserQuick(&UserS.User);
				} else {
					FDB_AddUser(&UserS.User);
				}
				gUserPinOffSet[UserS.User.PIN] = lseek(fdUser, 0, SEEK_CUR)-1*sizeof(TUser);
			}
		}
		else
		{
			printf("____________%s   %d\n",__FILE__, __LINE__);
		}
	}
	if(gUserPinOffSet != NULL) {
		free(gUserPinOffSet);
		gUserPinOffSet = NULL;
	}
	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		ProcessBatchTmp10(size);
		return 0;
	}

	lseek(fdbatchdata, 4, SEEK_SET);
	if(read(fdbatchdata, &len, sizeof(int)) != sizeof(int)) {
		return -2;
		printf("____error________%s   %d\n",__FILE__, __LINE__);
	}
	if(len <= 0) {
		printf("____no finger_____%s   %d\n",__FILE__, __LINE__);
		return 0;
	}

	lseek(fdbatchdata, 8, SEEK_SET);
	if(read(fdbatchdata, &oplen, sizeof(int)) != sizeof(int)) {
		return -1;
	}
	/*9.0算法按原来方式处理，一次性分配内存*/
	buffer = malloc(oplen + len + 1);
	if(buffer == NULL) {
		return -1;
	}

	lseek(fdbatchdata, 12 + size, SEEK_SET);
	printf("____oplen = %d, len = %d________%s   %d\n", oplen, len, __FILE__, __LINE__);
	if(read(fdbatchdata, buffer, oplen + len) != (oplen + len)) {
		printf("____error________%s   %d\n",__FILE__, __LINE__);
		free(buffer);
		buffer = NULL;
		return -1;
	}
	
	Process9TmpData(buffer, len);
	free(buffer);
	buffer = NULL;
	return 0;
	
}

int GetTmpPameters(int *fingercount,int *UserIDs)
{
	if(!fhdl && gOptions.ZKFPVersion == ZKFPV10)
		return 0;
	//BIOKEY_GET_PARAMETER(fhdl,ZKFP_PARAM_CODE_USERCOUNT,fingercount);
	BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERCOUNT,fingercount);
	//BIOKEY_GET_PARAMETER(fhdl,ZKFP_PARAM_CODE_USERIDS,UserIDs);
	BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERIDS,UserIDs);
	return 1;
}

int GetMax2MV10TmpData(int *Size,int fingercount,int *currentCnt,int*UserIDs,char* Buff)
{
	int count=0;
	TZKFPTemplate tmp;
	int lengthTmp=0;
	if(!fhdl && gOptions.ZKFPVersion == ZKFPV10)
		return 0;
	while(*currentCnt < fingercount)
	{
		memset(&tmp,0,sizeof(TZKFPTemplate));
		lengthTmp=FDB_GetTmp(UserIDs[*currentCnt]&0xffff,(UserIDs[*currentCnt]>>16)&0xf,(char *)&tmp);
		printf("________________%s%d,,,,,UserIDs[%d]=%d ,,lengthTmp=%d\n",__FILE__,__LINE__,*currentCnt,UserIDs[*currentCnt], lengthTmp);
		(*currentCnt)++;
		if(lengthTmp<=0)
			continue;
		count++;
		tmp.Size+=6;
		//printf("__________%s%d___Size=%d,fingercount=%d,currentCnt=%d,,,lengthTmp=%d\n",__FILE__,__LINE__,*Size,fingercount,*currentCnt, lengthTmp);
		memcpy(Buff+*Size,(char*)&tmp,6);
		*Size +=6;
		memcpy(Buff+*Size,tmp.Template,tmp.Size-6);
		*Size += tmp.Size-6;
		if(count > 500){
			break;
		}
	}
	lengthTmp=*Size;
	return lengthTmp;
}

int GetTmpDataByID(int *Size,int fingercount,int *currentCnt,int*UserIDs,char* Buff)
{
	int count=0;
	TZKFPTemplate tmp;
	int lengthTmp=0;
	*Size = 0;
	if(!fhdl && gOptions.ZKFPVersion == ZKFPV10)
		return 0;
	printf("______fingercount = %d, currentcnt= %d\n", fingercount, *currentCnt);
	//return 0;
	for(count = 0; count <fingercount; count++)
	{
		memset(&tmp,0,sizeof(TZKFPTemplate));
		lengthTmp=FDB_GetTmp(UserIDs[*currentCnt]&0xffff,(UserIDs[*currentCnt]>>16)&0xf,(char *)&tmp);
		printf("________________%s%d,,,,,UserIDs[%d]=%d ,,lengthTmp=%d\n",__FILE__,__LINE__,*currentCnt,UserIDs[*currentCnt], lengthTmp);
		(*currentCnt)++;
		if(lengthTmp<=0)
			continue;
		tmp.Size+=6;
		if(*Size +6 > 400*1024) {
			
			break;
		}
		
		memcpy(Buff+*Size,(char*)&tmp,6);
		*Size +=6;
		memcpy(Buff+*Size,tmp.Template,tmp.Size-6);
		*Size += tmp.Size-6;
	}
	lengthTmp=*Size;
	return lengthTmp;
}

int GetFreePIN2Fast(char *pin2)
{
	TSearchHandle sh;
	int *pCacheUser = NULL;
	int j = 0;
	int i = 0;
	int k = 0;
	int bSign = FALSE;
	int memflag = 0;

	j = FDB_CntUserByMap()+2;
	printf("ImageBufferLength=%d, cnt=%d\n", ImageBufferLength, (j*sizeof(int)) );

	if (!gOptions.IsOnlyRFMachine && ImageBufferLength >= j*sizeof(int)) {
			pCacheUser = (int*)gImageBuffer;
			memset(gImageBuffer, 0, ImageBufferLength);
	} else {
		pCacheUser = malloc(j * sizeof(int));
		if (pCacheUser == NULL){
			return -1;
		}
		memset(pCacheUser, 0, j*sizeof(int));
		memflag = 1;
	}

	/*read PIN2 and sort, when insert PIN2, will filter incorrect PIN2. for example: A123 or more than 0x7FFFFFFF */
	sh.ContentType=FCT_USER;
	sh.buffer=(char*)&gUser;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{
		if(sh.datalen <= 0) {
			continue;
		}
		i = atoi(gUser.PIN2);
		if (i > 0 && i <= j) {
			memcpy(pCacheUser + i, &i, sizeof(int));
		}
	}

	for(k = 1; k <= j; k++) {
		if(*(pCacheUser + k) == 0) {
			sprintf(pin2, "%u", k);
			bSign = TRUE;
			break;
		}
	}
	//printf("____, %s-%d, pin2 = %s\n", __FILE__, __LINE__, pin2);

	/*free memory*/
	if (memflag) {
		free(pCacheUser);
		pCacheUser=NULL;
	}
	return bSign;
}

TSerialCardNo gSerialCardNo;

PSerialCardNo FDB_GetSerialCardNumber(U16 pin, PSerialCardNo serialno)
{
  	TSearchHandle sh;
   	memset(&gSerialCardNo, 0, sizeof(TSerialCardNo));

	sh.ContentType=FCT_SERIALCARDNO;
	sh.buffer=(char*)&gSerialCardNo;
	SearchFirst(&sh);
	while(!SearchNext(&sh))
	{	
		if (((PSerialCardNo)sh.buffer)->PIN == pin)
		{
			if(serialno)
			{
				memcpy(serialno, sh.buffer, sizeof(TSerialCardNo));
				return serialno;
			}
			else
				return (PSerialCardNo)sh.buffer;
        	}
   	}
	printf("[%s]no serialcardnumber\n", __FUNCTION__);
    	return NULL;    
}


int FDB_AddSerialCardNumber(PSerialCardNo userserial)
{
	return SearchAndSave(FCT_SERIALCARDNO, (char*)userserial, sizeof(TSerialCardNo));
}

int FDB_ChgSerialCardNumber(PSerialCardNo spuser)
{
     	   PSerialCardNo u;
    	  if(NULL == (u = FDB_GetSerialCardNumber(spuser->PIN, NULL)))
      	  {
      		return FDB_ERROR_NODATA;
      	  }
	  if(0==memcmp((void*)spuser, (void*)u, sizeof(TSerialCardNo))) return FDB_OK;
     	  lseek(fdserialcardno, -1*sizeof(TSerialCardNo), SEEK_CUR);
      	  if(write(fdserialcardno,(void*)&spuser, sizeof(TSerialCardNo)) != sizeof(TSerialCardNo))
          {
        	 return FDB_ERROR_IO;
          }
	  else
      	   	return FDB_OK;
}

int FDB_DelSerialCardNumber(U16 pin)
{
	TSerialCardNo tmp;
	int ret;
	if(NULL == (FDB_GetSerialCardNumber(pin, &tmp)))
	{
        	return FDB_ERROR_NODATA;
  	}
    	memset(&tmp, 0, sizeof(TSerialCardNo));
       	lseek(fdserialcardno, -1*sizeof(TSerialCardNo), SEEK_CUR);
       	if(write(fdserialcardno,(void*)&tmp, sizeof(TSerialCardNo)) == sizeof(TSerialCardNo))
		ret = FDB_OK;
	else
		ret = FDB_ERROR_IO;
	fsync(fdserialcardno);
	return ret;
}

