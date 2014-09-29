#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include "serial.h"
#include "arca.h"
#include "hid_iclass.h"

//#define SHOWTIME

#ifndef iCLSTRY_TIMES
#define iCLSTRY_TIMES 2*1000
#endif

#define DELAYTIME  300

//#define SELECTCARDLOOP

/************************ Command Structure Format *******************************************
*
* Field		CLA		INS		P1		P2		P3		ACK		DATA	SW1		SW2
* 
* Offset	0		1		2		3		4		5		6		38		39
*
* Byte(s)	1		1		1		1		1		1		0-32	1		1
*
*
* Description: The command structure contains standrad Application Protocol Data Unit(APDU) 
*			 	variables which can be either a command or a response.A specific command will
*				generate a specific response.
*				
**********************************************************************************************/
#define CLA 		0		//Class,default as 80h
#define INS		1		//Instruction,pre-defined list,see following Command macro define.
#define P1		2		//Paramter
#define P2		3		//Paramter
#define P3		4		//Paramter
#define ACK		5		//Acknowledge(echo of the instruction)
#define DATA 	6		//Data(P3 bytes of)
#define SW1		38		//Status Word 1(command status)
#define SW2		39		//Status Word 2(status qualifier)

/* Command */
#define CLA_iCLASS 0x80
#define SELECT_CURRENT_KEY	0x52
#define DIVERSIFY_KEY 0x52
#define SELECT_CARD	0xA4
#define PAGE_SELECT	0xA6
#define TRANSMIT	0xC2	//Read or write data from or to card.
#define LED_SOUNDER_CONTROL 0x70
#define OPEN_COLLECTOR_OUTPUT 0x71
#define LOAD_KEY 0xD8
#define ASK_RANDOM 0x84
#define RETRIEVE_DATA 0xC0	//Read or write data from or to reader memory
#define READ_EEPROM 0xF2
#define WRITE_EEPROM 0xF4
#define RESET_RF 0xF4

/* Response */
#define CMD_OK	0x9000
#define ERR_P3	0x6700
#define ERR_P1P2 0x6B00
#define INVALID_INS	0x6D00
#define INVALID_CLS	0x6E00
#define CARD_NOT_IDENTIFIED 0x6982
#define CARD_NOT_FOUND	0x6A82
#define ERR_EEPROM	0x6200
#define ERR_CMD_FLOW	0x9835
#define READER_RST	0x3B00

/************************ Command-Response Message Types ******************************
*
*	For each of the command-response message types,there are specific fields within 
*   the APDU that are required.The following tables illustrate the four possible 
*   command structures for each command-response type
*
* Type1:
* 				Command				Status
*		host		CLA INS P1 P2 P3
*		reader						SW1 SW2
*
* Type2:
*				Command			 ACK			Data				Status	   
*		host		CLA	INS	P1 P2 P3		 
*		reader					 ACK=INS 	P3 Data Bytes		SW1 SW2
* Type3:
*				Command			 ACK			Data				Status
*		host		CLA	INS	P1 P2 P3				P3 Data Bytes
*		reader					 ACK=INS						SW1 SW2
* Type4:
*				Command			 ACK		Data				ACK		Data		Status
*		host		CLA	INS	P1 P2 P3			P3 Data Byptes
*		reader					 ACK=INS					ACK=INS Data		SW1 SW2
***************************************************************************************/
//		   Cmd 	  Rsp	  				
#define PTP_NODATA_NODATA	1	//Command-Response pair type: command no data,response no data
#define PTP_NODATA_DATA	2	//Command-Response pair type: command no data,response has data
#define PTP_DATA_NODATA	3	//Command-Response pair type: command has data,response no data
#define PTP_DATA_DATA		4	//Command-Response pair type: command has data,response has data

/* The application area on the page */
#define APP_AREA1	1
#define APP_AREA2	0

//P1: Read/write block(s) mode
#define RBM_4_14443B 	0xF0  //read 4 blocks in 14443B mode (16k cards only)
#define RBM_4_15693 	0xF1  //read 4 blocks in 15693 mode 
#define RBM_1_14443B 	0xF4  //read 1 block in 14443B mode (16k cards only)
#define RBM_1_15693		0xF5  //read 1 block in 15693 mode
#define WBM_1_15693	0x69  //write 1 block in 15693 mode
#define WBM_1_14443B 	0x68  //can not used on 2k card

//Type
#define RTP_4	0x06  // to read 4 blocks
#define RTP_1	0x0c  // to read 1 block
#define WTP_1	0x87  // to write a single block

typedef struct _CMD{
	U8 ins;
	U8 param1;
	U8 param2;
	U8 param3;
	U8 pairType;
}TCMD,*PCMD;

static U8 APDU[65] ={0,};

//const U8 ExampleSecretKey[8] ={0x5c, 0xbc, 0xf1, 0xda, 0x45, 0xd5, 0xfb, 0x5f};
//const U8 ExamplePermExKeyCrc[8] ={0xff, 0x0f, 0x33, 0x55, 0, 0xf0, 0xcc, 0x55};

const U8 DefaultPermExKeyCrc[8] = {0x6E, 0xFD, 0x46, 0xEF, 0xCB, 0xB3, 0xC8, 0x75};

