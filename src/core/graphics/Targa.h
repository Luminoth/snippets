#if !defined __TARGA_H__
#define __TARGA_H__

#include "Texture.h"

namespace energonsoftware {

class Targa : public Texture
{
private:
    enum class Type : uint8_t
    {
        NoImage        = 0,
        ColorMapped    = 1,
        RGB            = 2,
        Grayscale      = 3,
        RLEColorMapped = 9,
        RLERGB         = 10,
        RLEGrayscale   = 11
    };

    struct Header
    {
        uint8_t idlen;
        uint8_t cmtype;
        Type type;
        uint8_t cmoffset1;
        uint8_t cmoffset2;
        uint8_t cmlen1;
        uint8_t cmlen2;
        uint8_t cmdepth;
        uint8_t xorigin1;
        uint8_t xorigin2;
        uint8_t yorigin1;
        uint8_t yorigin2;
        uint8_t width1;
        uint8_t width2;
        uint8_t height1;
        uint8_t height2;
        uint8_t bpp;
        uint8_t descriptor;
    };

public:
    Targa();
    Targa(size_t width, size_t height, size_t bpp, const unsigned char* const pixels);
    virtual ~Targa() noexcept;

public:
    virtual size_t width() const override { return _width; }
    virtual size_t height() const override { return _height; }
    virtual size_t bpp() const override { return _bpp; }

public:
    virtual bool load(const boost::filesystem::path& filename, MemoryAllocator& allocator) override;
    virtual void unload() noexcept override;

    virtual bool save(const boost::filesystem::path& filename) const override;

private:
    Header _header;
    size_t _width, _height, _bpp;

private:
    DISALLOW_COPY_AND_ASSIGN(Targa);
};

}

#endif
