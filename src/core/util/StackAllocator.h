#if !defined __STACKALLOCATOR_H__
#define __STACKALLOCATOR_H__

#include "MemoryAllocator.h"

namespace energonsoftware {

/*
This allocator uses the global new to allocate (reserve) a chunk of memory on the heap.
*/
class StackAllocator : public MemoryAllocator
{
private:
    static Logger& logger;

public:
    virtual ~StackAllocator() throw();

public:
    virtual size_t total() const { return _size; }
    virtual size_t used() const { return _marker; }
    virtual size_t unused() const { return _size - _marker; }

    virtual void* allocate(size_t bytes);
    virtual void release(void* ptr);

    virtual void reset() { _marker = 0; }

private:
    friend class MemoryAllocator;
    explicit StackAllocator(size_t size);

private:
    boost::shared_array<unsigned char> _pool;
    uint32_t _size, _marker;

private:
    StackAllocator();
    DISALLOW_COPY_AND_ASSIGN(StackAllocator);
};

}

#endif
