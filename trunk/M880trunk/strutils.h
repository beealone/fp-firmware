#ifndef STRUTILS_H__
#define STRUTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "strlist.h"

#ifdef  __cplusplus
extern "C" {
#endif

//extract a string splited with ";" into a array of string
int extractStrs(const char *s, char *sp[], int maxSp);
#define ExtStrs(s, ss, c) \
{\
	char *ps[100];\
	char data[1024*100];\
	int i;\
	for(i=0; i<100; i++) ps[i]=data+i*1024;\
	i=extractStrs(s, ps, c);\
	while(i--) strcpy(ss[i], ps[i]);\
}
int extractValue(char* s, char *key, char sp, char *value, int maxlen);
int extractValueInt(char* s, char *key, char sp, int defValue);

//Tag processing <#IMAGE Ref=Value>
#define MAX_TAG_SIZE 10240
typedef char *(*TagReplacer)(char *tagName, char *tagProperty, void *param);
int replaceTag(char *content, int *contentSize, TagReplacer replacer, void *param);

char *deleteMemo(char *s, int size, int index, int count);
char *insertMemo(char *s, int size, int index, int count);
char *deleteStr(char *s, int index, int count);
char *insertStr(char *s, int index, const char *newStr);
char *insertChar(char *s, int index, int count, char ch);
char *replaceStr(const char *oldStr, const char *newStr, char *content, int *contentSize, int *count);

int replaceTagProp(char *content, int *contentSize, PStrList repTable, PStrList props);

int strToIntN(const char *s, int n);
char str2Hex(char c1, char c2);

int decodeLine(const char *code, const char *target); //target can be equal code, or NULL
int encodeLine(const char *code, const char *source);

char *decodeQueryStr(char *str);
char *strimStr(char *str);

int EncodeObject(char *code, void *object, int size);
int DecodeObject(const char *code, void *object);

#ifdef  __cplusplus
}
#endif

#endif

