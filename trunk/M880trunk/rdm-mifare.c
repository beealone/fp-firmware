#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include "serial.h"
#include "rdm-mifare.h"
#include "zlg500b.h"
#include "arca.h"
#include "options.h"

#define bool int

#define CRC_LEN 	4
#define	_LEN_		2		
#define MAX_BLOCKDATA_SIZE	((16 * (4-1)-1)*16)
#define MAX_TEMPLATE  (MAX_BLOCKDATA_SIZE -(CRC_LEN + _LEN_))

/* TRY_TIMES is highly dependent on Processor HZ */
#define TRY_TIMES (2*1000)
#define MAX_DATALENGTH_VAL 300//123->300, dsl 2012.1.18

 /****************MIFARE APPLICTION COMMAND FORMAT**************************
 * 	
 * Field:	STX  STATIONID  DATALEN   CMD/STATUS   DATAN       BCC     ETX
 *  
 * offset:	0	1	  2	    3		4          N+4     N+5	
 * 
 * value:	0xAA	0x00	 N+1	   CMD	   DATA[0..N-1]    BCC     0xBB
 *
 * Desc:	
 * 		BCC = STATIONID ^ DATALEN ^ CMD/STATUS ^ DATA[0] ^ DATA[1]...^... DATA[N-1]
 *
 *************************************************************************/



static U8 PacketBuf[256] = {0xAA, 0x00,};


/**************************MISC MACRO *****************************************
 *
 * 
 *  PACKET_BCC(PacketBuf):	CheckSum PacketBuf into PacketBuf[OFFSET_BCC(Pac				ketBuf)].
 *
 *  OFFSET_ETX(PacketBuf):	Offset of ETX in PacketBuf.
 *
 *  PACKET_LEN(PacketBuf):	Total Bytes Length in PacketBuf.
 *
 *
 ***************************************************************************/

#define OFFSET_BCC(PACKET)		(DATALENGTH + PACKET[2] + 1)			
#define OFFSET_ETX(PACKET)		(OFFSET_BCC(PACKET) + 1)		
#define PACKET_LEN(PACKET)		(OFFSET_ETX(PACKET) + 1)

#define PACKET_BCC(PACKET)\
do\
{int i = 1, bcc = 0;\
for(; i<OFFSET_BCC(PACKET); i++)\
	bcc ^= PACKET[i];PACKET[OFFSET_BCC(PACKET)] = bcc;\
} while(0);\

void PrintHex ( U8* buf, int len, bool prefix, char fmt) 
{
	int i;
	
	for(i=0;i<len;i++){
	       if(i%16 == 0 && prefix){ 
		       printf("\n %4.4X: " ,(unsigned short)i);
	       }
	       else{
		       putchar(' ');
	       }
	       switch(fmt){
		case 'c':
			printf("%c", buf[i]);
			break;
		case 'd':
			printf("%2d", buf[i]);
			break;
		default:
			printf("%2.2x", buf[i]);
			break;
	       }
	}
	putchar('\n');
}
	
void Mifare_Perror( int errno )
{
	DBPRINTF("Mifare Reader return status:%d\n", errno);
	switch (errno){
		case COMM_ERR:
			DBPRINTF("communication Failed!\n");
			break;
		case SET_FAILED:
			DBPRINTF("Parameter Setting Failed!\n");
			break;
		case TIME_OUT:
			DBPRINTF("Timeout!\n");
			break;
		case AUTH_FAILED:
			DBPRINTF("Card NOT found or Auth Failed!\n");
			break;
		case RECEIVE_ERROR:
			DBPRINTF("Recive data from card failed !\n");
			break;
		case UNKNOWN_ERROR:
			DBPRINTF("Unkown Error!\n");
			break;
		case BAD_PARAM:
			DBPRINTF("Parameters Error!\n");
			break;
		case BAD_COMMAND:
			DBPRINTF("Command Error!\n");
			break;
		default:
			break;
	}
}

