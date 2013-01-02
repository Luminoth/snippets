#if !defined __OCTREE_H__
#define __OCTREE_H__

#include "TreePartition.h"

namespace energonsoftware {

template<typename T, typename B>
class Octree : public TreePartition<T, B>
{
public:
    static void destroy(Octree* const partition, MemoryAllocator* const allocator)
    {
        partition->~Octree();
        operator delete(partition, 16, *allocator);
    }

public:
    virtual ~Octree() throw()
    {
    }

private:
    template<typename Y, typename V> friend class PartitionFactory;

    Octree(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : TreePartition<T, B>(allocator, data, container, pruned, depth)
    {
    }

    Octree(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : TreePartition<T, B>(allocator, data, depth)
    {
    }

private:
    virtual void on_build_subtrees(MemoryAllocator* const allocator) override
    {
        std::vector<std::list<std::shared_ptr<T>>> su(8);
        for(typename std::list<std::shared_ptr<T>>::const_iterator it=TreePartition<T, B>::_data.begin();
            it != TreePartition<T, B>::_data.end(); ++it)
        {
            std::shared_ptr<T> obj(*it);
            const Vector diff(obj->position() - TreePartition<T, B>::container().center());

            int x = diff.x() > 0.0f ? 1 : 0;
            int y = diff.y() > 0.0f ? 2 : 0;
            int z = diff.z() > 0.0f ? 4 : 0;
            su[x+y+z].push_back(obj);
        }

        // only leaves hold data
        TreePartition<T, B>::_data.clear();

        // subtrees should always calculate their container
        for(typename std::vector<std::list<std::shared_ptr<T>>>::const_iterator it=su.begin(); it != su.end(); ++it) {
            if(it->size() > 0) {
                TreePartition<T, B>::_subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(
                    new(16, *allocator) Octree(allocator, *it, TreePartition<T, B>::depth() - 1),
                    boost::bind(&Octree<T, B>::destroy, _1, allocator)));
            }
        }
    }

private:
    Octree();
    DISALLOW_COPY_AND_ASSIGN(Octree);
};

}

#endif
