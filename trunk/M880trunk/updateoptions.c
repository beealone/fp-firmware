/*///////////////////////////////////////////////////////
  升级U盘中的 option2.cfg 参数表，带入的参数为
  option2.cfg在 U 盘中的路径＋文件名（option2.cfg)；

  备注:
  1.  要升级的参数文件名为option2.cfg ，升级时，要在存入U  盘

  2. 增加此功能软件需增加的文件有: Updateoptions.c  , Des.c , Des.h

  3. 如果在采屏机器中增加此功能，并且软件修改方式如下:

  在"固件升级"中增加options 参数升级功能。

  (1). 如果U盘中只有UodateOptions.cfg 文件，就只升级options 参数，若升级成功就
  会提示"参数升级成功,请重新启动您的机器! "；

  (2). 如果U盘中只有固件升级文件emfw.cfg ，就只升级固件，若升级成功就
  会提示"固件升级成功，请重启机器"；

  (3). 如果二者都升级成功，就提示"固件和参数升级成功,请重新启动您的机器!"

  (4). ,如果U盘中两个文件都没有，或者两个都升级失败，将提示"固件和参数
  升级失败，请检测您的升级包！"


  那么采屏机器Language 中要修改和增加的语言有:

  LANGUAGE.S :
  开始升级,请稍后....
  参数升级成功,请重新启动您的机器!
  参数升级失败,请检查您的升级文件!
  固件和参数升级成功,请重新启动您的机器!
  固件和参数升级失败,请检查您的升级包!

  LANGUAGE.E :		
  Updateing,wait....
  Parameter Update Successfully!
  Parameter Update Failed!
  Firmware and Parameter Update Successfully!
  Firmware and Parameter Update Failed!

//////////////////////////////////////////////////////////*/
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "usb_helper.h"
#include "usb_helper.h"
#include "Des.h"
#include "options.h"
#include "utils.h"

extern int fdOptions;
int UpdateOptions(char *filename)
{
	int count=0, same=0;
	char buffer[VALUE_BUFFERLEN];
	char buf[VALUE_BUFFERLEN];
	char value_u[VALUE_BUFFERLEN];
	char value_o[VALUE_BUFFERLEN];
	char name_u[128],name_o[128];
	char key[128];
	int size=0;
	int fdU=-1;
	char *p=NULL;

	sprintf(buffer, "%s/%s", USB_MOUNTPOINT, filename);	
	fdU=open(buffer, O_RDONLY);
	if((fdU>0)&&(fdOptions>0))
	{
		memset(key,0,sizeof(key));
		lseek(fdU, 0, SEEK_SET); 
		while(TRUE)
		{
			memset(buf,0,sizeof(buf));
			memset(buffer,0,sizeof(buffer));
			if(ReadOneLine(fdU, buf, &size))
			{
				if((buf[0]=='A')&&(buf[1]=='A')&&(buf[2]=='A')&&(buf[3]=='A')
						&&(buf[4]=='5')&&(buf[5]=='5')&&(buf[6]=='5')&&(buf[7]=='5'))
				{
					p=buf+8;
					sprintf(buffer,"%s",p);
				}
				else
					Encrypt_Des(buffer, buf, strlen(buf), "20080808", 8, DECRYPT);

				SplitByChar(buffer, name_u, value_u, '=');

				same=0;
				lseek(fdOptions, 0, SEEK_SET); 
				while(TRUE)
				{
					if(ReadOneLine(fdOptions, buffer, &size))
					{
						SplitByChar(buffer, name_o, value_o, '=');
						if(strcmp(name_u,name_o)==0)
						{
							if(strcmp(value_u,value_o)==0)
								same=1;
							else
								same=0;
						}
					}
					else
						break;
				}
				if(!same)
				{
					sprintf(buffer,"%s=%s\n",name_u,value_u);
					lseek(fdOptions, 0, SEEK_END); 
					write(fdOptions,buffer,strlen(buffer));		
				}
				count++;
			}
			else
				break;
		}
	}
	if (fdU > 0) {	
		close(fdU);
	}
	if (fdOptions > 0) {
		fsync(fdOptions);
		//close(fdOptions);
	}
	sync();
	return count;

}

