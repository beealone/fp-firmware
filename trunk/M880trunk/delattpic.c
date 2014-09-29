#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "delattpic.h"
#include "flashdb.h"
#include "exfun.h"
#include "commu.h"
//#include "commu.h"

extern int fdpicidx;
int FDB_ClearIndexBy(char * p,int type);

int CheckPhotoFileStatus(const char* filename)
{
	struct stat statbuf;
	char mtdblockbuf[80];
	char passbuf[80];
	char badbuf[80];
	//int size;
	//printf("filename: %s\n",filename);
	memset(mtdblockbuf, 0, sizeof(mtdblockbuf));
	memset(passbuf, 0, sizeof(passbuf));
	memset(badbuf, 0, sizeof(badbuf));

	if(stat(filename,&statbuf)==-1)
	{
		snprintf(mtdblockbuf, sizeof(mtdblockbuf), "/mnt/mtdblock/%s",filename);
		if(stat(mtdblockbuf,&statbuf)==-1)
		{
			memset(passbuf, 0, sizeof(passbuf));
			snprintf(passbuf, sizeof(passbuf), "/mnt/mtdblock/capture/pass/%s",filename);//在pass 文件夹下找
			if(stat(passbuf,&statbuf)==-1)
			{
				memset(badbuf, 0, sizeof(badbuf));
				snprintf(badbuf, sizeof(badbuf), "/mnt/mtdblock/capture/bad/%s",filename);//在bad文件夹下找
				if(stat(badbuf,&statbuf)==-1)
				{
					DBPRINTF("no4-----error at stat file(checkfilestatus) %s\n",badbuf);
					return -1;
				}
				else
				{
					strcpy((char *)filename,badbuf);
				}
			}
			else
			{
				strcpy((char *)filename,passbuf);
			}
		}
		else
		{
			strcpy((char *)filename,mtdblockbuf);
		}
	}

	if(S_ISREG(statbuf.st_mode))
	{
		//DBPRINTF("file(at checkfilestatus) size is %d\n",statbuf.st_size);
		return statbuf.st_size;
	}
	else
	{
		return -1;
	}
}

int CalcCount(int type, time_t sttime, time_t edtime)
{
	int cnt=0;
	TSearchHandle sh;
	time_t curtime;
	//TTime tmptime;
	TPhotoIndex idxBuf;

	memset((void*)&idxBuf, 0, sizeof(TPhotoIndex));
	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&idxBuf;
	SearchFirst(&sh);
	//	printf("start time:%d, endtime:%d, user name:%s, type:%d\n", sttime, edtime, userid, type);
	while (!SearchNext(&sh))
	{
		//pictype,0--pass, 1--no pass
		if (((PhotoIndex)sh.buffer)->index && ((PhotoIndex)sh.buffer)->pictype==type)		//记录有效
		{
			//查询照片不需要精确到分秒
			curtime=((PhotoIndex)sh.buffer)->createtime;
			//不符合查询条件
			if ((sttime!=0 && curtime<sttime) || (edtime!=0 && curtime>edtime)) 
			{
				continue;
			}
			cnt++;
		}
	}
	return cnt;
}


int FDB_CntPhotoCountByPar(int flag, PCmdHeader chdr)
{
	int count;
	if(flag == 1) //统计获取通过的照片
	{
		return CalcCount(0, 0, 0);
	}
	else if(flag == 2) //统计不通过的照片
	{
		return CalcCount(1, 0, 0);
	}
	else if(flag == 0) //统计全部的照片
	{
		count  = FDB_CntPhotoCount();
		printf("count = %d\n",count);
		return count;
	}
	else  //非法标志位
	{
		chdr->Command=CMD_ACK_ERROR;
		return 0;
	}
}


int FDB_GetPhotoByName(char *p, PCmdHeader chdr,PCommSession session)
{
	char *namebuf=NULL;
	int readsize=0;

	char inputname[80];
	memset(inputname, 0, sizeof(inputname));
	memcpy(inputname, p, sizeof(inputname)-1);

	//inputname[strlen(inputname)] = '\0';
	//printf("after memcpy inputname =%s\n",inputname);
	readsize=CheckPhotoFileStatus(inputname);

	//printf("____%s%d, readsize = %d\n", __FILE__, __LINE__, readsize);
	if(readsize<0)
	{
		DBPRINTF("read file  not exists\n");
		chdr->Command=CMD_ACK_ERROR;
		chdr->SessionID=1;
		return -1;
	}
	else
	{
		namebuf=malloc(readsize);
		if (!namebuf)
		{
			chdr->Command=CMD_ACK_ERROR;
			chdr->SessionID=2;
			return -1;
		}
		if((readsize=readfile(inputname,namebuf))<0)
		{
			chdr->Command=CMD_ACK_ERROR;
			chdr->SessionID=3;
			free(namebuf);
			return -1;
		}
		//DBPRINTF("send size is %d\n",readsize);
		SendLargeData((void*)chdr, session, namebuf, readsize);
		free(namebuf);
	}
	return 0;
}

