#include "src/pch.h"
#include "StackAllocator.h"
#include "SystemAllocator.h"
#include "MemoryAllocator.h"

namespace energonsoftware {

#define ARRAY_OFFSET 0x10
//#define ARRAY_OFFSET 0x00

std::shared_ptr<MemoryAllocator> MemoryAllocator::new_allocator(Type type, size_t size)
{
    switch(type)
    {
    case Type::Stack:
        return std::shared_ptr<MemoryAllocator>(new StackAllocator(size));
    case Type::System:
        return std::shared_ptr<MemoryAllocator>(new SystemAllocator(size));
    }
    return std::shared_ptr<MemoryAllocator>();
}

MemoryAllocator::MemoryAllocator()
    : _mutex(), _allocation_count(0), _allocation_bytes(0)
{
}

MemoryAllocator::~MemoryAllocator() throw()
{
}

/*void* MemoryAllocator::allocate_array(size_t bytes)
{
    // need to allocate a little extra for systems
    // that store the size of the array in memory
    return allocate(bytes + ARRAY_OFFSET);
}

void MemoryAllocator::release_array(void* ptr)
{
    // for systems that store the size of the array
    // in memory, we need to back up over that to release
    release(reinterpret_cast<void*>(reinterpret_cast<size_t>(ptr) - ARRAY_OFFSET));
}*/

void* MemoryAllocator::allocate_aligned(size_t bytes, size_t alignment)
{
    // Game Engine Architecture 5.2.1.3
    assert(alignment > 1);

    // total memory (worst case)
    size_t expanded = bytes + alignment;

    // allocate unaligned block
    size_t address = reinterpret_cast<size_t>(allocate(expanded));

    // calculate adjustment by masking off the lower bits
    // of the address, to determine how "misaligned" it is
    size_t mask = alignment - 1;
    size_t misalignment = address & mask;

    // adjustment will always be at least 1 byte
    size_t adjustment = alignment - misalignment;

    // adjusted address
    size_t aligned = address + adjustment;

    // store the adjustment in the four bytes immediately
    // preceding the adjusted address that we're returning
    unsigned char* adjusted = reinterpret_cast<unsigned char*>(aligned - 4);
    *adjusted = adjustment;

    return reinterpret_cast<void*>(aligned);
}

/*void* MemoryAllocator::allocate_array_aligned(size_t bytes, size_t alignment)
{
    // need to allocate a little extra for systems
    // that store the size of the array in memory
    return allocate_aligned(bytes + ARRAY_OFFSET, alignment);
}*/

void MemoryAllocator::release_aligned(void* ptr, size_t alignement)
{
    // Game Engine Architecture 5.2.1.3
    size_t aligned = reinterpret_cast<size_t>(ptr);
    uint8_t* adjusted = reinterpret_cast<uint8_t*>(aligned - 4);
    size_t adjustment = static_cast<size_t>(*adjusted);
    size_t address = aligned - adjustment;
    release(reinterpret_cast<void*>(address));
}

/*void MemoryAllocator::release_array_aligned(void* ptr, size_t alignment)
{
    // for systems that store the size of the array
    // in memory, we need to back up over that to release
    release_aligned(reinterpret_cast<void*>(reinterpret_cast<size_t>(ptr) - ARRAY_OFFSET), alignment);
}*/

}

#if defined WITH_UNIT_TESTS
class TestObject
{
private:
    static const int DATA;
    static const float MOAR_DATA;
    static const std::string BUFFER;

public:
    TestObject()
        : _data(DATA), _moar_data(MOAR_DATA)
    {
    }

    virtual ~TestObject() throw() {}

public:
    void do_stuff()
    {
        ZeroMemory(_buffer, MAX_BUFFER);
        std::memcpy(_buffer, BUFFER.c_str(), BUFFER.length());

        on_do_stuff();
    }