//***********************************************************
//
// filename--must include path
//
//************************************************************
extern char *GetEnvFilePath(const char *EnvName, const char *filename, char *fullfilename);
int UpdateOptionsNoEncrypt(char *filename)
{
	int count=0, same=0;
	char buffer[VALUE_BUFFERLEN];
	char buf[VALUE_BUFFERLEN];
	char value_u[VALUE_BUFFERLEN];
	char value_o[VALUE_BUFFERLEN];
	char name_u[128],name_o[128];
	int size=0;
	int fdU=-1;
	//	char *p=NULL;

	if (fdOptions<=0)
	{
		GetEnvFilePath("USERDATAPATH", "options.cfg", buffer);
		fdOptions=open(buffer, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		memset(buffer,0,sizeof(buffer));
	}

	//sprintf(buffer, "%s/%s", "/mnt/sdcard", filename);
	sprintf(buffer, "%s", filename);
	fdU=open(buffer, O_RDONLY);
	if((fdU>0)&&(fdOptions>0))
	{
		lseek(fdU, 0, SEEK_SET); 
		while(TRUE)
		{
			memset(buf,0,sizeof(buf));
			memset(buffer,0,sizeof(buffer));
			if(ReadOneLine(fdU, buf, &size))
			{
				SplitByChar(buf, name_u, value_u, '=');

				same=0;
				lseek(fdOptions, 0, SEEK_SET); 
				while(TRUE)
				{
					if(ReadOneLine(fdOptions, buf, &size))
					{
						SplitByChar(buf, name_o, value_o, '=');
						if(strcmp(name_u,name_o)==0)
						{
							if(strcmp(value_u,value_o)==0)
								same=1;
							else
								same=0;
						}
					}
					else
						break;
				}
				if(!same)
				{
					sprintf(buf,"%s=%s\n",name_u,value_u);
					lseek(fdOptions, 0, SEEK_END); 
					write(fdOptions,buf,strlen(buf));		
				}
				count++;
			}
			else
				break;
		}
		//sprintf(buffer, "rm %s -rf", filename);
		//system("rm /mnt/sdcard/options.cfg -rf ");
		systemEx(buffer);
		fsync(fdOptions);
	}
	if (fdU > 0) {
		close(fdU);
	}
	sync();
	return count;
}

extern int SaveLinuxLOGOToFlash(const unsigned char *data, int size);
extern int SaveBufferTONandFlash(const unsigned char *data, int size, int a, int b, int c);
#define CFG_LEN		(65*1024)
#define UPDATEPATH	"/mnt/mtdblock/data"
int UpdateSoftWareFromUDisk(void)
{
	int mount;
	int sing=0,logo=0,err=0;		//update flag
	char buf[128],buffer[CFG_LEN];
	int fd=-1,flen;
	FILE *fp;

	mount=DoMountUdisk();
	if (mount==0)
	{
		memset(buf,0,sizeof(buf));
		memset(buffer,0,sizeof(buffer));
		//升级固件
		sprintf(buf, "%s/update.tgz",USB_MOUNTPOINT);
		fd=open(buf, O_RDONLY);
		if (fd>0)
		{
			close(fd);
			//修改固件升级路径
			//sprintf(buf, "tar xvzf %s -C %s && sync && rm %s -rf",buf,"/mnt/mtdblock/",buf);
			sprintf(buf, "tar xvzf %s/update.tgz -C %s && sync ",USB_MOUNTPOINT, UPDATEPATH);
			//printf("path:%s\n",buf);
			if (systemEx(buf)==EXIT_SUCCESS)
				err=1;
		}

		//升级option.cfg参数
		//sing=UpdateOptions("option2.cfg");
		memset(buf,0,sizeof(buf));
		sprintf(buf, "%s/%s", UPDATEPATH, "options.cfg");
		//printf("path:%s\n",buf);
		sing=UpdateOptionsNoEncrypt(buf);
		memset(buf,0,sizeof(buf));
		sprintf(buf, "rm %s/%s -f", UPDATEPATH, "options.cfg");
		systemEx(buf);


		//升级LOGO数据
		memset(buf,0,sizeof(buf));
		sprintf(buf, "%s/%s", UPDATEPATH, "linuxlogo");
		//printf("path:%s\n",buf);
		//fd=open(buf, O_RDONLY);
		fp=fopen((const char *)buf,"r");
		if (fp>0)
		{
			fseek(fp,0,SEEK_END);
			flen = ftell(fp);
			if (flen<=CFG_LEN)
			{
				fseek(fp,0,SEEK_SET);
				fread(buffer,1,flen,fp);
#ifdef ZEM600
				logo=SaveLinuxLOGOToFlash((unsigned char*)buffer, flen);		
#else
#if defined(ZEM510) && defined(LINUX_26KERNEL)
				logo=SaveLOGOToFlash(11,buffer,flen);
#else
				//针对510彩屏机器LOGO更新位置
				logo=SaveBufferTONandFlash(buffer, 704,831, flen,0);  //address is Nand page address
#endif
#endif
			}
			fclose(fp);
			memset(buf,0,sizeof(buf));
			sprintf(buf, "rm %s/%s -f", UPDATEPATH, "linuxlogo");
			systemEx(buf);
		}

		DoUmountUdisk();
		if((err)&&(sing)&&(logo))
			return 1;
	}

	return 0;
}



