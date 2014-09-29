#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/unaligned.h>
#include "ccc.h"
#include "flashdb.h"
#include "flashdb2.h"
#include "utils.h"
#include "options.h"
#include "main.h"
#include "sensor.h"
#include <sys/vfs.h>
#define _HAVE_TYPE_BYTE
#define _HAVE_TYPE_DWORD
#define _HAVE_TYPE_WORD
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#ifndef MINIGUI30
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#endif

#include <time.h>
#include <linux/delay.h>

#include "zkface.h"
#include "facedb.h"
#include "bmp.h"
#include "libcam.h"
#include "main.h"

#define FACE_GROUP_NUM	4

int fdface=-1;
int fdfacepos=-1;
TFGHandle  FaceHandle[FACE_GROUP_NUM];  // 0=def,

int gFaceGroupCnt=0;
int gFaceDefGroupCnt=0;
int gFaceGroupNum=0;
int CurFaceGroup=0;
int LastFaceCashGroup=0;
int AdminFaceScore=85;
int FaceLearnThreshold=75;

unsigned char MulGroup=0;
unsigned char FaceDBChg=0;

BOOL FaceInit=0;

TFaceTmp gFacetmp;

static void InitFaceTmpPos(void)
{
	TFacePos gFacepos;
	PFaceTmp ftmp=NULL;
	int len=0;
	int size=0;
	int i;
	int num=0;
	int Fbuf[FACE_MEM_MAX];

	if(lseek(fdfacepos,0,SEEK_END)>0)
	{
		return ;
	}

	len=lseek(fdface, 0,SEEK_END);
	if(len<=0)
		return;

	lseek(fdfacepos,0,SEEK_SET);
	lseek(fdface,0,SEEK_SET);
	while(len>0)
	{
		if(len >= FACE_MEM_MAX)
		{
			size=FACE_MEM_MAX;
			len-=FACE_MEM_MAX;
		}
		else
		{
			size=len;
			len=0;	
		}
		memset(Fbuf,0,FACE_MEM_MAX);
		if(read(fdface,(void*)&Fbuf, size) == size)
		{
			ftmp=(PFaceTmp)Fbuf;
			size/=sizeof(TFaceTmp);
			for(i=0;i<size;i++)
			{
				gFacepos.PIN=ftmp[i].PIN;
				gFacepos.FaceID=ftmp[i].FaceID;
				gFacepos.Valid=0;
				gFacepos.RecordNum=num;
				num++;
				write(fdfacepos, (void*)&gFacepos, sizeof(TFacePos));
				fsync(fdfacepos);
			}		
		}
	}	
}

extern void SearchFirst(PSearchHandle sh);
#if 0
extern int TruncFDAndSaveAs(int fd, char *filename, char *buffer, int size);
static void InitFaceGroup(void)
{
	TSearchHandle sh;
	PUser gUser;
	int cnt=0;
	int group=gOptions.DefFaceGroup;
	int i,j;
	char buf[200];

	printf("MAX group %d ,  MAX user %d \n", gFaceGroupNum, gFaceGroupCnt);

	int len=0;
	char* buffer=NULL;

	sh.ContentType=FCT_USER;
        sh.buffer=(char*)&gUser;

	SearchFirst(&sh);
	len=lseek(sh.fd,0,SEEK_END);
	j=len/(sizeof(TUser));

	buffer=MALLOC(len);
	if(buffer==NULL)
		return ;

	lseek(sh.fd,0,SEEK_SET);
	if(read(sh.fd,(void*)buffer, len) != len)
	{
		FREE(buffer);
		return ;
	}
	gUser=(PUser)buffer;

	for(i=0; i<j; i++)
	{
		printf(" init group  pin=%d , group=%d %d \n",gUser[i].PIN, group,gUser[i].Group);
                if(gUser[i].PIN > 0)
		{
			gUser[i].Group=group;
			cnt++;
		}
		else
			continue;

		if(group == gOptions.DefFaceGroup && cnt >= gFaceDefGroupCnt)
		{
			group++;
			cnt=0;
		}
		else if(group != gOptions.DefFaceGroup && cnt >= gFaceGroupCnt)
		{
			group++;
			cnt=0;
		}
	}
	sh.fd = TruncFDAndSaveAs(sh.fd, GetEnvFilePath("USERDATAPATH", "ssruser.dat", buf), NULL, 0);
	write(sh.fd,(void*)buffer, len);	

	return;
}

static int TestAddFace(void)
{
#if 0
        TFaceTmp  face[FACE_NUM];
        int i;
        int j;
        int pin2=0;
        char buf[25];
        TUser user;
        int pin=47;

        memset(face,0,sizeof(face));
        memset(&user,0,sizeof(TUser));
        for(i=0; i< FACE_NUM;i++)
        {
                FDB_GetFaceTmp(1, i , &face[i]);
        }

        for(j=46; j<695;j++)
        {
                for(i=0;i<FACE_NUM;i++)
                {
                        face[i].PIN=pin;
                        ChangeFaceTmp(pin, i, &face[i] );
                }
printf("add user = %d \n",pin);
                user.PIN=pin;
                memset(user.PIN2,0,24);
                sprintf(user.PIN2,"%d",pin);
                FDB_AddUser(&user);
                pin++;
        }
#endif
	return 1;
}
#endif


static int InitFaceParam(void)
{
	int score=0;

	gFaceGroupCnt=LoadInteger("FaceGroupCnt",50);
        gFaceDefGroupCnt=LoadInteger("FaceDefGroupCnt",100);
        gFaceGroupNum=LoadInteger("FaceGroupNum",24);
        AdminFaceScore=LoadInteger("AdminFaceScore",85);
	
	score = (gOptions.FaceVThreshold + (gOptions.FaceMThreshold - gOptions.FaceVThreshold)/2);
	if(score < 65)
		score=65;
        FaceLearnThreshold=LoadInteger("FaceLearnThreshold", score);
        CurFaceGroup=gOptions.DefFaceGroup;
	return 1;
}


int FaceDBInit(void)
{
	int i,j;

	memset(FaceHandle ,0,sizeof(FaceHandle));
	InitFaceTmpPos();
	InitFaceParam();
	FaceInit=0;
	
	if(gOptions.FaceFunOn == 0)
		return 0;

	for(i=0;i<FACE_GROUP_NUM;i++)
	{
		FaceHandle[i].Handle= ZKFaceInit(FOR_MATCH);
		//printf(" Face Handle : index=%d  handle=0x%x \n",i,FaceHandle[i].Handle);

		if(FaceHandle[i].Handle == (void*)NULL)
		{
			FaceInit=0;
			for(j=0;j<i;j++)
			{
				ZKFaceFinal(FaceHandle[j].Handle);
				FaceHandle[j].Group=0;
			}
			printf(" Init FaceDB  Error ! \n");
			return 0;
		}
	}

	FaceInit=1;
	FDB_LoadAllFaceTmp();

	if(FACE_GROUP_NUM >1)
		LastFaceCashGroup=FaceHandle[1].Group;
	else
		LastFaceCashGroup=FaceHandle[0].Group;
       
	if(CurFaceGroup ==0)
		CurFaceGroup=gOptions.DefFaceGroup;
	
	//init extract facedb
	FaceExtactDBInit();

	//printf(" %s ----------\n", __FUNCTION__);
	return (int)FaceHandle[0].Handle;
}


