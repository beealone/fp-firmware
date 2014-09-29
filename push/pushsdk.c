#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "pushsdk.h"
#include "http.h"
#include "lib.h"
#include "fw_api.h"
#include "cmd.h"
#include "pipe.h"
#include "strlist.h"
#include "signal.h"
#include "lib.h"
#include "thread.h"
#include "fuliye.h"

static pthread_t *pushsdk_thread_id = NULL;
iclock_options_t pushsdk_options;
static pushsdk_weburl_t pushsdk_svr;
pushsdk_main_fun_t pushsdk_main_fun;
data_list_t *data_list = NULL;		
cmd_list_t  *cmd_list = NULL;
extern data_list_t *data_help_list;
extern short int FramNumber;
extern unsigned short int gErrorCode;
extern unsigned int gWarningCode;
extern U8 gWarningLevel;
unsigned char gCardSerial[10];
extern U32 recindex;
unsigned int ListVer;
int gAttflag;
TPostAttLog  postattlog;
extern unsigned char commkey[16];
int gMFCardDevError;
int gMemoryError;
int gLCDError;
time_t error_time;
time_t  dev_time;

#define PUSHSDK_IS_CMD()		get_bit(pushsdk_options.stat.running_flag , RUN_STAT_CMD)
#define PUSHSDK_IS_PAUSE()		get_bit(pushsdk_options.stat.running_flag , RUN_STAT_PAUSE)
#define PUSHSDK_IS_MUTEX() 		get_bit(pushsdk_options.stat.running_flag , RUN_STAT_MUTEX)
#define PUSHSDK_IS_WORKING() 	get_bit(pushsdk_options.stat.running_flag , RUN_STAT_WORKING)
 int mainDoProxy(iclock_options_t *opt, char *url);
 int pushsdk_thread_create(void);
 void child_mainloop_handle(iclock_options_t *opt);
 int pushsdk_parent_cmd_cnt_log(int contype,char *start_time,char *end_time);

 static char * child_cmd_str[] = {
	"CHILD_CMD_ICLOCK_OPTIONS" ,
	"CHILD_CMD_RELOAD_OPTIONS",
	"CHILD_CMD_RELOAD_DATA",
	"CHILD_CMD_REBOOT",
	"CHILD_CMD_ENROLL_FP",
	"CHILD_CMD_UPDATE_ALARM",
	"CHILD_CMD_NEW_DATA",
	"CHILD_CMD_CLEAR_DATA",
	"CHILD_CMD_EXIT_WORKING",
	"CHILD_CMD_ONLINE",
	"CHILD_CMD_OFFLINE",
	"CHILD_CMD_RESET_ALARM",
	"CHILD_CMD_SET_OPTIONS",
	"CHILD_CMD_CNT",
	"CHILD_CMD_READ_ATTLOG",
};
 static char * parent_cmd_str[] = {
	"PARENT_CMD_PAUSE" ,
	"PARENT_CMD_RESUME",
	"PARENT_CMD_RESET" ,
	"PARENT_CMD_FROM_SVR" ,
	"PARENT_CMD_NEW_DATA" ,
	"PARENT_CMD_STOP_DATA" ,
	"PARENT_CMD_RESET_DATA",
	"PARENT_CMD_DSTF",
	"PARENT_CMD_SET_OPTIONS",
	"PARENT_CMD_UPDATE_DATA",
	"PARENT_CMD_OVER",
	"PARENT_CMD_RE_INIT",
	"PARENT_CMD_UPLOAD_ATTLOG",
	"CHILD_CMD_POST_ATT",
};
int DectoHex(int dec, unsigned char *hex, int length) 
{ 
    int i;
    for(i=length-1; i>=0; i--) 
    { 
        hex[i] = (dec%256)&0xFF; 
        dec /= 256; 
    }
    return 0; 
}
void CheckErrorStatus(iclock_options_t *opt)
{
	int freeflag;
	tell_and_wait_parent(opt, CHILD_CMD_USER_CNT);
	if(((opt->max_user_count - opt->cur_user_count) < (opt->max_user_count /10)) && (opt->max_user_count - opt->cur_user_count) >0)
	{
		if (abs(time(NULL) - error_time)  >= 86400) {   //一天检测一次用户是否满
			gWarningCode = 0x20000001;
			gWarningLevel = 0x02;
			error_time = time(NULL);
		}
	}
	if((opt->max_user_count - opt->cur_user_count) <= 0)
	{
		//gErrorCode = 0x0007;
		gWarningCode = 0x30000001;
		gWarningLevel = 0x03;
	}
}
int CommKey2Byte(char *strsrc, unsigned char bytedest[])
{
	int i=0;
	char *p=strsrc;
	char tmpstr[2];
	unsigned int num=0;
	unsigned char c=0;
	
	if(strsrc==NULL || bytedest==NULL || (strlen(strsrc)!=32))
	{
		return -1;
	}
	for(i=0;i<16;i++)
	{
		strncpy(tmpstr,p,2);
		c = str2Hex(tmpstr[0], tmpstr[1]);
		bytedest[i]=c;
		p+=2;	
	}
	return 0;
}
 
