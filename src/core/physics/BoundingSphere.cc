#include "src/pch.h"
#include "AABB.h"
#include "BoundingSphere.h"

namespace energonsoftware {

BoundingSphere::BoundingSphere(const Sphere& sphere)
    : BoundingVolume(), _sphere(sphere)
{
}

BoundingSphere::BoundingSphere(const Point3& center, float radius)
    : BoundingVolume(), _sphere(Sphere(center, radius))
{
}

BoundingSphere::BoundingSphere(const AABB& aabb)
    : BoundingVolume(), _sphere(aabb.center(), aabb.radius())
{
}

BoundingSphere::~BoundingSphere() throw()
{
}

float BoundingSphere::distance(const Intersectable& other) const
{
    if(typeid(other) == typeid(BoundingSphere)) {
        return distance(dynamic_cast<const BoundingSphere&>(other));
    } else if(typeid(other) == typeid(AABB)) {
        return distance(dynamic_cast<const AABB&>(other));
    }
    throw std::runtime_error(std::string("BoundingSphere doesn't know how to intersect ") + typeid(other).name());
}

float BoundingSphere::distance_squared(const Point3& p) const
{
    return _sphere.center().distance_squared(p) - std::pow(_sphere.radius(), 2);
}

float BoundingSphere::distance(const Point3& p) const
{
    return _sphere.center().distance(p) - _sphere.radius();
}

float BoundingSphere::distance_squared(const BoundingSphere& s) const
{
    return _sphere.center().distance_squared(s._sphere.center()) - std::pow(_sphere.radius() + s._sphere.radius(), 2);
}

float BoundingSphere::distance(const BoundingSphere& s) const
{
    return _sphere.center().distance(s._sphere.center()) - (_sphere.radius() + s._sphere.radius());
}

float BoundingSphere::distance_squared(const AABB& a) const
{
    return a.distance_squared(*this);
}

float BoundingSphere::distance(const AABB& a) const
{
    return a.distance(*this);
}

std::string BoundingSphere::str() const
{
    std::stringstream ss;
    ss << "BoundingSphere(" << _sphere.center().str() << ", " << _sphere.radius() << ")";
    return ss.str();
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class BoundingSphereTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(BoundingSphereTest);
        CPPUNIT_TEST(test_initialize);
        CPPUNIT_TEST(test_distance);
    CPPUNIT_TEST_SUITE_END();

public:
    BoundingSphereTest() : CppUnit::TestFixture() {}
    virtual ~BoundingSphereTest() throw() {}

public:
    void test_initialize()
    {
        static const energonsoftware::BoundingSphere bs1;
        CPPUNIT_ASSERT(bs1.center().is_zero());
        CPPUNIT_ASSERT_EQUAL(1.0f, bs1.radius());

        static const energonsoftware::Sphere s2(energonsoftware::Point3(1324.35f, -234.235f, 23.53f), 12.0f);
        static const energonsoftware::BoundingSphere bs2(s2);
        CPPUNIT_ASSERT(s2.center() == bs2.center());
        CPPUNIT_ASSERT_EQUAL(s2.radius(), bs2.radius());

        static const energonsoftware::Point3 min3(-15.52f, -235.35f, 143.235f);
        static const energonsoftware::Point3 max3(154.134f, -34.35f, 154.23f);
        static const energonsoftware::AABB a3(min3 + min3, max3 + max3);
        static const energonsoftware::BoundingSphere bs3(a3);
        CPPUNIT_ASSERT(a3.center() == bs3.center());
        CPPUNIT_ASSERT(a3.radius() == bs3.radius());
    }

    void test_distance()
    {
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BoundingSphereTest);

#endif
