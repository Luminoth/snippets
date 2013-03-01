#if !defined __BOUNDINGCAPSULE_H__
#define __BOUNDINGCAPSULE_H__

#include "src/core/math/Capsule.h"
#include "BoundingVolume.h"

namespace energonsoftware {

class AABB;
class BoundingSphere;

class BoundingCapsule : public BoundingVolume
{
public:
    explicit BoundingCapsule(const Capsule& capsule=Capsule());
    explicit BoundingCapsule(const Point3& center, float radius, float height);

    // sets both the radius and height to the AABB's radius
    explicit BoundingCapsule(const AABB& aabb);

    virtual ~BoundingCapsule() noexcept;

public:
    void sphere(const Capsule& capsule) { _capsule = capsule; }
    const Capsule& capsule() const { return _capsule; }

    virtual const Point3& center() const override { return _capsule.center(); }
    virtual float radius() const override { return _capsule.radius(); }
    virtual float radius_squared() const override { return std::pow(_capsule.radius(), 2); }

    float height() const { return _capsule.height(); }

    virtual float distance(const Intersectable& other) const override;

    // TODO: fix these
    /*float distance_squared(const Point3& p) const;
    float distance(const Point3& p) const;
    float distance_squared(const BoundingCapsule& c) const;
    float distance(const BoundingCapsule& c) const;
    float distance_squared(const BoundingSphere& s) const;
    float distance(const BoundingSphere& s) const;
    float distance_squared(const AABB& a) const;
    float distance(const AABB& a) const;*/

    virtual std::string str() const override;

protected:
    Capsule _capsule;
};

}

#endif
