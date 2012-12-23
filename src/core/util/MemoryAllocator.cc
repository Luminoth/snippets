#include "src/pch.h"
#include "StackAllocator.h"
#include "SystemAllocator.h"
#include "MemoryAllocator.h"

namespace energonsoftware {

#define ARRAY_OFFSET 0x10
//#define ARRAY_OFFSET 0x00

std::shared_ptr<MemoryAllocator> MemoryAllocator::new_allocator(AllocatorType type, size_t size)
{
    switch(type)
    {
    case AllocatorType::Stack:
        return std::shared_ptr<MemoryAllocator>(new StackAllocator(size));
    case AllocatorType::System:
        return std::shared_ptr<MemoryAllocator>(new SystemAllocator(size));
    }
    return std::shared_ptr<MemoryAllocator>();
}

MemoryAllocator::MemoryAllocator()
    : _allocation_count(0), _allocation_bytes(0)
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

void* MemoryAllocator::allocate(size_t bytes, size_t alignment)
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

/*void* MemoryAllocator::allocate_array(size_t bytes, size_t alignment)
{
    // need to allocate a little extra for systems
    // that store the size of the array in memory
    return allocate(bytes + ARRAY_OFFSET, alignment);
}*/

void MemoryAllocator::release(void* ptr, size_t alignement)
{
    // Game Engine Architecture 5.2.1.3
    size_t aligned = reinterpret_cast<size_t>(ptr);
    uint8_t* adjusted = reinterpret_cast<uint8_t*>(aligned - 4);
    size_t adjustment = static_cast<size_t>(*adjusted);
    size_t address = aligned - adjustment;
    release(reinterpret_cast<void*>(address));
}

/*void MemoryAllocator::release_array(void* ptr, size_t alignment)
{
    // for systems that store the size of the array
    // in memory, we need to back up over that to release
    release(reinterpret_cast<void*>(reinterpret_cast<size_t>(ptr) - ARRAY_OFFSET), alignment);
}*/

}

#if defined WITH_UNIT_TESTS
class TestObject
{
public:
    static void destroy(TestObject* const obj, energonsoftware::MemoryAllocator* const allocator)
    {
        obj->~TestObject();
        operator delete(obj, *allocator);
    }

    static void destroy_aligned(TestObject* const obj, size_t alignment, energonsoftware::MemoryAllocator* const allocator)
    {
        obj->~TestObject();
        operator delete(obj, alignment, *allocator);
    }

    static TestObject* create_array(size_t count, energonsoftware::MemoryAllocator& allocator)
    {
        TestObject* objs = reinterpret_cast<TestObject*>(allocator.allocate(sizeof(TestObject) * count));

        TestObject *obj = objs, *end = objs + count;
        while(obj != end) {
            new(obj) TestObject();
            obj++;
        }

        return objs;
    }

    static TestObject* create_array_aligned(size_t count, size_t alignment, energonsoftware::MemoryAllocator& allocator)
    {
        TestObject* objs = reinterpret_cast<TestObject*>(allocator.allocate(sizeof(TestObject) * count, alignment));

        TestObject *obj = objs, *end = objs + count;
        while(obj != end) {
            new(obj) TestObject();
            obj++;
        }

        return objs;
    }

    static void destroy_array(TestObject* const objs, size_t count, energonsoftware::MemoryAllocator* const allocator)
    {
        TestObject* obj = objs + count;
        while(obj > objs) {
            (--obj)->~TestObject();
        }
        operator delete[](objs, *allocator);
    }

    static void destroy_array_aligned(TestObject* const objs, size_t count, size_t alignment, energonsoftware::MemoryAllocator* const allocator)
    {
        TestObject* obj = objs + count;
        while(obj > objs) {
            (--obj)->~TestObject();
        }
        operator delete[](objs, alignment, *allocator);
    }

private:
    static const int DATA;
    static const float MOAR_DATA;
    static const std::string BUFFER;

public:
    TestObject()
        : _data(DATA), _moar_data(MOAR_DATA)
    {
        ZeroMemory(_buffer, MAX_BUFFER);
        std::memcpy(_buffer, BUFFER.c_str(), BUFFER.length());
    }

