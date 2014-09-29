/*************************************************
                                           
 ZEM 200                                          
                                                    
 jz4730.c init funtions for GPIO/AC97 MUTE/USB POWER
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      	
 
*************************************************/

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include "arca.h"
#include "gpio.h"
#include "flash.h"
#include "main.h"
//#include "options.h"
#define ZEM500

static int fdJzIO = -1;
//static int soundplay=0;

static int BASELOOPTESTCNT = 20;

int GPIOCvtTbl[]={      33,100,101,0,0,0,105,104,0,0,
                        0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,
                        103,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,103};


//because the minmize interval on linux is 10ms, use it when need
//it maybe not exatctly
void CalibrationTimeBaseValue(void)
{
	int i, j, k=0;
    	struct timeval tv;
    	struct timezone tz;
    	unsigned int s_msec, e_msec;
    	int DelayTimes=200*1000;
	
	gettimeofday(&tv, &tz);
    	s_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
    
    	for (i=0; i<DelayTimes; i++)
		for(j=0; j<BASELOOPTESTCNT; j++) k++; 
	
    	gettimeofday(&tv, &tz);
    	e_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
    
    	DBPRINTF("DELAY US=%d BASECNT=%f\n", e_msec-s_msec, ((float)DelayTimes/(e_msec-s_msec))*BASELOOPTESTCNT);    
    	BASELOOPTESTCNT=(int)(((float)DelayTimes/(e_msec-s_msec))*BASELOOPTESTCNT+1);
    
	#if 0
    	gettimeofday(&tv, &tz);
    	s_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
 
    	DelayUS(1000*1000);
    
    	gettimeofday(&tv, &tz);    
    	e_msec = tv.tv_sec*1000*1000 + tv.tv_usec;
    
	DBPRINTF("DELAY US=%d\n", e_msec-s_msec);    
	#endif    
}

void DelayUS(int us)
{
    	int i, j, k;
    
	for (i=0; i<us; i++)
		for(j=0; j<BASELOOPTESTCNT; j++) k++; 


}

void DelayMS(int ms)
{
	DelayUS(1000*ms);
}

void DelayNS(long ns)
{
        struct timespec req;

        req.tv_sec=0;
        req.tv_nsec=ns;
        nanosleep(&req, NULL);
}

extern int gpio_open(void);
BOOL GPIO_IO_Init(void)
{
        int i;
        fdJzIO = open ("/dev/mem", O_RDWR);
	int fd;
	fd=gpio_open();
	if (fd < 0)
	{
		printf("open nand fail!\n");
	}

        if (fdJzIO < 0)
                return FALSE;
        else
        {
                //mmap jz register
                jz_reg = (unsigned char *) mmap(0, 0x100000, PROT_READ|PROT_WRITE,
                                          MAP_SHARED, fdJzIO, JZ_REG_BASE);
                if (jz_reg == NULL)
                {
                        close(fdJzIO);
                        return FALSE;
                }

                //jz gpio base
                jz_gpio_base=(unsigned char *)(jz_reg + JZ_GPIO_REG);
                gpio_reg = (jz_gpio_t *)(jz_reg + JZ_GPIO_REG);

#ifdef ZEM500
                __gpio_as_output(GPIO33);     //multi used by sp2
                __gpio_as_output(GPIO100);    
                __gpio_as_output(GPIO101);    
                __gpio_as_output(GPIO104);    
                __gpio_as_output(GPIO105);
                __gpio_as_output(GPIO107);

                __gpio_as_input(GPIO_BATTERY);    
		
                __gpio_set_pin(GPIO107);        /* enable i2s voice */
                __gpio_set_pin(GPIO33);      /* output high level */
#else
                __gpio_as_output(USB_POWER_PIN); /* gpio pin as output */
                __gpio_as_output(RS485_SEND);    /* gpio pin as output */
                __gpio_as_output(SHUT_KEY);     //SHUT_KEY = 103 is Eth/232 switch

                __gpio_clear_pin(USB_POWER_PIN); /* output low level  */
                __gpio_set_pin(AC97_CTL);        /* output high level */
                __gpio_set_pin(RS485_SEND);      /* output high level */
        //      __gpio_set_pin(SHUT_KEY);        /* output high level */ none for ZEM500
#endif
                //flash 4M Bytes
                FlashBaseAddr = (REG16 *)mmap(0, 0x400000, PROT_READ|PROT_WRITE,
                                              MAP_SHARED, fdJzIO, FLASH_BASE);
                if (FlashBaseAddr == NULL)
                {
                        close(fdJzIO);
                        return FALSE;
                }
                gFlash16 = FlashBaseAddr + 0x100000; //2M BYTES OFFSET
                //if (GetFlashID() == FLASH_TOP_BOOT)  //跟nor flash的相关资料需要删除掉
                {
                        gFlash16 -= STD_SECTOR_SIZE; //64KBytes 
                        DBPRINTF("FLASH IS TOP BOOT TYPE!\n");
                }

                for(i = 0; i < FLASH_SECTOR_COUNT; i++)
                        FlashSectorSizes[i] = STD_SECTOR_SIZE;
                return TRUE;
        }
}

