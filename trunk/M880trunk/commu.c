/*************************************************

  ZEM 200                                          

  commu.c communication for PC                             

  Copyright (C) 2003-2005, ZKSoftware Inc.      		

 *************************************************/
#include <stdlib.h>
#include <string.h>
#include <asm/unaligned.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>

#include "arca.h"
#include "main.h"
#include "utils.h"
#include "commu.h"
#include "msg.h"
#include "flashdb.h"
#include "options.h"
#include "exfun.h"
#include "sensor.h"
#include "net.h"
#include "serial.h"
#include "ssracc.h"
#include "tempdb.h"
#include "kb.h"
#include "finger.h"
#include "flash.h"
#include "zlg500b.h"
#include "ssrpub.h"
#include "minilzo.h"
#include "libdlcl.h"
#include "tcpcomm.h"
#include "backup_restore.h"//add by zhc 2008.11.26
#include "delattpic.h" //dsl 2012.3.23
#include "pushapi.h"
#include "options.h"
#include "utils2.h"

#ifdef FACE
#include "facedb.h"
#endif

#define FCT_SYSTEM_NONE (U8)0xF0

//add by zhc 2008.11.25 for WriteLCD
#define MSG_WRITELCD (MSG_USER + 100)
extern HWND hWriteLCDWnd;
extern HWND hMainWindowWnd;
//end add by zhc
int CommSessionCount=0;
PCommSession CommSessions;
unsigned short in_chksum(unsigned char *p, int len);
void SendLargeData(char *in_buf, PCommSession session, char *buf, int size);
void AppendRTLog(int EventType,char *Data, int Len);
int GetFreeUserPIN(void);
char *GetDefaultOption(const char *name,char *value);
void ExSetPowerOnTime(int Hour, int Minute);
void ExPowerRestart(void);
int SaveUserTZ(int UserID, int *TZs);
BOOL SaveLOGOToFlash(int offset, char *config, int size);
BOOL SaveUBOOTToFlash(char *data, int size);
void ShowMainLCD(HWND hWnd);
int WriteCard(char *buffer,U32 PIN, int writelen);
void SwitchMsgType(int Enabled);

int SendTmpDataToSDKByID(char *in_buf, PCommSession session);
int QueryTmpData(char *in_buf, PCommSession session);

static int MaxCommSessionCount=0;

extern PRTLogNode rtloglist;
extern PRTLogNode rtlogheader;
extern HWND hMainWindowWnd;
extern long ParseIP(char *buffer,char *c1,char *c2,char *c3,char *c4);
extern int UpdateSoftWareFromUDisk(void );

int RTLogNum=0;
char* LastRTLogBuf=NULL;
int LastRTLogBufLen=0;

U32 UDiskCheckSum = 0;
U32 UDiskLength = 0;
char IsCheckEncryptFlag = 1;

extern int fdbatchdata;
static int gBatchDataLen=0;
static int gFinalBatchDataLen=0;
static int isWriteFlashTmp = 0;/*高速上传50000指纹时，需要写flash，内存不够使add by zxz */
int fast485modeflag = 0;/*设置485高速模式*/
static int *tmpUserIDs = NULL;

int SessionSendMsg(PCommSession session)
{
	char buf[1024];
	int start, isize, size;
	if(session->MsgCount>0)
	{
		start=session->MsgStartAddress[session->MsgLast];
		size=session->MsgLength[session->MsgLast];
		isize=(MAX_MSGBUFFER_SIZE-start);
		if(isize>=size) 
			memcpy(buf, session->MsgBuffer+start, size);
		else
		{
			memcpy(buf, session->MsgBuffer+start, isize);
			memcpy(buf+isize, session->MsgBuffer, size-isize);
		}
		//DBPRINTF("SEND MSG: Index=%d, isize=%d, size=%d, Start=%d\n",session->MsgLast, isize, size, start);
		session->Send(buf, size, session->Sender);
	}
	return TRUE;
}

int CheckSessionSendMsg(void)
{
	int i;
	for(i=0;i<CommSessionCount;i++)
	{
		if(CommSessions[i].MsgCached)
		{
			SessionSendMsg(CommSessions+i);
		}
	}
	return TRUE;
}

//消息的缓冲可以看成一个先进先出队列
//把消息压入，返回-1表示错误，其他表示正确
int SessionPushInAMsg(PCommSession session, PCmdHeader Data, int Len)
{
	int Res=MAX_MSGBUFFER_SIZE, index, Address, bsize, i; //缓存区剩余的字节数
	if(session->MsgCount==0)
	{
		index=session->MsgLast;
		Address=0;
	}
	else if(session->MsgCount>=MAX_BUFFER_MSG)      //已缓存的消息太多
	{
		session->MsgCached=0;	
		return -1;
	}
	else
	{
		Res=0;
		index=session->MsgLast;
		for(i=0;i<session->MsgCount;i++)
		{
			Res+=session->MsgLength[index];
			index++;
			if(index>=MAX_BUFFER_MSG) index-=MAX_BUFFER_MSG;
		}
		Address=Res+session->MsgStartAddress[session->MsgLast];
		if(Address>=MAX_MSGBUFFER_SIZE) Address-=MAX_MSGBUFFER_SIZE;
		Res=MAX_MSGBUFFER_SIZE-Res;
	}

	if(Res<Len)     //缓存区空间不够
	{//释放旧的消息
		int sindex=0;
		for(i=0;i<session->MsgCount;i++)
		{
			sindex=session->MsgLast+i;
			if(sindex>=MAX_BUFFER_MSG) sindex-=MAX_BUFFER_MSG;
			Res+=session->MsgLength[sindex];
			if(Res>=Len) break;
		}
		if(i>=session->MsgCount) return -1;
		sindex+=1;
		if(sindex>=MAX_BUFFER_MSG) sindex-=MAX_BUFFER_MSG;
		session->MsgLast=sindex;                
	}

	Data->ReplyID=index;
	Data->CheckSum=0;
	Data->CheckSum=in_chksum((void*)Data,Len);

	session->MsgStartAddress[index]=Address;
	session->MsgLength[index]=Len;
	session->MsgCount++;
	bsize=MAX_MSGBUFFER_SIZE-Address;
	if(Len<bsize)
	{
		memcpy(session->MsgBuffer+Address, (void*)Data, Len);
	}
	else
	{
		memcpy(session->MsgBuffer+Address, (void*)Data, bsize);
		memcpy(session->MsgBuffer, (char*)Data+bsize, Len-bsize);
	}	
	//DBPRINTF("Session PushMsg: Index=%d, Address=%d\n", index, Address);
	return index;
}

int SessionTakeOutMsg(PCommSession session, int Index)
{
	if(Index==session->MsgLast)
	{
		DBPRINTF("Session TakeOutMsg: %d\n",Index);
		session->MsgLast++;
		if(session->MsgLast>=MAX_BUFFER_MSG) session->MsgLast=0;
		if(session->MsgCount>0)
			session->MsgCount--;
		SessionSendMsg(session);
		return Index;
	}
	else
	{
		DBPRINTF("Session TakeOutMsg BAD: Index=%d, Last=%d\n",Index, session->MsgLast);
		return -1;
	}
}

int SessionClearMsg(PCommSession session)
{
	session->MsgCount=0;
	session->MsgLast=0;
	return TRUE;
}

PCommSession CreateSession(void *param)
{
	PCommSession res=NULL;
	int i;
	for(i=0;i<CommSessionCount;i++)
	{
		if(0==memcmp(CommSessions[i].Sender, param, SENDER_LEN))
		{
			res=CommSessions+i;
			break;
		}
	}
	if(res==NULL)
	{
		if(CommSessionCount+1>MaxCommSessionCount)
		{
			void *newp;
			MaxCommSessionCount+=5;
			newp=MALLOC(sizeof(TCommSession)*MaxCommSessionCount);
			if(CommSessionCount)
			{
				memcpy(newp, CommSessions, sizeof(TCommSession)*CommSessionCount);
				FREE(CommSessions);
			}
			CommSessions=newp;
		}
		res=CommSessions+CommSessionCount;
		CommSessionCount++;
		memset((void*)res,0,sizeof(TCommSession));
		do
		{
			res->SessionID=time(NULL) & 0xFFFF;
			for(i=0;i<CommSessionCount-1;i++) if(res->SessionID==CommSessions[i].SessionID) break;
		}while(i<CommSessionCount-1);
		memcpy(res->Sender,param, SENDER_LEN);
		res->Close=NULL;
		memset(res->Interface, 0, 16);
	}
	res->StartTime=EncodeTime(&gCurTime);
	res->LastReplyID=0;
	res->LastSendLen=0;
	res->LastActive=res->StartTime;
	res->LastCommand=CMD_CONNECT;
	SessionClearMsg(res);
	return res;
}

int CheckSessionTimeOut(void)
{
	int i;
	TTime gTime;

	for(i=0;i<CommSessionCount;i++)
	{
		int sec;

		if (CommSessions[i].TimeOutSec>0)
		{
			GetTime(&gTime);
			sec=EncodeTime(&gTime)-CommSessions[i].LastActive;

			if(CommSessions[i].TimeOutSec<=sec)
			{
				EnableDevice(TRUE);
				CommSessions[i].TimeOutSec = 0;
				return 1;
			}   
		}
		else if(CommSessions[i].MsgCached && CommSessions[i].MsgCount>0)
		{
			SessionSendMsg(CommSessions+i);
		}
	}
	return 0;
}

PCommSession GetSession(int SessionID)
{
	int i;
	for(i=0;i<CommSessionCount;i++)
		if(CommSessions[i].SessionID==SessionID)
		{
			return CommSessions+i;
		}
	return NULL;
}

int FreeSession(int SessionID)
{
	int i,j;
	for(i=0;i<CommSessionCount;i++)
		if(CommSessions[i].SessionID==SessionID)
		{
			if(CommSessions[i].Close)
				CommSessions[i].Close((void *)CommSessions[i].Sender);
			if(CommSessions[i].Buffer)
				freeBuffer(CommSessions[i].Buffer);
			for(j=i+1;j<CommSessionCount;j++)
				memcpy(CommSessions+j-1,CommSessions+j,sizeof(TCommSession));
			CommSessionCount--;
			if(CommSessionCount==0)
			{
				EnableDevice(TRUE);
				if(gOptions.EnableCommEndBeep)
				{
					ExBeep(1);
				}
			}
			return 1;
		}
	return 0;
}

void SendEvent(PCommSession session, int EventFlag, char *Data, int Len)
{
	int size;
	char buf[1024];
	PCmdHeader chdr=(PCmdHeader)buf;
	chdr->Command=CMD_REG_EVENT;
	chdr->ReplyID=0;
	chdr->SessionID=EventFlag;
	memcpy((void *)(chdr+1), Data, Len);
	size=sizeof(TCmdHeader)+Len;
	chdr->CheckSum=0;
	chdr->CheckSum=in_chksum((void*)buf,size);
	if(!session->MsgCached)
		SessionClearMsg(session);	
	if(SessionPushInAMsg(session, chdr, size)>=0)
		SessionSendMsg(session);

}

int CheckSessionSend(int EventFlag, char *Data, int Len)
{
	int i;

	AppendRTLog(EventFlag,Data,Len);
	AppendRTLogToFile(EventFlag,(unsigned char *)Data,Len);
	for(i=0;i<CommSessionCount;i++)
		if((CommSessions[i].RegEvents & EventFlag)==EventFlag)
			SendEvent(CommSessions+i, EventFlag, Data, Len);

	return 0;
}

PCommSession CheckSessionVerify(int *PIN, int *FingerID)
{
	int i;
	for(i=0;i<CommSessionCount;i++)
	{
		if(CommSessions[i].VerifyUserID>0)
		{
			*PIN=CommSessions[i].VerifyUserID;
			*FingerID=CommSessions[i].VerifyFingerID;
			CommSessions[i].RegEvents |= EF_VERIFY;
			return CommSessions+i;
		}
	}	
	return FALSE;
}

int MakeKey(int Key, int SessionID)
{
	int k,i;
	WORD swp;

	k=0;
	for(i=0;i<32;i++)
		if(Key & (1<<i))
			k=(k<<1 | 1);
		else
			k=k<<1;
	k+=SessionID;

	((BYTE*)&k)[0]^='Z';
	((BYTE*)&k)[1]^='K';
	((BYTE*)&k)[2]^='S';
	((BYTE*)&k)[3]^='O';

	swp=(k>>16);
	k=(k<<16)+swp;

	//DBPRINTF("Key: %d,%d->%d \n", Key, SessionID, k);

	return k;
}

//验证连接密码是否正确
int CheckCommKey(int Key, WORD SessionID, int AuthKey)
{
	//AuthKey的得到方法
	//Key按位反序，与SessionID相加，
	//与“ZKSO”异或，
	//比较高字和低字，如果高字>低字则交换字顺序
	//取1-255的随机数B，用B异或每一字节，然后用B替代第三个字节
	BYTE B=((BYTE*)&AuthKey)[2];
	Key=MakeKey(Key, SessionID);
	if((((BYTE*)&AuthKey)[0]^B)==((BYTE*)&Key)[0])
		if((((BYTE*)&AuthKey)[1]^B)==((BYTE*)&Key)[1])
			if((((BYTE*)&AuthKey)[3]^B)==((BYTE*)&Key)[3])
				return TRUE;
	return FALSE;
}

/*
   |--------|--------|--------|--------|--------|--------|--------|--------|
   |       CMD       |    Check Sum    |    Session ID   |    Reply ID     |
   */


extern int PowerSuspend;
extern int WaitPowerOff;
extern int gFPDirectProc;
extern int ShowMainLCDDelay;
extern int EnrollAFinger(char *tmp, int *len, int fingerid, char* PIN2, int enrollflag);

