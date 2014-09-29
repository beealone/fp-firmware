/*T9*/
/*author:liming*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "minigui/common.h" 
#include "minigui/minigui.h" 
#include "minigui/gdi.h" 
#include "minigui/window.h" 
#include "minigui/control.h" 
#include "minigui/endianrw.h" 
#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"
#include "exfun.h"

#include "t9mb.h"
#include "t9.h"

//全局变量
HWND sg_hTargetWnd = 0; 				// 输入的目标窗口.
HWND hIMEWnd = 0;
int IMEMode;			//
const T9PY_IDX* pt9idx;

char en_Buf[MAX_KEY_CHAR];	//当前的字母(用于英文输入方式)
char py_key[MAX_PY_LENGTH+1];	//中文输入中拼音输入键序列
char py_Buf[MAX_PY_SAME][MAX_PY_LENGTH+1];				//拼音
unsigned char hz_Buf[MAX_HZ_COUNT*2];					//汉字
unsigned char fh_Buf[50];

int	perpagecount;		//每页显示的文字个数
int	input_type;		//输入方式	0:拼音,1:英文,2:符号
int 	cur_status;		//输入状态	0.输入状态; 1.选择状态
int 	start_point;		//显示数据的起始位置
int	py_keycounts;		//中文输入键个数(拼音长度)
int 	cur_py;			//当前拼音序号
int	py_count;		//拼音个数

//function define
static void InputKeyProcess(unsigned char key, LPARAM lParam);
static void init_ime_parameter(void);
static void RefreshIMEBox(HWND hWnd, HDC hDC);
static void ClrInBuf(void);

static BOOL py_key_filter(char key);				//拼音输入键是否有效
static void get_py_from_table(void);
static void get_hz_from_table(char* py);
static void get_en_from_table(char key);
static void ime_writemsg(int sindex);
static void delete_last_py(void);

char* ime_method[3];
static const unsigned char fh_table[] = "~!@#$%^&*()_+{}|:\"<>?`-=[]\\;',./，。‘’“”";

//funcion process
static void init_ime_parameter(void)
{
	memset(fh_Buf,0,sizeof(fh_Buf));
	nmemcpy(fh_Buf,fh_table,strlen((char*)fh_table));

	perpagecount = MAX_ROW_COUNTS;
	cur_py = 0;
	py_keycounts = 0;
	py_count = 0;
	start_point = 0;
	input_type = (IMEMode)?0:1;	//默认输入方式
	cur_status = 0;			//初始输入状态
}

static void ClrInBuf(void)
{
	switch(input_type)
	{
		case 0:
			memset(hz_Buf,0,MAX_HZ_COUNT*2);
			memset(py_key,0,MAX_PY_LENGTH+1);		//清除拼音输入
			memset(py_Buf,0,MAX_PY_LENGTH+1);
			py_keycounts = 0;
			cur_status = 0;
			cur_py = 0;
			start_point = 0;
			py_count = 0;
			break;
		case 1:
			memset(en_Buf, 0, MAX_KEY_CHAR);
			cur_status = 0;
			break;
		case 2:
			cur_status = 1;		//符号输入直接进入选择状态
			break;
	}
	//printf("%s IMEMode %d\n", __FUNCTION__, IMEMode);
}

static void RefreshIMEBox(HWND hWnd, HDC hDC)
{
	char str[100];
	char sstr[100];
	char tmppy[MAX_PY_LENGTH+1];		//当前拼音
	char tmpstr[4];				//读取汉字

	PLOGFONT logfont;
	SIZE size;
	int i;
	int pos_x, cWidth, sWidth, tmpvalue = 0;			//绘制选择框

	SetBkColor(hDC,GetWindowElementColor(BKC_CONTROL_DEF));

	memset(str,0,sizeof(str));
	strcpy(str,ime_method[input_type]);			//写输入方式标识

	switch(input_type)
	{
		case 0:			//input chinese
			TextOut(hDC,2,2,str);

			GetTextExtent(HDC_SCREEN, str, strlen(str), &size);
			cWidth = GetMaxFontWidth(hDC)/2;		//一个字节宽度
			pos_x = (strlen(str))*cWidth;

			if(py_keycounts>0)
			{
				get_py_from_table();		//get py from table
				if(cur_status == 0)
				{
					for(i=0;i<py_count;i++)
						sprintf(str,"%s%s ",str,py_Buf[i]);
					
					for(i=0;i<cur_py;i++)
						pos_x += (strlen(py_Buf[i])+1)*cWidth;

					sWidth = (strlen(py_Buf[cur_py]))*cWidth+6;

					tmpvalue = SetBkMode(hDC,BM_TRANSPARENT);
					SetBrushColor(hDC,COLOR_blue);
					FillBox (hDC, pos_x ,2 ,sWidth, size.cy);
				}
				else if(cur_status == 1)
					sprintf(str,"%s%s",str,py_Buf[cur_py]);

				TextOut(hDC, 2, 2, str);	//output first line
			
				memset(tmppy,0,MAX_PY_LENGTH+1);
				strncpy(tmppy,py_Buf[cur_py],MAX_PY_LENGTH);
				get_hz_from_table(tmppy);	//get hz from table

				memset(str,0,4*perpagecount+2);
				if((start_point >= perpagecount) && (cur_status == 1))
					sprintf(str,"%s","<");
				else
					sprintf(str,"%s"," ");

				memset(sstr,0,4*perpagecount);
				for(i=0;i<perpagecount;i++)
				{
					memset(tmpstr,0,sizeof(tmpstr));
					tmpstr[0] = hz_Buf[(start_point+i)*2];
					tmpstr[1] = hz_Buf[(start_point+i)*2+1];
					tmpstr[2] = '\0';

					if(tmpstr[0]!='\0' || tmpstr[1] != '\0')
					{
						if(cur_status == 0) sprintf(sstr,"%s  %s",sstr,tmpstr);
						else if(cur_status == 1) sprintf(sstr,"%s%d.%s",sstr,i,tmpstr);
					}
				}

				if((((start_point+perpagecount)*2) < strlen((char*)hz_Buf)) && (cur_status == 1))
					sprintf(str,"%s%s%s",str,sstr,">");
				else
					sprintf(str,"%s%s%s",str,sstr," ");

				logfont = GetSystemFont(SYSLOGFONT_CONTROL);			//设置中文显示字体
				SelectFont (HDC_SCREEN, logfont);
				TextOut(hDC, 2, size.cy+2, str);	

			}		
			break;

		case 1:			//英文输入显示
			TextOut(hDC, 2, 2, str);
			memset(str,0,sizeof(str));
		
			GetTextExtent (HDC_SCREEN, en_Buf, strlen(en_Buf), &size);
			for(i = 0; i < strlen(en_Buf); i++){
				if(cur_status == 0)
					sprintf(str,"%s  %c  ",str,en_Buf[i]);
				else if(cur_status == 1)
					sprintf(str,"%s%d %c  ",str,i,en_Buf[i]);
			}

			TextOut(hDC,2+2,size.cy+2,str);
			break;

		case 2:			//符号输入
			TextOut(hDC, 2, 2, str);
			GetTextExtent(HDC_SCREEN, str, strlen(str), &size);
			memset(str,0,4*perpagecount+2);

			if((start_point >= perpagecount) && (cur_status == 1)) sprintf(str,"%s","<");
			else	sprintf(str,"%s"," ");
			
			memset(sstr,0,4*perpagecount);
			for(i=0;i<perpagecount;i++){
				if((start_point+i)<32)
					sprintf(sstr,"%s %d %c",sstr,i,fh_Buf[start_point+i]);
				else{
					if((int)(32+(start_point+i-32)*2) > (int)(sizeof(fh_table) -1))
						break;
					if((fh_Buf[32+(start_point+i-32)*2] != '\0')||(fh_Buf[32+(start_point+i-32)*2+1] != '\0')){
						memset(tmpstr,0,sizeof(tmpstr));
						tmpstr[0] = fh_Buf[32+(start_point+i-32)*2];
						tmpstr[1] = fh_Buf[32+(start_point+i-32)*2+1];
						sprintf(sstr,"%s %d %s",sstr,i,tmpstr);
					}
				}
			}

			if((start_point+perpagecount) < 38 && (cur_status == 1))
				sprintf(str,"%s%s %s",str,sstr,">");
			else
				sprintf(str,"%s%s %s",str,sstr," ");

			logfont = GetSystemFont(SYSLOGFONT_CONTROL);
			SelectFont (HDC_SCREEN, logfont);				
			TextOut(hDC, 2, size.cy+2, str);
			break;
	}
}

static void get_hz_from_table(char* py)
{
	int i;
	BOOL matchflag = FALSE;
	
	pt9idx = t9PY_index;
	memset(hz_Buf,0,MAX_HZ_COUNT*2);

	while(pt9idx->PY[0] != '\0'){
		matchflag = TRUE;
		if(strlen(pt9idx->PY) == py_keycounts){
			for(i=0;i<py_keycounts;i++){
				if(py[i] != pt9idx->PY[i]) 
					matchflag = FALSE;
			}
		}
		else
			matchflag = FALSE;

		if(matchflag){
			i = 0;
			while(pt9idx->MB[i] != '\0'){
				hz_Buf[i] = pt9idx->MB[i];
				i++;
			}
			break;
		}
		pt9idx++;
	}
}

static void get_py_from_table(void)
{
	int i;
	BOOL matchflag;
	pt9idx = t9PY_index;
	py_count = 0;

	for(i=0;i<MAX_PY_SAME;i++)
		memset(py_Buf[i],0,MAX_PY_LENGTH+1);

	while(pt9idx->T9[0] != '\0'){
		matchflag = TRUE;
		if(strlen(pt9idx->T9) == py_keycounts){
			for(i=0;i<py_keycounts;i++){
				if(py_key[i] != pt9idx->T9[i]) matchflag = FALSE;
			}
		}
		else
			matchflag = FALSE;
	
		if(matchflag){
			strcpy(py_Buf[py_count++],pt9idx->PY);
		}

		pt9idx++;
	}
}

static BOOL py_key_filter(char key)
{
	char tmpkey[MAX_PY_LENGTH];
	int i;
	BOOL matchflag;
	BOOL newpyflag;											//输入键是否有效

	pt9idx = t9PY_index;

	strcpy(tmpkey,py_key);									//记录已输入键序列
	
	if(py_keycounts < MAX_PY_LENGTH){
		sprintf(tmpkey,"%s%c",tmpkey,key);				//暂存新输入键
		newpyflag = FALSE;
		while(pt9idx->T9[0] != '\0'){
			matchflag = TRUE;
			if(strlen(pt9idx->T9) == py_keycounts +1){
				for(i=0;i<py_keycounts+1;i++){
					if(tmpkey[i] != pt9idx->T9[i]) matchflag = FALSE;
				}
			}
			else
				matchflag = FALSE;

			if(matchflag) return TRUE;	
			pt9idx++;
		}
	}
	return FALSE;

}

static void get_en_from_table(char key)
{
	T9EN_IDX* penidx;
	penidx = (T9EN_IDX*)t9EN_index;
	int i = 0;

	for(i=0;i<8;i++){
		if(key == penidx->key[0]){
			strcpy(en_Buf,penidx->Letter);
			cur_status = 1;
			break;
		}
		penidx++;
	}
}

static void ime_writemsg(int sindex)
{	
	WORD wDByte;
	unsigned char cc [2];

	switch(input_type){
		case 0:	/*将选择的汉字写入文本框*/
			if(sindex < perpagecount){
				cc[0] = hz_Buf[(start_point + sindex)*2];
				cc[1] = hz_Buf[(start_point + sindex)*2 + 1];
				wDByte = MAKEWORD(cc[0], cc[1]);
				if(sg_hTargetWnd && wDByte){
					PostMessage(sg_hTargetWnd,MSG_CHAR,wDByte,0);
					ClrInBuf();
				}
			}
			break;
				
		case 1:	/*将选择的英文字母写入文本框*/
			if((sg_hTargetWnd)&& (en_Buf[sindex] != '\0')){
				SendMessage(sg_hTargetWnd,MSG_CHAR,en_Buf[sindex],0);
				ClrInBuf();
			}
			break;

		case 2:	/*将选择的符号写入文本框*/
			if((sindex < perpagecount) && ((int)(32+(start_point+sindex-32)*2) <= (int)(sizeof(fh_table) - 1))){
				if((start_point + sindex)<32){
					if(sg_hTargetWnd)
						SendMessage(sg_hTargetWnd,MSG_CHAR,fh_Buf[start_point + sindex],0);
				}
				else{
					cc[0] = fh_Buf[32+(start_point+sindex-32)*2];
					cc[1] = fh_Buf[32+(start_point+sindex-32)*2+1];
					wDByte = MAKEWORD(cc[0],cc[1]);
  					if (sg_hTargetWnd) PostMessage (sg_hTargetWnd, MSG_CHAR, wDByte, 0);
				}
			}	
			break;
	}
						
}

