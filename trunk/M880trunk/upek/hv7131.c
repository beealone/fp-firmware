/*
 * Hv7131 CMOS camera sensor ops
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hv7131.h"
#include "arca.h"

// IOCTL commands
#define IOCTL_READ_REG   	0
#define IOCTL_WRITE_REG  	1

#define IOCTL_READ_EEPROM 	2
#define IOCTL_WRITE_EEPROM 	3
#define IOCTL_SET_ADDR            4 /* set i2c address */
#define IOCTL_SET_CLK             5 /* set i2c clock */


typedef struct
{
	unsigned char reg;
	unsigned char val;
} S_CFG;

typedef struct
{
	U8 len;
	U8 offset;
	U8 buf[256];
} EEPROM;

static int sensor_fd=-1;

/* integration time */
static unsigned int integration_time = 35; /* unit: ms */

/* master clock and video clock */
static unsigned int mclk_hz = 25000000;    /* 25 MHz */
static unsigned int vclk_div = 2;          /* VCLK = MCLK/vclk_div: 2,4,8,16,32 */

static unsigned char read_reg(unsigned char reg)
{
	S_CFG s;

	s.reg = reg;
	if (ioctl(sensor_fd, IOCTL_READ_REG, (unsigned long)&s) < 0) {
		printf("warning: read_reg failed!\n");
		return 0;
	}
	return s.val;
}

static int write_reg(unsigned char reg, unsigned char val)
{
	S_CFG s;
	s.reg = reg;
	s.val = val;
	if (ioctl(sensor_fd, IOCTL_WRITE_REG, (unsigned long)&s) < 0) {
		printf("warning: write_reg failed!\n");
		return -1;
	}
	return 0;
}


int sensor_set_clk(unsigned int clk)
{
        if (ioctl(sensor_fd, IOCTL_SET_CLK, (unsigned int)&clk) < 0) {
                printf("ioctl: set clock failed!\n");
                return -1;
        }
        return 0;
}

int sensor_set_addr(unsigned int addr)
{
        if (ioctl(sensor_fd, IOCTL_SET_ADDR, (unsigned int)&addr) < 0) {
                printf("ioctl: set address failed!\n");
                return -1;
        }
        return 0;
}

static void sensor_power_down(void)
{
        unsigned char val;

        val = read_reg(SCTRB);
	val |= 0x08;
        write_reg(SCTRB, val);
}

static void sensor_power_on(void)
{
        unsigned char val;

        val = read_reg(SCTRB);
	val &= ~0x08;
        write_reg(SCTRB, val);
}

/* left, top, width, height */
static void set_window(int l, int t, int w, int h)
{
	l=l/2*2;
	t=t/2*2;
	/* Set the row start address */
	write_reg(RSAU, (t >> 8) & 0xff);
	write_reg(RSAL, t & 0xff);

	/* Set the column start address */
	write_reg(CSAU, (l >> 8) & 0xff);
	write_reg(CSAL, l & 0xff);

	/* Set the image window width*/
	write_reg(WIWU, (w >> 8) & 0xff);
	write_reg(WIWL, w & 0xff);

	/* Set the image window height*/
	write_reg(WIHU, (h >> 8) & 0xff);
	write_reg(WIHL, h & 0xff);
#if 0
	{
		int i;
		for (i = 0x10; i < 0x18; i++)
			printf("REG[%02x]=%02x\n", i, read_reg(i));
	}
#endif
}

static void set_blanking_time(unsigned short hb_time, unsigned short vb_time)
{
	hb_time = (hb_time < 0xd0)? 0xd0 : hb_time;
	vb_time = (vb_time < 0x08)? 0x08 : vb_time;

	write_reg(HBLU, (hb_time >> 8) & 0xff);
	write_reg(HBLL, hb_time & 0xff);
	write_reg(VBLU, (vb_time >> 8) & 0xff);
	write_reg(VBLL, vb_time & 0xff);
}

static void set_integration_time(void)
{
        unsigned int regval;

	if (vclk_div == 0) vclk_div = 2;

	regval = (integration_time * mclk_hz)/ (1000*vclk_div); /* default: 0x065b9a */

	write_reg(INTH, (regval & 0xff0000) >> 16);
	write_reg(INTM, (regval & 0xff00) >> 8);
	write_reg(INTL, regval & 0xff);

	//xscale
	//write_reg(INTH, (FPReaderOpt&1)?1:0);
	//write_reg(INTM, 0x90);
}

