#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "3des.h"
#include "pushsdk.h"
#include "fw_api.h"
#include "fuliye.h"
#include "cmd.h"
#include "pipe.h"
#include "data_list.h"

//extern iclock_options_t *opt;
extern iclock_options_t pushsdk_options;
extern data_list_t *data_list;
extern unsigned int gLastAttRec;
//unsigned char buffer[MAX_DATA_LEGHT];
extern unsigned int ListVer;
extern time_t  dev_time;

unsigned short int FramNumber=0x0000;
unsigned short int gErrorCode;
unsigned int gWarningCode;
U8 gWarningLevel;

//开通请求的结果
unsigned char svrtime[7];
unsigned char commkey[16] = {0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};
unsigned char pRadom[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char svrIP[4];
short int svrPort;

unsigned int answer_transinfo_format(PTransinfo transinfo, PYMIP_HEAD head, char *data, unsigned int datalen, int checkcode);
/***************************
将当前时间按字节存入以下格式
unsigned char DevTime[7]={0x20,0x14,0x01,0x13,0x10,0x59,0x53}
****************************/
int time2byte(unsigned char devtime[])
{
	time_t gTime;
	struct tm *gDate;
	int c=0;

	gTime=time(0);
	gDate=localtime(&gTime);
	c=(gDate->tm_year+1900)/100;
	devtime[0]=(c/10<<4)|(c%10);
	c=(gDate->tm_year+1900)%100;
	devtime[1]=(c/10<<4)|(c%10);
	c=gDate->tm_mon+1;
	devtime[2]=(c/10<<4)|(c%10);
	c=gDate->tm_mday;
	devtime[3]=(c/10<<4)|(c%10);
	c=gDate->tm_hour;
	devtime[4]=(c/10<<4)|(c%10);
	c=gDate->tm_min;
	devtime[5]=(c/10<<4)|(c%10);
	c=gDate->tm_sec;
	devtime[6]=(c/10<<4)|(c%10);
	return 0;
}

/*********************************
判断请求的帧号和返回的帧号是否一致
**********************************/
BOOL IsRightFN(short int reqFN,short int retFN)
{
	return (reqFN==retFN)?TRUE:FALSE;
}



/*********************************
帧号处理
**********************************/
int incFN(void)
{
	if(FramNumber>=0xFF00)
		FramNumber=0x0001;
	FramNumber+=1;
	return 0;
}


/************************
BCD字串转换成10进制型字串
in bcd字符串，length bcd码位数
************************/
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
unsigned int HextoDec(const unsigned char *hex, int length) 
{ 
    int i; 
    unsigned int dec = 0;
    for(i=0; i<length; i++) 
    { 
        dec += (unsigned int)(hex[i])<<(8*(length-1-i)); 
                                                         
    }
    return dec; 
}


/*****************
判断是否是请求包
是  返回TRUE
否	返回FALSE
******************/
int IsRequest(PYMIP_HEAD head)
{
	if(head == NULL)
	{
		return FALSE;
	}
	return ((head->ServiceCode)&=(1<<15)==0x0000? TRUE:FALSE);
}

/*****************
判断是否是应答包
是  返回TRUE
否	返回FALSE
******************/
int IsReturn(PYMIP_HEAD head)
{
	if(head == NULL)
	{
		return FALSE;
	}
	return ((head->ServiceCode)&=(1<<15)==0x8000? TRUE:FALSE);
}

int CheckServerTime(char *time)
{
	unsigned char *str1=NULL;
	char a[5],b[5],c[5],d[5],e[5],f[5];
	str1 = bcd2str(time, 7);
	memset(a, 0, 5);
	memset(b, 0, 5);
	memset(c, 0, 5);
	memset(d, 0, 5);
	memset(e, 0, 5);
	memset(f, 0, 5);
	memcpy(a, str1, 4);
	memcpy(b, str1+4, 2);
	memcpy(c, str1+6, 2);
	memcpy(d, str1+8, 2);
	memcpy(e, str1+10, 2);
	memcpy(f,  str1+12, 2);
	printf("[%s]%s-%s-%s_%s:%s:%s\n", __FUNCTION__, a, b, c, d, e, f);
	if(atoi(b)>12 ||atoi(c)>31 || atoi(d)>24 || atoi(e)>=60 ||atoi(f)>=60)
	{
		return -1;
	}
	return 0;
}
/*****************************
更新机具时间
服务器下发时间格式 YYYYMMDDHHMMSS
					7字节
******************************/
int syncdevtime(time_t before, char *servertime, iclock_options_t *opt)
{	
	time_t cNow2;
	time_t httpNow;
	char buf[64];
	unsigned char *str=NULL;
	struct tm  tms, *tms1;
	if(NULL != servertime)
	{	
		str = bcd2str(servertime, 7);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%-.4s %-.2s %-.2s %-.2s %-.2s %-.2s\n", str, str+4, str+6,str+8, str+10, str+12, str+14);
		strptime(buf,"%Y %m %d %H %M %S",&tms);
		tms1 = &tms;
		printf("%04d-%02d-%02d %02d:%02d:%02d\n", tms1->tm_year+1900, tms1->tm_mon+1, tms1->tm_mday, tms1->tm_hour, tms1->tm_min, tms1->tm_sec);
		httpNow = mktime(&tms);
		printf("opt->is_enter_day_light =%d\n", opt->is_enter_day_light);

		cNow2 = time(NULL);
		if (abs(cNow2-before) < 5) {
			if (abs(httpNow - ((unsigned long)cNow2 + (unsigned long)before) / 2) > 10) {
				//memcpy(&tms, localtime(&httpNow),sizeof(struct tm));
				SetTime(&tms);
				printf("[PUSH SDK]    SYNC TIME WITH SERVER\n");
			}
		}
		return 1;
	}
	return 0;	
}

int AnswerToServer(iclock_options_t *opt, PTransinfo post, int buf_size)
{	
	FILE *svr_ans = NULL;
	int j = 0;
	while(j <= opt->reposttime){
		j++;
		svr_ans = ymip_proc(opt->svrip,opt->svrport, NULL, (char *)post, NULL, NULL, opt->error_intval, buf_size);
		if (NULL != svr_ans) {
			fclose(svr_ans);
			return 0;
		}
		printf("[%s]Trans Error (%d)\n", __FUNCTION__, j);
	}
	if(j >= opt->reposttime)
	{
		printf("[%s] opt->error_intval = %d\n", __FUNCTION__, opt->error_intval);
		return -1;
	}		
}

unsigned short int GetPacketLength(PYMIP_HEAD head)
{
	if(NULL == head)
	{
		return -1;
	}
	unsigned short len;
	memcpy(&len, (head->Lenght)+1, 2);
	return ntohs(len);
}

int ChectMAC(PYMIP_HEAD head, char *buffer)
{
	if(NULL == head  || NULL == buffer)
	{
		return -1;
	}
	unsigned char data[2048];
	unsigned int svr_mac, cur_mac;
	int i;
	unsigned short length;
	
	length = GetPacketLength(head);
	memset(data, 0, 2048);
	memcpy(data, (char *)head, 8);
	memcpy(data+8, buffer, length -12);
	memcpy(&svr_mac, buffer+length -12, 4);
	
	MACCAL_KEY16(commkey, pRadom, 0, data, length -4, &cur_mac);
	if(svr_mac == cur_mac)
	{
		printf("MAC check success\n");
		return 0;
	}
	else
		printf("MAC check failed, svr_mac=%u, cur_mac=%u\n", svr_mac, cur_mac);
	return -1;
}

int procregistret(PYMIP_HEAD head, char *data, iclock_options_t *opt)
{	
	TRegisterRet tmp;
	if(NULL == data ||NULL == head)
	{
		return -2;
	}
	memset(&tmp,0,sizeof(TRegisterRet));
	memcpy(&tmp,data,sizeof(TRegisterRet));

	if(0 != memcmp(tmp.DevNum, DevNumber, 11))
	{
		gErrorCode = 0x0008;
		printf("[%s] DevNumber error\n", __FUNCTION__);
		return -8;
	}
	if(tmp.Result==0x01)
	{
		//注册成功,同步开通密钥
		memset(commkey, 0, 16);
		memcpy(commkey, tmp.OpenKey, 16);
		if(0)
		{	
			int i=0;
			printf("\n\n");
			for(i=0;i<16;i++)
			{
				printf(" 0x%02x ", commkey[i]);
			}
			printf("\n\n");
		}
		printf("[PUSHSDK]	Register device success..\n");
		return 0;
	}
	else
	{	
		printf("[PUSH SDK]	Register device failed..Result=0x%02x\n", tmp.Result);
		return -2;
	}
}

int procheartbeat(PYMIP_HEAD head, char *param, iclock_options_t *opt)
{	
	char time[7];
	THeartbeatRet tmp;
	if(NULL == param ||NULL == head)
	{
		return -2;
	}
	else
	{
		memset(&tmp,0,sizeof(THeartbeatRet));
		memcpy(&tmp,param,sizeof(THeartbeatRet));
		if(0 != memcmp(tmp.DevNum, DevNumber, 11))
		{
			gErrorCode = 0x0008;
			return -8;
		}
		memcpy(time, tmp.ServerTime, 7);
		if(0 == CheckServerTime(time))
		{	
			syncdevtime(dev_time, time, opt);
		}
		else
		{
			gErrorCode = 0x0004;
			return -4;
		}
		printf("[PUSH SDK]	Heartbeat  success..\n");
	}
	return 0;
}

int proopendevret(PYMIP_HEAD head, char *data, iclock_options_t *opt)
{	
	int i=0;
	TOpenDevRet tmp;
	unsigned char newkey[16];
	unsigned char buffer[33];
	if(NULL == data ||NULL == head)
	{
		return -2;
	}
	memset(newkey, 0, 16);
	memset(buffer, 0, 33);
	memset(&tmp,0,sizeof(TOpenDevRet));
	memcpy(&tmp,data,sizeof(TOpenDevRet));
	if(0 != memcmp(tmp.DevNum, DevNumber, 11))
	{
		gErrorCode = 0x0008;
		printf("[%s] DevNumber error\n", __FUNCTION__);
		return -8;
	}
		
	//处理设备开通返回数据
	memcpy(svrtime,tmp.ServerTime,7);
	memcpy(newkey,tmp.CommKey,16);
	memcpy(svrIP,tmp.ServerIP,4);
	svrPort=ntohs(tmp.ServerPort);
	ThreeDES_DAtA16(commkey, tmp.CommKey, newkey, DECRYPT);

	memset(commkey, 0, 16);
	memcpy(commkey, newkey, 16);
	memcpy(opt->push_comm_key, commkey, 16);
	for(i=0; i<16; i++)
	{
		sprintf(buffer+i*2, "%02x", commkey[i]);
	}
	cmd_save_options(opt, "pushcommkey", buffer, 0);
	printf("[PUSH SDK]	Opendev  success...\n");
	return 0;
}


int prouploadatt(PYMIP_HEAD head, char *param)
{
	TUploadAttRet tmp;
	if(NULL == param ||NULL == head)
	{
		return -2;
	}
	else
	{
		memset(&tmp,0,sizeof(TUploadAttRet));
		memcpy(&tmp,param,sizeof(TUploadAttRet));
		if(0 != memcmp(tmp.DevNum, DevNumber, 11))
		{
			gErrorCode = 0x0008;
			printf("[%s] DevNumber error\n", __FUNCTION__);
			return -8;
		}
		if(gLastAttRec == tmp.LastAttSerNum)
		{
			printf("[PUSHSDK]	upload attlog return  success..\n");
		}
		else
		{	
			printf("[PUSHSDK]	upload attlog error: ");
			printf("tmp.LastAttSerNum=%u attcount=%d\n", ntohl(tmp.LastAttSerNum), ntohl(gLastAttRec));
			return -2;
		}
	}
	return 0;
}

int updatecommkey(PYMIP_HEAD head, char *param, iclock_options_t *opt)
{	
	TUpdateCommKey tmp;
	TNomalAns ans;
	TTransinfo post;
	unsigned int len;
	unsigned char newkey[16];
	char buffer[36];
	int i;
	memset(buffer, 0, 36);
	memset(newkey, 0, 16);
	memset(&ans, 0, sizeof(TNomalAns));
	memset(&post, 0, sizeof(TTransinfo));
	memset(&tmp, 0, sizeof(TUpdateCommKey));	
	if(NULL == param ||NULL == head)
	{
		 return -2;
	}
	else
	{
		memcpy(&tmp, param, sizeof(TUpdateCommKey));
		if(0 != memcmp(tmp.DevNum, DevNumber, 11))
		{
			gErrorCode = 0x0008;
			printf("[%s] DevNumber error\n", __FUNCTION__);
			return -8;
		}
		ThreeDES_DAtA16(commkey, tmp.MainCommKey, newkey, DECRYPT);
		memset(commkey, 0, 16);
		memcpy(commkey, newkey, 16);
		memcpy(opt->push_comm_key, commkey, 16);
		for(i=0; i<16; i++)
		{
			sprintf(buffer+i*2, "%02x", commkey[i]);
		}
		cmd_save_options(opt, "pushcommkey", buffer, 0);
		printf("buffer=%s\n", buffer);
		ans.Result = 0x01;
	}	
	memcpy(ans.DevNum, DevNumber, 11);		//ans to server
	len = answer_transinfo_format(&post, head, &ans, sizeof(TNomalAns), 0x00000000);
	return AnswerToServer(opt, &post, len);
}


int updateserveradd(PYMIP_HEAD head, char *param, iclock_options_t *opt)
{	
	TUpdateSrvAddr tmp;
	TNomalAns ans;
	TTransinfo post;
	char a[16], b[5];
	unsigned int len;
	memset(a, 0, 16);
	memset(b, 0, 5);
	memset(&post, 0, sizeof(TTransinfo));
	memset(&ans, 0, sizeof(TNomalAns));
	memset(&tmp,0,sizeof(TUpdateSrvAddr));
	if(NULL == param ||NULL == head)
	{
		return -2;
	}
	else
	{
		memcpy(&tmp,param,sizeof(TUpdateSrvAddr));
		if(0 != memcmp(tmp.DevNum, DevNumber, 11))
		{
			gErrorCode = 0x0008;
			printf("[%s] DevNumber error\n", __FUNCTION__);
			return -8;
		}
		sprintf(a, "%d.%d.%d.%d", tmp.ServerIP[0], tmp.ServerIP[1], tmp.ServerIP[2], tmp.ServerIP[3]);
		sprintf(b, "%d", ntohs(tmp.ServerPort));
		
		if((0 == cmd_save_options(opt, "WebServerIP", a, 0)) &&(0 == cmd_save_options(opt, "WebServerPort", b, 0)))
		{	
			ans.Result = 0x01;
		}
		else
			ans.Result = 0x00;
	}
	memcpy(ans.DevNum, DevNumber, 11);
	len = answer_transinfo_format(&post, head, &ans, sizeof(TNomalAns), 0x00000000);
	return AnswerToServer(opt, &post, len);
}

 
int downcommparam(PYMIP_HEAD head, char *param, iclock_options_t *opt)
{
	TCommParam tmp;
	TCommParamAns comm_ans;
	TTransinfo post;
	char request_intval[5];
	char error_intval[5];
	char reposttime[5];
	int ret = -1;
	unsigned int len;
	short int devtype = 0x0001;
	memset(request_intval, 0, 5);
	memset(error_intval, 0, 5);
	memset(reposttime, 0, 5);
	memset(&post, 0, sizeof(TTransinfo));
	memset(&tmp, 0, sizeof(TCommParam));
	memset(&comm_ans, 0, sizeof(TNomalAns));
		
	if(NULL == param ||NULL == head)
	{
		return -2;
	}
	else
	{
		printf("delay=%d, errordelay=%d\n", opt->get_request_intval, opt->error_intval);
		memcpy(&tmp, param, sizeof(TCommParam));
		if(0 != memcmp(tmp.DevNum, DevNumber, 11))
		{
			gErrorCode = 0x0008;
			printf("[%s] DevNumber error\n", __FUNCTION__);
			return -8;
		}
		if(tmp.UploadMode > 1)
		{
			gErrorCode = 0x0004;
			return -4;
		}
		else
		{
		opt->get_request_intval = ntohs(tmp.delay);		//delay
		opt->stat.get_interval=ntohs(tmp.delay);
		opt->error_intval = ntohs(tmp.ErrorDelay);		//ErrorDelay
		opt->reposttime = tmp.ReTransCnt;
		
		sprintf(request_intval, "%d", ntohs(tmp.delay));
		sprintf(error_intval, "%d", ntohs(tmp.ErrorDelay));
		sprintf(reposttime, "%d", tmp.ReTransCnt);
		cmd_save_options(opt, "Delay", request_intval, 0);
		cmd_save_options(opt, "ErrorDelay", error_intval, 0);
		cmd_save_options(opt, "Reposttime", reposttime, 0);
		printf("delay=%d, errordelay=%d, opt->reposttime=%d\n", opt->get_request_intval, opt->error_intval, opt->reposttime);
		}
	}
	memcpy(comm_ans.DevNum, DevNumber, 11);
	comm_ans.DevType = htons(devtype);
	len = answer_transinfo_format(&post, head, &comm_ans, sizeof(TCommParamAns), 0x00000000);
	return AnswerToServer(opt, &post, len);	
}

int downuserinfo(PYMIP_HEAD head, char *param, iclock_options_t *opt)
{
	TUserHead userhead;
	TUserRet user_ans;
	TTransinfo post;
	int UserCount;				//当前机具中名单数量
	int UserRemain;				//当前机具中剩余名单空间
	int ret = -1;
	unsigned int len;
	char listver[5];
	if(NULL == param ||NULL == head)
	{
		return -2;
	}
	memset(listver, 0, 5);
	memset(&post, 0, sizeof(TTransinfo));
	memset(&userhead, 0, sizeof(TUserHead));
	memset(&user_ans, 0, sizeof(TUserRet));
	memcpy(&userhead, param, sizeof(TUserHead));
	if(0 != memcmp(userhead.DevNum, DevNumber, 11))
	{
		gErrorCode = 0x0008;
		printf("[%s] DevNumber error\n", __FUNCTION__);
		return -8;
	}
	if(0x01 == userhead.OperationType)
	{
		
		ret = dl_call_fun(data_list,"userinfo",DL_UPDATE,param);
		if(0 == ret)
			ListVer = ntohl(userhead.ListVer);
		if(-4 == ret){
			gErrorCode = 0x0004;
			return -4;
		}
	}
	if(0x00 == userhead.OperationType)	//机具名单异常，擦除名单从版本0开始
	{
		if(0 ==pushsdk_clear_data(opt,FCT_ALL))
		{
			printf("abnormal list and clean all\n");
			ListVer = 0;
			ret = dl_call_fun(data_list,"userinfo",DL_UPDATE,param);
			if(0 == ret)
				ListVer = 0;
			if(-4 == ret){
				gErrorCode = 0x0004;
				return -4;
			}
		}
		else
		{
			printf("delete userinfo failed\n");
			ListVer = ntohl(userhead.ListVer);
			return -2;
		}

	}
	else if(0x01 < userhead.OperationType){
		gErrorCode = 0x0004;
		return -4;
	}
	
	tell_and_wait_parent(opt,  CHILD_CMD_USER_CNT);
	UserCount = opt->cur_user_count;
	UserRemain = opt->max_user_count - UserCount;
	printf("[%s] UserCount=%d\tUserRemain=%d\n", __FUNCTION__, UserCount, UserRemain);
	if(UserRemain < (opt->max_user_count /10) && UserRemain > 0)
	{
		
		gWarningCode = 0x20000001;
		gWarningLevel = 0x02;
	}
	if(UserRemain <= 0)
	{
		gErrorCode = 0x0007;
		gWarningCode = 0x30000001;
		gWarningLevel = 0x03;
		return -7;
	}
	memcpy(user_ans.DevNum,DevNumber, 11);
	printf("[%s]user version %d\n", __FUNCTION__, ListVer);
	sprintf(listver, "%d", ListVer);
	cmd_save_options(opt, "UserVersion", listver, 0);
	opt->UserVersion = ListVer;
	user_ans.ListVer = htonl(ListVer);
	user_ans.UserCount = htonl(UserCount);
	user_ans.UserRemain = htonl(UserRemain);
	memcpy(user_ans.CardSerialNumber, opt->last_user_card_serial, 10);
	len = answer_transinfo_format(&post, head, &user_ans, sizeof(TUserRet), 0x00000000);
	return AnswerToServer(opt, &post, len);	
}

int updatedevparam(PYMIP_HEAD head, char *param, iclock_options_t *opt)
{
	TUpParam devparam;
	TParamAns param_ans;
	TTransinfo post;
	unsigned int len;
	int flag = 0;
	int i;
	U8 h, m;
	if(NULL == param ||NULL == head)
	{
		return -2;
	}
	memset(&post, 0, sizeof(TTransinfo));
	memset(&devparam, 0, sizeof(TUpParam));
	memset(&param_ans, 0, sizeof(TParamAns));
	memcpy(&devparam, param, sizeof(TUpParam));
	if(0 != memcmp(devparam.DevNum, DevNumber, 11))
	{
		gErrorCode = 0x0008;
		printf("[%s] DevNumber error\n", __FUNCTION__);
		return -8;
	}
	for(i=0; i<16;i++)
	{	
		h = 0x00;
		m = 0x00;
		h = devparam.BellTime[i].time[0];
		m = devparam.BellTime[i].time[1];
		BCDTODEC(h);
		BCDTODEC(m);
		printf("hour=%d, min=%d i=%d\n", h, m, i);
		if(((h<0 || h>24||m<0 || m>=60) && (h != 99 && m != 99)) || (devparam.CardMode>1)){
			flag = 1;
			gErrorCode = 0x0004;
			return -1;
		}
	}
	if((0 == dl_call_fun(data_list,"BELLTIME",DL_UPDATE, param)) && 0 == flag)
		printf("save belltime success\n");
	else
		return -2;
	param_ans.ParamVer = devparam.ParamVer;
	memcpy(param_ans.DevNum,DevNumber, 11);
	len = answer_transinfo_format(&post, head, &param_ans, sizeof(TParamAns), 0x00000000);
	return AnswerToServer(opt, &post, len);	
}

int uploadwarninginfo(PYMIP_HEAD head, char *param, iclock_options_t *opt)
{
	TWarningRet tmp;
	if(NULL == param ||NULL == head)
	{
		return -2;
	}
	memset(&tmp, 0, sizeof(TWarningRet));
	memcpy(&tmp, param, sizeof(TWarningRet));
	if(0 != memcmp(tmp.DevNum, DevNumber, 11))
	{
		gErrorCode = 0x0008;
		printf("[%s] DevNumber error\n", __FUNCTION__);
		return -8;
	}
	if(0x01 == tmp.WaningResult)
		return 0;
	else
		printf("Return waning info result failed =%u\n", tmp.WaningResult);
	return -2;
}

int TransErrorCode(unsigned short int errorcode, iclock_options_t *opt)
{
	int len;
	TErrorRet er_trans;
	PYMIP_HEAD er_head;
	TTransinfo post;
	char discrinfo[50];
	memset(discrinfo, 0,50);
	memset(&er_trans, 0, sizeof(TErrorRet));
	memset(&er_head, 0, sizeof(PYMIP_HEAD));
	memset(&post, 0, sizeof(TTransinfo));
	unsigned char buflen[3]={0x00,0x00,0x4b};	
	memcpy(commkey, opt->push_comm_key, 16);
	memcpy(er_trans.DevNum, DevNumber, 11);
	memset(er_trans.ErrorDscription, 0x20, 50);
	
	if(0x0001 == errorcode)
		sprintf(discrinfo, "%s", "不支持的业务类型");
	if(0x0003 == errorcode)
		sprintf(discrinfo, "%s", "非法消息，未会话");
	if(0x0004 == errorcode)
		sprintf(discrinfo, "%s", "内容值不合法");
	if(0x0007 == errorcode)
		sprintf(discrinfo, "%s", "名单空间满");
	if(0x0008 == errorcode)
		sprintf(discrinfo, "%s", "终端不存在");
	if(0x0009 == errorcode)
		sprintf(discrinfo, "%s", "MAC校验错误");
	if(0x000a == errorcode)
		sprintf(discrinfo, "%s", "数据参数不合法");
	er_trans.ErrorCode = htons(errorcode);
	UTF8toLocal(21, discrinfo, er_trans.ErrorDscription);
	ymip_head_format(&er_head,LICENSEVERSION,buflen,FramNumber,0x8000);
	len = answer_transinfo_format(&post, &er_head, &er_trans, sizeof(TErrorRet), 0x00000000);
	return AnswerToServer(opt, &post, len);			
}

unsigned int answer_transinfo_format(PTransinfo transinfo, PYMIP_HEAD head, char *data, unsigned int datalen, int checkcode)
{
	unsigned int len, post_len;
	unsigned char answer_len[3]={0x00,0x00,0x00};
	unsigned int ServiceCode;
	int i;
	if(transinfo == NULL ||NULL == data || NULL == head)
	{
		return NULL;
	}
	ServiceCode = (ntohs(head->ServiceCode)|(1<<15));
	memset(head->Lenght, 0x0, 3);
	len=sizeof(TYMIP_HEAD)+datalen+4;
	post_len=htons(len);
	head->ServiceCode = htons(ServiceCode);
	memcpy(answer_len+1,&post_len,2);
	memcpy(head->Lenght, answer_len, 3);
	transinfo->Head=head;
	transinfo->Data=data;
	transinfo->CheckCode=checkcode;
	return len;
}

PRegister ymip_data_reg_format(PRegister data,U8 *DevNum,U8 *PSAM_num,short int DevType,U8 CommType,U8 *DevVersion,U8 *DevTime,U8 *CustomID)
{
	if(data==NULL)
	{
		return NULL;
	}
	memcpy(data->DevNum,DevNum,11);
	memcpy(data->PSAM_num,PSAM_num,6);
	data->DevType=htons(DevType);
	data->CommType=CommType;
	memcpy(data->DevVersion,DevVersion,8);
	memcpy(data->DevTime,DevTime,7);
	memcpy(data->CustomID,CustomID,4);
	return data;
}
PWarning ymip_data_warning_format(PWarning data,U8 *DevNum,U8 *PSAM_num,short int DevType,U8 CommType,U8 *DevVersion,U8 *DevTime,U8 warningtype,U8 WarningLevel,unsigned int WarningCode)
{
	if(NULL == data)
	{
		return NULL;
	}
	char info[40];
	memset(info, 0, 40);
	if(0x20000001 == WarningCode)
		sprintf(info, "%s", "名单将满");
	if(0x30000001 == WarningCode)
		sprintf(info, "%s", "名单已满");
	if(0x40000001 == WarningCode)
		sprintf(info, "%s", "读头异常");
	if(0x40000002 == WarningCode)
		sprintf(info, "%s", "显示器异常");
	if(0x40000003 == WarningCode)
		sprintf(info, "%s", "存储器异常");
	if(0x40000004 == WarningCode)
		sprintf(info, "%s", "时钟错误");

	UTF8toLocal(21, info, data->WarningInfo);
	memcpy(data->DevNum, DevNum, 11);
	memcpy(data->PSAM_num,PSAM_num,6);
	data->DevType=htons(DevType);
	data->CommType=CommType;
	memcpy(data->DevVersion,DevVersion,8);
	memcpy(data->DevTime,DevTime,7);
	data->WarningType = warningtype;
	data->WarningLevel = WarningLevel;
	data->WarningCode = htonl(WarningCode);
	return data;
}

PHeartbeat ymip_data_heartbeat_format(PHeartbeat data,U8 *DevNum,int CommVer,U8 ListType,int ListVer/*,int Reserved[]*/)
{
	if(data==NULL)
	{
		return NULL;
	}
	memcpy(data->DevNum,DevNum,11);
	data->CommVer=htonl(CommVer);
	data->ListType=ListType;
	data->ListVer=htonl(ListVer);
	//保留字段全部为空
	return data;
}

PYMIP_HEAD ymip_head_format(PYMIP_HEAD head,char lisensever,unsigned char lenght[],short int framenum,short int servicecode)
{
	if(head==NULL)
	{
		return NULL;
	}
	head->LisenceVer=lisensever;
	memcpy(head->Lenght,lenght,3);
	head->FrameNum=htons(framenum);
	head->ServiceCode=htons(servicecode);
	return head;
}

PTransinfo ymip_transinfo_format(PTransinfo transinfo,PYMIP_HEAD head,PYMIP_DATA data,unsigned int datalen,int checkcode)
{
	unsigned int len=0;
	if(transinfo==NULL)
	{
		return NULL;
	}
	len=sizeof(TYMIP_HEAD)+datalen+4;
	transinfo->Head=head;
	transinfo->Data=data;
	transinfo->CheckCode=checkcode;
	return transinfo;
}



/*****************************
根据业务代码，判断业务类型
*****************************/
ServiceType GetTypeByHead(PYMIP_HEAD head)
{
	unsigned short int Stype;
	if(head == NULL)
	{
			return INVALIDTYPE;
	}
	Stype = head->ServiceCode;
	Stype = ntohs(Stype);
	printf("stype= 0x%04x result=0x%04x\n",Stype,(Stype)&(~(1<<15)));
	switch ((Stype)&(~(1<<15)))
	{		
		case 0x0001:
			return DEVREGISTER;
			break;
		case 0x0002:
			return DEVOPEN;
			break;
		case 0x0003:
			return UPDATECOMMKEY;
			break;
		case 0x0004:
			return UPDATESERVADD;
			break;
		case 0x0005:
			return DOWNCOMMPARAM;
			break;
		case 0x0006:
			return HEARTBEAT;
			break;
		case 0x0301:
			return DOWNPARAM;
			break;
		case 0x0302:
			return DOWNUSERLIST;
			break;
		case 0x0303:
			return UPLOADATTDATA;
			break;
		case 0x0304:
			return UPLOADATTPIC;
			break;
		case 0x0401:
			return DOWNACCESSPARAM;
			break;
		case 0x0402:
			return DOWNACCESSUSER;
			break;
		case 0x0403:
			return DOWNWEEKPLAN;
			break;
		case 0x0404: 
			return DOWNHOLIDAYPLAN;
			break;
		case 0x0405: 
			return UPLOADACCESSSLOG;
			break;
		case 0x0007: 
			return WARNINTINFO;
			break;
		case 0x8000: 
			return ERRORINFO;
			break;
		default:
			return INVALIDTYPE;
	}
}

/***************************
该接口用来处理   服务器---->机具  的数据
****************************/
int ProcessService(PYMIP_HEAD head, char *param, iclock_options_t *opt)
{
	ServiceType codetype;
	int ret=-1;
	if(head==NULL || param==NULL)
	{
		return -2;
	}

	codetype=GetTypeByHead(head);
	printf("[%s]codetype=0x%04x\n",__FUNCTION__, codetype);
	if(codetype==DEVREGISTER)	//服务器注册请求的数据返回
	{
		 ret=procregistret(head, param, opt);
	}
	else if(codetype==DEVOPEN)	//开通设备请求的返回
	{
		ret=proopendevret(head, param, opt);
	}
	else if(codetype==UPDATECOMMKEY)	//服务器更新通信密钥
	{
		ret=updatecommkey(head,param, opt);
	}
	else if(codetype==UPDATESERVADD)	//更新服务器地址
	{
		ret = updateserveradd(head,param, opt);
		if(0 == ret)
		{
			pushsdk_reload_svrip_init(opt);
		}
			//pushsdk_reload_svrip_init(opt);
	}
	else if(codetype==HEARTBEAT)	//心跳
	{
		ret=procheartbeat(head, param, opt);
	}
	else if(codetype==DOWNCOMMPARAM)	//下发通信参数
	{
		ret=downcommparam(head, param, opt);
	}
	else if(codetype==DOWNPARAM)		//下发机具参数
	{
		ret = updatedevparam(head,param, opt);
	}
	else if(codetype==DOWNUSERLIST)		//下发考勤名单
	{
		ret = downuserinfo(head,param, opt);
	}
	else if(codetype==UPLOADATTDATA)	//考勤流水上传的返回
	{
		ret=prouploadatt(head, param);/*
		if(opt->cur_att_count >27000 && opt->cur_att_count < 30000)
		{
			gWarningCode = 0x20000001;
			gWarningLevel = 0x02;
		}
		if(opt->cur_att_count >= 29990)
		{
			gWarningCode = 0x30000001;
			gWarningLevel = 0x03;
		}
		*/
	}
	else if(codetype==UPLOADATTPIC)		//考勤照片的上传的返回
	{
		ret=0;
	}	
	else if(codetype==DOWNACCESSPARAM)		//门禁参数下发
	{
		ret=0;
	}	
	else if(codetype==DOWNACCESSUSER)		//门禁人员下发
	{
		ret=0;
	}	
	else if(codetype==DOWNWEEKPLAN)		//weekplan
	{
		ret=0;
	}	
	else if(codetype==DOWNHOLIDAYPLAN)		//holidayplan
	{
		ret=0;
	}	
	else if(codetype==UPLOADACCESSSLOG)		//上传门禁记录
	{
		ret=0;
	}	
	else if(codetype == WARNINTINFO)	//上传警告信息
	{
		ret = uploadwarninginfo(head,param, opt);
	}
	else if(codetype == ERRORINFO)	
	{
		ret = 0;
	}
	else if(codetype==INVALIDTYPE)	//无效的业务代码
	{
		ret=-1;
		gErrorCode = 0x0001;
	}
	return ret;
}