const U8 DefaultSecretKey[16][8] ={
   {0x01,		0x02,		0x03,		0x04,		0x05,		0x06,		0x07,		0x08},//K0
   {0xFD,		0xCB,		0x5A,		0x52,		0xEA,		0x8F,		0x30,		0x90},//K1
   {0x23,		0x7F,		0xF9,		0x07,		0x98,		0x63,		0xDF,		0x44},//K2
   {0x5A,		0xDC,		0x25,		0xFB,		0x27,		0x18,		0x1D,		0x32},//K3
   {0x83,		0xB8,		0x81,		0xF2,		0x93,		0x6B,		0x2E,		0x49},//K4
   {0x43,		0x64,		0x4E,		0x61,		0xEE,		0x86,		0x6B,		0xA5},//K5
   {0x89,		0x70,		0x34,		0x14,		0x3D,		0x01,		0x60,		0x80},//K6
   {0x82,		0xD1,		0x7B,		0x44,		0xC0,		0x12,		0x29,		0x63},//K7
   {0x48,		0x95,		0xCA,		0x7D,		0xE6,		0x5E,		0x20,		0x25},//K8
   {0xDA,		0xDA,		0xD4,		0xC5,		0x7B,		0xE2,		0x71,		0xB7},//K9
   {0xE4,		0x1E,		0x9E,		0xDE,		0xF5,		0x71,		0x9A,		0xBF},//K10
   {0x29,		0x3D,		0x27,		0x5E,		0xC3,		0xAF,		0x9C,		0x7F},//K11
   {0xC3,		0xC1,		0x69,		0x25,	 	0x1B,		0x8A,		0x70,		0xFB},//K12
   {0xF4,		0x1D,		0xAF,		0x58,		0xB2,		0x0C,		0x8B,		0x91},//K13
   {0x28,		0x87,		0x7A,		0x60,		0x9E,		0xC0,		0xDD,		0x2B},//K14
   {0x66,		0x58,		0x4C,		0x91,		0xEE,		0x80,		0xD5,		0xE5}//K15
};

U8 ISOSTANDRAD =ISO_15693;//ISO_14443B;
U8 gProtocolType =ISO_15693;//ISO_14443B;
char *AREA[4] ={NULL,"Area2", NULL,"Area1"};
//
// GetKeyAndChkSum: Generating key and check sum which loaded to reader. 
//
// keyAndChkSum:The key and check sum are calculated according to an specific algorithm.
//
// random: Request random number from reader after Ask_Random command.  
//
// PermExKeyCrc:
// SerectKey:
// 
// keyLoc: The key location in the reader to load newly generated key
//
// return: Return generated key and check sum pointer
//
static U8 *GetKeyAndChkSum(U8 *keyAndChkSum,const U8 *random, const U8 *PermExKeyCrc, const U8 *SecretKey, U8 rdKeyLoc)
{
	int i,j;
	U8 PermSecKeyCrc[8] ={0,};
	U8 EncrptedSecKey[8] ={0,};
	U8 ChkSum[4];
	U8 eightCrc=0;

	memset(PermSecKeyCrc, 0, 8);

	//Generate PermSecKeyCrc;
	for( i = 6; i >= 0; i--)	
	{		
		for( j = 7; j >= 0; j--)		
		{			
			PermSecKeyCrc[i] |=( ( SecretKey[j] & (1 << (7-i) ) ) >> (7-i) ) << j;
		}		

		eightCrc ^=PermSecKeyCrc[i];	
	}			

	PermSecKeyCrc[7] = ~eightCrc;

	//Generate EncrptedSecKey
	for( i = 0; i < 8; i++ )	
	{		
		EncrptedSecKey[i] =( random[i] ^ PermExKeyCrc[i] ) ^ PermSecKeyCrc[i];
	}

	//Generate ChkSum
	ChkSum[0] = PermSecKeyCrc[0] ^ 0x80
					^ PermSecKeyCrc[4] ^ 0x0c;
	ChkSum[1] = PermSecKeyCrc[1] ^ 0xd8
					^ PermSecKeyCrc[5] ^ 0x00;
	ChkSum[2] = PermSecKeyCrc[2] ^ 0x00
					^ PermSecKeyCrc[6] ^ 0x00;
	ChkSum[3] = PermSecKeyCrc[3] ^ rdKeyLoc	
					^ PermSecKeyCrc[7] ^ 0x00;

	//Combine EncrptedSecKey and ChkSum
	memcpy(keyAndChkSum, EncrptedSecKey, 8);
	memcpy(keyAndChkSum+8,ChkSum,4);

	return keyAndChkSum;
}

/*
* iCLASS_Perror:	Printing out what error occured.
*
* error : Value of error.
*
* return : void
*/
void iCLASS_Perror(int error)
{
#if 0
	DBPRINTF("\t\t\t\tHID iCLASS Reader return status: %x\n", error);
	switch(error){

		case ERR_P3:
			 DBPRINTF("Parameter P3 is incorrect !\n");
			 break;
		case ERR_P1P2:
			 DBPRINTF("Parameter P1 or P2 is incorrect !\n");
			 break;
		case INVALID_CLS:
			 DBPRINTF("Invalid Class !\n");
			 break;
		case INVALID_INS:
			 DBPRINTF("Invalid Instruction !\n");
			 break;
		case CARD_NOT_IDENTIFIED:
			  DBPRINTF("Card not identified !\n");
			  break;
		case CARD_NOT_FOUND:
			  DBPRINTF("Card not found !\n");
			  break;
		case ERR_EEPROM:
			  DBPRINTF("EEPROM error !\n");
			  break;
		case ERR_CMD_FLOW:
			  DBPRINTF("Command flow is incorrect !\n");
			  break;
		case READER_RST:
			  DBPRINTF("Reader reset !\n");
			  break;
		default:
			  DBPRINTF("Communication fail or the error is not defined !\n");
			  break;
	}
#endif

}

/*
* iCLASS_Write_Reader:	Write command or data to reader.
* 
* buf:	The data buffer wrote to reader.
*
* len:	The length of data.
*
* return:	The length of data wrote successfully to reader.
*/
static int iCLASS_Write_Reader(const U8 *buf, int len)
{
	int i, nCnt =0;

	//DBPRINTF("\tWrite data:\n\t\t");

#ifdef SHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif

	for( i=0; i<len; i++){
		if(iCLASS_raw_write(buf[i])<-1)
			printf("Write Failed!\n");
		nCnt++;

		//DBPRINTF("%x ",buf[i]);
	}

	//DBPRINTF("\n");

#ifdef SHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("\t\t\twrite %d bytes use time:%f ms\n",nCnt,msec);
#endif
	return nCnt;
}

