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
    const Position& position() const { return _position; }
    void position(const Position& position);

    const Direction& view_unrotated() const { return _view; }
    void view(const Direction& view);

    // NOTE: not normalized
    Direction view() const { return _orientation * _view; }

    const Direction& up_unrotated() const { return _up; }
    void up(const Direction& up);

    // NOTE: not normalized
    Direction up() const { return _orientation * _up; }

    const Quaternion& orientation() const { return _orientation; }
    void orientation(const Quaternion& orientation);

    // angle is in radians
    void rotate(float angle, const Vector3& around);
    void pitch(float angle);
    void yaw(float angle);
    void roll(float angle);

    // applies this physicals translation, rotation, and scale to the matrix
    // useful for generating the model matrix for this physical
    void transform(Matrix4& matrix) const;

    const Vector3& velocity() const { return _velocity; }
    void velocity(const Vector3& velocity) { _velocity = velocity; }

    const Vector3& acceleration() const { return _acceleration; }
    void acceleration(const Vector3& acceleration) { _acceleration = acceleration; }

    float mass() const { return _mass; }

    float scale() const { return _scale; }
    void scale(float scale) { _scale = scale; }

    AABB absolute_bounds() const { return _position + _bounds; }
    const AABB& relative_bounds() const { return _bounds; }

    void simulate();

    virtual std::string str() const;

protected:
    Physical();

    // NOTE: relative to the physical's position
    void bounds(const AABB& bounds) { _bounds = bounds; }

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
