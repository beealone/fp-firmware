#ifndef 	PUSH_CMD_H_
#define	PUSH_CMD_H_
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "pushsdk.h"
#include "http.h"
#include "lib.h"
#include "fw_api.h"
#include "pipe.h"
#include "strlist.h"
#include "data_list.h"

#define POST_FILE 	"/tmp/post.txt"
#define GET_FILE		"/tmp/reqs.txt"
#define DATA_FILE 	"/tmp/data.txt"


#define PUSH_CMD_ALAREADY_RETURN		0x7FFFFFFF

#define MAX_URL_LEN	256
#define MAX_HEADER_LENGTH 	(4096*2)
#define MAX_URL_LENGTH			(1024*2)

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

typedef enum {
	FCT_ADMIN = -2,
	FCT_ALL = -1,
	FCT_ATTLOG = 1,
	FCT_FINGERTMP,
	FCT_USERPIC,	
	FCT_OPLOG ,	
	FCT_USER ,	
	FCT_SMS,
	FCT_UDATA,
	FCT_EXTUSER,
	FCT_DEPT ,
	FCT_SCH   ,
	FCT_LOGHTM ,
	FCT_ABNORHTM,
	FCT_CALCUHTM,
	FCT_ABNORHTM2,
	FCT_ATTRULE,
	FCT_ALARM,
	FCT_WORKCODE	,
	FCT_SHORTKEY,
	FCT_PHOTOINDEX,
	FCT_FACE = 60,
	FCT_FACEPOS = 61,
}file_content_type_t;

extern char *pushsdk_mem;

typedef int (*cmd_deal_f)(iclock_options_t*opt, char *param,char *req_id,char * str_cmd);
typedef struct {
	char* cmd;
	cmd_deal_f fun;
}cmd_line_t;

typedef struct {
	char *start;
	char *end;
}time_range_t;

typedef struct {
	int con_type;
	pushsdk_filter_f filter_fun;
	pushsdk_format_f	format_fun;
	void *param;
}trans_help_t;

#define UPLOADE_MODE_QUERY 	1
#define UPLOADE_MODE_NOR		2

typedef struct {
	size_t starting_time;
	size_t cur_trans_log_time;
	char *url;
	TBuffer *cache;
	iclock_options_t* opt;
	data_list_t * list;
	int error;
	int count;
	char *file_path;
	int upload_mode;
	trans_help_t *trans_help;
	off_t file_off;
}trans_info_t;

typedef struct {
	char  *data;
	int size;
	int action;
	int type;
	void *info;
	int info_size;
	int ret;
} trans_data_t;

typedef int (*ForARecFun)(void *rec, int count, void *param);
typedef int (*time_sync_f)(time_t before, char *http_header, iclock_options_t *opt);

void trans_info_init(trans_info_t* info,data_list_t *list,iclock_options_t * opt,
										char *url,trans_help_t *trans_hep,int mode);
int upload_left_data(trans_info_t *info,data_list_t *parent_node);
int svr_init(iclock_options_t *opt);
int Lib_CntFaceUser(iclock_options_t *opt);
void resume_broken_transfer_init(void) ;
BOOL  check_left_data_to_upload(iclock_options_t*opt,int mode);
void real_time_event_proc(iclock_options_t *opt);
int cmd_get_proc(iclock_options_t *opt);
int  post_data_by_timing(iclock_options_t *opt);
int post_data_by_interval(iclock_options_t *opt);
int get_data_from_svr(char *furl, char *fileName, time_sync_f time_sync,iclock_options_t *opt);
int call_main_update_data(iclock_options_t *opt);
int main_clear_data(iclock_options_t *opt);
 void main_save_options(iclock_options_t *opt);
 int data_trans(iclock_options_t *opt,ForARecFun fun,trans_help_t *trans_hep);

int pushsdk_parent_cmd_cnt_log(int contype,char *str1,char *str2);
int pushsdk_cnt_log_by_time(iclock_options_t * opt) ;
 int tzd(char *s);
 int cmd_save_options(iclock_options_t *opt,const char *name, const char *value, int SaveTrue);
 int  tell_and_wait_parent(iclock_options_t * opt,char cmd);
#endif