/*
* iCLASS_Read_Reader:	Read data from reader.
* 
* buf:	The data buffer store data from reader.
*
* len:	The length of data want to read.
*
* return:	The length of data  read from reader,less than or equal the len.
*/
static int iCLASS_Read_Reader(U8 *buf, int len)
{
	int i,nCnt = len; 
	int rdcnt = iCLSTRY_TIMES;
	U8 data;

	i = 0;
	
	memset(buf, 0, len);

//	DBPRINTF("\tRead data: len =%d\n",nCnt);
	
	while( nCnt >0 && rdcnt){
//		DBPRINTF("\t\t\tRead nCnt =%d, rdcnt= %d\n",nCnt,rdcnt);
		if(iCLASS_raw_poll()){
			data = iCLASS_raw_read();
			--nCnt;
			buf[i++] = data;

//			DBPRINTF("\tReadData: %x\n ",data);
		}
		else
		{
			DelayUS(DELAYTIME);
			rdcnt--;
		}
	}

//	DBPRINTF("\n");

	return i;	
}
//
// iCLASS_SendCommand: The host send command to reader and get response from header.
//
// cmd: Command structure with specific command and parameter
// cmdData: Command data send to reader 
// rspData: Response data get from reader
//
// return: return communication status.but if get or send bytes is not matching of the specific number return 0;
//
static int iCLASS_SendCommand(const TCMD cmd, const U8* cmdData,U8* rspData)
{
	U8 ack =0;
	int nCount;
	int nBytes;
	int status =0;

	memset(APDU,0,sizeof(APDU));

	APDU[CLA] = CLA_iCLASS;
	APDU[INS] = cmd.ins;
	APDU[P1]  = cmd.param1;
	APDU[P2]  = cmd.param2;
	APDU[P3]  = cmd.param3;

	iCLASS_raw_flush_input();	
	if(cmd.pairType == PTP_NODATA_NODATA )
	{
		// Send command
		nBytes = 5;
		if( (nCount = iCLASS_Write_Reader(APDU, nBytes)) == nBytes)
		{
			//DBPRINTF("\tSend Command OK!\n");
			//Get status
			U8 buf[2]={0,};
			nBytes =2;
			if( (nCount = iCLASS_Read_Reader(buf, nBytes)) == nBytes)
			{	
				//DBPRINTF("\tGet status word nBytes OK!\n");
				memcpy(&APDU[SW1],buf,2);
			}
		}
	}
	else if( cmd.pairType == PTP_NODATA_DATA )
	{
		//Send command
		nBytes =5;
		if( (nCount = iCLASS_Write_Reader(APDU, nBytes)) == nBytes )
		{
			//DBPRINTF("\tSend Command OK!\n");
			//Get ACK
			nBytes =1;
			if( (nCount = iCLASS_Read_Reader(&ack, nBytes)) == nBytes )
			{
				//DBPRINTF("\tGet ACK nBytes OK!\n");
				if( ack == APDU[INS] )
				{
					//DBPRINTF("\tACK success!\n");
					//Get response data
					nBytes = APDU[P3];
					if( (nCount = iCLASS_Read_Reader(&APDU[DATA],nBytes)) == nBytes )
					{
						//DBPRINTF("\tGet response data nBytes OK!\n");
						//Get Status
						nBytes = 2;
						if( (nCount = iCLASS_Read_Reader(&APDU[SW1],nBytes)) == nBytes )
						{
							//DBPRINTF("\tGet status word nBytes OK!\n");
							int tmp=0;
							tmp = APDU[SW1];
							tmp = (tmp<<8) + APDU[SW1+1];
							if( CMD_OK == tmp )
							{
								if(rspData != NULL)
									memcpy(rspData, &APDU[DATA],APDU[P3]);
							}
						}
					}
				}
				else
				{	//Get status
					nBytes = 1;
					if( (nCount = iCLASS_Read_Reader(&APDU[SW1+1],nBytes)) == nBytes )
					{
						//DBPRINTF("\tGet status word nBytes OK!\n");
						APDU[SW1] = ack;
					}
				}
			}
		}
	}
	else if( cmd.pairType == PTP_DATA_NODATA )
	{
		//Send command
		nBytes =5;
		if( (nCount = iCLASS_Write_Reader(APDU, nBytes)) == nBytes )
		{	
			//DBPRINTF("\tSend Command OK!\n");
			//Get ack
			nBytes =1;
			if( (nCount = iCLASS_Read_Reader(&ack,nBytes)) == nBytes )
			{
				//DBPRINTF("\tGet ACK nBytes OK!\n");
				if( ack == APDU[INS] )
				{
					//DBPRINTF("\tACK success!\n");
					//Send command data
					nBytes = APDU[P3];
					memcpy(&APDU[DATA],cmdData,nBytes);
					if( (nCount = iCLASS_Write_Reader(&APDU[DATA],nBytes)) == nBytes )
					{
						//DBPRINTF("\tWrite command data nbytes OK!\n");
						//Get status
						U8 buf[2]={0,};
						nBytes =2;
						if( (nCount = iCLASS_Read_Reader(buf, nBytes)) == nBytes)
						{
							//DBPRINTF("\tGet status word nBytes OK!\n");
							memcpy(&APDU[SW1],buf,2);
						}
					}
				}
				else
				{
					//Get status
					nBytes = 1;
					if( (nCount = iCLASS_Read_Reader(&APDU[SW1+1],nBytes)) == nBytes )
					{
						//DBPRINTF("\tGet status word nBytes OK!\n");
						APDU[SW1] = ack;
					}
				}
			}
		}
	}
	else if( cmd.pairType == PTP_DATA_DATA )
	{
		//Send command
		nBytes =5;
		if( (nCount = iCLASS_Write_Reader(APDU, nBytes)) == nBytes )
		{
			//DBPRINTF("\tSend Command OK!\n");
			//Get ack
			nBytes =1;
			if( (nCount = iCLASS_Read_Reader(&ack,nBytes)) == nBytes )
			{
				//DBPRINTF("\tGet ACK nBytes OK!\n");
				if( ack == APDU[INS] )
				{
					//DBPRINTF("\tACK success!\n");
					//Send command data
					nBytes = APDU[P3];
					memcpy(&APDU[DATA],cmdData,nBytes);
					if( (nCount = iCLASS_Write_Reader(&APDU[DATA],nBytes)) == nBytes )
					{
						//DBPRINTF("\tWrite command data nbytes OK!\n");
						//Transmit_Read 4 blocks
						if( (APDU[INS] == TRANSMIT) && ( (APDU[P1] == RBM_4_14443B) || (APDU[P1] == RBM_4_15693) ))
						{
							//Get status
							U8 buf[2]={0,};
							nBytes =2;
							if( (nCount = iCLASS_Read_Reader(buf, nBytes)) == nBytes)
							{
								//DBPRINTF("\tGet status word nBytes OK!\n");
								memcpy(&APDU[SW1],buf,2);
							}

							status = APDU[SW1];
							status = (status << 8) + APDU[SW1+1];
							return status;
						}

						//Get ack
						nBytes =1;
						if( (nCount = iCLASS_Read_Reader(&ack,nBytes)) == nBytes )
						{
							//DBPRINTF("\tGet ACK nBytes OK!\n");
							if( ack == APDU[INS] )
							{
								//DBPRINTF("\tACK success!\n");
								//Get response data
								nBytes = APDU[P2];
								if( (nCount = iCLASS_Read_Reader(&APDU[DATA],nBytes)) == nBytes )
								{
									//DBPRINTF("\tGet response data nBytes OK!\n");
									//Get Status
									nBytes = 2;
									if( (nCount = iCLASS_Read_Reader(&APDU[SW1],nBytes)) == nBytes )
									{
										//DBPRINTF("\tGet status word nBytes OK!\n");
										int tmp=0;
										tmp = APDU[SW1];
										tmp = (tmp<<8) + APDU[SW1+1];
										if( CMD_OK == tmp )
										{
											if(rspData != NULL)
												memcpy(rspData, &APDU[DATA],APDU[P2]);
										}
									}
								}								
							}
							else
							{
								//Get status
								nBytes = 1;
								//DBPRINTF("\tGet status word nBytes OK!\n");
								if( (nCount = iCLASS_Read_Reader(&APDU[SW1+1],nBytes)) == nBytes )
								{
									APDU[SW1] = ack;
								}
							}
						}
					}
				}
				else
				{
					//Get status
					nBytes = 1;
					//DBPRINTF("\tGet status word nBytes OK!\n");
					if( (nCount = iCLASS_Read_Reader(&APDU[SW1+1],nBytes)) == nBytes )
					{
						APDU[SW1] = ack;
					}
				}
			}
		}
	}

	status = APDU[SW1];
	status = (status << 8) + APDU[SW1+1];

	//DBPRINTF("\t\tcurrent status:%x\n",status);
	return status;
}
/*
* Select_Current_Key: This command loads the selected key into the reader memory for use during future authentication commands.
*
* location: location of the key of the reader(1-7)
*
* Return:  If SELECT_CURRENT_KEY command implement successfully,return TRUE,otherwise FALSE
*		
*/	
static int Select_Current_Key(U8 rdKeyLoc)
{
	TCMD cmd;
	int status;
	U8 writeBuf[8] ={0,};
	 
	cmd.ins 	= SELECT_CURRENT_KEY;
	cmd.param1  = 0;
	cmd.param2  = rdKeyLoc;
	cmd.param3  = 0x08;
	cmd.pairType = PTP_DATA_NODATA;

	//DBPRINTF("Select_Current_Key:\n");
	status = iCLASS_SendCommand(cmd, writeBuf, NULL);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		return FALSE;
	}

	return TRUE;
}

