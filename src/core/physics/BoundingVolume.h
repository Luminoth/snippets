#if !defined __BOUNDINGVOLUME_H__
#define __BOUNDINGVOLUME_H__

#include "src/core/math/Intersectable.h"
#include "src/core/math/Vector.h"

namespace energonsoftware {

class BoundingVolume /*: public Intersectable*/
{
public:
    BoundingVolume() {}
    virtual ~BoundingVolume() throw() {}

public:
    virtual const Point3& center() const = 0;
    virtual float radius() const = 0;
    virtual float radius_squared() const = 0;
};

}

#endif
