#ifndef __TCPCOMM__
#define __TCPCOMM__

#define MAX_BUFFER_LEN_TCP	(64*1024)
#define CMD_TCP	0x82
typedef struct _TCPHeader_{
	unsigned char HeaderTag[2];
	unsigned char Cmd;
	unsigned char CmdCheck;
	unsigned int Len;
}GCC_PACKED TTCPHeader, *PTCPHeader;

int CloseTCPSocket(void *param);
int SendTCPData(void *buf, int size, void *param);
int ProcessTCPPackage(void *buf, int len, int tmp_server_socket);
int EthCommTCPCheck(void);
int InitTCPSocket();
#endif
