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

//Define gpio for zem500

#define	 GPIO33			33
#define	 GPIO100		100
#define	 GPIO101		101
#define	 GPIO_BATTERY		103
#define	 GPIO104		104
#define	 GPIO105		105
#define	 GPIO107		107

//Define gpio for zem300

#define  USB_POWER_PIN 		104   //HV7131
#define  LCM_BACK_LIGHT		105	
#define  AC97_CTL    	 	106   
#define  RS485_SEND		33
#define  ZK_SENSOR_INT		67   

#define  IO_GREEN_LED		32
#define  IO_RED_LED		33
#define  IO_LOCK		34
#define  IO_ALARM_STRIP		35
#define  IO_WIEGAND_OUT_D0	36
#define  IO_WIEGAND_OUT_D1	37
#define  IO_DOOR_BUTTON		38
#define  IO_DOOR_SENSOR		39

#define  EXT_GPIO_1		99
#define  EXT_GPIO_2		100
#define  EXT_GPIO_3		101
#define  EXT_GPIO_4		102

#define  SHUT_KEY     		103
#define  TFT_BACK_LIGHT           94

#define  WIE_IN_D0    		90
#define  WIE_IN_D1     		91
#define  WIE_OUT_D0     	88
#define  WIE_OUT_D1     	89

#define  WEI_DN			89
#define	 WEI_DP			88

#define  JZ_REG_BASE  0x10000000  /* physical regs base */
#define  JZ_GPIO_REG  0x00010000  /* GPIO regs offset */
#define  JZ_I2C_REG   0x00042000  /* I2C regs offset */
#define  JZ_AC97_REG  0x00020000  /* AC97 regs offset */
#define  JZ_EMI_REG   0x03010000  /* EMI regs offset */

#define  JZ_EMI_BASE  0x13010000  /* physical EMI regs base */

#define  FLASH_BASE   0x1fc00000

#define  JZ_AC97_ACFR_RST      0x00000008
#define  JZ_AC97_ACFR_ENB      0x00000001
#define  JZ_AC97_ACCR2_RDMS    0x00008000
#define  JZ_AC97_ACCR2_TDMS    0x00004000
#define  JZ_AC97_ACCR2_ENLBF   0x00000020
#define  JZ_AC97_ACCR2_ENRBL   0x00000010
#define  JZ_AC97_ACCR2_ENREC   0x00000008
#define  JZ_AC97_ACCR2_SR      0x00000004
#define  JZ_AC97_ACCR2_SS      0x00000002
#define  JZ_AC97_ACCR2_SA      0x00000001
#define  JZ_AC97_ACSR_CRDY     0x00100000
#define  JZ_AC97_ACSR_RSTO     0x00040000
#define  JZ_AC97_ACSR_SADR     0x00020000
#define  JZ_AC97_ACSR_CADT     0x00010000
#define  JZ_AC97_ACSR_ROR      0x00000040
#define  JZ_AC97_ACSR_TUR      0x00000020
#define  JZ_AC97_ACSR_RFS      0x00000010
#define  JZ_AC97_ACSR_TFS      0x00000008
#define  JZ_AC97_ACIER_ERSTO   0x00040000
#define  JZ_AC97_ACIER_ERADR   0x00020000
#define  JZ_AC97_ACIER_ECADT   0x00010000
#define  JZ_AC97_ACIER_EROR    0x00000040
#define  JZ_AC97_ACIER_ETUR    0x00000020
#define  JZ_AC97_ACIER_ERFS    0x00000010
#define  JZ_AC97_ACIER_ETFS    0x00000008

typedef struct jz_ac97 {
   unsigned int ac97_acfr;
   unsigned int ac97_accr;
   unsigned int ac97_accr1;
   unsigned int ac97_accr2;
   unsigned int ac97_i2scr;
   unsigned int ac97_sr;
   unsigned int ac97_acsr;
   unsigned int ac97_i2ssr;
   unsigned int ac97_accar;
   unsigned int ac97_accdr;
   unsigned int ac97_acsar;
   unsigned int ac97_acsdr;
   unsigned int ac97_i2sdiv;
   unsigned int ac97_acdr;
} jz_ac97_t;

