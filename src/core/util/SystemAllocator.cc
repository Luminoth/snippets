#include "src/pch.h"
#include "SystemAllocator.h"

namespace energonsoftware {

Logger& SystemAllocator::logger(Logger::instance("energonsoftware.core.util.SystemAllocator"));

SystemAllocator::SystemAllocator(size_t size)
    : MemoryAllocator(), _size(size), _used(0)
{
}

SystemAllocator::~SystemAllocator() noexcept
{
}

void* SystemAllocator::allocate(size_t bytes)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    assert(_used + bytes < _size);

    _allocation_count++;
    _allocation_bytes += bytes;

    _used += bytes;
    unsigned char* r = new unsigned char[bytes];

    //LOG_DEBUG("Allocating memory at " << reinterpret_cast<void*>(r) << "\n");
    return r;
}

void SystemAllocator::release(void* ptr)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    _allocation_count--;
    /*_allocation_bytes -= ???;

    _used -= ???;*/

    delete[] reinterpret_cast<unsigned char*>(ptr);
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class SystemAllocatorTest : public MemoryAllocatorTest
{
public:
    CPPUNIT_TEST_SUITE(SystemAllocatorTest);
        CPPUNIT_TEST(test_allocate);
        CPPUNIT_TEST(test_allocate_object);
        CPPUNIT_TEST(test_allocate_aligned);
        CPPUNIT_TEST(test_allocate_object_aligned);
        CPPUNIT_TEST_EXCEPTION(test_unreasonable_allocation, std::bad_alloc);
    CPPUNIT_TEST_SUITE_END();

public:
    SystemAllocatorTest() : MemoryAllocatorTest(energonsoftware::MemoryAllocator::Type::System) {}
    virtual ~SystemAllocatorTest() noexcept {}

public:
    void test_unreasonable_allocation()
    {
        // 2-4 terabytes is pretty unreasonable, right?
#if defined WIN32
        static const size_t size = INT_MAX;
#else
        static const size_t size = UINT_MAX;
#endif

        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(
            energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, size));
        // have to use size-1 here because of the allocation < size assertion in allocate()
        allocator->allocate(size-1);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SystemAllocatorTest);

#endif
