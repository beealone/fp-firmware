/*************************************************

 ZEM 200

 options.c all function for options

 Copyright (C) 2003-2005, ZKSoftware Inc.

 Author: Richard Chen

 Modified by David Lee for JFFS2 FS 2004.12.12

 $Log: options.c,v $
 Revision 5.19  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.18  2005/12/22 08:54:23  david
 Add workcode and PIN2 support

 Revision 5.17  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.16  2005/09/19 10:01:59  david
 Add AuthServer Function

 Revision 5.15  2005/08/18 07:16:56  david
 Fixed firmware update flash error

 Revision 5.14  2005/08/13 13:26:14  david
 Fixed some minor bugs and Modify schedule bell

 Revision 5.13  2005/08/04 15:42:53  david
 Add Wiegand 26 Output&Fixed some minor bug

 Revision 5.12  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.11  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk

 Revision 5.10  2005/07/07 08:09:02  david
 Fixed AuthServer&Add remote register

 Revision 5.9  2005/06/29 20:21:43  david
 Add MultiAuthServer Support

 Revision 5.8  2005/06/16 23:27:51  david
 Add AuthServer function

 Revision 5.7  2005/06/10 17:11:01  david
 support tcp connection

 Revision 5.6  2005/06/02 20:11:12  david
 Fixed SMS bugs and Add Power Button Control function

 Revision 5.5  2005/04/27 00:15:37  david
 Fixed Some Bugs

 Revision 5.4  2005/04/24 11:11:26  david
 Add advanced access control function

 Revision 5.3  2005/04/07 17:01:45  davidacc
 Modify to support A&C and 2 row LCD

*************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/tftmullan.h>


#include "arca.h"
#include "options.h"
#include "utils.h"
#include "sensor.h"
#include "finger.h"
#include "ccc.h"
#include "kb.h"
#include "netspeed.h"
#include "serial.h"
#include "exfun.h"
#include "locale.h"
#include "flashdb.h"

//仅仅支持中文的时候需要包含此文件，这个针对国内不支持多国语言的机器
#ifdef CHINESE_ONLY
#include "zk_chineseLng.h"
#endif

#define LngCachedStrCnt		3072	//2048
//#define LngCachedStrMaxLen	64	//64
#define LngCachedStrMaxLen      128      //64
#define LngCachedStrIndexCnt	3072

//dsl
#define PARAM_ENCYPT 0

//options file handle
int fdOptions = -1;
static int fdLanguage = -1;
static char CurLanguage = ' ';

static char LngCachedStr[LngCachedStrCnt][LngCachedStrMaxLen];
static WORD LngCachedStrIndex[LngCachedStrIndexCnt];

char *DateFormats[]={"YY-MM-DD","YY/MM/DD","YY.MM.DD", "MM-DD-YY","MM/DD/YY","MM.DD.YY","DD-MM-YY","DD/MM/YY","DD.MM.YY","YYYYMMDD"};


//zsliu
char Language[10]={0};	//for no language use

int FormatDate(char *buf, int index, int y, int m, int d)
{
	index=index%10;
	if(index==9)
        {
                //zsliu change
                if(gOptions.isUseHejiraCalendar)
                        sprintf(buf, "%04d-%02d-%02d", y,m,d);
                else
                        sprintf(buf, "%04d%02d%02d", y,m,d);
        }
	else
	{
		char ss;
		if(index<3) sprintf(buf, "%02d-%02d-%02d", y%100,m,d);
		else if(index<6) sprintf(buf, "%02d-%02d-%02d", m,d,y%100);
		else sprintf(buf, "%02d-%02d-%02d", d,m,y%100);
		if(index%3==0) ss='-';
		else if(index%3==1) ss='/';
		else ss='.';
		buf[2]=ss;buf[5]=ss;
	}
	return 8;
}

int FormatDate2(char *buf, int index, int m, int d)
{
	index=index%10;
	if(index==9)
		sprintf(buf, "%02d%02d", m,d);
	else
	{
		char ss;
		if(index<3) sprintf(buf, "%02d-%02d", m,d);
		else if(index<6) sprintf(buf, "%02d-%02d", m,d);
		else sprintf(buf, "%02d-%02d", d,m);
		if(index%3==0) ss='-';
		else if(index%3==1) ss='/';
		else ss='.';
		buf[2]=ss;
	}
	return 5;
}
//this function is only used for options.cfg, it is different with ReadOneLine
U32 PackStrBuffer(char *Buffer, const char *name, int size)
{
	char c, *cp, *namep, *valuep,*TheName;
	int i, isname, OriSize;
	char tmp[VALUE_BUFFERLEN];
	int offset=0;

	OriSize=size;
	TheName=(char*)MALLOC(size);
	namep=Buffer;
	valuep=namep;
	cp=Buffer;

	while(cp<(Buffer+size))
	{
		if(('='==*cp) && (valuep<=namep))
		{
			valuep=cp++;
			offset++;
		}
		else if((('\n'==*cp) || ('\r'==*cp)) && (cp>namep))
		{
			cp++;offset++;
			if (('\n'==*cp) || ('\r'==*cp)){cp++; offset++;}
			i=0;isname=1;
			while(1)
			{
				c=namep[i];
				if(c=='=')
				{
					TheName[i]=0;
					if(isname && name[i]) isname=0;
					break;
				}
				else
				{
					TheName[i]=c;
					if(c!=name[i]) isname=0;
				}
				i++;
			}
			if (isname || (LoadStrFromFile(fdOptions, TheName, tmp, TRUE, offset)!=-1))
			{ 	//delete this name and value
				memmove(namep,cp,size-(cp-Buffer));
				size-=cp-namep;
				memset(Buffer+size, 0, OriSize-size);
				cp=namep;
			}
			namep=cp;
		}
		else
		{
			cp++;
			offset++;
		}
		if('\0'==*cp) break;
	}
	FREE(TheName);
	return cp-Buffer;
}

//The strings are of the form name = value.
void CombineNameAndValue(const char *name, const char *value, int SaveTrue, char *processedStr)
{
	sprintf(processedStr, "%s=%s\n", name, value);
}

//support two format XXX=YYY OR "XXX=YYY" for compatible with zem100 language file format.
BOOL ReadOneLine(int fd, char *dest, int *size)
{
       char c;

       *size=0;
       while(TRUE)
       {
	       if (read(fd, &c, 1) == 1)
	       {
		       if((c == '\n') || (c == '\r') || (c == '"'))
		       {
			       if(*size==0)
				       continue;
			       else
				       break;
		       }
		       dest[*size] = c;
		       *size = *size + 1;
	       }
	       else
		       break;
       }
       if (*size > 0)
       {
	       dest[*size] = '\0';
       }
       return(*size > 0);
}

int GetFileCurPos(int fd)
{
       return lseek(fd, 0, SEEK_CUR);
}

void SplitByChar(char *buffer, char *name, char * value, char DeliChar)
{
       int cnt;
       char *p;

       p=buffer;
       cnt=0;
       while(*p)
       {
	       if (*p==DeliChar) break;
	       cnt++;
	       p++;
       }
       memcpy(name, buffer, cnt);
       name[cnt]='\0';
       if ((cnt+1)<strlen(buffer))
	       memcpy(value, buffer+cnt+1, strlen(buffer)-cnt-1);
       value[strlen(buffer)-cnt-1]='\0';
}

int LoadStrFromFile_helper(int fd, const char *name, char *value, BOOL ExitSign, int offset, int captial/*是否区分大小写 zw*/)
{
	char name1[128];
	char value1[VALUE_BUFFERLEN];
	char buffer[VALUE_BUFFERLEN];
	int size;
	int position;

	position=-1;
	lseek(fd, offset, SEEK_SET);
	while(TRUE) {
		if(ReadOneLine(fd, buffer, &size)) {
			SplitByChar(buffer, name1, value1, '=');
			if (captial && (strcmp(name1, name)==0)) {
				strcpy(value, value1);
				position = GetFileCurPos(fd);
				if (ExitSign) {
					break;
				}
			} else if(strcasecmp(name1, name)==0) {
				strcpy(value, value1);
				position = GetFileCurPos(fd);
				if (ExitSign) {
					break;
				}
			}
		} else {
			break;
		}
	}
	return position;
}

//return -1 mean can not find string by name
int LoadStrFromFile(int fd, const char *name, char *value, BOOL ExitSign, int offset)
{
	return LoadStrFromFile_helper(fd, name, value, ExitSign, offset, 1/*区分大小写*/);
}

int LoadCaseStrFromFile(int fd, const char *name, char *value, BOOL ExitSign, int offset)
{
	return LoadStrFromFile_helper(fd, name, value, ExitSign, offset, 0/*不区分大小写*/);
}

BOOL LoadCaseStr(const char *name, char *value)
{
	return (LoadCaseStrFromFile(fdOptions, name, value, FALSE, 0)!=-1?TRUE:FALSE);
}

BOOL LoadStr(const char *name, char *value)
{
	return (LoadStrFromFile(fdOptions, name, value, FALSE, 0)!=-1?TRUE:FALSE);
}

extern void SetDNServerAddress(unsigned char *DNServeraddress);
extern BOOL SetScreenTypeToFlash(const char *LCDType);
void ExecuteActionForOption(const char *name, const char *value)
{
	printf("param %s=%s\n", name, value);

	if (strcmp(name, "IPAddress")==0)
		SetIPAddress("IP", gOptions.IPAddress);
	else if (strcmp(name, "NetMask")==0)
		SetIPAddress("NETMASK", gOptions.NetMask);
	else if (strcmp(name, "GATEIPAddress")==0)
		SetGateway("add", gOptions.GATEIPAddress);
	else if (strcmp(name, "HiSpeedNet")==0)
		set_network_speed((unsigned char*)NET_DEVICE_NAME, gOptions.HiSpeedNet);
	else if (strcmp(name, "RS485On")==0)
	{
		if (gOptions.RS485On) RS485_setmode(FALSE);
	}
	//else if (strcmp(name, "~SerialNumber")==0)
	//	RedBootMac(value);
	else if (strcmp(name, "COMKey")==0)
		SetRootPwd(gOptions.ComKey);
        else if (strcmp(name, "MAC")==0)
                SaveMACToFlash(value);
        else if (strcmp(name, "DNS")==0)
                SetDNServerAddress(gOptions.DNS);
#ifdef ZEM600
	else if (strcmp(name, "Rote")==0) 
		SetScreenAddressToFlash(value);
	else if (strcmp(name, "LCDType")==0)
		SetScreenTypeToFlash(value);
#endif
}

