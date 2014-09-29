/*************************************************
  
 ZEM 200                                          
 
 net.c Simple network layer with UDP & TCP                              
 
 Copyright (C) 2003-2004, ZKSoftware Inc.      
 
 $Log: net.c,v $
 Revision 5.11  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.10  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.9  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.8  2005/08/07 08:13:15  david
 Modfiy Red&Green LED and Beep

 Revision 5.7  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.6  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk
 
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
#include <time.h>
//#define __KERNEL__
//#include <asm/types.h>
//#include <linux/sockios.h>
//#include <linux/ethtool.h>
#include "arca.h"
#include "net.h"
#include "utils.h"
#include "commu.h"
#include "options.h"
#include "exfun.h"
#include "main.h"
#include "flashdb.h"
#include "sensor.h"
#include "tcpcomm.h"

#define THEIP gOptions.IPAddress
#define THEUDP gOptions.UDPPort
#define THETCP gOptions.TCPPort
#define BROADCASTPORT 65535

static int comm_socket,listenAuthServer_socket, receive_broadcast_socket; //UDP

static int image_socket=-1; //TCP AuthServer
int AuthServerSessionCount=0;
PAuthServerSession AuthServerSessions=NULL;
int MaxAuthServerSessionCount=0;
int StaticDNSCount=0;

//send data with UDP protocol
//buf - buffer will be send
//size - length of data buffer
//param - this parameter save the ip address and port number of receiver, pls refer E TFPResult, *PFPResultthCommCheck function  
//该参数用于识别接收数据方的ip地址和端口号，该参数由 eth_rx 在一个通讯会话建立时保存下来。
int SendUDPData(void *buf, int size, void *param)
{
	return sendto(comm_socket, buf, size, 0, (struct sockaddr*)param, sizeof(struct sockaddr_in));
}

//检查是否有接收到的UDP数据，若有的话，进行处理；
//Check whether data arrival or not
int EthCommCheck(void)
{
	struct sockaddr_in from;
	char buf[1032];
	int cmd; 
	unsigned int fromlen=sizeof(from);
	//Maximize data packet size is 1032 
	memset(buf, 0, sizeof(buf));
	int len = recvfrom(comm_socket, buf, 1032, 0, (struct sockaddr*)&from, &fromlen);
	if(len>0){
		//把数据包交由RunCommand处理
		//RunCommand be called to process data packet
		if ((cmd = RunCommand(buf, len, TRUE)) == 0) return -2;  
		//如果是请求建立连接命令，则建立连接会话，并保存通讯参数
		//the client ask to make a connection session 
		if (cmd == CMD_CONNECT){
			extern int RTLogNum;
			char Sender[SENDER_LEN];		 
			PCmdHeader chdr=(PCmdHeader)buf;
			
			//生成通讯参数并创建通讯会话
			//也即在SendUDPData中，可以借此参数获得对方的ip地址和端口号，才能发送数据
			memset(Sender, 0, SENDER_LEN);
			memcpy(Sender, (void*)&from, sizeof(struct sockaddr));
			PCommSession session=CreateSession(Sender);
			
			RTLogNum=0;
			//向对方发送连接成功的会话标识ID
			chdr->SessionID=session->SessionID;
			chdr->CheckSum=0;
			chdr->CheckSum=in_chksum((unsigned char *)buf, sizeof(TCmdHeader));
			session->Send=SendUDPData;
			session->Speed=10*1024*1024;
			//注意session->Sender作为保存的通讯参数，在以后的通讯过程中向对方发送数据时使用，
			SendUDPData(chdr, sizeof(TCmdHeader), session->Sender); 
		}               
	}
	return 0;
}

int EthBoradcastCheck(void)
{
	struct sockaddr_in from;
	char buf[128];
	unsigned int fromlen=sizeof(from);
	//Maximize data packet size is 1032 
	int len = recvfrom(receive_broadcast_socket, buf, 128, 0, (struct sockaddr*)&from, &fromlen);
	if(len>0)
	{
		if(buf[0]=='X')
		{
			sprintf(buf, "X%s/%s", LoadStrOld("MAC"), LoadStrOld("IPAddress"));
			DBPRINTF("Yes! I got message for X then response %s len=%d\n", buf, strlen(buf));
			return sendto(comm_socket, buf, strlen(buf)+1, 0, (struct sockaddr*)&from, sizeof(struct sockaddr_in));
		}
	}
	return 0;
}


int InitUDPSocket(U16 port, int *udpsocket) 
{
	struct sockaddr_in sin;
	long save_file_flags;
	
	//Initialize socket address structure for internet protocol
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

void ForceClearMACCacheIP(unsigned char *ipaddress)
{
	char buffer[128];
	struct sockaddr_in pin;
	char msg[128];
	int i;
    
	if(ipaddress[0])
	{
		bzero(&pin, sizeof(pin));
		sprintf(buffer, "%d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
		pin.sin_family=AF_INET;
		pin.sin_addr.s_addr=inet_addr(buffer);
		pin.sin_port=htons(THEUDP+1);
		if(gOptions.StartUpNotify&2)
		{
			sprintf(msg, "\"%s-%s-%d\" Started.", DeviceName, SerialNumber, gOptions.DeviceID);
			for(i=0;i<5;i++)
				sendto(comm_socket, msg, strlen(msg), 0, (struct sockaddr*)&pin, sizeof(struct sockaddr_in));
		}
	}
}

//初始化UDP通讯，一般是建立通讯的UDP socket.
//Initilization UDP 
int EthInit(void)
{
	//struct sockaddr_in sin;
	
	if(InitUDPSocket(THEUDP, &comm_socket)!=0) return -1;
	if(InitUDPSocket(THEUDP+1, &listenAuthServer_socket)!=0) return -1;
	if(InitUDPSocket(BROADCASTPORT, &receive_broadcast_socket)!=0) return -1;
	//Force gateway router clear cached IP Address infomation
	ForceClearMACCacheIP(gOptions.GATEIPAddress);	
	
	if(InitTCPSocket()!=0)
	{
		return -1;
	}
	/*
	//create a tcp socket for monitor connection from clients
	if ((server_socket=socket(AF_INET, SOCK_STREAM, 0))==-1) return -1;
	//bind tcp socket with current parameter
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(gOptions.TCPPort);
	if (bind(server_socket, (struct sockaddr *)&sin, sizeof(sin))==-1) return -1; 
	if (listen(server_socket, 1)==-1) return -1;
	FD_ZERO(&rfds);
	FD_SET(server_socket, &rfds);
	tv.tv_sec=0;
	tv.tv_usec=0;
	*/
	return 0; 
}

