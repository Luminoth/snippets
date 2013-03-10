#if !defined __PHYSICAL_H__
#define __PHYSICAL_H__

#include "src/core/math/Quaternion.h"
#include "src/core/math/Vector.h"
#include "partition/Partitionable.h"
#include "AABB.h"

namespace energonsoftware {

class Matrix4;

class Transform final
{
public:
    Transform();
    Transform(const Transform& transform);
    virtual ~Transform() noexcept;

public:
    const Position& position() const { return _position; }
    void position(const Position& position);

    Direction forward() const { return _orientation * Vector::FORWARD; }
    Direction up() const { return _orientation * Vector::UP; }
    Direction right() const { return _orientation * Vector::RIGHT; }

    const Quaternion& orientation() const { return _orientation; }
    void orientation(const Quaternion& orientation);

    // angle is in radians
    void rotate(float angle, const Vector3& around);
    void pitch(float angle);
    void yaw(float angle);
    void roll(float angle);

    float scale() const { return _scale; }
    void scale(float scale);

    // applies this physicals translation, rotation, and scale to the matrix
    // useful for generating the model matrix for this physical
    void transform(Matrix4& matrix) const;

    std::string str() const;

public:
    Transform& operator=(const Transform& transform);

private:
    friend class Physical;
    void update(const Vector3& velocity, double dt);

private:
    boost::recursive_mutex _mutex;

    Position _position;
    Quaternion _orientation;
    float _scale;
};

class Collider
{
public:
    Collider() {}
    virtual ~Collider() noexcept {}

public:
    virtual void bounds(const BoundingVolume& bounds) = 0;
    virtual const BoundingVolume& bounds() const = 0;
    virtual std::string str() const = 0;
};

class AABBCollider : public Collider
{
public:
    AABBCollider();
    AABBCollider(const AABBCollider& collider);
    virtual ~AABBCollider() noexcept;

public:
    virtual void bounds(const BoundingVolume& bounds) override;
    virtual const BoundingVolume& bounds() const override { return _bounds; }
    virtual std::string str() const;

public:
    AABBCollider& operator=(const AABBCollider& collider);

private:
    boost::recursive_mutex _mutex;

    AABB _bounds;
};

class RigidBody final
{
public:
    RigidBody();
    RigidBody(const RigidBody& rigidbody);
    virtual ~RigidBody() noexcept;

public:
    const Vector3& velocity() const { return _velocity; }
    void velocity(const Vector3& velocity);

    const Vector3& acceleration() const { return _acceleration; }
    void acceleration(const Vector3& acceleration);

    float mass() const { return _mass; }
    void mass(float mass);

    std::string str() const;

public:
    RigidBody& operator=(const RigidBody& rigidbody);

private:
    friend class Physical;
    void update(double dt);

private:
    boost::recursive_mutex _mutex;

    Vector3 _velocity, _acceleration;
    float _mass;
};

class Physical : public Partitionable
{
public:
    Physical(const Physical& physical);
    virtual ~Physical() noexcept;

public:
    virtual const Transform& transform() const final { return _transform; }
    virtual const Collider& collider() const final { return *_collider; }
    virtual const RigidBody& rigid_body() const final { return _rigidbody; }

    virtual void simulate() final;

    virtual const Position& position() const override { return _transform.position(); }
    virtual const BoundingVolume& relative_bounds() const override { return _collider->bounds(); }
    virtual const BoundingVolume& absolute_bounds() const override { return _absolute_bounds; }

    // TODO: this is such a shit solution... wtf
    virtual void absolute_bounds(const BoundingVolume& bounds) final;

    virtual std::string str() const;

public:
    Physical& operator=(const Physical& rhs);

protected:
    Physical();

    virtual bool on_simulate(double dt) { return true; }

private:
    boost::recursive_mutex _mutex;

    Transform _transform;
    std::shared_ptr<Collider> _collider;
    RigidBody _rigidbody;
    double _last_simulate;

    // TODO: this is such a shit solution... wtf
    AABB _absolute_bounds;
};

}

#endif