void GPIO_IO_Free(void)
{
    	munmap((void *)jz_reg, 0x100000);
	munmap((void *)FlashBaseAddr, 0x400000);
    	if (fdJzIO > 0)
		close(fdJzIO);
}
#ifdef ZEM500
void GPIO_PIN_CTL_SET(int IsWiegandKeyPad, int IsNetSwitch)
{
}
#else
void GPIO_PIN_CTL_SET(int IsWiegandKeyPad, int IsNetSwitch)
{
	if (IsWiegandKeyPad)
	{
		//Init for control
		if(IsNetSwitch) //A8 V7
		{
			__gpio_as_input(IO_DOOR_SENSOR); 	//0
			__gpio_as_output(IO_DOOR_BUTTON); 	//1
			__gpio_as_output(IO_WIEGAND_OUT_D1); 	//2
			__gpio_as_output(IO_WIEGAND_OUT_D0); 	//3
			__gpio_as_input(IO_ALARM_STRIP); 	//4
			__gpio_as_output(IO_LOCK); 		//5
			__gpio_as_output(IO_RED_LED); 		//6
			__gpio_as_output(IO_GREEN_LED); 	//7
			//setup initial status for gpio pins
        		gpio_reg->gpio_gpdr1 |= 0x77;
        		gpio_reg->gpio_gpdr1 &= 0xffffff77;
		}
		else //F4+ A6 X688(A11)
		{
			__gpio_as_input(IO_DOOR_SENSOR); 	//0
			__gpio_as_input(IO_DOOR_BUTTON); 	//1
			__gpio_as_output(IO_WIEGAND_OUT_D1); 	//2
			__gpio_as_output(IO_WIEGAND_OUT_D0); 	//3
			__gpio_as_input(IO_ALARM_STRIP); 	//4
			__gpio_as_output(IO_LOCK); 		//5
			__gpio_as_output(IO_RED_LED); 		//6
			__gpio_as_output(IO_GREEN_LED); 	//7
			//setup initial status for gpio pins
        		gpio_reg->gpio_gpdr1 |= 0x37;
        		gpio_reg->gpio_gpdr1 &= 0xffffff37;
		}
	}
	else //A5 K8
	{
		//Init for keypad			arca jz 
		__gpio_as_output(IO_DOOR_SENSOR); 	//0  //7
		__gpio_as_output(IO_DOOR_BUTTON); 	//1  //6
		__gpio_as_output(IO_WIEGAND_OUT_D1); 	//2  //5
		__gpio_as_output(IO_WIEGAND_OUT_D0); 	//3  //4
		__gpio_as_input(IO_ALARM_STRIP); 	//4  //3
		__gpio_as_input(IO_LOCK); 		//5  //2
		__gpio_as_input(IO_RED_LED); 		//6  //1
		__gpio_as_input(IO_GREEN_LED); 		//7  //0

		__gpio_as_output(WEI_DP);
		__gpio_as_output(WEI_DN);
		
		gpio_reg->gpio_gpdr1 &= 0xffffff0f;
	}
}
#endif
//H/W exist some problems, so don't use it, we will fixed it on the next version
void GPIO_AC97_Mute(BOOL Switch)
{
	if (Switch)
		__gpio_clear_pin(AC97_CTL);
	else
		__gpio_set_pin(AC97_CTL);
}

//control SENSOR ON or OFF 
void GPIO_HY7131_Power(BOOL Switch)
{
	if (Switch)
		__gpio_clear_pin(USB_POWER_PIN); /* output low level */
	else
		__gpio_set_pin(USB_POWER_PIN); /* output high level */
}
//control the USB0 power/LCD backgound light 
void GPIO_LCD_USB0_Power(BOOL Switch)
{
	if (Switch)
		__gpio_clear_pin(LCM_BACK_LIGHT); /* output low level */
	else
		__gpio_set_pin(LCM_BACK_LIGHT); /* output high level */
}

