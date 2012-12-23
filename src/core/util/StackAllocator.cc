#include "src/pch.h"
#include "StackAllocator.h"

namespace energonsoftware {

Logger& StackAllocator::logger(Logger::instance("energonsoftware.core.util.StackAllocator"));

StackAllocator::StackAllocator(size_t size)
    : MemoryAllocator(), _size(size), _marker(0)
{
    _pool.reset(new unsigned char[_size]);
    //LOG_DEBUG("Pool at " << reinterpret_cast<void*>(_pool.get()) << "\n");
}

StackAllocator::~StackAllocator() throw()
{
}

void* StackAllocator::allocate(size_t bytes)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

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
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

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
        CPPUNIT_TEST(test_allocate);
        CPPUNIT_TEST(test_allocate_object);
        CPPUNIT_TEST(test_allocate_aligned);
        CPPUNIT_TEST(test_allocate_object_aligned);
        CPPUNIT_TEST(test_unreasonable_allocation);
    CPPUNIT_TEST_SUITE_END();

public:
    StackAllocatorTest() : MemoryAllocatorTest(energonsoftware::AllocatorType::Stack) {}
    virtual ~StackAllocatorTest() throw() {}

public:
    void test_unreasonable_allocation()
    {
        // 2 terabytes is pretty unreasonable, right?
        static const size_t size = INT_MAX - 1;

        CPPUNIT_ASSERT_THROW(std::shared_ptr<energonsoftware::MemoryAllocator> allocator(
            energonsoftware::MemoryAllocator::new_allocator(energonsoftware::AllocatorType::Stack, size)), std::bad_alloc);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(StackAllocatorTest);

#endif
