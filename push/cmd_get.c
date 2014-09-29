#include <stdio.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cmd.h"
#include "fw_api.h"
#include "lib.h"
#include "fuliye.h"

#define DATA_CMD_TYPE  0x00
#define WORKING_FLAG 	0x01
extern cmd_line_t cmdLines[];
extern data_list_t *data_list ;	
extern cmd_list_t  *cmd_list ;
extern iclock_options_t pushsdk_options;
extern int data_record_deal(void *data, int index, void *param);
extern int data_record_help(void *data, int index, void *param);
int svr_time_sync(time_t before, char *http_header, iclock_options_t *opt);
int downloadFile(char *furl, char *fileName, int timeoutSec,int SendBufferSize);

//extern unsigned char DevNumber[11];
extern short int FramNumber;
extern unsigned short int gErrorCode;


static trans_data_t trans_data;
#define PUSHSDK_MEM_OFFSET_TYPE		0
#define PUSHSDK_MEM_OFFSET_UNDEFILE	1
#define PUSHSDK_MEM_OFFSET_DATA_SIZE	0x02
#define PUSHSDK_MEM_OFFSET_INFO_SIZE	(PUSHSDK_MEM_OFFSET_DATA_SIZE+sizeof(int))
#define PUSHSDK_MEM_OFFSET_DATA		0x10
#define PUSHSDK_MEM_SIZE	(64*1024*sizeof(char))
#define PUSHSDK_MEM_OFFSET_RET		0
#define PUSHSDK_DATA_CHG 	0
#define PUSHSDK_DATA_ADD  	1
#define PUSHSDK_DATA_DEL	2

#define PUSHSDK_DATA_GET				0x10
#define PUSHSDK_DATA_GET_BY_PIN		0x11
#define PUSHSDK_DATA_GET_BY_PIN2		0x12
#define PUSHSDK_DATA_GET_BY_OFFSET	0x13
#define PUSHSDK_DATA_GET_USER_ALL_FP	0x14
#define PUSHSDK_DATA_GET_ALL_FP_INDEX 0x15
char *pushsdk_mem = NULL;
data_list_t *data_help_list = NULL;


int  tell_and_wait_parent(iclock_options_t * opt,char cmd)
{
	int sleep_ms;

	if ((opt->main_fun->dev_is_busy_fun != NULL) ) {
		sleep_ms = (opt->main_fun->dev_is_busy_fun()*1);
	} else {
		sleep_ms = 1;
	}
	opt->cmd_deal_over = 0;
	pipe_write_to_parent_cmd(cmd);
	
	while (1) {
		msleep(sleep_ms);
		if (opt->cmd_deal_over == 1) {
			break;
		}
	}
	return 0;
}
int call_main_update_data(iclock_options_t *opt)
{
	int ret = -13;
	if (opt->main_fun->main_update_data_fun != NULL) {
		ret = opt->main_fun->main_update_data_fun(trans_data.type, \
					trans_data.action, trans_data.data,trans_data.info,trans_data.info_size);
	}
	trans_data.ret = ret;
	opt->cmd_deal_over = 1;

	return ret;
}

int main_read_attlog(iclock_options_t *opt,PPostAttLog postbuf)
{
	int ret=-13;
	if(opt->main_fun->read_attlog_fun != NULL){
	//	read_attlog_func(unsigned char * buf,int maxrec,int currec,int maxcount,int * attlenght)
	//	opt->main_fun->read_attlog_fun(postbuf->attbuffer,postbuf->maxrec,postbuf->currec,35,&postbuf->reclenght);
		opt->main_fun->read_attlog_fun(postbuf->attbuffer,postbuf->maxrec,postbuf->currec,MAX_POST_ATT_COUNT,&postbuf->reclenght,&postbuf->curfp);
		printf("[%s][%d]lenght=%d,postbuf->maxrec=%d,postbuf->currec=%d, &postbuf->curfp=%d\n",__FUNCTION__,__LINE__, postbuf->reclenght, postbuf->maxrec, postbuf->currec, postbuf->curfp, postbuf->lastrec);
		if(1)
		{
			int i=0;
			printf("**********ATT START***************\n");
			for(i=0;i<postbuf->reclenght;i++)
			{
				printf("0x%02x ",postbuf->attbuffer[i]);
			}
			printf("\n***********ATT END****************\n");
		}	
	}
	return 0;
}

int main_clear_data(iclock_options_t *opt)
{
	signed char type = pushsdk_mem[PUSHSDK_MEM_OFFSET_TYPE] ;
	
	pushsdk_mem[PUSHSDK_MEM_OFFSET_RET] = 0;
	printf("[%s]type = %c\n", __FUNCTION__, type);
	switch (type) {
	case FCT_ATTLOG:
		if (opt->main_fun->attlog_clear_fun != NULL) {
			opt->main_fun->attlog_clear_fun();
		}
		opt->cmd_deal_over = 1;
		pushsdk_data_reset(FCT_ATTLOG);
		break;
	case FCT_ALL:
		if (opt->main_fun->photo_clear_fun != NULL) {
			opt->main_fun->photo_clear_fun();
		}
		if (opt->main_fun->data_clear_fun!= NULL) {
			opt->main_fun->data_clear_fun(FCT_ALL);
		}
		opt->cmd_deal_over = 1;
		pushsdk_data_reset(FCT_ATTLOG);
		pushsdk_data_reset(FCT_PHOTOINDEX);
		pushsdk_data_reset(FCT_OPLOG);
		break;
	case FCT_PHOTOINDEX:
		if (opt->main_fun->photo_clear_fun != NULL) {
			opt->main_fun->photo_clear_fun();
		}
		opt->cmd_deal_over = 1;		
		pushsdk_data_reset(FCT_PHOTOINDEX);
		break;
	case FCT_ADMIN:
		if (opt->main_fun->admin_clear_fun!= NULL) {
			opt->main_fun->admin_clear_fun();
		}
		opt->cmd_deal_over = 1;
		break;
	default:
		opt->cmd_deal_over = 1;
		break;
	}
	return 0;
}

int pushsdk_clear_data(iclock_options_t *opt,int type)
{
	pushsdk_mem[PUSHSDK_MEM_OFFSET_TYPE] = (signed char)type;
	return tell_and_wait_parent(opt,CHILD_CMD_CLEAR_DATA);
}
void context_update(iclock_options_t*opt, int type)
{
	int ret = -1;
	
	if (opt->stat.context == NULL ) {
		return;
	}
	switch (type) {
	case FCT_FACE :
		ret = DOWN_LOAD_FACE ;
		break;
	case FCT_FINGERTMP:
		ret = DOWN_LOAD_FP ;
		break;
	case FCT_USER:
		ret = DOWN_LOAD_USR ;
		break;
	case FCT_USERPIC:
		ret = DOWN_LOAD_USRPIC;
		break;
	default:
		break;
	}
	if (ret > 0) {
		set_bit(opt->stat.context, ret);
	}
}

int pushsdk_deal_data(void  *data,int size,int action,int type,void *info,int info_size)
{
	int ret = -1;

	if (pthread_self() == pushsdk_options.stat.child_thread_id) {
		if ( data == NULL) {
			return -31;
		}
		trans_data.size = size;
		trans_data.action = action;
		trans_data.info_size = info_size;
		trans_data.type = type;
		trans_data.data = data;
		trans_data.info = info;
		
		tell_and_wait_parent(&pushsdk_options,CHILD_CMD_NEW_DATA);
		ret = trans_data.ret;
#if 0
		printf("[PUSH SDK]    TYPE:%d[USER-5,FP-2,FACE-60,SMS-6]  ACTION:%d[CHG-0,ADD-1,DEl-2,GET-3] ret=%d\n",\
							type,action,ret);
#endif
	} else if (pthread_self() == pushsdk_options.stat.parent_thread_id) {
		if (pushsdk_options.main_fun->main_update_data_fun != NULL) {
			ret = pushsdk_options.main_fun->main_update_data_fun(type,action,data,info,info_size);
			context_update(&pushsdk_options,type);
		}
	}
	return ret;
}
int cmd_update(char *param)
{
	data_list_t*node= NULL;

	node = dl_get_node_by_name(data_list, param);
	if (node == NULL) {
		return -1;
	}

	if (node->update_fun != NULL) {
		param = param + strlen(node->table_name) + 1;
		return node->update_fun(param);
	} else {
		return -2;
	}
}


