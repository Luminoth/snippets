#include "src/pch.h"
#include "Texture.h"

namespace energonsoftware {

Texture::Texture()
    : _pixels()
{
}

Texture::Texture(boost::shared_array<unsigned char> pixels)
    : _pixels(pixels)
{
}

Texture::~Texture() throw()
{
}

void Texture::flip_vertical()
{
    // TODO: put this on an allocator!
    boost::scoped_array<unsigned char> scratch(new unsigned char[size()]);
    for(size_t i=0; i<height(); ++i) {
        const size_t dst = i * pitch();
        const size_t src = (height() - i - 1) * pitch();
        std::memcpy(scratch.get() + dst, pixels().get() + src, pitch());
    }
    std::memcpy(pixels().get(), scratch.get(), size());
}

void Texture::allocate(MemoryAllocator& allocator, size_t size)
{
    release();

    _pixels.reset(new(allocator) unsigned char[size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));
}

void Texture::release()
{
    _pixels.reset();
}

}
