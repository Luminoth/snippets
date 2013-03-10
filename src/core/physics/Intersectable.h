#if !defined __INTERSECTABLE_H__
#define __INTERSECTABLE_H__

namespace energonsoftware {

class Intersectable
{
public:
    Intersectable() {}
    virtual ~Intersectable() noexcept {}

public:
    bool intersect(const Intersectable& other) const
    {
        // NOTE: <= used to make AABBs work, don't change back to <
        return distance(other) <= 0.0f;
    }

public:
    virtual float distance(const Intersectable& other) const = 0;
    virtual std::string str() const = 0;
};

}

#endif