int cmd_del(char *param)
{
	data_list_t*node=data_list;

	while (node != NULL) {
		if (strncmp(node->table_name,param,strlen(node->table_name))==0) {
			break;
		} else if ((strncmp(node->table_name,param,6)==0)&&(strncmp(param,"fingerprint",11)==0)) {
			break;
		}
		node = node->next;
	}
	if (node == NULL)	 {
		return -1;
	}

	param=param+strlen(node->table_name)+1;

	if (node->del_fun != NULL) {
		return node->del_fun(param);
	} else {
		return -2;
	}
}
static void svr_error_data_display(char *http_head,FILE *http_data)
{
	char data[1024];
	
	printf("[PUSH SDK]    POST ERROR, RECV HEADER =%s\n",http_head);
	
	if (http_data != NULL) {
		printf("\n[PUSH SDK]    ERROR REASON:\n");
		memset(data,0,sizeof(data));
		while (fread(data,sizeof(char),sizeof(data)-1,http_data) > 0) {
			printf("%s",data);
			memset(data,0,sizeof(data));
		} 
		printf("\n\n");
	}
}
 int post_file_to_svr(const char *fileName, const char *url, iclock_options_t* opt)
{
	FILE *fStream=NULL;
	char *reUrl;
	char furl[200];
	int len, blen, ret=0;
	char reqStr[MAX_HEADER_LENGTH+MAX_URL_LENGTH+512+4]; 
	
	reqStr[0]=0;
	reUrl = loadFileMem((char *)fileName, &len, 0, NULL, 0);
	if (reUrl == NULL) {
		return 0;
	}
	if (strncasecmp(url,"http://",7) != 0) {
		snprintf(furl,sizeof(furl), "%s%s", opt->root_url, url);
	} else {
		snprintf(furl,sizeof(furl), "%s", url);
	}
	
	ret = len;
	if (opt->is_encrypt == 1) {
		blen = len + len/16 + 1024;
		char *buffer=(char*)malloc(blen);
		if (buffer != NULL) {
			blen = bufferEncrypt(reUrl, len, buffer+8, blen-8, opt->SN, strlen(opt->SN));
			((int*)buffer)[1] = len;
			strncpy((char *)buffer,"LZO1",4);
			fStream = http_proc(furl, NULL, buffer, blen+8, NULL, \
								opt->websvr_time_out,opt->send_buf_size);
			free(buffer);
			buffer = NULL;
		}
	} else {
		fStream = http_proc(furl, reqStr, reUrl, len, NULL, \
							opt->websvr_time_out,opt->send_buf_size);
		if (http_header_analysis(reqStr,  furl) && (strstr(reqStr,"text/plain") !=  NULL)) {
			printf("[PUSH SDK]    CMD RETURN OK\n");
			pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
			remove(fileName);
		} else {
			pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
			svr_error_data_display(reqStr,fStream);
		}
	}
	free(reUrl);
	
	if(fStream != NULL) {
		fclose(fStream);
	} else {
		ret = -1;
	}
	return ret;
}

static int cmd_return_svr_post(iclock_options_t *opt)
{
	char furl[200];

	opt->stat.last_post_time = time(NULL);
	snprintf(furl,sizeof(furl),"%sdevicecmd?SN=%s",  opt->root_url, opt->SN);
	return post_file_to_svr(POST_FILE, furl, opt);
}
static int cmd_return_svr(char *id, int ret, char *cmdName)
{
	FILE *fStream;

	if ((id==NULL) || (cmdName == NULL)) {
		return 0;
	}
	fStream = fopen("/tmp/post.txt", "a+");
	if (fStream != NULL) {
		fprintf(fStream, "ID=%s&Return=%d&CMD=%s\n", id, ret, cmdName);
		fsync(fileno(fStream));
		fclose(fStream);
	}
	return 0;
}
int cmd_return_file(iclock_options_t*opt,char *file_name,char *id, char *cmd)
{
	char furl[128];
	FILE *fStream =  NULL;
	char buffer[256];
	int len = -1;
	FILE *file_ret = NULL;

	if((fStream = fopen(file_name,"rb")) !=  NULL) {
		snprintf(furl,sizeof(furl),"%sdevicecmd?SN=%s", opt->root_url,opt->SN);
		len = file_get_length(fStream);
		fseek(fStream, 0, SEEK_SET);
		len = sprintf(buffer,"ID=%s\nSN=%s\nFILENAME=%s\nCMD=%s\nReturn=%d\nContent=", \
						id, opt->SN, file_name,cmd, len);
		file_ret = http_proc (furl, NULL, buffer, len, fStream, opt->websvr_time_out,opt->send_buf_size);
		fclose(fStream);
		if (file_ret != NULL) {
			fclose(file_ret);
		}
	}
	return len;
}

static int funSetDelayForAReq(iclock_options_t *opt, char *param, char *id,char *cmd )
{
	int ret = -1;

	if ((param != NULL) && (*param != '\0')) {
		opt->get_request_intval = atoi(param);
		ret = opt->get_request_intval;
	}
	cmd_return_svr( id, ret, cmd);

	return ret;
}

static int funPutFile(iclock_options_t *opt, char *param, char *id,char *cmd)
{
	char furl[256];
	char buffer[1024];
	int i = 0;
	
	/* param=file/fw/123.txt /mnt/mtdblock/1, iclockprx.c,funPutFile,956.*/
	if ((param != NULL) && (*param != '\0')) {
		while(('\t'!= *param) && (' ' != *param) && ('\0' != *param)) {
			if (i > (sizeof(furl)-2)) {
				cmd_return_svr( id, -1, cmd);
				return -1;
			}
			furl[i++]  = *param++;
		}
		param++;
		furl[i] = '\0';
		if (strncmp(furl, "http://", 7) != 0) {
			strcpy(buffer, furl);
			snprintf(furl, sizeof(furl),"%s%s", opt->root_url, buffer);
		}
		/*furl=http://192.168.8.37:80/iclock/file/fw/D:\push\123.txt 
		/mnt/mtdblock/1, iclockprx.c,funPutFile,968.*/
		i = downloadFile(furl, param, opt->websvr_time_out,opt->send_buf_size);
	} else {
		i = -1;
	}
	cmd_return_svr( id, i, cmd);
	return i;
}

static int funGetFile(iclock_options_t *opt, char *param,char *id,char * cmd)
{
	if ((param != NULL) && (*param != '\0')) {
		if (cmd_return_file(opt, param, id,cmd) < 0) {
			cmd_return_svr(id, -2, cmd);
			return -1;
		} else {
			return 1;
		}
	} else {
		cmd_return_svr(id, -1, cmd);
		return 0;
	}
}

static int funShellCmd(iclock_options_t*opt, char *param, char *id,char *cmd)
{
	int flag;
	int len;
	char furl[100];
	char paramCopy[100];
	char buffer[1024];
	FILE *fStream;
	FILE *iSvrResp;
	
	if ((param != NULL) && (*param != '\0')) {
		snprintf(paramCopy,sizeof(paramCopy),"%s%s", param, " >/tmp/shell.out 2>&1");
		flag  = system_vfork(paramCopy);
		if(flag == 127 || flag==-1) {
			cmd_return_svr( id, flag, cmd);
			return 0;
		}
		snprintf(furl,sizeof(furl),"%sdevicecmd?SN=%s", opt->root_url, opt->SN);
		len=snprintf(buffer,sizeof(buffer),"ID=%s\nSN=%s\nReturn=%d\nCMD=%s\nFILENAME=shell.out\nContent=", \
						id, opt->SN, flag,cmd);
		fStream = fopen("/tmp/shell.out","rb");
		iSvrResp = http_proc (furl, NULL, buffer, len, fStream, opt->websvr_time_out,opt->send_buf_size);
		if(fStream != NULL) {
			fclose(fStream);
		}
		if(iSvrResp != NULL) {
			pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
			fclose(iSvrResp);
		}
		remove("/tmp/shell.out");
		return 1;
	}
	cmd_return_svr( id, -1, cmd);
	return 0;
}
int cmd_query(iclock_options_t*opt, char *param)
{
	int ret;
	data_list_t*node;

	ret = -3;

	node = dl_get_node_by_name(data_list, param);
	if (node == NULL)  {
		return -1;
	}

	if ((node->filter_fun != NULL) && (node->format_fun != NULL))  {
		char url[1024];
		trans_info_t info;

		if (node->con_type== FCT_USER) {
			node->param = param;
		}  else {
			node->param = param + strlen(node->table_name) + 1;
		}
		opt->con_type= node->con_type;
		trans_info_init(&info,node,opt,url,NULL,UPLOADE_MODE_QUERY);
		ret = FDB_ForAllData(opt->con_type, data_record_deal, TRANS_MAX_RECORD_NO_LIMIT, &info);
		if((ret >= 0) && (info.error == 0) && (info.count > 0)) {
			ret = upload_left_data(&info,NULL);
		}
		freeBuffer(info.cache);
		node->param = NULL;
		opt->con_type = 0;
	}
	return ret;
}

 int funPutData(iclock_options_t*opt, char *param, char *id,char *cmd)
{
	int ret = -1;
	
	if ((param == NULL)) {
		return cmd_return_svr( id, ret, cmd);
	}
	if (strncmp(param,"USER",4) == 0) {
		param = param + 5;
		ret = dl_call_fun(data_list,"userinfo",DL_UPDATE,param);
	} else if (strncmp(param,"FP",2) == 0) {
		param = param + 3;
		ret = dl_call_fun(data_list,"fingertmp",DL_UPDATE,param);
	} else if (strncmp(param,"DEL_USER",8) == 0) {
		ret = dl_call_fun(data_list,"userinfo",DL_DEL,param);
	} else if (strncmp(param,"DEL_FP",6) == 0) {
		param=param + 7;
		ret = dl_call_fun(data_list,"fingertmp",DL_DEL,param);
	} else if (strncmp(param,"UPDATE",6) == 0) {
		param = param + 7;
		ret = cmd_update(param);
	} else if (strncmp(param,"DELETE",6) == 0) {
		param = param + 7;
		ret = cmd_del(param);
	} else if (strncmp(param,"QUERY",5) == 0) {
		param = param + 6;
		ret = cmd_query(opt,param);
	}
	cmd_return_svr(id, ret, cmd);

	if (abs(time(NULL) - opt->stat.last_post_time) >= opt->get_request_intval) {
		cmd_return_svr_post(opt);
	}

	if (ret == 0) {
		opt->stat.data_update = 1;
	}
	return 0;
}

static int funCheckData(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	cmd_return_svr(id, 0, cmd);
	svr_init(opt);
	opt->stat.get_interval = 0;
	opt->stat.post_interval = 0;
	return 0;
}

static int funClearLog(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	cmd_return_svr(id, pushsdk_clear_data(opt,FCT_ATTLOG), cmd);
	return 0;
}

