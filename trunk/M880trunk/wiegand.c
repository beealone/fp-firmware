/*************************************************
                                           
 ZEM 200                                          
                                                    
 wiegand.c define wiegand format and output wiegand data  
 
                                                      
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arca.h"
#include "wiegand.h"
#include "options.h"
#include "flashdb.h"

extern int WiegandInOrOut;

int wiegandDefineInit0(char *name, char *formatName, TWiegandDefine *wiegand)
{
	char optName[100]={0};
	char *fmtstr=NULL;

	sprintf(optName, "%sPulseWidth", name);
	wiegand->pulseWidth = LoadInteger(optName, 100);
	
	sprintf(optName, "%sPulseInterval", name);
	wiegand->pulseInterval = LoadInteger(optName, 1000);

	
	if (formatName) {
		sprintf(optName, "%s", formatName);
	} else {
		sprintf(optName, "%sFmt", name);
	}
	fmtstr = LoadStrOld(optName);

	printf("%s: %s, PulseWidth=%d, PulseInterval=%d\n", optName, fmtstr, wiegand->pulseWidth, wiegand->pulseInterval);

	if (fmtstr==NULL || isNewWGFormat(fmtstr)) {
		printf("create new fmt\n");
		wiegand->format = wgCreateFmt(fmtstr);
	} else {
		printf("create old fmt\n");
		wiegand->format = wgCreateFmtFromOldOption(fmtstr, ((strcmp(optName,"WiegandFmt")==0)?0:1));
	}

	if (wiegand->format) {
		printf("...success\n\n");
		return TRUE;
	}
	printf("...fail\n\n");
	return FALSE;
}

int gWGFailedID;	//Wiegand失败ID
int gWGDuressID;	//Wiegand胁迫ID
int gWGSiteCode;	//Wiegand区码
int gWGOEMCode;		//OEM Code or Manufacture Code
int gWGFacilityCode;	
int gWGOutputType;	

int wiegandDefineInit(int wiegandIndex)
{
	int ret = FALSE;
	
	if (wiegandIndex==WIEGAND_IN_EXTERNAL || (gOptions.ExtWGInFunOn && wiegandIndex==WIEGAND_ALL)) {
		printf("\nInit Wiegand IN format\n");
		WiegandInOrOut = WG_IN_MODE;
		ret = wiegandDefineInit0("ExtWGIn", "ExtWiegandInFmt", &gWiegandDefine[WIEGAND_IN_EXTERNAL]);
	}

	if (wiegandIndex==WIEGAND_IN_INTERNAL || wiegandIndex==WIEGAND_ALL) {
		printf("\nInit Wiegand IN format of ID/HID module\n");
		ret = wiegandDefineInit0("IntWGIn", "IntWiegandInFmt", &gWiegandDefine[WIEGAND_IN_INTERNAL]);
	}

	if (wiegandIndex==WIEGAND_OUT || wiegandIndex==WIEGAND_ALL) {
		WiegandInOrOut = WG_OUT_MODE;
		printf("\nInit Wiegand Out format\n");
		gWGFailedID = LoadInteger("WGFailedID", -1);
		gWGDuressID = LoadInteger("WGDuressID", -1);
		gWGOEMCode = LoadInteger("WGOEMCode", 0);
		gWGFacilityCode = LoadInteger("WGFacilityCode", 0);
		gWGSiteCode = LoadInteger("WGSiteCode", -1);
		gWGOutputType = LoadInteger("WGOutputType", 2);		// Default value is output directly.
		ret = wiegandDefineInit0("WG", "WiegandFmt", &gWiegandDefine[WIEGAND_OUT]);
	}

	return ret;
}

int wiegandEncode(int wiegandIndex, char *bitString, U32 manufactureCode, U32 facilityCode, U32 siteCode, U32 cardNumber)
{
	TWiegandFmt *fmt;
	TWiegandData data;

	if (wiegandIndex<0 || wiegandIndex>=MAX_WIEGAND_DEFINE) {
		return ERROR_WG_FMT;
	}

	fmt = gWiegandDefine[wiegandIndex].format;
	data.manufactureCode = manufactureCode;
	data.siteCode = siteCode;
	data.facilityCode = facilityCode;
	data.cardNumber = cardNumber;
	printf("___%s,,%d, siteCode = %d, cardNumber = %u\n",__FILE__, __LINE__, siteCode, cardNumber);
	//printf("___bitString = %s\n", bitString);
	return wgEncode(fmt, &data, bitString);
}

int wiegandDecode(int wiegandIndex, char *bitString, TWiegandData *wgdata, BOOL checksumflag)
{
	TWiegandFmt *fmt = NULL;

	if (wiegandIndex<0 || wiegandIndex>=MAX_WIEGAND_DEFINE)
		return ERROR_WG_FMT;
	
	fmt = gWiegandDefine[wiegandIndex].format;
	return wgDecode(fmt, bitString, wgdata, wiegandIndex, checksumflag);
}

static void buf2bits64Ex(char *buf, char *bits, int bitsCount, int invert)
{
	int i;
	for (i=0; i<bitsCount; i++)
	{
		if (buf[i/8] & (0x01<<(7-(i%8))))
			bits[i] = (invert) ? '0' : '1';
		else
			bits[i] = (invert)? '1' : '0';
		DBPRINTF("%c", bits[i]);
	}
	printf("\n");
	bits[bitsCount] = 0;
}

void buf2bits64(char *buf, char *bits, int bitsCount, int invert)
{
	int i;
	for (i=0; i<bitsCount; i++)
	{
		if(buf[i/8] & (0x01<<(i%8)))
			bits[i] = (invert)? '0' : '1';
		else
			bits[i] = (invert)? '1' : '0';
		DBPRINTF("%c", bits[i]);
	}
	printf("\n");
	bits[bitsCount]=0;
}

static int wiegandDataDecode(int wiegandIndex, char *data, TWiegandData* wgdata, BOOL checksumflag, BOOL BitsRevert)
{
	char bitString[MAX_WG_BITS];
	TWiegandFmt *fmt = NULL;
	
	if (wiegandIndex<0 || wiegandIndex>=MAX_WIEGAND_DEFINE) {
		return ERROR_WG_FMT;
	}
	
	fmt = gWiegandDefine[wiegandIndex].format;
	if (fmt==NULL) {
		return ERROR_WG_FMT;
	}

	if (BitsRevert) {
		buf2bits64Ex(data, bitString, fmt->count, gOptions.BitsInvertCardModule);
	} else {
		buf2bits64(data, bitString, fmt->count, gOptions.BitsInvertCardModule);
	}
	
	return wiegandDecode(wiegandIndex, bitString, wgdata, checksumflag);
}

TWiegandDefine *wiegandGetDefine(int wiegandIndex)
{
	if (wiegandIndex<0 || wiegandIndex>=MAX_WIEGAND_DEFINE) return NULL;
	return &gWiegandDefine[wiegandIndex];
}

int CalcWiegandDataDeployment(U8* InputData, int InputLen, U8 *data1)
{
	int i, pos, p1p, p2p, Iboundary, ii;
	U32 Test;

	memset(data1,'0',InputLen*8+2);
	p1p=0;
	p2p=InputLen*8+1;
	Iboundary=InputLen*8/2+1;

	pos=0;
	for (ii=0; ii<InputLen; ii++)
	{
		Test=1<<(8-1);
		for (i=0; i<8; i++)
		{
			if(pos==p1p || pos==p2p)
			{
				pos++;
			}
			if(InputData[ii] & Test)
			{
				data1[pos]='1';
			}
			pos++;
			Test>>=1;
		}
	}

	Test=0;
	pos=0;
	for (i=1; i<Iboundary; i++)
	{
		Test += data1[pos++];
	}
	
	if ((Test & 1)==0) 
		data1[p1p]='1';
	
	Test = 0;
	pos = Iboundary;
	
	for (i=Iboundary; i<InputLen*8/2+1; i++)
	{
		Test+=data1[pos++];
	}
	
	if (Test & 1)
		data1[p2p]='1';

	return InputLen*8/2+2;
}

U32 CheckCardEvent(int cmd, unsigned char* buffer)
{
	BOOL bParityFlag = gOptions.CardNumberParity;		// 是否需要进行奇偶校验
	BOOL bBitsRevert = FALSE;							// 高低位是否颠倒
	
	TWiegandFmt *wgfmt = NULL;
	TWiegandData data;
	
	int index;
	if ((cmd==MCU_CMD_HID) || (cmd==MCU_CMD_HID_AUTO) || (cmd==MCU_CMD_INTWGIN))
	{
		index = WIEGAND_IN_INTERNAL;
	}
	else if (((cmd==MCU_CMD_EXTWGIN) || (cmd==MCU_CMD_EXT_EXTWGIN)) && gOptions.ExtWGInFunOn)
	{
		index = WIEGAND_IN_EXTERNAL;
	}
	else
	{
		return 0;
	}

	wgfmt = gWiegandDefine[index].format;
	memset(&data, 0, sizeof(TWiegandData));

	if (cmd==MCU_CMD_HID)
	{
		/* 此情况下返回3字节数据，不包含奇偶校验位, 所以需要先将24位数据扩充为26位 */
		int i;
		char bits24[MAX_WG_BITS];
		char bits26[MAX_WG_BITS];

		if (wgfmt->count != 26)
		{
			DBPRINTF("Card setting error, bits count should be 26\n");
			return 0;
		}
		memset(bits24, 0, sizeof(bits24));
		memset(bits26, 0, sizeof(bits26));

		buf2bits64((char*)buffer, bits24, 24, bBitsRevert);
		for (i=0; i<24; i++)
		{
			bits26[i+1] = bits24[23-i];
		}
		bits26[25] = 0;
		
		bParityFlag = FALSE;				// 取消奇偶校验
		
		wgDecode(wgfmt, bits26, &data, index, bParityFlag);
		return data.cardNumber;
	}
	else if (cmd==MCU_CMD_HID_AUTO || cmd==MCU_CMD_EXT_EXTWGIN)
	{
		/* 此情况下返回8字节数据，首字节为卡位数，所以需要先去掉首字节 */
		unsigned char tmpBuf[8];
		
		int bitcount = (int)buffer[0];
		if (wgfmt->count != bitcount)
		{
			DBPRINTF("Card setting error, bits count should be: %d\n", bitcount);
			return 0;
		}

		memset(tmpBuf, 0, sizeof(tmpBuf));
		memcpy(tmpBuf, buffer+1, 7);

		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, tmpBuf, 7);
	}
	
	/* 是否高低位颠倒 */
	if (cmd==MCU_CMD_HID_AUTO)
	{
		bBitsRevert = TRUE;
	}
	
	/* Start to decode card number */
	wiegandDataDecode(index, (char*)buffer, &data, bParityFlag, bBitsRevert);

	return data.cardNumber;
}

void FreeWiegandFmt(void)
{
	int i;
	for (i=0; i<MAX_WIEGAND_DEFINE; i++) {
		if (gWiegandDefine[i].format) {
			free(gWiegandDefine[i].format);
			gWiegandDefine[i].format=NULL;
		}
	}
}

int ReloadWiegandOutFmt(void)
{
	if (gWiegandDefine[WIEGAND_OUT].format) {
		free(gWiegandDefine[WIEGAND_OUT].format);
		gWiegandDefine[WIEGAND_OUT].format=NULL;
	}
	return wiegandDefineInit(WIEGAND_OUT);
}


