#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <linux/sockios.h>
#include <sys/ioctl.h> 
#include "thread.h"
#include "pushapi.h"
#include "options.h"
#include "flashdb.h"
#include "exfun.h"
#include "finger.h"
#include "strutils.h"
#include "main.h"
#include "utils2.h"
#include "pushsdk_lib_api.h"
#include "exfun.h"
#include "flashdb.h"
#include "../face/facedb.h"
#include "commu.h"
#include "pushapi.h"

extern BOOL sleepflag;
extern BOOL menuflag;
extern TTime gCurTime;
#define BCDTODEC(bcd) ((bcd) = ((bcd) & 0x0f) + ((bcd)>>4) * 10)
int create_data_record(char *file_name,int contype);
int read_data(int con_type, void *usr_data,int index,void *data);
int read_face_data(void *usr_data,int index,void *out_data);
int read_usr_data(int con_type, char *data_buf,int file_index,int fd) ;
BOOL  att_log_cnt(char *start_time,char *end_time,unsigned int *cnt);
BOOL  oper_log_cnt(char *start_time,char *end_time,unsigned int *cnt);
int att_log_upload_filter(void *data, char *param);
int oper_log_upload_filter(void *data, char *param);
int ProcessEnroll(char* pin2, int fingerid, int *tmplen ,BYTE valid);
void register_push_fun(void);
void pushsdk_set_file_offset(int contype,off_t offset);
BOOL dev_is_busy(void);
int get_fp_recode(void);
unsigned int  file_get_size(int fd);

int read_attlog_func(unsigned char *buf, int maxrec, int currec, int maxcount,int *attlenght,off_t *curfp);


static int IclockSvrURLLength;
static char SN[32];
static off_t iCurAttLogPos = 0;
static off_t iCurOptLogPos = 0;
static off_t iCurrAttPhotoPos = 0;
#ifdef SUPPORT_FACE
static TFaceTmp pushsdk_facetmp[FACE_NUM];
pushsdk_face_tmp_t upload_face_tmp[FACE_NUM];
#endif


#ifndef ICON_PADDING_TOP
#define ICON_PADDING_TOP 	10
#endif
#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH 	32
#endif
#ifndef ICON_SIGNAL_WIDTH
#define ICON_SIGNAL_WIDTH	64
#endif
#ifndef ICON_WIDTH
#define ICON_WIDTH		8
#endif

static char *push_svr_data_string_type [] = {
	"AttLog",
	"OpLog",
	"AttPhoto",
	"EnrollFP",
	"EnrollUser",
	"FPImag",
	"ChgUser",
	"ChgFP",
	"FACE",
	"UserPic",
};
unsigned int  get_file_size(int fd)
{
    struct stat buf;

    if (fd < 0) {
	return 0;
    }
    if (fstat(fd, &buf)<0) {
        return 0;
    }
    return (unsigned int)(buf.st_size);

}

void changestr(char *src, char *des, int len)
{
	int i;
	unsigned char data[20];
	memset(data, 0, 20);
	memcpy(data, src, 20);
	for(i=0;i<len;i++)
	{
		if(data[i] == 0x20)
			data[i] = 0x00;
		continue;
	}
	memcpy(des, data, len);
}

void pushsdk_db_init(void)
{
	return;
}


void pushsdk_set_filepos(int con_type,off_t offset )
{
	int fd = -1;
	fd = SelectFDFromConentType(con_type);
	if ((fd >= 0) && (offset >= 0)) {
		lseek(fd, offset, SEEK_SET);
	}
}
int pushsdk_get_filepos(int ContentType,off_t *offset)
{
	int fd = -1;
	
	fd = SelectFDFromConentType(ContentType);
	if (fd >= 0) {
		*offset = lseek(fd, 0, SEEK_CUR);
	}
	return fd;
}
#ifdef SUPPORT_FACE
int pushsdk_get_user_face_info(TUser *user,pushsdk_face_tmp_t*face_tmp,size_t tmp_cnt)
{
	static char tmp[FACE_TMP_MAXSIZE * FACE_NUM];
	static TUser tmp_usr ;
	static int first = 0;
	int i =0;
	
	if (user == NULL || face_tmp == NULL) {
		return 0;
	}
	if ((first == 0) ||((first != 0) && (strcmp(user->PIN2,tmp_usr.PIN2) != 0))\
								|| (face_tmp[0].facetmp.PIN <= 0)) {
		if (pushsdk_face_handle == NULL) {
			return 0;
		}
		if (ZKFaceDBGet(pushsdk_face_handle, user->PIN2, \
						(unsigned char *)tmp, sizeof(tmp)) < FACE_NUM) {
			return -1;
		}
		memcpy(&tmp_usr,user,sizeof(TUser));
		first = 1;
	}

	tmp_cnt = FACE_NUM > tmp_cnt ? tmp_cnt:FACE_NUM;
	for (i=0; i< tmp_cnt; i++) {
		face_tmp[i].facetmp.PIN= user->PIN;
		face_tmp[i].facetmp.FaceID = i;
		face_tmp[i].facetmp.Valid = 1;
		face_tmp[i].facetmp.Size = FACE_TMP_MAXSIZE;
		face_tmp[i].facetmp.Reserved = gOptions.ZKFaceVersion;
		memcpy(face_tmp[i].facetmp.Face, tmp+i*FACE_TMP_MAXSIZE, FACE_TMP_MAXSIZE);
		memcpy(face_tmp[i].pin2,user->PIN2,strlen(user->PIN2)+1);
	}
	
	return 1;
}

int pushsdk_get_user_facetmp_by_faceid(TUser *user,pushsdk_face_tmp_t *face_tmp,size_t faceid)
{
	static char tmp[FACE_TMP_MAXSIZE * FACE_NUM];
	static TUser tmp_usr;
	static int first = 0;
	
	if (user == NULL || face_tmp == NULL) {
		return 0;
	}
	if ((first == 0) ||((first != 0) && (strcmp(user->PIN2,tmp_usr.PIN2) != 0))) {
		if (pushsdk_face_handle == NULL) {
			return 0;
		}
		if (ZKFaceDBGet(pushsdk_face_handle, user->PIN2, \
						(unsigned char *)tmp, sizeof(tmp)) < FACE_NUM) {
			return 0;
		}
		memcpy(&tmp_usr,user,sizeof(TUser));
		first = 1;
	}
	if (faceid >= FACE_NUM) {
		return -3;
	}
	
	face_tmp->facetmp.PIN = user->PIN;
	face_tmp->facetmp.FaceID = faceid;
	face_tmp->facetmp.Valid = 1;
	face_tmp->facetmp.Size = FACE_TMP_MAXSIZE;
	face_tmp->facetmp.Reserved = gOptions.ZKFaceVersion;
	memcpy(face_tmp->facetmp.Face,tmp+faceid*FACE_TMP_MAXSIZE,FACE_TMP_MAXSIZE);
	memcpy(face_tmp->pin2,user->PIN2,strlen(user->PIN2)+1);
	if (faceid == FACE_NUM-1) {
		first = 0;
	}

	return 1;
}
int pushsdk_get_facetmp_num(char *uid)
{
	unsigned char tmp[FACE_TMP_MAXSIZE * FACE_NUM];

	if (pushsdk_face_handle == NULL) {
		return 0;
	}

	return ZKFaceDBGet(pushsdk_face_handle, uid, tmp, sizeof(tmp));
}
#endif
int  pushsdk_get_user_by_pin2(char *pin2, PUser user)
{
	return pushsdk_deal_data(pin2, strlen(pin2)+1,\
			PUSHSDK_DATA_GET_BY_PIN2,FCT_USER,user, sizeof(TUser));
}
int pushsdk_get_user_by_pin(U16 pin, PUser user)
{
	return pushsdk_deal_data(&pin, sizeof(pin),\
			PUSHSDK_DATA_GET_BY_PIN,FCT_USER,user, sizeof(TUser));
}
int  pushsdk_get_fp_tmp(U16 UID, char FingerID, char *tmp)
{
	unsigned int fp_info = (FingerID << 16)  | (UID);
	
	return pushsdk_deal_data(&fp_info, sizeof(fp_info),\
			PUSHSDK_DATA_GET_BY_PIN,FCT_FINGERTMP,tmp, sizeof(pushsdk_fp_tmp_t));
}
int pushsdk_CntAttLog(void)
{
	int fd=SelectFDFromConentType(FCT_ATTLOG);
	int lst = get_file_size(fd);
	int ret=lst/sizeof(TAttLog);

	return ret;
}
time_t StrTimeToStamp(const char *strISOTime)
{
	struct tm ttDateTime;
	
	ttDateTime.tm_year = atoi(strISOTime)-1900;
	ttDateTime.tm_mon = atoi(strISOTime+5)-1;
	ttDateTime.tm_mday = atoi(strISOTime+5+3);

	ttDateTime.tm_hour = atoi(strISOTime+5+3+3);
	ttDateTime.tm_min = atoi(strISOTime+5+3+3+3);
	ttDateTime.tm_sec = atoi(strISOTime+5+3+3+3+3);
	
	return mktime(&ttDateTime);
}

int extractValueSample(char* s, char *key, char sp, char *value, int maxlen)
{
	char *cookieStart=NULL, *p;
	char cookieExp[200];
	int nl;

	if ((s == NULL) || (key == NULL) || (value == NULL)) {
		return  0;
	}
	
	*value=0;
	nl = snprintf(cookieExp,sizeof(cookieExp),"%s=", key);
	if (strncmp(s,cookieExp+1,nl-1) == 0) {
		cookieStart=s+nl-1;
	} else {
		cookieStart = strstr(s, cookieExp);
		if (cookieStart != NULL) {
			cookieStart += nl;
		}
	}
	if (cookieStart == NULL) {
		*value=0;
		return 0;
	}
	p = value;
	nl = 0;
	while (1) {
		int ch=*cookieStart++;
		if(ch==sp || ch==0)  {
			*p=0;
			return p-value;
		}
		*p++=ch;
		if (++nl >= maxlen) {
			break;
		}
	}
	return 0;
}
int get_iclock_svr_url(char * svr_url,int url_mode,char *svr_ip, int svr_port)
{
	char buff[128];
	
	if ((!LoadStr("ICLOCKSVRURL", svr_url)) || (strlen(svr_url) == 0) ) {
		if (svr_ip[0] != 0) {
			sprintf(buff, "http://%d.%d.%d.%d", svr_ip[0], svr_ip[1],svr_ip[2], svr_ip[3]);
		} else {
			svr_url[0] = '\0';
		}
	} else {
		sprintf(buff, "%s", svr_url);
	}
	return strlen(svr_url);
}
static int pushsdk_unlock(char *param)
{
	DoAuxOut(gOptions.LockOn, gOptions.OpenDoorDelay);
	return 0;
}
static int pushsdk_enroll_fp(char *param)
{
	char ow[200];
	char pin2[MAXNAMELENGTH];

	if (gOptions.IsOnlyRFMachine || (param == NULL)) {
		return -1;
	}

	U16 pin = extractValueSample(param, "PIN", '\t', pin2,sizeof(pin2));
	int fingerid = extractValueInt(param, "FID", '\t', 0);
	int retry=extractValueInt(param, "RETRY", '\t', 1);
	int owi=extractValueSample(param, "OVERWRITE", '\t', ow,sizeof(ow));
	TUser user;

	if (FDB_GetUserByCharPIN2(pin2, &user)==NULL) {
			return -2;
	}
	pin = user.PIN;
	if (owi) {
		if (strcasecmp(ow, "FALSE") == 0) {
			owi=0;
		} else if (strcasecmp(ow, "0")==0) {
			owi=0;
		}
	}
	if (!owi) {
		if(FDB_GetTmp(pin, fingerid, NULL) != 0) {
			return -3;
		}
	}
	ow[0]=fingerid; 
	ow[1]=retry; 
	ow[2]=owi?1:0;
	memcpy(ow+3, &pin, sizeof(pin));
	memset(param,0,5+sizeof(pin));
	memcpy(param,ow,3+sizeof(pin));
	
	return 0;
}

BOOL  init_push(void)
{
	int socket_send_buf = 64*1024;
	int ret = PUSH_ERROR_WEBURL;
	char url[256];

	
	gOptions.iClockServerStatus = ICLOCK_STATE_OFFLINE;
	memset(gOptions.IclockSvrURL,0,sizeof(gOptions.IclockSvrURL));
	
	if (gOptions.WebServerURLModel) {
		LoadStr("ICLOCKSVRURL", (char*)gOptions.IclockSvrURL);
	} else {
		snprintf((char *)gOptions.IclockSvrURL, sizeof(gOptions.IclockSvrURL),"http://%d.%d.%d.%d", 
		gOptions.WebServerIP[0], gOptions.WebServerIP[1], gOptions.WebServerIP[2], gOptions.WebServerIP[3]);
	}
	
	IclockSvrURLLength = strlen((char *)(gOptions.IclockSvrURL));
	if (IclockSvrURLLength == 0) {
		strcpy((char *)(gOptions.IclockSvrURL),"0.0.0.0");
		IclockSvrURLLength = strlen((char *)(gOptions.IclockSvrURL));
	}
	if (IclockSvrURLLength >= 0) {
		register_push_fun();
		if (gOptions.IsSupportModem) {
			socket_send_buf = 0;
		}
		 if (strncasecmp((char *)(gOptions.IclockSvrURL), "http://", 7) != 0) {
			snprintf(url,sizeof(url), "http://%s:%d/iclock/", \
					gOptions.IclockSvrURL,gOptions.WebServerPort);
		} else {
			snprintf(url,sizeof(url), "%s:%d/iclock/",\
					gOptions.IclockSvrURL,gOptions.WebServerPort);
		}
		ret = pushsdk_init((unsigned char*)url, (unsigned char *)(gOptions.WebServerIP),\
						gOptions.WebServerPort,(unsigned char *)(gOptions.ProxyServerIP),\
						gOptions.ProxyServerPort,socket_send_buf,MAINVERSIONTFT);
		if (ret == PUSH_OK) {
			LoadStr("~SerialNumber",SN);
			pushsdk_db_init();
			pushsdk_register_cnt_fun(NULL,pushsdk_CntAttLog,\
							FDB_CntUserByMap,FDB_CntTmp,att_log_cnt,oper_log_cnt);
			pushsdk_register_clear_fun(FDB_ClrAdmin,FDB_ClrAttLog,\
										FDB_ClrPhoto,FDB_ClearData);
			pushsdk_register_read_data_fun(read_usr_data,read_data,create_data_record,main_update_data,\
											dev_is_busy,pushsdk_set_file_offset,read_attlog_func);
			
			InitPushParam(0,0,0,gOptions.IsOnlyRFMachine,gOptions.ShowPhoto);
		} else {
			printf("[PUSH SDK]    INIT FAILED,ERROR CODE:%d\n",ret);
		}
	}
	return ((IclockSvrURLLength > 0) && (ret == PUSH_OK));
}