static int funClearData(iclock_options_t *opt, char *param,char *id,char *cmd)
{
	cmd_return_svr(id, pushsdk_clear_data(opt,FCT_ALL), cmd);
	return 0;
}
static int gFlashSize(int *capacity, int *freeCap) 
{
	int size=0;
	struct statfs s;

	if ((capacity == NULL) || (freeCap == NULL)) {
		return 0;
	}
	if ((statfs("/mnt/mtdblock", &s)==0)) {
		if ((s.f_blocks > 0)) {
			size= s.f_bsize/1024;
			*capacity= s.f_blocks * size;
			*freeCap= s.f_bavail *size;
			return 1;
		}
	}
	return 0;
}

static char *gMainTime(char *t) 
{
	struct stat buf;
	struct tm *p;
	struct tm tms;

	if (t == NULL) {
		return NULL;
	}
	lstat("main", &buf);
	memcpy(&tms,localtime(&buf.st_atime),sizeof(tms));
	p = &tms;
	sprintf(t,"%04d-%02d-%02d %02d:%02d:%02d",(1900+p->tm_year),\
			( 1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	return t;
}

static int funInfo(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	PStrList opts, opt1=slCreate("=");
	int i;
	char *text;
	
	if (slLoadFromFile(opt1,"/mnt/mtdblock/options.cfg") <= 0) {
		slFree(opt1);
		cmd_return_svr( id, -1, "INFO");
		return -1;
	}
	cmd_return_svr_post(opt);

	opts = slCreate("=");
	slAdd(opts,"INFO");
	for(i=0;i<opt1->count; i++) {
		char name[1024];
		int len=1024;
		char *value=slNameValueAt(opt1, i, name, &len);
		if(value && *value) {
			slSetValue(opts, name, value);
		}
	}
	slFree(opt1);

	char t[100];
	int TransactionCount=0,UserCount=0,FPCount=0,  freeCap= 0, capacity=0;
	gFlashSize(&capacity, &freeCap);
	gMainTime(t);
	dev_cnt_data(opt,&UserCount,&FPCount, &TransactionCount, NULL);
	slSetValueInt(opts, "TransactionCount", TransactionCount);
	slSetValueInt(opts, "FPCount", FPCount);
	slSetValueInt(opts, "UserCount", UserCount);
	slSetValue(opts, "MainTime", t);
	slSetValueInt(opts, "FlashSize", capacity);
	slSetValueInt(opts, "FreeFlashSize", freeCap);
	slSetValue(opts, "FWVersion", opt->fw_ver);
	text=slGetText(opts, "\n", &i);
	cmd_return_svr( id, 0, text);
	cmd_return_svr_post(opt);
	free(text);
	slFree(opts);
	return 0;
}

static int funReloadOpt(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	pipe_write_to_parent_cmd(CHILD_CMD_RELOAD_OPTIONS);
	cmd_return_svr( id, 0, cmd);
	return 1;
}

static int funReloadData(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	pipe_write_to_parent_cmd(CHILD_CMD_RELOAD_DATA);
	cmd_return_svr( id, 0,cmd);
	return 1;
}
static int funReboot(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	cmd_return_svr( id, 0, cmd);
	cmd_return_svr_post(opt);
	pipe_write_to_parent_cmd(CHILD_CMD_REBOOT);
	return 1;
}
 int cmd_save_options(iclock_options_t *opt,const char *name, const char *value, int SaveTrue)
{
	printf("[%s]options param name:%s\n", __FUNCTION__, name);
	memcpy(pushsdk_mem,name,strlen(name)+1);
	memcpy(pushsdk_mem+strlen(name)+1,value,strlen(value)+1);
	memcpy(pushsdk_mem+strlen(name)+strlen(value)+2,&SaveTrue,sizeof(SaveTrue));
	return tell_and_wait_parent(opt,CHILD_CMD_SET_OPTIONS);
	
}
 void main_save_options(iclock_options_t *opt)
{
	char *name;
 	char *value;
	int SaveTrue;

	name = pushsdk_mem;
	value = pushsdk_mem+strlen(name)+1;
	memcpy(&SaveTrue,pushsdk_mem+strlen(name)+strlen(value)+2,sizeof(SaveTrue));
	SaveStr(name, value, SaveTrue);
	opt->cmd_deal_over = 1;
}
static int funOption(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	int i=0;
	char name[200], *value=NULL;
	int paramLen;
	int ch;

	if (param == NULL) {
		cmd_return_svr( id, -1, cmd);
		return -1;
	}
	paramLen = strlen(param);

	while(i < (sizeof(name)-1)) {
		ch = param[i];
		if ((ch==0) || (ch==' ') || (ch=='=') || (ch=='\t')) {
			break;
		}
		name[i++]=ch;
	}
	name[i]=0;
	
	if ((i >=  paramLen) || (strstr(param,"=") == NULL)) {
		cmd_return_svr( id, -1, cmd);
		return -1;
	}
	
	if(param[i] != '\0') {
		value=param+i+1;
	}
	if (name[0] != '~') {
		cmd_save_options(opt,name, value, 0);
		i=0;
	} else {
		i=-1;
	}
	cmd_return_svr( id, i,cmd);
	if (i == 0) {
		opt->stat.options_update =1;
	}
	return 1;
}

static int funSMS(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	int ret;

	ret = dl_call_fun(data_list,"SMS",DL_UPDATE,param);
	cmd_return_svr( id, ret, cmd);
	return 0;
}
static int funACUnAlarm(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	pipe_write_to_parent_cmd(CHILD_CMD_RESET_ALARM);
	cmd_return_svr( id, 0, cmd);
	return 0;
}

static int funCheckLog(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	opt->stat.post_interval = 0;
	opt->stat.get_interval = 0;
	cmd_return_svr( id, 0, cmd);
	return 1;
}

static int funClearPhoto(iclock_options_t*opt, char *param,char *id,char *cmd)            //CLEAR PHOTO
{
	cmd_return_svr(id, pushsdk_clear_data(opt,FCT_PHOTOINDEX), cmd);
	return 0;
}

/*
*Function:funCheckAttlog
*Description:处理服务器自动校对考勤记录命令，服务器下发某段时间的考勤记录，
*考勤设备上传开始时间，截止时间及记录总数，由服务器实现校对
*Input Prama:opt--通信参数
*			param--服务器下发命令参数(StartTime=XX\tEndTime=XX)
*			id--命令序列号
*Return:
*/
static int funCheckAttlog(iclock_options_t*opt, char *param,char *id,char *cmd)            
{
	unsigned int sumAttlog;
	int ret;
	FILE *fStream;
	char *pStr;

	if (id == NULL) {
		return 0;
	}
	ret = FDB_CheckAttlog(param,&sumAttlog);

	pStr = strstr(param,"\t");
	if (pStr != NULL) {
		*pStr = '&';
	}
	
	fStream=fopen("/tmp/post.txt", "a+");
	if (fStream != NULL) {
		fprintf(fStream, "ID=%s&Return=%d&CMD=%s&%s&AttlogCount=%d\n", id, ret, cmd,param,sumAttlog);
		fsync(fileno(fStream));
		fclose(fStream);
	}
	return 1;
}
static int funClearAdmin(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	if ((cmd == NULL) || (id == NULL)) {
		return cmd_return_svr( id, -1, cmd);;
	}
	cmd_return_svr(id, pushsdk_clear_data(opt,FCT_ADMIN), cmd);
	return 0;
}
/*SetTZInfo TZIndex=1 TZ=07:00-12:00;00:00-23:59;
	00:00-23:59;00:00-23-59;00:00-23:59;00:00-23:59;00:00-23:59*/
void  GetFileByLine(char *strFileName,char *strFileContent,unsigned int u32MaxLen)
{   
    FILE *fp =NULL;
    
    if ((strFileName == NULL) || (strFileContent == NULL)) {
		return ;
    }
    fp = fopen(strFileName,"r");
    if (fp != NULL) {
        fgets(strFileContent,u32MaxLen,fp);
        fgets(strFileContent+20,u32MaxLen,fp);
        fclose(fp);
        remove(strFileName);
    }  else {
        strcpy(strFileContent,"NULL");
    }
}

/*
*Function:funCheckAttlog
*Description:处理服务器设置高级门禁及查询设备当前门禁状态命令
*Input Prama:opt--通信参数
*			param--服务器下发命令参数()
*			id--命令序列号
*Return:设备处理命令结果
*/
static int funACC(iclock_options_t*opt, char *param,char *id,char *cmd)        
{
	int ret = -1;
	if ((param == NULL) || (id == NULL) || (cmd == NULL)) {
		cmd_return_svr(id,-1, cmd);
		return -1;
	}
	if (!strncmp(param, "Status", 6)) {
		FILE *fStream;
		char strAccState[40];
		
		fStream = fopen("/tmp/post.txt", "a+");
		if(fStream != NULL) {
			memset(strAccState,0,sizeof(strAccState));
			GetFileByLine("/tmp/ACCState.txt",strAccState,20);
			fprintf(fStream, "ID=%s&SN=%s&Return=%d&CMD=ACC Status&Content=Acc&Fun=%s&LockStatus=%s&AlarmStatus=%s\n", 
			id, opt->SN, 0,\
			opt->push_lock_fun_on[1]==0?"None":opt->push_lock_fun_on[1]==1?"Simple":"Advanced",\
			strAccState, strAccState+20);
			fsync(fileno(fStream));
			fclose(fStream);
		} 
		return 1;
	}
	
	if(strncmp(param, "SetTZInfo", 9) == 0) {
		ret = PushSetTZInfo(param);
	} else if(strncmp(param, "SetGroupTZs", 11) == 0) {
		ret = PushSetGroupTZs(param);
	} else if(!strncmp(param, "SetUnlockGroup", 14)) {
	   ret = PushSetUnlockGroup(param);
	} else if(!strncmp(param, "SetUserTZStr", 12)) {
		ret = PushSetUserTZStr(param);
	} else if (!strncmp(param, "SetHTimeZone", 12)) {
		ret = PushSetHTimeZone(param+12);
	} 
	cmd_return_svr(id,ret, cmd);
	return 0;
}

int  trans_help_init(trans_help_t *trans_help,int con,void *param)
{
	data_list_t *data_list;
	
	data_list = dl_get_node_by_type(data_help_list, con);
	if (data_list == NULL) {
		return -1;
	}
	trans_help->con_type = con;
	trans_help->filter_fun = data_list->filter_fun;
	trans_help->format_fun = data_list->format_fun;
	trans_help->param = param;
	return 1;
}

int  oper_log_upload(iclock_options_t*opt, char *startTime,char *endTime,int count)
{
	unsigned int count_in_dev = 0;
	trans_help_t trans_help;
	time_range_t time_rang;
	
	if (opt->main_fun->operlog_cnt_with_time_fun == NULL) {
		return -2;
	}
	count_in_dev = pushsdk_parent_cmd_cnt_log(FCT_OPLOG,startTime,endTime);
	if (count_in_dev == 0 || count >= count_in_dev) {
		return count_in_dev;
	}  
	opt->con_type = FCT_OPLOG;

	time_rang.start = startTime;
	time_rang.end = endTime;
	if (trans_help_init(&trans_help,FCT_OPLOG,&time_rang) < 0) {
		return -3;
	}
	data_trans(opt,data_record_help,&trans_help);
	return count_in_dev;
}
int  att_log_upload(iclock_options_t*opt, char *startTime,char *endTime,int count)
{
	unsigned int count_in_dev = 0;
	trans_help_t trans_help;
	time_range_t time_rang;
	
	if (opt->main_fun->attlog_cnt_with_time_fun == NULL) {
		return -2;
	}

	count_in_dev = pushsdk_parent_cmd_cnt_log(FCT_ATTLOG,startTime,endTime);
	if (count_in_dev == 0 || count >= count_in_dev) {
		return count_in_dev;
	}  
	opt->con_type = FCT_ATTLOG;

	time_rang.start = startTime;
	time_rang.end = endTime;
	if (trans_help_init(&trans_help,FCT_ATTLOG,&time_rang) < 0) {
		return -3;
	}
	data_trans(opt,data_record_help,&trans_help);
	
	return count_in_dev;
}
/*ACCOUNT Start=2010-05-01    End=2010-05-01    Count=700 */
static int funCheckAttlogUpload(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	char startTime[41];
	char endTime[41];
	int count_in_device = 0;
	int len = strlen("2010-05-01");
	int i = extractValueSample(param, "Start", '\t', startTime,40);
	int j = extractValueSample(param, "End", '\t', endTime,40);
	int count = extractValueInt(param, "Count", '\t', 0);
	
	if ((i < len) || (j < len)) {	
		cmd_return_svr(id, -1,cmd);
		return 1;
	} 
	len = strlen("2010-05-01 00:00:00");
	if (i < len) {
		strcat(startTime," 00:00:00");
	}
	if (j < len) {
		strcat(endTime," 23:59:59");
	}
	count_in_device = att_log_upload(opt, startTime, endTime, count);
	cmd_return_svr(id, count_in_device,cmd);
	
	return 1;
}

/*SYNC Start=2010-05-01    End=2010-05-01  */
static int funCheckOperlogUpload(iclock_options_t*opt, char *param,char *id,char *cmd)
{
	char startTime[41];
	char endTime[41];
	int count_in_device = 0;
	int len = strlen("2010-05-01");
	int i = extractValueSample(param, "Start", '\t', startTime,40);
	int j = extractValueSample(param, "End", '\t', endTime,40);
	int count = extractValueInt(param, "Count", '\t', 0);
	
	if ((i < len) || (j < len)) {	
		cmd_return_svr(id, -1,cmd);
		return 1;
	} 
	len = strlen("2010-05-01 00:00:00");
	if (i < len) {
		strcat(startTime," 00:00:00");
	}
	if (j < len) {
		strcat(endTime," 23:59:59");
	}
	count_in_device = oper_log_upload(opt, startTime, endTime, count);
	cmd_return_svr(id, count_in_device,cmd);
	
	return 1;
}
cmd_line_t cmdLines[]={
	{"DelayForAReq", funSetDelayForAReq},
	{"GetFile", funGetFile},
	{"PutFile", funPutFile},
	{"Shell", funShellCmd},
	{"DATA",funPutData},
	{"CHECK",funCheckData},
	{"CLEAR LOG",funClearLog},
	{"CLEAR DATA",funClearData},
	{"INFO",funInfo},
	{"RELOAD OPTIONS", funReloadOpt},
	{"RELOAD DATA", funReloadData},
	{"REBOOT", funReboot},
	{"SET OPTION", funOption},
	{"SMS", funSMS},
	{"AC_UNALARM", funACUnAlarm},
	{"LOG",funCheckLog},
	{"CLEAR PHOTO",funClearPhoto},
	{"VERIFY SUM ATTLOG", funCheckAttlog},
	{"ACCOUNT",funCheckAttlogUpload},
	{"ACC",funACC},
	{"CLEAR ADMIN",funClearAdmin},
	{"SYNC",funCheckOperlogUpload},
};

char * cmd_get_id(char *req_str,char *id)
{
	int j = 0;
	
	while (*req_str != '\0') {
		if (':' == *req_str++) {
			while ((':' != *req_str) && ('\0' != *req_str)) {
				id[j++] = *req_str++;
			}
			if (':' == *req_str) {
				req_str++;
				id[j] = '\0';
				return req_str;
			}
			break;
		}
	}
	return NULL;
}
static void skip_white_space(char **p_cur)
{
	if (*p_cur == NULL) {
		return;
	}

	while (**p_cur != '\0' && **p_cur == ' ') {
		(*p_cur)++;
	}
}
int cmd_deal_with_list(iclock_options_t *opt,char *svr_cmd,char *cmd_id)
{
	int i;
	int len;
	cmd_list_t  *cmd_node = cmd_list;
	
	for (i=0; i<sizeof(cmdLines)/sizeof(cmdLines[0]); i++) 
	{
		len = strlen(cmdLines[i].cmd);
		if (strncasecmp(svr_cmd, cmdLines[i].cmd, len) == 0) 
		{
			if (svr_cmd[len] == ' ') 
			{
				cmdLines[i].fun(opt, svr_cmd+len+1, cmd_id,cmdLines[i].cmd);
			} 
			else if (svr_cmd[len] == 0) 
			{
				cmdLines[i].fun(opt, NULL, cmd_id,cmdLines[i].cmd);
			}
			return 0;
		}
	}

	while (cmd_node != NULL) {
		len = strlen(cmd_node->cmd);
		if (strncasecmp(svr_cmd, cmd_node->cmd, len) == 0) {
			if(svr_cmd[len] == ' ') {
				len = cmd_node->fun(svr_cmd+len+1);
			} else {
				len = cmd_node->fun(NULL);
			}
			if ((len == 0) && (strncasecmp("ENROLL_FP",svr_cmd,9)==0)) {
				char ow[20];
				ow[0] = CHILD_CMD_ENROLL_FP;
				memcpy(ow+1,svr_cmd+strlen(cmd_node->cmd)+1,3+sizeof(int));
				pipe_write_to_parent(ow, 4+sizeof(int));
				while(1) {
					if (pipe_read_from_parent(ow, 1) == 1) {
						if (ow[0] == PARENT_CMD_RESUME) {
							break;
						} else if (ow[0] == PARENT_CMD_NEW_DATA) {
							opt->stat.post_interval = 0;
						}
					}
				}
				pipe_read_from_parent(&len, 4);
			}
			cmd_return_svr(cmd_id, len,cmd_node->cmd);
			return 0;
		}
		cmd_node = cmd_node->next;
	}
	return 1;
}
static int single_cmd_proc(iclock_options_t * opt,char *svr_cmd)
{
	char cmd_id[20];
	
	svr_cmd = cmd_get_id(svr_cmd,cmd_id);
	if (svr_cmd == NULL) {
		printf("[PUSH SDK]    PUSH PROTOCOL ERROR:NOT GET CMD ID\n");
		return 0;	
	}
	skip_white_space(&svr_cmd);
	
	if ('\0' == *svr_cmd) {
		printf("[PUSH SDK]    PUSH PROTOCOL ERROR:NOT GET CMD CONTENT\n");
		return 0;
	}
	
	if (opt->is_only_upload_photo != 0) {
		printf("[PUSH SDK]    Options.cfg PRAMA IsUploadPhotoOnly NOT 0,NOT SUPPORT DEAL SVR CMD\n\n");
		cmd_return_svr(cmd_id, -1, "DEV CFG ERROR");
		return 0;
	}
	
	if (cmd_deal_with_list(opt,svr_cmd,cmd_id) != 0) {
		printf("[PUSH SDK]    UNKNOWN CMD : %s,%s\n", cmd_id, svr_cmd);
		cmd_return_svr(cmd_id, -22, "UNKNOWN CMD");
	}
	return 1;
}

BOOL  is_unknow_dev(char *cmd)
{
	char *p = NULL;
	
	if (cmd == NULL) {
		return FALSE;
	}
	
	if (strncmp(cmd,"UNKNOWN DEVICE",14)==0) {
		return TRUE;				
	} else if (strchr(cmd,'U') && strchr(cmd,'N') && strchr(cmd,'K')) {
		p=strstr(cmd,"UNK");
		if (p && (p-cmd<20) && strncmp(p,"UNKNOWN DEVICE",14)==0) {
			return TRUE;	
		}
	}
	return FALSE;
}
int svr_data_check(FILE *svr_data,FILE* get_file,time_t websvr_time_out)
{
	char cmd[1024*2] = {0};
	int cnt = 0;
	int sum =0;

	if ((svr_data == NULL) || (get_file == NULL)) {
		return -1;
	}
	while (1) {
		cnt = http_data_read(svr_data, cmd, sizeof(cmd), websvr_time_out);
		if( cnt <= 0) {
			break;
		} else {
			sum += cnt;
			if (is_unknow_dev(cmd)) {
				return -1;
			} else {
				fwrite(cmd, sizeof(char), cnt, get_file);
			}
		}
	}
	fflush(get_file);
	return sum;
}

int get_data_from_svr(char *furl, char *fileName, time_sync_f time_sync,iclock_options_t *opt)
{
	FILE *fStream;
	FILE *gParam = NULL;
	char reqStr[1024*4] = {0};
	time_t time_cur_dev = time(NULL);
	int sum = -1;
	char fname[100] = {0};
	
	fStream = http_proc(furl, reqStr, NULL, 0, NULL, opt->websvr_time_out,opt->send_buf_size);
	
	if (fStream && http_header_analysis(reqStr, furl) ) {
		if (time_sync !=NULL) {
			if (time_sync(time_cur_dev,reqStr, opt) == 0) {
				//return -1;
				printf("[PUSH SDK]    time_sync failed!___error\n");
			}
		}
		snprintf(fname,sizeof(fname), "%s.tmp", fileName);
		gParam = fopen(fname,"w+");
		if(gParam != NULL) {
			sum = svr_data_check(fStream,gParam,opt->websvr_time_out);
		}
	} 
	if (sum >= 0) {
		rename(fname,fileName);	
	} else {
		clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
	}
	if (fStream != NULL) {
		fclose(fStream);
	}
	if (gParam != NULL) {
		fclose(gParam);
	}
	return sum;
}

static int is_data_cmd(char *reqStr)
{
	while (*reqStr != '\0') {
		if ( ':' == *reqStr++) {
			while ((':' != *reqStr) && (*reqStr != '\0')) {
				reqStr++;
			}
			if (*reqStr != '\0') {
				reqStr++;
			}
			break;
		}
	}
	while ((*reqStr != '\0')) {
		if (' '==*reqStr) {
			reqStr++; 
		} else {
			break;
		}
	}
	if (*reqStr == '\0') {
		return 0;
	}
	if ((strncasecmp(reqStr, "DATA ", 5) == 0) || \
	    (strncasecmp(reqStr,"SET OPTION",10)==0)  ||\
	    (strncasecmp(reqStr,"ACC",3)==0)||\
	    (strncasecmp(reqStr,"CLEAR ",6) == 0)) {
		return 1;
	}
	return 0;
}

int get_cmd_type(int ch)
{
	int type;
	
	if (ch == 'R')  { /*a single request*/
		type = 2;
	} else if (ch == 'r') { /* request list*/
		type = 1;
	} else if (ch == 'C') { /*ommand*/
		type = 3 ;
	} else if (ch == 'U') {
		type = -1;
	} else {		/*a single request*/
		type = 0;
	}

	return type;
}
void  svr_delay_set(int cmd_flag,int last_work_flag,iclock_options_t* opt,int cmd_ret)
{
	if (cmd_ret > 0) {
		opt->stat.get_interval = 1;
	} else if (cmd_ret < 0) {
		opt->stat.get_interval = opt->error_intval;
	} else {
		opt->stat.get_interval += opt->get_request_intval/5;
		if (opt->stat.get_interval > opt->get_request_intval) {
			opt->stat.get_interval = opt->get_request_intval;
		}
	}
	if (opt->stat.options_update) {
		pipe_write_to_parent_cmd(CHILD_CMD_RELOAD_OPTIONS);
		opt->stat.options_update = 0;
	}
	if (opt->stat.data_update) {
		pipe_write_to_parent_cmd(CHILD_CMD_RELOAD_DATA);
		opt->stat.data_update = 0;
		opt->stat.info_update = 1;
	}
}

int cmd_deal(char *svr_cmd,int *cmd_flag,iclock_options_t *opt)
{
	int cmd_type;
	
	*cmd_flag =  0x00;
	cmd_type = get_cmd_type(svr_cmd[0]);
	if ((cmd_type == 1) || (cmd_type == 2)) {
		printf("[PUSH SDK]    DO NOT SUPPORT R OR r CMD\n");
		return -21;
	} else if (cmd_type == 3) {
		if (is_data_cmd(svr_cmd)) {
			set_bit(cmd_flag,DATA_CMD_TYPE);
		}
		single_cmd_proc(opt,svr_cmd);
		return 1;
	} else {
		printf("[PUSH SDK]    DO NOT SUPPORT CMD\n");
		return -1;
	}
}
int svr_cmd_proc(char *svr_data,int data_len,iclock_options_t *opt)
{
	int index=0;
	int ch;
	char *cmd;
	int memLen;
	int i;
	static  int  last_work_flag = 0;	/*服务器下发注册数据到设备时，处理完所有数据后，再退出"工作中"界面*/
	int cmd_flag = 0x00;
	int cmd_ret = 0;
	
	if (svr_data == NULL) {
		return opt->error_intval;
	}
	memLen = 4*1024*sizeof(char);
	cmd = (char *)malloc(memLen);
	if (cmd == NULL) {
		printf("[PUSH SDK]    MALLOC FAILED \n\n");
		_exit(2);
	}
	
	for(i=0; i< data_len; i++) {
		ch = svr_data[i];
		if (ch=='\n' || ch=='\r')   {
			cmd[index] = 0;
			if (index > 0) {
				index = 0;	
				cmd_ret =  cmd_deal(cmd,&cmd_flag,opt);
			}
		} else {
			if (index >= (memLen-2)) {
				memLen += 30*1024;
				cmd = (char *)realloc(cmd, memLen); 
				if (cmd == NULL) {
					printf("[PUSH SDK]    MALLOC FAILED \n\n");
					_exit(2);
				}
			}
			cmd[index++] = ch;
		}
	}
	free(cmd);
	free(svr_data);
	svr_delay_set(cmd_flag,last_work_flag,opt,cmd_ret);
	last_work_flag = get_bit(cmd_flag, DATA_CMD_TYPE);
	cmd_return_svr_post(opt);
	
	return opt->stat.get_interval;
}
void update_stamp(iclock_options_t * opt)
{
	data_list_t *node = data_list;
	
	if (strlen(opt->svr_ver)<=0) {
		while (node != NULL) {
			if (node->con_type == FCT_ATTLOG) {
				node->first_read=1;
				node->last_stamp = opt->attlog_stamp;
			} else if (node->con_type == FCT_OPLOG) {
				node->first_read=1;
				node->last_stamp = opt->operlog_stamp;
			} else if (node->con_type == FCT_PHOTOINDEX) {
				node->first_read=1;
				node->last_stamp = opt->attphoto_stamp;
			}
			node=node->next;
		}
	}
}
int iclock_opions_update(char *buffer,iclock_options_t *opt,int *valid_reponse)
{
	int ret = 1;
	
	if ((strncmp(buffer,"UN", 2)==0) || (strncmp(buffer,"RETRY", 5)==0)) {
		ret = -1;
	} else if (checkIntKey(buffer, "Stamp", (unsigned int *)(&opt->attlog_stamp))) {
		*valid_reponse = 1;
	} else if (checkStringKey(buffer,"ICLOCKSVRURL", opt->root_url, sizeof(opt->root_url))) {
		ret = -1;
	} else if (checkIntKey(buffer,"TimeZone",(unsigned int *)&opt->TZ)) {
		if (abs(opt->TZ) < 25) {
			opt->TZ = opt->TZ*60;
		}
	} else if (checkIntKey(buffer, "Delay", (unsigned int *)&opt->get_request_intval)) {
		;
	} else if (checkIntKey(buffer, "OpStamp", (unsigned int *)&opt->operlog_stamp)) {
		;
	} else if (checkIntKey(buffer, "PhotoStamp", (unsigned int *)&opt->attphoto_stamp)) {
		;
	} else if (checkIntKey(buffer, "ErrorDelay", (unsigned int *)&opt->error_intval))	{
		;
	} else if (checkIntKey(buffer,"TransInterval",(unsigned int *)&opt->trans_intval_s)) {
		;
	} else if (checkStringKey(buffer,"TransFlag",opt->svr_trans_flag,sizeof(opt->svr_trans_flag)-1)) {
		;
	} else if (checkStringKey(buffer,"TransTimes",opt->post_timing,sizeof(opt->post_timing)-1)) {
		;
	} else if (checkIntKey(buffer,"Realtime",(unsigned int *)&opt->is_real_time)) {
		;
	} else if (checkIntKey(buffer,"Timeout",(unsigned int *)&opt->websvr_time_out)) {
		;
	} else if (checkIntKey(buffer,"Encrypt",(unsigned int *)&opt->is_encrypt)) {
		;
	} else if (checkStringKey(buffer,"ServerVer",opt->svr_ver,sizeof(opt->svr_ver)-1)) {
		*valid_reponse = 1;
	} else {
		dl_node_add_stamp(data_list,buffer);
	}
	
	return ret;
}
int svr_options_analysis(FILE *svr_opt,iclock_options_t *opt)
{
	int ch;
	char buffer[2048];
	int index = 0;
	int error_code = -1;
	int valid_reponse = 0;

	while (!feof(svr_opt))  {
		ch = fgetc(svr_opt);
		if ((ch == '\r')||(ch == '\n'))  {
			buffer[index] = 0;
			index = 0;
			printf("%s\n", buffer);
			if (iclock_opions_update(buffer,opt,&valid_reponse)  < 0) {
				error_code = -1;
				break;
			} else {
				error_code = 1;
			}
		} else if(ch == EOF) {
			break;
		} else {
			buffer[index++] = (char)ch;
			if (index >= sizeof(buffer))  {
				index = 0;
			}
		}
	}
	if (svr_opt != NULL) {
		fclose(svr_opt);
	}
	if (valid_reponse == 1) {
		update_stamp(opt);
	}
	return error_code;
}
int tzd(char *s)
{
	int d=atoi(s);
	int r=d%100;
	d=d/100*60+r;
	return d;
}
int svr_time_sync(time_t before, char *http_header, iclock_options_t *opt)
{
	time_t cNow2;
	time_t httpNow;
	char buf[64];
	struct tm  tms;

	if (strstr(http_header,"text/plain") == NULL) {
		return 0;
	}
	if(strstr(http_header,"Date:") != NULL) {
		/*----------Server time-------------------------*/
		/*Tue, 09 Oct 2007 05:05:38 GMT*/
		strncpy(buf,(strstr(http_header,"Date:")+6), 31);
		buf[31] = 0;
		strptime(buf,"%a,%d %b %Y %H:%M:%S",&tms);
		
		httpNow = mktime(&tms) - (tzd(buf+26) - opt->TZ)*60; /*8*60*/
		if(opt->is_enter_day_light) {
			httpNow +=  (60*60);
		}
		cNow2 = time(NULL);
		
		if (abs(cNow2-before) < 5) {
			if (abs(httpNow - ((unsigned long)cNow2 + (unsigned long)before) / 2 ) > 10) {
				memcpy(&tms, localtime(&httpNow),sizeof(struct tm));
				SetTime(&tms);
				printf("[PUSH SDK]    SYNC TIME WITH SERVER\n");
			}
		}
		return 1;
	}
	return 0;
}

void display_svr_support_data_type(char *dev_flag)
{
	int i;
	static char *push_svr_data_string_type[] = {
	"AttLog",
	"OpLog",
	"AttPhoto",
	"EnrollFP",
	"EnrollUser",
	"FPImag",
	"ChgUser",
	"ChgFP",
	"FACE",
	"UserPic",};
	if ((strstr(dev_flag,"TransData") != NULL)) {
		printf("[PUSH SDK]    DEV SUPPORT DATA:%s\n",dev_flag+strlen("TransData"));
	} else {
		printf("[PUSH SDK]    DEV SUPPORT DATA:");
		for (i = 0; i<strlen(dev_flag); i++) {
			if (i < sizeof(push_svr_data_string_type)/sizeof(char *)) {
				if (dev_flag[i] == '1') {
					printf("%s ",push_svr_data_string_type[i]);
				}
			} else {
				printf("UNKNOW DATA TYPE");
			}
		}
		printf("\n");
	}
}

int  svr_return_proc(FILE* iSvrResp, char *data_return, int timeout)
{
	int i=0;
	if (NULL == iSvrResp) {
		return FALSE;
	}
	else
	{
		char strchar[10*1024] = {0};
		int len=0;
		len = http_data_read(iSvrResp, strchar, sizeof(strchar), timeout);
		if(len <=0){
			return 0;
		}
		else
		{
/*
			printf("\n----recv data\n");
			for(i=0;i<len;i++)
			{
				printf("0x%02x ",strchar[i]);
			}
			printf("\n\n");
*/			
			memcpy(data_return,strchar,len);
		}
		return len;
	}
}
/*
			
接收到报文:0100003300010001----1122334455667788990011--000000000000--0002--01--3030303030303030--20140113105953--00000000----00000000   
报文的业务类型：0x0001   
报文的帧序号：0001   
报文的机具号：1122334455667788990011   
服务端回复报文：01000028000100011122334455667788990011303030303030303030303030303030300100000000
*/

int svr_init_ymip(iclock_options_t *opt)
{	
	int return_len;
	int error_code = -1;
	FILE *svr_return;
	TYMIP_HEAD recv_head;
	TYMIP_HEAD post_head;
	TRegister  post_reg_data;
	TTransinfo transinfo;
	unsigned int transinfolen;
	unsigned char data_return[1024];
	extern unsigned char commkey[16];
	BOOL checkFN=FALSE;

	unsigned char registerlen[3]={0x00,0x00,0x33};			
	unsigned char PSAM_num[6]={0x00,0x00,0x00,0x00,0x00,0x00};							
	unsigned char DevVersion[8]={0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};			
	unsigned char DevTime[7];			
	unsigned char CustomID[4]={0x00,0x00,0x00,0x00};	
	unsigned char key[16] = {0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};
	memcpy(commkey, key, 16);
	memset(&recv_head,0,sizeof(TYMIP_HEAD));
	memset(&post_head,0,sizeof(TYMIP_HEAD));
	memset(&post_reg_data,0,sizeof(TRegister));
	memset(&transinfo,0,sizeof(TTransinfo));
	transinfolen=sizeof(TYMIP_HEAD)+sizeof(TRegister)+4;
//head
	incFN();
	ymip_head_format(&post_head,LICENSEVERSION,registerlen,FramNumber,0x0001);
	
	time2byte(DevTime);
	
//data
	ymip_data_reg_format(&post_reg_data,DevNumber,PSAM_num,0x0001,0x01,DevVersion,DevTime,CustomID);

//check 
	//给head和data赋值
	ymip_transinfo_format(&transinfo,&post_head,&post_reg_data,sizeof(TRegister),0x00000000);
	svr_return=ymip_proc(opt->svrip,opt->svrport, (char *)&recv_head, (char *)&transinfo, NULL, NULL, opt->error_intval, transinfolen);
	if (NULL == svr_return) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
		clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
		return error_code;
	}
	return_len = svr_return_proc(svr_return, data_return, opt->websvr_time_out);
	if(0 != ChectMAC(&recv_head, data_return))
	{
		printf("[PUSH SDK]	[%s]_MAC check failed...regist again\n", __FUNCTION__);
		gErrorCode = 0x0009;
		return -9;
	}
	if((return_len+8)  != GetPacketLength(&recv_head))
	{
		gErrorCode = 0x000a;
		fclose(svr_return);
		return -10;
	}
	if(svr_return)
	{
		fclose(svr_return);
	}
	
	checkFN=IsRightFN(post_head.FrameNum,recv_head.FrameNum);//帧号检验
	//if(checkFN==FALSE){
	//	printf("[%s] frame number error...\n", __FUNCTION__);
	//	error_code=-2;
	//}
	error_code = ProcessService(&recv_head, data_return, opt);
	if ((error_code < 0)/*||(checkFN==FALSE)*/) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
	} else {
		pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
	}

	return error_code;
}


