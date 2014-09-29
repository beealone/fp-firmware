#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "arca.h"
#include "wiegandcode.h"
#include "options.h"
#include "flashdb.h"

// Get the value from wiegand bits, according to the format and key, return the value
int WiegandInOrOut;
static int wgGetValue(char *bits, char *fmt, char key)
{
	long long value = 0;
	int i;
	int bitcount=0;

	for (i=0; i<MAX_WG_BITS; i++)
	{
		if (fmt[i]==0) break;
		if (fmt[i]==key)
		{
			bitcount++;
			value<<=1;
			if (bits[i]=='1')
			{
				value |= 1;
			}
		}
	}

	if (bitcount>32)		// Card number is bigger than U32
	{
		char tmpString[64];
		char valString[40];
		int stpos = 0;

		memset(tmpString, 0, sizeof(tmpString));
		sprintf(tmpString, "%lld", value);
		printf("Got long card number: %s\n", tmpString);

		i = 0;
		stpos = gOptions.StartPosition-1;
		memset(valString, 0, sizeof(valString));

		while(i < gOptions.CardNumberLen)
		{
			valString[i++] = tmpString[stpos++];
		}

		if (valString[0])
		{
			value = atoi(valString);
		}
	}

	return (int)value;
}

// Get the count of 1 from wiegand bits, according to the format and key, return the value
static int wgGetBit1Count(char *bits, char *fmt, char key)
{
	int c=0;
	int i;
	for (i=0; i<MAX_WG_BITS; i++)
	{
		if (fmt[i]==0) break;
		if (fmt[i]==key || fmt[i]=='b')
		{
			if (bits[i]=='1')
			{
				c++;
			}
		}
	}
	return c;
}

//Wiegand 26 with sitecode
static TWiegandFmt WiegandFmt26a = {
	26,
	"pssssssssccccccccccccccccp",
	"eeeeeeeeeeeeeooooooooooooo"
};

//Wiegand 26 without sitecode
static TWiegandFmt WiegandFmt26 = {
	26,
	"pccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeooooooooooooo"
};

//Wiegand 34 with sitecode
static TWiegandFmt WiegandFmt34 = {
	34,
	"pssssssssccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeooooooooooooooooo"
};

//Wiegand 34 without sitecode
static TWiegandFmt WiegandFmt34a = {
	34,
	"pccccccccccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeooooooooooooooooo"
};

static TWiegandFmt WiegandFmt36a = {
	36,
	"pffffffffffffffffccccccccccccccccmmp",
	"ooooooooooooooooooeeeeeeeeeeeeeeeeee"
};

static TWiegandFmt WiegandFmt36 = {
	36,
	"pffffffffffffffffffccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeoooooooooooooooooo"
};

static TWiegandFmt WiegandFmt37a = {
	37,
	"pmmmmsssssssssssscccccccccccccccccccp",
	"oeobeobeobeobeobeobeobeobeobeobeobeoe"
};

static TWiegandFmt WiegandFmt37 = {
	37,
	"pmmmffffffffffssssssccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeooooooooooooooooooo"
};

static TWiegandFmt WiegandFmt50 = {
	50,
	"pssssssssssssssssccccccccccccccccccccccccccccccccp",
	"eeeeeeeeeeeeeeeeeeeeeeeeeooooooooooooooooooooooooo"
};

static int ID_getCardNum(int cardNum, int readCardStyle)
{
	int numHigh = cardNum >> 16;
	int numLow = cardNum&0xFFFF;
	char tmp[11] = {0};

	sprintf(tmp, "%03d%05d\n", numHigh, numLow);
	int result = atoi(tmp);

	switch(readCardStyle)
	{
		case 0:
			break;
		case 1:
			cardNum = result;
			break;
		case 2:
			cardNum = numLow;
			break;
		default:
			printf("Warning: Default Card style is %d\n", readCardStyle);
			printf("Example: the card print is:0010930095  166£¬51119\n;"
					"Card style is 0, read card num is 0010930095\n"
					"Card style is 1, read card num is 16651119\n"
					"Card style is 2, read card num is 51119\n");
			break;
	}
	printf("Got card number:%u\n", cardNum);
	return cardNum;
}

int wgDecode(TWiegandFmt *fmt, char *bits, TWiegandData *data, int index, BOOL checksumflag)
{
	if (checksumflag)
	{
		int oParity = wgGetBit1Count(bits, fmt->parity, 'o');
		int eParity = wgGetBit1Count(bits, fmt->parity, 'e');
		if (oParity%2==0) return ERROR_PARITY_ODD;
		if (eParity%2==1) return ERROR_PARITY_EVEN;
	}

	if (data)
	{
		printf(">1\n");
		data->manufactureCode = wgGetValue(bits, fmt->code, 'm');
		printf(">1.1\n");
		data->facilityCode = wgGetValue(bits, fmt->code, 'f');
		printf(">1.2\n");
		data->siteCode = wgGetValue(bits, fmt->code, 's');
		printf(">1.3\n");
		printf("fmt->code = %s\n", fmt->code);
		data->cardNumber = ID_getCardNum(wgGetValue(bits, fmt->code, 'c'), gOptions.IDCardStyle);
		printf(">1.4\n");
	}
	return ERROR_OK;
}

