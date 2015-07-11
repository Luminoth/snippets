#include "src/pch.h"
#include <fstream>
#include "Bitmap.h"

namespace energonsoftware {

Bitmap::Bitmap()
    : Texture(), _width(0), _height(0), _bpp(0)
{
}

Bitmap::Bitmap(size_t width, size_t height, size_t bpp, const unsigned char* const pixels)
    : Texture(pixels, width * height), _width(width), _height(height), _bpp(bpp)
{
}

Bitmap::~Bitmap() noexcept
{
}

bool Bitmap::load(const boost::filesystem::path& filename, MemoryAllocator& allocator)
{
    unload();

    std::ifstream f(filename.string().c_str(), std::ios::binary);
    if(!f) {
        return false;
    }

    throw NotImplementedError("Bitmap::load()");
}

void Bitmap::unload() noexcept
{
    _width = _height = _bpp = 0;
    release();
}

bool Bitmap::save(const boost::filesystem::path& filename) const
{
    throw NotImplementedError("Bitmap::save()");
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class BitmapTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(BitmapTest);
    CPPUNIT_TEST_SUITE_END();

public:
    BitmapTest() : CppUnit::TestFixture() {}
    virtual ~BitmapTest() noexcept {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapTest);

#endif
