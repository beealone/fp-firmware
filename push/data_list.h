#ifndef DATA_LIST_H_
#define DATA_LIST_H_
#include <time.h>
#define MAX_CMD_LEN	50

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

typedef struct CmdList{
	char cmd[MAX_CMD_LEN];
	int (*fun)(char *param);
	struct CmdList  *next;
}cmd_list_t;

typedef enum {
	DL_UPDATE = 0x00,
	DL_DEL,
	DL_FILTER,
	DL_FORMAT,
}dl_fun_type_e;
typedef int (*pushsdk_cmd_f)(char *param);
typedef int (*pushsdk_update_f)(char *param);
typedef int (*pushsdk_del_f)(char *param);
typedef int (*pushsdk_filter_f)(void *data, char *param);
typedef int (*pushsdk_format_f)(void *data, char *text, size_t *stamp,char *filepath);

typedef struct DataList {
	char table_name[MAX_CMD_LEN];
	int con_type; 
	int parent_con_type;
	char auto_push_flag;
	char first_read;
	int last_stamp;
	char *param;
    	pushsdk_update_f update_fun;
    	pushsdk_del_f		del_fun;
	pushsdk_filter_f	filter_fun;
	pushsdk_format_f	format_fun;
	struct DataList *next;
}data_list_t;

int dl_init(data_list_t **list,const char *table_name, int con_type, int	parent_con_type,char	auto_push_flag,
	pushsdk_update_f	update_fun,pushsdk_del_f	del_fun,pushsdk_filter_f filter_fun, pushsdk_format_f format_fun);
data_list_t *dl_get_node_by_name(data_list_t *list,char *name);

void dl_node_add_stamp(data_list_t *list,char *svr_info);

data_list_t *dl_get_node_by_parent_type(data_list_t * list, int type);

 data_list_t *dl_get_node_by_type(data_list_t * list, int type);

data_list_t *dl_get_node(data_list_t * list,int con_type,int *parent_type);
int dl_call_fun(data_list_t *list,char *table_name,int fun_type,char *param);
int cl_init(cmd_list_t **cmd_list,char* cmd, pushsdk_cmd_f fun);

#endif
