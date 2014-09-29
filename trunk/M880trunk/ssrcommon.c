#include <minigui/common.h>
#include "ssrcommon.h"
#include "main.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"
#include "libdlcl.h"

RECT gMainMenu_rc={1,1,319,239};

char *BauteStr[]= { "9600","19200","38400","57600","115200" };
char *DtFmtStr[]={"YY-MM-DD","YY/MM/DD","YY.MM.DD", "MM-DD-YY","MM/DD/YY","MM.DD.YY","DD-MM-YY","DD/MM/YY","DD.MM.YY","YYYYMMDD"};

int TestItem, TestFlage = -1;

/*
 * -1, open emfw.cfg fail
 * -2, incorrect check sum of emfw.cfg 
 * -3, the incorrect firmware version
 * -4, emfw.cfg used the incorrect encryption method
 * -5, system function return incorrect value
 * -6, upgrade parameters fail
 * -7, the memory not enough
 *  */
static int gUpdateFirmwareErrorNo = 0;

char *GetAttName(int attstate)
{
	switch(attstate)
	{
		case 0:
			return LoadStrByID(HIT_SYSTEM5KEY1);
		case 1:
			return LoadStrByID(HIT_SYSTEM5KEY2);
		case 2:
			return LoadStrByID(HIT_SYSTEM5KEY3);
		case 3:
			return LoadStrByID(HIT_SYSTEM5KEY4);
		case 4:
			return LoadStrByID(HIT_SYSTEM5KEY5);
		case 5:
			return LoadStrByID(HIT_SYSTEM5KEY6);
		default:
			return NULL;
	}
}


long GetKeyState(int i)
{
	if(i==0) return SCANCODE_TAB;
	else if(i==1) return SCANCODE_BACKSPACE;
	else if(i==2) return SCANCODE_CURSORBLOCKUP;
	else if(i==3) return SCANCODE_CURSORBLOCKDOWN;
	else if(i==4) return SCANCODE_CURSORBLOCKLEFT;
	else if(i==5) return SCANCODE_CURSORBLOCKRIGHT;
	else if(i==6) return -1;
	return 0;
}

void GetKeyStateName(short i,char *info)
{
	if(i<6) {
		strcpy(info,GetAttName(i));
	} else {
		strcpy(info,LoadStrByID(HID_WELCOME));
	}
}


void InfoShowStr(HDC hdc,char *name,char *text,int x,int y)
{
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		TabbedTextOut(hdc,x,y+5,text);
		TabbedTextOut(hdc,x+160,y+5,name);
	}
	else
	{
		TabbedTextOut(hdc,x,y+5,name);
		TabbedTextOut(hdc,x+80,y+5,text);
	}
}

void InfoShow(HDC hdc,char *name,long num,int x,int y,int sd)
{
	char text[256];
	int tmpvalue =0;
	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);

	SetTextColor(hdc,0x00000000);
	memset(text, 0, sizeof(text));
	if(num>=0) {
		sprintf(text,"%ld",num);
	} else {
		text[0]=0;
	}

	if (fromRight==1) { //modify by jazzy 2008.07.24
		printf("text:%s len:%d\n",text,strlen(text));
		if(num>=0) {
			TabbedTextOut(hdc,x,y,text);
		}
		if(name) {
			TabbedTextOut(hdc,x+sd,y,name);
		}
	} else {
		if(name) {
			TabbedTextOut(hdc,x,y,name);
		}
		if(num>=0) {
			TabbedTextOut(hdc,x+sd,y,text);
		}
	}
}

long CheckText(char *str)
{
	char *p=str;

	if(strlen(p)<=0) return 0;
	while(*p)
	{
		if(*p>='0'&&*p<='9') ++p;
		else return 0;
	}
	return 1;
}

long CheckText2(char *str,int ul,int dl)
{
	if(CheckText(str))
	{
		if(atol(str)>=ul&&atol(str)<=dl)
			return 1;
		else
			return 0;
	}	
	else return 0;
}

long CheckIP(char *c1,char *c2,char *c3,char *c4,int fag)
{
	if(CheckText2(c1,fag,fag==1?223:255)&&CheckText2(c2,0,255)&&CheckText2(c3,0,255)&&CheckText2(c4,0,255))	
	{
		return 1;
	}
	else return 0;
}

long ParseIP(char *buffer,char *c1,char *c2,char *c3,char *c4)
{
	char *p=buffer;
	char *pp;
	int err=0;

	*c1=*c2=*c3=*c4=0;

	pp=c1;
	while(*p>='0'&&*p<='9') *pp++=*p++;
	*pp=0;

	if(*p=='.') ++p;else ++err;

	pp=c2;
	while(*p>='0'&&*p<='9') *pp++=*p++;
	*pp=0;
	if(*p=='.') ++p;else ++err;

	pp=c3;
	while(*p>='0'&&*p<='9') *pp++=*p++;
	*pp=0;
	if(*p=='.') ++p;else ++err;

	pp=c4;
	while(*p>='0'&&*p<='9') *pp++=*p++;
	*pp=0;
	if(err)
		return 0;
	return 1;
}

