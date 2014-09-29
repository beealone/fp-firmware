/*************************************************
  
 ZEM 200                                          
 
 kb.c scan the keypad and get keypad value                             
 
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
 
*************************************************/

#include <string.h>
#include "arca.h"
#include "kb.h"
#include "exfun.h"
#include "options.h"
#include "utils.h"

#define MAXKEYBUFFERLEN 5

static int PressKeyBuffer[MAXKEYBUFFERLEN] = {-1, -1, -1, -1, -1};
static int CurrentPos = -1;

unsigned char KeyLayouts[4][FunKeyCount]={
	//0   1   2   3   4   5   6   7   8   9 MENU OK  ESC UP DOWN PWR OTI OTO TIN TOU IN OUT BELL DURESS
	{'0','1','2','3','4','5','6','7','8','9','D','#','A','B','C','*','3','2','1','B','I','J','J','C'},  	//BioClockI II
	{'0','1','2','3','4','5','6','7','8','9','#','B','A','C','D','*','A','B','C','D','*','#','J','D'}, 	//A5, K8, K9
	{'0','1','2','3','4','5','6','7','8','9','D','#','A','B','C','*','A','B','C','D','*','#','J','C'}, 	//F4+, F8
	{'0','1','2','3','4','5','6','7','8','9','H','I','E','G','F','B','D','C','#','A','*','B','J','F'} 	//A6
};	

unsigned char ConvertWiegandKeyValueToASCII(int WiegandValue)
{
	int MachineKeyPad=0;
	unsigned char WiegandKeyPad[2][FunKeyCount+1]={
		{0,'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D','*','#','A','B','C','D'},  	//A5, K8, F4+, F8
		{0,'1','2','3','4','5','6','7','8','9','*','#','A','B','C','D','E','F','G','H','I','J','0'}  	//A6
	}; 
	if (WiegandValue>FunKeyCount) return 0;
	if (gOptions.KeyLayout==KeyLayout_A6) 
		MachineKeyPad=1;
	else
		MachineKeyPad=0;	    
	return (WiegandKeyPad[MachineKeyPad][WiegandValue]);    
}

#define MAX_KEY_INTERVAL  50*1000   //50ms

void WriteKeyToBuffer(unsigned char KeyValue)
{
	CurrentPos = CurrentPos + 1;
	if (CurrentPos >= MAXKEYBUFFERLEN)
		CurrentPos = 0;
	PressKeyBuffer[CurrentPos] = KeyValue;
	//printf("%c %d\n", KeyValue,gOptions.KeyLayout);
	if(gOptions.KeyPadBeep&&KeyValue) ExBeep(1);
}

#if 0
void GetKeyFromWiegand(int WiegandKey)
{
	WriteKeyToBuffer(ConvertWiegandKeyValueToASCII(WiegandKey));
}
#endif

BOOL GetKeyFromBuffer(unsigned char *KeyValue)
{
	int i, j;
	
	j = CurrentPos + 1;
	for (i = 0; i < MAXKEYBUFFERLEN; i++){
		if (j >= MAXKEYBUFFERLEN)
			j = 0;
		if (PressKeyBuffer[j] != -1){
			*KeyValue = (unsigned char)PressKeyBuffer[j];  
			PressKeyBuffer[j] = -1;
			return TRUE; 
		}
		j++;
	}
	return FALSE;
}

