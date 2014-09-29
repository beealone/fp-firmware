/*************************************************
                                           
 ZEM 200                                          
                                                    
 net.h UDP protocol for communcion                               
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      
 
 $Log: net.h,v $
 Revision 5.7  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.6  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.5  2005/07/14 16:59:50  david
 Add update firmware by SDK and U-disk
                                               
*************************************************/

#ifndef ZK_NET_H
#define ZK_NET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "ccc.h"
#include "arca.h"

//command for data management
#define AUTH_CMD_IMAGE_IDENTIFY		1	/* identification from auth server */
#define AUTH_CMD_FINGER_REGISTER	2	/* register fingerprint on auth server*/
#define AUTH_CMD_ACQUIRE_FREEID		3	/* Acquire Free ID for register*/
#define AUTH_CMD_CHECK_ISIDLEID		4	/* Check ID is free or not*/

#define AUTH_CMD_ACQUIRE_DNSADDRS	5	/* Get region Auth Server IP address*/
#define AUTH_CMD_UPLOAD_IDINFO 		6	/* Send HID card info to Server */
#define AUTH_CMD_UPLOAD_ATTLOG		7	/* Send attlog record to Server */
#define AUTH_CMD_UPLOAD_PHOTO 		8	/* Send a photo to Server */
#define AUTH_CMD_REFRESH_USERDATA	9	/* Update user data from Authserver */
#define AUTH_CMD_SYNC_LOCALTIME	10	/* Sync local time form Authserver */

#define AUTH_LOCAL	0
#define AUTH_GSM	1
/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,start*/
#define AUTH_PUSH	2	//后台验证模式，pushsdk通信方式
/*PUSH_SERVER_AUTH,add by yangxiaolong,2011-05-31,start*/

enum ETH_PHY_mode {
        ETH_10MHD   = 0,
        ETH_100MHD  = 1,
        ETH_10MFD   = 4,
        ETH_100MFD  = 5,
        ETH_AUTO    = 8,
        ETH_1M_HPNA = 0x10
};

typedef struct _authserver_session_{
	struct sockaddr_in sin;
	U32 LastActive;
	U32 LastREQ;
	BOOL Connected;
}TAuthServerSession, *PAuthServerSession;

typedef struct _FPIdentify_{
	BYTE MachineNumber;
	U32 PIN;
	U32 ImageSize;
}GCC_PACKED TFPIdentify, *PFPIdentify;

typedef struct _FPResult_{
	U32 PIN;
	char Name[20];
	char Group[20];
}GCC_PACKED TFPResult, *PFPResult;

typedef struct _FPRegister_{
	BYTE MachineNumber;
	U32 PIN;
	U32 ImageSize;	
	BYTE Index; //Valid numbers 有效发送次数 Index=1表示新登记
}GCC_PACKED TFPRegister, *PFPRegister;

typedef struct _FPID_{
	BYTE MachineNumber;
	U32 PIN;
}GCC_PACKED TFPID, *PFPID;

typedef struct _PhotoSize_{
        BYTE MachineNumber;
        U32 PIN;
        U16 Width;
        U16 Height;
        U8 BPP;
        U8 Type;
}GCC_PACKED TPhotoSize, *PTPhotoSize;

struct str_net_addr     //ccc
{
	unsigned char ip[4];
    unsigned char mask[4];
    unsigned char gateway[4];
	char name[20];
}gwifiaddr;

int EthCommCheck(void);
int EthCommTCPCheck(void);
int EthInit(void);
void EthFree(void);
int EthBoradcastCheck(void);
//int ethernet_cmd(const char *ifname, unsigned short speed, unsigned char duplex);

//AuthServer
int CheckAuthServerSessionTimeOut(int TimeOutSec, int mode);
int EthAuthServerCheck(int mode);
int AuthServerREQ(int TimeInterval, int mode);

int InitAuthServer(void);
int FreeAuthServer(void);
BOOL CheckAllAuthServerRuning(void);
BOOL SendImageAndIdentify(char *buffer, int size, U32 pin2, PFPResult result);
BOOL SendImageToRegister(char *buffer, int size, U32 pin2, U8 Index, U16 *result);
BOOL GetFreeIDFromAuthServer(U32 *result);
BOOL CheckIsIdleIDFromAuthServer(U32 pin2, U8 *result);
void RefreshAuthServerListFromAuthServer(void);
void UploadAttLogByAuthServer(void);
BOOL UploadIDCardToAuthServer(U32 cardnumber, PFPResult result);
BOOL SendImageAndIdentify(char *buffer, int size, U32 pin2, PFPResult result);
//BOOL SendPhotoToAuthServer(U32 pin, char *buffer, int size);
BOOL SendPhotoToAuthServer(U32 pin, char *buffer, U16 width, U16 height, U8 bpp, U8 type);

BOOL RefreshUserDataFromAuthServer(void);
BOOL GetTimeFromAuthServer(U32 *result);
BOOL SyncLocalTimeFromAuthServer();

//------ GPRS --------//
int icmp_echo();
int udp_echo();
int tcp_echo();
// GPRS end //
#endif
