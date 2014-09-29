//多国语言支持
/*******************************************************
 **
 **  2008.07.19
 **	modify int Str2UTF8() for cp1255 by jazzy
 **	add UTF8toLocal() for UTF-8 to local Language(cp1255)
 **
 **************************************************************************
 */
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <asm/unaligned.h>
#include <fcntl.h>


#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>
#include "options.h"
#include "locale.h"
#include "ssrpub.h"
#include "flashdb.h"
#include "utils.h"

unsigned char *tftDataBuf=NULL;
int codevaluecount=0;

unsigned char  *tftloadfontfile(int lid)
{
	static int fp;
	char buf[64];
	int len=0;

	memset(buf,0,sizeof(buf));
	if (lid==LID_BIG5)
	{
		fp=open(GetEnvFilePath("USERDATAPATH", "lib/font/big5.dat", buf), O_RDONLY);
	}
	else if (lid==LID_JAPANESE)
	{
		fp=open(GetEnvFilePath("USERDATAPATH", "lib/font/japan.dat", buf), O_RDONLY);
	}

	if (fp == -1)
		return NULL;

	len=lseek(fp, 0, SEEK_END);
	if(len)
	{
		codevaluecount=len/sizeof(TCodeConvert);
		tftDataBuf=MALLOC(len);
		lseek(fp, 0, SEEK_SET);
		if(read(fp, tftDataBuf, len)!=len)
		{
			FREE(tftDataBuf);
			tftDataBuf=NULL;
		}
	}
	close(fp);
	return tftDataBuf;
}

U16 SearchvalueByCode(U16 incode)
{
	int i=0;
	PCodeConvert mycodeconvert;

	mycodeconvert=(PCodeConvert)tftDataBuf;
	while(i<codevaluecount)
	{
		//printf("codevalue:%x=%x\n",mycodeconvert[i].code,mycodeconvert[i].value);
		if (mycodeconvert[i].code==incode)
			return mycodeconvert[i].value;
		i++;	
	}
	return 0;


}

U16 SearchCodeByValue(U16 Value)
{
	int i=0;
	PCodeConvert mycodeconvert;

	mycodeconvert=(PCodeConvert)tftDataBuf;
	while(i<codevaluecount)
	{
		//printf("codevalue:%x=%x\n",mycodeconvert[i].code,mycodeconvert[i].value);
		if (mycodeconvert[i].value==Value)
			return mycodeconvert[i].code;
		i++;
	}
	return 0;
}

int UCS2toUTF8Code(unsigned short ucs2_code, unsigned char * utf8_code){
	int length = 0;
	unsigned char * out = utf8_code;
	if(!utf8_code){
		return length;
	}
	if(0x0080 > ucs2_code){
		*out = (unsigned char)ucs2_code;
		length++;
	}
	else if(0x0800 > ucs2_code){
		*out = ((unsigned char)(ucs2_code >> 6)) | 0xc0;
		*(out+1) = ((unsigned char)(ucs2_code & 0x003F)) | 0x80;
		length += 2;
	}
	else{
		*out = ((unsigned char)(ucs2_code >> 12)) | 0xE0;
		*(out+1) = ((unsigned char)((ucs2_code & 0x0FC0)>> 6)) | 0x80;
		*(out+2) = ((unsigned char)(ucs2_code & 0x003F)) | 0x80;
		length += 3;
	}
	return length;
}

int Thai2UTF8(unsigned char *thai, unsigned char *utf8)
{       int  len=0;
	while(1)
	{
		unsigned char b=*thai++;
		unsigned short ucs2=b;
		if(b==0x80) ucs2=0x20ac;
		else if(b==0x85) ucs2=0x2026;
		else if(b==0x91) ucs2=0x2018;
		else if(b==0x92) ucs2=0x2019;
		else if(b==0x93) ucs2=0x201c;
		else if(b==0x94) ucs2=0x201d;
		else if(b==0x95) ucs2=0x2022;
		else if(b==0x96) ucs2=0x2013;
		else if(b==0x97) ucs2=0x2014;
		else if(b>=0xa1 && b<=0xFB)
		{
			ucs2=0x0E01+b-0xa1;
		}
		len+=UCS2toUTF8Code(ucs2, utf8+len);
		if(b==0) break;
	}
	return len;
}

static int tftjisx_pos_first_char (const unsigned char* mstr, int mstrlen)
{
	unsigned char ch1;
	unsigned char ch2;
	int i, left;

	i = 0;
	left = mstrlen;
	while (left) {
		if (left < 2) return -1;

		ch1 = mstr [i];
		if (ch1 == '\0') return -1;

		ch2 = mstr [i + 1];
		if (((ch1 >= 0x81 && ch1 <= 0x9F) || (ch1 >= 0xE0 && ch1 <= 0xEF)) &&
				((ch2 >= 0x40 && ch2 <= 0x7E) || (ch2 >= 0x80 && ch2 <= 0xFC)))
			return i;

		i += 1;
		left -= 1;
	}

	return -1;
}


static int tftbig5_pos_first_char (const unsigned char* mstr, int mstrlen)
{
	unsigned char ch1;
	unsigned char ch2;
	int i, left;

	i = 0;
	left = mstrlen;
	while (left) {
		if (left < 2) return -1;

		ch1 = mstr [i];
		if (ch1 == '\0') return -1;

		ch2 = mstr [i + 1];
		if (ch1 >= 0xA1 && ch1 <= 0xFE &&
				((ch2 >=0x40 && ch2 <= 0x7E) || (ch2 >= 0xA1 && ch2 <= 0xFE)))
			return i;

		i += 1;
		left -= 1;
	}

	return -1;
}


