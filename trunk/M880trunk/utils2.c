#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>


#include "utils2.h"
#include "strutils.h"
#include "minilzo.h"

int DumpData(BYTE *data, int dataSize)
{
	static char Hex[17]="0123456789ABCDEF";
	int i, linec=0;
	char line[20];
	printf("Dump Data: Size=%d\n", dataSize);
	for(i=0; i<dataSize;i++)
	{
		char HexData[3];
		HexData[0]=Hex[data[i]>>4]; HexData[1]=Hex[data[i]&0x0F]; HexData[2]=0;
		if(linec==0) printf("  ");
		printf("%s ", HexData);
		if(data[i]>=0x20 && data[i]<0xFF)
			line[linec]=data[i];
		else
			line[linec]='.';
		if(++linec>=16)
		{
			line[linec]=0;
			printf("  %s\n", line);
			linec=0;
		}
	}
	if(linec)
	{
		for(i=0;i<16-linec;i++) printf("   ");
		line[linec]=0;
		printf("  %s\n",line);
	}
	return 0;
}

int writeBuffer(TBuffer *buffer, char *data, int size)
{
	if(size>resOfBuffer(buffer)) size=resOfBuffer(buffer);
	memcpy(buffer->bufPtr, data, size);
	buffer->bufPtr+=size;
	return size;
}

int writeBufferAuto(TBuffer *buffer, char *data, int size)
{
	int len=size-resOfBuffer(buffer);
	if(len>0)
	{
		if(len<10*1024) len=10*1024;
		len+=buffer->bufferSize;
		if(resizeRamBuffer(buffer, len)!=len) return 0;
	}
    memcpy(buffer->bufPtr, data, size);
	buffer->bufPtr+=size;
	return size;
}

int insertBufferAuto(TBuffer *buffer, char *data, int size, int offset)
{
	unsigned char *p;
	int i,len=size-resOfBuffer(buffer);
	if(len>0)
	{
		if(len<10*1024) len=10*1024;
		len+=buffer->bufferSize;
		if(resizeRamBuffer(buffer, len)!=len) return 0;
	}
	p=buffer->bufPtr+size-1;
	for(i=buffer->bufPtr-buffer->buffer-offset;i>0;i--){ *p=*(p-size); p--;}
    memcpy(buffer->buffer+offset, data, size);
	buffer->bufPtr+=size;
	return size;
}

int resizeRamBuffer(TBuffer *buffer, int newSize)
{
	int i=buffer->bufPtr-buffer->buffer;
	BYTE *p;
	if(buffer->isRom) return 0;
	p=(BYTE *)REALLOC(buffer->buffer, newSize);
	if(p==NULL) return 0;
	buffer->buffer=p;
	buffer->bufferSize=newSize;
	buffer->bufEnd=buffer->buffer+newSize;
	buffer->bufPtr=buffer->buffer+i;
	return newSize;
}

int resOfBuffer(TBuffer *buffer)
{
	return buffer->bufEnd-buffer->bufPtr;
}

void StringEncode(char *buf, char *str, int size)
{
	int base64_encode(const unsigned char *in,  unsigned long len,unsigned char *out);
	base64_encode((unsigned char *)str, size,(unsigned char *)buf);
}

int StringDecode(char *buf, char *str)
{
	int base64_decode(const unsigned char *in, unsigned char *out);
	return base64_decode((unsigned char *)str,(unsigned char *)buf);
}

void xorEncode2(char *buf, int size)
{
	int i;
	for(i=0;i<size;i++)
	{
		buf[i]^=(char)(i%128);
	}
}

void xorEncode(char *buf, int size, char *key, int keySize)
{
	int i,j=0;
	if(key && keySize)
	for(i=0; i<size; i++)
	{
		buf[i]^=key[j++];
		if(j>=keySize) j=0;
	}
}

