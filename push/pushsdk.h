#ifndef ICLOCK_H
#define ICLOCK_H
#include "thread.h"
#include "data_list.h"
#include <sys/types.h>
#include <unistd.h>

#define PUSHVER  "2.31"
#define ICLOCK_STATE_ONLINE 	0
#define ICLOCK_STATE_OFFLINE	1

typedef enum {
	CHILD_CMD_ICLOCK_OPTIONS = 0x31,
	CHILD_CMD_RELOAD_OPTIONS,
	CHILD_CMD_RELOAD_DATA,
	CHILD_CMD_REBOOT,
	CHILD_CMD_ENROLL_FP,
	CHILD_CMD_UPDATE_ALARM,
	CHILD_CMD_NEW_DATA,
	CHILD_CMD_CLEAR_DATA,
	CHILD_CMD_EXIT_WORKING,
	CHILD_CMD_ONLINE,
	CHILD_CMD_OFFLINE,
	CHILD_CMD_RESET_ALARM,
	CHILD_CMD_SET_OPTIONS,
	CHILD_CMD_CNT,
	CHILD_CMD_READ_ATTLOG,
	CHILD_CMD_USER_CNT,
}child_cmd_e;

typedef enum {
	PARENT_CMD_PAUSE = 0x31,
	PARENT_CMD_RESUME ,
	PARENT_CMD_RESET ,
	PARENT_CMD_FROM_SVR ,
	PARENT_CMD_NEW_DATA ,
	PARENT_CMD_STOP_DATA ,
	PARENT_CMD_RESET_DATA,
	PARENT_CMD_DSTF,
	PARENT_CMD_SET_OPTIONS,
	PARENT_CMD_UPDATE_DATA,
	PARENT_CMD_OVER,
	PARENT_CMD_RE_INIT,
	PARENT_CMD_UPLOAD_ATTLOG,
	CHILD_CMD_POST_ATT,
}parent_cmd_e;


typedef enum {
	DEV_SUPPOPT_FP = 0x00,
	DEV_SUPPOPT_FACE,
	DEV_SUPPOPT_USERPIC,
}dev_support_data_e;

#define RUN_STAT_CMD 		0x00
#define RUN_STAT_STOP 		0x01
#define RUN_STAT_PAUSE		0x02
#define RUN_STAT_MUTEX		0x03
#define RUN_STAT_CONNECT	0x04
#define RUN_STAT_WORKING	0x05

enum _DOWN_LOAD_USR_TYPE_ {
	DOWN_LOAD_USR = 0,
	DOWN_LOAD_FP = 1,
	DOWN_LOAD_FACE = 2,
	DOWN_LOAD_USRPIC = 3,
};
typedef struct {
	int running_flag;
	time_t post_interval;
	time_t get_interval;
	int options_update;
	int data_update;
	int info_update;
	time_t last_post_time;
	pthread_t parent_thread_id;
	pthread_t child_thread_id;
	void *context;
} iclock_run_state_t;

typedef int (*fdb_cnt_log_f)(void);
typedef int (*fdb_clear_log_f)(void);
typedef int (*fdb_clear_data_f)(int type);
typedef int (*fdb_upload_usr_data_f)(int con_type, char *data_buf,int file_index,int fd); 
typedef int (*fdb_upload_read_data_f)(int contype,void *usr_data,int index,void *data);
typedef int (*create_data_record_f)(char *file_name,int contype);
typedef int ( *main_update_data_f)(int type,int action,void * data,void *addinfo,unsigned int info_len);
typedef int  (*fdb_log_cnt_with_time_f)(char *start_time,char *end_time,unsigned int *cnt);
typedef int  (*dev_is_busy_f)(void);
typedef void (*update_file_offset_f)(int contype,off_t offset);
typedef int (*fdb_read_attlog_f)(unsigned char *buf, int maxrec, int currec, int maxcount,int *attlenght,off_t *curfp);

typedef struct {
	fdb_cnt_log_f face_cnt_fun;
	fdb_cnt_log_f attlog_cnt_fun;
	fdb_cnt_log_f user_cnt_fun;
	fdb_cnt_log_f fp_cnt_fun;
	fdb_clear_log_f admin_clear_fun;
	fdb_clear_log_f attlog_clear_fun;
	fdb_clear_log_f photo_clear_fun;
	fdb_clear_data_f data_clear_fun;
	fdb_upload_usr_data_f upload_usr_read_fun;
	fdb_upload_read_data_f upload_read_data_fun;
	create_data_record_f  create_data_record_fun;
	main_update_data_f main_update_data_fun;
	fdb_log_cnt_with_time_f attlog_cnt_with_time_fun;
	fdb_log_cnt_with_time_f operlog_cnt_with_time_fun;
	dev_is_busy_f 		dev_is_busy_fun;
	update_file_offset_f    update_file_offset_fun;
	fdb_read_attlog_f	read_attlog_fun;
}pushsdk_main_fun_t;

typedef struct  {
	char fw_ver[64];
	char root_url[128];
	char svrip[16];
	unsigned int svrport;
	int send_buf_size;
	size_t trans_intval;
	size_t get_request_intval;
	size_t error_intval;
	int is_real_time;
	size_t websvr_time_out;
	int TZ;
	int language;
	char SN[32];
	unsigned char push_comm_key[16];
	int ZKFaceVer;
	int FaceTmpCnt;	
	char dev_support_data[8];
	int push_lock_fun_on[2];	
	int day_light_fun;
	int is_enter_day_light;
	char ip_address[32];
	char zk_fp_ver;
	int upload_photo;
	int is_only_upload_photo;
	size_t attlog_stamp;
	size_t operlog_stamp;
	size_t attphoto_stamp;
	char svr_trans_flag[256];
	char post_timing[128];
	int is_encrypt;
	char svr_ver[64];
	int con_type;
	iclock_run_state_t stat;
	int buf_size;
	size_t trans_intval_s;
	int trans_max_rec;
	pushsdk_main_fun_t *main_fun;
	int cmd_deal_over;
	int max_user_count;
	int cur_user_count;
	int cur_att_count;
	unsigned short int reposttime;
	unsigned char last_user_card_serial[10];
	unsigned int CustomID;
	int RegistOpenFlag;
	int attlogwarningcnt;
	int UserVersion;
}iclock_options_t;

typedef struct {
	char weburl[256];
	int porxy_fun;
	int porxy_port;
	char porxy_ip[128];
}pushsdk_weburl_t;


int pushsdk_register_fun(char* cmd, pushsdk_cmd_f fun);
int pushsdk_register_data_fun(const char *table_name, int con_type, 
			int	parent_con_type,char	auto_push_flag,pushsdk_update_f	update_fun,
			pushsdk_del_f del_fun,pushsdk_filter_f	filter_fun,    pushsdk_format_f	format_fun);
void pushsdk_cmd_printf(int type,int cmd);
pid_t pushsdk_get_pid(void);
void pushsdk_data_reset(int ContentType);
void dev_cnt_data(iclock_options_t *opt,int *user_cnt,int *fp_cnt,int *attlog_cnt ,int *face_cnt);
int pushsdk_register_help_fun( const char *table_name,int con_type,
						pushsdk_filter_f filter_fun, pushsdk_format_f format_fun) ;
void child_deal_parent_cmd(iclock_options_t *opt,char parent_cmd);
extern  pushsdk_main_fun_t pushsdk_main_fun;

#endif