#define CLOCKSPERSEC 1000000l

unsigned long Delay(unsigned long sec)
{
	volatile unsigned long base=0,m=0,n=0;
	while(base<CLOCKSPERSEC*sec)
	{
		base=m+n;
		++m;
	}
	return 0;
}

char *LoadFirmware(char *FirmwareFile, char *Version, int *Length)
{
	FILE *fh;
	char line[4000], linename[20];
	int i=0, dataline=0, checksum=0, position=0;
	char *fdata=NULL;
	char platform[60];
	char platformvalue[20];

	memset(platform,0,sizeof(platform));
	memset(platformvalue,0,sizeof(platformvalue));

	if (LoadStr(PLATFORM,platformvalue))
		sprintf(platform,"%s%s",platformvalue,"_FirmwareVersion=");
	else
		sprintf(platform,"%s%s","ZEM200","_FirmwareVersion=");

	gUpdateFirmwareErrorNo = 0;

	fh=fopen(FirmwareFile, "r");
	if(fh!=NULL)
	{
		sprintf(linename,"Data%d=",dataline);
		while(fgets(line, 4000, fh))
		{
			if((i==0)&&(strncmp(line,platform, strlen(platform))==0))
			{
				strcpy(Version, line+strlen(platform));
			}
			else if(strncmp(line, "FirmwareLength=",strlen("FirmwareLength="))==0)
			{
				*Length=atoi(line+strlen("FirmwareLength="));
				fdata=(char*)malloc(4000+*Length);
				if (fdata == NULL) {
					gUpdateFirmwareErrorNo = -7;
					fclose(fh);
					return NULL;
				}
				i++;
			}
			else if(strncmp(line, "FirmwareCheckSum=",strlen("FirmwareCheckSum="))==0)
			{
				checksum=atoi(line+strlen("FirmwareCheckSum="));
				i++;
			}
			else if((i==2) && (strncmp(line, linename, strlen(linename))==0))
			{
				line[strlen(line)-1]='\0';
				position+=Decode16(line+strlen(linename), fdata+position);
				sprintf(linename,"Data%d=",++dataline);
				if(position==*Length) {
					break;
				}
			}
		}
		fclose(fh);
	} else {
		/*open emfw.cfg fail*/
		gUpdateFirmwareErrorNo = -1;
	}

	if((position > 0) && (position==*Length))
	{
		if(in_chksum((unsigned char*)fdata, position)==checksum) {
			return fdata;
		} else {
			/*checksum fail*/
			gUpdateFirmwareErrorNo = -2;
		}
	}
	if(fdata) {
		free(fdata);
		fdata = NULL;
	}
	return NULL;
}

