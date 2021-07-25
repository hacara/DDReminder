#pragma once
#include "pch.h"
#include "./libjpeg/jpeglib.h"

typedef struct {
	long filesize;
	char reserved[2];
	long headersize;
	long infoSize;
	long width;
	long depth;
	short biPlanes;
	short bits;
	long biCompression;
	long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	long biClrUsed;
	long biClrImportant;
} BMPHEAD;

void ConvertJpeg2Bitmap(Buffer* img_buf){
    BMPHEAD bh;
    ZeroMemory(&bh,sizeof(bh));
    //deconpress jpeg
    JSAMPROW row_pointer[1];
    
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo,img_buf->buf,img_buf->size);
    jpeg_read_header(&cinfo,TRUE);
    
    bh.width = cinfo.image_width;
    bh.depth = cinfo.image_height;
    int bytes_per_pixel = cinfo.num_components;

    jpeg_start_decompress(&cinfo);
    byte* raw_image = (unsigned char*)malloc(cinfo.output_width*cinfo.output_height*cinfo.num_components);
    row_pointer[0] = (unsigned char *)malloc(cinfo.output_width*cinfo.num_components);
    size_t location = 0;
    while (cinfo.output_scanline < cinfo.image_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for (size_t i = 0; i<cinfo.image_width*cinfo.num_components; i++)
			raw_image[location++] = row_pointer[0][i];
	}
    jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(row_pointer[0]);
    
    //convert to bmp
    Buffer_Flush(img_buf);
    Buffer_Write(img_buf,(void*)"BM",2);

	bh.headersize = 54L;
	bh.infoSize = 0x28L;
	bh.biPlanes = 1;
	bh.bits = 24;
	bh.biCompression = 0L;

	int bytesPerLine = bh.width * 3;
	if (bytesPerLine & 0x0003)
	{
		bytesPerLine |= 0x0003;
		++bytesPerLine;
	}
	bh.filesize = bh.headersize + (long)bytesPerLine*bh.depth;

	Buffer_Write(img_buf,&bh,sizeof(bh));

	char *linebuf = (char *)calloc(1, bytesPerLine);

	for (int line = bh.depth - 1; line >= 0; line--)
	{
		for (int x = 0; x < bh.width; x++)
		{
			*(linebuf + x*bytes_per_pixel) = *(raw_image + (x + line*bh.width)*bytes_per_pixel + 2);
			*(linebuf + x*bytes_per_pixel + 1) = *(raw_image + (x + line*bh.width)*bytes_per_pixel + 1);
			*(linebuf + x*bytes_per_pixel + 2) = *(raw_image + (x + line*bh.width)*bytes_per_pixel + 0);
		}
        Buffer_Write(img_buf,linebuf,bytesPerLine);
	}
	free(linebuf);
    free(raw_image);
}   		
    		