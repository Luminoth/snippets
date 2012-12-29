#include "src/pch.h"
#include "AABB.h"
#include "BoundingSphere.h"

namespace energonsoftware {

BoundingSphere::BoundingSphere(const Sphere& sphere)
    : BoundingVolume(), _sphere(sphere)
{
}

BoundingSphere::BoundingSphere(const AABB& aabb)
    : BoundingVolume(), _sphere(aabb.center(), aabb.radius())
{
}

BoundingSphere::~BoundingSphere() throw()
{
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

float BoundingSphere::distance(const AABB& other) const
{
    return other.distance(*this);
}

std::string BoundingSphere::str() const
{
    std::stringstream ss;
    ss << "BoundingSphere(" << _sphere.center().str() << ", " << _sphere.radius() << ")";
    return ss.str();
}

}

#ifdef WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class BoundingSphereTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(BoundingSphereTest);
    CPPUNIT_TEST_SUITE_END();

public:
    BoundingSphereTest() : CppUnit::TestFixture() {}
    virtual ~BoundingSphereTest() throw() {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(BoundingSphereTest);

#endif