    virtual ~TestObject() throw() {}

public:
    void check_data()
    {
        CPPUNIT_ASSERT_EQUAL(DATA, _data);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(MOAR_DATA, _moar_data, 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0, std::strncmp(_buffer, BUFFER.c_str(), BUFFER.length()));
    }

private:
    int _data;
    float _moar_data;
    char _buffer[MAX_BUFFER];
};

const int TestObject::DATA = 100;
const float TestObject::MOAR_DATA = 150.35f;
const std::string TestObject::BUFFER("object test buffer");

MemoryAllocatorTest::MemoryAllocatorTest(energonsoftware::AllocatorType type)
    : _type(type)
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

    *value = EXPECTED;
    CPPUNIT_ASSERT_EQUAL(EXPECTED, *value);
}

void MemoryAllocatorTest::check_buffer(char* const buffer)
{
    static const std::string EXPECTED("this is a test string");

    CPPUNIT_ASSERT(nullptr != buffer);

    ZeroMemory(buffer, MAX_BUFFER);
    std::memcpy(buffer, EXPECTED.c_str(), EXPECTED.length());
    CPPUNIT_ASSERT_EQUAL(0, std::strncmp(buffer, EXPECTED.c_str(), EXPECTED.length()));
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
    std::shared_ptr<TestObject> obj(new(*_allocator) TestObject(),
        std::bind(&TestObject::destroy, std::placeholders::_1, _allocator.get()));
    CPPUNIT_ASSERT(obj);
    obj->check_data();
    CPPUNIT_ASSERT_EQUAL(1U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(sizeof(TestObject), _allocator->allocation_bytes());

    obj.reset();
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());

    static const int OBJ_COUNT = 100;

    boost::shared_array<TestObject> objs(TestObject::create_array(OBJ_COUNT, *_allocator),
        std::bind(&TestObject::destroy_array, std::placeholders::_1, OBJ_COUNT, _allocator.get()));
    CPPUNIT_ASSERT(objs);
    for(int i=0; i<OBJ_COUNT; ++i) {
        objs[i].check_data();
    }
    CPPUNIT_ASSERT_EQUAL(1U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(OBJ_COUNT * sizeof(TestObject), _allocator->allocation_bytes());

    objs.reset();
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());
}

void MemoryAllocatorTest::test_allocate_aligned()
{
    static const size_t ALIGNMENT = 4;

    int* value = new(ALIGNMENT, *_allocator) int;
    check_value(value);
    CPPUNIT_ASSERT(0 == (reinterpret_cast<size_t>(value) % ALIGNMENT));

    operator delete(value, ALIGNMENT, *_allocator);
    value = nullptr;
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());

    char* buffer = new(ALIGNMENT, *_allocator) char[1024];
    check_buffer(buffer);
    CPPUNIT_ASSERT(0 == (reinterpret_cast<size_t>(buffer) % ALIGNMENT));

    operator delete[](buffer, ALIGNMENT, *_allocator);
    buffer = nullptr;
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());
}

void MemoryAllocatorTest::test_allocate_object_aligned()
{
    static const size_t alignment = 4;

    std::shared_ptr<TestObject> obj(new(alignment, *_allocator) TestObject(),
        std::bind(&TestObject::destroy_aligned, std::placeholders::_1, alignment, _allocator.get()));
    CPPUNIT_ASSERT(obj);
    CPPUNIT_ASSERT(0 == (reinterpret_cast<size_t>(obj.get()) % alignment));
    obj->check_data();

    obj.reset();
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());

    static const int OBJ_COUNT = 100;

    boost::shared_array<TestObject> objs(TestObject::create_array_aligned(OBJ_COUNT, alignment, *_allocator),
        std::bind(&TestObject::destroy_array_aligned, std::placeholders::_1, OBJ_COUNT, alignment, _allocator.get()));
    CPPUNIT_ASSERT(objs);
    CPPUNIT_ASSERT(0 == (reinterpret_cast<size_t>(objs.get()) % alignment));
    for(int i=0; i<OBJ_COUNT; ++i) {
        objs[i].check_data();
    }

    objs.reset();
    CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_count());
    //CPPUNIT_ASSERT_EQUAL(0U, _allocator->allocation_bytes());
}
#endif