int FaceDBFree(void)
{
	int i;

	for(i=0; i< FACE_GROUP_NUM;i++)
	{
		ZKFaceCacheReset(FaceHandle[i].Handle);
		ZKFaceFinal(FaceHandle[i].Handle);
	}

	return 1;
}

int FaceDBReset(void)
{
        int i;

        for(i=0; i< FACE_GROUP_NUM;i++)
                ZKFaceCacheReset(FaceHandle[i].Handle);
        return 1;
}

extern BOOL SearchNext(PSearchHandle sh);
U32 FDB_GetFaceRecord(U16 UID, char FaceID , PFacePos tmp)
{
        TSearchHandle sh;
        sh.ContentType=FCT_FACEPOS;
        TFacePos gFacepos;

	if(tmp)
		sh.buffer=(char *)tmp;
	else
        	sh.buffer=(char *)&gFacepos;

        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                if (sh.datalen==0)
                {
                        if (UID==0)
                                return sh.bufferlen;
                        else
                                continue;
                }

                if(((PFacePos)sh.buffer)->PIN==UID && ((PFacePos)sh.buffer)->FaceID==FaceID)
                {
                        return sh.datalen;
                }
        }
	return 0;
}

int ChangeFaceRecordData(PFacePos Facepos)
{
        TSearchHandle sh;
        sh.ContentType=FCT_FACEPOS;
        TFacePos gFacepos;

	sh.buffer=(char *)&gFacepos;
        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                if(gFacepos.PIN==Facepos->PIN && gFacepos.FaceID == Facepos->FaceID)
                {
			lseek(fdfacepos, -1*sizeof(TFacePos), SEEK_CUR);
        		if (write(fdfacepos, (void*)Facepos, sizeof(TFacePos))== sizeof(TFacePos))
        		{
        			fsync(fdfacepos);
				return FDB_OK;
        		}
        		else
               		 	return FDB_ERROR_IO;
                }
        }
	lseek(fdfacepos, 0, SEEK_END);
        if (write(fdfacepos, (void*)Facepos, sizeof(TFacePos))== sizeof(TFacePos))
	{
        	fsync(fdfacepos);
        	return FDB_OK;
	}

	return FDB_ERROR_IO;

}

int DeleteFaceRecordData(PFacePos Facepos)
{
        int size=sizeof(TFacePos);
        int ret;

       	Facepos->PIN=0;
	Facepos->FaceID=0;
	Facepos->Valid=0;
        lseek(fdfacepos, -1*size, SEEK_CUR);
        if (write(fdfacepos, (void*)Facepos, size)== size)
        {
                ret = FDB_OK;
        }
        else
        {
                lseek(fdfacepos, size, SEEK_CUR);
                ret = FDB_ERROR_IO;
        }

        fsync(fdfacepos);
        return ret;
}

U32 FDB_DelFaceRecord(U16 UID, char FaceID)
{
        TSearchHandle sh;
        sh.ContentType=FCT_FACEPOS;
        TFacePos gFacepos;

	sh.buffer=(char *)&gFacepos;

        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                if(((PFacePos)sh.buffer)->PIN==UID && ((PFacePos)sh.buffer)->FaceID==FaceID)
                {
                        return DeleteFaceRecordData(&gFacepos);
                }
        }
	return 0;
}

U32 FDB_DelUserFaceRecords(U16 UID)
{
        TSearchHandle sh;
        sh.ContentType=FCT_FACEPOS;
        TFacePos gFacepos;

	sh.buffer=(char *)&gFacepos;

        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                if(((PFacePos)sh.buffer)->PIN==UID)
                {
                	DeleteFaceRecordData(&gFacepos);
                }
        }
	return FDB_OK;
}

int ReadFaceTmpData(char*buf, int size, int offset)
{
	lseek(fdface, offset, SEEK_SET);
	if(read(fdface,(void*)buf, size)==size)
		return 1;
	else
		return 0;
}

int WriteFaceTmpData(char*buf, int size, int offset)
{
	lseek(fdface, offset, SEEK_SET);
	if(write(fdface,(void*)buf, size)==size)
	{
		fsync(fdface);
		return 1;
	}
	else
		return 0;
}

int FDB_GetUserFaceNum(void)
{
	return (FACE_NUM + FACE_LEARN_NUM);
}

int FDB_GetUserFaceTemps(int pin, char*buf)
{
	PFaceTmp face=(PFaceTmp)buf;
	TFacePos gFacepos;
	int Count=0;

        TSearchHandle sh;
        sh.ContentType=FCT_FACEPOS;

	sh.buffer=(char *)&gFacepos;

        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
		if(gFacepos.PIN == pin)
		{
			ReadFaceTmpData((char*)&face[Count], sizeof(TFaceTmp), gFacepos.RecordNum*sizeof(TFaceTmp));
			Count++;
		}
        }
        return Count;
}

U32 FDB_GetFaceTmp(U16 UID, char FaceID , PFaceTmp tmp)
{
	TFacePos gFacepos;

	if(FDB_GetFaceRecord(UID, FaceID , &gFacepos)) //get face ok
	{
		if(tmp)
		{
			memset(tmp,0,sizeof(TFaceTmp));
			ReadFaceTmpData((char*)tmp, sizeof(TFaceTmp), gFacepos.RecordNum*sizeof(TFaceTmp));
		}
		return sizeof(TFaceTmp);
	}
	return 0;
}

int FDB_AddFaceTmp(void* tmp, int Cnt)
{
	int len=0;
        int i=0;
        int fsize=sizeof(TFaceTmp);
	int poslen=sizeof(TFacePos);
	PFaceTmp face=NULL;
	TFacePos gFacepos;

	//unsigned int start=GetTickCount1();

	if(Cnt<=0)
		return -1;

	FaceDBChg=1;
	face=(PFaceTmp)tmp;

	i=0;
	lseek(fdfacepos,0,SEEK_SET);
	while(read(fdfacepos,(void*)&gFacepos, poslen) == poslen)
	{
		if(gFacepos.PIN==0)  //add  face tmp and face record
		{
			gFacepos.PIN=face[i].PIN;
			gFacepos.FaceID=face[i].FaceID;
			if(!WriteFaceTmpData((char*)&face[i],fsize,fsize*gFacepos.RecordNum))
				return FDB_ERROR_IO;
			
			lseek(fdfacepos,-1*poslen,SEEK_CUR);
			if(write(fdfacepos,(void*)&gFacepos, poslen) != poslen)
				return FDB_ERROR_IO;

			fsync(fdface);
			fsync(fdfacepos);
			
			i++;
			if(i>= Cnt)
				return FDB_OK;

		}
	}

	if(i<Cnt)  //add  face tmp and face record
	{
		lseek(fdfacepos, 0,SEEK_END);
		len=lseek(fdface, 0,SEEK_END);
		len = (len/fsize) ;
		for(;i<Cnt;i++)
		{
			gFacepos.RecordNum=len;
			gFacepos.PIN=face[i].PIN;
			gFacepos.FaceID=face[i].FaceID;
			gFacepos.Valid=0;
			write(fdfacepos, (void*)&gFacepos, poslen);
			write(fdface, (void*)&face[i], fsize);
			len++;
			fsync(fdface);
			fsync(fdfacepos);
		}
	}

	//printf(">>>deug\t  end  face save time : %dms cnt=%d \n",GetTickCount1()-start,i );

	return FDB_OK;
}

