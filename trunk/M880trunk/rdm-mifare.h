#ifndef __RDM_MIFARE_H_
#define __RDM_MIFARE_H_

#ifndef U8 
#define U8 unsigned char 
#endif 


#ifndef U32 
#define U32 unsigned int
#endif
#ifndef U16 
#define U16 unsigned short
#endif

#define IDLE			0x00
#define ALL			0x01
#define REQA_IDLE		0x26
#define REQA_ALL		0x52

#define STX			0
#define STATION			1	
#define DATALENGTH		2		
#define COMMAND			3		
#define STATUS			3
#define DATA			4


#define AUTH_KEYB		0x02
#define AUTH_KEYA		0x00

/*ISO14443 Type A Command */
#define REQA			0x03
#define ANTICOLLA		0x04
#define SELECTA			0x05
#define HALTA			0x06

/*Aplication Command */
#define MF_READ			0x20		
#define MF_WRITE		0x21		
#define MF_INIT_VAL		0x22
#define MF_DEC			0x23
#define MF_INC			0x24
#define MF_GET_SNR		0x25
/*System Command  */
#define SET_ADDRESS		0x80
#define SET_BAUDRATE		0x81
#define SET_SERNUM		0x82
#define GET_SERNUM		0x83
#define WRITE_USERINFO		0x84
#define READ_USERINFO		0x85
#define GET_VERNUM		0x86
#define CONTROL_LED		0x88
#define CONTROL_BUZZER		0x89

/* Status and Error code */
#define COMM_OK			0x00
#define PARAM_OK		0x80
#define COMM_ERR		0xFF
#define	SET_FAILED		0x81
#define	TIME_OUT		0x82
#define NO_CARD			0x83
#define AUTH_FAILED		0x83
#define RECEIVE_ERROR		0x84
#define UNKNOWN_ERROR		0x87
#define BAD_PARAM		0x85
#define PASS_FAILED		0x8C
#define BAD_COMMAND		0x8F

int (* mifare_raw_read)();
int  (* mifare_raw_write)(int value);
int (* mifare_raw_poll)();
int (* mifare_raw_flush_output)();
void (* mifare_raw_close)();

int Mifare_REQA(U8 mode);
int SetBaudRate(U32 );
int Mifare_AnticollA(U32 *);
int Mifare_SelectA(U32);
int Mifare_GetSerNum(U8 *);
int Mifare_SetSerNum(U8 *);
int Mifare_REQA(U8);
int Mifare_HaltA();
int Mifare_SelectA(U32);
int Mifare_Read(U8 addr, U8 blocks, U8 *key, U8 auth,U8 mode, U8 *buf, U32 * uid);
int Mifare_Write(U8 addr, U8 blocks, U8 *key, U8 auth, U8 mode, U8 * buf, U32 * uid, int protect);
int Mifare_Get_SNR(U8 mode, U8 halt, U8 *serialnumber);
void Mifare_Perror(int errno);
void Mifare_GetSecInfo(const U8 BlockAddr, U8 *sector, U8 *AllowBlocks, int protect);//dsl 2012.1.16
#endif
