#ifndef PUSH_EXTERN_H_
#define PUSH_EXTERN_H_
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef  BOOL 
#define BOOL int 
#endif

#ifndef  U32
#define U32 unsigned int 
#endif

#ifndef  U16 
#define U16 unsigned short 
#endif

#ifndef  BYTE 
#define BYTE unsigned char
#endif

#ifndef  U8
#define U8 unsigned char 
#endif

#define TRANS_MAX_RECORD_NO_LIMIT -1
typedef struct tm TTime;
enum { FALSE=0, TRUE=1 };


#ifndef FACE_BASE_VERSION
#define FACE_BASE_VERSION	7
#endif
#define GCC_PACKED __attribute__((packed))

typedef struct _TFacePos_{
	unsigned short PIN;
	unsigned char  FaceID;
	unsigned char Valid;
	int RecordNum;
}GCC_PACKED TTFacePos, *PTFacePos;

typedef struct _OPLog_{
        unsigned short  Admin;               
        unsigned char OP;                
        time_t time_second;     //4
        unsigned short Users[4];           
}GCC_PACKED TOPLog, *POPLog;

#define OP_ENROLL_FACE 			65
#define OP_CHG_FACE 			66
typedef struct {
    unsigned char *buffer;
    int bufferSize;
    unsigned char *bufPtr, *bufEnd;
    int isRom;
}TBuffer;
#define clearBuffer(p)	((p)->bufPtr=(p)->buffer)
#define bufferSize(p)	((p)->bufPtr-(p)->buffer)

int avialible(int fd, int timeout);
int base64_encode(const unsigned char *in,  unsigned long len,unsigned char *out);
int LoadInteger(const char *Name, int DefaultValue);
BOOL LoadStr(const char *name, char *value);
char *LoadStrOld(const char *name);
int FDB_InitDBs(BOOL OpenSign);
void msleep(int msec);
int SetTime(struct tm *t);
char *loadFileMem(char *fname, int *fsize, int encrypt, char *key, int keySize);
int bufferEncrypt(char *buf, int size, char *target, int targetSize, char *key, int keySize);
time_t OldEncodeTime(TTime *t);
void GetTime(TTime *t);
int fAvialible(FILE *f, int timeout);
TBuffer *createRamBuffer(int size);
int writeBuffer(TBuffer *buffer, char *data, int size);
int resOfBuffer(TBuffer *buffer);
void freeBuffer(TBuffer *buffer);
void resumedisabletime(void);
//extern unsigned char DevNumber[11];


typedef int (*signle_record_deal_f)(void *rec, int count, void *param);
int FDB_ForAllData(int ContentType, signle_record_deal_f fun, int MaxRecCount, void *param);
int FDB_ForDataFromCur(int ContentType, signle_record_deal_f fun, int MaxRecCount, void *param);
void DelayMS(int ms);
void ReLoadOptions(void);
int FPInit(char *FingerCacheBuf);
void EnableDevice(int Enabled);
int  mainEnrollCmd(const char *cmd);
int IsDaylightSavingTime();
int InitUDPSocket(unsigned short port, int *udpsocket) ;
void reset_alarm();
int DoAlarmOff(int Index);
BOOL SaveStr(const char *name, const char *value, int SaveTrue);
 
int PushSetTZInfo(char *pstrSrc);
int PushSetUserTZStr(char *pstrSrc);
int PushSetUnlockGroup(char *pstrSrc);
int PushSetGroupTZs(char *pstrSrc);
int PushSetHTimeZone (char * pstrSrc);
int FDB_CheckAttlog(char *param,unsigned int *sumAttLog);
int ServerAuthUpdate(FILE *pfileRes,const char *pszHttpInfoHeader, void *pUserInfo);
int pushsdk_get_filepos(int ContentType,off_t *offset);
void pushsdk_set_filepos(int con_type,off_t offset );
#endif
