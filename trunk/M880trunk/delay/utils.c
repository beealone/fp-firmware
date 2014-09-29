#include <stdio.h>
#include <string.h>
#include "utils.h"

void doPrint(char * buffer, char * fmt, va_list ap)
{
    void *p1, *p2, *p3, *p4, *p5, *p6;

    p1 = va_arg(ap,void*);
    p2 = va_arg(ap,void*);
    p3 = va_arg(ap,void*);
    p4 = va_arg(ap,void*);

    p5 = va_arg(ap,void*);
    p6 = va_arg(ap,void*);

    sprintf(buffer,fmt,p1,p2,p3,p4,p5,p6);
}

char *nstrcpy(char *Dest, const char *Source, int Size)
{
	char *s;
	s=Dest;
	while((*Source) && Size)
	{
		*s++=*Source++;
		Size--;
	}
	return Dest;
}

int nstrcmp(const char *s1, const char *s2, int size)
{
	while(size)
	{
		if((*s2)<(*s1)) return -1;
		if((*s2)>(*s1)) return 1;
		if((*s1)==0) return 0;
		s1++;s2++;
		size--;
	}
	return 0;
}

/* test for a digit. return value if digit or -1 otherwise */
static int digitvalue(char isdigit)
{
	if (isdigit >= '0' && isdigit <= '9' )
		return isdigit - '0';
	else
		return -1;
}




/* test for a hexidecimal digit. return value if digit or -1 otherwise */
static int xdigitvalue(char isdigit)
{
	if (isdigit >= '0' && isdigit <= '9' )
		return isdigit - '0';
	if (isdigit >= 'a' && isdigit <= 'f')
		return 10 + isdigit - 'a';
	return -1;
}

/* convert a string to an u32 value. if the string starts with 0x, it
 * is a hexidecimal string, otherwise we treat is as decimal. returns
 * the converted value on success, or -1 on failure. no, we don't care
 * about overflows if the string is too long.
 */
int strtou32(const char *str, unsigned int *value)
{
	int i;

	*value = 0;

	if((str[0]=='0') && (str[1]=='x')) {
		/* hexadecimal mode */
		str += 2;
		
		while(*str != '\0') {
			if((i = xdigitvalue(*str)) < 0)
				return -1;
			
			*value = (*value << 4) | (unsigned int)i;

			str++;
		}
	} else {
		/* decimal mode */
		while(*str != '\0') {
			if((i = digitvalue(*str)) < 0)
				return -1;
			
			*value = (*value * 10) + (unsigned int)i;

			str++;
		}
	}

	return 0;
}

int str2mac(char* str, char* mac)
{
	int i=0;
	char *p=str;
	char t[6];
	int addr;
	
	while(i < 6) {
		addr = 0;
		while( *p && (*p != ':') ) {
			if( (*p >= '0') && (*p <= '9') ) {
				addr = addr * 16 + (*p - '0');
				p++;
			}
			else if( (*p >= 'a') && (*p <= 'f') ) {
				addr = addr * 16 + (*p - 'a'+10);
				p++;
			}
			else if ( (*p >= 'A') && (*p <= 'F') ) {
				addr = addr * 16 + (*p - 'A'+10);
			       	p++;
			}
			else return -1; /* invalid */
		}

		if (addr < 255) t[i] = (addr&0xFF);
		else break;

		i++;

		if(*p) p++;
		else break;
	}

	if( i!=6 )  return -1;

	memcpy(mac, t, sizeof(t));

	return 0;
}

int str2ip(char* str, char* ip)
{
	int i=0;
	char *p=str;
	char t[4];
	int addr;
	
	while(i < 4) {
		addr = 0;
		while( *p && (*p != '.') ) {
			if( (*p >= '0') && (*p <= '9') ) {
				addr = addr * 10 + (*p - '0');
			       	p++;
			}
			else return -1; /* invalid */
		}

		if (addr < 255) t[i] = (addr&0xFF);
		else break;

		i++;

		if(*p) p++;
		else break;

	}

	if( i!=4 )  return -1;

	memcpy(ip, t, sizeof(t));

	return 0;
}

char* PadCenterStr(char *buf, int size, const char *s)
{
	int len;
	for(len=0;len<size;len++) buf[len]=' ';buf[size]=0;
	if(NULL==s) return buf;
	len=0;
	while(*s)
		if(' '==*s) s++;
		else break;
	while(s[len]) len++;
	if(len==0) return buf;
	while(' '==s[len-1]) len--;
	if(size<len)
		memcpy(buf, s, size);
	else
		memcpy(buf+(size-len)/2,s,len);
	return buf;
}

char* PadRightStr(char *buf, int size, const char *s)
{
	int len;
	for(len=0;len<size;len++) buf[len]=' ';buf[size]=0;
	if(NULL==s) return buf;
	len=0;
	while(*s)
		if(' '==*s) s++;else break;
	while(s[len]) len++;
	if(len==0) return buf;
	while(' '==s[len-1]) len--;
	if(size<len)
		memcpy(buf, s, size);
	else
		memcpy(buf+size-len,s,len);
	return buf;	
}

int SearchIndex(char **Items, const char *Text, int ItemCount)
{
	int i;
	for(i=0;i<ItemCount;i++)
	{
		if(strcmp(Items[i], Text)==0) return i;
	}
	return -1;
}

char *Pad0Num(char *buf, int size, int value)
{
	char fmt[20];
	sprintf(fmt,"%%0%dd",size);
	sprintf(buf, fmt, value);
	return buf;
}
