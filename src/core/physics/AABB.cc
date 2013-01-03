#include "src/pch.h"
#include "src/core/math/math_util.h"
#include "BoundingSphere.h"
#include "AABB.h"

namespace energonsoftware {

AABB::AABB()
    : BoundingVolume(),
        _minimum(FLT_MAX, FLT_MAX, FLT_MAX),
        _maximum(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
    calculate_center();
}

AABB::AABB(const Point3& minimum, const Point3& maximum)
    : BoundingVolume(), _minimum(minimum), _maximum(maximum)
{
    calculate_center();
}

AABB::AABB(const Point3& center, float radius)
    : BoundingVolume(), _center(center), _radius(radius)
{
    Vector3 rv(radius, radius, radius);
    _minimum = Point3(center - rv);
    _maximum = center + rv;
}

AABB::~AABB() throw()
{
}

void AABB::update(const AABB& bounds)
{
    if(bounds.minimum().x() < _minimum.x()) {
        _minimum.x(bounds.minimum().x());
    }
    if(bounds.minimum().y() < _minimum.y()) {
        _minimum.y(bounds.minimum().y());
    }
    if(bounds.minimum().z() < _minimum.z()) {
        _minimum.z(bounds.minimum().z());
    }

    if(bounds.maximum().x() > _maximum.x()) {
        _maximum.x(bounds.maximum().x());
    }
    if(bounds.maximum().y() > _maximum.y()) {
        _maximum.y(bounds.maximum().y());
    }
    if(bounds.maximum().z() > _maximum.z()) {
        _maximum.z(bounds.maximum().z());
    }

    calculate_center();
}

void AABB::update(const Point3& point)
{
    if(point.x() < _minimum.x()) {
        _minimum.x(point.x());
    }
    if(point.y() < _minimum.y()) {
        _minimum.y(point.y());
    }
    if(point.z() < _minimum.z()) {
        _minimum.z(point.z());
    }

    if(point.x() > _maximum.x()) {
        _maximum.x(point.x());
    }
    if(point.y() > _maximum.y()) {
        _maximum.y(point.y());
    }
    if(point.z() > _maximum.z()) {
        _maximum.z(point.z());
    }

    calculate_center();
}

float AABB::distance(const Intersectable& other) const
{
    if(typeid(other) == typeid(BoundingSphere)) {
        return distance(dynamic_cast<const BoundingSphere&>(other));
    } else if(typeid(other) == typeid(AABB)) {
        return distance(dynamic_cast<const AABB&>(other));
    }
    throw std::runtime_error(std::string("BoundingSphere doesn't know how to intersect ") + typeid(other).name());
}

float AABB::distance_squared(const Point3& p) const
{
    return p.distance_squared(closest_point(p));
}

float AABB::distance(const Point3& p) const
{
    return p.distance(closest_point(p));
}

float AABB::distance_squared(const BoundingSphere& s) const
{
    return s.distance_squared(closest_point(s.center()));
}

float AABB::distance(const BoundingSphere& s) const
{
    return s.distance(closest_point(s.center()));
}

float AABB::distance_squared(const AABB& a) const
{
    return a.distance_squared(closest_point(a.center()));
}

float AABB::distance(const AABB& a) const
{
    return a.distance(closest_point(a.center()));
}

Point3 AABB::closest_point(const Point3& point) const
{
    float x = point.x();
    if(point.x() < _minimum.x()) {
        x = _minimum.x();
    } else if(point.x() > _maximum.x()) {
        x = _maximum.x();
    }

    float y = point.y();
    if(point.y() < _minimum.y()) {
        y = _minimum.y();
    } else if(point.y() > _maximum.y()) {
        y = _maximum.y();
    }

    float z = point.z();
    if(point.z() < _minimum.z()) {
        z = _minimum.z();
    } else if(point.z() > _maximum.z()) {
        z = _maximum.z();
    }

    return Point3(x, y, z);
}

std::string AABB::str() const
{
    std::stringstream ss;
    ss << "AABB(" << _minimum.str() << ", " << _maximum.str() << ")";
    return ss.str();
}

void AABB::calculate_center()
{
    _center = _minimum + ((_maximum - _minimum) / 2.0f);
    _radius = center().distance(_minimum);
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class AABBTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(AABBTest);
        CPPUNIT_TEST(test_initialize);
        CPPUNIT_TEST(test_update);
        CPPUNIT_TEST(test_distance);
        CPPUNIT_TEST(test_closest_point);
        CPPUNIT_TEST(test_addition);
    CPPUNIT_TEST_SUITE_END();

public:
    AABBTest() : CppUnit::TestFixture() {}
    virtual ~AABBTest() throw() {}

public:
    void test_initialize()
    {
        static const energonsoftware::AABB a1;
        CPPUNIT_ASSERT(a1.radius() < -FLT_MAX || a1.radius() > FLT_MAX);

        static const energonsoftware::Point3 min2(-15.52f, -235.35f, 143.235f);
        static const energonsoftware::Point3 max2(154.134f, -34.35f, 154.23f);
        static const energonsoftware::Point3 c2(min2 + ((max2 - min2) / 2.0f));
        static const energonsoftware::AABB a2(min2, max2);
        CPPUNIT_ASSERT(min2 == a2.minimum());
        CPPUNIT_ASSERT(max2 == a2.maximum());
        CPPUNIT_ASSERT(c2 == a2.center());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(c2.distance(min2), a2.radius(), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(std::pow(c2.distance(min2), 2.0f), a2.radius_squared(), 0.0001f);

        static const energonsoftware::Point3 c3(1.0f, -1.0f, 2.0f);
        static const energonsoftware::Vector3 r3(100.0f, 100.0f, 100.0f);
        static const energonsoftware::AABB a3(c3, r3.x());
        CPPUNIT_ASSERT(energonsoftware::Vector3(c3 - r3) == a3.minimum());
        CPPUNIT_ASSERT((c3 + r3) == a3.maximum());
        CPPUNIT_ASSERT(c3 == a3.center());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(r3.x(), a3.radius(), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(std::pow(r3.x(), 2.0f), a3.radius_squared(), 0.0001f);
    }

    void test_update()
    {
        static const energonsoftware::Point3 min1(-15.52f, -235.35f, 143.235f);
        static const energonsoftware::Point3 max1(154.134f, -34.35f, 154.23f);
        static const energonsoftware::AABB a1(min1 + min1, max1 + max1);
        energonsoftware::AABB a2(min1, max1);
        a2.update(a1);
        static const energonsoftware::Point3 expected_min1(a1.minimum().x(), a1.minimum().y(), min1.z());
        static const energonsoftware::Point3 expected_max1(a1.maximum().x(), max1.y(), a1.maximum().z());
        static const energonsoftware::Point3 expected_c1(expected_min1 + ((expected_max1 - expected_min1) / 2.0f));
        CPPUNIT_ASSERT_EQUAL(expected_min1.x(), a2.minimum().x());
        CPPUNIT_ASSERT_EQUAL(expected_min1.y(), a2.minimum().y());
        CPPUNIT_ASSERT_EQUAL(expected_min1.z(), a2.minimum().z());
        CPPUNIT_ASSERT_EQUAL(0.0f, a2.minimum().w());
        CPPUNIT_ASSERT_EQUAL(expected_max1.x(), a2.maximum().x());
        CPPUNIT_ASSERT_EQUAL(expected_max1.y(), a2.maximum().y());
        CPPUNIT_ASSERT_EQUAL(expected_max1.z(), a2.maximum().z());
        CPPUNIT_ASSERT_EQUAL(0.0f, a2.maximum().w());
        CPPUNIT_ASSERT(expected_c1 == a2.center());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(expected_c1.distance(expected_min1), a2.radius(), 0.0001f);

        static const energonsoftware::Point3 p3(-1111.03f, 1354.03f, 1235.34f);
        energonsoftware::AABB a3(min1, max1);
        a3.update(p3);
        static const energonsoftware::Point3 expected_min3(p3.x(), min1.y(), min1.z());
        static const energonsoftware::Point3 expected_max3(max1.x(), p3.y(), p3.z());
        static const energonsoftware::Point3 expected_c3(expected_min3 + ((expected_max3 - expected_min3) / 2.0f));
        CPPUNIT_ASSERT_EQUAL(expected_min3.x(), a3.minimum().x());
        CPPUNIT_ASSERT_EQUAL(expected_min3.y(), a3.minimum().y());
        CPPUNIT_ASSERT_EQUAL(expected_min3.z(), a3.minimum().z());
        CPPUNIT_ASSERT_EQUAL(0.0f, a3.minimum().w());
        CPPUNIT_ASSERT_EQUAL(expected_max3.x(), a3.maximum().x());
        CPPUNIT_ASSERT_EQUAL(expected_max3.y(), a3.maximum().y());
        CPPUNIT_ASSERT_EQUAL(expected_max3.z(), a3.maximum().z());
        CPPUNIT_ASSERT_EQUAL(0.0f, a3.maximum().w());
        CPPUNIT_ASSERT(expected_c3 == a3.center());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(expected_c3.distance(expected_min3), a3.radius(), 0.0001f);
    }

    void test_distance()
    {
        energonsoftware::AABB a1(energonsoftware::Point3(0, 0, 0), energonsoftware::Point3(10, 10, 10));
        CPPUNIT_ASSERT_EQUAL(5.0f, a1.distance(energonsoftware::Point3(5, 5, 15)));
        CPPUNIT_ASSERT_EQUAL(5.0f, a1.distance(energonsoftware::Point3(5, 5, -5)));
        CPPUNIT_ASSERT_EQUAL(5.0f, a1.distance(energonsoftware::Point3(5, -5, 5)));
        CPPUNIT_ASSERT_EQUAL(5.0f, a1.distance(energonsoftware::Point3(-5, 5, 5)));

        energonsoftware::AABB a2(energonsoftware::Point3(0, 0, 0), energonsoftware::Point3(10, 10, 10));
        energonsoftware::AABB a3(energonsoftware::Point3(20, 20, 20), energonsoftware::Point3(23, 23, 23));
        CPPUNIT_ASSERT(a2.distance(a3) - 17.3205 < 0.01);

        energonsoftware::AABB a4(energonsoftware::Point3(0, 0, 0), energonsoftware::Point3(10, 10, 10));
        CPPUNIT_ASSERT_EQUAL(4.0f, a4.distance(energonsoftware::BoundingSphere(energonsoftware::Sphere(energonsoftware::Point3(5, 5, 15)))));
        CPPUNIT_ASSERT_EQUAL(4.0f, a4.distance(energonsoftware::BoundingSphere(energonsoftware::Sphere(energonsoftware::Point3(5, 5, -5)))));
        CPPUNIT_ASSERT_EQUAL(4.0f, a4.distance(energonsoftware::BoundingSphere(energonsoftware::Sphere(energonsoftware::Point3(5, -5, 5)))));
        CPPUNIT_ASSERT_EQUAL(4.0f, a4.distance(energonsoftware::BoundingSphere(energonsoftware::Sphere(energonsoftware::Point3(-5, 5, 5)))));
    }

    void test_closest_point()
    {
        static const energonsoftware::AABB box(energonsoftware::Point3(0, 0, 0), energonsoftware::Point3(10, 10, 10));
        energonsoftware::Point3 p1(5, 5, 15);
        energonsoftware::Point3 p2(5, 5, -5);
        energonsoftware::Point3 p3(5, -5, 5);

        energonsoftware::Point3 cp1(5, 5, 10);
        energonsoftware::Point3 cp2(5, 5, 0);
        energonsoftware::Point3 cp3(5, 0, 5);

        CPPUNIT_ASSERT_EQUAL(0.0f, box.closest_point(p1).distance(cp1));
        CPPUNIT_ASSERT_EQUAL(0.0f, box.closest_point(p2).distance(cp2));
        CPPUNIT_ASSERT_EQUAL(0.0f, box.closest_point(p3).distance(cp3));
    }

    void test_addition()
    {
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AABBTest);

#endif
