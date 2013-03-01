#if !defined __AABB_H__
#define __AABB_H__

#include "BoundingVolume.h"

namespace energonsoftware {

class BoundingCapsule;
class BoundingSphere;

class AABB : public BoundingVolume
{
public:
    // creates an infinite/invalid bounding box
    AABB();

    AABB(const Point3& minimum, const Point3& maximum);
    AABB(const Point3& center, float radius);
    virtual ~AABB() noexcept;

public:
    const Point3& minimum() const { return _minimum; }
    const Point3& maximum() const { return _maximum; }

    virtual const Point3& center() const override { return _center; }
    virtual float radius() const override { return _radius; }
    virtual float radius_squared() const override { return std::pow(_radius, 2); }

    // updates the current bounds to encompass the given point(s)
    void update(const AABB& bounds);
    void update(const Point3& point);

    virtual float distance(const Intersectable& other) const override;

    float distance_squared(const Point3& p) const;
    float distance(const Point3& p) const;
    /*float distance_squared(const BoundingCapsule& c) const;
    float distance(const BoundingCapsule& c) const;*/
    float distance_squared(const BoundingSphere& s) const;
    float distance(const BoundingSphere& s) const;
    float distance_squared(const AABB& a) const;
    float distance(const AABB& a) const;

    Point3 closest_point(const Point3& point) const;

    std::string str() const;

public:
    friend AABB operator+(const Position& lhs, const AABB& rhs) { return AABB(lhs + rhs._minimum, lhs + rhs._maximum); }

private:
    void calculate_center();

private:
    Point3 _minimum, _maximum, _center;
    float _radius;
};

}

#endif
