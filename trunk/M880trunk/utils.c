/*************************************************

  ZEM 200                                          

  utils.c 

  Copyright (C) 2003-2005, ZKSoftware Inc.      		

 *************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <crypt.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <fcntl.h>
#include "utils.h"
#include "arca.h"
#include "flash.h"
#include "options.h"
#include "rtc.h"
#include "netspeed.h"
#include "net.h"
#include "serial.h"
#include "flashdb.h"
#include <errno.h> /*dsl 2012.4.19*/
#include "flash.h"

#define cmux1_seril "/dev/ttyS3"
#ifdef CMUX
#undef cmux1_seril
#define cmux1_seril "/dev/cmux1"
#undef ZEM600
#endif

#ifdef ZEM600
#undef cmux1_seril
#define cmux1_seril "/dev/ttyS0"
#undef CMUX
#endif

int fd_serial_cmux1=-1;
unsigned char gsim_id[64];
typedef struct _Config_ {
	unsigned long key1;
	unsigned char config_data[1024-(3*4)];
	unsigned long key2;
	unsigned long cksum;
}TConfig, *PConfig;

/*dsl 2011.5.4*/
int wifi_rausb0_flag=-1;

//xsen add
int systemEx(char * CmdString)
{
#if defined(LINUX_26KERNEL) || defined(ZEM600)
	int child_pid=-1,status;
	if(CmdString==NULL) {
		return -2;
	}
	child_pid=vfork();
	if (child_pid==0) {
		execl("/bin/sh", "sh", "-c", CmdString, (char *)0);;
		_exit(EXIT_SUCCESS);
	} else if (child_pid>0) {
		waitpid(child_pid,&status,0);
	} else {
		status=-1;
		perror("vfork fail!!!\n");
	}
	return status;
#else
	return system(CmdString);
#endif
}
//end

U32 GetTickCount1(void)
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

U32 GetUS()
{
	return GetTickCount1()*1000;
}


char *GetEnvFilePath(const char *EnvName, const char *filename, char *fullfilename)
{
	if (getenv(EnvName)) 
		sprintf(fullfilename, "%s%s", getenv(EnvName), filename);
	else
		sprintf(fullfilename, "%s", filename);	
	return fullfilename;
}

void SetMACAddress(unsigned char *MAC)
{
	char buffer[256];
	char macstr[20];

	memset(macstr,0,sizeof(macstr));
	memset(buffer,0,sizeof(buffer));
	sprintf(macstr, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
			gOptions.MAC[0], gOptions.MAC[1], gOptions.MAC[2], gOptions.MAC[3], gOptions.MAC[4], gOptions.MAC[5]);

	sprintf(buffer, "ifconfig eth0 down && ifconfig eth0 hw ether %s && ifconfig eth0 up", macstr); 
	systemEx(buffer);
}

void SetIPAddress(char *Action, unsigned char *ipaddress)
{
	char buffer[128];    

	memset(buffer,0,sizeof(buffer));
	if(strcmp(Action, "IP")==0)
		sprintf(buffer, "ifconfig eth0 %d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
	else if (strcmp(Action, "NETMASK")==0)
		sprintf(buffer, "ifconfig eth0 netmask %d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
	systemEx(buffer);
}

BOOL SetGateway(char *Action, unsigned char *ipaddress)
{
	char buffer[128];    
	int rc;

	memset(buffer,0,sizeof(buffer));
	if(ipaddress[0]==0) return TRUE;
	systemEx("route del default gw 0.0.0.0");
	sprintf(buffer, "route %s default gw %d.%d.%d.%d", Action, ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
	rc=systemEx(buffer);
	return (rc==EXIT_SUCCESS);
}

void SetDNServerAddress(unsigned char *DNServeraddress)
{
	char buffer[128];    

	memset(buffer,0,sizeof(buffer));
	sprintf(buffer, "nameserver %d.%d.%d.%d\n", DNServeraddress[0], DNServeraddress[1], DNServeraddress[2], DNServeraddress[3]);
	FILE *fp=NULL;
	fp = fopen("/etc/resolv.conf", "w+");
	if(fp == NULL)
	{
		printf("err open file resolv.conf\n");
		return ;
	}
	fwrite(buffer, strlen(buffer), 1, fp);
	fflush(fp);
	fclose(fp);
	return ;
}

void SetNetworkIP_MASK(BYTE *ipaddress, BYTE *netmask)
{
	char buffer[128];

	memset(buffer,0,sizeof(buffer));
	sprintf(buffer, "ifconfig eth0 %d.%d.%d.%d netmask %d.%d.%d.%d",
			ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3],
			netmask[0], netmask[1], netmask[2], netmask[3]);
	systemEx(buffer);
	//        DBPRINTF("Setup network ip&netmask OK!\n");
}

void SetNetIP_MASK(const char *netname, BYTE *ipaddress, BYTE *netmask) //ccc
{
	char buffer[128];

	memset(buffer,0,sizeof(buffer));
	sprintf(buffer, "ifconfig %s %d.%d.%d.%d netmask %d.%d.%d.%d",
			netname,ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3],
			netmask[0], netmask[1], netmask[2], netmask[3]);
	systemEx(buffer);
	//       DBPRINTF("Setup network %s ip&netmask OK!\n", netname);
}


void SetNetworkPara(BYTE *ipaddress, BYTE *netmask, BYTE *gateway)
{
	char buffer[128];
	int pid, rc;

	memset(buffer,0,sizeof(buffer));
	pid=fork(); 
	if (pid==0)
	{	
		sprintf(buffer, "ifconfig eth0 %d.%d.%d.%d netmask %d.%d.%d.%d",
				ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3],
				netmask[0], netmask[1], netmask[2], netmask[3]);
		systemEx(buffer);
		//DBPRINTF("Setup network ip&netmask OK!\n");	
		if(gateway[0])
		{
			//ping gateway--clear the cached MAC(ROUTER)
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer, "ping -c 1 %d.%d.%d.%d",
					gateway[0], gateway[1], gateway[2], gateway[3]);
			while(TRUE)
			{
				rc=systemEx(buffer);
				if(rc==EXIT_SUCCESS)
					break;
				else
					sleep(10);
			}
			systemEx("route del default gw 0.0.0.0");
			//setup gateway
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer, "route add default gw %d.%d.%d.%d",
					gateway[0], gateway[1], gateway[2], gateway[3]);
			while(TRUE)
			{
				rc=systemEx(buffer);
				if(rc==EXIT_SUCCESS)
				{
					//			DBPRINTF("Setup network Gateway OK!\n");
					break;
				}
				else
				{
					DBPRINTF("Setup network Gateway failed!\n");
					sleep(30);
				}
			}
		}
		exit(EXIT_SUCCESS);	
	}
}

void RebootMachine(void)
{
	//system("reboot");
	//dsl 2011.9.14, Now the memory not enough for zem510, zem560 and zem515 coreboard when support TTS, Webserver, 3200FPs
	TTime t;
	GetTime(&t);
	ExSetPowerOnTime(t.tm_hour, t.tm_min);
	DelayMS(10);
	ExPowerRestart();	
}

int Decode16(char *String, char* Data)
{
	int i,v1,v2, c; 
	int Size=(int)strlen(String)/2;
	for(i=0;i<Size;i++)
	{
		c=String[i*2];
		v1=(c>='a')?(c-'a'+10):(c>='A'?(c-'A'+10):(c-'0'));
		c=String[i*2+1];
		v2=(c>='a')?(c-'a'+10):(c>='A'?(c-'A'+10):(c-'0'));
		if(v1<0) return 0;
		if(v2<0) return 0;
		if(v1>15) return 0;
		if(v2>15) return 0;
		Data[i]=v1*16+v2;
	}
	return Size;
}

