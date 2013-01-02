#if !defined __PHYSICAL_H__
#define __PHYSICAL_H__

#include "src/core/math/Quaternion.h"
#include "src/core/math/Vector.h"
#include "AABB.h"

namespace energonsoftware {

class Matrix4;

class Physical
{
public:
    virtual ~Physical() throw();

public:
    virtual const Position& position() const final { return _position; }
    virtual void position(const Position& position) final;

    virtual const Direction& view_unrotated() const final { return _view; }
    virtual void view(const Direction& view) final;

    // NOTE: not normalized
    virtual Direction view() const final { return _orientation * _view; }

    virtual const Direction& up_unrotated() const final { return _up; }
    virtual void up(const Direction& up) final;

    // NOTE: not normalized
    virtual Direction up() const final { return _orientation * _up; }

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

    virtual AABB absolute_bounds() const final { return _position + _bounds; }
    virtual const AABB& relative_bounds() const final { return _bounds; }

    virtual void simulate() final;

    std::string str() const;

protected:
    Physical();

    // NOTE: relative to the physical's position
    virtual void bounds(const AABB& bounds) final { _bounds = bounds; }

    virtual bool on_simulate(double dt) { return true; }

private:
    boost::recursive_mutex _mutex;

    // "camera" properties
    Position _position;
    Direction _view, _up;
    Quaternion _orientation;

    // physical properties
    Vector3 _velocity, _acceleration;
    float _mass;
    float _scale;
    AABB _bounds;

    double _last_simulate;
};

}

#endif
