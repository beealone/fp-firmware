#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <asm/mman.h>
#include <sys/mman.h>
#include <string.h>

#include "cim.h"
#include "config.h"

typedef unsigned int u32;

struct cim_config {
        int mclk;/* Master clock output to the sensor */
        int vsp; /* VSYNC Polarity:0-rising edge is active,1-falling edge is active */
        int hsp; /* HSYNC Polarity:0-rising edge is active,1-falling edge is active */
        int pcp; /* PCLK Polarity:0-rising edge sampling,1-falling edge sampling */
        int dsm; /* Data Sampling Mode:0-CCIR656 Progressive Mode, 1-CCIR656 Interlace Mode,2-Gated Clock Mode,3-Non-Gated Clock Mode */
        int pack; /* Data Packing Mode: suppose received data sequence were 0x11 0x22 0x33 0x44, then packing mode will rearrange the sequence in the RXFIFO:
                     0: 0x11 0x22 0x33 0x44
                     1: 0x22 0x33 0x44 0x11
                     2: 0x33 0x44 0x11 0x22
                     3: 0x44 0x11 0x22 0x33
                     4: 0x44 0x33 0x22 0x11
                     5: 0x33 0x22 0x11 0x44
                     6: 0x22 0x11 0x44 0x33
                     7: 0x11 0x44 0x33 0x22
                  */
        int inv_dat;    /* Inverse Input Data Enable */
        int frc;        /* Frame Rate Control:0-15 */
        int trig;   /* RXFIFO Trigger:
                       0: 4
                       1: 8
                       2: 12
                       3: 16
                       4: 20
                       5: 24
                       6: 28
                       7: 32
                    */
};

static struct cim_config cim_cfg = {
#ifdef CONFIG_OV7660
	24000000, 1, 0, 0, 2, 2, 0, 0, 1, //ov7660
#else
	24000000, 0, 0, 1, 2, 2, 0, 0, 1,
		
#endif
};

/* image parameters */
typedef struct
{
        u32 width;      /* width */
        u32 height;     /* height */
        u32 bpp;        /* bits per pixel: 8/16/32 */
} IMG_PARAM;


typedef struct
{
        unsigned int cfg;
        unsigned int ctrl;
        unsigned int mclk;
} cim_config_t;


/*
 * IOCTL_XXX commands
 */
#define IOCTL_SET_IMG_PARAM     0       // arg type: IMG_PARAM *
#define IOCTL_CIM_CONFIG        1

static int cim_fd=-1;

static void set_img_param(IMG_PARAM *img)
{
	if (ioctl(cim_fd, IOCTL_SET_IMG_PARAM, (unsigned long)img) < 0) {
		printf("set_img_param failed!\n");
	}
}

int cim_open(int width, int height, int bpp)
{
	IMG_PARAM i;
	cim_config_t c;

	if(cim_fd>0) cim_close();
	cim_fd = open("/dev/cim", O_RDWR);
	if(cim_fd < 0)
	{
		printf("Error opening /dev/cim\n");
		return 0;
	}

	i.width = width;
	i.height = height;
	i.bpp = bpp;

	set_img_param(&i);
        c.mclk = cim_cfg.mclk;
        c.cfg = (cim_cfg.vsp << 14) | (cim_cfg.hsp << 13) | (cim_cfg.pcp << 12) | (cim_cfg.dsm << 0) | (cim_cfg.pack << 4) | (cim_cfg.inv_dat << 15);
        c.ctrl = (cim_cfg.frc << 16) | (cim_cfg.trig << 4);

        if (ioctl(cim_fd, IOCTL_CIM_CONFIG, (unsigned long)&c) < 0) {
                printf("IOCTL_CIM_CONFIG failed!\n");
                return 0;
        }

	return 1;
}

void cim_close(void)
{
	close(cim_fd);
	cim_fd=-1;
}

/* Read a frame of image */
int cim_read(unsigned char *buf, int frame_size)
{
	return read(cim_fd, buf, frame_size);
}

