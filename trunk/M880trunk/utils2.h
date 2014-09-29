#ifndef __UTILS2_H__
#define __UTILS2_H__

#include "utils.h"

#define bufferSize(p)	((p)->bufPtr-(p)->buffer)
#define clearBuffer(p)	((p)->bufPtr=(p)->buffer)

int writeBuffer(TBuffer *buffer, char *data, int size);

int writeBufferAuto(TBuffer *buffer, char *data, int size);

int insertBufferAuto(TBuffer *buffer, char *data, int size, int offset);

int resizeRamBuffer(TBuffer *buffer, int newSize);

int resOfBuffer(TBuffer *buffer);

void closeStream(FILE *iSvrResp);

void msleep(int msec);

int avialible(int fd, int timeout);

int fAvialible(FILE *f, int timeout);

void xorEncode(char *buf, int size, char *key, int keySize);

int bufferEncrypt(char *buf, int size, char *target, int targetSize, char *key, int keySize);

int bufferDecrypt(char *buf, int size, char *target, int targetSize, char *key, int keySize);

char *fileDecryptSelf(char *fname, char *key, int keySize);

char *loadFileMem(char *fname, int *fsize, int encrypt, char *key, int keySize);

#define loadFile(f, size) loadFileMem(f, size, 0, NULL, 0);

#define MaxHeaderLength (4096*2)
#define MaxURLLength	(1024*2)
#include "strlist.h"
typedef struct _Req_
{
	int ReqID;
	char Method[8];
	char ContentType[100];
	char URL[MaxURLLength];
	char Cookies[MaxHeaderLength];
	char *Content;
	int ContentSize;
}TReq, *PReq;

int encodeReq(PReq req, char *target);
int decodeReq(const char *reqStr, char *content, PReq req);
PStrList queryStrToList(const char *queryStr);

int delFileHead(int fd, int size);
#endif