void push_child_end(pid_t pid)
{
	if((gOptions.IclockSvrFun == 1) && (pid == pushsdk_get_pid())) {
		printf("[PUSH SDK] PARENT RECEIVE END SIGNAL\n");
		pushsdk_stop();
	}
}
int pushsdk_is_running(void)
{
	if ((gOptions.IclockSvrFun == 1) &&(pushsdk_get_pid() > 0)) {
		return TRUE;
	} else {
		printf("test---pushsdk_thread_id <= 0\n");
		return FALSE;
	}
}

static int checkIntKey(char *buffer, char *key, unsigned int *value)
{
	int len;
	
	if ((buffer == NULL) || (key == NULL) || (value == NULL)) {
		return 0;
	}
	
	len = strlen(key);
	if ((strncmp(buffer, key, len)==0) && \
	   ((buffer[len]=='=') || (buffer[len]=='>') || (buffer[len]=='<'))) {
		len += 1;
		if(buffer[len] != '\0') {
			*value = atoi(buffer+len);
		} else {
			*value = '\0';
		}
		return 1;
	}
	return 0;
}
static int getPINFrom(char *param, char *pin2)
{
	char tmp[MAXPIN2LEN+1];
	TUser user;
	int	i = extractValueSample(param, "PIN", '\t', tmp,sizeof(tmp));

	if (i<=0 || i>sizeof(user.PIN2)) {
		printf("[PUSH SDK]		DOWNLOAD PIN2 ERROR:%s\n",param);
		return 0;
	}
	if (pin2 != NULL) {
		memcpy(pin2, tmp, sizeof(user.PIN2));
	}
	return 1;
}

int data_is_alaredy_trans(time_t log_stamp,time_t svr_stamp)
{
	struct tm  t;
	
	GetTime(&t);
	if (OldEncodeTime(&t) < svr_stamp) {
		return 1;
	} else if (log_stamp <= svr_stamp) {
		return 0;
	} else {
		return 1;
	}
}

static int svr_is_need_data(char *dev_flag,int data_type)
{
	int ret = 1;
	
	if (dev_flag == NULL) {
		return 0;
	}
	if (data_type < 0 || (data_type >= sizeof(push_svr_data_string_type)/sizeof(char *))) {
		return 0;
	}

	if ((strstr(dev_flag,"TransData") != NULL) && \
		(strstr(dev_flag,push_svr_data_string_type[data_type]) == NULL)) {
		ret = 0;
	} else if ((strlen(dev_flag) > data_type) && (dev_flag[data_type] == '0')) {
		ret = 0;
	}
	return ret;
}
BOOL is_oper_user(char oper)
{
	char oper_user[] = {
	OP_ACC_USER,	OP_ENROLL_USER,	OP_CHG_FP,
	OP_ENROLL_USERPIC,	OP_CHG_USERPIC,
	OP_MF_CREATE,	OP_CHG_USER_NAME,OP_ENROLL_FACE,OP_CHG_FACE};
	
	if ((oper > 5) && (oper < 13) && oper != OP_DEL_USER) {
		return TRUE;
	} else {
		int i;
		for (i=0; i<sizeof(oper_user)/sizeof(char); i++) {
			if (oper == oper_user[i]) {
				return TRUE;
			}
		}
	}
	return FALSE;
}
void  strToOplog( char *strOplog,POPLog pOplog,char *user_pin2)
{
	int i;
	char *ptmp;
	/* 	sprintf(text,"OPLOG %d\t%d\t%04d-%02d-%02d %02d:%02d:%02d\t%d\t%d\t%d\t%d\n",
		log->OP, log->Admin, t.tm_year+1900, t.tm_mon+1, t.tm_mday,
		t.tm_hour, t.tm_min, t.tm_sec,
		log->Users[0], log->Users[1], log->Users[2], log->Users[3]);*/
	pOplog->OP = atoi(strOplog);
	strOplog = strstr(strOplog,"\t"); 
	strOplog++;
	
	pOplog->Admin = atoi(strOplog);
	strOplog = strstr(strOplog,"\t"); 
	strOplog++;
	/*skip the time_second*/
	strOplog = strstr(strOplog,"\t"); 
	strOplog++;

	/*get the user pin2*/
	ptmp = strOplog;
	strOplog = strstr(strOplog,"\t"); 
	*strOplog = '\0';
	strOplog++;
	if (is_oper_user(pOplog->OP)) {
		strcpy(user_pin2,ptmp);
	} else {
		pOplog->Users[0] = atoi(ptmp);
	}
	
	for (i=1; i<4; i++)
	{
		pOplog->Users[i] = atoi(strOplog);
		strOplog = strstr(strOplog,"\t"); 
		strOplog++;
	}
}

static int file_get_length_by_name(char *name)
{
	int len = -1;
	FILE * file_stream;
	
	if (name == NULL) {
		return len;
	}
	
	file_stream = fopen(name, "rb");
	if(file_stream != NULL) {
		fseek(file_stream, 0, SEEK_END);
		len = ftell(file_stream);
		fclose(file_stream);
	}
	return len;
}
static int file_get_length(FILE *file_stream)
{
	int len = -1;
	int cur_offset;
	
	if (file_stream == NULL) {
		return len;
	}
	
	cur_offset = ftell(file_stream);
	fseek(file_stream, 0, SEEK_END);
	len = ftell(file_stream);
	fseek(file_stream, cur_offset, SEEK_SET);

	return len;
}
static int operlog_filter(void *data, char *param)
{
	U32 startStamp;
	char gTransFlag[256];
	TOPLog *log=(TOPLog*)data;

	if ((data == NULL) || (param == NULL)) {
		return -1;
	}
	if (!checkIntKey(param,"startStamp",&startStamp)) {
		return -1;
	}
	extractValueSample(param, "gTransFlag", '\n', gTransFlag,sizeof(gTransFlag));
	if (svr_is_need_data(gTransFlag,PUSH_SVR_OPERLOG) == 0) {
		return -2;
	}
	return data_is_alaredy_trans(log->time_second,startStamp);
}
static int user_filter(void *data, char *param)
{
	int ret = 1;
	char gTransFlag[256];
	TUser *user = (TUser *)data;
	char user_pin2[25];
	TOPLog log;
	
	if (strstr(param,"USERINFO")) {
		if (extractValueSample(param, "PIN", '\0', user_pin2,sizeof(user_pin2)) >= 0) {	
			if (pushsdk_get_user_by_pin2(user_pin2,user) > 0) {
				return 1;
			}
			return 0;
		} else {
			if (user->PIN > 0)	{
				return 1;
			}
			return 0;
		}
	}
	extractValueSample(param, "gTransFlag", '\n', gTransFlag,sizeof(gTransFlag));
	if (strncmp(param,"OPLOG",5)==0) {
		strToOplog(param+6, &log, user_pin2);
		switch (log.OP) {
		case OP_ENROLL_USER:
			if (!svr_is_need_data(gTransFlag,PUSH_SVR_EN_USER)) {
				return 0;
			}
			break;
		case OP_ENROLL_PWD:
		case OP_ENROLL_RFCARD:
		case OP_ACC_USER:
		case OP_DEL_PWD:
		case OP_DEL_RFCARD:
		case OP_CLEAR_ADMIN:
		case OP_ENROLL_FP:
		case OP_CHG_USER_NAME:
			if (!svr_is_need_data(gTransFlag,PUSH_SVR_CHG_USER)) {
				return 0;
			}
			break;
		default:
			return 0;
			break;
		}
		if (pushsdk_get_user_by_pin2(user_pin2,user) <= 0) {
			return 0;
		}
	} else if (strncmp(gTransFlag,"0000000000",10) == 0) {
		;
	} else if (user->PIN > 0) {
		if (!(svr_is_need_data(gTransFlag,PUSH_SVR_EN_USER) || \
			svr_is_need_data(gTransFlag,PUSH_SVR_CHG_USER))) {
			return 0;	
		}
	}
	return ret;
}
static int fp_filter(void *data, char *param)
{
	int ret = 0;
	char gTransFlag[256];
	char user_pin2[25];
	TOPLog log;
	pushsdk_fp_tmp_t *tmp = (pushsdk_fp_tmp_t*)data;
	TUser user;
	
	if ((data == NULL) || (param == NULL)) {
		return -1;
	}
	
	extractValueSample(param, "gTransFlag", '\n', gTransFlag,sizeof(gTransFlag));
	if (strncmp(param,"OPLOG",5) == 0) {
		strToOplog(param+6,&log,user_pin2);
		if ((log.OP == OP_CHG_FP) && (svr_is_need_data(gTransFlag,PUSH_SVR_CHG_FP))) {
			ret = 1;
		} else if ((log.OP == OP_ENROLL_FP) && (svr_is_need_data(gTransFlag,PUSH_SVR_EN_FP))) {
			ret = 1;
		} 
		if (ret != 1) {
			return 0;
		}
		if (pushsdk_get_user_by_pin2(user_pin2,&user) <= 0) {
			return ret;
		} 
		
		if (pushsdk_get_fp_tmp(user.PIN,log.Users[2],(char *)(&(tmp->fp_tmp))) > 0)  {
			tmp->fp_tmp.Size += 6;
			memcpy(tmp->pin2,user_pin2,strlen(user_pin2)+1);
			ret = 1;
		}
	} else if (svr_is_need_data(gTransFlag,PUSH_SVR_CHG_FP) || \
			(svr_is_need_data(gTransFlag,PUSH_SVR_EN_FP))) {
		ret = 1;
	} 
	
	return ret;
}


static int userpic_filter(void *data, char *param)
{
	char gTransFlag[256];
	char user_pin2[25];
	TOPLog oplog;
	char strFileName[50];
	char strFilePath[100];
	TUser user;
	FILE * fp;
	if ((data == NULL) || (param == NULL)) {
		return -1;
	}
	extractValueSample(param, "gTransFlag", '\n', gTransFlag,sizeof(gTransFlag));
	if (strncmp(param,"OPLOG",5) == 0) {
		strToOplog(param+6,&oplog,user_pin2);
		if (!((OP_ENROLL_USERPIC == oplog.OP) && 
			(svr_is_need_data(gTransFlag,PUSH_SVR_USER_PIC)))) {
			return -1;
		}
		if (pushsdk_get_user_by_pin2(user_pin2,&user) <= 0) {
			return 0;
		}
		snprintf(strFileName,sizeof(strFileName),"%s.jpg",user.PIN2);
		snprintf(strFilePath,sizeof(strFilePath),"%s",GetPhotoPath(strFileName));
		if ((fp=fopen(strFilePath,"rb")) == NULL) {
			return 0;
		} else {
			fclose(fp);
		}
		memcpy(data, &user, sizeof(user));
		return 1;
	} else if (svr_is_need_data(gTransFlag,PUSH_SVR_USER_PIC)) {
		return 1;
	}
	return 0;
}

static int attlog_filter(void *data, char *param)
{
	char gTransFlag[256];
	TTime t;
	TAttLog *log=data;
	U32 stamp,startStamp;

	char   startTime[20],endTime[20];
	time_t endStamp = 0;
	int attlogMode;

	if ((data == NULL) || (param == NULL)) {
		return -1;
	}
	 
	attlogMode = UPLOADE_MODE_NOR;	
	memset(startTime,0,sizeof(startTime));
	memset(endTime,0,sizeof(endTime));
	
	extractValueSample(param, "StartTime", '\t', startTime,sizeof(startTime)-1);
	extractValueSample(param, "EndTime", '\0', endTime,sizeof(endTime)-1);
	if  ((strlen(startTime)> 0) || (strlen(endTime) > 0)) {
		attlogMode = UPLOADE_MODE_QUERY;
		startStamp = StrTimeToStamp(startTime);
		endStamp = StrTimeToStamp(endTime);
		if ((startStamp < 0) ||(endStamp <= 0)) {
			return -1;
		}
	}

	if (attlogMode == UPLOADE_MODE_NOR) {
		if (!checkIntKey(param,"startStamp",&startStamp)) {
			return -1;
		}
		extractValueSample(param, "gTransFlag", '\n', gTransFlag,sizeof(gTransFlag));
		if (!svr_is_need_data(gTransFlag,PUSH_SVR_ATTLOG)) {
			return -2;
		}
	}
	
	if (!FDB_IsOKLog(log)) {
		return 0;
	}
	stamp = log->time_second;
	
	if (attlogMode == UPLOADE_MODE_NOR) {
		return data_is_alaredy_trans(stamp,startStamp);
	} else {
		OldDecodeTime(log->time_second, &t);
		stamp = mktime(&t);
		if ((stamp >= startStamp) && (stamp <= endStamp)) {
			return 1;
		} else {
			return 0;
		}
	}
}


static int attphoto_filter(void *data, char *param)
{
	U32 stamp,startStamp;
	char gTransFlag[256];
	TPhotoIndex *photodata=(TPhotoIndex*)data;
	char   startTime[20];
	char endTime[20];
	time_t endStamp;
	TTime t;
	
	if ((data == NULL) || (param == NULL)) {
		return -1;
	}
	stamp = photodata->createtime;
	memset(startTime,0,sizeof(startTime));
	memset(endTime,0,sizeof(endTime));
	
	extractValueSample(param, "StartTime", '\t', startTime,sizeof(startTime)-1);
	extractValueSample(param, "EndTime", '\0', endTime,sizeof(endTime)-1);
	
	if  ((strlen(startTime)> 0) || (strlen(endTime) > 0)) {
		startStamp = StrTimeToStamp(startTime);
		endStamp = StrTimeToStamp(endTime);
		if ((startStamp < 0) ||(endStamp <= 0)) {
			return -1;
		}
		OldDecodeTime(photodata->createtime, &t);
		stamp = mktime(&t);
		if ((stamp >= startStamp) && (stamp <= endStamp)) {
			return 1;
		}
		return 0;
	}
	
	if (!checkIntKey(param,"startStamp",&startStamp)) {
		return -1;
	}
	memset(gTransFlag,0,sizeof(gTransFlag));
	extractValueSample(param, "gTransFlag", '\n', gTransFlag,sizeof(gTransFlag));
	/*for att2008 tran Att photo*/
	if (strncmp(gTransFlag,"0000000000",10) != 0) {
		if (!svr_is_need_data(gTransFlag,PUSH_SVR_ATTPHOTO)) {
			return -2;
		}
	}
	return data_is_alaredy_trans(stamp,startStamp);
}
static int operlog_format(void *data, char *text, size_t *stamp,char *filepath)
{
	int ret=0;
	TTime t,gCurTime;
	TOPLog *log=(TOPLog*)data;
	TUser user;
	U16 pin;

	if ((data == NULL) || (text == NULL) ||(stamp == NULL)) {
		return -1;
	}
	
	GetTime(&t);
	if (OldEncodeTime(&t)>= (log->time_second-10)) {
		*stamp=log->time_second;
	}
	
	OldDecodeTime(*stamp, &t);
	GetTime(&gCurTime);
	if (t.tm_mon<0 || t.tm_mday<0 || t.tm_hour<0 ||\
		t.tm_min<0 || t.tm_sec<0 || t.tm_year>gCurTime.tm_year+1) {
		return 0;
	}
	pin = log->Users[0];
	if (!(is_oper_user(log->OP))) {
		ret = sprintf(text,"OPLOG %d\t%d\t%04d-%02d-%02d %02d:%02d:%02d\t%d\t%d\t%d\t%d\n",
					log->OP, log->Admin, t.tm_year+1900, t.tm_mon+1, t.tm_mday,\
					t.tm_hour, t.tm_min, t.tm_sec,\
					log->Users[0], log->Users[1], log->Users[2], log->Users[3]);
	} else if (pushsdk_get_user_by_pin(pin,&user) > 0) {
		ret = sprintf(text,"OPLOG %d\t%d\t%04d-%02d-%02d %02d:%02d:%02d\t%s\t%d\t%d\t%d\n",\
				   log->OP, log->Admin, t.tm_year+1900, t.tm_mon+1, t.tm_mday,\
					t.tm_hour, t.tm_min, t.tm_sec,\
					user.PIN2, log->Users[1], log->Users[2], log->Users[3]);
	} 
		
	return ret;
}

