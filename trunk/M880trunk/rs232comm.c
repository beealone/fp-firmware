/*************************************************
                                           
 ZEM 200                                          
                                                    
 rs232comm.c                                
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "arca.h"
#include "serial.h"
#include "utils.h"
#include "commu.h"
#include "options.h"
#include "exfun.h"
#include "rs232comm.h"

#define CMD_RS 0x81
#define WORD 	unsigned short

#define THEBR gOptions.RS232BaudRate
#define THEDID gOptions.DeviceID

extern int fast485modeflag;

typedef struct _RSHeader_{
	unsigned char HeaderTag[2];
	unsigned char Cmd;
	unsigned char CmdCheck;
	WORD Len;
	WORD CheckSum;
}GCC_PACKED TRSHeader, *PRSHeader;

void RS_SendDataPack(serial_driver_t *rs, unsigned char *buf, unsigned short size)
{
	unsigned short cs;
	//int etime;
	TMyBuf *packet=bget(1);
	PRSHeader ch=(PRSHeader)(packet->buf);
	memset(packet->buf, 0, sizeof(TRSHeader));
	packet->len=sizeof(TRSHeader)+size;
	ch->HeaderTag[0]=THEDID; ch->HeaderTag[1]=THEDID;
	ch->Cmd=(char)CMD_RS; 
	ch->CmdCheck=0xFF-CMD_RS;
	ch->Len=size;
	memcpy(packet->buf+sizeof(TRSHeader), buf, size);
	ch->CheckSum=in_chksum(packet->buf, packet->len);
	if(gOptions.RS485On || (fast485modeflag == 1)) 
	{
		if(gOptions.RS485On) {
			RS485_setmode(TRUE);
		}
		cs=0;
		while(cs<packet->len)
		{ 
			rs->write(packet->buf[cs++]);

#ifdef ZEM300
			if(cs%256==0 && fast485modeflag == 0)
			{
				DelayUS(10*1000);
			} else {
				DelayUS(30);
			}
#endif
		}
		//i don't know how to do here
	//	DBPRINTF("Data SEND %d\n", cs);

#ifdef ZEM300
		rs->tcdrain();
		//DBPRINTF("tcdrain time:%d\n",GetTickCount()-etime);
		if(gOptions.RS485On) {
			while(rs->get_lsr()!=TIOCSER_TEMT);	//wait for the transmit shift register empty	
		}
		//DBPRINTF("get_lsr() time:%d\n",GetTickCount()-etime);
		if(fast485modeflag == 0) {
			DelayMS(115200/gOptions.RS232BaudRate);
		} else {
			DelayUS(10);
		}
#else
		DelayMS(1.5*cs*9600/gOptions.RS232BaudRate);
#endif
		if(gOptions.RS485On) {
			RS485_setmode(FALSE);
		}
	}
	else
	{
		/*speed very slow at USB 232,so change it
		for(cs=0;cs<packet->len;cs++) rs->write(packet->buf[cs]);
		*/
		int n=0;
		if(packet->len%64==0)
		{
			n=packet->len-20;
			rs->write_buf(packet->buf,n);
			rs->write_buf(packet->buf+n,20);
		}else
			rs->write_buf(packet->buf,packet->len);

		//DBPRINTF("Write data:%d\n",packet->len);
		//rs->write_buf(zeros,10);
	}
	packet->len=0;
}

int CloseHangUp(void *param)		
{
	DelayUS(1000*1000);
	SerialOutputString(*(void **)param, "+++");
	DelayUS(1000*1000);
	SerialOutputString(*(void **)param, "ATH0\r");
	return 0;
}

int SendRSData(void *buf, int size, void *param)
{
	RS_SendDataPack(*(void **)param, (unsigned char *)buf, (WORD)size);
	return 0;
}

WORD rsc=0, PS1[256], PS2[256], PSIndex=0;

