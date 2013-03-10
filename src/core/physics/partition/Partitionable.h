#if !defined __PARTITIONABLE_H__
#define __PARTITIONABLE_H__

#include "src/core/math/Vector.h"
#include "src/core/physics/BoundingVolume.h"

namespace energonsoftware {

class Partitionable
{
public:
    Partitionable() {}
    virtual ~Partitionable() noexcept {}

public:
    virtual const Position& position() const = 0;
    virtual const BoundingVolume& relative_bounds() const = 0;
    virtual const BoundingVolume& absolute_bounds() const = 0;
};

}

#endif
