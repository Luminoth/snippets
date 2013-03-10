#include "src/pch.h"
#include "AABB.h"
#include "BoundingSphere.h"
#include "BoundingCapsule.h"

namespace energonsoftware {

BoundingCapsule::BoundingCapsule(const Capsule& capsule)
    : BoundingVolume(), _capsule(capsule)
{
}

BoundingCapsule::BoundingCapsule(const Point3& center, float radius, float height)
    : BoundingVolume(), _capsule(Capsule(center, radius, height))
{
}

BoundingCapsule::BoundingCapsule(const AABB& aabb)
    : BoundingVolume(), _capsule(aabb.center(), aabb.radius(), aabb.radius())
{
}

BoundingCapsule::~BoundingCapsule() noexcept
{
}

float BoundingCapsule::distance(const Intersectable& other) const
{
    /*if(typeid(other) == typeid(BoundingCapsule)) {
        return distance(dynamic_cast<const BoundingCapsule&>(other));
    } else if(typeid(other) == typeid(BoundingSphere)) {
        return distance(dynamic_cast<const BoundingSphere&>(other));
    } else if(typeid(other) == typeid(AABB)) {
        return distance(dynamic_cast<const AABB&>(other));
    }*/
    throw std::runtime_error(std::string("BoundingCapsule doesn't know how to intersect ") + typeid(other).name());
}

/*float BoundingCapsule::distance_squared(const Point3& p) const
{
    return _capsule.center().distance_squared(p) - std::pow(_capsule.radius(), 2);
}

float BoundingCapsule::distance(const Point3& p) const
{
    return _capsule.center().distance(p) - _capsule.radius();
}

float BoundingCapsule::distance_squared(const BoundingCapsule& c) const
{
    return _capsule.center().distance_squared(c._capsule.center()) - std::pow(_capsule.radius() + c._capsule.radius(), 2);
}

float BoundingCapsule::distance(const BoundingCapsule& c) const
{
    return _capsule.center().distance_squared(c._capsule.center()) - (_capsule.radius() + c._capsule.radius());
}

float BoundingCapsule::distance_squared(const BoundingSphere& s) const
{
    return _capsule.center().distance_squared(s.sphere().center()) - std::pow(_capsule.radius() + s.sphere().radius(), 2);
}

float BoundingCapsule::distance(const BoundingSphere& s) const
{
    return _capsule.center().distance(s.shere().center()) - (_capsule.radius() + s.sphere().radius());
}

float BoundingCapsule::distance_squared(const AABB& a) const
{
    return a.distance_squared(*this);
}

float BoundingCapsule::distance(const AABB& a) const
{
    return a.distance(*this);
}*/

std::string BoundingCapsule::str() const
{
    std::stringstream ss;
    ss << "BoundingCapsule(" << _capsule.center().str() << ", " << _capsule.radius() << ", " << _capsule.height() << ")";
    return ss.str();
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class BoundingCapsuleTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(BoundingCapsuleTest);
        CPPUNIT_TEST(test_initialize);
    CPPUNIT_TEST_SUITE_END();

public:
    BoundingCapsuleTest() : CppUnit::TestFixture() {}
    virtual ~BoundingCapsuleTest() noexcept {}

public:
    void test_initialize()
    {
        static const energonsoftware::BoundingCapsule bc1;
        CPPUNIT_ASSERT(bc1.center().is_zero());
        CPPUNIT_ASSERT_EQUAL(1.0f, bc1.radius());
        CPPUNIT_ASSERT_EQUAL(1.0f, bc1.height());

        static const energonsoftware::Capsule c2(energonsoftware::Point3(1324.35f, -234.235f, 23.53f), 12.0f, 10.12f);
        static const energonsoftware::BoundingCapsule bc2(c2);
        CPPUNIT_ASSERT_EQUAL(c2.center(), bc2.center());
        CPPUNIT_ASSERT_EQUAL(c2.radius(), bc2.radius());
        CPPUNIT_ASSERT_EQUAL(c2.height(), bc2.height());

        static const energonsoftware::Point3 min3(-15.52f, -235.35f, 143.235f);
        static const energonsoftware::Point3 max3(154.134f, -34.35f, 154.23f);
        static const energonsoftware::AABB a3(min3 + min3, max3 + max3);
        static const energonsoftware::BoundingCapsule bc3(a3);
        CPPUNIT_ASSERT_EQUAL(a3.center(), bc3.center());
        CPPUNIT_ASSERT_EQUAL(a3.radius(), bc3.radius());
        CPPUNIT_ASSERT_EQUAL(a3.radius(), bc3.height());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BoundingCapsuleTest);

#endif