int wgEncode(TWiegandFmt *fmt, TWiegandData *data, char*bits)
{
	U32 mfg, facility, site, card;
	int i, eParity, oParity, ePos, oPos;

	if (fmt == NULL) {
		return ERROR_WG_FMT;
	}

	for (i=0; i<fmt->count; i++) {
		bits[i]='0';
	}

	ePos=0;
	oPos=0;
	eParity=0;
	oParity=0;

	mfg = data->manufactureCode;
	facility = data->facilityCode;
	site = data->siteCode;
	card = data->cardNumber;

	bits[fmt->count] = 0;
	for (i=fmt->count-1; i>=0; i--)
	{
		char code = fmt->code[i];
		if (code=='m')
		{
			if (mfg&1) bits[i]='1';
			mfg>>=1;
		}
		else if (code=='s')
		{
			if (site&1) bits[i]='1';
			site>>=1;
		}
		else if (code=='c')
		{
			if (card&1) bits[i]='1';
			card>>=1;
		}
		else if (code=='f')
		{
			if (facility&1) bits[i]='1';
			facility>>=1;
		}
		else if (code=='p')
		{
			if (fmt->parity[i]=='o')
				oPos=i;
			else if (fmt->parity[i]=='e')
				ePos=i;
			else
				return ERROR_WG_FMT;
		}
		else if (code==0)
		{
			return ERROR_WG_FMT;
		}

		if (bits[i]=='1')
		{
			code = fmt->parity[i];
			if (code=='e')
				eParity++;
			else if (code=='o')
				oParity++;
			else if (code=='b')
			{
				eParity++;
				oParity++;
			}
		}
	}
	if (eParity%2==1) bits[ePos]='1';
	if (oParity%2==0) bits[oPos]='1';
	return ERROR_OK;
}

char *str2upper(char *str)
{
	char *p=str;
	while(*str !='\0'){
		*str = toupper(*str);
		str++;
	}
	return p;
}

BOOL isNewWGFormat(char *format)
{
	int i;
	int c;

	if (format == NULL) {
		return FALSE;
	}

	c=strlen(format);
	for (i=0; i<c; i++) {
		if (format[i]==':') {
			return TRUE;
		}
	}
	if(strcmp(str2upper(format), "WIEGANDFMT26A")==0)
	{
		return TRUE;
	}

	if(strcmp(str2upper(format), "WIEGAND34")==0)
	{
		return TRUE;
	}

	if(strcmp(str2upper(format), "WIEGAND26")==0)
	{
		return TRUE;
	}

	if(strcmp(str2upper(format), "WIEGANDFMT34A")==0)
	{
		return TRUE;
	}

	if(strcmp(str2upper(format), "WIEGANDUSERFMT")==0)
	{
		return TRUE;
	}

	if (strcmp(format, "36")==0 || strcmp(str2upper(format), "WIEGAND36")==0 ||
			strcmp(format, "36a")==0 || strcmp(str2upper(format), "WIEGAND36A")==0 ||
			strcmp(format, "37")==0 || strcmp(str2upper(format), "WIEGAND37")==0 ||
			strcmp(format, "37a")==0 || strcmp(str2upper(format), "WIEGAND37A")==0 ||
			strcmp(format, "50")==0 || strcmp(str2upper(format), "WIEGAND50")==0) {
		return TRUE;
	}

	return FALSE;
}