#if 0
static void dbg_Packet(const char* tips, char fmt)
{	printf("%s", tips);	
	PrintHex(PacketBuf,  PACKET_LEN(PacketBuf), 1, fmt);
}
#endif
	
/******************************************************************************
 * SendCommand:		Construct and  Send  Command Packet to Mifare Reader\
 *
 * 
 *	cmd:		Command Type	
 *	data:		data to be send to Reader as the DATAN field in the Comand 			Packet
 *	len: 		Data length (bytes)
 *
 * Return:		void .
 *
 ***************************************************************************/

static void SendCommand(U8 cmd, U8 *data, U8 len)
{	
	U8  i = 0;
	
	memset(PacketBuf + 2, sizeof PacketBuf -2, 0);
	PacketBuf[DATALENGTH] = len + 1;
	PacketBuf[COMMAND] = cmd;
	if(data) {
		memcpy(PacketBuf + DATA, data, len);
	}
	PACKET_BCC(PacketBuf);
	PacketBuf[OFFSET_ETX(PacketBuf)] = 0xBB;
	//dbg_Packet("->", 'x');
	for(i = 0 ; i < PACKET_LEN(PacketBuf); i++) {
		mifare_raw_write(PacketBuf[i]);
		//zsliu test	fix a bug read error cardnum 这种问题引起MF写卡太慢所以去掉
		//DelayMS(1); //dddddddddddddddddd
	}
	//mifare_raw_flush_output();
}

/**************************************************************************
 * GetReply:	
 *	Get Response of last Command, The Reply as a Reply Packet in PacketBuf. 
 *
 * Return:
 *	-2:	Communication error or CheckSum error.
 *	PacketBuf[STATUS]:	the Reply Packet is  integrate and CheckSum is ok.
 *  
 *
 *******************d********************************************************/

static int GetReply()
{
  	int rdindex = 0, bcc = 0, left = 3;
	U16 data = 0;
	int rdcnt = TRY_TIMES;

#ifdef New_U8
	static int HaveInited=0;

	if (gOptions.MifareAsIDCard==1)
	{
		if (HaveInited==1)
		{
			if(mifare_raw_poll()==0)    return -1;
		}
	}
#endif

	while(left > 0 && rdcnt){
		if(mifare_raw_poll()){	
			//rdcnt = TRY_TIMES;
			data = mifare_raw_read();
			left --;
			if(rdindex == STX && data == 0xAA){
				rdindex ++;
			}
			else if (rdindex == STX && data != 0xAA){
				DBPRINTF("The STX is %2.2x, expect 0xAA\n", data);
				break;
			}
			else if(rdindex == STATION && data == 0x00){
				rdindex ++;
			}
			else if (rdindex == STATION && data != 0x00){
				DBPRINTF("The STATIOIN is %2.2x, expected 00\n", data);
				break;
			}
			else if(rdindex == DATALENGTH && data <= MAX_DATALENGTH_VAL + 1){
				PacketBuf[DATALENGTH] = data;
				left = data + 2;
				rdindex ++;
			}
			else if(rdindex == DATALENGTH && (data > MAX_DATALENGTH_VAL + 1 || data ==0)){
				DBPRINTF("Too many data coming(%d)\n", data);
				break;
			}
			else if(rdindex > DATALENGTH)
				PacketBuf[rdindex ++] = data;
		}
		else{
			DelayUS(2000);
			rdcnt--;
			//DBPRINTF("Waiting Data arrival %d!\n", rdcnt);
		}
	}
	if(left > 0){
		DBPRINTF("Data Invalid! rdindex = %d, left = %d  \n", rdindex, left);
		return -2;
	}
	else if(rdcnt == 0){
		DBPRINTF("Wait Reader response Time out!\n");
		return -2;
	}

	if(PacketBuf[rdindex -1] != 0xBB){
		DBPRINTF("Data Invalid, rdindex = %d \n", rdindex);
		return -2;
	}

	//dbg_Packet("->", 'x');

	bcc = PacketBuf[rdindex - 2];
	PACKET_BCC(PacketBuf);
	if(bcc != PacketBuf[OFFSET_BCC(PacketBuf)]){
		DBPRINTF("CheckSum error\n");
		return -2;
	}
	else{
		//DBPRINTF("All data receive OK!\n");
#ifdef New_U8
		if (gOptions.MifareAsIDCard==1)
			HaveInited=1;
#endif
		return PacketBuf[STATUS];
	}
}

