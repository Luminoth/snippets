#if !defined __BOUNDINGVOLUME_H__
#define __BOUNDINGVOLUME_H__

#include "src/core/math/Vector.h"
#include "Intersectable.h"

namespace energonsoftware {

class BoundingVolume : public Intersectable
{
public:
    BoundingVolume() : Intersectable() {}
    virtual ~BoundingVolume() throw() {}

public:
    virtual const Point3& center() const = 0;
    virtual float radius() const = 0;
    virtual float radius_squared() const = 0;
};

}

#endif
