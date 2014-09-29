/************************************************

 ZEM 300 iClock-888

 main.c Main source file

 Copyright (C) 2006-2010, ZKSoftware Inc.
*************************************************/
#ifdef ZEM600
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "convertbmp.h"
#include "arca.h"
char *tmp_file = "/tmp/tmpsave.bmp";
char *save_file = "/tmp/photo.jpg";

extern int i353;

void bmpfile_copy(char *dest_file,char *src_file)
{
	int size;
	FILE *srcfp,*destfp;
	char buff[1024];
	
	if ((srcfp=fopen(src_file,"r")) == NULL)
		return;
	if ((destfp=fopen(dest_file,"wb")) == NULL)
	{
		fclose(srcfp);
		return;
	}

	while ((size=fread(buff,sizeof(char),sizeof(buff),srcfp)) > 0)
	{
		if (fwrite(buff,sizeof(char),size,destfp) != size)
			break;
	}

	fclose(srcfp);
	fclose(destfp);
}

int read_byte (bmp_source_ptr sinfo)
{
	register FILE *infile = sinfo->pub.input_file;
	register int c;

	c = getc(infile);
	return c;
}

void read_colormap (bmp_source_ptr sinfo, int cmaplen, int mapentrysize)
{
	int i;
	switch (mapentrysize) {
		case 3:
			for (i = 0; i < cmaplen; i++) {
				sinfo->colormap[2][i] = (JSAMPLE) read_byte(sinfo);
				sinfo->colormap[1][i] = (JSAMPLE) read_byte(sinfo);
				sinfo->colormap[0][i] = (JSAMPLE) read_byte(sinfo);
			}
			break;
		case 4:
			for (i = 0; i < cmaplen; i++) {
				sinfo->colormap[2][i] = (JSAMPLE) read_byte(sinfo);
				sinfo->colormap[1][i] = (JSAMPLE) read_byte(sinfo);
				sinfo->colormap[0][i] = (JSAMPLE) read_byte(sinfo);
				(void) read_byte(sinfo);
			}
			break;
	}
}

unsigned int get_8bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{ 
	bmp_source_ptr source = (bmp_source_ptr) sinfo;
	register JSAMPARRAY colormap = source->colormap;
	JSAMPARRAY image_ptr;
	register int t;
	register JSAMPROW inptr, outptr;
	register unsigned int col; 

	source->source_row--;
	image_ptr = (*cinfo->mem->access_virt_sarray)
		((j_common_ptr) cinfo, source->whole_image,
		 source->source_row, (unsigned int) 1, FALSE);
	inptr = image_ptr[0];
	outptr = source->pub.buffer[0];
	for (col = cinfo->image_width; col > 0; col--) {
		t = GETJSAMPLE(*inptr++);
		*outptr++ = colormap[0][t];
		*outptr++ = colormap[1][t];
		*outptr++ = colormap[2][t];
	}

	return 1;
}

unsigned int get_24bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
	bmp_source_ptr source = (bmp_source_ptr) sinfo;
	JSAMPARRAY image_ptr;
	register JSAMPROW inptr, outptr;
	register unsigned int col;

	source->source_row--;
	image_ptr = (*cinfo->mem->access_virt_sarray)
		((j_common_ptr) cinfo, source->whole_image,
		 source->source_row, (unsigned int) 1, FALSE);
	inptr = image_ptr[0];
	outptr = source->pub.buffer[0];
	for (col = cinfo->image_width; col > 0; col--) {
		outptr[2] = *inptr++;  
		outptr[1] = *inptr++;
		outptr[0] = *inptr++;
		outptr += 3;
	}

	return 1;
}