    void check_data()
    {
        CPPUNIT_ASSERT_EQUAL(DATA, _data);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(MOAR_DATA, _moar_data, 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0, std::strncmp(_buffer, BUFFER.c_str(), BUFFER.length()));

        on_check_data();
    }

protected:
    virtual void on_do_stuff() = 0;
    virtual void on_check_data() const = 0;

private:
    int _data;
    float _moar_data;
    char _buffer[MAX_BUFFER];
};

const int TestObject::DATA = 100;
const float TestObject::MOAR_DATA = 150.35f;
const std::string TestObject::BUFFER("object test buffer");

class DerivedObject : public TestObject
{
public:
    static void destroy(DerivedObject* const obj, energonsoftware::MemoryAllocator* const allocator)
    {
        obj->~DerivedObject();
        operator delete(obj, *allocator);
    }

    static void destroy_aligned(DerivedObject* const obj, size_t alignment, energonsoftware::MemoryAllocator* const allocator)
    {
        obj->~DerivedObject();
        operator delete(obj, alignment, *allocator);
    }

    static DerivedObject* create_array(size_t count, energonsoftware::MemoryAllocator& allocator)
    {
        DerivedObject* objs = reinterpret_cast<DerivedObject*>(allocator.allocate(sizeof(DerivedObject) * count));

        DerivedObject *obj = objs, *end = objs + count;
        while(obj != end) {
            new(obj) DerivedObject();
            obj++;
        }

        return objs;
    }

    static DerivedObject* create_array_aligned(size_t count, size_t alignment, energonsoftware::MemoryAllocator& allocator)
    {
        DerivedObject* objs = reinterpret_cast<DerivedObject*>(allocator.allocate_aligned(sizeof(DerivedObject) * count, alignment));

        DerivedObject *obj = objs, *end = objs + count;
        while(obj != end) {
            new(obj) DerivedObject();
            obj++;
        }

        return objs;
    }

    static void destroy_array(DerivedObject* const objs, size_t count, energonsoftware::MemoryAllocator* const allocator)
    {
        DerivedObject* obj = objs + count;
        while(obj > objs) {
            (--obj)->~DerivedObject();
        }
        operator delete[](objs, *allocator);
    }

    static void destroy_array_aligned(DerivedObject* const objs, size_t count, size_t alignment, energonsoftware::MemoryAllocator* const allocator)
    {
        DerivedObject* obj = objs + count;
        while(obj > objs) {
            (--obj)->~DerivedObject();
        }
        operator delete[](objs, alignment, *allocator);
    }
public:
    DerivedObject()
        : TestObject(), _did_stuff(false)
    {
    }

    virtual ~DerivedObject() throw() {}

private:
    virtual void on_do_stuff() override
    {
        _did_stuff = true;
    }

    virtual void on_check_data() const override
    {
        CPPUNIT_ASSERT(_did_stuff);
    }

private:
    bool _did_stuff;
};

MemoryAllocatorTest::MemoryAllocatorTest(energonsoftware::MemoryAllocator::Type type)
    : _type(type), _allocator()
{
}

void MemoryAllocatorTest::setUp()
{
    // 5MB
    _allocator = energonsoftware::MemoryAllocator::new_allocator(_type, 5 * 1024 * 1024);
}

void MemoryAllocatorTest::tearDown()
{
    _allocator.reset();
}

void MemoryAllocatorTest::check_value(int* const value)
{
    static const int EXPECTED = 10;

    CPPUNIT_ASSERT(nullptr != value);
    if(nullptr != value) {
        *value = EXPECTED;
        CPPUNIT_ASSERT_EQUAL(EXPECTED, *value);
    }
}

void MemoryAllocatorTest::check_buffer(char* const buffer)
{
    static const std::string EXPECTED("this is a test string");

    CPPUNIT_ASSERT(nullptr != buffer);
    if(nullptr != buffer) {
        ZeroMemory(buffer, MAX_BUFFER);
        std::memcpy(buffer, EXPECTED.c_str(), EXPECTED.length());
        CPPUNIT_ASSERT_EQUAL(0, std::strncmp(buffer, EXPECTED.c_str(), EXPECTED.length()));
    }
}

