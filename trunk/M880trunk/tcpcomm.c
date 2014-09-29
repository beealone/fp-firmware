/*************************************************
 tcpcomm.c
                                                      
 Copyright (C) 2003-2009, ZKSoftware Inc.      		

 TCP communication module 

 $Log $
 Revision 1.01  2009/09/06 23:20:00  Zhang Honggen
 Spupport full tcp communication function

*************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "net.h"
#include "arca.h"
#include "net.h"
#include "utils.h"
#include "commu.h"
#include "options.h"
#include "flashdb.h"
#include "sensor.h"
#include "tcpcomm.h"
#include "exfun.h"

static fd_set rfds;
static int server_socket; //TCP listen 
struct timeval tv;
//TCP communication then close the temp socket
int CloseTCPSocket(void *param)
{
	int tmp_server_socket;	
	memcpy(&tmp_server_socket, param, 4);
	close(tmp_server_socket);
	return 1;
}

int SendTCPData(void *buf, int size, void *param)
{
	int tmp_server_socket;
	int byteSent=0;	
	int sentLen = 0;
	char buff2[MAX_BUFFER_LEN_TCP + sizeof(TTCPHeader) ];
	PTCPHeader ch=(PTCPHeader)(buff2);
	memset(buff2, 0, sizeof(TTCPHeader));
	ch->HeaderTag[0]=0x50; ch->HeaderTag[1]=0x50;
	ch->Cmd=CMD_TCP;
	ch->CmdCheck=0xFF-CMD_TCP;
	ch->Len=size;
	memcpy(buff2+sizeof(TTCPHeader),buf,size);
	memcpy(&tmp_server_socket, param, 4);
	size += sizeof(TTCPHeader);
	while(sentLen<size)
	{
		byteSent = send(tmp_server_socket, buff2+sentLen, size-sentLen, 0);
		if(byteSent>0)
			sentLen += byteSent;
		else
		{
			if(errno == EAGAIN)
				continue;
			else
			{
				printf("TCP send ERROR %s\n",strerror(errno));
				break;
			}
		}
	}
	//printf("TCP sent len:%d,len=%d\n",sentLen,size);
	return sentLen;
}


int ProcessTCPPackage(void *buf, int len, int tmp_server_socket)
{	
	int cmd; 
	//把数据包交由RunCommand处理
	//RunCommand be called to process data packet
	if ((cmd = RunCommand(buf, len, TRUE)) == 0) return -2;  
	//如果是请求建立连接命令，则建立连接会话，并保存通讯参数
	//the client ask to make a connection session 
	if (cmd == CMD_CONNECT)
	{
		char Sender[SENDER_LEN];
		PCmdHeader chdr=(PCmdHeader)buf;				
		//保存会话句柄
		memset(Sender, 0, SENDER_LEN);
		memcpy(Sender, (void*)&tmp_server_socket, 4);
		PCommSession session=CreateSession(Sender);
		//向对方发送连接成功的会话标识ID
		chdr->SessionID=session->SessionID;
		chdr->CheckSum=0;
		chdr->CheckSum=in_chksum(buf, sizeof(TCmdHeader));
		session->Send=SendTCPData;
		session->Speed=10*1024*1024;
		session->Close=CloseTCPSocket;
		strcpy(session->Interface, "TCP");
		//注意session->Sender作为保存的通讯参数，在以后的通讯过程中向对方发送数据时使用
		SendTCPData(chdr, sizeof(TCmdHeader), session->Sender); 
	}
	return 0;
}

extern int CommSessionCount;
extern PCommSession CommSessions;
//check tcp communication

int EthCommTCPCheck(void)
{
	int address_size=sizeof(struct sockaddr);
	static struct sockaddr_in pin;
	char headbuf[1032];
	char *buf=NULL;
	long save_file_flags;
	static int tmp_server_socket;
	static int cur_comm_count=0;
	int rc=0;	  
	int retval; 
	
	FD_ZERO(&rfds);
	FD_SET(server_socket, &rfds);
	tv.tv_sec=0;
	tv.tv_usec=0;
	
	//whether new tcp connection is coming or not 
	retval=select(server_socket+1, &rfds, NULL, NULL, &tv);
	if((retval>0)&&FD_ISSET(server_socket, &rfds))
	{
		printf("Got tcp request\n");
		tmp_server_socket=accept(server_socket, (struct sockaddr *)&pin,(unsigned int*) &address_size);
		//printf("Error :%s\n",strerror(errno));
		if (tmp_server_socket!=-1)
		{		
			struct timeval tmp_tv;
			tmp_tv.tv_sec = 8;
			tmp_tv.tv_usec = 0;
			setsockopt(tmp_server_socket,SOL_SOCKET,SO_RCVTIMEO,&tmp_tv,sizeof(tmp_tv));
			setsockopt(tmp_server_socket,SOL_SOCKET,SO_SNDTIMEO,&tmp_tv,sizeof(tmp_tv));
			//check whether data arrival or not, Maximize data packet size is 1032 
			int len=0;
			len = recv(tmp_server_socket, headbuf, sizeof(TTCPHeader), 0);
			//printf("Check Connect len=%d\n",len);
			if(len==sizeof(TTCPHeader))
			{
				PTCPHeader tcpch = (PTCPHeader)headbuf;
				if(tcpch->Cmd == CMD_TCP && tcpch->CmdCheck == (0xff-CMD_TCP))
				{
					int curlen=0;
					//printf("to recv len:%d\n",tcpch->Len);
					buf = (char*)MALLOC(tcpch->Len<1032?1032:tcpch->Len);
					len = 0;
					while(len<tcpch->Len)
					{
						curlen = recv(tmp_server_socket,buf + len,tcpch->Len-len,0);
						if(curlen>0)
						{
							len += curlen;
							//printf("curlen %d\n",curlen);
						}
						else
						{
							printf("recv error %s\n",strerror(errno));
							break;
						}
					}
					//printf("recv len =%d\n",len);
				}
				else
					len = 0;
			}else
			{
				if(len != 0)
					len = -1;
			}
			if(len>0)
			{
				//set socket to non-blocking
				save_file_flags=fcntl(tmp_server_socket, F_GETFL);
				save_file_flags|=O_NONBLOCK;
				fcntl(tmp_server_socket, F_SETFL, save_file_flags);
				//process the data
				rc=ProcessTCPPackage(buf, len, tmp_server_socket);
			}
			else if(len==0)
			{
				printf("clse socket 1\n");
				close(tmp_server_socket);
			}
		}
	}
	else
	{
		if (cur_comm_count>=CommSessionCount)	
		{		
			cur_comm_count=0;	
		}
		if ((CommSessionCount>0)&&(nstrcmp("TCP", CommSessions[cur_comm_count].Interface, 3)==0))
		{
			memcpy(&tmp_server_socket, CommSessions[cur_comm_count].Sender, 4);
			//check whether data arrival or not, Maximize data packet size is 1032 
			//zzz int len=recv(tmp_server_socket,headbuf, 1032, 0);

			int len=0;
			len = recv(tmp_server_socket, headbuf, sizeof(TTCPHeader), 0);
			//printf("***** recv head :%d\n",len);
			if(len==sizeof(TTCPHeader))
			{
				PTCPHeader tcpch = (PTCPHeader)headbuf;
				//printf("***** cmd =%x\n",tcpch->Cmd);
				if(tcpch->Cmd == CMD_TCP && tcpch->CmdCheck == (0xff-CMD_TCP))
				{
					int curlen=0;
					//printf("***** to recv len:%d\n",tcpch->Len);
					buf = (char*)MALLOC(tcpch->Len<1032?1032:tcpch->Len);
					len = 0;
					while(len<tcpch->Len)
					{
						curlen = recv(tmp_server_socket,buf + len,tcpch->Len-len,0);
						if(curlen>0)
						{
							len += curlen;
							//printf("curlen %d\n",curlen);
						}
						else
						{
							if(errno == EAGAIN)
								continue;
							else
							{
								//printf("recv error %s\n",strerror(errno));
								break;
							}
						}
					}
					//printf("***** recv len =%d\n",len);
				}else
					len = -1;
			}else
			{
				if(len != 0)
					len = -1;
			}

			//printf("***** else tcpprocess \n");
			if(len>0)
			{
				rc=ProcessTCPPackage(buf, len, tmp_server_socket);
			}
			else if(len==0)
			{
				printf("clse socket 2\n");
				close(tmp_server_socket);
			}
		}
		cur_comm_count++;
	}
	if(buf)		FREE(buf);
	return rc;
}

int InitTCPSocket()
{
	struct sockaddr_in sin;
	if ((server_socket=socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		return -1;
	}

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(gOptions.TCPPort);
		
	if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin))==-1)
	{
		printf("TCP bind ERROR %s\n",strerror(errno));
		return -1;
	}

	if (listen(server_socket, 1)==-1)
	{
		close(server_socket);
		printf("TCP listen ERROR %s\n",strerror(errno));
		return -1;
	}

	return 0;
}


