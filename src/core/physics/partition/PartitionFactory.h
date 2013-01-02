#if !defined __PARTITIONFACTORY_H__
#define __PARTITIONFACTORY_H__

#include "Partition.h"
#include "KdTree.h"
#include "Octree.h"
#include "SphereTree.h"
#include "TreePartition.h"

namespace energonsoftware {

// T must inherit from Partitionable
template<typename T, typename B>
class PartitionFactory
{
public:
    static std::shared_ptr<Partition<T, B>> new_partition(const std::string& type, MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth=0) throw(PartitionError)
    {
        std::string scratch(boost::algorithm::to_lower_copy(type));
        if(scratch == "octree") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) Octree<T, B>(allocator, data, container, pruned, depth),
                boost::bind(&Octree<T, B>::destroy, _1, allocator));
        } else if(scratch == "spheretree") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) SphereTree<T, B>(allocator, data, container, pruned, depth),
                boost::bind(&SphereTree<T, B>::destroy, _1, allocator));
        } else if(scratch == "kdtree3") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) KdTree3<T, B>(allocator, data, container, pruned, depth),
                boost::bind(&KdTree3<T, B>::destroy, _1, allocator));
        } else if(scratch == "tree") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) TreePartition<T, B>(allocator, data, container, pruned, depth),
                boost::bind(&TreePartition<T, B>::destroy, _1, allocator));
        } else if(scratch == "flat") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) Partition<T, B>(data, container, pruned),
                boost::bind(&Partition<T, B>::destroy, _1, allocator));
        }
        throw PartitionError("Unknown partition type: " + type);
    }

    static std::shared_ptr<Partition<T, B>> new_partition(const std::string& type, MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, const B& container, unsigned int depth=0) throw(PartitionError)
    {
        std::list<std::shared_ptr<T>> pruned;
        return new_partition(type, allocator, data, container, pruned, depth);
    }

    static std::shared_ptr<Partition<T, B>> new_partition(const std::string& type, MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, unsigned int depth=0) throw(PartitionError)
    {
        std::string scratch(boost::algorithm::to_lower_copy(type));
        if(scratch == "octree") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) Octree<T, B>(allocator, data, depth),
                boost::bind(&Octree<T, B>::destroy, _1, allocator));
        } else if(scratch == "spheretree") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) SphereTree<T, B>(allocator, data, depth),
                boost::bind(&SphereTree<T, B>::destroy, _1, allocator));
        } else if(scratch == "kdtree3") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) KdTree3<T, B>(allocator, data, depth),
                boost::bind(&KdTree3<T, B>::destroy, _1, allocator));
        } else if(scratch == "tree") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) TreePartition<T, B>(allocator, data, depth),
                boost::bind(&TreePartition<T, B>::destroy, _1, allocator));
        } else if(scratch == "flat") {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) Partition<T, B>(data),
                boost::bind(&Partition<T, B>::destroy, _1, allocator));
        }
        throw PartitionError("Unknown partition type: " + type);
    }

private:
    PartitionFactory();

    virtual ~PartitionFactory() throw()
    {
    }
};

}

#endif
