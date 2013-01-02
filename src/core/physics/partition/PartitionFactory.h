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
    static Partition<T, B>* new_partition(const std::string& type, const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth=0) throw(PartitionError)
    {
        std::string scratch(boost::algorithm::to_lower_copy(type));
        if(scratch == "octree") {
            return new Octree<T, B>(data, container, pruned, depth);
        } else if(scratch == "spheretree") {
            return new SphereTree<T, B>(data, container, pruned, depth);
        } else if(scratch == "kdtree3") {
            return new KdTree3<T, B>(data, container, pruned, depth);
        } else if(scratch == "tree") {
            return new TreePartition<T, B>(data, container, pruned, depth);
        } else if(scratch == "flat") {
            return new Partition<T, B>(data, container, pruned);
        }
        throw PartitionError("Unknown partition type: " + type);
    }

    static Partition<T, B>* new_partition(const std::string& type, const std::list<std::shared_ptr<T>>& data, const B& container, unsigned int depth=0) throw(PartitionError)
    {
        std::list<std::shared_ptr<T>> pruned;
        return new_partition(type, data, container, pruned, depth);
    }

    static Partition<T, B>* new_partition(const std::string& type, const std::list<std::shared_ptr<T>>& data, unsigned int depth=0) throw(PartitionError)
    {
        std::string scratch(boost::algorithm::to_lower_copy(type));
        if(scratch == "octree") {
            return new Octree<T, B>(data, depth);
        } else if(scratch == "spheretree") {
            return new SphereTree<T, B>(data, depth);
        } else if(scratch == "kdtree3") {
            return new KdTree3<T, B>(data, depth);
        } else if(scratch == "tree") {
            return new TreePartition<T, B>(data, depth);
        } else if(scratch == "flat") {
            return new Partition<T, B>(data);
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