void EthFree(void)
{
	if (comm_socket>0) close(comm_socket);
	//if (server_socket>0) close(server_socket);	
	if (listenAuthServer_socket>0) close(listenAuthServer_socket);
	if (receive_broadcast_socket>0) close(receive_broadcast_socket);
}

/*
int ethernet_cmd(const char *ifname, u16 speed, u8 duplex)
{
	struct ethtool_cmd cmd;
	 struct ifreq ifr;
	int s, i;
	char *media[] = {
		"10BaseT-HD ", "10BaseT-FD ","100baseTx-HD ",
		"100baseTx-FD", "100baseT4", 0
	};
	
	s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (s == -1) {
		return 0;
	}
	strcpy(ifr.ifr_name, ifname);
	
	ifr.ifr_data = (__caddr_t )&cmd;
	
	cmd.cmd = ETHTOOL_GSET;
	
	if (ioctl(s, SIOCETHTOOL, &ifr) == -1) {
		return 0;
	}
	
	DBPRINTF("Provide Mode All: ");
	for (i=0;i<5;i++)
	    if (cmd.advertising & (1<<i))
		DBPRINTF("(%d)%s",i+1,media[i]);
	DBPRINTF("\n");
	
	cmd.cmd = ETHTOOL_SSET;
	cmd.speed= speed;
	cmd.duplex= duplex;
	cmd.autoneg=(speed==0)?AUTONEG_ENABLE:AUTONEG_DISABLE;	
	
	if (ioctl(s, SIOCETHTOOL, &ifr) == -1) {
		return 0;
	}
	
	return 1;
}
*/


/*----------------------Auth Server------------------------ */

BOOL ConnectWithTimeout(int sockfd, const struct sockaddr *serv_addr,socklen_t addrlen, int timeout)
{	
	struct timeval tm;
	fd_set set;
	BOOL ret = FALSE;
	int error=-1, len;
	long save_file_flags;
	int retcount=0;
	
	save_file_flags=fcntl(sockfd, F_GETFL);
	//set the NOBLOCK flags to file descriptor
	fcntl(sockfd, F_SETFL, save_file_flags|O_NONBLOCK);
	if(connect(sockfd, serv_addr, addrlen)==-1)
	{
		tm.tv_sec  = timeout;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
#if 0
		if(select(sockfd+1, NULL, &set, NULL, &tm)>0)
		{
			len=sizeof(int);
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if(error==0) 
				ret=TRUE;
			else 
				ret=FALSE;
			DBPRINTF("Data arrival! and Error=%d\n", error);
		} 
		else
		{ 
			ret=FALSE;
			DBPRINTF("select timeout!\n");
		}
#endif
#if 1
                //连接不成功重试200次 特别是gprs连接时 --2008.04.03
                while(retcount<=200)
                {
                        if(select(sockfd+1, NULL, &set, NULL, &tm)>0)
                        {
                                len=sizeof(int);
                                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
                                if(error==0)
                                {
                                        ret=TRUE;
                                        break;
                                }
                                else
                                {
                                        retcount++;
                                        ret=FALSE;
                                }
                                DBPRINTF("Data arrival! and Error=%d\n", error);
                        }
                        else
                        {
                                ret=FALSE;
                                //DBPRINTF("select timeout!\n");
                                retcount++;
                        }
                }
#endif
	}
	else 
		ret=TRUE;
	
	//restore old file status flags
	fcntl(sockfd, F_SETFL, save_file_flags);
	
	return ret;
}

