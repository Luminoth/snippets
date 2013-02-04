#if !defined __CAPSULE_H__
#define __CAPSULE_H__

#include "Vector.h"

namespace energonsoftware {

class Capsule
{
public:
    explicit Capsule(const Point3& center=Point3(), float radius=1.0f, float height=1.0f);
    virtual ~Capsule() throw();

public:
    void center(const Point3& center) { _center = center; }
    const Point3& center() const { return _center; }

    void radius(float radius) { _radius = radius; }
    float radius() const { return _radius; }

    void height(float height) { _height = height; }
    float height() const { return _height; }

    // TODO: if we add distance calculations to this
    // then update the BoundingCapsule class to make use of them

private:
    Point3 _center;
    float _radius, _height;
};

}

#endif