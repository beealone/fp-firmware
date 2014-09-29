#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <asm/unaligned.h>
#include "serial.h"
#include "arca.h"
#include "options.h"
#include "utils.h"
#include "flashdb.h"
#include "hid_iclass.h"
#include "iclsrw.h"

#define ICLSRWSHOWTIME

extern int giCLSCardType;

enum _CONFIGURATIONBLOCK_{

	APPLIMIT=0,
	APPOTPAREA,
	BLOCKWRITELOCK=3,
	CHIPCONFIG,
	MEMORYCONFIG,
	EAS,
	FUSES,
	CONFIGBLOCKSIZE=8
	
};

//
// iCLSGetCardType: Read card configurationg block to confirm card type.
// 
// return: CardType return , if failure return negativ.
//
int iCLSGetCardType(void)
{	
	U8 cfg[CONFIGBLOCKSIZE] ={0,};
	U8 bitMasks =0;
	
	
	if(!iCLASS_GetCardCFG(cfg))
	{
		//DBPRINTF("Get card type fail !\n");
		return -1;
	}

	bitMasks =  ( (cfg[CHIPCONFIG] & 0x10) ? 0x04:0 )
			+ ( (cfg[MEMORYCONFIG] & 0x80) ? 0x02:0)
			+ ( (cfg[MEMORYCONFIG] & 0x20) ? 0x01:0);

	//DBPRINTF("iCLSGetCardType: %d\n", bitMasks);
	return bitMasks;
}

//
// iCLSGetResBlocks: Get the all blocks memory for user. 
//
// cardType: The card with different memory.
// 
// return: Return all blocks memory for user .
//
int iCLSGetResBlocks(int cardType)
{
	U8 resBlocks =0;

	if(cardType ==CT_2K)
	{
		resBlocks =AREA2_ENDBLOCK_2K - AREA2_STARTBLOCK_2K;
	}
	else if(cardType ==CT_16K_2)
	{
		resBlocks =AREA2_ENDBLOCK_16K_2 - AREA2_STARTBLOCK_16K_2;
	}
	else if(cardType == CT_16K_16)
	{
		resBlocks =(AREA2_ENDBLOCK_16K_16-AREA2_STARTBLOCK_16K_16)+7*(AREA2_ENDBLOCK_16K_16 -AREA1_STARTBLOCK_16K_16);
	}
	else if(cardType == CT_32K_BOOK_16K_2)
	{}
	else if(cardType == CT_32K_BOOK_16K_16)
	{}

	//DBPRINTF("iCLSGetResBlocks:%d\n", resBlocks);
	return resBlocks;
}

//
// iCLSGetResSize: Get reserved bytes for user.
//
// return: Return reserved size.
//
int iCLSGetResSize(void)
{
	//int ret;
	//ret =iCLSGetResBlocks(CT_16K_16)*BLOCKSIZE;
	//DBPRINTF("iCLSGetResSize: %d CardType:%d\n",ret, CT_16K_16);
	//return ret;
		
	return iCLSGetResBlocks(giCLSCardType)*BLOCKSIZE;
}

//
// KeyRemap: load default protected key for areas of pages to location 0x01-0x07 in reader
//
// return: TRUE success.
//		 FALSE fail.
//
int KeyRemap(void)
{
	U8 i;
	
	for(i =1; i<=7; i++)
	{
		if(!LoadKeyIntoReader(DefaultSecretKey[i], i))
			return FALSE;		
	}

	return TRUE;
}

