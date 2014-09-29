#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"

int Is8MFlash()
{
	struct stat sb;
	if(stat("/mnt/mtdblock/data",&sb) == -1)
		return 0;
	else 
	{
		if((sb.st_mode & S_IFMT) == S_IFDIR)
		{
			return 1;
		}
	}
	return 0;
}

int backup(const char *filepath)
{
	int ret=0;
	FILE *errfp;
	char cmdbuf[100];
	sprintf(cmdbuf,"cd RAMPATH\n tar -czvf %s.tgz ../mtdblock/*.dat ../mtdblock/data/*.dat 2>RAMPATH/err.txt",filepath);
	//printf("backup cmdbuf=%s\n",cmdbuf);
	systemEx(cmdbuf);
	if( (errfp=fopen("RAMPATH/err.txt","r")) == NULL)
	{
		ret=1;
	}
	else
	{
		char buff[256];
		if(fgets(buff,255,errfp) != NULL)
			ret=0;
		else 
			ret = 1;
		fclose(errfp);
	}
	return ret;

}

int restore(const char* filepath)
{
	//printf("restore file name =%s\n",filepath);
	int ret=0;
	FILE *errfp;
	char cmdbuf[100];
	sprintf(cmdbuf,"cd RAMPATH\n tar -xzvf %s 2>RAMPATH/err.txt",filepath);
	systemEx(cmdbuf);
	if( (errfp=fopen("err.txt","r")) == NULL)
	{
		ret=0;
	}
	else
	{
		char buff[256];
		if(fgets(buff,255,errfp) != NULL)
			ret=0;
		else 
			ret = 1;
		fclose(errfp);
	}
	return ret;

}


