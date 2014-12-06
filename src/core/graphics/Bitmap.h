#if !defined __BITMAP_H__
#define __BITMAP_H__

#include "Texture.h"

namespace energonsoftware {

// TODO: this class is not implemented!

class Bitmap : public Texture
{
public:
    Bitmap();
    Bitmap(size_t width, size_t height, size_t bpp, const unsigned char* const pixels);
    virtual ~Bitmap() noexcept;

public:
    virtual size_t width() const override { return _width; }
    virtual size_t height() const override { return _height; }
    virtual size_t bpp() const override { return _bpp; }

public:
    virtual bool load(const boost::filesystem::path& filename, MemoryAllocator& allocator) override;
    virtual void unload() noexcept override;

    virtual bool save(const boost::filesystem::path& filename) const override;

private:
    size_t _width, _height, _bpp;

private:
    DISALLOW_COPY_AND_ASSIGN(Bitmap);
};

}

#endif
