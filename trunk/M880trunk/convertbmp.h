/************************************************

 ZEM 300 iClock-888

 main.c Main source file

 Copyright (C) 2006-2010, ZKSoftware Inc.
*************************************************/
#ifdef ZEM600
#ifndef _CONVERTBMP_H_
#define _CONVERTBMP_H_
#include <jpeglib.h>

#define BMP_WIDTH       320
#define BMP_HEIGHT      240

typedef struct cjpeg_source_struct * cjpeg_source_ptr;

struct cjpeg_source_struct {
	JMETHOD(void, start_input, (j_compress_ptr cinfo, cjpeg_source_ptr sinfo));
	JMETHOD(JDIMENSION, get_pixel_rows, (j_compress_ptr cinfo, cjpeg_source_ptr sinfo));
	JMETHOD(void, finish_input, (j_compress_ptr cinfo, cjpeg_source_ptr sinfo));
	FILE *input_file;
	JSAMPARRAY buffer;
	unsigned int buffer_height;
};

typedef struct _bmp_source_struct * bmp_source_ptr;

typedef struct _bmp_source_struct {
	struct cjpeg_source_struct pub;
	j_compress_ptr cinfo;
	JSAMPARRAY colormap;
	jvirt_sarray_ptr whole_image;
	unsigned int source_row;
	unsigned int row_width;
	int bits_per_pixel;
} bmp_source_struct;

struct djpeg_dest_struct {
	void (*start_output)(j_decompress_ptr cinfo, struct djpeg_dest_struct*dinfo);
	void (*put_pixel_rows)(j_decompress_ptr cinfo, struct djpeg_dest_struct *dinfo, unsigned int rows_supplied);
	void (*finish_output)(j_decompress_ptr cinfo, struct djpeg_dest_struct *dinfo);
	FILE *output_file;
	JSAMPARRAY buffer;
	unsigned int buffer_height;
};

typedef struct djpeg_dest_struct * djpeg_dest_ptr;

struct cdjpeg_progress_mgr {
  struct jpeg_progress_mgr pub;	
  int completed_extra_passes;	
  int total_extra_passes;
  int percent_done;
};

typedef struct cdjpeg_progress_mgr * cd_progress_ptr;

typedef struct {
	struct djpeg_dest_struct pub;
	int is_os2;
	jvirt_sarray_ptr whole_image;
	unsigned int data_width;
	unsigned int row_width;
	int pad_bytes;
	unsigned int cur_output_row;
}bmp_dest_struct;

typedef bmp_dest_struct *bmp_dest_ptr;

typedef struct tagBITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long bfOffBits;
}__attribute__((packed)) TBITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
	unsigned long biSize;
	unsigned long biWidth;
	unsigned long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long biCompression;
	unsigned long biSizeImage;
	unsigned long biXPelsPerMeter;
	unsigned long biYPelsPerMeter;
	unsigned long biClrUsed;
	unsigned long biClrImportant;
}__attribute__((packed)) TBITMAPINFOHEADER, *PBITMAPINFOHEADER;

#endif /* _CONVERTBMP_H_ */
#endif