void FreePhotoList(PPhotoList list)
{
	PPhotoList p=list, pTmp=NULL;
	if (p!=NULL)
	{
//		int i=0;
		while (p->next != p)
		{
			pTmp=p->next;
			p->next=pTmp->next;
			//pTmp->next->pre = p;
			free(pTmp);
//			i++;
//			printf("%d, ", i);
		}
		free(p);
		p=NULL;
//		i++;
//		printf("%d\n", i);
	}
}

char * FDB_GetNamesString(time_t stime, time_t etime)
 {
 	int count1=0,count2=0;
 	PPhotoList plist = FDB_GetPhotoToList(stime,etime, NULL, 1, &count1, 0);//指定时间段内pass 文件夹下的照片节点链表
	PPhotoList blist = FDB_GetPhotoToList(stime,etime, NULL, 1, &count2, 1);//指定时间段内bad 文件夹下的照片节点链表
	
	printf("pass=%d, bad=%d\n", count1, count2);

	if(plist == NULL || count1 == 0)
	{
		printf("not find pictrues in derectory pass \n");
	}

	int isize=(count1+count2)*40;
	if (isize==0)
	{
		return NULL;
	}

	char *names =NULL;
	names=(char*)malloc(isize);
	if (names==NULL)
	{
		if (plist)
		{
			FreePhotoList(plist);
		}
		if (blist)
		{
			FreePhotoList(blist);
		}
		return NULL;
	}
	//memset((void*)names, 0, isize);
	printf("FDB_GetNamesString malloc size=%d\n", isize);

	char *q = names;
	if (plist)
	{
		PPhotoList p1=plist;
		int i;
		for(i=0;i<count1;i++)
		{
			//printf("name1 = %s\n",plist->PhotoFile);
			memcpy(q,plist->PhotoFile,strlen(plist->PhotoFile));
			q += strlen(plist->PhotoFile);
			*q = '\t';
			q += 1;
			plist = plist->next;
		}
		printf("free pass list\n");
		FreePhotoList(p1);
	}
	
	if(blist == NULL || count2== 0)
	{
		printf("not find pictrues in derectory bad \n");
	}
	//dsl 2012.2.27
	char *r=NULL;
	if (count1 > 0)
	{
		*q = '\n';//在pass 与bad 文件夹照片名字之间用'\n'分开
		r = (q+1);
	}
	else
	{
		r = q;
	}
	if (blist)
	{
		PPhotoList p1=blist;
		int j;
		for(j=0;j<count2;j++)
		{
			//printf("name2 = %s\n",blist->PhotoFile);
			memcpy(r,blist->PhotoFile,strlen(blist->PhotoFile));
			r +=strlen(blist->PhotoFile);
			*r = '\t';
			r += 1;
			blist = blist->next;
		}
		printf("free bad list\n");
		FreePhotoList(p1);
	}
	*r = '\0';//结尾
	printf("strlen(names) =  %d\n",strlen(names));
	q = NULL;
	r = NULL;
	plist = NULL;
	blist = NULL;
	return names;
 }


