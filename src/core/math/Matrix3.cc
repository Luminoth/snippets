#include "src/pch.h"
#include "math_util.h"
#include "Matrix3.h"

namespace energonsoftware {

std::string Matrix3::str() const
{
    std::stringstream ss;
    ss << _m[0] << " " << _m[1] << " " << _m[2] << "\n"
       << _m[3] << " " << _m[4] << " " << _m[5] << "\n"
       << _m[6] << " " << _m[7] << " " << _m[8];
    return ss.str();
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class Matrix3Test : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(Matrix3Test);
        CPPUNIT_TEST(test_allocation);
        CPPUNIT_TEST(test_initialize);
        CPPUNIT_TEST(test_row);
        CPPUNIT_TEST(test_determinant);
        CPPUNIT_TEST(test_addition);
        CPPUNIT_TEST(test_subtraction);
        CPPUNIT_TEST(test_multiply);
        CPPUNIT_TEST(test_vector_multiply);
        CPPUNIT_TEST(test_scalar_multiply);
        CPPUNIT_TEST(test_scalar_divide);
        CPPUNIT_TEST(test_equality);
        CPPUNIT_TEST(test_inverse);
        CPPUNIT_TEST(test_transposition);
    CPPUNIT_TEST_SUITE_END();

public:
    Matrix3Test() : CppUnit::TestFixture() {}
    virtual ~Matrix3Test() noexcept {}

public:
    void test_allocation()
    {
        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, 10 * 1024));
        std::shared_ptr<energonsoftware::Matrix3> m1(new(16, *allocator) energonsoftware::Matrix3(),
            energonsoftware::MemoryAllocator_delete_aligned<energonsoftware::Matrix3, 16>(allocator.get()));
        CPPUNIT_ASSERT(m1);
        CPPUNIT_ASSERT(m1->is_identity());

        m1.reset();
    }

    void test_initialize()
    {
    }

    void test_row()
    {
        static const energonsoftware::Matrix3 m1;
        energonsoftware::Vector r1 = m1.row(1);
        CPPUNIT_ASSERT_EQUAL(0.0f, r1.x());
        CPPUNIT_ASSERT_EQUAL(1.0f, r1.y());
        CPPUNIT_ASSERT_EQUAL(0.0f, r1.z());
        CPPUNIT_ASSERT_EQUAL(0.0f, r1.w());
    }

    void test_determinant()
    {
        static const energonsoftware::Matrix3 m1;
        CPPUNIT_ASSERT_EQUAL(1.0f, m1.determinant());

#if !defined _MSC_VER || (defined _MSC_VER && _MSC_VER > 1700)
        static const energonsoftware::Matrix3 m2({
            1.0f, 2.0f, 3.0f,
            4.0f, 5.0f, 4.0f,
            3.0f, 2.0f, 1.0f
        });
        CPPUNIT_ASSERT_EQUAL(-8.0f, m2.determinant());
#endif

        static const float M3_MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
        static const energonsoftware::Matrix3 m3(M3_MATRIX);
        CPPUNIT_ASSERT_EQUAL(-8.0f, m3.determinant());
    }

    void test_addition()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };

        energonsoftware::Matrix3 m1 = energonsoftware::Matrix3(MATRIX) + energonsoftware::Matrix3(MATRIX);
        CPPUNIT_ASSERT_EQUAL(2.0f, m1(0, 0));
        CPPUNIT_ASSERT_EQUAL(4.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(6.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(8.0f, m1(1, 0));
        CPPUNIT_ASSERT_EQUAL(10.0f, m1(1, 1));
        CPPUNIT_ASSERT_EQUAL(12.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(14.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(16.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(18.0f, m1(2, 2));

// TODO: test +=
    }

    void test_subtraction()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };

        energonsoftware::Matrix3 m1 = energonsoftware::Matrix3(MATRIX) - energonsoftware::Matrix3(MATRIX);
        CPPUNIT_ASSERT(m1.is_zero());

// TODO: test -=
    }

    void test_multiply()
    {
    }

    void test_vector_multiply()
    {
    }

    void test_scalar_multiply()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };

        energonsoftware::Matrix3 m1 = 2.0f * energonsoftware::Matrix3(MATRIX);
        CPPUNIT_ASSERT_EQUAL(2.0f, m1(0, 0));
        CPPUNIT_ASSERT_EQUAL(4.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(6.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(8.0f, m1(1, 0));
        CPPUNIT_ASSERT_EQUAL(10.0f, m1(1, 1));
        CPPUNIT_ASSERT_EQUAL(12.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(14.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(16.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(18.0f, m1(2, 2));

// TODO: test *=
    }

    void test_scalar_divide()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };

        energonsoftware::Matrix3 m1 = energonsoftware::Matrix3(MATRIX) / 2.0f;
        CPPUNIT_ASSERT_EQUAL(0.5f, m1(0, 0));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(1.5f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(2.0f, m1(1, 0));
        CPPUNIT_ASSERT_EQUAL(2.5f, m1(1, 1));
        CPPUNIT_ASSERT_EQUAL(3.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(3.5f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(4.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(4.5f, m1(2, 2));

// TODO: test /=
    }

    void test_equality()
    {
        static const float MATRIX1[] = { 1.0f, 2.0f, 3.0f, 4.0, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };
        static const float MATRIX2[] = { 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
        CPPUNIT_ASSERT_EQUAL(energonsoftware::Matrix3(MATRIX1), energonsoftware::Matrix3(MATRIX1));
        CPPUNIT_ASSERT(energonsoftware::Matrix3(MATRIX1) != energonsoftware::Matrix3(MATRIX2));
    }

    void test_inverse()
    {
    }

    void test_transposition()
    {
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Matrix3Test);

#endif