int FDB_DeleteFaceTmps(U16 UID)
{
	TFacePos gFacepos;
	TFaceTmp delface;
        int fsize=sizeof(TFaceTmp);
	int poslen=sizeof(TFacePos);

	if (FDB_GetFaceRecord(UID,0, &gFacepos)==0)  //no tmp
		return FDB_OK;

	FaceDBChg=1;
	memset(&delface,0,sizeof(TFaceTmp));
	lseek(fdfacepos,0,SEEK_SET);
	while(read(fdfacepos,(void*)&gFacepos, poslen) == poslen)
	{
		if(gFacepos.PIN== UID)
		{
			WriteFaceTmpData((char*)&delface,fsize,fsize*gFacepos.RecordNum); //del face template dat

			gFacepos.PIN=0;
			gFacepos.FaceID=0;
			gFacepos.Valid=0;
			lseek(fdfacepos,-1*poslen,SEEK_CUR);
			write(fdfacepos,(void*)&gFacepos, poslen) ; //del face  record  dat

			fsync(fdface);
			fsync(fdfacepos);
		}
	}

	return FDB_OK;
}


int ChangeFaceTmp(U16 pin, char id, PFaceTmp tmp)
{
	TFacePos gFacepos;
	int fsize=sizeof(TFaceTmp);
	int poslen=sizeof(TFacePos);

	FaceDBChg=1;
	if(FDB_GetFaceRecord(pin,id, &gFacepos)) //get face ok
	{
		gFacepos.PIN=tmp->PIN;
		gFacepos.FaceID=tmp->FaceID;
		gFacepos.Valid=0;
		lseek(fdfacepos,-1*poslen,SEEK_CUR);
		write(fdfacepos,(void*)&gFacepos, poslen) ; //change face  record  dat

		lseek(fdface, fsize*gFacepos.RecordNum,SEEK_SET);	
		write(fdface, (void*)tmp, fsize);	//change face template dat

		fsync(fdface);
		fsync(fdfacepos);
	}
	else
	{
		return FDB_AddFaceTmp(tmp,1);
	}
	return FDB_OK;
}

int FDB_ClrFaceTmp(void)
{
	int i;
        if(!FaceInit)
        {
                printf(" clear FaceDB Failed  No Licence ! \n");
                return 0;
        }
	

	for(i=0; i<FACE_GROUP_NUM; i++)
	{
		ZKFaceCacheReset(FaceHandle[i].Handle);
		FaceHandle[i].Group=0;
	}

    	return FDB_ClearData(FCT_FACE);
}


int FDB_CntFaceUser(void)
{
	TFacePos gFacepos;
	int poslen=sizeof(TFacePos);
	unsigned short* pin=NULL;
	int len=0;
        int cnt=0;
	int i;
	char flag=0;

	len=lseek(fdfacepos,0,SEEK_END);
	cnt = len/poslen;

	pin=(unsigned short*)MALLOC(sizeof(short)* cnt);
	if(pin==NULL)
		return -1;

	memset(pin,0,sizeof(short)* cnt);
	len=0;
	cnt=0;
	flag=0;
	lseek(fdfacepos,0,SEEK_SET);
	while(read(fdfacepos,(void*)&gFacepos, poslen) == poslen)
	{
		if(gFacepos.PIN>0)
		{
			if(cnt==0)
			{
				pin[cnt]=gFacepos.PIN;
				cnt++;
			}
			flag=0;
			for(i=0; i<cnt;i++)
			{
				if(pin[0] == 0)
					printf("dddddd  pin=0 i=%d \n", i);
				if(pin[i] == gFacepos.PIN)
				{
					flag=1;
					break;
				}
			}
			if(flag)
				continue;
			else
			{
				pin[cnt]=gFacepos.PIN;
				cnt++;
#if 0
				printf("pin=%d,",gFacepos.PIN);
				if(cnt %15==0)
					printf("\n");
#endif
			}
		}
	}
	FREE(pin);

//	printf("ddddddddddddddddddddddddd   endf  cnt=%d............\n",cnt);

	return cnt;
}

/********************************
 * mode =0 find pin2 and face group
 * mode=1  find group
 * mode=2 find pin2
 * *****************************/
int GetUserFreePINAndFaceGroup(char* Freepin2, int* FreeGroup,int mode)
{
	TSearchHandle sh;
	PUserCash pCacheUser= NULL;
	TUser gUser;
    	int cuser=0;;
    	int i=0,testpin=0;
	int bSign=TRUE, group=0,Cnt=0;
	char pin2[24];

//	int start=GetTickCount1();

	sh.ContentType=FCT_USER;
    	sh.buffer=(char*)&gUser;

    	SearchFirst(&sh);
	i=lseek(sh.fd, 0,SEEK_END);
	cuser=i/sizeof(TUser);

	i= cuser*sizeof(TUserCash);
	
    	pCacheUser = MALLOC(i);
	if(NULL == pCacheUser)
	{
		memset(Freepin2,0,24);
		*FreeGroup=0;
		return -1;
	}
    	memset(pCacheUser, 0, i);
	*FreeGroup=0;
	group=0;
	
	i=0;
    	SearchFirst(&sh);
    	while(!SearchNext(&sh))
    	{
    	    if(sh.datalen>0)
    	    {
    	        memcpy(pCacheUser[i].PIN2, gUser.PIN2, 24);
		pCacheUser[i].Group=gUser.Group;
    	        i++;
    	    }
    	}
	
	if(mode ==0 || mode == 1)
	{
		group=1;
		for(i=0;i<cuser;i++)
		{
			if((pCacheUser[i].Group ==group)) //find group
        	        {
        	        	Cnt++;
        	        	if(group == gOptions.DefFaceGroup && Cnt >= gFaceDefGroupCnt)
        	                {
        	                	group++;
        	                        Cnt=0;
					i=0;
					continue;
                	        }
                	        else if(group != gOptions.DefFaceGroup && Cnt >= gFaceGroupCnt)
                	        {
                	        	group++;
                	                Cnt=0;
					i=0;
					continue;
				}
			}
		}
		if(group>0)
			*FreeGroup=group;
	}


	if(mode ==0 || mode == 2)
	{
		testpin=0;
	    	do
	    	{
	        	bSign=1;
	        	testpin++;
	        	sprintf(pin2, "%d",testpin);
	        	for (i=0;i<cuser;i++)
	        	{
	            		if(strncmp(pCacheUser[i].PIN2,pin2,24)==0)
				{
					bSign=0;
	            			break;
				}
	        	}
	
	    	}while(!bSign);

		memcpy(Freepin2,pin2,24);
	}

    	FREE(pCacheUser);

//	printf("find pin2=%s  find  Group %d ,   %dms \n", pin2, group, GetTickCount1()-start);

	return 1;
}