int RS232Check(serial_driver_t *rs)
{
	int i, j,waitingChars, curChars,
				devIDOK=0,cmdHeadOK=0,dataSizeOK=0,cmdSizeOK=0;
	U32 ch,sum;//,dataCheck;
	TMyBuf *packet;
	PRSHeader pch;
	//int etime=GetTickCount();
	if((waitingChars=rs->poll())==0) return 0;
//	DBPRINTF("---Begin poll size:%d\n",waitingChars);
	i=0;sum=0;
	packet=bget(0);
	pch=(PRSHeader)(packet->buf);
	rsc++;
	while(1)
	{
		j=200;
		while((waitingChars=rs->poll())==0) 
		{
#ifdef FACE
			if(gOptions.FaceFunOn)
			{
				DelayUS(5);
			}
			else
#endif
				DelayUS(200);
#ifdef FACE
			if(((!gOptions.FaceFunOn)&&(++j>=2200)) || (gOptions.FaceFunOn&&(++j>2200*20))) 
#else
			if(++j>=2200) 
#endif
			{
				if(i>8)	//FIFO溢出，要求重发
				{
					PCmdHeader chdr=(PCmdHeader)(packet->buf+sizeof(TRSHeader));
					PCommSession session=GetSession(chdr->SessionID);
					if(session)
					{
						chdr->Command=CMD_ACK_RETRY;
						chdr->CheckSum=0;
						chdr->CheckSum=in_chksum((BYTE*)chdr,sizeof(TCmdHeader));
						session->Send((char *)chdr,sizeof(TCmdHeader),session->Sender);
						PS1[PSIndex]=i;
						PS2[PSIndex++]=rsc;
						PSIndex &= 0xFF;
						j=0;	//等待重发
						i=0;
						sum=0;
						devIDOK=0;cmdHeadOK=0;dataSizeOK=0;cmdSizeOK=0;
						DBPRINTF("FIFO overflow, SO SEND CMD_ACK_RETRY\n");
						continue;
					}
				}
//				ExBeep(5);
				DBPRINTF("wait data Time out\n");
				return -1;
			}
		}
		/*ch=rs->read();
		packet->buf[i]=ch;*/
		//DBPRINTF("poll size:%d\n",waitingChars);
		if(waitingChars+i>BUFSZ) break;
		curChars=rs->read_buf(packet->buf+i,waitingChars);
	//	DBPRINTF("read buf size:%d\n",curChars);
		i+=curChars;
		if(!devIDOK && i>=2)
		{
			if(gOptions.RS485On)
			{
				if((packet->buf[0]!=(U32)THEDID) || (packet->buf[1]!=(U32)THEDID))
				{
					DBPRINTF("DevID ERROR:\n");
					break;
				}
		//		DBPRINTF("DevID =%d:\n",packet->buf[0]);
				devIDOK=TRUE;
			}else
				devIDOK=TRUE;
		}
		//else if(i==3)
                if(!cmdHeadOK && i>=4)
		{
			if(pch->Cmd+pch->CmdCheck!=0xFF) 
			{
				//int jj;
				DBPRINTF("HEAD ERROR:\n");
				/*for(jj=0;jj<8;jj++)
					DBPRINTF("[%d]",packet->buf[jj]);
				DBRINTF("\n");*/
				break;
			}
			if(pch->Cmd!=CMD_RS) break;			
		//	DBPRINTF("cmdHeadOK\n");
			cmdHeadOK=TRUE;
		}
		if(!dataSizeOK && i>=sizeof(TRSHeader))
		{
			if(pch->Len>BUFSZ-sizeof(TRSHeader)) 
			{
//				DBPRINTF("DATA SIZE ERROR\n");
				break;
			}
//		DBPRINTF("dataSizeOK:%d\n",pch->Len);
			dataSizeOK=TRUE;
		}
		if(!cmdSizeOK && (i>=(int)(pch->Len+sizeof(TRSHeader))))
		{
			sum=pch->CheckSum;
			pch->CheckSum=0;
			if(sum==in_chksum(packet->buf, pch->Len+sizeof(TRSHeader)))
			{
				PCommSession session;
							
		//		DBPRINTF("CHECKSUM OK DataLength=%d,time=%d\n", pch->Len,GetTickCount()-etime);
				int cmd=RunCommand(packet->buf+sizeof(TRSHeader), pch->Len, TRUE);
			//	DBPRINTF("Processed!cmd %d\n",cmd);
				if(cmd==0) return -2;
				if(cmd==CMD_CONNECT)
				{
					extern int RTLogNum;
					char Sender[SENDER_LEN];
					PCmdHeader chdr=(PCmdHeader)(packet->buf+sizeof(TRSHeader));
					memset(Sender, 0, SENDER_LEN);
					memcpy(Sender, (void*)&rs, sizeof(rs));
					session=CreateSession(Sender);
					chdr->SessionID=session->SessionID;
					RTLogNum=0;
					chdr->CheckSum=0;
					chdr->CheckSum=in_chksum((void*)chdr,sizeof(TCmdHeader));
					session->Send=SendRSData;
					session->Speed=THEBR;
					SendRSData((void*)chdr, sizeof(TCmdHeader), Sender);
				}				
			}
			else
				DBPRINTF("CHECKSUM ERROR!,pch->Len=%d\n",pch->Len);
			return pch->Cmd;			
		}
	}
	while(rs->poll()) ch=rs->read();
	return -1;	
}

int SerialInputStr(serial_driver_t *serial, char *s, const int len)
{
	int c;
	int i;
	int numRead;
	int skipNewline = 1;
	int maxRead = len;
	
	for(numRead=0, i=0; numRead<maxRead;)
	{
		c = serial->read();

		/* check for errors */
		if(c < 0) 
		{
			s[i++] = '\0';
			return c;
		}
		/* eat newline characters at start of string */
		if((skipNewline == 1) && (c != '\r') && (c != '\n'))
			skipNewline = 0;
		if(skipNewline == 0)
		{
			if((c == '\r') || (c == '\n'))
			{
				s[i++] = '\0';
				return(numRead);
			} 
			else 
			{
				s[i++] = (char)c;
				numRead++;
			}
		}
	}
	return(numRead);
}
