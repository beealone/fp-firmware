#ifndef __ZLG500B_H_
#define __ZLG500B_H_
#include "serial.h"

#define OP_READ		0		//读卡内数据
#define OP_WRITE	1		//写数据到卡中
#define OP_EMPTY	2		//清空卡内数据

#ifndef BYTE
#define BYTE unsigned char 
#endif 

#define NOFINGERTAG  0xFF

#define MFCARD_ERROR_KEY        -2      //卡授权失败
#define MFCARD_ERROR_READ       -1      //读错误
#define MFCARD_ERROR_UNKNOWN    -4      //未知卡
#define MFCARD_ERROR_LIMITED    -5      //受限卡，该卡不能在此机上使用
#define MFCARD_ERROR_EMPTY      -3      //空卡
#define MFCARD_ERROR_DATA       -6     	//卡内的数据错误
#define MFCARD_ERROR_WRITE      -7      //写错误
#define MFCARD_ERROR_FREE       -8      //卡上的保留空间不够
#define MFCARD_ERROR_OK         0       //操作正确

typedef struct _FPCardOP_{
	U32 PIN;
	int TempSize;
	BYTE Finger[4];
	BYTE *Templates;
	BYTE OP;
}TFPCardOP, *PFPCardOP;

void TestMifare(void);

//int MFInit(void);
int MFInit(serial_driver_t *serial);
int MFCheckCard(U8 *sn);

int MFRead(PFPCardOP fpdata, int OnlyPINCard);
int MFWrite(PFPCardOP fpdata);
int MFEmpty(void);
int MFFinishCard(void);
int MFGetResSize(void);

#endif