unsigned int preload_image (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
	bmp_source_ptr source = (bmp_source_ptr) sinfo;
	register FILE *infile = source->pub.input_file;
	register int c;
	register JSAMPROW out_ptr;
	JSAMPARRAY image_ptr;
	unsigned int row, col;
	cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

	for (row = 0; row < cinfo->image_height; row++) {
		if (progress != NULL) {
			progress->pub.pass_counter = (long) row;
			progress->pub.pass_limit = (long) cinfo->image_height;
			(*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
		}
		image_ptr = (*cinfo->mem->access_virt_sarray)
			((j_common_ptr) cinfo, source->whole_image,
			 row, (unsigned int) 1, TRUE);
		out_ptr = image_ptr[0];
		for (col = source->row_width; col > 0; col--) {
			c = getc(infile);
			*out_ptr++ = (JSAMPLE) c;
		}
	}
	if (progress != NULL)
		progress->completed_extra_passes++;

	switch (source->bits_per_pixel) {
		case 8:
			source->pub.get_pixel_rows = get_8bit_row;
			break;
		case 24:
			source->pub.get_pixel_rows = get_24bit_row;
			break;
	}
	source->source_row = cinfo->image_height;
	return (*source->pub.get_pixel_rows) (cinfo, sinfo);
}

void start_input_bmp (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{ 
	bmp_source_ptr source = (bmp_source_ptr) sinfo;
	unsigned char bmpfileheader[14];
	unsigned char bmpinfoheader[64];
#define GET_2B(array,offset)  ((unsigned int) (int)(array[offset]) + \
		(((unsigned int) (int)(array[offset+1])) << 8))
#define GET_4B(array,offset)  ((INT32) (int)(array[offset]) + \
		(((INT32) (int)(array[offset+1])) << 8) + \
		(((INT32) (int)(array[offset+2])) << 16) + \
		(((INT32) (int)(array[offset+3])) << 24))
	INT32 bfOffBits;
	INT32 headerSize;
	INT32 biWidth = 0;   
	INT32 biHeight = 0;
	unsigned int biPlanes;
	INT32 biCompression;
	INT32 biXPelsPerMeter,biYPelsPerMeter;
	INT32 biClrUsed = 0;
	int mapentrysize = 0; 
	INT32 bPad;
	unsigned int row_width;

	fread(bmpfileheader, 1, 14, source->pub.input_file);
	//GET_2B(bmpfileheader,0);
	bfOffBits = (INT32) GET_4B(bmpfileheader,10);
	fread(bmpinfoheader, 1, 4, source->pub.input_file);
	headerSize = (INT32) GET_4B(bmpinfoheader,0);
	fread(bmpinfoheader+4, 1, headerSize-4, source->pub.input_file);
	switch ((int) headerSize) {
		case 12:
			biWidth = (INT32) GET_2B(bmpinfoheader,4);
			biHeight = (INT32) GET_2B(bmpinfoheader,6);
			biPlanes = GET_2B(bmpinfoheader,8);
			source->bits_per_pixel = (int) GET_2B(bmpinfoheader,10);
			switch (source->bits_per_pixel) {
				case 8:                 
					mapentrysize = 3;
					break;
				default:
					break;
			}
			break;
		case 40:
		case 64:
			biWidth = GET_4B(bmpinfoheader,4);
			biHeight = GET_4B(bmpinfoheader,8);
			biPlanes = GET_2B(bmpinfoheader,12);
			source->bits_per_pixel = (int) GET_2B(bmpinfoheader,14);
			biCompression = GET_4B(bmpinfoheader,16);
			biXPelsPerMeter = GET_4B(bmpinfoheader,24);
			biYPelsPerMeter = GET_4B(bmpinfoheader,28);
			biClrUsed = GET_4B(bmpinfoheader,32);

			switch (source->bits_per_pixel) {
				case 8:
					mapentrysize = 4;
				default:
					break;
			}
			if (biXPelsPerMeter > 0 && biYPelsPerMeter > 0) {
				cinfo->X_density = (UINT16) (biXPelsPerMeter/100);
				cinfo->Y_density = (UINT16) (biYPelsPerMeter/100);
				cinfo->density_unit = 2;
			}
			break;
		default:
			break;
	}

	bPad = bfOffBits - (headerSize + 14);
	if (mapentrysize > 0) {
		if (biClrUsed <= 0)
			biClrUsed = 256;
		source->colormap = (*cinfo->mem->alloc_sarray)
			((j_common_ptr) cinfo, JPOOL_IMAGE,
			 (unsigned int) biClrUsed, (unsigned int) 3);
		read_colormap(source, (int) biClrUsed, mapentrysize);
		bPad -= biClrUsed * mapentrysize;
	}
	while (--bPad >= 0) {
		(void) read_byte(source);
	}

	if (source->bits_per_pixel == 24)
		row_width = (unsigned int) (biWidth * 3);
	else
		row_width = (unsigned int) biWidth;
	while ((row_width & 3) != 0) row_width++;
	source->row_width = row_width;
	source->whole_image = (*cinfo->mem->request_virt_sarray)
		((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
		 row_width, (unsigned int) biHeight, (unsigned int) 1);
	source->pub.get_pixel_rows = preload_image;
	if (cinfo->progress != NULL) {
		cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
		progress->total_extra_passes++;
	}

	source->pub.buffer = (*cinfo->mem->alloc_sarray)
		((j_common_ptr) cinfo, JPOOL_IMAGE,
		 (unsigned int) (biWidth * 3), (unsigned int) 1);
	source->pub.buffer_height = 1;
	cinfo->in_color_space = JCS_RGB;
	cinfo->input_components = 3;
	cinfo->data_precision = 8;
	cinfo->image_width = (unsigned int) biWidth;
	cinfo->image_height = (unsigned int) biHeight;
}

void finish_input_bmp (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
}

cjpeg_source_ptr jinit_read_bmp (j_compress_ptr cinfo)
{
	bmp_source_ptr source;

	source = (bmp_source_ptr)
		(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE, sizeof(bmp_source_struct));
	source->cinfo = cinfo;
	source->pub.start_input = start_input_bmp;
	source->pub.finish_input = finish_input_bmp;
	return (cjpeg_source_ptr) source;
}

void start_output_bmp(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
}

void write_colormap (j_decompress_ptr cinfo, bmp_dest_ptr dest,
		int map_colors, int map_entry_size)
{
	JSAMPARRAY colormap = cinfo->colormap;
	int num_colors = cinfo->actual_number_of_colors;
	FILE * outfile = dest->pub.output_file;
	int i;

	if (colormap != NULL) {
		if (cinfo->out_color_components == 3) {
			for (i = 0; i < num_colors; i++) {
				putc(GETJSAMPLE(colormap[2][i]), outfile);
				putc(GETJSAMPLE(colormap[1][i]), outfile);
				putc(GETJSAMPLE(colormap[0][i]), outfile);
				if (map_entry_size == 4)
					putc(0, outfile);
			}
		} else {
			for (i = 0; i < num_colors; i++) {
				putc(GETJSAMPLE(colormap[0][i]), outfile);
				putc(GETJSAMPLE(colormap[0][i]), outfile);
				putc(GETJSAMPLE(colormap[0][i]), outfile);
				if (map_entry_size == 4)
					putc(0, outfile);
			}
		}
	} else {
		for (i = 0; i < 256; i++) {
			putc(i, outfile);
			putc(i, outfile);
			putc(i, outfile);
			if (map_entry_size == 4)
				putc(0, outfile);
		}
	}
	for (; i < map_colors; i++) {
		putc(0, outfile);
		putc(0, outfile);
		putc(0, outfile);
		if (map_entry_size == 4)
			putc(0, outfile);
	}
}

void write_bmp_header (j_decompress_ptr cinfo, bmp_dest_ptr dest)
{
	char bmpfileheader[14];
	char bmpinfoheader[40];
#define PUT_2B(array,offset,value)  \
	(array[offset] = (char) ((value) & 0xFF), \
	 array[offset+1] = (char) (((value) >> 8) & 0xFF))
#define PUT_4B(array,offset,value)  \
	(array[offset] = (char) ((value) & 0xFF), \
	 array[offset+1] = (char) (((value) >> 8) & 0xFF), \
	 array[offset+2] = (char) (((value) >> 16) & 0xFF), \
	 array[offset+3] = (char) (((value) >> 24) & 0xFF))
	INT32 headersize, bfSize;
	int bits_per_pixel, cmap_entries;

	if (cinfo->out_color_space == JCS_RGB) {
		if (cinfo->quantize_colors) {
			bits_per_pixel = 8;
			cmap_entries = 256;
		} else {
			bits_per_pixel = 24;
			cmap_entries = 0;
		}
	} else {
		bits_per_pixel = 8;
		cmap_entries = 256;
	}
	headersize = 14 + 40 + cmap_entries * 4; 
	bfSize = headersize + (INT32) dest->row_width * (INT32) cinfo->output_height;

	bzero(bmpfileheader, sizeof(bmpfileheader));
	bzero(bmpinfoheader, sizeof(bmpinfoheader));

	bmpfileheader[0] = 0x42;     
	bmpfileheader[1] = 0x4D;
	PUT_4B(bmpfileheader, 2, bfSize); 
	PUT_4B(bmpfileheader, 10, headersize); 
	PUT_2B(bmpinfoheader, 0, 40); 
	PUT_4B(bmpinfoheader, 4, cinfo->output_width);
	PUT_4B(bmpinfoheader, 8, cinfo->output_height); 
	PUT_2B(bmpinfoheader, 12, 1); 
	PUT_2B(bmpinfoheader, 14, bits_per_pixel); 
	if (cinfo->density_unit == 2) { 
		PUT_4B(bmpinfoheader, 24, (INT32) (cinfo->X_density*100)); 
		PUT_4B(bmpinfoheader, 28, (INT32) (cinfo->Y_density*100));
	}
	PUT_2B(bmpinfoheader, 32, cmap_entries); 

	fwrite(bmpfileheader, 1, 14, dest->pub.output_file);
	fwrite(bmpinfoheader, 1, 40, dest->pub.output_file);

	if (cmap_entries > 0)
		write_colormap(cinfo, dest, cmap_entries, 4);
}

void finish_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
	bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
	register FILE * outfile = dest->pub.output_file;
	JSAMPARRAY image_ptr;
	register JSAMPROW data_ptr;
	unsigned int row;
	register unsigned int col;
	cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

	write_bmp_header(cinfo, dest);

	for (row = cinfo->output_height; row > 0; row--) {
		if (progress != NULL) {
			progress->pub.pass_counter = (long) (cinfo->output_height - row);
			progress->pub.pass_limit = (long) cinfo->output_height;
			(*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
		}
		image_ptr = (*cinfo->mem->access_virt_sarray)
			((j_common_ptr) cinfo, dest->whole_image, row-1, (unsigned int) 1, FALSE);
		data_ptr = image_ptr[0];
		for (col = dest->row_width; col > 0; col--) {
			putc(GETJSAMPLE(*data_ptr), outfile);
			data_ptr++;
		}
	}
	if (progress != NULL)
		progress->completed_extra_passes++;

	fflush(outfile);
}

void put_gray_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		unsigned int rows_supplied)
{
	bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
	JSAMPARRAY image_ptr;
	register JSAMPROW inptr, outptr;
	register unsigned int col;
	int pad;

	/* Access next row in virtual array */
	image_ptr = (*cinfo->mem->access_virt_sarray)
		((j_common_ptr) cinfo, dest->whole_image,
		 dest->cur_output_row, (unsigned int) 1, TRUE);
	dest->cur_output_row++;

	/* Transfer data. */
	inptr = dest->pub.buffer[0];
	outptr = image_ptr[0];
	for (col = cinfo->output_width; col > 0; col--) {
		*outptr++ = *inptr++;       /* can omit GETJSAMPLE() safely */
	}

	/* Zero out the pad bytes. */
	pad = dest->pad_bytes;
	while (--pad >= 0)
		*outptr++ = 0;
}

