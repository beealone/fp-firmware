#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct _tag_RS
{
	unsigned long first;
	unsigned long last;
	time_t lasttime;
	unsigned char *InBuffer;
	unsigned long total;
}RS;