typedef struct jz_i2c {
   unsigned char  i2c_dr;
   unsigned char  res1[3];
   unsigned char  i2c_cr;
   unsigned char  res2[3];
   unsigned char  i2c_sr;
   unsigned char  res3[3];
   unsigned short i2c_gr;
   unsigned char  res4[2];
} jz_i2c_t;

typedef struct jz_gpio {
   unsigned int   gpio_gpdr0;
   unsigned int   gpio_gpdir0;
   unsigned int   gpio_gpodr0;
   unsigned int   gpio_gppur0;
   unsigned int   gpio_gpalr0;
   unsigned int   gpio_gpaur0;
   unsigned int   gpio_gpidlr0;
   unsigned int   gpio_gpidur0;
   unsigned int   gpio_gpier0;
   unsigned int   gpio_gpimr0;
   unsigned int   gpio_gpfr0;
   unsigned int   res0;
   unsigned int   gpio_gpdr1;
   unsigned int   gpio_gpdir1;
   unsigned int   gpio_gpodr1;
   unsigned int   gpio_gppur1;
   unsigned int   gpio_gpalr1;
   unsigned int   gpio_gpaur1;
   unsigned int   gpio_gpidlr1;
   unsigned int   gpio_gpidur1;
   unsigned int   gpio_gpier1;
   unsigned int   gpio_gpimr1;
   unsigned int   gpio_gpfr1;
   unsigned int   res1;
   unsigned int   gpio_gpdr2;
   unsigned int   gpio_gpdir2;
   unsigned int   gpio_gpodr2;
   unsigned int   gpio_gppur2;
   unsigned int   gpio_gpalr2;
   unsigned int   gpio_gpaur2;
   unsigned int   gpio_gpidlr2;
   unsigned int   gpio_gpidur2;
   unsigned int   gpio_gpier2;
   unsigned int   gpio_gpimr2;
   unsigned int   gpio_gpfr2;
   unsigned int   res2;
   unsigned int   gpio_gpdr3;
   unsigned int   gpio_gpdir3;
   unsigned int   gpio_gpodr3;
   unsigned int   gpio_gppur3;
   unsigned int   gpio_gpalr3;
   unsigned int   gpio_gpaur3;
   unsigned int   gpio_gpidlr3;
   unsigned int   gpio_gpidur3;
   unsigned int   gpio_gpier3;
   unsigned int   gpio_gpimr3;
   unsigned int   gpio_gpfr3;
} jz_gpio_t;

typedef struct jz_emi {
   unsigned int   emi_bcr;
   unsigned int   res1[3];
   unsigned int   emi_smcr0;
   unsigned int   emi_smcr1;
   unsigned int   emi_smcr2;
   unsigned int   emi_smcr3;
   unsigned int   emi_smcr4;
   unsigned int   emi_smcr5;
   unsigned int   res2[2];
   unsigned int   emi_sacr0;
   unsigned int   emi_sacr1;
   unsigned int   emi_sacr2;
   unsigned int   emi_sacr3;
   unsigned int   emi_sacr4;
   unsigned int   emi_sacr5;
   unsigned int   res3[2];
   unsigned int   emi_nfcsr;
   unsigned int   emi_nfecc;
   unsigned int   res4[2];
   unsigned int   emi_pccr1;
   unsigned int   emi_pccr2;
   unsigned int   emi_pccr3;
   unsigned int   emi_pccr4;
   unsigned int   res5[4];
   unsigned int   emi_dmcr;
   unsigned short emi_rtcsr;
   unsigned short res6;
   unsigned short emi_rtcnt;
   unsigned short res7;
   unsigned short emi_rtcor;
   unsigned short res8;
   unsigned int   emi_dmar1;
   unsigned int   emi_dmar2;
} jz_emi_t;

#define	REG8	 volatile unsigned char
#define	REG16	 volatile unsigned short
#define	REG32	 volatile unsigned int
#define	VPchar	 *(REG8 *)
#define	VPshort	 *(REG16 *)
#define	VPint	 *(REG32 *)
#define	Pchar	 (REG8 *)
#define	Pshort	 (REG16 *)
#define	Pint	 (REG32 *)

//JZ register
unsigned char *jz_reg;
unsigned char *jz_gpio_base;
//JZ GPIO register
jz_gpio_t *gpio_reg;

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

