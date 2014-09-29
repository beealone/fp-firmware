#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#include "pipe.h"
#include "lib.h"
#include "fw_api.h"
#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

static int pipe_fd1[2];
static int pipe_fd2[2];

int pipe_set_nonblock(int pipe_fd)
{
	int pipe_flag;
	
	pipe_flag = fcntl(pipe_fd, F_GETFL,0);
	pipe_flag |= O_NONBLOCK;
	
	if (fcntl(pipe_fd, F_SETFL,pipe_flag) < 0) {
		return -1;
	}
	return 0;
}

int pipe_init(void)
{
	if (pipe(pipe_fd1) < 0 || pipe(pipe_fd2) < 0) {
		return -1;
	}
	if ((pipe_set_nonblock(pipe_fd1[PIPE_WRITE_END]) != 0) ||\
		(pipe_set_nonblock(pipe_fd2[PIPE_WRITE_END]) != 0)) {
		return -1;
	}
	return 0;
}

int pipe_write_to_parent(void *buf, int len)
{
	if (pipe_fd2[PIPE_WRITE_END] < 0 || buf == NULL) {
		return -1;
	}

	if (write(pipe_fd2[PIPE_WRITE_END], buf, len) != len) {
		return -1;
	}
	return 0;
}
int pipe_write_to_parent_cmd(char cmd)
{	
	if (pipe_fd2[PIPE_WRITE_END] < 0 ) {
		return -1;
	}

	if (write(pipe_fd2[PIPE_WRITE_END], &cmd, 1) != 1) {
		return -1;
	}
	return 0;
}
int pipe_write_to_child(void *buf, int len)
{
	if (pipe_fd1[PIPE_WRITE_END] < 0 || buf == NULL) {
		return -1;
	}

	if (write(pipe_fd1[PIPE_WRITE_END], buf, len) != len) {
		return -1;
	}

	return 0;
}
int pipe_write_to_child_cmd(char cmd)
{
	if (pipe_fd1[PIPE_WRITE_END] < 0) {
		return -1;
	}
	if (write(pipe_fd1[PIPE_WRITE_END], &cmd, 1) != 1) {
		return -1;
	}
	return 0;
}

int pipe_read_from_parent(void *buf, int len)
{
	int rlen;
	int ret = 0;
	if (pipe_fd1[PIPE_READ_END] < 0 || buf == NULL) {
		return -1;
	}
	//ioctl(pipe_fd1[PIPE_READ_END], FIONREAD, &rlen);
	rlen = avialible(pipe_fd1[PIPE_READ_END],1000);
	if (rlen >= len) {
		ret = read(pipe_fd1[PIPE_READ_END], buf, len);
		printf("[%s]___%d__ret=%d,len=%d\n",__FUNCTION__,__LINE__, ret, len);
		if (ret < 0) {
			return -1;
		}
      } else if (rlen != 0) {
		return ret;
	}
	
	return ret;
}
void pipe_clean_parent_cmd(void)
{
	int rlen;
	char *buf = NULL;

	if (pipe_fd1[PIPE_READ_END] < 0 || buf == NULL) {
		return ;
	}
	while ((rlen = avialible(pipe_fd1[PIPE_READ_END],1000)) > 0) {
		buf = (char *)malloc(sizeof(char)*rlen);
		if (buf == NULL) {
			return;
		}
		read(pipe_fd1[PIPE_READ_END], buf, rlen);
		free(buf);
      } 
}
int pipe_read_from_child(void *buf, int len)
{
	int rlen;
	int ret = 0;

	if (pipe_fd2[PIPE_READ_END] < 0 || buf == NULL) {
		return -1;
	}	
	ioctl(pipe_fd2[PIPE_READ_END], FIONREAD, &rlen);
	if ((rlen >= len) ) {
		ret = read(pipe_fd2[PIPE_READ_END], buf, len);
	} else {
		return rlen;
	}
	return ret;

}

void pipe_childend_close(void)
{

	if (pipe_fd1[PIPE_WRITE_END] > 0) {
		close(pipe_fd1[PIPE_WRITE_END]);
		pipe_fd1[PIPE_WRITE_END] = -1;
	}

	if (pipe_fd2[PIPE_READ_END] > 0) {
		close(pipe_fd2[PIPE_READ_END]);
		pipe_fd2[PIPE_READ_END] = -1;
	}

}

void pipe_parentend_close(void)
{
	if (pipe_fd1[PIPE_READ_END] > 0) {
		close(pipe_fd1[PIPE_READ_END]);
		pipe_fd1[PIPE_READ_END] = -1;
	}

	if (pipe_fd2[PIPE_WRITE_END] > 0) {
		close(pipe_fd2[PIPE_WRITE_END]);
		pipe_fd2[PIPE_WRITE_END] = -1;
	}

}

void pipe_free(void)
{
	if (pipe_fd1[PIPE_READ_END] > 0) {
		close(pipe_fd1[PIPE_READ_END]);
		pipe_fd1[PIPE_READ_END] = -1;
	}

	if (pipe_fd1[PIPE_WRITE_END] > 0) {
		close(pipe_fd1[PIPE_WRITE_END]);
		pipe_fd1[PIPE_WRITE_END] = -1;
	}

	if (pipe_fd2[PIPE_READ_END] > 0) {
		close(pipe_fd2[PIPE_READ_END]);
		pipe_fd2[PIPE_READ_END] = -1;
	}

	if (pipe_fd2[PIPE_WRITE_END] > 0) {
		close(pipe_fd2[PIPE_WRITE_END]);	
		pipe_fd2[PIPE_WRITE_END] = -1;
	}
}

