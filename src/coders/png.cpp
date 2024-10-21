#include "png.hpp"

#include <png.h>
#include <GL/glew.h>

#include <iostream>

#include "debug/Logger.hpp"
#include "files/files.hpp"
#include "graphics/core/GLTexture.hpp"
#include "graphics/core/ImageData.hpp"

static debug::Logger logger("png-coder");

// returns 0 if all-right, 1 otherwise
int _png_write(
    const char* filename, uint width, uint height, const ubyte* data, bool alpha
) {
    uint pixsize = alpha ? 4 : 3;

    // Open file for writing (binary mode)
    FILE* fp = fopen(filename, "wb");
    if (fp == nullptr) {
        logger.error() << "could not open file " << filename << " for writing";
        return 1;
    }

    // Initialize write structure
    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr
    );
    if (png_ptr == nullptr) {
        logger.error() << "could not allocate write struct";
        fclose(fp);
        return 1;
    }

    // Initialize info structure
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        logger.error() << "could not allocate info struct";
        fclose(fp);
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        png_destroy_write_struct(&png_ptr, (png_infopp) nullptr);
        return 1;
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        logger.error() << "error during png creation";
        fclose(fp);
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 1;
    }

    png_init_io(png_ptr, fp);

    // Write header (8 bit colour depth)
    png_set_IHDR(
        png_ptr,
        info_ptr,
        width,
        height,
        8,
        alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    png_write_info(png_ptr, info_ptr);

    auto row = std::make_unique<png_byte[]>(pixsize * width);
    // Write image data
    for (uint y = 0; y < height; y++) {
        for (uint x = 0; x < width; x++) {
            for (uint i = 0; i < pixsize; i++) {
                row[x * pixsize + i] =
                    (png_byte)data[(y * width + x) * pixsize + i];
            }
        }
        png_write_row(png_ptr, row.get());
    }

    // End write
    png_write_end(png_ptr, nullptr);

    fclose(fp);
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
}

std::unique_ptr<ImageData> _png_load(const char* file) {
    FILE* fp = nullptr;
    if ((fp = fopen(file, "rb")) == nullptr) {
        return nullptr;
    }
    png_struct* png = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr
    );
    if (png == nullptr) {
        fclose(fp);
        return nullptr;
    }
    png_info* info = png_create_info_struct(png);
    if (info == nullptr) {
        png_destroy_read_struct(&png, (png_info**)nullptr, (png_info**)nullptr);
        fclose(fp);
        return nullptr;
    }
    png_info* end_info = png_create_info_struct(png);
    if (end_info == nullptr) {
        png_destroy_read_struct(&png, (png_info**)nullptr, (png_info**)nullptr);
        fclose(fp);
        return nullptr;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, &end_info);
        fclose(fp);
        return nullptr;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    int bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16) png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    int row_bytes = png_get_rowbytes(png, info);
    // color_type = png_get_color_type(png, info);
    //  png_get_color_type returns 2 (RGB) but raster always have alpha channel
    //  due to PNG_FILLER_AFTER

    color_type = 6;
    bit_depth = png_get_bit_depth(png, info);

    auto image_data = std::make_unique<png_byte[]>(row_bytes * height);
    auto row_pointers = std::make_unique<png_byte*[]>(height);
    for (int i = 0; i < height; ++i) {
        row_pointers[height - 1 - i] = image_data.get() + i * row_bytes;
    }
    png_read_image(png, row_pointers.get());

    ImageFormat format = ImageFormat::rgba8888;
    switch (color_type) {
        case PNG_COLOR_TYPE_RGBA:
            format = ImageFormat::rgba8888;
            break;
        case PNG_COLOR_TYPE_RGB:
            format = ImageFormat::rgb888;
            break;
        default:
            logger.error() << "color type " << color_type
                           << " is not supported!";
            png_destroy_read_struct(&png, &info, &end_info);
            fclose(fp);
            return nullptr;
    }
    auto image = std::make_unique<ImageData>(
        format, width, height, std::move(image_data)
    );
    png_destroy_read_struct(&png, &info, &end_info);
    fclose(fp);
    return image;
}

std::unique_ptr<ImageData> png::load_image(const std::string& filename) {
    auto image = _png_load(filename.c_str());
    if (image == nullptr) {
        throw std::runtime_error("could not load image " + filename);
    }
    return image;
}

std::unique_ptr<Texture> png::load_texture(const std::string& filename) {
    auto image = load_image(filename);
    auto texture = GLTexture::from(image.get());
    texture->setNearestFilter();
    return texture;
}

void png::write_image(const std::string& filename, const ImageData* image) {
    _png_write(
        filename.c_str(),
        image->getWidth(),
        image->getHeight(),
        (const ubyte*)image->getData(),
        image->getFormat() == ImageFormat::rgba8888
    );
}
