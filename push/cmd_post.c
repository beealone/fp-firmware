#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "cmd.h"
#include "fw_api.h"
#include "fuliye.h"
extern data_list_t *data_list ;	
extern cmd_list_t  *cmd_list ;
extern short int FramNumber;
extern unsigned char DevNumber[11];
extern unsigned char commkey[16];
extern unsigned char pRadom[8];
unsigned int gLastAttRec;
extern unsigned short int gErrorCode;

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
int data_record_deal(void *data, int index, void *param);
 int post_file_to_svr(const char *fileName, const char *url, iclock_options_t* opt);

/*
*Function:Lib_IsUploadFace
*Description:判断是否为上传人脸模板的操作日志，如果是，
*则将stamp设为上一次时间戳,并返回本次时间戳
*Input Prama:param:SrcData 操作日志数据
*				 LastStamp 上次时间戳
*				 CurStamp 本次时间戳
*
*Return:>0 -- 本次时间戳
	    0 --非人脸模板操作日志
*/
unsigned int Lib_IsUploadFace(void *SrcData,size_t *CurStamp)
{
	TOPLog *log=(TOPLog*)SrcData;

	if (SrcData == NULL) {
		return *CurStamp;
	}
	if ((log->OP == OP_ENROLL_FACE) || (log->OP == OP_CHG_FACE)) {
		*CurStamp = (log->time_second) - 1;
		return (log->time_second);
	}
	return *CurStamp;
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
static int svr_is_need_contype(char *dev_flag,int data_type)
{
	int ret = 0;
	
	switch (data_type) {
		case FCT_USER:
			if (svr_is_need_data(dev_flag,PUSH_SVR_CHG_USER) || \
			    svr_is_need_data(dev_flag,PUSH_SVR_EN_USER)) {
				ret = 1;
			 }
		break;
		case FCT_FINGERTMP:
			if (svr_is_need_data(dev_flag,PUSH_SVR_CHG_FP) || \
			    svr_is_need_data(dev_flag,PUSH_SVR_EN_FP)) {
				ret = 1;
			 }
		break;
		case FCT_FACE:
			if (svr_is_need_data(dev_flag,PUSH_SVR_FACE)) {
				ret = 1;
			 }
			break;
		case FCT_USERPIC:
			if (svr_is_need_data(dev_flag,PUSH_SVR_USER_PIC)) {
				ret = 1;
			 }
			break;
		
		default:
			break;
	}
	return ret;
}
int check_options_cfg_setting(iclock_options_t *opt,data_list_t *cur_node)
{
	if ((opt == NULL) || cur_node == NULL) {
		return 1;
	}
	/*针对ContentType=0时，自动检测数据上传，
	在使用GPRS传输时，可以不传输相对数据较大的图片功能*/
	if ((opt->upload_photo == 0) && (opt->is_only_upload_photo == 0)){
		if (opt->con_type == FCT_PHOTOINDEX) {
			cur_node->first_read= 0;
			return 1;
		}
	}
	/*针对ContentType=0时，自动检测数据上传，
	不支持ADMS功能，支持传输照片功能：（针对考勤软件传输照片功能）*/
	if ((opt->upload_photo == 1) && (opt->is_only_upload_photo == 1)) {
		if (opt->con_type != FCT_PHOTOINDEX) {
			cur_node->first_read= 0;
			return 1;
		}
	}
	return 0;
}

void url_init(iclock_options_t*opt, char * url,char *table_name)
{
	if ((strlen(opt->svr_ver) <= 0) && (opt->con_type == FCT_OPLOG)) {
		sprintf(url, "%scdata?SN=%s&table=%s&OpStamp=", opt->root_url, opt->SN, table_name);
        } else if ((strlen(opt->svr_ver) <= 0) && (opt->con_type == FCT_PHOTOINDEX)) {
		sprintf(url, "%sfdata?SN=%s&table=%s&PhotoStamp=", opt->root_url, opt->SN, table_name);
    	 } else if ((strlen(opt->svr_ver) <= 0) && (opt->con_type != FCT_ATTLOG)) {
		sprintf(url, "%scdata?SN=%s&table=%s&OpStamp=", opt->root_url, opt->SN, table_name);
       } else {
		sprintf(url, "%scdata?SN=%s&table=%s&Stamp=", opt->root_url, opt->SN, table_name);
       }
}

void trans_info_init(trans_info_t* info,data_list_t *list,iclock_options_t * opt,char *url,trans_help_t *trans_hep,int mode)
{
	memset(info, 0, sizeof(trans_info_t));
	info->file_path = NULL;
	info->cache = createRamBuffer(opt->buf_size);
	info->starting_time = list->last_stamp;
	info->opt = opt;
	info->list = list;
	url_init(opt,url,info->list->table_name);
	info->url = url;
	info->upload_mode = mode;
	info->trans_help = trans_hep;
}
int record_convert_to_svr(int *data,trans_info_t *info,char *url_rec,size_t *cur_trans_record_time,char  *file_path)
{
	char str[512];
	int len = -1;
	struct tm time;

	if (info->list->param != NULL) {
		snprintf(str,sizeof(str),"%s",info->list->param);
		*cur_trans_record_time = info->cur_trans_log_time;
	} else {
		snprintf(str,sizeof(str),"startStamp=%u\ngTransFlag=%s\n", \
				(unsigned int)info->starting_time,info->opt->svr_trans_flag);
	}
	if ((info->list->filter_fun == NULL) || \
	   (info->list->filter_fun && info->list->filter_fun(data,str) != 1)) {
		return 0;
	}
	if (info->list->format_fun != NULL) {
		len = info->list->format_fun(data,url_rec,cur_trans_record_time,file_path);
	}
	/*data no time stamp,set current time stamp*/
	if ((*cur_trans_record_time == 0)) {
    		GetTime(&time);
		*cur_trans_record_time = OldEncodeTime(&time);
	}
	return (len < 0) ? -1 : len;
}
int all_passive_data_auto_upload(trans_info_t *info,char * rec_con,int rec_con_size,int rec_len)
{
	data_list_t *cur_node = data_list;
	trans_info_t tmpinfo;
	int send_count = -0xff;
	
	memcpy(&tmpinfo, info, sizeof(trans_info_t));
	snprintf(rec_con + rec_len, rec_con_size-rec_len,\
					"gTransFlag=%s", info->opt->svr_trans_flag);
	while (cur_node != NULL) {
		if  (!cur_node->auto_push_flag) {
			tmpinfo.list = cur_node;
			cur_node->first_read = 1;
			tmpinfo.list->param = rec_con;
			tmpinfo.list->con_type = cur_node->con_type;
			send_count = FDB_ForDataFromCur(cur_node->con_type, \
						data_record_deal, TRANS_MAX_RECORD_NO_LIMIT, &tmpinfo);
			if (send_count < 0) {
				break;
			}
		}	
		cur_node=cur_node->next;
	}
	if (send_count != -0xff) {
		info->error = tmpinfo.error;
		info->count = tmpinfo.count;
	}
	return send_count;
}
int  trans_face_spec_deal(trans_info_t *info,int face_index,time_t last_face_stamp)
{
	int cache_left_len;

	if (strncasecmp(info->list->table_name,"FACE",4) == 0) {
		if (face_index == (info->opt->FaceTmpCnt -1)) {
			info->cur_trans_log_time = last_face_stamp;
			cache_left_len = info->opt->buf_size;
		} else if (((face_index % 4) == 0)  && (face_index > 0)) {
			cache_left_len = info->opt->buf_size;
		} else {
			cache_left_len = 4*1024;
		}
	} else {
		cache_left_len = 4*1024;
	}
	return cache_left_len;
}

/*****************************************************************
*	0100350000060303 			head
*	1122334455667788990011		机具编号
*	01							流水数量
*	
	流水
	00000000000000000000		卡应用序列号
	4A308644					卡物理序列号	十进制:1244694084
	20140122175027				打卡时间
	00000001					打卡序号
	
*	00000001					剩余流水数量
*	00000000					MAC校验
*******************************************************************/

 int cache_att_to_svr(iclock_options_t *opt,PPostAttLog attlog)
{
	FILE *iSvrResp = NULL;
	FILE *fStream = NULL;
	unsigned char head[20480];
	unsigned char postbuf[2048];
	int size = 0;
	int len=0;
	int j=0;
	int i=0;
	int fdatt=-1;
	unsigned char checkcode[4];
	unsigned int macsum;
	unsigned char registerlen[3]={0x00,0x00,0x00};
	unsigned int leftcount=0;	//剩余流水数量

	TYMIP_HEAD recv_head;
	TYMIP_HEAD post_head;
	unsigned char attcount;
	unsigned int transinfolen=0;
	unsigned short tmplem=0;
	unsigned int return_len;
	unsigned char data_return[512];
	BOOL checkFN=FALSE;
	int error_code = -1;

	unsigned int writevalue[2];	//写入BsAtt.dat记录内容
		
	memset(postbuf,0,sizeof(postbuf));
	transinfolen=8+11+1+attlog->reclenght+4+4;

	fdatt=open(BS_TRANS_ATT_FILE,O_RDWR|O_NONBLOCK);

	tmplem=htons(transinfolen);
	memcpy(registerlen+1,&tmplem,2);
	incFN();
	ymip_head_format(&post_head,LICENSEVERSION,registerlen,FramNumber,UPLOADATTDATA);
	
	attcount=(0xFF) & ((attlog->reclenght)/25);			//一个字节来存储发送的数量
	if((attlog->maxrec)>(attlog->currec+attcount))
	{
		leftcount=htonl(attlog->maxrec-(attlog->currec+attcount));
	}
	else
	{
		leftcount=0x00000000;
	}
	memcpy(&gLastAttRec, attlog->attbuffer+attlog->reclenght-4, 4);
	printf("\ngLastAttRec=%d\n", ntohl(gLastAttRec));
	memcpy(postbuf,&post_head,sizeof(TYMIP_HEAD));		//head
	memcpy(postbuf+8,DevNumber,11);					//devnumber
	memcpy(postbuf+8+11,&attcount,1);
	memcpy(postbuf+8+12,attlog->attbuffer,attlog->reclenght);	//att
	memcpy(postbuf+8+12+attlog->reclenght,&leftcount,4);	//leftcount
//MAC check:
	printf("[PUSHSDK]	[%s]__Commkey:\n", __FUNCTION__);
	for(i=0; i<16; i++)
		printf("%02x ", commkey[i]);
	printf("\ttranslen=%d\n", transinfolen-4);
	MACCAL_KEY16(commkey, pRadom, 0, postbuf, transinfolen-4, checkcode);
	memcpy(postbuf+transinfolen-4, checkcode, 4);	//MAC
//end	
	/*如果发送数据后，没有收到服务器确认信息，将重发，三次都失败后，认为发送失败*/

	printf("opt->websvr_time_out=%d\n",opt->websvr_time_out);
//	opt->websvr_time_out=10;	//only test
	while(j<opt->reposttime){
//	for (j=0; j<3;  j++) {
		j++;
		iSvrResp = ymip_att_proc(opt->svrip,opt->svrport, (char *)&recv_head, postbuf, NULL, NULL, opt->error_intval, transinfolen);
		if(iSvrResp==NULL)
		{
			printf("[PUSH SDK]    TRANS ATTLOG (%d)TIMES FAILED\n\n",j);
			continue;
		}
		return_len = svr_return_proc(iSvrResp, data_return, opt->websvr_time_out);
		if((return_len+8)  != GetPacketLength(&recv_head))
		{
			gErrorCode = 0x000a;
			fclose(iSvrResp);
			return -1;
		}
		if(return_len<=0)
		{
			printf("[PUSH SDK]    TRANS ATTLOG (%d)TIMES FAILED\n\n",j);
			continue;
		}
		checkFN=IsRightFN(post_head.FrameNum,recv_head.FrameNum);//帧号检验
		error_code = ProcessService(&recv_head, data_return, opt);
		 if ((error_code < 0)/*||(checkFN==FALSE)*/) {
			printf("[PUSH SDK]    TRANS ATTLOG (%d)TIMES FAILED\n\n",j);
		 } else {
		 	memset(attlog->attbuffer,0,1024);		//发送成功之后，重置数据
		 	attlog->currec+=attcount;
			attlog->reclenght=0;
			writevalue[0]=attlog->currec;
			writevalue[1]=attlog->curfp;
			printf("\n[%s]currec=%d,attcount=%d,maxrec=%d\n",__FUNCTION__,attlog->currec,attcount, attlog->maxrec);
			write_file_with_offset(fdatt,0,(char *)&(writevalue),sizeof(writevalue));
			if(attlog->maxrec<=attlog->currec)
			{
				opt->stat.post_interval = 1;
			}
		 	printf("[PUSH SDK]    TRANS ATTLOG  SUCCESS\n\n");
			 break;
		 }
	}

	if (fStream) {
		fclose(fStream);
	}

	if(fdatt>0)
	{
		close(fdatt);
	}

	if (j >= opt->reposttime) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
		return 0;
	} else {
		return 1;
	}
}