int GetFaceGroupCnt(int Group)
{
	TSearchHandle sh;
	TUser gUser;
	int Cnt=0;
	
//	int start=GetTickCount1();

        sh.ContentType=FCT_USER;
        sh.buffer=(char*)&gUser;

	SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
              if(gUser.PIN>0&&gUser.Group==Group)
		{
			if(FDB_GetFaceRecord(gUser.PIN, 0 , NULL))
				Cnt++;		
		}
	}

//	printf("Get  Group =%d  , cnt=%d ,  : %dms \n", Group, Cnt, GetTickCount1()-start);
	return Cnt;

}

int FDB_LoadUserFaceTmps(int pin, void* Handle)
{
        TSearchHandle sh;
        sh.ContentType=FCT_FACEPOS;
        TFacePos gFacepos;
	char id[32];
	int cnt=0;

	sh.buffer=(char *)&gFacepos;

        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                if(gFacepos.PIN==pin)
                {
                	ReadFaceTmpData((char*)&gFacetmp, sizeof(TFaceTmp), gFacepos.RecordNum*sizeof(TFaceTmp));
			memset(id,0,sizeof(id));
			if(gFacetmp.FaceID >= FACE_LEARN_ID)
				sprintf(id, "%d_%06d_%02d", 4,gFacetmp.PIN, gFacetmp.FaceID);
			else
				sprintf(id, "%d_%06d_%02d", gFacetmp.FaceID%3,gFacetmp.PIN, gFacetmp.FaceID);
			ZKFaceCacheSet(Handle, id, gFacetmp.Face);
			cnt++; 
                }
        }
	return cnt;
}


int FDB_LoadFaceDefGroup(void)
{
        if(!FaceInit)
        {
                printf(" Load FaceDB Failed  No Licence ! \n");
                return 0;
        }

	CurFaceGroup=gOptions.DefFaceGroup;
	if(gOptions.DefFaceGroup == 0)
		ZKFaceCacheReset(FaceHandle[0].Handle);
	else
		FDB_LoadFaceTmpByGroup(gOptions.DefFaceGroup,FaceHandle[0].Handle);
	return 1;
}

int FDB_LoadFaceGroupTmp(int Group)
{
	int i;
	int lastpos=0;

        if(!FaceInit)
        {
                printf(" InitFaceDB Failed  No Licence ! \n");
                return 0;
        }


	if(Group <= 0)
		return 0;

	for(i=0; i< FACE_GROUP_NUM ; i++)
	{
		if(FaceHandle[i].Group == Group)
		{
			printf(" aleardy  load group=%d \n",Group);
			return 1;
		}
		else if(FaceHandle[i].Group == LastFaceCashGroup)
			lastpos=i;
		else if(FaceHandle[i].Group ==0 )
		{
			lastpos=i;
		}
		
	}
	
	if(FACE_GROUP_NUM > 1 && FaceHandle[lastpos].Group != 0)
		lastpos++;

	if(lastpos>= FACE_GROUP_NUM)
	{
		if(FACE_GROUP_NUM>1)
			lastpos=1;
		else
			lastpos=0;
	}

	CurFaceGroup=Group;
	FaceHandle[lastpos].Group=Group;
	LastFaceCashGroup=Group;
	ZKFaceCacheReset(FaceHandle[lastpos].Handle);
        FDB_LoadFaceTmpByGroup(Group,FaceHandle[lastpos].Handle);

#if 0
	for(i=0; i< FACE_GROUP_NUM ; i++)
	{
		printf(" FaceHandle[%d].Group=%d \n", i,FaceHandle[i].Group);
	}
#endif
	return 1;
}

int FDB_LoadFaceTmpByGroup(int Group, void* Handle)
{
	TSearchHandle sh;
	PFacePos gFacepos;
	TUser gUser;
	char* buf=NULL;
	char id[32];
	int len=0;
	int Cnt=0;
	int cuser=0;
	int i;
	int lface=0;

	if(Group <= 0 || Group > gFaceGroupNum )
		return -1;

//	int start=GetTickCount1();

	lseek(fdfacepos,0,SEEK_SET);
	len=lseek(fdfacepos,0,SEEK_END);
	if(len<=0)
		return 0;

	buf=MALLOC(len);
	if(buf==NULL)
		return 0;

	lseek(fdfacepos,0,SEEK_SET);
	if(read(fdfacepos,(void*)buf, len) != len)
	{
		FREE(buf);
		printf(" Load Face Template flailed  Group=%d \n",Group);
		return 0;
	}
	Cnt=len/sizeof(TFacePos);
	gFacepos=(PFacePos)buf;

	ZKFaceCacheReset(Handle);

	len=0;
	cuser=0;
        sh.ContentType=FCT_USER;
        sh.buffer=(char*)&gUser;

	SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                if(gUser.Group==Group)
		{
			lface=0;
			for(i=0;i<Cnt; i++)
			{
				if(gFacepos[i].PIN==gUser.PIN)
				{
					ReadFaceTmpData((char*)&gFacetmp, sizeof(TFaceTmp), gFacepos[i].RecordNum*sizeof(TFaceTmp));
					memset(id,0,sizeof(id));
					if(gFacetmp.FaceID >= FACE_LEARN_ID)
						sprintf(id, "%d_%06d_%02d", 4,gFacetmp.PIN, gFacetmp.FaceID);
					else
						sprintf(id, "%d_%06d_%02d", gFacetmp.FaceID%3,gFacetmp.PIN, gFacetmp.FaceID);
					ZKFaceCacheSet(Handle, id, gFacetmp.Face);		
					len++; 
					lface=1;

				}
			}
			if(lface)
				cuser++;

			if(Group == gOptions.DefFaceGroup && cuser >= gFaceDefGroupCnt)
				break;
                	else if(Group != gOptions.DefFaceGroup && cuser >= gFaceGroupCnt)
				break;
		}
	}
	FREE(buf);
	ZKFaceCacheSort(Handle, 1);

//	printf("Load  Group %d , face template %d  Cnt=%d  : %dms \n", Group, len,Cnt, GetTickCount1()-start);
	return len;

} 