extern DWORD GetTickCount();
BOOL SendDataToAuthServer(U16 Command, void *Header, U16 HeaderLen, 
			  void *Data, U32 DataLen)
{
	int totalSent=0;
	int byteSent=0;	
	U32 size;
	unsigned int s_msec, e_msec;
	
	size=sizeof(U16)+HeaderLen+DataLen;	
//	DBPRINTF("Data sending......\n");
	s_msec=GetTickCount();       
	//Send Size Command Buffer(Header+Data)
	if((send(image_socket, &size, sizeof(U32), MSG_NOSIGNAL)==sizeof(U32))&&
	   (send(image_socket, &Command, sizeof(U16), MSG_NOSIGNAL)==sizeof(U16))&&
	   ((Header==NULL)||(send(image_socket, Header, HeaderLen, MSG_NOSIGNAL)==HeaderLen)))
	{
		e_msec=GetTickCount();
	//	DBPRINTF("Packet Header Sending FINISHED!time=%d\n", e_msec-s_msec);
		if(Data)
		{
			do
			{
			//	DBPRINTF("Sending Data....\n");
				byteSent=send(image_socket, (char *)Data+totalSent, DataLen-totalSent, MSG_NOSIGNAL);
			//	DBPRINTF("Sending Data....DataLen=%d byteSent=%d totalSent=%d\n", DataLen, byteSent, totalSent);
				if (byteSent==-1) break;
				totalSent+=byteSent;
			}while(totalSent<DataLen);
			e_msec=GetTickCount();
	//		DBPRINTF("Packet Data sending FINISHED!time=%d Data=%d\n", e_msec-s_msec, totalSent);
			if (totalSent==DataLen)
			{
//				DBPRINTF("Packet Data have already sent to server successfuly!\n");
				return TRUE;
			}
		}
		else
		{
//			DBPRINTF("Packet No Data!\n"); 
			return TRUE; 
		}
	}
	else
		DBPRINTF("Packet Header sending FAILED!\n"); 
	
	return FALSE;
}

