#include <stdlib.h>
#include <stdio.h>

#include "ttsts.h"

TS g_TSLIST[MAXTS];
int g_TSTOTAL;

int TS_Load(char *Name,TS *ts,int maxts)
{
	FILE *fp;
	int cnt;
	TS cts;

	fp=fopen(Name,"rb+");

	cnt=0;

	if(fp)
	{
		while(!feof(fp))
		{
			if(fread(&cts,sizeof(TS),1,fp)==0) 
				break;
			memcpy(&ts[cnt++],&cts,sizeof(TS));
		}
		fclose(fp);
	}
	return cnt;	
}

int TS_Save(char *Name,TS *ts,int maxts)
{
	FILE *fp;
	int cnt;

	fp=fopen(Name,"wb+");

	cnt=0;

	if(fp)
	{
		for(cnt=0;cnt<maxts;cnt++)
			fwrite(&ts[cnt],sizeof(TS),1,fp);
		fclose(fp);
	}
	return cnt;
}