BOOL SaveStr(const char *name, const char *value, int SaveTrue)
{
	char buffer[VALUE_BUFFERLEN];
	int len;

	len=strlen(value);
	if (LoadStr(name, buffer))
	{
		//the value is the same as old value, then return.
		if (0==strcmp(value, buffer))
		{
			printf("value is same:%s\n", value);
			return TRUE;
		}
	}
	//check language item whether can be setup or not
	if(!gOptions.MultiLanguage)
	{
		if(strcmp(name, "Language")==0) return TRUE;
	}
	CombineNameAndValue(name, value, SaveTrue, buffer);
	len=lseek(fdOptions, 0, SEEK_END);
	if (len>=MAX_OPTION_SIZE)
	{
	    ClearOptionItem("NONE");
	    len=lseek(fdOptions, 0, SEEK_END);
	}
	if (len<MAX_OPTION_SIZE)
	    write(fdOptions, buffer, strlen(buffer));
	fsync(fdOptions);

	ExecuteActionForOption(name, value);

	return ((len<MAX_OPTION_SIZE)?TRUE:FALSE);
}

BOOL RemoteSaveStr(const char *name, const char *value, int SaveTrue)
{
	char buffer[VALUE_BUFFERLEN];
	int len;

	len=strlen(value);
	if (LoadStr(name, buffer))
	{
		//the value is the same as old value, then return.
		if (0==strcmp(value, buffer)) return TRUE;
	}
	//check language item whether can be setup or not
	if(!gOptions.MultiLanguage)
	{
		if(strcmp(name, "Language")==0) return TRUE;
	}
	CombineNameAndValue(name, value, SaveTrue, buffer);
	len=lseek(fdOptions, 0, SEEK_END);
	if (len>=MAX_OPTION_SIZE)
	{
	    ClearOptionItem("NONE");
	    len=lseek(fdOptions, 0, SEEK_END);
	}
	if (len<MAX_OPTION_SIZE)
	    write(fdOptions, buffer, strlen(buffer));
	fsync(fdOptions);
	return ((len<MAX_OPTION_SIZE)?TRUE:FALSE);
}

char *GetCardKeyStr(char *Buffer, BYTE *Key)
{
	int i;
	BYTE *tmp=(BYTE *)Buffer;
        memcpy(tmp,Key,6);
        tmp[6]=0;
        for(i=5;i>=0;i--)
                if(tmp[i]==0xff) tmp[i]=0;
        return Buffer;
}

static char ln[40];

int LoadInteger(const char *Name, int DefaultValue)
{
	char tmp[VALUE_BUFFERLEN];
	char *buf;
	int v,n=1,d,c;

	buf=tmp;
	if(LoadStr(Name, buf))
	{
		if(*buf)
		{
			if('-'==*buf)
			{
				n=-1;
				buf++;
			}
			v=0;c=0;
			do{
				d=buf[c];
				if(d==0) break;
				if(d<'0' || d>'9')
				{
					return DefaultValue;
				}
				v=v*10+(d-'0');
				c++;
			}while(1);
			if(c)
				return n*v;
		}
	}
	return DefaultValue;
}

int SaveInteger(const char *Name, int Value)
{
	char Buf[20];
	sprintf(Buf,"%d",Value);
	if (SaveStr(Name, Buf, FALSE))
		return 0;
	else
		return 1;
}

TOptions gOptions;

char* SaveOptionItem(char *buf, const char *name, const char *value)
{
	char *p=buf;
	while(*name) *p++=*name++;
	*p++='=';
	while(*value) *p++=*value++;
	*p++=0;
	return p;
}

int SaveDefaultOptions(char *buffer)
{
	char *p=buffer;
	return p-buffer;
}

int InitOptions(void)
{
	char Buffer[80];
	int sel;

	memset(Buffer,0,sizeof(Buffer));
	GetEnvFilePath("USERDATAPATH", "options.cfg", Buffer);
	fdOptions=open(Buffer, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);

	memset(&gOptions, 0, sizeof(TOptions));
	LoadOptions(&gOptions);

	if(LoadStr("BCIIKeyLayouts", Buffer))
	{
		SetKeyLayouts(Buffer);
	}
#ifndef ZEM510
#ifndef ZEM600
	if(!gOptions.IsOnlyRFMachine)
	{
		if(gOptions.MaxUserCount>100) gOptions.MaxUserCount=100;
	}
#endif
#endif

#ifndef ZEM600
	if(gOptions.ScreenAddress == 0 || gOptions.ScreenAddress==1)
	{
		//printf("gOptions.ScreenAddress %d \n", gOptions.ScreenAddress);
		int lyy;
		lyy=SetScreenAddressToFlash();
		printf("screen address lyy %d \n", lyy);
		gOptions.ScreenAddress=2;
		SaveInteger("~ScreenAddress",gOptions.ScreenAddress);
	}
#endif

	if ((sel=LoadInteger("ChangeVersion",0))!=0)
	{
		SaveInteger("~ZKFPVersion", sel);
		gOptions.ZKFPVersion=sel;
		SaveInteger("ChangeVersion", 0);
	}
	if(gOptions.supportLCDType==1)			//3.5 support usbdisk
	{
		gOptions.IsSupportUSBDisk=1;
		SaveInteger("~USBDisk",gOptions.IsSupportUSBDisk);
	}
	gOptions.enableVideoRotate=1;

	return 1;
}