//转换原始序列号码为键盘码 排列次序
//Convert the origin number to key value according the following below
//Keypad use 4 pin GPIO for out and 4 pin GPIO for in (refer line 43-45 in arca.c)
//1   	2	3	A
//4	5 	6	B
//7	8	9 	C
//*	0      #     	D
unsigned char ConvertKeyValueToASCII(unsigned char bits)
{
	unsigned char KEYMAPTABLE[4][4] = {{'1', '2', '3', 'A'}, {'4', '5', '6', 'B'},
					   {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
	unsigned char biti[4]={0x70, 0xb0, 0xd0, 0xe0};
	int i, row, col;
	
	row = 0;
	for(i=0; i<4; i++)
		if ((bits&0xf0) == biti[i]){
		row = 3-i;
		break;
	}
	
	col = 0;
	for(i=0; i<4; i++)
		if ((bits&0x0f) == (biti[i]>>4)){
		col = i;
		break;
	}
	return KEYMAPTABLE[row][col];
}

//检测并返回当前的键值 松开按键时返回键值=0
//detect and return the current keypad value, when release the key, the result is 0 
BOOL CheckKeyPad(void)
{
	unsigned char biti[4]={0x70, 0xb0, 0xd0, 0xe0};
	int i;
	unsigned char highbits;
	static unsigned char LastestKeyValue = 0;
	
	//没有按键 
	//there is no key pressed
	if (LastestKeyValue == 0){
		for(i=0; i<4; i++){
			GPIO_KEY_SET(biti[i]);
#ifdef URU
			DelayUS(1000);	
#else
			DelayUS(4000);	
#endif
			//检测是否有键按下
			//check whether key press or not
			if ((highbits = GPIO_KEY_GET()&0x0f)!=0x0f){
				//防止抖动 
				DelayUS(MAX_KEY_INTERVAL);
				if ((highbits = GPIO_KEY_GET()&0x0f)==0x0f) break; 
				LastestKeyValue = ConvertKeyValueToASCII((unsigned char)highbits + biti[i]);
				WriteKeyToBuffer(LastestKeyValue);
				return TRUE;
			}
		}
	}else{ //检测是否松开按键 
		//detect whether release the key or not
#ifdef URU
		DelayUS(1000);
#else 
		DelayUS(4000);
#endif
		if ((GPIO_KEY_GET()&0x0f)==0x0f){
			LastestKeyValue = 0;
			//通知主程序已经松开按键 
			//key have released and then notify main procedure
			WriteKeyToBuffer(LastestKeyValue);
			return TRUE;
		}
	}
	return FALSE;
}

int GetKeyPadValue(unsigned char *KeyValue)
{
	return GetKeyFromBuffer(KeyValue);
}

int GetStateKeyCode(int keychar)
{
	int i;
	int key;
	int MaxState;
	if(gOptions.ShowCheckIn) MaxState=IKeyOut; else MaxState=IKeyTOut;
	if(keychar>='0') keychar-='0';
	if(keychar<FunKeyCount)  
	{
		key=KeyLayouts[gOptions.KeyLayout][keychar];
		for(i=IKeyOTIn;i<=MaxState;i++)
			if(key==KeyLayouts[gOptions.KeyLayout][i])
				return i;
	}
	return 0;
}

//该函数返回按键的对应功能，SecondFun返回第二功能
int GetKeyChar(int key, int *SecondFun)
{
        int i,ret=key,j;
        *SecondFun=0;
        for(i=0;i<FunKeyCount;i++)
                if(KeyLayouts[gOptions.KeyLayout][i]==key)
                {
                        if(i<10)
                                ret='0'+i;
                        else
                                ret=i;
                        for(j=(i>9?i+1:10);j<FunKeyCount;j++)
                                if(KeyLayouts[gOptions.KeyLayout][j]==key)
                                if(j!=IKeyDuress || gOptions.DuressHelpKeyOn)
                                if((gOptions.ShowState && (gOptions.ShowCheckIn||(j<IKeyIn))) || j>IKeyOut || j<IKeyOTIn )
                                {
                                        *SecondFun=j;
                                        break;
                                }
                        break;
                }
	if(*SecondFun)
        {
                //电源键通常是第二功能，不是第一功能
                if(IKeyPower==ret)
                {
                        ret=*SecondFun;
                        *SecondFun=IKeyPower;
                }
                //菜单键通常是第二功能，不是第一功能
                else if(IKeyMenu==ret)
                {
                        ret=*SecondFun;
                        *SecondFun=IKeyMenu;
                }
        }
        return ret;
}

int GetPowerKeyCode(void)
{
	return KeyLayouts[gOptions.KeyLayout][IKeyPower];
}

void SetKeyLayouts(char *Buffer)
{
	int index, j;
	index=0;
	
	for(j=0;j<(int)strlen(Buffer);j+=2)
		KeyLayouts[gOptions.KeyLayout][index++]=Hex2Char(Buffer+j)*16+Hex2Char(Buffer+j+1);
}
