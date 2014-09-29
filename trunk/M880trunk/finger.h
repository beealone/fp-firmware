/*************************************************
                                           
 ZEM 200                                          
                                                    
 finger.h                             
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _FINGER_H_
#define _FINGER_H_

#include "zkfp.h"

#define MSPEED_AUTO 	2
#define IDENTIFYSPEED 	(gOptions.MSpeed<MSPEED_AUTO?gOptions.MSpeed:(FDB_CntTmp()>400?SPEED_HIGH:SPEED_LOW))

extern HANDLE fhdl;

int FPDBInit(void);
void FPFree(void);
int FPInit(char *FingerBuf);
void biokey_get_param_lock_free(void);
int BIOKEY_GET_PARAM(HANDLE Handle, int ParameterCode, int *ParameterValue);
int biokey_get_param_lock_init(void);

#endif