int FDB_GetAllPhotoNamesByTime(char * p,PCmdHeader chdr,PCommSession session)
{
	char *pNames=NULL;
	int flag;
	time_t stime,etime;
	memcpy(&flag,p,4);
	printf("get photonames flag = %d\n",flag);
	if(flag == 1)//参数传来起止时间
	{
		memcpy(&stime,p+4,4);
		memcpy(&etime,p+8,4);
	}
	else if(flag == 0)//没有时间段控制
	{
		stime = 0;
		etime = 2145916800;
	}
	else //非法标志
	{
		chdr->Command = CMD_ACK_ERROR;
		return -1;
	}
	printf("stime = %ld,etime = %ld\n",stime,etime);
	TTime ts;
	OldDecodeTime(stime, &ts);
	printf("%4d %02d %02d %02d %02d %02d\n",ts.tm_year+1900,ts.tm_mon+1,ts.tm_mday,ts.tm_hour,ts.tm_min,ts.tm_sec);
	OldDecodeTime(etime, &ts);
	printf("%4d %02d %02d %02d %02d %02d\n",ts.tm_year+1900,ts.tm_mon+1,ts.tm_mday,ts.tm_hour,ts.tm_min,ts.tm_sec);
	
	pNames = FDB_GetNamesString(stime,etime);
	//printf("pNames=%p\n", pNames);

	if(pNames==NULL)
	{
		chdr->Command = CMD_ACK_ERROR;
		return -1;
	}
	if(pNames[0] == '\n')
	{
		free(pNames);
		pNames=NULL;
		chdr->Command = CMD_ACK_ERROR;
		printf("Can not find the pictrues within timespan\n");
		return -1;
	}
	//malloc_trim(0);
	SendLargeData((void*)chdr, session, pNames, strlen(pNames));
	printf("free(pNames)\n");
	free(pNames);
	pNames = NULL;
	
	//printf("all names:\np = %s\n",p);
	printf("exit FDB_GetAllPhotoNamesByTime\n");
	return 0;
}

int DelPhotoFromList(PPhotoList plist, int type)
{
	printf("------type==%d\n", type);
	PPhotoList ptmp=NULL;
	int ret=0;
	char pPath[80], picPath[80];
	if (!plist)
	{
		return -1;
	}

	memset(picPath, 0, sizeof(picPath));
	if (type)
	{
		snprintf(picPath, sizeof(picPath), "%s", GetCapturePath("capture/bad"));
	}
	else
	{
		snprintf(picPath, sizeof(picPath), "%s", GetCapturePath("capture/pass"));
	}

	int i=sizeof(pPath);
	ptmp = plist->next;
	while(ptmp != plist)
	{
		memset(pPath, 0, i);
		sprintf(pPath, "%s/%s.jpg", picPath, ptmp->PhotoFile);
		printf("picture name=%s\n", pPath);
		if (remove(pPath) == 0)
		{
			if((ret=FDB_DelPhotoIndex(ptmp->index))!=FDB_OK)
			{
				break;
			}
		}
		else
		{
			printf("fail to delete\n");
		}
		plist->next = ptmp->next;
		free(ptmp);
		ptmp = plist->next;
	}

	memset(pPath, 0, i);
	sprintf(pPath, "%s/%s.jpg", picPath, ptmp->PhotoFile);
	printf("picture name=%s\n", pPath);
	if (remove(pPath) == 0)
	{
		ret=FDB_DelPhotoIndex(ptmp->index);
	}
	free(plist);
	return ret;
}

int FDB_ClearPhotoByTime(char * p,PCmdHeader chdr,PCommSession session)
{
	int flag,stime,etime,ret;
	//char pPath[80];

	if(p == NULL)
	{
		chdr->Command=CMD_ACK_ERROR;
		return  -1;
	}

	memcpy(&flag,p,4);
	if(flag==0)//把captrue文件夹内的照片全部删掉
	{
		if(FDB_ClrPhoto())
		{
			printf("clear all photos sucessfully\n");
			return 0;
		}
	}
	else if(flag == 1)//根据指定时间段
	{
		memcpy(&stime,p+4,4);
		memcpy(&etime,p+8,4);
	}
	else//非法标志位
	{
		chdr->Command=CMD_ACK_ERROR;
		return -1;
	}

	TTime ts;
	OldDecodeTime((time_t)stime, &ts);
	printf("%4d %02d %02d %02d %02d %02d\n",ts.tm_year+1900,ts.tm_mon+1,ts.tm_mday,ts.tm_hour,ts.tm_min,ts.tm_sec);
	OldDecodeTime((time_t)etime, &ts);
	printf("%4d %02d %02d %02d %02d %02d\n",ts.tm_year+1900,ts.tm_mon+1,ts.tm_mday,ts.tm_hour,ts.tm_min,ts.tm_sec);

	int count1=0,count2=0;
	ret=0;
	PPhotoList plist = FDB_GetPhotoToList(stime, etime, NULL, 1, &count1, 0);// 指定时间段内pass 文件夹中的照片
	if (plist)
	{
		ret=DelPhotoFromList(plist, 0);
	}

	if (ret==0)
	{
		PPhotoList blist = FDB_GetPhotoToList(stime, etime, NULL, 1, &count2, 1);//指定时间段内bad 文件夹中的照片
		ret=DelPhotoFromList(blist, 1);
	}

	if (ret!=0)
	{
		chdr->Command=CMD_ACK_ERROR;
	}
	return ret;
}

