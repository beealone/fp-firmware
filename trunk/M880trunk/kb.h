/*************************************************
                                           
 ZEM 200                                          
                                                    
 kb.h the header file of kb.c                             
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/
                                                                                                               
#ifndef _KB_H_
#define _KB_H_

#include "arca.h"

#define FunKeyCount 24

#define IKey0		0
#define IKeyMenu 	10
#define IKeyOK		11
#define IKeyESC		12
#define IKeyUp		13
#define IKeyDown 	14
#define IKeyPower 	15
#define IKeyOTIn 	16	//加班上
#define IKeyOTOut 	17	//加班下 
#define IKeyTIn 	18	//外出返回
#define IKeyTOut 	19	//外出
#define IKeyIn		20	//上班
#define IKeyOut		21	//下班
#define IKeyBell	22	//Bell
#define IKeyDuress      23	//胁迫报警

#define KeyLayout_BioClockII 	0
#define KeyLayout_BioClockIII 	1
#define KeyLayout_F4 		2
#define KeyLayout_A6 		3

//该函数返回键的对应功能，SecondFun返回第二功能
int GetKeyChar(int key, int *SecondFun);
int GetPowerKeyCode(void); 
int GetStateKeyCode(int keychar);

BYTE *GetKeyStr(BYTE *Buffer, BYTE *Key);
BOOL ProcStateKey(int i);

//检测并返回当前的键值 子进程
//Detect and return current key value
BOOL CheckKeyPad(void);
//void GetKeyFromWiegand(int WiegandKey);

//主进程调用取键盘值
//main procedure call this function from msg.c to get key value
int GetKeyPadValue(unsigned char* KeyValue);

void SetKeyLayouts(char *Buffer);

#endif