char *ConvertMonth(char *version, char *iversion)
{
	char *FWMonths[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	int i;
	char mon[10];

	memset(iversion, 0, sizeof(iversion));
	memset(mon, 0, sizeof(mon));
	strncpy(mon, version+9, 3); //month name 
	for(i=1;i<=12;i++)
	{
		if(strcmp(mon, FWMonths[i-1])==0)
			sprintf(iversion+13, " %02d ", i);
	}
	memcpy(iversion, version, 9);
	memcpy(iversion+9, version+16, 4);
	memcpy(iversion+17, version+13, 2);
	return iversion;
}

void doPrint(char * buffer, char * fmt, va_list ap)
{
	void *p1, *p2, *p3, *p4, *p5, *p6;

	p1 = va_arg(ap,void*);
	p2 = va_arg(ap,void*);
	p3 = va_arg(ap,void*);
	p4 = va_arg(ap,void*);

	p5 = va_arg(ap,void*);
	p6 = va_arg(ap,void*);

	sprintf(buffer,fmt,p1,p2,p3,p4,p5,p6);
}

char *nstrcpy(char *Dest, const char *Source, int Size)
{
	char *s;
	s=Dest;
	while((*Source) && Size)
	{
		*s++=*Source++;
		Size--;
	}
	return Dest;
}

BYTE *nmemcpy(BYTE *Dest, const BYTE *Source, int Size)
{
	BYTE *s;
	s=Dest;
	while(Size--)
	{
		*s++=*Source++;
	}
	return Dest;
}

int nstrcmp(const char *s1, const char *s2, int size)
{
	while(size)
	{
		if((*s2)<(*s1)) return -1;
		if((*s2)>(*s1)) return 1;
		if((*s1)==0) return 0;
		s1++;s2++;
		size--;
	}
	return 0;
}

int nmemcmp(const BYTE *s1, const BYTE *s2, int size)
{
	while(size)
	{
		if((*s2)<(*s1)) return -1;
		if((*s2)>(*s1)) return 1;
		s1++;s2++;
		size--;
	}
	return 0;
}

int nmemset(BYTE *Dest, BYTE Data, int Size)
{
	while(Size--)
		*Dest++=Data;
	return 1;
}

/* test for a digit. return value if digit or -1 otherwise */
static int digitvalue(char isdigit)
{
	if (isdigit >= '0' && isdigit <= '9' )
		return isdigit - '0';
	else
		return -1;
}

/* test for a hexidecimal digit. return value if digit or -1 otherwise */
static int xdigitvalue(char isdigit)
{
	if (isdigit >= '0' && isdigit <= '9' )
		return isdigit - '0';
	if (isdigit >= 'a' && isdigit <= 'f')
		return 10 + isdigit - 'a';
	return -1;
}

/* convert a string to an u32 value. if the string starts with 0x, it
 * is a hexidecimal string, otherwise we treat is as decimal. returns
 * the converted value on success, or -1 on failure. no, we don't care
 * about overflows if the string is too long.
 */
int strtou32(const char *str, unsigned int *value)
{
	int i;

	*value = 0;

	if((str[0]=='0') && (str[1]=='x')) {
		/* hexadecimal mode */
		str += 2;

		while(*str != '\0') {
			if((i = xdigitvalue(*str)) < 0)
				return -1;

			*value = (*value << 4) | (unsigned int)i;

			str++;
		}
	} else {
		/* decimal mode */
		while(*str != '\0') {
			if((i = digitvalue(*str)) < 0)
				return -1;

			*value = (*value * 10) + (unsigned int)i;

			str++;
		}
	}

	return 0;
}

int str2mac(char* str, BYTE* mac)
{
	int i=0;
	char *p=str;
	char t[6];
	int addr;

	while(i < 6) {
		addr = 0;
		while( *p && (*p != ':') ) {
			if( (*p >= '0') && (*p <= '9') ) {
				addr = addr * 16 + (*p - '0');
				p++;
			}
			else if( (*p >= 'a') && (*p <= 'f') ) {
				addr = addr * 16 + (*p - 'a'+10);
				p++;
			}
			else if ( (*p >= 'A') && (*p <= 'F') ) {
				addr = addr * 16 + (*p - 'A'+10);
				p++;
			}
			else return -1; /* invalid */
		}

		if (addr <= 255) t[i] = (addr&0xFF);
		else break;

		i++;

		if(*p) p++;
		else break;
	}

	if( i!=6 )  return -1;

	memcpy(mac, t, sizeof(t));

	return 0;
}

int str2ip(char* str, BYTE* ip)
{
	int i=0;
	char *p=str;
	char t[4];
	int addr;

	memset(t,0,sizeof(t));
	while(i < 4) {
		addr = 0;
		while( *p && (*p != '.') ) {
			if( (*p >= '0') && (*p <= '9') ) {
				addr = addr * 10 + (*p - '0');
				p++;
			}
			else return -1; /* invalid */
		}

		if (addr <= 255) t[i] = (addr&0xFF);
		else break;

		i++;

		if(*p) p++;
		else break;

	}

	if( i!=4 )  return -1;

	memcpy(ip, t, sizeof(t));

	return 0;
}

int str2int(char *buf, int DefaultValue)
{
	int v,n=1,d,c;
	if('-'==*buf)
	{
		n=-1;
		buf++;
	}
	v=0;c=0;
	do{
		d=buf[c];
		if(d==0) break;
		if(d==' ')
		{
			if(v) break;
		}
		else if(d<'0' || d>'9')
		{
			return DefaultValue;
		}
		else
			v=v*10+(d-'0');
		c++;
	}while(1);
	if(c)
		return n*v;
	else
		return DefaultValue;
}

char *TrimRightStr(char *buf)
{
	int i=strlen(buf);
	while(i--)
		if(((BYTE*)buf)[i]==0x20) buf[i]=0; else break;
	return buf;
}

char* SPadCenterStr(char *buf, int size, const char *s)
{
	int len;
	for(len=0;len<size;len++) buf[len]=' ';buf[size]=0;
	if(NULL==s) return buf;
	len=0;
	while(*s)
		if(' '==*s) s++;
		else break;
	while(s[len]) len++;
	if(len==0) return buf;
	while(' '==s[len-1]) len--;
	if(size<len)
		memcpy(buf, s, size);
	else
		memcpy(buf+(size-len)/2,s,len);
	return buf;
}

char* SPadRightStr(char *buf, int size, const char *s)
{
	int len;
	for(len=0;len<size;len++) buf[len]=' ';buf[size]=0;
	if(NULL==s) return buf;
	len=0;
	while(*s)
		if(' '==*s) s++;else break;
	while(s[len]) len++;
	if(len==0) return buf;
	while(' '==s[len-1]) len--;
	if(size<len)
		memcpy(buf, s, size);
	else
		memcpy(buf+size-len,s,len);
	return buf;
}

char *TrimLeftStr(char *buf)
{
	BYTE *p=(BYTE*)buf, *p0=(BYTE*)buf;
	while(0x20==*p)
		if(*p)
			p++;
		else
		{
			*buf=0;
			return buf;
		}
	while(1)
	{
		BYTE x=*p++;
		*p0++=x;
		if(x==0) break;
	}
	return buf;
}
//2006.08.03 ssr
//trim '0' char
char *TrimLeft0(char *buf)
{
	BYTE *p=(BYTE*)buf, *p0=(BYTE*)buf;
	while(0x30==*p)
		if(*p)
			p++;
		else
		{
			*buf=0;
			return buf;
		}
	while(1)
	{
		BYTE x=*p++;
		*p0++=x;
		if(x==0) break;
	}
	return buf;
}
char *TrimStr(char *buf)
{
	return TrimLeftStr(TrimRightStr(buf));
}

int SearchIndex(char **Items, const char *Text, int ItemCount)
{
	int i;
	for(i=0;i<ItemCount;i++)
	{
		if(strcmp(Items[i], Text)==0) return i;
	}
	return -1;
}

char *Pad0Num(char *buf, int size, int value)
{
	char fmt[20];
	sprintf(fmt,"%%0%dd",size);
	sprintf(buf, fmt, value);
	return buf;
}
//2006.08.03 ssr
//add '0' to pre str
char *Pad0Str(char *buf, int size,char *value)
{
	//printf("value: %s\n",value);
	int l=0;
	l = strlen(value);
	if (l == size)
	{
		nstrcpy(buf,value,size);
		return value;
	}
	if (gOptions.PinPreAdd0)
		memset(buf,'0',size);
	else
		memset(buf,' ',size);
	//printf("buf: %s\n",buf);
	memcpy(buf+(size-l),value,l);	
	//printf("buf2: %s\n",buf);

	return buf;
}
int StrValue(const char *p, int *Next)
{
	int i;
	unsigned ret=0;
	for(i=0;i<19;i++)
	{
		if((p[i]>='0') && (p[i]<='9'))
			ret=ret*10+(p[i]-'0');
		else if(!((p[i]==' ') && (ret==0)))
		{
			if(Next) *Next=i;
			break;
		}
	}
	return ret;
}

int HexStrValue(const char *p, int *Next)
{
	int i;
	unsigned ret=0;
	for(i=0;i<19;i++)
	{
		if((p[i]>='0') && (p[i]<='9'))
			ret=ret*16+(p[i]-'0');
		else
			if((p[i]>='A') && (p[i]<='F'))
				ret=ret*16+(p[i]-'A'+10);
			else
				if((p[i]>='a') && (p[i]<='f'))
					ret=ret*16+(p[i]-'a'+10);
				else
				{
					if(Next) *Next=i;
					break;
				}
	}
	return ret;
}

int HexStrToInt(const char *str, int *value)
{
	static int i;
	*value=HexStrValue(str, &i);
	return str[i];
}

//从p中复制第index个值到buf中
int SCopyStrFrom(char *buf, char *p, int index)
{
	static int i,j;

	j=0;
	if(p){
		for(i=0; p[i] && (i<8*1024); i++)
		{
			if(p[i]==':')
			{
				if(index==0)
				{
					buf[j]=0;
					break;
				}
				else
					index--;
			}
			else if(index==0)
				buf[j++]=p[i];
		}
		if(p[i]==0) buf[j]=0;
	}
	return j;
}

int SPackStr(char *buf)
{
	int c=0;
	char ch, lc=0, *p=buf;
	if(!buf) return c;
	lc=*p;
	while(1)
	{
		ch=*buf;
		if(ch==':')
		{
			if(lc==':') 
			{
				buf++;
				continue;
			}
		}
		else if(!ch) 
			break;
		*p=ch;
		buf++;
		p++;
		c++;
		lc=ch;
	}
	*p=0;
	p--;
	if(*p==':')
		*p=0;
	return TRUE;
}

int SaveIntList(char *buf, int *INTs, int Count, int InvalidInt, int PackInvalidInt)
{
	char *p;
	int i;
	p=buf;
	*p=0;
	if(Count<=0) return 0;
	if(INTs[0]!=InvalidInt) sprintf(p, "%d", INTs[0]);
	for(i=1;i<Count;i++)
	{
		p+=strlen(p);
		*p=':'; *++p=0;
		if(INTs[i]!=InvalidInt)
			sprintf(p, "%d", INTs[i]);
	}
	if(PackInvalidInt)
		return SPackStr(buf);
	else
		return (p-buf)+strlen(p);
}

//计算p中被分割的字符串的个数
int SCountStr(char *p)
{
	int i, ret=0;
	if(':'!=*p) ret=1;
	if(*p==0) return 0;
	while(1)
	{
		i=*p;
		if(i==0)
			return ret;
		else if(i==':')
		{
			if(p[1])
				ret++;
		}
		p++;
	}
	return ret;
}
//返回p中第index个值的整数形式，并在Next中返回该数值的结束地址
int SIntValueFrom(char *p, int index, int *Next)
{
	int c=*p,i=0;
	while(index)
	{
		c=*p;
		if(c==':') index--; else if(c==0) break;
		p++;
		i++;
	}
	if(c)
	{
		c=StrValue(p, Next);
		*Next=*Next+i;
		return c;
	}
	else
	{
		*Next=i;
		return 0;
	}
}

//返回p中第index个值（16进值）的整数形式，并在Next中返回该数值的结束地址
int SHexValueFrom(char *p, int index, int *Next)
{
	int c=*p,i=0;
	while(index)
	{
		c=*p;
		if(c==':') index--; else if(c==0) break;
		p++;
		i++;
	}
	if(c)
	{
		c=HexStrValue(p, Next);
		*Next=*Next+i;
		return c;
	}
	else
	{
		*Next=i;
		return 0;
	}
}

int Hex2Char(char *s)
{
	int ret=0;
	ret=*s;
	if(ret>='A')
	{	
		ret=ret-'A'+10;
		if(ret>15) ret=-1;
	}
	else if(ret>='0')
	{
		ret=ret-'0';
		if(ret>9) ret=-1;
	}
	else
		return -1;
	return ret;
}

const BYTE HEXCHARS[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

int EncodeHex(BYTE *String, BYTE *Data, int Size)
{
	int i;
	for(i=0;i<Size;i++)
	{
		String[i*2]=HEXCHARS[Data[i]/16];
		String[i*2+1]=HEXCHARS[Data[i]&0x0F];
	}
	String[2*i]=0;
	return 2*i;
}

void SetBit(BYTE *Buffer, int Index)
{
	Buffer[Index>>3]|=(1<<(Index & 7));
}

void ClearBit(BYTE *Buffer, int Index)
{
	Buffer[Index>>3]&=~(1<<(Index & 7));
}

int GetBit(BYTE *Buffer, int Index)
{
	return (Buffer[Index>>3]>>(Index & 7)) & 1;
}

void memor(char *s1, char *s2, int len)
{
	do {
		*s1++ |= *s2++;
	} while (--len);
}

char *GenerateMacBySN(BYTE *vendor, U32 serialnumber, char *MAC)
{
	sprintf(MAC,"%02X:%02X:%02X:%02X:%02X:%02X",
			vendor[0],vendor[1],vendor[2],
			(serialnumber>>16)&0xFF,(serialnumber>>8)&0xFF,serialnumber&0xFF);
	return MAC;
}
#ifdef ZEM600

#define CFG_BLOCK_BASE          8
#define CFG_BLOCK_LEN           4

#define CFG_INDEX               0
#define CFG_LEN                 (16*1024)
#define CFG_RECEIVED_LEN        2048
#define CFG_ROTATE_INDEX        4
#define CFG_ROTATE_LEN          4
#define CFG_MAC_LEN             6
#define CFG_MAC_INDEX           (CFG_INDEX+CFG_RECEIVED_LEN)

#define LOGO_BLOCK_BASE          12
#define LOGO_BLOCK_LEN           4

#define CFG_LCDTYPE_INDEX       8 //Eighth byte

BOOL SaveBufferToSector(int block, char *data, int index, int size)
{
	U8 buffer[130*1024];
	U32 StartBlock, StartPage,NumPage;
	U32 i=0;

	StartBlock = block;
	StartPage = StartBlock*NAND_PAGE_PER_BLOCK;
	NumPage = sizeof(buffer)/NAND_PAGE_SIZE;

	//printf("SaveBufferTOSector:StartBlock %d,StartPage %d, NumPage %d\r\n",StartBlock,StartPage,NumPage);

	for(i=0; i<NumPage;i++)
		FlashReadPage(StartPage+i, buffer+i*NAND_PAGE_SIZE);
	//printf("read %d pages from nand to buffer\n",i);
	//DBPRINTF("Old MAC %x:%x:%x:%x:%x:%x\n", *((BYTE *)buffer+index), *((BYTE *)buffer+index+1), *((BYTE *)buffer+index+2), *((BYTE *)buffer+index+3), *((BYTE *)buffer+index+4), *((BYTE *)buffer+index+5));

	if(FlashEraseBlock(StartBlock)==0)
	{
		//DBPRINTF("DELETE SECTOR OK!,index=%d,size=%d\n",index,size);
		memcpy((BYTE *)buffer+index, data, size);
		//DBPRINTF("MAC %x:%x:%x:%x:%x:%x\n", *((BYTE *)buffer+index), *((BYTE *)buffer+index+1), *((BYTE *)buffer+index+2), *((BYTE *)buffer+index+3), *((BYTE *)buffer+index+4), *((BYTE *)buffer+index+5));

		for(i=0; i<NumPage;i++)
		{
			FlashWritePage(StartPage+i, buffer+i*NAND_PAGE_SIZE);
			DelayMS(1);
		}
		//printf("write %d pages to nand\n",i);
		DBPRINTF("WRITE SECTOR OK!\n");
		return TRUE;
	}
	DBPRINTF("SaveBufferToSector Failed\n");
	return FALSE;
}

/* this function can be used to change 16k bytes config data when offset=0 */
BOOL SaveLOGOToFlash(int offset, char *config, int size)
{
	static int len=0;
	static int i=0;

	if(size>NAND_BLOCK_SIZE*2)	//256KB
		return FALSE;
	while(1)
	{
		if(size>NAND_BLOCK_SIZE)
		{
			len = NAND_BLOCK_SIZE;	
		}
		else
			len = size;

		size -= len;
		SaveBufferToSector(LOGO_BLOCK_BASE+i, config+i*NAND_BLOCK_SIZE, CFG_INDEX, len);
		i++;
		//	printf("i:%d size:%d, blksize:%d\n",i,size,NAND_BLOCK_SIZE);

		if(size==0)
			return TRUE;
	}
}

BOOL SaveMACToFlash(const char *MAC)
{
	BYTE mac[6];

	//	printf(" %s\n", __FUNCTION__);
	if(str2mac((char *)MAC, mac)==0)
	{
		Nand_save_MAC(mac);
		//		printf(" save MAC success\n");
		return TRUE;
	}
	return FALSE;
}	

BOOL SetScreenAddressToFlash(const char *Rote)
{
	int offsetLogoBlock=CFG_BLOCK_BASE;
	int data;
	data= strtoul(Rote, NULL, 0);

	printf("Rote=%s, %d\n", Rote, data);
	while(1)
	{
		if(FlashBlockIsBad(offsetLogoBlock)>0)
		{
			printf("skip bad block #%d\n",offsetLogoBlock);
			offsetLogoBlock++;
			continue;
		}
		return SaveBufferToSector(offsetLogoBlock, (char *)&data, CFG_ROTATE_INDEX, CFG_ROTATE_LEN);
	}


#if 0
	int data;
	//	printf("gOptions.ScreenAddress %d \n", gOptions.ScreenAddress);
	if(gOptions.ScreenAddress==1)
	{
		data=1;
	}
	else
	{
		data=0;
	}

	return SaveBufferToSector(CFG_BLOCK_BASE, (char *)&data, CFG_ROTATE_INDEX, CFG_ROTATE_LEN);
#endif
}

int SaveLinuxLOGOToFlash(const unsigned char *data, int size)
{
	int i=0;
	int len;

	while(1)
	{
		if(size<=NAND_BLOCK_SIZE)
			len = size;
		else
			len = NAND_BLOCK_SIZE;

		SaveBufferToSector(LOGO_BLOCK_BASE, (char *)(data+len*i++), 0, len);
		size -= len;
		if(size<=0)
			break;
	}
	return 1;
}

BOOL SaveUBOOTToFlash(unsigned char *data, int size)
{
	return FALSE;
}

/*dsl 2011.5.4.To set lcd type*/
BOOL SetScreenTypeToFlash(const char *LCDType)
{
	int offsetLogoBlock=CFG_BLOCK_BASE;
	int data;
	data= strtoul(LCDType, NULL, 0);
	printf("LCDType--------%s %x \n",LCDType,data);
	while(1)
	{
		if(FlashBlockIsBad(offsetLogoBlock)>0)
		{
			printf("skip bad block #%d\n",offsetLogoBlock);
			offsetLogoBlock++;
			continue;
		}
		return SaveBufferToSector(offsetLogoBlock, (char *)&data, CFG_LCDTYPE_INDEX, CFG_ROTATE_LEN);
	}
}

#else	//else ZEM600

#if defined(ZEM510) && defined(LINUX_26KERNEL)

#define CFG_BLOCK_BASE          8
#define CFG_BLOCK_LEN           4

#define CFG_INDEX               0
#define CFG_LEN                 (16*1024)
#define CFG_RECEIVED_LEN        2048
#define CFG_ROTATE_INDEX        4
#define CFG_ROTATE_LEN          4
#define CFG_MAC_LEN             6
#define CFG_MAC_INDEX           (CFG_INDEX+CFG_RECEIVED_LEN)

#define LOGO_BLOCK_BASE          11
#define LOGO_BLOCK_LEN           4

#define UBOOT_BLOCK_BASE	0

BOOL SaveBufferToSector(int block, char *data, int index, int size)
{
	U8 buffer[128*1024];
	U32 StartBlock, StartPage,NumPage;
	U32 i=0;

	StartBlock = block;
	StartPage = StartBlock*NAND_PAGE_PER_BLOCK;
	NumPage = sizeof(buffer)/NAND_PAGE_SIZE;

	printf("SaveBufferTOSector:StartBlock %d,StartPage %d, NumPage %d\r\n",StartBlock,StartPage,NumPage);

	for(i=0; i<NumPage;i++)
		FlashReadPage(StartPage+i, buffer+i*NAND_PAGE_SIZE);
	//printf("read %d pages from nand to buffer\n",i);
	//DBPRINTF("Old MAC %x:%x:%x:%x:%x:%x\n", *((BYTE *)buffer+index), *((BYTE *)buffer+index+1), *((BYTE *)buffer+index+2), *((BYTE *)buffer+index+3), *((BYTE *)buffer+index+4), *((BYTE *)buffer+index+5));

	if(FlashEraseBlock(StartBlock)==0)
	{
		//DBPRINTF("DELETE SECTOR OK!,index=%d,size=%d\n",index,size);
		memcpy((BYTE *)buffer+index, data, size);
		//DBPRINTF("MAC %x:%x:%x:%x:%x:%x\n", *((BYTE *)buffer+index), *((BYTE *)buffer+index+1), *((BYTE *)buffer+index+2), *((BYTE *)buffer+index+3), *((BYTE *)buffer+index+4), *((BYTE *)buffer+index+5));

		for(i=0; i<NumPage;i++)
			FlashWritePage(StartPage+i, buffer+i*NAND_PAGE_SIZE);
		//printf("write %d pages to nand\n",i);
		DBPRINTF("WRITE SECTOR OK!\n");
		return TRUE;
	}
	DBPRINTF("SaveBufferToSector Failed\n");
	return FALSE;
}

extern int Nand_save_MAC(U8 *buf);
BOOL SaveMACToFlash(const char *MAC)
{
	BYTE mac[6];

	//	printf(" %s\n", __FUNCTION__);
	if(str2mac((char *)MAC, mac)==0)
	{
		Nand_save_MAC(mac);
		//		printf(" save MAC success\n");
		return TRUE;
	}
	return FALSE;
}	

//offset is from 16K config area    logo offset is 0x812=2048(uboot config)+6(MAC)+12(License)
BOOL SaveLOGOToFlash(int offset, char *config, int size)
{               
	static int len=0;
	int i=0, offsetLogoBlock=0;
	offsetLogoBlock=LOGO_BLOCK_BASE;

	if(size>NAND_BLOCK_SIZE*2)	//256KB
		return FALSE;
	while(1)
	{
		if(size>NAND_BLOCK_SIZE)
		{
			len = NAND_BLOCK_SIZE;	
		}
		else
			len = size;
		if(FlashBlockIsBad(offsetLogoBlock+i)>0)
		{
			printf("skip bad block #%d\n",offsetLogoBlock+i);
			offsetLogoBlock++;
			continue;
		}
		size -= len;
		SaveBufferToSector(offsetLogoBlock+i, config+i*NAND_BLOCK_SIZE, 0, len);
		i++;
		printf("i:%d size:%d, blksize:%d\n",i,size,NAND_BLOCK_SIZE);

		if(size==0)
			return TRUE;
	}
}

//UBOOT size= 8*8k + 64k
BOOL SaveUBOOTToFlash(unsigned char *data, int size)
{	
	static int len=0;
	int i=0, offsetUbootBlock;
	offsetUbootBlock=UBOOT_BLOCK_BASE;

	if(size<STD_SECTOR_SIZE)
		return FALSE;
	while(1)
	{
		if(size>NAND_BLOCK_SIZE)
		{
			len = NAND_BLOCK_SIZE;	
		}
		else
			len = size;
		if(FlashBlockIsBad(offsetUbootBlock+i)>0)
		{
			printf("skip bad block #%d\n",offsetUbootBlock+i);
			offsetUbootBlock++;
			continue;
		}
		size -= len;
		SaveBufferToSector(offsetUbootBlock+i, data+i*NAND_BLOCK_SIZE, 0, len);
		i++;
		printf("i:%d size:%d, blksize:%d\n",i,size,NAND_BLOCK_SIZE);

		if(size==0)
			return TRUE;
	}
}

//add cn 200-03-04
BOOL SetScreenAddressToFlash(void)
{	
	BYTE mac[6];
	memset(mac,0,6);
	//	printf("gOptions.ScreenAddress %d \n", gOptions.ScreenAddress);
	if(gOptions.ScreenAddress==1)
	{
		mac[0]=0x10;
		//		printf(" screen rotation mac[0] %x\n", mac[0]);
	}
	else
	{
		//		printf(" mac mac  ===0\n");
		mac[0]=0;
	}

	//	printf(" save screen buffer %x, rentrun %d \n", mac[0], SaveBufferTONandFlashAddr(mac, 512*2048+0x10,512*2048+0x12, 2 ,1));
	//	return SaveBufferTONandFlashAddr(mac, 512*2048+0x10,512*2048+0x12, 2 ,1);
	return SaveBufferToSector(CFG_BLOCK_BASE,(char *)mac,0x10,2);
}


#else

#define CFG_MONITOR_BASE 	((U32)FlashBaseAddr)
#define CFG_LEN                 (65*1024)         //TFT Logo Max file size 64K    by jazzy 2008.12.17
#define CFG_UBOOT_CFG_LEN	2048
#define CFG_MAC_LEN		6
#define CFG_MAC_INDEX		(STD_SECTOR_SIZE-CFG_LEN+CFG_UBOOT_CFG_LEN)
#define CFG_INDEX		(STD_SECTOR_SIZE-CFG_LEN)
#define TOP_SECTOR_SIZE		(8*1024)

#if 0  //跟nor flash的相关资料需要删除掉
BOOL SaveBufferToTopSector(int sector_index, char *data, int index, int size)
{
	WORD buffer[TOP_SECTOR_SIZE/2];
	U32 StartAddr, EndAddr;
	U32 Addr, i=0;
	StartAddr=CFG_MONITOR_BASE+TOP_SECTOR_SIZE*sector_index;
	memcpy((BYTE *)buffer, (BYTE *)StartAddr, TOP_SECTOR_SIZE);

	if(FlashSectorErase(StartAddr))
	{
		//DBPRINTF("DELETE SECTOR %d OK!\n", sector_index);
		memcpy((BYTE *)buffer+index, data, size);
		EndAddr=StartAddr+TOP_SECTOR_SIZE;
		for(Addr=StartAddr;Addr<EndAddr;Addr+=2,i++)
		{
			if(!FlashWriteWord(Addr, buffer[i]))
			{
				return FALSE;
			}
		}
		//DBPRINTF("WRITE SECTOR %d OK!\n", sector_index);
		return TRUE;
	}
	return FALSE;
}

BOOL SaveBufferToSector(char *data, int index, int size)
{
	WORD buffer[STD_SECTOR_SIZE/2];
	U32 StartAddr, EndAddr;
	U32 Addr, i=0;

	StartAddr=CFG_MONITOR_BASE+STD_SECTOR_SIZE;
	memcpy((BYTE *)buffer, (BYTE *)StartAddr, STD_SECTOR_SIZE);

	if(FlashSectorErase(StartAddr))
	{
		//DBPRINTF("DELETE SECTOR OK!\n");
		memcpy((BYTE *)buffer+index, data, size);
		EndAddr=StartAddr+STD_SECTOR_SIZE;
		for(Addr=StartAddr;Addr<EndAddr;Addr+=2,i++)
		{
			if(!FlashWriteWord(Addr, buffer[i]))
			{
				return FALSE;
			}
		}
		//DBPRINTF("WRITE SECTOR OK!\n");		
		return TRUE; 
	}
	DBPRINTF("SET MAC Failed\n");
	return FALSE;
}
#endif  //跟nor flash的相关资料需要删除掉

BOOL SaveMACToFlash(const char *MAC)
{
	BYTE mac[6];
	if(str2mac((char *)MAC, mac)==0)
	{                         
#ifdef ZEM510
		//	printf(" save macbuffer %s, rentrun %d \n", mac, SaveBufferTONandFlashAddr(mac, 513*2048,513*2048+6, CFG_MAC_LEN,1));
		return SaveBufferTONandFlashAddr(mac, 513*2048,513*2048+6, CFG_MAC_LEN,1);
#else   
		return SaveBufferToSector(mac, CFG_MAC_INDEX, CFG_MAC_LEN);
#endif
	}
	return FALSE;
}

//offset is from 16K config area    logo offset is 0x812=2048(uboot config)+6(MAC)+12(License)
BOOL SaveLOGOToFlash(int offset, char *config, int size)
{               
#ifdef ZEM510
	if(size<=CFG_LEN*4)       //64KB    add by jazzy 2008.12.17 for update LOGO by PC
		//return SaveBufferTONandFlash(config, 704,831, size,0);  //address is Nand page address
		return SaveBufferTONandFlashAddr(config, 704*2048,831*2048, size, 1);  //address is Nand page address
#else
	if((offset+size)<=CFG_LEN)//16KB			
		return SaveBufferToSector(config, CFG_INDEX+offset, size);
#endif
	else
		return FALSE;
}

//UBOOT size= 8*8k + 64k
BOOL SaveUBOOTToFlash(unsigned char *data, int size)
{
	int i;

	if(size>STD_SECTOR_SIZE)
	{
#ifdef ZEM510
		//for (i=0;i<10;i++)
		//	printf(" [%x]  ", data[i]);
		return SaveBufferTONandFlash(data, 0, 191, size,0);
#else
		for(i=0;i<8;i++)
		{
			SaveBufferToTopSector(i, data+i*TOP_SECTOR_SIZE, 0, TOP_SECTOR_SIZE);	
		}
		return SaveBufferToSector(data+STD_SECTOR_SIZE, 0, STD_SECTOR_SIZE-CFG_LEN);		
#endif
	}
	else
		return FALSE;
}

//add cn 200-03-04
BOOL SetScreenAddressToFlash(void)
{
	BYTE mac[6];
	memset(mac,0,6);
	//	printf("gOptions.ScreenAddress %d \n", gOptions.ScreenAddress);
	if(gOptions.ScreenAddress==1)
	{
		mac[0]=0x10;
		//		printf(" screen rotation mac[0] %x\n", mac[0]);
	}
	else
	{
		//		printf(" mac mac  ===0\n");
		mac[0]=0;
	}

#ifdef ZEM510
	//	printf(" save screen buffer %x, rentrun %d \n", mac[0], SaveBufferTONandFlashAddr(mac, 512*2048+0x10,512*2048+0x12, 2 ,1));
	return SaveBufferTONandFlashAddr(mac, 512*2048+0x10,512*2048+0x12, 2 ,1);
#else
	return FALSE;
#endif
}
#endif
#endif

void RedBootMac(const char *serialnumber)
{
	U32 sn=0;
	BYTE vendor[]={0x00, 0x0A, 0x5D};
	char mac[16];

	if(!serialnumber[0])
		sn=0;
	else if((serialnumber[0]>'9')||(serialnumber[0]<'0'))
		sn=atol(serialnumber+1);
	else
		sn=atol(serialnumber);
	if(sn==0) sn=time(NULL);
	GenerateMacBySN(vendor, sn, mac);
	SaveStr("MAC", mac, FALSE);
}

void CreateRootPwd(char *pwdfile, char *newpassword)
{
	int fp;
	char pwd[128], buf[256], tmp[8];
	char *p;

	if(strlen(newpassword)<5)
		strcpy(tmp, "solokey");
	else
		strcpy(tmp, newpassword);
	if(strlen(tmp)>=5)
	{

		fp=open(pwdfile, O_CREAT|O_RDWR|O_TRUNC|O_SYNC, S_IRUSR|S_IWUSR);
		if(fp!=-1)
		{	
			memset(pwd, 0, 128);
			p=crypt(tmp, "$1$");
			if(!p) p=crypt("solokey", "$1$");
			if(!p) return;
			nstrcpy(pwd, p, 128);
			sprintf(buf, "root:%s:0:0:root:/root:/bin/sh\n", pwd);
			write(fp, buf, strlen(buf));
			close(fp);
		}
	}
}

void SetRootPwd(int comkey)
{
}

int GetSensorSerialNumber(char *TitleStr, char *IDStr)
{

	FILE *fp;
	char buffer[4096];
	char *p;
	int i;
	BOOL bSign=FALSE;

	fp=fopen("/proc/bus/usb/devices", "rb");
	if(!fp) return 0;
	while(!feof(fp))
	{
		memset(buffer, 0, 4096);
		if(!fgets(buffer, 4096, fp)) break;
		i=0;
		while(buffer[i])
		{
			if(buffer[i]=='=')
			{
				p=buffer+i+1;
				buffer[strlen(buffer)-1]='\0';
				//printf("%s\n", p);
				if((!bSign)&&(TitleStr)&&(strncmp(p, TitleStr, strlen(TitleStr))==0))
					bSign=TRUE;
				else if(bSign)
				{
					strcpy(IDStr, p);
					return 1;
				}
				break;
			}
			i++;
		}
	}
	fclose(fp);
	return 0;
}

void SyncTimeByTimeServer(BYTE *TimeServerIP)
{
	char buffer[128];	
	time_t t;

	if(TimeServerIP[0])
	{
		sprintf(buffer, "rdate -s %d.%d.%d.%d", TimeServerIP[0], TimeServerIP[1], TimeServerIP[2], TimeServerIP[3]);
		if(systemEx(buffer)==EXIT_SUCCESS)
		{
			t=time(NULL);
			//process the timzezone offset(hour) seconds
			t+=LoadInteger("TZAdj", 0)*60*60;
			SetRTCClock(localtime(&t));
		}
	}
}

int str2cardkey(char *str, BYTE* cardkey)
{
	memset(cardkey,0xff,6);
	if(str) nstrcpy((char*)cardkey, str, 6);
	return 0;
}

void ExportProxySetting(void)
{
	char buf[128];
	int fd;

	if(gOptions.IClockFunOn)
	{
		fd=open(GetEnvFilePath("USERDATAPATH", "icserver.cfg", buf), O_RDWR|O_CREAT|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
		//http proxy server ip&port
		sprintf(buf, "%d.%d.%d.%d:%d\n", 
				gOptions.ProxyServerIP[0], gOptions.ProxyServerIP[1], gOptions.ProxyServerIP[2], gOptions.ProxyServerIP[3],
				gOptions.ProxyServerPort);
		write(fd, buf, strlen(buf));
		//serial number 
		sprintf(buf, "%s\n", LoadStrOld("~SerialNumber"));
		write(fd, buf, strlen(buf));
		//Enabled sign 
		sprintf(buf, "%d\n", gOptions.EnableProxyServer);
		write(fd, buf, strlen(buf));
		close(fd);
	}
}

#define PRIME_TBLSIZ (65536*255)

unsigned int hashpjw(const void *key, int keySize)
{
	unsigned char     *ptr=(unsigned char *)key;
	unsigned int       val=0;
	while (keySize--) {
		int tmp;
		val = (val << 4) + (*ptr);
		if ((tmp = (val & 0xf0000000))!=0) {
			val = val ^ (tmp >> 24);
			val = val ^ tmp;
		}
		ptr++;
	}
	return val % (PRIME_TBLSIZ-1);
}

unsigned int hashBuffer(TBuffer *buffer)
{
	return hashpjw(buffer->buffer, buffer->bufferSize);
}

TBuffer *createRomBuffer(unsigned int Address, int size)
{
	TBuffer *ret=(TBuffer*)MALLOC(sizeof(TBuffer));
	if(ret)
	{
		ret->buffer=(unsigned char *)Address;
		ret->bufferSize=size;
		ret->bufEnd=ret->buffer+size;
		ret->bufPtr=ret->buffer;
		ret->isRom=1;
	}
	return ret;
}

TBuffer *createRamBuffer(int size)
{
	char *Address = MALLOC(size);
	if(Address)
	{
		TBuffer *ret=createRomBuffer((unsigned int)Address, size);
		if(ret)
		{
			ret->isRom=0;
			return ret;
		}
		else
		{
			FREE(Address);
			return NULL;
		}
	}
	else
		return NULL;
}

void freeBuffer(TBuffer *buffer)
{
	if(buffer==NULL) return;
	if((buffer->buffer) && (!buffer->isRom))
		FREE(buffer->buffer);
	FREE(buffer);
}


//lim
static int failures;
static const char * addr_string (struct sockaddr *sa, char *buf, size_t size)
{
	if (sa == NULL)
		return "<none>";

	switch (sa->sa_family)
	{
		case AF_INET:
			return inet_ntop (AF_INET, &((struct sockaddr_in *) sa)->sin_addr, buf, size);
		case AF_INET6:
			return inet_ntop (AF_INET6, &((struct sockaddr_in6 *) sa)->sin6_addr, buf, size);
#ifdef AF_LINK 
		case AF_LINK:
			return "<link>";
#endif
		case AF_UNSPEC:
			return "---";
#ifdef AF_PACKET
		case AF_PACKET:
			return "<packet>";
#endif
		default:
			++failures;
			//printf ("sa_family=%d %08x\n", sa->sa_family,
			//*(int*)&((struct sockaddr_in *) sa)->sin_addr.s_addr);
			return "<unexpected sockaddr family>";
	}
}

//get net interface info
void getnetinfo(const char *netname, struct str_net_addr *net_info) //ccc
{
	struct ifaddrs *ifaces, *ifa;
	char abuf[64], mbuf[64], dbuf[64];

	if (getifaddrs (&ifaces) < 0) {
		printf ("Couldn't get any interfaces\n");
		if(ifaces) {
			freeifaddrs (ifaces);
		}
		return;
	}

	for (ifa = ifaces; ifa != NULL; ifa = ifa->ifa_next) {
		if(strcmp(ifa->ifa_name, netname)==0) {
			printf ("%-15s%#.4x  %-15s %-15s %-15s\n",
					ifa->ifa_name, ifa->ifa_flags,
					addr_string (ifa->ifa_addr, abuf, sizeof (abuf)),
					addr_string (ifa->ifa_netmask, mbuf, sizeof (mbuf)),
					addr_string (ifa->ifa_broadaddr, dbuf, sizeof (dbuf)));
			str2ip(abuf, net_info->ip);
			str2ip(mbuf, net_info->mask);
		}
	}
	freeifaddrs(ifaces);
}

extern int gSetGatewayWaitCount;
extern char DHCPIPAdress[18];
void connect_wifi(void)			//ccc
{
	if(gOptions.wifidhcpfunon) {     //set dhcp wifi
		int wififlag=0;
		do {
			FILE *fp=NULL;
			unsigned char buf[128];
			unsigned char buf1[256];
			char *p=NULL;
			char *pp=NULL;

			printf("dhcp rausb0...\n");
			sleep(5);       			//ccc
			systemEx("./udhcpc -i rausb0 -q -n");          //dhcp get wifi ip addr
			sync();

			memset(buf, 0x00, sizeof(buf));
			fp = fopen("dhcp.txt", "r");
			if (fp == NULL) {
				printf("err open dhcp.txt\n");
			} else {
				while(fgets((char*)buf, 128, fp))
				{
					if ((p = strstr((char*)buf, "ip="))!=NULL) {
						//保存DHCP动态分配的IP地址(liming)
						memset(DHCPIPAdress, 0, sizeof(DHCPIPAdress));
						memset(buf1,0,sizeof(buf1));

						pp=p;
						while(*pp) {
							if((*pp=='\n')||(*pp=='\r')||(*pp==' '))
								*pp=0;
							pp++;
						}
						sprintf(DHCPIPAdress, "%s", p+3);
						//printf("DHCP get IP:%s\n",DHCPIPAdress);
						sprintf((char*)buf1, "ifconfig rausb0 %s", p+3);
						systemEx((char*)buf1);
						wififlag=5;
					}

					if ((p = strstr((char*)buf, "subnet="))!=NULL) {
						memset(buf1,0,sizeof(buf1));
						//printf("%s", p+7);
						pp=p;
						while(*pp) {
							if ((*pp=='\n')||(*pp=='\r')||(*pp==' ')) {
								*pp=0;
							}
							pp++;
						}
						sprintf((char*)buf1, "ifconfig rausb0 netmask %s", p+7);
						systemEx((char*)buf1);
					}

					if((p = strstr((char*)buf, "router="))!=NULL) {
						memset(buf1,0,sizeof(buf1));
						pp=p;
						while(*pp) {
							if((*pp=='\n')||(*pp=='\r')||(*pp==' ')) {
								*pp=0;
							}
							pp++;
						}
						systemEx("route del default gw 0.0.0.0");
						sprintf((char*)buf1, "route add default gw %s", p+7);
						systemEx((char*)buf1);
					}
				}
				fclose(fp);
				getnetinfo("rausb0", &gwifiaddr);       //fresh wifi addr struct
			}
		} while(wififlag++<4);
	} else {           //set static wifi ip
		char tmp[16];
		memset(tmp, 0, sizeof(tmp));

		LoadStr("wifiip", tmp);
		str2ip(tmp, gwifiaddr.ip);
		memset(tmp,0,sizeof(tmp));
		LoadStr("wifimask", tmp);
		str2ip(tmp,gwifiaddr.mask);
		memset(tmp,0,sizeof(tmp));
		LoadStr("wifigateway", tmp);
		str2ip(tmp,gwifiaddr.gateway);

		if(gwifiaddr.ip[0]){     //if ip is valid
			SetNetIP_MASK("rausb0", gwifiaddr.ip, gwifiaddr.mask);
			SetGateway("add", gwifiaddr.gateway);
			getnetinfo("rausb0", &gwifiaddr);       //fresh wifi addr struct
		}
	}
}

void loadwificfg(const char * name, char *out)//ccc
{
	unsigned char cfg[256];
	unsigned char buff[256];

	memset(cfg, 0x00, sizeof(cfg));
	memset(buff,0,sizeof(buff));
	LoadStr(name, (char*)cfg);

	if ((cfg[0]==0)&&(strcmp(name, "WPAPSK")==0)) {
		return;
	}
	sprintf((char*)buff, "%s=%s\n",name, (char*)cfg);
	strcat(out, (char*)buff);
}

int  setwifipara(void)                                  //ccc
{
	char out[2048];
	FILE *fp;

	memset(out, 0x00, sizeof(out));
	loadwificfg("NetworkType",out);
	loadwificfg("SSID",  out);
	loadwificfg("AuthMode",  out);
	loadwificfg("EncrypType",out);
	loadwificfg("DefaultKeyID",  out);                                          //seletct whic  pass str
	loadwificfg("Key1Type",  out);
	loadwificfg("Key1Str",   out);
	loadwificfg("Key2Type",  out);
	loadwificfg("Key2Str",   out);
	loadwificfg("Key3Type",  out);
	loadwificfg("Key3Str",   out);
	loadwificfg("Key4Type",  out);
	loadwificfg("Key4Str",   out);
	loadwificfg("WPAPSK",    out);

	//printf("WiFi Parameter:%s", out);
	fp = fopen("/etc/rt73sta.dat", "a+");
	if(fp == NULL)
	{
		printf("err open file rt73sta.dat");
		return 0;
	}
	fwrite(out, strlen(out), 1, fp);
	fflush(fp);
	fclose(fp);
	return 1;
}

/*
   the input voltage range  is 0 - 5v
   if the value from ak4182a is 0 - 2048,
   the input voltage = (value/2048)*5
   */ 
#define  IOCTL_SET_MSG  0
#define  IOCTL_SET_NUM  1
#define IOCTL_READ_BAT 2

int battery_fd=-1;  // 文件句柄
unsigned char battery_state=0;  // 检测到电池变化时，为避免丢失消息，就多检测几次，另外，用于检测电池的最大值
unsigned char free_battery=0;

#define BAT_VOL1        1500 //1500
#define BAT_VOL2        1450 //1436
#define BAT_VOL3        1425 //1411
#define BAT_VOL4        1400 //1378
#define BAT_VOL5        900 //1378

#define DIFF(a,b) ((a>b)?(a-b):(b-a))

int check_battery_voltage(int *voltage)
{
	static int battery,val;
	static int v;
	static unsigned char count=0;

	if(battery_fd<0)
	{
#ifdef ZEM600
		battery_fd = open("/proc/power", O_RDWR);
#else
		battery_fd = open("/dev/ts",O_RDONLY);
#endif
	}
	if (battery_fd>0)
	{
#ifdef ZEM600
		lseek(battery_fd, 0, SEEK_SET);
		if(!read(battery_fd, &battery, 4))
#else
			if(ioctl(battery_fd, IOCTL_READ_BAT,&battery))
#endif
				printf("error\n");
			else
			{
				//printf("battery value %d \n", battery);
				if(battery>=0)
				{
					if(battery>(BAT_VOL1))
					{
						val=DIFF(BAT_VOL1,battery);
						v = 4;
					}
					else if(battery>BAT_VOL2)
					{
						val=DIFF(BAT_VOL2,battery);
						v = 3;
					}
					else if(battery>BAT_VOL3)
					{
						val=DIFF(BAT_VOL3,battery);
						v = 2;
					}
					else if(battery>BAT_VOL4)
					{
						val=DIFF(BAT_VOL4,battery);
						v = 1;
					}
					else if(battery <BAT_VOL5)
					{
						val=DIFF(BAT_VOL5,battery);
						v=-1;
						*voltage=v;
						return v;
					}
					else
					{
						v=0;
					}
				}
				//printf(" val %d v %d free_battery %d\n", val, v, free_battery);

				//cn 2009-03-02 解决向上跳动的问题
				if(free_battery==0)
				{
					if(v!=4)
						v+=1;
					free_battery=v;
					*voltage=v;
					count=7;
					return v;
				}

				// cn 2009－03－04 处于两个分界点时， 解决电压跳动的问题，
				if(free_battery != v && val>5)
				{
					if(count>10)
					{
						count=0;
						free_battery=v;
					}
					else
					{
						count++;
					}
				}
				else
				{
					count=0;
				}

				*voltage=free_battery;

				return free_battery;
			}
	}
	return -1;
}

//xsen add
void DecomposeStr(char *InputStr, char *delim, char (*OutStr)[],int size)
{
	char *token = NULL;
	char defaultdelim[] = ":";
	int count=0;
	if(!InputStr || !OutStr)
	{
		return ;
	}
	if(!delim)
	{
		delim = defaultdelim;
	}
	while((token = strsep(&InputStr, delim)) != NULL)
	{
		if(token != NULL)
		{
			strncpy(*OutStr+count, token, size);
			count=count+size;
		}
	}
}
//end

void set_wifi_rausb0_flag(int value)
{
	wifi_rausb0_flag=value;
}

void setMACAddress(void)
{
	char bf[128] = {0};
	//struct timeval tv;
	//struct timezone tz;
	TTime gTime;
	U8 rdm[6] = {0};
	U8 j=0;
	struct timeval tt;
	ReadRTCClockToSyncSys(&gTime);
	gettimeofday(&tt, NULL);
	srand(tt.tv_usec);
	//DBPRINTF("Auto Create MAC, T=%d\n", tt.tv_usec);
	for(j=1;j<6;j++)
	{
		rdm[j]=(U8)(1+(int)(255.0*rand()/(RAND_MAX+1.0)));/* return=0..255 random */
		gettimeofday(&tt, NULL);
		srand(tt.tv_usec);
		//DBPRINTF("T=%d\n", tt.tv_usec);
	}
	memset(bf, 0, sizeof(bf));
	sprintf(bf,"ifconfig eth0 down && ifconfig eth0 hw ether %02X:%02X:%02X:%02X:%02X:%02X", rdm[0],rdm[1],rdm[2],rdm[3],rdm[4],rdm[5]);
	printf("%s\n", bf);
	systemEx(bf);
	systemEx("ifconfig eth0 up");
}

/*dsl 2012.4.17*/
int _eval(char *const argv[], char *path, int timeout, int *ppid)
{
	pid_t pid;
	int status;
	int fd;
	int flags;
	int sig;
	char buf[254];
	int i;

	memset((void*)buf, 0, sizeof(buf));
	switch (pid = fork()) {
		case -1:
			perror("fork");
			return errno;
		case 0:
			for (sig = 0; sig < (_NSIG-1); sig++) {
				signal(sig, SIG_DFL);
			}

			ioctl(0, TIOCNOTTY, 0);
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			setsid();

			if ((fd = open("/dev/console", O_RDWR)) < 0) {
				(void) open("/dev/null", O_RDONLY);
				(void) open("/dev/null", O_WRONLY);
				(void) open("/dev/null", O_WRONLY);
			} else {
				close(fd);
				(void) open("/dev/console", O_RDONLY);
				(void) open("/dev/console", O_WRONLY);
				(void) open("/dev/console", O_WRONLY);
			}
			if (path) {
				flags = O_WRONLY | O_CREAT;
				if (!strncmp(path, ">>", 2)) {
					flags |= O_APPEND;
					path += 2;
				} else if (!strncmp(path, ">", 1)) {
					flags |= O_TRUNC;
					path += 1;
				}
				if ((fd = open(path, flags, 0644)) < 0)
					perror(path);
				else {
					dup2(fd, STDOUT_FILENO);
					close(fd);
				}
			}

			for(i=0 ; argv[i] ; i++) {
				snprintf(buf+strlen(buf), sizeof(buf), "%s ", argv[i]);
			}
			//dprintf("cmd=[%s]\n", buf);
			setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
			alarm(timeout);
			execvp(argv[0], argv);
			perror(argv[0]);
			exit(errno);

		default:
			if (ppid) {
				*ppid = pid;
				return 0;
			} else {
				waitpid(pid, &status, 0);
				if (WIFEXITED(status)) {
					return WEXITSTATUS(status);
				} else {
					return status;
				}
			}
	}
}

int kill_pidfile(char *pidfile)
{
	FILE *fp = fopen(pidfile, "r");
	char buf[256];

	memset((void*)buf, 0, sizeof(buf));
	if (fp && fgets(buf, sizeof(buf), fp)) {
		pid_t pid = strtoul(buf, NULL, 0);
		fclose(fp);
		return kill(pid, SIGTERM);
	} else {
		return errno;
	}
}

unsigned int hashBufferLog(int fd, unsigned char *key, int keySize)
{
	unsigned char *ptr=(unsigned char *)key;
	unsigned int val=0;
	unsigned int cnt=0;
	int len = 2048;
	unsigned char *pBuf = malloc(len);
	if (pBuf == NULL) {
		return 0;
	}

	while (keySize--) {
		int tmp;
		val = (val << 4) + (*ptr);
		if ((tmp = (val & 0xf0000000))!=0) {
			val = val ^ (tmp >> 24);
			val = val ^ tmp;
		}
		ptr++;
	}

	keySize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	if (keySize >= len) {
		read(fd, pBuf, len);
	} else {
		read(fd, pBuf, keySize);
	}

	while (keySize--) {
		int tmp;
		val = (val << 4) + pBuf[cnt];
		if ((tmp = (val & 0xf0000000))!=0) {
			val = val ^ (tmp >> 24);
			val = val ^ tmp;
		}

		//pBuf[cnt++];
		cnt++;
		if (cnt == len)
		{
			cnt = 0;
			if (keySize >= len) {
				read(fd, pBuf, len);
			} else {
				read(fd, pBuf, keySize);
			}
		}
	}

	free(pBuf);
	return val % (PRIME_TBLSIZ-1);
}

