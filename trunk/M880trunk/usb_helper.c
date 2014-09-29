/*************************************************
                                           
 ZEM 200                                          
                                                    
 usb_helper.c support for USB FLASH PEN DRIVE 
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/mount.h>
#include <time.h>
#include <mntent.h>
#include <dirent.h>
#include <ctype.h>
#include "options.h"
#include "usb_helper.h"

//dsl 2012.4.16
//#define  S_IFDIR 16749

#define False 0
#define RETURN_ERROR -1
struct partitionEntry{
	char * device;
	unsigned int blocks; 
};

static struct partitionEntry *readPartitionList(void)
{
	FILE * f;
	struct partitionEntry *list=NULL;
	int numItems = 0;
	char line[256];
	int major, minor;
	int blocks;
	char name[256];
	char * frc;
	char *iobuf = NULL;

	f = fopen("/proc/partitions", "r");
	if (!f) 
	{
		DBPRINTF("failed to open /proc/partitions\n");
		return NULL;
	}
	iobuf = (char *)MALLOC(16384);
	if (iobuf) 
		setvbuf(f, iobuf, _IOFBF, 16384);
	/* first two lines are junk */
	frc = fgets(line, sizeof(line), f);
	if (frc) frc = fgets(line, sizeof(line), f);
	if (frc) frc = fgets(line, sizeof(line), f);
	while (frc) 
	{
		if (sscanf(line, "%d %d %d %s", &major, &minor, &blocks, name) != 4) 
		{
			line[strlen(line) - 1] = '\0';  /* chop */
			DBPRINTF("invalid line in /proc/partitions: '%s'\n", line);
			break;
		}
        
		/* throw away devices we aren't partitions, or don't begin with a "sd"*/
		/* if the blocks don't add up properly, give up on this device */
		if (((memcmp(name, "sd", 2)==0) || (memcmp(name, "ub", 2)==0)) && (blocks>0))
		{
			list = REALLOC(list, (numItems + 1) * sizeof(*list));
			list[numItems].device = strdup(name);
			list[numItems].blocks = blocks;
			numItems++;
		}
		frc = fgets(line, sizeof(line), f);
	}
	list = REALLOC(list, (numItems + 1) * sizeof(*list));
	list[numItems++].device = NULL;

	fclose(f);
	if (iobuf) FREE(iobuf);

	return list;
}

int CheckProcBusUsb()
{
	char  *ProcBusUsbPath_26="/proc/scsi/usb-storage";
#if 0
	int ret;
	struct stat buf = {0};
	ret = stat(ProcBusUsbPath_26, &buf);
	printf(" buf.st_mode =%d  buf.st_size = %d ,S_IFDIR=%d\n", buf.st_mode,buf.st_size, S_IFDIR);
	if (!ret)
	{
		if (buf.st_mode == S_IFDIR )
		{
			return  buf.st_mode;
		}
		else
		{
			printf("--------------1\n");
			return False;
		}

	}
#endif			
	if (opendir(ProcBusUsbPath_26)) {		
		return TRUE;
	}
	
	int FileCount=0;  
	int  tmp;
	char  *ProcBusUsbPath="/proc/bus/usb/001";
	
	struct dirent **ProcBusUsbEntryList;
	
	if((FileCount=scandir(ProcBusUsbPath,&ProcBusUsbEntryList,0,alphasort))<0)   
	{
		//printf("FileCount < 0 \n");
		return  0;        
	}
	else
	{	
		//printf("FileCount = %d !!!\n", FileCount);
		tmp=FileCount;        
		while(tmp--) FREE(ProcBusUsbEntryList[tmp]);
		FREE(ProcBusUsbEntryList);
		return FileCount-gOptions.USBFileCount; 			/* .and.. and 001(hub) */ 
	}
}

int DoMount(const char *device)
{
	char * MountPoint=USB_MOUNTPOINT;
	char * FileSystemType="vfat";
	int ret;	
	/* Call mount system call to mount device to MountPoint   */
	ret=mount(device,MountPoint,FileSystemType,MS_NOEXEC,NULL); 
	return ret==-1?0:1;   
}

int DoUmount(void)
{
	const char * MPoint=USB_MOUNTPOINT;
	int ret;
	
	ret=umount(MPoint); 
	if(ret==-1)
		DBPRINTF("umounted %s failed!\n",MPoint);
	else if (ret==0)
		DBPRINTF("umounted %s successfully!\n",MPoint);
	else
		DBPRINTF("umounted state Unknown!\n");
	return  ret==0?1:0;  
}

int DoLoopMount(struct partitionEntry * Entry)
{
	int i=0;
	char DevBuffer[261];
        
	while((Entry+i)&&(Entry+i)->device)
	{
		sprintf(DevBuffer, "/dev/%s", (Entry+i)->device);
		if(DoMount(DevBuffer))
		{
			DBPRINTF("capacity=%dM\n",((Entry+i)->blocks)/1024);
			DBPRINTF("Mount %s sucessfully\n",DevBuffer);
			return 1;
		}
		else   
			DBPRINTF("Mount %s failed\n",DevBuffer);
		i++;
	}
	return 0;
}	

void DoUmountUdisk(void)
{
	DoUmount();
}

int DoMountUdisk(void)
{
	struct partitionEntry  *Entry=NULL;
	int ret=0;
	int i;

	Entry = readPartitionList();
	i = CheckProcBusUsb();
	if (i>0)
	{
		DoUmountUdisk();
		if(!DoLoopMount(Entry))	
		{
			ret=1;	
			DBPRINTF("mounted failed\n");
		}
		else
			DBPRINTF("mount successful\n");    
	}
	else
	{
		ret=2;      
		DBPRINTF("Insert udisk please\n");
	}   
	
	if(Entry!=NULL)  FREE(Entry);
       
	return ret;  
}

int CheckUsbState(void)
{
	int ret=0;
	
	ret= CheckProcBusUsb();
      
	return ret;  
}

#if 0
int  main()
{
       struct partitionEntry  * Entry=NULL;
	int i;

	/* Debug readPartitionlist()*/        
	Entry=readPartitionList();
	i=0; 
 	while((Entry+i)&&(Entry+i)->device){
		DBPRINTF("Name:%s,Partition:%d,Block:%d\n",(Entry+i)->device,(Entry+i)->partition,(Entry+i)->blocks); 
		i++;
	}
	if(Entry!=NULL)  FREE(Entry);
        
	/* Debug CheckProcBusUsb      */
	DBPRINTF("proc/bus/usb/001/ have %d files\n",CheckProcBusUsb());  

	DoMountUdisk();

	//DoUmountUdisk();
}

#endif
