#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fw_api.h"
#include "data_list.h"
#include "lib.h"

data_list_t *dl_get_node_by_name(data_list_t *list,char *name)
{
	data_list_t *node = list;

	if ((name == NULL) || (name[0] == '\0')) {
		return NULL;
	}
	while (node != NULL) {
		if (strncasecmp(node->table_name,name,strlen(node->table_name))==0) {
			break;
		}
		node = node->next;
	}
	return node;
}
data_list_t *dl_get_last_node(data_list_t *list)
{
	data_list_t *node = list;

	if (list == NULL) {
		return NULL;
	}
	while (node->next != NULL) {
		node = node->next;
	}
	return node;
}
int dl_init(data_list_t **list, const char *table_name, int con_type, int	parent_con_type,char	auto_push_flag,
	pushsdk_update_f	update_fun,pushsdk_del_f	del_fun,pushsdk_filter_f filter_fun, pushsdk_format_f format_fun)
{
	data_list_t *last_node = NULL;
	data_list_t *new_node;

	if ((strlen(table_name) <= 0) || (strlen(table_name)+1 >= MAX_CMD_LEN)) {
		return PUSH_ERROR_PARAMS;
	}
	
	if ((update_fun==NULL)&&(del_fun==NULL)&&(filter_fun==NULL)&&(format_fun==NULL)) {
		return PUSH_ERROR_FUN_NULL;
	}

	if (dl_get_node_by_name(*list,(char *)table_name)) {
		return PUSH_ERROR_DATAEXIST;
	}
	last_node = dl_get_last_node(*list);

	new_node = (data_list_t *)malloc(sizeof(data_list_t));
	if (new_node==NULL) {
		printf("[PUSH SDK]    MALLOC NULL!\n\n");
		return PUSH_ERROR_MALLOC;
	}

	new_node->next = NULL;
	new_node->param = NULL;
	snprintf(new_node->table_name,sizeof(new_node->table_name),"%s",table_name);
	
	new_node->con_type = con_type;
	if (auto_push_flag) {
		new_node->parent_con_type = 0;	/*auto send data not used parent data table*/
	} else {
		new_node->parent_con_type = parent_con_type;
	}
	new_node->auto_push_flag = auto_push_flag;
	new_node->first_read= 1;
	new_node->last_stamp= 0;
	new_node->update_fun = update_fun;
	new_node->del_fun = del_fun;
	new_node->filter_fun = filter_fun;
	new_node->format_fun = format_fun;

	if (*list == NULL) {
		*list = new_node;
	} else {
		last_node->next = new_node;
	}
	return PUSH_OK;
}

void dl_node_add_stamp(data_list_t *list,char *svr_info)
{
	data_list_t* cur_node = list;
	char table_stamp[32];
	size_t  stamp;
	
	while (cur_node != NULL) {
		if (cur_node->auto_push_flag) {
			snprintf(table_stamp,sizeof(table_stamp),"%sStamp",cur_node->table_name);
			if(checkIntKey(svr_info,table_stamp, (unsigned int *)&(stamp))) {
				cur_node->last_stamp= stamp;
				cur_node->first_read = 1;
				break;
			}
		}
		cur_node = cur_node->next;
	}
}

 data_list_t *dl_get_node_by_parent_type(data_list_t * list, int type)
 {
	data_list_t *node = list;

	if (list == NULL) {
		return NULL;
	}
	while (node != NULL) {
		if (node->parent_con_type == type) {
			break;
		}
		node = node->next;
	}
	return node;
 }

 data_list_t *dl_get_node_by_type(data_list_t * list, int type)
 {
	data_list_t *node = list;
	
	while (node != NULL) {
		if (node->con_type== type) {
			break;
		}
		node = node->next;
	}
	return node;
 }

data_list_t *dl_get_node(data_list_t * list,int con_type,int *parent_type)
{
	data_list_t *cur_node = list;

	if (list == NULL) {
		return NULL;
	}
	while ((cur_node != NULL)) {
		if (cur_node->con_type == con_type) {
			if (parent_type != NULL) {
				*parent_type = cur_node->parent_con_type;
			}
			return cur_node;
		} 
		cur_node = cur_node->next;
	}
	return NULL;
}
int is_register_cmd(cmd_list_t * cmd_list,char *cmd)
{
	cmd_list_t *node = cmd_list;

	if (cmd == NULL) {
		return FALSE;
	}
	while (node != NULL) {
		if (strncmp(cmd,node->cmd,strlen(cmd)) == 0) {
			return TRUE;
		}
		node = node->next;
	}
	return FALSE;
}

int dl_call_fun(data_list_t *list,char *table_name,int fun_type,char *param)
{
	data_list_t*node= NULL;

	node = dl_get_node_by_name(list, table_name);
	if (node == NULL) {
		return -1;
	}
	switch (fun_type) {
	case DL_DEL:
		if (node->del_fun!= NULL) {
			return node->del_fun(param);
		}
		break;
	case DL_UPDATE:
		if (node->update_fun!= NULL) {
			return node->update_fun(param);
		}
		break;
	default:
		break;
	}
	return -2;
}
cmd_list_t *cl_get_last_node(cmd_list_t* list)
{
	cmd_list_t *node = list;

	if (list == NULL) {
		return NULL;
	}
	while (node->next != NULL) {
		node = node->next;
	}
	return node;
}
int cl_init(cmd_list_t **cmd_list,char* cmd, pushsdk_cmd_f fun)
{
	cmd_list_t *last_node = NULL;
	cmd_list_t *new_node;

	if ((strlen(cmd)<=0)||(strlen(cmd)+1>=MAX_CMD_LEN)) {
		return PUSH_ERROR_PARAMS;
	}
	if (fun == NULL) {
		return PUSH_ERROR_FUN_NULL;
	}
	if (is_register_cmd(*cmd_list,cmd)) {
		return PUSH_ERROR_DATAEXIST;
	}
	if ((new_node=(cmd_list_t *)malloc(sizeof(cmd_list_t))) == NULL) {
		printf("[PUSH SDK]    MALLOC FAILED\n\n");
		return PUSH_ERROR_MALLOC;
	}
	last_node = cl_get_last_node(*cmd_list);
	new_node->next = NULL;
	strcpy(new_node->cmd,cmd);
	new_node->fun = fun;

	if (*cmd_list  == NULL)  {
		*cmd_list = new_node;
	} else {
		last_node->next = new_node;
	}
	return PUSH_OK;
}