BOOL  svr_res_proc(FILE* iSvrResp,trans_info_t*info,char* strHead);
 int cache_data_to_svr(char *url, trans_info_t *info)
{
	FILE *iSvrResp = NULL;
	FILE *fStream = NULL;
	char head[20480];
	int  size = 0;
	int len=0;
	char *buffer = NULL, *tmp=NULL;
	int j;

	head[0]=0;
	if(info->opt->is_encrypt == 1) {
		tmp=(char *)malloc(bufferSize(info->cache)+1024);
		buffer = tmp;
		size=bufferEncrypt((char *)info->cache->buffer, bufferSize(info->cache), \
						buffer+8, bufferSize(info->cache)+1024, info->opt->SN, strlen(info->opt->SN));
		((int*)buffer)[1] = bufferSize(info->cache);
		strcpy(buffer,"LZO1");
		size += 8;
	} else {
		buffer = (char *)info->cache->buffer;
		size = bufferSize(info->cache);
	}

	if( NULL != info->file_path) {
		fStream = fopen(info->file_path,"rb");
		if (fStream != NULL) {
			len = file_get_length(fStream);
		}
		buffer[size]=0;
		size++;
	}
	/*如果发送数据后，没有收到服务器确认信息，将重发，三次都失败后，认为发送失败*/
	for (j=0; j<3;  j++) {
		iSvrResp = http_proc(url,head,buffer,size, fStream, \
							((size+len)>10*1024)?info->opt->websvr_time_out*4:info->opt->websvr_time_out,\
							info->opt->send_buf_size);
		if (svr_res_proc(iSvrResp,info,head)) {
			printf("[PUSH SDK]    UPLOAD %s(%d) INFO SUCESS\n\n",info->list->table_name,info->count);
			if (info->file_path != NULL) {
				printf("[PUSH SDK]    UPLOAD  %s(%s) FILE SUCESS\n\n",info->list->table_name,info->file_path);
			}
			pipe_write_to_parent_cmd(CHILD_CMD_ONLINE);
			break;
		}
		printf("[PUSH SDK]    TRANS  %s (%d)TIMES FAILED\n\n",info->list->table_name,j+1);
		msleep(300*(j+1));
	}

	if (fStream) {
		fclose(fStream);
	}
	if (tmp != NULL) {
		free(tmp);
	}

	if (j > 2) {
		pipe_write_to_parent_cmd(CHILD_CMD_OFFLINE);
		return 0;
	} else {
		return 1;
	}
}

