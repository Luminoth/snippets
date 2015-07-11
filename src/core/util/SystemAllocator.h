#if !defined __SYSTEMALLOCATOR_H__
#define __SYSTEMALLOCATOR_H__

#include "MemoryAllocator.h"

namespace energonsoftware {

class SystemAllocator : public MemoryAllocator
{
private:
    static Logger& logger;

public:
    virtual ~SystemAllocator() noexcept;

public:
    virtual size_t total() const override { return _size; }
    virtual size_t used() const override { return _used; }
    virtual size_t unused() const override { return _size - _used; }

    virtual void* allocate(size_t bytes) override;
    virtual void release(void* ptr) override;

    virtual void reset() override { _used = 0; }

private:
    friend class MemoryAllocator;
    explicit SystemAllocator(size_t size);

private:
    uint32_t _size, _used;

private:
    SystemAllocator() = delete;
    DISALLOW_COPY_AND_ASSIGN(SystemAllocator);
};

}

#endif