TWiegandFmt *wgCreateFmtFromOldOption(char *fmtName, int flag)
{
	TWiegandFmt *fmt = NULL;
	char *p=NULL, *q=NULL;
	int i, p1=-1, p2=-1;
	int oddlen=0, evenlen=0;
	BOOL EvenFirst = FALSE;
	char buffer[256];

	int fmtErrFlag = 0;

	fmt = (TWiegandFmt*)malloc(sizeof(TWiegandFmt));
	if (fmt==NULL) {
		printf("%s, return NULL, 0\n", __FUNCTION__);
		return NULL;
	}
	memset((void*)fmt, 0, sizeof(TWiegandFmt));
	
	if (fmtName != NULL) {
		memset((void*)buffer, 0, sizeof(buffer));
		strncpy(buffer, fmtName, sizeof(buffer)-1);
	}

	if ((fmtName==NULL) || (strcmp(fmtName, "26")==0) || (strcmp(str2upper(fmtName), "WIEGAND26")==0)){
		memcpy((void*)fmt, (void*)&WiegandFmt26, sizeof(TWiegandFmt));
		return fmt;
	} else if ((strcmp(fmtName, "34")==0 )|| (strcmp(str2upper(fmtName), "WIEGAND34")==0)) {
		memcpy((void*)fmt, (void*)&WiegandFmt34, sizeof(TWiegandFmt));
		return fmt;
	}

	fmt->count = strlen(buffer);
	p = fmt->code;
	q = fmt->parity;
	for (i=0; i<fmt->count; i++) {
		char code = buffer[i];
		if (flag==0) {	// Wiegand
			switch(code) {
				case 'P':
					p[i] = 'p';
					if (p1==-1) {
						p1=i;
					} else if (p2==-1) {
						p2=i;
					} else {
						fmtErrFlag = 1;
						//free(fmt);
						//printf("%s, return NULL, 1\n", __FUNCTION__);
						//return NULL;
					}
					break;
				case 'E':
				case 'e':
					p[i] = (code=='e')?'f':'c';
					q[i] = 'e';
					evenlen++;
					if (oddlen==0) EvenFirst=TRUE;
					break;
				case 'O':
				case 'o':
					p[i] = (code=='o')?'f':'c';
					q[i] = 'o';
					oddlen++;
					break;
				case 'D':
				case 'd':
					p[i] = 'd';
					if (code=='D') {
						q[i] = 'e';
						evenlen++;
						if (oddlen==0) {
							EvenFirst=TRUE;
						}
					} else {
						q[i] = 'o';
						oddlen++;
					}
					break;
				default:
					fmtErrFlag = 2;
					//free(fmt);
					//printf("%s, return NULL, 2\n", __FUNCTION__);
					//return NULL;
			}
		} else {			// ExtWiegandIn or IntWiegandIn
			switch(code) {
				case 'P':
					p[i] = 'p';
					if (p1==-1) {
						p1=i;
					} else if (p2==-1) {
						p2=i;
					} else {
						fmtErrFlag = 3;
						//free(fmt);
						//printf("%s, return NULL, 3\n", __FUNCTION__);
						//return NULL;
					}
					break;
				case 'O':
				case 'F':
				case 'I':
					if (code=='F') {
						p[i] = 'f';
					} else if (code=='I') {
						p[i] = 'c';
					} else {
						p[i] = 'd';
					}

					q[i] = 'e';
					evenlen++;
					if (oddlen==0) {
						EvenFirst=TRUE;
					}
					break;
				case 'o':
				case 'f':
				case 'i':
					if (code=='f') {
						p[i] = 'f';
					} else if (code=='i') {
						p[i] = 'c';
					} else {
						p[i] = 'd';
					}

					q[i] = 'o';
					oddlen++;
					break;
				default:
					//free(fmt);
					//printf("%s, return NULL, 4\n", __FUNCTION__);
					//return NULL;
					fmtErrFlag = 4;
			}
		}

		if (fmt->count<3 || (!(p[0]=='p' && p[fmt->count-1]=='p'))) {
			//free(fmt);
			//printf("%s, return NULL, 5\n", __FUNCTION__);
			//return NULL;
			fmtErrFlag = 5;
		}

		if (fmtErrFlag > 0) {
			break;
		}

		if (p1>=0) {	
			q[p1] = (EvenFirst)?'e':'o';
		}

		if (p2>=0) {
			q[p2] = (EvenFirst)?'o':'e';
		}

		return fmt;
	}

	/*create a default wiegand format*/
	if (fmtErrFlag > 0) {
		printf("The format define error in options.cfg, so use a default wingand26 format\n");
		memcpy((void*)fmt, (void*)&WiegandFmt26, sizeof(TWiegandFmt));
	}
	return fmt;
}