//
// Select_Card:	This command performs anti-collision and authentication for all cards in the field.
//
// selectCardArea: 	Key of the card for application area
//
// rdKeyLocCdTp:The location of key of the reader and card type.
//
// serialNumber:Pointer to 8-byte serial number manufacturer encoded if SELECT_CARD command performed successfully,otherwise NULL. 
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
static int Select_Card(U8 selectCardArea, U8 rdKeyLocCdTp, U8* serialNumber)
{
	TCMD cmd;
	int status;
 
	cmd.ins = SELECT_CARD;
	cmd.param1  	= selectCardArea;
	cmd.param2  	= rdKeyLocCdTp;
	cmd.param3  	= 0x09;
	cmd.pairType = PTP_NODATA_DATA;

	U8 *readBuf = NULL;
	if( (readBuf = (U8*)MALLOC(cmd.param3)) ==NULL )
	{
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}
	//DBPRINTF("Select_Card:\n");

#ifndef SELECTCARDLOOP
	status = iCLASS_SendCommand(cmd, NULL, readBuf);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		FREE(readBuf);
		readBuf = NULL;
		return FALSE;
	}
#else
	U8 loop =5;
	do{

		status = iCLASS_SendCommand(cmd, NULL, readBuf);

		if(CMD_OK ==status)
		{
			//DBPRINTF("\t\t\t\tCMD_OK\n");
			break;
		}
		//DBPRINTF("SelectCard: %d\n", loop);
	}while(--loop);

	if(CMD_OK != status  )
	{
		//DBPRINTF("\t\t\t\tCMD_FAIL\n");
		iCLASS_Perror(status);
		FREE(readBuf);
		readBuf = NULL;
		return FALSE;
	}
#endif

	if(serialNumber != NULL )
		memcpy(serialNumber, readBuf+1, cmd.param3-1);

	FREE(readBuf);
	readBuf = NULL;
	
	return TRUE;
}

//
// Page_Select:	This command selects and/or authenticates a given page of a sixteen-application card.
//
// selectPageMode: Select page with authenication or no authenication at 15693 or 14443B mode.
// rdKeyLocCdKeyPgNum:	A 8 bit byte combine the reader key location and card key location and page number.
// configBlock:	If this command success,store the configuration block(block 1) for the select page.
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
static int Page_Select(U8 selectPageMode, U8 rdKeyLocCdKeyPgNum,U8 *configBlock)
{
	TCMD cmd;
	int status;
 
	cmd.ins = PAGE_SELECT;
	cmd.param1  = selectPageMode;
	cmd.param2  = rdKeyLocCdKeyPgNum;
	cmd.param3  = 0x08;
	cmd.pairType = PTP_NODATA_DATA;

	U8 *readBuf = NULL;
	if( (readBuf = (U8*)MALLOC(cmd.param3)) ==NULL )
	{
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}
	//DBPRINTF("Page_Select:\n");
	status = iCLASS_SendCommand(cmd, NULL, readBuf);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		FREE(readBuf);
		readBuf = NULL;
		return FALSE;
	}

	if(configBlock != NULL )
		memcpy(configBlock, readBuf, cmd.param3);

	FREE(readBuf);
	readBuf = NULL;
	
	return TRUE;
}
//
// Ask_Random:	Request an 8 byte random number from the reader.
//
// RND: Store the random number from the reader
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
static int Ask_Random(U8* RND)
{
	TCMD cmd;
	int status;
 
	cmd.ins = ASK_RANDOM;
	cmd.param1  = 0;
	cmd.param2  = 0;
	cmd.param3  = 0x08;
	cmd.pairType = PTP_NODATA_DATA;

	U8 *readBuf = NULL;
	if( (readBuf = (U8*)MALLOC(cmd.param3)) ==NULL )
	{
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}
	//DBPRINTF("Ask_Random:\n");
	status = iCLASS_SendCommand(cmd, NULL, readBuf);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		FREE(readBuf);
		readBuf = NULL;
		return FALSE;
	}

	if(RND != NULL )
		memcpy(RND, readBuf, cmd.param3);

	FREE(readBuf);
	readBuf = NULL;
	
	return TRUE;
}

