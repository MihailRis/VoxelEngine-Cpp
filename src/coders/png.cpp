#include "png.h"

#include <iostream>
#include <memory>
#include <GL/glew.h>

#include "../graphics/ImageData.h"
#include "../graphics/Texture.h"
#include "../files/files.h"

#ifndef _WIN32
#define LIBPNG
#endif

#ifdef LIBPNG
#include <png.h>

// returns 0 if all-right, 1 otherwise
int _png_write(const char* filename, u_int width, u_int height, const u_char* data, bool alpha) {
    u_int pixsize = alpha ? 4 : 3;

	// Open file for writing (binary mode)
	FILE* fp = fopen(filename, "wb");
	if (fp == nullptr) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		fclose(fp);
		return 1;
	}

	// Initialize write structure
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (png_ptr == nullptr) {
		fprintf(stderr, "Could not allocate write struct\n");
		fclose(fp);
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		return 1;
	}

	// Initialize info structure
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr) {
		fprintf(stderr, "Could not allocate info struct\n");
		fclose(fp);
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		return 1;
		
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		fclose(fp);
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		return 1;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, 
            alpha ? PNG_COLOR_TYPE_RGBA : 
                       PNG_COLOR_TYPE_RGB, 
            PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, 
            PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	std::unique_ptr<png_byte[]> row(new png_byte[pixsize * width]);

	// Write image data
	for (uint y = 0; y < height; y++) {
		for (uint x = 0; x < width; x++) {
            for (uint i = 0; i < pixsize; i++) {
                row[x * pixsize + i] = (png_byte)data[(y * width + x) * pixsize + i];
            }
		}
		png_write_row(png_ptr, row.get());
	}

	// End write
	png_write_end(png_ptr, nullptr);

	fclose(fp);
	png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
	return 0;
}

ImageData* _png_load(const char* file){
    int bit_depth, color_type, row_bytes;
    png_uint_32 t_width, t_height;
    png_byte header[8];

    FILE *f = nullptr;
    if ((f = fopen(file, "r")) == nullptr) {
        return nullptr;
    }
    if (fread(header, 1, 8, f) < 8) { // check of read elements count
		fclose(f);
		return nullptr;
	}
	
    bool is_png = (png_sig_cmp(header, 0, 8) == 0);
    if (!is_png) {
        fclose(f);
        return nullptr;
    }
    png_struct* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
        nullptr, nullptr);
    if (png_ptr == nullptr) {
        fclose(f);
        return nullptr;
    }
    png_info* info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        png_destroy_read_struct(&png_ptr, (png_info**) nullptr,
            (png_info**) nullptr);
        fclose(f);
        return nullptr;
    }
    png_info* end_info = png_create_info_struct(png_ptr);
    if (end_info == nullptr) {
        png_destroy_read_struct(&png_ptr, (png_info**) nullptr,
            (png_info**) nullptr);
        fclose(f);
        return nullptr;
    }
    
    if (setjmp(png_jmpbuf(png_ptr)) != 0) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(f);
        return nullptr;
    }

    png_init_io(png_ptr, f);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &t_width, &t_height, &bit_depth,
                 &color_type, nullptr, nullptr, nullptr);
    png_read_update_info(png_ptr, info_ptr);
    row_bytes = png_get_rowbytes(png_ptr, info_ptr);

    std::unique_ptr<png_byte[]> image_data (new png_byte[row_bytes * t_height]);
    if (!image_data) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(f);
        return nullptr;
    }

    std::unique_ptr<png_byte*[]> row_pointers (new png_byte*[t_height]);
    if (row_pointers == nullptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(f);
        return nullptr;
    }
    for (uint i = 0; i < t_height; ++i ) {
        row_pointers[t_height - 1 - i] = image_data.get() + i * row_bytes;
    }
    png_read_image(png_ptr, row_pointers.get());

    ImageFormat format;
    switch (png_get_color_type(png_ptr, info_ptr)) {
        case PNG_COLOR_TYPE_RGBA:
            format = ImageFormat::rgba8888;
            break;
        case PNG_COLOR_TYPE_RGB:
            format = ImageFormat::rgb888;
            break;
        default:
            printf("Color type %d not supported!\n",
                   png_get_color_type(png_ptr, info_ptr));
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
			fclose(f);
            return nullptr;
    }
    ImageData* image = new ImageData(format, t_width, t_height, (void*)image_data.release());
    png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
    fclose(f);
    return image;
}
#else
#include <spng.h>
#include <stdio.h>
#include <inttypes.h>

static const int SPNG_SUCCESS = 0;
//returns spng result code
int _png_write(const char* filename, uint width, uint height, const ubyte* data, bool alpha) {
	int fmt;
	int ret = 0;
	spng_ctx* ctx = nullptr;
	spng_ihdr ihdr = { 0 };
	uint pixsize = alpha ? 4 : 3;

	ctx = spng_ctx_new(SPNG_CTX_ENCODER);
	spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

	ihdr.width = width;
	ihdr.height = height;
	ihdr.color_type = alpha ? SPNG_COLOR_TYPE_TRUECOLOR_ALPHA : SPNG_COLOR_TYPE_TRUECOLOR;
	ihdr.bit_depth = 8;

	spng_set_ihdr(ctx, &ihdr);
	fmt = SPNG_FMT_PNG;
	ret = spng_encode_image(ctx, data, (size_t)width * (size_t)height * pixsize , fmt, SPNG_ENCODE_FINALIZE);
	if (ret != SPNG_SUCCESS) {
		printf("spng_encode_image() error: %s\n", spng_strerror(ret));
		fflush(stdout);
		spng_ctx_free(ctx);
		return ret;
	}

	size_t png_size;
	void* png_buf = spng_get_png_buffer(ctx, &png_size, &ret);

	if (png_buf == nullptr) {
		printf("spng_get_png_buffer() error: %s\n", spng_strerror(ret));
	}
	else {
		files::write_bytes(filename, (const char*)png_buf, png_size);
	}
	fflush(stdout);
	spng_ctx_free(ctx);
	return ret;
}