static int post_buf_to_svr(int max_left_len,trans_info_t*info,char *file)
{
	int sendcount=0;
	char url[1024];
	char *table_name = info->list->table_name;
	int ret = 0;
	static int fp_count = 0;

	/*服务器查询时上传数据，时间戳设为为0*/
	if (info->upload_mode == UPLOADE_MODE_NOR) {
		snprintf(url, sizeof(url),"%s%u", info->url, (size_t)(info->cur_trans_log_time));
	} else {
		sprintf(url, "%s%u", info->url,(size_t)(info->starting_time));
	}
	if (strncasecmp(table_name, "fingertmp", strlen("fingertmp")+1) == 0) {
		fp_count++;
	}
	if (strlen(file) > 0) {
		info->file_path = file;
		if (cache_data_to_svr(url, info)) {
			info->file_path = NULL;
			ret = 1;
		} else {
			ret = -2;
		}
	} else if ((resOfBuffer(info->cache) < max_left_len)  || \
		(info->count >= info->opt->trans_max_rec)|| \
		(fp_count > 2)||\
		(strncasecmp(table_name, "USERPIC", strlen("USERPIC")) == 0)) {
		if (cache_data_to_svr(url, info)) {
			ret = 1;
			if ( info->opt->trans_max_rec < 140) {
				info->opt->trans_max_rec += 10;
			}
		} else {
			ret = -1;
			if ( info->opt->trans_max_rec > 30) {
				info->opt->trans_max_rec -= 30;
			}
		}
	} 
	
	if (ret == 1) {
		if ((info->list->auto_push_flag) && \
		    (info->upload_mode == UPLOADE_MODE_NOR))  {
			info->list->last_stamp = info->cur_trans_log_time;
		} 
		fp_count = 0;
		sendcount = info->count;
		info->count = 0;
		clearBuffer(info->cache);	
		memset(info->cache->buffer,0,info->cache->bufferSize);
		return sendcount;
	} else if (ret < 0){
		info->error = -1;
	}
	return ret;
}
int data_record_deal(void *data, int index, void *param)
{
	char file_path[256];
	size_t cur_trans_record_time = 0;
	char url_rec[64*1024];
	int len = -1;
	trans_info_t *info=(trans_info_t *)param;
	static time_t  last_face_stamp = 0;
	int send_con_max_size = 0;
	
	memset(url_rec,0,sizeof(url_rec));
	memset(file_path,0,sizeof(file_path));
	info->file_off =  index;
	len = record_convert_to_svr(data, info,url_rec,&cur_trans_record_time,file_path);
	if (len <= 0) {
		return len;
	}
	if (info->list->con_type == FCT_OPLOG) {
		last_face_stamp = Lib_IsUploadFace(data,&cur_trans_record_time);
	}
	info->cur_trans_log_time = cur_trans_record_time;
	writeBuffer(info->cache, url_rec, len);
	info->count++;
	/*check all not auto_push_flag table to send*/
	if (info->list->con_type==FCT_OPLOG) {
		all_passive_data_auto_upload(info,url_rec,sizeof(url_rec),len);
	}
	send_con_max_size = trans_face_spec_deal(info,index,last_face_stamp);
	if (info->count > 0) {
		return post_buf_to_svr(send_con_max_size,info,file_path);
	} else {
		return 0;
	}
}
int data_record_help(void *data, int index, void *param)
{
	char file_path[256];
	char url_rec[64*1024];
	int len = -1;
	trans_info_t *info=(trans_info_t *)param;
	static time_t  last_face_stamp = 0;
	int send_con_max_size = 0;
	
	memset(url_rec,0,sizeof(url_rec));
	memset(file_path,0,sizeof(file_path));
	if (info->trans_help == NULL) {
		return -1;
	}
	if ((info->trans_help->format_fun == NULL) || (info->trans_help->param== NULL)) {
		return -1;
	}
	if ((info->trans_help->filter_fun == NULL) || \
	   (info->trans_help->filter_fun && info->trans_help->filter_fun(data,info->trans_help->param) != 1)) {
		return 0;
	}
	
	len = info->trans_help->format_fun(data,url_rec,&(info->cur_trans_log_time),file_path);
	if (len <= 0) {
		return len;
	}
	info->cur_trans_log_time = info->starting_time;
	writeBuffer(info->cache, url_rec, len);
	info->count++;
	/*check all not auto_push_flag table to send*/
	if (info->trans_help->con_type == FCT_OPLOG) {
		all_passive_data_auto_upload(info,url_rec,sizeof(url_rec),len);
	}
	send_con_max_size = trans_face_spec_deal(info,index,last_face_stamp);
	if (info->count > 0) {
		return post_buf_to_svr(send_con_max_size,info,file_path);
	} else {
		return 0;
	}
}
int upload_left_data(trans_info_t *info,data_list_t *parent_node)
{
	char url[1024];
	int ret = 0;

	if (info->upload_mode == UPLOADE_MODE_NOR) {
		snprintf(url,sizeof(url), "%s%u",info->url, (size_t)(info->cur_trans_log_time));
	} else {
		sprintf(url, "%s%u", info->url,(size_t)(info->starting_time));
	}
	if (cache_data_to_svr(url, info)) {
		if (info->opt->trans_max_rec < 150) {
			info->opt->trans_max_rec += 10;
		}
		ret = info->count;
		info->count = 0;
	} else {
		info->error = -1;
		ret = -1;
		if (info->opt->trans_max_rec > 30) {
			info->opt->trans_max_rec -= 30;
		} 
	}

	if ((ret > 0) && (info->upload_mode == UPLOADE_MODE_NOR) ) {
		info->list->last_stamp = info->cur_trans_log_time;
		if ((parent_node != NULL) && (parent_node->last_stamp == 0)) {
			parent_node->last_stamp = info->cur_trans_log_time;
		}
	}
	return ret;
}


