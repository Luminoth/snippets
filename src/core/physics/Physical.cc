#include "src/pch.h"
#include "src/core/math/math_util.h"
#include "src/core/math/Matrix4.h"
#include "src/core/util/util.h"
#include "Physical.h"

namespace energonsoftware {

Physical::Physical()
    : _view(0.0f, 0.0f, 1.0f), _up(0.0f, 1.0f, 0.0f),
        _mass(1.0f), _scale(1.0f), _last_simulate(get_time())
{
}

Physical::~Physical() throw()
{
}

void Physical::position(const Position& position)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _position = position;
}

void Physical::view(const Direction& view)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _view = view;
}

void Physical::up(const Direction& up)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _up = up;
}

void Physical::orientation(const Quaternion& orientation)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    _orientation = orientation;
}

void Physical::rotate(float angle, const Vector3& around)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    Quaternion q(Quaternion::new_axis(angle, around));
    _orientation = q * _orientation;
}

void Physical::pitch(float angle)
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    // need to pitch against our local x-axis
    // TODO: need a better explanation for why this is a special case!
    Quaternion q(Quaternion::new_axis(angle, Vector3(1.0f, 0.0f, 0.0f)));
    _orientation = _orientation * q;
}

void Physical::yaw(float angle)
{
    rotate(angle, Vector3(0.0f, 1.0f, 0.0f));
}

void Physical::roll(float angle)
{
    rotate(angle, Vector3(0.0f, 0.0f, 1.0f));
}

void Physical::transform(Matrix4& matrix) const
{
    matrix.translate(_position);
    matrix *= _orientation.matrix();
    matrix.uniform_scale(_scale);
}

void Physical::simulate()
{
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);

    const double now = get_time();
    const double dt = now - _last_simulate;

    if(!on_simulate(dt)) {
        return;
    }

    // adjust our velocity by our acceleration
    _velocity += _acceleration * dt;

    // apply the velocity to our position
    _position += _velocity * dt;

    _last_simulate = now;
}

std::string Physical::str() const
{
    // TODO: expand this
    std::stringstream ss;
    ss << "Physical(p:" << _position.str() << ", o:" << _orientation.str() << ", s:" << _scale << ")";
    return ss.str();
}

}

#ifdef WITH_UNIT_TESTS
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