int ProcessEnroll(char* pin2, int fingerid, int *tmplen ,BYTE valid)
{
	TUser user;
	int ret;
	TZKFPTemplate tmp;
	int IsNewEnroll=0;
	int IsNewUser=0;
	U16 pin;
	//printf("%s %d PIN2:%s fingerid:%d  valid:%d\n",__FUNCTION__,__LINE__,pin2,fingerid,valid);

	//清空指纹缓存区
	memset(usertmp, 0, sizeof(usertmp));
	memset(usertmplen, 0, sizeof(usertmplen));

	memset(&user, 0, sizeof(TUser));

	//用户存在
	if (FDB_GetUserByCharPIN2(pin2, &user))
	{
		pin = user.PIN;
		if (fingerid==0) IsNewEnroll=1;
	}
	else
	{
		pin = GetFreeUserPIN();
		if (pin==0) return ERR_FAIL;
		IsNewUser=1;
		IsNewEnroll=1;
	}

	usertmplen[fingerid]=0;
	EnrollAFinger(usertmp[fingerid], &usertmplen[fingerid], fingerid, pin2, IsNewEnroll);

	if (usertmplen[fingerid]>0)
	{
		*tmplen = usertmplen[fingerid];
		if(FDB_CntTmp()>=gOptions.MaxFingerCount*100)
			return ERR_FAIL;


		int j=-1;
		if(valid==0)
			valid=1;
		else
			valid &= 0x03;

		memset(&tmp,0, sizeof(tmp));
		if(gOptions.ZKFPVersion==ZKFPV10)
		{
			if( !fhdl)
				return ERR_FAIL;

		}
		j=FDB_AddTmp((char *)FDB_CreateTemplate((char*)&tmp, (U16)pin, (char)fingerid, usertmp[fingerid], *tmplen,valid));
		FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, pin,j,fingerid,usertmplen[fingerid]);

		if(FDB_OK==j)
		{
			if (IsNewUser)
			{
				FDB_CreateUser(&user, (U16)pin, NULL, NULL, PRI_VALIDUSER);
				memcpy(user.PIN2, pin2, gOptions.PIN2Width);
				FDB_AddUser(&user);
				FDB_AddOPLog(ADMINPIN, OP_ENROLL_USER, user.PIN,j,0,0);
			}
			FDB_InitDBs(FALSE);
			FPInit(NULL);
			ret=ERR_OK;
		}
		else
		{
			ret=ERR_SAVE;
		}

	}
	else
		ret=ERR_FAIL;

	return ret;
}

extern int WaitCardAndWriteTemp(PFPCardOP tmp);
extern int PackTemplates(U8* Temp, U8 *Templates[], int TempCount, int ResSize);

extern int EmptyCard();
extern int WrineCard(char *buffer,U32 PIN, int writelen);
int ProcessWriteCard(char *a,int *pin,int *writelen)
{
	int ret;
	ret=WriteCard(a,*pin,*writelen);
	return ret;
}

int ProcessEmptyCard()
{
	int ret;
	ret=EmptyCard();
	return ret;
}
extern char *ReadDataBlock(int *size, int ContentType);
/*
   QueryData - Query
   CompressMethod - IN for specifying a compression method

*/
TBuffer *QueryData(char *CompressMethod, U16 DataType, char *Param, int *OriLen, PCommSession session)
{
	char *data=NULL;
	U32 Start, Size=BYTE2M;
	BYTE ram=1;
	TBuffer *ret=NULL;
	//char *buffer=NULL;
	//read data
	//DBPRINTF("QueryData: 0x%X, 0x%X, 0x%X\n", DataType, GETDWORD(Param), GETDWORD(Param+4));
	switch(DataType)
	{
		case CMD_QUERY_FIRWARE:
			Start=GETDWORD(Param);
			Size=GETDWORD(Param+4);
			ram=0;
			break;
		case CMD_DB_RRQ:
			data=ReadDataBlock((int*)&Size, *Param);
			break;
		case CMD_READ_NEW:
			Size=BYTE2M;
			if(FCT_ATTLOG==*Param) {
				data=FDB_ReadNewAttLogBlock((int*)&Size);
			} else if(FCT_OPLOG==*Param) {
				data=FDB_ReadNewOpLogBlock((int*)&Size);
			}
			break;
		case CMD_USERTEMP_RRQ:
			printf("CMD_USERTEMP_RRQ_________%s%d\n", __FILE__, __LINE__);
			data=ReadDataBlock((int*)(&Size), *Param);
			break;
		case CMD_ATTLOG_RRQ:
			Start=GETDWORD(Param);
			if(Start==0){
				/*dsl 2012.3.23*/
				if (gOptions.DownloadNewLog) {
					data=FDB_ReadAttBlock((int*)&Size, FCT_ATTLOG);
				} else {
					char fdbuf[128]={0};
					int isize = GetDataInfo(FCT_ATTLOG, STAT_VALIDLEN, 0);
					if (isize > 0) {
						if (isize+4 <= 1024) {
							data=ReadDataBlock((int*)&Size, FCT_ATTLOG);
						} else {
							session->fdCacheData = TruncFDAndSaveAs(session->fdCacheData,
									GetEnvFilePath("USERDATAPATH", "CacheData.dat", fdbuf), NULL, 0);
							data = (char*)malloc(16);
							Size = isize+4;
						}
					}
				}
			} else {
				int count=FDB_CntAttLog();
				Size=GETDWORD(Param+4);
				data=MALLOC(count*sizeof(TAttLog));
				if(data){
					Size=FDB_GetAttLog(Size, Start, MaxDate,(TAttLog*)data, count)*sizeof(TAttLog);
				}
				else {
					Size=0;
				}
			}
			break;
		case CMD_OPLOG_RRQ:
			data=ReadDataBlock((int*)&Size, FCT_OPLOG);
			break;
		case CMD_OPTIONS_RRQ:
			break;
		default:
			Size=0;
	}
	*OriLen=Size;
	if(data==NULL || Size==0) {
		return NULL;
	}
#if 0
	//dsl 2012.3.23
	if(data && (Size<=6*640*480) && (((Size>4*1024)&&(0xFF==*CompressMethod)) || (1==*CompressMethod)))
	{
		buffer=malloc(1024*1024);
		if (!buffer)
		{
			return NULL;
		}
	}
#endif
	ret=createRomBuffer((U32)data, Size);
	ret->isRom=!ram;
	*CompressMethod=0;/*dont compress dats*/
#if 0
	//compress?	
	if(data && (Size<=6*640*480) && (((Size>4*1024)&&(0xFF==*CompressMethod)) || (1==*CompressMethod)))//compress by lzo
	{
		char wrkmem[LZO1X_MEM_COMPRESS];
		int DataLen=0;
		//DBPRINTF("compress data at QueryData()\n");
		//dsl 2012.3.23
		//if(LZO_E_OK==lzo1x_1_compress((const lzo_byte *)data, Size, (lzo_byte*)gImageBuffer, (lzo_uintp)&DataLen, wrkmem))
		if(LZO_E_OK==lzo1x_1_compress((const lzo_byte *)data, Size, (lzo_byte*)buffer, (lzo_uintp)&DataLen, wrkmem))
		{
			if((int)Size>DataLen) //large than source
			{
				*CompressMethod=1;
				if(ram==0)
				{
					data=MALLOC(DataLen);
					if(data==NULL)
					{
						//dsl 2012.3.23
						if (buffer) free(buffer);
						return ret;
					}
				}
				//dsl. 2012.3.23
				//memcpy(data, gImageBuffer, DataLen);
				memcpy(data, buffer, DataLen);
				ret->buffer=(void*)data; 
				ret->bufEnd=(void*)(data+DataLen);
				ret->bufPtr=(void*)data;
				ret->isRom=0;
			}
		}
	}
	//dsl 2012.3.23
	if (buffer) free(buffer);
#endif
	return ret;
}

int ProcessCommand(PCommSession session, int cmd, PCmdHeader chdr, int size);
static char SaveIPAddress[16];
extern int CreateWriteLCDWindow(HWND hwnd);
int RunCommand(void *buf, int size, int CheckSecury)
{
	PCmdHeader chdr=(PCmdHeader)buf;
	int checksum,cmd,len, LastReplyID;
	PCommSession session;

	cmd=chdr->Command;
	if(size<sizeof(TCmdHeader)) return 0;
	session=GetSession(chdr->SessionID);
	if((session==NULL) && ! (cmd==CMD_CONNECT)) return 0;

	checksum=chdr->CheckSum;
	chdr->CheckSum=0;
	if(CheckSecury && !(checksum==in_chksum(buf,size))) return 0;

	if(cmd==CMD_CONNECT) WakeUpFromSleepStatus();

	if(cmd==CMD_CONNECT&&HasInputControl())
	{
		return 0;
	}

	chdr->Command=CMD_ACK_OK;
	len=sizeof(TCmdHeader);
	size-=len;

	if(session)	//已经建立了连接
	{
		if(chdr->ReplyID==session->LastReplyID)	//重复发送的数据包
		{
			if(session->LastSendLen)
			{
				//DBPRINTF("SEND PACKET AGAIN!lastlen=%d\n",session->LastSendLen);
				session->Send(session->LastSendData, session->LastSendLen, session->Sender);
				return cmd;
			}
		}
		else if((chdr->ReplyID<session->LastReplyID) && session->LastReplyID!=65535) //非递增顺序出现的数据包非法
		{
			return 0;
		}
	}
	LastReplyID=chdr->ReplyID;

	if(CheckSecury && 
			session &&			//已经建立了连接
			gOptions.ComKey &&		//需要验证连接密码
			!(session->Authen) &&		//还没有验证连接密码
			(cmd!=CMD_AUTH))		//不是授权命令
	{
		chdr->Command=CMD_ACK_UNAUTH;
	} else {
		len=ProcessCommand(session, cmd, chdr, size);
	}

	if(len)
	{
		session->LastCommand=cmd;
		session->LastReplyID=LastReplyID;
		chdr->CheckSum=0;
		if(CheckSecury) chdr->CheckSum=in_chksum(buf,len);
		session->Send(buf,len,session->Sender);
		memcpy(session->LastSendData, buf, len);
		session->LastSendLen=len;
		TTime ActTime;
		GetTime(&ActTime);
		//session->LastActive=EncodeTime(&gCurTime);
		session->LastActive=EncodeTime(&ActTime);
		if(cmd==CMD_EXIT) {
			FreeSession(chdr->SessionID);
		}
		else if(cmd==CMD_STARTENROLL && chdr->Command==CMD_ACK_OK) {
			int fingerid, tmplen;
			U16 ret;
			char pin2[24];

			char *p=(char*)(chdr+1);
			session->RegEvents|=EF_FINGER|EF_FPFTR;
			memset(pin2, 0, sizeof(pin2));
			memcpy(pin2, p, gOptions.PIN2Width);		//工号
			fingerid=((U8 *)p)[24];				//指纹序号
			ret = ProcessEnroll(pin2, fingerid, &tmplen,p[25]);
			memcpy(p, &ret, 2);
			if(ret==ERR_OK)	{			    
				memcpy(p+2, &tmplen, 2);
				memcpy(p+4, pin2, gOptions.PIN2Width);
				p[gOptions.PIN2Width+4]=fingerid;
				len=gOptions.PIN2Width+5;
			} else {
				len=2;
			}
			SendEvent(session, EF_ENROLLFINGER, p, len);
		} else if(cmd==CMD_WRITE_MIFARE && chdr->Command==CMD_ACK_OK) {
			int ret;
			U32 pin=0, len=0, writeLen=session->Buffer->bufferSize;
			char w[16];
			session->RegEvents|=EF_WRITECARD;
			ret=ProcessWriteCard((char *)session->Buffer,(int *) &pin,(int *) &writeLen);
			memcpy(w, &ret, 2);
			if(ret==ERR_OK)	{
				memcpy(w+2, &writeLen, 2);
				memcpy(w+4, &pin, 4);
				len=8;
			} else {
				len=2;
			}
			SendEvent(session, EF_WRITECARD, w, len);
			ShowMainLCDDelay=1;
		} else if(cmd==CMD_EMPTY_MIFARE && chdr->Command==CMD_ACK_OK) {
			int ret,len=0;
			char w[2];
			session->RegEvents|=EF_EMPTYCARD;
			ret=ProcessEmptyCard();
			memcpy(w, &ret, 2);
			len=2;
			SendEvent(session, EF_WRITECARD, w, len);
			ShowMainLCDDelay=1;
		} else if(cmd==CMD_OPTIONS_WRQ && chdr->Command==CMD_ACK_OK) {
			char *p=(char*)(chdr+1);
			if((strcmp(p, "IPAddress")==0))	{
				ExecuteActionForOption("IPAddress",SaveIPAddress);
			}
		} else if (cmd==CMD_RESTART) {
			RebootMachine();
		} else if (cmd==CMD_POWEROFF) {
			ExPowerOff(FALSE);
		} else if (cmd==CMD_SLEEP) {
			ExPowerOff(TRUE);
		}// else if (cmd == CMD_CLEAR_DATA) {//屏蔽删除数据重启
		//	RebootMachine();
		//}
	}
	return cmd;
}

char *ReadDataBlock(int *size, int ContentType)
{
	char *buffer;
	buffer=FDB_ReadBlock(size, ContentType);
	return buffer;
}

int ReadBlockAndSend(PCmdHeader chdr, PCommSession session, int ContentType)
{
	int size;
	char *buffer;

	buffer=FDB_ReadBlock(&size, ContentType);
	if(buffer)
	{
		SendLargeData((void*)chdr, session, buffer, size);
		FREE(buffer);
		return size;
	}
	else
		return FALSE;
}

