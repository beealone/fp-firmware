#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "lib.h"
#include "fw_api.h"
#include "fuliye.h"

int tcp_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen, int timeout)
{
	struct timeval tm;
	fd_set set;
	int ret=-1;
	int error=-1, len;
	long save_file_flags;
	int netFlag;
	time_t  beforeStamp;

	beforeStamp = time(NULL);

	save_file_flags=fcntl(sockfd, F_GETFL);
	errno = 0;
	if (connect(sockfd, serv_addr, addrlen)==-1) {
		tm.tv_sec  = timeout;
		tm.tv_usec = 0;
		/*set the NOBLOCK flags to file descriptor*/
		fcntl(sockfd, F_SETFL, save_file_flags|O_NONBLOCK);
		while (1) {
			if  ((abs(time(NULL) - beforeStamp) >=  timeout) || 
			   ((netFlag = interface_get_stat(AF_INET)) != 1 )) {
				ret=-1;
				break;
			}
			
			FD_ZERO(&set);
			FD_SET(sockfd, &set);
			if (select(sockfd+1, NULL, &set, NULL, &tm) > 0) {
				len = sizeof(int);
				getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
				if(error == 0) {
					ret = 0;
				} else  {
					ret=-1;
				}
				break;
			} else {
				if (errno == EINTR || errno == EAGAIN) {
					sleep(1);
					continue;
				} else {	
					ret=-1;
					break;
				}
				
			}
		}
		
	} else {
		ret=0;
	}

	/*restore old file status flags*/
	fcntl(sockfd, F_SETFL, save_file_flags);

	return ret;
}

int tcp_init(struct in_addr addr, unsigned short port, time_t time_out_sec, int send_buf)
{
	struct sockaddr_in sin;
	int sock = -1;
	struct timeval tm;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock >= 0) {
		tm.tv_sec  = time_out_sec;    
		tm.tv_usec = 0;
		
		setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&tm,sizeof(struct timeval));
		setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char*)&send_buf,sizeof(int));
		
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr = addr;
		sin.sin_port = htons(port);
		
		if (tcp_connect(sock, (struct sockaddr *)&sin, \
			sizeof(struct sockaddr_in), time_out_sec) < 0) {
			close(sock);
			sock = -1;
		}
	}
	return sock;
}

int tcp_send(int fd, char *data,int data_len,time_t t_out)
{
        int count = 0;
        int nwrite = -1;
        struct timeval tm_out;
        tm_out.tv_sec  = t_out;
        tm_out.tv_usec = 0;
	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(char *)&tm_out,sizeof(struct timeval));	
	
        while (data_len - count > 0) {
                do {
                        nwrite = write(fd, data + count, data_len - count);
                } while ((nwrite < 0) && (errno == EINTR || errno == EAGAIN));

                if (nwrite > 0) {
                        count += nwrite;
                } else {
                        return 0;
                }
        }

        return 1;
}

int tcp_recv(FILE *f,char *string, int max_len, int timeoutSec,signed char end_char)
{
	int pos = 0;
	int ret;
	int fd = fileno(f);
	time_t enter_time = time(NULL);

	while(1) {
		if (abs(time(NULL) - enter_time) > timeoutSec || (interface_get_stat(AF_INET) <= 0)) {
			return 0;
		}
		 ret = avialible(fd, timeoutSec*1000);

		if(ret <= 0) {
			if ((errno != EINTR) &&  (errno != EAGAIN)) {
				break;
			} else {
				sleep(1);
				continue;
			}
		}
		if(read(fd,string+pos,1) == 1) {
			pos++;
			if(((string[pos-1] == end_char) && (end_char > 0)) || (pos >= max_len)) {
				string[pos] = 0;
				return 1;
			}
		} else if ((errno != EINTR) &&  (errno != EAGAIN)) {
			break;
		}
	}
	
	string[pos]=0;
	return 0;
}

int tcp_answer_server(int fd, char *data,int data_len,time_t t_out)
{
        int count = 0;
        int nwrite = -1;
        struct timeval tm_out;
        tm_out.tv_sec  = t_out;
        tm_out.tv_usec = 0;
	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(char *)&tm_out,sizeof(struct timeval));	
	printf("[%s][%d]\n",__FUNCTION__,__LINE__);
        while (data_len - count > 0) {
                do {
                        nwrite = write(fd, data + count, data_len - count);
                } while ((nwrite < 0) && (errno == EINTR || errno == EAGAIN));

                if (nwrite > 0) {
                        count += nwrite;
                } else {
                        return 0;
                }
        }

        return 1;
}
int tcp_recv_ymip(FILE *f,unsigned char *string, int max_len, int timeoutSec,signed char end_char)
{
	int pos = 0;
	int ret;
	int fd = fileno(f);
	int count=0;
	time_t enter_time = time(NULL);
	

	while(1) {
		if (abs(time(NULL) - enter_time) > timeoutSec || (interface_get_stat(AF_INET) <= 0)) {
			return 0;
		}
		 ret = avialible(fd, timeoutSec*1000);

		if(ret <= 0) {
			if ((errno != EINTR) &&  (errno != EAGAIN)) {
				break;
			} else {
				sleep(1);
				continue;
			}
		}
		if(read(fd,string+pos,1) == 1) {
			pos++;
			count++;
			//if((count==8)||((( string[pos-1] == end_char) && (end_char > 0)) || (pos >= max_len))) {
			if((count==8)|| (pos >= max_len)) {
				string[pos] = 0;
				return 1;
			}
		} else if ((errno != EINTR) &&  (errno != EAGAIN)) {
			break;
		}
	}
	
	string[pos]=0;
	return 0;
}

