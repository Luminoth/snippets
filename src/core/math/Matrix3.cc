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
    CPPUNIT_TEST_SUITE_END();

public:
    Matrix3Test() : CppUnit::TestFixture() {}
    virtual ~Matrix3Test() throw() {}
};

#endif