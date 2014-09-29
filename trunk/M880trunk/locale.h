/*************************************************
                                           
 ZEM 200                                          
                                                    
 locale.h the header file for Multi-language                               
                                                      
 Copyright (C) 2005-2006, ZKSoftware Inc.      		
                                                      
*************************************************/
#ifndef _LOCALE_H_
#define _LOCALE_H_

#include "arca.h"

//多国语言支持
//
#define LID_INVALID	-1
#define LID_SYMBOL	255
#define LID_UNICODE2	254
#define LID_ROM		0
#define LID_ISO8859_1	1
#define LID_ISO8859_2	2
#define LID_ISO8859_3	3
#define LID_ISO8859_4	4
#define LID_ISO8859_5	5
#define LID_ISO8859_6	6
#define LID_ISO8859_7	7
#define LID_ISO8859_8	8
#define LID_ISO8859_9	9
#define LID_ISO8859_10	10
#define LID_ISO8859_11	11
#define LID_ISO8859_13	13
#define LID_ISO8859_14	14
#define LID_ISO8859_15	15
#define LID_ISO8859_16	16
#define LID_KOI8_R	12
#define LID_THAI	LID_ISO8859_11
#define LID_ENGLISH	LID_ISO8859_1

#define LID_GB2312	21
#define LID_GB18030	22
#define LID_UTF8	23
#define LID_GB23122	24	//只用于字库
#define LID_BIG5	25

#define LID_CP1250	50	//	 Central Europe
#define LID_CP1251	51	//	 Cyrillic		
#define LID_CP1252	52	//	 Latin I		
#define LID_CP1253	53	//	 Greek			
#define LID_CP1254	54	//	 Turkish		
#define LID_CP1255	55	//	 Hebrew		
#define LID_CP1256	56	//	 Arabic
#define LID_CP1257	57	//	 Baltic		
#define LID_CP1258	58	//	 Vietnam	
#define LID_CP874	LID_THAI
#define LID_CP949	59	//	Koren
#define LID_KOR		LID_CP949
#define LID_JOHAB	61	//韩国字母，只用于字库
#define LID_SJIS	60	// Japanese
#define LID_JAPANESE	LID_SJIS

typedef struct _fontlib_{
	unsigned char *buffer; //font data buffer
	int width;  //字符的宽度
	int height;	//字库的高度
	unsigned short firstchar;
	int size;	//最大字符数，firstchar+size-1＝最大字符
	int symbolcount;//点阵字符的总数
	int compressed;
	unsigned short *offset;	//字符位置偏移数据
	unsigned char *bits;	//字符点阵数据
	int codeid;	//编码方法
}TFontLib;

typedef int (* fGetTextWidthFun)(void *LangDriver, char *Text);
typedef int (* fGetTextHeightFun)(void *LangDriver, char *Text);
typedef char* (* fGetTextDotsFun)(void *LangDriver, char *Text, char *Dots, int *DotsSize, int *ByteCount);
typedef char* (* fGetNextTextFun)(void *LangDriver, char *Text, int Width);

typedef struct _LangDriver_{
	int CharHeight;
	int CharWidth;
	int LanguageID;
	int Bidi;	///双向视觉顺序支持, 如显示混合英文和阿拉伯语的语句
	int RightToLeft; //Arabic 和 Hebrew 是从右到左显示的显示支持
	fGetTextWidthFun		GetTextWidthFun;
	fGetTextHeightFun		GetTextHeightFun;
	fGetTextDotsFun			GetTextDotsFun;
	fGetNextTextFun			GetNextTextFun;
	TFontLib	*FontLib;
}TLangDriver, *PLangDriver;

extern TLangDriver *gLangDriver;
extern TLangDriver *gSymbolDriver;
extern TLangDriver *gROMDriver;

//语言
int SetDefaultLanguage(int LocaleID, int RowHeight);
PLangDriver CreateLanguageDriver(int LocaleID, char *FontName, int FontSize);
void FreeLanguageDriver(PLangDriver LangDriver);