//**********************************************************
//
// Transmit_Read:	Reading data from a card.
// 
// RWBlkMode: Protocol for card read/write
//	   	F0: Read 4 blocks in 14443B mode(16k cards only).
//		F1: Read 4 blocks in 15693 mode .
//		F4: Read 1 block in 14443B mode(16k card only).
//		F5: Read 1 block in 15693 mode.
//		69: Write 1 block in 15693 mode.
// 
// length: The length of read/write data.
//		08: Read/Write a single block
//		20: Read 4 blocks
//
// RWBlktype:  read or write block type
//		06: To read 4 blocks
//		0C: To read a single block
//		87: To write a single block
// 
// addr: Block address to read. Address must be within the appliaction area limit
//
// readData: 0 or 8 bytes data. 0 byte data returned during 0xF0 and 0xF1 4 block read
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
//*************************************************************
static int Transmit_Read(U8 RWBlkMode, U8 length, U8 RWBlktype, U8 addr, U8 *readData)
{
	TCMD cmd;
	int status;
 
	cmd.ins = TRANSMIT;
	cmd.param1  = RWBlkMode;
	cmd.param2  = length;
	cmd.param3  = 0x02;
	cmd.pairType = PTP_DATA_DATA;

	U8 *readBuf = NULL;
	if( (readBuf = (U8*)MALLOC(cmd.param2)) ==NULL )
	{	
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}

	U8 *writeBuf = NULL;
	if( (writeBuf = (U8*)MALLOC(cmd.param3)) ==NULL )
	{
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}
	writeBuf[0] = RWBlktype;
	writeBuf[1] = addr;
	//DBPRINTF("\t\tTransmit_Read: addr:%3d\n", addr);
	status = iCLASS_SendCommand(cmd, writeBuf, readBuf);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		FREE(readBuf);
		FREE(writeBuf);
		readBuf = NULL;
		writeBuf = NULL;
		return FALSE;
	}

	if(readData != NULL )
		memcpy(readData, readBuf, cmd.param2);

	FREE(readBuf);
	FREE(writeBuf);
	readBuf = NULL;
	writeBuf = NULL;
	
	return TRUE;	
}

//
// Transmit_Write:	Writting data to a card.
// 
// RWBlkMode: Protocol for card read/write
//	   	F0: Read 4 blocks in 14443B mode(16k cards only).
//		F1: Read 4 blocks in 15693 mode .
//		F4: Read 1 block in 14443B mode(16k card only).
//		F5: Read 1 block in 15693 mode.
//		69: Write 1 block in 15693 mode.
// 
// length: The length of read/write data.
//		08: Read/Write a single block
//		20: Read 4 blocks
//
// RWBlktype: 
//		06: To read 4 blocks
//		0C: To read a single block
//		87: To write a single block
// 
// addr: Block address to write. Address must be within the appliaction area limit
//
// writeData: 8 bytes of data
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
static int Transmit_Write(U8 RWBlkMode, U8 length, U8 RWBlktype, U8 addr, U8 *writeData)
{
	TCMD cmd;
	int status;
 
	cmd.ins = TRANSMIT;
	cmd.param1  = RWBlkMode;
	cmd.param2  = length;
	cmd.param3  = 0x0A;
	cmd.pairType = PTP_DATA_NODATA;

	U8 *writeBuf = NULL;
	if( (writeBuf = (U8*)MALLOC(cmd.param3)) ==NULL )
	{
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}
	writeBuf[0] = RWBlktype;
	writeBuf[1] = addr;
	memcpy(&writeBuf[2],writeData, cmd.param2);
	//DBPRINTF("\t\tTransmit_Write: addr:%3d\n",addr);
	status = iCLASS_SendCommand(cmd, writeBuf, NULL);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		FREE(writeBuf);
		writeBuf = NULL;
		return FALSE;
	}

	FREE(writeBuf);
	writeBuf = NULL;
	
	return TRUE;	
}

//
// Retrieve_Data: This command retrieve data from reader memory.This command should be call after successful calls for key diversification or a 4 block read
//
// length: Length of data(0x08 or 0x20)
//
// data: 8 or 32 bytes data depandent upon len
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
static int Retrieve_Data( U8 *data, U8 length )
{
	TCMD cmd;
	int status;
 
	cmd.ins = RETRIEVE_DATA;
	cmd.param1  = 0;
	cmd.param2  = 0;
	cmd.param3  = length;
	cmd.pairType = PTP_NODATA_DATA;

	U8 *readBuf = NULL;
	if( (readBuf = (U8*)MALLOC(cmd.param3)) ==NULL )
	{
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}
	//DBPRINTF("Retrieve_Data:\n");
	status = iCLASS_SendCommand(cmd, NULL, readBuf);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		FREE(readBuf);
		readBuf = NULL;
		return FALSE;
	}

	if(data != NULL )
		memcpy(data, readBuf, cmd.param3);

	FREE(readBuf);
	readBuf = NULL;
	
	return TRUE;
}