int Str2UTF8(int lid,unsigned char *str, unsigned char *utf8)
{
	int len=0;

	if (!str)
	{
		utf8=(unsigned char *)"";
		return 1;
	}
	if ((lid==LID_ENGLISH) || (lid==LID_GB2312) || (lid==LID_CP1256))
	{
		memcpy(utf8,str,strlen((char*)str));
		return 1;
	}

	//日语
	if (lid==LID_JAPANESE)
	{
		int l;
		int jappos;
		int i=0;
		int k=0;
		unsigned short mycode;

		l=strlen((char*)str);
		while (i<l)
		{
			jappos=tftjisx_pos_first_char(str,(l-i));
			if (jappos==0)
			{
				mycode=get_unaligned((U16*)str);
				mycode=((mycode&0xFF)<<8)|((mycode&0xFF00)>>8);
				mycode=SearchvalueByCode(mycode);
				//if (mycode <0x9640)
				//	mycode =jisxtounicode1(mycode);
				//else
				//	mycode =jisxtounicode2(mycode);
				str++;
				str++;			
				i+=2;
			}
			else
			{
				mycode=*str;
				i++;
				str++;
			}
			k++;
			len+=UCS2toUTF8Code(mycode, utf8+len);

		}
		return 1;
	}

	//繁体
	if (lid==LID_BIG5)
	{
		int l;
		int big5pos;
		int i=0;
		int k=0;
		unsigned short mycode;

		l=strlen((char*)str);
#if 1
		while (i<l)
		{
			//printf("str:%x\n",str[k]);
			big5pos=tftbig5_pos_first_char(str,(l-i));
			//printf("big5pos:%d\n",big5pos);
			if (big5pos==0)
			{
				mycode=get_unaligned((U16*)str);
				mycode=((mycode&0xFF)<<8)|((mycode&0xFF00)>>8);				
				mycode=SearchvalueByCode(mycode);
				//mycode =Big5ToUnicodeMap(mycode);
#if 0
				if (mycode==0xb4b5)
					mycode=0x65AF;
				if (mycode==0xb0ec)
					mycode=0x57DF;
				if (mycode==0xb373)
					mycode=0x9023;
				if (mycode==0xbd75)
					mycode=0x7DDA;
#endif

				str++;
				str++;			
				i+=2;
			}
			else
			{
				mycode=*str;
				i++;
				str++;
			}
			k++;
			//printf("mygoodecode:%x\n",mycode);
			len+=UCS2toUTF8Code(mycode, utf8+len);

		}
#endif
		return 1;
	}

	while(1)
	{
		unsigned char b=*str++;
		unsigned short ucs2=b;
		//printf("b:%x",b);
		switch (lid)
		{
			case LID_CP1258:
				/*Vietnamese language */
				if(b==0x80) ucs2=0x20AC;
				else if(b==0x82) ucs2=0x201A;
				else if(b==0x83) ucs2=0x0192;
				else if(b==0x84) ucs2=0x201E;
				else if(b==0x85) ucs2=0x2026;
				else if(b==0x86) ucs2=0x2020;
				else if(b==0x87) ucs2=0x2021;
				else if(b==0x88) ucs2=0x02C6;
				else if(b==0x89) ucs2=0x2030;
				else if(b==0x8B) ucs2=0x2039;
				else if(b==0x8C) ucs2=0x0152;
				else if(b==0x91) ucs2=0x2018;
				else if(b==0x92) ucs2=0x2019;
				else if(b==0x93) ucs2=0x201C;
				else if(b==0x94) ucs2=0x201D;
				else if(b==0x95) ucs2=0x2022;
				else if(b==0x96) ucs2=0x2013;
				else if(b==0x97) ucs2=0x2014;
				else if(b==0x98) ucs2=0x02DC;
				else if(b==0x99) ucs2=0x2122;
				else if(b==0x9B) ucs2=0x203A;
				else if(b==0x9C) ucs2=0x0153;
				else if(b==0x9F) ucs2=0x0178;
				else if(b==0xC3) ucs2=0x0102;
				else if(b==0xCC) ucs2=0x0300;
				else if(b==0xD0) ucs2=0x0110;
				else if(b==0xD2) ucs2=0x0309;
				else if(b==0xD5) ucs2=0x01A0;
				else if(b==0xDD) ucs2=0x01AF;
				else if(b==0xDE) ucs2=0x0303;
				else if(b==0xE3) ucs2=0x0103;
				else if(b==0xEC) ucs2=0x0301;
				else if(b==0xF0) ucs2=0x0111;
				else if(b==0xF2) ucs2=0x0323;
				else if(b==0xF5) ucs2=0x01A1;
				else if(b==0xFD) ucs2=0x01B0;
				else if(b==0xFE) ucs2=0x20AB;
				break;
			case LID_CP874://泰语
				{ 		
					if(b==0x80) ucs2=0x20ac;
					else if(b==0x85) ucs2=0x2026;
					else if(b==0x91) ucs2=0x2018;
					else if(b==0x92) ucs2=0x2019;
					else if(b==0x93) ucs2=0x201c;
					else if(b==0x94) ucs2=0x201d;
					else if(b==0x95) ucs2=0x2022;
					else if(b==0x96) ucs2=0x2013;
					else if(b==0x97) ucs2=0x2014;
					else if(b>=0xa1 && b<=0xFB)
					{
						ucs2=0x0E01+b-0xa1;
					}
					break;
				}
			case LID_CP1250://阿尔巴尼亚语,克罗地亚语   add by jazzy 2008.08.07
				{
					if(b==0x80) ucs2=0x20ac;
					else if(b==0x82) ucs2=0x201a;
					else if(b==0x84) ucs2=0x201e;
					else if(b==0x85) ucs2=0x2026;
					else if(b==0x86) ucs2=0x2020;
					else if(b==0x87) ucs2=0x2021;
					else if(b==0x89) ucs2=0x2030;
					else if(b==0x8a) ucs2=0x0160;
					else if(b==0x8b) ucs2=0x2039;
					else if(b==0x8c) ucs2=0x015a;
					else if(b==0x8d) ucs2=0x0164;
					else if(b==0x8e) ucs2=0x017d;
					else if(b==0x8f) ucs2=0x0179;
					else if(b==0x91) ucs2=0x2018;
					else if(b==0x92) ucs2=0x2019;
					else if(b==0x93) ucs2=0x201c;
					else if(b==0x94) ucs2=0x201d;
					else if(b==0x95) ucs2=0x2022;
					else if(b==0x96) ucs2=0x2013;
					else if(b==0x97) ucs2=0x2014;
					else if(b==0x99) ucs2=0x2122;
					else if(b==0x9a) ucs2=0x0161;
					else if(b==0x9b) ucs2=0x203a;
					else if(b==0x9c) ucs2=0x015b;
					else if(b==0x9d) ucs2=0x0165;
					else if(b==0x9e) ucs2=0x017e;
					else if(b==0x9f) ucs2=0x017a;

					else if(b==0xa0) ucs2=0x00a0;
					else if(b==0xa1) ucs2=0x02c7;
					else if(b==0xa2) ucs2=0x02d8;
					else if(b==0xa3) ucs2=0x0141;
					else if(b==0xa4) ucs2=0x00a4;
					else if(b==0xa5) ucs2=0x0104;
					else if(b==0xa6) ucs2=0x00a6;
					else if(b==0xa7) ucs2=0x00a7;
					else if(b==0xa8) ucs2=0x00a8;
					else if(b==0xa9) ucs2=0x00a9;
					else if(b==0xaa) ucs2=0x015e;
					else if(b==0xab) ucs2=0x00ab;
					else if(b==0xac) ucs2=0x00ac;
					else if(b==0xad) ucs2=0x00ad;
					else if(b==0xae) ucs2=0x00ae;
					else if(b==0xaf) ucs2=0x017b;

					else if(b==0xb0) ucs2=0x00b0;
					else if(b==0xb1) ucs2=0x00b1;
					else if(b==0xb2) ucs2=0x02db;
					else if(b==0xb3) ucs2=0x0142;
					else if(b==0xb4) ucs2=0x00b4;
					else if(b==0xb5) ucs2=0x00b5;
					else if(b==0xb6) ucs2=0x00b6;
					else if(b==0xb7) ucs2=0x00b7;
					else if(b==0xb8) ucs2=0x00b8;
					else if(b==0xb9) ucs2=0x0105;
					else if(b==0xba) ucs2=0x015f;
					else if(b==0xbb) ucs2=0x00bb;
					else if(b==0xbc) ucs2=0x013d;
					else if(b==0xbd) ucs2=0x02dd;
					else if(b==0xbe) ucs2=0x013e;
					else if(b==0xbf) ucs2=0x017c;

					else if(b==0xc0) ucs2=0x0154;
					else if(b==0xc1) ucs2=0x00c1;
					else if(b==0xc2) ucs2=0x00c2;
					else if(b==0xc3) ucs2=0x0102;
					else if(b==0xc4) ucs2=0x00c4;
					else if(b==0xc5) ucs2=0x0139;
					else if(b==0xc6) ucs2=0x0106;
					else if(b==0xc7) ucs2=0x00c7;
					else if(b==0xc8) ucs2=0x010c;
					else if(b==0xc9) ucs2=0x00c9;
					else if(b==0xca) ucs2=0x0118;
					else if(b==0xcb) ucs2=0x00cb;
					else if(b==0xcc) ucs2=0x011a;
					else if(b==0xcd) ucs2=0x00cd;
					else if(b==0xce) ucs2=0x00ce;
					else if(b==0xcf) ucs2=0x010e;

					else if(b==0xd0) ucs2=0x0110;
					else if(b==0xd1) ucs2=0x0143;
					else if(b==0xd2) ucs2=0x0147;
					else if(b==0xd3) ucs2=0x00d3;
					else if(b==0xd4) ucs2=0x00d4;
					else if(b==0xd5) ucs2=0x0150;
					else if(b==0xd6) ucs2=0x00d6;
					else if(b==0xd7) ucs2=0x00d7;
					else if(b==0xd8) ucs2=0x0158;
					else if(b==0xd9) ucs2=0x016e;
					else if(b==0xda) ucs2=0x00da;
					else if(b==0xdb) ucs2=0x0170;
					else if(b==0xdc) ucs2=0x00dc;
					else if(b==0xdd) ucs2=0x00dd;
					else if(b==0xde) ucs2=0x0162;
					else if(b==0xdf) ucs2=0x00df;

					else if(b==0xe0) ucs2=0x0155;
					else if(b==0xe1) ucs2=0x00e1;
					else if(b==0xe2) ucs2=0x00e2;
					else if(b==0xe3) ucs2=0x0103;
					else if(b==0xe4) ucs2=0x00e4;
					else if(b==0xe5) ucs2=0x013a;
					else if(b==0xe6) ucs2=0x0107;
					else if(b==0xe7) ucs2=0x00e7;
					else if(b==0xe8) ucs2=0x010d;
					else if(b==0xe9) ucs2=0x00e9;
					else if(b==0xea) ucs2=0x0119;
					else if(b==0xeb) ucs2=0x00eb;
					else if(b==0xec) ucs2=0x011b;
					else if(b==0xed) ucs2=0x00ed;
					else if(b==0xee) ucs2=0x00ee;
					else if(b==0xef) ucs2=0x010f;

					else if(b==0xf0) ucs2=0x0111;
					else if(b==0xf1) ucs2=0x0144;
					else if(b==0xf2) ucs2=0x0148;
					else if(b==0xf3) ucs2=0x00f3;
					else if(b==0xf4) ucs2=0x00f4;
					else if(b==0xf5) ucs2=0x0151;
					else if(b==0xf6) ucs2=0x00f6;
					else if(b==0xf7) ucs2=0x00f7;
					else if(b==0xf8) ucs2=0x0159;
					else if(b==0xf9) ucs2=0x016f;
					else if(b==0xfa) ucs2=0x00fa;
					else if(b==0xfb) ucs2=0x017f;
					else if(b==0xfc) ucs2=0x00fc;
					else if(b==0xfd) ucs2=0x00fd;
					else if(b==0xfe) ucs2=0x0163;
					else if(b==0xff) ucs2=0x02d9;

					break;
				}
			case LID_ISO8859_15:	//法文
				break;
			case LID_CP1251:		//俄语
				{
					if(b==0x80) ucs2=0x0402;
					else if(b==0x81) ucs2=0x0403;
					else if(b==0x82) ucs2=0x201a;
					else if(b==0x83) ucs2=0x0453;
					else if(b==0x84) ucs2=0x201e;
					else if(b==0x85) ucs2=0x2026;
					else if(b==0x86) ucs2=0x2020;
					else if(b==0x87) ucs2=0x2021;
					else if(b==0x88) ucs2=0x20ac;
					else if(b==0x89) ucs2=0x2030;
					else if(b==0x8a) ucs2=0x0409;
					else if(b==0x8b) ucs2=0x2039;
					else if(b==0x8c) ucs2=0x040a;
					else if(b==0x8d) ucs2=0x040c;
					else if(b==0x8e) ucs2=0x040b;
					else if(b==0x8f) ucs2=0x040f;
					else if(b==0x90) ucs2=0x0452;
					else if(b==0x91) ucs2=0x2018;
					else if(b==0x92) ucs2=0x2019;
					else if(b==0x93) ucs2=0x201c;
					else if(b==0x94) ucs2=0x201d;
					else if(b==0x95) ucs2=0x2022;
					else if(b==0x96) ucs2=0x2013;
					else if(b==0x97) ucs2=0x2014;
					else if(b==0x99) ucs2=0x2122;
					else if(b==0x9a) ucs2=0x0459;
					else if(b==0x9b) ucs2=0x203a;
					else if(b==0x9c) ucs2=0x045a;
					else if(b==0x9d) ucs2=0x045c;
					else if(b==0x9e) ucs2=0x045b;
					else if(b==0x9f) ucs2=0x045f;

					else if(b==0xa0) ucs2=0x00a0;
					else if(b==0xa1) ucs2=0x040e;
					else if(b==0xa2) ucs2=0x045e;
					else if(b==0xa3) ucs2=0x0408;
					else if(b==0xa4) ucs2=0x00a4;
					else if(b==0xa5) ucs2=0x0490;
					else if(b==0xa6) ucs2=0x00a6;
					else if(b==0xa7) ucs2=0x00a7;
					else if(b==0xa8) ucs2=0x0401;
					else if(b==0xa9) ucs2=0x00a9;
					else if(b==0xaa) ucs2=0x0404;
					else if(b==0xab) ucs2=0x00ab;
					else if(b==0xac) ucs2=0x00ac;
					else if(b==0xad) ucs2=0x00ad;
					else if(b==0xae) ucs2=0x00ae;
					else if(b==0xaf) ucs2=0x0407;

					else if(b==0xb0) ucs2=0x00b0;
					else if(b==0xb1) ucs2=0x00b1;
					else if(b==0xb2) ucs2=0x0406;
					else if(b==0xb3) ucs2=0x0456;
					else if(b==0xb4) ucs2=0x0491;
					else if(b==0xb5) ucs2=0x00b5;
					else if(b==0xb6) ucs2=0x00b6;
					else if(b==0xb7) ucs2=0x00b7;
					else if(b==0xb8) ucs2=0x0451;
					else if(b==0xb9) ucs2=0x2116;
					else if(b==0xba) ucs2=0x0454;
					else if(b==0xbb) ucs2=0x00bb;
					else if(b==0xbc) ucs2=0x0458;
					else if(b==0xbd) ucs2=0x0405;
					else if(b==0xbe) ucs2=0x0455;
					else if(b==0xbf) ucs2=0x0457;

				//else if(b>=0xc0 && b<=0xFF)
					else if(b >= 0xc0)
					{
						ucs2=0x0410+b-0xc0;
					}

					break;
				}
			case LID_CP1254:		//土耳其语//lzs add 20100223
				{
					if(b==0x80) ucs2=0x20AC;
					else if(b==0x82) ucs2=0x201A;
					else if(b==0x83) ucs2=0x0192;
					else if(b==0x84) ucs2=0x201E;
					else if(b==0x85) ucs2=0x2026;
					else if(b==0x86) ucs2=0x2020;
					else if(b==0x87) ucs2=0x2021;
					else if(b==0x88) ucs2=0x02C6;
					else if(b==0x89) ucs2=0x2030;
					else if(b==0x8a) ucs2=0x0160;
					else if(b==0x8b) ucs2=0x2039;
					else if(b==0x8c) ucs2=0x0152;

					else if(b==0x91) ucs2=0x2018;
					else if(b==0x92) ucs2=0x2019;
					else if(b==0x93) ucs2=0x201C;
					else if(b==0x94) ucs2=0x201D;
					else if(b==0x95) ucs2=0x2022;
					else if(b==0x96) ucs2=0x2013;
					else if(b==0x97) ucs2=0x2014;
					else if(b==0x98) ucs2=0x02DC;				

					else if(b==0x99) ucs2=0x2122;
					else if(b==0x9a) ucs2=0x0161;
					else if(b==0x9b) ucs2=0x203A;
					else if(b==0x9c) ucs2=0x0153;

					else if(b==0x9f) ucs2=0x0178;
					else if(b>=0xa0 && b<=0xCF)
					{
						ucs2=b;
					}
					else if(b==0xd0) ucs2=0x011E;
					else if(b>=0xd1 && b<=0xEF)
					{
						ucs2=b;
					}
					else if(b==0xF0) ucs2=0x011F;
					else if(b>=0xF1 && b<=0xFC)
					{
						ucs2=b;
					}
					else if(b==0xFD) ucs2=0x0131;
					else if(b==0xFE) ucs2=0x015F;
					else if(b==0xFF) ucs2=0x00FF;

					break;
				}

			case LID_CP1255:  //HEBREW		add by jazzy 2008.07.19
				{
					if(b==0x80) ucs2=0x20ac;
					else if(b==0x82) ucs2=0x201a;
					else if(b==0x83) ucs2=0x0192;
					else if(b==0x84) ucs2=0x201e;
					else if(b==0x85) ucs2=0x2026;
					else if(b==0x86) ucs2=0x2020;
					else if(b==0x87) ucs2=0x2021;
					else if(b==0x88) ucs2=0x02c6;
					else if(b==0x89) ucs2=0x2030;
					else if(b==0x8b) ucs2=0x2039;
					else if(b==0x91) ucs2=0x2018;
					else if(b==0x92) ucs2=0x2019;
					else if(b==0x93) ucs2=0x201c;
					else if(b==0x94) ucs2=0x201d;
					else if(b==0x95) ucs2=0x2022;
					else if(b==0x96) ucs2=0x2013;
					else if(b==0x97) ucs2=0x2014;
					else if(b==0x98) ucs2=0x02dc;
					else if(b==0x99) ucs2=0x2122;
					else if(b==0x9b) ucs2=0x203a;
					else if(b==0xfd) ucs2=0x200e;
					else if(b==0xfe) ucs2=0x200f;
					else if(b>=0xa0 && b<=0xbf)
					{
						if (b==0xa4) 
							ucs2=0x20aa;
						else
							ucs2=b;
					}
					else if(b>=0xc0 && b<=0xd3)
					{
						ucs2=0x0500+b-0x10;
					}
					else if(b>=0xd4 && b<=0xd8)
					{
						ucs2=0x0500+b+0x1c;
					}
					else if(b>=0xe0 && b<=0xfa)
					{
						ucs2=0x0500+b-0x10;
					}
					break;

				}
			case LID_CP1256:  //arabic		add by jazzy 2008.07.19
				{
					if(b==0x80) ucs2=0x20ac;
					else if(b==0x81) ucs2=0x067e;
					else if(b==0x82) ucs2=0x201a;
					else if(b==0x83) ucs2=0x0192;
					else if(b==0x84) ucs2=0x201e;
					else if(b==0x85) ucs2=0x2026;
					else if(b==0x86) ucs2=0x2020;
					else if(b==0x87) ucs2=0x2021;
					else if(b==0x88) ucs2=0x02c6;
					else if(b==0x89) ucs2=0x2030;
					else if(b==0x8a) ucs2=0x2679;
					else if(b==0x8b) ucs2=0x2039;
					else if(b==0x8c) ucs2=0x0152;
					else if(b==0x8d) ucs2=0x0686;
					else if(b==0x8e) ucs2=0x0698;
					else if(b==0x8f) ucs2=0x0688;
					else if(b==0x90) ucs2=0x06af;
					else if(b==0x91) ucs2=0x2018;
					else if(b==0x92) ucs2=0x2019;
					else if(b==0x93) ucs2=0x201c;
					else if(b==0x94) ucs2=0x201d;
					else if(b==0x95) ucs2=0x2022;
					else if(b==0x96) ucs2=0x2013;
					else if(b==0x97) ucs2=0x2014;
					else if(b==0x98) ucs2=0x06a9;
					else if(b==0x99) ucs2=0x2122;
					else if(b==0x9a) ucs2=0x0691;
					else if(b==0x9b) ucs2=0x203a;
					else if(b==0x9c) ucs2=0x0153;
					else if(b==0x9d) ucs2=0x200c;
					else if(b==0x9e) ucs2=0x200d;
					else if(b==0x9f) ucs2=0x06ba;
//					else if(b==0xfd) ucs2=0x200e;
//					else if(b==0xfe) ucs2=0x200f;
					else if(b==0xe0) ucs2=0x00e0;
					else if(b==0xe1) ucs2=0x0644;
					else if(b==0xe2) ucs2=0x00e2;
					else if(b==0xe3) ucs2=0x0645;
					else if(b==0xe4) ucs2=0x0646;
					else if(b==0xe5) ucs2=0x0647;
					else if(b==0xe6) ucs2=0x0648;
					else if(b==0xe7) ucs2=0x00e7;
					else if(b==0xe8) ucs2=0x00e8;
					else if(b==0xe9) ucs2=0x00e9;
					else if(b==0xea) ucs2=0x00ea;
					else if(b==0xeb) ucs2=0x00eb;
					else if(b==0xec) ucs2=0x0649;
					else if(b==0xed) ucs2=0x064a;
					else if(b==0xee) ucs2=0x00ee;
					else if(b==0xef) ucs2=0x00ef;

					else if(b==0xf0) ucs2=0x064b;
					else if(b==0xf1) ucs2=0x064c;
					else if(b==0xf2) ucs2=0x064d;
					else if(b==0xf3) ucs2=0x064e;
					else if(b==0xf4) ucs2=0x00f4;
					else if(b==0xf5) ucs2=0x064f;
					else if(b==0xf6) ucs2=0x0650;
					else if(b==0xf7) ucs2=0x00f7;
					else if(b==0xf8) ucs2=0x0651;
					else if(b==0xf9) ucs2=0x00f9;
					else if(b==0xfa) ucs2=0x0652;
					else if(b==0xfb) ucs2=0x00fb;
					else if(b==0xfc) ucs2=0x00fc;
					else if(b==0xfd) ucs2=0x200e;
					else if(b==0xfe) ucs2=0x200f;
					else if(b==0xff) ucs2=0x06d2;
					else if(b==0xbf) ucs2=0x061f;
					else if(b==0xc0) ucs2=0x06c1;
					else if(b==0xd7) ucs2=0x00d7;
					else if(b>=0xa0 && b<=0xbe)
					{
						if (b==0xa1)	ucs2=0x060c;
						else if (b==0xaa)  ucs2=0x06be;
						else if (b==0xba)  ucs2=0x061b;
						else
							ucs2=b;
					}
					else if(b>=0xc1 && b<=0xd6)
					{
						ucs2=0x0560+b;
					}
					else if(b>=0xdc && b<=0xdf)
					{
						ucs2=0x0564+b;
					}
					else if(b>=0xd8 && b<=0xdb)
					{
						ucs2=0x055f+b;
					}
					break;
				}
			default:
				break;
		}
		//printf("-%x ",ucs2);
		len+=UCS2toUTF8Code(ucs2, utf8+len);
		if(b==0) break;
	}
	//printf("\n");
	return len;

}