//
// iCLSInit: Open serial port for iCLASS reader and check whether it is a iCLASS machine.
//
// return: TRUE success. FALSE failure.
//
int iCLSInit(serial_driver_t*  serial)
{
	DBPRINTF("START... iCLASS\n");

	serial->flush_input();
	serial->flush_output();
	iCLASS_raw_write= serial->write;
	iCLASS_raw_read = serial->read;
	iCLASS_raw_poll	= serial->poll;
	iCLASS_raw_flush_output = serial->flush_output;
	iCLASS_raw_close = serial->free;
	iCLASS_raw_flush_input = serial->flush_input;
	
/*	
	if(!iCLSCheckReader() )
	{
		DBPRINTF("Check iCLASS reader fail!\n");
		return FALSE;
	}
	else
	{
		DBPRINTF("iCLASS reader check ok!\n");
	}
*/
	if(!KeyRemap())
	{
		DBPRINTF("iCLASS Key remap fail Init fail\n");
		return FALSE;
	}
	else
		DBPRINTF("Key remap OK!\n");

	DBPRINTF("Finish iCLASS Init!\n");

	return TRUE;
}

//
// iCLSWrite_16K16: Write blocks to  card with 16K bits and 16 application areas.
// 
// pageNum: Page number block address in it.
// curAddr: Block address to write.
// blocks: Blocks to write.
// writeData: Data to write.
// cardRemainBlocks: Current blocks remain in card.
//
// return: Blocks have wrote to card, otherwise negative.
//
int iCLSWrite_16K16(U8 pageNum, U8 curAddr, U8 blocks, U8 *writeData, U8* cardRemainBlocks)
{
//	U8 i;
	U8 selectCardArea;
	U8 areaRemainBlocks;
	U8 writeRemainBlocks;
	U8 writeBlocks;
	U8 rdKeyLoc=0;
	U8 offset=0;
	U8 keyNum=0;

#ifdef ICLSRWSHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif

	if(blocks > *cardRemainBlocks || *cardRemainBlocks ==0)
	{
		DBPRINTF("Page reserved blocks is not enough!\n");
		return -1;
	}
	else
	{	
		if(curAddr >=6 && curAddr <=18)
		{
			if(pageNum ==0)
			{
				DBPRINTF("Block address can not writable!\n");
				return -1;
			}
			selectCardArea = SC_AREA1;
			areaRemainBlocks =AREA1_ENDBLOCK_16K_16 - curAddr;
		}
		else if(curAddr >=19 && curAddr <=31)
		{
			selectCardArea = SC_AREA2;
			areaRemainBlocks = AREA2_ENDBLOCK_16K_16 - curAddr;
		}
		else
			return -1;

		writeRemainBlocks =blocks;

		do
		{
			//loadkey
			if(selectCardArea ==SC_AREA1)
				rdKeyLoc =keyNum = pageNum*2;
			else if(selectCardArea == SC_AREA2)
				rdKeyLoc =keyNum = pageNum*2+1;

			if(rdKeyLoc >7)
			{
					
				rdKeyLoc = rdKeyLoc%7 + (rdKeyLoc-7)/7;
				if(!LoadKeyIntoReader(DefaultSecretKey[keyNum], rdKeyLoc))
					return -1;
			}
		
			writeBlocks =(writeRemainBlocks <= areaRemainBlocks) ? writeRemainBlocks:areaRemainBlocks;

			//DBPRINTF("\tiCLSWrite_16K16:pageNum:%d,curAddr:%2d,writeBlocks:%d,areaRemainBlocks:%d\n", pageNum,curAddr, writeBlocks,areaRemainBlocks);
			if(iCLASS_Write(pageNum, curAddr, writeBlocks, rdKeyLoc, selectCardArea, writeData+offset*BLOCKSIZE) ==-1)
				return -1;

			offset +=writeBlocks;
			curAddr +=writeBlocks;

			if( curAddr == AREA1_ENDBLOCK_16K_16 )
			{
				selectCardArea = SC_AREA2;
				areaRemainBlocks = AREA2_SIZE_16K_16;
			}
			else if( curAddr == AREA2_ENDBLOCK_16K_16 )
			{

				++pageNum;
				selectCardArea = SC_AREA1;
				curAddr = AREA1_STARTBLOCK_16K_16;
				areaRemainBlocks = AREA1_SIZE_16K_16;
			}
			else
			{
				areaRemainBlocks -= writeBlocks;
			}
			
			writeRemainBlocks -=writeBlocks;

			if(pageNum >7)
			{

#ifdef ICLSRWSHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("##iCLSWrite_16K16(0_0)write %d bytes use time:%f ms\n",(blocks-writeRemainBlocks)*BLOCKSIZE,msec);
#endif
				*cardRemainBlocks =0;
				
				return (blocks-writeRemainBlocks);
			}
			
		}while(writeRemainBlocks);

		*cardRemainBlocks -=blocks;
	}

#ifdef ICLSRWSHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("##iCLSWrite_16K16(0_0)write %d bytes use time:%f ms\n",(blocks-writeRemainBlocks)*BLOCKSIZE,msec);
#endif

	return (blocks-writeRemainBlocks);
}