/*删除最近一个拼音字母*/
static void delete_last_py(void)
{
	char tmpkey[MAX_PY_LENGTH];
	memset(tmpkey,0,MAX_PY_LENGTH);
	memcpy(tmpkey,py_key,py_keycounts-1);
	memset(py_key,0,MAX_PY_LENGTH);
	memcpy(py_key,tmpkey,py_keycounts--);

	cur_py = 0;
	start_point = 0;
}

static void InputKeyProcess(unsigned char key, LPARAM lParam)
{
	const char ckey[] = "0123456789";

	char skey = 0;
	int sel_index = 0;

	skey = (char)ckey[key - myKEY_0];
	sel_index = (int)(key - myKEY_0);

	switch(cur_status){
		case 0:		//键盘输入状态(只有中/英文输入才进入此状态)
			if(key >= myKEY_2){
				switch(input_type){
					//中文输入
					case 0:
						if(py_key_filter(skey)){
							sprintf(py_key,"%s%c",py_key,skey);		//保存输入键
							cur_py=0;
							start_point=0;
							py_keycounts++;
						}
						break;
					//英文输入
					case 1:
						get_en_from_table(skey);
						break;
				}
			}
//			else if(key == myKEY_0)
//				if(sg_hTargetWnd) PostMessage(sg_hTargetWnd,MSG_KEYDOWN,SCANCODE_SPACE,0);

			break;
		case 1:					//选择状态
			ime_writemsg(sel_index);
			break;
	}
}

