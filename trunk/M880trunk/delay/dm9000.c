/*------------------------------------------------------------------------
 . DM9000.c
 . This is a driver for DM9000E single-chip Ethernet device.
 .
 .
 . Arguments:
 . 	io	= for the base address
 .	irq	= for the IRQ
 . 
 . author:
 . 	Richard Chen
 .
 .
 . Hardware multicast code from Peter Cammaert ( pc@denkart.be )
 .
 . Sources:
 .    o   DM9000E
 .
 ----------------------------------------------------------------------------*/

#include <time.h>
#include <string.h>
#include "pxa255.h"
#include "net.h"
#include "dm9000.h"
#include "options.h"

#define ReadPort(p) *(volatile U32*)(p)
#define WritePort(val, p) *(volatile U32*)(p)=val

#ifdef MAIN_PRG
#define THEMAC gOptions.MAC
#else
char THEMAC[]={0,0xf0,8,0xee,0x13,9}; 
#endif

static U32 DM9_ReadReg(U32 reg)
{
	U32 base = CONFIG_DM991X_BASE;
	WritePort(reg, base);
	return ReadPort(base+4);
}

static int DM9_ReadRegs(U32 reg, U32 *buf, int Cnt)
{
	int i;
	U32 base = CONFIG_DM991X_BASE;
	WritePort(reg, base);
	for(i=0;i<Cnt;i++)
		buf[i]=ReadPort(base+4);
	return i;
}

static U32 DM9_WriteReg(U32 reg, U32 val)
{
	U32 base = CONFIG_DM991X_BASE;
	WritePort(reg, base);
	WritePort(val, base+4);
	return 1;
}

static int DM9_WriteRegs(U32 reg, U32 *buf, int Cnt)
{
	int i;
	U32 base = CONFIG_DM991X_BASE;
	WritePort(reg, base);
	for(i=0;i<Cnt;i++)
		WritePort(buf[i],base+4);
	return i;
}

#define ETH_ZLEN 60

static int dm9_send_packet(volatile void *packet, int packet_length)
{
	int	length, time_out;

	length = ETH_ZLEN < packet_length ? packet_length : ETH_ZLEN;
	
	DM9_WriteRegs(0xf8,(U32*)packet,(length+3)>>2);
	/* Set TX length to DM9000 */
	DM9_WriteReg(0xfc,length & 0xff);
	DM9_WriteReg(0xfd,(length >> 8) & 0xff);
	/* Issue TX polling command */
	DM9_WriteReg(2,1);
	/* wait for transmited */
	time_out=0;
	while(1 & DM9_ReadReg(2)) if(time_out++>10000) return 0;
	
	return length;	
}

int dm9_init(void);

static int dm9_rcv()
{
	U8 rxbyte;
	U16 RxStatus, RxLen, GoodPacket;
	U32 tmpdata;
 	int device_wait_reset=FALSE;
 	TMyBuf *packet;
	DM9_ReadReg(0xf0);			/* Dummy read */
	rxbyte = DM9_ReadReg(0xf0);	/* Got most updated data */

	/* Status check: this byte must be 0 or 1 */
	if (rxbyte > DM9000_PKT_RDY) {
		DM9_WriteReg(0x05, 0x00);	/* Stop Device */
		DM9_WriteReg(0xfe, 0x80);	/* Stop INT request */
		device_wait_reset = TRUE; 
	}

	/* packet ready to receive check */
	if (rxbyte == DM9000_PKT_RDY) {
		/* A packet ready now  & Get status/length */
		GoodPacket = TRUE;
		WritePort(0xf2, CONFIG_DM991X_BASE);
		tmpdata  = ReadPort(CONFIG_DM991X_BASE+4);
		RxStatus = tmpdata;
		RxLen	 = tmpdata >> 16;
		
		/* Packet Status check */
		if (RxLen < 0x40) GoodPacket = FALSE; 
		if (RxLen > DM9000_PKT_MAX) device_wait_reset = TRUE; 
		if (RxStatus & 0xbf00) GoodPacket = FALSE;

		/* Move data from DM9000 */
		if(RxLen)
		if (!device_wait_reset) {
			if ( GoodPacket ) {
				packet=bget(0);
				DM9_ReadRegs(0xf2,(U32*)(packet->buf),(RxLen+3)/4);
				packet->len=RxLen;
				return RxLen;
			} else {
				DM9_ReadRegs(0xf2,(U32*)(packet->buf),(RxLen+3)/4);
			}
		}
	}
	if(device_wait_reset) dm9_init();
	return 0;
}

static int dm9_close(void)
{
	return 1;
}

static int dm9_open(void)
{
	return dm9_test();
}

int dm9_test(void)
{
	int i;
	char mac[6];
	for(i=0;i<6;i++)
		mac[i]=DM9_ReadReg(i+0x10);
	for(i=0;i<6;i++)	
		if(mac[i]!=THEMAC[i]) 
		{
			return 0;
		}
	return 1;
}

int dm9_init(void)
{
	int i,v=0x40000000,time_out,c;
	
	GPDR1|=v;
	GPSR1=v;
	DelayUS(0x2710);
	GPCR1=v;
	DelayUS(0x186a0);
	
	//TODO DM9000_SetPhy
	DM9_WriteReg(0,1);	DelayUS(0x5e8); 
	DM9_WriteReg(0,0);  
	
	DM9_WriteReg(0x1e, 1);
	DM9_WriteReg(0x1f, 0);	
	
	/* wait for rx transmited */
	time_out=0;
	while(1 & DM9_ReadReg(2)) if(time_out++>10000) return 0;
	
	//Set Physical Address Bytes
	c=0;
	while(c++<100)
	{
		for(i=0;i<6;i++)
		{
			DM9_WriteReg(i+0x10,THEMAC[i]);
			DelayUS(100);
		}
		if(dm9_test()) break;
	}
		
	//Set Multicast Address Bytes
	v=0xff;
	for(i=0x16;i<=0x1d;i++)
		DM9_WriteReg(i,v);

	DM9_WriteReg(v,0x80);
	DM9_WriteReg(5,0x31);
	
	return 1;	
}

int eth_init(void) {
	int ok;

	ok = dm9_init();

	if (ok) return dm9_open();

	return 0;
}

void eth_halt() {
	dm9_close();
}
 
int eth_rx() {
	if(dm9_rcv()>0)
	{
		/* Pass the packet up to the protocol layers. */
		net_rx();
		return 1;
	}
	else
		return 0;
}

int eth_rcv(){
	return dm9_rcv();
}

int eth_xmit(PMyBuf packet) {
	int ret;

	ret = dm9_send_packet(packet->buf, packet->len);
	packet->len = 0;
	return ret;
}