BOOL GetDataFromAuthServer(void *AuthResult, U32 Len)
{
	U32 size;
	unsigned int s_msec, e_msec;
	void *buffer;
	
//	DBPRINTF("Receiving......\n");
	s_msec=GetTickCount();       	
	if(recv(image_socket, &size, sizeof(U32), 0)==sizeof(U32))
	{
		DBPRINTF("Data reply Len=%d!\n", size);
		memset(AuthResult, 0, Len);
		if(size==0)
		{
			return FALSE;
		}
		if(size>Len)
		{
			buffer=MALLOC(size);
			if(recv(image_socket, buffer, size, 0)==size)
			{
				memcpy(AuthResult, buffer, Len);
				e_msec=GetTickCount();
				DBPRINTF("Data received!time=%d\n", e_msec-s_msec);	
				return TRUE;
			}
			FREE(buffer);			
		}
		else
		{
			if(recv(image_socket, AuthResult, size, 0)==size)
			{
				e_msec=GetTickCount();
				DBPRINTF("Data received!time=%d, pin = %u\n", e_msec-s_msec, ((PFPResult)AuthResult)->PIN);	
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL PrcocessCMD(int sockfd, const struct sockaddr *serv_addr,
		 U16 Command, void *Header, U16 HeaderLen, void *Data, U32 DataLen,
		 void *AuthResult, U32 Len)
{//Liaozz 5 to 20 20081013
	return (ConnectWithTimeout(sockfd, (void *)serv_addr, sizeof(*serv_addr), 20)&&
		SendDataToAuthServer(Command, Header, HeaderLen, Data, DataLen)&&
		GetDataFromAuthServer(AuthResult, Len));
}

BOOL ExecuteCMD(U16 Command, void *Header, U16 HeaderLen, void *Data, U32 DataLen,void *AuthResult, U32 Len)
{
	struct sockaddr_in pin;
	int i;
	BOOL rc=FALSE;
	
	if(!CheckAllAuthServerRuning()) return FALSE;
	
	bzero(&pin, sizeof(pin));
	pin.sin_family=AF_INET;
	pin.sin_port=htons(gOptions.RISServerTCPPort);
	
	if(image_socket!=-1) close(image_socket);
	image_socket=socket(AF_INET, SOCK_STREAM, 0);
	if(image_socket==-1)
		DBPRINTF("Image socket create error!\n");
	
	switch(Command)
	{
	case AUTH_CMD_IMAGE_IDENTIFY:
	case AUTH_CMD_ACQUIRE_DNSADDRS:
	case AUTH_CMD_UPLOAD_IDINFO:
	case AUTH_CMD_UPLOAD_ATTLOG:
	case AUTH_CMD_UPLOAD_PHOTO:
	case AUTH_CMD_REFRESH_USERDATA:
		for(i=0;i<AuthServerSessionCount;i++)
		{
			if(AuthServerSessions[i].Connected)
			{
				pin.sin_addr.s_addr=AuthServerSessions[i].sin.sin_addr.s_addr;
				//create a send tcp socket for auth server
				if(image_socket!=-1) close(image_socket);
				image_socket=socket(AF_INET, SOCK_STREAM, 0);
				if(image_socket==-1)
				{
					DBPRINTF("Image socket create error!\n");
					break;
				}
				else
				{
					
					if(PrcocessCMD(image_socket, (struct sockaddr *)&pin,Command, Header, HeaderLen, 
							Data, DataLen,AuthResult, Len))
					{
						rc=TRUE;
						break;
					}
					close(image_socket);
					image_socket=-1;
				}
			}
		}
		break;
	case AUTH_CMD_FINGER_REGISTER:
	case AUTH_CMD_ACQUIRE_FREEID:
	case AUTH_CMD_CHECK_ISIDLEID:
	case AUTH_CMD_SYNC_LOCALTIME:
		pin.sin_addr.s_addr=AuthServerSessions[0].sin.sin_addr.s_addr;
		if(PrcocessCMD(image_socket, (struct sockaddr *)&pin, 
			       Command, Header, HeaderLen, Data, DataLen,
			       AuthResult, Len))
			rc=TRUE;
		break;
	default:
		break;
	}	
	if(image_socket!=-1)
	{
		close(image_socket);
		image_socket=-1;
	}
	return rc;
}

/*-------------------AuthServer system fucntions-------------------*/

int CheckAuthServerSessionTimeOut(int TimeOutSec, int mode)
{
	int i;

	if(mode==AUTH_GSM) return 0;

	for(i=0;i<AuthServerSessionCount;i++)
	{
		AuthServerSessions[i].LastActive++;
		if(AuthServerSessions[i].LastActive>TimeOutSec)
		{
			if(AuthServerSessions[i].Connected)
			{
				AuthServerSessions[i].Connected=FALSE;
			}
			AuthServerSessions[i].LastActive=0;
			DBPRINTF("AuthServer %s is Down! TimeOut\n", inet_ntoa(AuthServerSessions[i].sin.sin_addr));
		}
	}
	return 0;
}

int EthAuthServerCheck(int mode)
{
	struct sockaddr_in from;
	char buf[1032];
	unsigned int fromlen=sizeof(from);
	int len, i;	

	if(mode==AUTH_GSM) return 0;
	
	len=recvfrom(listenAuthServer_socket, buf, 1032, 0, (struct sockaddr*)&from,&fromlen);
	if(len>0)
	{
		DBPRINTF("REQ Received\n");
		for(i=0;i<AuthServerSessionCount;i++)
		{
			if(memcmp(&from.sin_addr, &(AuthServerSessions[i].sin.sin_addr), sizeof(from.sin_addr))==0)
			{
				AuthServerSessions[i].Connected=TRUE;
				AuthServerSessions[i].LastActive=0;
				DBPRINTF("AuthServer %s is Up!\n", inet_ntoa(AuthServerSessions[i].sin.sin_addr));
			}
		}		
	}
	return 0;
}

int AuthServerREQ(int TimeInterval, int mode)
{
	int i;
	
	for(i=0;i<AuthServerSessionCount;i++)
	{
		if((EncodeTime(&gAuthServerTime)-AuthServerSessions[i].LastREQ)>TimeInterval)
		{
			switch(mode)
			{
				case AUTH_LOCAL:
				{	
					sendto(comm_socket, "REQ", 3, 0, 
						(struct sockaddr*)&(AuthServerSessions[i].sin), sizeof(struct sockaddr_in));
					AuthServerSessions[i].LastREQ=EncodeTime(&gAuthServerTime);
					DBPRINTF("AuthServer %s REQ is sending!\n", inet_ntoa(AuthServerSessions[i].sin.sin_addr));
					break;
				}
				case AUTH_GSM:
				{
					AuthServerSessions[i].LastREQ=EncodeTime(&gAuthServerTime);
					AuthServerSessions[i].LastActive=0;

					if(systemEx("ifconfig ppp0")==EXIT_SUCCESS)
					{
						AuthServerSessions[i].Connected=TRUE;
						DBPRINTF("AuthServer %s is Up!\n", inet_ntoa(AuthServerSessions[i].sin.sin_addr));
					}
					else
					{
						AuthServerSessions[i].Connected=FALSE;
						DBPRINTF("AuthServer %s is Down!\n", inet_ntoa(AuthServerSessions[i].sin.sin_addr));
					}
					break;
				}	
			}
		}
	}
	return 0;
}

int InitAuthServer(void)
{
	int i;
	char *hostname;
	char tmp[128];
	struct sockaddr_in pin;
	void *newp;
	BYTE ip[16];

	//if(gOptions.IsSupportModem>1 && gOptions.AuthServerIPType) setup_auth_dns();       //lm
	
	if (AuthServerSessions) FREE(AuthServerSessions);
	AuthServerSessionCount=0;
	MaxAuthServerSessionCount=0;
	i=0;
	StaticDNSCount=0;
	while(TRUE)
	{
		if(i==0)
			sprintf(tmp, "AuthServerIP");
		else
			sprintf(tmp, "AuthServerIP%d", i);
		hostname=LoadStrOld(tmp);
		if(hostname)
		{
			if((str2ip(hostname, ip)==0)&&(ip[0]!=0))
			{
				if((AuthServerSessionCount+1)>MaxAuthServerSessionCount)
				{
					MaxAuthServerSessionCount+=5;
					newp=MALLOC(sizeof(TAuthServerSession)*MaxAuthServerSessionCount);
					if(AuthServerSessionCount)
					{
						memcpy(newp, AuthServerSessions, AuthServerSessionCount*sizeof(TAuthServerSession));
						FREE(AuthServerSessions);
					}
					AuthServerSessions=newp;
				}			
				bzero(&pin, sizeof(pin));
				pin.sin_family=AF_INET;
				pin.sin_addr.s_addr=inet_addr(hostname);
				pin.sin_port=htons(THEUDP+1);
				memcpy(&(AuthServerSessions[i].sin), &pin, sizeof(pin));
				AuthServerSessions[i].LastActive=0;
				AuthServerSessions[i].LastREQ=EncodeTime(&gAuthServerTime)-10;
				AuthServerSessions[i].Connected=FALSE;
				AuthServerSessionCount++;
			}
		}
		else break;
		i++;
	}
	StaticDNSCount=AuthServerSessionCount;
	return 0;
}

int FreeAuthServer(void)
{
	FREE(AuthServerSessions);
	AuthServerSessionCount=0;
	return 0;
}

BOOL CheckAllAuthServerRuning(void)
{
	int i;

	for(i=0;i<AuthServerSessionCount;i++)
	{
		if(AuthServerSessions[i].Connected) return TRUE;
	}
	return FALSE;
}

/*-----------Auth Server application function--------------*/

BOOL SendImageAndIdentify(char *buffer, int size, U32 pin2, PFPResult result)
{
	TFPIdentify header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=pin2;
	header.ImageSize=size;
	
	return ExecuteCMD(AUTH_CMD_IMAGE_IDENTIFY, (void *)&header, sizeof(header), buffer, size, (void *)result, sizeof(TFPResult));
}

BOOL SendImageToRegister(char *buffer, int size, U32 pin2, U8 Index, U16 *result)
{
	TFPRegister header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=pin2;
	header.ImageSize=size;
	header.Index=Index;
	
	return ExecuteCMD(AUTH_CMD_FINGER_REGISTER, (void *)&header, sizeof(header), buffer, size, (void *)result, sizeof(U16));
}

BOOL GetFreeIDFromAuthServer(U32 *result)
{
	BYTE DeviceID;
		
	DeviceID=gOptions.DeviceID;
	return ExecuteCMD(AUTH_CMD_ACQUIRE_FREEID, (void *)&DeviceID, sizeof(U8), NULL, 0, (void *)result, sizeof(U32));
}

BOOL CheckIsIdleIDFromAuthServer(U32 pin2, U8 *result)
{
	TFPID header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=pin2;
	
	return ExecuteCMD(AUTH_CMD_CHECK_ISIDLEID, (void *)&header, sizeof(header), NULL, 0, (void *)result, sizeof(U8));	
}

BOOL GetDNSAddrsFromAuthServer(U8 *result, int len)
{
	BYTE DeviceID;
		
	DeviceID=gOptions.DeviceID;
	return ExecuteCMD(AUTH_CMD_ACQUIRE_DNSADDRS, (void *)&DeviceID, sizeof(U8),NULL, 0, (void *)result, len);
}

BOOL UploadIDCardToAuthServer(U32 cardnumber, PFPResult result)
{
	TFPID header;
	
	printf("UploadIDCardToServer:%d\n",cardnumber);
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=cardnumber;
	
	return ExecuteCMD(AUTH_CMD_UPLOAD_IDINFO, (void *)&header, sizeof(header), NULL, 0, (void *)result, sizeof(TFPResult));	
}

BOOL UploadAttlogToAuthServer(char *buffer, int size, U8 *result)
{
	TFPID header;
	
	header.MachineNumber=gOptions.DeviceID;
	header.PIN=size;
	
	return ExecuteCMD(AUTH_CMD_UPLOAD_ATTLOG, (void *)&header, sizeof(header), buffer, size, (void *)result, sizeof(U8));	
}

BOOL GetUserDataFromAuthServer(U8 *result, int len)
{
	BYTE DeviceID;
		
	DeviceID=gOptions.DeviceID;
	return ExecuteCMD(AUTH_CMD_REFRESH_USERDATA, (void *)&DeviceID, sizeof(U8),(void *)&len, 4, (void *)result, len);
}

int RefreshAuthServerByDNS(U8 *result, int len)
{
	char tmp[128];
	struct sockaddr_in pin;
	void *newp;
	U8 *p;
	int i;
	BOOL bSign;
	
	AuthServerSessionCount=StaticDNSCount;
	p=result;
	while(TRUE)
	{
		if(!p[0]) break;
		sprintf(tmp, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
		DBPRINTF("%s\n", tmp);
		if(tmp)
		{		
			if((AuthServerSessionCount+1)>MaxAuthServerSessionCount)
			{
				MaxAuthServerSessionCount+=5;
				newp=MALLOC(sizeof(TAuthServerSession)*MaxAuthServerSessionCount);
				if(AuthServerSessionCount)
				{
					memcpy(newp, AuthServerSessions, AuthServerSessionCount*sizeof(TAuthServerSession));
					FREE(AuthServerSessions);
				}
				AuthServerSessions=newp;
			}			
			bzero(&pin, sizeof(pin));
			pin.sin_family=AF_INET;
			pin.sin_addr.s_addr=inet_addr(tmp);
			pin.sin_port=htons(THEUDP+1);
			
			bSign=TRUE;
			//check whether exist or not
			for(i=0;i<StaticDNSCount;i++)
			{
				if(memcmp(&pin.sin_addr, &(AuthServerSessions[i].sin.sin_addr), sizeof(pin.sin_addr))==0)
				{
					bSign=FALSE;
					break;
				}
			}	
			if(bSign)
			{
				memcpy(&(AuthServerSessions[AuthServerSessionCount].sin), &pin, sizeof(pin));
				AuthServerSessions[AuthServerSessionCount].LastActive=0;
				AuthServerSessions[AuthServerSessionCount].LastREQ=EncodeTime(&gAuthServerTime)-10;
				AuthServerSessions[AuthServerSessionCount].Connected=FALSE;
				AuthServerSessionCount++;
			}
		}
		//next dns ip address
		if((p-result+4)>len) break;				
		p+=4;
	}
	return 0;
}

void RefreshAuthServerListFromAuthServer(void)
{
	const int MaxAuthDNSCount=15; 
	U8 buffer[MaxAuthDNSCount*4];
	
	if(GetDNSAddrsFromAuthServer(buffer, MaxAuthDNSCount*4))
		RefreshAuthServerByDNS(buffer, MaxAuthDNSCount*4);
}

void UploadAttLogByAuthServer(void)
{
	char *buffer;
	int size;
	BYTE ret=0;
	
	if(CheckAllAuthServerRuning())
	{			
		buffer=FDB_ReadBlock(&size, FCT_ATTLOG);
		if(buffer)
		{
			buffer[3]=gOptions.AttLogExtendFormat;
			if(UploadAttlogToAuthServer(buffer+3, size-3, &ret)&&ret) FDB_ClearData(FCT_ATTLOG);
			FREE(buffer);
		}
	}
}

BOOL SendPhotoToAuthServer(U32 pin, char *buffer, U16 width, U16 height, U8 bpp, U8 type)
{
        TPhotoSize header;
        BYTE result;
        int size;

        header.MachineNumber=gOptions.DeviceID;         header.PIN=pin;
        header.Width=width;
        header.Height=height;         header.BPP=bpp;
        header.Type=type;
        size=width*height*bpp;

        return (ExecuteCMD(AUTH_CMD_UPLOAD_PHOTO, (void *)&header, sizeof(header),
                          buffer, size, (void *)&result, sizeof(U8))&&result);
}


BOOL RefreshUserDataFromAuthServer(void)
{
	//dsl 2012.3.23.Anywhere dont use this function.
#if 0
	while(TRUE)
	{
#ifdef URU
		if(GetUserDataFromAuthServer(gImageBuffer, 2*URU_IMAGE_SIZE))
#else
		if(GetUserDataFromAuthServer((U8 *)gImageBuffer, 5*gOptions.OImageWidth*gOptions.OImageHeight))
#endif
		{
			BatchOPUserData((BYTE *)gImageBuffer+1);
			if(*((BYTE *)gImageBuffer)==0) return TRUE;
		}
		else
			break;
	}
#endif
	return FALSE;
}

BOOL GetTimeFromAuthServer(U32 *result)
{
        BYTE DeviceID;

        DeviceID=gOptions.DeviceID;
        return ExecuteCMD(AUTH_CMD_SYNC_LOCALTIME, (void *)&DeviceID, sizeof(U8),
                          NULL, 0, (void *)result, sizeof(U32));
}
BOOL SyncLocalTimeFromAuthServer()
{
	time_t newtm;
	U32 tmptm;
	TTime ltm;
	if (GetTimeFromAuthServer(&tmptm))	
	{
		newtm = tmptm;
		if (newtm > 0) {
			OldDecodeTime(newtm, &ltm);
			SetTime(&ltm);
			return TRUE;
		} else {
			return FALSE;
		}
	}
	return FALSE;
}

void UploadAttLogByAuthServer_gprs(void)	//lm
{
        char *buffer;
        int size;
        BYTE ret=0;
        static int upload_retry=0;
        char tmp_buf[1024*8+3];
        int sent_total_size=0;
        int sent_size=0;

        if(CheckAllAuthServerRuning())
        {
                buffer=FDB_ReadBlock(&size, FCT_ATTLOG);
                if(buffer)
                {
                        size = size-4;
                        printf("size = %d\n", size);

                        while(sent_total_size< size)
                        {
                                if((sent_total_size+sizeof(TAttLog)*200)<size)
                                        sent_size = sizeof(TAttLog)*200;
                                else
                                        sent_size= size - sent_total_size;

                                tmp_buf[0]=(char)2;//gOptions.AttLogExtendFormat;
                                memcpy(tmp_buf+1, buffer+4+sent_total_size, sent_size);

retry_sent_attlog:
                                ret=0;
                                if(UploadAttlogToAuthServer(tmp_buf, sent_size+1 ,&ret)&&ret)//if upload Ok we clear the alltlog 
                                {
                                        upload_retry=0;
                                }
                                else    //if upload fail we need judge if the network is online still???
                                {
                                        //printf("fail upload to server\n");
                                        upload_retry++;
                                        if(upload_retry>=gOptions.RedialCount)//if transaction threes times fail we can decide that network is unreachalbled, so we redial network
                                        {
                                                upload_retry = 0;
                                                if(gOptions.AuthServerCheckMode == AUTH_GSM)
                                                {
                                                        systemEx("killall pppd"); //we must redial ppp
                                                }
//                                              LCDWriteCenterStr(1, LoadStrByID(MID_GPRS_UPLOADFAIL));
                                                sleep(1);
                                                break;
                                        }
                                        else
                                                goto retry_sent_attlog;
                                }
                                sent_total_size += sent_size;
                                printf("sent_size = %d\n", sent_size);
                                printf("sent_total_size = %d\n", sent_total_size);
                        }
                        if(sent_total_size == size)
                                FDB_ClearData(FCT_ATTLOG);

                        FREE(buffer);
                }
        }
}

int AuthServerconncetset(void)  //ccc
{
        int i;
        for(i=0;i<AuthServerSessionCount;i++)
        {
                AuthServerSessions[i].LastREQ=EncodeTime(&gAuthServerTime);
                AuthServerSessions[i].LastActive=0;
                AuthServerSessions[i].Connected=TRUE;
        }
        return 0;
}

//-------------- PUSH_SDK ------------------//

#include "traceroute.h"
int icmp_echo()
{
	printf("icmp_echo \n");
	register int code;
	//register char *cp;
	register u_char *outp;
	//register u_int32_t *ap;
	register struct sockaddr_in *from = (struct sockaddr_in *)&wherefrom;
	register struct sockaddr_in *to = (struct sockaddr_in *)&whereto;
	//register struct hostinfo *hi;
	int on = 1;
	//register struct protoent *pe;
	register int ttl, probe, i;
	register int seq = 0;
	int tos = 0, settos = 0;
	//register int lsrr = 0;
	register u_short off = 0;
	
	char ipaddr[64];
	if(gOptions.IclockSvrFun)
	{
		if(gOptions.EnableProxyServer)
		{
			ipformat(ipaddr,gOptions.ProxyServerIP);
			setsin(to,inet_addr(ipaddr));
		}
		else
		{
			setsin(to, inet_addr((const char*)ipformat(ipaddr,gOptions.WebServerIP)));
		}
	}
	else
		setsin(to, inet_addr("202.96.134.134"));	
	
	prog = "traceroute";

	first_ttl =1;					/*[-f first_ttl]*/
	max_ttl =  64;
		

	
	minpacket = sizeof(*outip) + sizeof(*outdata) + optlen;
	packlen = minpacket + sizeof(*outudp);
	

	
	outip = (struct ip *)MALLOC((unsigned)packlen);
	if (outip == NULL) 
	{
		ERREXIT( "%s: malloc: %s\n", prog, strerror(errno));		
	}

	memset((char *)outip, 0, packlen);

	outip->ip_v = IPVERSION;

	if (settos)
		outip->ip_tos = tos;

	outip->ip_len = packlen;
	outip->ip_off = off;//=0;


	outp = (u_char *)(outip + 1);		/*相当于outp指向结构outip[1]*/

	
	outip->ip_dst = to->sin_addr;

	/*因为outp - (u_char *)outip是以char为单位的个数，即多少个字节，比如20>>2=5*/
	outip->ip_hl = (outp - (u_char *)outip) >> 2;

	ident = (getpid() & 0xffff) | 0x8000;


	outip->ip_p = IPPROTO_UDP;

	outudp = (struct udphdr *)outp;
	
	outudp->len = htons((u_short)(packlen - (sizeof(*outip) + optlen)));	/*=sizeof(*outudp)+sizeof(*outdata)*/

	outdata = (struct outdata *)(outudp + 1);



	/*建立接收icmp数据报的socket*/
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) 		
			ERREXIT("%s: icmp socket: %s\n", prog, strerror(errno));
		

	/*建立发送udp数据报的socket*/
	if ((sndsock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) <0)		
			ERREXIT("%s: raw socket: %s\n", prog, strerror(errno));
		

	/*设置IP_HDRINCL以自己填充IPv4首部,亲自对IP头进行处理*/
	if (setsockopt(sndsock, IPPROTO_IP, IP_HDRINCL, (char *)&on,  sizeof(on)) < 0) 		
			ERREXIT("traceroute: IP_HDRINCL");

	

	outip->ip_src = from->sin_addr;

	if (bind(sndsock, (struct sockaddr *)from, sizeof(*from)) < 0) 
			ERREXIT("%s: bind: %s\n", prog, strerror(errno));



	printf("%s to %s (%s)", prog, hostname, inet_ntoa(to->sin_addr));
	printf(", %d hops max, %d byte packets\n", max_ttl, packlen);


	for (ttl = first_ttl; ttl <= max_ttl; ++ttl)
	{
		u_int32_t lastaddr = 0;
		int gotlastaddr = 0;
		int got_there = 0;
		int unreachable = 0;
		int sentfirst = 0;

		for (probe = 0; probe < nprobes; ++probe) 
		{
			register int cc;
			struct timeval t1, t2;
			struct timezone tz;
			register struct ip *ip;

			(void)gettimeofday(&t1, &tz);

			send_probe(++seq, ttl, &t1);

			++sentfirst;

			while ((cc = wait_for_reply(s, from, &t1)) != 0) 
			{
				(void)gettimeofday(&t2, &tz);

				i = packet_ok(packet, cc, from, seq);

				/* Skip short packet */
				if (i == 0)
					continue;

				if (!gotlastaddr ||	from->sin_addr.s_addr != lastaddr) 
				{
					print(packet, cc, from);
					lastaddr = from->sin_addr.s_addr;
					++gotlastaddr;
				}

				printf("  %.3f ms", deltaT(&t1, &t2));

				/* time exceeded in transit */
				if (i == -1)
					break;
				code = i - 1;
			
				switch (code) 
				{

					case ICMP_UNREACH_PORT:
#ifndef ARCHAIC
						ip = (struct ip *)packet;
						if (ip->ip_ttl <= 1)
							printf(" !");
#endif
						++got_there;
						break;

					case ICMP_UNREACH_NET:
						++unreachable;
						printf(" !N");
						break;

					case ICMP_UNREACH_HOST:
						++unreachable;
						printf(" !H");
						break;

					case ICMP_UNREACH_PROTOCOL:
						++got_there;
						printf(" !P");
						break;

					case ICMP_UNREACH_NEEDFRAG:
						++unreachable;
						//printf(" !F-%d", pmtu);
						break;

					case ICMP_UNREACH_SRCFAIL:
						++unreachable;
						printf(" !S");
						break;

					case ICMP_UNREACH_FILTER_PROHIB:
						++unreachable;
						printf(" !X");
						break;

					case ICMP_UNREACH_HOST_PRECEDENCE:
						++unreachable;
						printf(" !V");
						break;

					case ICMP_UNREACH_PRECEDENCE_CUTOFF:
						++unreachable;
						printf(" !C");
						break;

					default:
						++unreachable;
						printf(" !<%d>", code);
						break;
				}
				break;
			}/*end while*/
			close(sndsock);
			return 1;
			if (cc == 0)
				printf(" *");
			
			
		}/*end for (probe = 0; probe < nprobes; ++probe)*/

		putchar('\n');

		if (got_there ||(unreachable > 0 && unreachable >= nprobes - 1))
			break;

	}/*end for (ttl = first_ttl; ttl <= max_ttl; ++ttl)*/

	close(sndsock);
	return 0;

}

int udp_echo()
{
	printf("udp echo\n");
	return 1;
}

int tcp_echo()
{
	int sockFD = socket(AF_INET,SOCK_STREAM,0);
	struct  sockaddr_in addr;
	int ret = 0;
	char ipaddr[64];
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	if(gOptions.IclockSvrFun)
	{
		if(gOptions.EnableProxyServer)
		{
			ipformat(ipaddr,gOptions.ProxyServerIP);
			addr.sin_addr.s_addr = inet_addr(ipaddr);
			addr.sin_port = htons(gOptions.ProxyServerPort);
		}
		else
		{
			addr.sin_addr.s_addr = inet_addr(ipformat(ipaddr,gOptions.WebServerIP));
			addr.sin_port = htons(gOptions.WebServerPort);
		}
	}
	//printf("___________TCP ECHO___IP:%s port:%d __________\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
	ret =  ConnectWithTimeout(sockFD, (struct sockaddr *)&addr,sizeof(addr),20);
//_exit:
	if(sockFD > 0)
		close(sockFD);
	//printf("_________ TCP Echo return %d ________\n",ret);
	return ret;	
}

//PUSH_SDK
