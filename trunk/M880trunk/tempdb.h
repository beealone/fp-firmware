/*************************************************
                                           
 ZEM 200                                          
                                                    
 tempdb.h                              
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _TEMPDB_H_
#define _TEMPDB_H_

#include "arca.h"

typedef struct{
	U16 UserID;
	BYTE Template[1024];
}TTempDB, *PTempDB;

int TDB_Count(void);
int TDB_Clear(void);
int TDB_ADDTemp(U16 UserID, BYTE *Temp);
PTempDB TDB_GetTemp(int Index);

#endif