int data_trans(iclock_options_t *opt,ForARecFun fun,trans_help_t *trans_hep)
{
	char url[1024];
	trans_info_t info;
	data_list_t *cur_node = NULL;
	data_list_t *parent_node = NULL;
	int parent_type = 0;
	int ret = 0;

	cur_node = dl_get_node(data_list,opt->con_type, &parent_type);
	if (cur_node == NULL) {
		return 0;
	}
	if (parent_type != 0) {
		parent_node = dl_get_node(data_list,parent_type,NULL);
	}
	if (check_options_cfg_setting(opt,cur_node)) {
		printf("[PUSHSDK]    OPTIONS SET NOT SEND:%d DATA\n\n",opt->con_type);
		return 1;
	}
	trans_info_init(&info,cur_node,opt,url,trans_hep,UPLOADE_MODE_NOR);
	if ((info.starting_time == 0) || (cur_node->first_read) || (trans_hep != NULL)) {
		ret  = FDB_ForAllData(opt->con_type, fun, TRANS_MAX_RECORD_NO_LIMIT, &info);
	} else {
		ret = FDB_ForDataFromCur(opt->con_type, fun, TRANS_MAX_RECORD_NO_LIMIT, &info);
	}
	cur_node->first_read = 0;
	/*all data alread trans by stamp, sync the file offset*/
	if ((ret == 0) &&  (info.error == 0) && (info.count == 0) && (info.file_off > 0)) {
		if (opt->main_fun->update_file_offset_fun != NULL) {
			opt->main_fun->update_file_offset_fun(opt->con_type,info.file_off);
		}	
	} else if ((ret >= 0) && (info.error == 0) && (info.count > 0)) {
		ret = upload_left_data(&info,parent_node);
	}
	/*trans data success,sync the active data type file offset*/
	if ((ret > 0) && (info.error == 0)) {
		if (opt->main_fun->update_file_offset_fun != NULL) {
			opt->main_fun->update_file_offset_fun(opt->con_type,info.file_off);
		}
	}
	
	if (info.error) {
		cur_node->first_read = 1;
	} 
	freeBuffer(info.cache);
	opt->con_type=0;	
	return ret;
}
void resume_broken_transfer_init(void)  
{
	int fd;
	int value;

	/*if BS_TRANS_HELP_FILE not exit,then create and init it*/
	fd = open(BS_TRANS_HELP_FILE,O_RDWR|O_EXCL|O_CREAT,0666);
	
	if ((fd >= 0 && errno == 0)) {
		value = -1;
		lseek(fd,TRANS_HELP_FILE_USER,SEEK_SET);
		write(fd,&value,sizeof(int));

		lseek(fd,TRANS_HELP_FILE_FP,SEEK_SET);
		write(fd,&value,sizeof(int));
		
		fsync(fd);
		close(fd);
		 fd = -1;
	}
	if (fd >= 0) {
		close(fd);	
	}
}

