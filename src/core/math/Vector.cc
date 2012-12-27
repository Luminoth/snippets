#include "src/pch.h"
#include "Vector.h"

namespace energonsoftware {

void Vector::destroy(Vector* const vector, MemoryAllocator* const allocator)
{
    vector->~Vector();
    operator delete(vector, 16, *allocator);
}

Vector* Vector::create_array(size_t count, MemoryAllocator& allocator)
{
    Vector* vectors = reinterpret_cast<Vector*>(allocator.allocate(sizeof(Vector) * count, 16));

    Vector *vector = vectors, *end = vectors + count;
    while(vector != end) {
        new(vector) Vector();
        vector++;
    }

    return vectors;
}

void Vector::destroy_array(Vector* const vectors, size_t count, MemoryAllocator* const allocator)
{
    Vector* vector = vectors + count;
    while(vector > vectors) {
        (--vector)->~Vector();
    }
    operator delete[](vectors, 16, *allocator);
}

std::string Vector::str() const
{
    std::stringstream ss;
    ss << "Vector(x:" << std::fixed << x() << ", y:" << y() << ", z:" << z() << ", w:" << w() << ")";
    return ss.str();
}

}

#ifdef WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class VectorTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(VectorTest);
        CPPUNIT_TEST(test_allocate);
        CPPUNIT_TEST(test_initialize);
        CPPUNIT_TEST(test_zero);
        CPPUNIT_TEST(test_components);
        CPPUNIT_TEST(test_length);
        CPPUNIT_TEST(test_normalize);
        CPPUNIT_TEST(test_perpendicular);
        CPPUNIT_TEST(test_direction);
        CPPUNIT_TEST(test_normal);
        CPPUNIT_TEST(test_distance);
        CPPUNIT_TEST(test_angle);
        CPPUNIT_TEST(test_lerp);
        CPPUNIT_TEST(test_multiply);
        CPPUNIT_TEST(test_homogeneous);
// TODO: test other operators
        CPPUNIT_TEST(test_dot);
        CPPUNIT_TEST(test_cross);
    CPPUNIT_TEST_SUITE_END();

public:
    VectorTest() : CppUnit::TestFixture() {}
    virtual ~VectorTest() throw() {}