void pushsdk_cmd_printf(int type,int cmd)
{
	signed char i = cmd-0x31;
	if (type == 0) {
		if ((i >= 0) && (i < sizeof(parent_cmd_str)/sizeof(char *))) {
			printf("[PUSH SDK]    PARENT REQUEST:%s\n",parent_cmd_str[i]);
		} else {
			printf("[PUSH SDK]    PARENT REQUEST:%s----%02x\n","UNKNOW CHILD CMD",cmd);
		}
	} else {
		if ((i >= 0) && (i < sizeof(child_cmd_str)/sizeof(char *))) {
			printf("[PUSH SDK]    CHILD REQUEST:%s\n",child_cmd_str[i]);
		} else {
			printf("[PUSH SDK]    CHILD REQUEST: %s----%02x\n","UNKNOW PARENT CMD",cmd);
		}
	}
}
void pushsdk_main_fun_init(iclock_options_t *opt)
{
	opt->main_fun = &pushsdk_main_fun;
}
void pushsdk_register_cnt_fun(fdb_cnt_log_f face_cnt_fun,fdb_cnt_log_f attlog_cnt_fun,
									fdb_cnt_log_f user_cnt_fun,fdb_cnt_log_f fp_cnt_fun,
									fdb_log_cnt_with_time_f attlog_cnt_with_time_fun,
									fdb_log_cnt_with_time_f operlog_cnt_with_time_fun)
{
	pushsdk_main_fun.user_cnt_fun = user_cnt_fun;
	pushsdk_main_fun.face_cnt_fun = face_cnt_fun;
	pushsdk_main_fun.attlog_cnt_fun = attlog_cnt_fun;
	pushsdk_main_fun.fp_cnt_fun = fp_cnt_fun;
	pushsdk_main_fun.attlog_cnt_with_time_fun = attlog_cnt_with_time_fun;
	pushsdk_main_fun.operlog_cnt_with_time_fun = operlog_cnt_with_time_fun;
}

void pushsdk_register_clear_fun(fdb_clear_log_f admin_clear_fun,fdb_clear_log_f attlog_clear_fun,
									  fdb_clear_log_f photo_clear_fun,fdb_clear_data_f data_clear_fun)
{
	pushsdk_main_fun.admin_clear_fun = admin_clear_fun;
	pushsdk_main_fun.attlog_clear_fun = attlog_clear_fun;
	pushsdk_main_fun.photo_clear_fun = photo_clear_fun;
	pushsdk_main_fun.data_clear_fun = data_clear_fun;
}
void pushsdk_register_read_data_fun(fdb_upload_usr_data_f upload_usr_read_fun,
				fdb_upload_read_data_f upload_read_data_fun,
				create_data_record_f  create_data_fun,main_update_data_f main_update_data,
				dev_is_busy_f dev_is_busy_fun,update_file_offset_f update_file_offset_fun,fdb_read_attlog_f read_attlog_fun)
{
	pushsdk_main_fun.upload_usr_read_fun = upload_usr_read_fun;
	pushsdk_main_fun.upload_read_data_fun = upload_read_data_fun;
	pushsdk_main_fun.create_data_record_fun = create_data_fun;
	pushsdk_main_fun.main_update_data_fun= main_update_data;
	pushsdk_main_fun.dev_is_busy_fun = dev_is_busy_fun;
	pushsdk_main_fun.update_file_offset_fun = update_file_offset_fun;
	pushsdk_main_fun.read_attlog_fun = read_attlog_fun;
}
int pushsdk_register_data_fun(const char *table_name, int con_type, int parent_con_type,
						char	auto_push_flag,pushsdk_update_f update_fun,pushsdk_del_f del_fun,
						pushsdk_filter_f filter_fun,  pushsdk_format_f	format_fun) 
{
	return dl_init(&data_list,table_name,  con_type,parent_con_type,auto_push_flag, \
			update_fun,del_fun,filter_fun,format_fun);
}
int pushsdk_register_help_fun( const char *table_name,int con_type,
						pushsdk_filter_f filter_fun, pushsdk_format_f format_fun) 
{
	return dl_init(&data_help_list,table_name, con_type,0,0, NULL,NULL,filter_fun,format_fun);
}
void dev_cnt_data(iclock_options_t*opt,int *user_cnt,int *fp_cnt,int *attlog_cnt ,int *face_cnt)
{
	if (user_cnt != NULL) {
		*user_cnt = (opt->main_fun->user_cnt_fun == NULL) ? 0 : opt->main_fun->user_cnt_fun();
	}
	if (fp_cnt != NULL) {
		*fp_cnt = (opt->main_fun->fp_cnt_fun == NULL) ? 0 : pushsdk_parent_cmd_cnt_log(FCT_FINGERTMP,NULL,NULL);
	}
	if (attlog_cnt != NULL) {
		*attlog_cnt = (opt->main_fun->attlog_cnt_fun == NULL) ? 0 : opt->main_fun->attlog_cnt_fun();
	}
	if (face_cnt != NULL) {
		*face_cnt = (opt->main_fun->face_cnt_fun == NULL) ? 0 : opt->main_fun->face_cnt_fun();
	}
}
int pushsdk_register_fun(char* cmd, pushsdk_cmd_f fun)
{	
	return cl_init(&cmd_list,cmd,fun);
}
void pushsdk_clean_resource(void)
{
	return;
}

void pushsdk_reload_svrip_init(iclock_options_t *opt)
{
	opt->svrport = LoadInteger("WebServerPort",80);
	LoadStr("WebServerIP", opt->svrip);
	printf("[%s]reload WebServerIP:%s:%d\n", __FUNCTION__, opt->svrip, opt->svrport);
	pipe_write_to_child_cmd(PARENT_CMD_RE_INIT);

}
void pushsdk_reload_cfg(void)
{
	char url[256];
	int port;
	char iclock_url[128];

	//port = LoadInteger("WebServerPort",80);
	//LoadStr("WebServerIP",iclock_url);
	//if (strncasecmp(iclock_url, "http://", 7) != 0) {
	//	snprintf(url,sizeof(url), "http://%s:%d/iclock/", iclock_url,port);
	//} else {
	//	snprintf(url,sizeof(url), "%s:%d/iclock/",iclock_url,port);
	//}
	//snprintf(pushsdk_svr.weburl,sizeof(pushsdk_svr.weburl),"%s",url);
	
	pushsdk_svr.porxy_port = LoadInteger("WebServerPort",6000);
	//memset(url,0,sizeof(url));
	LoadStr("WebServerIP", pushsdk_svr.porxy_ip);
	//pushsdk_svr.porxy_fun = LoadInteger("EnableProxyServer",0);

}

