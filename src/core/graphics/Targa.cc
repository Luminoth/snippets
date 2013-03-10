#include "src/pch.h"
#include <fstream>
#include "Targa.h"

namespace energonsoftware {

void Targa::destroy(Targa* const texture, MemoryAllocator* const allocator)
{
    texture->~Targa();
    operator delete(texture, *allocator);
}

Targa::Targa()
    : Texture(), _header(), _width(0), _height(0), _bpp(0)
{
    // TODO: replace with a Header constructor
    ZeroMemory(&_header, sizeof(Header));
}

Targa::Targa(size_t width, size_t height, size_t bpp, boost::shared_array<unsigned char> pixels)
    : Texture(pixels), _header(), _width(width), _height(height), _bpp(bpp)
{
    // TODO: replace with a Header constructor
    ZeroMemory(&_header, sizeof(Header));
}

Targa::~Targa() noexcept
{
}

bool Targa::load(const boost::filesystem::path& filename, MemoryAllocator& allocator)
{
    unload();

    std::ifstream f(filename.string().c_str(), std::ios::binary);
    if(!f) {
        return false;
    }

    // NOTE: we only support RGB/RGBA
    f.read(reinterpret_cast<char*>(&_header), sizeof(Header));
    if(_header.type != Type::RGB) {
        return false;
    }

    if(_header.bpp != 24 && _header.bpp != 32) {
        return false;
    }

    // little-endian ordering
    _width = (_header.width2 << 8) + _header.width1;
    _height = (_header.height2 << 8) + _header.height1;
    _bpp = _header.bpp;

    // skip TARGA junk
    f.seekg(_header.idlen, std::ios::cur);

    allocate(allocator, size());
    f.read(reinterpret_cast<char*>(pixels().get()), size());

    flip_vertical();
    return true;
}

void Targa::unload() noexcept
{
    ZeroMemory(&_header, sizeof(Header));
    _width = _height = _bpp = 0;
    release();
}

bool Targa::save(const boost::filesystem::path& filename) const
{
// TODO: implement me!
    return false;
}

}
