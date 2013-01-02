#if !defined __PARTITIONABLE_H__
#define __PARTITIONABLE_H__

#include "src/core/math/Vector.h"
#include "src/core/physics/BoundingVolume.h"

namespace energonsoftware {

class Partitionable
{
public:
    Partitionable() {}
    virtual ~Partitionable() throw() {}

public:
    // override these
    virtual const BoundingVolume& bounds() const = 0;
    virtual const Position& position() const = 0;
};

}

#endif
