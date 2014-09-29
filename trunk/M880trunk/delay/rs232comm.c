#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pxa255.h"
#include "serial.h"
#include "net.h"
#include "utils.h"
#include "commu.h"
#include "options.h"

#define CMD_RS	0x81
#define WORD unsigned short

#ifdef MAIN_PRG
#define THEBR gOptions.RS232BaudRate
#define THEDID gOptions.DeviceID
#else
#define THEBR 115200
#define THEDID 1
#endif


typedef PACKED struct _RSHeader_{
	char HeaderTag[2];
	char Cmd;
	char CmdCheck;
	WORD Len;
	WORD CheckSum;
}GCC_PACKED TRSHeader, *PRSHeader;

void RS_SendDataPack(serial_driver_t *rs, unsigned char *buf, unsigned short size)
{
	unsigned short cs;
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
#ifdef MAIN_PRG
	if(rs==&st232) 
	{
		int dd=gOptions.RS232BaudRate/10000;
		RS485_setmode(TRUE);
		if(dd>0) dd=4*115200/dd; else dd=4*115200;
		cs=0;
		while(cs<packet->len)
		{ 
			rs->write(packet->buf[cs++]);
			if(cs%256==0) 
			{
				DelayUS(dd);
			}
		}
		rs->flush_output();
//		DelayUS(3000); 		//for 115200
		DelayUS(3000*115200/gOptions.RS232BaudRate);
		RS485_setmode(FALSE);

	}
	else
#endif
	{
		for(cs=0;cs<packet->len;cs++) rs->write(packet->buf[cs]);
		rs->flush_output();
	}
	packet->len=0;
}

int SendRSData(void *buf, int size, void *param)
{
	RS_SendDataPack(*(void **)param, (unsigned char *)buf, (WORD)size);
	return 0;
}

WORD rsc=0, PS1[256], PS2[256], PSIndex=0;

int RS232Check(serial_driver_t *rs)
{
	int i, j;
	U32 ch,sum;
	TMyBuf *packet;
	PRSHeader pch;
	
	if(rs->poll()==0) return 0;
	i=0;sum=0;
	packet=bget(0);
	pch=(PRSHeader)(packet->buf);
	rsc++;
	while(1)
	{
		j=200;
		while(rs->poll()==0) 
		{
			DelayUS(2000);
			if(++j>=220) 
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
						continue;
					}
				}
//				ExBeep(5);
				return -1;
			}
		}
		ch=rs->read();
		packet->buf[i]=ch;
		if(i<=1)
		{
#ifdef MAIN_PRG
			if(rs==&st232 && (ch!=(U32)THEDID)) break;
#endif
		}
		else if(i==3)
		{
			if(pch->Cmd+pch->CmdCheck!=0xFF) break;
			if(pch->Cmd!=CMD_RS) break;
		}
		else if(i==sizeof(TRSHeader)-1)
		{
			if(pch->Len>BUFSZ-sizeof(TRSHeader)) break;
		}
		else if(i==(int)(pch->Len+sizeof(TRSHeader)-1))
		{
			sum=pch->CheckSum;
			pch->CheckSum=0;
			if(sum==in_chksum(packet->buf, pch->Len+sizeof(TRSHeader)))
			{
				PCommSession session;
				int cmd=RunCommand(packet->buf+sizeof(TRSHeader), pch->Len);
				if(cmd==0) return -2;
				if(cmd==CMD_CONNECT)
				{
					char Sender[SENDER_LEN];
					PCmdHeader chdr=(PCmdHeader)(packet->buf+sizeof(TRSHeader));
					memset(Sender, 0, SENDER_LEN);
					memcpy(Sender, (void*)&rs, sizeof(rs));
					session=CreateSession(Sender);
					chdr->SessionID=session->SessionID;
					chdr->CheckSum=0;
					chdr->CheckSum=in_chksum((void*)chdr,sizeof(TCmdHeader));
					session->Send=SendRSData;
					session->Speed=THEBR;
					SendRSData((void*)chdr, sizeof(TCmdHeader), Sender);
				}
				else if(CMD_RESTART==cmd)
					SetPC(0);		
//				while(rs->poll()) ch=rs->read();
			}
			return pch->Cmd;
		}
		i++;
	}
	while(rs->poll()) ch=rs->read();
//	ExBeep(5);DelayUS(100000);ExBeep(5);
	return -1;	
}