#define FACE_MAX_GROUP	99
int FDB_LoadAllFaceTmp(void)
{
	TSearchHandle sh;
	PFacePos gFacepos;
	TUser gUser;
	char* buf=NULL;
	char id[32];
	unsigned char UGroup[FACE_MAX_GROUP];
	int  Num[FACE_GROUP_NUM];
	int  flag[FACE_GROUP_NUM];
	int len=0;
	int Cnt=0;
	int index=0;
	int lface=0;
	int i,j;

        if(!FaceInit)
        {
                printf(" InitFaceDB Failed  No Licence ! \n");
                return 0;
        }


	//int start=GetTickCount1();

	/********   read face  **********/
	len=lseek(fdfacepos,0,SEEK_END);
	if(len<=0)
		return 0;

	buf=MALLOC(len);
	if(buf==NULL)
		return 0;

	lseek(fdfacepos,0,SEEK_SET);
	if(read(fdfacepos,(void*)buf, len) != len)
	{
		FREE(buf);
		printf(" Load Face Template flailed ! \n");
		return 0;
	}
	Cnt=len/sizeof(TFacePos);
	gFacepos=(PFacePos)buf;

	for(i=0;i<FACE_GROUP_NUM;i++)
	{
		ZKFaceCacheReset(FaceHandle[i].Handle);
		FaceHandle[i].Count=0;
		FaceHandle[i].Group=0;
		Num[i]=0;
		flag[i]=0;
	}

	sh.ContentType=FCT_USER;
        sh.buffer=(char*)&gUser;

	/********   find group **********/
	memset(UGroup,0,sizeof(UGroup));
	SearchFirst(&sh);
        while(!SearchNext(&sh))
	{
		for(i=0;i<Cnt; i++)
		{
			if(gFacepos[i].PIN==gUser.PIN)
			{
				UGroup[gUser.Group-1]++;
				break;
			}
		}
	}
	for(j=0;j<FACE_GROUP_NUM;j++)
	{
		lface=UGroup[0];
		index=0;
		for(i=1; i< FACE_MAX_GROUP ;i++)
		{
			if(lface < UGroup[i])
			{
				lface=UGroup[i];
				index=i;
			}
		}
		UGroup[index]=0;
		if(lface >0)
		{
			FaceHandle[j].Count=lface;
			FaceHandle[j].Group=index+1;
		}
	}

	lface=FaceHandle[0].Count;
	index=0;
	for(i=1; i< FACE_GROUP_NUM ;i++)
	{
		if(FaceHandle[i].Count > lface)
			index=i;
	}
	if(index>0)
	{
		lface=FaceHandle[index].Count;
		FaceHandle[index].Count=FaceHandle[0].Count;
		FaceHandle[0].Count=lface;
		lface=FaceHandle[index].Group;
		FaceHandle[index].Group=FaceHandle[0].Group;
		FaceHandle[0].Group=lface;
	}
	MulGroup=0;
	for(i=1; i< FACE_GROUP_NUM; i++)
	{
		if(FaceHandle[i].Count > 0)
		{
			MulGroup=1;
			break;
		}
	}
	CurFaceGroup=FaceHandle[0].Group;

#if 0
	for(i=0;i<FACE_GROUP_NUM; i++)
	{
		printf("load face temp: \t count=%d ,Group=%d \n", FaceHandle[i].Count,FaceHandle[i].Group);
	}
#endif


	/********    load  face template **********/
	memset(Num,0,sizeof(Num));
	memset(flag,0,sizeof(flag));
	SearchFirst(&sh);
        while(!SearchNext(&sh))
	{
		for(j=0; j< FACE_GROUP_NUM ; j++)
		{
			if(gUser.Group==FaceHandle[j].Group && !flag[j])
			{
				lface=0;
				for(i=0;i<Cnt; i++)
				{
					if(gFacepos[i].PIN==gUser.PIN)
					{
						ReadFaceTmpData((char*)&gFacetmp, sizeof(TFaceTmp), gFacepos[i].RecordNum*sizeof(TFaceTmp));
						memset(id,0,sizeof(id));
						if(gFacetmp.FaceID >= FACE_LEARN_ID)
						{
							sprintf(id, "%d_%06d_%02d", 4,gFacetmp.PIN, gFacetmp.FaceID);
							//printf("%s : group=%d\n",id,gUser.Group);
						}
						else
							sprintf(id, "%d_%06d_%02d", gFacetmp.FaceID%3,gFacetmp.PIN, gFacetmp.FaceID);
						ZKFaceCacheSet(FaceHandle[j].Handle, id, gFacetmp.Face);		
						lface=1;
					}
				}
				if(lface)
					Num[j]++;
				if(FaceHandle[j].Group == gOptions.DefFaceGroup && Num[j] >= gFaceDefGroupCnt)
					flag[j]=1;
				else if(FaceHandle[j].Group != gOptions.DefFaceGroup && Num[j] >= gFaceGroupCnt)
					flag[j]=1;
			}
		}

	}
	FREE(buf);
	for(j=0; j< FACE_GROUP_NUM ; j++) ZKFaceCacheSort(FaceHandle[j].Handle, 1);
	
	//printf("  Load all  Group face template  Cnt=%d  %dms \n", Cnt, GetTickCount1()-start);
	
	if(CurFaceGroup == 0)
        	CurFaceGroup=gOptions.DefFaceGroup;

	return Cnt;
}


int FDB_AddLearnFaceTemplate(int pin, PUser User, char* SFace,int FaceLen)
{
#if 0
	static int LastActiveTime=0;
	static int LastActivePin=0;
#endif
	int i;
	int LastTime=0;
	int Pos=0;
	int FaceID=0;
	int t;
	int flen=sizeof(TFaceTmp);
	char id[32];
	TUser user;
	TFacePos gFacepos;

        if(!FaceInit)
        {
                printf(" InitFaceDB Failed  No Licence ! \n");
                return 0;
        }

	if(NULL==User)
	{
		if(FDB_GetUser(pin, &user) == NULL)
			return 0;
	}
	else
		memcpy(&user,User,sizeof(TUser));

	pin=user.PIN;

	GetTime(&gCurTime);
	t=(int)OldEncodeTime(&gCurTime);
#if 0
	if(pin == LastActivePin && LastActiveTime+60 >= (int)t)
		return 0;
	LastActivePin=pin;
	LastActiveTime=(int)t;
#endif

	for(i=0; i< FACE_LEARN_NUM ; i++)
	{
       	 	if(FDB_GetFaceRecord(pin, FACE_LEARN_ID+i , &gFacepos)) //get face ok
        	{
                        ReadFaceTmpData((char*)&gFacetmp, flen, gFacepos.RecordNum*flen);
			if(LastTime ==0 ||  gFacetmp.ActiveTime < LastTime)
			{
				LastTime = gFacetmp.ActiveTime;
				Pos=gFacepos.RecordNum;
				FaceID=gFacepos.FaceID;
			}
                }
		else
			break;
	}

	if(i < FACE_LEARN_NUM)
	{
		FaceID=FACE_LEARN_ID+i;
		Pos=lseek(fdface,0,SEEK_END);
	}
	else
		Pos *= flen;

	if(FaceID < FACE_LEARN_ID)
		return 0;

	memset(&gFacetmp,0,sizeof(gFacetmp));
	gFacetmp.PIN=pin;
	gFacetmp.FaceID=FaceID;
	gFacetmp.ActiveTime=t;
	gFacetmp.Size=FaceLen;
	memcpy(gFacetmp.Face,SFace,FaceLen);

	printf(" Add face learn : Threshold=%d  FaceID =%d, t=%x, pin=%d Pos=%d\n",FaceLearnThreshold, FaceID, t,pin,Pos);

	if(WriteFaceTmpData((char*)&gFacetmp, flen, Pos))
	{
		memset(&gFacepos,0,sizeof(gFacepos));
		gFacepos.PIN=pin;
		gFacepos.FaceID=FaceID;
		gFacepos.RecordNum=Pos/flen;
		ChangeFaceRecordData(&gFacepos);

		memset(id,0,sizeof(id));
		sprintf(id, "%d_%06d_%02d", 4,gFacetmp.PIN, gFacetmp.FaceID);

		for(i=0; i< FACE_GROUP_NUM; i++)
        	{
                	if(FaceHandle[i].Group == user.Group)
                	{
				if(LastTime)
					ZKFaceCacheDel(FaceHandle[i].Handle, id);
				ZKFaceCacheSet(FaceHandle[i].Handle, id, (unsigned char*)SFace);
				ZKFaceCacheSort(FaceHandle[i].Handle, 1);
				printf("Add face learn template to cache  group=%d, \n",user.Group);
				break;
			}
                }
		return 1;
        }
	return 0;
}