int create_upload_file(iclock_options_t *opt,int con_type)
{
	int result = 0;
	unsigned int value[2];
	int file_fp = -1;

	if (!svr_is_need_contype(opt->svr_trans_flag,con_type)) {
		return result;
	}
	switch (con_type)
	{
		case FCT_USER:
			file_fp = open(BS_TRANS_HELP_FILE,O_RDWR|O_CREAT|O_NONBLOCK);
			if (opt->main_fun->create_data_record_fun != NULL) {
				result = opt->main_fun->create_data_record_fun(BS_TRANS_USER_FILE,con_type);
			}
			value[0] = (result == 1) ? 0 : -1;
			value[1] = time(NULL);
			write_file_with_offset(file_fp,TRANS_HELP_FILE_USER,(char *)value,sizeof(value));
			break;
		default:
			break;
	}
	
	if (file_fp >= 0) {
		close(file_fp);
	}
	
	return result;
}

int  get_file_info(int con_type,int mode,char *file_name)
{
	int file_offset = -1;

	if (file_name == NULL) {
		return file_offset;
	}
	if (mode == TRANS_TYPE_SVR) {
		if (con_type == FCT_USER) {
			strcpy(file_name,BS_TRANS_USER_FILE);
			file_offset = TRANS_HELP_FILE_USER;
		} 
	} else if (mode == TRANS_TYPE_U) {
		if (con_type == FCT_USER) {
			strcpy(file_name,U_USER_DATA_FILE);
			file_offset = TRANS_U_FILE_USER;
		}
	}
	
	return file_offset;
}
static void thread_free_cpu(iclock_options_t *opt)
{
	int time_out = 1;
	if ((opt->main_fun->dev_is_busy_fun != NULL)) {
		time_out = opt->main_fun->dev_is_busy_fun();
	}
	if ((opt->dev_support_data[DEV_SUPPOPT_FP] == '1' ) \
		&& (time_out >= 2) && (opt->con_type == FCT_FINGERTMP)) {
		sleep(1);
	}
	time_out = time_out*5;
	msleep(time_out);
}
int trans_user_all_data(char *usr_buf,trans_info_t *info,char *str_temp,int contype)
{
	int i;
	int ret = 0;
	char data[30*1024];
	data_list_t * list;
	int read_data_len =  0;

	
	if (info->opt->main_fun->upload_read_data_fun == NULL) {
		return -1;	
	}
	list = dl_get_node_by_type(data_list,contype);
	if ((list == NULL) || \
		!svr_is_need_contype(info->opt->svr_trans_flag,contype)) {
		return 0;
	}
	list->param = str_temp;
	info->list  = list;
	info->opt->con_type = contype;
	
	thread_free_cpu(info->opt);
	if (info->opt->con_type == FCT_USER ) {
		ret = data_record_deal(usr_buf, 0, info);
	} else if (info->opt->con_type == FCT_FINGERTMP) {
		for (i=0; i<10; i++) {
			read_data_len = info->opt->main_fun->upload_read_data_fun(info->opt->con_type,usr_buf,i,data);
			if (read_data_len <= 0) {
				break;
			}
			ret = data_record_deal(data, 0, info);
			if (ret < 0 || info->error != 0) {
				return ret;
			}
		}
		
	} else if (info->opt->con_type == FCT_FACE) {
		for (i=0; i<info->opt->FaceTmpCnt; i++) {
			read_data_len = info->opt->main_fun->upload_read_data_fun(info->opt->con_type,usr_buf,i,data);
			if (read_data_len <= 0) {
				break;
			}
			ret = data_record_deal(data, i, info);
			if (ret < 0 || info->error != 0) {
				return ret;
			}
		}
	} else if (info->opt->con_type == FCT_USERPIC) {
		read_data_len = info->opt->main_fun->upload_read_data_fun(info->opt->con_type,usr_buf,0,data);
		if (read_data_len > 0) {
			ret = data_record_deal(data, 0, info);
		}
	}
	thread_free_cpu(info->opt);

	return  ret;
}
BOOL  trans_upload_file(iclock_options_t *opt,data_list_t*CurrentNode,int mode)
{
	char url[1024];
	char buf[1024];
	char str_temp[512];
	int c = -1;
	int file_index;
	trans_info_t info;
	int fd;
	char file_name[128];
	int file_offset;
	int read_data_len = 0;
	int trans_help_fp;
	BOOL  ret = TRUE;

	if (CurrentNode == NULL || opt == NULL) {
		return TRUE;
	}
	chmod(BS_TRANS_HELP_FILE,0666);
	trans_help_fp = open(BS_TRANS_HELP_FILE,O_RDWR|O_NONBLOCK);
	file_offset = get_file_info(CurrentNode->con_type,mode,file_name);
	if (trans_help_fp < 0) {
		return TRUE;
	}
	if (file_offset < 0) {
		close(trans_help_fp);
		return TRUE;
	}
	if ((!read_file_with_offset(trans_help_fp,file_offset,(char *)&file_index,sizeof(file_index)))
		|| (file_index < 0)) {	
		close(trans_help_fp);
		return TRUE;
	}
	memset(&info,0,sizeof(info));
	sprintf(str_temp,"gTransFlag=%s",opt->svr_trans_flag);
	info.cache = createRamBuffer(opt->buf_size);
	info.opt = opt;
	info.list = CurrentNode;
	read_file_with_offset(trans_help_fp, file_offset+sizeof(file_index),\
						(char *)&info.cur_trans_log_time,sizeof(info.cur_trans_log_time));
       info.starting_time = (size_t)OldEncodeTime(localtime(&info.cur_trans_log_time));
	opt->con_type = CurrentNode->con_type;
	url_init(opt,url,"OPERLOG");
	info.url = url;
	info.upload_mode = UPLOADE_MODE_QUERY;
	CurrentNode->param = str_temp;
	fd = open(file_name,O_RDWR|O_NONBLOCK);
	
	if ((fd >= 0) && (file_index >= 0) && (opt->main_fun->upload_usr_read_fun != NULL)) {
		if (svr_is_need_contype(opt->svr_trans_flag,CurrentNode->con_type)) {
			while ((read_data_len = opt->main_fun->upload_usr_read_fun(\
										CurrentNode->con_type,buf,file_index,fd)) > 0) {
				c = trans_user_all_data(buf,&info,str_temp,FCT_USER);
				if ( info.error != 0 || (c < 0)) {
					break;
				} 
				c = trans_user_all_data(buf,&info,str_temp,FCT_USERPIC);
				if ( info.error != 0 || (c < 0)) {
					break;
				} 
				c = trans_user_all_data(buf,&info,str_temp,FCT_FACE);
				if ( info.error != 0 || (c < 0)) {
					break;
				} 
				c = trans_user_all_data(buf,&info,str_temp,FCT_FINGERTMP);
				if ( info.error != 0 || (c < 0)) {
					break;
				} 
				if (( info.error == 0) && (info.count == 0) && (c > 0)) {
					write_file_with_offset(trans_help_fp,file_offset,(char *)&file_index,sizeof(int));
				}
				file_index += read_data_len;
			}
			if (( info.error == 0) && (info.count > 0) && (c >= 0)) {
				snprintf(info.url+strlen(info.url),sizeof(url)-strlen(info.url), "%u",(unsigned int)info.starting_time);
				if (cache_data_to_svr(url, &info)) {
					write_file_with_offset(trans_help_fp,file_offset,(char *)&file_index,sizeof(int));	
				}	
			}
		}
		if (read_data_len <  1) {
			file_index  = -1;
			write_file_with_offset(trans_help_fp,file_offset,(char *)&file_index,sizeof(int));	
			if (fd > 0) {
				close(fd);
			}
			fd = open(file_name, O_RDWR|O_CREAT|O_TRUNC|O_SYNC);
		}
		ret =  (read_data_len < 1);
	} else {
		ret = TRUE;
	}
	freeBuffer(info.cache);
	if (fd >= 0) {
		close(fd);
	}
	if (trans_help_fp >= 0) {
		close(trans_help_fp);
	}
	
	return ret;
}
BOOL  check_left_data_to_upload(iclock_options_t*opt,int mode)
{
	data_list_t*node = NULL;
	int old_con_type;

	old_con_type = opt->con_type;
	node = dl_get_node_by_type(data_list,FCT_USER);
	if ((node != NULL) && (!trans_upload_file(opt, node,mode))) {
		return FALSE;
	}
	
	opt->con_type = old_con_type;
	return TRUE;
}