void pushsdk_sync_cfg(void)
{
	
	snprintf(pushsdk_options.svrip,sizeof(pushsdk_svr.porxy_ip),"%s",pushsdk_svr.porxy_ip);
	pushsdk_options.svrport = pushsdk_svr.porxy_port;
	printf("[PUSH SDk]   [%s]  SVR URL:%s:%d\n", __FUNCTION__, pushsdk_options.svrip, pushsdk_options.svrport);
	//if (pushsdk_svr.porxy_fun) {
	//	http_porxy.set(pushsdk_svr.porxy_ip,pushsdk_svr.porxy_port);
	//} else {
	//	http_porxy.clear();
	//}
}
int pushsdk_init(unsigned char* web_url,unsigned char* web_svr_ip,unsigned int web_svr_port,
		 unsigned char* proxy_ip,unsigned int proxy_port,int send_buf_size,char *fw_ver)
{
	if (pushsdk_thread_id != NULL) {
		return PUSH_ERROR_ISRUNNING;
	}
	
	memset(&pushsdk_options,0,sizeof(iclock_options_t));
	
	//add by lqw 2014.1.15
	snprintf(pushsdk_options.svrip,16,"%d.%d.%d.%d",	\
	web_svr_ip[0],web_svr_ip[1],web_svr_ip[2],web_svr_ip[3]);
	pushsdk_options.svrport=web_svr_port;

//	printf("Init>>>svrip:%s\n>>>svrport%d\n",pushsdk_options.svrip,pushsdk_options.svrport);
	//add end
	
	snprintf(pushsdk_options.fw_ver,sizeof(pushsdk_options.fw_ver),"%s",fw_ver);
	if ((web_url!=NULL) && (strncasecmp((char *)web_url,"HTTP://",7) != 0)) {
		if(web_svr_ip[0] != 0) {
			snprintf(pushsdk_options.root_url,sizeof(pushsdk_options.root_url),\
			"http://%d.%d.%d.%d:%d/iclock/",\
			web_svr_ip[0],web_svr_ip[1],web_svr_ip[2],web_svr_ip[3], web_svr_port);
		} else {
			return PUSH_ERROR_WEBURL;
		}
	} else {
		snprintf(pushsdk_options.root_url,\
		strlen((char *)web_url)<100?strlen((char *)web_url)+1:100,"%s",web_url);
	}
	
	//printf("[PUSH SDK]    ICLOCK SVR URL:%s\n",pushsdk_options.root_url);
	pushsdk_options.send_buf_size = send_buf_size;
	pushsdk_options.trans_intval = 300;
	pushsdk_options.get_request_intval = LoadInteger("Delay", 10);
	pushsdk_options.reposttime=LoadInteger("Reposttime", 3);		//发送失败重发次数
	pushsdk_options.error_intval = LoadInteger("ErrorDelay", 10);
	pushsdk_options.is_real_time = LoadInteger("LogRealTime",1);
	pushsdk_options.websvr_time_out = LoadInteger("WebServerTimeout",60);
	pushsdk_options.cur_user_count= FDB_CntUserByMap();
	pushsdk_options.max_user_count= LoadInteger("~MaxUserCount",0)*100;
	if ((send_buf_size == 0) && (pushsdk_options.websvr_time_out < 150)) {
		pushsdk_options.websvr_time_out = 150;
	}
	pushsdk_options.TZ = LoadInteger("TZAdj", 8);
	pushsdk_options.language = LoadInteger("Language", 'E');
	if((pushsdk_options.TZ) < 25) {
		pushsdk_options.TZ *= 60;
	}
	pushsdk_options.buf_size = LoadInteger("PushBuferSize",64*1024);

	if (!LoadStr("~SerialNumber",pushsdk_options.SN)) {
		printf("[PUSH SDK]   ERROR--~SerialNumber OPTIONS CFG NOT INIT\n");
		return  PUSH_ERROR_NOSERIAL;
	}
	pushsdk_options.RegistOpenFlag = LoadInteger("RegistOpenFlag", 0);
	char buffer[36];
	memset(buffer, 0, 36);
	LoadStr("pushcommkey", buffer);
	CommKey2Byte(buffer, pushsdk_options.push_comm_key);
	pushsdk_options.UserVersion = LoadInteger("UserVersion", 0);
	pushsdk_options.push_lock_fun_on[0] =LoadInteger("~PushLockFunOn", 0);
	pushsdk_options.push_lock_fun_on[1] =LoadInteger("~LockFunOn", 0);

	if (LoadInteger("~DSTF",0) && LoadInteger("DaylightSavingTimeOn",0)) {
		pushsdk_options.day_light_fun = 1;
	}
	snprintf(pushsdk_options.ip_address,sizeof(pushsdk_options.ip_address),"%s",LoadStrOld("IPAddress"));
	pushsdk_options.zk_fp_ver = LoadInteger("~ZKFPVersion",9);
	
	pushsdk_options.upload_photo = LoadInteger("UploadPhoto",1);
	pushsdk_options.is_only_upload_photo= LoadInteger("IsUploadPhotoOnly",0);
	pushsdk_options.trans_max_rec = 200;

	if (LoadInteger("EnableProxyServer",0) != 0) {
		http_porxy.set((char *)proxy_ip,proxy_port);
	}
	if (pushsdk_mem == NULL) {
		pushsdk_mem = (char *)malloc(64*1024*sizeof(char));
		if (pushsdk_mem == NULL) {
			return PUSH_ERROR_MALLOC;
		}
	}
	return PUSH_OK;
}