/*****************************************************************************
 * Mifare_Read:		low level Mifare routine for read block 
 *
 * 	addr:		the block addr from where read data, range[0..63]
 * 	blocks:		the blocks number to be read[1..4]
 * 	key:		key , 6 bytes key 
 * 	auth:		auth mode, AUTH_KEYA or AUTH_KEYB
 * 	mode:		Request mode, ALL or IDLE
 * 	buf:		pointer to buf. size: blocks << 4
 * 	return:	
 * 	-3:		Invalid parameters.
 *	-2:		Communication  or CheckSum error.
 *	0:		the communication  is ok 
 *	errno:		the error number returned by Mifare Reader
 *
 **************************************************************************/


int Mifare_Read(U8 addr, U8 blocks, U8 *key, U8 auth, U8 mode, U8 *buf, U32 * uid) 
{
	static U8 data[9] = {0, };
	int status = 0, nbr = 0;//blocks;
	U8 SecAddr = 0;
#if 0	
	if(addr > 0x3F){
		DBPRINTF("invalid block addr (%d)\n", addr);
		return -2;
	}
	if((addr + nbr) > ((addr >> 2) +1) << 2){
		nbr = (((addr >> 2) +1) << 2) - addr;
	}
#endif
	if(addr == 0 || blocks == 0)
	{
		printf("invalid block addr:%d, blocks:%d\n", addr, nbr);
		return BAD_PARAM;
	}

	Mifare_GetSecInfo(addr, &SecAddr, (U8*)&nbr, 1);
	printf("%s, %s,-- blocks: %d nbr: %d\n",__FILE__,__FUNCTION__,blocks,nbr);
	if(blocks < nbr)
	{
		nbr = blocks;
	}

	if (addr < 128)
	{
		data[0] = auth | mode;
		data[1] = nbr;
		data[2] = addr;
		memcpy(data + 3, key, 6);

		SendCommand(MF_READ, data, sizeof data);	
		status = GetReply();
		if(COMM_OK == status ){
			memcpy(uid, PacketBuf + DATA, 4); /* 4 bytes uid, Little endian */
			memcpy(buf, PacketBuf + DATA + 4, nbr <<4);
			DBPRINTF("Read %d bytes from Card ok\n", nbr <<4);
		}
	}
	else
	{
		int i = 0;
		int j = 0;
		while(nbr)
		{
			if(nbr > 3)
				j=3;
			else
				j=nbr;
			data[0] = auth | mode;
			data[1] = j;
			data[2] = addr+i;
			memcpy(data+3, key, 6);

			SendCommand(MF_READ, data, sizeof(data));
			status = GetReply();
			if(COMM_OK == status)
			{
				memcpy(uid, PacketBuf + DATA, 4);
				memcpy(buf + i*(1<<4), PacketBuf + DATA + 4, j <<4);
				i += 3;
				if(nbr > 3)
					nbr -= 3;
				else
					nbr = 0;
				DBPRINTF("big sector, Read %d bytes to Card ok\n", 1 <<4);
			}
			else
			{
				DBPRINTF("big sector, Read to Card Failed:%d\n",status);
				break;
			}
		}
	}
	return (status == 1)?PacketBuf[DATA]:status;
}

/*****************************************************************************
 * Mifare_Write:	low level Mifare routine for read block 
 *
 * 	addr:		the block addr to where write to .
 * 	blocks:		the blocks number to be read
 * 	key:			key  
 * 	auth:		auth mode, AUTH_KEYA or AUTH_KEYB
 * 	mode:		Request mode, ALL or IDLE
 * 	buf:	
 * 	protect:	if 1 protect trailer block from writing ,or write trailer block as normal 
 * 	return:	
 * 		-3:		Invalid parameters.
 *		-2:		Communication  or CheckSum error.
 *	    	 0:		the communication  is ok 
 *		errno:		the error number returned by Mifare Reader
 *
 **************************************************************************/