int active_data_upload_immediately(iclock_options_t *opt)
{
	int  already_send = 0;
	int con_type;
	int ret = 0;
	
	data_list_t *cur_node = data_list;

	con_type = opt->con_type;
	while ((cur_node != NULL)) {
		if ((cur_node->last_stamp > 0) && \
		  	(cur_node->auto_push_flag) && \
		  	(cur_node->first_read == 1)) {
			opt->con_type = cur_node->con_type;
			ret = data_trans(opt,data_record_deal,NULL);
			if (con_type == opt->con_type) {
				already_send = 1;
			}
		} else if ((con_type == cur_node->con_type) && \
				(!cur_node->auto_push_flag)) {
			already_send = 1;
		}
		cur_node = cur_node->next;
	}
	if (already_send != 1) {	
		opt->con_type = con_type;
		ret = data_trans(opt,data_record_deal,NULL);
	}
	return ret;
}
int active_data_upload_interval(iclock_options_t *opt)
{
	int ret = 0;
	
	data_list_t *cur_node = data_list;

	while ((cur_node != NULL)) {
		if ((cur_node->last_stamp > 0) && \
		  	(cur_node->auto_push_flag)) {
			opt->con_type = cur_node->con_type;
			ret = data_trans(opt,data_record_deal,NULL);
		} 
		cur_node = cur_node->next;
	}
	return ret;
}
int passive_data_upload(int  type,iclock_options_t *opt)
{
	data_list_t *child_node = data_list;
	int have_child = 0;
	
	while  ((child_node = dl_get_node_by_parent_type(child_node,type)) != NULL) {
		if (child_node->con_type == FCT_USER) {
			if (create_upload_file(opt,child_node->con_type) > 0) {
				trans_upload_file(opt, child_node,TRANS_TYPE_SVR);
			}
			have_child = 1;
			break;	
		}
		child_node =  child_node->next;
	}
	return have_child;
}
int all_data_upload(iclock_options_t *opt)
{
	int ret = 0;
	struct tm time;
	
	data_list_t *cur_node = data_list;

	while ((cur_node != NULL)) {
		if ((cur_node->auto_push_flag) && (cur_node->last_stamp == 0))   {
			if (!passive_data_upload(cur_node->con_type,opt)){
				opt->con_type = cur_node->con_type;
				ret =  data_trans(opt,data_record_deal,NULL);
			}
			/*for no data,set current stamp to current time*/
			if (cur_node->last_stamp==0) {
				GetTime(&time);
				cur_node->last_stamp= OldEncodeTime(&time)-60;
			}
		} 
		cur_node = cur_node->next;
	}
	return ret;
}
int cmd_post(iclock_options_t *opt)
{
	int ret=0;

	if (opt->con_type > 0)	{
		ret =  active_data_upload_immediately(opt);
	} else {
		/*if the last stamp value is 0*/
		ret = all_data_upload(opt);
		if (ret >= 0) {
			ret = active_data_upload_interval(opt);
		}
		opt->con_type = 0;
	}
	return ret;
}
int  post_data_by_timing(iclock_options_t *opt)
{
	time_t t;
	static char oldltm[16];
	char ltm[16];
	int ret = 0;
	
	time(&t);
	strftime(ltm,6,"%H:%M",localtime(&t));
	if(strstr(opt->post_timing,ltm) != NULL) {
		if (memcmp(ltm, oldltm, 6) != 0) {
			opt->con_type = 0;
			ret = cmd_post(opt);
			memcpy(oldltm,ltm, 6);
			opt->stat.post_interval = 60*opt->trans_intval;
			opt->con_type  = 0;
		}
	} else {
		oldltm[0]=0;
	}
	return ret;
}

