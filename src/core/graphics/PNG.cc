#include "src/pch.h"
#include <fstream>
#include <png.h>
#include "PNG.h"

namespace energonsoftware {

void png_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::ifstream* f = reinterpret_cast<std::ifstream*>(png_get_io_ptr(png_ptr));
    f->read(reinterpret_cast<char*>(data), length);
}

void png_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::ofstream* f = reinterpret_cast<std::ofstream*>(png_get_io_ptr(png_ptr));
    f->write(reinterpret_cast<char*>(data), length);
}

void png_user_warn(png_structp ctx, png_const_charp str)
{
    std::cerr << "libpng: warning: " << str << "\n";
}

void png_user_error(png_structp ctx, png_const_charp str)
{
    std::cerr << "libpng: error: " << str << "\n";
}

void PNG::destroy(PNG* const texture, MemoryAllocator* const allocator)
{
    texture->~PNG();
    operator delete(texture, *allocator);
}

Logger& PNG::logger(Logger::instance("energonsoftware.core.graphics.PNG"));

PNG::PNG()
    : Texture(), _width(0), _height(0), _bpp(0)
{
}

PNG::PNG(size_t width, size_t height, size_t bpp, boost::shared_array<unsigned char> pixels)
    : Texture(pixels), _width(width), _height(height), _bpp(bpp)
{
}

PNG::~PNG() throw()
{
}

bool PNG::load(const boost::filesystem::path& filename, MemoryAllocator& allocator)
{
    unload();

    std::ifstream f(filename.string().c_str(), std::ios::binary);
    if(!f) {
        return false;
    }

    png_byte header[8];
    f.read(reinterpret_cast<char*>(header), 8);
    if(png_sig_cmp(header, 0, 8)) {
        return false;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, png_user_error, png_user_warn);
    if(nullptr == png_ptr) {
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(nullptr == info_ptr) {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return false;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if(nullptr == end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return false;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return false;
    }

    png_set_read_fn(png_ptr, &f, png_read);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    if(bit_depth == 16) {
        png_set_strip_16(png_ptr);
    } else if(bit_depth < 8) {
        /*if(PNG_COLOR_TYPE_GRAY == color_type) {
            png_set_gray_1_2_4_to_8(png_ptr);
        } else {*/
            LOG_ERROR("Unsupported bit depth: " << bit_depth << "\n");
            return false;
        //}
    }

    if(PNG_COLOR_TYPE_PALETTE == color_type) {
        png_set_palette_to_rgb(png_ptr);
    } else if(PNG_COLOR_TYPE_GRAY == color_type || PNG_COLOR_TYPE_GRAY_ALPHA == color_type) {
        png_set_gray_to_rgb(png_ptr);
    }

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    //png_set_invert_alpha(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    _width = png_get_image_width(png_ptr, info_ptr);
    _height = png_get_image_height(png_ptr, info_ptr);

    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    int channels = png_get_channels(png_ptr, info_ptr);
    _bpp = channels * bit_depth;

    allocate(allocator, size());

    png_bytep* row_pointers = (png_bytep*)png_malloc(png_ptr, height() * sizeof(png_bytep));
    for(size_t i=0; i<height(); ++i) {
        row_pointers[i] = (png_bytep)png_malloc(png_ptr, width() * Bpp());
    }

    png_read_image(png_ptr, row_pointers);

    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    for(size_t i=0; i<height(); ++i) {
        std::memcpy(pixels().get() + (i * pitch()), row_pointers[i], pitch());
        png_free(png_ptr, row_pointers[i]);
    }
    png_free(png_ptr, row_pointers);

    return true;
}

void PNG::unload() throw()
{
    _width = _height = _bpp = 0;
    release();
}

bool PNG::save(const boost::filesystem::path& filename) const
{
// TODO: change to use std::ofstream
    FILE* fp = nullptr;
    if(0 != fopen_s(&fp, filename.string().c_str(), "wb")) {
        return false;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, png_user_error, png_user_warn);
    if(nullptr == png_ptr) {
        fclose(fp);
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(nullptr == info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
        fclose(fp);
        return false;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width(), height(), 8, Bpp() == 4 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    //png_set_invert_alpha(png_ptr);

    png_write_info(png_ptr, info_ptr);
    png_set_packing(png_ptr);

    boost::shared_array<png_bytep> row_pointers(new png_bytep[height()]);
    for(size_t i=0; i<height(); ++i) {
        row_pointers[i] = (png_bytep)(pixels().get() + (i * pitch()));
    }
    png_write_image(png_ptr, row_pointers.get());
    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
    return true;
}

}
