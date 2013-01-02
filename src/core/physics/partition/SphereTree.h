#if !defined __SPHERETREE_H__
#define __SPHERETREE_H__

#include "TreePartition.h"

namespace energonsoftware {

// TODO: rename this... it doesn't *really* require spheres
template<typename T, typename B>
class SphereTree : public TreePartition<T, B>
{
public:
    virtual ~SphereTree() throw()
    {
    }

private:
    template<typename Y, typename V> friend class PartitionFactory;

    SphereTree(const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : TreePartition<T, B>(data, container, pruned, depth)
    {
    }

    SphereTree(const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : TreePartition<T, B>(data, depth)
    {
    }

public:
    /*virtual std::shared_ptr<TreePartition<T, B>>& left() final { return _subtrees[0]; }
    virtual std::shared_ptr<TreePartition<T, B>>& right() final { return _subtrees[1]; }*/

private:
    virtual void on_build_subtrees() override
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
                new SphereTree(left, TreePartition<T, B>::depth() - 1)));
        }

        if(right.size() > 0) {
            TreePartition<T, B>::_subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(
                new SphereTree(right, TreePartition<T, B>::depth() - 1)));
        }
    }

private:
    SphereTree();
    DISALLOW_COPY_AND_ASSIGN(SphereTree);
};

}

#endif
