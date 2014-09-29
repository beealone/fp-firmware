#ifndef BITMAPHEADER
#define BITMAPHEADER


#include <time.h>
//#include "arca.h"
#include "ccc.h"


/*
#define FAR 
#define DWORD unsigned long
#define WORD unsigned short
#define LONG int
#define BYTE unsigned char

*/
#ifndef PACKED

#if defined(__GNUC__)
    #ifndef PACKED
	#define PACKED
	#endif
	#define GCC_PACKED __attribute__((packed))
	#define GCC_ALIGN0 __attribute__((aligned(1)))
#elif defined(__arm)
	#define PACKED __packed
	#define GCC_PACKED
	#define GCC_ALIGN0
#elif defined(__DCC__)
	#define PACKED  __packed__
	#define GCC_PACKED
	#define GCC_ALIGN0
#else //_MSC_VER
  	#define PACKED
	#define GCC_PACKED
	#define GCC_ALIGN0
#endif

#endif

/* structures for defining DIBs */
typedef PACKED struct tagBITMAPCOREHEADER
{
	DWORD   bcSize;                 /* used to get to color table */
	WORD    bcWidth;
	WORD    bcHeight;
	WORD    bcPlanes;
	WORD    bcBitCount;
} GCC_PACKED BITMAPCOREHEADER,  *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef PACKED struct tagBITMAPINFOHEADER
{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} GCC_PACKED BITMAPINFOHEADER,  *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef PACKED struct tagRGBQUAD
{
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} GCC_PACKED RGBQUAD;
typedef RGBQUAD * LPRGBQUAD;

typedef PACKED struct tagRGBTRIPLE
{
	BYTE    rgbtBlue;
	BYTE    rgbtGreen;
	BYTE    rgbtRed;
} GCC_PACKED RGBTRIPLE;


typedef PACKED struct tagBITMAPINFO
{
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} GCC_PACKED BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

typedef PACKED struct tagBITMAPCOREINFO
{
	BITMAPCOREHEADER    bmciHeader;
	RGBTRIPLE           bmciColors[1];
} GCC_PACKED BITMAPCOREINFO,  *LPBITMAPCOREINFO, *PBITMAPCOREINFO;

typedef PACKED struct tagBITMAPFILEHEADER
{
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
}GCC_PACKED BITMAPFILEHEADER,  *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

#endif
