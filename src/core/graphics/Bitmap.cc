#include "src/pch.h"
#include <fstream>
#include "Bitmap.h"

namespace energonsoftware {

Bitmap::Bitmap()
    : Texture(), _width(0), _height(0), _bpp(0)
{
}

Bitmap::Bitmap(size_t width, size_t height, size_t bpp, boost::shared_array<unsigned char> pixels)
    : Texture(pixels), _width(width), _height(height), _bpp(bpp)
{
}

Bitmap::~Bitmap() throw()
{
}

bool Bitmap::load(const boost::filesystem::path& filename, MemoryAllocator& allocator)
{
    unload();

    std::ifstream f(filename.string().c_str(), std::ios::binary);
    if(!f) {
        return false;
    }

    // TODO: implement me!

    return false;
}

void Bitmap::unload() throw()
{
    _width = _height = _bpp = 0;
    release();
}

bool Bitmap::save(const boost::filesystem::path& filename) const
{
    // TODO: implement me!

    return false;
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
    virtual ~BitmapTest() throw() {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapTest);

#endif
