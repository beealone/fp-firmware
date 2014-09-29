#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "lock.h"

#define LOCKFILE "sqgno.lock"
#define PERMS 0666
extern int errno;
extern int msleep(int s);

int lockTest(const char *lockName)
{
	FILE *f;
//	printf("Lock Test ");
	f=fopen(lockName?lockName:LOCKFILE, "rb");
	if(f)
	{
		fclose(f);
		printf("LOCKED!\n");
		return 1;
	}
//	printf("FREE.\n");
	return 0;
}

int lockStart(const char *lockName)
{
	int tempfd;
//	printf("Lock Start\n");
	return 1;
	while((tempfd=open(lockName?lockName:LOCKFILE,O_RDWR|O_CREAT|O_EXCL,PERMS))<0)
	{
		if(errno!=17)
		{
			printf("open error: %d\n", errno);
			return 0;
		}
		msleep(100);
	}
	close(tempfd);
//	printf("Lock OK.\n");
	return 1;
}

int lockEnd(const char *lockName)
{
	return 1;
	if(unlink(lockName?lockName:LOCKFILE)<0)
	{
		return 0;
	}
//	printf("UNLock.\n");
	return 1;
}

#ifdef LOCK_TEST
int msleep(int s){}
	
main()
{
	int i;
	for(i=0;i<10000;i++)
	{
		lockStart(DB_LOCK);
		lockEnd(DB_LOCK);
	}
}
#endif