void pushsdk_init_param(int FaceFunOn,int ZKFaceVersion,int FaceTmpCnt,int IsOnlyRFMachine,int ShowPhoto)
{
	if (FaceFunOn == 1) {
		pushsdk_options.ZKFaceVer = ((ZKFaceVersion==0) ? 5:ZKFaceVersion);
		pushsdk_options.FaceTmpCnt = FaceTmpCnt;	
	} else {
		pushsdk_options.ZKFaceVer = -1;
		pushsdk_options.FaceTmpCnt = 0;	
	}
	memset(pushsdk_options.dev_support_data,0,sizeof(pushsdk_options.dev_support_data) );
	pushsdk_options.dev_support_data[DEV_SUPPOPT_FP] = (char)((IsOnlyRFMachine==0) ? '1':'0');
	pushsdk_options.dev_support_data[DEV_SUPPOPT_FACE] = \
				(char)(((FaceFunOn==1) && dl_get_node_by_name(data_list,"FACE")) ? '1':'0');
	pushsdk_options.dev_support_data[DEV_SUPPOPT_USERPIC] = \
				(char)(((ShowPhoto==1) && dl_get_node_by_name(data_list,"USERPIC"))? '1':'0');
	
}
int  InitPushParam(int FaceFunOn,int ZKFaceVersion,int FaceTmpCnt,int IsOnlyRFMachine,int ShowPhoto)
{
	pushsdk_init_param(FaceFunOn, ZKFaceVersion, FaceTmpCnt, IsOnlyRFMachine, ShowPhoto);
	if (pushsdk_thread_create() == 0) {
		return PUSH_OK;
	}
	return PUSH_ERROR_PARAMS;
}

void pushsdk_pause(void)
{
	pid_t pid = pushsdk_get_pid();
	if ((pid > 0)) {
		pipe_write_to_child_cmd(PARENT_CMD_PAUSE);
	}
}
void pushsdk_re_init(void)
{
	pid_t pid = pushsdk_get_pid();
	if ((pid > 0)) {
		pipe_write_to_child_cmd(PARENT_CMD_RE_INIT);
	}
}
void pushsdk_init_cfg(void)
{
	pid_t pid = pushsdk_get_pid();
	if ((pid > 0)) {
		pushsdk_reload_cfg();
		pushsdk_sync_cfg();
		pipe_write_to_child_cmd(PARENT_CMD_RE_INIT);
	}
}
void pushsdk_resume(void)
{
	pid_t pid = pushsdk_get_pid();
	if ((pid > 0)) {
		pipe_write_to_child_cmd(PARENT_CMD_RESUME);
	}
}
pid_t pushsdk_get_pid(void)
{
	return (pushsdk_thread_id != NULL) ? 1:-1;
}
void pushsdk_close(void)
{
	pushsdk_thread_id = NULL;
	pipe_free();
	
}
void pushsdk_stop(void)
{
	pushsdk_re_init();
}

void pushsdk_data_reset(int ContentType)
{
	pid_t pid = pushsdk_get_pid();
	if ((pid > 0)) {
		pipe_write_to_child_cmd(PARENT_CMD_RESET_DATA);
		pipe_write_to_child((void *)&ContentType,sizeof(ContentType));
	}
}

void pushsdk_data_new(int ContentType)
{
	pid_t pid = pushsdk_get_pid();
	
	if ((pid > 0)) {
		pipe_write_to_child_cmd(PARENT_CMD_NEW_DATA);
		pipe_write_to_child(&ContentType,sizeof(ContentType));
	}
}

void pushsdk_stop_send_data(void)
{
	pid_t pid = pushsdk_get_pid();
	if ((pid > 0)) {
		pipe_write_to_child_cmd(PARENT_CMD_STOP_DATA);
	}
}
void daylight_check(iclock_options_t *opt)
{
	static int  day_light_bak = 12;
	int day_light;
	
	if (opt->day_light_fun) {
		day_light = IsDaylightSavingTime();
		if (day_light != day_light_bak) {
			day_light_bak = day_light;
			pipe_write_to_child_cmd(PARENT_CMD_DSTF);
			pipe_write_to_child_cmd((char)(day_light));
		}
	}
}

static void   SystemChildAbort(int   nSigNo)
{
	printf("[PUSH SDK]    %s error=%s\n",__FUNCTION__,strerror(errno));
	sleep(5);
}
/*
static void   SystemChildPipe(int   nSigNo)
{
	printf("[PUSH SDK]    %s error=%s\n",__FUNCTION__,strerror(errno));
	sleep(5);
}
*/
static void   SystemChildQuit(int   nSigNo)
{
	printf("[PUSH SDK]    %s error=%s\n",__FUNCTION__,strerror(errno));
}
static void   SystemChildIll(int   nSigNo)
{
	printf("[PUSH SDK]    %s error=%s\n",__FUNCTION__,strerror(errno));
	sleep(5);
}
static void   SystemChildKill(int   nSigNo)
{
	printf("[PUSH SDK]    %s error=%s\n",__FUNCTION__,strerror(errno));
	sleep(2);
}

static void   SystemChildSegv(int   nSigNo)
{
	printf("[PUSH SDK]    %s error=%s\n",__FUNCTION__,strerror(errno));
	sleep(3);
}
void pushsdk_sync_data(int con_type)
{
	data_list_t *node = NULL;

	if (FCT_ALL == con_type) {
		node = data_list;;
		while (node != NULL) {
			if (node->auto_push_flag) {
				node->first_read = 1;
			}
			node = node->next;
		}
		file_clr_data(BS_TRANS_USER_FILE);
	} else {
		node = dl_get_node_by_type(data_list,con_type);
		if (node != NULL) {
			node->first_read = 1;
		}
	}
}