void put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		unsigned int rows_supplied)
{ 
	bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
	JSAMPARRAY image_ptr;
	register JSAMPROW inptr, outptr;
	register unsigned int col;
	int pad;

	image_ptr = (*cinfo->mem->access_virt_sarray)
		((j_common_ptr) cinfo, dest->whole_image,
		 dest->cur_output_row, (unsigned int) 1, TRUE);
	dest->cur_output_row++;

	inptr = dest->pub.buffer[0];
	outptr = image_ptr[0];
	for (col = cinfo->output_width; col > 0; col--) {
		outptr[2] = *inptr++;  
		outptr[1] = *inptr++;
		outptr[0] = *inptr++;
		outptr += 3;
	}

	pad = dest->pad_bytes;
	while (--pad >= 0)
		*outptr++ = 0;
}

struct djpeg_dest_struct *jinit_write_bmp(j_decompress_ptr cinfo, int is_os2)
{
	bmp_dest_ptr dest;
	unsigned int row_width;

	dest = (bmp_dest_ptr)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_IMAGE, sizeof(bmp_dest_struct));

	dest->pub.start_output = start_output_bmp;
	dest->pub.finish_output = finish_output_bmp;
	dest->is_os2 = is_os2;

	if (cinfo->out_color_space == JCS_GRAYSCALE) {
		dest->pub.put_pixel_rows = put_gray_rows;
	} else if (cinfo->out_color_space == JCS_RGB) {
		if (cinfo->quantize_colors)
			dest->pub.put_pixel_rows = put_gray_rows;
		else
			dest->pub.put_pixel_rows = put_pixel_rows;
	} else {
		printf("error-%s\n", __FUNCTION__);	
	}

	jpeg_calc_output_dimensions(cinfo);
	row_width = cinfo->output_width * cinfo->output_components;
	dest->data_width = row_width;
	while ((row_width & 3) != 0) row_width++;
	dest->row_width = row_width;
	dest->pad_bytes = (int) (row_width - dest->data_width);

	dest->whole_image = (*cinfo->mem->request_virt_sarray)
		((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
		 row_width, cinfo->output_height, (unsigned int) 1);
	dest->cur_output_row = 0;
	if (cinfo->progress != NULL) {
		cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
		progress->total_extra_passes++;
	}

	dest->pub.buffer = (*cinfo->mem->alloc_sarray)
		((j_common_ptr) cinfo, JPOOL_IMAGE, row_width, (unsigned int) 1);
	dest->pub.buffer_height = 1;
	return (djpeg_dest_ptr) dest;
}