static unsigned short utf8_conv_to_uc16 (const unsigned char* mchar,int* wordlen)
{
	int c = *((unsigned char *)(mchar++));
	int n, t;

	*wordlen=1;
	if (c & 0x80) {
		n = 1;
		while (c & (0x80 >> n))
			n++;

		c &= (1 << (8-n)) - 1;
		while (--n > 0) {
			t = *((unsigned char *)(mchar++));

			if ((!(t & 0x80)) || (t & 0x40))
				return '^';

			*wordlen+=1;

			c = (c << 6) | (t & 0x3F);
		}
	}

	return (unsigned short) c;
}

//从UTF-8转换到指定代码页
int UTF8toLocal(int lid,const unsigned char *utf8, unsigned char *str)
{
	int strlength=0;

	if (str == NULL)
	{
		utf8 = NULL;
		return 1;
	}

	if ((lid==LID_ENGLISH) || (lid==LID_GB2312) || (lid==LID_CP1256)) //LANGUAGE.X is local language page ID
	{
		memcpy(str, utf8, strlen((char*)utf8));
		return 1;
	}

	if ((lid== LID_BIG5)||(lid==LID_JAPANESE))	//繁体,日文
	{
		while(1)
		{
			unsigned short ucs2,b1=0;
			int len;

			ucs2=utf8_conv_to_uc16(utf8, &len);
			b1=ucs2;
			//printf("ucs2:%x",ucs2);
			b1=SearchCodeByValue(ucs2);
			if (!b1)
				b1=ucs2;
			if (b1>>8)
			{
				*str++=b1>>8;
				strlength++;
			}
			*str++=b1;
			//printf("-%x *str:%x  ",b1,(unsigned short *)str);
			strlength++;
			utf8+=len;
			if (!b1)
				break;
		}
		//printf("\n");
		return strlength;
	}

	//printf("Locale ID:%d ",lid);
	while(1)
	{
		unsigned char b=0;
		unsigned short ucs2=b;
		int len;

		ucs2=utf8_conv_to_uc16(utf8, &len);
		b=ucs2;
		//printf("ucs2:%x",ucs2);
		switch (lid)
		{
			case LID_CP1258:
				/*Vietnamese language */
				if(ucs2==0x20AC) b=0x80;
				else if(ucs2==0x201A) b=0x82;
				else if(ucs2==0x0192) b=0x83;
				else if(ucs2==0x201E) b=0x84;
				else if(ucs2==0x2026) b=0x85;
				else if(ucs2==0x2020) b=0x86;
				else if(ucs2==0x2021) b=0x87;
				else if(ucs2==0x02C6) b=0x88;
				else if(ucs2==0x2030) b=0x89;
				else if(ucs2==0x2039) b=0x8B;
				else if(ucs2==0x0152) b=0x8C;
				else if(ucs2==0x2018) b=0x91;
				else if(ucs2==0x2019) b=0x92;
				else if(ucs2==0x201C) b=0x93;
				else if(ucs2==0x201D) b=0x94;
				else if(ucs2==0x2022) b=0x95;
				else if(ucs2==0x2013) b=0x96;
				else if(ucs2==0x2014) b=0x97;
				else if(ucs2==0x02DC) b=0x98;
				else if(ucs2==0x2122) b=0x99;
				else if(ucs2==0x203A) b=0x9B;
				else if(ucs2==0x0153) b=0x9C;
				else if(ucs2==0x0178) b=0x9F;
				else if(ucs2==0x0102) b=0xC3;
				else if(ucs2==0x0300) b=0xCC;
				else if(ucs2==0x0110) b=0xD0;
				else if(ucs2==0x0309) b=0xD2;
				else if(ucs2==0x01A0) b=0xD5;
				else if(ucs2==0x01AF) b=0xDD;
				else if(ucs2==0x0303) b=0xDE;
				else if(ucs2==0x0103) b=0xE3;
				else if(ucs2==0x0301) b=0xEC;
				else if(ucs2==0x0111) b=0xF0;
				else if(ucs2==0x0323) b=0xF2;
				else if(ucs2==0x01A1) b=0xF5;
				else if(ucs2==0x01B0) b=0xFD;
				else if(ucs2==0x20AB) b=0xFE;
				break;
			case LID_CP874://泰语
				{ 		
					if(ucs2==0x20ac) b=0x80;
					else if(ucs2==0x2026) b=0x85;
					else if(ucs2==0x2018) b=0x91;
					else if(ucs2==0x2019) b=0x92;
					else if(ucs2==0x201c) b=0x93;
					else if(ucs2==0x201d) b=0x94;
					else if(ucs2==0x2022) b=0x95;
					else if(ucs2==0x2013) b=0x96;
					else if(ucs2==0x2014) b=0x97;
					else if(ucs2>=0x0E01 && ucs2<=0x0E5B)
					{
						b=ucs2-0x0E01+0xa1;
					}
					break;
				}
			case LID_CP1250://阿尔巴尼亚语，克罗地亚语
				{
					if(ucs2==0x20ac) b=0x80;
					else if(ucs2==0x201a) b=0x82;
					else if(ucs2==0x201e) b=0x84;
					else if(ucs2==0x2026) b=0x85;
					else if(ucs2==0x2020) b=0x86;
					else if(ucs2==0x2021) b=0x87;
					else if(ucs2==0x2030) b=0x89;
					else if(ucs2==0x0160) b=0x8a;
					else if(ucs2==0x2039) b=0x8b;
					else if(ucs2==0x015a) b=0x8c;
					else if(ucs2==0x0164) b=0x8d;
					else if(ucs2==0x017d) b=0x8e;
					else if(ucs2==0x0179) b=0x8f;
					else if(ucs2==0x2018) b=0x91;
					else if(ucs2==0x2019) b=0x92;
					else if(ucs2==0x201c) b=0x93;
					else if(ucs2==0x201d) b=0x94;
					else if(ucs2==0x2022) b=0x95;
					else if(ucs2==0x2013) b=0x96;
					else if(ucs2==0x2014) b=0x97;
					else if(ucs2==0x2122) b=0x99;
					else if(ucs2==0x0161) b=0x9a;
					else if(ucs2==0x203a) b=0x9b;
					else if(ucs2==0x015b) b=0x9c;
					else if(ucs2==0x0165) b=0x9d;
					else if(ucs2==0x017e) b=0x9e;
					else if(ucs2==0x017a) b=0x9f;

					else if(ucs2==0x00a0) b=0xa0;
					else if(ucs2==0x02c7) b=0xa1;
					else if(ucs2==0x02d8) b=0xa2;
					else if(ucs2==0x0141) b=0xa3;
					else if(ucs2==0x00a4) b=0xa4;
					else if(ucs2==0x0104) b=0xa5;
					else if(ucs2==0x00a6) b=0xa6;
					else if(ucs2==0x00a7) b=0xa7;
					else if(ucs2==0x00a8) b=0xa8;
					else if(ucs2==0x00a9) b=0xa9;
					else if(ucs2==0x015e) b=0xaa;
					else if(ucs2==0x00ab) b=0xab;
					else if(ucs2==0x00ac) b=0xac;
					else if(ucs2==0x00ad) b=0xad;
					else if(ucs2==0x00ae) b=0xae;
					else if(ucs2==0x017b) b=0xaf;

					else if(ucs2==0x00b0) b=0xb0;
					else if(ucs2==0x00b1) b=0xb1;
					else if(ucs2==0x02db) b=0xb2;
					else if(ucs2==0x0142) b=0xb3;
					else if(ucs2==0x00b4) b=0xb4;
					else if(ucs2==0x00b5) b=0xb5;
					else if(ucs2==0x00b6) b=0xb6;
					else if(ucs2==0x00b7) b=0xb7;
					else if(ucs2==0x00b8) b=0xb8;
					else if(ucs2==0x0105) b=0xb9;
					else if(ucs2==0x015f) b=0xba;
					else if(ucs2==0x00bb) b=0xbb;
					else if(ucs2==0x013d) b=0xbc;
					else if(ucs2==0x02dd) b=0xbd;
					else if(ucs2==0x013e) b=0xbe;
					else if(ucs2==0x017c) b=0xbf;

					else if(ucs2==0x0154) b=0xc0;
					else if(ucs2==0x00c1) b=0xc1;
					else if(ucs2==0x00c2) b=0xc2;
					else if(ucs2==0x0102) b=0xc3;
					else if(ucs2==0x00c4) b=0xc4;
					else if(ucs2==0x0139) b=0xc5;
					else if(ucs2==0x0106) b=0xc6;
					else if(ucs2==0x00c7) b=0xc7;
					else if(ucs2==0x010c) b=0xc8;
					else if(ucs2==0x00c9) b=0xc9;
					else if(ucs2==0x0118) b=0xca;
					else if(ucs2==0x00cb) b=0xcb;
					else if(ucs2==0x011a) b=0xcc;
					else if(ucs2==0x00cd) b=0xcd;
					else if(ucs2==0x00ce) b=0xce;
					else if(ucs2==0x010e) b=0xcf;

					else if(ucs2==0x0110) b=0xd0;
					else if(ucs2==0x0143) b=0xd1;
					else if(ucs2==0x0147) b=0xd2;
					else if(ucs2==0x00d3) b=0xd3;
					else if(ucs2==0x00d4) b=0xd4;
					else if(ucs2==0x0150) b=0xd5;
					else if(ucs2==0x00d6) b=0xd6;
					else if(ucs2==0x00d7) b=0xd7;
					else if(ucs2==0x0158) b=0xd8;
					else if(ucs2==0x016e) b=0xd9;
					else if(ucs2==0x00da) b=0xda;
					else if(ucs2==0x0170) b=0xdb;
					else if(ucs2==0x00dc) b=0xdc;
					else if(ucs2==0x00dd) b=0xdd;
					else if(ucs2==0x0162) b=0xde;
					else if(ucs2==0x00df) b=0xdf;

					else if(ucs2==0x0155) b=0xe0;
					else if(ucs2==0x00e1) b=0xe1;
					else if(ucs2==0x00e2) b=0xe2;
					else if(ucs2==0x0103) b=0xe3;
					else if(ucs2==0x00e4) b=0xe4;
					else if(ucs2==0x013a) b=0xe5;
					else if(ucs2==0x0107) b=0xe6;
					else if(ucs2==0x00e7) b=0xe7;
					else if(ucs2==0x010d) b=0xe8;
					else if(ucs2==0x00e9) b=0xe9;
					else if(ucs2==0x0119) b=0xea;
					else if(ucs2==0x00eb) b=0xeb;
					else if(ucs2==0x011b) b=0xec;
					else if(ucs2==0x00ed) b=0xed;
					else if(ucs2==0x00ee) b=0xee;
					else if(ucs2==0x010f) b=0xef;

					else if(ucs2==0x0111) b=0xf0;
					else if(ucs2==0x0144) b=0xf1;
					else if(ucs2==0x0148) b=0xf2;
					else if(ucs2==0x00f3) b=0xf3;
					else if(ucs2==0x00f4) b=0xf4;
					else if(ucs2==0x0151) b=0xf5;
					else if(ucs2==0x00f6) b=0xf6;
					else if(ucs2==0x00f7) b=0xf7;
					else if(ucs2==0x0159) b=0xf8;
					else if(ucs2==0x016f) b=0xf9;
					else if(ucs2==0x00fa) b=0xfa;
					else if(ucs2==0x017f) b=0xfb;
					else if(ucs2==0x00fc) b=0xfc;
					else if(ucs2==0x00fd) b=0xfd;
					else if(ucs2==0x0163) b=0xfe;
					else if(ucs2==0x02d9) b=0xff;

					break;
				}
			case LID_ISO8859_15://法文
				break;
			case LID_CP1251://俄语
				{
					if(ucs2==0x0402) b=0x80;
					else if(ucs2==0x0403) b=0x81;
					else if(ucs2==0x201a) b=0x82;
					else if(ucs2==0x0453) b=0x83;
					else if(ucs2==0x201e) b=0x84;
					else if(ucs2==0x2026) b=0x85;
					else if(ucs2==0x2020) b=0x86;
					else if(ucs2==0x2021) b=0x87;
					else if(ucs2==0x20ac) b=0x88;
					else if(ucs2==0x2030) b=0x89;
					else if(ucs2==0x0409) b=0x8a;
					else if(ucs2==0x2039) b=0x8b;
					else if(ucs2==0x040a) b=0x8c;
					else if(ucs2==0x040c) b=0x8d;
					else if(ucs2==0x040b) b=0x8e;
					else if(ucs2==0x040f) b=0x8f;
					else if(ucs2==0x0452) b=0x90;
					else if(ucs2==0x2018) b=0x91;
					else if(ucs2==0x2019) b=0x92;
					else if(ucs2==0x201c) b=0x93;
					else if(ucs2==0x201d) b=0x94;
					else if(ucs2==0x2022) b=0x95;
					else if(ucs2==0x2013) b=0x96;
					else if(ucs2==0x2014) b=0x97;
					else if(ucs2==0x2122) b=0x99;
					else if(ucs2==0x0459) b=0x9a;
					else if(ucs2==0x203a) b=0x9b;
					else if(ucs2==0x045a) b=0x9c;
					else if(ucs2==0x045c) b=0x9d;
					else if(ucs2==0x045b) b=0x9e;
					else if(ucs2==0x045f) b=0x9f;

					else if(ucs2==0x00a0) b=0xa0;
					else if(ucs2==0x040e) b=0xa1;
					else if(ucs2==0x045e) b=0xa2;
					else if(ucs2==0x0408) b=0xa3;
					else if(ucs2==0x00a4) b=0xa4;
					else if(ucs2==0x0490) b=0xa5;
					else if(ucs2==0x00a6) b=0xa6;
					else if(ucs2==0x00a7) b=0xa7;
					else if(ucs2==0x0401) b=0xa8;
					else if(ucs2==0x00a9) b=0xa9;
					else if(ucs2==0x0404) b=0xaa;
					else if(ucs2==0x00ab) b=0xab;
					else if(ucs2==0x00ac) b=0xac;
					else if(ucs2==0x00ad) b=0xad;
					else if(ucs2==0x00ae) b=0xae;
					else if(ucs2==0x0407) b=0xaf;

					else if(ucs2==0x00b0) b=0xb0;
					else if(ucs2==0x00b1) b=0xb1;
					else if(ucs2==0x0406) b=0xb2;
					else if(ucs2==0x0456) b=0xb3;
					else if(ucs2==0x0491) b=0xb4;
					else if(ucs2==0x00b5) b=0xb5;
					else if(ucs2==0x00b6) b=0xb6;
					else if(ucs2==0x00b7) b=0xb7;
					else if(ucs2==0x0451) b=0xb8;
					else if(ucs2==0x2116) b=0xb9;
					else if(ucs2==0x0454) b=0xba;
					else if(ucs2==0x00bb) b=0xbb;
					else if(ucs2==0x0458) b=0xbc;
					else if(ucs2==0x0405) b=0xbd;
					else if(ucs2==0x0455) b=0xbe;
					else if(ucs2==0x0457) b=0xbf;
					else if ((ucs2>=0x0410)&&(ucs2<=0x044f))
					{
						b=ucs2-0x0410+0xc0;
					}

					break;
				}

			case LID_CP1254:		//土耳其语//lzs add 20100223
				{
					if(ucs2==0x20AC) b=0x80;
					else if(ucs2==0x201A) b=0x82;
					else if(ucs2==0x0192) b=0x83;
					else if(ucs2==0x201E) b=0x84;
					else if(ucs2==0x2026) b=0x85;
					else if(ucs2==0x2020) b=0x86;
					else if(ucs2==0x2021) b=0x87;
					else if(ucs2==0x02C6) b=0x88;
					else if(ucs2==0x2030) b=0x89;
					else if(ucs2==0x0160) b=0x8a;
					else if(ucs2==0x2039) b=0x8b;
					else if(ucs2==0x0152) b=0x8c;

					else if(ucs2==0x2018) b=0x91;
					else if(ucs2==0x2019) b=0x92;
					else if(ucs2==0x201C) b=0x93;
					else if(ucs2==0x201D) b=0x94;
					else if(ucs2==0x2022) b=0x95;
					else if(ucs2==0x2013) b=0x96;
					else if(ucs2==0x2014) b=0x97;
					else if(ucs2==0x02DC) b=0x98;

					else if(ucs2==0x2122) b=0x99;
					else if(ucs2==0x0161) b=0x9a;
					else if(ucs2==0x203A) b=0x9b;
					else if(ucs2==0x0153) b=0x9c;

					else if(ucs2==0x0178) b=0x9f;
					else if(ucs2>=0xa0 && ucs2<=0xCF)
					{
						b=ucs2;
					}
					else if(ucs2==0x011E) b=0xd0;
					else if(ucs2>=0xd1 && ucs2<=0xEF)
					{
						b=ucs2;
					}
					else if(ucs2==0x011F) b=0xF0;
					else if(ucs2>=0xF1 && ucs2<=0xFC)
					{
						b=ucs2;
					}
					else if(ucs2==0x0131) b=0xFD;
					else if(ucs2==0x015F) b=0xFE;
					else if(ucs2==0x00FF) b=0xFF;

					break;
				}

			case LID_CP1255://HEBREW
				{ 		
					if(ucs2==0x20ac) b=0x80;
					else if(ucs2==0x201a) b=0x82;
					else if(ucs2==0x0192) b=0x83;
					else if(ucs2==0x201e) b=0x84;
					else if(ucs2==0x2026) b=0x85;
					else if(ucs2==0x2020) b=0x86;
					else if(ucs2==0x2021) b=0x87;
					else if(ucs2==0x02c6) b=0x88;
					else if(ucs2==0x2030) b=0x89;
					else if(ucs2==0x2039) b=0x8b;
					else if(ucs2==0x2018) b=0x91;
					else if(ucs2==0x2019) b=0x92;
					else if(ucs2==0x201c) b=0x93;
					else if(ucs2==0x201d) b=0x94;
					else if(ucs2==0x2022) b=0x95;
					else if(ucs2==0x2013) b=0x96;
					else if(ucs2==0x2014) b=0x97;
					else if(ucs2==0x02dc) b=0x98;
					else if(ucs2==0x2122) b=0x99;
					else if(ucs2==0x203a) b=0x9b;
					else if(ucs2==0x200e) b=0xfd;
					else if(ucs2==0x200f) b=0xfe;
					else if(ucs2==0x20aa) b=0xa4;
					else if(ucs2>=0xa0 && ucs2<=0xbf)
					{
						b=ucs2;
					}
					else if(ucs2>=0x05b0 && ucs2<=0x05c3)
					{
						b=ucs2-0x0500+0x10;
					}
					//else if(ucs2>=0x05f0 && b<=0x05f4)
					else if(ucs2>=0x05f0 && ucs2<=0x05f4)//modified by zxz 2012-09-13
					{
						b=ucs2-0x0500-0x1c;
					}
					else if(ucs2>=0x05d0 && ucs2<=0x05ea)
					{
						b=ucs2-0x0500+0x10;
					}
					break;
				}
			case LID_CP1256:  //arabic		add by jazzy 2008.07.19
				{
					if(ucs2==0x20ac) b=0x80;
					else if(ucs2==0x067e) b=0x81;
					else if(ucs2==0x201a) b=0x82;
					else if(ucs2==0x0192) b=0x83;
					else if(ucs2==0x201e) b=0x84;
					else if(ucs2==0x2026) b=0x85;
					else if(ucs2==0x2020) b=0x86;
					else if(ucs2==0x2021) b=0x87;
					else if(ucs2==0x02c6) b=0x88;
					else if(ucs2==0x2030) b=0x89;
					else if(ucs2==0x2679) b=0x8a;
					else if(ucs2==0x2039) b=0x8b;
					else if(ucs2==0x0152) b=0x8c;
					else if(ucs2==0x0686) b=0x8d;
					else if(ucs2==0x0698) b=0x8e;
					else if(ucs2==0x0688) b=0x8f;
					else if(ucs2==0x06af) b=0x90;
					else if(ucs2==0x2018) b=0x91;
					else if(ucs2==0x2019) b=0x92;
					else if(ucs2==0x201c) b=0x93;
					else if(ucs2==0x201d) b=0x94;
					else if(ucs2==0x2022) b=0x95;
					else if(ucs2==0x2013) b=0x96;
					else if(ucs2==0x2014) b=0x97;
					else if(ucs2==0x06a9) b=0x98;
					else if(ucs2==0x2122) b=0x99;
					else if(ucs2==0x0691) b=0x9a;
					else if(ucs2==0x203a) b=0x9b;
					else if(ucs2==0x0153) b=0x9c;
					else if(ucs2==0x200c) b=0x9d;
					else if(ucs2==0x200d) b=0x9e;
					else if(ucs2==0x06ba) b=0x9f;
					else if(ucs2==0x200e) b=0xfd;
					else if(ucs2==0x200f) b=0xfe;
					else if (ucs2==0x060c) b=0xa1;
					else if (ucs2==0x06be)  b=0xaa;
					else if (ucs2==0x061b)  b=0xba;
					else if (ucs2==0x00a0)  b=0xa0;
					else if(ucs2==0x00e0) b=0xe0;

					else if(ucs2==0x0637) b=0xd8;
					else if(ucs2==0x0638) b=0xd9;
					else if(ucs2==0x0639) b=0xda;
					else if(ucs2==0x063a) b=0xdb;

					else if(ucs2==0x0640) b=0xdc;
					else if(ucs2==0x0641) b=0xdd;
					else if(ucs2==0x0642) b=0xde;
					else if(ucs2==0x0643) b=0xdf;

					else if(ucs2==0x0644) b=0xe1;
					else if(ucs2==0x00e2) b=0xe2;
					else if(ucs2==0x0645) b=0xe3;
					else if(ucs2==0x0646) b=0xe4;
					else if(ucs2==0x0647) b=0xe5;
					else if(ucs2==0x0648) b=0xe6;
					else if(ucs2==0x00e7) b=0xe7;
					else if(ucs2==0x00e8) b=0xe8;
					else if(ucs2==0x00e9) b=0xe9;
					else if(ucs2==0x00ea) b=0xea;
					else if(ucs2==0x00eb) b=0xeb;
					else if(ucs2==0x0649) b=0xec;
					else if(ucs2==0x064a) b=0xed;
					else if(ucs2==0x00ee) b=0xee;
					else if(ucs2==0x00ef) b=0xef;

					else if(ucs2==0x064b) b=0xf0;
					else if(ucs2==0x064c) b=0xf1;
					else if(ucs2==0x064d) b=0xf2;
					else if(ucs2==0x064e) b=0xf3;
					else if(ucs2==0x00f4) b=0xf4;
					else if(ucs2==0x064f) b=0xf5;
					else if(ucs2==0x0650) b=0xf6;
					else if(ucs2==0x00f7) b=0xf7;
					else if(ucs2==0x0651) b=0xf8;
					else if(ucs2==0x00f9) b=0xf9;
					else if(ucs2==0x0652) b=0xfa;
					else if(ucs2==0x00fb) b=0xfb;
					else if(ucs2==0x00fc) b=0xfc;
					else if(ucs2==0x200e) b=0xfd;
					else if(ucs2==0x200f) b=0xfe;
					else if(ucs2==0x06d2) b=0xff;
					else if(ucs2==0x061f) b=0xbf;
					else if(ucs2==0x06c1) b=0xc0;
					else if(ucs2==0xd7) b=0x00d7;
					else if(ucs2>=0xa2 && ucs2<=0xbe)
					{
						ucs2=b;
					}
					//else if(ucs2>=0x0621 && b<=0x0636)
					else if(ucs2>=0x0621 && ucs2<=0x0636)//modified by zxz 2012-09-13
					{
						b=ucs2-0x0560;
					}
					break;
				}
			default:
				{
					b=*utf8;		len=1;
				}
		}
		*str++=b;
		//printf("-%x ",b);
		strlength++;
		utf8+=len;
		if (!b) 
			break;
	}
	//printf("\n");
	return strlength;
}
