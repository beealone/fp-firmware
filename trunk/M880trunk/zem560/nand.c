

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#define LEDR	104
#define LEDG	105

#define U32	unsigned int
#define U8	unsigned char

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
#define NANDGETBLOCKSTATUS      0x12

int fdnandflash = -1;

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

int gpio_open(void)
{
	fdnandflash = open("/dev/nand", O_RDWR);
	if(fdnandflash<0)	
		printf("Error: open dev/gpio\n");

	return fdnandflash;
}
int gpio_close(void)
{
	if(fdnandflash>=0)
		close(fdnandflash);
	return 0;
}


int Nand_read_onePage(U32 page, U8 *buf)
{
	if(fdnandflash<0)
	{
		printf("invalid operation!\n");
		return -1;
	}
	Info.param0 = page;

	memset(Info.buf, 0, sizeof(Info.buf));
	if(ioctl(fdnandflash, NANDREADPAGE,&Info)==0)
	{
		memcpy(buf, Info.buf, sizeof(Info.buf));
		return 0;
	}
	else
		return -1;
}

int Nand_write_onePage(U32 page, U8 *buf)
{
	if(fdnandflash<0)
	{
		printf("invalid operation!\n");
		return -1;
	}
	Info.param0 = page;
	memcpy(Info.buf, buf, sizeof(Info.buf));
	return ioctl(fdnandflash, NANDWRITEPAGE,&Info);
}

void Nand_read_oneBlock(U32 page, U8 *buf)
{
	if(fdnandflash<0)
	{
		printf("invalid operation!\n");
		return;
	}
//	ioctl(fd, NANDREADBLOCK,&buf);
	return;
}

void Nand_write_oneBlock(U32 page, U8 *buf)
{
	if(fdnandflash<0)
	{
		printf("invalid operation!\n");
		return;
	}
//	ioctl(fd, NANDWRITEBLOCK,&buf);
	return;
}

int Nand_erase_oneBlock(U32 block)
{
	if(fdnandflash<0)
	{
		printf("invalid operation!\n");
		return -1;
	}
	Info.param0 = block;
	return ioctl(fdnandflash, NANDREASEBLOCK,&Info);
}

int Nand_Block_isBad(U32 block)
{
	if(fdnandflash<0)
	{
		printf("invalid operation!\n");
		return -1;
	}
	Info.param0 = block;
	if(ioctl(fdnandflash, NANDGETBLOCKSTATUS,&Info))
		return -1;
	return Info.param1;
}

int Nand_save_MAC(U8 *buf)
{
	if(fdnandflash<0)
	{
		printf("invalid operation!\n");
		return -1;
	}
	memcpy(Info.buf, buf,6);
	return ioctl(fdnandflash, NANDSAVEMAC,&Info);
}

#if 0
int main(int argc, int **argv)
{
#define LOGOSIZE	(2*128*1024)
	fd = open("/dev/nand", O_RDWR);
	if(fd<0)	
	{
		printf("Error: open dev/gpio\n");
		return 0;
	}
	
	char mac[6]={0x00,0x17,0x61,0x33,0x44,0x55};

//	Nand_save_MAC(mac);

	unsigned char logo[LOGOSIZE];

	int fdf=open("linuxlogo.bin",O_RDWR);
	if(fdf<0)
	{
		printf("Error: open linuxlogo.bin\n");
	close(fd);
		return 0;
	}
	
	int fsize=lseek(fdf, 0, SEEK_END);
	lseek(fdf, 0, SEEK_SET); 

	printf("logo size:%d\n",fsize);
	if(fsize<LOGOSIZE)
		if(read(fdf, logo, fsize)==fsize)
			SaveLOGOToFlash(11, logo, fsize);
		else
			printf("read failed\n");
	else
		printf("fsize is too large\n");

	close(fdf);
	close(fd);
	printf("ok .\n");

	return 0;
	//	printf("The Status of Pin: %d, %d\n",__gpio_get_pin(LEDR),__gpio_get_pin(LEDG));

}
#endif