TWiegandFmt *wgCreateFmt(char *fmtName)
{
	TWiegandFmt *fmt = NULL;
	int i, pIndex=0;
	char *p=NULL;
	char tmp[256];
	char buffer[256];
	char *userfmt = NULL;

	fmt = (TWiegandFmt*)malloc(sizeof(TWiegandFmt));
	if (fmt == NULL) {
		return NULL;
	}

	memset((void*)fmt, 0, sizeof(TWiegandFmt));
	memset(tmp, 0, sizeof(tmp));

	if (fmtName!=NULL) {
		memset(buffer, 0, sizeof(buffer));
		strncpy(tmp, fmtName, sizeof(tmp)-1);
		strncpy(buffer, tmp, sizeof(buffer)-1);
	}

	if ((fmtName==NULL) || (strcmp(tmp, "26")==0) || (strcmp(str2upper(tmp), "WIEGAND26")==0)){
		memcpy((void*)fmt, (void*)&WiegandFmt26, sizeof(TWiegandFmt));
		return fmt;
	}

	if ((fmtName==NULL) || (strcmp(tmp, "26a")==0) || (strcmp(str2upper(tmp), "WIEGANDFMT26A")==0)){
		memcpy((void*)fmt, (void*)&WiegandFmt26a, sizeof(TWiegandFmt));
		return fmt;
	}
	
	if ((strcmp(tmp, "34")==0) || (strcmp(str2upper(tmp), "WIEGAND34")==0)) {
		memcpy((void*)fmt, (void*)&WiegandFmt34, sizeof(TWiegandFmt));
		return fmt;
	}

	if ((strcmp(tmp, "34a")==0) || (strcmp(str2upper(tmp), "WIEGANDFMT34A")==0)) {
		memcpy((void*)fmt, (void*)&WiegandFmt34a, sizeof(TWiegandFmt));
		return fmt;
	}
	
	if ((strcmp(tmp, "36")==0) || (strcmp(str2upper(tmp), "WIEGAND36")==0)) {
		memcpy((void*)fmt, (void*)&WiegandFmt36, sizeof(TWiegandFmt));
		return fmt;
	}

	if ((strcmp(tmp, "36a")==0) || (strcmp(str2upper(tmp), "WIEGAND36A")==0)) {
		memcpy((void*)fmt, (void*)&WiegandFmt36a, sizeof(TWiegandFmt));
		return fmt;
	}

	if ((strcmp(tmp, "37")==0) ||( strcmp(str2upper(tmp), "WIEGAND37")==0)) {
		memcpy((void*)fmt, (void*)&WiegandFmt37, sizeof(TWiegandFmt));
		return fmt;
	}

	if ((strcmp(tmp, "37a")==0) ||( strcmp(str2upper(tmp), "WIEGAND37A")==0)) {
		memcpy((void*)fmt, (void*)&WiegandFmt37a, sizeof(TWiegandFmt));
		return fmt;
	}

	if ((strcmp(tmp, "50")==0) || (strcmp(str2upper(tmp), "WIEGAND50")==0)) {
		memcpy((void*)fmt, (void*)&WiegandFmt50, sizeof(TWiegandFmt));
		return fmt;
	}
	/*add by zxz 2012-11-29*/
	if(strcmp(str2upper(tmp),"WIEGANDUSERFMT") == 0) {	//user format WG Out
		//printf("_______%s%d,\n", __FILE__, __LINE__);
		if(WiegandInOrOut == WG_IN_MODE) {
			userfmt = LoadStrOld("ExWiegandInUserFmt");
		} else {
			userfmt = LoadStrOld("ExWiegandOutUserFmt");
		}
		if(userfmt != NULL) {
			printf("_______%s%d,userfmt = %s\n", __FILE__, __LINE__, userfmt);
			memset(buffer, 0, sizeof(buffer));
			strncpy(buffer, userfmt, sizeof(buffer)-1);
		} else {
			memset(buffer, 0, sizeof(buffer));
			strncpy(buffer, "pssssssssccccccccccccccccp:eeeeeeeeeeeeeooooooooooooo", sizeof(buffer)-1);
			//printf("_______%s%d,\n", __FILE__, __LINE__);
		}
	}

	/* New wiegand setting format:
	 * pssssssssccccccccccccccccp:eeeeeeeeeeeeeooooooooooooo
	 * */

	p = fmt->code;
	for (i=0; i<MAX_WG_BITS*2+1; i++)
	{
		char code = buffer[i];
		p[pIndex] = code;
		if (code==0) {
			break;
		}

		if (code==':') {
			//code field finished, start parity field
			p[pIndex]=0;
			if (p==fmt->parity) {
				break;
			}
			fmt->count=pIndex;
			p=fmt->parity;
			pIndex=0;
		} else {
			pIndex++;
		}
	}

	printf("Wiegand bits=%d, fmt:%s \n", fmt->count, fmt->code);

	if (p==fmt->code)
	{
		// Not defined parity field, we can give a default.
		fmt->count = pIndex;
		if (pIndex<3 || (!(p[0]=='p' && p[pIndex-1]=='p')))
		{
			free(fmt);
			return NULL;
		}
		for (i=0; i<pIndex; i++)
		{
			if (i<pIndex/2)
				fmt->parity[i]='e';
			else
				fmt->parity[i]='o';
		}
	}
	else
	{
		for (i=0; i<fmt->count; i++)
		{
			if (fmt->code[i]=='p' && !(fmt->parity[i]=='o' || fmt->parity[i]=='e'))
			{
				// The parity bit must be one of even parity or odd parity
				free(fmt);
				return NULL;
			}
		}
	}
	return fmt;
}
