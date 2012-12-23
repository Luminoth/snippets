#if !defined __SYSTEMALLOCATOR_H__
#define __SYSTEMALLOCATOR_H__

#include "MemoryAllocator.h"

namespace energonsoftware {

class SystemAllocator : public MemoryAllocator
{
private:
    static Logger& logger;

public:
    virtual ~SystemAllocator() throw();

public:
    virtual size_t total() const { return _size; }
    virtual size_t used() const { return _used; }
    virtual size_t unused() const { return _size - _used; }

    virtual void* allocate(size_t bytes);
    virtual void release(void* ptr);

    virtual void reset() { _used = 0; }

private:
    friend class MemoryAllocator;
    explicit SystemAllocator(size_t size);

private:
    uint32_t _size, _used;

private:
    SystemAllocator();
    DISALLOW_COPY_AND_ASSIGN(SystemAllocator);
};

}

#endif
