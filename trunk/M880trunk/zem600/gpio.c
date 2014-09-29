
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include "arca.h"

#define LEDR	104
#define LEDG	105


#define GPIO_DIR_IN             0
#define GPIO_DIR_OUT            1
#define GPIO_LEVEL_LOW          0
#define GPIO_LEVEL_HIGH         1

/* ioctl command */

#define SETDIRECTION            0x01
#define GETDIRECTION            0x02
#define SETLEVEL                0x03
#define GETLEVEL                0x04
#define SETRISINGEDGE           0x05
#define GETRISINGEDGE           0x06
#define SETFALLINGEDGE          0x07
#define GETFALLINGEDGE          0x08
#define GETEDGEDETECTSTATUS     0x09
#define CLEAREDGEDETECTSTATUS   0x0A

#define NANDREADPAGE		0x0B
#define NANDWRITEPAGE		0x0C
#define NANDREADBLOCK		0x0D
#define NANDWRITEBLOCK		0x0E
#define NANDREASEBLOCK		0x0F
#define NANDSAVEMAC		0x10
#define NANDGETBLOCKSTATUS      0x12 //dsl 2011.5.5

int fdIO = -1;

struct _MSG
{
	int param0;
	int param1;
	unsigned char buf[2048];
}Info;


//static int BASELOOPTESTCNT = 30;
/*
void DelayUS(int us)
{
        int i, j, k;

        if (us <= 20*1000)
        {
                for (i=0; i<us; i++)
                        for(j=0; j<BASELOOPTESTCNT; j++) k++;
        }
        else
        {
                usleep(us);
        }
}

void DelayMS(int ms)
{
        usleep(1000*ms);
}
*/
void __gpio_as_output(int n)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
	Info.param0 = n;
	Info.param1 = GPIO_DIR_OUT;

	ioctl(fdIO, SETDIRECTION,&Info);
	
	return;
}
 
void __gpio_as_input(int n)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
	Info.param0 = n;
	Info.param1 = GPIO_DIR_IN;

	ioctl(fdIO, SETDIRECTION,&Info);
	
	return;
}


void __gpio_set_pin(int n)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
	Info.param0 = n;
	Info.param1 = GPIO_LEVEL_HIGH;

	ioctl(fdIO, SETLEVEL, &Info);
	
	return;
}

void __gpio_clear_pin(int n)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
	Info.param0 = n;
	Info.param1 = GPIO_LEVEL_LOW;

	ioctl(fdIO, SETLEVEL, &Info);
	
	return;
}

int __gpio_get_pin(int n)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return -1;
	}
	Info.param0 = n;

	ioctl(fdIO, GETLEVEL,&Info);
	
	return Info.param1;
}

int gpio_open(void)
{
	fdIO = open("/dev/gpio", O_RDWR);
	if(fdIO<0)	
		printf("Error: open dev/gpio\n");

	return fdIO;
}


void Nand_read_onePage(U32 page, U8 *buf)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
	Info.param0 = page;
	memset(Info.buf, 0, sizeof(Info.buf));
	ioctl(fdIO, NANDREADPAGE,&Info);
	memcpy(buf, Info.buf, sizeof(Info.buf));
	return;
}

void Nand_write_onePage(U32 page, U8 *buf)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
	Info.param0 = page;
	memcpy(Info.buf, buf, sizeof(Info.buf));
	ioctl(fdIO, NANDWRITEPAGE,&Info);
	return;
}

void Nand_read_oneBlock(U32 page, U8 *buf)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
//	ioctl(fdIO, NANDREADBLOCK,&buf);
	return;
}

void Nand_write_oneBlock(U32 page, U8 *buf)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
//	ioctl(fdIO, NANDWRITEBLOCK,&buf);
	return;
}

void Nand_erase_oneBlock(U32 block)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
	Info.param0 = block;
	ioctl(fdIO, NANDREASEBLOCK,NULL);
	return;
}

void Nand_save_MAC(U8 *buf)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return;
	}
	memcpy(Info.buf, buf,6);
	ioctl(fdIO, NANDSAVEMAC,&Info);
	return;
}

/*
int main(int argc, int **argv)
{
	fdIO = open("/dev/gpio", O_RDWR);
	if(fdIO<0)	
	{
		printf("Error: open dev/gpio\n");
		return 0;
	}
	
	__gpio_as_output(LEDR);
	__gpio_as_output(LEDG);
	__gpio_as_output(100);
	__gpio_as_output(101);
	__gpio_as_output(103);
	__gpio_as_output(108);

	
	while(1)
	{
		printf("Flash LED ...\n");
		__gpio_set_pin(104);
		__gpio_set_pin(105);
		__gpio_set_pin(100);
		__gpio_set_pin(101);
		__gpio_set_pin(103);
		__gpio_set_pin(108);

		DelayMS(1000);

		__gpio_clear_pin(104);
		__gpio_clear_pin(105);
		__gpio_clear_pin(100);
		__gpio_clear_pin(101);
		__gpio_clear_pin(103);
		__gpio_clear_pin(108);
		DelayMS(1000);

	//	printf("The Status of Pin: %d, %d\n",__gpio_get_pin(LEDR),__gpio_get_pin(LEDG));
	}

	return 0;
}
*/

/*dsl 2011.5.5*/
int Nand_Block_isBad(U32 block)
{
	if(fdIO<0)
	{
		printf("invalid operation!\n");
		return -1;
	}
	return 0;
#if 0
	Info.param0 = block;
	if(ioctl(fdIO, NANDGETBLOCKSTATUS,&Info))
	{
		return -1;
	}
	return Info.param1;
#endif
}
