#if !defined __TREEPARTITION_H__
#define __TREEPARTITION_H__

#include "Partition.h"

namespace energonsoftware {

// for a TreePartition, size() == (sum(_subtrees.size()) + count())
// use count to get number objects a given node holds
template<typename T, typename B>
class TreePartition : public Partition<T, B>
{
public:
    typedef std::shared_ptr<TreePartition<T, B>> Subtree;
    typedef std::list<Subtree> Subtrees;

public:
    static void destroy(TreePartition* const partition, MemoryAllocator* const allocator)
    {
        partition->~TreePartition();
        operator delete(partition, 16, *allocator);
    }

public:
    virtual ~TreePartition() noexcept
    {
    }

protected:
    template<typename Y, typename V> friend class PartitionFactory;

    TreePartition(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : Partition<T, B>(data, container, pruned), _subtrees(), _depth(depth), _count(0)
    {
        if(_depth >= 1 && Partition<T, B>::size() > 1) {
            on_build_subtrees(allocator);
        }
        _count = Partition<T, B>::_data.size();
    }

    TreePartition(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : Partition<T, B>(data), _subtrees(), _depth(depth), _count(0)
    {
        if(_depth >= 1 && Partition<T, B>::size() > 1) {
            on_build_subtrees(allocator);
        }
        _count = Partition<T, B>::_data.size();
    }

public:
    virtual unsigned int depth() const final { return _depth; }
    virtual size_t count() const final { return _count; }

    // post-order dfs
    /* TODO: make this work */
    virtual void depth_first_traverse(std::list<std::shared_ptr<T>>& nodes) const final
    {
        // visit subtrees (left first)
        for(const Subtree& tree : _subtrees) {
            tree->depth_first_traverse(nodes);
        }

        // visit ourself
        for(typename std::list<std::shared_ptr<T>>::const_iterator it=TreePartition<T, B>::_data.begin();
            it != TreePartition<T, B>::_data.end(); ++it)
        {
            std::shared_ptr<T> obj(*it);
            nodes.push_back(obj);
        }
    }

    virtual void breadth_first_traverse(std::list<std::shared_ptr<T>>& nodes) const final
    {
        // push the root onto the heap
        std::deque<const TreePartition<T, B>*> heap;
        heap.push_back(this);

        while(!heap.size() == 0) {
            // pop the next node
            const TreePartition<T, B>* node = heap.front();
            heap.pop_front();

            // visit the node
            for(auto obj : node->_data) {
                nodes.push_back(obj);
            }

            // push subtrees onto the heap (left first)
            for(const Subtree& tree : node->_subtrees) {
                heap.push_back(tree.get());
            }
        }
    }

    // defaults to bfs
    virtual void traverse(std::list<std::shared_ptr<T>>& nodes) const override
    {
        nodes.clear();

        //return depth_first_traverse(nodes);
        return breadth_first_traverse(nodes);
    }

protected:
    // override this to implement intersect checking
    virtual void on_intersect(const Intersectable& obj, std::list<std::shared_ptr<T>>& collided) const override
    {
        // have to repeat this check here for subtrees
        if(obj.intersect(Partition<T, B>::container())) {
            // intersect local objects
            Partition<T, B>::on_intersect(obj, collided);

            // add subtree data
            for(const Subtree& tree : _subtrees) {
                tree->on_intersect(obj, collided);
            }
        }
    }

    // override this to implement intersect checking and removal, should return the number of elements removed
    virtual size_t on_remove(const Intersectable& obj, std::list<std::shared_ptr<T>>& collided) override
    {
        // have to repeat this check here for subtrees
        if(obj.intersect(Partition<T, B>::container())) {
            // intersect and remove local objects
            // can't use Partition::on_remove because it will call TreePartition::on_intersect()
            Partition<T, B>::on_intersect(obj, collided);
            for(auto c : collided) {
                Partition<T, B>::_data.remove(c);
            }

            // update our count
            size_t count = _count;
            _count = Partition<T, B>::_data.size();

            // intersect and remove subtrees
            size_t removed = count - _count;
            for(Subtree& tree : _subtrees) {
                removed += tree->on_remove(obj, collided);
            }

            // update our size
            size_t size = Partition<T, B>::_size;
            Partition<T, B>::_size -= removed;
            return size - Partition<T, B>::_size;
        }
        return 0;
    }

    // override this
    virtual void on_build_subtrees(MemoryAllocator* const allocator)
    {
        std::deque<std::shared_ptr<T>> data;
        for(typename std::list<std::shared_ptr<T>>::const_iterator it=TreePartition<T, B>::_data.begin();
            it != TreePartition<T, B>::_data.end(); ++it)
        {
            std::shared_ptr<T> obj(*it);
            data.push_back(obj);
        }

        // our data is the first item
        Partition<T, B>::_data.clear();
        Partition<T, B>::_data.push_back(data.front());
        data.pop_front();

        // split everything else in 'half'
        unsigned int median = data.size() >> 1;

        // subtrees always calculate their container
        std::list<std::shared_ptr<T>> subtree(data.begin(), data.begin() + median);
        _subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(new(16, *allocator) TreePartition(allocator, subtree, depth() - 1),
            std::bind(&TreePartition<T, B>::destroy, std::placeholders::_1, allocator)));

        subtree = std::list<std::shared_ptr<T>>(data.begin() + median, data.end());
        _subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(new(16, *allocator) TreePartition(allocator, subtree, depth() - 1),
            std::bind(&TreePartition<T, B>::destroy, std::placeholders::_1, allocator)));
    }

protected:
    Subtrees _subtrees;

private:
    unsigned int _depth;
    size_t _count;

private:
    TreePartition();
    DISALLOW_COPY_AND_ASSIGN(TreePartition);
};

}

#endif
