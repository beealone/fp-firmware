#ifndef	_ARCA_H_
#define	_ARCA_H_

#include <unistd.h>

#define TRUE	1
#define FALSE	0

#define	U8    unsigned char
#define	U16   unsigned short
#define	U32   unsigned int
#define BOOL  int
#define BYTE  unsigned char

#define  RELAY_PIN      	0x01
#define  USB_POWER_PIN 	0x02
#define  AC97_CTL    	 	0x04  //gpb2/irq2
#define  SHUT_KEY     	0x40  //gpb6/irq6
#define  RS485_SEND		0x20  //gpd5

#define  ARCA_GPIO_REG  0x0500
#define  ARCA_I2C_REG   0x0800
#define  ARCA_AC97_REG  0x0900
#define  ARCA_EMI_REG   0xe1020000
#define  ARCA_REG_BASE  0xe0000000

#define  ARCA_AC97_ACFR_RST      0x00000008
#define  ARCA_AC97_ACFR_ENB      0x00000001
#define  ARCA_AC97_ACCR2_RDMS    0x00008000
#define  ARCA_AC97_ACCR2_TDMS    0x00004000
#define  ARCA_AC97_ACCR2_ENLBF   0x00000020
#define  ARCA_AC97_ACCR2_ENRBL   0x00000010
#define  ARCA_AC97_ACCR2_ENREC   0x00000008
#define  ARCA_AC97_ACCR2_SR      0x00000004
#define  ARCA_AC97_ACCR2_SS      0x00000002
#define  ARCA_AC97_ACCR2_SA      0x00000001
#define  ARCA_AC97_ACSR_CRDY     0x00100000
#define  ARCA_AC97_ACSR_RSTO     0x00040000
#define  ARCA_AC97_ACSR_SADR     0x00020000
#define  ARCA_AC97_ACSR_CADT     0x00010000
#define  ARCA_AC97_ACSR_ROR      0x00000040
#define  ARCA_AC97_ACSR_TUR      0x00000020
#define  ARCA_AC97_ACSR_RFS      0x00000010
#define  ARCA_AC97_ACSR_TFS      0x00000008
#define  ARCA_AC97_ACIER_ERSTO   0x00040000
#define  ARCA_AC97_ACIER_ERADR   0x00020000
#define  ARCA_AC97_ACIER_ECADT   0x00010000
#define  ARCA_AC97_ACIER_EROR    0x00000040
#define  ARCA_AC97_ACIER_ETUR    0x00000020
#define  ARCA_AC97_ACIER_ERFS    0x00000010
#define  ARCA_AC97_ACIER_ETFS    0x00000008


typedef struct arca_ac97 {
   unsigned int ac97_acfr;
   unsigned int ac97_accr1;
   unsigned int ac97_accr2;
   unsigned int ac97_acsr;
   unsigned int ac97_acier;
   unsigned int ac97_accar;
   unsigned int ac97_accdr;
   unsigned int ac97_acsar;
   unsigned int ac97_acsdr;
   unsigned int ac97_acodr;
   unsigned int ac97_acidr;
} arca_ac97_t;

typedef struct arca_i2c {
   unsigned char  i2c_dr;
   unsigned char  res1[3];
   unsigned char  i2c_cr;
   unsigned char  res2[3];
   unsigned char  i2c_sr;
   unsigned char  res3[3];
   unsigned short i2c_gr;
   unsigned char  res4[2];
} arca_i2c_t;
   
typedef struct arca_gpio {
   unsigned int   gpio_pacr;
   unsigned char  gpio_padr;
   unsigned char  res1[11];

   unsigned int   gpio_pbcr;
   unsigned char  gpio_pbdr;
   unsigned char  res2[3];
   unsigned short gpio_pbetr;
   unsigned char  res3[2];
   unsigned char  gpio_pbfr;
   unsigned char  res4[3];

   unsigned int   gpio_pccr;
   unsigned char  gpio_pcdr;
   unsigned char  res5[11];

   unsigned int   gpio_pdcr;
   unsigned char  gpio_pddr;
   unsigned char  res6[11];

   unsigned int   gpio_pecr;
   unsigned char  gpio_pedr;
   unsigned char  res7[11];

   unsigned int   gpio_pfcr;
   unsigned char  gpio_pfdr;
   unsigned char  res8[3];
} arca_gpio_t;

typedef struct arca_emi {
   unsigned int   emi_bcr;
   unsigned int   emi_smcr0;
   unsigned int   emi_smcr1;
   unsigned int   emi_smcr2;
   unsigned int   emi_smcr3;
   unsigned int   emi_dmcr;
   unsigned short emi_rtcsr;
   unsigned char  res1[2];
   unsigned short emi_rtcnt;
   unsigned char  res2[2];
   unsigned short emi_rtcor;
   unsigned char  nop_rtcor;
   unsigned int   emi_dmar1;
   unsigned int   emi_dmar2;
   unsigned int   emi_dmar3;
   unsigned int   emi_dmar4;
} arca_emi_t;

#define	REG8	 volatile unsigned char
#define	REG16	 volatile unsigned short
#define	REG32	 volatile unsigned int
#define	VPchar	 *(REG8 *)
#define	VPshort	 *(REG16 *)
#define	VPint	 *(REG32 *)
#define	Pchar	 (REG8 *)
#define	Pshort	 (REG16 *)
#define	Pint	 (REG32 *)

//ARCA¼Ä´æÆ÷
unsigned char *arca_reg;
//ARCA GPIO ¼Ä´æÆ÷
arca_gpio_t *gpio_reg;

void DelayUS(int us);

BOOL ARCA_IO_Init(void);
void ARCA_IO_Free(void);

void ARCA_AC97_Mute(BOOL Switch);
void ARCA_USB_Power(BOOL Switch);

#endif

