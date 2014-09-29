#ifndef PUSH_COMM_H_
#define PUSH_COMM_H_
#include <time.h>
#include <stdio.h>
#include <netinet/in.h>

typedef int (*comm_session_init_f)(struct in_addr addr, unsigned short port, time_t time_out_sec, int send_buf);
typedef int (*comm_session_send_f)(int fd, char *data,int data_len,time_t t_out);
typedef int (*comm_session_recv_f)(FILE *f,char *data, int max_len, time_t timeoutSec,signed char end_char);

typedef struct   {
	comm_session_init_f init;
	comm_session_send_f send;
	comm_session_recv_f recv;
}comm_session_t;

typedef void (*http_porxy_set_f)(char *addr,unsigned short port);
typedef int(*http_porxy_get_f)(char **addr,unsigned short *port);
typedef void(*http_porxy_clear_f)(void);
typedef struct {
	char addr[64];
	unsigned short port;
	http_porxy_clear_f clear;
	http_porxy_set_f set;
	http_porxy_get_f get;
}http_porxy_t;

extern http_porxy_t http_porxy;
FILE * http_proc(char *url,char *recv_header,char *post, int postCount,FILE *postStream, int time_out_sec,int buf_size);
int  http_header_analysis(char *header,char *url);
int http_data_read(FILE *fStream, char *buffer, int maxSize, int timeoutSec);
int udp_init(unsigned short port, int *udpsocket) ;
#endif
