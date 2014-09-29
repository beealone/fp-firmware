#include "rs.h"
#include <time.h>
#include "flashdb.h"

/*
#ifndef ZEM600
#ifndef LINUX_26KERNEL
typedef unsigned long U32;
typedef unsigned char BYTE;
#endif
#endif
*/

time_t GetLastByAttLog(RS *r,int fd)
{
	TAttLog att;
	if(fd>0)
	{
		lseek(fd,-1*sizeof(TAttLog),SEEK_END);
		if(read(fd,&att,sizeof(TAttLog))==sizeof(TAttLog))
			return r->lasttime=att.time_second;
	}
	return 0;
}

int init_rs(RS *r,unsigned long size)
{
	if(size)
	{
		r->first=0;
		r->last=0;
		r->lasttime=0;
		r->total=0;
		r->InBuffer=(unsigned char *)MALLOC(size*sizeof(TAttLog));
		if(r->InBuffer)
		{
			r->total=size;
			memset(r->InBuffer,0,size*sizeof(TAttLog));
			return 1;
		}
	}
	return 0;
}

void free_rs(RS *r)
{
	if(r->InBuffer)
		FREE(r->InBuffer);
}

void add_attlog_queue(RS *r,TAttLog *att)
{
	TAttLog *list=(TAttLog*)r->InBuffer;
	r->lasttime=att->time_second;

	if( ((r->last+1) % r->total) ==r->first)
	{
		r->first=(r->first+1) % r->total;
	}

	memcpy(&list[r->last],att,sizeof(TAttLog));
	r->last=(r->last+1) % r->total;
}

TAttLog *ref_attlog_queue(RS *r)
{
	TAttLog *list=(TAttLog*)r->InBuffer;
	TAttLog *att;

	if(r->first==r->last)
		return NULL;
		
	att=&list[r->first];
	r->first=(r->first+1) % r->total;
	return att;
}

int isEmpty(RS *r)
{
	if(r->first==r->last)
		return 1;
	return 0;
}