int GetOptionNameAndValue(char *p, int size)
{
	char value[1024]={0};
	int l,vl;
	p[size]=0;
	/*dsl 2011.5.18*/
	if(!strcmp(p, "BuildVersion"))
	{
		strcpy(value, MAINVERSIONTFT);
		printf("BuildVersion=%s\n", value);
	}
	else if(!LoadStr(p, value))	//如果没有该 Option,则取其缺省值
	{
		GetDefaultOption(p,value);
	}
	if(value[0])	
	{
		l=strlen(p);
		vl=strlen(value);
		if(value && vl<1024)
		{
			p[l]='=';
			strcpy(p+l+1,value);
			return l+2+vl;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

extern TSensorBufInfo SensorBufInfo;
extern void CancelEnrollFinger(void);		//关闭在线登记指纹窗
int ProcessCommand(PCommSession session, int cmd, PCmdHeader chdr, int size)
{
	TUser user;
	TSms sms;
	TWORKCODE workcode;	
	TTimeZone tz;
	THTimeZone htz;
	TExtUser extuser;
	TGroup gp;
	TCGroup lgp;

	int i;
	char *p=(char*)(chdr+1);
	int len=sizeof(TCmdHeader);
	//printf("ProcessCommand cmd = %d\n", cmd);
	switch(cmd)
	{
		case CMD_CONNECT: 
			if (gOptions.ComKey)
				chdr->Command=CMD_ACK_UNAUTH;
			return 0;
		case CMD_EXIT:
			FlushSensorBuffer();
			break;
		case CMD_ACK_OK:
			session->MsgCached=TRUE;
			SessionTakeOutMsg(session, chdr->ReplyID);
			len=0;  //不需发送回应
			break;		
		case CMD_AUTH:
			session->Authen=CheckCommKey(gOptions.ComKey, session->SessionID, get_unaligned((int *)p));
			if(!(session->Authen)) chdr->Command=CMD_ACK_UNAUTH;
			break;
		case CMD_REG_EVENT:		
			memcpy(&session->RegEvents, p, 4);
			break;
		case CMD_RTLOG_RRQ:
			{
				extern int gRTLogListCount;
				int n=0, rtlognum;

				if(size>0 && RTLogNum>(rtlognum=GETDWORD(p)))
				{
					n=LastRTLogBufLen;
					SET_DWORD(p,n,0);
					if(n>0) memcpy(p+4, LastRTLogBuf, n);
				}
				else if(gRTLogListCount>0)
				{
					char *buf,*cbuf;
					PRTLogNode prtlognode,tmprtlognode;
					n=gRTLogListCount*sizeof(TRTLog);

					if(LastRTLogBuf) FREE(LastRTLogBuf);

					SET_DWORD(p,n,0);
					buf=MALLOC(n);
					cbuf=buf;
					prtlognode=rtlogheader->Next;
					while(prtlognode)
					{
						tmprtlognode=prtlognode;
						memcpy(cbuf,(char*)&(prtlognode->log),sizeof(TRTLog));					
						prtlognode=prtlognode->Next;
						cbuf+=sizeof(TRTLog);
						FREE(tmprtlognode);
					}
					gRTLogListCount=0;
					rtloglist=rtlogheader;
					memcpy(p+4,buf,n);

					LastRTLogBuf=MALLOC(n);
					memcpy(LastRTLogBuf,buf,n);
					LastRTLogBufLen=n;
					RTLogNum++;

					FREE(buf);
				}
				else
					SET_DWORD(p,0,0);
				len=len+4+n;
				break;
			}
		case CMD_DISABLEDEVICE:
			if(size==sizeof(U32))
			{
				session->TimeOutSec=get_unaligned((U32*)p);
			}
			EnableDevice(FALSE);
			break;
		case CMD_ENABLEDEVICE:
			EnableDevice(TRUE);
			break;


		case CMD_USERTEMP_EX_WRQ:
			{
				char *fptemp;
				BYTE duress=0;
				U16 PIN=get_unaligned((U16*)p);
				char fingerID=p[2];
				U16 tempLen=0;

				duress=p[3];
				if(duress==0)
					duress=1;
				else
					duress &=0x03;
				if(gOptions.ZKFPVersion == ZKFPV10)
				{
					fptemp=(char *)session->Buffer->buffer;
					tempLen = session->Buffer->bufferSize;
				}
				else
				{
					fptemp =p+6;
					tempLen = get_unaligned((U16*)(p+4));
				}
				if(FDB_OK!=(chdr->SessionID=AppendUserTemp(PIN, NULL, fingerID, fptemp, tempLen,duress)))
					chdr->Command=CMD_ACK_ERROR;
			}
			break;
		case CMD_USERTEMP_EX_RRQ:
			{
				if(size==0)
				{
					if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_USER)))
						chdr->Command=CMD_ACK_ERROR;
				}
				else if(size==1)
				{
					if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, *p)))
						chdr->Command=CMD_ACK_ERROR;
				}
				else if(size==3)
				{
					PUser u;
					TZKFPTemplate tmp;
					int ret;
					BYTE duress=0;
					u=FDB_GetUser(get_unaligned((U16*)p),NULL);
					memset(&tmp,0,sizeof(TZKFPTemplate));
					if(u && (ret=FDB_GetTmp(u->PIN,p[2],(char *)&tmp)))
					{
						if(gOptions.ZKFPVersion == ZKFPV10)
						{
							duress=tmp.Valid;
							memcpy(tmp.Template+ret,&duress,1);
							SendLargeData((void*)chdr, session, (char *)tmp.Template, ret+1);
						}
						else
						{
							duress =tmp.Valid;
							memcpy(p,tmp.Template, tmp.Size);
							memcpy((void*)(p+tmp.Size), &duress,1);
							len+=tmp.Size+1;
							chdr->SessionID=tmp.Size+1;
						}
					}
					else
						chdr->Command=CMD_ACK_ERROR;
				}
				break;
			}

		case CMD_APPEND_USERTEMP:
			{

				if(FDB_OK!=(chdr->SessionID=AppendUserTemp(get_unaligned((U16*)p), p+3, *(p+2), p+13, get_unaligned((U16*)(p+11)),1)))
					chdr->Command=CMD_ACK_ERROR;
			}

			break;
		case CMD_USERTEMP_WRQ:
			{
				char *fptemp;
				int fptemplen=0;
				if(gOptions.ZKFPVersion == ZKFPV10)
				{
					fptemp=(char *)session->Buffer->buffer;
					fptemplen = session->Buffer->bufferSize;
				}
				else
				{
					fptemp =p+5;
					fptemplen = get_unaligned((U16*)(p+3));
				}
				if(FDB_OK!=(chdr->SessionID=AppendUserTemp(get_unaligned((U16*)p), NULL, *(p+2), fptemp,fptemplen ,1)))
					chdr->Command=CMD_ACK_ERROR;
			}
			break;

		case CMD_STKEY_WRQ:
			chdr->Command = CMD_ACK_ERROR;
			if (size==sizeof(TSHORTKEY))
			{
				TSHORTKEY tstkey;
				nmemcpy((BYTE *)&tstkey, (BYTE *)p, sizeof(TSHORTKEY));
				if (FDB_GetShortKey(get_unaligned((U8*)p), NULL))
				{
					if (FDB_ChgShortKey(&tstkey) == FDB_OK)
						chdr->Command = CMD_ACK_OK;
					else
						chdr->SessionID = 1;
				}
				else
					chdr->SessionID=1;
			}
			break;
			//2008.09.05 增加SDK直接传输工号作为参数上传模板功能
		case CMD_SSRUSERTEMP_WRQ:
			{

				char acno[24];
				TUser ssruser;

				memset(&ssruser,0,sizeof(TUser));
				memset(acno,0,24);
				nmemcpy((BYTE *)acno,(const BYTE *)p,24);
				if (FDB_GetUserByCharPIN2(acno,&ssruser))
				{
					if(gOptions.ZKFPVersion == ZKFPV10)
					{
						int ret = BIOKEY_TEMPLATELEN((BYTE *)(p+28));
						if(ret <= 0)
						{
							printf("CMD_SSRUSERTEMP_WRQ: BIOKEY_TEMPLATELEN(tmp->Template) ret is %d\n", ret);
							chdr->Command=CMD_ACK_ERROR;
							break;
						}
					}

					if(FDB_OK!=(chdr->SessionID=AppendSSRUserTemp(ssruser.PIN, *(p+24), *(p+25), p+28, get_unaligned((U16*)(p+26)),1)))
					{
						chdr->Command=CMD_ACK_ERROR;
					}
				}
				else
				{
					chdr->Command=CMD_ACK_ERROR;
				}
				break;
			}
#ifdef FACE
			//For face
		case CMD_USERFACE_WRQ:	//CMD_APPEND_USERFACE:
			{
				char acno[24];
				TUser ssruser;


				memset(&ssruser,0,sizeof(TUser));
				memset(acno,0,24);
				nmemcpy((BYTE*)acno,(BYTE *)p,24);
				if (gOptions.FaceFunOn&&FDB_GetUserByCharPIN2(acno,&ssruser))
				{
					if(NULL==FDB_GetFaceRecord(ssruser.PIN,0,NULL))
					{
						if(FDB_CntFaceUser()>=(gOptions.MaxFaceCount)*100)
						{
							chdr->Command=CMD_ACK_ERROR;
							break;
						}	
					}				

					printf("------FACE--- acno:%s  pin=%d \n",acno,ssruser.PIN);
					if(session->Buffer->bufferSize<=0) 
						chdr->SessionID=1; 
					else if(NULL==session->Buffer) 
						chdr->SessionID=2; 
					else if(session->Buffer->bufPtr>session->Buffer->bufEnd) 
						chdr->SessionID=3; 
					else 
					{ 
						unsigned char *p1=session->Buffer->buffer;
						U32 L=session->Buffer->bufferSize;
						//DBPRINTF("CMD_USERFACE_WRQ L=%d \n",L);
#if 1
						int count=L/sizeof(TFaceTmp);
						int i,j;
						TFaceTmp Facetmp;
						//printf("%s SOCKET_CMD_USERFACETEMP_WRQ UserID:%d j:%d\n",__FUNCTION__,ssruser.PIN,j);
						for (j=0;j<count;j++)
						{
							//printf("%s SOCKET_CMD_USERFACETEMP_WRQ UserID:%d j:%d\n",__FUNCTION__,ssruser.PIN,j);
							memcpy(&Facetmp,(char *)p1+j*sizeof(TFaceTmp),sizeof(TFaceTmp));
							Facetmp.PIN=ssruser.PIN;
							i=ChangeFaceTmp(Facetmp.PIN,Facetmp.FaceID,&Facetmp);
						}
						if(ssruser.PIN ==0)
							printf("----:%s \n",acno);
						if(FDB_OK!=i)
#else
							if(FDB_OK!=(chdr->SessionID=AppendSSRUserFaceTemp(ssruser.PIN,(char *)p1,L)))
#endif
								chdr->Command=CMD_ACK_ERROR;
					}
				}
				else
					chdr->Command=CMD_ACK_ERROR;
				break;
			}
#endif

			/*
			   2008.09.05
			   用户数据有PIN,PIN2 重复现像,分析commu.c ,CMD_USER_WR
			   原来在找不到PIN2时进行新增时没有判断PIN是否存在
			   */
		case CMD_USER_WRQ:
			chdr->Command=CMD_ACK_ERROR;
			if((size==sizeof(TUser)))
			{
				TUser u;
				nmemcpy((BYTE *)&u, (BYTE *)p, sizeof(TUser));
				if (IDTFlag)
				{
					if (u.Group!=gOptions.MachineGroup)
					{
						chdr->SessionID=1;
						break;
					}
				}
				if(u.PIN2 &&((strlen(u.PIN2)==0)||(strcmp(u.PIN2,"0")==0)))
				{
					printf("user.PIN2 error\n");
					break;
				}
				else{
					int j;
					TUser u1;
					memset(&u1,0,sizeof(TUser));

					printf("u.PIN=%d, PIN2=%s\n", u.PIN, u.PIN2);

					if (FDB_GetUserByCharPIN2(u.PIN2, &u1))
					{
						if(u1.PIN == u.PIN)
						{
							if((j=FDB_ChgUser(&u))!=FDB_OK)
								chdr->SessionID=1;
							else
								chdr->Command=CMD_ACK_OK;
						}
						else
						{
							printf("user error\n");
							chdr->SessionID=1;
							break;
						}
					}
					else
					{
						printf("u1.PIN=%d, PIN2=%s\n", u1.PIN, u1.PIN2);

						if((FDB_CntUserByMap()<gOptions.MaxUserCount*100) &&(FDB_GetUser(u.PIN,NULL)==NULL))
						{
							if((j=FDB_AddUser(&u))!=FDB_OK)
							{
								printf("save error!!!!!!\n");
								chdr->SessionID=2;
							}
							else
							{
								//添加完用户应刷新 08.06.05
								sync();
								chdr->Command=CMD_ACK_OK;
							}
						}
					}
#ifdef SDK2ADMS
					FDB_AddOPLog(ADMINPIN, OP_ENROLL_USER, u.PIN,j,0,0);    //add by jazzy 2010.01.13 for test ADMS use SDK download data
#endif
				}
			}
			break;
		case CMD_EXTUSER_WRQ:
			chdr->Command=CMD_ACK_ERROR;
			if((size==sizeof(TExtUser)))
			{
				nmemcpy((BYTE *)&extuser, (BYTE *)p, sizeof(TExtUser));
				//printf("extuser.VerifyStyle:%d\n",extuser.VerifyStyle);
				if(FDB_GetExtUser(get_unaligned((U16*)p), NULL))
				{
					if(FDB_ChgExtUser(&extuser)!=FDB_OK)
						chdr->SessionID=1;
					else
						chdr->Command=CMD_ACK_OK;
				}
				else if(FDB_AddExtUser(&extuser)!=FDB_OK)
					chdr->SessionID=2;
				else 
					chdr->Command=CMD_ACK_OK;

			}
			break;
		case CMD_WorkCode_WRQ:
			chdr->Command=CMD_ACK_ERROR;
			if (size==sizeof(TWORKCODE))
			{
				int ret;
				nmemcpy((BYTE *)&workcode, (BYTE *)p, sizeof(TWORKCODE));

				if (FDB_RecreateWorkCode(&workcode))
				{
					//					printf("New WorkCode: PIN=%d, Code=%s, Name=%s\n", workcode.PIN, workcode.Code, workcode.Name);
					ret = FDB_ChgWorkCode(&workcode);
					if(ret==FDB_ERROR_NODATA)
					{
						if (FDB_AddWorkCode(&workcode)!=FDB_OK)
							chdr->SessionID=2;
						else
							chdr->Command=CMD_ACK_OK;
					}
					else if (ret==FDB_ERROR_IO)
						chdr->SessionID=1;
					else
						chdr->Command=CMD_ACK_OK;
				}	
			}
			break;
		case CMD_SMS_WRQ:
			chdr->Command=CMD_ACK_ERROR;
			if (size==sizeof(TSms))
			{
				int ret;			
				nmemcpy((BYTE *)&sms, (BYTE *)p, sizeof(TSms));
				ret=FDB_ChgSms(&sms); 
				if(ret==FDB_ERROR_NODATA)
				{
					if (FDB_AddSms(&sms)!=FDB_OK)
						chdr->SessionID=2;
					else
						chdr->Command=CMD_ACK_OK;
				}
				else if (ret==FDB_ERROR_IO)
					chdr->SessionID=1;
				else 
					chdr->Command=CMD_ACK_OK;
			}
			break;
		case CMD_UDATA_WRQ:
			if(size==sizeof(TUData))
			{
				//解决PC下传短消息时用户分配出现冲突的现象2007.7.26
				TUData udata;
				int res;
				memcpy((void*)&udata, p, sizeof(TUData));

				//如果用户不存在，则返回错误
				if (FDB_GetUser(udata.PIN, NULL)==NULL)
					chdr->Command = CMD_ACK_ERROR;
				else
				{	
					{
						res = FDB_AddUData(&udata);
						chdr->SessionID = res;
						if (res != FDB_OK)
							chdr->Command=CMD_ACK_ERROR;
						else
							chdr->Command=CMD_ACK_OK;
					}
				}
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_WRITE_MIFARE:
			{
				chdr->Command=CMD_ACK_ERROR;
				if(gMachineState==STA_VERIFYING || gMachineState==STA_IDLE)
				{
					chdr->Command=CMD_ACK_OK;
				}
				else
					chdr->SessionID=ERR_STATE;
				break;
			}
		case CMD_EMPTY_MIFARE:
			{
				chdr->Command=CMD_ACK_ERROR;
				if(gMachineState==STA_VERIFYING || gMachineState==STA_IDLE)
				{
					chdr->Command=CMD_ACK_OK;
				}
				else
					chdr->SessionID=ERR_STATE;
				break;		
			}
		case CMD_FREEID_RRQ:
			if(size==0)
			{
				i=1;
				while(i<MAX_PIN)
				{
					if(!FDB_GetUser(i,&user)) break;
					i++;
				}
				if(i<MAX_PIN)
					chdr->SessionID=(WORD)i;
				else
					chdr->Command=CMD_ACK_ERROR;
			}
			else
			{
				BYTE fid=0xff;
				i=get_unaligned((U16*)p);
				i=FDB_GetFreeFingerID(i,&fid);
				if(i!=FDB_OK)
				{
					chdr->Command=CMD_ACK_ERROR;
					chdr->SessionID=(WORD)i;
				}
				else
					chdr->SessionID=fid;
			}
			break;
		case CMD_USER_RRQ:
			if(FDB_GetUser(get_unaligned((U16*)p),&user))
			{
				len+=sizeof(TUser);
				memcpy(p, (void*)&user, sizeof(TUser));
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_EXTUSER_RRQ:
			{
				if(FDB_GetExtUser(get_unaligned((U16*)p), &extuser))
				{
					len+=sizeof(TExtUser);
					//				printf("extuser.VerifyStyle:%d\n",extuser.VerifyStyle);
					memcpy(p, (void*)&extuser, sizeof(TExtUser));
				}
				else
					chdr->Command=CMD_ACK_ERROR;
				break;
			}
		case CMD_STKEY_RRQ:			//86
			if (!(chdr->SessionID = ReadBlockAndSend(chdr, session, FCT_SHORTKEY)))
				chdr->Command = CMD_ACK_ERROR;
			break;
		case CMD_WorkCode_RRQ:
			{
				char tmpCode[MAX_WORKCODE_LEN+1];
				char rflag[1];

				nmemcpy((BYTE *)rflag,(BYTE *) p, 1);
				nmemcpy((BYTE *)tmpCode, (BYTE *)p+1, MAX_WORKCODE_LEN);

				if (rflag[0])              //Get all workcode
				{
					if (!(chdr->SessionID = ReadBlockAndSend(chdr, session, FCT_WORKCODE)))
						chdr->Command = CMD_ACK_ERROR;
				}
				else
				{
					if (FDB_GetWorkCodeByCode(tmpCode, &workcode))
					{
						len+=sizeof(TWORKCODE);
						memcpy(p, (void*)&workcode, sizeof(TWORKCODE));
					}
					else
						chdr->Command = CMD_ACK_ERROR;
				}
				break;
			}
		case CMD_SMS_RRQ:
			if(FDB_GetSms(get_unaligned((U16*)p), &sms))
			{
				len+=sizeof(TSms);
				memcpy(p, (void*)&sms, sizeof(TSms));
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_DB_RRQ:
			if(size>0)
			{
				if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, *p)))
					chdr->Command=CMD_ACK_ERROR;
			}
			break;
		case CMD_USERTEMP_RRQ:
			printf("CMD_USERTEMP_RRQ:: size=%d ",size);
			if(size==0)
			{
				if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_USER)))
					chdr->Command=CMD_ACK_ERROR;
			}
			else if(size==1)
			{
				if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, *p)))
					chdr->Command=CMD_ACK_ERROR;
			}
			else if(size==3)
			{
				PUser u;
				TZKFPTemplate tmp;
				int ret;
				u=FDB_GetUser(get_unaligned((U16*)p),NULL);
				if(u && (ret=FDB_GetTmp(u->PIN,p[2],(char *)&tmp)))
				{
					if(gOptions.ZKFPVersion == ZKFPV10)
					{
						SendLargeData((void*)chdr, session, (char *)tmp.Template, ret);
					}
					else
					{
						memcpy(p,tmp.Template, tmp.Size);
						len+=tmp.Size;
						chdr->SessionID=tmp.Size;
					}
				}
				else
					chdr->Command=CMD_ACK_ERROR;
			}		
			break;	
			//For face