//
// LED_Sounder_Control: This command allows the control of the LED and the sounder
//
// p1: |  bit7  |   bit6  |   bit5  |   bit4  |   bit3  |   bit2  |   bit1  |   bit0  |
//		
//	     Green     Red    Sounder   Scb4     Scb3     Scb2     Scb1     Scb0                                                  
//		LED      LED                                                                                 
//
// p2: 
//		Play a Tune(Scb4 = 1): P1 = xx11yyyy, where yyyy is one of the sixteen prefedined tunes
//		Play a Sound(Scb4=0): P1 = xx10yyyy, where yyyy*0.1 is the sound duration
//							P2 = XX(hex), where 210,000/XX is the sound frequency
//
// p3: Control LED
// 
//	   0000 0000 Turn off LED		101y yyyy Flash Green for yyyy*0.1s
//      1000 0000 Turn on Green		011y yyyy Flash Red for yyyy*0.1s
//	   0100 0000 Turn on Red		111y yyyy Flash both for yyyy*0.1s
//	   1100 0000 Turn on Amber
// 
// return: If command implement successfully,return TRUE,otherwise FALSE
//
#if 0
static int LED_Sounder_Control(U8 p1, U8 p2, U8 p3)
{
	TCMD cmd;
	int status;
	 
	cmd.ins = LED_SOUNDER_CONTROL;
	cmd.param1  = p1;
	cmd.param2  = p2;
	cmd.param3  = p3;
	cmd.pairType = PTP_NODATA_NODATA;

	status = iCLASS_SendCommand(cmd, NULL, NULL);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		return FALSE;
	}

	return TRUE;
}

//
// Open_Collector_Output: Control the output of the open collector.
//
// param2:
//		01: Latch,output pulled to +5Vdc which is the same as default state	
//		02: Unlatchm output sinks to ground
// 		03: Unlatch momentarilly, output sinks to ground for P3 seconds
//
// param3:	The length in second that the open collector will momentarilly unlatch, only enabled if P2 = 03.
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
static int Open_Collector_Output(U8 param2, U8 param3)
{
	TCMD cmd;
	int status;
	 
	cmd.ins = OPEN_COLLECTOR_OUTPUT;
	cmd.param1  = 0x01;
	cmd.param2  = param2;
	cmd.param3  = param3;
	cmd.pairType = PTP_NODATA_NODATA;

	status = iCLASS_SendCommand(cmd, NULL, NULL);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		return FALSE;
	}

	return TRUE;
}
#endif
//
// Load_Key: This command allows for loading of master keys into the reader.
//
// rdKeyLoc: The reader key location to load the newly generated key.
//
// keyAndChkSum: 12 total bytes of data include 8 bytes for the encrpted key and 4 bytes for its checksum.
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
static int Load_Key(U8 rdKeyLoc,U8* keyAndChkSum)
{
	TCMD cmd;
	int status;
 
	cmd.ins = LOAD_KEY;
	cmd.param1  = 0;
	cmd.param2  = rdKeyLoc;
	cmd.param3  = 0x0C;
	cmd.pairType = PTP_DATA_NODATA;

	//DBPRINTF("Load_Key:\n");
	status = iCLASS_SendCommand(cmd, keyAndChkSum, NULL);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		return FALSE;
	}

	return TRUE;	
}

//
// Diversify_Key: This command diversifies the selected key with the CSN and places it into the reader memory.
//
// rdKeyLoc: The reader key location used for key diversification with the CSN.
//
// CSN: Card serial number. Select_Card command should be used.
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
#if 0
static int Diversify_Key( U8 rdKeyLoc, U8 * CSN)
{
	TCMD cmd;
	int status;
 
	cmd.ins = DIVERSIFY_KEY;
	cmd.param1  = 0;
	cmd.param2  = rdKeyLoc;
	cmd.param3  = 0x08;
	cmd.pairType = PTP_DATA_NODATA;

	//DBPRINTF("Diversify_Key:\n");
	status = iCLASS_SendCommand(cmd, CSN, NULL);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		return FALSE;
	}

	return TRUE;	
}
#endif
//
// LoadKeyIntoReader: Load protected key correspongding to area of page to reader for following authentication
//
// secretKey: Protect key for area of page.
// rdKeyLoc: Location of key of the reader.
//
// return: TRUE success. FALSE fail
//
int LoadKeyIntoReader(const U8 * secretKey, U8 rdKeyLoc)
{
	U8 random[8] = {0,};
	U8 keyAndChkSum[12] ={0,};

	if( !Ask_Random(random) )
		return FALSE;
	
	GetKeyAndChkSum(keyAndChkSum, random, DefaultPermExKeyCrc, secretKey, rdKeyLoc);

	if( !Load_Key( rdKeyLoc, keyAndChkSum ) )
		return FALSE;

	return TRUE;	
}