void jpg_convert_bmp(char *src_file, char *dest_file)
{
	FILE *input_file, *output_file;
	struct jpeg_decompress_struct cinfo;	
	struct jpeg_error_mgr jerr;
	struct djpeg_dest_struct *dest_mgr = NULL;
	unsigned int num_scanlines;

	if (src_file == NULL || dest_file == NULL)
		return;
#if 0
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
#endif
	if ((input_file = fopen(src_file, "r")) == NULL) {
		perror("fopen input file");
		//exit(1);
		return;
	}

	if ((output_file = fopen(dest_file, "w")) == NULL) {
		perror("fopen output file");
		//exit(1);
		fclose(input_file);
		return;
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);


	jpeg_stdio_src(&cinfo, input_file);
	jpeg_read_header(&cinfo, 1);

	dest_mgr = jinit_write_bmp(&cinfo, 0);
	dest_mgr->output_file = output_file;
	jpeg_start_decompress(&cinfo);
	(*dest_mgr->start_output)(&cinfo, dest_mgr);
	
	while (cinfo.output_scanline < cinfo.output_height) {
		num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer, dest_mgr->buffer_height);
		(*dest_mgr->put_pixel_rows)(&cinfo, dest_mgr, num_scanlines);
	}

	(*dest_mgr->finish_output)(&cinfo, dest_mgr);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(input_file);
	fclose(output_file);
}

