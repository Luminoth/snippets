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
    virtual ~StackAllocator() noexcept;

public:
    virtual size_t total() const override { return _size; }
    virtual size_t used() const override { return _marker; }
    virtual size_t unused() const override { return _size - _marker; }

    virtual void* allocate(size_t bytes) override;
    virtual void release(void* ptr) override;

    virtual void reset() override { _marker = 0; }

private:
    friend class MemoryAllocator;
    explicit StackAllocator(size_t size);

private:
    std::unique_ptr<unsigned char[]> _pool;
    uint32_t _size, _marker;

private:
    StackAllocator() = delete;
    DISALLOW_COPY_AND_ASSIGN(StackAllocator);
};

}

#endif