#ifdef FACE
		case CMD_USERFACE_RRQ:
			{
				if (!gOptions.FaceFunOn)
				{
					chdr->Command=CMD_ACK_ERROR;
					break;
				}
				//printf(" CMD_USERFACE_RRQ size: %d \n",size);
				if(size==0)
				{
					if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_USER)))
						chdr->Command=CMD_ACK_ERROR;
				}
				else if(size==1)
				{
					if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, *p)))
						chdr->Command=CMD_ACK_ERROR;
				}
				else if((size==3) || (size==28))
				{
					TFaceTmp facetmp;
					char acno[24];
					TUser ssruser;
					int tmplen=16*1024;
					char *tmpdatabuf=(char*)MALLOC(MAX_BUFFER_SIZE);
					int dbret=0,facecount=0;
					memset(acno,0,24);
					nmemcpy((BYTE *)acno,(BYTE *)p,24);
					if (size==3)
						dbret=FDB_GetUser(get_unaligned((U16*)p),&ssruser);
					else if (size==28)
						dbret=FDB_GetUserByCharPIN2(acno, &ssruser); 
					printf("UserID:%d dbret:%d\n",ssruser.PIN,dbret);
					if((((size==3)&&(p[2]>=FACE_NUM)) ||((size==28)&&(p[25]>=FACE_NUM))) && dbret \
							//&& (tmplen=ProcessGetFaceTmp(ssruser.PIN,tmpdatabuf,tmplen)))
						&&(tmplen=FDB_GetUserFaceTemps(ssruser.PIN,tmpdatabuf)))
						{
							tmplen=tmplen*sizeof(TFaceTmp);
							// printf("templen=0x%x\n",tmplen);
							if(tmplen<1024)
							{
								memcpy(p,tmpdatabuf, tmplen);
								len+=tmplen;
								chdr->SessionID=tmplen;
							}
							else
							{
								SendLargeData((void*)chdr, session, tmpdatabuf, tmplen);
								//printf("Send IDKIT template finish\n");
							}
						}
					else
						chdr->Command=CMD_ACK_ERROR;
					FREE(tmpdatabuf);
				}
				break;
			}	