//
// iCLSRead_16K16: Read blocks from  card with 16K bits and 16 application areas.
//
// pageNum: Page number block address in it
// curAddr: Block address to read.
// blocks: Blocks to read.
// writeData: Data to read.
//
// return: Blocks have readed from card, otherwise negative.
//
int iCLSRead_16K16(U8 pageNum, U8 curAddr, U8 blocks, U8 *blockData)
{
	U8 areaRemainBlocks;
	U8 selectCardArea;
	U8 readRemainBlocks;
	U8 rdKeyLoc=0 ;
	U8 readBlocks;
	U8 offset =0;
	U8 keyNum=0;

#ifdef ICLSRWSHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif

	if(curAddr >=6 && curAddr <=18)
	{
		if(pageNum ==0)
		{
			DBPRINTF("Block address can not readable!\n");
			return -1;
		}
		selectCardArea = SC_AREA1;
		areaRemainBlocks =AREA1_ENDBLOCK_16K_16 - curAddr;
	}
	else if(curAddr >=19 && curAddr <=31)
	{
		selectCardArea = SC_AREA2;
		areaRemainBlocks = AREA2_ENDBLOCK_16K_16 - curAddr;
	}
	else
		return -1;

	readRemainBlocks =blocks;

	do
	{
		//loadkey
		if(selectCardArea ==SC_AREA1)
			rdKeyLoc =keyNum = pageNum*2;
		else if(selectCardArea == SC_AREA2)
			rdKeyLoc =keyNum = pageNum*2+1;

		if(rdKeyLoc >7)
		{
				
			rdKeyLoc = rdKeyLoc%7 + (rdKeyLoc-7)/7;
			if(!LoadKeyIntoReader(DefaultSecretKey[keyNum], rdKeyLoc))
				return -1;
		}

		//DBPRINTF("\tiCLSRead_16K16:pagenum:%d, curAddr:%2d, rdKeyLoc:%d, cardArea:%s\n", pageNum,curAddr,rdKeyLoc,AREA[selectCardArea/0x10]);
		readBlocks =(readRemainBlocks <= areaRemainBlocks) ? readRemainBlocks:areaRemainBlocks;

		if(iCLASS_Read(pageNum, curAddr, readBlocks, rdKeyLoc, selectCardArea, blockData+offset*BLOCKSIZE) ==-1)
			return -1;

		offset +=readBlocks;
		curAddr +=readBlocks;

		if( curAddr == AREA1_ENDBLOCK_16K_16 )
		{
			selectCardArea = SC_AREA2;
			areaRemainBlocks = AREA2_SIZE_16K_16;
		}
		else if( curAddr == AREA2_ENDBLOCK_16K_16 )
		{
			++pageNum;
			selectCardArea = SC_AREA1;
			curAddr = AREA1_STARTBLOCK_16K_16;
			areaRemainBlocks = AREA1_SIZE_16K_16;
		}
		else
		{
			areaRemainBlocks -= readBlocks;
		}
		
		readRemainBlocks -=readBlocks;	

		if(pageNum >7)
		{

#ifdef ICLSRWSHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("##iCLSRead_16K16(0_0)Read %d bytes use time:%f ms\n",(blocks-readRemainBlocks)*BLOCKSIZE,msec);
#endif
			return (blocks-readRemainBlocks);
		}
		
	}while(readRemainBlocks);

#ifdef ICLSRWSHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("##iCLSRead_16K16(0_0)Read %d bytes use time:%f ms\n",(blocks-readRemainBlocks)*BLOCKSIZE,msec);
#endif

	return (blocks-readRemainBlocks);
}