PLangDriver CreateLanguage_LT(int LangID, char *FontName, int FontSize);	//各种单字节字符集
PLangDriver CreateLanguage_SYM(int LangID, char *FontName, int FontSize);	//符号
PLangDriver CreateLanguage_ROM(int LangID);									//内置小英文字体
PLangDriver CreateLanguage_BIG5(int LangID, char *FontName, int FontSize);	//BIG5 码的繁体中文
PLangDriver CreateLanguage_SJIS(int LangID, char *FontName, int FontSize);	//Shift JIS 码的日文
PLangDriver CreateLanguage_CP949(int LangID, char *FontName, int FontSize);	//Windows CP949 编码的韩国文
PLangDriver CreateLanguage_CN(int LangID, char *FontName, int FontSize);	//GB2312编码的简体中文
PLangDriver CreateLanguage_UTF8(int LangID, char *FontName, int FontSize);	//UTF8编码的节字符集，目前只支持半宽字符

//取当前系统语言设置
int GetDefaultLanguage(void);

char *GetCharDotsEN(char ch, char *Dots);

//取文本的像素宽度
#define GetTextWidth(Text) gLangDriver->GetTextWidthFun(gLangDriver,Text)

//取文本的像素高度
#define GetTextHeight(Text) gLangDriver->GetTextHeightFun(gLangDriver,Text)

//取文本第一个字符的字符点阵
//Text - 查询文本
//Dots - 点阵存放的缓冲区
//DotsSize - [IN]缓冲区长度 [OUT]实际的点阵字节数
//ByteCount - 第一个文本所占的字节数, 若文本结束，则为0; 若有编码错误，则<0
//Modify - 文本修饰标志
//[RET] 下一个字符的位置
#define GetTextDots(Text, Dots, DotsSize, ByteCount) gLangDriver->GetTextDotsFun(gLangDriver, Text, Dots, DotsSize, ByteCount)

//取超出设定宽度的下一段文本位置
#define GetNextText(Text, Width) gLangDriver->GetNextTextFun(gLangDriver, Text, Width)

//取文本的像素宽度
int GetTextWidth_Default(void *BasedLangDriver, char *Text);
//取文本的像素高度
int GetTextHeight_Default(void *BasedLangDriver, char *Text);
//取超出设定宽度的下一段文本位置
char* GetNextText_Default(void *BasedLangDriver, char *Text, int Width);
char* GetNextText_EUC(void *BasedLangDriver, char *Text, int Width);
PLangDriver CreateLanguage_Default(int LangID);

BYTE *LoadFontLib(char *FontName, TFontLib *FontLib);
unsigned char *FullFontDots16(TFontLib *FontLib, unsigned short code, unsigned char *Dots);
TFontLib *LoadOldFont(void);

unsigned short *StrToUCS2(int LangID, char *str);
unsigned short * GetTextDots_UCS2(unsigned short *Text, char *Dots, int *DotsSize, int *WordCount);

BYTE *LoadFile(char *FileName, int *FileSize);

unsigned short *bidi_l2v(const unsigned short *uscbuf, int orientation);

/*
文件表
====================
GB2312.UNI	GB2312 -> UNICODE 的转换表
GB2312.FT	GB2312 的字库
hz2.dat		旧版本的字库，包括简体和繁体、一些符号等，GB2132的内部编码
UNI2.FT		Unicode 字库，全角字符
LATIN.UNI	Latin字母（半角字符）到Unicode的转换表
ffiso.dat	Latin字母（半角字符）到Unicode的转换表以及Unicode字库
B8.FT		Unicode 字库，半角字符
B8_%X.FT	特定编码表字库，半角字符
BIG5.UNI	BIG5 -> UNICODE 的转换表(Windows Codepage 950)
BIG5.FT		BIG5 的字库(Windows Codepage 950)BYTE *LoadFile(char *FileName, int *FileSize)
SJIS.UNI	Shift JIS -> Unicode 的转换表(Windows Codepage 932)
SJIS.FT		Shift JIS 的字库(Windows Codepage 932)
KOR.UNI		Korean -> Unicode 的转换表(Windows Codepage 949)
KOR.FT		Korean 的字库(Windows Codepage 949)
JOHAB.CD	Korean KSC->Johab 的转换表
JOHAB.FT	Korean Johab字库
SYM.FT		符号字库
TEXT.R		字符串资源
DEF.OPT		出厂默认设置
*/

