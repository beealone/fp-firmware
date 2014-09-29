#define MaxUser 500


typedef struct _tag_ValIndex
{
	unsigned long Val;
	struct _tag_ValIndex *pNext;
}
ValIndex;

typedef struct _tag_KeyIndex
{
	long Key;
	long Grp;
	ValIndex *ValHR;
	ValIndex *ValLR;
	struct _tag_KeyIndex *pNext;
}
KeyIndex;

typedef struct _tag_NdbIndex
{
	unsigned long Total;
	short Group;
	short Style;
	short ID;
	long Min;
	long Max;
	KeyIndex *Headr;
	KeyIndex *Lastr;
}
NdbIndex;
#ifndef ZEM600
#pragma pack(1)
#endif
typedef struct _tag_UserAttLog
{
	time_t dt;
	unsigned char CheckType;
	unsigned long aAddr;
	struct _tag_UserAttLog *pNext;
}
UserAttLog;

typedef struct _tag_UserContent
{
	U32 UserID;
	U32 uAddr;
	UserAttLog *pHead;
	UserAttLog *pLast;
	UserAttLog *StartDate;
	UserAttLog *EndDate;
	TUserInfoDB UserInfo;
	NdbIndex USD;
	NdbIndex AUSC;
	NdbIndex ES;
	NdbIndex UTS;
	int fdusd,fdausc,fdes,fduts;
	struct _tag_UserContent *pNext;
}
UserContent;

typedef struct _tag_UserManage
{
	UserContent *uHead;
}
User_Manage;

#pragma pack()


void UserContentInit(void);
void UserContentDel(U32 UserID);
int UserContentAdd(int index,U32 UserID,U32 Addr);
void UserContentLoad(int Fd);
int UserContentFree(void);
int UserContentAttAppend(UserContent *db,time_t DateTime,unsigned char CheckType,unsigned long Addr);
int UserContentAttAdd(int index,U32 UserID,U32 time_second,U8 CheckType,U32 Addr);
void UserContentAttLoad(int Fd);
int UserInfoLoad(U32 UserID,TUserInfoDB *Node,char *Name);
unsigned long UserSpeUserLoad(NdbIndex *Index,unsigned long Userid,int Fd);
unsigned long UserAutoLoad(NdbIndex *Index,unsigned long Userid,int Fd);
unsigned long UserEmpLoad(NdbIndex *Index,unsigned long Userid,int Fd);
unsigned long UserTempLoad(NdbIndex *Index,unsigned long Userid,int Fd);
UserContent *GetUserContent(U32 Userid,time_t StartDate,time_t EndDate);
void FreeUserContent(U32 Userid);
int LoadData(void);

void AttRulesRead(TAttRules *AttRulesTable)
{
FILE *fp;
char Value[256];
char *SysPath=".";
sprintf(Value,"%s/webdb/%s",SysPath,"AttRulesDB.dat");
fp=fopen(Value,"rb");
if(fp==NULL)
fp=fopen(Value,"wb");
if(fp)
{
fread(AttRulesTable,sizeof(TAttRules),1,fp);
fclose(fp);
}
}

