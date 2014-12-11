#if !defined __TEXTURE_H__
#define __TEXTURE_H__

namespace energonsoftware {

class MemoryAllocator;

class Texture
{
public:
    enum class Format
    {
        PNG,
        TGA,
        BMP,
    };

public:
    virtual ~Texture() noexcept;

public:
    unsigned char* pixels() { return _pixels.get(); }
    const unsigned char* pixels() const { return _pixels.get(); }

    virtual size_t width() const = 0;
    virtual size_t pitch() const final { return width() * Bpp(); }

    virtual size_t height() const = 0;

    virtual size_t bpp() const = 0;
    virtual size_t Bpp() const final { return bpp() >> 3; }

    virtual size_t size() const final { return width() * height() * Bpp(); }

public:
    virtual bool load(const boost::filesystem::path& filename, MemoryAllocator& allocator) = 0;
    virtual void unload() noexcept = 0;

    void flip_vertical();

    virtual bool save(const boost::filesystem::path& filename) const = 0;

protected:
    void allocate(MemoryAllocator& allocator, size_t size);
    void release();

private:
    std::shared_ptr<unsigned char> _pixels;

protected:
    Texture();
    Texture(const unsigned char* const pixels, int count);

private:
    DISALLOW_COPY_AND_ASSIGN(Texture);
};

}

#endif