int initpostatt(PPostAttLog log)
{	
	int attfp=0;
	unsigned int attcur[2];
	attfp=open(BS_TRANS_ATT_FILE,O_CREAT|O_RDWR|O_SYNC|O_NONBLOCK,S_IRWXU|S_IRWXG|S_IRWXO);
	if(attfp<0)
	{
		printf("open BS_TRANS_ATT_FILE failed\n");
		return -1;
	}
	if ((!read_file_with_offset(attfp,0,(char *)&attcur,sizeof(attcur)))
		|| (attcur[0] <= 0)) {		
		printf("read BS_TRANS_ATT_FILE failed\n");
		log->currec=0;		//可以从记录文件读取
		log->curfp=0;
	}else{
		printf("read BS_TRANS_ATT_FILE OK attcur=%d curfp=%d\n",attcur[0], attcur[1]);
		log->currec=attcur[0];
		log->curfp=attcur[1];
	}
	memset(log->attbuffer,0,1024);	
	log->maxrec=recindex;
	log->reclenght=0;
	if(attfp)
		close(attfp);
	return 0;
}

static void init(iclock_options_t *opt)
{
	int ret = 0;
	gErrorCode = 0;
	gWarningLevel=0;
	gWarningCode=0;
	error_time= 0;
	printf("[%s]RegistOpenFlag=%d\n", __FUNCTION__, opt->RegistOpenFlag);
	pushsdk_main_fun_init(opt);
	signal(SIGABRT,SystemChildAbort);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGQUIT,SystemChildQuit);
	signal(SIGILL,SystemChildIll);
	signal(SIGKILL,SystemChildKill);
	signal(SIGSEGV,SystemChildSegv);
	
	clean_tmp_file("/tmp/");
	opt->stat.running_flag = 0;
	opt->stat.last_post_time = time(NULL);
	opt->stat.info_update = 1;
	resume_broken_transfer_init();

	opt->stat.get_interval=opt->get_request_intval;		//心跳间隔时间
	opt->stat.post_interval=1;
	initpostatt(&postattlog);
	gAttflag = 1;
	if(1 == opt->RegistOpenFlag)
	{	
		ret = svr_heartbeat_ymip(opt);
		if (ret >= 0)
			set_bit(&opt->stat.running_flag, RUN_STAT_CONNECT);
		else
		{
			printf("[PUSH SDK]  HEARTBEAT FAILED [%s:%d]\n", opt->svrip, opt->svrport);
			clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
			if(-1 == ret)
				msleep(1000*180);
		}
	}
	else
	{
	if(svr_init_ymip(opt) < 0)
	{
		printf("[PUSH SDK]  INIT FAILED [%s:%d]\n", opt->svrip, opt->svrport);
		clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
	}
	else
	{
		if(svr_open_ymip(opt) >= 0)
		{
			opt->RegistOpenFlag = 1;
			cmd_save_options(opt, "RegistOpenFlag", "1", 1);
			set_bit(&opt->stat.running_flag, RUN_STAT_CONNECT);
		}
		else
		{
			printf("[PUSH SDK]  OPEN DEV FAILED [%s:%d]\n", opt->svrip, opt->svrport);
			clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
		}
	}
	}
}

void *pushsdk_mainloop_handle(void * arg)
{
	pushsdk_options.stat.child_thread_id = pthread_self();
	if (pipe_init() < 0) {
		return NULL;
	}
	init(&pushsdk_options);//在初始化的时候，进行一次注册
	child_mainloop_handle(&pushsdk_options);
	thread_exit();

	if (pushsdk_thread_id != NULL) {
		free(pushsdk_thread_id);
		pushsdk_thread_id = NULL;
	}
	if (pushsdk_mem != NULL) {
		free(pushsdk_mem);
		pushsdk_mem = NULL;
	}
	return NULL;
}
int pushsdk_thread_create(void)
{
	if (pushsdk_thread_id != NULL) {
		return -1;
	}
	
	pushsdk_thread_id = thread_create(pushsdk_mainloop_handle, NULL);
	if (pushsdk_thread_id == NULL) {
		return -1;
	}
	pushsdk_options.stat.parent_thread_id = pthread_self();
	return 0;
}


/**********************************************
心跳请求
head部分:
0100003000020002
data部分:
112233445566778899001100000000010000000000000000000000000000000000000000
maccheck部分:
00000000

***********************************************/
int svr_heartbeat_ymip(iclock_options_t *opt)
{	
	unsigned short int return_len;
	int error_code = -1;
	FILE *svr_return;
	TYMIP_HEAD recv_head;
	TYMIP_HEAD post_head;
	THeartbeat post_data;
	TTransinfo transinfo;
	unsigned int translen;
	char data_return[1024];
	BOOL checkFN=FALSE;
	dev_time = time(NULL);
	int j = 0;
	unsigned char hearbeatlen[3]={0x00,0x00,0x30};
	memcpy(commkey, opt->push_comm_key, 16);


	memset(&recv_head,0,sizeof(TYMIP_HEAD));
	memset(&post_head,0,sizeof(TYMIP_HEAD));
	memset(&post_data,0,sizeof(THeartbeat));
	memset(&transinfo,0,sizeof(TTransinfo));
	translen=sizeof(TYMIP_HEAD)+sizeof(THeartbeat)+4;
//head
	incFN();
	ymip_head_format(&post_head,LICENSEVERSION,hearbeatlen,FramNumber,HEARTBEAT);
		
//data
	printf("[%s]__[%d]user version %d\n", __FUNCTION__, __LINE__, opt->UserVersion);
	ymip_data_heartbeat_format(&post_data,DevNumber,0x00000001,0x01,opt->UserVersion);
//check 

	//给head和data赋值
	ymip_transinfo_format(&transinfo,&post_head,&post_data,sizeof(THeartbeat),0x00000000);
	while(j <=  opt->reposttime){
		j++;
		svr_return=ymip_proc(opt->svrip,opt->svrport, (char *)&recv_head, (char *)&transinfo, NULL, NULL, opt->error_intval, translen);
		if (NULL == svr_return) {
				(j == 1) ? printf("[PUSH SDK]    TRANS HEARTBEAT FAILED\n\n") : printf("[PUSH SDK]    RETRANS HEARTBEAT (%d)TIMES FAILED\n\n", j-1);
				msleep(300);
				continue;
		}
		return_len = svr_return_proc(svr_return, data_return, opt->websvr_time_out);
	if(0 != ChectMAC(&recv_head, data_return))
	{
			printf("[PUSH SDK]	[%s]_MAC check failed...regist again\n", __FUNCTION__);
		gErrorCode = 0x0009;
		return -9;
	}
	if(svr_return)
	{
	    fclose(svr_return);
	}
		printf("[%s]_[%d]\n",__FUNCTION__,__LINE__);
		if((return_len + 8) != GetPacketLength(&recv_head))
		{
			printf("[%s] Return len error  ret_len+8=%d\n", __FUNCTION__, return_len+8);
			msleep(300);
			gErrorCode = 0x000a;
			return -10;
		}
	checkFN=IsRightFN(post_head.FrameNum,recv_head.FrameNum);//帧号检验
		error_code = ProcessService(&recv_head, data_return, opt);

		if ((error_code < 0) /*||(checkFN==FALSE)*/) {
			return -2;
		} 
		else
		{
			pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
			error_code = 0;
			break;
		}
	}
	if (j >= opt->reposttime) {
		error_code = -1;
	}
	return error_code;
}