int svr_open_ymip(iclock_options_t *opt)
{
	FILE *svr_return = NULL;
	int return_len;
	int error_code = -1;
	TYMIP_HEAD recv_head;
	TYMIP_HEAD post_head;
	TOpenDev  post_open_data;  //开通设备结构体
	TTransinfo transinfo;
	unsigned int transinfolen;
	unsigned char data_ret[1024];
	BOOL checkFN=FALSE;

	memset(&recv_head,0,sizeof(TYMIP_HEAD));
	memset(&post_head,0,sizeof(TYMIP_HEAD));
	memset(&post_open_data,0,sizeof(TOpenDev));
	memset(&transinfo,0,sizeof(TTransinfo));
	transinfolen=sizeof(TYMIP_HEAD)+sizeof(TOpenDev)+4;
	unsigned char CustomID[4]={0x00,0x00,0x00,0x00};
	unsigned char opendevlen[3]={0x00,0x00,0x1b};
	
//head
	incFN();
	ymip_head_format(&post_head,LICENSEVERSION,opendevlen,FramNumber,0x0002);
	
//data
	memcpy(&post_open_data.DevNum,DevNumber,11);
	memcpy(&post_open_data.CustomID,CustomID,4);
	//ymip_data_reg_format(&post_reg_data,DevNumber,PSAM_num,0x0002,0x01,DevVersion,DevTime,CustomID);
	
//check 
	
	//给head和data赋值
	ymip_transinfo_format(&transinfo,&post_head,&post_open_data,sizeof(TOpenDev),0x00000000);
	svr_return=ymip_proc(opt->svrip,opt->svrport,(char *)&recv_head, (char *)&transinfo, NULL, NULL, opt->error_intval, transinfolen);
	if (NULL == svr_return) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
		clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
		return error_code;
	}
	printf("[%s][%d]\n",__FUNCTION__,__LINE__);
	return_len = svr_return_proc(svr_return, data_ret, opt->websvr_time_out);
	if(0 != ChectMAC(&recv_head, data_ret))
	{
		printf("[PUSH SDK]	[%s]_MAC check failed...regist again\n", __FUNCTION__);
		gErrorCode = 0x0009;
		return -9;
	}
	if((return_len+8)  != GetPacketLength(&recv_head))
	{
		gErrorCode = 0x000a;
		fclose(svr_return);
		return -1;
	}
	if(svr_return)
	{
	    fclose(svr_return);
	}
	checkFN=IsRightFN(post_head.FrameNum,recv_head.FrameNum);//帧号检验
	
	error_code = ProcessService(&recv_head, data_ret, opt);

	if ((error_code < 0)/*||(checkFN==FALSE)*/) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
	} else {
		pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
	}
	printf("[%s][%d]\n",__FUNCTION__,__LINE__);
	return error_code;
	
}