int Mifare_Write(U8 addr, U8 blocks, U8 *key, U8 auth, U8 mode, U8 * buf, U32 * uid, int protect)
{	

	//static U8 data[128] = {0, };
	static U8 data[300] = {0};
	int status = 0, nbr = 0;//blocks;
#if 0
	if(addr > 0x3F){
		DBPRINTF("invalid block addr (%d)\n", addr);
		return -2;
	}
	if((addr + nbr) > ((addr >> 2) +1) << 2){
		nbr = (((addr >> 2) +1) << 2) - addr;
	}
	if(protect&( (addr&0x03) == 0x03|| ((addr+nbr - 1)& 0x03) == 0x03)){
		DBPRINTF("The Tralier block hit, no permission to write!\n");
		return -2;
	}
#endif
	U8 SecAddr=0;
	if(addr == 0 || blocks == 0)
	{
		printf("invalid block addr:%d, blocks:%d\n", addr, nbr);
		return BAD_PARAM;
	}

	Mifare_GetSecInfo(addr, &SecAddr, (U8*)&nbr, protect);
	if(nbr == 0)
	{
		printf("Error:protect, make 0 AllowBlocks\n");
		return BAD_PARAM;
	}
	if(blocks < nbr)
	{
		nbr = blocks;
	}

	if (addr < 128)
	{
		data[0] = auth | mode;
		data[1] = nbr;
		data[2] = addr;
		printf("data[2]=%d\n", addr);

		memcpy(data+3, key, 6);
		memcpy(data+9, buf, nbr << 4);
		SendCommand(MF_WRITE, data, (nbr << 4) + 9);
		status = GetReply();
		if(COMM_OK == status){
			/* 4 bytes uid, Little endian */
			memcpy(uid, PacketBuf + DATA, 4); 
			DBPRINTF("Write %d bytes to Card ok\n", nbr<<4);
		}
		else{
			DBPRINTF("Write to Card Failed\n");
		}
	}
	else
	{
		int i = 0;
		do
		{
			//printf("alex debug addr=%d,nbr=%d,i=%d\n",addr+i,nbr,i);
			data[0] = auth | mode;
			data[1] = 1;
			data[2] = addr+i;
			memcpy(data+3, key, 6);
			memcpy(data+9, (buf + i*16), 1 << 4);
			SendCommand(MF_WRITE, data, (1 << 4) + 9);
			status = GetReply();
			if(COMM_OK == status)
			{
				i++;//zhc
				memcpy(uid, PacketBuf + DATA, 4);
				DBPRINTF("big sector, Write %d bytes to Card ok\n", 1 <<4);
			}
			else
			{
				DBPRINTF("big sector, Write to Card Failed:%d\n",status);
				break;
			}
		}while(--nbr);

	}
	return (status == 1)?PacketBuf[STATUS + 1]:status;
}


int Mifare_Initval(U8 addr, U8 auth, U8 mode, U8 * key, U32 val)
{
	U8 data[12] = {0, };
	int status;


	data[0] = auth | mode;
	data[1] = addr & 0x0F;
	memcpy(data + 2, key, 6);
	memcpy(data + 8, &val, 4);
	
	SendCommand(MF_INIT_VAL, data, sizeof data);
	status = GetReply();
	return (status == 1)?PacketBuf[STATUS + 1]:status;
}

int Mifare_Dec(U8 addr, U8 auth, U8 mode, U8 * key, U32 val)
{
	U8 data[12] = {0, };
	int status;


	data[0] = auth | mode;
	data[1] = addr & 0x0F;
	memcpy(data + 2, key, 6);
	memcpy(data + 8, &val, 4);
	
	SendCommand(MF_DEC, data, sizeof data);
	status = GetReply();
	return (status == 1)? PacketBuf[STATUS + 1]:status;
}