int UpdateOptionsByUSB(int surfd, int destfd)
{
	char buffer[VALUE_BUFFERLEN];
	int size=0;

	if ((surfd > 0) && (destfd > 0))
	{
		lseek(surfd, 0, SEEK_SET);
		lseek(destfd, 0, SEEK_END);
		while(TRUE)
		{
			memset(buffer, 0, sizeof(buffer));
			if(ReadOneLine(surfd, buffer, &size))
			{
				sprintf(buffer, "%s\n", buffer);
				write(destfd, buffer, strlen(buffer));
			}
			else
			{
				break;
			}
		}
		fsync(destfd);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int UpdateFirmware(char *filename)
{
	int fd;
	char *filebuf;
	char buf[128], version[128], sFirmwareFiles[128];
	int filebuflen;
	BOOL res=FALSE;
	char iMainVersion[64], iFWVersion[64];

	memset(buf,0,sizeof(buf));
	memset(iFWVersion,0,sizeof(iFWVersion));
	sprintf(buf, "%s/%s", USB_MOUNTPOINT, filename);
	if((filebuf=LoadFirmware(buf, version, &filebuflen))!=NULL)
	{
		if(strcmp(ConvertMonth(version, iFWVersion), ConvertMonth(MAINVERSION, iMainVersion))>=0) {
			if((((BYTE *)filebuf)[0]==0x1b)&&(((BYTE *)filebuf)[1]==0x55)) {
				sprintf(buf,"%supdate.tgz",RAMPATH);
				fd=open(buf, O_CREAT|O_WRONLY|O_TRUNC|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
				if (fd>0) {
					printf("ExtractPakage \n");
					zkfp_ExtractPackage(filebuf,NULL,(int*)filebuflen);
					write(fd, filebuf, filebuflen);
					close(fd);
					
					if (!useSDCardFlag) {
						sprintf(sFirmwareFiles, "tar xvzf %s -C %s && sync && rm %s -rf",buf,"/mnt/mtdblock/",buf);
					} else {
						sprintf(sFirmwareFiles, "tar xvzf %s -C %s && sync && rm %s -rf",buf,"/mnt/sdcard/",buf);
					}

					res = (systemEx(sFirmwareFiles)==EXIT_SUCCESS);

					if (!res) {
						gUpdateFirmwareErrorNo = -5;
						printf("system failed====\n");
					}
				}
			} else {
				/*emfw.cfg used the incorrect encryption method */
				gUpdateFirmwareErrorNo = -4;
			}
		} else {
			/*the incorrect firmware version*/
			gUpdateFirmwareErrorNo = -3;
			printf("update firmeare: incorrect Date version\n");
		}
		free(filebuf);
		filebuf = NULL;
	}

	if (res)
	{
		char surPath[80], desPath[80];
		int surfd=-1, desfd=-1;

		memset(surPath, 0, sizeof(surPath));
		memset(desPath, 0, sizeof(desPath));
		strcpy(surPath, "/mnt/mtdblock/upoptions.cfg");
		strcpy(desPath, "/mnt/mtdblock/options.cfg");

		surfd = open(surPath, O_RDONLY);
		if (surfd>0) {
			desfd = open(desPath, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
			if (desfd) {
				res = UpdateOptionsByUSB(surfd, desfd);
				close(desfd);
			}
			close(surfd);

			memset(buf, 0, sizeof(buf));
			sprintf(buf, "rm -rf %s && sync", surPath);
			systemEx(buf);
		}
		if (!res) {
			/*update parameters fail*/
			gUpdateFirmwareErrorNo = -6;
		}
	}
	return res;
}


void RESETDEFAULT(HWND hWnd)
{
	//printf("Reset default success!\n");
	SaveOptions(GetDefaultOptions(&gOptions));
	ClearOptionItem("NONE");
	FDB_AddOPLog(ADMINPIN, OP_RES_OPTION, 0, 0, 0, 0);
	if(gOptions.ShowState && gOptions.TFTKeyLayout==3)
	{
		FDB_InitShortKey();
	}
	MessageBox1 (hWnd ,LoadStrByID(HIT_UPDATEINFO) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
}

void SetMenuTimeOut(time_t timer)
{
	busyflag=0;
	if(timer) {
		gMenuTimeOut=timer;
	} else {
		gMenuTimeOut=time(NULL);
	}
}

void SetDHCPStr(void)
{
	strcpy(dhcpstr[0], LoadStrByID(MID_WIFI_MANUIP));
	strcpy(dhcpstr[1], "DHCP");
}

void SetFreeSetStr(void)
{
	strcpy(FreeSetStr[0],LoadStrByID(HIT_FREESET1));
	strcpy(FreeSetStr[1],LoadStrByID(HIT_FREESET2));
}

void SetSwitchStr(void)
{
	strcpy(SwitchStr[0],LoadStrByID(HIT_SWITCH2));
	strcpy(SwitchStr[1],LoadStrByID(HIT_SWITCH1));
}

void SetSpeedStr(void)
{
	strcpy(SpeedStr[0],LoadStrByID(HIT_SPEED1));
	strcpy(SpeedStr[1],LoadStrByID(HIT_SPEED2));
	strcpy(SpeedStr[2],LoadStrByID(HIT_SPEED3));
}

void SetKeyStr(void)
{
	strcpy(KeyStr[0],LoadStrByID(HIT_KEY1));
	strcpy(KeyStr[1],LoadStrByID(HIT_KEY2));
	strcpy(KeyStr[2],LoadStrByID(HIT_KEY3));
	strcpy(KeyStr[3],LoadStrByID(HIT_KEY4));
	strcpy(KeyStr[4],LoadStrByID(HIT_KEY5));
	strcpy(KeyStr[5],LoadStrByID(HIT_KEY6));
	strcpy(KeyStr[6],LoadStrByID(HIT_NULLKEY));
}

void SetStateStr(void)
{
	strcpy(StateStr[0],LoadStrByID(HIT_SYSTEM5KEY1));
	strcpy(StateStr[1],LoadStrByID(HIT_SYSTEM5KEY2));
	strcpy(StateStr[2],LoadStrByID(HIT_SYSTEM5KEY3));
	strcpy(StateStr[3],LoadStrByID(HIT_SYSTEM5KEY4));
	strcpy(StateStr[4],LoadStrByID(HIT_SYSTEM5KEY5));
	strcpy(StateStr[5],LoadStrByID(HIT_SYSTEM5KEY6));
}


int GetUpdatingFirmeareErrorNo(void)
{
	return gUpdateFirmwareErrorNo;
}