int post_data_by_interval(iclock_options_t *opt)
{
	static int power_on = 1;
	static int con_type = 0;
	int ret = 0;

	if (opt->con_type == FCT_OPLOG && (power_on == 1)) {
		power_on = 0;
		con_type = opt->con_type;
		opt->con_type = 0;
		printf("[PUSH SDK]    upload_photo:%d\t,is_only_upload_photo=%d\n",\
			opt->upload_photo,opt->is_only_upload_photo);
	} else if (opt->con_type == 0 && con_type != 0) {
		opt->con_type = FCT_OPLOG;
		con_type = 0;
	}
	printf("[PUSH SDK]    HTTP POST CONTENT:%d\n",opt->con_type);
	ret  = cmd_post(opt);
	opt->con_type = 0;
	
	if (con_type > 0) {
		opt->stat.post_interval = 0;
	} else {
		opt->stat.post_interval = 60*opt->trans_intval;
	}
	clear_bit(&opt->stat.running_flag,RUN_STAT_CMD);
	return ret;
}

BOOL  svr_res_proc(FILE* iSvrResp,trans_info_t *info,char* strHead)
{
	if (NULL == iSvrResp) {
		return FALSE;
	} else {
		char strchar[10*1024] = {0};
		int i=0;

		while(!feof(iSvrResp)) {
			if ((i = fread(strchar,sizeof(char),sizeof(strchar),iSvrResp)) > 0) {
				if (strncmp(strchar,"OK",2) == 0) {
					break;
				} else if (strncmp(strchar,"UNKNOWN DEVICE",14)==0) {
					break;
				}
			}
		}
		strchar[i] = 0;
		fclose(iSvrResp);
		if (strncmp(strHead+8, " 200 ", 5)==0) {
			if ((strncmp(strchar,"OK",2)==0)||(strstr(strchar,"OK")!=NULL)) {
				return TRUE;
			} else if ((strncmp(strchar,"UNKNOWN DEVICE",14))==0)	 {
				printf("[PUSH SDK]    RESTART PUSH SDK,SVR RETURN=%s\n\n",strchar);
				clear_bit(&info->opt->stat.running_flag,RUN_STAT_CONNECT);
				return FALSE;
			}
		}
		/*for old att 2008*/
		if ((strncmp(info->opt->svr_trans_flag,"0000000000",10)==0) \
		   &&(strstr(strHead,"OK")!=NULL)) {
			return TRUE;
		}
		printf("[PUSH SDK]    UPLOAD %s FAILED\nSVR SAY:%s\n\n",info->list->table_name,strchar);
	}
	return FALSE;
}

void real_time_event_proc(iclock_options_t *opt)
{
    char furl[200];
    time_t ttTimeOut;
    
    if (opt->push_lock_fun_on[0] != 1) {
		return;
    }
    snprintf(furl,sizeof(furl),"%sevent?SN=%s",  opt->root_url, opt->SN);
    if( rename("/tmp/RTData.txt", "/tmp/RTData.txt.tmp") != 0) {
		return;	
    }
    ttTimeOut =  opt->websvr_time_out;
    opt->websvr_time_out= 2; 
    post_file_to_svr("/tmp/RTData.txt.tmp", furl, opt);
    opt->websvr_time_out= ttTimeOut;
}