int Mifare_Inc(U8 addr,U8 auth, U8 mode, U8 * key, U32 val)
{
	U8 data[12] = {0, };
	int status;


	data[0] = auth | mode;
	data[1] = addr & 0x0F;
	memcpy(data + 2, key, 6);
	memcpy(data + 8, &val, 4);
	
	SendCommand(MF_INC, data, sizeof data);
	status = GetReply();
	return (status == 1)?PacketBuf[STATUS + 1]:status;
}


int Mifare_Get_SNR(U8 mode, U8 halt, U8 *serialnumber)
{
	U8 data[2] = {0, 0};
	int status;
	int cardtype=0;

	data[0] = mode;
	data[1] = halt & 0x01;

	SendCommand(MF_GET_SNR, data, sizeof data);
	status = GetReply();

	if(status==COMM_OK)
	{
		memcpy(serialnumber, PacketBuf+DATA+1, 4); 
		memcpy(&cardtype,PacketBuf+DATA,1);
		printf("Mifare card type is %d\n",cardtype);
	}
	else
	{
		status=PacketBuf[STATUS];
	}

	return status;
}

int SetAddr( U8 address)
{
	U8 data[1];
	int status;

	data[0] = address;
	SendCommand(SET_ADDRESS, data, sizeof data);
	status = GetReply();
	return (status == 1)?PacketBuf[STATUS + 1]: status;
}

/* The Reader Setting  Must match the Setting of Serial */
int SetBaudRate( U32 baudrate)
{
	U8 data;
	int status;


	switch (baudrate){
		case B9600:
			data = 0x00;
			break;
		case B19200:
			data = 0x01;
			break;
		case B38400:
			data = 0x02;
			break;
		case B57600:
			data = 0x03;
			break;	
		case B115200:
			data = 0x04;
			break;	
		default:
			data = 0x00;
			break;	
	}
	SendCommand(SET_BAUDRATE, &data, sizeof data);
	status = GetReply();
	if(COMM_OK == status){
		DBPRINTF("The baudrate of Mifare Reader has been changed!\n");
	}
	return (status == 1)?PacketBuf[STATUS + 1]: status;
}


int Mifare_Ctrl_LED(U8 duration, U8 times)
{
	U8 data[2] = {0, 0};
	int status;

	data[0] = duration;
	data[1] = times;
	SendCommand(CONTROL_LED, data, sizeof data);
	status = GetReply();
	return (status == 1)?PacketBuf[STATUS + 1]: status;

}

int Mifare_Ctrl_Buzzer(U8 duration, U8 times)
{
	U8 data[2] = {0, 0};
	int status = 0;

	data[0] = duration;
	data[1] = times;
	SendCommand(CONTROL_BUZZER, data, sizeof data);
	status = GetReply();

	return (status == 1)?PacketBuf[STATUS + 1]: status;
}

