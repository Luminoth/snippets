#include "src/pch.h"
#include "Sphere.h"

namespace energonsoftware {

Sphere::Sphere(const Point3& center, float radius)
    : _center(center), _radius(std::fabs(radius))
{
}

Sphere::~Sphere() throw()
{
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class SphereTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(SphereTest);
        CPPUNIT_TEST(test_initialize);
    CPPUNIT_TEST_SUITE_END();

public:
    SphereTest() : CppUnit::TestFixture() {}
    virtual ~SphereTest() throw() {}

public:
    void test_initialize()
    {
        static const energonsoftware::Sphere s1;
        CPPUNIT_ASSERT(s1.center().is_zero());
        CPPUNIT_ASSERT_EQUAL(1.0f, s1.radius());

        static const energonsoftware::Point3 p2(235.62f, 26.62f, -235.325f);
        static const float r2 = 22345.235f;
        static const energonsoftware::Sphere s2(p2, r2);
        CPPUNIT_ASSERT(s2.center() == p2);
        CPPUNIT_ASSERT_EQUAL(r2, s2.radius());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SphereTest);

#endif
