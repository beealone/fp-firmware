#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>


#include <netinet/in.h>
#include <arpa/inet.h>

#include "3des.h"
#include "lib.h"
#include "tcp.h"
#include "strlist.h"
#include "http.h"
#include "fw_api.h"
#include "fuliye.h"
extern unsigned char commkey[16];
extern unsigned char pRadom[8];

void http_porxy_set(char *addr,unsigned short port);
int  http_porxy_get(char **addr,unsigned short *port);
void http_porxy_clear(void);

http_porxy_t http_porxy = {
	{0},
	80,
	http_porxy_clear,
	http_porxy_set,
	http_porxy_get,
};

static comm_session_t  comm_session = {
	 tcp_init,
	 tcp_send,
	 tcp_recv,
};


void http_porxy_set(char *addr,unsigned short port)
{
	int size;
	if (addr == NULL) {
		return ;
	}
	size = strlen(addr) > (sizeof(http_porxy.addr) -1) ? \
			(sizeof(http_porxy.addr) -1) : strlen(addr);
			
	memset(http_porxy.addr,0,sizeof(http_porxy.addr));
	
	strncpy(http_porxy.addr,addr,size);
	http_porxy.port = port;
}

void http_porxy_clear(void)
{
	memset(http_porxy.addr,0,sizeof(http_porxy.addr));
	http_porxy.port = 0;
}
int  http_porxy_get(char **addr,unsigned short *port)
{	
	if ((addr == NULL) || (port == NULL)) {
		return FALSE;
	}
	*addr = http_porxy.addr;
	*port = http_porxy.port;
	
	return (http_porxy.addr[0] != '\0');
}

struct hostent *get_host_by_name(const char *name)
{
	struct hostent *tmp = NULL;
	static struct hostent host;

	if ((name == NULL)) {
		return NULL;
	}
	tmp = gethostbyname(name);
	if (tmp != NULL) {
		memcpy(&host,tmp,sizeof(struct hostent));
	} else {
		return NULL;	
	}
	return &host;
}
int get_auth_from_url(char *url,char *auth)
{
	char *pos;

	if ((url == NULL) || (auth == NULL)) {
		return 0;
	}
	*auth = 0;

	if (!(strncasecmp(url, "http://", 7))) {
	url += 7;
	}

	if (!(strncasecmp(url, "ftp://", 6))) {
		url += 6;
	}

	if( (pos = strchr(url,'@')) ) {
		int i;
		for(i=0;i<pos-url;i++) {
			if( url[i] == '/' ) {
			 	return 0;
			}
		}
		strncpy(auth,url,pos-url);
		auth[pos-url] = 0;
		strcpy(url,pos+1);
		return 1;
	}
	return 0;
}

int get_send_size(int postCount, FILE *postStream)
{
	int slen=0;
	int flen;
	
	if (postCount>0 || (postStream != NULL)) {
		if (postStream != NULL) {
			flen = ftell(postStream);
			if (flen >= 0) {
				fseek(postStream, 0, SEEK_END);
				slen = ftell(postStream);
				fseek(postStream, flen, SEEK_SET);
			}
			if (slen >= 0) {
				slen = slen-flen+postCount;
			}
		} else {
			slen = postCount;
		}
	}
	return slen;
}
void http_header_construct(char *header,char *url,int post_len,char *host,unsigned short port,char *add_data,char *auth)
{
	if(post_len > 0) {
		strcpy(header, "POST ");
	} else {
		strcpy (header, "GET ");
	}
	strcat (header, url);

	sprintf(header + strlen(header)," HTTP/1.0\r\nHost: %s:%d\r\n", host, port);
	sprintf (header + strlen(header),\
		"User-Agent: %s/%s\r\nConnection: close\r\n",\
		"iClock Proxy", "1.09");
		
	if (post_len > 0) {
		sprintf(header + strlen(header),"Content-Length: %d\r\n",post_len);
	}
	
	strcat (header, "Accept: */*\r\n");
	
	if(add_data && *add_data) {
		strcat (header, add_data);
	}
	
	if (strlen(auth)) {
		char buf[1023];
		
		strcat (header,"Authorization: Basic ");
		base64_encode((unsigned char *)auth,strlen(auth), (unsigned char *)buf);
		strcat (header,buf);
		strcat (header,"\r\n");
	}
	strcat (header, "\r\n");
}
int http_send_file(int sock,FILE *file_stream,time_t time_out)
{
	int count = 0;
	char post_buffer[1024*4] ;

	fseek(file_stream, 0, SEEK_SET);
	while (!feof(file_stream)) {
		count = fread(post_buffer, sizeof(char), sizeof(post_buffer), file_stream);
		if (comm_session.send(sock, post_buffer, count,time_out) == 0) {
			return 0;
		}
	}
	return 1;
}

