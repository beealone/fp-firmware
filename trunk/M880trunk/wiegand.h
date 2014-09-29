#ifndef __WIEGAND_H__
#define __WIEGAND_H__

#include "arca.h"
#include "wiegandcode.h"

#define MCU_CMD_HID 		50
#define MCU_CMD_HID_AUTO 	51	//自动识别，但是前提条件大于26位的卡
#define MCU_CMD_EXTWGIN		215
#define MCU_CMD_INTWGIN		216
#define MCU_CMD_EXT_EXTWGIN	218

typedef struct _WiegandDefine_
{
	TWiegandFmt *format;
	int pulseWidth;
	int pulseInterval;
}TWiegandDefine, PWiegandDefine;

#define WIEGAND_OUT		0
#define WIEGAND_IN_1		1
#define WIEGAND_IN_2		2
#define WIEGAND_IN_INTERNAL	WIEGAND_IN_1
#define WIEGAND_IN_EXTERNAL	WIEGAND_IN_2
#define WIEGAND_ALL		65535

extern int gWGFailedID;
extern int gWGDuressID;
extern int gWGSiteCode;
extern int gWGOEMCode;
extern int gWGFacilityCode;
extern int gWGOutputType;

#define MAX_WIEGAND_DEFINE      3 
TWiegandDefine gWiegandDefine[MAX_WIEGAND_DEFINE];

//char OriginalData[8];	/* 存放未经格式解析的卡号数据 */

int wiegandDefineInit(int wiegandIndex);
TWiegandDefine *wiegandGetDefine(int wiegandIndex);
int wiegandEncode(int wiegandIndex, char *bitString, U32 manufactureCode, U32 facilityCode, U32 siteCode, U32 cardNumber);
int wiegandDecode(int wiegandIndex, char *bitString, TWiegandData *wgdata, BOOL checksumflag);
int CalcWiegandDataDeployment(U8* InputData, int InputLen, U8 *data1);

void buf2bits64(char *buf, char *bits, int bitsCount, int invert);
U32 CheckCardEvent(int cmd, unsigned char* buffer);
void FreeWiegandFmt(void);
int ReloadWiegandOutFmt(void);

#endif