//
// iCLSWrite_16K2: Write blocks to  card with 16K bits,2 application areas.
// 
// curAddr: Block address to write.
// blocks: Blocks to write.
// writeData: Data to write.
// cardRemainBlocks: Current blocks remain in card.
//
// return: Blocks have wrote to card, otherwise negative.
//
int iCLSWrite_16K2(U8 curAddr, U8 blocks, U8 *writeData, U8* cardRemainBlocks)
{
	U8 selectCardArea=SC_AREA2;
	U8 rdKeyLoc =RKEY1;
	U8 pageNum=0; 
	int nCount=0;

#ifdef ICLSRWSHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif

	if(blocks > *cardRemainBlocks || *cardRemainBlocks ==0)
	{
		DBPRINTF("Page remain blocks is not enough!\n");
		return -1;
	}
	else
	{	
		if(curAddr < 19)
		{
			DBPRINTF("Block address is not writable!\n");
			return -1;
		}

		//DBPRINTF("\tiCLSWrite_16K2:pageNum:%d,curAddr:%d,writeBlocks:%d\n", pageNum,curAddr, blocks);
		if((nCount =iCLASS_Write(pageNum, curAddr, blocks, rdKeyLoc, selectCardArea, writeData)) ==-1)
			return -1;

		*cardRemainBlocks -=nCount;
	}

#ifdef ICLSRWSHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("##iCLSWrite_16K2(0_0)Write %d bytes use time:%f ms\n",nCount*BLOCKSIZE,msec);
#endif

	return (nCount);
}

//
// iCLSRead_16K2: Read blocks from  card with 16K bits and 2 application areas.
// 
// curAddr: Block address to read.
// blocks: Blocks to read.
// blockData: Data to read.
//
// return: Blocks have readed from card, otherwise negative.
//
int iCLSRead_16K2(U8 curAddr, U8 blocks, U8 *blockData)
{
	U8 selectCardArea=SC_AREA2;
	U8 rdKeyLoc =RKEY1;
	U8 pageNum=0;
	int nCount=0;

#ifdef ICLSRWSHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif

	if(curAddr <19)
	{
		DBPRINTF("Block address is not writable!\n");
		return -1;
	}

	//DBPRINTF("\tiCLSRead_16K2:pageNum:%d,curAddr:%d,cardArea:%s\n", pageNum,curAddr, AREA[selectCardArea/0x10]);
	if( (nCount =iCLASS_Read(pageNum, curAddr, blocks, rdKeyLoc, selectCardArea, blockData))  ==-1)
		return -1;

#ifdef ICLSRWSHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("##iCLSRead_16K2(0_0)Read %d bytes use time:%f ms\n",nCount*BLOCKSIZE,msec);
#endif

	return (nCount);
}

//
// iCLSWrite_2: Write blocks to  card with 2K bits and 2 application areas.
// 
// curAddr: Block address to write.
// blocks: Blocks to write.
// writeData: Data to write.
// cardRemainBlocks: Current blocks remain in card.
//
// return: Blocks have wrote to card, otherwise negative.
//
int iCLSWrite_2K(U8 curAddr, U8 blocks, U8 *writeData, U8* cardRemainBlocks)
{
	U8 selectCardArea=SC_AREA2;
	U8 rdKeyLoc =RKEY1;
	U8 pageNum=0; 
	int nCount=0;

#ifdef ICLSRWSHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif

	if(blocks > *cardRemainBlocks || *cardRemainBlocks ==0)
	{
		DBPRINTF("Page remain blocks is not enough!\n");
		return -1;
	}
	else
	{	
		if(curAddr <19 || curAddr >32)
		{
			DBPRINTF("Block address is not writable!\n");
			return -1;
		}

		//DBPRINTF("\tiCLSWrite_2K:pageNum:%d,curAddr:%d,writeBlocks:%d\n", pageNum,curAddr, blocks);
		if((nCount =iCLASS_Write(pageNum, curAddr, blocks, rdKeyLoc, selectCardArea, writeData)) ==-1)
			return -1;

		*cardRemainBlocks -=nCount;
	}

#ifdef ICLSRWSHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("##iCLSWrite_2K(0_0)Write %d bytes use time:%f ms\n",nCount*BLOCKSIZE,msec);
#endif

	return (nCount);
}