unsigned long crc_table[256];
static int crc_table_computed = 0;
void make_crc_table(void)
{
	U32 c;
	U32 n, k;
   
	for (n = 0; n < 256; n++) {
		c = (unsigned long) n;
		for (k = 0; k < 8; k++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[n] = c;
	}
	crc_table_computed = 1;
}
    
U32 update_crc(U32 crc, U8 *buf, U32 len)
{
	U32 c = crc;
	U32 n;
   
	if (!crc_table_computed)
		make_crc_table();
	for (n = 0; n < len; n++) {
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c;
}
   
U32 crc(U8 *buf, U32 len)
{
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

int Mifare_GetSerNum(U8 * SerNum)
{
	int status;
	SendCommand(GET_SERNUM, NULL, 0);
	status = GetReply();
	if(status == COMM_OK ){
		memcpy(SerNum, PacketBuf+DATA+1, 8);
	}
	return (status == 1)? PacketBuf[DATA]:status;
}
int Mifare_SetSerNum( U8 * SerNum)
{	
	int status;

	SendCommand(SET_SERNUM, SerNum, 8);
	status = GetReply();
	return (status == 1)? PacketBuf[DATA]:status;
}

int Mifare_ReadInfo(U8 zone, U8 * buf, int len)
{
	int status = 0;
	U8 data[2];

	if(zone > 3)  zone = 0;
	if(len  > 120) len = 120;
	data[0] = zone;
	data[1] = len;
	SendCommand(READ_USERINFO, data, sizeof data);
	status = GetReply();
	if(COMM_OK == status){
		memcpy(buf, PacketBuf+DATA, len);
	}
	return (status == 1)? PacketBuf[DATA]: status;
}

int Mifare_WriteInfo(U8 zone, U8 * buf, int len)
{
	int status = 0;
	
	static U8 data[122];
	if(zone > 3)  zone = 0;
	if(len > 120 ) len = 120;

	data[0] = zone;
	data[1] = len;
	memcpy( data + 2, buf, len);

	SendCommand(WRITE_USERINFO, data, len + 2);
	status = GetReply();
	
	return (status == 1)? PacketBuf[DATA]: status;
}

/* ISO14443A  Commands */
int Mifare_REQA(U8 mode)
{
	U8 data[1];
	int status = 0;
	data[0] = mode;
	SendCommand(REQA, data, 1);
	status = GetReply();
	return  (status == 1)? PacketBuf[DATA]:status;
}

int Mifare_AnticollA(U32 * CardNumber )
{
	int status = 0;
	SendCommand(ANTICOLLA, NULL, 0);
	status = GetReply();
	if(COMM_OK == status){
		memcpy(CardNumber, PacketBuf+DATA+1, 4);
	}
	return  (status == 1)? PacketBuf[DATA]:status;
}

int Mifare_SelectA(U32  uid)
{
	static U8 data[4] = {0, };
	int status = 0;
	memcpy(data, &uid, 4);
	SendCommand(SELECTA, data, 4);
	status = GetReply();
	return(status == 1)?PacketBuf[DATA]:status;
}

int Mifare_HaltA()
{
	int status = 0;
	SendCommand(HALTA, NULL, 0);
	status = GetReply();
	return(status == 1)?PacketBuf[DATA]:status;
}
/*Mifare  Routines   End */

/*****************************************************************************
 *  Mifare_GetSecInfo:		low level Get the sector info
 *  
 *  BlockAddr:the block addr from where it started, range[0..256]
 *  sector:the sector of the BlockAddr.
 *  AllowBlocks: the allowed block,because of s70,it is not only 4 blocks
 *  protect:if 1 protect trailer block from writing ,or write trailer block as normal
 *  note:for S70,sector 32-39 contain 15 blocks,but i only use 0-2,4-6,8-10,12-14 block;
 *  because i can not write anything to block 3,7,11,15;please notice this information.
 *  **************************************************************************/
void Mifare_GetSecInfo(const U8 BlockAddr, U8 *sector, U8 *AllowBlocks, int protect)
{
	if(BlockAddr <= (U8)0x7F)
	{
		*sector = BlockAddr >> 2;
		if(protect)
		{
			*AllowBlocks = (3 - BlockAddr % 4);
		}
		else
		{
			*AllowBlocks = (4 - BlockAddr % 4);
		}
	}
	//else if(BlockAddr <= (U8)0xFF)
	else
	{
		*sector = 32 + ((BlockAddr >> 4) - 8);
		if(protect)
		{
			*AllowBlocks = (15 - BlockAddr % 16);
		}
		else
		{
			*AllowBlocks = (16 - BlockAddr % 16);
		}
	}

	return;
#if 0
	else
	{
		DBPRINTF("invalid block addr (%d)\n", BlockAddr);
	}
#endif
	//printf("Mifare_GetSecInfo addr:%d, sector:%d, protect:%d, AllowBlocks:%d\n", BlockAddr, *sector, protect, *AllowBlocks);
}