int http_recv_header(FILE *file,char *recv_header,time_t t_out)
{
	char recv_buf[1024];

	recv_header[0] = '\0';
	do {
		if (comm_session.recv(file,recv_buf,sizeof(recv_buf)-1,t_out,'\n') == 0) {
			return 0;
		}
		memcpy(recv_header+strlen(recv_header),recv_buf,strlen(recv_buf)+1);
	}while ((recv_buf[0] != '\r') && (recv_buf[1] != '\n'));
	return 1;
}


int http_recv_header_ymip(FILE *file,unsigned char *recv_header,time_t t_out)
{
	unsigned char recv_buf[1024];
	int i=0;
	if (tcp_recv_ymip(file,recv_buf,sizeof(recv_buf)-1,t_out,'\n') == 0) {
		printf("[%s][%d]\n",__FUNCTION__,__LINE__);
		return 0;
	}
	memcpy(recv_header, recv_buf, 8);
	
	printf("\nrev head:\n");
	for(i=0;i<8;i++)
	{
		printf("0x%02x ",recv_buf[i]);
	}
	printf("\n");

	return 1; 
}
/*
*Transfer-Encoding: chunked
* ea5 \r\n …………\r\n ea5 \r\n………………\r\n 0\r\n\r\n
*/
FILE *http_data_decode(char* http_header,FILE *file,time_t t_out)
{
	if (strstr(http_header,"\r\nContent-Length:") != NULL) {
		return file;
	} else if (strstr(http_header,"\r\nTransfer-Encoding: chunked") != NULL) {
		static unsigned int file_node = 0;
		FILE *decode_file ;
		int ch;
		char buf[32] = {0};
		int i = 0;
		int len = 0;
		char *data = NULL;
		char decode_file_name[56];

		snprintf(decode_file_name,sizeof(decode_file_name),"/tmp/http_data_%d.txt",file_node++);
		decode_file = fopen(decode_file_name,"w+");
		
		if (decode_file == NULL) {
			return NULL;
		}
		while (!feof(file)) {
			ch = fgetc(file);
			if (ch != EOF) {
				buf[i] = ch; 
			}
			if ((i > 1) && (buf[i-1] == '\r') && (buf[i] == '\n')) {
				len = strtol(buf,NULL,16);
				if (len == 0) {
					fseek(decode_file,0,SEEK_SET);
					return decode_file;
				}
				data = (char *)malloc(sizeof(char)*(len));
				if (data == NULL) {
					break;
				}

				if (fread(data,sizeof(char),len,file) != len) {
					free(data);
					break;
				}
				if (fwrite(data,sizeof(char),len,decode_file) != len) {
					free(data);
					break;
				}
				memset(buf,0,sizeof(buf));
				i = 0;
				if (fread(data,sizeof(char),2,file) != 2) {
					free(data);
					break;
				}
				free(data);
			}else {
				i++;
			}
		}
	} else {
		return file;
	}
	return NULL;
}