//
// write diversified key to card
//
// newRdKeyLoc: The key location in the reader to diverified.
//
#if 0
static int WriteDivKeyToCard(U8 pageNum, U8 selectCardArea,U8 oldRdKeyLoc, U8 newRdKeyLoc)
{
	U8 serialNum[8] = {0,};
	U8 newDivKey[8] = {0,};
	U8 selectPageMode =0;
	U8 cdKeyLoc;
	U8 appArea=0;
	U8 writeMode=0;

	if( ISOSTANDRAD == ISO_15693 )
	{
		writeMode = WBM_1_15693;
		selectPageMode =SP_AUTH_15693;
	}
	else if(ISOSTANDRAD == ISO_14443B)
	{	
		writeMode = WBM_1_14443B;
		selectPageMode =SP_AUTH_14443B;
	}
	else
	{
		DBPRINTF("Card protocol is not support!\n");
		return -1;
	}
	
	if( selectCardArea == SC_AREA1 )
	{
		cdKeyLoc = 0x03;
		appArea =APP_AREA1;
	}
	else if( selectCardArea == SC_AREA2 )
	{
		cdKeyLoc = 0x04;
		appArea =APP_AREA2;
	}

	if( !iCLASS_GetSN(serialNum) )
		return FALSE;

	if(!Diversify_Key(newRdKeyLoc, serialNum))
		return FALSE;

	if(!Retrieve_Data(newDivKey, 0x08) )
		return FALSE;

	if(!Select_Current_Key(oldRdKeyLoc) )
		return FALSE;

	if( pageNum > 0)
	{
		if(!Select_Card(SC_NOAUTH,RDKEYLOCCDTP(oldRdKeyLoc,gProtocolType),NULL) )	
			return FALSE;

		//DBPRINTF("\t\t\t\trdKeyLoc:%d,appArea:%d,pageNum:%d\n",oldRdKeyLoc, appArea, pageNum);
		if( !Page_Select(selectPageMode, RDKEYLOCCDKEYPGNUM(oldRdKeyLoc,appArea, pageNum),NULL) )
			return FALSE;
	}
	else
	{
		if( !Select_Card(selectCardArea,RDKEYLOCCDTP(oldRdKeyLoc,gProtocolType),NULL) )
		return FALSE;		
	}

	if( !Transmit_Write( writeMode, 0x08, WTP_1, cdKeyLoc, newDivKey) )
		return FALSE;

	return TRUE;
}
#endif
//
// Read_EEPROM: This command reads a selected address within the reader EEPROM.
//
// addr: EEPROM memory address.
//
// data:Data stored in the corresponding EEPROM address.
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
int Read_EEPROM(U8 addr, U8 *data)
{
	TCMD cmd;
	int status;
 
	cmd.ins = READ_EEPROM;
	cmd.param1	= 0;
	cmd.param2  = addr;
	cmd.param3  = 0x01;
	cmd.pairType = PTP_NODATA_DATA;

	U8 *readBuf = NULL;
	if( (readBuf = (U8*)MALLOC(cmd.param3)) ==NULL )
	{
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}
	//DBPRINTF("Read_EEPROM:\n");
	status = iCLASS_SendCommand(cmd, NULL, readBuf);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		FREE(readBuf);
		readBuf = NULL;
		return FALSE;
	}

	if(data != NULL )
		memcpy(data, readBuf, cmd.param3);

	FREE(readBuf);
	readBuf = NULL;
	
	return TRUE;
}

//
// Write_EEPROM: Write to a selected address within the reader EEPROM.
// 
// addr: EEPROM memory address. The only writeable address is 0xDD(Baud Rate)
//
// data: 1 byte data stored in the corresponding EEPROM address.
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
static int Write_EEPROM(U8 addr, U8 data)
{
	TCMD cmd;
	int status;
 
	cmd.ins = WRITE_EEPROM;
	cmd.param1  = 0;
	cmd.param2  = addr;
	cmd.param3  = 0x01;
	cmd.pairType = PTP_DATA_NODATA;

	U8 *writeBuf = NULL;
	if( (writeBuf = (U8*)MALLOC(cmd.param3)) ==NULL )
	{
		DBPRINTF("Alloc memory failure !\n");
		return FALSE;
	}
	*writeBuf = data;
	//DBPRINTF("Write_EEPROM:\n");
	status = iCLASS_SendCommand(cmd, writeBuf, NULL);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		FREE(writeBuf);
		writeBuf = NULL;
		return FALSE;
	}

	FREE(writeBuf);
	writeBuf = NULL;
	
	return TRUE;	
}

//
// Reset_RF: Reset the RF field of the reader. This should be done after writing the configuration(Block 1)	
//
// return: If command implement successfully,return TRUE,otherwise FALSE
//
#if 0
static int Reset_RF(void)
{
	TCMD cmd;
	int status;
 
	cmd.ins = RESET_RF;
	cmd.param1  = 0x40;
	cmd.param2  = 0;
	cmd.param3  = 0;
	cmd.pairType = PTP_NODATA_NODATA;

	//DBPRINTF("Reset_RF:\n");
	status = iCLASS_SendCommand(cmd, NULL, NULL);
	if(CMD_OK != status  )
	{
		iCLASS_Perror(status);
		return FALSE;
	}

	return TRUE;	
}
#endif
// 
// iCLASS_GetCSN: Get iCLASS card serial number.
//
// return: Success return TRUE,otherwise FALSE
//
int iCLASS_GetSN( U8 *serNum)
{	
	U8 buf[8];

	memset(buf,0,8);
	//DBPRINTF("\t\t=>iCLASS_GetSN:\n");
	if( !Select_Card(SC_NOAUTH,RDKEYLOCCDTP(0,gProtocolType),buf) )
		return FALSE;

	memcpy(serNum,buf,8);

	return TRUE;
}

//
// iCLASS_GetCardCFG: Read 8 bytes configuration data from block 1 of page 0
//
int iCLASS_GetCardCFG(U8 *cfg)
{
	U8 buf[8]={0,};

	if( !Select_Card( SC_NOAUTH, RDKEYLOCCDTP(0,gProtocolType), NULL) )
		return FALSE;
	if( !Transmit_Read( RBM_1_15693, 0x08, RTP_1, 0x01, buf) )
		return FALSE;

	memcpy(cfg, buf, 8);
	
	return TRUE;
}

//
//
//
int iCLASS_GetReaderCFG(U8 *cfg)
{
	U8 i;

	for(i=0; i<8; i++)
	{
		if(!Read_EEPROM( i, cfg+i))
			return FALSE;
	}

	return TRUE;
}

//
// iCLASS_SetBaudRate: Set Baudrate.
//
int iCLASS_SetBaudRate(U32 baudRate)
{	
	U8 data;

	switch (baudRate){
		case B9600:
			data = 0x57;
			break;
		case B19200:
			data = 0x2D;
			break;
		case B38400:
			data = 0x15;
			break;
		case B57600:
			data = 0x0E;
			break;	
//		case B115200:
//			data = 0x04;
//			break;	
		default:
			data = 0x0E;
			break;	
	}

	if( !Write_EEPROM(0xDD, data) )
		return FALSE;

	return TRUE;
}