static int user_format(void *data, char *text, size_t *stamp,char *filepath)
{
	char card[40]={0};
	char pwd[9]={0};
	char name[25]={0};
	TUser *user = (TUser *)data;
	
	if ((text == NULL) || (user == NULL)) {
		return -1;
	}
	
	if (user->Card[0] ||  user->Card[1] || user->Card[2] || user->Card[3] || user->Card[4]) {
		snprintf(card,sizeof(card), "\tCard=[%02x%02x%02x%02x00]",user->Card[0], user->Card[1], user->Card[2], user->Card[3]);
	} else {
		snprintf(card,sizeof(card), "\tCard=");
	}
	memcpy(pwd, user->Password, sizeof(user->Password));
	memcpy(name, user->Name, sizeof(user->Name));
	if (user->PIN2[0] == '\0') {
		return 0;
	}
	return sprintf(text, "USER PIN=%s\tName=%s\tPri=%d\tPasswd=%s%s\tGrp=%d\tTZ=%04X%04X%04X%04X\n",\
				user->PIN2, name, user->Privilege, pwd,card,user->Group,\
				user->TimeZones[0],user->TimeZones[1],user->TimeZones[2],user->TimeZones[3]);

}
static int fp_format(void *data, char *text, size_t *stamp,char *filepath)
{
	pushsdk_fp_tmp_t *tmp = (pushsdk_fp_tmp_t*)data;
	char tmpStr[ZKFPV10_MAX_FP_LEN];

	if ((data == NULL) || (text == NULL)) {
		return 0;
	}
	base64_encode(tmp->fp_tmp.Template, tmp->fp_tmp.Size, (unsigned char *)tmpStr);
	return sprintf(text, "FP PIN=%s\tFID=%d\tSize=%d\tValid=%d\tTMP=%s\n", \
				tmp->pin2,  tmp->fp_tmp.FingerID, strlen(tmpStr),  tmp->fp_tmp.Valid, tmpStr);

}
static int userpic_format(void *data, char *text, size_t *stamp,char *strFilePath)
{
	PUser puser;
	char strFileName[50];
	FILE *fStream;
	int textLen;
	unsigned char PhotoSrc[30*1024];
	unsigned char PhotoEncode[41*1024];
	size_t count = 0;
	size_t nread = 0;
	
	if  ((NULL == data) || (strFilePath == NULL) ||(text == NULL)) {
		return -1;
	}
	textLen = 0;
	puser = (PUser) data;
	snprintf((char *)strFileName,sizeof(strFileName),"%s.jpg",puser->PIN2);
	sprintf((char *)strFilePath,"%s",GetPhotoPath((char *)strFileName));
	/*
	*USERPIC PIN=%d\nFileName=%s\n Size=%d\nContent=ssss
	*/

	fStream = fopen(strFilePath,"rb");
	textLen = file_get_length(fStream);
	if (textLen >=  30*1024 || (textLen < 0)) {
		return 0;
	}
	fseek(fStream, 0, SEEK_SET);
	while (textLen - count > 0) {
		do {
			nread = fread(PhotoSrc+count, sizeof(char),textLen-count, fStream);
		} while (nread < 0 && (errno == EINTR || errno == EAGAIN));

		if (nread > 0) {
			count += nread;
		} else {
			return -1;
		}
	}
	fclose(fStream);		
	memset(PhotoEncode,0,sizeof(PhotoEncode));
	textLen = base64_encode(PhotoSrc, textLen, PhotoEncode);
	textLen = sprintf(text,"USERPIC PIN=%s\tFileName=%s\tSize=%d\tContent=%s\n",
						puser->PIN2,strFileName,textLen,PhotoEncode);
	*strFilePath = '\0';
	return textLen;
}

static int attlog_format(void *data, char *text, size_t*stamp,char *filepath)
{
	int len=0;
	TTime t;
	TAttLog *log=data;
	
	if ((data == NULL) || (text == NULL)) {
		return -1;
	}

	GetTime(&t);
	if ((stamp != NULL) && (OldEncodeTime(&t) >= (log->time_second-10))) {
		*stamp=log->time_second;
	}
	OldDecodeTime(log->time_second, &t);
	len = sprintf(text,"%s\t%04d-%02d-%02d %02d:%02d:%02d\t%d\t%d\n",\
				fmtPinLog(log), t.tm_year+1900, t.tm_mon+1, t.tm_mday, \
				t.tm_hour, t.tm_min, t.tm_sec,log->status, log->verified);
	return len;
}


void stamp_display(time_t *stamp)
{
	char namestr[100];
	TTime tmptime;
	
	OldDecodeTime((time_t)*stamp, &tmptime);

	snprintf(namestr,sizeof(namestr), "%04d-%02d-%02d\t%02d-%02d-%02d", \
				tmptime.tm_year+1900, tmptime.tm_mon+1,\
				tmptime.tm_mday, tmptime.tm_hour, tmptime.tm_min, \
				tmptime.tm_sec);
	printf("%s\n",namestr);
}
static int attphoto_format(void *data, char *text, size_t *stamp,char *filepath)
{
	int len=0;
	char namestr[100];
	TTime tmptime;
	TPhotoIndex *photodata=(TPhotoIndex*)data;

	if ((data == NULL) || (text == NULL) ||(stamp == NULL) || (filepath == NULL)) {
		return -1;
	}
	GetTime(&tmptime);
	if ((stamp!=NULL) && (OldEncodeTime(&tmptime)>=photodata->createtime)) {
		*stamp=photodata->createtime;
	}
	OldDecodeTime((time_t)*stamp, &tmptime);
	/*pass user*/
	if (photodata->pictype == 0) {
		sprintf(namestr, "%04d%02d%02d%02d%02d%02d-%s.jpg", \
				tmptime.tm_year+1900, tmptime.tm_mon+1,\
				tmptime.tm_mday, tmptime.tm_hour, tmptime.tm_min, \
				tmptime.tm_sec,photodata->userid);
		sprintf(filepath, "%s%s", GetCapturePath("capture/pass/"), namestr);

	} else {
		sprintf(namestr, "%04d%02d%02d%02d%02d%02d.jpg", \
			 	tmptime.tm_year+1900, tmptime.tm_mon+1,\
				 tmptime.tm_mday, tmptime.tm_hour, \
				 tmptime.tm_min, tmptime.tm_sec);
		sprintf(filepath, "%s%s", GetCapturePath("capture/bad/"), namestr);
	}

	len = file_get_length_by_name(filepath);
	if (len <= 0) {
		return 0;
	}
	len = sprintf(text,"PIN=%s\nSN=%s\nsize=%d\nCMD=uploadphoto",namestr,SN,len);
	return len;
}
static int pushsdk_del_user(char *pin2)
{
	return pushsdk_deal_data((char *)pin2,strlen(pin2)+1,\
					PUSHSDK_DATA_DEL,FCT_USER,NULL,0);
}
static int user_del(char *param)
{
	char pin2[MAXPIN2LEN];

	if (param == NULL) {
		return -1;
	}
	
	if (getPINFrom(param, pin2) > 0) {
		return pushsdk_del_user((char *)pin2);
	} else {
		return -2;
	}
}
static int  pushsdk_del_fp(char *pin2,int fid)
{
	return pushsdk_deal_data((char *)&fid,sizeof(fid),PUSHSDK_DATA_DEL,\
							FCT_FINGERTMP,pin2,strlen(pin2)+1);
}
/*DELETE FINGERTMP PIN=%d\tFID=%d */
static int fp_del(char *param)
{
	int fid;
	char pin2[MAXPIN2LEN];

	if (param == NULL) {
		return -1;
	}
	if (getPINFrom(param,pin2) <= 0) {
		return -2;
	}
	fid = extractValueInt(param, "FID", '\t', 15);
	
	return pushsdk_del_fp(pin2,fid);
	
}
static int  userpic_del(char *param)
{
	char pin2[25];
	char strFileName[30];
	char strFilePath[100];
	
	if (param == NULL) {
		return -1;
	}
	memset(pin2,0,sizeof(pin2));
	extractValueSample(param, "PIN", '\t', pin2,sizeof(pin2));

	if ((pin2[0] == 0)) {
		return -1; 
	}
	snprintf(strFileName,sizeof(strFileName),"%s.jpg",pin2);
	snprintf(strFilePath,sizeof(strFilePath),"%s",GetPhotoPath(strFileName));

  	return unlink(strFilePath);
}
static int pushsdk_del_sms(TSms *sms)
{
	return pushsdk_deal_data((char *)sms,sizeof(TSms),\
						PUSHSDK_DATA_DEL, FCT_SMS,NULL,0);
}
static int  sms_del(char *strSvrCmd)
{
	int smsUid;
	int ret;
	TSms sms;

	ret = -1;
	if (strSvrCmd != NULL) {
		return ret;
	}
	smsUid = extractValueInt(strSvrCmd, "UID", '\t', 0);
	if (FDB_GetSms(smsUid,&sms) != NULL) {
		ret =  pushsdk_del_sms(&sms);
	}
	return ret;
}
static int  pushsdk_add_user(TUser *user)
{
	return pushsdk_deal_data((char *)user,sizeof(TUser),\
						PUSHSDK_DATA_ADD, FCT_USER,NULL,0);
}
static unsigned char *bcd2str(unsigned char *bcd, int length) 
{ 
	int i=0, tmp; 
     	static unsigned char decstr[50];
	memset(decstr, 0, sizeof(decstr));
     	for(i=0; i<length; i++) 
     	{ 
		tmp = ((bcd[i]>>4)&0x0F)*10 + (bcd[i]&0x0F);  
       		sprintf(decstr +(2*i), "%02d", tmp);
     	}
     	return decstr; 
}

static int belltime_update(char *param)
{	
	TUpParam devparam;
	TBellNew up_bell;
	unsigned short int rechecttime;
	U16 i;
	U8 h, m;
	if(NULL == param)
	{
		return -1;
	}
	memset(&devparam, 0, sizeof(TUpParam));
	memcpy(&devparam, param, sizeof(TUpParam));
	rechecttime = devparam.VFInterval;
	printf("vf =%d\n", rechecttime);
	printf("card mode%d\n", devparam.CardMode);

	SaveInteger("ReadCardInterval", rechecttime);
	for(i=0; i<16;i++)
	{	
		h = 0x00;
		m = 0x00;
		memset(&up_bell, 0, sizeof(TBellNew));
		up_bell.BellID = i+1;
		up_bell.SchInfo |= 0xFF;
		h = devparam.BellTime[i].time[0];
		m = devparam.BellTime[i].time[1];
		BCDTODEC(h);
		BCDTODEC(m);
		printf("hour=%d, min=%d i=%d\n", h, m, i);
		up_bell.BellTime[0] = h;
		up_bell.BellTime[1] = m;
		up_bell.BellWaveIndex = 1;
		up_bell.BellVolume = 99;
		up_bell.BellTimes = 3;
		if(99 !=h && 99 !=m)
		{
			if (FDB_GetBell(up_bell.BellID, NULL) != NULL)
			{
				if (FDB_ChgBell(&up_bell)==FDB_ERROR_IO){
					printf("change alarm status failed!\n");
					return -1;
				}
			}
			else
				FDB_AddBell(&up_bell);
		}
	}
	return 0;
}