int bufferEncrypt(char *buf, int size, char *target, int targetSize, char *key, int keySize)
{
	char tmp[LZO1X_MEM_COMPRESS];
	int ret;
	lzo_uint retSize=targetSize;
	xorEncode(buf, size, key, keySize);
	ret=lzo1x_1_compress((lzo_byte*)buf, size, (lzo_byte*)target, &retSize, tmp);
	if(ret==LZO_E_OK)
	{
		xorEncode2(target, retSize);
//		printf("Encrpt(%d->%d),key=%s(%d): %02X%02X...%02X%02X ->  %02X%02X...%02X%02X\n",size,retSize,key,keySize,
//				buf[0],buf[1],buf[size-2],buf[size-1], target[0], target[1], target[retSize-2], target[retSize-1]);
//		if(retSize<256) DumpData(target, retSize);
		return retSize;
	}
	return ret;
}

int bufferDecrypt(char *buf, int size, char *target, int targetSize, char *key, int keySize)
{
	int ret, retSize=targetSize;
	xorEncode2(buf, size);
	ret=lzo1x_decompress_safe((lzo_byte*)buf, size, (lzo_byte*)target, (lzo_uintp)&retSize, NULL);
	if(ret==LZO_E_OK)
	{
		xorEncode(target, retSize, key, keySize);
		return retSize;
	}
	return ret;
}

char *fileDecrypt(char *fname, int *size, char *key, int keySize)
{
	char *buffer, *fbuf;
	int flen, blen, ret;
	FILE *f=fopen(fname,"rb");
	*size=0;
	if(f==NULL) return NULL;
	fseek(f, 0, SEEK_END);
	flen=ftell(f);
	fseek(f, 0, SEEK_SET);
	if(flen>=8)
	{
		fread(&blen, 4, 1, f);
		if(memcmp(&blen,"LZO1",4)!=0)
		{
			fclose(f);
			return NULL;
		}
		fread(&blen, 4, 1, f);
	}
	flen-=8;
	if(flen<=0 || blen>flen*500 || blen>5*1024*1024)
	{
		fclose(f);
		return NULL;
	}
	buffer=(char*)MALLOC(blen);
	fbuf=(char*)MALLOC(flen);
	fread(fbuf, flen, 1, f);
	fclose(f);
	ret=bufferDecrypt(fbuf, flen, buffer, blen, key, keySize);
	FREE(fbuf);
	if(ret==blen)
	{
		*size=ret;
		return buffer;
	}
	FREE(buffer);
	return NULL;
}

char *loadFileMem(char *fname, int *fsize, int encrypt, char *key, int keySize)
{
	int flen;
	FILE *f;
	char *buffer;
	if(encrypt==1)
		return fileDecrypt(fname, fsize, key, keySize);
	f=fopen(fname, "rb");
	if(f==NULL) return NULL;
	fseek(f, 0, SEEK_END);
	flen=ftell(f);
	*fsize=flen;
	fseek(f, 0, SEEK_SET);
	buffer=(char*)MALLOC(flen+2);
	if(buffer)
	{
		memset(buffer,0,flen+2);
		fread(buffer, flen, 1, f);
		fclose(f);
		return buffer;
	}
	return NULL;
}

void closeStream(FILE *iSvrResp)
{
	if(iSvrResp==NULL) return;
	while(1)
	{
		int ch=fgetc(iSvrResp);
		if(ch!=EOF)
			putchar(ch);
		else
			break;
	}
	fclose(iSvrResp);
}

void msleep(int msec)
{
	struct timeval delay;
	delay.tv_sec =msec /1000;
	delay.tv_usec =(msec%1000)*1000;
	select(0, NULL, NULL, NULL, &delay);
}

int numberOfAvialible(int fd)
{
	int chars=0;
	int ret=ioctl(fd, FIONREAD, &chars);
	if(ret==0 && chars>=0)
		return chars;
	printf("error of ioctl:%d\n", ret);
	return -10000;
}

