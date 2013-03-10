#if !defined __BOUNDINGSPHERE_H__
#define __BOUNDINGSPHERE_H__

#include "src/core/math/Sphere.h"
#include "BoundingVolume.h"

namespace energonsoftware {

class AABB;
class BoundingCapsule;

class BoundingSphere : public BoundingVolume
{
public:
    explicit BoundingSphere(const Sphere& sphere=Sphere());
    explicit BoundingSphere(const Point3& center, float radius);
    explicit BoundingSphere(const AABB& aabb);
    virtual ~BoundingSphere() noexcept;

public:
    void sphere(const Sphere& sphere) { _sphere = sphere; }
    const Sphere& sphere() const { return _sphere; }

    virtual const Point3& center() const override { return _sphere.center(); }
    virtual float radius() const override { return _sphere.radius(); }
    virtual float radius_squared() const override { return std::pow(_sphere.radius(), 2); }

    virtual float distance(const Intersectable& other) const override;

    float distance_squared(const Point3& p) const;
    float distance(const Point3& p) const;
    /*float distance_squared(const BoundingCapsule& c) const;
    float distance(const BoundingCapsule& c) const;*/
    float distance_squared(const BoundingSphere& s) const;
    float distance(const BoundingSphere& s) const;
    float distance_squared(const AABB& a) const;
    float distance(const AABB& a) const;

    virtual std::string str() const override;

public:
    friend BoundingSphere operator+(const Position& lhs, const BoundingSphere& rhs) { return BoundingSphere(lhs + rhs._sphere); }

protected:
    Sphere _sphere;
};

}

#endif