//
// iCLSRead_2K: Read blocks from  card with 2K bits and 2 application areas.
// 
// curAddr: Block address to read.
// blocks: Blocks to read.
// writeData: Data to read.
//
// return: Blocks have readed from card, otherwise negative.
//
int iCLSRead_2K(U8 curAddr, U8 blocks, U8 *blockData)
{
	U8 selectCardArea=SC_AREA2;
	U8 rdKeyLoc =RKEY1;
	U8 pageNum=0;
	int nCount=0;

#ifdef ICLSRWSHOWTIME
	struct timeval tv_start, tv_end;	
	struct timezone tz;
	unsigned long susec, eusec;
	double msec;
	gettimeofday(&tv_start, &tz);
      	susec = tv_start.tv_sec*1000*1000 + tv_start.tv_usec;
#endif

	if(curAddr <19 || curAddr >32)
	{
		DBPRINTF("Block address is not writable!\n");
		return -1;
	}

	//DBPRINTF("\tiCLSRead_2K:pageNum:%d,curAddr:%d, cardArea:%s\n", pageNum,curAddr, AREA[selectCardArea/0x10]);
	if( (nCount =iCLASS_Read(pageNum, curAddr, blocks, rdKeyLoc, selectCardArea, blockData))  ==-1)
		return -1;

#ifdef ICLSRWSHOWTIME
	gettimeofday(&tv_end, &tz);
      	eusec = (tv_end.tv_sec *1000*1000 + tv_end.tv_usec);
      	msec = (eusec -susec)/1000.0 ;
      	DBPRINTF("##iCLSRead_2K(0_0)Read %d bytes use time:%f ms\n",nCount*BLOCKSIZE,msec);
#endif

	return (nCount);
}