#endif
			//end face

		case CMD_DELETE_USER:
			if(FDB_OK!=FDB_DelUser(get_unaligned((U16*)p))) chdr->Command=CMD_ACK_ERROR;
			//			printf("FDB_DelUser %d\n",chdr->Command);
			break;
			/*
			   2.CMD_SSRDELETE_USER 删除用户 以工号为参数
			   */
		case CMD_SSRDELETE_USER:
			{
				char acno[24];
				PUser ssruser=NULL;

				//                        printf("CMD_SSRDELETE_USER\n");
				memset(acno,0,24);
				nmemcpy((BYTE *)acno,(BYTE *)p,24);
				ssruser=FDB_GetUserByCharPIN2(acno, NULL);
				if (ssruser)
				{
					if(FDB_OK!=FDB_DelUser(ssruser->PIN))
						chdr->Command=CMD_ACK_ERROR;
				}
				else
					chdr->Command=CMD_ACK_ERROR;
				break;
			}

		case CMD_DELETE_EXTUSER:
			if(FDB_OK!=FDB_DelExtUser(get_unaligned((U16*)p))) chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_DELETE_WorkCode:
			{
				PWorkCode pwc=NULL;
				char tmpCode[MAX_WORKCODE_LEN+1];
				nmemcpy((BYTE *)tmpCode, (BYTE *)p, MAX_WORKCODE_LEN);
				pwc = FDB_GetWorkCodeByCode(tmpCode, NULL);
				if (pwc)
				{
					if (FDB_DelWorkCode(pwc->PIN) != FDB_OK)
						chdr->Command = CMD_ACK_ERROR;
				}
				else
					chdr->Command = CMD_ACK_ERROR;
				break;
			}
		case CMD_DELETE_SMS:
			if(FDB_OK!=FDB_DelSms(get_unaligned((U16*)p))) chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_DELETE_UDATA:
			if(size==sizeof(TUData))
			{
				TUData udata;
				memcpy((void*)&udata, p, sizeof(TUData));

				//解决PC下传短消息时用户分配出现冲突的现象2007.7.26
				//清除已分配了短消息的用户UData记录
				if(FDB_OK!=FDB_DelUData(udata.PIN, 0))
					chdr->Command=CMD_ACK_ERROR;
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_CLEAR_ADMIN:
			{
				int kk=FDB_ClrAdmin();
				if(FDB_OK!=kk) chdr->Command=CMD_ACK_ERROR;
				FDB_AddOPLog(ADMINPIN, OP_CLEAR_ADMIN, 0, kk, 0, 0);
				break;
			}
		case CMD_REFRESHDATA:
			sync();
			FDB_InitDBs(FALSE);
			//if(gOptions.ZKFPVersion!=ZKFPV10)//zsliu
			if(gOptions.IsOnlyRFMachine==0)
			{
				FPInit(NULL);
			}

			if(gOptions.IsSupportSMS) CheckBoardSMS();
			break;
		case CMD_DELETE_USERTEMP:
			chdr->Command=CMD_ACK_ERROR;
			if(FDB_GetUser(get_unaligned((U16*)p),&user))
			{
				//i=FDB_DeleteTmps(user.PIN);
				i=FDB_DelTmp(user.PIN,p[2]); //changed by cxp at 2010-04-16
				if(i==FDB_ERROR_NODATA) chdr->SessionID=2;
				else if(i!=FDB_OK) chdr->SessionID=1;
				else chdr->Command=CMD_ACK_OK;
			}
			else
				chdr->SessionID=2;
			break;
			//CMD_SSRDELETE_USERTEMP 删除用户模板 当指纹ID=13时一次删除所有模板
		case CMD_SSRDELETE_USERTEMP:
			{
				char acno[24];
				TUser ssruser;
				int fingerid=0;

				memset(&ssruser,0,sizeof(TUser));
				memset(acno,0,24);
				nmemcpy((BYTE *)acno,(BYTE *)p,24);
				chdr->Command=CMD_ACK_ERROR;
				if (FDB_GetUserByCharPIN2(acno,&ssruser))
				{
					fingerid=(int)p[24];
					if(fingerid ==13)////删除所有指纹数据
					{
						i=FDB_DeleteTmps(ssruser.PIN);
					}
					else
					{
						i=FDB_DelTmp(ssruser.PIN,p[24]);
					}
					if(i==FDB_ERROR_NODATA)
					{
						chdr->SessionID=2;
						chdr->Command=CMD_ACK_OK;
					}
					else if(i!=FDB_OK) chdr->SessionID=1;
					else chdr->Command=CMD_ACK_OK;
				}
				else
					chdr->SessionID=2;
				break;
			}
			//For face
#ifdef FACE
		case CMD_DELETE_USERFACE:
			{
				char acno[24];
				TUser ssruser;

				printf("CMD_SSRDELETE_USERTEMP\n");
				memset(&ssruser,0,sizeof(TUser));
				memset(acno,0,24);
				nmemcpy((BYTE *)acno,(BYTE *)p,24);
				chdr->Command=CMD_ACK_ERROR;
				if (!gOptions.FaceFunOn)
					break;
#if 1
				printf("%s acno:%s \n",__FUNCTION__,acno);
				if (FDB_GetUserByCharPIN2(acno,&ssruser))
				{
					int i;
					printf("%s ssruser.PIN:%d \n",__FUNCTION__,ssruser.PIN);
					i=FDB_DeleteFaceTmps(ssruser.PIN);
					if(i==FDB_ERROR_NODATA) chdr->SessionID=2;
					else if(i!=FDB_OK) chdr->SessionID=1;
					else chdr->Command=CMD_ACK_OK;
				}
#else
				if(FDB_GetUser(get_unaligned((U16*)p),&user))
				{
					int i;
					i=FDB_DeleteFaceTmps(user.PIN);
					if(i==FDB_ERROR_NODATA) chdr->SessionID=2;
					else if(i!=FDB_OK) chdr->SessionID=1;
					else chdr->Command=CMD_ACK_OK;
				}
#endif
				else
					chdr->SessionID=2;

				break;
			}
#endif
			//end face

		case CMD_TEST_TEMP:
			{
				int result,score=55;
				PUser u;
				if( fhdl && BIOKEY_IDENTIFYTEMP(fhdl, (BYTE*)p, &result, &score))
				{
					u=FDB_GetUser((U16)result, NULL);
					if(u)
					{
						memcpy(p, &u->PIN, 2);
					}
				}
				else
					*p=0;
				len+=2;
				break;
			}
			/*dsl 2012.4.23*/
#if 0
		case CMD_RESTART:
			{
				pid_t pid=-1;
				if ((pid=fork())==0)  //changed by cxp at 2010-04-20
				{
					sleep(2);
					//ExSetPowerOnTime(gCurTime.tm_hour, gCurTime.tm_min);
					//DelayMS(10);
					//ExPowerRestart();
					RebootMachine();
				}
				break;
			}
		case CMD_POWEROFF:
			{
				//DBPRINTF("Power off!\n");
				pid_t pid=-1;
				if ((pid=fork())==0)  //changed by cxp at 2010-04-20
				{
					sleep(2);
					ExPowerOff(FALSE);
				}
			}
			break;
		case CMD_SLEEP:
			{
				pid_t pid=-1;
				if ((pid=fork())==0)  //changed by cxp at 2010-04-20
				{
					sleep(2);
					ExPowerOff(TRUE);
					_exit(0); //dsl 2012.4.6
				}
			}
			break;
#endif
		case CMD_GET_FREE_SIZES:
			{
				int Flag=0;
				int Len=FDB_GetSizes(p);
				if(size>0) Flag=*(BYTE*)p;
				if((size>0) && ((U32)Flag<Len/sizeof(int))) {
					memcpy(p, p+Flag*sizeof(int),sizeof(int));
					len+=sizeof(int);
				} else {
					len+=Len;
				}
				break;
			}
		case CMD_CAPTUREFINGER:
			{
				char *sbuf=gImageBuffer, *dbuf=gImageBuffer, *img;
				int i=0,j,w=gOptions.ZF_WIDTH,c=gOptions.OImageHeight*gOptions.OImageWidth;
				if(size>=sizeof(int))
				{
					dbuf=(char*)MALLOC(512*512);
					i=*(int*)p;
					if(500==i)
					{
						if(size>sizeof(int))
							w=((int*)p)[1];
					}
					else if(501==i)
						sbuf=gImageBuffer+c*4;
					else if(502==i)
					{
						w=gOptions.OImageWidth;
						sbuf=gImageBuffer+c*4;
					}
				}
				if(sbuf==gImageBuffer && !TestEnabledMsg(MSG_TYPE_FINGER))
					CaptureSensor(gImageBuffer, SENSOR_CAPTURE_MODE_STREAM, &SensorBufInfo);
				if(dbuf!=gImageBuffer)
				{
					*(int *)dbuf=500;
					((int *)dbuf)[1]=w;
					((int *)dbuf)[2]=i==502?gOptions.OImageHeight:w*gOptions.ZF_HEIGHT/gOptions.ZF_WIDTH;
					c=((int *)dbuf)[1]*((int *)dbuf)[2]+sizeof(int)*3;
					if(i!=502)
					{
						memcpy((BYTE*)sbuf,(BYTE*)dbuf+sizeof(int)*3,SensorBufInfo.DewarpedImgLen);
						if(w!=gOptions.ZF_WIDTH)
						{
							dbuf+=sizeof(int)*3;
							img=sbuf;
							for(i=0;i<gOptions.ZF_HEIGHT*w/gOptions.ZF_WIDTH;i++)
							{
								char *row=dbuf+gOptions.ZF_WIDTH*(i*gOptions.ZF_WIDTH/w);
								for(j=0;j<w;j++)
								{
									*img=row[j*gOptions.ZF_WIDTH/w];
									img++;
								}
							}
							memcpy(dbuf, sbuf, c);
							dbuf-=sizeof(int)*3;
						}
					}
					else
						memcpy((BYTE*)dbuf+sizeof(int)*3,sbuf,((int *)dbuf)[1]*((int *)dbuf)[2]);
				}
				SendLargeData((void*)chdr, session, dbuf, c);
				if(dbuf!=gImageBuffer) FREE(dbuf);
			}
			break;
		case CMD_CAPTUREIMAGE:
			{
				int is=CMOS_WIDTH*CMOS_HEIGHT;
				char *image=MALLOC(is);
				int mtemp;
				InitSensor(&gOptions.ZF_WIDTH,&gOptions.ZF_HEIGHT,&mtemp);
				CaptureSensor(image, SENSOR_CAPTURE_MODE_STREAM, &SensorBufInfo);
				CaptureSensor(image, SENSOR_CAPTURE_MODE_STREAM, &SensorBufInfo);
				if(size>=sizeof(int))
				{
					int width=*(int*)p;
					int i,j;
					char *img=gImageBuffer, *row;
					if(width*CMOS_HEIGHT*width/CMOS_WIDTH<=5*gOptions.OImageWidth*gOptions.OImageHeight)
					{
						is=width*CMOS_HEIGHT*width/CMOS_WIDTH;
						for(i=0;i<CMOS_HEIGHT*width/CMOS_WIDTH;i++)
						{
							row=image+CMOS_WIDTH*(i*CMOS_WIDTH/width);
							for(j=0;j<width;j++)
							{
								*img=row[j*CMOS_WIDTH/width];
								img++;
							}
						}
						((int*)image)[0]=500*width/CMOS_WIDTH;
						((int*)image)[1]=width;
						((int*)image)[2]=CMOS_HEIGHT*width/CMOS_WIDTH;
						memcpy(image+3*sizeof(int), gImageBuffer, is);
						is+=3*sizeof(int);
					}
				}
				SendLargeData((void*)chdr, session, image, is);
				DBPRINTF("Send IMAGE OK! Length=%d\n", is);
				InitSensor(&gOptions.ZF_WIDTH,&gOptions.ZF_HEIGHT,&mtemp);
				FREE(image);
				CaptureSensor(gImageBuffer, SENSOR_CAPTURE_MODE_STREAM, &SensorBufInfo);
			}
			break;
		case CMD_CLEAR_ATTLOG:
			{
				int kk=FDB_ClrAttLog();
				FDB_AddOPLog(ADMINPIN, OP_CLEAR_LOG, 0, kk, 0, 0);
				break;
			}
		case CMD_CLEAR_OPLOG:
			FDB_ClrOPLog();
			break;
		case CMD_CLEAR_DATA:
			if(size==1) {
				FDB_ClearData(*p);
			} else {
				int kk;
				char commandstr[100];

				memset(commandstr,0,sizeof(commandstr));
				kk=FDB_ClearData(FCT_ALL);
				FDB_AddOPLog(ADMINPIN, OP_CLEAR_DATA, 0, kk, 0, 0);
				if (gOptions.CameraOpen)
				{
					//zsliu add 2009-10-14
					//删除考勤照片时直接删除文件夹，避免删除3000以上照片时，提示Arugument list too long的错误，并且无法删除。
					sprintf(commandstr, "rm -rf %s/ && sync", GetCapturePath("capture/pass"));
					systemEx(commandstr);
					sprintf(commandstr,"mkdir %s && sync",GetCapturePath("capture/pass"));//再创建 pass 文件夹。
					systemEx(commandstr);
					FDB_ClearIndex(0);
					sync();

					memset(commandstr, 0, sizeof(commandstr));

					sprintf(commandstr, "rm -rf %s/ && sync", GetCapturePath("capture/bad"));
					systemEx(commandstr);
					sprintf(commandstr,"mkdir %s && sync",GetCapturePath("capture/bad"));//再创建 bad 文件夹。
					systemEx(commandstr);
					FDB_ClearIndex(1);
					sync();
					//add end
				}
				//clear webserver index file
				unlink("./dpm.dat");
				unlink("./dpmidx.dat");
				extern int gCurCaptureCount;
				gCurCaptureCount = FDB_CntPhotoCount();
			}
			break;
		case CMD_OPTIONS_RRQ:
			{
				char value[1024],name[64];
				int l,temp;
				l=strlen(p);
				strcpy(name,p);
				if(size==2)
				{
					chdr->Command=CMD_ACK_ERROR;
				}
				else if(0==(i=GetOptionNameAndValue(p, size)))
				{
					chdr->Command=CMD_ACK_ERROR;
				}
				else
				{
					strcpy(value,p+l+1);
					temp=atoi(value);
					//add by cxp
					if(strcmp(name,"HiSpeedNet")==0)
					{
						if (temp==2)
						{
							temp=8;
							sprintf(value,"%d",temp);
							strcpy(p+l+1,value);
						}
					}
					//add end
					len+=i;
				}
			}
			break;
		case CMD_OPTIONS_WRQ:
			{
				char *value=p;
				int namel=0;

				p[size]=0;
				while(*value)
				{
					if('='==*value++)
						break;
					if(namel++>size) break;
				}
				if(namel>=size || namel<1)
				{
					chdr->Command=CMD_ACK_ERROR;
					chdr->SessionID=1;
				}
				else
				{
					p[namel]=0;
					//dsl 2012.3.23, remove the encrypt options function.

					//zsliu 设置不需要重启即刻生效
					if (strcmp(p, "IPAddress")==0) 
					{
						char c1[16],c2[16],c3[16],c4[16];
						int len=0;
						if(ParseIP(value,c1,c2,c3,c4))
						{
							len=strlen(value);
							len=len<16?len:16;
							memcpy(SaveIPAddress,value,len);
							gOptions.IPAddress[0]=atoi(c1);
							gOptions.IPAddress[1]=atoi(c2);
							gOptions.IPAddress[2]=atoi(c3);
							gOptions.IPAddress[3]=atoi(c4);
							RemoteSaveStr("IPAddress",value,TRUE);
						}
					}
					//zsliu end
					else if(strcmp(p,"~ZKFPVersion")==0)
					{
						SaveInteger("ChangeVersion", atoi(value));
					}
					else if(!SaveStr(p,value,TRUE))
					{
						chdr->Command=CMD_ACK_ERROR;
						chdr->SessionID=2;					
					}
				}
			}
			break;
		case CMD_ATTLOG_RRQ:
			printf("%s, CMD_ATTLOG_RRQ, row:%d\n", __FUNCTION__, __LINE__);
			if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_ATTLOG)))
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_RULE_RRQ:
			if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_ATTRULE)))
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_DEPT_RRQ:
			if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_DEPT)))
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_SCH_RRQ:
			if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_SCH)))
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_GET_PINWIDTH:
			p[0]=gOptions.PIN2Width;
			len++;
			break;		
		case CMD_GET_IOSTATUS:
			p[0]=ExGetIOStatus();
			len++;
			break;
		case CMD_OPLOG_RRQ:
			if(!(chdr->SessionID=ReadBlockAndSend(chdr, session, FCT_OPLOG)))
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_SET_TIME:
			{
				TTime t;
				memcpy(&i,p,4);
				OldDecodeTime((time_t)i, &t);
				SetTime(&t);			
				GetTime(&gCurTime);
				ShowMainLCD(hMainWindowWnd);
			}
			break;
		case CMD_GET_TIME:
			{
				time_t tmp=OldEncodeTime(&gCurTime);
				memcpy(p, &tmp, sizeof(time_t));
				len+=sizeof(time_t);
			}
			break;
		case CMD_ENABLE_CLOCK:
			ClockEnabled=*p;
			if(*p)
				ShowMainLCD(hMainWindowWnd);
			break;
		case CMD_REFRESHOPTION:
			{
				if((gOptions.RS485On || gOptions.RS232On) && (gOptions.RS232BaudRate!=LoadInteger("RS232BaudRate",115200) || gOptions.DeviceID!=LoadInteger("DeviceID",1)))	//if don't so,it will hung up at 232/485 communication
					break;
				LoadOptions(&gOptions);

				if(gOptions.IsOnlyRFMachine==0)
				{
					FPInit(NULL);
				}
			}
			break;
		case CMD_TESTVOICE:
			if(size==2)
				ExPlayVoiceFrom(p[0],p[1]);
			else
				ExPlayVoice(p[0]);
			break;
		case CMD_SETFPDIRECT:
			gFPDirectProc=*(BYTE*)p;
			if(gFPDirectProc==255)
				gFPDirectProc=-1;
			else if(gFPDirectProc==254)
				gFPDirectProc=-2;
			else if(gFPDirectProc==253)
				gFPDirectProc=-3;
			break;
		case CMD_GET_VERSION:
			if(size==1 && 0==*p)
			{
				if(LoadStrOld(PLATFORM))
					strcpy(p, LoadStrOld(PLATFORM));
			}
			else
				strcpy(p,MAINVERSION); // don't modify,if modify PLS test SDK   jazzy 2009.01.23
			len+=strlen(p)+1;
			break;
		case CMD_QUERY_FIRWARE: 
			break; 
		case CMD_HASH_DATA:
			if(size==4)
			{
				if(GETDWORD(p)!=hashBuffer(session->Buffer))
					chdr->Command=CMD_ACK_ERROR;
			}
			else if(size==0)
			{
				SET_DWORD(p, hashBuffer(session->Buffer), 0);
				len+=4;
			}
			break;
		case CMD_UPDATE_READALL:
			if(size==1)
			{
				if(*p==FCT_ATTLOG)
					FDB_SetAttLogReadAddr(LOG_READ_ALL);
				else if(*p==FCT_OPLOG)
					FDB_SetOpLogReadAddr(LOG_READ_ALL);
				else
					chdr->Command=CMD_ACK_ERROR;
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_READ_NEW:
			if(size>0)
			{
				int Size=BYTE2M;
				char *data=NULL;
				if(FCT_ATTLOG==*p)
					data=FDB_ReadNewAttLogBlock((int*)&Size);
				else if(FCT_OPLOG==*p)
					data=FDB_ReadNewOpLogBlock((int*)&Size);
				if(data)
				{
					SendLargeData((void*)chdr, session, data, Size);
					FREE(data);
				}
				else
					chdr->Command=CMD_ACK_ERROR;
			}
			else 
				chdr->Command=CMD_ACK_ERROR;
			break;		
		case CMD_UPDATE_FIREWARE: 
			chdr->Command=CMD_ACK_ERROR; 
			/*change by zxz 2013-01-14*/
			if(!isWriteFlashTmp) {
				if(session->Buffer->bufferSize<=0) {
					chdr->SessionID=1; 
					break;
				}
				else if(NULL==session->Buffer) {
					chdr->SessionID=2; 
					break;
				}
				else if(session->Buffer->bufPtr>session->Buffer->bufEnd) {
					chdr->SessionID=3; 
					break;
				}
			}
			
			{ 
				U32 C=get_unaligned((U32*)p);
				char sFileName[128], sFirmwareFiles[128];
				U32 A=0;
				U32 Addr=get_unaligned((U32*)p+1);
				U32 L;
				/*change by zxz 2013-01-14*/
				if(isWriteFlashTmp) {
					L = gBatchDataLen;
				} else {
					L=session->Buffer->bufferSize;
				}

				//DBPRINTF("CMD_UPDATE_FIRMWARE C=%d L=%d Addr=%x\n", C, L, Addr);
				switch(C) 
				{ 
					case UPDATE_FONT: 
						GetEnvFilePath("USERDATAPATH", "hz2.dat.gz", sFileName);
						A=1;
						break; 
					case UPDATE_OPTIONS: 
						GetEnvFilePath("USERDATAPATH", "options.cfg", sFileName);
						A=1;
						break; 
					case UPDATE_TEMPS: 
						GetEnvFilePath("USERDATAPATH", "data/template.dat", sFileName);
						A=1;
						break; 
					case UPDATE_USERS: 
						GetEnvFilePath("USERDATAPATH", "data/ssruser.dat", sFileName);
						A=1;
						break; 
					case UPDATE_FIRMWARE: 
						GetEnvFilePath("USERDATAPATH", "main.gz", sFileName);
						A=1;
						break; 
					case UPDATE_CFIRMWARE: 
						//考虑ramdisk升级空间不够，直接升级到/mnt/ramdisk1/
						sprintf(sFileName,"%supdate.tgz",RAMPATH);
						if((((BYTE *)session->Buffer->buffer)[0]==0x1f)&&(((BYTE *)session->Buffer->buffer)[1]==0x8b))
							A=1;
						break; 
					case UPDATE_BOOTLOADER:
						A=1; 
						break; 
					case UPDATE_FLASH: 
						A=1;
						break; 					
					case UPDATE_SOUND:
						strcpy(sFileName, "/tmp/res.tgz");
						A=1;
						break;
					case UPDATE_ALGORITHM: 
						GetEnvFilePath("USERDATAPATH", "libzkfp.so.3.5.1", sFileName);
						A=1;
						break; 		
					case UPDATE_LANGUAGE: 
						strcpy(sFileName, "/tmp/language.tgz");
						A=1;
						break; 		
					case UPDATE_AUTOSHELL: 
						GetEnvFilePath("USERDATAPATH", "auto.sh", sFileName);
						A=1;
						break; 
					case UPDATE_BATCHUSERDATA: 
						A=1;
						break; 				
						//2008.09.05 支持自定义文件上传功能(包括宣传图片,用户照片)
					case CMD_UPDATEFILE:
						{
							char sTmp[40];
							char sTmp1[10];

							memset(sTmp,0,40);
							memset(sTmp1,0,10);
							//DBPRINTF("FileName=%s\n", p+4);
							memcpy(sTmp, p+4, sizeof(sTmp));
#ifdef FORFACTORY
							/*To upload a file to Mtdblock directory*/
							GetEnvFilePath("USERDATAPATH", sTmp, sFileName);
							printf("fliename=%s\n", sFileName);
							A=1;
							break;
#endif

							//GetEnvFilePath("USERDATAPATH", sTmp, sFileName);
							//printf("sTmp1:%s\n",sTmp1);
							if (strstr(sTmp, ".jpg")) {
								if (strncmp(sTmp, "ad_", 3) == 0) {
									sprintf(sFileName,"%s%s",GetPicPath("adpic/"),sTmp);
								} else {
									sprintf(sFileName,"%s%s",GetPicPath("photo/"),sTmp);
								}
							} else {
								GetEnvFilePath("USERDATAPATH", sTmp, sFileName);
							}
							//printf("_________%s-%d____sFileName = %s\n", __FILE__, __LINE__, sFileName);
							//printf("sTmp1:%s\tsfilename:%s\n",sTmp1,sFileName);
							A=1;
							break;
						}
					case UPDATE_UDISKUPGRADEPACKAGE:
						{
							A = 1;
							break;
						}
					default: 
						A=0; 
				} 
				if(A==1) 
				{  
					if(C==UPDATE_FLASH)
					{
						//DBPRINTF("Addr=%x L=%d\n", (U32)gFlash16+Addr, L);
						if(SaveLOGOToFlash(Addr, (char *)session->Buffer->buffer , L))
							chdr->Command=CMD_ACK_OK;
						else
							chdr->SessionID=4; 	
						DBPRINTF("UPDATE FLASH OK!\n");
					}
					else if(C==UPDATE_BOOTLOADER)
					{
						if(SaveUBOOTToFlash((char *)session->Buffer->buffer, L))
							chdr->Command=CMD_ACK_OK;
						else
							chdr->SessionID=4;
					}
					else if(C==UPDATE_BATCHUSERDATA)
					{	
						unsigned int s_msec, e_msec;	

						s_msec=GetTickCount();
						/*change by zxz 2013-01-14*/
						if(isWriteFlashTmp == 1) {
							printf("gFinalBatchDataLen = %d, gBatchDataLen= %d\n", gFinalBatchDataLen, gBatchDataLen);
							ProcessBatchData();
							close(fdbatchdata);
							remove("/mnt/mtdblock/batch.dat");
							fdbatchdata = -1;
							isWriteFlashTmp = 0;
						} else {
							BatchOPUserData(session->Buffer->buffer);
						}
						e_msec=GetTickCount();
						DBPRINTF("User Data Batch Command FINISHED! time = %d\n", e_msec - s_msec);

						chdr->Command=CMD_ACK_OK; 
					}
					else if(C == UPDATE_UDISKUPGRADEPACKAGE)
					{
						char *filebuf = (char*)session->Buffer->buffer;
						int fd = -1;
						char buf[128], sFirmwareFiles[128];
						memset(buf,0,sizeof(buf));
						memset(sFirmwareFiles,0,sizeof(sFirmwareFiles));
						if(in_chksum((unsigned char*)filebuf, UDiskLength) == UDiskCheckSum)
						{
							if((((BYTE *)filebuf)[0]==0x1b)&&(((BYTE *)filebuf)[1]==0x55))
							{
								sprintf(buf,"%supdate.tgz",RAMPATH);
								//save to file
								fd=open(buf, O_CREAT|O_WRONLY|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
								if (fd>0)
								{
									printf("ExtractPakage \n");
									zkfp_ExtractPackage(filebuf,NULL,(int*)UDiskLength);
									write(fd, filebuf, UDiskLength);
									close(fd);
									if (!useSDCardFlag)
										sprintf(sFirmwareFiles, "tar xvzf %s -C %s && sync && rm %s -rf",buf,"/mnt/mtdblock/",buf);
									else
										sprintf(sFirmwareFiles, "tar xvzf %s -C %s && sync && rm %s -rf",buf,"/mnt/sdcard/",buf);

										if (systemEx(sFirmwareFiles)==EXIT_SUCCESS)
											chdr->Command=CMD_ACK_OK;
										else
											printf("system failed====\n");
								}
							}
						}
					}
					else
					{
						int fd=open(sFileName, O_CREAT|O_RDWR|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
						if (fd>0)
						{
							//通过SDK不需要加密处理zsliu change
							//zkfp_ExtractPackage((char *)session->Buffer->buffer,NULL,(int *)session->Buffer->bufferSize);
							if(write(fd, session->Buffer->buffer, session->Buffer->bufferSize)==session->Buffer->bufferSize)
							{
								close(fd);
								if(C==UPDATE_CFIRMWARE)
								{
									if (!useSDCardFlag)								
										sprintf(sFirmwareFiles, "tar xvzf %s -C %s && sync && rm %s -rf", sFileName, "/mnt/mtdblock/", sFileName);
									else
										sprintf(sFirmwareFiles, "tar xvzf %s -C %s && sync && rm %s -rf", sFileName, "/mnt/sdcard/", sFileName);
									systemEx(sFirmwareFiles);
									//DBPRINTF("sFirmwarFiles: %s\n",sFirmwareFiles);
								}
								else if(C==UPDATE_LANGUAGE)
								{
									sprintf(sFirmwareFiles, "rm %sLANGUAGE.* -rf && tar xvzf %s -C %s && sync && rm %s -rf","/mnt/mtdblock/data/", sFileName, "/mnt/mtdblock/data/", sFileName);
									systemEx(sFirmwareFiles);
								}
								else if(C==UPDATE_SOUND)
								{
									sprintf(sFirmwareFiles, "tar xvzf %s -C %s && sync && rm %s -rf", sFileName, getenv("USERDATAPATH"), sFileName);

									systemEx(sFirmwareFiles);
								}
								chdr->Command=CMD_ACK_OK;
							}
							else
							{
								close(fd);
								sprintf(sFirmwareFiles, "rm %s -rf", sFileName);
								systemEx(sFirmwareFiles);
								chdr->SessionID=4;
							}
						}
						else						
							chdr->SessionID=4; 				
					}
				} 
				else 
					chdr->SessionID=5; 
			} 
			break; 		
		case CMD_FREE_DATA:
			if (session->Buffer && session->Buffer->bufferSize>0) {
				freeBuffer(session->Buffer);
			}
			session->Buffer=NULL;

			if (session->fdCacheData) {
				char fdbuf[128];
				memset((void*)fdbuf, 0, sizeof(fdbuf));
				close(session->fdCacheData);
				session->fdCacheData = 0;
			}
			break;

		case CMD_PREPARE_DATA:
			if (isWriteFlashTmp == 1) {
				fdbatchdata = open("/mnt/mtdblock/batch.dat", O_CREAT|O_RDWR|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
				if(fdbatchdata > 0){
					gFinalBatchDataLen = 0;
					gBatchDataLen = GETDWORD(p);
					chdr->Command = CMD_ACK_OK;
				} else {
					chdr->Command = CMD_ACK_ERROR;
				}
			} else {
				freeBuffer(session->Buffer);
				session->Buffer=createRamBuffer(GETDWORD(p));
				if(session->Buffer)
				{
					chdr->Command = CMD_ACK_OK;
				}
				else
				{
					chdr->Command = CMD_ACK_ERROR;
				}
			}
			break;
		case CMD_DATA:
			if(isWriteFlashTmp == 1) {
				if(write(fdbatchdata,p,size) == size)
				{
					gFinalBatchDataLen += size;
				} else {
					chdr->Command=CMD_ACK_ERROR;
				}
			} else {
				i=session->Buffer->bufEnd-session->Buffer->bufPtr;
				if((session->LastReplyID+1==chdr->ReplyID) && (i>=size) && session->Buffer)
				{
					memcpy((char*)(session->Buffer->bufPtr), p, size);
					session->Buffer->bufPtr+=size;
				}
				else
					chdr->Command=CMD_ACK_ERROR;
				//printf("session->BufferLen=%d BufferPos=%d\n", session->BufferLen, session->BufferPos);
			}
			break;
		case CMD_QUERY_DATA:
			{
				int RealSize=0;
				int DataType = 0;
				//DBPRINTF("CMD_QUERY_DATA, size=%d\n", size);
				freeBuffer(session->Buffer);
				session->Buffer=NULL;

				DataType = GETWORD(p+1);
				if(size==11) {
					session->Buffer=QueryData(p, GETWORD(p+1), p+3, &RealSize, session);
				}

				if(session->Buffer){
					if(session->Buffer->bufferSize<=1024){//Less than 1K send directly
						len+=session->Buffer->bufferSize;
						memcpy(p, (char*)(session->Buffer->buffer), session->Buffer->bufferSize);
						chdr->Command=CMD_DATA;
					} else if (session->fdCacheData > 0 && DataType==CMD_ATTLOG_RRQ){
						/*dsl 2012.5.21, optimizes to send large attendence log. for example: support 20W records.
						 *0-------------1-----------5---------------9---------13---------------
						 *|compress flag|buffer size|attendence size|hash value|attendence data|
						 */
						unsigned char tmpstr[4]={0};
						//printf("bufferSize:%d, RealSize=%d\n", session->Buffer->bufferSize, RealSize);
						SET_DWORD(p, session->Buffer->bufferSize, 1);
						SET_DWORD(p, RealSize, 1+4);

						/*Copy the attendence file size to buffer for create a hash value*/
						SET_DWORD(tmpstr,session->Buffer->bufferSize-4,0);
						unsigned int hash = hashBufferLog(SelectFDFromConentType(FCT_ATTLOG), tmpstr, 4);
						SET_DWORD(p, hash, 1+4+4);

						len+=1+4+4+4;
					} else {
						SET_DWORD(p, session->Buffer->bufferSize, 1);
						SET_DWORD(p, RealSize, 1+4);
						SET_DWORD(p, hashBuffer(session->Buffer), 1+4+4);
						len+=1+4+4+4;
					}
				} else {
					chdr->Command=CMD_ACK_ERROR;
				}
				break;
			}
		case CMD_READ_DATA:
			{
				int Size=GETDWORD(p+4);
				int Addr=GETDWORD(p);
				printf("Receive CMD_READ_DATA, size=%d, addr=%d\n", Size, Addr);
				if (size==8 && session->Buffer && session->Buffer->bufferSize>=Addr+Size) {
					if (Size<=1024) { 
						/*Less than 1K send directly*/
						if (session->fdCacheData > 0) {
							SendLargeDataByFile((void*)chdr, session, Addr, Size);
						} else {
							len+=Size;
							memcpy(p, (char*)(session->Buffer->buffer)+Addr, Size);
							chdr->Command=CMD_DATA;
						}
					} else if (session->fdCacheData > 0) {
						SendLargeDataByFile((void*)chdr, session, Addr, Size);
					} else {
						printf("_______%s%d\n", __FILE__, __LINE__);
						SendLargeData((void*)chdr, session, (char*)(session->Buffer->buffer)+Addr, Size);
					}
				} else {
					chdr->Command=CMD_ACK_ERROR;
				}
				break;
			}				
		case CMD_USERGRP_RRQ:			//读用户分组
			*p=GetUserGrp(get_unaligned((U32*)p));
			len+=1;
			break;

		case CMD_USERGRP_WRQ:			//设置用户分组???????????????????????????
			{
				U8 gid =p[4];  //changed by cxp at 2010-04-21
				TGroup group;
				if(gOptions.LockFunOn<2)	//仅仅高级门禁功能支持该命令
					chdr->Command=CMD_ACK_ERROR;
				else
				{
					if(FDB_GetGroup(gid,&group)!=NULL)
					{
						if(!SaveUserGrp(get_unaligned((U32*)p), p[4]))
							chdr->Command=CMD_ACK_ERROR;
					}
					else
						chdr->Command=CMD_ACK_ERROR;
				}
			}

			break;
		case CMD_USERTZ_RRQ:		//读用户时间段设置
			{
				TUser tu;

				if(gOptions.LockFunOn<2)
					chdr->Command=CMD_ACK_ERROR;
				else
				{
					memset(&tu, 0, sizeof(TUser));
					if(FDB_GetUser(get_unaligned((U16*)p), &tu)!=NULL)
					{
						len+=8;
						/*//delete the stupid code 2013-07-02
						if (tu.TimeZones[0]==0)  //add by cxp at 2010-04-21
							tu.TimeZones[0]=1;
						else
							tu.TimeZones[0]=0;
							*/
						memcpy(p, (void*)&tu.TimeZones, 8);
					}
					else
						chdr->Command=CMD_ACK_ERROR;
				}
				break;	
			}		
		case CMD_GRPTZ_RRQ:		//读组时间段设置
			if(FDB_GetGroup(get_unaligned((U8*)p), &gp))
			{
				len+=sizeof(TGroup);
				memcpy(p, (void*)&gp, sizeof(TGroup));
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;

		case CMD_USERTZ_WRQ:			//写用户时间段设置
			if(gOptions.LockFunOn<2)        //仅仅高级门禁功能支持该命令
				chdr->Command=CMD_ACK_ERROR;
			else
			{
				U32 TZs[10];
				memcpy(TZs, p+4, size);
				/*not necessary 2013-07-02
				if(TZs[0]==0)	//使用组时间段
				{
					for(i=1;i<4;i++)
						TZs[i]=0;	//清除自定义时间段
				}
*/
				SaveUserTZ(get_unaligned((U32*)p),(int*)TZs);
			}
			break;

		case CMD_GRPTZ_WRQ:			//写组时间段设置
			{
				int i;
				int errcount=0;

				if(gOptions.LockFunOn<2)
					chdr->Command=CMD_ACK_ERROR;
				else
				{
					chdr->Command=CMD_ACK_ERROR;
					if((size==sizeof(TGroup)))
					{
						nmemcpy((BYTE *)&gp, (BYTE *)p, sizeof(TGroup));
						for(i=0;i<3;i++)
						{
							if(gp.TZID[i] && !FDB_GetTimeZone(gp.TZID[i], NULL))
								errcount++;
						}
						if(!errcount)
						{
							PGroup pgp=NULL;
							pgp = FDB_GetGroup(get_unaligned((U8*)p), NULL);
							if (pgp)
							{
								if(FDB_ChgGroup(&gp)!=FDB_OK)
									chdr->SessionID=1;
								else
									chdr->Command=CMD_ACK_OK;
							}
							else if(FDB_AddGroup(&gp)!=FDB_OK)
								chdr->SessionID=2;
							else
								chdr->Command=CMD_ACK_OK;
						}
					}
				}
				break;
			}
		case CMD_TZ_RRQ:		//读时间段设置
			if(FDB_GetTimeZone(get_unaligned((U32*)p), &tz))
			{
				len+=sizeof(TTimeZone);
				memcpy(p, (void*)&tz, sizeof(TTimeZone));
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;
		case CMD_HTZ_RRQ:	//读节假日设置
			if(FDB_GetHTimeZone(get_unaligned((U32*)p), &htz))
			{
				len+=sizeof(THTimeZone);
				memcpy(p, (void*)&htz, sizeof(THTimeZone));
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;

		case CMD_HTZ_WRQ:	//写节假日设置
			if(gOptions.LockFunOn<2)
				chdr->Command=CMD_ACK_ERROR;
			else
			{
				chdr->Command=CMD_ACK_ERROR;

				if(size==sizeof(THTimeZone))
				{
					PHTimeZone phtz=NULL;

					nmemcpy((BYTE *)&htz, (BYTE *)p, sizeof(THTimeZone));
					phtz = FDB_GetHTimeZone(get_unaligned((U16*)p),NULL);
					if (phtz)
					{
						if (FDB_ChgHTimeZone(&htz)!=FDB_OK)
							chdr->SessionID=1;
						else
							chdr->Command=CMD_ACK_OK;
					}
					else if (FDB_AddHTimeZone(&htz)!=FDB_OK)
						chdr->SessionID=2;
					else
						chdr->Command=CMD_ACK_OK;
				}
			}
			break;


		case CMD_TZ_WRQ:			//写时段设置
			if(gOptions.LockFunOn<2)
				chdr->Command=CMD_ACK_ERROR;
			else
			{
				chdr->Command=CMD_ACK_ERROR;
				if(size==sizeof(TTimeZone))
				{
					PTimeZone ptz=NULL;

					tz.ID = get_unaligned((U32*)p);
					memcpy(tz.ITime, p+sizeof(int), 28);
					ptz = FDB_GetTimeZone(get_unaligned((U32*)p),NULL);
					if (ptz)
					{
						if (FDB_ChgTimeZone(&tz)!=FDB_OK)
							chdr->SessionID=1;
						else
							chdr->Command=CMD_ACK_OK;
					}
					else if (FDB_AddTimeZone(&tz)!=FDB_OK)
						chdr->SessionID=2;
					else
						chdr->Command=CMD_ACK_OK;
				}
			}
			break;			
		case CMD_ULG_RRQ:					//读开锁组合
			if(FDB_GetCGroup(get_unaligned((U8*)p), &lgp))
			{
				len+=sizeof(TCGroup);
				memcpy(p, (void*)&lgp, sizeof(TCGroup));
			}
			else
				chdr->Command=CMD_ACK_ERROR;
			break;

		case CMD_ULG_WRQ:					//写开锁组合
			{
				int i;
				int errcount=0;

				if(gOptions.LockFunOn<2)
					chdr->Command=CMD_ACK_ERROR;
				else
				{	
					chdr->Command=CMD_ACK_ERROR;

					//printf("size:%d, sizeof(TCGroup):%d\n",size, sizeof(TCGroup));

					if(size==sizeof(TCGroup))
					{
						nmemcpy((BYTE *)&lgp, (BYTE *)p, sizeof(TCGroup));
						for(i=0;i<5;i++)
						{
							if(lgp.GroupID[i]>0 && !(FDB_GetGroup(lgp.GroupID[i], NULL)))
								errcount++;
						}

						if(!errcount)
						{
							PCGroup pcgp=NULL;
							pcgp = FDB_GetCGroup(get_unaligned((U8*)p), NULL);
							if (pcgp!=NULL)
							{
								if (FDB_ChgCGroup(&lgp)!=FDB_OK)
									chdr->SessionID=1;
								else
									chdr->Command=CMD_ACK_OK;
							}
							else if(FDB_AddCGroup(&lgp)!=FDB_OK)
								chdr->SessionID=2;
							else
								chdr->Command=CMD_ACK_OK;
						}
					}
				}                       
				break;
			}

		case CMD_UNLOCK:					//打开锁
			ExAuxOut(*p, p[1]);
			break;
		case CMD_CLEAR_ACC:
			ClearAllACOpt(TRUE);
			break;
		case CMD_CHANGE_SPEED:
			if(size==sizeof(U32))
			{
				i=get_unaligned((U32*)p);
				if(i==1)
				{
					session->Speed=session->Speed*5/4;
					if(session->Speed>50*1024*1024) session->Speed=50*1024*1024;
				}
				else if(i==0)
				{
					if(session->Speed>512*1024) 
					{
						session->Speed=session->Speed/2;
					}
				}
				else if(i>9600 && i<101*1024*1024)
				{
					session->Speed=session->Speed/2;
				}
				else
					chdr->Command=CMD_ACK_ERROR; 
			}
			else
				chdr->Command=CMD_ACK_ERROR; 
			break;
		case CMD_STARTVERIFY:
			chdr->Command=CMD_ACK_ERROR;
			if(gMachineState==STA_IDLE || gMachineState==STA_VERIFYING)
			{
				int pin;
				BYTE fingerid;
				TZKFPTemplate tmp;
				pin=get_unaligned((U16*)p);
				fingerid=p[2];
				if((size!=0 && size!=3))
					chdr->SessionID=ERR_PARAM;
				else if(size==3)
				{
					if(pin<0 || pin>MAX_PIN || ((fingerid!=0xFF) && (fingerid>=gOptions.MaxUserFingerCount)))
						chdr->SessionID=ERR_PARAM;
					else if((fingerid!=0xFF) && !FDB_GetTmp((U16)pin, (char)fingerid, (char *)&tmp))
						chdr->SessionID=ERR_NOTENROLLED;
					else
					{
						session->VerifyFingerID=fingerid;
						session->VerifyUserID=pin;
						chdr->Command=CMD_ACK_OK;
						gMachineState=STA_VERIFYING;
						session->RegEvents |= EF_VERIFY|EF_FINGER;
					}
				}
				else
				{
					session->VerifyUserID=0;
					chdr->Command=CMD_ACK_OK;
					gMachineState=STA_VERIFYING;
					session->RegEvents |= EF_VERIFY|EF_FINGER;
				}
			}
			else
				chdr->SessionID=ERR_STATE;			
			break;
		case CMD_STARTENROLL:
			chdr->Command=CMD_ACK_ERROR;
			if (gMachineState==STA_IDLE || gMachineState==STA_VERIFYING)
			{
				TZKFPTemplate tmp;
				BYTE fingerid;
				TUser user;
				char pin2[24];

				nmemcpy((BYTE *)pin2, (BYTE *)p, gOptions.PIN2Width); 			//工号
				fingerid = p[24];						//指纹序号

				if ((pin2[0]=='\0') || (fingerid>=gOptions.MaxUserFingerCount))
					chdr->SessionID = ERR_PARAM;

				memset(&user, 0, sizeof(TUser));
				if (FDB_GetUserByCharPIN2(pin2, &user) && FDB_GetTmp(user.PIN, (char)fingerid, (char *)&tmp))
					chdr->SessionID=ERR_ENROLLED;
				else
					chdr->Command=CMD_ACK_OK;
			}
			else
			{
				chdr->SessionID=ERR_STATE;
			}
			break;
		case CMD_CANCELCAPTURE:
			if(gOptions.IsModule)
				gMachineState=STA_IDLE;
			else
				gMachineState=STA_VERIFYING;

			CancelEnrollFinger();
			break;
		case CMD_WRITE_LCD:
			{
				//modified by zhc 2008.11.25 for WriteLCD
				int row = p[0], col = p[1], wparam;
				char msg[50] = {0};
				strcpy(msg, p+3);
				//printf("row=%d, col=%d, title=%s\n", row, col, msg);
				wparam = (((row & 0xFF) << 8) | (col & 0xFF));
				if (hWriteLCDWnd == HWND_NULL || hWriteLCDWnd == HWND_INVALID)
				{
					CreateWriteLCDWindow(hMainWindowWnd);
					if (hWriteLCDWnd != HWND_INVALID)
					{
						SendMessage(hWriteLCDWnd, MSG_WRITELCD, wparam, (LPARAM)msg);
					}
				}
				else
				{
					SendMessage(hWriteLCDWnd, MSG_WRITELCD, wparam, (LPARAM)msg);
				}
			}
			//end modified by zhc
			break;
		case CMD_LASTTEMP_RRQ:
			break;
		case CMD_TRANSSTATE:
			i=get_unaligned((U32*)p);
			if(size>0 && i>=0 && i<=IKeyOut-IKeyOTIn) 
				ProcStateKey(IKeyOTIn+i);
			chdr->SessionID=gOptions.AttState;
			break;
		case CMD_STATE_RRQ:
			chdr->SessionID=gMachineState;
			break;
		case CMD_TEMPDB_CLEAR:
			TDB_Clear();
			if(size>2)
			{
				TDB_ADDTemp(get_unaligned((U16*)p), (BYTE*)p+2);
			}
			break;
		case CMD_TEMPDB_ADD:
			TDB_ADDTemp(get_unaligned((U16*)p), (BYTE*)p+2);
			break;
		case CMD_WIEGAND:
			len+=WiegandSend(get_unaligned((U32*)p), get_unaligned((U32*)p+1), get_unaligned((U32*)p+2));
			break;
			//add by zhc 2008.11.24 for resd file
		case CMD_READFILE:
			{
				char *buf = NULL;
				char readfilename[64] = {0};
				int readsize = 0;;

				memset(readfilename, 0, sizeof(readfilename));
				memcpy(readfilename, p, sizeof(readfilename));

				readsize=checkfilestatus(readfilename);		
				if(readsize <= 0) {
					DBPRINTF("read file  not exists\n");
					chdr->Command=CMD_ACK_ERROR;

				} else {	
					buf=(char*)malloc(readsize);
					if((readsize=readfile(readfilename,buf))<0) {
						chdr->Command=CMD_ACK_ERROR;
						break;
					}
					SendLargeData((void*)chdr, session, buf,readsize);
					free(buf);
					buf=NULL;
				}
			}
			break;
		case CMD_MCU_COMMAND:
			{
				chdr->Command=CMD_ACK_ERROR;
				int itmp = 0;
				memcpy(&itmp,p+4,1);
				itmp = 0xFF & itmp;

				for(i=0;i<itmp;i++)
				{
					ExBeep(itmp);
				}

				chdr->Command=CMD_ACK_OK;
				break;
			}

		case CMD_UPDATEFROMUDISK:
			{
				if (UpdateSoftWareFromUDisk())
					chdr->Command=CMD_ACK_OK;		
				else
					chdr->Command=CMD_ACK_ERROR;
				break;
			}
		case CMD_CHECKUDISKUPDATEPACKPAGE:
			{
				//char p[] = "ZEM560_TFT_FirmwareVersion=Ver 6.60 Sep 17 2010:FirmwareLength=748624:FirmwareCheckSum=61302";
				//printf("%s\n",p);
				chdr->Command = CMD_ACK_ERROR;
				char UDiskHead[1024];
				const int BufferSize = 100;
				char value[3][BufferSize];
				char version[BufferSize];
				char Length[BufferSize];
				char CheckSum[BufferSize];
				char platform[60];
				char platformvalue[20];
				char iMainVersion[64], iFWVersion[64];
				char *ApplyTest=NULL;
				int i = 0;
				memset(platform, 0, sizeof(platform));
				memset(platformvalue, 0, sizeof(platformvalue));
				memset(UDiskHead, 0, sizeof(UDiskHead));
				if (LoadStr(PLATFORM,platformvalue))
					sprintf(platform,"%s%s",platformvalue,"_FirmwareVersion=");
				else
					sprintf(platform,"%s%s","ZEM200","_FirmwareVersion=");
				//DBPRINTF("%s\n",p);
				memcpy(UDiskHead, p, size);
				DBPRINTF("%s\n", UDiskHead);
				for(i = 0; i < 3; i++)
				{
					memset(value[i], 0, BufferSize);
				}
				DecomposeStr(UDiskHead, ":", value, BufferSize);
				for(i = 0; i < 3; i++)
				{
					//printf("value=%s\n",value[i]);
					if(strncmp(platform, value[i], strlen(platform)) == 0)
					{
						strcpy(version, value[i]+strlen(platform));
						printf("version=%s\n", version);
					}
					else if(strncmp("FirmwareLength=", value[i], strlen("FirmwareLength=")) == 0)
					{
						strcpy(Length, value[i]+strlen("FirmwareLength="));
						printf("Length=%s\n", Length);
					}
					if(strncmp("FirmwareCheckSum=", value[i], strlen("FirmwareCheckSum=")) == 0)
					{
						strcpy(CheckSum, value[i]+strlen("FirmwareCheckSum="));
						printf("CheckSum=%s\n", CheckSum);
					}
				}
				if(strcmp(ConvertMonth(version, iFWVersion), ConvertMonth(MAINVERSION, iMainVersion))<0)
				{
					chdr->SessionID=1;
				}
				else if((ApplyTest = MALLOC(4000+atoi(Length))) == NULL)
				{
					if (!gOptions.IsOnlyRFMachine)
					{
						FPFree();
						FREE(gImageBuffer);
						FreeSensor();
						if((ApplyTest = MALLOC(4000+atoi(Length))) == NULL)
						{
							chdr->SessionID=2;
						}
						else
						{
							if(ApplyTest != NULL)
							{
								FREE(ApplyTest);
								ApplyTest = NULL;
							}
							UDiskCheckSum = atoi(CheckSum);
							UDiskLength = atoi(Length);
							chdr->Command = CMD_ACK_OK;
						}
					}
					else
						chdr->SessionID=2;
				}
				else
				{
					if(ApplyTest != NULL)
					{
						FREE(ApplyTest);
						ApplyTest = NULL;
					}
					UDiskCheckSum = atoi(CheckSum);
					UDiskLength = atoi(Length);
					chdr->Command = CMD_ACK_OK;
				}
			}
			break;
		case CMD_OPTIONS_DECIPHERING:
			{
				int PasswodLen = 0;
				chdr->Command = CMD_ACK_ERROR;
				memset((char*)gOptions.OptionsPassword, 0x00, sizeof(gOptions.OptionsPassword));
				LoadStr("OPTIONSPASSWORD", (char*)gOptions.OptionsPassword);
				PasswodLen = strlen((char*)gOptions.OptionsPassword);
				DBPRINTF("%s, len=%d, OptionsPassword = %s\n", p, PasswodLen, (char*)gOptions.OptionsPassword);
				if(PasswodLen && strncmp((char*)gOptions.OptionsPassword, p, PasswodLen) == 0)
				{
					DBPRINTF("Options Deciphering successful!\n");
					chdr->Command = CMD_ACK_OK;
					IsCheckEncryptFlag = 0;
				}
				else
				{
					DBPRINTF("Options Deciphering failed!\n");
					chdr->SessionID = 1;
					IsCheckEncryptFlag = 1;
				}
				break;
			}
		case CMD_OPTIONS_ENCRYPT:
			DBPRINTF("Options Encrypt back!\n");
			IsCheckEncryptFlag = 1;
			break;
			//dsl 2012.3.23
		case CMD_GET_PHOTO_COUNT:
			{
				U32 flag=1;
				SwitchMsgType(0);
				memcpy(&flag,p,4);
				int num = FDB_CntPhotoCountByPar(flag, chdr);
				//printf("num = %d\n",num);
				SwitchMsgType(1);
				memcpy(p,&num,4);
				len+=strlen(p)+1;
				break;
			}
		case CMD_GET_PHOTO_BYNAME:
			SwitchMsgType(0);
			FDB_GetPhotoByName(p,chdr,session);
			SwitchMsgType(1);
			break;
		case CMD_CLEAR_PHOTO_BY_TIME:
			SwitchMsgType(0);
			FDB_ClearPhotoByTime(p,chdr,session);
			SwitchMsgType(1);
			break;
		case CMD_GET_PHOTONAMES_BY_TIME:
			SwitchMsgType(0);
			FDB_GetAllPhotoNamesByTime(p,chdr,session);
			SwitchMsgType(1);
			break;
			//end dsl
		/*读取10算法指纹模板，与CMD_LARGE_FINGER_MACHINE配合使用*/
		case CMD_READ_LARGE_TMP:
		{
			int tmpsize=0;
			tmpsize = QueryTmpData((void*)chdr,session);
			if(tmpsize > 0) {
				chdr->Command=CMD_ACK_OK;
			}
			memcpy(p,&tmpsize,sizeof(int));
			len += sizeof(int);
			break;
		}
		case CMD_DATA_LARGE_TMP:
		{
			if(SendTmpDataToSDKByID((void*)chdr,session) > 0) {
				chdr->Command=CMD_ACK_OK;
			}
			break;
		}
		case CMD_FREE_LARGE_TMP://释放资源
		{
			if(tmpUserIDs) {
				free(tmpUserIDs);
				tmpUserIDs = NULL;
			}
			chdr->Command=CMD_ACK_OK;
			break;
		}
		case CMD_LARGE_FINGER_MACHINE:
			/*9.0算法不用此方法，按原来方式*/
			if(gOptions.ZKFPVersion == ZKFPV10) {
				chdr->Command=CMD_ACK_OK;
			}
			else
			{
				chdr->Command = CMD_ACK_ERROR;
			}
			break;
		case CMD_LARGE_FINGER_UPDATEFLAG: /*下发用户和指纹数据时使用写文件模式*/
			isWriteFlashTmp = 1;
			chdr->Command=CMD_ACK_OK;
			break;
		default:
			chdr->Command=CMD_ACK_UNKNOWN;
			break;
	}
	return len;
}

void SendLargeData(char *in_buf, PCommSession session, char *buf, int size)
{
	PCmdHeader chdr=(PCmdHeader)in_buf;
	char *p=(char*)(chdr+1);
	//char *largeBlockBuffer;
	char largeBlockBuffer[MAX_BUFFER_LEN_TCP+sizeof(TCmdHeader)];//dsl 2012.3.23
	int index, psize=1024, len=sizeof(TCmdHeader)+sizeof(U32)*2;
	if(nstrcmp("TCP", session->Interface, 3)==0)
	{
		//最大能传输的数据应该是 TCP 连接最多能够承载的字节数 减去 TCmdHeader 的长度 modify by jic 2013-11-5
		psize = MAX_BUFFER_LEN_TCP - sizeof(TCmdHeader);
		//largeBlockBuffer = (char*)MALLOC(psize+sizeof(TCmdHeader));
		memcpy(largeBlockBuffer,in_buf,sizeof(TCmdHeader));
		chdr = (PCmdHeader)largeBlockBuffer;
		p=(char*)(chdr+1);
	}
	memcpy(p, &size, 4);
	memcpy(p+4, &psize, 4);
	chdr->Command=CMD_PREPARE_DATA;
	chdr->CheckSum=0;
	chdr->CheckSum=in_chksum((unsigned char *)chdr,len);
	session->Send((char*)chdr,len,session->Sender);
	index=0;
	chdr->Command=CMD_DATA;
	while(size>0)
	{
		len=size;
		if(len>psize) len=psize;
		chdr->SessionID=index;
		memcpy(p,buf, len);
		chdr->CheckSum=0;
		chdr->CheckSum=in_chksum((unsigned char *)chdr,len+sizeof(TCmdHeader));
		session->Send(chdr,len+sizeof(TCmdHeader),session->Sender);
		DelayMS(20);
		buf+=len;
		index++;
		printf("_______%s%d______len = %d\n", __FILE__, __LINE__, len);
		size-=len;
		if((index%(6+(time(NULL)%10))==5) && (session->Speed>=256*1024))//以太网连接，延时
		{
			int dd=200*1024/(session->Speed/(2*1024));
			if(dd>1200) dd=1200;
			DelayMS(dd);
		}
	}
	//if(nstrcmp("TCP", session->Interface, 3)==0)
	//	FREE(largeBlockBuffer);
	chdr->Command=CMD_ACK_OK;
}

void SendLargeDataByFile(char *in_buf, PCommSession session, int offset, int size)
{
	PCmdHeader chdr=(PCmdHeader)in_buf;
	char *p=(char*)(chdr+1);
	char *largeBlockBuffer=NULL;
	int index=0;
	int psize=1024;
	int len=sizeof(TCmdHeader)+sizeof(U32)*2;

	if(nstrcmp("TCP", session->Interface, 3)==0)
	{
		//最大能传输的数据应该是 TCP 连接最多能够承载的字节数 减去 TCmdHeader 的长度 modify by jic 2013-11-5
		psize = MAX_BUFFER_LEN_TCP - sizeof(TCmdHeader); 
		largeBlockBuffer = (char*)malloc(psize+sizeof(TCmdHeader));
		memcpy(largeBlockBuffer,in_buf,sizeof(TCmdHeader));
		chdr = (PCmdHeader)largeBlockBuffer;
		p=(char*)(chdr+1);
	}

	memcpy(p, &size, 4);
	memcpy(p+4, &psize, 4);
	chdr->Command=CMD_PREPARE_DATA;
	chdr->CheckSum=0;
	chdr->CheckSum=in_chksum((unsigned char *)chdr,len);
	session->Send((char*)chdr,len,session->Sender);
	index=0;
	chdr->Command=CMD_DATA;
	while(size>0)
	{
		len=size;
		if(len>psize) {
		       	len=psize;
		}

		chdr->SessionID=index;

		/*translation data format:
		 * 0---------------4--------------xx
		 * |attendence size|attendence data|
		 * the xx value equl session->Buffer->bufferSize;
		 * */
		if (offset == 0) {
			/*we need send the total size of attendence in first read.*/
			SET_DWORD(p, session->Buffer->bufferSize-4, 0);
			lseek(SelectFDFromConentType(FCT_ATTLOG), offset, SEEK_SET);
			read(SelectFDFromConentType(FCT_ATTLOG), p+4, len);
		} else {
			lseek(SelectFDFromConentType(FCT_ATTLOG), offset-4, SEEK_SET);
                        read(SelectFDFromConentType(FCT_ATTLOG), p, len);
		}

		chdr->CheckSum=0;
		chdr->CheckSum=in_chksum((unsigned char *)chdr,len+sizeof(TCmdHeader));
		session->Send(chdr,len+sizeof(TCmdHeader),session->Sender);
		DelayMS(20);
		offset+=len;
		index++;
		size-=len;
		if((index%(6+(time(NULL)%10))==5) && (session->Speed>=256*1024))//以太网连接，延时
		{
			int dd=200*1024/(session->Speed/(2*1024));
			if(dd>1200) dd=1200;
			DelayMS(dd);
		}
	}
	if(nstrcmp("TCP", session->Interface, 3)==0) {
		free(largeBlockBuffer);
	}
	chdr->Command=CMD_ACK_OK;
}

int QueryTmpData(char *in_buf, PCommSession session)
{
	int fingercnt = 0;
	PCmdHeader chdr=(PCmdHeader)in_buf;
	chdr->Command=CMD_ACK_ERROR;
	if(tmpUserIDs) {
		free(tmpUserIDs);
		tmpUserIDs = NULL;
	}
	tmpUserIDs = (int*)malloc(gOptions.MaxUserCount*1000*sizeof(int));
	if(tmpUserIDs == NULL) {
		return -1;
	}

	GetTmpPameters(&fingercnt, tmpUserIDs);
	if(fingercnt <= 0) {
		return 0;
	}
	chdr->Command=CMD_ACK_OK;
	return fingercnt;
}

int SendTmpDataToSDKByID(char *in_buf, PCommSession session)
{
	int size, psize, len, index;
	int currentcnt;
	int readfingercnt;
	U16 datachecksum;
	PCmdHeader chdr=(PCmdHeader)in_buf;
	char *sendbuf;
	char *tmpfingerbuff = NULL;
	char *tmpfingerbuff1 = NULL;
	index = 0;
	
	tmpfingerbuff = (char*)malloc(400*1024);
	if(tmpfingerbuff == NULL) {
		return -1;
	}
	memset(tmpfingerbuff, 0, 400*1024);

	memcpy(&currentcnt, in_buf+sizeof(TCmdHeader), sizeof(int));
	memcpy(&readfingercnt, in_buf+sizeof(TCmdHeader)+sizeof(int), sizeof(int));
	printf("_____%s%d,currentcnt = %d, readfingercnt = %d\n", __FILE__, __LINE__, currentcnt, readfingercnt);
	GetTmpDataByID(&size, readfingercnt, &currentcnt, tmpUserIDs, tmpfingerbuff);
	tmpfingerbuff1 = tmpfingerbuff;

	/*先发校验*/
	datachecksum = in_chksum((unsigned char*)tmpfingerbuff, size);
	memcpy(in_buf+sizeof(TCmdHeader), &size, sizeof(int));//数据长度
	memcpy(in_buf+sizeof(TCmdHeader)+sizeof(int), &datachecksum, 2);//总校验值
	printf("_____%s%d,size = %d, datachecksum = %d\n", __FILE__, __LINE__, size, datachecksum);
	len=sizeof(TCmdHeader)+sizeof(int)*2;
	chdr->Command=CMD_PREPARE_DATA;
	chdr->CheckSum=0;
	chdr->CheckSum=in_chksum((unsigned char *)chdr,len);
	session->Send((char*)chdr,len,session->Sender);
	msleep(100);

	fast485modeflag = 1;
	if(nstrcmp("TCP", session->Interface, 3)==0) {
		psize = 1024*40; 
	} else {
		psize = 1024;
	}
	sendbuf = (char*)malloc(psize+sizeof(TCmdHeader));
	
	if(sendbuf == NULL) {
		if(tmpfingerbuff1 != NULL) {
			free(tmpfingerbuff1);
			tmpfingerbuff = NULL;
			tmpfingerbuff1 = NULL;
		}
		return -1;
	}
	
	memset(sendbuf, 0, psize+sizeof(TCmdHeader));
	memcpy(sendbuf, in_buf, sizeof(TCmdHeader));
	chdr = (PCmdHeader)sendbuf;
	chdr->Command=CMD_DATA;
	while(size>0)
	{
		len=size;
		if(len>psize) len=psize;
		memcpy(sendbuf+ sizeof(TCmdHeader),tmpfingerbuff, len);
		chdr->CheckSum=0;
		chdr->SessionID=index;
		chdr->CheckSum=in_chksum((unsigned char *)chdr,len+sizeof(TCmdHeader));
		
		printf("CheckSum=%d,  len=%d____________________________________________________\n",chdr->CheckSum,len);
		session->Send(chdr,len+sizeof(TCmdHeader),session->Sender);
		DelayMS(10);
		tmpfingerbuff += len;
		index++;
		size -= len;
		if((index%(6+(time(NULL)%10))==5) && (session->Speed>=256*1024))//以太网连接，延时
		{
			int dd=200*1024/(session->Speed/(2*1024));
			if(dd>1200) dd=1200;
			DelayMS(dd);
		} else {
			if(nstrcmp("TCP", session->Interface, 3) != 0) {
				int delaytime = 200*115200/gOptions.RS232BaudRate;
				DelayMS(delaytime);
			}
		}
	}

	if(tmpfingerbuff1 != NULL) {
		free(tmpfingerbuff1);
		tmpfingerbuff = NULL;
		tmpfingerbuff1 = NULL;
	}

	if(sendbuf != NULL) {
		free(sendbuf);
		sendbuf = NULL;
	}
	fast485modeflag = 0;
	printf("______%s%d\n", __FILE__, __LINE__);
	return 1;
}

