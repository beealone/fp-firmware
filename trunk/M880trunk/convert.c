#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "convert.h"
#include "arca.h"

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
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
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;
#pragma pack()

void write_bitmap(char *imgout, unsigned char *buffer, int nWidth, int nHeight)
{
	BITMAPFILEHEADER bfh;
	RGBQUAD* m_lpvColorTable;
	BITMAPINFOHEADER *m_lpBMIH;
	
	int m_nColorTableEntries=256;
	int m_nBitCount=8;
	
	FILE *fo;
	int i;

	if((fo = fopen(imgout, "w+b")) == NULL) {
		printf("cannot open %s\n", imgout);
		return;
	}

	m_lpBMIH = (BITMAPINFOHEADER*) MALLOC(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries);
        m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
        m_lpBMIH->biWidth  = nWidth;
        m_lpBMIH->biHeight = nHeight;
        m_lpBMIH->biPlanes = 1;
        m_lpBMIH->biBitCount = m_nBitCount;
        m_lpBMIH->biCompression = 0;
        m_lpBMIH->biSizeImage = 0;
        m_lpBMIH->biXPelsPerMeter = 0;
        m_lpBMIH->biYPelsPerMeter = 0;
        m_lpBMIH->biClrUsed      = m_nColorTableEntries;
        m_lpBMIH->biClrImportant = m_nColorTableEntries;

        m_lpvColorTable = (RGBQUAD*)((unsigned char *)m_lpBMIH + sizeof(BITMAPINFOHEADER)); // points inside m_lpBMIH.
        for (i = 0; i < m_nColorTableEntries; i++)
        {
              BYTE k = m_nColorTableEntries==256?(BYTE)i:(BYTE)i<<4;
              
              m_lpvColorTable[i].rgbRed      = k;  // Gray Scale !
              m_lpvColorTable[i].rgbGreen    = k;
              m_lpvColorTable[i].rgbBlue     = k;
              m_lpvColorTable[i].rgbReserved = 0;
        }

	// Fill in the Bitmap File Header
	memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
	bfh.bfType = ( (WORD) ('M' << 8) | 'B');

	// Calculate the size of the bitmap including the palette
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
			m_lpBMIH->biClrUsed * sizeof(RGBQUAD) +
			((((m_lpBMIH->biWidth * m_lpBMIH->biBitCount + 31)/32) * 4) * m_lpBMIH->biHeight);
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;

	// Offset to actual bits is after palette
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
		+ m_lpBMIH->biClrUsed * sizeof(RGBQUAD);

	// Write the results
	fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fo);
	fwrite(m_lpBMIH, sizeof(BITMAPINFOHEADER)+m_lpBMIH->biClrUsed*sizeof(RGBQUAD), 1, fo);

	fwrite(buffer, bfh.bfSize-sizeof(BITMAPFILEHEADER)-sizeof(BITMAPINFOHEADER)-m_lpBMIH->biClrUsed*sizeof(RGBQUAD), 1, fo);
	
	FREE(m_lpBMIH);
	
	fclose(fo);
}

void write_file(char * filename, unsigned char *buffer, int length)
{
	FILE *fp;

	fp = fopen(filename, "w+b");
	fwrite(buffer, 1, length, fp); 	
	fclose(fp);
}