//
// iCLSWriteCard: Write blocks to  all iCLASS series cards.
// 
// cardType: Card type with memory configuration.
// offsetAddr: Offset address to start block.
// blocks: Blocks to write.
// writeData: Data to write.
// len: The length of write data.
//
// return: Blocks have wrote to card, otherwise negative.
//
int iCLSWriteCard(int cardType, U8 offsetAddr, U8 blocks ,U8* writeData, U16 len)
{
	int nCount;
	U8 cardRemainBlocks=0;
	U8 startAddr=0;

	//DBPRINTF("iCLSWriteCard:\n");
	if(len > blocks*BLOCKSIZE) len =blocks*BLOCKSIZE;
	if(!KeyRemap())
	{
		DBPRINTF("KeyRemap after write 16k/16 fail!\n");
		return -1;
	}               

	if(cardType ==-1)
	{
		DBPRINTF("Card type error!\n");
		return -1;
	}

	if( CT_2K == cardType )
	{
		startAddr =AREA2_STARTBLOCK_2K + offsetAddr;

		blocks =(len+BLOCKSIZE-1)/BLOCKSIZE;

		cardRemainBlocks =AREA2_ENDBLOCK_2K - startAddr;

		if( (nCount =iCLSWrite_2K( startAddr, blocks,  writeData, &cardRemainBlocks))  ==-1)
		{
			DBPRINTF("Write data to 2K card fail!\n");
			return -1;
		}
	}
	else if( CT_16K_2 ==cardType )
	{
		startAddr =AREA2_STARTBLOCK_16K_2 + offsetAddr;

		blocks =(len+BLOCKSIZE-1)/BLOCKSIZE;

		cardRemainBlocks =AREA2_ENDBLOCK_16K_2 - startAddr;

		if( (nCount =iCLSWrite_16K2( startAddr, blocks, writeData, &cardRemainBlocks)) ==-1)
		{
			DBPRINTF("Write data to 16K/2 card fail!\n");
			return -1;
		}
	}
	else if( CT_16K_16 ==cardType )
	{
		U8 pageNum =0;
		
		if(offsetAddr > AREA2_SIZE_16K_16-1)
		{
			pageNum++;
			offsetAddr -=AREA2_SIZE_16K_16;

			pageNum +=offsetAddr/(AREA1_SIZE_16K_16+AREA2_SIZE_16K_16);
			startAddr = offsetAddr%(AREA1_SIZE_16K_16+AREA2_SIZE_16K_16)+AREA1_STARTBLOCK_16K_16;
			
		}
		else
		{
			startAddr =AREA2_STARTBLOCK_16K_16 +offsetAddr;
		}

		blocks =(len+BLOCKSIZE-1)/BLOCKSIZE;

		cardRemainBlocks =AREA2_ENDBLOCK_16K_16 - startAddr
						+(7-pageNum)*(AREA1_SIZE_16K_16+AREA2_SIZE_16K_16);

		//DBPRINTF("iCLSWriteCard: pageNum:%d, startAddr:%d, blocks:%d\n", pageNum, startAddr, blocks);
		if( (nCount =iCLSWrite_16K16( pageNum, startAddr, blocks, writeData, &cardRemainBlocks)) ==-1)
		{
			DBPRINTF("Write data to 16K/16 card fail!\n");
			return -1;
		}
	}
	else if( CT_32K_BOOK_16K_2 ==cardType )
	{
		DBPRINTF("Current not support!\n");
		return -1;
	}
	else if( CT_32K_BOOK_16K_16)
	{
		DBPRINTF("Current not support!\n");
		return -1;
	}
	return nCount;
}

//
// iCLSReadCard: Read blocks from all iCLASS series cards.
// 
// cardType: Card type with memory configuration.
// offsetAddr: Offset address to start block.
// blocks: Blocks to write.
// blockData: Data read from card.
// len: The length of read data.
//
// return: Blocks have readed to card, otherwise negative.
//
int iCLSReadCard(int cardType, U8 offsetAddr, U8 blocks ,U8* blockData, U16 len)
{
	int nCount;
	U8 startAddr=0;

	if(len > blocks*BLOCKSIZE) len =blocks*BLOCKSIZE;
	if(!KeyRemap())
	{
		DBPRINTF("KeyRemap after read 16k/16 fail!\n");
		return -1;
	}

	if(cardType ==-1)
	{
		DBPRINTF("Card type error!\n");
		return -1;
	}

	if( CT_2K == cardType )
	{
		startAddr =AREA2_STARTBLOCK_2K + offsetAddr;

		blocks =(len+BLOCKSIZE-1)/BLOCKSIZE;

		if( (nCount =iCLSRead_2K( startAddr, blocks,  blockData))  ==-1)
		{
			DBPRINTF("Read data from 2K card fail!\n");
			return -1;
		}
	}
	else if( CT_16K_2 ==cardType )
	{
		startAddr =AREA2_STARTBLOCK_16K_2 + offsetAddr;

		blocks =(len+BLOCKSIZE-1)/BLOCKSIZE;

		if( (nCount =iCLSRead_16K2( startAddr, blocks, blockData)) ==-1)
		{
			DBPRINTF("Read data from 16K/2 card fail!\n");
			return -1;
		}
	}
	else if( CT_16K_16 ==cardType )
	{
		U8 pageNum =0;
		
		if(offsetAddr > AREA2_SIZE_16K_16-1)
		{
			pageNum++;
			offsetAddr -=AREA2_SIZE_16K_16;

			pageNum +=offsetAddr/(AREA1_SIZE_16K_16+AREA2_SIZE_16K_16);
			startAddr = offsetAddr%(AREA1_SIZE_16K_16+AREA2_SIZE_16K_16)+AREA1_STARTBLOCK_16K_16;
			
		}
		else
		{
			startAddr =AREA2_STARTBLOCK_16K_16 +offsetAddr;
		}

		blocks =(len+BLOCKSIZE-1)/BLOCKSIZE;

		//DBPRINTF("##iCLSReadCard: pageNum:%d, startAddr:%d, blocks:%d\n", pageNum, startAddr, blocks);
		if( (nCount =iCLSRead_16K16( pageNum, startAddr, blocks, blockData)) ==-1)
		{
			DBPRINTF("Read data from 16K/16 card fail!\n");
			return -1;
		}
	}
	else if( CT_32K_BOOK_16K_2 ==cardType )
	{
		DBPRINTF("Current not support!\n");
		return -1;
	}
	else if( CT_32K_BOOK_16K_16)
	{
		DBPRINTF("Current not support!\n");
		return -1;
	}
	return nCount;
}

