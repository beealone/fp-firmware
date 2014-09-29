#ifndef _utils_h
#define _utils_h

#include "stdarg.h"

void doPrint(char * buffer, char * fmt, va_list ap);
char *nstrcpy(char *Dest, const char *Source, int size);
int nstrcmp(const char *s1, const char *s2, int size);
char* PadCenterStr(char *buf, int size, const char *s);
char* PadRightStr(char *buf, int size, const char *s);
char *Pad0Num(char *buf, int size, int value);
int SearchIndex(char **Items, const char *Text, int ItemCount);
int strtou32(const char *str, unsigned int *value);
int str2ip(char* str, char* ip);
int str2mac(char* str, char* mac);

#endif

