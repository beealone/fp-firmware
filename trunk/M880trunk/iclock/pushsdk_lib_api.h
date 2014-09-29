#ifndef PUSHSDK_LIB_H_
#define PUSHSDK_LIB_H_

#define TRANS_MAX_RECORD_NO_LIMIT -1

#define BS_TRANS_HELP_FILE 		"/mnt/mtdblock/data/BsTransHelp.dat"
#define BS_TRANS_USER_FILE		"/mnt/mtdblock/data/BsUser.dat"
#define BS_TRANS_FP_FILE		"/mnt/mtdblock/data/BsFp.dat"
#define BS_TRANS_FACE_FILE		"/mnt/mtdblock/data/BsFace.dat"
#define BS_TRANS_USRPIC_FILE	"/mnt/mtdblock/data/BsUsrPic.dat"

#define BS_TRANS_ATT_FILE		"/mnt/mtdblock/BsAtt.dat"

#define U_USER_DATA_FILE 		"/mnt/mtdblock/data/U_User.dat"
#define U_FP_DATA_FILE    		"/mnt/mtdblock/data/U_Fp.dat"

#define TRANS_HELP_OFFSET_SIZE 8
enum _trans_help_offset_ {
	 TRANS_HELP_FILE_USER = 0*TRANS_HELP_OFFSET_SIZE,
	 TRANS_HELP_FILE_FP = TRANS_HELP_OFFSET_SIZE,
	 TRANS_HELP_FILE_FACE = 2*TRANS_HELP_OFFSET_SIZE,	
	 TRANS_HELP_FILE_USERPIC = 3*TRANS_HELP_OFFSET_SIZE,	
	 TRANS_U_FILE_USER = 4*TRANS_HELP_OFFSET_SIZE	,	
	 TRANS_U_FILE_FP = 5*TRANS_HELP_OFFSET_SIZE,
};

#define TRANS_TYPE_U			0
#define TRANS_TYPE_SVR			2

#define PUSHSDK_DATA_CHG 	0
#define PUSHSDK_DATA_ADD  	1
#define PUSHSDK_DATA_DEL	2
#define PUSHSDK_DATA_GET				0x10
#define PUSHSDK_DATA_GET_BY_PIN		0x11
#define PUSHSDK_DATA_GET_BY_PIN2		0x12
#define PUSHSDK_DATA_GET_BY_OFFSET	0x13
#define PUSHSDK_DATA_GET_USER_ALL_FP  0x14
#define PUSHSDK_DATA_GET_ALL_FP_INDEX 0x15

#define UPLOADE_MODE_QUERY 	1
#define UPLOADE_MODE_NOR		2

#define ICLOCK_STATE_ONLINE 	0
#define ICLOCK_STATE_OFFLINE	1

typedef enum {
	PUSH_OK = 0x00,
	PUSH_ERROR_WEBURL,
	PUSH_ERROR_NOSERIAL,
	PUSH_ERROR_ISRUNNING,
	PUSH_ERROR_PARAMS,
	PUSH_ERROR_FUN_NULL,
	PUSH_ERROR_MALLOC,
	PUSH_ERROR_DATAEXIST,
	PUSH_ERROR_DATAOVER,
}push_error_e;

typedef enum {
	PUSH_SVR_ATTLOG = 0x00,
	PUSH_SVR_OPERLOG,
	PUSH_SVR_ATTPHOTO,
	PUSH_SVR_EN_FP,
	PUSH_SVR_EN_USER,
	PUSH_SVR_FP_IMG,
	PUSH_SVR_CHG_USER,
	PUSH_SVR_CHG_FP,
	PUSH_SVR_FACE,
	PUSH_SVR_USER_PIC,
}push_svr_data_type_e;

typedef enum {
	PUSH_RET_SUCESS = 0x00,
	PUSH_RET_PIN2,
	PUSH_RET_SIZE,
	PUSH_RET_NO_REG,
}push_svr_return_value_e;

typedef struct {
	char *start;
	char *end;
}time_range_t;