//
// iCLASS_Read: Read blocks from iCLASS card
//
// pageNum: 		Page number,the page to select
// addr: 			Blocks address to read. Address must be within the appliaction area limit
// pageAuthMode: Select page method and mode
// ISOSTANDRAD: 	The international ISO standrad to complied with
// rdKeyLoc:		The location of key of the reader
// selectCardArea: 	Key of the card for application area
// blockData:		The buffer to store the data read from card.
//
// return: If communication fail or other exception return nagetive,otherwise return blocks readed.
//
int iCLASS_Read(U8 pageNum,U8 addr, U8 blocks, U8 rdKeyLoc, U8 selectCardArea, U8* blockData)
{
//	U8 blocks =4;
	int i;
	int oneBlockNum;
	int fourBlocksNum;
	U8 readOneBlockMode;
	U8 readFourBlocksMode;
	int offset =0;
	U8 appArea=-1;
	U8 selectPageMode;
	U8 oneBlockBuf[BLOCKSIZE] ={0,};
	U8 fourBlocksBuf[4*BLOCKSIZE] ={0,};
//	U8 *buf = NULL;

	oneBlockNum = blocks%4;
	fourBlocksNum = blocks/4;

	if( ISOSTANDRAD == ISO_14443B )
	{
		readOneBlockMode = RBM_1_14443B;
		readFourBlocksMode = RBM_4_14443B;
		selectPageMode =SP_AUTH_14443B;
	}
	else if( ISOSTANDRAD == ISO_15693 )
	{
		readOneBlockMode = RBM_1_15693;
		readFourBlocksMode = RBM_4_15693;
		selectPageMode =SP_AUTH_15693;
	}
	else
	{
		DBPRINTF("Current ISOStandrad not support!\n");
		return -1;
	}

	if( selectCardArea == SC_AREA1 )
		appArea = APP_AREA1;
	else if(selectCardArea == SC_AREA2 )
		appArea = APP_AREA2;

#ifdef SHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif	
	
	if(!Select_Current_Key(rdKeyLoc))
		return -1;
		
	if( pageNum > 0)
	{
		if(!Select_Card(SC_NOAUTH,RDKEYLOCCDTP(rdKeyLoc,gProtocolType),NULL) )	
			return -1;

		//DBPRINTF("\t\tiCLASS_Read:rdKeyLoc:%d,appArea:%d,pageNum:%d\n",rdKeyLoc, appArea, pageNum);
		if( !Page_Select(selectPageMode, RDKEYLOCCDKEYPGNUM(rdKeyLoc,appArea, pageNum),NULL) )
			return -1;
	}
	else
	{
		//DBPRINTF("\t\t\tiCLASS_Read:rdKeyLoc:%d,appArea:%d,pageNum:%d\n",rdKeyLoc, appArea, pageNum);
		if( !Select_Card(selectCardArea,RDKEYLOCCDTP(rdKeyLoc,gProtocolType),NULL) )
		return -1;		
	}

	i = fourBlocksNum;
	while(i)
	{
		if( !Transmit_Read(readFourBlocksMode, 0x20, RTP_4, addr+offset, fourBlocksBuf) )
			return -1;
		if( !Retrieve_Data(fourBlocksBuf, 0x20))
			return -1;

		memcpy(blockData+offset*BLOCKSIZE,fourBlocksBuf,4*BLOCKSIZE);
		offset += 4;
		--i;
	}		

	i = oneBlockNum;
	while(i)
	{
		if( !Transmit_Read(readOneBlockMode, 0x08, RTP_1, addr+offset, oneBlockBuf) )
			return -1;

		memcpy(blockData+offset*BLOCKSIZE,oneBlockBuf, BLOCKSIZE);
		offset += 1;
		--i;
	}

#ifdef SHOWTIME
 	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
     	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("\t\tRead %d bytes use time:%f ms\n",offset*BLOCKSIZE,msec);
#endif  
  	return offset;	
}

//
// iCLASS_Write: Write blocks to iCLASS card
//
// pageNum: 		Page number,the page to select
// addr: 			Blocks address to write. Address must be within the appliaction area limit
// pageAuthMode: Select page method and mode
// ISOSTANDRAD: 	The international ISO standrad to complied with
// rdKeyLoc:		The location of key of the reader
// selectCardArea: 	Key of the card for application area
// writeData:		The data Write to card
//
// return: If communication fail or other exception return nagetive,otherwise return blocks readed.
//
int iCLASS_Write(U8 pageNum, U8 addr, U8 blocks,  U8 rdKeyLoc, U8 selectCardArea, U8 *writeData)
{
	int i,offset=0;
	U8 writeMode;
	U8 appArea=-1;
	U8 selectPageMode;

	if( ISOSTANDRAD == ISO_15693 )
	{
		writeMode = WBM_1_15693;
		selectPageMode =SP_AUTH_15693;
	}
	else if(ISOSTANDRAD == ISO_14443B)
	{	
		writeMode = WBM_1_14443B;
		selectPageMode =SP_AUTH_14443B;
	}
	else
	{
		DBPRINTF("Card protocol is not support!\n");
		return -1;
	}


	if( selectCardArea == SC_AREA1 )
		appArea = APP_AREA1;

	else if(selectCardArea == SC_AREA2 )
		appArea = APP_AREA2;

#ifdef SHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif	
		
	if( !Select_Current_Key(rdKeyLoc) )
		return -1;

	if( pageNum > 0)
	{
		if(!Select_Card(SC_NOAUTH,RDKEYLOCCDTP(rdKeyLoc,gProtocolType),NULL) )	
			return -1;

		//DBPRINTF("\t\t\t\trdKeyLoc:%d,appArea:%d,pageNum:%d\n",rdKeyLoc, appArea, pageNum);
		if( !Page_Select(selectPageMode, RDKEYLOCCDKEYPGNUM(rdKeyLoc,appArea, pageNum),NULL) )
			return -1;
	}
	else
	{
		if( !Select_Card(selectCardArea,RDKEYLOCCDTP(rdKeyLoc,gProtocolType),NULL) )
			return -1;		
	}

	for(i=0;i<blocks;i++)
	{
		if( !Transmit_Write(writeMode, 0x08, WTP_1, addr+offset, writeData+offset*BLOCKSIZE) )
			return FALSE;

		offset +=1;
	}
	
#ifdef SHOWTIME
	gettimeofday(&tv_end, &tz);
      eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      msec = (eusec -susec)/1000.0 ;
      DBPRINTF("\t\t\twrite %d bytes use time:%f ms\n",offset*BLOCKSIZE,msec);
#endif

	 return offset;
}

//
// iCLSCheckReader: Check out whether it is a iCLASS machine
//
// return: If it is a iCLASS machine return TRUE. otherwise FALSE
// 
int iCLSCheckReader(void)
{
	if(!Ask_Random(NULL))
		return FALSE;
	
	return TRUE;
}