//-------------------创建输入法窗口------------------
int T9IMEWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message) 
	{
		case MSG_NCCREATE:
			ime_method[0]=LoadStrByID(MID_IMEMETHOD1);
			ime_method[1]=LoadStrByID(MID_IMEMETHOD2);
			ime_method[2]=LoadStrByID(MID_IMEMETHOD3);

			init_ime_parameter();			//初始化输入法
			memset(en_Buf,0,MAX_KEY_CHAR);
			memset(py_key,0,MAX_PY_LENGTH+1);
			memset(py_Buf,0,MAX_PY_SAME * (MAX_PY_LENGTH+1));
			memset(hz_Buf,0,MAX_HZ_COUNT*2);

			SendMessage (HWND_DESKTOP, MSG_IME_REGISTER, (WPARAM)hWnd, 0); //打开窗口前先注册
			break;

		case MSG_IME_SETTARGET:
			if (sg_hTargetWnd != hWnd)
				sg_hTargetWnd = (HWND)wParam;
			break;

		case MSG_IME_GETTARGET:
			return (int)sg_hTargetWnd;
			break;

		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}

			switch(wParam)
			{
				case SCANCODE_ESCAPE:			//.ESC键 关闭输入法
					SendMessage(hWnd,MSG_CLOSE,0,0);
					break;

				case SCANCODE_BACKSPACE:		//.Backspace键(#)
					switch(input_type)
					{
						case 0:
							if(cur_status == 1)
							{
								if(gOptions.KeyPadBeep)
									ExKeyBeep();
								cur_status = 0;
								InvalidateRect(hWnd, NULL, TRUE);
							}
							else
							{
								if(strlen(py_key)>0)
								{
									if(gOptions.KeyPadBeep)
						                                ExKeyBeep();
									delete_last_py();	//删除前一个拼音字母
									InvalidateRect(hWnd, NULL, TRUE);
								}
								else
								{
									if(sg_hTargetWnd)
									{
										//PostMessage(sg_hTargetWnd, MSG_KEYDOWN, wParam, lParam);
										PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_BACKSPACE, lParam);
									}
								}
							}
							break;

						case 1:
							if(cur_status == 1)
							{
								if(gOptions.KeyPadBeep)
									ExKeyBeep();
								ClrInBuf();			//清除英文输入？
								InvalidateRect(hWnd, NULL, TRUE);
							}
							else
							{
								if(sg_hTargetWnd) 
									PostMessage(sg_hTargetWnd, MSG_KEYDOWN, wParam, lParam);
							}
							break;

						case 2:
							if(sg_hTargetWnd) PostMessage (sg_hTargetWnd, MSG_KEYDOWN, wParam, lParam);
							break;
					}	
					break;

				case SCANCODE_F11:
				case SCANCODE_F9:
//				case SCANCODE_TAB:			//.Tab键(*) : 切换输入法
					if(gOptions.TFTKeyLayout!=3)
					{
			                if(LOWORD(wParam)==SCANCODE_F9 || 
						((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && (LOWORD(wParam)==SCANCODE_F11)))
						{
						if(gOptions.KeyPadBeep)
			                                ExKeyBeep();

						if(++input_type > 2)
							input_type = (IMEMode)?0:1;

						ClrInBuf();
						InvalidateRect(hWnd, NULL, TRUE);
						}
					}
					break;
				case SCANCODE_F12:
					if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
					{
						if(sg_hTargetWnd)
							PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_SPACE, 0);
					}
					break;
				case SCANCODE_SPACE:
					if(sg_hTargetWnd)
						PostMessage(sg_hTargetWnd,MSG_KEYDOWN,SCANCODE_SPACE,0);
					break;
				case SCANCODE_CURSORBLOCKUP:	
					if(gOptions.TFTKeyLayout!=3)
					{
						if(sg_hTargetWnd)
						 PostMessage(sg_hTargetWnd,MSG_KEYDOWN,wParam,lParam);
					}	
					else
					{
						switch(input_type)
						{
						case 0:
							if(cur_status == 0 && py_count > 0 && cur_py >0)
							{
								if(gOptions.KeyPadBeep)
			                                ExKeyBeep();
								cur_py--;
								InvalidateRect(hWnd, NULL, TRUE);
							}
							else if(cur_status == 1 && (strlen((char *)hz_Buf)/2) > perpagecount)
							{
								if(gOptions.KeyPadBeep)
				        	ExKeyBeep();
								if(start_point >= perpagecount) 
									start_point -= perpagecount;
								InvalidateRect(hWnd, NULL, TRUE);
							}
							else if(py_count == 0)
							{
								if(sg_hTargetWnd) 
									PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKUP,lParam);
							}
							break;

						case 1:
							if(sg_hTargetWnd) 
								PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKUP,lParam);
							break;

						case 2:
							if(gOptions.KeyPadBeep)
				        ExKeyBeep();

							if(start_point >=perpagecount) start_point -= perpagecount;
							InvalidateRect(hWnd, NULL, TRUE);
							break;	
						}
					}
					break;

				case SCANCODE_CURSORBLOCKDOWN:
					if(gOptions.TFTKeyLayout!=3)
					{
						if(sg_hTargetWnd)
							 PostMessage(sg_hTargetWnd,MSG_KEYDOWN,wParam,lParam);
						break;
					}
					else
					{
						switch(input_type)
						{
							case 0:
								if(cur_status == 0 && py_count >0 && cur_py < py_count-1)
								{
									if(gOptions.KeyPadBeep)
					                                ExKeyBeep();
									cur_py++;
									InvalidateRect(hWnd, NULL, TRUE);
								}
								else if(cur_status == 1 && (strlen((char *)hz_Buf)/2) > perpagecount)
								{
									if(gOptions.KeyPadBeep)
					                                ExKeyBeep();
									if((start_point + perpagecount) < strlen((char *)hz_Buf)/2) 
										start_point += perpagecount;
									InvalidateRect(hWnd, NULL, TRUE);
								}
								else if(py_count == 0)
								{
									if(sg_hTargetWnd) 
										PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKDOWN,lParam);
								}
								break;
						
							case 1:
								if(sg_hTargetWnd) 
									PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKDOWN,lParam);
								break;

							case 2:
								if(gOptions.KeyPadBeep)
				                                ExKeyBeep();

								if((start_point + perpagecount) < 38) 
									start_point += perpagecount;
								InvalidateRect(hWnd, NULL, TRUE);
								break;
							}
					}
					break;

				case SCANCODE_CURSORBLOCKLEFT:			//向前翻页
					switch(input_type)
					{
					case 0:
						if(cur_status == 0 && py_count > 0 && cur_py >0)
						{
							if(gOptions.KeyPadBeep)
				                                ExKeyBeep();
							cur_py--;
							InvalidateRect(hWnd, NULL, TRUE);
						}
						else if(cur_status == 1 && (strlen((char*)hz_Buf)/2) > perpagecount)
						{
							if(gOptions.KeyPadBeep)
				                                ExKeyBeep();
							if(start_point >= perpagecount) 
								start_point -= perpagecount;
							InvalidateRect(hWnd, NULL, TRUE);
						}
						else if(py_count == 0)
						{
							if(sg_hTargetWnd) 
								PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKLEFT,lParam);
						}
						break;

					case 1:
						if(sg_hTargetWnd) 
							PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKLEFT,lParam);
						break;

					case 2:
						if(gOptions.KeyPadBeep)
			                                ExKeyBeep();

						if(start_point >=perpagecount) start_point -= perpagecount;
						InvalidateRect(hWnd, NULL, TRUE);
						break;	
					}					
					break;

				case SCANCODE_CURSORBLOCKRIGHT:			//向后翻页
					if(gOptions.TFTKeyLayout!=3)
					{
					switch(input_type)
					{
						case 0:
							if(cur_status == 0 && py_count >0 && cur_py < py_count-1)
							{
								if(gOptions.KeyPadBeep)
					                                ExKeyBeep();
								cur_py++;
								InvalidateRect(hWnd, NULL, TRUE);
							}
							else if(cur_status == 1 && (strlen((char*)hz_Buf)/2) > perpagecount)
							{
								if(gOptions.KeyPadBeep)
					                                ExKeyBeep();
								if((start_point + perpagecount) < strlen((char*)hz_Buf)/2) 
									start_point += perpagecount;
								InvalidateRect(hWnd, NULL, TRUE);
							}
							else if(py_count == 0)
							{
								if(sg_hTargetWnd) 
									PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKRIGHT,lParam);
							}
							break;
						
						case 1:
							if(sg_hTargetWnd) 
								PostMessage(sg_hTargetWnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKRIGHT,lParam);
							break;

						case 2:
							if(gOptions.KeyPadBeep)
				                                ExKeyBeep();

							if((start_point + perpagecount) < 38) 
								start_point += perpagecount;
							InvalidateRect(hWnd, NULL, TRUE);
							break;
					}
					}
					else//右键(×) : 切换输入法
					{
						if(gOptions.KeyPadBeep)
							ExKeyBeep();

						if(++input_type > 2)
							input_type = (IMEMode)?0:1;

						ClrInBuf();
						InvalidateRect(hWnd, NULL, TRUE);
					}					
					break;

				//输入选择		
				case SCANCODE_F10:
				case SCANCODE_ENTER:		//Enter键
					if(gOptions.KeyPadBeep)
		                                ExKeyBeep();

					switch(input_type)
					{
						case 0:
							if(hz_Buf[0] != '\0')
							{
								if(cur_status==0) 
									cur_status = 1;
								else
									ime_writemsg(0);
							}
							break;
						case 1:
							if(en_Buf[0] != '\0')
								ime_writemsg(0);
							break;
						case 2:
							ime_writemsg(0);
							break;
					}
					InvalidateRect(hWnd, NULL, TRUE);
					return 0;
			}
			return 0;

		case MSG_CHAR:
			if((BYTE)wParam >= myKEY_0 && (BYTE)wParam <= myKEY_9)
			{
				if(gOptions.KeyPadBeep)
	                                ExKeyBeep();

				InputKeyProcess((BYTE)wParam, lParam);
				InvalidateRect(hWnd, NULL, TRUE);
			}
			return 0;
   	    
		case MSG_PAINT:
			hdc = BeginPaint (hWnd);
			RefreshIMEBox(hWnd, hdc);
			EndPaint (hWnd, hdc);
			return 0;

		case MSG_CLOSE:
			SendMessage(HWND_DESKTOP,MSG_IME_UNREGISTER,(WPARAM)hWnd, 0);
			DestroyMainWindow (hWnd);
			return 0;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);

}

HWND T9IMEWindow(HWND hosting, int lx, int ty, int rx, int by, int imemode)
{
	MSG msg;
	MAINWINCREATE CreateInfo;
	
	IMEMode = imemode;

	hosting = GetMainWindowHandle(hosting);
	CreateInfo.dwStyle = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "ime";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = T9IMEWinProc;
	CreateInfo.lx = lx;
	CreateInfo.ty = ty;
	CreateInfo.rx = rx;
	CreateInfo.by = by;
	CreateInfo.iBkColor = GetWindowElementColor(BKC_CONTROL_DEF);
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hosting;

	hIMEWnd = CreateMainWindow (&CreateInfo);
	if (hIMEWnd == HWND_INVALID)
		return HWND_INVALID;

	ShowWindow (hIMEWnd, SW_SHOWNORMAL); 

	while (GetMessage(&msg,hIMEWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hIMEWnd);

	return hIMEWnd;
}
