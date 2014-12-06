#if !defined __PNG_H__
#define __PNG_H__

#include "Texture.h"

namespace energonsoftware {

class PNG : public Texture
{
public:
    static void destroy(PNG* const texture, MemoryAllocator* const allocator);

private:
    static Logger& logger;

public:
    PNG();
    PNG(size_t width, size_t height, size_t bpp, const unsigned char* const pixels);
    virtual ~PNG() noexcept;

public:
    virtual size_t width() const { return _width; }
    virtual size_t height() const { return _height; }
    virtual size_t bpp() const { return _bpp; }

public:
    virtual bool load(const boost::filesystem::path& filename, MemoryAllocator& allocator);
    virtual void unload() noexcept;

    virtual bool save(const boost::filesystem::path& filename) const;

private:
    size_t _width, _height, _bpp;

private:
    DISALLOW_COPY_AND_ASSIGN(PNG);
};

}

#endif