/*
			
接收到报文:0100003300010001----1122334455667788990011--000000000000--0002--01--3030303030303030--20140113105953--00000000----00000000   
报文的业务类型：0x0001   
报文的帧序号：0001   
报文的机具号：1122334455667788990011   
服务端回复报文：01000028000100011122334455667788990011303030303030303030303030303030300100000000
*/

int svr_warning_ymip(iclock_options_t *opt)
{	
	int return_len;
	int error_code = -1;
	FILE *svr_return;
	TYMIP_HEAD recv_head;
	TYMIP_HEAD post_head;
	TWarning  post_warning_data;
	TTransinfo transinfo;
	unsigned int transinfolen, infolen;
	unsigned char data_return[1024];
	BOOL checkFN=FALSE;

	memcpy(commkey, opt->push_comm_key, 16);
	unsigned char registerlen[3]={0x00,0x00,0x45};		
	//unsigned char registerlen[3]={0x00,0x00,0x00};	
	unsigned char PSAM_num[6]={0x00,0x00,0x00,0x00,0x00,0x00};							
	unsigned char DevVersion[8]={0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};			
	unsigned char DevTime[7];			
	unsigned char CustomID[4]={0x00,0x00,0x00,0x00};	
	memset(&recv_head,0,sizeof(TYMIP_HEAD));
	memset(&post_head,0,sizeof(TYMIP_HEAD));
	memset(&post_warning_data,0,sizeof(TWarning));
	memset(&transinfo,0,sizeof(TTransinfo));
	transinfolen=sizeof(TYMIP_HEAD)+sizeof(TWarning)+4;
//head
	incFN();
	ymip_head_format(&post_head,LICENSEVERSION,registerlen,FramNumber,0x0007);
	
	time2byte(DevTime);
//data
	printf("[%s] Get warning info and post\n", __FUNCTION__);
	//ymip_data_reg_format(&post_warning_data,DevNumber,PSAM_num,0x0001,0x01,DevVersion,DevTime,CustomID);
	ymip_data_warning_format(&post_warning_data,DevNumber,PSAM_num,0x0001,0x01,DevVersion,DevTime,0x01,gWarningLevel,gWarningCode);
//check 
	//给head和data赋值
	ymip_transinfo_format(&transinfo,&post_head,&post_warning_data,sizeof(TWarning),0x00000000);
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
		return -1;
	}
	if(svr_return)
	{
		fclose(svr_return);
	}
	
	checkFN=IsRightFN(post_head.FrameNum,recv_head.FrameNum);//帧号检验
	error_code = ProcessService(&recv_head, data_return, opt);
	if ((error_code < 0)/*||(checkFN==FALSE)*/) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
	} else {
		pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
	}
	//if(checkFN==FALSE){
	//	error_code=-2;
	//}
	return error_code;
}



