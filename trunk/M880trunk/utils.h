/*************************************************
                                           
 ZEM 200                                          
                                                    
 utils.h 
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _utils_h
#define _utils_h

#include "stdarg.h"
#include "arca.h"

#define BYTE2M (2*1024*1024)

int systemEx(char * CmdString);

U32 GetTickCount1(void);
U32 GetUS();
char *GetEnvFilePath(const char *EnvName, const char *filename, char *fullfilename);
void SetMACAddress(unsigned char *MAC);
void SetIPAddress(char *Action, unsigned char *ipaddress);
void RebootMachine(void);
BOOL SetGateway(char *Action, unsigned char *ipaddress);
void SetNetworkPara(BYTE *ipaddress, BYTE *netmask, BYTE *gateway);
void SetNetworkIP_MASK(BYTE *ipaddress, BYTE *netmask);

int getnetisalive(const char *netname);

int Decode16(char *String, char* Data);
char *ConvertMonth(char *version, char *iversion);

void doPrint(char * buffer, char * fmt, va_list ap);

int Hex2Char(char *s);

char *nstrcpy(char *Dest, const char *Source, int size);
int nstrcmp(const char *s1, const char *s2, int size);
BYTE *nmemcpy(BYTE *Dest, const BYTE *Source, int size);
int nmemcmp(const BYTE *s1, const BYTE *s2, int size);
int nmemset(BYTE *Dest, BYTE Data, int Size);

char* SPadCenterStr(char *buf, int size, const char *s);
char* SPadRightStr(char *buf, int size, const char *s);
char *Pad0Num(char *buf, int size, int value);
char *Pad0Str(char *buf, int size, char *value);
char *TrimRightStr(char *buf);
char *TrimLeftStr(char *buf);
char *TrimLeft0(char *buf);
char *TrimStr(char *buf);

int SearchIndex(char **Items, const char *Text, int ItemCount);

//--------字符串转换函数-------------
//字符串str转换成无符号整数*value，成功返回0，失败返回非零
int strtou32(const char *str, unsigned int *value);
//字符串转换成IP地址
int str2ip(char* str, BYTE* ip);
//字符串转换成MAC地址
int str2mac(char* str, BYTE* mac);
int str2int(char *buf, int DefaultValue);
//16进制字符串str转换成无符号整数*value，成功返回0，失败返回非零
int HexStrToInt(const char *str, int *value);
//字符串p转换成无符号整数返回，Next返回p中数值结束的地址
int StrValue(const char *p, int *Next);
//16进制字符串p转换成无符号整数返回，Next返回p中数值结束的地址
int HexStrValue(const char *p, int *Next);

//-- “：” 分隔串处理函数-----------
//从p中复制第index个值到buf中,返回复制的字符串的长度
int SCopyStrFrom(char *buf, char *p, int index);
//把整数列表变成字符串，InvalidInt表示要去掉的整数，PackInvalidInt表示是否压缩多余的分隔符号
int SaveIntList(char *buf, int *INTs, int Count, int InvalidIntO_RDONLY, int PackInvalidInt);
//去掉字符串中开头、结尾和重复的分隔符号，并返回处理后的长度
int SPackStr(char *buf);
//计算p中被分割的字符串的个数
int SCountStr(char *p);
//返回p中第index个值的整数形式，并在Next中返回该数值的结束地址
int SIntValueFrom(char *p, int index, int *Next);
//返回p中第index个值（16进值）的整数形式，并在Next中返回该数值的结束地址O_RDONLY
int SHexValueFrom(char *p, int index, int *Next);

//编码固定长度的字符串成为0结束的字符串，返回编码后的长度
void StringEncode(char *buf, char *str, int size);
//解码0结束的字符串为固定长度字符串，返回解码后的长度
int StringDecode(char *buf, char *str);
//按16进制字符编码
int EncodeHex(BYTE *String, BYTE *Data, int Size);


void SetBit(BYTE *Buffer, int Index);
void ClearBit(BYTE *Buffer, int Index);
int GetBit(BYTE *Buffer, int Index);
void memor(char *s1, char *s2, int len);

BOOL SaveMACToFlash(const char *MAC);
void RedBootMac(const char *serialnumber);
void SetRootPwd(int comkey);

int GetSensorSerialNumber(char *TitleStr, char *IDStr);
//void SaveMACAddr(void);

void SyncTimeByTimeServer(BYTE *TimeServerIP);

int str2cardkey(char *str, BYTE* cardkey);

void ExportProxySetting(void);

BOOL SaveLOGOToFlash(int offset, char *config, int size);

void SetGSMCDMAPara(int ModemType);

typedef struct {
    unsigned char *buffer;
    int bufferSize;
    unsigned char *bufPtr, *bufEnd;
	int isRom;
}TBuffer;


TBuffer *createRomBuffer(unsigned int Address, int size);
TBuffer *createRamBuffer(int size);
TBuffer *createFileBuffer(const char *fileName);
void freeBuffer(TBuffer *buffer);
unsigned char readByte(TBuffer *buffer);
unsigned short readWord(TBuffer *buffer);
unsigned int readDWord(TBuffer *buffer);
int offsetOfBuffer(TBuffer *buffer);
int resOfBuffer(TBuffer *buffer);
int seekBuffer(TBuffer *buffer, int position);
int seekSetBuffer(TBuffer *buffer, int position);
unsigned int hashBuffer(TBuffer *buffer);
int  setwifipara(void);
int setup_auth_dns(void);
int check_battery_voltage(int *voltage);
#ifdef ZEM600
BOOL SetScreenAddressToFlash(const char *Rote);
#else
BOOL SetScreenAddressToFlash(void);
#endif
void DecomposeStr(char *InputStr, char *delim, char (*OutStr)[],int size);
void set_wifi_rausb0_flag(int value);

/*dsl 2012.4.19*/
int kill_pidfile(char *pidfile);
int _eval(char *const argv[], char *path, int timeout, int *ppid);
unsigned int hashBufferLog(int fd, unsigned char *key, int keySize);
void SetDNServerAddress(unsigned char *DNServeraddress);
BOOL SetScreenTypeToFlash(const char *LCDType);
#endif