static int user_update(char *param)
{	
	TSerialCardNo serialcardno;
	TUserHead userhead;
	TUserData uplouser;
	TUser usercpy, deluser, u;
	int i, count, ret;
	unsigned int cardnum;
	unsigned char name[20];
	unsigned char pin2[20];
	char *TZ=NULL, *card = NULL;
	int pin;
	int k;
	unsigned char CardSerialNumber[10];
	int cardstatus;		
	if (param == NULL) {
		return -1;
	}
	memset(&userhead, 0, sizeof(TUserHead));	
	memcpy(&userhead, param, sizeof(TUserHead));
	
	count = ntohs(userhead.UserNum);
	printf("update user count=%d\t remain user=%d\n", count, ntohl(userhead.UserRemain));
	for(i=1; i<=count; i++)
	{	
		memset(name, 0, 24);
		memset(pin2, 0, 24);
		memset(&uplouser, 0, sizeof(TUserData));	
		memset(&usercpy, 0, sizeof(TUser));
		memset(&deluser, 0, sizeof(TUser));
		memset(&u, 0, sizeof(TUser));
		memset(&serialcardno, 0, sizeof(TSerialCardNo));
		memcpy(&uplouser, (param+sizeof(TUserHead)+((i - 1)*sizeof(TUserData))), sizeof(TUserData));
		if((uplouser.CardStatus > 1) || (uplouser.Privilege > 2))
			return -4;
		changestr(uplouser.Name, name, 20);
		changestr(uplouser.PIN2, pin2, 16);
		Str2UTF8(21, name, usercpy.Name);			//name
		Str2UTF8(21, pin2, usercpy.PIN2);				//pin2
		memcpy(usercpy.Card, uplouser.Card, 4);
		memcpy(&cardnum, usercpy.Card, 4);
		//card = bcd2str(uplouser.CardSerial, 5);			// serial num
		TZ = bcd2str(uplouser.Password, 6);
		cardstatus = uplouser.CardStatus;					//card status
		usercpy.Privilege = uplouser.Privilege;				//pri
		memcpy(serialcardno.CardSerialNumber, uplouser.CardSerial, 10);
		memcpy(usercpy.Password, TZ, 6);
		printf("\n\t用户名	   :%s\n", usercpy.Name);
		printf("\t工号	   :%s\n", usercpy.PIN2);
		printf("\t卡号	   :%u\n", htonl(cardnum));
		printf("\t密码	   :%s\n", usercpy.Password);
		printf("\t卡状态	   :%d\n", cardstatus);
		printf("\t权限	   :%d\n", usercpy.Privilege);
		//printf("uplouser.CardSerial=%s\n", card);
		if(i == count)
		{
			GetLastUserCardSerial(uplouser.CardSerial);
		}
		if(0x00 == uplouser.CardStatus)
		{	
			if(NULL != FDB_GetUserByCharPIN2(usercpy.PIN2, &deluser))
			{
				ret = FDB_DelUser(deluser.PIN);
			}
			else
			{
				printf("can't find the user\n");
			}
		}
		if(0x01 == uplouser.CardStatus)
		{	
			ret = pushsdk_add_user(&usercpy);
			if(NULL != FDB_GetUserByCharPIN2(usercpy.PIN2, &u))
			{	
				serialcardno.PIN = u.PIN;
				if(FDB_GetSerialCardNumber(u.PIN, NULL) != NULL)
				{
					ret = FDB_ChgSerialCardNumber(&serialcardno);
					printf("[%s] [%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
				}
				else
				{
					ret = FDB_AddSerialCardNumber(&serialcardno);
					printf("[%s] [%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
				}
			}
		}
	}
	return ret;
}

#if 0
	memset(&usercpy,0,sizeof(usercpy));
	if (getPINFrom(param, usercpy.PIN2) == 0) {
		return -1;
	}
	extractValueSample(param, "Passwd", '\t', usercpy.Password,sizeof(usercpy.Password));
	memset(card,0,sizeof(card));
	i = extractValueSample(param, "Name", '\t', card,sizeof(card));
	if (i >= 0) {
		memcpy(usercpy.Name,card,sizeof(usercpy.Name)-1);
	}
	usercpy.Privilege = extractValueInt(param, "Pri", '\t', 0);
	usercpy.Group = extractValueInt(param, "Grp", '\t', 0);
	extractValueSample(param, "TZ", '\t', TZ,sizeof(TZ));
	memset(card,0,sizeof(card));
	extractValueSample(param, "Card", '\t', card,sizeof(card));
	if (card[0]=='[' && card[11]==']') {
		for (i=1; i<=10; i++) {
			if (card[i]>='a' && card[i]<='z') {
				card[i]-=32;
			}
		}
		for (i=0; i<4; i++) {
			usercpy.Card[i] = str2Hex(card[1+i*2], card[2+i*2]);
		}
	} else {
		for (i=0; i<4; i++) {
			usercpy.Card[i] = 0;
		}
	}
	return pushsdk_add_user(&usercpy);
	
#endif




#if 0
static int user_update(char *param)
{
	printf("[%s]____[%d]_______\n",__FUNCTION__,__LINE__);
	TUser usercpy;
	int i;
	char card[2*MAXNAMELENGTH];
	char TZ[100];

	if (param == NULL) {
		return -1;
	}
	memset(&usercpy,0,sizeof(usercpy));
	if (getPINFrom(param, usercpy.PIN2) == 0) {
		return -1;
	}
	extractValueSample(param, "Passwd", '\t', usercpy.Password,sizeof(usercpy.Password));
	memset(card,0,sizeof(card));
	i = extractValueSample(param, "Name", '\t', card,sizeof(card));
	if (i >= 0) {
		memcpy(usercpy.Name,card,sizeof(usercpy.Name)-1);
	}
	usercpy.Privilege = extractValueInt(param, "Pri", '\t', 0);
	usercpy.Group = extractValueInt(param, "Grp", '\t', 0);
	extractValueSample(param, "TZ", '\t', TZ,sizeof(TZ));
	memset(card,0,sizeof(card));
	extractValueSample(param, "Card", '\t', card,sizeof(card));
	if (card[0]=='[' && card[11]==']') {
		for (i=1; i<=10; i++) {
			if (card[i]>='a' && card[i]<='z') {
				card[i]-=32;
			}
		}
		for (i=0; i<4; i++) {
			usercpy.Card[i] = str2Hex(card[1+i*2], card[2+i*2]);
		}
	} else {
		for (i=0; i<4; i++) {
			usercpy.Card[i] = 0;
		}
	}
	return pushsdk_add_user(&usercpy);

}
#endif 
static int pushsdk_add_fp_tmp(char *pin2,TZKFPTemplate *fp_tmp)
{
	return pushsdk_deal_data((char *)fp_tmp,sizeof(TZKFPTemplate),\
					PUSHSDK_DATA_ADD, FCT_FINGERTMP,(void *)pin2,strlen(pin2)+1);
}
static int fp_update(char *param)
{
	int  pin=0, ret=0;
	char pin2[MAXPIN2LEN+1];
	int len;
	char temStr[ZKFPV10_MAX_FP_LEN*2+1024];	
	TZKFPTemplate tempcpy;

	if (param == NULL) {
		return -1;
	}
	if (!getPINFrom(param,pin2)) {
		return -1;
	}
	memset(&tempcpy,0,sizeof(tempcpy));
	tempcpy.PIN=pin;
	tempcpy.FingerID = extractValueInt(param, "FID", '\t', 0);
	tempcpy.Size = extractValueInt(param, "Size", '\t', 0);
	tempcpy.Valid = extractValueInt(param, "Valid", '\t', 1);
	extractValueSample(param, "TMP", '\t', temStr,sizeof(temStr));

	int tempencodelen = strlen(temStr);
	int templen = tempcpy.Size;
	int tempdecodelen = 0;
	len = hex_decode((unsigned char*)temStr, tempcpy.Template);
	if(len <= 0) {
		len = base64_decode((unsigned char*)temStr, tempcpy.Template);
	}
	tempcpy.Size = len-6;
	tempdecodelen = tempcpy.Size+6;
	if ((templen>0) && (tempencodelen != templen)) {
		return -9;
	}
	if ((len <= 100) ||(len > 20*1024)) {	
		ret=-11;
	} else {
		ret =  pushsdk_add_fp_tmp(pin2,&tempcpy);
	}
	return ret;
}
static int  userpic_update(char *param)
{
	char pin2[24];
	int PhotoSize;
	unsigned char PhotoEncode[32*1024];
	unsigned char PhotoDecode[32*1024];
	unsigned char strFileName[30];
	unsigned char strFilePath[100];
	FILE *fStream;

	if (param == NULL) {
		return -1;
	}
	memset(pin2,0,sizeof(pin2));
	extractValueSample(param, "PIN", '\t', pin2,sizeof(pin2));

	if ((pin2[0] == 0) || (pushsdk_get_user_by_pin2(pin2,NULL) <= 0)) {
		return -1; 
	}

	PhotoSize = extractValueInt(param, "Size", '\t',-2);
	if ((PhotoSize == -2) || \
	(PhotoSize != extractValueSample(param, "Content", '\t', (char *)PhotoEncode,sizeof(PhotoEncode)))) {
		return -2;
	}
	
	PhotoSize = base64_decode(PhotoEncode, PhotoDecode);	

	snprintf((char *)strFileName,sizeof(strFileName),"%s.jpg",pin2);
	snprintf((char *)strFilePath,sizeof(strFilePath),"%s",GetPhotoPath((char *)strFileName));

	fStream = fopen((char *)strFilePath, "wb");
      fwrite((char *)PhotoDecode, sizeof(char), PhotoSize, fStream);
      fclose(fStream);
      sync();

      return 0;
}

extern char *strptime(const char *buf,const char *format, struct tm *tm); 
static int pushsdk_add_sms(TSms *sms)
{
	return pushsdk_deal_data((char *)sms,sizeof(TSms),\
						PUSHSDK_DATA_ADD, FCT_SMS,NULL,0);
}
static int sms_update(char *param)
{
	int ret = -1;
	char info[2*1024];
	int i;
	TSms sms;
	char start_time[128];
	TTime tms;
	
	if (param == NULL) {
		return -1;
	}
	i = extractValueSample(param, "MSG", '\t', info,sizeof(info));
	if (i <= 0) {
		return -2;
	}
	sms.Tag = extractValueInt(param, "TAG", '\t', 0xFE);
	sms.ID = extractValueInt(param, "UID", '\t', 1);
	sms.ValidMinutes = extractValueInt(param, "MIN", '\t', 0);
	i = extractValueSample(param, "StartTime", '\t', start_time,sizeof(start_time));
	if (i <= 0) {
		return -2;
	}

	strptime(start_time, "%Y-%m-%d %T", &tms);
	sms.StartTime = OldEncodeTime(&tms);
	memcpy((char *)(sms.Content), (char *)info, MAX_SMS_CONTENT_SIZE*2);
	ret = pushsdk_add_sms(&sms);
	return ret;
}
static int pushsdk_add_user_sms(char *pin2,TUData *udata)
{
	return pushsdk_deal_data((char *)udata,sizeof(TUData),\
			PUSHSDK_DATA_ADD, FCT_UDATA,(void *)(pin2),strlen(pin2)+1);
}
static int sms_user_update(char *param)
{
	TUser usercpy;
	int uid;
	TUData udata;

	if(param == NULL) {
		return -1;
	}
	uid = extractValueInt(param, "UID", '\t', 0);
	if (uid == 0) {
		return -2;
	}
	if (getPINFrom(param,usercpy.PIN2) != 1) {
		return -3;
	}
	FDB_CreateUData(&udata,0,uid);
	return pushsdk_add_user_sms(usercpy.PIN2,&udata);
}
#ifdef SUPPORT_FACE
static int pushsdk_add_face_tmp(char *pin2,TFaceTmp* facetmp)
{
	return pushsdk_deal_data((char *)facetmp,\
						sizeof(TFaceTmp)*FACE_NUM,\
						PUSHSDK_DATA_ADD,FCT_FACE,pin2,strlen(pin2)+1);
}

static int face_update(char *param)
{
	static char  current_face_pin2[MAXPIN2LEN+1] = {0};
	static int current_face_cnt = 0;
	static char current_face_flags[FACE_NUM];
	static int current_face_all = 0;
	int i = 0;
	int face_index = 0;
	int pin = 0;
	char buffer[128];
	TUser user;
	int ret = 0;
	int len = 0;
	int size;
	char temStr[PUSHSDK_MAX_BUFSIZ];

	if (param == NULL) {
		return -1;
	}
	if (getPINFrom(param, buffer) != 1) {
		return -20;
	}
	
	if (strcmp(current_face_pin2,buffer) != 0) {
		memcpy(current_face_pin2,buffer,sizeof(user.PIN2));
		memset(pushsdk_facetmp, 0x00, sizeof(pushsdk_facetmp));
		memset(current_face_flags, 0x00, sizeof(current_face_flags));
		current_face_cnt = 0;
		current_face_all = 0;
	}

	face_index = extractValueInt(param, "FID", '\t', 0);
	if ((face_index < 0) || (face_index >= sizeof(pushsdk_facetmp)/sizeof(TFaceTmp))) {
		return -4;
	}
	size = extractValueInt(param, "SIZE", '\t', 0);
	if (size == 0) {
		return -21;
	}
	memset(temStr,0,sizeof(temStr));
	extractValueSample(param, "TMP", '\t', temStr, sizeof(temStr));
	if (size != strlen(temStr)) {
		return -9;
	}
	len = hex_decode((unsigned char*)temStr, (unsigned char*)&pushsdk_facetmp[face_index]);
	if (len <= 0) {
		len = base64_decode((unsigned char*)temStr, (unsigned char*)&pushsdk_facetmp[face_index]);
	}
	pushsdk_facetmp[face_index].FaceID = face_index;
	pushsdk_facetmp[face_index].PIN = pin;
	pushsdk_facetmp[face_index].Valid  = extractValueInt(param, "VALID", '\t', 0);
	
	current_face_flags[face_index] = 0x01;
	
	if (current_face_cnt == FACE_NUM - 1) {
		for (i = 0; i < FACE_NUM; i++) {
			if (current_face_flags[i] == 0x00) {
				current_face_all = 0;
				break;
			} else {
				current_face_all = 1;
			}
		}

		if (current_face_all == 0) {
			return -3;
		}
		ret = pushsdk_add_face_tmp(current_face_pin2,pushsdk_facetmp);
		memset(pushsdk_facetmp, 0x00, sizeof(pushsdk_facetmp));
		memset(current_face_flags, 0x00, sizeof(current_face_flags));
		memset(current_face_pin2,0,sizeof(current_face_pin2));
		current_face_cnt = 0;
		current_face_all = 0;
	}  else {
		current_face_cnt++;
	}
	return ret;
}

static int pushsdk_del_face_tmp(char *pin2)
{
	return pushsdk_deal_data((char *)pin2,\
				strlen(pin2)+1,PUSHSDK_DATA_DEL,FCT_FACE,NULL,0);
}

static int face_del(char *param)
{
	char  tmpBuffer[25];
	
	/*DATA DELETE FACE PIN=125800000*/
	if (param == NULL) {
		return -1;
	}
	memset(tmpBuffer,0,sizeof(tmpBuffer));
	if (getPINFrom(param, tmpBuffer) != 1) {
		return -2;
	}
	return  pushsdk_del_face_tmp(tmpBuffer);

}

static int face_filter(void *data, char *param)
{
	int ret = -1;
	char gTransFlag[256];
	TOPLog log;
	TUser user;

	if ((param == NULL) || (data == NULL)) {
		return -1;
	}
	extractValueSample(param, "gTransFlag", '\n', gTransFlag,sizeof(gTransFlag));
	if (strncmp(param, "OPLOG", 5) == 0) {
		strToOplog(param + 6,&log,user.PIN2);
		if (!(((log.OP == OP_ENROLL_FACE) || (log.OP == OP_CHG_FACE)) \
			&& (svr_is_need_data(gTransFlag, PUSH_SVR_FACE)))) {
			return -1;
		}
		if (pushsdk_get_user_by_pin2(user.PIN2,&user) <= 0) {
			return 0;
		}
		if (pushsdk_get_user_face_info(&user, \
				upload_face_tmp,sizeof(upload_face_tmp)/sizeof(TFaceTmp)) > 0) {
			ret = 1;		
		}
	} else if (svr_is_need_data(gTransFlag, PUSH_SVR_FACE)) {
		ret = 1;
	}

	return ret;
}

static int face_format(void *data, char *text, size_t *stamp,char *filepath)
{
	pushsdk_face_tmp_t *face_tmp = (pushsdk_face_tmp_t*)data;
	char tmpStr[PUSHSDK_MAX_BUFSIZ];

	if ((data == NULL) || (text == NULL)) {
		return -1;
	}
	memset(tmpStr, 0x00, sizeof(tmpStr));
	base64_encode((unsigned char *)(&(face_tmp->facetmp)), \
				sizeof(TFaceTmp)-sizeof(face_tmp->facetmp.Face)+face_tmp->facetmp.Size, \
					(unsigned char *)tmpStr);

	return sprintf(text, "FACE PIN=%s\tFID=%d\tSIZE=%d\tVALID=%d\tTMP=%s\n",\
			face_tmp->pin2, face_tmp->facetmp.FaceID, strlen(tmpStr), face_tmp->facetmp.Valid, tmpStr);
}
#endif
void register_push_fun(void)
{
	pushsdk_register_fun("AC_UNLOCK",pushsdk_unlock);

	if (!gOptions.IsOnlyRFMachine) {
		pushsdk_register_fun("ENROLL_FP",pushsdk_enroll_fp);
	}

	pushsdk_register_data_fun("OPERLOG",FCT_OPLOG,0,TRUE,\
							   	NULL,NULL,operlog_filter,operlog_format);

	pushsdk_register_data_fun("USERINFO",FCT_USER,FCT_OPLOG,FALSE,\
								user_update,user_del,user_filter,user_format);
	
	if (!gOptions.IsOnlyRFMachine) {
		pushsdk_register_data_fun("FINGERTMP",FCT_FINGERTMP,FCT_OPLOG,FALSE,\
									fp_update,fp_del,fp_filter,fp_format);
	}
	
	pushsdk_register_data_fun("USERPIC",FCT_USERPIC,FCT_OPLOG,FALSE,\
								userpic_update,userpic_del,userpic_filter,userpic_format);
	
	pushsdk_register_data_fun("ATTLOG",FCT_ATTLOG,0,TRUE,\
								NULL,NULL,attlog_filter,attlog_format);

	pushsdk_register_data_fun("ATTPHOTO",FCT_PHOTOINDEX,0,TRUE,\
								NULL,NULL,attphoto_filter,attphoto_format);	

	pushsdk_register_data_fun("BELLTIME", FCT_BELL, 0, FALSE, belltime_update, NULL, NULL, NULL);
	if ((gOptions.IsSupportSMS) && (gOptions.IMEFunOn == 1)) {
		pushsdk_register_data_fun("SMS",FCT_SMS,FCT_OPLOG,FALSE,\
									sms_update,sms_del,NULL,NULL);			
		pushsdk_register_data_fun("USER_SMS",FCT_UDATA,FCT_OPLOG,FALSE,\
									sms_user_update,NULL,NULL,NULL);
	}
#ifdef SUPPORT_FACE
	if (gOptions.FaceFunOn) {
		pushsdk_register_data_fun("FACE",FCT_FACE,FCT_OPLOG,FALSE,\
							face_update,face_del,face_filter,face_format);
	}
#endif

	pushsdk_register_help_fun( "ATTLOG",FCT_ATTLOG,att_log_upload_filter,attlog_format);
	pushsdk_register_help_fun( "OPERLOG",FCT_OPLOG,oper_log_upload_filter,operlog_format) ;
}

void ReLoadOptions(void)
{
	LoadOptions(&gOptions);
}

void reset_alarm()
{
	gErrTimes=0;
	gAlarmDelay=0;
}

int  mainEnrollCmd(const char *cmd)
{
#if 1
	int pin;
	int fingerid=cmd[1];
	int retry=cmd[2];
	int owi=cmd[3];
	int ret=ERR_CANCEL;

	memcpy(&pin, cmd+4, sizeof(pin));
	do {
		int tmplen;
		TUser user;

		if (FDB_GetUser(pin, &user)) {
			ret = ProcessEnroll(user.PIN2, fingerid, &tmplen, owi);
		} else {
			break;
		}
/*
		if (ret!=ERR_REPEAT) {
			break;
		}
*/
		msleep(3000);
	} while (--retry > 0);
#endif
	return 0;
}

void sync_data_file_offset(int fd)
{
	int CurOffSet;
	int FileSize;

	if (fd < 0) {
		return ;
	}
	CurOffSet = lseek(fd, 0, SEEK_CUR);
	FileSize = get_file_size(fd);
	if (FileSize < CurOffSet) {
		CurOffSet = lseek(fd, 0, SEEK_SET);
	}
}
off_t pushsdk_get_file_offset(int contype)
{
	if (contype == FCT_ATTLOG) {
		return iCurAttLogPos;
	} else if (contype==FCT_OPLOG) {
		return iCurOptLogPos ;
	} else if (contype==FCT_PHOTOINDEX) {
		return iCurrAttPhotoPos;
	}
	return -1;
}
void pushsdk_set_file_offset(int contype,off_t offset)
{
	if (contype == FCT_ATTLOG) {
		 iCurAttLogPos = offset;
	} else if (contype==FCT_OPLOG) {
		 iCurOptLogPos = offset;
	} else if (contype==FCT_PHOTOINDEX) {
		 iCurrAttPhotoPos = offset;
	}
}
typedef  struct log_tmp_buffer {
	off_t start;
	off_t end;
	int conype;
	char buf[100*256];
} log_tmp_buf_t;
static int pushsdk_get_next_log(int contype,off_t offset,char *buffer)
{
	static log_tmp_buf_t log_tmp_buf = {-1,-1,0,{0}};
	int size = 0;
	int len;
	
	if (contype == FCT_OPLOG) {
		size = sizeof(TOPLog);
	} else if (contype == FCT_ATTLOG) {
		size = sizeof(TAttLog);
	} else if (contype == FCT_PHOTOINDEX) {
		size = sizeof(TPhotoIndex);
	} else {
		return 0;
	}
	if ((contype == log_tmp_buf.conype) && (log_tmp_buf.start <= offset) && (log_tmp_buf.end >= offset)) {
		memcpy(buffer,log_tmp_buf.buf+(offset-log_tmp_buf.start)*size,size);
		return 1;
	} 

	len = pushsdk_deal_data(&offset,sizeof(offset),PUSHSDK_DATA_GET_BY_OFFSET,\
		contype,log_tmp_buf.buf,sizeof(log_tmp_buf.buf)/size);
		
	if (len > 0) {
		log_tmp_buf.start = offset;
		log_tmp_buf.end = offset + len/size - 1 ;
		log_tmp_buf.conype = contype;
		memcpy(buffer,log_tmp_buf.buf+(offset-log_tmp_buf.start)*size,size);
		return 1;
	} 
	return 0;
}
static int FDB_ForDataFun(int ContentType, int fromFirst, ForARecFun fun, int MaxRecCount, void *param)
{
	int count=0;
	char buffer[1024*6];
	int ret = 0;
	off_t offset = 0;

	if (FCT_FINGERTMP==ContentType && gOptions.ZKFPVersion == ZKFPV10) {
		TZKFPTemplate tmp;
		ret = fun(&tmp, count, param);
		return ret;
#ifdef SUPPORT_FACE
	} else if (FCT_FACE == ContentType) {
		int i = 0;
		memset(upload_face_tmp, 0x00, sizeof(upload_face_tmp));
		for (i = 0; i < FACE_NUM; i++) {
			ret = fun(&upload_face_tmp[i], i, param);
			if (ret < 0 || (upload_face_tmp[i].facetmp.PIN <= 0)) {
				break;
			}
		}
		return ret;
#endif
	} else if (FCT_USER == ContentType ||(FCT_USERPIC == ContentType) \
			|| (FCT_FINGERTMP == ContentType)) {
		ret = fun(buffer, count, param);
		return ret;	
	}
	offset = pushsdk_get_file_offset(ContentType);
	if (offset == -1) {
		return ret;
	}
	while (pushsdk_get_next_log(ContentType,offset,buffer) > 0) {
		offset++;
		ret = fun(buffer, offset, param);
		if (ret < 0) {
			break;
		} else if (ret > 0) {
			count++;
		}
		if (MaxRecCount != TRANS_MAX_RECORD_NO_LIMIT) {
			if (count >= MaxRecCount) {
				break;
			}
		}
	}
	return ret;
}

int FDB_ForAllData(int ContentType, ForARecFun fun, int MaxRecCount, void *param)
{
	int ret;

	pushsdk_set_file_offset(ContentType,0);
	
	ret = FDB_ForDataFun(ContentType, TRUE, fun, MaxRecCount, param);

	return ret;
}

int FDB_ForDataFromCur(int ContentType, ForARecFun fun, int MaxRecCount, void *param)
{
	return FDB_ForDataFun(ContentType, FALSE, fun, MaxRecCount, param);
}

void iClock_Invoke(void)
{
	static int HBEchoPendings;
	static int HBEchoTime;

	gOptions.iClockServerStatus = pushsdk_check();
	if ( gOptions.EchoInterval > 0 && 
	  (OldEncodeTime(&gCurTime) > HBEchoTime+gOptions.EchoInterval)) {
		HBEchoTime = OldEncodeTime(&gCurTime);
		if ((gOptions.iClockServerStatus==1) || (gOptions.iClockServerStatus>5)) {
			HBEchoPendings++;
			if (HBEchoPendings > gOptions.EchoFailRestart && gOptions.EchoFailRestart > 0) {
				pushsdk_re_init();
				HBEchoPendings = 0;
				gOptions.iClockServerStatus = 1;  
			}
		} else {
			HBEchoPendings = 0;
		}
	}
}

int ServerAuthUpdate(FILE *pfileRes,const char *pszHttpInfoHeader, void *pUserInfo)
{
	int index;
	char abBuffer[HTTP_MAXBUF_LEN];
	char strTZ[20];
	unsigned int u32PTSize;
	char abPhoto[HTTP_MAXBUF_LEN];
	char strPhotoName[256];
	FILE* pfilePhoto;
	char ch;
	PUser  userinfo;

	userinfo = (PUser) pUserInfo;
	
	if (NULL == pfileRes) {
		return 0;
	}
	
	if((strncmp(pszHttpInfoHeader, "HTTP/1.0 200",12) != 0) 
	&& (strncmp(pszHttpInfoHeader, "HTTP/1.1 200 ",12) != 0)) {
		if(pfileRes)
			fclose(pfileRes);
		return 0;
	}
	
	index = 0;
	memset(abBuffer, 0x00, sizeof(abBuffer));
	while (!feof(pfileRes)) {
		ch = fgetc(pfileRes);
		if (index < HTTP_MAXBUF_LEN) {
			abBuffer[index++] = ch;
		} else {
			break;
		}
		if ( '\r' == ch || '\n' == ch) {
			abBuffer[index] = '\0';
			index = 0;
			/*Auth=Success\tPIN=%d\tName=%s\tPri=%d\tGrp=%d\tTZ=%s\tPhotoSize=%d\tPhoto=%s\n*/
			if (strncmp(abBuffer,"AUTH=Success",12) != 0) {
				if(pfileRes)
					fclose(pfileRes);
				return 0;
			}
			memset(userinfo->PIN2, 0x00 , sizeof(userinfo->PIN2));
			extractValueSample(abBuffer, "PIN", '\t', userinfo->PIN2,sizeof(userinfo->PIN2));
			memset(userinfo->Name, 0x00 , sizeof(userinfo->Name));
			extractValueSample(abBuffer, "Name", '\t', userinfo->Name,sizeof(userinfo->Name));
			userinfo->Privilege = extractValueInt(abBuffer, "Pri", '\t', 0);
			userinfo->Group = extractValueInt(abBuffer, "Grp", '\t', 0);
			extractValueSample(abBuffer, "TZ", '\t', strTZ,sizeof(strTZ));

			if (u32PTSize <= 0 && u32PTSize > HTTP_MAXBUF_LEN) {
				if(pfileRes)
					fclose(pfileRes);
				return 1;
			}
			
			u32PTSize = extractValueInt(abBuffer, "PhotoSize", '\t', 0);
			memset(abPhoto, 0x00, sizeof(abPhoto));
			extractValueSample(abBuffer, "Photo", '\t', abPhoto,sizeof(abPhoto));
			memset(abBuffer, 0x00, sizeof(abBuffer));
			base64_decode((unsigned char *)abPhoto, (unsigned char *)abBuffer);
			memset(strPhotoName, 0x00, sizeof(strPhotoName));
			sprintf(strPhotoName,"%s/%s_Svr.jpg", GetPhotoPath(""), userinfo->PIN2);

			pfilePhoto = fopen(strPhotoName,"wb");
			if (pfilePhoto == NULL) {
				if(pfileRes)
					fclose(pfileRes);
				return 1;
			}
			fwrite(abBuffer, sizeof(char), u32PTSize, pfilePhoto);
			fsync(fileno(pfilePhoto));
			fclose(pfilePhoto);
		}
	}
	if(pfileRes)
		fclose(pfileRes);
	return 0;
}

void DelRmAttUser(void)
{
	time_t ttCurTime;
	struct tm * TTCurDateTime;
	char timeChar[16];
	static int isPowerOn = 1;
	
	ttCurTime = time(NULL);
	TTCurDateTime = localtime(&ttCurTime);
	strftime(timeChar,3,"%M",TTCurDateTime);

	 if ((strcmp("00",timeChar) == 0) || (isPowerOn == 1))  {
		TSearchHandle sh;
		TUserDLTime	UserDLTimelog;

		isPowerOn = 0;
		sh.ContentType = FCT_USER_DL_TIME;
		sh.buffer=(char*)&UserDLTimelog;

		SearchFirst(&sh);
		while (!SearchNext(&sh)) {
			if (UserDLTimelog.m_pin == 0) {
				return ;
			}
			if ((ttCurTime - UserDLTimelog.m_downLoadTime) >= (gOptions.RmUserSaveTime*24*60*60)) {
				FDB_DelUserDLTime(UserDLTimelog.m_pin);
				FDB_DelUser(UserDLTimelog.m_pin);
			}
		}
	}
}

int SaveUserDLStamp(char *pin2)
{
	int i;
	TUser user;
	TUserDLTime UserDLTimelog;
	
	i = CheckUserPIN(pin2, &user);

	if (i <= 0) {
		return 0;
	}
	UserDLTimelog.m_pin = i;
	UserDLTimelog.m_downLoadTime = time(NULL);
	FDB_AddUserDLTime(&UserDLTimelog);
	
	return i;
}

void GetCurTimeByStr(char * pstrCutTime)
{
	struct tm *pstm = NULL;
	time_t ttCurStamp;

	time(&ttCurStamp);
	pstm = localtime(&ttCurStamp);
	strftime(pstrCutTime,20,"%F %H:%M:%S",pstm);
}
void  AppendRTLogToFile(int EventFlag, unsigned char *Data, int Len)
{
	int c=0;
	char buf[256]={0};
	int obj;
	char doorState[20];
	char alarmState[20];
	 FILE *fAccState;

	 if (gOptions.PushLockFunOn != 1) {
	 		return ;
	 }

	strncpy(doorState,"NULL",4);
	strncpy(alarmState,"NULL",4);


	obj =(Data && (Len>=2))? Data[0]+Data[1]*256: 0;
	switch(EventFlag) {
    	case EF_ALARM:
	{
		int t;
		t = (Data && Len>=8)?Data[4]+Data[5]*256: 0;

		switch(obj)
		{
		    case News_Door_Button: 
		        sprintf(buf, "Event=Unlock\tObject=Button");
		        strcpy(doorState, "UNLOCK\n");
		        strcpy(alarmState, "NULL");
		        break;
		    case News_Door_Sensor:
		        if(t==DOOR_SENSOR_CLOSE){
		            sprintf(buf, "Event=Lock");
		            strcpy(doorState, "LOCK\n");
		            strcpy(alarmState, "NULL");
		        }
		        else if(t==DOOR_SENSOR_OPEN){
		            sprintf(buf, "Event=Unlock");
		            strcpy(doorState, "UNLOCK\n");
		            strcpy(alarmState, "NULL");
		        }
		        else if(t==DOOR_SENSOR_BREAK){
		            sprintf(buf, "Event=Alarm\tMessage=Break");
		            strcpy(doorState, "UNLOCK\n");
		            strcpy(alarmState, "BREAK");
		        }
		        else
		            sprintf(buf, "Event=Alarm\tMessage=%d", t);
		        break;
		    case News_Alarm_Strip:
		        sprintf(buf, "Event=Alarm\tMessage=Strip");
		        strcpy(doorState, "NULL\n");
		        strcpy(alarmState, "STRIP");
		        break;

		    default:
		        {
		             if(t==32)//胁迫报警
		            {
		                sprintf(buf, "Event=Alarm\tMessage=Force");
		                strcpy(doorState, "UNLOCK\n");
		                strcpy(alarmState, "FORCE");
		            }
		             else
		             {
		                sprintf(buf, "Event=Alarm\tMessage=(%d,%d)", obj, t);
		             }
		        }
		}
		break;
	}
	case EF_UNLOCK:
	    sprintf(buf, "Event=Unlock\tObject=%d", atoi((char *)Data));
	    strcpy(doorState, "UNLOCK\n");
	    strcpy(alarmState, "NULL");
	    break;
	case EF_ATTLOG:
	    sprintf(buf, "Event=Unlock\tObject=%d", atoi((char *)Data));
	    strcpy(doorState, "UNLOCK\n");
	    strcpy(alarmState, "NULL");
	    break;
	case EF_ALARMOFF:
	    sprintf(buf, "Event=AlarmOff");
	    strcpy(doorState, "AlarmOff\n");
	    strcpy(alarmState, "NULL");
	    break;

	default:
	    return ;
	}
	fAccState = fopen("/tmp/ACCState.txt","w+");
	if (fAccState) {
	    fputs(doorState,fAccState);
	    fputs(alarmState,fAccState);
	    fclose(fAccState);
	}
	fAccState = fopen("/tmp/RTData.txt", "w+");
	if(fAccState) {
		 char  strCutTime[20];
		 GetCurTimeByStr((char *)&strCutTime);
		c=fprintf(fAccState, "%s\tTime=%s\n", buf,strCutTime);
		fclose(fAccState);
	}
}

//SetUserTZStr  ACC SetUserTZStr PIN=1234 TZs=1;2;3
int PushSetUserTZStr(char *pstrSrc)
{

        int PIN=0;
        char buf_tmp[10];
        char tmp[10];
        unsigned short  TZs[3];
        char *p;
        char pin_tmp[25];
        TUser user;

        pstrSrc = pstrSrc + 12;
        memset(TZs,0,sizeof(TZs));
        memset(buf_tmp,0,sizeof(buf_tmp));
        memset(tmp,0,sizeof(tmp));
        memset(pin_tmp,0,sizeof(pin_tmp));

        PIN=extractValueInt(pstrSrc, "PIN", ' ', 0);
        extractValueSample(pstrSrc,"PIN", ' ', pin_tmp,sizeof(pin_tmp));
        extractValueSample(pstrSrc, "TZs", ' ', buf_tmp,sizeof(buf_tmp));
        p=strtok(buf_tmp,";");
        if (p) {
            strcpy(tmp,p);
            TZs[0]=atoi(tmp);
        }
        p=strtok(NULL,";");
        if (p) {
            strcpy(tmp,p);
            TZs[1]=atoi(tmp);
        }
        p=strtok(NULL,";");
        if (p) {
            strcpy(tmp,p);
            TZs[2]=atoi(tmp);
        }

	 int i;
	 for (i=0; i<3; i++)  {
		if (TZs[i] != 0 && (FDB_GetTimeZone(TZs[i], NULL) == NULL)) {
			return -3;
	        }
	 }    
        if(FDB_GetUserByCharPIN2(pin_tmp,&user) != NULL) {
		user.TimeZones[0] = 1;
		memcpy(user.TimeZones+1,TZs,sizeof(TZs));
		FDB_ChgUser(&user);
            FDB_AddOPLog(ADMINPIN,OP_ACC_USER,PIN,1,2,0);
            return 0;
        } else {
            return -1;
        }
        
    }
// ACC SetUnlockGroup 1|11;21|21|2;3|3|3
int PushSetUnlockGroup(char *pstrSrc)
{
        char name[200];
        char name_tmp[200];
        int i=0,count=0;
        TCGroup stuTcg;
         int svrID;
         int subSvrID;

        pstrSrc = pstrSrc + 15;
        memset(name,0,sizeof(name));
        memset(&stuTcg,0,sizeof(stuTcg));
        memset(name_tmp,0,sizeof(name_tmp));

        strcpy(name_tmp,pstrSrc);
        int len=strlen(name_tmp);
         name_tmp[len]='\0';
        svrID = 1;
        count = 0;
        subSvrID = 0;
        while (name_tmp[i] != '\0') {
        	name[count++]=name_tmp[i];
          //4|3|2|1|1;
            if(name_tmp[i] == ';') {	
            		count = 0;
            		stuTcg.ID = svrID;
                	stuTcg.GroupID[subSvrID] = atoi(name);
                	 
                	 if ((stuTcg.GroupID[subSvrID] != 0)  && \
                	 	FDB_GetGroup(stuTcg.GroupID[subSvrID], NULL) == NULL) {
				return -3;
			 } else  {
			 	if (stuTcg.GroupID[subSvrID] != 0) {
					++subSvrID;
			 	}
			 	stuTcg.MemberCount = (subSvrID);
				subSvrID=0;
			 }
			 if (FDB_GetCGroup(stuTcg.ID, NULL) != NULL) {
				FDB_ChgCGroup(&stuTcg);
			 } else {
				FDB_AddCGroup(&stuTcg);
			 }
            		svrID++;
            		memset(&stuTcg,0,sizeof(stuTcg));
            }  else if(name_tmp[i] == '|') {
                count = 0;
                stuTcg.GroupID[subSvrID] = atoi(name);
               
                 if (stuTcg.GroupID[subSvrID] == 0) {
                		i++;
				continue;
                }
		   if (FDB_GetGroup(stuTcg.GroupID[subSvrID], NULL) == NULL) {
			return -3;
		   }
                subSvrID++;
            }
            i++;
        }
        FDB_AddOPLog(ADMINPIN, OP_ACC, 0, 1, 0, 0);
       return 0;
}

/*ACC SetGroupTZs GroupNo=1 TZs=1;0;0*/
int PushSetGroupTZs(char *pstrSrc)
{
        char buf_tmp[20];
        char tmp[10];
        char *p;
        TGroup stuTgp;
        int ret;

	pstrSrc = pstrSrc + 11;
	memset(tmp,0,sizeof(tmp));
	memset(buf_tmp,0,sizeof(buf_tmp));

	extractValueSample(pstrSrc, "TZs", '\t', buf_tmp,sizeof(buf_tmp));
	stuTgp.ID = extractValueInt(pstrSrc, "GroupNo", ' ', 0);
	p=strtok(buf_tmp,";");
	if(p) {
	    strcpy(tmp,p);
	    stuTgp.TZID[0]=atoi(tmp);
	}
	p=strtok(NULL,";");
	if(p) {
	    strcpy(tmp,p);
	    stuTgp.TZID[1]=atoi(tmp);
	}
	p=strtok(NULL,";");
	if(p) {
	    strcpy(tmp,p);
	    stuTgp.TZID[2]=atoi(tmp);
	}
	//ACC SetGroupTZs GroupNo=1 TZs=1;0;0\tVerifyType=xx\tHolidayValid=x\n
	if (extractValueSample(pstrSrc, "VerifyType", '\t', tmp,3) > 0 ) {
		stuTgp.VerifyStyle = atoi(tmp);
		if (/*(stuTgp.VerifyStyle < 0) || */(stuTgp.VerifyStyle > 14)) {
			return -3;
		}
	} else {
		stuTgp.VerifyStyle = 0;
	}
	
	if (extractValueSample(pstrSrc, "HolidayValid", '\0', tmp,3) > 0 ) {
		if (atoi(tmp) == 1) {
			stuTgp.VerifyStyle |=  0x80;
		} else {
			stuTgp.VerifyStyle &=  0x7F;
		}
	} else {
		stuTgp.VerifyStyle |=  0x80;
	}
	
	if (FDB_GetGroup(stuTgp.ID,NULL) != NULL) {
		ret = FDB_ChgGroup(&stuTgp);
	} else {
		ret = FDB_AddGroup(&stuTgp);
	}
	if (FDB_OK == ret) {
		FDB_AddOPLog(ADMINPIN,OP_ACC_GRP,stuTgp.ID,1,stuTgp.TZID[0],stuTgp.TZID[1]);
		return 0;
	}
	return -1;
}
/*SetTZInfo TZIndex=1 TZ=07:00-12:00;00:00-23:59;00:00-23:59;
	00:00-23-59;00:00-23:59;00:00-23:59;00:00-23:59*/
int PushSetTZInfo(char *pstrSrc)
{
	int i,m,n;
	char *p;
	int TZIndex;
	char TZ_tmp[512];
	unsigned  char strTmp[10];
	TTimeZone CTZ;


	pstrSrc = pstrSrc+9;
	memset(TZ_tmp,0,sizeof(TZ_tmp));
	TZIndex=extractValueInt(pstrSrc, "TZIndex", ' ', '\0');
	extractValueSample(pstrSrc, "TZ", ' ', TZ_tmp,sizeof(TZ_tmp));
	p=TZ_tmp;
	for(i=0; i<7; i++)
	    for(m=0; m<2; m++)
	        for(n=0; n<2; n++)
	        {
	            memset(strTmp,0,sizeof(strTmp));
	            memcpy(strTmp,p,2);
	            CTZ.ITime[i][m][n]=atoi((char *)strTmp);
	            p=p+3;
	        }
	  	CTZ.ID  = TZIndex;
	  	i = 0;
		if(FDB_GetTimeZone(TZIndex,NULL)!=NULL) {
			if(FDB_ChgTimeZone(&CTZ)==FDB_OK) {
				sync();
				i =  1;
			}
		} else {
			if(FDB_AddTimeZone(&CTZ)==FDB_OK) {
				sync();
				i =  1;
			}
		}
	if ( i == 1) {
		FDB_AddOPLog(ADMINPIN, OP_ACC_TZ, TZIndex, 1, 0, 0);
		return TRUE;
	} else {
		return FALSE;
	}
 }
/*ACC SetHTimeZone  HTNO=1\tStartTime=1-1\tEndTime=12-1\tTZ=1\n*/
 int PushSetHTimeZone (char * pstrSrc)
 {
 	THTimeZone stuHT;
 	char strTmp[10];
 	char *pstr;
 	int ret;
 
	memset(strTmp,'\0',sizeof(strTmp));
	if (extractValueSample(pstrSrc, "HTNO", '\t', strTmp, sizeof(strTmp)) <= 0) {
		return -3;
	}
	stuHT.ID = atoi(strTmp);
	if (extractValueSample(pstrSrc, "StartTime", '\t', strTmp, sizeof(strTmp)) <= 0) {
		return -3;
	}

	stuHT.HDate[0][0] = atoi(strTmp);
	pstr = strstr(strTmp,"-");
	pstr++;

	if (pstr == NULL) {
		return -4;
	}
	stuHT.HDate[0][1] = atoi(pstr);
	if (extractValueSample(pstrSrc, "EndTime", '\t', strTmp, sizeof(strTmp)) <= 0) {
		return -3;
	}
	stuHT.HDate[1][0] = atoi(strTmp);
	pstr = strstr(strTmp,"-");
	pstr++;

	if (pstr == NULL) {
		return -4;
	}
	stuHT.HDate[1][1] = atoi(pstr);
	if (extractValueSample(pstrSrc, "TZ", '\0', strTmp, sizeof(strTmp)) <= 0) {
		return -3;
	}
	stuHT.TZID = atoi(strTmp);
	if (FDB_GetTimeZone(stuHT.TZID,NULL) == NULL) {
		return -4;
	}

	if (FDB_GetHTimeZone(stuHT.ID,NULL) != NULL) {
		 ret = FDB_ChgHTimeZone(&stuHT);
	} else {
		ret = FDB_AddHTimeZone(&stuHT);
	}
	return ret;
 }


int  file_copy(char *dest_file,char *src_file)
{
	int size;
	FILE *srcfp,*destfp;
	char buff[4096];
	int result = 1;
	
	if ((srcfp=fopen(src_file,"r")) == NULL) {
		return -1;
	}
	if ((destfp=fopen(dest_file,"wb")) == NULL) {
		fclose(srcfp);
		return -2;
	}
	while ((size=fread(buff,sizeof(char),sizeof(buff),srcfp)) > 0) {
		if (fwrite(buff,sizeof(char),size,destfp) != size) {
			result = -3;
			break;	
		}
	}
	fsync(fileno(destfp));
	
	fclose(srcfp);
	fclose(destfp);

	return result;
}



int create_data_record(char *file_name,int contype)
{
	int result = 0;

	if (file_name == NULL) {
		return result;
	}
	switch (contype) {
	case FCT_USER:
		result = file_copy(file_name,"/mnt/mtdblock/ssruser.dat");
		if (result > 0) {
			get_fp_recode();
		}
	break;
	default:
	break;
	}
	return result;
}
static int  write_file_with_offset(int filefp, int file_offset, char  *file_content, int size)
{
	int result;
	
	if (filefp < 0) {
		return -1;
	}
	lseek(filefp,file_offset,SEEK_SET);
	result = write(filefp,file_content,size);
	fsync(filefp);
	
	return (result == size);
}
static int  read_file_with_offset(int filefp, int file_offset, char  *file_content, int size)
{
	int result;
	
	if (filefp < 0) {
		return TRUE;
	}
	
	lseek(filefp,file_offset,SEEK_SET);
	result = read(filefp,file_content,size);
	
	fsync(filefp);
	
	return (result == size);
}
void adjust_file_offset(int fd,int file_offset)
{
	int trans_recode = -1;
	time_t cur_time;
	int trans_help_fp;

	trans_help_fp = open(BS_TRANS_HELP_FILE,O_RDWR|O_CREAT|O_NONBLOCK);
	if (trans_help_fp < 0) {
		return ;
	}
	
	if (lseek(fd,0,SEEK_END) <= 0) {
		write_file_with_offset(trans_help_fp,file_offset , (char *)(&trans_recode), sizeof(int));
	} else if (read_file_with_offset(trans_help_fp,file_offset ,(char *)(&trans_recode), sizeof(int)) 
			&& (trans_recode == -1)) {
		trans_recode = 0;
		write_file_with_offset(trans_help_fp,file_offset , (char *)(&trans_recode), sizeof(int));
	}
	cur_time = time(NULL);
	write_file_with_offset(trans_help_fp,file_offset+sizeof(int) , (char *)(&cur_time), sizeof(time_t));
	
	close(trans_help_fp);
}

int read_userpic_data(void *usr_data,void *out_data)
{
	char strFileName[50];
	char strFilePath[256];
	FILE *fStream;
	int len;
	TUser *user = usr_data;
	
	snprintf((char *)strFileName,sizeof(strFileName),"%s.jpg",user->PIN2);
	sprintf((char *)strFilePath,"%s",GetPhotoPath((char *)strFileName));

	fStream = fopen(strFilePath,"rb");
	len = file_get_length(fStream);
	if (len >=  30*1024 || (len < 0)) {
		return 0;
	}
	len = sizeof(TUser);
	memcpy(out_data,usr_data,len);
	return len;
	
}	

int read_face_data(void *usr_data,int index,void *out_data)
{
#ifdef SUPPORT_FACE
	return  pushsdk_get_user_facetmp_by_faceid((TUser* )usr_data,\
								(pushsdk_face_tmp_t*)out_data,index);
#else 
	return 0;
#endif
}
static int * fp_index_info = NULL;
int get_fp_recode(void)
{
	int size = (gOptions.MaxFingerCount+1)*100*sizeof(int);
	int ret;

	if (gOptions.IsOnlyRFMachine || gOptions.ZKFPVersion == ZKFPV09) {
		return 0;
	}else if ( gOptions.ZKFPVersion == ZKFPV10 &&  fhdl == 0) {
		return 0;
	}
	if (fp_index_info == NULL) {
		fp_index_info = (int *)malloc(size);
	}
	if (fp_index_info == NULL) {
		return 0;
	}
	memset(fp_index_info,0,size);
	return pushsdk_deal_data(&ret, sizeof(ret),PUSHSDK_DATA_GET_ALL_FP_INDEX,\
					FCT_FINGERTMP,fp_index_info, size);
}

static user_fp_tmp_t user_fp_tmp[MAX_USER_FINGER_COUNT];
int read_fp_data(void *usr_data,int index,void *out_data)
{
	int ret = -1;
	TUser * usr = (TUser *)usr_data;
	U16 pin = (usr->PIN) ;
	int i = 0;
	fp_index_t  fp_index ;
	pushsdk_fp_tmp_t *fp = (pushsdk_fp_tmp_t *)out_data;
	memset(&fp_index, 0, sizeof(fp_index_t));
	if (index < 0 || index >= MAX_USER_FINGER_COUNT || usr->PIN <= 0) {
		return 0;
	}
	if (gOptions.ZKFPVersion == ZKFPV10) { 
		if (fp_index_info == NULL) {
			get_fp_recode();
		}
		if ( fp_index_info == NULL) {
			return 0;
		}
		
		fp_index.fp_count = fp_index_info[0];
		fp_index.fp_index = fp_index_info + 1;
		
		if (fp_index.fp_count <= 0) {
			return 0;
		}
	}
	
	if (user_fp_tmp[0].PIN !=  usr->PIN) {
		if (gOptions.ZKFPVersion == ZKFPV10) { 
			for (i=0; i < fp_index.fp_count; i++) {
				if ((fp_index.fp_index[i] & 0xffff) == pin) {
					break;
				}
			}
			if (i == fp_index.fp_count) {
				return 0;
			}
		}
		
		for (i=0; i< MAX_USER_FINGER_COUNT; i++) {
			user_fp_tmp[i].PIN = 0;
		} 
		ret = pushsdk_deal_data(&pin, sizeof(pin),PUSHSDK_DATA_GET_USER_ALL_FP,\
					FCT_FINGERTMP,user_fp_tmp, sizeof(user_fp_tmp_t)*MAX_USER_FINGER_COUNT);
		if (ret <= 0) {
			return 0;
		}

	}
	if (user_fp_tmp[index].PIN == usr->PIN) {
		memcpy(&(fp->fp_tmp),user_fp_tmp+index,sizeof(user_fp_tmp_t));
		fp->fp_tmp.Size += 6;
		memcpy(fp->pin2,usr->PIN2,sizeof(usr->PIN2));
		return sizeof(pushsdk_fp_tmp_t);
	} else {
		for (i=0; i< MAX_USER_FINGER_COUNT; i++) {
			user_fp_tmp[i].PIN = 0;
		} 
		return 0;
	}
	
}	

int filltime(unsigned char devtime[],TTime *time)
{

	int c=0;

	printf("filetime function: %d-%d-%d %d:%d:%d\n",time->tm_year,time->tm_mon,time->tm_mday,\
		time->tm_hour,time->tm_min,time->tm_sec);
	c=(time->tm_year+1900)/100;
	devtime[0]=(c/10<<4)|(c%10);
	c=(time->tm_year+1900)%100;
	devtime[1]=(c/10<<4)|(c%10);
	c=time->tm_mon+1;
	devtime[2]=(c/10<<4)|(c%10);
	c=time->tm_mday;
	devtime[3]=(c/10<<4)|(c%10);
	c=time->tm_hour;
	devtime[4]=(c/10<<4)|(c%10);
	c=time->tm_min;
	devtime[5]=(c/10<<4)|(c%10);
	c=time->tm_sec;
	devtime[6]=(c/10<<4)|(c%10);
	return 0;
}


//将考勤流水转为字节数据
int iconvattlog(PAttLog log, PAttData data)
{
	TUser tmpuser;
	TSerialCardNo serialno;
	memset(&tmpuser,0,sizeof(TUser));
	memset(&serialno, 0, sizeof(TSerialCardNo));
	TTime ttDateTime;

	
	if(log==NULL || data==NULL)
	{
		return -1;
	}
	if(log->PIN)	//因为是考勤记录，所以必须有pin号	
	{
		if(FDB_GetUser(log->PIN,&tmpuser)==NULL)
		{
			return -1;
		}
		FDB_GetSerialCardNumber(log->PIN, &serialno);
		memcpy(data->CardSerial,serialno.CardSerialNumber,10);
		memcpy(data->Card,tmpuser.Card,4);
		memcpy(&(data->AttSerialNum),log->reserved,4);
		data->AttSerialNum=htonl(data->AttSerialNum);
		OldDecodeTime(log->time_second, &ttDateTime);
		filltime(data->AttTime,&ttDateTime);
	}
	else
	{
		return -1;
	}
	return 0;
}

int read_attlog_func(unsigned char *buf, int maxrec, int currec, int maxcount,int *attlenght,off_t *curfp)
{
	int ret=0;
	unsigned char *p=buf;
	unsigned char tmpbuf[2048];
	PAttLog pbuf = (PAttLog)tmpbuf;
	TAttLog tmplog;
	unsigned int maxbufcount=2048/sizeof(TAttLog);
	int CurRec=currec;
	int count=0;
	int i=0;
	TAttData data;

	if(buf==NULL)
		return 0;
	
	if(maxbufcount<=maxcount)
	{
		maxcount=maxbufcount;
	}
	printf("[%s]_maxindex=%d,curindex=%d, maxcount=%d, count=%d curfp=%d\n", __FUNCTION__, maxrec, CurRec, maxcount, count, *curfp);
	count = FDB_GetAttByIndex(pbuf,maxrec,CurRec,maxcount,NULL,curfp);
	for(i=0;i<count;i++)	//将每条考勤记录转成字节数据
	{
		memset(&data,0,sizeof(TAttData));
		memset(&tmplog,0,sizeof(TAttLog));
		memcpy(&tmplog,&pbuf[i],sizeof(TAttLog));
		//printf("attlog.pin=%d pin2=%s,timesecond=%u", tmplog.PIN, tmplog.PIN2, tmplog.time_second);
		iconvattlog(&tmplog,&data);
		memcpy(p+i*(sizeof(TAttData)),&data,sizeof(TAttData));
	}
	*attlenght=count*sizeof(TAttData);
	if(0 == count)
	{
		printf("count = 0\n");
		return -1;
	}
	return 0;
}


int read_usr_data(int con_type, char *data_buf,int file_index,int fd) 
{
	int len = -1;
	int ret = 0;
	
	if (data_buf == NULL) {
		return -1;
	}
	
	switch(con_type) {
		case FCT_USER: 
			if (get_file_size(fd) <=  file_index*sizeof(TUser)) {
				break;
			}
			lseek(fd,file_index*sizeof(TUser),SEEK_SET);
			do  {
				len =  read(fd,data_buf,sizeof(TUser));
				if (len != sizeof(TUser))  {
					return -1;
				}
				ret++;
			} while (((TUser *)data_buf)->PIN <= 0);
			break;
		default:
			break;
	}
	
	return ret;
}

int read_data(int con_type, void *usr_data,int index,void *data)
{
	int len = -1;
	TUser *usr = usr_data;
	
	if ((usr_data == NULL) || (data == NULL)) {
		return -1;
	}
	if ((usr->PIN <= 0) || usr->PIN2[0] == '\0') {
		return 0;
	}
	switch(con_type) {
		case FCT_FINGERTMP:
	             len = read_fp_data(usr_data,index,data);
			break;
		case FCT_FACE:
	             len = read_face_data(usr_data,index,data);
			break;
		case FCT_USERPIC:
			len =  read_userpic_data(usr_data,data);
			break;	
		default:
			break;
	}
	return len;
}
int dev_is_busy(void)
{
	int i = 0;
	
	sleepflag ? i:(i++);
	menuflag ? (i+=2):i;
	
	return i;
}
int user_deal(int action,void *data,void *addinfo,unsigned int info_len)
{
	TUser user;
	int ret = 0;
	
	if (action == PUSHSDK_DATA_ADD) {
		TUser* puser = (TUser *)data;
		if (FDB_GetUserByCharPIN2(puser->PIN2, &user) != NULL) {
			puser->PIN = user.PIN;
			ret = FDB_ChgUser(puser);
			if (ret == FDB_ERROR_NODATA) {
				puser->PIN = GetNextPIN(1, TRUE);
				ret = FDB_AddUser(puser);
			}
		} else {
			if (FDB_CntUserByMap() > gOptions.MaxUserCount*100) {
				return PUSH_ERROR_DATAOVER;
			}
			puser->PIN = GetNextPIN(1, TRUE);
			ret = FDB_AddUser(puser);
		}
	} else if (action == PUSHSDK_DATA_DEL) {
		if (FDB_GetUserByCharPIN2(data, &user) != NULL) {
			ret = FDB_DelUser(user.PIN);
		}
	} else if (action == PUSHSDK_DATA_GET_BY_PIN2) {
		memset(&user,0,sizeof(user));
		if (FDB_GetUserByCharPIN2((char *)data, (PUser)&user) != NULL) {
			ret = sizeof(user);
			if ((addinfo != NULL) && (info_len >= sizeof(user))) {
				memcpy(addinfo,&user,sizeof(user));
			}
		} else {
			ret = -13;
		}
	}  else if (action == PUSHSDK_DATA_GET_BY_PIN) {
		U16 pin;
		memset(&user,0,sizeof(user));
		memcpy(&pin,data,sizeof(pin));
		if (FDB_GetUser(pin, (PUser)&user) != NULL) {
			ret = sizeof(user);
			if ((addinfo != NULL) && (info_len >= sizeof(user))) {
				memcpy(addinfo,&user,sizeof(user));
			}
		} else {
			ret = -13;
		}
	}
	return ret;
}
int fp_deal(int action,void *data,void *addinfo,unsigned int info_len)
{
	int ret = -1;
	TUser usr;
	int fingerID;
	int pin = 0;

	if ((FDB_GetUserByCharPIN2(addinfo, &usr) == NULL) \
		&& (PUSHSDK_DATA_ADD == action || PUSHSDK_DATA_DEL == action)) {
		return  ret;
	}
	
	if (action == PUSHSDK_DATA_ADD) {	
		((TZKFPTemplate *)data)->PIN = usr.PIN;
		FDB_DelTmp(((TZKFPTemplate *)data)->PIN, ((TZKFPTemplate *)data)->FingerID);
		ret = FDB_AddTmp((char *)data);
	} else if (action == PUSHSDK_DATA_DEL) {
		memcpy(&fingerID,data,sizeof(fingerID));
		if (fingerID != 15) {
			ret=FDB_DelTmp(usr.PIN,fingerID);
		} else  {
			for (fingerID=0;fingerID<10;fingerID++) {
				ret=FDB_DelTmp(usr.PIN,fingerID);
			}
		}
	} else if (action == PUSHSDK_DATA_GET_BY_PIN) {
		if ((addinfo == NULL) || (info_len < sizeof(user_fp_tmp_t)) || (data == NULL)) {
			return -3;
		}
		unsigned int fp_info;
		
		memcpy(&fp_info,data,sizeof(fp_info));
		pin = fp_info&0x0000ffff;
		fingerID = ((fp_info&0xffff0000) >> 16) & 0x000f;
		if (FDB_GetTmp(pin, fingerID, addinfo) > 0) {
			ret = sizeof(user_fp_tmp_t);
		} else {
			ret = 0;
		}
	} else if (action == PUSHSDK_DATA_GET_USER_ALL_FP) {
		int i ;
		user_fp_tmp_t * user_fp_tmp = (user_fp_tmp_t *)addinfo;
		if ((addinfo == NULL)  || (data == NULL)) {
			return -3;
		}
		ret = 0;
		memcpy(&pin,data,sizeof(pin));
		
		for (i=0; i<MAX_USER_FINGER_COUNT; i++) {
			if (FDB_GetTmp(pin, i, (char *)(user_fp_tmp+ret)) > 0) {
				ret++;
			}
		}
		ret = ret * sizeof(user_fp_tmp_t);
	} else if (action == PUSHSDK_DATA_GET_ALL_FP_INDEX) {
		int *fp = (int *)(addinfo);

		if ((addinfo == NULL)  || (data == NULL)) {
			return -3;
		}
		BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERCOUNT,fp);
      		BIOKEY_GET_PARAM(fhdl,ZKFP_PARAM_CODE_USERIDS,fp+1);
      		ret = info_len; 
	}
	return ret;
}
static int udata_deal(int action,void *data,void *addinfo,unsigned int info_len)
{
	int ret = -1;
	TUser usr;
	TUData * udata = (TUData *)data;
	
	if (FDB_GetUserByCharPIN2(addinfo, &usr) == NULL) {
		return  ret;
	}
	if (FDB_GetSms(udata->SmsID, NULL) == NULL) { 
		return -1;
	}
	
	/*one user,one sms.if the usr have sms,then delete it*/
	if (FDB_GetUData(usr.PIN, NULL) != NULL) {
		 FDB_DelUData(usr.PIN, 0);
	}
	FDB_CreateUData(udata,usr.PIN,udata->SmsID);
	ret = FDB_AddUData(udata);
	
	return ret;
}
static int sms_deal(int action,void *data,void *addinfo,unsigned int info_len)
{
	int ret = -1;
	
	if (action == PUSHSDK_DATA_ADD) {
		ret = FDB_ChgSms((PSms)data);
		if (ret == FDB_ERROR_NODATA) {
			ret=FDB_AddSms((PSms)data);
		}
	} else if (action == PUSHSDK_DATA_DEL) {
		if (((TSms *)data)->Tag == UDATA_TAG_USERSMS) {
			FDB_DelUData(0, ((TSms *)data)->ID);
		}
		ret = FDB_DelSms(((TSms *)data)->ID);
	}
	bsmscount = FDB_GetBoardNum();
	return ret;
}
static int face_deal(int action,void *data,void *addinfo,unsigned int info_len)
{
	int ret = -1;
#ifdef SUPPORT_FACE
	if (action == PUSHSDK_DATA_ADD) {
		 TFaceTmp *face_tmp = (TFaceTmp *)data;
		 TUser user;
		 unsigned char *temp[FACE_NUM];
		 int i;
		 if (pushsdk_CntFaceUser() > gMaxFaceCount) {
			return -8;
		 }
		 if (FDB_GetUserByCharPIN2(addinfo,&user) == NULL) {
			return -2;
		 }
		 FDB_DeleteFaceTmps(user.PIN2);
		 for (i = 0; i < FACE_NUM; i++) {
			temp[i] = face_tmp[i].Face;
		}
		if ((ret= FDB_AddFaceTmps(user.PIN2, temp,FACE_NUM)) < 0) {
			ret = -34 + ret ;
		} else {
			ret = 0;
		}
	} else if (action == PUSHSDK_DATA_DEL) {
		return FDB_DeleteFaceTmps(data);
	}
#endif
	return ret;
}
static int attlog_deal(int action,void *data,void *addinfo,unsigned int info_len)
{
	off_t offset = 0;
	int fd;
	int len;

	if ((addinfo == NULL)|| (info_len < sizeof(TAttLog) )) {
		return 0;
	}
	memcpy(&offset,data,sizeof(offset));
	fd = SelectFDFromConentType(FCT_ATTLOG);
	if (file_get_size(fd) <= offset* sizeof(TAttLog)) {
		return 0;
	}
	lseek(fd,offset* sizeof(TAttLog),SEEK_SET);
	len = read(fd,addinfo,sizeof(TAttLog)*info_len);
	if (len < 0) {
		len = 0;
	}
	return len;
}
static int operlog_deal(int action,void *data,void *addinfo,unsigned int info_len)
{
	off_t offset = 0;
	int fd;
	int len;
	
	if ((addinfo == NULL)|| (info_len < sizeof(TOPLog) )) {
		return 0;
	}
	memcpy(&offset,data,sizeof(offset));
	fd = SelectFDFromConentType(FCT_OPLOG);
	if (file_get_size(fd) <= offset*sizeof(TOPLog)) {
		return 0;
	}
	
	lseek(fd,offset*sizeof(TOPLog),SEEK_SET);
	len = read(fd,addinfo,sizeof(TOPLog)*info_len);
	if (len < 0) {
		len = 0;
	}
	return len;
}

static int attphoto_deal(int action,void *data,void *addinfo,unsigned int info_len)
{
	off_t offset = 0;
	int fd;
	int len ;
	
	if ((addinfo == NULL)|| (info_len < sizeof(TPhotoIndex) )) {
		return 0;
	}
	memcpy(&offset,data,sizeof(offset));
	fd = SelectFDFromConentType(FCT_PHOTOINDEX);
	if (file_get_size(fd) <= offset*sizeof(TPhotoIndex)) {
		return 0;
	}
	lseek(fd,offset*sizeof(TPhotoIndex),SEEK_SET);
	len = read(fd,addinfo,sizeof(TPhotoIndex)*info_len);
	if (len < 0) {
		len = 0;
	}
	return len;
}
int main_update_data(int type,int action,void * data,void *addinfo,unsigned int info_len)
{
	int ret = -12;

	switch (type) {
	case FCT_USER:
		 ret = user_deal(action,data,addinfo,info_len);
		 break;
	case FCT_FINGERTMP:
		ret = fp_deal(action,data,addinfo,info_len);
		break;
	case FCT_SMS:
		ret = sms_deal(action,data,addinfo,info_len);
		break;
	case FCT_FACE:
		ret = face_deal(action,data,addinfo,info_len);
		break;
	case FCT_UDATA:
		ret = udata_deal(action,data,addinfo,info_len);
		break;
	case FCT_ATTLOG:
		ret = attlog_deal(action,data,addinfo,info_len);
		break;
	case FCT_OPLOG:
		ret = operlog_deal(action,data,addinfo,info_len);
		break;
	case FCT_PHOTOINDEX:
		ret = attphoto_deal(action,data,addinfo,info_len);
		break;
	default:
		break;
	}
	return ret;
}
BOOL  att_log_cnt(char *start_time,char *end_time,unsigned int *cnt)
{
	TSearchHandle sh;
	TAttLog	UserAttLog;
	time_t  startStamp;
	time_t  endStamp;
	time_t  attlogStamp;
	TTime ttDateTime;
	off_t cur_filepos = 0;

	if ((start_time==NULL) || (end_time==NULL) || (cnt==NULL)) {
		return FALSE;
	}
	startStamp = StrTimeToStamp(start_time);
	endStamp = StrTimeToStamp(end_time);
	if ((startStamp < 0) ||(endStamp <= 0)) {
		return FALSE;
	}
	
	*cnt = 0;
	sh.ContentType = FCT_ATTLOG;
	sh.buffer=(char*)&UserAttLog;

	pushsdk_get_filepos(FCT_ATTLOG,&cur_filepos);
	SearchFirst(&sh);
	while (!SearchNext(&sh)) {
		if (!FDB_IsOKLog(&UserAttLog)) {
			continue;
		}
		OldDecodeTime(UserAttLog.time_second, &ttDateTime);
		attlogStamp = mktime(&ttDateTime);
		if  (( attlogStamp>= startStamp) && (attlogStamp <= endStamp)) {
			(*cnt)++;
		}
	}
	pushsdk_set_filepos(FCT_ATTLOG,cur_filepos);
	return TRUE;
}
BOOL  oper_log_cnt(char *start_time,char *end_time,unsigned int *cnt)
{
	TSearchHandle sh;
	TOPLog opLog;
	time_t  startStamp;
	time_t  endStamp;
	time_t  attlogStamp;
	TTime ttDateTime;
	off_t cur_filepos = 0;

	if ((start_time==NULL) || (end_time==NULL) || (cnt==NULL)) {
		return FALSE;
	}
	startStamp = StrTimeToStamp(start_time);
	endStamp = StrTimeToStamp(end_time);
	if ((startStamp < 0) ||(endStamp <= 0)) {
		return FALSE;
	}
	
	*cnt = 0;
	sh.ContentType = FCT_OPLOG;
	sh.buffer=(char*)&opLog;

	pushsdk_get_filepos(FCT_OPLOG,&cur_filepos);
	SearchFirst(&sh);
	while (!SearchNext(&sh)) {
		OldDecodeTime(opLog.time_second, &ttDateTime);
		attlogStamp = mktime(&ttDateTime);
		if  (( attlogStamp>= startStamp) && (attlogStamp <= endStamp)) {
			(*cnt)++;
		}
	}
	pushsdk_set_filepos(FCT_OPLOG,cur_filepos);
	return TRUE;
}
 int  FDB_CheckAttlog(char *param,unsigned  int *sumAttLog)
{
	char startTime[20]; //标准时间格式
	char endTime[20]; //标准时间格式
	
	memset(startTime,0,sizeof(startTime));
	memset(endTime,0,sizeof(endTime));
	
	extractValueSample(param, "StartTime", '\t', startTime,sizeof(startTime)-1);
	extractValueSample(param+30, "EndTime", '\t', endTime,sizeof(endTime)-1);
	if  ((strlen(startTime) <= 0) || (strlen(endTime) <= 0)) {
		return -1;
	}
	return att_log_cnt(startTime,endTime,sumAttLog);
}
int oper_log_upload_filter(void *data, char *param)
{
	TTime t;
	char txt[64];
	TOPLog *log = (TOPLog *)data;
	time_range_t* range = (time_range_t *)param;

	if ((data == NULL) || (param == NULL)) {
		return -1;
	}
	OldDecodeTime(log->time_second, &t);
	snprintf(txt, sizeof(txt),"%04d-%02d-%02d %02d:%02d:%02d", \
			t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	if (strcmp(txt, range->start) < 0) {
		return 0;
	}
	if (strcmp(txt, range->end) > 0) {
		return 0;
	}
	return 1;
}
int att_log_upload_filter(void *data, char *param)
{
	TTime t;
	char txt[64];
	TAttLog *log = (TAttLog *)data;
	time_range_t* range = (time_range_t *)param;

	if ((data == NULL) || (param == NULL)) {
		return -1;
	}
	OldDecodeTime(log->time_second, &t);
	snprintf(txt, sizeof(txt),"%04d-%02d-%02d %02d:%02d:%02d", \
			t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	if (strcmp(txt, range->start) < 0) {
		return 0;
	}
	if (strcmp(txt, range->end) > 0) {
		return 0;
	}
	return 1;
}

static void Draw_Rect(HDC hdc,int x1,int y1,int x2,int y2)
{
	LineEx(hdc,x1,y1,x2,y1);
	LineEx(hdc,x2,y1,x2,y2);
	LineEx(hdc,x2,y2,x1,y2);
	LineEx(hdc,x1,y2,x1,y1);
}
#ifdef FACE
#define ICON_PADDING_LEFT 5
#define ICON_PADDING_TOP 50
void Draw_Iclock_Status(HDC hdc, int status)
{
	RECT rect;
	gal_pixel old_TextColor = GetTextColor(hdc);
	gal_pixel old_pen_color = GetPenColor(hdc);
	gal_pixel old_BKColor = GetBkMode(hdc);
	PLOGFONT  old_font = GetCurFont(hdc);
	PTCapStyle old_style = GetPenCapStyle(hdc);
	unsigned int old_pen_width = GetPenWidth(hdc);

	SetBkMode(hdc, BM_TRANSPARENT);
	SetPenCapStyle(hdc, PT_CAP_ROUND);
	SetPenWidth(hdc, 2);

	if (status == 0) {
		SetTextColor(hdc, PIXEL_green);
		SetPenColor(hdc, PIXEL_green);
		SetBrushColor(hdc, PIXEL_green);
	} else {
		SetTextColor(hdc, PIXEL_darkgray);
		SetPenColor(hdc, PIXEL_darkgray);
		SetBrushColor(hdc, PIXEL_darkgray);
	}

	SelectFont(hdc,gfont);

	rect.left = ICON_PADDING_LEFT + 28; 
	rect.top = ICON_PADDING_TOP - 2;; 
	rect.right = rect.left + 18; 
	rect.bottom = ICON_PADDING_TOP + 16;
	/*
	rect.left = ICON_PADDING_LEFT;
	rect.top = ICON_PADDING_TOP - 2;
	rect.right = ICON_PADDING_LEFT + 18;
	rect.bottom = ICON_PADDING_TOP + 16;
	*/

	Draw_Rect(hdc, rect.left, rect.top, rect.right, rect.bottom);
	DrawTextEx(hdc,"I",1, &rect, 0,  DT_CENTER |DT_SINGLELINE | DT_VCENTER );

	SetTextColor (hdc,old_TextColor);
	SelectFont(hdc, old_font);
	SetPenColor(hdc,old_pen_color);
	SetPenWidth(hdc,old_pen_width);
	SetPenCapStyle(hdc,old_style);
	SetBkMode(hdc, old_BKColor);
}
#endif

void show_iclock_status_icon(HWND hWnd, int LCDWidth)
{
	int x, tmpvalue = 0;
	HDC hdc = GetClientDC(hWnd);;
	RECT rect;

	gal_pixel old_TextColor = GetTextColor(hdc);
	gal_pixel old_pen_color = GetPenColor(hdc);
	gal_pixel old_BKColor = GetBkMode(hdc);
	PTCapStyle old_style = GetPenCapStyle(hdc);
	PLOGFONT  old_font = GetCurFont(hdc);
	unsigned int old_pen_width = GetPenWidth(hdc);

	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
	SetPenWidth(hdc, 2);
	tmpvalue = SetPenCapStyle(hdc, PT_CAP_ROUND);
	if (!gOptions.iClockServerStatus && get_all_net_link_status()){
		/*iclock server connected*/
		SetTextColor(hdc, PIXEL_green);
		SetPenColor(hdc, PIXEL_green);
		SetBrushColor(hdc, PIXEL_green);
	} else {
		/*iclock server disconnect*/
		SetTextColor(hdc, PIXEL_darkgray);
		SetPenColor(hdc, PIXEL_darkgray);
		SetBrushColor(hdc, PIXEL_darkgray);
	}
	x = LCDWidth - ICON_WIDTH;
	rect.left = x - 3;
	rect.top = ICON_PADDING_TOP;
	rect.right = x - 17;
	rect.bottom = ICON_PADDING_TOP + 16;

	DrawTextEx(hdc, "I", 1, &rect, 0, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	Draw_Rect(hdc, x, ICON_PADDING_TOP, x - 17, ICON_PADDING_TOP + 16);

	SetTextColor(hdc, old_TextColor);
	SetPenColor(hdc, old_pen_color);
	SetPenWidth(hdc, old_pen_width);
	tmpvalue = SetPenCapStyle(hdc, old_style);
	tmpvalue = SetBkMode(hdc, old_BKColor);
	SelectFont(hdc, old_font);

	ReleaseDC(hdc);
}

int  GetPushSDKVersion(char * version, int buflen)
{
	return PushSDKVersion(version, buflen);
}

/*
* if_name like "ath0", "eth0". Notice: call this function
* need root privilege.
* return value:
*	-1 -- error , details can check errno
*	1 -- interface link up
*	0 -- interface link down.
*/
int GetNetLinkStatus(const char *if_name)
{
	int skfd;
	struct ifreq ifr;

	if (strcmp("lo",if_name) == 0)
	{
		return -1;
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		printf("socket-%s",strerror(errno));
		return -1;
	}

	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) == -1)
	{
		printf("ioctl-%s",strerror(errno));
		return -2;
	}
	close(skfd);
	return ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING));
}