FILE *ymip_att_proc(char *svrip,unsigned int svrport,unsigned char *recv_header,char *post, int postCount, FILE *postStream, int time_out_sec,int buf_size)
{
	unsigned char last_url[2048];

	unsigned short connect_port;

	char postbuf[4096];

	struct hostent *hp;
	int sock = -1;
	struct in_addr addr;
	FILE *file_read = NULL;
	if(svrip){
	hp = get_host_by_name(svrip);
		}else{
	hp = get_host_by_name("192.168.10.215");
	}

	if (hp != NULL) {
		if (hp->h_length == sizeof(struct in_addr)) {
			memcpy(&addr, hp->h_addr, hp->h_length);
			connect_port=svrport;
			if ((sock = comm_session.init(addr,connect_port,time_out_sec,4096)) <= 0) {
				return NULL;
			}
		}
	}
	

	if(1)
	{
		int i=0;
		for(i=0;i<buf_size;i++)
		{
			if(i<8)
			{
				printf("0x%02x  ",post[i]);
			}
			else if((i>=8) && (i<(buf_size-4)))
			{
				printf("0x%02x  ",post[i]);
			}
			else
				printf("0x%02x  ",post[i]);
		}
		printf("\n\n");

	}
	
	if (0 == comm_session.send(sock,post,buf_size,time_out_sec)) {
		close(sock);
		return NULL;
	}
	if ((post > 0) && (postCount > 0)) {
		if (0== comm_session.send(sock,post,postCount,time_out_sec)) {
			close(sock);
			return NULL;
		}
	}
	

	file_read = fdopen(sock, "rb");
	if (file_read == NULL) {
		close(sock);
		printf("[%s][%d]\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	if (http_recv_header_ymip(file_read,last_url,time_out_sec) == 0) {
		fclose(file_read);
		printf("[%s][%d]\n",__FUNCTION__,__LINE__);
		return NULL;
	}
	if (recv_header != NULL) {
		memcpy(recv_header,last_url,8);
	}	
//	return http_data_decode(last_url,file_read, time_out_sec);
	return file_read;
}


// http://192.168.10.215:8081/iclock/cdata?SN=12355666666&commkey=111111

// 192.168.10.215:8081

//head\n\rdata

FILE *ymip_proc(char *svrip,unsigned int svrport,unsigned char *recv_header,char *post, int postCount, FILE *postStream, int time_out_sec,int buf_size)
{
	unsigned char last_url[2048];
	unsigned int checkcode = 0;
	unsigned short connect_port;
	int i=0;
	PTransinfo info=(PTransinfo)post;
	unsigned char postbuf[4096];

	struct hostent *hp;
	int sock = -1;
	struct in_addr addr;
	FILE *file_read = NULL;
	if(svrip){
	hp = get_host_by_name(svrip);
		}else{
	hp = get_host_by_name("192.168.10.75");
	}
	
	if (hp != NULL) {
		if (hp->h_length == sizeof(struct in_addr)) {
			memcpy(&addr, hp->h_addr, hp->h_length);
			connect_port=svrport;
			if ((sock = comm_session.init(addr,connect_port,time_out_sec,4096)) <= 0) {
				return NULL;
			}
		}
	}
	if(0)
	{
		printf("version=0x%02x\n",((PYMIP_HEAD)(info->Head))->LisenceVer);
		printf("Lenght[0]=0x%02x\n",((PYMIP_HEAD)(info->Head))->Lenght[0]);
		printf("Lenght[1]=0x%02x\n",((PYMIP_HEAD)(info->Head))->Lenght[1]);
		printf("Lenght[2]=0x%02x\n",((PYMIP_HEAD)(info->Head))->Lenght[2]);
		printf("FrameNum=0x%04x\n",((PYMIP_HEAD)(info->Head))->FrameNum);
		printf("ServiceCode=0x%04x\n",((PYMIP_HEAD)(info->Head))->ServiceCode);
	}
		//将数据memcpy到postbuf
	memcpy(postbuf,info->Head,sizeof(TYMIP_HEAD));
	memcpy(postbuf+8,info->Data,buf_size-12);
	printf("Commkey:\n");
		for(i=0; i<16; i++)
			printf("%02x ", commkey[i]);
		printf("\n");
	MACCAL_KEY16(commkey, pRadom, 0, postbuf, buf_size-4, &checkcode);
	info->CheckCode = checkcode;
	memcpy(postbuf+buf_size-4,&(info->CheckCode),4);
		
		printf("\nsend data:\n");
		for(i=0;i<buf_size;i++)
		{
			if(i<8)
			{
				printf("0x%02x  ",postbuf[i]);
			}
			else if((i>=8) && (i<(buf_size-4)))
			{
				printf("0x%02x  ",postbuf[i]);
			}
			else
				printf("0x%02x  ",postbuf[i]);
		}
		printf("\n\n");
		
	if (0 == comm_session.send(sock,postbuf,buf_size,time_out_sec)) {
		close(sock);
		return NULL;
	}
	if ((post > 0) && (postCount > 0)) {
		if (0== comm_session.send(sock,post,postCount,time_out_sec)) {
			close(sock);
			return NULL;
		}
	}

	file_read = fdopen(sock, "rb");
	if (file_read == NULL) {
		close(sock);
		printf("[%s][%d]\n",__FUNCTION__,__LINE__);
		return NULL;
	}

	if (recv_header != NULL) {
		if (http_recv_header_ymip(file_read,last_url,time_out_sec) == 0) {
			fclose(file_read);
			printf("[%s][%d]\n",__FUNCTION__,__LINE__);
			return NULL;
		}
		memcpy(recv_header,last_url,8);
	}	
	return file_read;
}


FILE *http_proc(char *url,char *recv_header,char *post, int postCount, FILE *postStream, int time_out_sec,int buf_size)
{
	char last_url[2048];
	char *proxy_host;
	unsigned short proxy_port;
	char *svr_host;
	unsigned short svr_port;
	char *connect_host;
	unsigned short connect_port;

	struct hostent *hp;
	int sock = -1;
	struct in_addr addr;
	char *spurl;
	char send_header[2*1024];

	char http_auth[256];
	FILE *file_read = NULL;
	url_check(url, last_url);
	http_porxy.get(&proxy_host,&proxy_port);
	spurl = url_to_host_port(last_url,&svr_host,&svr_port);
	if (spurl == NULL) {
		return NULL;
	}
	if (proxy_host[0] != '\0') {
		connect_host = proxy_host;
		connect_port = proxy_port;
	} else {
		connect_host = svr_host;
		connect_port = svr_port;
	}
	hp = get_host_by_name(connect_host);
	if (hp != NULL) {
		if (hp->h_length == sizeof(struct in_addr)) {
			memcpy(&addr, hp->h_addr, hp->h_length);
			if ((sock = comm_session.init(addr,connect_port,time_out_sec,buf_size)) <= 0) {
				free(svr_host);
				return NULL;
			}
		}
	}
	get_auth_from_url(last_url,http_auth);
	http_header_construct(send_header,(proxy_host[0] != '\0') ? last_url:spurl,\
						get_send_size(postCount,postStream),svr_host,\
						svr_port,recv_header,http_auth);
	free(svr_host);
	if (0 == comm_session.send(sock,send_header,strlen(send_header),time_out_sec)) {
		close(sock);
		return NULL;
	}
	if ((post > 0) && (postCount > 0)) {
		if (0== comm_session.send(sock,post,postCount,time_out_sec)) {
			close(sock);
			return NULL;
		}
	}
	
	if (postStream != NULL) {
		if (http_send_file(sock, postStream, time_out_sec) == 0) {
			close(sock);
			return NULL;
		}
	}
	file_read = fdopen(sock, "rb");
	if (file_read == NULL) {
		close(sock);
		return NULL;
	}
	if (http_recv_header(file_read,last_url,time_out_sec) == 0) {
		fclose(file_read);
		return NULL;
	}
	if (recv_header != NULL) {
		strcpy(recv_header,last_url);
	}
	
	return http_data_decode(last_url,file_read, time_out_sec);
}



BOOL http_header_analysis(char *header,char *url)
{
	BOOL svr_code = TRUE;
	
	if (!((strncmp(header, "HTTP/1.1 200 ",13) == 0) || \
		(strncmp(header, "HTTP/1.0 200 ",13)==0))) {
		svr_code = FALSE;
		if ((strncmp(header, "HTTP/1.1 404 ", 13) == 0) || \
			(strncmp(header, "HTTP/1.0 404 ", 13) == 0)) {
			printf("[PUSH SDK]    Server is not ready or url is wrong- %s \n", url);
		} else if ((strncmp(header, "HTTP/1.1 500 ", 13) == 0) || \
				(strncmp(header, "HTTP/1.0 500 ", 13) == 0)) {
			printf("[PUSH SDK]    Server Error!\n");
		} else {
			printf("[PUSH SDK]    Server UNKNOW ERROR\n");
		}
	}	
	return svr_code;
}

int http_data_read(FILE *fStream, char *buffer, int maxSize, int timeoutSec)
{
	int cc;

	if ((fStream == NULL) || (buffer == NULL)) {
		return -1;
	}
	if (feof(fStream)) {
		return 0;
	}
	cc = fAvialible(fStream, timeoutSec*1000);
	
	if (cc < 0 ) {
		return -1;
	} else if (cc == 0) {
		while (!feof(fStream)) {
			int ch = fgetc(fStream);
			if (ch == EOF) {
				break;
			}
			buffer[cc]=ch;
			cc++;
			if (cc >= (maxSize-1)) {
				break;
			}
		}
	} else {
		if(cc>maxSize) {
			cc=maxSize;
		}
		if (fread(buffer, cc, 1, fStream) <= 0) {
			return -2;
		}
	}
	return cc;
}

int udp_init(unsigned short port, int *udpsocket) 
{
	struct sockaddr_in sin;
	long save_file_flags;
	

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);
	
	//create a receive UDP Scoket
	*udpsocket=socket(AF_INET, SOCK_DGRAM, 0);
	if (*udpsocket==-1) return -1; 	 
	//bind it to the port
	if (bind(*udpsocket, (struct sockaddr *)&sin, sizeof(sin))==-1)
	{
		close(*udpsocket);
		*udpsocket=-1;
		return -1;
	}
	//set socket to non-blocking
	save_file_flags = fcntl(*udpsocket, F_GETFL);
	save_file_flags |= O_NONBLOCK;
	if (fcntl(*udpsocket, F_SETFL, save_file_flags) == -1)
	{
		close(*udpsocket);
		*udpsocket=-1;
		return -1;
	}
	return 0;
}	
