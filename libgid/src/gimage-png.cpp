#include <gstdio.h>
#include <gimage.h>
#include <png.h>
#include <vector>

static void read_png_data(png_structp pngPtr, png_bytep data, png_size_t length)
{
    G_FILE* file = (G_FILE*)png_get_io_ptr(pngPtr);
    g_fread(data, 1, length, file);
}

static void write_png_data(png_structp pngPtr, png_bytep data, png_size_t length)
{
    G_FILE* file = (G_FILE*)png_get_io_ptr(pngPtr);
    g_fwrite(data, 1, length, file);
}

static void flush_png_data(png_structp pngPtr)
{
    G_FILE* file = (G_FILE*)png_get_io_ptr(pngPtr);
    g_fflush(file);
}

extern "C" {

int gimage_parsePng(const char *pathname, int *width, int *height, int *comp)
{
    G_FILE* fp = g_fopen(pathname, "rb");

    if (!fp)
        return GIMAGE_CANNOT_OPEN_FILE;

    #define HEADERSIZE 8
    unsigned char header[HEADERSIZE];
    if (g_fread(header, 1, HEADERSIZE, fp) != HEADERSIZE)
    {
        g_fclose(fp);
        return GIMAGE_UNRECOGNIZED_FORMAT;
    }

    if (png_sig_cmp(header, 0, HEADERSIZE) != 0)
    {
        g_fclose(fp);
        return GIMAGE_UNRECOGNIZED_FORMAT;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        g_fclose(fp);
        return GIMAGE_ERROR_WHILE_READING;
    }

    png_set_read_fn(png_ptr, fp, read_png_data);

    png_set_sig_bytes(png_ptr, HEADERSIZE);
    png_read_info(png_ptr, info_ptr);

    // Strip 16 bit/color files down to 8 bits/color
    png_set_strip_16(png_ptr);

    // Pack 1, 2, 4 bit into bytes
    png_set_packing(png_ptr);

    // Auto-convert 1-, 2-, and 4- bit images to 8 bits, palette to RGB,
    // and transparency to alpha
    png_set_expand(png_ptr);

    // Update the information struct appropriately
    png_read_update_info(png_ptr, info_ptr);

    if (width)
        *width = png_get_image_width(png_ptr, info_ptr);

    if (height)
        *height = png_get_image_height(png_ptr, info_ptr);

    if (comp)
        *comp = png_get_channels(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    g_fclose(fp);

    return GIMAGE_NO_ERROR;
}

int gimage_loadPng(const char *pathname, void *buf)
{
    G_FILE* fp = g_fopen(pathname, "rb");

    if (!fp)
        return GIMAGE_CANNOT_OPEN_FILE;

    #define HEADERSIZE 8
    unsigned char header[HEADERSIZE];
    if (g_fread(header, 1, HEADERSIZE, fp) != HEADERSIZE)
    {
        g_fclose(fp);
        return GIMAGE_UNRECOGNIZED_FORMAT;
    }

    if (png_sig_cmp(header, 0, HEADERSIZE) != 0)
    {
        g_fclose(fp);
        return GIMAGE_UNRECOGNIZED_FORMAT;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        g_fclose(fp);
        return GIMAGE_ERROR_WHILE_READING;
    }

    png_set_read_fn(png_ptr, fp, read_png_data);

    png_set_sig_bytes(png_ptr, HEADERSIZE);
    png_read_info(png_ptr, info_ptr);

    // Strip 16 bit/color files down to 8 bits/color
    png_set_strip_16(png_ptr);

    // Pack 1, 2, 4 bit into bytes
    png_set_packing(png_ptr);

    // Auto-convert 1-, 2-, and 4- bit images to 8 bits, palette to RGB,
    // and transparency to alpha
    png_set_expand(png_ptr);

    // Update the information struct appropriately
    png_read_update_info(png_ptr, info_ptr);

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    png_byte channels = png_get_channels(png_ptr, info_ptr);

    std::vector<unsigned char *> row_pointers(height);

    for (png_uint_32 i = 0; i < height; ++i)
        row_pointers[i] = (unsigned char*)buf + (i * width * channels);

    png_read_image(png_ptr, &row_pointers[0]);

    // If one has no need for the post-IDAT chunk data, the second argument can be NULL
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    g_fclose(fp);

    return GIMAGE_NO_ERROR;
}

int gimage_savePng(const char *filename, int width, int height, unsigned char *data)
{
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        return GIMAGE_ERROR_WHILE_WRITING;

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, &info);
        return GIMAGE_ERROR_WHILE_WRITING;
    }

    G_FILE* fp = g_fopen(filename, "wb");

    if (!fp) {
        png_destroy_write_struct(&png, &info);
        return GIMAGE_CANNOT_OPEN_FILE;
    }

    png_set_write_fn(png, fp, write_png_data, flush_png_data);
    png_set_IHDR(png, info, width, height, 8 /* depth */, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_colorp palette = (png_colorp)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
    if (!palette) {
        g_fclose(fp);
        png_destroy_write_struct(&png, &info);
        return false;
    }
    png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
    png_write_info(png, info);
    png_set_packing(png);

    png_bytepp rows = (png_bytepp)png_malloc(png, height * sizeof(png_bytep));
    for (int i = 0; i < height; ++i)
        rows[i] = (png_bytep)(data + i * width * 4);

    png_write_image(png, rows);
    png_write_end(png, info);
    png_free(png, palette);
    png_destroy_write_struct(&png, &info);

    g_fclose(fp);
    png_free(png,rows);
    return GIMAGE_NO_ERROR;
}

}
