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
        if("octree" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) Octree<T, B>(allocator, data, container, pruned, depth),
                MemoryAllocator_delete_aligned<Octree<T, B>, 16>(allocator));
        } else if("spheretree" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) SphereTree<T, B>(allocator, data, container, pruned, depth),
                MemoryAllocator_delete_aligned<SphereTree<T, B>, 16>(allocator));
        } else if("kdtree3" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) KdTree3<T, B>(allocator, data, container, pruned, depth),
                MemoryAllocator_delete_aligned<KdTree3<T, B>, 16>(allocator));
        } else if("tree" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) TreePartition<T, B>(allocator, data, container, pruned, depth),
                MemoryAllocator_delete_aligned<TreePartition<T, B>, 16>(allocator));
        } else if("flat" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) Partition<T, B>(data, container, pruned),
                MemoryAllocator_delete_aligned<Partition<T, B>, 16>(allocator));
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
        if("octree" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) Octree<T, B>(allocator, data, depth),
                energonsoftware::MemoryAllocator_delete_aligned<Octree<T, B>, 16>(allocator));
        } else if("spheretree" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) SphereTree<T, B>(allocator, data, depth),
                energonsoftware::MemoryAllocator_delete_aligned<SphereTree<T, B>, 16>(allocator));
        } else if("kdtree3" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) KdTree3<T, B>(allocator, data, depth),
                energonsoftware::MemoryAllocator_delete_aligned<KdTree3<T, B>, 16>(allocator));
        } else if("tree" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) TreePartition<T, B>(allocator, data, depth),
                energonsoftware::MemoryAllocator_delete_aligned<TreePartition<T, B>, 16>(allocator));
        } else if("flat" == scratch) {
            return std::shared_ptr<Partition<T, B>>(new(16, *allocator) Partition<T, B>(data),
                energonsoftware::MemoryAllocator_delete_aligned<Partition<T, B>, 16>(allocator));
        }
        throw PartitionError("Unknown partition type: " + type);
    }

private:
    PartitionFactory();

    virtual ~PartitionFactory() noexcept
    {
    }
};

}

#endif
