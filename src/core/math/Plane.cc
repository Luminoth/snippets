#include "src/pch.h"
#include "Plane.h"

namespace energonsoftware {

Plane::Plane(const Point3& p1, const Point3& p2, const Point3& p3)
{
    const Vector3 q1(p2 - p1), q2(p3 - p1);
    const Vector3 n((q1 ^ q2).normalize());
    _plane = Vector4(n, -n * p1);
}

}

#ifdef WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class PlaneTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(PlaneTest);
        CPPUNIT_TEST(test_initialize);
        CPPUNIT_TEST(test_distance);
        CPPUNIT_TEST(test_coplanar);
    CPPUNIT_TEST_SUITE_END();

public:
    PlaneTest() : CppUnit::TestFixture() {}
    virtual ~PlaneTest() throw() {}

public:
    void test_initialize()
    {
        /*static const energonsoftware::Plane p1;
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, p1.normal().length(), 0.0001f);*/

        static const float d2 = 643.236f;
        energonsoftware::Normal3 n2(34.53f, 356.35f, 366.035f);
        n2.normalize();
        static const energonsoftware::Plane p2(n2, d2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, p2.normal().length(), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(d2, p2.distance());

// TODO: test point constructor
    }

    void test_distance()
    {
    }

    void test_coplanar()
    {
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PlaneTest);

#endif