int svr_init(iclock_options_t *opt)
{
	FILE *svr_return;
	char url[1024];
	char header[2*4096];
	time_t  dev_time = time(NULL);
	
	int  len;
	int error_code = -1;
	BOOL flagFdClosed = FALSE;
	
	len = snprintf(url, sizeof(url),"%scdata?SN=%s&options=all&pushver=%s&language=%d", \
					opt->root_url,opt->SN,PUSHVER,opt->language);

	if (opt->push_comm_key[0] != '\0') {
		snprintf(url+len, sizeof(url)-len,"&pushcommkey=%s", opt->push_comm_key);
	}
	
	header[0] = 0;
	svr_return = http_proc(url, header, NULL, 0, NULL,  opt->websvr_time_out,opt->send_buf_size);
	if (NULL == svr_return) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
		return error_code;
	}
	printf("[PUSH SDK]    READ OPTIONS FROM SVR\n %s\n\n",header);
	if (http_header_analysis(header, url)) {
		svr_time_sync(dev_time,header,opt);
		error_code = svr_options_analysis(svr_return,opt); 

		flagFdClosed = TRUE;
	} 
	if (error_code <= 0) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);

		if(svr_return && !flagFdClosed)
		{
		    fclose(svr_return);
		}
		
	} else {
		display_svr_support_data_type(opt->svr_trans_flag);
		pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
	}
	return error_code;
}