void MemoryAllocatorTest::test_allocate()
{
    int* value = new(*_allocator) int;
    check_value(value);
    CPPUNIT_ASSERT_EQUAL(1U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(sizeof(int), _allocator->allocation_bytes());

    operator delete(value, *_allocator);
    value = nullptr;
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());

    char* buffer = new(*_allocator) char[MAX_BUFFER];
    check_buffer(buffer);
    CPPUNIT_ASSERT_EQUAL(1U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(MAX_BUFFER * sizeof(char), _allocator->allocation_bytes());

    operator delete[](buffer, *_allocator);
    buffer = nullptr;
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());
}

void MemoryAllocatorTest::test_allocate_object()
{
    std::shared_ptr<TestObject> obj(new(*_allocator) DerivedObject(),
        std::bind(&DerivedObject::destroy, std::placeholders::_1, _allocator.get()));
    CPPUNIT_ASSERT(obj);
    obj->do_stuff();
    obj->check_data();
    CPPUNIT_ASSERT_EQUAL(1U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(sizeof(DerivedObject), _allocator->allocation_bytes());

    obj.reset();
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());

    static const int OBJ_COUNT = 100;

    // TODO: not sure why this has to be an array of DerivedObjects and not TestObjects
    boost::shared_array<DerivedObject> objs(DerivedObject::create_array(OBJ_COUNT, *_allocator),
        std::bind(&DerivedObject::destroy_array, std::placeholders::_1, OBJ_COUNT, _allocator.get()));
    CPPUNIT_ASSERT(objs);
    for(int i=0; i<OBJ_COUNT; ++i) {
        objs[i].do_stuff();
        objs[i].check_data();
    }
    CPPUNIT_ASSERT_EQUAL(1U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(OBJ_COUNT * sizeof(DerivedObject), _allocator->allocation_bytes());

    objs.reset();
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());
}

void MemoryAllocatorTest::test_allocate_aligned()
{
    static const size_t ALIGNMENT = 4;

    int* value = new(ALIGNMENT, *_allocator) int;
    check_value(value);
    CPPUNIT_ASSERT_EQUAL(0U, reinterpret_cast<size_t>(value) % ALIGNMENT);

    operator delete(value, ALIGNMENT, *_allocator);
    value = nullptr;
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());

    char* buffer = new(ALIGNMENT, *_allocator) char[1024];
    check_buffer(buffer);
    CPPUNIT_ASSERT_EQUAL(0U, reinterpret_cast<size_t>(buffer) % ALIGNMENT);

    operator delete[](buffer, ALIGNMENT, *_allocator);
    buffer = nullptr;
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());
}

void MemoryAllocatorTest::test_allocate_object_aligned()
{
    static const size_t alignment = 4;

    std::shared_ptr<TestObject> obj(new(alignment, *_allocator) DerivedObject(),
        std::bind(&DerivedObject::destroy_aligned, std::placeholders::_1, alignment, _allocator.get()));
    CPPUNIT_ASSERT(obj);
    CPPUNIT_ASSERT_EQUAL(0U, reinterpret_cast<size_t>(obj.get()) % alignment);
    obj->do_stuff();
    obj->check_data();

    obj.reset();
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());

    static const int OBJ_COUNT = 100;

    // TODO: not sure why this has to be an array of DerivedObjects and not TestObjects
    boost::shared_array<DerivedObject> objs(DerivedObject::create_array_aligned(OBJ_COUNT, alignment, *_allocator),
        std::bind(&DerivedObject::destroy_array_aligned, std::placeholders::_1, OBJ_COUNT, alignment, _allocator.get()));
    CPPUNIT_ASSERT(objs);
    CPPUNIT_ASSERT_EQUAL(0U, reinterpret_cast<size_t>(objs.get()) % alignment);
    for(int i=0; i<OBJ_COUNT; ++i) {
        objs[i].do_stuff();
        objs[i].check_data();
    }

    objs.reset();
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());
}
#endif
