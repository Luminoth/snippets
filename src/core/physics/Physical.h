#if !defined __PHYSICAL_H__
#define __PHYSICAL_H__

#include "src/core/math/Quaternion.h"
#include "src/core/math/Vector.h"
#include "partition/Partitionable.h"
#include "AABB.h"

namespace energonsoftware {

class Matrix4;

class Physical : public Partitionable
{
public:
    virtual ~Physical() throw();

public:
    virtual const Position& position() const override { return _position; }
    virtual void position(const Position& position) final;

    virtual const Direction& forward_unrotated() const final { return _forward; }
    virtual void forward(const Direction& forward) final;

    // NOTE: not normalized
    virtual Direction forward() const final { return _orientation * _forward; }

    virtual const Direction& up_unrotated() const final { return _up; }
    virtual void up(const Direction& up) final;

    // NOTE: not normalized
    virtual Direction up() const final { return _orientation * _up; }

    virtual const Direction& right_unrotated() const final { return _right; }
    virtual void right(const Direction& right) final;

    // NOTE: not normalized
    virtual Direction right() const final { return _orientation * _right; }

    virtual const Quaternion& orientation() const final { return _orientation; }
    virtual void orientation(const Quaternion& orientation) final;

    // angle is in radians
    virtual void rotate(float angle, const Vector3& around) final;
    virtual void pitch(float angle) final;
    virtual void yaw(float angle) final;
    virtual void roll(float angle) final;

    // applies this physicals translation, rotation, and scale to the matrix
    // useful for generating the model matrix for this physical
    virtual void transform(Matrix4& matrix) const final;

    virtual const Vector3& velocity() const final { return _velocity; }
    virtual void velocity(const Vector3& velocity) final { _velocity = velocity; }

    virtual const Vector3& acceleration() const final { return _acceleration; }
    virtual void acceleration(const Vector3& acceleration) final { _acceleration = acceleration; }

    virtual float mass() const final { return _mass; }

    virtual float scale() const final { return _scale; }
    virtual void scale(float scale) final { _scale = scale; }

    virtual const BoundingVolume& relative_bounds() const override { return _relative_bounds; }
    virtual const BoundingVolume& absolute_bounds() const override { return _absolute_bounds; }

    virtual void simulate() final;

    std::string str() const;

protected:
    Physical();

    virtual void relateive_bounds(const AABB& bounds) final
    {
        _relative_bounds = bounds;
        _absolute_bounds = position() + _relative_bounds;
    }

    virtual bool on_simulate(double dt) { return true; }

private:
    boost::recursive_mutex _mutex;

    // "camera" properties
    Position _position;
    Direction _forward, _up, _right;
    Quaternion _orientation;

    // physical properties
    Vector3 _velocity, _acceleration;
    float _mass;
    float _scale;
    AABB _relative_bounds, _absolute_bounds;

    double _last_simulate;
};

}

#endif