static int doProxy(iclock_options_t *opt, char *url)
{
	int data_len;
	char *buffer;
	time_t delay = 0;
	data_len = get_data_from_svr(url, GET_FILE, svr_time_sync,opt);
	if (data_len > 0) {
		buffer = loadFileMem(GET_FILE, &data_len, opt->is_encrypt, opt->SN, strlen(opt->SN));
		pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
		if (buffer != NULL) {
			delay = svr_cmd_proc(buffer,data_len,opt);
		}
	} else {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
		if (opt->stat.post_interval < opt->error_intval) {
			opt->stat.post_interval = opt->error_intval;
		}
		delay = opt->error_intval;
	}
	return delay;
}

int cmd_get_proc(iclock_options_t *opt)
{
	char url[400];
	int len = 0;
	int user_cnt = 0;
	int fp_cnt = 0;
	int attlog_cnt = 0;
	int face_cnt = 0;

	if (file_get_length_by_name(POST_FILE) > 0) {
		cmd_return_svr_post(opt);
		opt->stat.get_interval +=  opt->get_request_intval / 5;
		msleep(500);
	}
	
	if (opt->stat.info_update) {
		dev_cnt_data(opt,&user_cnt,&fp_cnt,&attlog_cnt ,&face_cnt);
		len = snprintf(url,sizeof(url),"%sgetrequest?SN=%s&INFO=%s,%d,%d,%d,%s,%d", \
					opt->root_url, opt->SN, opt->fw_ver, user_cnt, fp_cnt, \
					attlog_cnt, opt->ip_address,opt->zk_fp_ver);
		snprintf(url+len,sizeof(url)-len,",%d,%d,%d,%s",opt->ZKFaceVer,opt->FaceTmpCnt,\
				face_cnt,opt->dev_support_data);
		opt->stat.info_update = 0;
	} else {
		snprintf(url,sizeof(url),"%sgetrequest?SN=%s", opt->root_url, opt->SN);
	}
	printf("[PUSH SDK]    HTTP GET REQUEST\n");
	opt->stat.get_interval = doProxy(opt, url);
	
	return opt->stat.get_interval;
}

 int mainDoProxy(iclock_options_t *opt, char *url)
{
	int i,ret;
	char *buffer=loadFileMem(DATA_FILE, &ret, opt->is_encrypt, opt->SN, strlen(opt->SN));
	if (buffer != NULL) {
		int c=0;
		int index=0;
		int ch;
		char reqStr[30*1024];
		reqStr[0]=0;
		for (i=0; i<ret; i++) {
			ch = buffer[i];
			if (ch=='\n' || ch=='\r') {
				reqStr[index]=0;
				if (index > 0) {
					c = get_cmd_type(buffer[0]);
					if (c == 3) {	
						single_cmd_proc(opt,reqStr);
					}
					index=0;
				}
			} else {
				reqStr[index++]=ch;
			}
		}
		free(buffer);
		remove(DATA_FILE);
		if(opt->zk_fp_ver != 10) {
			FDB_InitDBs(FALSE);
			FPInit(NULL);
		}
	}

	return 0;
}
int readBuffer(FILE *fStream, char *buffer, int maxSize, int timeoutSec)
{
	int cc;
	if(feof(fStream)) return 0;
	cc=fAvialible(fStream, timeoutSec*1000);
	if(cc<0) {
		return -1;
	}
	else if(cc==0)
	{
		while(!feof(fStream))
		{
			int ch=fgetc(fStream);
			if(ch==EOF)	break;
			buffer[cc]=ch;
			cc++;
			if (cc>=(maxSize-1)) break;
		}
	}
	else
	{
		if(cc>maxSize) cc=maxSize;
		if(fread(buffer, cc, 1, fStream)<=0) return -2;
	}
	return cc;
}

