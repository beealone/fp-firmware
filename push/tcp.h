#ifndef HTTP_H_
#define HTPP_H_
#include <time.h>
#include <stdio.h>
#include <netinet/in.h>

int tcp_init(struct in_addr addr, unsigned short port, time_t time_out_sec, int send_buf);
int tcp_send(int fd, char *data,int data_len,time_t t_out);
int tcp_recv(FILE *f,char *data, int max_len, time_t timeoutSec,signed char end_char);
#endif
