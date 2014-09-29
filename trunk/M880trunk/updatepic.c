#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "usb_helper.h"
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

extern int pictype;
int AnalysePicFile(char *filename);
unsigned long UpdatePic(char *MountPoint,char *Name,char List[16][16]);
unsigned long GetPIC(char *MountPoint,char *Name,char List[10][16]);

int AnalysePicFile(char *filename)
{
	int i;
	char extname[4];
	memset(extname,0,sizeof(extname));
	for(i=0;i<strlen(filename);i++)
	{
		if(filename[i]=='.')
		{
			memcpy(extname,&filename[i+1],3);
			break;
		}
	}
	if(!strncmp(extname,"jpg",3))
		return 1;
	else
		return 0;
}

extern char* GetPicPath(char *name);
#define PIC_SIZE_LIMIT (30*1024)
unsigned long UpdatePic(char *MountPoint,char *Name,char List[16][16])
{
	char temp[256];
	FILE *fp;
	int i;
	int c=0;
	struct stat picsize;

//	printf("UpdatePic pictype %d\n",pictype);
	if(!pictype)
	{
		//只能上传文件名为ad_0～ad_9的图片
		for(i=0;i<10;i++)
		{
			memset(temp,0,sizeof(temp));
			sprintf(temp,"%s/%s%d.jpg",MountPoint,Name,i);
//			printf("upload pic file path:%s\n", temp);
			if((fp=fopen(temp,"rb"))==NULL)
				continue;
			else
			{
				stat(temp, &picsize);
//				printf("Picture size = %d\n", picsize.st_size);
				if(picsize.st_size>PIC_SIZE_LIMIT)
				{
//					printf("Picture size err!\n");
					fclose(fp);
					continue;
				}
				fclose(fp);
				if(c>15) 
					break;
				else
					sprintf(List[c++],"%s%d.jpg",Name,i);
			}
		}
	}
	else 
	{
		struct dirent *dent; 
		DIR *dir; 
		char temp2[100];
		memset(temp,0,sizeof(temp));
		//sprintf(buffer,"%s",GetPicPath("photo"));
		if(pictype==1)
			sprintf(temp,"%s/%s",USB_MOUNTPOINT,"photo/");
		else if(pictype==2)
			sprintf(temp,"%s",GetPicPath("photo/"));
		if((dir=opendir(temp))==NULL)
		{
			memset(temp2,0,sizeof(temp2));
			sprintf(temp2,"mkdir %s",temp);
			systemEx(temp2);
			sync();
			//opendir(&temp);
			return c;
		}
	        //done   =   findfirst("*.*",&ffile,0);   
	        while   ((dent=readdir(dir))!=NULL)     
	        {   
			if(AnalysePicFile(dent->d_name))
			{
				c++;
			}
	        }  
		//if(c>2)
		//	c-=2; 
 		closedir(dir);	
	}
	return c;
}

unsigned long GetPIC(char *MountPoint,char *Name,char List[10][16])
{
        char temp[256];
        FILE *fp;
        int i;
        struct stat picsize; //changed by cxp
        int c=0;
        for(i=0;i<99;i++)
        {
            sprintf(temp,"%s/%s%d.jpg",MountPoint,Name,i);
            if((fp=fopen(temp,"rb"))==NULL)
                        continue;
            else
            {
            	stat(temp, &picsize);  //changed by cxp at 2010-04-17
				if(picsize.st_size>PIC_SIZE_LIMIT)
				{
					fclose(fp);
					continue;
				}
            else
            {
				fclose(fp);
				if(c>9)
					break;
				else
					sprintf(List[c++],"%s%d.jpg",Name,i);
            }
        }
        }
        return c;
}