/* VCLK = MCLK/div */
static void set_sensor_clock(int div)
{
#define ABLC_EN (1 << 3)
	/* ABLC enable */
	switch (div) {
	case 2:
		write_reg(SCTRA, ABLC_EN | 0x01);       // DCF=MCLK
		break;
	case 4:
		write_reg(SCTRA, ABLC_EN | 0x11);       // DCF=MCLK/2
		break;
	case 8:
		write_reg(SCTRA, ABLC_EN | 0x21);       // DCF=MCLK/4
		break;
	case 16:
		write_reg(SCTRA, ABLC_EN | 0x31);       // DCF=MCLK/8
		break;
	case 32:
		write_reg(SCTRA, ABLC_EN | 0x41);       // DCF=MCLK/16
		break;
	default:
		break;
	}
}

static int CMOSGain=0x30;

static void sensor_init(int left, int top, int width, int height, int FPReaderOpt)
{
        /* set clock */
        set_sensor_clock(vclk_div); 

        write_reg(SCTRB, 0x15);       // VsHsEn, HSYNC mode

        write_reg(OUTIV, 0x00);       //VCLK ouput polarrity is inverted

        /* set captured image size */
        set_window(left, top, width, height);

        set_blanking_time(0xff, 0x08); /* (hblank, vblank) */
        //set_blanking_time(0xd0, 0x04); /* (hblank, vblank) */
	
	set_integration_time(); //??

	if(FPReaderOpt&0x01)
	{	
		CMOSGain=0x90;
        	write_reg(PAG, CMOSGain);
	}
	else
	{
		CMOSGain=0x30;
        	write_reg(PAG, CMOSGain);
	}
        write_reg(RCG, 0x08);
        write_reg(GCG, 0x08);
        write_reg(BCG, 0x08);

        write_reg(ACTRA, 0x17);
        write_reg(ACTRB, 0x7f);

        write_reg(BLCTH, 0xff);       // set black level threshold
        write_reg(ORedI, 0x7f);
        write_reg(OGrnI, 0x7f);
        write_reg(OBluI, 0x7f);

        sensor_power_on();
}

void sensor_close(void)
{
	if(sensor_fd>0) sensor_power_down();
	close(sensor_fd);
	sensor_fd=-1;
}

int sensor_open(int left,int top, int width, int height, int FPReaderOpt)
{
	
	if(sensor_fd>0) sensor_close();

	sensor_fd = open("/dev/sensor", O_RDWR);
	if (sensor_fd < 0)
	{
		printf("Error opening /dev/sensor\n");
		return 0;
	}
      	//new 7131 must set
	if (sensor_set_addr(0x22) == 0)
		 sensor_set_clk(100000);

	sensor_init(left, top, width, height, FPReaderOpt);
	return sensor_fd;
}

int SetCMOSGain(int Gain)
{
	return write_reg(PAG, Gain);//Pre-amp Gain
}

int IncCMOSGain(void)
{
        if(CMOSGain>4)
                CMOSGain=(CMOSGain*5+2)/4;
        else
                CMOSGain=10;
        if(CMOSGain>255) CMOSGain=255;
        return SetCMOSGain(CMOSGain);
}

int DecCMOSGain(void)
{
        CMOSGain=CMOSGain*2/3;
        if(CMOSGain==0) CMOSGain=1;
        return SetCMOSGain(CMOSGain);
}

void FilterRGB(char *PixelsBuffer, int Width, int Height)
{
        int i,j;
        unsigned char p,*p1,*p2,*p3,*p4;
        p1 = (unsigned char *)PixelsBuffer; p2 = p1+1;
        p3 = p1+Width; p4 = p3+1;
        for(i = 0; i<Height-1; i++)
        {
                for(j = 0; j<Width-1; j++)
                {
                        p=(*p1+*p2+++*p3+++*p4+++2) / 4;
                        *p1++=p;
                }
                p1++; p2++; p3++; p4++;
        }
}

int Write24WC02(int Address, char *data, int size)
{
	EEPROM rom;
	memset(&rom, 0, sizeof(EEPROM));
	rom.len=size;
	rom.offset=0;
	memcpy(rom.buf, data, size);
	return (!ioctl(sensor_fd, IOCTL_WRITE_EEPROM, &rom));
}

int Read24WC02(int Address, char *data, int size)
{
	EEPROM rom;	
        if(sensor_fd>0) sensor_close();
        sensor_fd = open("/dev/sensor", O_RDWR);
	memset(&rom, 0, sizeof(EEPROM));
	rom.len=size;
	rom.offset=0;
	if(ioctl(sensor_fd, IOCTL_READ_EEPROM, &rom)==0)
	{
		memcpy(data, rom.buf, size);
		return 1;
	}
	else
		return 0;
}

