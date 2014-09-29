#ifndef HID_iCLASS_H
#define HID_iCLASS_H

#ifndef BYTE 
#define BYTE unsigned char
#endif

#ifndef U8
#define U8 unsigned char
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef U32
#define U32 unsigned int
#endif

#ifndef FALSE 
#define FALSE 0 
#endif 

#ifndef TRUE
#define TRUE 1
#endif 

//Generate Select_Card parameter: rdKeyLocCdTp
#define RDKEYLOCCDTP(a,b) ( ( ( (U8)a & 0x07 ) << 4 ) | b )

//Generate Page_Select command parameter :rdKeyLocCdKeyPgNum
#define RDKEYLOCCDKEYPGNUM(a,b,c) ( (((U8)a & 0x07) << 5) |((U8)b << 4) | ((U8)c & 0x0F) ) 

#define BLOCKSIZE 8	//bytes of a block in card
#define AREA1_ENDBLOCK_16K_16 19
#define AREA2_ENDBLOCK_16K_16 32
#define AREA1_ENDBLOCK_16K_2	19
#define AREA2_ENDBLOCK_16K_2	256
#define AREA1_ENDBLOCK_2K	19
#define AREA2_ENDBLOCK_2K	32
#define AREA1_SIZE_16K_16 13
#define AREA2_SIZE_16K_16 13
#define AREA1_SIZE_16K_2 13
#define AREA2_SIZE_16K_2 26
#define AREA1_SIZE_2K 13
#define AREA2_SIZE_2K 13
#define AREA1_STARTBLOCK_16K_16 6
#define AREA2_STARTBLOCK_16K_16 19
#define AREA1_STARTBLOCK_16K_2 6
#define AREA2_STARTBLOCK_16K_2 19
#define AREA1_STARTBLOCK_2K 6
#define AREA2_STARTBLOCK_2K 19

/*Select page with following mode */
#define SP_NOAUTH_15693	0x05
#define SP_NOAUTH_14443B	0x04
#define SP_AUTH_15693 		0x0D
#define SP_AUTH_14443B 		0x0C

/* Select the location of the key in the reader */
#define RKEY1		0x01
#define RKEY2		0x02
#define RKEY3		0x03
#define RKEY4		0x04
#define RKEY5		0x05
#define RKEY6		0x06
#define RKEY7		0x07

#define SC_NOAUTH 0x00 //Select card with no authentication
#define SC_AREA1	0x30	  //Select card with authentication - Key of application Area1(BLOCK3)
#define SC_AREA2	0x10  //Select card with authentication - Key of application Area2(BLOCK4)

//International ISO Standrad complied with 
#define ISO_14443B 0x01 //14443B protocol 
#define ISO_15693 0x02 //iCLASS card type
#define ISO_RFU 0x04 //RFU protocol
#define ISO_14443A 0x08 //14443A protocol


extern U8 ISOSTANDRAD;
extern U8 gProtocolType;
extern const U8 DefaultSecretKey[16][8];
extern char *AREA[4];

int (* iCLASS_raw_read)();
int  (* iCLASS_raw_write)(int value);
int (* iCLASS_raw_poll)();
int (* iCLASS_raw_flush_output)();
void (* iCLASS_raw_close)();
int (* iCLASS_raw_flush_input)();

int iCLSCheckReader(void);
int iCLASS_GetSN(U8 *serNum);
int iCLASS_Read(U8 pageNum,U8 addr,U8 blocks,U8 rdKeyLoc, U8 selectCardArea, U8* blockData);
int iCLASS_Write(U8 pageNum,U8 addr,U8 blocks,U8 rdKeyLoc, U8 selectCardArea, U8* writeData);
int iCLASS_GetReaderCFG(U8 *cfg);
int iCLASS_GetCardCFG(U8 *cfg); //8 byte configuration data from block 1 of page 0 of card
int iCLASS_SetBaudRate(U32);
int LoadKeyIntoReader(const U8 * secretKey, U8 rdKeyLoc);

#endif