ImageData* _png_load(const char* file){
	int r = 0;
	FILE *png = nullptr;
	char *pngbuf = nullptr;
	spng_ctx *ctx = nullptr;
	unsigned char *out = nullptr;

	png = fopen(file, "rb");
	if (png == nullptr){
		std::cerr << "could not to open file " << file << std::endl;
		return nullptr;
	}

	fseek(png, 0, SEEK_END);
	long siz_pngbuf = ftell(png);
	rewind(png);
	if(siz_pngbuf < 1) {
		fclose(png);
		std::cerr << "could not to read file " << file << std::endl;
		return nullptr;
	}
	pngbuf = new char[siz_pngbuf];
	if(fread(pngbuf, siz_pngbuf, 1, png) != 1){ //check of read elements count
		fclose(png);
		delete[] pngbuf;
		std::cerr << "fread() failed" << std::endl;
		return nullptr;
	}
	fclose(png); // <- finally closing file
	ctx = spng_ctx_new(0);
	if (ctx == nullptr){
		delete[] pngbuf;
		std::cerr << "spng_ctx_new() failed" << std::endl;
		return nullptr;
	}
	r = spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);
	if (r != SPNG_SUCCESS){
		delete[] pngbuf;
		spng_ctx_free(ctx);
		std::cerr << "spng_set_crc_action() error: " << spng_strerror(r) << std::endl;
		return nullptr;
	}
	r = spng_set_png_buffer(ctx, pngbuf, siz_pngbuf);
	if (r != SPNG_SUCCESS){
		delete[] pngbuf;
		spng_ctx_free(ctx);
		std::cerr << "spng_set_png_buffer() error: " << spng_strerror(r) << std::endl;
		return nullptr;
	}

	spng_ihdr ihdr;
	r = spng_get_ihdr(ctx, &ihdr);
	if (r != SPNG_SUCCESS){
		delete[] pngbuf;
		spng_ctx_free(ctx);
		std::cerr << "spng_get_ihdr() error: " << spng_strerror(r) << std::endl;
		return nullptr;
	}
	//// Unused "something"
	//const char *clr_type_str;
	//if(ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE)
	//	clr_type_str = "grayscale";
	//else if(ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR)
	//	clr_type_str = "truecolor";
	//else if(ihdr.color_type == SPNG_COLOR_TYPE_INDEXED)
	//	clr_type_str = "indexed color";
	//else if(ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA)
	//	clr_type_str = "grayscale with alpha";
	//else
	//	clr_type_str = "truecolor with alpha";

	size_t out_size;
	r = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size);
	if (r != SPNG_SUCCESS){
		delete[] pngbuf;
		spng_ctx_free(ctx);
		std::cerr << "spng_decoded_image_size() error: " << spng_strerror(r) << std::endl;
		return nullptr;
	}
	out = new unsigned char[out_size];
	r = spng_decode_image(ctx, out, out_size, SPNG_FMT_RGBA8, 0);
	if (r != SPNG_SUCCESS){
		delete[] out;
		delete[] pngbuf;
		spng_ctx_free(ctx);
		std::cerr << "spng_decode_image() error: " << spng_strerror(r) << std::endl;
		return nullptr;
	}

	unsigned char* flipped = new unsigned char[out_size];

	for (size_t i = 0; i < ihdr.height; i+=1){
		size_t rowsize = ihdr.width*4;
		for (size_t j = 0; j < rowsize; j++){
			flipped[(ihdr.height-i-1)*rowsize+j] = out[i*rowsize+j];
		}
	}
	delete[] out; // <- finally delete out // no, delete spng usage

    ImageData* image = new ImageData(ImageFormat::rgba8888, ihdr.width, ihdr.height, (void*)flipped);

	delete[] pngbuf;
	spng_ctx_free(ctx);

    return image;
}
#endif

ImageData* png::load_image(std::string filename) {
	ImageData* image (_png_load(filename.c_str()));
	if (image == nullptr) {
		std::cerr << "Could not load image " << filename << std::endl;
		return nullptr;
	}
    return image;
}

Texture* png::load_texture(std::string filename) {
	std::unique_ptr<ImageData> image (_png_load(filename.c_str()));
	if (image == nullptr){
		std::cerr << "Could not load texture " << filename << std::endl;
		return nullptr;
	}
	return Texture::from(image.get());
}

void png::write_image(std::string filename, const ImageData* image) {
    _png_write(filename.c_str(), image->getWidth(), image->getHeight(), (const u_char*)image->getData(), image->getFormat() == ImageFormat::rgba8888);
}