int VerifyFaceTemps(char* SFace, int size, PUser user)
{
        TSearchHandle sh;
        sh.ContentType=FCT_FACEPOS;
        TFacePos gFacepos;
	int Count=0;
	int pin=user->PIN;
	int result=0;
	int score=0;

	FaceVScore=0;
        sh.buffer=(char *)&gFacepos;
        SearchFirst(&sh);
        while(!SearchNext(&sh))
        {
                if(gFacepos.PIN==pin)
                {
                        ReadFaceTmpData((char*)&gFacetmp, sizeof(TFaceTmp), gFacepos.RecordNum*sizeof(TFaceTmp));

			result=ZKFaceVerify((void*)FaceHandle[0].Handle,(unsigned char*)gFacetmp.Face,(unsigned char*)SFace);
		//	printf("ddd  1:1  score: %d  id=%d \n",result,gFacetmp.FaceID);
			if(result > score) 
                                 score =result;
			if(score >= gOptions.FaceMThreshold)
				break;
			Count++;
			if( Count >= FACETEMPS_NUM)
				break;
                }
        }

//	printf("ddddd   1:1  verify  \t FaceLearnThreshold=%d  score=%d pin=%d  group=%d \n",FaceLearnThreshold, score,user->PIN,user->Group);
	if(score >= FaceLearnThreshold)
	{
		FDB_AddLearnFaceTemplate(pin, user, SFace, size);
	}

	if (score >= gOptions.FaceVThreshold)
	{
		if(admflag && score < AdminFaceScore)
			return 0;

		FacePin=pin;
		FaceVScore=score;
		return FacePin;
	}
	return 0;
}


//return user pin
int IdentifyFaceTemp(void* Handle, char * SFace , int size)
{
	int pin=0;
	int score=0;
	char id[20];
	int i;

        if(!FaceInit)
        {
                printf(" FaceDB   No Licence ! \n");
                return 0;
        }


//	unsigned int start=GetTickCount1();	

	score = ZKFaceIdentifyInCache((void*)Handle, (unsigned char*)SFace, (char*)id, sizeof(id), 50, 105);

//	printf("\n\n Identify  face  template    score=%d  %dms\n\n", score,GetTickCount1()-start);
	//printf(" Identify  score=%d \n", score);

	if(score>=gOptions.FaceMThreshold)
	{
		if(admflag && score < AdminFaceScore)
			return 0;

		for(i=2;i<sizeof(id);i++)
		{
			if(id[i]=='_') id[i]='\0';
		}
		pin=atoi(id+2);
		FaceVScore=score;
		return pin;
	}
	return 0;

}

int FDB_FaceIdentify(int Group, char* SFace, int size)
{
	int i;

        if(!FaceInit)
        {
                printf(" FaceDB   No Licence ! \n");
                return 0;
        }


	for(i=0; i< FACE_GROUP_NUM; i++)
	{
		if(FaceHandle[i].Group == Group)
			break;
	}
	if(i< FACE_GROUP_NUM)
	{
		return IdentifyFaceTemp(FaceHandle[i].Handle, SFace ,size);
	}
	return 0;
}



// check user face and save flag to user.Password[2];
int GetUserFaceInfoForListview(PUserlb header)
{
        PUserlb usrlb=header;
        TFacePos gFacepos;

        if(NULL==header)
                return 1;

//      int start=GetTickCount1();


	usrlb=header;
	while(usrlb)
	{
		if(usrlb->userlb.PIN >0 )
		{
			usrlb->userlb.Password[2]=0;
		}
		usrlb=usrlb->next;
	}

	lseek(fdfacepos,0,SEEK_SET);
	//read user face record dat
	while (read(fdfacepos,(void*)&gFacepos, sizeof(TFacePos))==sizeof(TFacePos))
        {
                if(gFacepos.PIN>0)
                {
                        usrlb=header;
                        while(usrlb)
                        {
                                if(usrlb->userlb.PIN == gFacepos.PIN)  //find user face template
                                {
                                        usrlb->userlb.Password[2]++;
                                }
                                usrlb=usrlb->next;
                        }
                }
        }

//      printf(">>>deug\t user list face : %dms  \n",GetTickCount1()-start);
	return 1;
}

/***************************************************************
*用户登记的处理相关函数
* Face Enroll
*****************************************************************/
#define REG_FACE_CNT	20
char Extracttmp[FACE_TMP_MAXSIZE];
TFaceTmp regface[FACE_NUM];
unsigned char FaceCount=0;
int FaceLen=0;
char*FaceTmp[FACE_NUM];

unsigned short face_bmp[FACE_BMP_SIZE];

int RegBegin=0;
int RegCnt=0;
PFaceTmp EnrollTmp;



int  InitFaceEnroll(void)
{
        RegCnt=0;
        memset(regface,0,sizeof(regface));
        FaceCount=0;
        RegBegin=0;
	
	return 1;
}

int FreeFaceEnroll(void)
{
	return 1;
}

#define PREPARE_NUM	4
int RegPrepare(char* facetemp, int size)
{
	int i=0;
	int empty=-1;
	int score=0;

        if(!FaceInit)
        {
                printf(" FaceDB   No Licence ! \n");
                return 0;
        }


	 for(i=0;i<PREPARE_NUM;i++)
	{
         	if(regface[i].Valid <= 0)
                {
                	if(-1==empty)
                        	empty=i;
			continue;
		}

                score=ZKFaceVerify((void*)FaceHandle[0].Handle, (unsigned char*)facetemp, (unsigned char*)regface[i].Face);
//                printf("RegPrepare: score=%d, i=%d , RegBegin=%d ,FaceCount=%d \n",score,i,RegBegin,FaceCount);

                if(score<60)
                {
                	memset(&regface[i],0,sizeof(regface[i]));
                        empty=i;
                        break;
		}
	}
        if(-1==empty)
        	empty=FaceCount;

	//printf("RegPrepare add temp : empty=%d  size=%d  FaceLen=%d \n",empty,size,FaceLen);

	memcpy(regface[empty].Face,facetemp,size);
        regface[empty].Valid=1;

	if(empty>=FaceCount)
                        FaceCount++;

        if(FaceCount>=PREPARE_NUM)
        {
		RegCnt=0;
	        memset(regface,0,sizeof(regface));
        	FaceCount=0;
	#ifdef MERGE_FACE
        	memset(EnrollTmp,0,REG_FACE_CNT*sizeof(TFaceTmp));
	#endif
        	RegBegin=REG_START;
	}

	return RegBegin;
}