/*
Delphi 字符集代码
==============================
ANSI_CHARSET = 0;
DEFAULT_CHARSET = 1;
SYMBOL_CHARSET = 2;
SHIFTJIS_CHARSET = $80;
HANGEUL_CHARSET = 129;
GB2312_CHARSET = 134;
CHINESEBIG5_CHARSET = 136;
OEM_CHARSET = 255;
JOHAB_CHARSET = 130;
HEBREW_CHARSET = 177;
ARABIC_CHARSET = 178;
GREEK_CHARSET = 161;
TURKISH_CHARSET = 162;
VIETNAMESE_CHARSET = 163;
THAI_CHARSET = 222;
EASTEUROPE_CHARSET = 238;
RUSSIAN_CHARSET = 204;
MAC_CHARSET = 77;
BALTIC_CHARSET = 186;

Windows Code pages
===================================
SBCS (Single Byte Character Set) Codepages
?	1250 (Central Europe)
?	1251 (Cyrillic)
?	1252 (Latin I)
?	1253 (Greek)
?	1254 (Turkish)
?	1255 (Hebrew)
?	1256 (Arabic)
?	1257 (Baltic)
?	1258 (Vietnam)
?	874 (Thai)
Top of pageTop of page
DBCS (Double Byte Character Set) Codepages

In these graphical representations, leadbytes are indicated by light gray background shading. Each of these leadbytes hyperlinks to a new page showing the 256 character block associated with that leadbyte. Unused leadbytes are identified by a darker gray background.
?	932 (Japanese Shift-JIS)
?	936 (Simplified Chinese GBK)
?	949 (Korean)
?	950 (Traditional Chinese Big5)

*/
/*

* ISO 8859-1 (Latin-1) - 西欧语言
* ISO 8859-2 (Latin-2) - 中欧语言
* ISO 8859-3 (Latin-3) - 南欧语言。世界语也可用此字符集显示。
* ISO 8859-4 (Latin-4) - 北欧语言
* ISO 8859-5 (Cyrillic) - 斯拉夫语言
* ISO 8859-6 (Arabic,178) - 阿拉伯语		
* ISO 8859-7 (Greek) - 希腊语
* ISO 8859-8 (Hebrew, 177) - 希伯来语(视觉顺序)
*		ISO 8859-8-I - 希伯来语(逻辑顺序)

* ISO 8859-9 (Latin-5 或 Turkish) - 它把Latin-1的冰岛语字母换走，加入土耳其语字母。
* ISO 8859-10 (Latin-6 或 Nordic) - 北日耳曼语族，用来代替Latin-4。
* ISO 8859-11 (Thai) - 泰语，从泰国的TIS620标准字集演化而来。
* ISO 8859-13 (Latin-7 或 Baltic Rim) - 波罗的海语族
* ISO 8859-14 (Latin-8 或 Celtic) - 塞尔特语族
* ISO 8859-15 (Latin-9) - 西欧语言，加入Latin-1欠缺的法语及芬兰语重音字母，以及欧元(�)符号。

* Windows CP1250-CP1258

---------Table Charsets aliases---------------------
armscii-8 armscii-8 
Big5 big-5, big-five, big5, bigfive, cn-big5, csbig5 
Big5-HKSCS big5-hkscs, big5_hkscs, big5hk, hkscs 
cp1026 1026, cp-1026, cp1026, ibm1026 
cp1133 1133, cp-1133, cp1133, ibm1133 
cp437 437, cp437, ibm437 
cp500 500, cp500, ibm500 
cp850 850, cp850, cspc850multilingual, ibm850 
cp852 852, cp852, ibm852 
cp855 855, cp855, ibm855 
cp857 857, cp857, ibm857 
cp860 860, cp860, ibm860 
cp861 861, cp861, ibm861 
cp862 862, cp862, ibm862 
cp863 863, cp863, ibm863 
cp864 864, cp864, ibm864 
cp865 865, cp865, ibm865 
cp866 866, cp866, csibm866, ibm866 
cp866u 866u, cp866u 
cp869 869, cp869, csibm869, ibm869 
cp874 874, cp874, cs874, ibm874, windows-874 
cp875 875, cp875, ibm875, windows-875 
cp950 950, cp950, windows-950 
EUC-JP cseucjp, euc-jp, euc_jp, eucjp, x-euc-jp 
EUC-KR cseuckr, euc-kr, euc_kr, euckr 
GB2312 chinese, cn-gb, csgb2312, csiso58gb231280, euc-cn, euc_cn, euccn, gb2312, gb_2312-80, iso-ir-58 
GBK cp936, gbk, windows-936 
geostd8 geo8-gov, geostd8 
IBM037 037, cp037, csibm037, ibm037 
ISIRI3342 isiri-3342, isiri3342 
ISO-2022-JP csiso2022jp, iso-2022-jp 
ISO-8859-1 cp819, csisolatin1, ibm819, iso-8859-1, iso-ir-100, iso8859-1, iso_8859-1, iso_8859-1:1987, l1, latin1 
ISO-8859-10 csisolatin6, iso-8859-10, iso-ir-157, iso8859-10, iso_8859-10, iso_8859-10:1992, l6, latin6 
ISO-8859-11 iso-8859-11, iso8859-11, iso_8859-11, iso_8859-11:1992, tactis, tis-620, tis620 
ISO-8859-13 iso-8859-13, iso-ir-179, iso8859-13, iso_8859-13, l7, latin7 
ISO-8859-14 iso-8859-14, iso-ir-199, iso8859-14, iso_8859-14, iso_8859-14:1998, l8, latin8 
ISO-8859-15 iso-8859-15, iso-ir-203, iso8859-15, iso_8859-15, iso_8859-15:1998 
ISO-8859-16 iso-8859-16, iso-ir-226, iso8859-16, iso_8859-16, iso_8859-16:2000 
ISO-8859-2 csisolatin2, iso-8859-2, iso-ir-101, iso8859-2, iso_8859-2, iso_8859-2:1987, l2, latin2 
ISO-8859-3 csisolatin3, iso-8859-3, iso-ir-109, iso8859-3, iso_8859-3, iso_8859-3:1988, l3, latin3 
ISO-8859-4 csisolatin4, iso-8859-4, iso-ir-110, iso8859-4, iso_8859-4, iso_8859-4:1988, l4, latin4 
ISO-8859-5 csisolatincyrillic, cyrillic, iso-8859-5, iso-ir-144, iso8859-5, iso_8859-5, iso_8859-5:1988 
ISO-8859-6 arabic, asmo-708, csisolatinarabic, ecma-114, iso-8859-6, iso-ir-127, iso8859-6, iso_8859-6, iso_8859-6:1987 
ISO-8859-7 csisolatingreek, ecma-118, elot_928, greek, greek8, iso-8859-7, iso-ir-126, iso8859-7, iso_8859-7, iso_8859-7:1987 
ISO-8859-8 csisolatinhebrew, hebrew, iso-8859-8, iso-ir-138, iso8859-8, iso_8859-8, iso_8859-8:1988 
ISO-8859-9 csisolatin5, iso-8859-9, iso-ir-148, iso8859-9, iso_8859-9, iso_8859-9:1989, l5, latin5 
KOI-7 iso-ir-37, koi-7 
KOI8-R cskoi8r, koi8-r 
KOI8-U koi8-u 
MacArabic macarabic 
MacCE cmac, macce, maccentraleurope, x-mac-ce 
MacCroatian maccroation 
MacCyrillic maccyrillic, x-mac-cyrillic 
MacGreek macgreek 
MacGujarati macgujarati 
MacHebrew machebrew 
MacIceland macisland 
MacRoman csmacintosh, mac, macintosh, macroman 
MacRomania macromania 
MacThai macthai 
MacTurkish macturkish 
Shift_JIS csshiftjis, ms_kanji, s-jis, shift-jis, shift_jis, sjis, x-sjis 
sys-int sys-int 
tscii tscii 
US-ASCII ansi_x3.4-1968, ascii, cp367, csascii, ibm367, iso-ir-6, iso646-us, iso_646.irv:1991, us, us-ascii 
UTF-16BE utf-16, utf-16be, utf16, utf16be 
UTF-16LE utf-16le, utf16le 
UTF-8 utf-8, utf8 
VISCII csviscii, viscii, viscii1.1-1 
windows-1250 cp-1250, cp1250, ms-ee, windows-1250 
windows-1251 cp-1251, cp1251, ms-cyr, win-1251, win1251, windows-1251 
windows-1252 cp-1252, cp1252, ms-ansi, windows-1252 
windows-1253 cp-1253, cp1253, ms-greek, windows-1253 
windows-1254 cp-1254, cp1254, ms-turk, windows-1254 
windows-1255 cp-1255, cp1255, ms-hebr, windows-1255 
windows-1256 cp-1256, cp1256, ms-arab, windows-1256 
windows-1257 cp-1257, cp1257, winbaltrim, windows-1257 
windows-1258 cp-1258, cp1258, windows-1258 

1097 	 IBM 伊朗（波斯语）/波斯语
1098 	IBM 伊朗（波斯语）/波斯语 (PC)

*/
#endif
