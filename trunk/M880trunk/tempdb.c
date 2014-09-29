/*************************************************
                                           
 ZEM 200                                          
                                                    
 tempdb.c                             
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdlib.h>
#include <string.h>
#include "tempdb.h"

int TDBCount=0;
int TDBSize=0;
PTempDB TDB=NULL;

int TDB_Count(void)
{
	return TDBCount;
}

int TDB_Clear(void)
{
	TDBCount=0;
	return TRUE;
}

int TDB_ADDTemp(U16 UserID, BYTE *Temp)
{
	if(TDBCount+1>TDBSize)
	{
		char *buffer=NULL;
		if(TDB) 
			buffer=(char*)TDB;
		TDB=(PTempDB)MALLOC(sizeof(TTempDB)*(TDBCount+20));
		if(buffer)
		{
			memcpy(TDB, buffer, sizeof(TTempDB)*(TDBCount));
			FREE(buffer);
		}
		TDBSize=TDBCount+20;
	}
	TDB[TDBCount].UserID=UserID;
	memcpy(TDB[TDBCount].Template, Temp, 1024);
	return ++TDBCount;
}

PTempDB TDB_GetTemp(int Index)
{
	if((Index<0) || (Index>=TDBCount))
		return FALSE;
	return TDB+Index;
}
