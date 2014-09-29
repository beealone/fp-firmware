/********************************************************************
                                           
 ZEM 200                                          
                                                    
 jz4730.h defines all the constants and others for Jz4730 CPU
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
********************************************************************/

#ifndef	_JZ4730_H_
#define	_JZ4730_H_

#include <unistd.h>

#define TRUE	1
#define FALSE	0

#define	U8    	unsigned char
#define	U16   	unsigned short
#define	U32   	unsigned int
#define BOOL  	int

#define BYTE  	unsigned char

#define WORD 	unsigned short

#define DWORD	unsigned long

#define LONG	long

#define CONFIG_DEBUG

#ifdef CONFIG_DEBUG
#define DBPRINTF(format, arg...) do { printf(format, ## arg); } while (0)
#else
#define DBPRINTF(format, arg...) 
#endif

#ifndef XL_DEBUG
#define XLDBPRINTF(format, arg...)					\
do 									\
{ 									\
	printf(format, ## arg);						\
	printf(", %s,%s,%d.\n",__FILE__,__FUNCTION__,__LINE__);		\
} while (0)
#else 
#define XLDBPRINTF(format, arg...) 	
#endif

//#define  MEM_DEBUG
#ifdef MEM_DEBUG 
	extern void *pmalloc;
	extern U32 malloci ;
	extern U32 realloci ;
	#define	FREE(p)	 	\
	do					\
	{					\
		if (p != NULL) 	\
		{				\
			XLDBPRINTF("free p=%p,freei=%u",p,--malloci);\
			free(p);   	\
			p = NULL;	\
		}				\
	}while(0)

	#define	MALLOC(s)	 	( pmalloc= (void*)malloc(s),	\
	printf("malloc p=%p,malloci=%u,s=%u, %s,%s,%d.\n",  pmalloc,\
	malloci,s,__FILE__,__FUNCTION__,__LINE__),++malloci,pmalloc)

	#define REALLOC(p,s)      ( pmalloc= (void*)realloc(p,s),	\
	printf("realloc p=%p,realloci=%u,s=%u, %s,%s,%d.\n",  pmalloc,\
	realloci,s,__FILE__,__FUNCTION__,__LINE__),++realloci,pmalloc)
#else	
	#define	FREE(p)	 	\
	do					\
	{					\
		if (p != NULL) 	\
		{				\
			free(p);   	\
			p = NULL;	\
		}				\
	}while(0)
	#define MALLOC(s)	 	(malloc(s))
	#define REALLOC(p,s)  	(realloc(p,s))
#endif


#define  AC97_CTL    	 	53   
#define  I2S_CTL		AC97_CTL
#define  RS485_SEND		102

#define  IO_GREEN_LED		104
#define  IO_RED_LED		105
#define  IO_LOCK		103
#define  IO_WIEGAND_OUT_D0	100
#define  IO_WIEGAND_OUT_D1	101

#define  WEI_DN			104
#define	 WEI_DP			105
#define  USB_SEL                120
#define TFT_BACK_LIGHT		20
#define GPIO_BATTERY		103

#define	REG8	 volatile unsigned char
#define	REG16	 volatile unsigned short
#define	REG32	 volatile unsigned int
#define	VPchar	 *(REG8 *)
#define	VPshort	 *(REG16 *)
#define	VPint	 *(REG32 *)
#define	Pchar	 (REG8 *)
#define	Pshort	 (REG16 *)
#define	Pint	 (REG32 *)

void CalibrationTimeBaseValue(void);
void DelayUS(int us);
void DelayMS(int ms);
void DelayNS(long ns);

BOOL GPIO_IO_Init(void);
void GPIO_IO_Free(void);
void GPIO_PIN_CTL_SET(int IsWiegandKeyPad, int IsNetSwitch);

void GPIO_AC97_Mute(BOOL Switch);
void GPIO_HY7131_Power(BOOL Switch);
void GPIO_LCD_USB0_Power(BOOL Switch);
void GPIO_RS485_Status(U32 SendMode);
void GPIO_SYS_POWER(void);
void GPIO_WIEGAND_LED(BYTE GPIOPIN, BOOL Light);
void GPIO_KEY_SET(BYTE Value);
int GPIO_KEY_GET(void);

void GPIOSetLevel(BYTE IOPIN, int High);
BOOL GPIOGetLevel(BYTE IOPIN);
BOOL ExCheckGPI(BYTE GPIOPIN);

#endif /* _JZ4730_H_ */

