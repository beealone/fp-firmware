#ifndef _TTS_TS_
#define _TTS_TS_

#include <stdlib.h>
#include <stdio.h>

#define HZWIDTH 2

#define MAXTSTEXT 8*HZWIDTH

#define MAXTS 10

#pragma pack(1)
typedef struct _tag_TS
{
	char Name[MAXTSTEXT+1];
	unsigned short Start;
	unsigned short End;
	char Content[MAXTSTEXT+1];
	short Flage;
}
TS;
#pragma pack()

#endif
