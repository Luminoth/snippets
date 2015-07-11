#include "src/pch.h"
#include "StackAllocator.h"

namespace energonsoftware {

Logger& StackAllocator::logger(Logger::instance("energonsoftware.core.util.StackAllocator"));

StackAllocator::StackAllocator(size_t size)
    : MemoryAllocator(), _pool(new unsigned char[_size]), _size(size), _marker(0)
{
    //LOG_DEBUG("Pool at " << reinterpret_cast<void*>(_pool.get()) << "\n");
}

StackAllocator::~StackAllocator() noexcept
{
}

void* StackAllocator::allocate(size_t bytes)
{
    std::lock_guard<std::recursive_mutex> guard(_mutex);

    assert(_marker + bytes < _size);

    size_t r = _marker;
    _marker += bytes;

    _allocation_count++;
    _allocation_bytes += bytes;

    //LOG_DEBUG("Allocating memory at " << reinterpret_cast<void*>(_pool.get() + r) << "\n");
    return _pool.get() + r;
}

void StackAllocator::release(void* ptr)
{
    std::lock_guard<std::recursive_mutex> guard(_mutex);

    //_marker -= ???;
    _allocation_count--;
    /*_allocation_bytes -= ???;

    _used -= ???;*/
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class StackAllocatorTest : public MemoryAllocatorTest
{
public:
    CPPUNIT_TEST_SUITE(StackAllocatorTest);
        CPPUNIT_TEST(test_allocate_nosmart);
        CPPUNIT_TEST(test_allocate_shared);
        CPPUNIT_TEST(test_allocate_unique);

        CPPUNIT_TEST(test_allocate_object);

        CPPUNIT_TEST(test_allocate_aligned_nosmart);
        CPPUNIT_TEST(test_allocate_aligned_shared);
        CPPUNIT_TEST(test_allocate_aligned_unique);

        CPPUNIT_TEST(test_allocate_object_aligned);

        CPPUNIT_TEST_EXCEPTION(test_unreasonable_allocation, std::bad_alloc);
    CPPUNIT_TEST_SUITE_END();

public:
    StackAllocatorTest() : MemoryAllocatorTest(energonsoftware::MemoryAllocator::Type::Stack) {}
    virtual ~StackAllocatorTest() noexcept {}

public:
    void test_unreasonable_allocation()
    {
        // 2-4 terabytes is pretty unreasonable, right?
#if defined WIN32
        static const size_t size = INT_MAX - 1;
#else
        static const size_t size = UINT_MAX - 1;
#endif

        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(
            energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::Stack, size));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(StackAllocatorTest);

#endif