int get_all_net_link_status(void)
{
	int sockfd, len, lastlen;
    int family;
	char *buf,*ptr;
	struct ifreq *ifr;
	struct ifconf ifc;

    family = AF_INET;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		printf("socket-%s",strerror(errno));
		return -1;
	}

	lastlen = 0;
	len = 100 * sizeof(struct ifreq);	/* initial buffer size guess */
	for ( ; ; ) {
		buf = malloc(len);
		if (buf == NULL)
		{
			printf("malloc-%s",strerror(errno));
			return -1;
		}
		ifc.ifc_len = len;
		ifc.ifc_buf = buf;
		if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
			if (errno != EINVAL || lastlen != 0)
				printf("ioctl error....\n");
		} else {
			if (ifc.ifc_len == lastlen)
				break;		/* success, len has not changed */
			lastlen = ifc.ifc_len;
		}
		len += 10 * sizeof(struct ifreq);	/* increment */
		free(buf);
	}
	for (ptr = buf; ptr < buf + ifc.ifc_len; ) {
		ifr = (struct ifreq *) ptr;
#ifdef	HAVE_SOCKADDR_SA_LEN
		len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
#else
		switch (ifr->ifr_addr.sa_family) {
#ifdef	IPV6
		case AF_INET6:	
			len = sizeof(struct sockaddr_in6);
			break;
#endif
		case AF_INET:	
		default:	
			len = sizeof(struct sockaddr);
			break;
		}
#endif
		
		ptr += sizeof(ifr->ifr_name) + len;
		if (ifr->ifr_addr.sa_family != family)
			continue;
		
		if (GetNetLinkStatus(ifr->ifr_name) == 1)
		{
			close(sockfd);
			free(buf);
			return 1;
		}
		
	}
	printf("net link failed %s",ifr->ifr_name);
	free(buf);
	close(sockfd);
	return 0;	
}


