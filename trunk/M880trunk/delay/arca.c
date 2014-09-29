#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include "arca.h"

static int fdArcaIO = -1;

void DelayUS(int us)
{
    int i, j;
    int loopTimes = 500; //²âÊÔ½á¹û500´Î´ó¸Å10us
    int DelayTimes = us/10;
    
    if (us <= 20*1000){
	for (j=0; j<DelayTimes; j++)
	    for(i=0; i<loopTimes; i++) ;
    }else
    {
	usleep(us);
    }  
}

BOOL ARCA_IO_Init(void)
{
    fdArcaIO = open ("/dev/mem", O_RDWR);
    if (fdArcaIO < 0)
	return FALSE;
    else{
	//mmap arca ¼Ä´æÆ÷
	arca_reg = (unsigned char *) mmap(0, 32*1024, PROT_READ|PROT_WRITE,
					  MAP_SHARED, fdArcaIO, ARCA_REG_BASE);
	if (arca_reg == NULL) {
	   close(fdArcaIO);
	   return FALSE;
       }
	//arca gpio ¼Ä´æÆ÷
	gpio_reg = (arca_gpio_t *)(arca_reg + ARCA_GPIO_REG);
	
	//Init for keypad
	gpio_reg->gpio_pacr &= ~0xf0;        /* 4,5,6,7 for in  */
	gpio_reg->gpio_pacr |=   0x0f;        /* 0,1,2,3 for out  */
	gpio_reg->gpio_padr &= ~0x0f;	
	//Init for USB LCM CMOS power and AC97 
	gpio_reg->gpio_pbcr &= ~((RELAY_PIN|USB_POWER_PIN|AC97_CTL)<<8); /* gpio */
	gpio_reg->gpio_pbcr |= (RELAY_PIN|USB_POWER_PIN|AC97_CTL);      /* gpio for out  */
	//Init for RS485
	gpio_reg->gpio_pdcr &= ~((RS485_SEND)<<8); /* gpio */
	gpio_reg->gpio_pdcr |= (RS485_SEND);       /* gpio for out  */
	gpio_reg->gpio_pddr &= ~(RS485_SEND);     /* default setting: 0 for receive  */
	
       return TRUE;
   }
}

void ARCA_IO_Free(void)
{
    munmap((void *)arca_reg, 32*1024);
    if (fdArcaIO > 0)
	close(fdArcaIO);
}

void ARCA_AC97_Mute(BOOL Switch)
{
    if (Switch)
	gpio_reg->gpio_pbdr |= AC97_CTL;
    else
	gpio_reg->gpio_pbdr &= ~AC97_CTL;
}

void ARCA_USB_Power(BOOL Switch)
{
    if (Switch)
	gpio_reg->gpio_pbdr |= USB_POWER_PIN;
    else
	gpio_reg->gpio_pbdr &= ~USB_POWER_PIN;
}
