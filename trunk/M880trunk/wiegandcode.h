#ifndef __WGCODE__
#define __WGCODE__

#define MAX_WG_BITS 128

#define ERROR_WG_FMT		-10090
#define ERROR_PARITY_EVEN	-10091
#define ERROR_PARITY_ODD	-10092
#define ERROR_OK			0

typedef struct _WiegandFmt_
{
	int count;					// The count of all bits
	char code[MAX_WG_BITS];		// m-Manufacture code, f-Facility code, s-Site code, c-Card number, p-Parity bit
	char parity[MAX_WG_BITS];	// o-Odd parity bit, e-Even parity bit, b-Both parity bit
}TWiegandFmt;

typedef struct _WiegandData_
{
	U32 manufactureCode;
	U32 facilityCode;
	U32 siteCode;
	U32 cardNumber;
}TWiegandData;

BOOL isNewWGFormat(char*format);
TWiegandFmt *wgCreateFmt(char *fmtName);
TWiegandFmt *wgCreateFmtFromOldOption(char *fmtName, int flag);
int wgDecode(TWiegandFmt *fmt, char *bits, TWiegandData *data, int index, BOOL checksumflag);
int wgEncode(TWiegandFmt *fmt, TWiegandData *data, char *bits);

#endif
