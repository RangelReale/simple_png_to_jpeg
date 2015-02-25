#include "simple_png_to_jpeg.h"

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <assert.h>
#include <jpeglib.h>

int png_to_jpeg(const char *pngfile, const char *jpegfile, int jpegquality)
{
	FILE *fpin = fopen(pngfile, "rb");
	if (!fpin)
	{
		//perror(pngfile);
		return 1;
	}

	unsigned char header[8];
	fread(header, 1, 8, fpin);
	if (png_sig_cmp(header, 0, 8))
	{
		//fprintf(stderr, "this is not a PNG file\n");
		return 2;
	}
	
	int ret = 0;

	png_structp png_ptr = png_create_read_struct
		(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	assert(png_ptr);

	png_infop info_ptr = png_create_info_struct(png_ptr);
	assert(info_ptr);

	png_infop end_info = png_create_info_struct(png_ptr);
	assert (end_info);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		//fprintf(stderr, "failed.\n");
		ret = 3;
		goto error_png;
	}

	png_init_io(png_ptr, fpin);
	png_set_sig_bytes(png_ptr, 8);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);
	png_bytep * row_pointers = png_get_rows(png_ptr, info_ptr);

	png_uint_32 width, height;
	int bit_depth, color_type;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, 
		&bit_depth, &color_type, 0, 0, 0);

	if (color_type != PNG_COLOR_TYPE_RGB_ALPHA)
	{
		//fprintf(stderr, "input PNG must be RGB+Alpha\n");
		ret = 4;
		goto error_png;
	}
	if (bit_depth != 8)
	{
		//fprintf(stderr, "input bit depth must be 8bit!\n");
		ret = 5;
		goto error_png;
	}

	//printf("png is %ldx%ld\n", width, height);
	int channels = png_get_channels(png_ptr, info_ptr);
	if (channels != 4)
	{
		//fprintf(stderr, "channels must be 4.\n");
		ret = 6;
		goto error_png;
	}
	
	/* now write jpeg */
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW jrow_pointer[1];
	FILE *outfp;

	outfp = fopen(jpegfile, "wb");
	if (!outfp)
	{
		//perror(jpegfile);
		ret = 7;
		goto error_png;
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfp);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, jpegquality, 1);
	jpeg_start_compress(&cinfo, 1);

	unsigned char *row = malloc(width * 3);
	while (cinfo.next_scanline < cinfo.image_height)
	{
		int x;
		jrow_pointer[0] = row;
		unsigned char *source = row_pointers[cinfo.next_scanline];
		for (x = 0; x < width; ++x)
		{
			row[x * 3 + 0] = source[0];
			row[x * 3 + 1] = source[1];
			row[x * 3 + 2] = source[2];
			source += 4;
		}
		jpeg_write_scanlines(&cinfo, jrow_pointer, 1);
	}
	
error_jpeg:
	if (row)
		free(row);
	
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	if (outfp)
		fclose(outfp);
error_png:
	if (fpin)
		fclose(fpin);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	return ret;
}
