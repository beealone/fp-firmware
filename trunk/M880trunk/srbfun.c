#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arca.h"
#include "exfun.h"
#include "options.h"
#include "utils.h"

#include "srbfun.h"


static int str2wg(char* buf, U8* wg_value)
{
	int i, len=0;
	if (buf != NULL)
	{
		len = strlen(buf);
		for (i=0; i<len; i++)
		{
			wg_value[i] = (U8)buf[i];
		}
		return 1;
	}
	return 0;
}

static char* byte2binary(unsigned char value)
{
	static char rebuf[10] = {0};
	int i;

	for (i=0; i<8; i++)
	{
		if (((value>>(7-i))&0x01) == 1)
			rebuf[i] = '1';
		else
			rebuf[i] = '0';
		DBPRINTF("%c", rebuf[i]);
	}
	DBPRINTF("\n");
	return rebuf;
}

/* 检查是否为有效的二进制串 */
static int isValidBinaryString(int type, char* bstring)
{
	int i, len=0;

	if (bstring==NULL)
	{
		DBPRINTF("No SRB output string setting\n");
		return 0;
	}

	len = strlen(bstring);
	if (((type==0) && (len != SRB_BINARY_LEN)) || ((type==1) && (len != SRB_BINARY_LEN-8)))
	{
		DBPRINTF("Length of SRB output string setting error\n");
		return 0;
	}

	for (i=0; i<len; i++)
	{
		if ((bstring[i] != '0') && (bstring[i]!= '1'))
		{
			DBPRINTF("Invalid SRB output string setting\n");
			return 0;
		}
	}

	return 1;
}

BOOL WiegandSRBOutput(int srbtype, unsigned char lockdelay)
{
	unsigned char data[256] = {0};
	const char dfdata[] = "11111000001010101010001111100010101011100001010110001100";
	char* setstr = NULL;
	char tmpdata[128];

	memset(tmpdata, 0, sizeof(tmpdata));
	setstr = LoadStrOld("~SecureOutData");

	if (isValidBinaryString(srbtype, setstr))
	{
		// 如果在Options.cfg中设置了二进制串，则使用设定的串
		sprintf(tmpdata, "%s", setstr);
	}
	else
	{
		// 使用指定的二进制串
		if (strlen(dfdata) < SRB_BINARY_LEN)
		{
			DBPRINTF("Length of default output string error, %d, %d\n", strlen(dfdata), SRB_BINARY_LEN);
			return FALSE;
		}

		if (srbtype == NEW_SRB_MODE)
		{
			nstrcpy(tmpdata, dfdata, SRB_BINARY_LEN-8);
		}
		else
		{
			nstrcpy(tmpdata, dfdata, SRB_BINARY_LEN);
		}
	}

	if (srbtype == NEW_SRB_MODE)
	{
		DBPRINTF("Lock delay is: %d ms\n", lockdelay);
		sprintf(tmpdata, "%s%s", tmpdata, byte2binary(lockdelay));
	}

	DBPRINTF("Wiegand send: %s, len: %d\n", tmpdata, strlen(tmpdata));
	if (str2wg(tmpdata, data))
	{
		return WiegandSendData(data);
	}
	else
	{
		DBPRINTF("Invalid Wiegand String Value\n");
	}

	return FALSE;
}