#define USB_HOST_CTL    108
void GPIO__USB_Host(BOOL Switch)
{
	if (Switch)
	{
		 __gpio_set_pin(USB_HOST_CTL); /* output high level */
	}
	else
	{
		__gpio_clear_pin(USB_HOST_CTL); /* output low level */
	}
}

//get network state(2008.01.21)
int GPIO_NETWORK_STATE(void)
{
	int fd;
	int LinkState = 0;
#if 1
	fd = open("/dev/tft_lcd", O_WRONLY);
	if (fd)
	{
		ioctl(fd, 0x468b, &LinkState);
		close(fd);	
	}	
#endif
	return LinkState;
}

//control the tft backlight 2007.04.30
void GPIO_TFT_BACK_LIGHT(BOOL Switch, int mode)
{
	if(!mode)
	{
		printf("GPIO_TFT_BACK_LIGHT\n");
		__gpio_as_output(TFT_BACK_LIGHT);
		if (Switch)
			__gpio_clear_pin(TFT_BACK_LIGHT); /* output low level */
		else
			__gpio_set_pin(TFT_BACK_LIGHT); /* output high level */
	}
	else
	{
		int fd;
		int back_level = 0;

		fd = open("/dev/tft_lcd", O_WRONLY);
		if(fd)
		{
			if(Switch)
			{
				if(brightness)		//liming
					back_level = (int)(100-brightness)*255/100;
				else
					back_level=(int)30*255/100;
			}
			else
			{
				back_level=10;
				ioctl(fd, 0x4688, &back_level);
				back_level=0;
			}
			ioctl(fd, 0x4688, &back_level);
			close(fd);
		}
		else
			printf("Error opening /dev/tft_lcd\n");
	}
}

void GPIO_RS485_Status(U32 SendMode)
{
	if (SendMode)
		__gpio_set_pin(RS485_SEND);      /* output high level */
	else
		__gpio_clear_pin(RS485_SEND);      /* output low level */
	DelayUS(1000);
}

#ifdef ZEM500
BOOL GPIOGetLevel(BYTE IOPIN)
{
        return __gpio_get_pin(GPIOCvtTbl[IOPIN]);
}

void GPIOSetLevel(BYTE IOPIN, int High)
{
        if (High)
                __gpio_clear_pin(GPIOCvtTbl[IOPIN]);
        else
                __gpio_set_pin(GPIOCvtTbl[IOPIN]);
}
/* return 1 means battery is full; return 0 means battery is empty */
int GetBatteryStatus(void)
{
	return __gpio_get_pin(GPIO_BATTERY);
}

#else
BOOL GPIOGetLevel(BYTE IOPIN)
{
	return __gpio_get_pin(IO_DOOR_SENSOR-(IOPIN&0x0f));
}

void GPIOSetLevel(BYTE IOPIN, int High)
{
	if (High)
		__gpio_clear_pin(IO_DOOR_SENSOR-(IOPIN&0x0f));
	else
		__gpio_set_pin(IO_DOOR_SENSOR-(IOPIN&0x0f));
}
#endif
BOOL ExCheckGPI(BYTE GPIOPIN)
{
	int i=200,c=0;
	while(--i)
	{
		if(!GPIOGetLevel(GPIOPIN)) if(++c>20) return FALSE;
		DelayUS(5);
	}
	return TRUE;
}

void GPIO_SYS_POWER(void)
{
	__gpio_clear_pin(SHUT_KEY);
}

void GPIO_WIEGAND_LED(BYTE GPIOPIN, BOOL Light)
{
	if (Light)
		__gpio_set_pin(GPIOPIN);
	else
		__gpio_clear_pin(GPIOPIN);
}

//It is used for A5 keypad scanning 
void GPIO_KEY_SET(BYTE Value)
{
#ifndef ZEM500
	gpio_reg->gpio_gpdr1&=0xffffff00;
	gpio_reg->gpio_gpdr1|=Value;
#endif
}
int GPIO_KEY_GET(void)
{
	return gpio_reg->gpio_gpdr1;
}

#if 0
int main(int argc, char **argv)
{
	CalibrationTimeBaseValue();   
}
#endif

