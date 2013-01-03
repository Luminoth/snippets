#if !defined __SPHERE_H__
#define __SPHERE_H__

#include "Vector.h"

namespace energonsoftware {

class Sphere
{
public:
    explicit Sphere(const Point3& center=Point3(), float radius=1.0f);
    virtual ~Sphere() throw();

public:
    void center(const Point3& center) { _center = center; }
    const Point3& center() const { return _center; }

    void radius(float radius) { _radius = radius; }
    float radius() const { return _radius; }

    // TODO: if we add distance calculations to this
    // then update the BoundingSphere class to make use of them

public:
    friend Sphere operator+(const Position& lhs, const Sphere& rhs) { return Sphere(lhs + rhs._center, rhs._radius); }

private:
    Point3 _center;
    float _radius;
};

}

#endif
