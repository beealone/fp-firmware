#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <asm/unaligned.h>
#include "rdm-mifare.h"
#include "zlg500b.h"
#include "serial.h"
#include "arca.h"
#include "options.h"
#include "utils.h"
#include "flashdb.h"

U8 NKeyDef[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define TestMifare() do{; }while(0);

U8 *FillKeyData(U8 *Buffer, U8 *Key)
{
        int i;         
	
	memcpy(Buffer, Key, 6);
        Buffer[6]=0xff;Buffer[7]=0x07;Buffer[8]=0x80; Buffer[9]=0x69;
        //设定访问条件
        //C1C2C3_0123=0;C1C2_0123=0;C3_3=1
        //数据区访问条件为 C1C2C3=000 尾区访问条件为C1C2C3=001
        for(i=10;i<16;i++) Buffer[i]=0xff;
        return Buffer;
}

int Mifare_SetSectorKey(U8 sector, U8 * oldkey, U8 *newkey)
{
	int status = 0;
	U8 buf[16];
	U32 uid=0;
	U8 curkey[6];
	U8 sn[4];

	U8 CtlBlock = 0;
	if(sector > (BYTE)39)
	{
		return BAD_PARAM;
	}
	if(sector < (BYTE)32)
	{
		CtlBlock = (U8)((sector << 2) + 3);
	}
	else
	{
		CtlBlock = (U8)(127 + (sector - 31) * 16);
	}

	FillKeyData(buf, newkey);
	memcpy(curkey, oldkey, 6);
	//status = Mifare_Write((sector << 2)+3, 1, curkey, AUTH_KEYA, IDLE, buf, &uid, 0);
	status = Mifare_Write(CtlBlock, 1, curkey, AUTH_KEYA, IDLE, buf, &uid, 0);
	//AUTH FAILED
	if((status==AUTH_FAILED)&&(Mifare_Get_SNR(REQA_IDLE, 0, sn)==COMM_OK))
	{
		//try with default key
		if(memcmp(NKeyDef, oldkey, 6)!=0)
			memcpy(curkey, NKeyDef, 6);
		else
			memcpy(curkey, (U8 *)gOptions.CardKey, 6);
		if(memcmp(curkey, oldkey, 6)!=0)
		{
			//status = Mifare_Write((sector << 2)+3, 1, curkey, AUTH_KEYA, IDLE, buf, &uid, 0);
			status = Mifare_Write(CtlBlock, 1, curkey, AUTH_KEYA, IDLE, buf, &uid, 0);
		}
	}			
	if(COMM_OK == status)
		DBPRINTF("Set new keyA for sector %d Success!\n",CtlBlock);
	else 
		DBPRINTF("Set new keyAmifare for sector %d Failed!\n", CtlBlock);
	return  status;
}

BOOL Mifare_SetCardKey(U8 secaddr, U8 sectors, U8 * oldkey, U8 *newkey)
{
   	U8 sr = 0;

	for(sr = secaddr; sr < (secaddr + sectors); sr++)
		if(COMM_OK != Mifare_SetSectorKey(sr, oldkey, newkey)) 	break;
	
	if (sr == (secaddr + sectors))
		DBPRINTF("set new keyA for card ok\n");
	else
		DBPRINTF("set new key to card Failed!\n");
	return (sr == (secaddr + sectors));
}

int Mifare_SetOneCardKey(U8 * oldkey, U8 *newkey)
{
	return Mifare_SetCardKey(0, 16, oldkey, newkey);
}

int Read_data_card(U8 addr, U8 blocks, U8 *blockdata, U16 len, U8 * key)
{
	int status = 0;
	U8 br = 1, nblocks, curblocks; 
	U8 * p;
	U32 uid;
	U8 curkey[6];
	U8 sn[4];
	U8 sector=1;

	//解决两张MFCard叠加在一起的时候，任何一个卡的指纹都可以验证通过的Bug.
	U32 readFlag = 0;
	U32 cardID = 0;
	//----------- end ----------
	if(len > blocks*16) len = blocks*16;
	nblocks = len >> 4;
	if(len%16) nblocks++;
	p = blockdata;
	br = addr;
	memcpy(curkey, key, 6);

	for(;nblocks > 0;)
	{
		//curblocks = 3-br%4;
		Mifare_GetSecInfo(br,&sector,&curblocks,1);//dsl 2012.1.17
		if(nblocks <= curblocks)
		{
			curblocks = nblocks; 
		}
		
		status = Mifare_Read(br, curblocks, curkey, AUTH_KEYA, IDLE, p, &uid);
		
		//解决两张MFCard叠加在一起的时候，任何一个卡的指纹都可以验证通过的Bug.zsliu 2009-06-09
		if(readFlag == 0)
		{
			readFlag = 1;
			cardID = uid;
		}

		if(cardID != uid)
		{
			readFlag = 0;
			status=PASS_FAILED;
			printf("\nError: use more than a MF card, the state cardID=%u, the current uid=%u \n\n", cardID, uid);
		}
		//-------------- add end .......
		if(status==PASS_FAILED)
		{
			//Card found and AUTH FAILED
			if(Mifare_Get_SNR(REQA_IDLE, 0, sn)==COMM_OK)
			{
				DBPRINTF("CARD FOUND!....\n");
				//try to write with default key
				if(memcmp(NKeyDef, key, 6)!=0)
				{
					status = Mifare_Read(br, curblocks, NKeyDef, AUTH_KEYA, IDLE, p, &uid);
					if(COMM_OK == status)
					{
						memcpy(curkey, NKeyDef, 6);
					}
				}
			}
		}		
		if(status != COMM_OK) break;	
		
		p += curblocks*16;
		nblocks -= curblocks;
		br += curblocks + 1;
	}
	
	
	if( COMM_OK == status)
	{
		DBPRINTF("R/W blockdata(%d bytes) from/to Card OK!\n",len);
	}
	else
	{
		DBPRINTF("R/W blockdata from/to Card Failed!\n");
		Mifare_Perror(status);
	}	
	return (COMM_OK == status)?len :-1;
}

int Write_data_card(U8 addr, U8 blocks, U8 *blockdata, U16 len, U8 * key, U8 *newkey)
{
	int status = 0;
	U8 br = 1, nblocks, curblocks; 
	U8 *p;
	U32 uid;
	U8 curkey[6];
	//U8 sn[4];
	U8 sector=1;

	if(len > blocks*16) len = blocks*16;

	nblocks = len >> 4;
	if(len%16) nblocks++;
	p = blockdata;
	br = addr;
	memcpy(curkey, key, 6);

	printf("%s, %s, template size=%d, need %d block\n", __FILE__,__func__, len, nblocks);
	printf("curkey = %x %x %x %x %x %x\n", curkey[0], curkey[1], curkey[2], curkey[3], curkey[4], curkey[5]);

	for(;nblocks > 0;)
	{
		//curblocks = 3-br%4;
		Mifare_GetSecInfo(br,&sector,&curblocks,1);//dsl 2012.1.17
		if(nblocks <= curblocks)
		{
			curblocks = nblocks; 
		}
		printf("curblocks=%d\n", curblocks);	
		status = Mifare_Write(br, curblocks, curkey, AUTH_KEYA, IDLE, p, &uid, 1);
		//if(status==AUTH_FAILED)
		if(status != COMM_OK)   //解决请空卡但是返回错误信息的问题   zsliu 2009-06-19
		{
			//Card found and AUTH FAILED
			//if(Mifare_Get_SNR(REQA_IDLE, 0, sn)==COMM_OK) //解决请空卡但是返回错误信息的问题 zsliu 2009-06-19
			{
				DBPRINTF("CARD FOUND!....\n");
				//try to write with default key
				if(memcmp(NKeyDef, curkey, 6)!=0)
				{
					status = Mifare_Write(br, curblocks, NKeyDef, AUTH_KEYA, IDLE, p, &uid, 1);
					if(COMM_OK == status)
						memcpy(curkey, NKeyDef, 6);
				}	
				else if(memcmp(key, curkey, 6)!=0) //try to write with parameter key
				{
					status = Mifare_Write(br, curblocks, key, AUTH_KEYA, IDLE, p, &uid, 1);
					if(COMM_OK == status)
						memcpy(curkey, key, 6);					
				}
				else if(memcmp(newkey, curkey, 6)!=0)
				{
					status = Mifare_Write(br, curblocks, newkey, AUTH_KEYA, IDLE, p, &uid, 1);
					if(COMM_OK == status)
						memcpy(curkey, newkey, 6);										
				}
				if(COMM_OK == status)
					DBPRINTF("Sector password retry OK!\n");
			}
		}			
		if(status != COMM_OK) break;	
		//write key blocks
		//DBPRINTF("curkey = %x %x %x %x %x %x\n", curkey[0], curkey[1], curkey[2], curkey[3], curkey[4], curkey[5]);
		if(memcmp(curkey, newkey, 6)!= 0)
		{
			DBPRINTF("writing password....\n");
			//status = Mifare_SetSectorKey(br>>2, curkey, newkey);
			status = Mifare_SetSectorKey((br<128 ? br>>2 : 32 + ((br >> 4) - 8)), curkey, newkey);  //support S70
			if(status != COMM_OK) break;	
		}
		
		
		p += curblocks*16;
		nblocks -= curblocks;
		br += curblocks + 1;
	}
	if( COMM_OK == status)
	{
		DBPRINTF("R/W blockdata(%d bytes) from/to Card OK!\n",len);
	}
	else
	{
		DBPRINTF("R/W blockdata from/to Card Failed!\n");
		Mifare_Perror(status);
	}	
	return (COMM_OK == status)?len :-1;
}

//计算保留扇区的大小
int MFGetResBlocks(void)
{
        int i,si,ret=0;
        for(i=0; i<gOptions.RFCardSecLen; i++)
        {
                si=i+gOptions.RFCardSecStart;
                if(si==0) ret+=2;
                else if(si<32) ret+=3;
                else ret+=15;
        }
        return ret;
}

int MFGetResSize(void)
{
	return MFGetResBlocks()*16;
}

//计算开始块编号
int MFGetStartBlock(void)
{
        int i,ret=0;
        for(i=0;i<gOptions.RFCardSecStart;i++)
        {
                if(i<32) ret+=4; else ret+=16;
        }
        if(ret==0) ret=1; //Block0 为厂商信息
        return ret;
}

//计算结束块编号
int MFGetEndBlock(void)
{
        int i,ret=-1;
        for(i=0;i<gOptions.RFCardSecStart+gOptions.RFCardSecLen;i++)
        {
                if(i<32) ret+=4; else ret+=16;
        }
        return ret;
}

//计算块是否区尾密码块
int MFIsTailBlock(int BlockIndex)
{
        if(BlockIndex<128)
        {
                return (BlockIndex % 4)==3;
        }
        else
	{
                return (BlockIndex % 16)==15;
	}
}

//计算块所属的扇区
int MFGetBlockSec(int BlockIndex)
{
        if(BlockIndex<128)
        {
                return BlockIndex/4;
        }
        else
        {
                return 24+(BlockIndex/16);
        }
}

/* ************************************************************************
 * MFInit:	Only write to then read back the serial number  of Mifare reader 
 *		to check whether it's there or NOT.
 *
 * param:	void
 *
 * return:	TRUE. Find a Card Reader Connecting to Current System
 * 		FALSE. No Card Reader Connecting
 **************************************************************************/
//int MFInit(void)
int MFInit(serial_driver_t *serial)
{
	//U8 ser[] = {'#', 'Z', 'K', 'S', 'O', 'F', 'T', '#'};
#if 1
	static U8 buf[8];
	BYTE *p;
	int next;
	int i;
	
	serial->flush_input();
	serial->flush_output();
	mifare_raw_read = serial->read;
	mifare_raw_write = serial->write;
	mifare_raw_poll = serial->poll;
	mifare_raw_flush_output = serial->flush_output;
	mifare_raw_close = serial->free;
	
	p=(BYTE *)LoadStrOld("MFDefKey");
	if(p)
	{
		for(i=0;i<6;i++)
		{
			NKeyDef[i]=SHexValueFrom((char *)p, i, &next);
			DBPRINTF("KEY[%d]=%d\n", i, NKeyDef[i]);
		}
	}
	if(COMM_OK!=Mifare_GetSerNum(buf))
	{
		DelayMS(200);
		serial->flush_input();
		serial->flush_output();
		//try again
		if(COMM_OK!=Mifare_GetSerNum(buf)) return FALSE;
	}
        return TRUE;
#else
	static U8 buf[8];
	BYTE *p;
	int next;
	int i;
	
	st232.flush_input();
	st232.flush_output();
	mifare_raw_read = st232.read;
	mifare_raw_write = st232.write;
	mifare_raw_poll = st232.poll;
	mifare_raw_flush_output = st232.flush_output;
	mifare_raw_close = st232.free;
	
	p=(BYTE *)LoadStrOld("MFDefKey");
	if(p)
	{
		for(i=0;i<6;i++)
		{
			NKeyDef[i]=SHexValueFrom((char *)p, i, &next);
			DBPRINTF("KEY[%d]=%d\n", i, NKeyDef[i]);
		}
	}
	if(COMM_OK!=Mifare_GetSerNum(buf))
	{
		DelayMS(200);
		st232.flush_input();
		st232.flush_output();
		//try again
		if(COMM_OK!=Mifare_GetSerNum(buf)) return FALSE;
	}
	return TRUE;
#endif
}

int MFCheckCard(U8 *sn)
{
	//U32 CardNumber = 0; 
        //if(COMM_OK != Mifare_REQA(REQA_IDLE)) return FALSE;
        //if(COMM_OK != Mifare_AnticollA(&CardNumber)) return FALSE;
        //if(COMM_OK != Mifare_SelectA(CardNumber)) return FALSE;
        //return TRUE;
	return (Mifare_Get_SNR(REQA_IDLE, 0, sn) == COMM_OK);	
}

int MFRead(PFPCardOP fpdata, int OnlyPINCard)
{	
	int len=0;
	U8 databuf[4096]={0};//[752];
	int Block=0, size;
	
	Block=MFGetStartBlock();
	//标志字(2B)+用户ID(2B)+指纹模板数据大小(2B)+指纹1／2编号[1B]+指纹3／4编号[1B]+指纹模板
	if(Read_data_card(Block, 1, databuf, 16, (U8 *)gOptions.CardKey)==-1)	
		return MFCARD_ERROR_READ;
	if((databuf[0]!=('Z'+16))||(databuf[1]!=1)) 
		return MFCARD_ERROR_UNKNOWN;	
	Block++;

	if(gOptions.PIN2Width==PIN_WIDTH2) //ID 2bytes
	{
		if(OnlyPINCard)
			size=0;
		else
			size=get_unaligned(((U16*)databuf)+2);
		fpdata->PIN=get_unaligned(((U16*)databuf)+1);
		fpdata->Finger[0]=databuf[6] >> 4;
		fpdata->Finger[1]=databuf[6] & 0x0f;
		fpdata->Finger[2]=databuf[7] >> 4;
		fpdata->Finger[3]=databuf[7] & 0x0f;
		if(size>0)
			memcpy(fpdata->Templates, databuf+8, 8);
		if(!OnlyPINCard && size>0)
			len=8;
		else
			len=0;
	}
	else //ID 4bytes
	{
		if(OnlyPINCard)
			size=0;
		else
			size=get_unaligned(((U16*)databuf)+3);

		memcpy(&fpdata->PIN, databuf+2, 4);
		fpdata->Finger[0]=databuf[8] >> 4;
		fpdata->Finger[1]=databuf[8] & 0x0f;
		fpdata->Finger[2]=databuf[9] >> 4;
		fpdata->Finger[3]=databuf[9] & 0x0f;

		if(size>0)
			memcpy(fpdata->Templates, databuf+10, 6);

		if(!OnlyPINCard && size>0)
			len=6;
		else
			len=0;		
	}	
	fpdata->TempSize=size;

        if((size==0) && (0==fpdata->PIN)) return MFCARD_ERROR_EMPTY;
	if(len>0)
	{
		if(Read_data_card(Block, MFGetResBlocks()-1, fpdata->Templates+len, size-len, (U8 *)gOptions.CardKey)==-1)
			return MFCARD_ERROR_READ;
		memset(fpdata->Templates+size, 0, 16);
		len=size;
	}	
	if(len==size) return size;
        return MFCARD_ERROR_DATA;
}

int MFWrite(PFPCardOP fpdata)
{	
	int len = 0;
	U8 buf[4096];//[752];
	
	if((OP_WRITE == fpdata->OP)&&(0 == fpdata->PIN)) return MFCARD_ERROR_DATA;
	memset(buf, 0xFF, sizeof(buf));
	buf[0] ='Z' + 16; buf[1] = 1;
	if(gOptions.PIN2Width==PIN_WIDTH2) //ID 2bytes
	{
		memcpy(buf + 2, &fpdata->PIN, 2);
		memcpy(buf + 4, &fpdata->TempSize, 2);
		len = 6;
	}
	else //ID 4bytes
	{
		memcpy(buf + 2, &fpdata->PIN, 4);
		memcpy(buf + 6, &fpdata->TempSize, 2);
		len = 8;		
	}
	if(fpdata->TempSize != 0)
	{
		buf[len++] = ((fpdata->Finger[0] << 4) | (fpdata->Finger[1] & 0x0F));
		buf[len++] = ( (fpdata->Finger[2] << 4) | (fpdata->Finger[3] & 0x0F));
		memcpy(buf + len, fpdata->Templates, fpdata->TempSize);
		len += fpdata->TempSize;
	}	
	if(Write_data_card(MFGetStartBlock(), MFGetResBlocks(), buf, len, NKeyDef, (U8 *)gOptions.CardKey)==-1)
		return MFCARD_ERROR_WRITE;
	else
		return len;
}

int MFEmpty()
{
	U8 buf[4096];//[1024];
	//clear card data
	memset(buf, 0xFF, MFGetResBlocks()*16);
	return Write_data_card(MFGetStartBlock(), MFGetResBlocks(), buf, MFGetResBlocks()*16, (U8 *)gOptions.CardKey, NKeyDef);
}

int MFFinishCard(void)
{
	return Mifare_HaltA();
	
}