void TruncOptionAndSaveAs(char *buffer, int size)
{
	char tmp[80];

	memset(tmp,0,sizeof(tmp));
	GetEnvFilePath("USERDATAPATH", "options.cfg", tmp);

	if (fdOptions > 0) close(fdOptions);
    	fdOptions = open(tmp, O_RDWR|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	if (buffer!=NULL)
	{
	    write(fdOptions, buffer, size);
	    fsync(fdOptions);
	}
	close(fdOptions);
	fdOptions = open(tmp, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	//flush the cached data to disk
	sync();
}

static char gBuffer[VALUE_BUFFERCACHE+1];
static int gPosition=0;

char *strCache(char *value)
{
       char *p;
       int len;

       len=strlen(value);
       p=gBuffer;
       if ((gPosition+len)>=VALUE_BUFFERCACHE) gPosition=0;
       p+=gPosition;
       memcpy(p, value, len+1);
       gPosition+=len+1;
       return p;
}

char *LoadStrOld(const char *name)
{
       char tmp[VALUE_BUFFERLEN];

       return (LoadStr(name, tmp)?strCache(tmp):NULL);
}

//char* LoadStrByIDDef(int ID, const char *defStr)
char* LoadStrByIDDef(int ID, char *defStr)
{
	char *pChar=NULL;

#ifdef CHINESE_ONLY
	char Language[40];
	if(ID>=LngCachedStrIndexCnt)
	{
		memset(Language, 0, sizeof(Language));
		sprintf(Language, "%s%d", "ID=",ID);
		pChar=Language;
		return pChar;
	}
	if(getZKLibLanguageState())
	{
		unsigned short LngCached = LngCachedStrIndex[ID];
		if(LngCached)
			pChar = LngCachedStr[LngCached];
	}
	//else  pChar is NULL
	if(pChar==NULL)
	{
		memset(Language, 0, sizeof(Language));
		sprintf(Language, "%s%d", "ID=",ID);
		pChar=Language;
		printf("%s() language	  %s\n", __FUNCTION__, pChar);

	}
#else
	if(ID>=LngCachedStrIndexCnt)
	{
		pChar = LoadDefaultLanguageByID(ID);
		return pChar;
	}

	unsigned short LngCached = LngCachedStrIndex[ID];
	if(LngCached)
	{
		pChar = LngCachedStr[LngCached];
	}
	else
	{
		pChar = LoadDefaultLanguageByID(ID);
	}
#endif

	/*add the default choice,add by yangxiaolong,2011-7-26,start*/
	if ((defStr != NULL) && (strstr(pChar,"ID=") != NULL))
	{
		pChar = defStr;
	}
	/*add the default choice,add by yangxiaolong,2011-7-26,end*/

	return pChar;
}

char* LoadStrByID(int ID)
{
	return LoadStrByIDDef(ID, NULL);
}

char* LoadStrByIDPad(int ID, int Len)
{
	char *p;
	int i;

	p=LoadStrByID(ID);
	memset(ln,' ',Len); ln[Len]=0;
	if(p)
	{
		for(i=0;i<Len;i++)
		{
			if(p[i]==0) break;
			ln[i]=p[i];
		}
	}
	return ln;
}

char* GetYesNoName(int Yes)
{
	if(Yes) return LoadStrByID(HID_YES); else return LoadStrByID(HID_NO);
}

static char SMSBuf[100];

char *GetSMS(int UserID)
{
        int i, id;
        char *p;
        for(i=0;i<=100;i++)
        {
                sprintf(SMSBuf, "SMS%d", i);
                p=LoadStrOld(SMSBuf);
                if(p && *p)
                {
                        id=(Hex2Char(p)<<12)+(Hex2Char(p+1)<<8)+(Hex2Char(p+2)<<4)+Hex2Char(p+3);
                        if(UserID==id)
                        {
                                memset(SMSBuf, 0, 100);
                                return nstrcpy(SMSBuf, p+5, 100);
                        }
                }
        }
        return NULL;
}

int ClearAllACOpt(int All)
{
	int i;
	//clear group setting
	for(i=1;i<GP_MAX+1;i++)
	{
		if(FDB_GetGroup(i,NULL)!=NULL)
			if(FDB_DelGroup(i)==FDB_ERROR_IO)
				return FALSE;
	}

	if(All)
	{
		//clear time zone setting
		for(i=1;i<=TZ_MAX+1;i++)
		{
			if(FDB_GetTimeZone(i,NULL)!=NULL)
				if(FDB_DelTimeZone(i)==FDB_ERROR_IO)
					return FALSE;
		}
		//clear holiday setting
		for(i=1;i<=HTZ_MAX+1;i++)
		{
			if(FDB_GetTimeZone(i,NULL)!=NULL)
				if(FDB_DelHTimeZone(i)==FDB_ERROR_IO)
					return FALSE;
		}
		//clear lock group setting
		for(i=1;i<=CGP_MAX+1;i++)
		{
			if(FDB_GetCGroup(i,NULL)!=NULL)
				if(FDB_DelCGroup(i)==FDB_ERROR_IO)
					return FALSE;
		}

	}
	
	return TRUE;
}

int ClearOptionItem(char *name)
{
       int size, orisize;
       char *Buffer;

       size=lseek(fdOptions, 0, SEEK_END);
       Buffer=(char*)MALLOC(size);
       lseek(fdOptions, 0, SEEK_SET);
       if (read(fdOptions, Buffer, size)!=size)
       {
	       FREE(Buffer);
	       return FALSE;
       }
       orisize=size;

       size=PackStrBuffer(Buffer, name, size);

       if(orisize!=size)
       {
	       TruncOptionAndSaveAs(Buffer, size);
       }
       FREE(Buffer);
       return TRUE;
}

void RefreshCachedStr(int fd)
{
#ifdef CHINESE_ONLY
	//仅仅支持中文的时候需要，这个针对国内不支持多国语言的机器
	zk_initChineseLanguage_35(LngCachedStr, LngCachedStrIndex);
#else
	//针对正常支持多国语言的机器使用
	char buffer[LngCachedStrMaxLen]={0};
	char name[LngCachedStrMaxLen]={0};
	char value[LngCachedStrMaxLen]={0};

	int size=0;
	int iID=0;
	char *p=NULL;
	WORD index=0;

	memset(LngCachedStr, 0, LngCachedStrMaxLen*LngCachedStrCnt);
	memset(LngCachedStrIndex, 0, LngCachedStrIndexCnt*sizeof(WORD));

	lseek(fd, 0, SEEK_SET);
	while(TRUE)
	{
		if(ReadOneLine(fd, buffer, &size))
		{
			memset(name, 0, sizeof(name));
			memset(value, 0, sizeof(value));

			SplitByChar(buffer, name, value, '=');

			p=name;
			p[strlen(name)-1]='\0';
			while(TRUE)
			{
				if((*p>='0')&&(*p<='9'))
					break;
				else
					p++;
			}
			iID=atoi(p);
			if(iID>=LngCachedStrIndexCnt)
				break;
			strncpy(LngCachedStr[index], value, LngCachedStrMaxLen);
			LngCachedStrIndex[iID]=index;
			/*
			int count=0;
			printf("%d=", iID);
			for(count=0; count<10;count++)
			{
				printf("0x%02x, ", LngCachedStr[iID][count]&0xFF);
			}
			printf("\n");
			*/
			index++;
			if(index>=LngCachedStrCnt)
				break;
		}
		else
			break;
	}
	lseek(fd, 0, SEEK_SET);
#endif
}

//Language
void SelectLanguage(char Language)
{
	char buffer[128];
	char *tmp;

	if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
		sprintf(buffer, "%s%s.%c", tmp, "LANGUAGE", Language);
	else
		sprintf(buffer, "%s.%c", "LANGUAGE", Language);
	printf("load language path=%s\n", buffer);
	
	if (Language!=CurLanguage)
	{
		if (fdLanguage > 0) 
		{
			close(fdLanguage);
		}
		fdLanguage = open(buffer, O_RDONLY);
		CurLanguage = Language;
		RefreshCachedStr(fdLanguage);
	}
}

int GetLocaleID(int fd, int LngID)
{
	char *p, buf[]="E/_0_";
	char tmp[VALUE_BUFFERLEN];

	buf[0]=LngID;
	p=((LoadStrFromFile(fd, buf, tmp, FALSE, 0)!=-1)?strCache(tmp):NULL);
	if(p)
                return str2int(p,LID_INVALID);
	else
		return -2;
}

int GetDefaultLocaleID(void)
{
        return GetLocaleID(fdLanguage, gOptions.Language);
}

char *GetLangName(char LngID)
{
        char *p, buf[]="E/_0_";
	int fdTmp;
	char path[128];
	char value[VALUE_BUFFERLEN];
	char *tmp;

	buf[0]=LngID;
	//该资源的语言与当前的系统语言一致,则取本地化的语言名称，否则取英语名称
	if(CurLanguage==LngID)
	{
                buf[3]='1';
		p=((LoadStrFromFile(fdLanguage, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
		//English name
		if(p==NULL)
		{
			buf[3]='2';
			p=((LoadStrFromFile(fdLanguage, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
		}
	}
	else
	{
		buf[3]='2';
		if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
			sprintf(path, "%s%s.%c", tmp, "LANGUAGE", LngID);
		else
			sprintf(path, "%s.%c", "LANGUAGE", LngID);
		fdTmp=open(path, O_RDONLY);
		if(fdTmp==-1)
			p=NULL;
		else
		{
			p=((LoadStrFromFile(fdTmp, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
			if(p==NULL)
			{
				buf[3]='1';
				p=((LoadStrFromFile(fdTmp, buf, value, FALSE, 0)!=-1)?strCache(value):NULL);
			}
			close(fdTmp);
		}
	}
	return p;
}

int GetSupportedLang(int *LngID, int MaxLngCnt)
{
	DIR *dir;
        struct dirent *entry;
	char *filename;
	int LngCnt=0;
	char path[128];
	char *tmp;

	if ((tmp=LoadStrOld("FONTFILEPATH"))!=NULL)
		sprintf(path, "%s", tmp);
	else
		sprintf(path, "./");
        dir=opendir(path);
        if(dir)
        {
		while((LngCnt<MaxLngCnt)&&((entry=readdir(dir))!=NULL))
		{
			filename=entry->d_name;
			if((strlen(filename)==10)&&(strncmp(filename, "LANGUAGE.", 9)==0))
			{
				LngID[LngCnt++]=filename[9];
			}
		}
		closedir(dir);
		dir=0;
	}
        return LngCnt;
}

BOOL UpdateNetworkInfoByDHCP(char *dhcp)
{
	FILE *fp;
	char buffer[1024];
	char tmp[128];
	char *name, *value;
	int len, i;
	char OpName[128];
	BOOL bSign=FALSE;

	if((fp=fopen(dhcp, "rb"))==NULL) return FALSE;
	while(!feof(fp))
	{
		memset(buffer, 0, 1024);
		if(!fgets(buffer, 1024, fp)) break;
		i=0;
		name=buffer;
		value=NULL;
		while(buffer[i])
		{
			if(buffer[i]=='=')
			{
				buffer[i]='\0';
				value=buffer+i+1;
				//trunc the CR
				i=0;
				while(value[i])
				{
					if((value[i]=='\r')||(value[i]=='\n'))
					{
						value[i]='\0';
						break;
					}
					i++;
				}
				TrimRightStr(value);
				break;
			}
			i++;
		}
		//OK, we get a valid line
		if(value)
		{
			memset(OpName, 0, 128);
			if(strcmp(name, "ip")==0)
			{
				strcpy(OpName, "IPAddress");
				str2ip(value, gOptions.IPAddress);
			}
			else if(strcmp(name, "router")==0)
			{
				strcpy(OpName, "GATEIPAddress");
				str2ip(value, gOptions.GATEIPAddress);
			}
			else if(strcmp(name, "subnet")==0)
			{
				strcpy(OpName, "NetMask");
				str2ip(value, gOptions.NetMask);
			}
			else if(strcmp(name, "dns")==0)
			{
				strcpy(OpName, "DNS");;
				str2ip(value, gOptions.DNS);
			}
			if(OpName[0])
			{
				//Check OpName
				if(LoadStr(OpName, tmp))
				{
					//the value is the same as old value, then return.
					if (0==strcmp(value, tmp)) continue;
				}
				CombineNameAndValue(OpName, value, TRUE, tmp);
				len=lseek(fdOptions, 0, SEEK_END);
				if (len>=MAX_OPTION_SIZE)
				{
					ClearOptionItem("NONE");
					len=lseek(fdOptions, 0, SEEK_END);
				}
				if (len<MAX_OPTION_SIZE)
				{
					write(fdOptions, tmp, strlen(tmp));
					fsync(fdOptions);
				}
				bSign=TRUE;
			}
		}
	}
	fclose(fp);
	return bSign;
}

char * macformat(char *str, BYTE *value)
{
	sprintf(str,"%02X:%02X:%02X:%02X:%02X:%02X", value[0],value[1],value[2],value[3],value[4],value[5]);
	return str;
}

char * ipformat(char *str, BYTE *value)
{
	sprintf(str,"%d.%d.%d.%d",value[0],value[1],value[2],value[3]);
	return str;
}

TOptionsResStr OptionsResStr[]={
	//配置名称	长度	缺省值				是否需要恢复出厂设置
	{"MAC",		6,	{0x00,0x17,0x61,0x09,0x11,0x23},0,	optoffset(MAC),		str2mac,	macformat,	PARAM_ENCYPT},
	{"CardKey",	6,	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},0,	optoffset(CardKey),	str2cardkey,	GetCardKeyStr,	},
	{"IPAddress",	4,	{192,168,1,201},		1,	optoffset(IPAddress),	str2ip,		ipformat,	},
	{"GATEIPAddress",4,	{0,0,0,0},			1,	optoffset(GATEIPAddress),str2ip,	ipformat,	},
	{"NetMask",	4,	{255,255,255,0},		1,	optoffset(NetMask),	str2ip,		ipformat,	},
	{"AuthServerIP",4,	{0,0,0,0},			1,	optoffset(AuthServerIP),str2ip,		ipformat,	},
	{"WebServerIP",	4,	{0,0,0,0},			1,	optoffset(WebServerIP),	str2ip,		ipformat,	},
	{"TimeServerIP",4,	{0,0,0,0},			1,	optoffset(TimeServerIP),str2ip,		ipformat,	},
	{"ProxyServerIP",4,	{0,0,0,0},			1,	optoffset(ProxyServerIP),str2ip,	ipformat,	},
	{"DNS",		4,	{0,0,0,0},			1,	optoffset(DNS),		str2ip,		ipformat,	}
};

TOptionsResInt OptionsResInt[]={
	//配置名称		缺省值		是否需要恢复出厂设置		菜单项资源 		最大值		最小值
	{"~ML",			1,			0,	optoffset(MultiLanguage),		0,		1,	0,	0},
	{"Language",	LanguageSimplifiedChinese,	0,	optoffset(Language),		MID_OS_LANGUAGE,	255,	32+1,	0},
	{"DeviceID",		0x01,			1,	optoffset(DeviceID),			0,		255,	1,	0},
	{"MThreshold",		55,			1,	optoffset(MThreshold),							},
	{"EThreshold",		45,			0,	optoffset(EThreshold),							},
	{"VThreshold",		35,			1,	optoffset(VThreshold),							},
	{"LastAttLo",		0,			1,	optoffset(LastAttLog),							},
	{"UDPPort",		0x1112,			0,	optoffset(UDPPort),							},
	{"TCPPort",		0x1112,			0,	optoffset(TCPPort),							},
	{"RISServerTCPPort",	0x1110,			0,	optoffset(RISServerTCPPort),						},
	{"OImageWidth",		404,			0,	optoffset(OImageWidth),		0,		CMOS_WIDTH,	200, 	},
	{"OImageHeight",	300,			0,	optoffset(OImageHeight),	0,		CMOS_HEIGHT,	200, 	},
	{"OTopLine",		40,			0,	optoffset(OTopLine),		0,		CMOS_HEIGHT,	0, 	},
	{"OLeftLine",		144,			0,	optoffset(OLeftLine),		0,		CMOS_WIDTH,	0, 	},
	{"CPX0",		377,			0,	optoffset(CPX[0]), 							},
	{"CPX1",		28,			0,	optoffset(CPX[1]),							},
	{"CPX2",		424,			0,	optoffset(CPX[2]),							},
	{"CPX3",		-20,			0,	optoffset(CPX[3]),							},
	{"CPY0",		300,			0,	optoffset(CPY[0]),							},
	{"CPY1",		300,			0,	optoffset(CPY[1]),							},
	{"CPY2",		0,			0,	optoffset(CPY[2]),							},
	{"CPY3",		0,			0,	optoffset(CPY[3]),							},
	{"ZF_WIDTH",		276,			0,	optoffset(ZF_WIDTH),							},
	{"ZF_HEIGHT",		294,			0,	optoffset(ZF_HEIGHT),							},
	{"MSpeed",		MSPEED_AUTO,		0,	optoffset(MSpeed),							},
	{"AttState",		1,			1,	optoffset(AttState),							},
	{"~MaxUserCount",	30,			0,	optoffset(MaxUserCount),						},
	{"~MaxAttLogCount",	3,			0,	optoffset(MaxAttLogCount),	0,		0,		0,	PARAM_ENCYPT},
	{"~MaxFingerCount",	8,			0,	optoffset(MaxFingerCount),	0,		0,		0,	PARAM_ENCYPT},
	{"LockOn",		10,			1,	optoffset(LockOn),		0,		254,		0,	},
	{"AlarmAttLog",		99,			1,	optoffset(AlarmAttLog),							},
	{"AlarmOpLog",		99,			1,	optoffset(AlarmOpLog),							},
	{"AlarmReRec",		0,			1,	optoffset(AlarmReRec),							},
	{"RS232BaudRate",	115200,			1,	optoffset(RS232BaudRate),	0,		115200,		9600,	},
	{"RS232CRC",		0,			0,	optoffset(RS232CRC),							},
	{"RS232Stop",		1,			0,	optoffset(RS232Stop),							},
	{"WEBPort",		80,			0,	optoffset(WEBPort),							},
	{"~ShowState",		0,			0,	optoffset(ShowState),							},
	{"~KeyLayout",		KeyLayout_BioClockIII,	0,	optoffset(KeyLayout),							},
	{"VoiceOn",		1,			1,	optoffset(VoiceOn),		0,		1,		0,	},
	{"AutoPowerOff",	0xFFFF,			1,	optoffset(AutoPowerOff),	0,					},
	{"AutoPowerOn",		0xFFFF,			1,	optoffset(AutoPowerOn),		0,					},
	{"AutoPowerSuspend",	0xFFFF,			1,	optoffset(AutoPowerSuspend),	0,					},
	{"AutoAlarm1",		0xFFFF,			1,	optoffset(AutoAlarm[0]),						},
	{"AutoAlarm2",		0xFFFF,			1,	optoffset(AutoAlarm[1]),						},
	{"AutoAlarm3",		0xFFFF,			1,	optoffset(AutoAlarm[2]),						},
	{"AutoAlarm4",		0xFFFF,			1,	optoffset(AutoAlarm[3]),						},
	{"AutoAlarm5",		0xFFFF,			1,	optoffset(AutoAlarm[4]),						},
	{"AutoAlarm6",		0xFFFF,			1,	optoffset(AutoAlarm[5]),						},
	{"AutoAlarm7",		0xFFFF,			1,	optoffset(AutoAlarm[6]),						},
	{"AutoAlarm8",		0xFFFF,			1,	optoffset(AutoAlarm[7]),						},
        {"IdlePower",		HID_SUSPEND,		1,	optoffset(IdlePower),		0,		HID_SUSPEND,	0,	},
        {"IdleMinute",		0,			1,	optoffset(IdleMinute),		0,					},
        {"ShowScore",		0,			1,	optoffset(ShowScore),		0,		1,		0,	},
        {"NetworkOn",		1,			1,	optoffset(NetworkOn),	 	0,		1,		0,	},
        {"RS232On",		1,			1,	optoffset(RS232On),		0,		1,		0,	},
        {"RS485On",		0,			1,	optoffset(RS485On),		0,		1,		0,	},
	{"USB232On",		0,			1,	optoffset(USB232On),		0,		1,		0,	},
	{"USB232FunOn",		1,			0,	optoffset(USB232FunOn),		0,		1,		0,	},
	{"~NetworkFunOn",	1,			0,	optoffset(NetworkFunOn),	0,		1,		0,	PARAM_ENCYPT},
	{"~LockFunOn",		0,			0,	optoffset(LockFunOn),							},
	{"~RFCardOn",		0,			0,	optoffset(RFCardFunOn),		0,		1,		0,	PARAM_ENCYPT},
	{"~One2OneFunOn",	1,			0,	optoffset(One2OneFunOn),						},
	{"~PowerMngFunOn",	1,			0,	optoffset(PowerMngFunOn),	0,		1,		0,	},
	{"~NewFPReader",	0,			0,	optoffset(NewFPReader),							},
	{"~ShowName",		1,			0,	optoffset(ShowName),		0,		1,		0,	},
	{"UnlockPerson",	1,			1,	optoffset(UnlockPerson),						},
	{"ShowCheckIn",		0,			0,	optoffset(ShowCheckIn),							},
	{"OnlyPINCard",		0,			0,	optoffset(OnlyPINCard),		0,		1,		0,	},
	{"~IsTestMachine",	0,			1,	optoffset(IsTestMachine),						},
	{"~MustChoiceInOut",	0,			1,	optoffset(MustChoiceInOut),						},
	{"HiSpeedNet",		2,			1,	optoffset(HiSpeedNet),							},
	{"~MenuStyle",		1,			0,	optoffset(MenuStyle),							},
	{"CCCKey",		1,			0,	optoffset(CanChangeCardKey),						},
	{"Must1To1",		0,			1,	optoffset(Must1To1),		0,		1,		0,	},
	{"LCDM",		0,			0,	optoffset(LCDModify),							},
	{"COMKey",		0,			1,	optoffset(ComKey),							},
	{"MustEnroll",		1,			1,	optoffset(MustEnroll),		0,					},
	{"TOMenu",		60,			0,	optoffset(TimeOutMenu),		0,		65535*32768,	10,	},
	{"TOPin",		10,			0,	optoffset(TimeOutPin),		0,		65535,		5,	},
	{"TOState",		10,			0,	optoffset(TimeOutState),						},
	{"SaveAttLog",		1,			0,	optoffset(SaveAttLog),							},
	{"RS232Fun",		1,			0,	optoffset(RS232Fun),							},
        {"~IsModule",		0,			0,	optoffset(IsModule),							},
	{"~ShowSecond",		0,			0,	optoffset(ShowSecond),							},
	{"~RFSStart",		0,			0,	optoffset(RFCardSecStart),						},
	{"~RFSLen",		10,			0,	optoffset(RFCardSecLen),						},
	{"~RFFPC",		1,			0,	optoffset(RFCardFPC),							},
	{"~PIN2Width",		1,			0,	optoffset(PIN2Width),		0,		22,		1,	PARAM_ENCYPT},
	{"DtFmt",		0,			1,	optoffset(DateFormat),							},
	{"~OPLM1",		-1,			0,	optoffset(OPLogMask1),							},
	{"~OPLM2",		0,			0,	optoffset(OPLogMask2),							},
	{"AS1",			-1,			1,	optoffset(AutoState[0]),						},
	{"AS2",			-1,			1,	optoffset(AutoState[1]),						},
	{"AS3",			-1,			1,	optoffset(AutoState[2]),						},
	{"AS4",			-1,			1,	optoffset(AutoState[3]),						},
	{"AS5",			-1,			1,	optoffset(AutoState[4]),						},
	{"AS6",			-1,			1,	optoffset(AutoState[5]),						},
	{"AS7",			-1,			1,	optoffset(AutoState[6]),						},
	{"AS8",			-1,			1,	optoffset(AutoState[7]),						},
	{"AS9",			-1,			1,	optoffset(AutoState[8]),						},
	{"AS10",		-1,			1,	optoffset(AutoState[9]),						},
	{"AS11",		-1,			1,	optoffset(AutoState[10]),						},
	{"AS12",		-1,			1,	optoffset(AutoState[11]),						},
	{"AS13",		-1,			1,	optoffset(AutoState[12]),						},
	{"AS14",		-1,			1,	optoffset(AutoState[13]),						},
	{"AS15",		-1,			1,	optoffset(AutoState[14]),						},
	{"AS16",		-1,			1,	optoffset(AutoState[15]),						},
	{"~DC",			3,			0,	optoffset(DelayCount),							},
	{"~IncThr",		14,			0,	optoffset(IncThr),							},
	{"~TopThr",		50,			0,	optoffset(TopThr),							},
	{"~MinThr",		30,			0,	optoffset(MinThr),							},
	{"NoiseThreshold",	100,			0,	optoffset(MaxNoiseThr),							},
	{"~MinM",		12,			0,	optoffset(MinMinutiae),							},
	{"~MaxTL",		MAXVALIDTMPSIZE,	0,	optoffset(MaxTempLen),							},
	{"AdminCnt",		1,			0,	optoffset(AdminCnt),							},
	{"ODD",			10,			1,	optoffset(OpenDoorDelay),						},
	{"DSM",			2,			1,	optoffset(DoorSensorMode),						},
	{"ECnt",		3,			0,	optoffset(EnrollCount),		0,		10,		3,	},
	{"~AAFO",		0,			0,	optoffset(AutoAlarmFunOn),	0,		0,		0,	PARAM_ENCYPT},
	{"~ASFO",		0,			0,	optoffset(AutoStateFunOn),						},
	{"DUHK",		0,			1,	optoffset(DuressHelpKeyOn),	MID_AD_DURESSHELP,1,		0,	},
	{"DU11",		0,			1,	optoffset(Duress1To1),		MID_AD_DURESS11,1,		0,	},
	{"DU1N",		0,			1,	optoffset(Duress1ToN),		MID_AD_DURESS1N,1,		0,	},
	{"DUPWD",		0,			1,	optoffset(DuressPwd),		MID_AD_DURESSPWD,1,		0,	},
	{"DUAD",		10,			1,	optoffset(DuressAlarmDelay),						},
	{"LockPWRButton",	0,			1,	optoffset(LockPowerButton),	0,		1,		0,	},
	{"SUN",			3,			0,	optoffset(StartUpNotify),						},
//	{"I1NFrom",		0,			1,	optoffset(I1ToNFrom),							},
//	{"I1NTo",		0,			1,	optoffset(I1ToNTo),							},
//	{"I1H",			0,			1,	optoffset(I1ToH),		MID_OS_1TOH	,1		,0	},
//	{"I1G",			0,			1,	optoffset(I1ToG),		MID_OS_1TOG	,1		,0	},
	{"~MaxUserFingerCount",	10,			0,	optoffset(MaxUserFingerCount),						},
	{"~MIFARE",		0,			0,	optoffset(IsSupportMF),		0,		0,		0,	PARAM_ENCYPT},
	{"~FlashLed",		1,			0,	optoffset(IsFlashLed),		0,		10,		0,	},
	{"~IsInit",		1,			0,	optoffset(IsInit),							},
	{"CMOSGC",		0,			0,	optoffset(CMOSGC),		0,		255,		0,	},
	{"~ADMATCH",		0,			0,	optoffset(AdvanceMatch),						},
	{"ERRTimes",		3,			1,	optoffset(ErrTimes),		0,		255,		0,	},
	{"~IsOnlyOneSensor",	1,			1,	optoffset(IsOnlyOneSensor),	0,		1,		0,	},
	{"AuthServerEnabled",	0,			1,	optoffset(AuthServerEnabled),						},
        {"AuthServerREQ",       3,                      1,      optoffset(AuthServerREQ),       0,              0,              0,	},
        {"AuthServerCheckMode", 0,                      1,      optoffset(AuthServerCheckMode), 0,              2,              0,	},
        {"~ASDewarpedImage",0,                          0,      optoffset(ASDewarpedImage),     0,              1,              0,	PARAM_ENCYPT},
	{"DNSCheckTime",	0,			1,	optoffset(DNSCheckInterval),						},
	{"AutoUPLogTime",	0,			1,	optoffset(AutoUploadAttlog),						},
	{"DisableUser",		0,			1,	optoffset(DisableNormalUser),	0,		1,		0,	},
	{"KeyPadBeep",		1,			1,	optoffset(KeyPadBeep),		0, 		1,		0,	},
	{"WorkCode",		0,			0,	optoffset(WorkCode),		0,		1,		0,	},
	{"MustChoiceWorkCode",	0,			0,	optoffset(MustChoiceWorkCode),	0,		1,		0,	},
        {"~MCWKCD",             0,                      0,      optoffset(MustCheckWorkCode),   0,              1,              0,	},
#ifdef ZEM300
	{"VOLUME",		67,			1,	optoffset(AudioVol), 		0,		99,		1, 	},
	{"AAVOLUME",		67,			1,	optoffset(AutoAlarmAudioVol),	0,		99,		1, 	},
#else
	{"VOLUME",		34,			1,	optoffset(AudioVol), 		0,		99,		1,	},
	{"AAVOLUME",		34,			1,	optoffset(AutoAlarmAudioVol),	0,		99,		1,	},
#endif
	{"DHCP",		0,			1,	optoffset(DHCP),		0,		1,		0,	},
	{"AutoSyncTime",	0xFFFF,			1,	optoffset(AutoSyncTime),						},
	{"~IsOnlyRFMachine",	0,			0,	optoffset(IsOnlyRFMachine),	0,		1, 		0,	},
	{"~OS",			1,			0,	optoffset(OS),			0,		255, 		0, 	},
	{"~IsWiegandKeyPad",	0,			0,	optoffset(IsWiegandKeyPad),	0,		1, 		0,	},
	{"~SMS",		0,			0,	optoffset(IsSupportSMS),	0,		1, 		0,	},
	{"~USBDisk",		1,			0,	optoffset(IsSupportUSBDisk),	0,		1, 		0,	},
	{"ModemEnable",	0,			0,	optoffset(ModemEnable),		0,		1, 		0,	},
	{"ModemModule",	1,			0,	optoffset(ModemModule),		0,		4, 		0,	},
	{"~MODEM",		0,			0,	optoffset(IsSupportModem),	0,		3, 		0,	},
	{"~AuthServer",		0,			0,	optoffset(IsSupportAuthServer),	0,		1, 		0,	PARAM_ENCYPT},
	{"~ACWiegand",		0,			0,	optoffset(IsACWiegand),		0,		1, 		0,	},
	{"~ExtendFmt",		0,			0,	optoffset(AttLogExtendFormat),	0,		1, 		0,	PARAM_ENCYPT},
	{"~DRPass",		0,			0,	optoffset(DisableRootPassword),	0,		1, 		0,	},
	{"~MP3",		0,			0,	optoffset(IsSupportMP3),	0,		1, 		0,	},
	{"~MIFAREID",		0,			0,	optoffset(MifareAsIDCard),	0,		1, 		0,	PARAM_ENCYPT},
	{"~GroupVoice",		0,			0,	optoffset(PlayGroupVoice),	0,		1, 		0,	},
	{"~TZVoice",		0,			0,	optoffset(PlayTZVoice),		0,		1, 		0,	},
	{"~ASTFO",		0,			0,	optoffset(AutoSyncTimeFunOn),	0,		1, 		0,	},
	{"~CFO",		0,			0,	optoffset(CameraFunOn),		0,		1, 		0,	},
	{"~SaveBitmap",		0,			0,	optoffset(SaveBitmap),		0,		1, 		0,	},
	{"~ProcessImage",	0,			0,	optoffset(ProcessImage),	0,		1, 		0, 	},
	{"ASTimeOut",		10,			0,	optoffset(AuthServerTimeOut),	0,		30, 		0,	},
	{"~TLLCM",		0,			0,	optoffset(TwoLineLCM),		0,		1, 		0,	},
	{"~UserExtFmt",		0,			0,	optoffset(UserExtendFormat),	0,		1, 		0, 	PARAM_ENCYPT},
	{"RefreshUserData",	0,			1,	optoffset(RefreshUserData),						},
	{"~DisableAU",		0,			0,	optoffset(DisableAdminUser),						},
	{"~ICFO",		0,			0,	optoffset(IClockFunOn),							},
	{"ProxyServerPort",	0,			1,	optoffset(ProxyServerPort),						},
	{"WebServerPort",	0,			1,	optoffset(WebServerPort),						},
	{"~C2",			0,			0,	optoffset(IsSupportC2),							},
	{"EnableProxyServer",	0,			1,	optoffset(EnableProxyServer),						},
	{"~WCFO",		0,			0,	optoffset(WorkCodeFunOn),	                                        },
        {"~VALF",               0,                      0,      optoffset(ViewAttlogFunOn),     0,              10,             0,	},
	{"~SSR",                0,                      0,      optoffset(SSRFunOn),		0,		10,		0,	},
	{"HzImeOn",		0,			0,	optoffset(HzImeOn),		0,		1,		0,	},
	{"PinPreAdd0",          1,                      0,     	optoffset(PinPreAdd0),		0,		1,		0,	},
	{"~HID",		0,			0,	optoffset(IsSupportHID),        0,      	1,             	0,	},
	{"FPRetry",		3,			1,	optoffset(FPRetry),		0,		9,		0,	},
	{"AdmRetry",		3,			1,	optoffset(AdmRetry),		0,		9,		0,	},
	{"PwdRetry",		3,			1,	optoffset(PwdRetry),		0,		9,		0,	},
	{"InterStyle",		1,			1,	optoffset(InterStyle),		0,		2,		0,	},
	{"LogoTime",		10,			1,	optoffset(LogoTime),		0,		999,		0,	},
	{"SMSTime",		30,			1,	optoffset(SMSTime),		10,		999,		0,	},
	{"ClockTime",		30,			1,	optoffset(ClockTime),		0,		999,		0,	},
//alarm:
	{"AATimes",		2,			1,	optoffset(AutoAlarmTimes),		 				},
	{"isSupportAlarmExt",	0,			0,	optoffset(isSupportAlarmExt),	0,		1,		0,	PARAM_ENCYPT},
	{"~STKCT",		1,			0,	optoffset(ShortKeyCount),	0,		15,		1,	},
	{"AADelay",		10,			1,	optoffset(AutoAlarmDelay),		 				},
	{"AAMaxCount",		10,			0,	optoffset(AlarmMaxCount),	0,		99,		0,	},
	{"SMSCount", 		100,			0, 	optoffset(SMSCount),		0,		1000,		0,	},
	{"TFTKeyLayout",	0,			0,	optoffset(TFTKeyLayout),	0,		100,		0,	},
	{"IMEFunOn",		0,			0,	optoffset(IMEFunOn),		0,		1,		0,	PARAM_ENCYPT},
	{"Brightness",		80,			1,	optoffset(Brightness),		0,		100,		0,	},
	{"LcdMode",		0,			0,	optoffset(LcdMode),		0,		1,		0,	},
	{"DoorSensorTimeout",   30,                     1,      optoffset(DoorSensorTimeout),   0,              99,             0,	},
	{"~DCTZ",		0,			1,	optoffset(DoorCloseTimeZone),	0,		50,		0,	},
	{"~DOTZ",		0,			1,	optoffset(DoorOpenTimeZone),	0,		50,		0,	},
	{"IsHolidayValid",	0,			0,	optoffset(IsHolidayValid),	0,		1,		0,	},
	{"~iCLASSID",		0,			0,	optoffset(iCLASSAsIDCard),	0,		1,		0,	},
	{"~iCLASS",		0,			0,	optoffset(IsSupportiCLSRW),	0,		3,		0,	},
	{"~iCLASSTYPE",		0,			0,	optoffset(iCLASSCardType),	0,		5,		0,	},
        {"~APBFO",              0,                      0,      optoffset(AntiPassbackFunOn),	0,		1,		0,	},
        {"AntiPassbackOn",      0,                      1,      optoffset(AntiPassbackOn),	0,		3,		0,	},
        {"MasterState",         0,                      1,      optoffset(MasterState),		0,		1,		-1,	},
        {"WiegandID",           0,                      0,      optoffset(WiegandID),							},
        {"~ExtWGInFunOn",	0,                      0,      optoffset(ExtWGInFunOn),  						},
        {"WiegandOutType",	0,                      0,      optoffset(WiegandOutType),	0,		1,		0,	},
        {"WiegandInType",	0,                      0,      optoffset(WiegandInType),	0,		1,		0,	},
        {"WIFI",                0,                      0,      optoffset(IsSupportWIFI),       0,  		1,              0,	},
        {"wifidhcp",            0,                      0,      optoffset(wifidhcpfunon),       0,              1,              0,	},
        {"~PrinterFunOn",       0,             		0,      optoffset(PrinterFunOn),        0,              1,		0,	PARAM_ENCYPT},
        {"~SASBTZ",    		0,      		0,      optoffset(SwitchAttStateByTimeZone),    0,      1,     		0,	},
        {"PrinterOn",           0,              	1,      optoffset(PrinterOn),           0,          	10,             0,	},
        {"RedialTimeInterval",	60,                      1,      optoffset(RedialTimeInterval),  0,		0,              0,	},
        {"RedialCount",		0,                      1,      optoffset(RedialCount), 	0,              0,              0,	},
//	{"GPRSFrequency",	0,			0,	optoffset(GPRSFrequency),	0,		6,		0	},
//      {"isgprstest",		0,                      1,      optoffset(isgprstest), 	 	0,              1,              0       },
        {"ReconnectInterval",  45,     	                1,      optoffset(ReconnectInterval),   0,              0,              0,	},
	{"AuthServerIPType", 	0,			0,	optoffset(AuthServerIPType), 	0,		1,		0,	},
	{"ShowPhoto",		0,			0,	optoffset(ShowPhoto),		0, 		1,		0,	},
	{"PhotoFunOn",		0,			0,	optoffset(IsSupportPhoto),	0,		1,		0,	PARAM_ENCYPT},
	{"CameraOpen",		0,        		0,      optoffset(CameraOpen),		1,		0,		0,	},
        {"CameraBright",	50,     		1,      optoffset(CameraBright),	100,		0,			},
        {"CameraContrast",	50,   			1,      optoffset(CameraContrast),	100,		0,			},
        {"PicQuality",		1,        		1,      optoffset(PicQuality),		3,		0,			},
        {"CameraScene",		0,       		1,      optoffset(CameraScene),		1,		0,			},
        {"CapturePic",		0,        		0,	optoffset(CapturePic),		10,		0,			},
        {"CapturevfTimes",	3,    			0,      optoffset(CapturevfTimes),	20,		0,			},
        {"CaptureAlarm",	20,     		0,      optoffset(CaptureAlarm),	100,		0,			},
	{"IMESwitchKey",	0,			0,	optoffset(IMESwitchKey),	0,		0,		0,	},
	{"~NSRI",		0,			0,	optoffset(NotShowRecordInfo),	0,		1,		0,	},
        {"~DSTF", 		0,			0, 	optoffset(DaylightSavingTimeFun),0,					},
        {"DaylightSavingTimeOn",0,			1,	optoffset(DaylightSavingTimeOn),1,		0,			},
        {"CurTimeMode",		0,			1,	optoffset(CurTimeMode),		0,		2,		0,	},
        {"DaylightSavingTime",	0,			1,	optoffset(DaylightSavingTime),						},
        {"StandardTime",	0,			1,	optoffset(StandardTime),						},
	{"FpSelect",		2,			1,	optoffset(FpSelect),		0,		0,		0,	},
	{"~NSRI",		0,			0,	optoffset(NotShowRecordInfo),	0,		1,		0,	},
	{"AlwaysShowState",	0,			0,	optoffset(AlwaysShowState),	0,		1,		0,	},
        {"AttUseTZ",		0,			0,	optoffset(AttUseTZ),		0,		1,		0,	},
	{"MachineGroup",	1,			0,	optoffset(MachineGroup),	0,		0,		0,	},
	{"HasFPThreshold",      300,    		0,      optoffset(HasFPThreshold),      0,      	0,      	0,	},
	{"NoFPThreshold",       280,    		0,      optoffset(NoFPThreshold),       0,      	0,      	0,	},
	{"UseNewBell",      	0,    			0,      optoffset(UseNewBell),     	1,      	0,      	0,	},
	{"HavePowerKey",	0,			0,	optoffset(HavePowerKey),	1,		0,		0,	},
	{"LockPowerKey",	0,			0,	optoffset(LockPowerKey),	1,		0,		0,	},
	{"LimitFpCount",        3000,                   0,      optoffset(LimitFpCount),        0,              0,              0,	},
	{"ExAlarmDelay",	60,			0,	optoffset(ExAlarmDelay),	0,		0,		0,	},
	{"DelRecord",		0,			0,	optoffset(DelRecord),		0,		0,		0,	},
	{"ApiPort",		8000,			0,	optoffset(ApiPort),		0,		65533,		20,	},
        {"RSize",		0,			0,	optoffset(RSize),		0,		0,		0,	},
        {"ShowStyle",		0,			0,	optoffset(ShowStyle),		0,		0,		0,	},
	//zsliu add     user Hejira Calendar's flag 判断是否使用伊朗日历
        {"HejiraCalendar",	0,			0,	optoffset(isUseHejiraCalendar),	0,		0,		0,	},

        {"IrFunOn",		0,			0,	optoffset(IR_SensorOn),		0,		0,		255,	},
        {"IrBLS",		1,			0,	optoffset(IRSS_BLSwitch),	0,		0,		1,	},
        {"IrBLOff",		0,			0,	optoffset(IRSS_BLOffR),		0,		0,		255,	},
        {"IrOn",		128,			0,	optoffset(IRSS_BLOnR),		0,		0,		255,	},
        {"IrBLDT",		30,			0,	optoffset(IRSS_BLDlyT),		0,		0,		255,	},
        {"IrAuto",		63,			0,	optoffset(IRSS_AutoCon),	0,		0,		255,	},
        {"IrRange",		32,			0,	optoffset(IRSS_Range),		0,		0,		255,	},

	{"BatteryInfo",		0,			0,	optoffset(BatteryInfo),		0,		1,		0,	},
	{"~ScreenAddress",	2,			0,	optoffset(ScreenAddress),	0,		2,		0,	},
	{"~isRotateCamera",	0,			0,	optoffset(isRotateCamera),	0,		2,		0,	},
	{"~ExtBell",		0,			0,	optoffset(IsSupportExtBell),	0,		1,		0,	},
	{"DetectFpAlg",		1,			0,	optoffset(DetectFpAlg),		0,		1,		0,	},
	{"~ZKFPVersion",	9,			0,	optoffset(ZKFPVersion),							},
	{"FingerSensitivity",	1,			0,	optoffset(FingerSensitivity),						},
	{"~FPRotation",		0,			0,	optoffset(FPRotation),		0,		180,		0,	},

	{"~EU",			0,			0,	optoffset(EuropeDevice),	0,		2,		0,	},//add by mjh
	{"RotateDev",		0,			0,	optoffset(RotateDev),		0,		1,		0,	},
	//{"UploadPhoto",     0,  0,  optoffset(UploadPhoto),    0,   1,  0       }, //seiya china 
	{"~MulAlgVer",		0,			0,	optoffset(MulAlgVer),		0,		1,		0,	},
	{"FP10DBVersion",	0,			0,	optoffset(FP10DBVersion),	0,		255,		0,	},
	{"~LCDType",		0,			0,	optoffset(supportLCDType),	0,		0,		0,	},
	{"keyboardStyle",	0,			0,	optoffset(keyboardStyle),	0,		0,		0,	},
	//ADMS

	{"IclockSvrFun",	0,			0,	optoffset(IclockSvrFun),	0,		0,		0,	PARAM_ENCYPT},

	{"WriteLCDDelay",	30,			0,	optoffset(WriteLCDDelay),	0,		0,		0,	},//add by zhc 2008.11.25

	//GPRS RADIO Function
	{"EchoFailRestart",	4,			1,	optoffset(EchoFailRestart),	0,		99,		0,	},
	{"EchoInterval",	60,			1,	optoffset(EchoInterval),	0,		600,		0,	},
	{"EchoAction",		1,			1,	optoffset(EchoAction),		0,		5,		0,	},
	{"EchoType",		1,			1,	optoffset(EchoType),		0,		5,		0,	},
	{"~TTSIPO",		1,			1,	optoffset(ttsIntegralPointOpen),0,		1,		0,	},
	{"WGPassThrough",	0,			0,	optoffset(WGPassThrough),	0,		0,		0,	},
	{"FreeTime",		30,			1,	optoffset(FreeTime),		0,		0,		0,	},
	{"EnableCommEndBeep",	1,			0,	optoffset(EnableCommEndBeep),	0,		0,		0,	},
	{"maxCaptureCount",	8000,			0,	optoffset(maxCaptureCount),	0,		0,		0,	},
	{"sensorOffsetY",	10,			0,	optoffset(sensorOffsetY),	0,		50,		0,	},
	//add by caona for face
        {"FaceFunOn",		0,			0,	optoffset(FaceFunOn),		0,		1,		0,	},
        {"~MaxFaceCount",	7,			1,	optoffset(MaxFaceCount),	0,		0xffff,		0,	},
        {"~FaceVThr",		70,			1,	optoffset(FaceVThreshold),	0,		120,		55,	},
        {"~FaceMThr",		80,			1,	optoffset(FaceMThreshold),	0,		120,		65,	},
        {"~DefFaceGroup",	1,			1,	optoffset(DefFaceGroup),	0,		99,		1,	},
        {"~FaceRegMode",	1,			1,	optoffset(FaceRegMode),		0,		1,		0,	},

        {"~FaceExposure",	320,			1,	optoffset(FaceExposoure),	0,		0xFFFF,		80,	},
        {"~FaceExposRang",	50,			1,	optoffset(FaceExposRang),	0,		1000,		0,	},
        {"~VideoGain",		100,			1,	optoffset(VideoGain),		0,		0xFF,		30,	},
        {"~VideQuality",	80,			1,	optoffset(VideQuality),		0,		0xFF,		30,	},

	{"KeyType",		0,			0,	optoffset(KeyType),		0,		0,		0,	},
	{"IsSupportDNS",	1,			0,	optoffset(IsSupportDNS),	0,		0,		0,	PARAM_ENCYPT},
	{"~RS232FO",		1,			0,	optoffset(RS232FunOn),		0,		0,		0,	},
	{"~RS485FO",		1,			0,	optoffset(RS485FunOn),		0,		0,		0,	},
	{"WebServerURLModel",	0,			0,	optoffset(WebServerURLModel),	0,		0,		0,	},
	{"IsSupportFlowCtrl",	0,			0,	optoffset(IsSupportFlowCtrl),	0,		0,		0,	},
	{"IsUploadPhotoOnly",     0,  0,  optoffset(IsUploadPhotoOnly),    0,   1,  0       }, //seiya china
	{"IsRotatePhoto",	1,			0,	optoffset(IsRotatePhoto),	0,		0,		0,	},
	/*异地考勤,add by yangxiaolong,2011-06-14,start*/
	{"RemoteAttFunOn",		0,		0,	optoffset(RemoteAttFunOn),	0,	1,			0,	},
	{"RmUserSaveTime",		5,		0,	optoffset(RmUserSaveTime),		0,	0xffff,		1,	},
	/*异地考勤,add by yangxiaolong,2011-06-14,end*/
	{"USBCheckTime",	5,			0,	optoffset(USBCheckTime),	0,		0xFF,		0,	0},
	{"USBFileCount",	3,			0,	optoffset(USBFileCount),	0,		0xFF,		0,	0},
	{"USBCheckOn",		0,			0,	optoffset(USBCheckOn),		0,		0xFF,		0,	0},
	{"~PushLockFunOn",	0,			0,	optoffset(PushLockFunOn),	0,		1,		0,	0},
	{"DownloadNewLog",	0,			0,	optoffset(DownloadNewLog),	0,		1,		0,	0},
	/*RS485 Reader*/
	{"~RS485Port",		1,			0,	optoffset(RS485Port),		0,		255,		0,	0},
	/*modified by zxz if the value of Reader485FunOn is 1, cannot changed algversion*/
	{"Reader485FunOn",	0,			0,	optoffset(Reader485FunOn),	1,		0,		0,	0},
	{"InBIOComType",	3,			0,	optoffset(InBIOComType),	0,		255,		0,	0},
	{"Reader485On",		1,			0,	optoffset(Reader485On),		1,		0,		0,	0},
	{"ZEM800Platform",	1,			0,	optoffset(ZEM800Platform),	1,		0,		0,	0},
	{"VideoWidth",		320,			0,	optoffset(VideoWidth),		1,		0,		0,	0},
	{"VideoHeight",		240,			0,	optoffset(VideoHeight),		1,		0,		0,	0},
	{"WifiModule",		1,			0,	optoffset(WifiModule),		1,		0,		0,	0},
	{"ISLR",		0,			0,	optoffset(LoopWriteRecordOn),	1,		0,		0,	0},
	{"BitsInvertCardModule",0,                      0,      optoffset(BitsInvertCardModule),1,              0,              0,      0},
	{"CardNumberParity",	0,                      0,      optoffset(CardNumberParity),   	1,              0,              0,      0},
	{"IDCardStyle",		0,                      0,      optoffset(IDCardStyle),		1,              0,              0,      0},
	{"StartPosition",	0,                      0,      optoffset(StartPosition),   	1,              0,              0,      0},
	{"CardNumberLen",	0,                      0,      optoffset(CardNumberLen),	1,              0,              0,      0},
	{"DelPictureCnt",	20,                     0,      optoffset(DelPictureCnt),	1,              0,              0,      0},
	{"IsWGSRBFunOn",	0,		0,		optoffset(IsWGSRBFunOn),		0,		1,		0},
	{"SRBOn",			0,		0,		optoffset(SRBOn),				0,		1,		0},
	{"SRBType",			0,		0,		optoffset(SRBType),				0,		1,		0},
	{"FingerMaxRotation",		180,		0,		optoffset(FingerMaxRotation),				0,		180,		0},
	{"RegistOpenFlag",		0,			1,	optoffset(RegistOpenFlag),		1,		0,		0,	0},
	{"ReadCardInterval",		0,			1,	optoffset(ReadCardInterval),		1,		0,		0,	0},
};

POptionsResInt QueryOptResByOffset(int Offset)
{
	int i;
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		if(OptionsResInt[i].Offset==Offset)
			return OptionsResInt+i;
	}
	return NULL;
}

POptions GetDefaultOptions(POptions opts)
{
	int i=0;
	//Get common default value
	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		if(OptionsResStr[i].IsNeedRestoreFactory)
			memcpy(((char*)opts)+OptionsResStr[i].Offset, OptionsResStr[i].DefaultValue, OptionsResStr[i].OptionLong);
	}

	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		if(OptionsResInt[i].IsNeedRestoreFactory)
			memcpy(((char*)opts)+OptionsResInt[i].Offset, &(OptionsResInt[i].DefaultValue), 4);
	}

#ifdef OEM_CMI
	opts->MaxNoiseThr=124;
	opts->RS232On =0;
	opts->RS485On =0;
#endif
	//special options
	if(LOCKFUN_ADV & LoadInteger("~LockFunOn",0))
	{
		opts->MThreshold=65;
		opts->VThreshold=55;
	}
	opts->Saved =1;
	/*add by zxz 2013-01-10 for gprs*/
	if (gOptions.IsSupportModem && (LoadInteger("ModemModule", 0) == 1)) {
		opts->RS232CRC = 0;
		opts->RS232Fun = 0;
		opts->RS232FunOn = 0;
		opts->RS232On = 0;
		opts->RS485FunOn = 0;
		opts->RS485On = 0;
		opts->USB232FunOn = 0;
		opts->USB232On = 0;
	}
	return opts;
}

POptions LoadOptions(POptions opts)
{
	int i;
#ifndef URU
	static BOOL LoadSign=FALSE;
#endif
	char name1[128], value1[VALUE_BUFFERLEN];
	char buffer[VALUE_BUFFERLEN];
	int size;
	BOOL exitsign;
	int value;

	//setting default value
	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		memcpy(((char*)opts)+OptionsResStr[i].Offset, OptionsResStr[i].DefaultValue, OptionsResStr[i].OptionLong);
	}
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		memcpy(((char*)opts)+OptionsResInt[i].Offset, &(OptionsResInt[i].DefaultValue), 4);
	}

	//Read option from options.cfg
	lseek(fdOptions, 0, SEEK_SET);
	while(TRUE)
	{
		if(ReadOneLine(fdOptions, buffer, &size))
		{

			exitsign=FALSE;
			SplitByChar(buffer, name1, value1, '=');

			for(i=0;i<OPTIONSRESSTRCOUNT;i++)
			{
				if(strcmp(name1, OptionsResStr[i].OptionName)==0)
				{
					if(OptionsResStr[i].Convertor)
						OptionsResStr[i].Convertor(value1, ((BYTE*)opts)+OptionsResStr[i].Offset);
					else
						strcpy(((char*)opts)+OptionsResStr[i].Offset, value1);
					exitsign=TRUE;
					break;
				}
			}


			if(!exitsign)
			{
				for(i=0;i<OPTIONSRESINTCOUNT;i++)
				{
					if(strcmp(name1, OptionsResInt[i].OptionName)==0)
					{
						value=str2int(value1, OptionsResInt[i].DefaultValue);
						if(OptionsResInt[i].MaxValue>OptionsResInt[i].MinValue)
						{
							if(OptionsResInt[i].MaxValue<value)
								value=OptionsResInt[i].MaxValue;
							else if(OptionsResInt[i].MinValue>value)
								value=OptionsResInt[i].MinValue;
						}
						memcpy(((char*)opts)+OptionsResInt[i].Offset, &value, 4);
						break;
					}
				}
			}

		}
		else
			break;
	}

#ifndef URU
	//Read from sensor EEPROM
	if(!LoadSign)
	{
#if 0 //zzz
printf("ReadSensorOptions\n");
		ReadSensorOptions(opts);
#endif
	//	gOptions.IR_SensorOn=LoadInteger("IRSensorOn", 0);	//add by jazzy 2009.02.23 for IR sensor
          //      DBPRINTF("Read sensor options  gOptions.IR_SensorOn =%d\n",gOptions.IR_SensorOn);
#ifdef IR_SENSOR
                if(gOptions.IR_SensorOn)
                        IRSensor_Init();
#endif

		LoadSign=TRUE;
	}
#endif
	return opts;
}

char *GetDefaultOption(const char *name,char *value)
{
        extern int gWGFailedID;
        extern int gWGDuressID;
        extern int gWGSiteCode;
        //extern int gWGPulseWidth;
        //extern int gWGPulseInterval;
        char *s=NULL;
        int i;
        for(i=0;i<OPTIONSRESINTCOUNT;i++)
        {
                if(strcmp(name,OptionsResInt[i].OptionName)==0)
                {
                        int v=LoadInteger(OptionsResInt[i].OptionName, OptionsResInt[i].DefaultValue);
                        if(v!=-1)
                        if(OptionsResInt[i].MaxValue>OptionsResInt[i].MinValue)
                        {
                                if(OptionsResInt[i].MaxValue<v)
                                        v=OptionsResInt[i].MaxValue;
                                else if(OptionsResInt[i].MinValue>v)
                                        v=OptionsResInt[i].MinValue;
                        }
                        sprintf(value,"%d",v);
                        break;
                }
        }
        if(strlen(value)==0)
        {
                if(strcmp(name, "WiegandFmt")==0)
                {
                        LoadStr("WiegandFmt",s);
                        if(s)
                                sprintf(value,"%s",s);
                        else
                                sprintf(value,"%d",26);
                }
                else if(strcmp(name, "WGFailedID")==0)
                        sprintf(value,"%d",gWGFailedID );
                else if(strcmp(name, "WGDuressID")==0)
                        sprintf(value,"%d",gWGDuressID );
                else if(strcmp(name, "WGSiteCode")==0)
                        sprintf(value,"%d",gWGSiteCode);
               // else if(strcmp(name,"WGPulseWidth")==0)
               //         sprintf(value,"%d",gWGPulseWidth);
               // else if(strcmp(name,"WGPulseInterval")==0)
               //         sprintf(value,"%d",gWGPulseInterval);
                else if(strcmp(name,"~RFSStart")==0)
                        sprintf(value,"%d",gOptions.RFCardSecStart);
                else if(strcmp(name,"~RFSLen")==0)
                        sprintf(value,"%d",gOptions.RFCardSecLen );
                else if(strcmp(name,"~RFFPC")==0)
                        sprintf(value,"%d",gOptions.RFCardFPC);
        }
        return value;
}


BOOL SaveStrIgnoreCheck(const char *name, const char *value)
{
	char buffer[VALUE_BUFFERLEN];
	int len;

	len=strlen(value);
	//check language item whether can be setup or not
	if(!gOptions.MultiLanguage)
	{
		if(strcmp(name, "Language")==0) return TRUE;
	}
	CombineNameAndValue(name, value, TRUE, buffer);
	len=lseek(fdOptions, 0, SEEK_END);
	if (len>=MAX_OPTION_SIZE)
	{
		ClearOptionItem("NONE");
		len=lseek(fdOptions, 0, SEEK_END);
	}
	if (len<MAX_OPTION_SIZE)
	{
		write(fdOptions, buffer, strlen(buffer));
		fsync(fdOptions);
	}

	ExecuteActionForOption(name, value);

	return ((len<MAX_OPTION_SIZE)?TRUE:FALSE);
}

POptions SaveOptions(POptions opts)
{
	int i;
	TOptions OldOpt;
	char Buf[20];
	char buffer[1000]={0};
	int value;

	memset(&OldOpt, 0, sizeof(TOptions));
	LoadOptions(&OldOpt);
	if(gMachineState==STA_MENU) //正在进行设置
	{
		for(i=0;i<sizeof(OldOpt)/sizeof(int)-2;i++)
			if(((int*)&OldOpt)[i]!=((int*)opts)[i])
				FDB_AddOPLog(ADMINPIN, OP_CHG_OPTION, i,0,0,0);
	}
	for(i=0;i<OPTIONSRESSTRCOUNT;i++)
	{
		if(memcmp((((char*)opts)+OptionsResStr[i].Offset), (((char*)&OldOpt)+OptionsResStr[i].Offset), OptionsResStr[i].OptionLong))
		{
			memset(buffer, 0x00, sizeof(buffer));
			if(OptionsResStr[i].Formator)
				OptionsResStr[i].Formator(buffer, (BYTE*)(((char*)opts)+OptionsResStr[i].Offset));
			else
				nstrcpy(buffer,(((char*)opts)+OptionsResStr[i].Offset), OptionsResStr[i].OptionLong);
			SaveStrIgnoreCheck(OptionsResStr[i].OptionName, buffer);
		}
	}
	for(i=0;i<OPTIONSRESINTCOUNT;i++)
	{
		//DBPRINTF("Name=%s New=%d old=%d\n", OptionsResInt[i].OptionName, *(int*)(((char*)opts)+OptionsResInt[i].Offset), *(int*)(((char*)&OldOpt)+OptionsResInt[i].Offset));

		if(memcmp((((char*)opts)+OptionsResInt[i].Offset), (((char*)&OldOpt)+OptionsResInt[i].Offset), 4))
		{
			memset(Buf, 0x00, sizeof(Buf));
			memcpy(&value, ((char*)opts)+OptionsResInt[i].Offset, 4);
			sprintf(Buf, "%d", value);
//			printf("option:%s\n",OptionsResInt[i].OptionName);
			SaveStrIgnoreCheck(OptionsResInt[i].OptionName, Buf);
		}
	}

	//flush the cached data to disk
	fsync(fdOptions);

	opts->Saved=TRUE;
	return opts;
}


//extern int fromRight;
//zsliu
int initLCDTypeOptions()
{
	//supportLCDType=0  //支持3寸彩屏固件
	if(gOptions.supportLCDType == 0)
	{
		gOptions.MenuOffset = 20;
		gOptions.LCDWidth = 400;
		gOptions.LCDHeight = 240;
		
		gOptions.ControlOffset = 80;
		gOptions.GridWidth = 20;
		gOptions.MainVerifyOffset = 70;
	}
	//supportLCDType=1  //支持3.5寸彩屏固件
	else if(gOptions.supportLCDType == 1)
	{
		gOptions.MenuOffset = 0;
		gOptions.ControlOffset = 0;
		gOptions.LCDWidth = 320;
		gOptions.LCDHeight = 240;
		gOptions.GridWidth = 0;
		gOptions.MainVerifyOffset = 0;
	}
	//printf("initLCDTypeOptions supportLCDType = %d, MenuOffset=%d, ControlOffset=%d,fromRight=%d, MainVerifyOffset=%d\n",	gOptions.supportLCDType, gOptions.MenuOffset, gOptions.ControlOffset,fromRight, gOptions.MainVerifyOffset);

	return 1;
}


char* LoadDefaultLanguageByID(int LanguageID)
{
	//读取语言，如果读取不到，就直接读取英语，如果再读取不到，就显示为空
	char buffer[128]={0};
  	//char *tmp=NULL;
  	int fdEnglishLng=0;
	char temp[VALUE_BUFFERLEN];
	char *pChar=NULL;
	char lanage[40]={0};

	sprintf(buffer, "/mnt/mtdblock/%s", "DefaultLanguage");
	fdEnglishLng = open(buffer, O_RDONLY);	
	if(fdEnglishLng<=0)	
	{
		printf("Error: load default language failed : %s\n", buffer);
		
		memset(Language, 0, sizeof(Language));
		sprintf(Language, "%s%d", "ID=",LanguageID);
		pChar=Language;

		return pChar;
	}
		
	lanage[0]=69;
	sprintf(lanage+1,"/_%d_",LanguageID);
	
	if (LoadStrFromFile(fdEnglishLng, lanage, temp, FALSE, 0) != -1)
	{
		pChar = strCache(temp);
	}
	else
	{
		memset(Language, 0, sizeof(Language));
		sprintf(Language, "%s%d", "ID=",LanguageID);
		pChar=Language;
	}
	
	if (fdEnglishLng > 0)
	{
		close(fdEnglishLng);
	}
	
	return pChar;
}

int CheckOptionsIsEncrypt(char *str)
{
#if 0
	BOOL exitsign=FALSE;
	int i = 0;
	char name[128], value[VALUE_BUFFERLEN];
	if(!str)
		return 0;
	//SplitByChar(str, name, value, '=');
	if(strcmp(str, "~SerialNumber") == 0)
		return 1;
	else if(strcmp(str, "~OEMVendor") == 0)
		return 1;
	else if(strcmp(str, "~AlgVer") == 0)
		return 1;
	else if(strcmp(str, "~DeviceName") == 0)
		return 1;
	else if(strcmp(str, "~ProductTime") == 0)
		return 1;
	for(i = 0; i < OPTIONSRESSTRCOUNT; i++)
	{
		if(strcmp(str, OptionsResStr[i].OptionName) == 0)
		{
			if(OptionsResStr[i].IsEncrypt)
				return 1;
			else
				return 0;
		}
	}

	for(i = 0; i < OPTIONSRESINTCOUNT; i++)
	{
		if(strcmp(str, OptionsResInt[i].OptionName) == 0)
		{
			if(OptionsResInt[i].IsEncrypt)
			{
				DBPRINTF("Options %s, str = %s\n", __FUNCTION__, str);
				printf("the option need password\n");
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
#endif
	return 0;
}
