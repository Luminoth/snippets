#include "src/pch.h"
#include "src/core/math/math_util.h"
#include "src/core/math/Matrix4.h"
#include "src/core/util/util.h"
#include "Physical.h"

namespace energonsoftware {

Transform::Transform()
    : _mutex(), _position(), _orientation(), _scale(1.0f)
{
}

Transform::~Transform() throw()
{
}

void Transform::position(const Position& position)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _position = position;
}

void Transform::orientation(const Quaternion& orientation)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _orientation = orientation;
}

void Transform::rotate(float angle, const Vector3& around)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    Quaternion q(Quaternion::new_axis(angle, around));
    _orientation = q * _orientation;
}

void Transform::pitch(float angle)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    // need to pitch against our local x-axis
    // TODO: need a better explanation for why this is a special case!
    Quaternion q(Quaternion::new_axis(angle, Vector::XAXIS));
    _orientation = _orientation * q;
}

void Transform::yaw(float angle)
{
    rotate(angle, Vector::YAXIS);
}

void Transform::roll(float angle)
{
    rotate(angle, Vector::ZAXIS);
}

void Transform::scale(float scale)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _scale = scale;
}

void Transform::transform(Matrix4& matrix) const
{
    matrix.translate(_position);
    matrix *= _orientation.matrix();
    matrix.uniform_scale(_scale);
}

void Transform::update(const Vector3& velocity, double dt)
{
    _position += velocity * dt;
}

AABBCollider::AABBCollider()
    : Collider(), _mutex(), _bounds()
{
}

void AABBCollider::bounds(const BoundingVolume& bounds)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    // TODO: typeof check this and convert if necessary!
    _bounds = dynamic_cast<const AABB&>(bounds);
}

RigidBody::RigidBody()
    : _velocity(), _acceleration(), _mass(1.0f)
{
}

RigidBody::~RigidBody() throw()
{
}

void RigidBody::velocity(const Vector3& velocity)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _velocity = velocity;
}

void RigidBody::acceleration(const Vector3& acceleration)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _acceleration = acceleration;
}

void RigidBody::mass(float mass)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _mass = mass;
}

void RigidBody::update(double dt)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _velocity += _acceleration * dt;
}

Physical::Physical()
    : Partitionable(),
        _mutex(), _transform(), _collider(new AABBCollider()), _rigidbody(),
        _last_simulate(get_time()), _absolute_bounds()
{
}

Physical::Physical(const Physical& physical)
    : Partitionable(),
        _mutex(), _transform(physical._transform), _collider(new AABBCollider(*physical._collider)), _rigidbody(physical._rigidbody),
        _last_simulate(get_time()), _absolute_bounds(physical._absolute_bounds)
{
}

Physical::~Physical() throw()
{
}

void Physical::simulate()
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    const double now = get_time();
    const double dt = now - _last_simulate;

    if(!on_simulate(dt)) {
        return;
    }

    _rigidbody.update(dt);
    _transform.update(_rigidbody.velocity(), dt);

    _last_simulate = now;
}

void Physical::absolute_bounds(const BoundingVolume& bounds)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    _collider->bounds(bounds);
    _absolute_bounds = position() + dynamic_cast<const AABB&>(bounds);
}

std::string Physical::str() const
{
    // TODO: expand this
    std::stringstream ss;
    ss << "Physical(t:" << _transform.str() << ", c:" << _collider->str() << ", r:" << _rigidbody.str() << ")";
    return ss.str();
}

Physical& Physical::operator=(const Physical& rhs)
{
    _transform = rhs._transform;
    _collider.reset(new AABBCollider(*rhs._collider));
    _rigidbody = rhs._rigidbody;
    return *this;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class TestPhysical : public energonsoftware::Physical
{
public:
    TestPhysical()
        : energonsoftware::Physical(),
            _simulated(false)
    {
    }

    virtual ~TestPhysical() throw()
    {
    }

public:
    bool simulated() const { return _simulated; }

private:
    virtual bool on_simulate(double dt) override
    {
        _simulated = true;
        return true;
    }

private:
    bool _simulated;
};

class PhysicalTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(PhysicalTest);
        CPPUNIT_TEST(test_simulate);
    CPPUNIT_TEST_SUITE_END();

public:
    PhysicalTest() : CppUnit::TestFixture() {}
    virtual ~PhysicalTest() throw() {}

public:
    virtual void test_simulate()
    {
        TestPhysical p;
        CPPUNIT_ASSERT(!p.simulated());
        p.simulate();
        CPPUNIT_ASSERT(p.simulated());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhysicalTest);

#endif
