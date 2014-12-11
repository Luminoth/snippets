#if !defined __MEMORYALLOCATOR_H__
#define __MEMORYALLOCATOR_H__

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
    virtual ~MemoryAllocator() noexcept;

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
    virtual void* allocate_aligned(size_t bytes, size_t alignment) final;
    //virtual void* allocate_array_aligned(size_t bytes, size_t alignment) final;
    virtual void release_aligned(void* ptr, size_t alignment) final;
    //virtual void release_array_aligned(void* ptr, size_t alignment) final;

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

// unaligned allocators/deleters
template<typename T>
T* MemoryAllocator_new(size_t count, MemoryAllocator& allocator)
{
    T* objs = reinterpret_cast<T*>(allocator.allocate(sizeof(T) * count));

    T *obj = objs, *end = objs + count;
    while(obj != end) {
        new(obj) T();
        obj++;
    }

    return objs;
}

template<typename T, typename E=void>
class MemoryAllocator_delete;

template<typename T>
class MemoryAllocator_delete<T, typename std::enable_if<std::is_class<T>::value>::type>
{
public:
    explicit MemoryAllocator_delete(MemoryAllocator* allocator)
        : _allocator(allocator)
    {
    }

public:
    void operator()(T* ptr) const
    {
        ptr->~T();
        operator delete(ptr, *_allocator);
    }

private:
    MemoryAllocator* _allocator;

private:
    MemoryAllocator_delete() = delete;
};

template<typename T>
class MemoryAllocator_delete<T[], typename std::enable_if<std::is_class<T>::value>::type>
{
public:
    MemoryAllocator_delete(size_t count, MemoryAllocator* allocator)
        : _count(count), _allocator(allocator)
    {
    }

public:
    void operator()(T* ptr) const
    {
        T* current = ptr + _count;
        while(current > ptr) {
            (--current)->~T();
        }
        operator delete[](ptr, *_allocator);
    }

    template<typename U>
    void operator()(U* ptr) const = delete;

private:
    size_t _count;
    MemoryAllocator* _allocator;

private:
    MemoryAllocator_delete() = delete;
};

// aligned allocators/deleters
template<typename T, size_t align>
T* MemoryAllocator_new_aligned(size_t count, MemoryAllocator& allocator)
{
    T* objs = reinterpret_cast<T*>(allocator.allocate_aligned(sizeof(T) * count, align));

    T *obj = objs, *end = objs + count;
    while(obj != end) {
        new(obj) T();
        obj++;
    }

    return objs;
}

template<typename T, size_t align, typename E=void>
class MemoryAllocator_delete_aligned;

template<typename T, size_t align>
class MemoryAllocator_delete_aligned<T, align, typename std::enable_if<std::is_class<T>::value>::type>
{
public:
    explicit MemoryAllocator_delete_aligned(MemoryAllocator* allocator)
        : _allocator(allocator)
    {
    }

public:
    void operator()(T* ptr) const
    {
        ptr->~T();
        operator delete(ptr, align, *_allocator);
    }

private:
    MemoryAllocator* _allocator;

private:
    MemoryAllocator_delete_aligned() = delete;
};

template<typename T, size_t align>
class MemoryAllocator_delete_aligned<T[], align, typename std::enable_if<std::is_class<T>::value>::type>
{
public:
    MemoryAllocator_delete_aligned(size_t count, MemoryAllocator* allocator)
        : _count(count), _allocator(allocator)
    {
    }

public:
    void operator()(T* ptr) const
    {
        T* current = ptr + _count;
        while(current > ptr) {
            (--current)->~T();
        }
        operator delete[](ptr, align, *_allocator);
    }

    template<class U>
    void operator()(U* ptr) const = delete;

private:
    size_t _count;
    MemoryAllocator* _allocator;

private:
    MemoryAllocator_delete_aligned() = delete;
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
    return allocator.allocate_aligned(bytes, alignment);
}

inline void operator delete(void* mem, size_t alignment, energonsoftware::MemoryAllocator& allocator)
{
    allocator.release_aligned(mem, alignment);
}

inline void* operator new[](size_t bytes, size_t alignment, energonsoftware::MemoryAllocator& allocator)
{
    //return allocator.allocate_array(bytes, alignment);
    return allocator.allocate_aligned(bytes, alignment);
}

inline void operator delete[](void* mem, size_t alignment, energonsoftware::MemoryAllocator& allocator)
{
    //allocator.release_array(mem, alignment);
    allocator.release_aligned(mem, alignment);
}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

/*
Implementers should implement the following tests:

CPPUNIT_TEST(test_allocate_nosmart);
CPPUNIT_TEST(test_allocate_shared);
CPPUNIT_TEST(test_allocate_unique);

CPPUNIT_TEST(test_allocate_object);

CPPUNIT_TEST(test_allocate_aligned_nosmart);
CPPUNIT_TEST(test_allocate_aligned_shared);
CPPUNIT_TEST(test_allocate_aligned_unique);

CPPUNIT_TEST(test_allocate_object_aligned);
*/

class MemoryAllocatorTest : public CppUnit::TestFixture
{
public:
    explicit MemoryAllocatorTest(energonsoftware::MemoryAllocator::Type type);
    virtual ~MemoryAllocatorTest() noexcept {}

public:
    void setUp();
    void tearDown();

    void test_allocate_nosmart();
    void test_allocate_shared();
    void test_allocate_unique();

    void test_allocate_object();

    void test_allocate_aligned_nosmart();
    void test_allocate_aligned_shared();
    void test_allocate_aligned_unique();

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
