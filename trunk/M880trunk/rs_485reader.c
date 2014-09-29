#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "rs_485reader.h"


#ifdef RS485_MASTER
void *shared_Inbio_Comm = (void *)0;
PInbioCommData shared_Inbio_Comm_stuff = NULL;
int g_inbio_comm_shmid = 0;

int g_iSlaveID = 0;
int g_iVerifyCnt = 0;


int InitReaderMem(void)
{
	printf("init memory Starting....\n");
	g_inbio_comm_shmid = shmget((key_t)12,sizeof(TInbioCommData),0666|IPC_CREAT);
	if(g_inbio_comm_shmid == -1)
	{
		printf("g_inbio_comm_shmid failed\n");
		exit(EXIT_FAILURE);
	}
	shared_Inbio_Comm = shmat(g_inbio_comm_shmid,(void *)0,0);
	if(shared_Inbio_Comm == (void *)-1)
	{
		printf("g_comm_shmid failed\n");
		exit(EXIT_FAILURE);
	}
	shared_Inbio_Comm_stuff = (PInbioCommData )shared_Inbio_Comm;
	printf("shared_Inbio_Comm attached at %X\n",(int)shared_Inbio_Comm);
	memset(shared_Inbio_Comm_stuff,0x00,sizeof(TInbioCommData));
	printf("init memory end....\n");

	return 0;
}


#endif
