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
#include "utils.h"

static int fdIO = -1;
//static int soundplay=0;

static int BASELOOPTESTCNT = 20;
int GPIOCvtTbl[]={	102,100,101,0,0,0,105,104,0,0,
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
    
//    DBPRINTF("DELAY US=%d BASECNT=%f\n", e_msec-s_msec, ((float)DelayTimes/(e_msec-s_msec))*BASELOOPTESTCNT);    
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
	//DBPRINTF("DelayMS zem500.c\n");
	DelayUS(1000*ms);
}

void DelayNS(long ns)
{
        struct timespec req;

        req.tv_sec=0;
        req.tv_nsec=ns;
        nanosleep(&req, NULL);
}

BOOL GPIO_IO_Init(void)
{
	fdIO = gpio_open();
    	if (fdIO < 0)
		return FALSE;
    	else 	
	{

		__gpio_as_output(AC97_CTL);      /* gpio pin as output */
		__gpio_as_output(RS485_SEND);    /* gpio pin as output */
		__gpio_as_output(IO_RED_LED); 		
		__gpio_as_output(IO_GREEN_LED); 	
		__gpio_as_output(TFT_BACK_LIGHT);
		//__gpio_as_input(TFT_BACK_LIGHT);
		
		__gpio_set_pin(AC97_CTL);        /* output high level */
	//	__gpio_clear_pin(AC97_CTL);        /* output low level */
		__gpio_set_pin(RS485_SEND);      /* output high level */
		
		CalibrationTimeBaseValue();
		return TRUE;
   	}
}
	
void GPIO_IO_Free(void)
{
    	if (fdIO > 0)
		close(fdIO);
}

void GPIO_PIN_CTL_SET(int IsWiegandKeyPad, int IsNetSwitch)
{	
#if 0
		
	__gpio_as_output(IO_WIEGAND_OUT_D1); 	//2
	__gpio_as_output(IO_WIEGAND_OUT_D0); 	//3
	__gpio_as_output(IO_LOCK); 		//5
	__gpio_as_output(IO_RED_LED); 		//6
	__gpio_as_output(IO_GREEN_LED); 	//7
	__gpio_as_output(I2S_CTL); 	//7

	__gpio_set_pin(I2S_CTL); //enable is2 
#endif
}
//H/W exist some problems, so don't use it, we will fixed it on the next version
void GPIO_AC97_Mute(BOOL Switch)
{
	if (!Switch)
		__gpio_clear_pin(AC97_CTL);
	else
		__gpio_set_pin(AC97_CTL);
}

//control SENSOR ON or OFF 
void GPIO_HY7131_Power(BOOL Switch)
{
#if 0
	if (Switch)
		__gpio_clear_pin(USB_POWER_PIN); /* output low level */
	else
		__gpio_set_pin(USB_POWER_PIN); /* output high level */
#endif
}
//control the USB0 power/LCD backgound light 
void GPIO_LCD_USB0_Power(BOOL Switch)
{
#if 0
	if (Switch)
		__gpio_clear_pin(LCM_BACK_LIGHT); /* output low level */
	else
		__gpio_set_pin(LCM_BACK_LIGHT); /* output high level */
#endif
}

void GPIO_RS485_Status(U32 SendMode)
{
	if (SendMode)
		__gpio_set_pin(RS485_SEND);      /* output high level */
	else
		__gpio_clear_pin(RS485_SEND);      /* output low level */
	DelayUS(1000);
}

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

BOOL ExCheckGPI(BYTE GPIOPIN)
{
/*
	int i=200,c=0;
	while(--i)
	{
		if(!GPIOGetLevel(GPIOPIN)) if(++c>20) return FALSE;
		DelayUS(5);
	} */
	return TRUE;
}

void GPIO_SYS_POWER(void)
{
//	__gpio_clear_pin(SHUT_KEY);
	return;
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

}

int GPIO_KEY_GET(void)
{
	return 0;
}

void GPIO_MODEM_INIT(void)
{
//        __gpio_as_input(MODEM_DCD);
}

int GPIO_MODEM_DCD(void)
{
//        return  __gpio_get_pin(MODEM_DCD);
	return 0;
}

//void GPIO_TFT_BACK_LIGHT(BOOL Switch)
void GPIO_TFT_BACK_LIGHT(BOOL Switch,int mode)
{

        if (Switch)
                __gpio_clear_pin(TFT_BACK_LIGHT); /* output low level */
        else
                __gpio_set_pin(TFT_BACK_LIGHT); /* output high level */
}

int GPIO_NETWORK_STATE(void)
{
	return 0;
}

/* return 1 means battery is full; return 0 means battery is empty */
int GetBatteryStatus(void)
{
        return __gpio_get_pin(GPIO_BATTERY);
}

