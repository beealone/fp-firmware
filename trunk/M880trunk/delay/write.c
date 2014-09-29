/*  demo/wiegand/wiegand_rev.c
 *  ZEM200 demostration program for wiegand I/O 
 *  Copyright(C) by ZK software 2004-2005
 */

#include <stdio.h>
#include <stdlib.h> 
#include <fcntl.h>
#include <unistd.h>
#include "wiegand.h"

int main(int argc, char  ** argv)
{
  int  fd = -1, loops, bytes; 
  unsigned int data[2];
  
  
  if((fd = open("/dev/wiegand",O_WRONLY |O_NONBLOCK )) < 0){
        printf("can`t open device\n");   
	return -1;
  }
  for(loops = 65535; loops >= 0; ){
      data[0] = 0xFF;
      data[1] = 0x10;

      ioctl(fd, WIEGAND_IO_WRITE, data);
      usleep(100000);
  }  
  close(fd);  
  return  0;
}