static int downloadFile_action(char *furl, char *reqStr, FILE *tempFile, int timeoutSec, int option,int SendBufferSize)
{
	FILE *fStream;

	if (tempFile == NULL) {
		return -5;
	}
	if (option == 0)
	{//寮�濮嬩笅杞?
		int i = 0, size = 0, count = 0;
		char str[1024*2] = {0};
		for (; i < 5; i++)
		{//5娆¤秴鏃讹紝缁撴潫鏈涓嬭浇
			if (i > 0)
			{
				fflush(tempFile);
				fseek(tempFile, 0, SEEK_END);
				long tempFileLength = ftell(tempFile);  //宸蹭笅杞介噺
				sprintf(reqStr, "RANGE: bytes=%ld-\r\n", tempFileLength);
				printf("[PUSH SDK]    Start Download: continue download %d\n", i);
			}
			fStream = http_proc (furl, reqStr, NULL, 0, NULL, timeoutSec,SendBufferSize);
			if (fStream && (strncmp(reqStr,"HTTP/1.", 7)==0) && (strncmp(reqStr+8, " 20", 3)==0))
			{
				while (1)
				{
					size = readBuffer(fStream, str, sizeof(str), timeoutSec);
					if (size == 0)
					{//涓嬭浇缁撴潫
						fclose(fStream);
						return count;
					}
					else if (size < 0)
					{//瓒呮椂
						break;
					}
					else
					{
						fwrite(str, size, 1, tempFile);
						count += size;
					}
				}//end of while(1)
			}
			if(fStream) 	
				fclose(fStream);
			break;	//--for test
		}//end of for()
		return -2;	//瓒呮椂閫�鍑?
	}
	else if (option == 128 || option == -128)
	{//姣斿
		fStream = http_proc (furl, reqStr, NULL, 0, NULL, timeoutSec,SendBufferSize);
		int size = fAvialible(fStream, timeoutSec*1000);
		if(size <= 0)
		{//瓒呮椂
			printf("Action\tCompare\tOut Of Time %d\n", size);
			if(fStream)     fclose(fStream);
			return -2;
		}
		else
		{
			char temp128[129] = {0}, down128[129] = {0};
			fread(down128, 128, 1, fStream);
			if (option == 128)
			{
				fseek(tempFile, 0, SEEK_SET);
				fread(temp128, 128, 1, tempFile);
			}
			else if (option == -128)
			{
				fseek(tempFile, -128, SEEK_END);
				fread(temp128, 128, 1, tempFile);
			}

			if(fStream)     fclose(fStream);
			return strcmp(temp128, down128) == 0;
		}
	}
	return 0;
}