typedef int (*ForARecFun)(void *rec, int count, void *param);
typedef int (*pushsdk_cmd_f)(char *param);
typedef int (*pushsdk_update_f)(char *param);
typedef int (*pushsdk_del_f)(char *param);
typedef int (*pushsdk_filter_f)(void *data, char *param);
typedef int (*pushsdk_format_f)(void *data, char *text, size_t *stamp,char *filepath);

int InitPushParam(int FaceFunOn,int ZKFaceVersion,int FaceTmpCnt,int IsOnlyRFMachine,int ShowPhoto);
int pushsdk_register_fun(char* cmd, pushsdk_cmd_f fun);
int pushsdk_register_data_fun(const char *table_name, int con_type, 
			int	parent_con_type,char	auto_push_flag,pushsdk_update_f	update_fun,
			pushsdk_del_f del_fun,pushsdk_filter_f	filter_fun,    pushsdk_format_f	format_fun);
int pushsdk_register_help_fun( const char *table_name,int con_type,
						pushsdk_filter_f filter_fun, pushsdk_format_f format_fun);

int pushsdk_init(unsigned char *WebServerURL,unsigned char* WebServerIP,\
					unsigned int WebServerPort, unsigned char* ProxyIP,unsigned int ProxyPort,\
					int SendBufferSize,char *firmwareVer);
typedef int (*fdb_cnt_log_f)(void);
typedef int (*fdb_clear_log_f)(void);
typedef int (*fdb_clear_data_f)(int type);
typedef int (*fdb_upload_usr_data_f)(int con_type, char *data_buf,int file_index,int fd);
typedef int (*fdb_upload_read_data_f)(int contype,void *usr_data,int index,void *data);
typedef int (*create_data_record_f)(char *file_name,int contype);
typedef int ( *main_update_data_f)(int type,int action,void * data,void *addinfo,unsigned int info_len);
typedef BOOL  (*fdb_log_cnt_with_time_f)(char *start_time,char *end_time,unsigned int *cnt);
typedef int  (*dev_is_busy_f)(void);
typedef void (*update_file_offset_f)(int contype,off_t offset);
typedef int (*fdb_read_attlog_f)(unsigned char *buf, int *maxrec, int *currec, int *attlenght);

void pushsdk_register_cnt_fun(fdb_cnt_log_f face_cnt_fun,fdb_cnt_log_f attlog_cnt_fun,
									fdb_cnt_log_f user_cnt_fun,fdb_cnt_log_f fp_cnt_fun,
									fdb_log_cnt_with_time_f attlog_cnt_with_time_fun,
									fdb_log_cnt_with_time_f operlog_cnt_with_time_fun);
void pushsdk_register_clear_fun(fdb_clear_log_f admin_clear_fun,fdb_clear_log_f attlog_clear_fun,
									  fdb_clear_log_f photo_clear_fun,fdb_clear_data_f data_clear_fun);

void pushsdk_register_read_data_fun(fdb_upload_usr_data_f upload_usr_read_fun,
				fdb_upload_read_data_f upload_read_data_fun,
				create_data_record_f  create_data_fun,main_update_data_f main_update_data,
				dev_is_busy_f dev_is_busy_fun,update_file_offset_f update_file_offset_fun,fdb_read_attlog_f read_attlog_fun);
int pushsdk_check(void);
void pushsdk_pause(void);
void pushsdk_resume(void);
void pushsdk_stop(void);
void pushsdk_data_reset(int ContentType);
void pushsdk_data_new(int ContentType);
void pushsdk_stop_send_data(void);
char* pushsdk_get_server_ver(void);
pid_t pushsdk_get_pid(void);
void pushsdk_re_init(void);
int DownLoadUser(char *pin);
int AuthFromHttpServer(char *PIN2,char *pcAuthInfo, int AuthInfoLen,void *userinfo,int u32AuthType);
void pushsdk_init_cfg(void);
int pushsdk_deal_data(void  *data,int size,int action,int type,void *info,int info_size);
void GetLastUserCardSerial(char *cardserial);
#endif

