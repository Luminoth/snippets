#if !defined __PARTITION_H__
#define __PARTITION_H__

#include "src/core/math/Vector.h"
#include "src/core/physics/BoundingVolume.h"

namespace energonsoftware {

class PartitionError : public std::exception
{
public:
    explicit PartitionError(const std::string& what) throw() : _what(what) { }
    virtual ~PartitionError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

protected:
    std::string _what;
};

// defines the partition interface using a simple flat partition
// T must inherit from Partitionable and B must inherit from BoundingVolume
template<typename T, typename B>
class Partition
{
public:
    static void destroy(Partition* const partition, MemoryAllocator* const allocator)
    {
        partition->~Partition();
        operator delete(partition, 16, *allocator);
    }

private:
    static Logger& logger;

public:
    virtual ~Partition() throw()
    {
    }

protected:
    template<typename Y, typename V> friend class PartitionFactory;

    Partition(const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned)
        : _container(container)
    {
        for(auto obj : data) {
            if(appendable(obj, _container, pruned)) {
                _data.push_back(obj);
            }
        }

        // store the actual size
        _size = _data.size();

        // find the center of mass
        if(_size > 0) {
            calculate_center();
        }
    }

    explicit Partition(const std::list<std::shared_ptr<T>>& data)
        : _data(data)
    {
        // store the actual size
        _size = _data.size();

        // find the center of mass
        if(_size > 0) {
            calculate_center();
            calculate_container();
        }
    }

public:
    virtual size_t size() const final { return _size; }
    virtual const Point3& center_of_mass() const final { return _center_of_mass; }
    virtual const B& container() const final { return _container; }

    // NOTE: used to be intersect() and append_intersect() in python
    // returns a list of all objects that intersect with the object
    virtual void collide(const Intersectable& obj, std::list<std::shared_ptr<T>>& collided) const final
    {
        if(obj.intersect(_container)) {
            on_intersect(obj, collided);
        }
    }

    // NOTE: used to be remove() and append_remove() in python
    // same as intersect() but removes any objects that intersected with the object size is updated by this, but container is not
    virtual void remove(const Intersectable& obj, std::list<std::shared_ptr<T>>& collided) final
    {
        if(obj.intersect(_container)) {
            on_remove(obj, collided);
        }
    }

    // override this to traverse the partition
    virtual void traverse(std::list<std::shared_ptr<T>>& nodes) const
    {
        nodes = _data;
    }

private:
    virtual bool appendable(std::shared_ptr<T> obj, const BoundingVolume& container, std::list<std::shared_ptr<T>>& pruned) const final
    {
        // prune any objects not inside the container
        if(obj->bounds().intersect(container)) {
            return true;
        }

        pruned.push_back(obj);
        return false;
    }

    virtual void calculate_center() final
    {
        Point3 center(0.0f, 0.0f, 0.0f);
        for(auto obj : _data) {
            center += obj->position();
        }

        if(size() > 0) {
            center /= size();
        }

        _center_of_mass = center;
    }

    virtual void calculate_container() final
    {
        // find the radius of the container by finding the bounding point furthest from the center of mass
        float distance = 0.0f;
        std::shared_ptr<T> farthest;
        for(auto obj : _data) {
            float r = obj->bounds().radius_squared() < 1.0f ? obj->bounds().radius() : obj->bounds().radius_squared();
            float d = obj->position().distance_squared(_center_of_mass) + r;
            if(d > distance) {
                farthest = obj;
                distance = d;
            }
        }

        if(farthest) {
            distance = farthest->position().distance(_center_of_mass) + farthest->bounds().radius();
            _container = B(_center_of_mass, distance);
        } else {
            // TODO: radius of 1.0f cannot be right
            LOG_WARNING("Defaulting to generic container of radius 1!\n");
            _container = B(_center_of_mass, 1.0f);
        }
    }

protected:
    // override this to implement intersect checking
    virtual void on_intersect(const Intersectable& obj, std::list<std::shared_ptr<T>>& collided) const
    {
        for(auto d : _data) {
            if(obj.intersect(d->bounds())) {
                collided.push_back(d);
            }
        }
    }

    // override this to implement intersect checking and removal, should return the number of elements removed
    virtual size_t on_remove(const Intersectable& obj, std::list<std::shared_ptr<T>>& collided)
    {
        // intersect and remove local objects
        on_intersect(obj, collided);
        for(auto c : collided) {
            _data.remove(c);
        }

        // update our size
        size_t size = _size;
        _size = _data.size();
        return size - _size;
    }

protected:
    std::list<std::shared_ptr<T>> _data;
    size_t _size;

private:
    Point3 _center_of_mass;
    B _container;

private:
    Partition();
    DISALLOW_COPY_AND_ASSIGN(Partition);
};

template<typename T, typename B>
Logger& Partition<T, B>::logger(Logger::instance("energonsoftware.core.physics.partition.Partition"));

}

#endif
