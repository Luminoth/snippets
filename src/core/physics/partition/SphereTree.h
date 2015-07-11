#if !defined __SPHERETREE_H__
#define __SPHERETREE_H__

#include "TreePartition.h"

namespace energonsoftware {

// TODO: rename this... it doesn't *really* require spheres
template<typename T, typename B>
class SphereTree : public TreePartition<T, B>
{
public:
    virtual ~SphereTree() noexcept
    {
    }

private:
    template<typename Y, typename V> friend class PartitionFactory;

    SphereTree(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : TreePartition<T, B>(allocator, data, container, pruned, depth)
    {
    }

    SphereTree(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : TreePartition<T, B>(allocator, data, depth)
    {
    }

private:
    virtual void on_build_subtrees(MemoryAllocator* const allocator) override
    {
        std::list<std::shared_ptr<T>> left;
        std::list<std::shared_ptr<T>> right;

        // find a decent normal to work with
        Vector norm(TreePartition<T, B>::center_of_mass() - TreePartition<T, B>::container().center());
        norm = norm.length() == 0 ? Vector::random() : norm.normalized();

        // partition the objects
        for(typename std::list<std::shared_ptr<T>>::const_iterator it=TreePartition<T, B>::_data.begin();
            it != TreePartition<T, B>::_data.end(); ++it)
        {
            std::shared_ptr<T> obj(*it);
            const Vector diff(obj->position() - TreePartition<T, B>::container().center());

            if((norm * diff) > 0.0f) {
                left.push_back(obj);
            } else {
                right.push_back(obj);
            }
        }

        // only leaves hold data
        TreePartition<T, B>::_data.clear();

        // subtrees should always calculate their container
        if(left.size() > 0) {
            TreePartition<T, B>::_subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(
                new(16, *allocator) SphereTree(allocator, left, TreePartition<T, B>::depth() - 1),
                MemoryAllocator_delete_aligned<SphereTree<T, B>, 16>(allocator)));
        }

        if(right.size() > 0) {
            TreePartition<T, B>::_subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(
                new(16, *allocator) SphereTree(allocator, right, TreePartition<T, B>::depth() - 1),
                MemoryAllocator_delete_aligned<SphereTree<T, B>, 16>(allocator)));
        }
    }

private:
    SphereTree() = delete;
    DISALLOW_COPY_AND_ASSIGN(SphereTree);
};

}

#endif