char* ExtractFileName(const char* filepath)
{
	char tempfilepath[1024]={0},*p;
	static char filename[256];

	sprintf(tempfilepath,"%s",filepath);
	p=strtok(tempfilepath,"/");
	while (p)
	{
		memset(filename,0,sizeof(filename));
		snprintf(filename,strlen(p)<sizeof(filename)-1?strlen(p)+1:sizeof(filename)-1,"%s",p);
		p=strtok(NULL,"/");
	}

	return filename;
}
#define REMOTE_UPGRADE_TMP_PATH  "/mnt/mtdblock/"
int downloadFile(char *furl, char *fileName, int timeoutSec,int SendBufferSize)
{
	long tempFileLength=0;
	int status = 0;
	FILE *tempFile;	
	char tempFileName[1024] = {0};
	char reqStr[1024*4] = {0};		
	char cmd[1024 *2] = {0};
	int i=0;

	/*	PutFile file/fw/X938/main.tgz /mnt/*/
	if (strstr(furl,".tgz") != NULL) {
		sprintf(tempFileName, "%s%s.tmp",REMOTE_UPGRADE_TMP_PATH,ExtractFileName(furl));
	} else {
		sprintf(tempFileName,"%s.tmp",fileName);
	}

	tempFile = fopen(tempFileName, "r");
	if (tempFile == NULL) {
		tempFile = fopen(tempFileName, "w+");
		status = downloadFile_action(furl, reqStr, tempFile, timeoutSec, 0,SendBufferSize);		
	} else {
		fclose(tempFile);
		tempFile = fopen(tempFileName, "a+");
		fseek(tempFile, 0, SEEK_END);
		tempFileLength = ftell(tempFile);
		if (tempFileLength < 256) {
			printf("remove a small old temp file: %s,%ld\n", tempFileName, tempFileLength);
			fclose(tempFile);
		 	tempFile = fopen(tempFileName, "w+");
			status = downloadFile_action(furl, reqStr, tempFile, timeoutSec, 0,SendBufferSize);   
		} else {
			int resume = 1 ;
			sprintf(reqStr, "RANGE: bytes=0-127\r\n");
			if (downloadFile_action(furl, reqStr, tempFile, timeoutSec, 128,SendBufferSize) == 0) {
				resume=0;
			} else {
				memset(reqStr, 0, (1024*4)*sizeof(char));
				sprintf(reqStr, "RANGE: bytes=%ld-%ld\r\n", tempFileLength-1-127, tempFileLength-1);
				if (downloadFile_action(furl, reqStr, tempFile, timeoutSec, -128,SendBufferSize) == 0) {
					resume=0;
				}
			}
			if (resume != 0) {
				printf("resume at: %ld\n", tempFileLength);
				sprintf(reqStr, "RANGE: bytes=%ld-\r\n", tempFileLength);
				fseek(tempFile, 0, SEEK_END);
			} else {
				printf("error of old file: %s\n", tempFileName);
				if (tempFile) {
					fclose(tempFile);
				}
				tempFile=fopen(tempFileName, "w");
			}
			status = downloadFile_action(furl, reqStr, tempFile, timeoutSec, 0,SendBufferSize); 
		}
	}
	if (tempFile) {
		fclose(tempFile);
	}
	if (status < 0) {
		return status;
	}
	printf("Finished: %s, %ld+%d=%ld Bytes\n", fileName, tempFileLength, status, tempFileLength+status);
	if (strstr(furl,".tgz")==NULL) {
		sprintf(cmd, "mv \"%s\" \"%s\"", tempFileName, fileName);
		system_vfork(cmd);
		return tempFileLength+status;
	} else {
		sprintf(cmd, "mv \"%s\" \"%s%s\"", tempFileName, \
					REMOTE_UPGRADE_TMP_PATH,ExtractFileName(furl));
		system_vfork(cmd);
		
	        memset(cmd,0,sizeof(cmd));
	        sprintf(cmd, "tar vxzf \"%s%s\" -C %s",REMOTE_UPGRADE_TMP_PATH,ExtractFileName(furl),REMOTE_UPGRADE_TMP_PATH);
	        i=system_vfork(cmd);
	        if (i==0) {
	        	sprintf(cmd, "%s%s",REMOTE_UPGRADE_TMP_PATH,ExtractFileName(furl));
	        	remove(cmd);
	             return tempFileLength+status;
	        } else {
	                printf("Update firmware ERROR of file tar vxzf\n");
	                return -3;
	        } 
	  }
}
/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,start*/
#define HTTP_URL_LEN 128
#define HTTP_HEADER_LEN 512
#define HTTP_MAXBUF_LEN 22*1024
#define PHOTO_PREFIX_LEN 256
#define  AUTH_TYPE_PWD				0	//用户校验方式(密码)
#define AUTH_TYPE_CARD 			1	//用户校验方式(刷卡)
#define  AUTH_TYPE_FP				2	//用户校验方式(指纹)
#define  AUTH_TYPE_FACE				3	//用户校验方式(人脸)
const char *pstrAuthType[]={"PWD","CARD","FP","FACE"};

/*异地考勤,add by yangxiaolong,2011-06-14,start*/
/*
*Function:auth_from_http_server
*Description:向服务器发送异地考勤请求，服务器返回用户认证结果
*Input Prama:PIN2--1:1(用户输入工号) 1:N PIN2=0
*			pcAuthInfo--请求信息(指纹/人脸/卡)
*			AuthInfoLen--请求信息长度
*			u32AuthType--验证类型(0--card,1--FP,2--FACE)
*Out Prama:userinfo--用户信息
*Return: 0--认证失败
*		1--认证成功
*/
extern iclock_options_t pushsdk_options;;
int AuthFromHttpServer(char *PIN2,char *pcAuthInfo, int AuthInfoLen, void *userinfo,int u32AuthType)
{
	FILE *resp;
	int len;
	char url[HTTP_URL_LEN];
	char http_info_cache[HTTP_MAXBUF_LEN];
	char http_info_header[HTTP_HEADER_LEN];
	char http_photo_buf[HTTP_MAXBUF_LEN];
	
	if ((u32AuthType<0) || (u32AuthType>3) || (AuthInfoLen <= 0)) {
		return 0;
	}

	if (pushsdk_get_pid()<= 0) {
		return 0;
	}

	
	memset(url, 0, sizeof(url));
	memset(http_info_header, 0, sizeof(http_info_header));
	memset(http_info_cache, 0x00, sizeof(http_info_cache));
	/*
	*POST /iclock/cdata?SN=%s&AuthType=%s 
	*PIN=%d\tSize=%d\tTMP=%s\n
	*/
	sprintf(url,"%scdata?SN=%s&AuthType=%s", pushsdk_options.root_url, pushsdk_options.SN,pstrAuthType[u32AuthType]);
	if (PIN2[0] == 0) {
		PIN2[0] = '0';
		PIN2[1] = '\0';
	}
	
	if (u32AuthType == AUTH_TYPE_FP) {
		AuthInfoLen = base64_encode((unsigned char *)pcAuthInfo, AuthInfoLen, (unsigned char *)(http_photo_buf));
	} else {
		memcpy(http_photo_buf,pcAuthInfo,AuthInfoLen);
	}
	
	len = sprintf(http_info_cache,"PIN=%s\tSize=%d\tTMP=", PIN2,AuthInfoLen);
	memset(PIN2,0,strlen(PIN2)+1);
	memcpy(http_info_cache+len,http_photo_buf,AuthInfoLen);
	resp = http_proc(url, http_info_header, http_info_cache, len+AuthInfoLen, \
		NULL, 3,pushsdk_options.send_buf_size);
	return ServerAuthUpdate(resp,http_info_header, userinfo);
}

#define DOWN_USER_FILE "/tmp/user.txt"
/*
*Function:DownLoadUser
*Description:异地考勤请求
*Input Prama:pin--用户工号
*Return: <=0--请求失败
*		bit0 == 1-- 下载user成功
*		bit1 == 1-- 下载fp成功
*		bit2 == 1-- 下载人脸成功
*		bit3 == 1-- 下载用户照片成功
*/

int DownLoadUser(char *pin)
{
	char url[400];
	int result = 1;
	int i = 0;
	int datac=0;
	int index = 0;
	char line[10*1024];
	int ret ;
	char *buffer = NULL;
	int update_flag = 0;

	if (pin == NULL) {
		return -1;
	}

	/*GET /iclock/cdata?SN=%s&PIN=%d */
	sprintf(url, "%scdata?SN=%s&PIN=%s", pushsdk_options.root_url, pushsdk_options.SN, pin);
	unlink(DOWN_USER_FILE);
	
	ret = get_data_from_svr(url, DOWN_USER_FILE, NULL, &pushsdk_options);
	if (ret <= 0) {
		return -1;
	}
	buffer = loadFileMem(DOWN_USER_FILE, &ret, pushsdk_options.is_encrypt, \
					pushsdk_options.SN, strlen(pushsdk_options.SN));
	if (buffer == NULL) {
		return -2;
	}
	pushsdk_options.stat.context = (void *)&update_flag;
	while (i < ret) {
		int ch=buffer[i++];
		if (ch == '\n') {
			line[index]=0;
			if (memcmp("DATA ",line, 5)==0) {
				funPutData(&pushsdk_options, line+5,NULL, NULL);
				datac++;
			} else if (memcmp("NONE",line, 4)==0) {
				result = 0;
			}
			index=0;
		} else {
			line[index++]=ch;
			if (index >= sizeof(line) ) {
				break;
			}
		}
	}
	if ( index > 0) {
		line[index] = 0;
		if (memcmp("DATA ",line, 5)==0) {
			funPutData(&pushsdk_options, line+5,NULL, NULL);
			datac++;
		} else if (memcmp("NONE",line, 4)==0) {
			result = 0;
		}
	}
	free(buffer);
	if (datac) {
		FPInit(NULL);
	}
	pushsdk_options.stat.context = NULL;
	return update_flag;
}

int pushsdk_parent_cmd_cnt_log(int contype,char *start_time,char *end_time)
{
	trans_data.data = pushsdk_mem;
	
	if (start_time != NULL && end_time != NULL) {
		memcpy(trans_data.data,start_time,strlen(start_time)+1);
		trans_data.info = pushsdk_mem+strlen(start_time)+1;
		memcpy(trans_data.info,end_time,strlen(end_time)+1);
	}
	trans_data.type = contype;
	
	tell_and_wait_parent(&pushsdk_options, CHILD_CMD_CNT);
	return trans_data.ret;
}

int pushsdk_cnt_log_by_time(iclock_options_t * opt)
{
	unsigned int ret = 0;
	
	if (trans_data.type == FCT_OPLOG) {
		 opt->main_fun->operlog_cnt_with_time_fun(trans_data.data,trans_data.info,&ret);
	} else if (trans_data.type == FCT_ATTLOG) {
		opt->main_fun->attlog_cnt_with_time_fun(trans_data.data,trans_data.info,&ret);
	} else if (trans_data.type == FCT_FINGERTMP) {
		ret = opt->main_fun->fp_cnt_fun();
	}
	trans_data.ret = ret;
	opt->cmd_deal_over = 1;
	return ret;
}