int FDB_ClearPhotoAt(PPhotoList h,int length,char * p,int type)
{
	//printf("into FDB_ClearPhotoAt p=%s,type = %d>>>>>>>>>>>>>\n",p,type);
	if(h == NULL)
	{
		return 0;
	}
	
	char * pos = strchr(p,'.');
	if( pos != NULL)
	{
		//printf("strchar(p,'.') != NULL\n");
		*pos = '\0';
		//printf("current name = %s\n",p);
	}
	PPhotoList list = h;//h 是一个循环链表
	int k;
	for(k=0;k<length&&(strcmp(list->PhotoFile,p) != 0);k++)
	{
		//printf("strcmp(%s,%s) != 0\n",list->PhotoFile,p);
		list = list->next;
	}
	//printf("after while list>>>>>>>>>>>>>\n");
	if(strcmp(list->PhotoFile,p) != 0 && (k==length))
	{
		printf("can not find pictrue %s.jpg\n",p);
		return 0;
	}
	if(strcmp(list->PhotoFile,p) == 0)//找到了照片的名字
	{
		//printf("strcmp(%s,%s) == 0\n",list->PhotoFile,p);
		char delcmd[1024];
		if(type == 0)
		{
			sprintf(delcmd,"rm /mnt/mtdblock/capture/pass/%s.jpg && sync ",p);
		}
		else if(type == 1)
		{
			sprintf(delcmd,"rm /mnt/mtdblock/capture/bad/%s.jpg && sync ",p);
		}
		else
		{
			return 0;
		}
		delcmd[strlen(delcmd)] = '\0';
		if(systemEx(delcmd) == -1)//删除文件不成功
		{
			return 0;
		}
		printf("delcmd = %s\n",delcmd);

		//删除索引
		int ret = FDB_ClearIndexBy(p,type);
		if( ret == FDB_OK)
		{
			printf("delete index of %s OK \n",p);
			return 1;
		}
	}
	return 0;
}

static TPhotoIndex gTPI;

int FDB_ClearIndexBy(char * p,int type)
{
	TSearchHandle sh;
	TPhotoIndex s;
	int ret = FDB_OK;

	sh.ContentType = FCT_PHOTOINDEX;
	sh.buffer = (char*)&gTPI;
	SearchFirst(&sh);

	while(!SearchNext(&sh))
	{
		PhotoIndex tmp  = (PhotoIndex)sh.buffer;
		if (tmp->index)
		{
			struct tm tmptime;
			OldDecodeTime(tmp->createtime,&tmptime);
			char namestr[40];
			if (type==0)
			{
				sprintf(namestr, "%04d%02d%02d%02d%02d%02d-%s", tmptime.tm_year+1900, tmptime.tm_mon+1,
					tmptime.tm_mday, tmptime.tm_hour, tmptime.tm_min, tmptime.tm_sec, tmp->userid);
			}
			else
			{
				sprintf(namestr, "%04d%02d%02d%02d%02d%02d", tmptime.tm_year+1900, tmptime.tm_mon+1,
					tmptime.tm_mday, tmptime.tm_hour, tmptime.tm_min, tmptime.tm_sec);
			}
			namestr[strlen(namestr)] = '\0';
			//printf("namestr = %s\n",namestr);
			if(strcmp(namestr,p) == 0)
			{
				//printf("strcmp(namestr,p) == 0\n");
				memset(&s, 0, sizeof(TPhotoIndex));
				lseek(fdpicidx, -1*sizeof(TPhotoIndex), SEEK_CUR);
				if (write(fdpicidx, (void*)&s, sizeof(TPhotoIndex)) != sizeof(TPhotoIndex))
				{
					ret = FDB_ERROR_IO;
					break;
				}
				ret = FDB_DelPhotoID(((PhotoIndex)sh.buffer)->index);
				if (ret == FDB_ERROR_IO)
				{
					break;
				}
				return FDB_OK;
			}
		}
	}

	fsync(fdpicidx);
	return ret;
}

