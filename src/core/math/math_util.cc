#include "src/pch.h"
#include "math_util.h"

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class MathTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(MathTest);
        CPPUNIT_TEST(test_is_odd);
        CPPUNIT_TEST(test_is_even);
        CPPUNIT_TEST(test_rad_deg);
        CPPUNIT_TEST(test_deg_rad);
        CPPUNIT_TEST(test_useful_degrees);
        CPPUNIT_TEST(test_min_max);
        CPPUNIT_TEST(test_manhattan_distance);
        CPPUNIT_TEST(test_ilog2);
        CPPUNIT_TEST(test_power_of_2);
        CPPUNIT_TEST(test_invsqrt);
    CPPUNIT_TEST_SUITE_END();

public:
    MathTest() : CppUnit::TestFixture() {}
    virtual ~MathTest() noexcept {}

public:
    void test_is_odd()
    {
        CPPUNIT_ASSERT(IS_ODD(5));
        CPPUNIT_ASSERT(!IS_ODD(4));
        CPPUNIT_ASSERT(!IS_ODD(0));
    }

    void test_is_even()
    {
        CPPUNIT_ASSERT(IS_EVEN(4));
        CPPUNIT_ASSERT(!IS_EVEN(5));
        CPPUNIT_ASSERT(IS_EVEN(0));
    }

    void test_rad_deg()
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2864.78898, RAD_DEG(50), 0.00001);
    }

    void test_deg_rad()
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.872665, DEG_RAD(50), 0.000001);
    }

    void test_useful_degrees()
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6.283185, RAD_360, 0.000001);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.047198, RAD_60, 0.000001);
    }

    void test_min_max()
    {
        CPPUNIT_ASSERT_EQUAL(15, MIN(25, 15));
        CPPUNIT_ASSERT_EQUAL(25, MAX(15, 25));
    }

    void test_manhattan_distance()
    {
        CPPUNIT_ASSERT_EQUAL(3555L, energonsoftware::manhattan_distance(123, 3456, 245, 23));
        CPPUNIT_ASSERT_EQUAL(0L, energonsoftware::manhattan_distance(145, 234, 145, 234));
    }

    void test_ilog2()
    {
        CPPUNIT_ASSERT_EQUAL(0U, energonsoftware::ilog2(1));
        CPPUNIT_ASSERT_EQUAL(1U, energonsoftware::ilog2(2));
        CPPUNIT_ASSERT_EQUAL(2U, energonsoftware::ilog2(4));
        CPPUNIT_ASSERT_EQUAL(3U, energonsoftware::ilog2(10));
    }

    void test_power_of_2()
    {
        CPPUNIT_ASSERT_EQUAL(0U, energonsoftware::power_of_2(0));
        CPPUNIT_ASSERT_EQUAL(1U, energonsoftware::power_of_2(1));
        CPPUNIT_ASSERT_EQUAL(2U, energonsoftware::power_of_2(2));
        CPPUNIT_ASSERT_EQUAL(4U, energonsoftware::power_of_2(3));
        CPPUNIT_ASSERT_EQUAL(256U, energonsoftware::power_of_2(200));
        CPPUNIT_ASSERT_EQUAL(2048U, energonsoftware::power_of_2(2000));
    }

    void test_invsqrt()
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f / std::sqrt(10.0f), energonsoftware::invsqrt(10.0f), 0.0001);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f / std::sqrt(1234.0f), energonsoftware::invsqrt(1234.0f), 0.0001);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f / std::sqrt(5000.0f), energonsoftware::invsqrt(5000.0f), 0.0001);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MathTest);

#endif
