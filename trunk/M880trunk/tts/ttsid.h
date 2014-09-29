#ifndef _TTS_ID_
#define _TTS_ID_

#include <stdlib.h>
#include <stdio.h>

static char *g_strTTSID[MaxTTSID];

void ParseID(char *Buffer,unsigned long *ID,char *Text,char Opt)
{
	char Index[256];
        char *Value;

	Value=Index;

        while (*Buffer)
        {
        	if (*Buffer=='\n'||*Buffer=='\t'||*Buffer==0x0A||*Buffer==0x0D||*Buffer=='\"')
        		++Buffer;
        	else
        	{
        		if (*Buffer==Opt)
        		{
        			*Value=0;
				*ID=atol(Index);
        			Value=Text;
        		}
        		else 
				*Value++=*Buffer;
			Buffer++;
        	}
        }
        *Value=0;
}

void InitTTSID(void)
{
	int i;

	for(i=0;i<MaxTTSID;i++)
		g_strTTSID[i]=NULL;
}

int LoadTTSID(char *Name)
{
        FILE *MyDB;
	char Buffer[512],Text[512];
	unsigned long ID;

        MyDB=fopen(Name,"rb+");
        if(!MyDB)
        {
        	fprintf(stderr,"error open TTS config file\n");
		return 0;
        }
        memset(Buffer,0,512);
        while (!feof(MyDB))
        {
        	if(fgets(Buffer,512,MyDB)==0)
        		break;
		if(*Buffer=='#'||*Buffer==0x0a||*Buffer==0x0d)
			continue;

        	if(strlen(Buffer))
        	{
        		ParseID(Buffer,&ID,Text,'=');

			if(ID>=0 && ID<=MaxTTSID)
				g_strTTSID[ID]=(char *)MALLOC(sizeof(char)*strlen(Text)+1);
			if(g_strTTSID[ID]) 
				strcpy(g_strTTSID[ID],Text);
        		else 
				break;
        	}
        }
        fclose(MyDB);
        return 1;										
}

void FreeTTSID(void)
{
	int i;
	
	for(i=0;i<MaxTTSID;i++)
	{
		if(g_strTTSID[i]) 
			FREE(g_strTTSID[i]);
	}
}

char *GetTTSID(int index)
{
	if(index>=0 && index<MaxTTSID)
	{
		return g_strTTSID[index];
	}
	return NULL;
}

#endif
