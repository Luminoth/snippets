#ifndef __BOUNDINGSPHERE_H__
#define __BOUNDINGSPHERE_H__

#include "src/core/math/Sphere.h"
#include "BoundingVolume.h"

namespace energonsoftware {

class AABB;

class BoundingSphere : public BoundingVolume
{
public:
    explicit BoundingSphere(const Sphere& sphere=energonsoftware::Sphere());
    explicit BoundingSphere(const AABB& aabb);
    virtual ~BoundingSphere() throw();

public:
    void sphere(const Sphere& sphere) { _sphere = sphere; }
    const Sphere& sphere() const { return _sphere; }

    virtual const Point3& center() const { return _sphere.center(); }
    virtual float radius() const { return _sphere.radius(); }
    virtual float radius_squared() const { return std::pow(_sphere.radius(), 2); }

    float distance_squared(const Point3& p) const;
    float distance(const Point3& p) const;
    float distance_squared(const BoundingSphere& s) const;
    float distance(const BoundingSphere& s) const;
    float distance_squared(const AABB& a) const;
    float distance(const AABB& a) const;

    virtual std::string str() const;

protected:
    Sphere _sphere;
};

}

#endif
