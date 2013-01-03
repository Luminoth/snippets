#if !defined __MEMORYALLOCATOR_H__
#define __MEMORYALLOCATOR_H__

/*
Example:

Object::destroy/destroy_array calls the appropriate destructor and delete operator
* allocator is held as a pointer (required for std::bind)
    eg - Object->~Object(); operator delete(obj, *allocator)
    eg - for(object : objs) { object->~Object(); } operator delete[](objs, *allocator)
        * NOTE: this should ideally be done in reverse order to match creation semantics
    eg - Object->~Object(); operator delete(obj, alignment, *allocator)
    eg - for(object : objs) { object->~Object(); } operator delete[](objs, alignment, *allocator)
        * NOTE: this should ideally be done in reverse order to match creation semantics

DerivedAllocator allocator(500 * 1024 * 1024);    // allocate 500MB
std::shared_ptr<Object> obj1(new(allocator) Object(), std::bind(&Object::destroy_object, std::placeholders::_1, &allocator));
std::shared_ptr<Object> obj2(new(alignment, allocator) Object(), std::bind(&Object::destroy_object, std::placeholders::_1, alignment, &allocator));
...
obj2.reset();
obj1.reset();
allocator.reset();

Arrays of objects must be allocated specially:
    boost::shared_array<Object> objects1(reinterpret_cast<Object*>(allocator.allocate(sizeof(Object) * count)),
        std::bind(&Object::destroy_array, std::placeholders::_1, count, &allocator));
    Object::assign_array(objects1.get(), count);

    boost::shared_array<Object> objects2(reinterpret_cast<Object*>(allocator.allocate(sizeof(Object) * count, alignment)),
        std::bind(&Object::destroy_array, std::placeholders::_1, count, alignment, &allocator));
    Object::assign_array(objects2.get(), count);

This is necessary to work around compilers storing the size of the array in memory.

For objects that don't require a destructor call, allocator.release() may be called directly:
    boost::shared_array<unsigned char> data(new(allocator) unsigned char[size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));
*/

namespace energonsoftware {

class MemoryAllocator
{
public:
    enum class Type
    {
        Stack,
        System
    };

public:
    // size is in bytes
    static std::shared_ptr<MemoryAllocator> new_allocator(Type type, size_t size);

public:
    virtual ~MemoryAllocator() throw();

public:
    // values returned in bytes
    // TODO: used and unused counts are not accurate wrt release() calls
    virtual size_t total() const = 0;
    virtual size_t used() const = 0;
    virtual size_t unused() const = 0;

    virtual unsigned int allocation_count() const final { return _allocation_count; }
    // TODO: this is unreliable for now because we don't know deletion bytes
    // if we can sort out a good way to handle that, re-enable this
    //virtual size_t allocation_bytes() const final { return _allocation_bytes; }

    // NOTE: all of the allocation() and release() overrides
    // must lock the allocator with a boost::lock_guard

    // allocate unaligned memory
    // NOTE: overriding classes *must* maintain
    // allocation statistics inside of this
    virtual void* allocate(size_t bytes) = 0;
    //virtual void* allocate_array(size_t bytes) final;

    // release unaligned memory
    // NOTE: depending on the implementation, this may do nothing
    virtual void release(void* ptr) = 0;
    //virtual void release_array(void* ptr) final;

    // allocate/release aligned memory
    // NOTE: alignment must be a power of 2 greater than 1,
    // and must be used consistenty across these methods
    virtual void* allocate(size_t bytes, size_t alignment) final;
    //virtual void* allocate_array(size_t bytes, size_t alignment) final;
    virtual void release(void* ptr, size_t alignment) final;
    //virtual void release_array(void* ptr, size_t alignment) final;

    // resets (but does not free memory) any internal state
    virtual void reset() = 0;

protected:
    MemoryAllocator();

protected:
    boost::recursive_mutex _mutex;
    size_t _allocation_count;
    unsigned int _allocation_bytes;

private:
    DISALLOW_COPY_AND_ASSIGN(MemoryAllocator);
};

}

// NOTE: these only free the *memory*, not the *object* (if there is one)

// unaligned operators
inline void* operator new(size_t bytes, energonsoftware::MemoryAllocator& allocator)
{
    return allocator.allocate(bytes);
}

inline void operator delete(void* mem, energonsoftware::MemoryAllocator& allocator)
{
    allocator.release(mem);
}

inline void* operator new[](size_t bytes, energonsoftware::MemoryAllocator& allocator)
{
    //return allocator.allocate_array(bytes);
    return allocator.allocate(bytes);
}

inline void operator delete[](void* mem, energonsoftware::MemoryAllocator& allocator)
{
    //allocator.release_array(mem);
    allocator.release(mem);
}

// aligned operators
inline void* operator new(size_t bytes, size_t alignment, energonsoftware::MemoryAllocator& allocator)
{
    return allocator.allocate(bytes, alignment);
}

inline void operator delete(void* mem, size_t alignment, energonsoftware::MemoryAllocator& allocator)
{
    allocator.release(mem, alignment);
}

inline void* operator new[](size_t bytes, size_t alignment, energonsoftware::MemoryAllocator& allocator)
{
    //return allocator.allocate_array(bytes, alignment);
    return allocator.allocate(bytes, alignment);
}

inline void operator delete[](void* mem, size_t alignment, energonsoftware::MemoryAllocator& allocator)
{
    //allocator.release_array(mem, alignment);
    allocator.release(mem, alignment);
}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

/*
Implementers should implement the following tests:

CPPUNIT_TEST(test_allocate);
CPPUNIT_TEST(test_allocate_object);
CPPUNIT_TEST(test_allocate_aligned);
CPPUNIT_TEST(test_allocate_object_aligned);
*/

class MemoryAllocatorTest : public CppUnit::TestFixture
{
public:
    explicit MemoryAllocatorTest(energonsoftware::MemoryAllocator::Type type);
    virtual ~MemoryAllocatorTest() throw() {}

public:
    void setUp();
    void tearDown();

    void test_allocate();
    void test_allocate_object();
    void test_allocate_aligned();
    void test_allocate_object_aligned();

private:
    void check_value(int* value);
    void check_buffer(char* buffer);

private:
    energonsoftware::MemoryAllocator::Type _type;
    std::shared_ptr<energonsoftware::MemoryAllocator> _allocator;

private:
    MemoryAllocatorTest();
};
#endif

#endif