static void  proc(iclock_options_t *opt)
{
	int post = 0;
	int ret = -1;
	static time_t get_t = 0;
	static time_t post_t = 0;
	opt->stat.post_interval = 1;
	if ((abs(time(NULL) - get_t)  >= opt->stat.get_interval) ||\
	   (abs(time(NULL) - post_t) >= opt->stat.post_interval)) {
//		if ((!check_left_data_to_upload(opt,TRANS_TYPE_SVR)) ) {
//			return;
//		}
	}
//	real_time_event_proc(opt);
	 if (abs(time(NULL) - get_t)  >= opt->stat.get_interval) {
		get_t = time(NULL);
//		opt->stat.get_interval = cmd_get_proc(opt);
		ret = svr_heartbeat_ymip(opt);
		CheckErrorStatus(opt);
		printf("[%s] gErrorCode=%04x gWarningCode=%04x\n", __FUNCTION__,gErrorCode, gWarningCode);
		if(0 != gErrorCode){	
			msleep(100*3);
			ret = TransErrorCode(gErrorCode, opt);
			gErrorCode = 0;
		}		
		if(0 != gWarningCode){
			msleep(100*3);
			svr_warning_ymip(opt);
 			gWarningCode = 0;
			gWarningLevel = 0;
		}
		if(ret < 0){
			clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
			pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
			if(-1 == ret)
				msleep(1000*180);
		}		
	}
	/*Compare Time*/
	/*1.iClockOptions.realTimeLog=1 >> 2.opt->gTransInterval>0 >> 3.opt->gTransTimes*/
	//post = post_data_by_timing(opt);
	if ((abs(time(NULL) - post_t) >= opt->stat.post_interval) &&
		((opt->trans_intval > 0) || (opt->con_type > 0)))  {
		post_t = time(NULL);
		postattlog.maxrec=recindex;
		//printf("Memset postattlog  postattlog.maxrec=%d  postattlog.currec=%d\n",postattlog.maxrec,postattlog.currec);
		if((postattlog.currec<postattlog.maxrec) && (!get_bit(&opt->stat.running_flag, RUN_STAT_CMD)) && (1 == gAttflag))	//上传考勤记录
		{	
			gAttflag = 0;
			pipe_write_to_parent_cmd(CHILD_CMD_READ_ATTLOG);
		}
//		post += post_data_by_interval(opt);
	} else if ((PUSHSDK_IS_CMD()) && \
		(((opt->is_real_time == 0) && (opt->trans_intval == 0)) ||\
		(opt->con_type == 0))) {
		clear_bit(&opt->stat.running_flag,RUN_STAT_CMD);
	}
	if(post > 0) {
		opt->stat.info_update = 1;
	}
}
static void show_data_type(char *cmd)
{
	data_list_t *node;
	int type;
	printf("[%s]___%d\n",__FUNCTION__,__LINE__);
	memcpy(&type,cmd,sizeof(int));
	node = dl_get_node_by_type(data_list, type);
	if (node != NULL) {
		printf("[PUSH SDK]    CHILD GET PARENT DATA:%s\n",node->table_name);
	} else {
		printf("[PUSH SDK]    CHILD GET PARENT DATA:UNKNOW TYPE-%d\n",type);
	}
}
static void child_call_proc(iclock_options_t * opt)
{	
	if (PUSHSDK_IS_MUTEX()) {
		static int stop_cnt = 0;
		msleep(400);
		if (stop_cnt++ == 3) {
			stop_cnt = 0;
			clear_bit(&opt->stat.running_flag, RUN_STAT_MUTEX);
		}
	} else if (!get_bit(opt->stat.running_flag,RUN_STAT_CONNECT)){
		if(1 == opt->RegistOpenFlag)
		{	
			int ret = 0;
			ret = svr_heartbeat_ymip(opt);
			if (ret >= 0)
			{
				pipe_clean_parent_cmd();
				opt->stat.running_flag = 0;
				opt->stat.last_post_time = time(NULL);
				set_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
			} 
			else 
			{
				printf("[PUSH SDK]  INIT FAILED:%s, error_intval=%d\n",opt->root_url, opt->error_intval);
				clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
				if(-1 == ret)
					msleep(1000*180);
			}
		}
		else
		{
			if (svr_init_ymip(opt)>=0)
			{
				pipe_clean_parent_cmd();
				opt->stat.running_flag = 0;
				opt->stat.last_post_time = time(NULL);
				if(svr_open_ymip(opt) >=0)
				{	
					opt->RegistOpenFlag = 1;
					cmd_save_options(opt, "RegistOpenFlag", "1", 1);
					set_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
				}
				else
					clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
			} 
			else 
			{
				printf("[PUSH SDK]  INIT FAILED:%s\n",opt->root_url);
				clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
				msleep(1000*3);
			}
		}		
	}
	else if (!PUSHSDK_IS_PAUSE())
	{
		proc(opt);
	}
}
void child_deal_parent_cmd(iclock_options_t *opt,char parent_cmd)
{
	char cmd[1024];
	
	switch (parent_cmd) {
	case PARENT_CMD_RE_INIT:
		clear_bit(&opt->stat.running_flag,RUN_STAT_CONNECT);
		break;
	case PARENT_CMD_PAUSE:
		set_bit(&opt->stat.running_flag, RUN_STAT_PAUSE);
		break;
	case PARENT_CMD_RESUME:
		clear_bit(&opt->stat.running_flag, RUN_STAT_PAUSE);
		break;
	case PARENT_CMD_DSTF:
		if (pipe_read_from_parent(cmd,1) == 1) {
			opt->is_enter_day_light = cmd[0];
		}
		break;
	case PARENT_CMD_STOP_DATA:
		set_bit(&opt->stat.running_flag, RUN_STAT_MUTEX);
		break;
	case PARENT_CMD_RESET_DATA:
		if (pipe_read_from_parent(cmd,sizeof(int)) == sizeof(int)) {
			int data;
			memcpy(&data,cmd,sizeof(int));
			pushsdk_sync_data(data);
		}
		break;
	default:
		break;
	}
}
void child_mainloop_handle(iclock_options_t *opt)
{
	char cmd[1024*2];
	int cnt = 0;
	while (1) {
		msleep(150);
		if (!PUSHSDK_IS_CMD()|| PUSHSDK_IS_PAUSE() || \
			(!get_bit(opt->stat.running_flag,RUN_STAT_CONNECT))) {
			cnt = pipe_read_from_parent(cmd, 1);
		} else {
			cnt = 0;
		}
		if (cnt == 1) {
			pushsdk_cmd_printf(0, cmd[0]);
			switch (cmd[0]) {
			case PARENT_CMD_NEW_DATA:
				if (pipe_read_from_parent(cmd,sizeof(int)) == sizeof(int)) {
					if (opt->is_real_time) {
						msleep(400);
						memcpy(&opt->con_type,cmd,4);
					//	set_bit(&opt->stat.running_flag, RUN_STAT_CMD);
						opt->stat.post_interval = 1;	
					}
					show_data_type(cmd);
				}
				break;
			case PARENT_CMD_FROM_SVR: 
				opt->stat.get_interval = 0;
				printf("[%s]___%d\n",__FUNCTION__,__LINE__);
				while ((cnt=pipe_read_from_parent(cmd, 1)) == 1) {
					if (cmd[0] != PARENT_CMD_FROM_SVR)  {
						break;
					}
				}
				if (cnt != 1) {
					break;
				}

			case CHILD_CMD_POST_ATT:		//发送考勤流水记录
				set_bit(&opt->stat.running_flag, RUN_STAT_CMD);
				opt->stat.post_interval = 1;
				if(1)
				{
					int i=0;
					printf("[%s]___%d post att log**ATT START***************\n", __FUNCTION__,__LINE__);
					for(i=0;i<postattlog.reclenght;i++)
					{
						printf("0x%02x ",postattlog.attbuffer[i]);
					}
					printf("\n*****************************ATT END************************\n");
				}	
				cache_att_to_svr(opt,&postattlog);
				gAttflag = 1;
				if(postattlog.maxrec<=postattlog.currec)
				{
					opt->stat.post_interval = 1;
				}
				clear_bit(&opt->stat.running_flag, RUN_STAT_CMD);
				break;
			default:
				printf("[%s]___%d\n",__FUNCTION__,__LINE__);
				child_deal_parent_cmd(opt,cmd[0]);
				break;
			}
		}
		child_call_proc(opt);
		if (cnt < 0 ) {
			printf("[PUSH SDK]    PIPE COMM FAILED\n");
			_exit(10); 
		}
	}
}

 int svr_cmd_check_by_udp(void)
{
	static int svrCmdSocket=0;
	if (svrCmdSocket == 0) {
		if (udp_init(4374, &svrCmdSocket) != 0){
			svrCmdSocket=-1;
		}
	}
	if (svrCmdSocket > 0) {
		struct sockaddr_in from;
		char buf[16]={0};
		unsigned int flen=sizeof(from);
		
		int len = recvfrom(svrCmdSocket, buf, 16, 0, (struct sockaddr*)&from, &flen);
		if (len > 0) {
			buf[15]=0;
			pipe_write_to_child_cmd(PARENT_CMD_FROM_SVR);
		} else if ((len==-1) && (errno != EAGAIN)) {
			if (errno == EBADF) {
				svrCmdSocket = 0;
			}
		}
		return len;
	}
	return 0;
}