int RegFaceAloneTemplate(int pin)
{
	int i;
	int score=0;
	int same=0;

	if( FaceCount<FACE_NUM)
	{
		for(i=0;i<FaceCount;i++)
		{
			score=ZKFaceVerify((void*)FaceHandle[0].Handle,(unsigned char*)regface[i].Face,(unsigned char*)Extracttmp);
			//printf(" >>>deug \t Face verify score= %d \n",score);

		#if 0
			if(score == 120)
				same++;
			else if(score < 45)
				return RET_CONTINUE;
		#endif

		}

		//if(same > FaceCount/2 + 1 )
		if(same >1)
			return RET_SAME;
		else
		{
			regface[FaceCount].PIN=pin;
                        regface[FaceCount].FaceID=FaceCount;
                        regface[FaceCount].Size=FaceLen;
                        regface[FaceCount].Valid=1;
                        memcpy((void*)regface[FaceCount].Face,(void*)Extracttmp,FaceLen);
                        FaceCount++;
		//	printf(">>>deug\t Face add template ok  FaceCount=%d  FaceLen=%d\n", FaceCount,FaceLen);
		}

		if(FaceCount >= FACE_NUM)
			return RET_SUCCESS;
	}
	return RET_CONTINUE;
}



// return 2 	Register Finished
// return -1 	Register Failed
int RegFaceTmpProc(int pin)
{
        if(!FaceInit)
        {
                printf(" FaceDB   No Licence ! \n");
                return 0;
        }

	if(RegBegin<= REG_PREPARE)
	{
		RegPrepare(Extracttmp,FaceLen);
	}
	else
	{
//	#ifdef MERGE_FACE
	#if 0
		return	RegAloneTmp_Once(pin);	
	#endif
		return RegFaceAloneTemplate(pin);
	}
	return RET_CONTINUE;
}

int FaceRegTempTestProc(int pin)
{
	int result=0;

        if(!FaceInit)
        {
                printf(" FaceDB   No Licence ! \n");
                return 0;
        }


	result = ZKFaceVerify((void*)FaceHandle[0].Handle,(unsigned char*)Extracttmp, (unsigned char*)regface[0].Face);
	if (result >= gOptions.FaceVThreshold+5)
		return 1;
	else
		return 0;
}

/***************************************************************
*用户识别的处理相关函数
* Face Verify  
*****************************************************************/
int FacePin=0;
int FaceVScore=0;

unsigned char facevfwin=0;  // 0: quit fave verify window ; 1: in face verify window
int VerifyFaceWithTmp( char* pin2 ,char* tmpface , int size)
{
	TUser user;

        if(!FaceInit)
        {
                printf(" FaceDB   No Licence ! \n");
                return 0;
        }


	FaceVScore=0;
	if(pin2[0])  //1:1 verify
	{
		if(NULL==FDB_GetUserByCharPIN2(pin2,&user))
			return 0;

		FacePin=VerifyFaceTemps(tmpface, size, &user);
		if(FacePin >0)
			return FacePin;
		else
		{
			FacePin=0;
			return 0;
		}
	}
	else //1:G
	{
		FacePin=FDB_FaceIdentify( CurFaceGroup, tmpface,size);
		if(FacePin>0)
		{
			return FacePin;
		}
		else
		{
			FacePin=0;
			return 0;
		}
	}
	return 0;
}




/**********************************************
 * Get face image
 * Face extract
 *
 * *******************************************/
extern struct vdIn videoIn;

void* hFaceExtact=NULL;
BOOL FaceTest=0;
int FaceStatus=0;
int FaceDistance=0;
int FaceQuality=0;
int Position[MAX_POSITIOV_NUMBER];
int Quality[3];
int FaceGray=0;

void SetFaceExtactParams(void)
{
	//ZKFaceSetParam(hFaceExtact, PARAM_FACE_QUALITY_THRESHOLD, gOptions.VideQuality);
	spcaSetGlobalGain(&videoIn, gOptions.VideoGain);
        spcaSetExposoure(&videoIn, gOptions.FaceExposoure);
}

int FaceExtactDBInit(void)
{
	int ret=-1;

	FaceTest=0;
	if(cameraflag <= 0)
	{
		ret=CameraOpen(0);
		if(ret<0)
		{
			printf(" Init camera falied \n");
			FaceInit=0;
			return 0;
		}
	}
	printf(" Open Camera Success \n");
	

	hFaceExtact=ZKFaceInit(FOR_EXTRACT);
	if(hFaceExtact == NULL)
	{
		FaceInit=0;
		printf(" zk face db init Error ! \n");
	}
	else
	{

		printf("PARAM_IMAGE_FILE_GRAY=%s\n", (char *)ZKFaceGetParam(FaceHandle, PARAM_IMAGE_FILE_GRAY));
		printf(" Handle: %d   Image quality=%d \n",(int)hFaceExtact,gOptions.VideQuality);

		ZKFaceSetParam(hFaceExtact, PARAM_IMAGE_ROTATION, PARAM_IMAGE_ROTATIOV_90);
		ZKFaceSetParam(hFaceExtact, PARAM_FACE_QUALITY_THRESHOLD, gOptions.VideQuality);
		ZKFaceSetParam(hFaceExtact, PARAM_IMAGE_FLIP, PARAM_IMAGE_FLIP_X);
		ZKFaceSetParam(hFaceExtact, PARAM_IMAGE_FILE_GRAY, (int)"");
																							
		ZKFaceSetParam(hFaceExtact, 101, 20);
		ZKFaceSetParam(hFaceExtact, 1200, 4);
	}

	spcaSetGlobalGain(&videoIn, gOptions.VideoGain);
	spcaSetExposoure(&videoIn, gOptions.FaceExposoure);
	ChangeExposal(0);

	return (int)hFaceExtact;
}

void ChangeExposal( int mode)
{
	char src_jpg[FACE_IMG_W*FACE_IMG_H];
	int i;
	int size;
	static int CurExpos=0;
	
	if(mode == 1) // change to high
	{
		if(CurExpos >= gOptions.FaceExposoure + gOptions.FaceExposRang)
			return ;
		else
			CurExpos += gOptions.FaceExposRang;
	}
	else if( mode ==2) // low
	{
		if( CurExpos <= gOptions.FaceExposoure - gOptions.FaceExposRang)
			return ;
		else
			CurExpos -= gOptions.FaceExposRang;
	}
	else
	{
		if(CurExpos == gOptions.FaceExposoure)
			return ;
		else
			CurExpos =gOptions.FaceExposoure;
	}

	spcaSetExposoure(&videoIn,CurExpos);
	msleep(50);
	for(i=0; i<2; i++)
	{
		CaptureSensorFace((char *)src_jpg, 0 ,&size);
	}

//	printf("----Face sensor change exposal : %d \n", CurExpos);
}


