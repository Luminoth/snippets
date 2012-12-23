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

#ifdef WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class Matrix3Test : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(Matrix3Test);
        CPPUNIT_TEST(test_allocate);
        CPPUNIT_TEST(test_determinant);
    CPPUNIT_TEST_SUITE_END();

public:
    Matrix3Test() : CppUnit::TestFixture() {}
    virtual ~Matrix3Test() throw() {}

public:
    void test_allocate()
    {
        energonsoftware::Matrix3 m;
        CPPUNIT_ASSERT(m.is_identity());
    }

    void test_determinant()
    {
        energonsoftware::Matrix3 m1;
        CPPUNIT_ASSERT_EQUAL(0.0f, m1.determinant());

#if defined _MSC_VER && _MSC_VER > 1700
        energonsoftware::Matrix3 m2({
            1.0f, 2.0f, 3.0f,
            4.0f, 5.0f, 4.0f,
            3.0f, 2.0f, 1.0f
        });
        CPPUNIT_ASSERT_EQUAL(-8.0f, m2.determinant());
#endif

        static const float M3_MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
        energonsoftware::Matrix3 m3(M3_MATRIX);
        CPPUNIT_ASSERT_EQUAL(-8.0f, m3.determinant());
    }
};

#endif