public:
    void test_allocate()
    {
        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(energonsoftware::MemoryAllocator::new_allocator(energonsoftware::AllocatorType::System, 10 * 1024));
        std::shared_ptr<energonsoftware::Vector> v1(new(16, *allocator) energonsoftware::Vector(10.0f, 123.123f, 25.75f),
            std::bind(energonsoftware::Vector::destroy, std::placeholders::_1, allocator.get()));
        CPPUNIT_ASSERT(v1);
        CPPUNIT_ASSERT_EQUAL(10.0f, v1->x());
        CPPUNIT_ASSERT_EQUAL(123.123f, v1->y());
        CPPUNIT_ASSERT_EQUAL(25.75f, v1->z());

        v1.reset();

        static const size_t COUNT = 100;
        boost::shared_array<energonsoftware::Vector> vectors(energonsoftware::Vector::create_array(COUNT, *allocator),
            std::bind(energonsoftware::Vector::destroy_array, std::placeholders::_1, COUNT, allocator.get()));
        CPPUNIT_ASSERT(vectors);
        for(size_t i=0; i<COUNT; ++i) {
            CPPUNIT_ASSERT(vectors[i].is_zero());
        }
        vectors.reset();
    }

    void test_initialize()
    {
#if !defined _MSC_VER || (defined _MSC_VER && _MSC_VER > 1700)
        static const energonsoftware::Vector v1({ 1.0f, 2.0f, 3.0f, 4.0f });
        CPPUNIT_ASSERT_EQUAL(1.0f, v1.x());
        CPPUNIT_ASSERT_EQUAL(2.0f, v1.y());
        CPPUNIT_ASSERT_EQUAL(3.0f, v1.z());
        CPPUNIT_ASSERT_EQUAL(4.0f, v1.w());
#endif

        static const float V2_VECTOR[] = { 1.0f, 2.0f, 3.0f, 4.0f };
        static const energonsoftware::Vector v2(V2_VECTOR);
        CPPUNIT_ASSERT_EQUAL(1.0f, v2.x());
        CPPUNIT_ASSERT_EQUAL(2.0f, v2.y());
        CPPUNIT_ASSERT_EQUAL(3.0f, v2.z());
        CPPUNIT_ASSERT_EQUAL(4.0f, v2.w());
    }

    void test_zero()
    {
        static const energonsoftware::Vector v1(0.0f, 0.0f, 0.0f);
        CPPUNIT_ASSERT(v1.is_zero());

        static const energonsoftware::Vector v2(0.0f, 1.0f, 0.0f);
        CPPUNIT_ASSERT(!v2.is_zero());

        energonsoftware::Vector v3(1.0f, 2.0f, 3.0f);
        v3.zero();
        CPPUNIT_ASSERT(v3.is_zero());
    }

    void test_components()
    {
        static const float VALUES[] = { 10.0f, 12.0f, 145.0f, 14.0f };

        energonsoftware::Vector v1(1.0f, 2.0f, 3.0f);
        v1.x(VALUES[0]);
        CPPUNIT_ASSERT_EQUAL(VALUES[0], v1.x());

        v1.y(VALUES[1]);
        CPPUNIT_ASSERT_EQUAL(VALUES[1], v1.y());

        v1.z(VALUES[2]);
        CPPUNIT_ASSERT_EQUAL(VALUES[2], v1.z());

        v1.w(VALUES[3]);
        CPPUNIT_ASSERT_EQUAL(VALUES[3], v1.w());

        CPPUNIT_ASSERT_EQUAL(0, std::memcmp(VALUES, v1.array(), sizeof(VALUES) / sizeof(VALUES[0])));

        // TODO: test xy, xz, yz, and xyz
    }

    void test_length()
    {
        static const energonsoftware::Vector v1(3.0f, 4.0f, 12.0f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0f, v1.length(), 0.001f);

        static const energonsoftware::Vector v2(17.0f, 3.13f, 22.75f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(816.3594f, v2.length_squared(), 0.001f);
    }

    void test_normalize()
    {
        energonsoftware::Vector v1(17.0f, 88.5f, 127.25f);
        v1.normalize();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.109f, v1.x(), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.567f, v1.y(), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.816f, v1.z(), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, v1.length(), 0.001f);

        static const energonsoftware::Vector v2(3.0f, 4.0f, 5.77f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(7.635f, v2.length(), 0.001f);

        const energonsoftware::Vector n(v2.normalized());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.392f, n.x(), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.524f, n.y(), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.756f, n.z(), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, n.length(), 0.001f);
    }

    void test_perpendicular()
    {
        static const energonsoftware::Vector v1(1.0f, 0.0f, 0.0f);
        static const energonsoftware::Vector v2(0.0f, 100.0f, 0.0f);

        CPPUNIT_ASSERT(v1.perpendicular(v2));
    }

    void test_direction()
    {
        static const energonsoftware::Vector v1(500.0f, 0.0f, 0.0f);
        static const energonsoftware::Vector v2(200.0f, 0.0f, 0.0f);
        CPPUNIT_ASSERT(v1.same_direction(v2));
        CPPUNIT_ASSERT(!v2.opposite_direction(v1));

        static const energonsoftware::Vector v3(0.0f, 100.0f, 0.0f);
        CPPUNIT_ASSERT(!v3.same_direction(v1));

        static const energonsoftware::Vector v4(0.0f, -50.0f, 0.0f);
        CPPUNIT_ASSERT(v4.opposite_direction(v3));
    }

    void test_normal()
    {
        static const energonsoftware::Vector v1(10.50f, -20.0f, 11.1f, 35.2f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(76.8f, v1.manhattan_normal(), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(35.2, v1.infinite_normal(), 0.001f);
    }

    void test_distance()
    {
        static const energonsoftware::Vector v1(1.0f, 3.0f, 0.0f);
        static const energonsoftware::Vector v2(1.0f, 9.0f, 0.0f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, v1.distance(v2), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0f, v2.distance(v1), 0.001f);

        static const energonsoftware::Vector v3(1.0f, 3.0f, 0.0f);
        static const energonsoftware::Vector v4(1.0f, 9.0f, 0.0f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(36.0f, v3.distance_squared(v4), 0.001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(36.0f, v4.distance_squared(v3), 0.001f);
    }

    void test_angle()
    {
    }

    void test_lerp()
    {
    }

    void test_multiply()
    {
        static const energonsoftware::Vector v1(2.0f, 3.0f, 4.0f, 5.0f);
        static const energonsoftware::Vector v2(10.0f, 20.0f, 30.0f, 40.0f);
        static const energonsoftware::Vector EXPECTED(20.0f, 60.0f, 120.0f, 200.0f);
        energonsoftware::Vector v3(v1.multiply(v2));
        CPPUNIT_ASSERT_EQUAL(EXPECTED.x(), v3.x());
        CPPUNIT_ASSERT_EQUAL(EXPECTED.y(), v3.y());
        CPPUNIT_ASSERT_EQUAL(EXPECTED.z(), v3.z());
        CPPUNIT_ASSERT_EQUAL(EXPECTED.w(), v3.w());
    }

    void test_homogeneous()
    {
    }

    void test_dot()
    {
        static const energonsoftware::Vector v1(1.0f, 3.0f, 6.0f);
        static const energonsoftware::Vector v2(4.0f, -2.0f, 1.0f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0f, v1 * v2, 0.001f);
// TODO: test *=
    }

    void test_cross()
    {
        static const energonsoftware::Vector expected(0.0f, 0.0f, 1.0f);

        static const energonsoftware::Vector v1(1.0f, 0.0f, 0.0f);
        static const energonsoftware::Vector v2(0.0f, 1.0f, 0.0f);
        //CPPUNIT_ASSERT_EQUAL(expected, v1 ^ v2);
        CPPUNIT_ASSERT(expected == (v1 ^ v2));
// TODO: test ^=
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(VectorTest);

#endif