int CheckFaceDistance(int* Distance, int* positions, int gray ,int quality)
{
	int ret=0;
//	int center= positions[POSITIOV_FACE_Y] + (positions[POSITIOV_FACE_HEIGHT]) /2;

        if(Quality[QUALITY_DEFINITION] < gOptions.VideQuality && gray < FACE_GRAY_FAR )
        { 
                ret= HINT_FNEAR;
        }
        else if ( Quality[QUALITY_BANLANCE] <  gOptions.VideQuality && gray > FACE_GRAY_HIGH )
        {
                ret= HINT_FFAR;
        }
	else if(positions[POSITIOV_FACE_Y] > FACE_Y_LOW &&  gray < FACE_GRAY_FAR )
	{
                ret= HINT_FNEAR;
	}
	else if((positions[POSITIOV_FACE_Y] + positions[POSITIOV_FACE_HEIGHT]) <  FACE_Y_HIGH )
	{
                ret= HINT_FFAR;
	}
	if( ret ==0 && quality < gOptions.VideQuality)
	{
		if(gray > FACE_GRAY_HIGH)
			ChangeExposal(2); //low
		else if( gray < FACE_GRAY_LOW)
			ChangeExposal(1); //high
	}

	*Distance=ret;
	return ret;
}


int CaptureSensorFace(char *Buffer, BOOL Sign,int* ImageSize)
{
	int sizein = 0;
	int sizeout = 0;
	static unsigned char CapFailCnt=0;
	if( cameraflag <0)
	{
		CapFailCnt=0;
		cameraflag=CameraOpen(0);
		DelayMS(10);
		SetCameraLed(&videoIn,2,1);
		SetFaceExtactParams();
	}

	if(cameraflag<0 || hFaceExtact <= 0)// || (FaceInit == 0 && FaceTest==0))
	{
		printf(" Open Face Camera Failed or Init face DB failed \n");
		return 0;
	}
	sizein = FACE_IMG_W * FACE_IMG_H;
	sizeout=read(videoIn.fd,(void *)Buffer,sizein);
	if((sizeout<=0) || (!((sizeout >0)&&chech_jpeg((void *)Buffer,sizeout,FACE_IMG_H,FACE_IMG_W))))
	{
		CapFailCnt++;
		if(CapFailCnt>3)
		{
			CloseCamera(&videoIn);
			cameraflag=-1;
		}
		printf(" check jpg failed \n");
		return 0;
	}

#if 0  // save jpg file
        FILE * fj=NULL;
        fj=fopen("/mnt/ramdisk/fa.jpg","wb");
        if(sizeout == fwrite(Buffer,1, sizeout,fj))
           ; //    printf("write fa.jpg ok \n");
        fclose(fj);
        sync();
#endif

	*ImageSize=sizeout;

	return TRUE;
}

int FaceDetection(char* face_img)
{
	char src_jpg[FACE_IMG_W*FACE_IMG_H];
	int len=FACE_IMG_W*FACE_IMG_H+1200;
	int size;
	int quality=0;
	static unsigned  int img_state=0;

	memset(src_jpg,0,sizeof(src_jpg));
	if(! CaptureSensorFace(src_jpg, 0, &size))
		return 0;


	FaceQuality=0;
	if(FaceStatus == FACE_IDETIFY_FAILED)
	{
		 if(FaceGray > FACE_GRAY_HIGH)
                        ChangeExposal(2); //low
                else if( FaceGray < FACE_GRAY_LOW)
                        ChangeExposal(1); //high
		msleep(50);
		if(!CaptureSensorFace(src_jpg, 0,&size))
		{

			FaceGray=0;
			FaceDistance=0;
			return 0;
		}
	}

	FaceGray=0;
	memset(Quality,0,sizeof(Quality));
	FaceDistance=0;
	FaceStatus=0;

	size=ZKFaceJPGPrepare(hFaceExtact,(unsigned char*)src_jpg, len, (unsigned char*)face_img, len);
	if(size > 0)
	{
		FaceGray=ZKFaceFetchFace(hFaceExtact);
		if(FaceGray>0)  // Detect face ok
		{
			if(img_state>3)
			{
				img_state=0;
				return 1;
			}
			img_state=0;

			//find face position
			size=ZKFaceGetFaceEyesPosition(hFaceExtact, Position, MAX_POSITIOV_NUMBER);
			if(size > POSITIOV_FACE_X)
			{
				//Detect eyes 
				size=ZKFaceFetchEyes(hFaceExtact);
				ZKFaceGetLastQuality(hFaceExtact, Quality, 3);
				quality= ((size *100) /180);
				if(quality>100)
					quality=100;
				FaceQuality=quality;
				CheckFaceDistance(&FaceDistance,Position , FaceGray, size);
				if(size >= gOptions.VideQuality) // fine eyes
					return 3;  //get face eyes
				else
					return 2; //get face position
			}
		}
		else
		{
			img_state++;
			ChangeExposal(0);
		}
		return 1;
	}
	return 0;
}

int FaceExtractProc(void)
{
	memset(Extracttmp,0,FACE_TMP_MAXSIZE);
	FaceLen=ZKFaceFetchTemplate(hFaceExtact, (unsigned char *)Extracttmp);
	if(FaceLen>0)
		return 1;

	FaceLen=0;
	return 0;
}













/***************************************************************
 * *用户识别的处理相关函数
 * * exposure
 * *****************************************************************/

/*
 *  * convert 24bit RGB888 to 16bit RGB565 color format
 *   */
unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue)
{
        unsigned short  B = ((blue)&0xf8)>>3; //((blue)&0x1f)<<11;
        unsigned short  G = ((blue)&0xfc)<<3; // ((green)&0x3f)<<5;
        unsigned short  R = ((blue)&0xf8)<<8;// ((red)&0x1f);

        return (unsigned short) (R | G | B);
}



int Bmp8ToRGB565(unsigned char* bmpfile, int w, int h)
{
        int y,x;
	unsigned char index=0;
        int i=0;
        unsigned short color;
	char* temp=NULL;

	temp=MALLOC(640*480);
	if(temp ==NULL)
		return 0;

	memcpy(temp,(char*)(bmpfile+54+1024), 640*480);

        for(y=0; y < h; y++)
        {
                for (x = 0; x < w; x++)
                {
                        index=temp[(h-y-1)*w+x];
                        color = RGB888toRGB565(index, index, index);
			face_bmp[i]=color;
			i++;
                }
        }
	FREE(temp);
        return 1;
}

int FPBmpToRGB565(char* bmpfile, int w, int h)
{
        unsigned short* buffer=(unsigned short*)(bmpfile);
        char* tmpbuf=NULL;
        int y,x,index;
        int i=0;
        unsigned short color;

	w=((w *8 + 31)/32) * 4;

        tmpbuf=MALLOC(w*h);
        if(tmpbuf==NULL)
                return 0;

        memcpy(tmpbuf,(char*)buffer,w*h);

        for(y=0; y < h; y++)
        {
                for (x = 0; x < w; x++)
                {
                        index=tmpbuf[(h-y-1)*w+x];
                        color = RGB888toRGB565(index, index,index);
                        buffer[i]=color;
                        i++;
                }
        }
        FREE(tmpbuf);
        return 1;
}


