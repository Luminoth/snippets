#include "src/pch.h"
#include "Capsule.h"

namespace energonsoftware {

Capsule::Capsule(const Point3& center, float radius, float height)
    : _center(center), _radius(std::fabs(radius)), _height(std::fabs(height))
{
}

Capsule::~Capsule() noexcept
{
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class CapsuleTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(CapsuleTest);
        CPPUNIT_TEST(test_initialize);
    CPPUNIT_TEST_SUITE_END();

public:
    CapsuleTest() : CppUnit::TestFixture() {}
    virtual ~CapsuleTest() noexcept {}

public:
    void test_initialize()
    {
        static const energonsoftware::Capsule c1;
        CPPUNIT_ASSERT(c1.center().is_zero());
        CPPUNIT_ASSERT_EQUAL(1.0f, c1.radius());
        CPPUNIT_ASSERT_EQUAL(1.0f, c1.height());

        static const energonsoftware::Point3 p2(235.62f, 26.62f, -235.325f);
        static const float r2 = 22345.235f;
        static const float h2 = 352.632f;
        static const energonsoftware::Capsule c2(p2, r2, h2);
        CPPUNIT_ASSERT_EQUAL(p2, c2.center());
        CPPUNIT_ASSERT_EQUAL(r2, c2.radius());
        CPPUNIT_ASSERT_EQUAL(h2, c2.height());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CapsuleTest);

#endif
