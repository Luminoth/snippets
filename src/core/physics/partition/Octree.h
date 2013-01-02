#if !defined __OCTREE_H__
#define __OCTREE_H__

#include "TreePartition.h"

namespace energonsoftware {

template<typename T, typename B>
class Octree : public TreePartition<T, B>
{
public:
    virtual ~Octree() throw()
    {
    }

private:
    template<typename Y, typename V> friend class PartitionFactory;

    Octree(const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : TreePartition<T, B>(data, container, pruned, depth)
    {
    }

    Octree(const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : TreePartition<T, B>(data, depth)
    {
    }

private:
    virtual void on_build_subtrees() override
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
                    new Octree(*it, TreePartition<T, B>::depth() - 1)));
            }
        }
    }

private:
    Octree();
    DISALLOW_COPY_AND_ASSIGN(Octree);
};

}

#endif