void bmp_convert_jpg(char *src_file, char *dest_file)
{
	FILE *input_file, *output_file;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cjpeg_source_ptr src_mgr;
	unsigned int num_scanlines;

	if (src_file == NULL || dest_file == NULL)
		return;
#if 0
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
#endif
	if ((input_file = fopen(src_file, "r")) == NULL) {
		perror("fopen input file");
		//exit(1);
		return;
	}

	if ((output_file = fopen(dest_file, "w")) == NULL) {
		perror("fopen output file");
		fclose(input_file);
		//exit(1);
		return;
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);

	src_mgr = jinit_read_bmp(&cinfo);
	src_mgr->input_file = input_file;
	(*src_mgr->start_input) (&cinfo, src_mgr);
	jpeg_default_colorspace(&cinfo);
	jpeg_stdio_dest(&cinfo, output_file);
	jpeg_start_compress(&cinfo, 1);
	while (cinfo.next_scanline < cinfo.image_height) {
		num_scanlines = (*src_mgr->get_pixel_rows) (&cinfo, src_mgr);
		(void) jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
	}

	(*src_mgr->finish_input) (&cinfo, src_mgr);
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(input_file);
	fclose(output_file);
}

int rotate_bmp(char *bmp_file)
{
	PBITMAPINFOHEADER head;
	int x, y;
	int lineByte;
	FILE *fp;
	char bmpBuf[BMP_HEIGHT][BMP_WIDTH*3];
	char bmpNewBuf[BMP_WIDTH][BMP_HEIGHT*3];

	if ((fp = fopen(bmp_file,"r+")) == NULL)
		return 0;

	fseek(fp, sizeof(TBITMAPFILEHEADER), 0);
	fread(bmpBuf, sizeof(TBITMAPINFOHEADER), 1, fp);
	head = (PBITMAPINFOHEADER)bmpBuf;
	head->biWidth = BMP_HEIGHT;
	head->biHeight = BMP_WIDTH;
	fseek(fp, sizeof(TBITMAPFILEHEADER), 0);
	fwrite(bmpBuf, sizeof(TBITMAPINFOHEADER),1, fp);

	lineByte=(BMP_WIDTH*3+3)/4*4;
	memset(bmpBuf, 0, sizeof(bmpBuf));
	fseek(fp, sizeof(TBITMAPFILEHEADER)+sizeof(TBITMAPINFOHEADER), 0);
	fread(bmpBuf, lineByte * BMP_HEIGHT, 1, fp);
	for (y = 0; y < BMP_HEIGHT; y++)
	{
		for (x = 0; x < BMP_WIDTH*3; x=x+3)
		{
			if(i353 != 2){
					bmpNewBuf[BMP_WIDTH-x/3-1][BMP_HEIGHT*3-y*3-3] = bmpBuf[y][x];
					bmpNewBuf[BMP_WIDTH-x/3-1][BMP_HEIGHT*3-y*3-2] = bmpBuf[y][x+1];
					bmpNewBuf[BMP_WIDTH-x/3-1][BMP_HEIGHT*3-y*3-1] = bmpBuf[y][x+2];
			}else{
				bmpNewBuf[x/3][y*3] = bmpBuf[y][x];
				bmpNewBuf[x/3][y*3+1] = bmpBuf[y][x+1];
				bmpNewBuf[x/3][y*3+2] = bmpBuf[y][x+2];
			}
		}
	}
	fseek(fp, sizeof(TBITMAPFILEHEADER)+sizeof(TBITMAPINFOHEADER), 0);
	fwrite(bmpNewBuf, lineByte * BMP_HEIGHT, 1, fp);

	fclose(fp);
	return 1;
}

void rotate_photo(char *input_file)
{
	unlink(tmp_file);
	unlink(save_file);
	jpg_convert_bmp(input_file, tmp_file);
	rotate_bmp(tmp_file);
	bmp_convert_jpg(tmp_file, save_file);
	bmpfile_copy(input_file, save_file);
}
#endif