static int parent_mainloop_handle(void)
{	
	char cmd[1024];
	int ch = 0;
	static int iclock_state = ICLOCK_STATE_OFFLINE;

	while (pipe_read_from_child(cmd, 1) == 1) {
		ch = cmd[0];
		if (ch != CHILD_CMD_NEW_DATA) {
			pushsdk_cmd_printf(1,ch);
		}

		switch(ch) {
		case CHILD_CMD_CNT:
			pushsdk_cnt_log_by_time(&pushsdk_options);
			break;
		case CHILD_CMD_ONLINE:
			iclock_state = ICLOCK_STATE_ONLINE;
			break;
		case CHILD_CMD_OFFLINE:
			iclock_state = ICLOCK_STATE_OFFLINE;
			break;
		case CHILD_CMD_EXIT_WORKING:
			iclock_state = ICLOCK_STATE_ONLINE;
			break;
		case CHILD_CMD_RELOAD_OPTIONS:  
			ReLoadOptions();
			break;
		case CHILD_CMD_RELOAD_DATA: 
			pushsdk_pause();
			FDB_InitDBs(FALSE);
			FPInit(NULL);
			pushsdk_resume();
			break;
		case CHILD_CMD_REBOOT:  
			system_vfork("reboot"); 
			break;
		case CHILD_CMD_NEW_DATA:
			call_main_update_data(&pushsdk_options);
			break;
		case CHILD_CMD_ENROLL_FP:
			if(sizeof(int)+3==pipe_read_from_child(cmd+1, sizeof(int)+3)) {
				char buf[10];
				int ret=mainEnrollCmd(cmd);
				buf[0]=PARENT_CMD_RESUME;
				memcpy(buf+1, &ret, 4);
				pipe_write_to_child(buf, 5);
			}
			break;
		case CHILD_CMD_CLEAR_DATA:
			main_clear_data(&pushsdk_options);
			break;
		case CHILD_CMD_RESET_ALARM:
			DoAlarmOff(0);
			reset_alarm();
			break;
		case CHILD_CMD_SET_OPTIONS:
			main_save_options(&pushsdk_options);
			break;
		case CHILD_CMD_READ_ATTLOG:
			set_bit(&pushsdk_options.stat.running_flag, RUN_STAT_CMD);
			memset(postattlog.attbuffer,0,1024);
			pushsdk_options.cur_att_count = FDB_CntAttLog();
			main_read_attlog(&pushsdk_options,&postattlog);
			printf("[%s]___%d reclebght=%d  curfp=%d\n",__FUNCTION__,__LINE__,postattlog.reclenght,postattlog.curfp);
			if(postattlog.reclenght>0)
			{	printf("read cmd:CHILD_CMD_post_ATTLOG\n");
				//clear_bit(&pushsdk_options.stat.running_flag, RUN_STAT_CMD);
				pipe_write_to_child_cmd(CHILD_CMD_POST_ATT);
			}else
			{	
				gAttflag = 1;
				pushsdk_options.stat.post_interval = 1;
			}
			clear_bit(&pushsdk_options.stat.running_flag, RUN_STAT_CMD);
			break;
		case CHILD_CMD_USER_CNT:
			if(1){
				int maxusercnt;
				int usercnt;
				usercnt = FDB_CntUserByMap();
				maxusercnt = LoadInteger("~MaxUserCount",0)*100;
				pushsdk_options.cur_user_count = usercnt;
				pushsdk_options.max_user_count = maxusercnt;
				pushsdk_options.cmd_deal_over = 1;
			}
			break;

		default:
			break;
		}
	}
	daylight_check(&pushsdk_options);

	while (svr_cmd_check_by_udp() >  0) {
		continue;
	}
	return iclock_state;
}

int pushsdk_check(void)
{
	if (pushsdk_thread_id != NULL) {
		return parent_mainloop_handle();
	} 
	return ICLOCK_STATE_OFFLINE;
}

int  PushSDKVersion(char * version, int buflen)
{
	if(version) {
		snprintf(version, buflen, "%s",  PUSHVER);
		return 0;
	}
	return -1;
}

void GetLastUserCardSerial(char *cardserial)
{
	memset(&pushsdk_options.last_user_card_serial, 0x0, 10);
	memcpy(&pushsdk_options.last_user_card_serial, cardserial, 10);
}