//
// iCLSWrite: Write fingerprint data to iCLASS card
//
// fpdata: Fingerprint structure.
//
// return: Bytes wrote to iCLASS card.
//
int iCLSWrite(PFPCardOP fpdata)
{
	int len = 0;
	U8 buf[752];

	//DBPRINTF("iCLSWrite:\n");
	if((iCLSOP_WRITE == fpdata->OP)&&(0 == fpdata->PIN)) return iCLSCARD_ERROR_DATA;
	memset(buf, 0xFF, 752);
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

	if(iCLSWriteCard(giCLSCardType, 0, iCLSGetResBlocks(giCLSCardType), buf, len)==-1)
		return iCLSCARD_ERROR_WRITE;
	else
		return len;	
}

//
// iCLSRead: Read fingerprint data or PIN from iCLASS card.
//
// fpdata: Fingerprint data structur.
// OnlyPINCard: The flag specific the iCLASS card only as PIN card.
//
// return: Bytes readed from iCLASS card. 
//
int iCLSRead(PFPCardOP fpdata, int OnlyPINCard)
{
	int len=0;
	U8 databuf[752];
	int Block=0, size;

	//DBPRINTF("iCLSRead:\n");
	
	//标志字(2B)+用户ID(2B)+指纹模板数据大小(2B)+指纹1／2编号[1B]+指纹3／4编号[1B]+指纹模板
	if(iCLSReadCard( giCLSCardType, 0, 2, databuf, 16)==-1)	
		return iCLSCARD_ERROR_READ;
	if((databuf[0]!=('Z'+16))||(databuf[1]!=1)) 
		return iCLSCARD_ERROR_UNKNOWN;	
	Block++;
	if(gOptions.PIN2Width==PIN_WIDTH) //ID 2bytes
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
        if((size==0) && (0==fpdata->PIN)) return iCLSCARD_ERROR_EMPTY;
	if(len>0)
	{
		if(iCLSReadCard( giCLSCardType, 2, iCLSGetResBlocks(giCLSCardType)-2, fpdata->Templates+len, size-len)==-1)
			return iCLSCARD_ERROR_READ;
		memset(fpdata->Templates+size, 0, 16);
		len=size;
	}	
	if(len==size) return size;
        return iCLSCARD_ERROR_DATA;
}

//
// iCLSEmpty: Clear data field, set all bytes 0xFF 
//
// return: If success return bytes wrote to iCLASS card. otherwise nagetive.
//
int iCLSEmpty(void)
{
	U8 buf[2048];
	U8 resBlocks =0;
	
	//clear card data
	resBlocks =iCLSGetResBlocks(giCLSCardType);
	
	memset(buf, 0xFF, resBlocks*BLOCKSIZE);	
	return iCLSWriteCard(giCLSCardType, 0, resBlocks, buf, resBlocks*BLOCKSIZE);
}

//
// iCLSCheckCard: Get card serial number.
//
// return: TRUE success. FALSE fail.
//
int iCLSCheckCard(U8 *sn)
{
	int ret;
	ret = iCLASS_GetSN(sn);
//	iCLASS_raw_flush_input();		
	return ret;
}	