int avialible(int fd, int timeout)
{

	fd_set set;
	struct timeval tm;
	int ret=numberOfAvialible(fd);
	if(timeout==0 || ret!=0) return ret;
	tm.tv_sec  = timeout/1000;
	timeout%=1000;
	tm.tv_usec = timeout*1000;
	FD_ZERO(&set);
	FD_SET(fd, &set);
	ret=select(fd+1, &set, NULL, NULL, &tm);
	if(ret==-1)
	{
		//perror("select()");
		return -1;
	}
	if (FD_ISSET(fd, &set))
	{
		return numberOfAvialible(fd);
	}
	//printf("timeout avialible\n");
	return -2;
}

int fAvialible(FILE *f, int timeout)
{
	return avialible(fileno(f), timeout);
}

int fetchExec(char *cmd, char *buf, int len)
{
	int i, ch;
	char *start=buf;
	FILE *pipe=popen(cmd, "r");
	if(pipe==NULL)
	{
		printf("Error open \"%s\"\n", cmd);
		return -1;
	}
	for (i = 0; i < len; i++)
	{
		if ((ch = fgetc(pipe)) == EOF)
		{
			break;
		}
		if(ch=='\r')
		{
			*buf++='\\';
			*buf++='r';
			i++;
		}
		else if(ch=='\n')
		{
			*buf++='\\';
			*buf++='n';
			i++;
		}
		else
			*buf++ = ch;
	}
	*buf=0;
	pclose(pipe);
	return buf-start;
}

int encodeReq(PReq req, char *target)
{
	int ret=sprintf(target, 
		"ReqID=%d\nURL=%s\nCT=%s\nCookies=%s\nMethod=%s\nCL=%d\n",
		req->ReqID, req->URL, req->ContentType, req->Cookies, req->Method, req->ContentSize);
	if(req->Content)
	{
		ret+=sprintf(target+ret, "Content=");
		ret+=encodeLine(target+ret, req->Content);
	}
	return ret;
}

int decodeReq(const char *reqStr, char *content, PReq req)
{
	PStrList reqList=slCreate("");
	char *p;
	memset(req, 0, sizeof(TReq));
	slSetText(reqList, reqStr, "\n");
//	logMsg("ReqLine Count=%d,",reqList->count);
//	logMsg("'ReqID',");
	req->ReqID=slGetValueInt(reqList, "ReqID", -1);
	p=slGetValue(reqList, "URL");
	if(p) strcpy(req->URL, p);
	p=slGetValue(reqList, "CT");
	if(p) strcpy(req->ContentType, p);
	p=slGetValue(reqList, "Cookies");
	if(p) decodeLine(p, req->Cookies);
	req->Content=slGetValue(reqList, "Content");
	p=slGetValue(reqList, "CL");
	if(p) req->ContentSize=atoi(p);
	if(req->Content)
	{
		if(content==NULL)
			content=(char *)reqStr;
		req->ContentSize=decodeLine(req->Content, content);
		req->Content=content;
//		logMsg("%d bytes. Finish.\n", req->ContentSize);
	}
	slFree(reqList);
	return 1;
}

PStrList queryStrToList(const char *queryStr)
{
	PStrList res=slCreate("");
	if(queryStr)
	{
		int i;
		PStrList content=slCreate("");
		slSetText(content, queryStr, "&");
		for(i=0;i<content->count;i++)
		if(content->strs[i])
			slAdd(res, decodeQueryStr(content->strs[i]));
		slFree(content);
	}
	return res;
}

int delFileHead(int fd, int size)
{
	int tail=size;
	int head=0;
	int block=4*1024;
	int len = 0;
	char buf[4*1024];

	memset((void*)buf, 0, sizeof(buf));
	while(1) {
		if(lseek(fd, tail, SEEK_SET)==tail) {
			len=read(fd, buf, block);
			if(len>0) {
				lseek(fd, head, SEEK_SET);
				write(fd, buf, len);
				head+=len;
				tail+=len;
			}
			if(len<block) {
				printf("READ OVER head=%d, tail=%d, len=%d\n", head, tail, len);
				break;
			}
		} else {
			printf("SEEK TAIL ERROR: head=%d, tail=%d\n", head, tail);
			break;
		}
	}
	if (head > 0) {
		ftruncate(fd, head);
		return head;
	}
	return 0;
}
