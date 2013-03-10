#if !defined __KDTREE_H__
#define __KDTREE_H__

#include "TreePartition.h"

namespace energonsoftware {

template<typename T>
struct AxisCmp
{
public:
    AxisCmp(unsigned int axis)
        : _axis(axis)
    {
    }

public:
    bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) throw(std::runtime_error)
    {
        if(_axis == 0) {
            return lhs->position().x() < rhs->position().x();
        } else if(_axis == 1) {
            return lhs->position().y() < rhs->position().y();
        } else if(_axis == 2) {
            return lhs->position().z() < rhs->position().z();
        }
        throw std::runtime_error("Cannot compare more than 3 axis!");
    }

private:
    unsigned int _axis;
};

template<typename T, typename B, size_t Dim>
class KdTree : public TreePartition<T, B>
{
public:
    static void destroy(KdTree* const partition, MemoryAllocator* const allocator)
    {
        partition->~KdTree();
        operator delete(partition, 16, *allocator);
    }

public:
    virtual ~KdTree() noexcept
    {
    }

protected:
    template<typename Y, typename V> friend class PartitionFactory;

    KdTree(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : TreePartition<T, B>(allocator, data, container, pruned, depth)
    {
    }

    KdTree(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : TreePartition<T, B>(allocator, data, depth)
    {
    }

public:
    virtual size_t dimensions() const final { return Dim; }

private:
    virtual void on_build_subtrees(MemoryAllocator* const allocator) override
    {
        // sort the points
        unsigned int axis = TreePartition<T, B>::depth() % dimensions();
        TreePartition<T, B>::_data.sort(AxisCmp<T>(axis));

        std::vector<std::shared_ptr<T>> data(TreePartition<T, B>::_data.begin(), TreePartition<T, B>::_data.end());

        // get the median
        unsigned int median = data.size() >> 1;

        // our data is the median
        TreePartition<T, B>::_data.clear();
        TreePartition<T, B>::_data.push_back(data[median]);

        // subtrees always calculate their container
        std::list<std::shared_ptr<T>> subtree(data.begin(), data.begin() + median);
        TreePartition<T, B>::_subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(
            new(16, *allocator) KdTree<T, B, Dim>(allocator, subtree, TreePartition<T, B>::depth() - 1),
            std::bind(&KdTree<T, B, Dim>::destroy, std::placeholders::_1, allocator)));

        subtree = std::list<std::shared_ptr<T>>(data.begin() + median + 1, data.end());
        TreePartition<T, B>::_subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(
            new(16, *allocator) KdTree<T, B, Dim>(allocator, subtree, TreePartition<T, B>::depth() - 1),
            std::bind(&KdTree<T, B, Dim>::destroy, std::placeholders::_1, allocator)));
    }

private:
    KdTree();
    DISALLOW_COPY_AND_ASSIGN(KdTree);
};

template<typename T, typename B>
class KdTree3 : public KdTree<T, B, 3>
{
public:
    static void destroy(KdTree3* const partition, MemoryAllocator* const allocator)
    {
        partition->~KdTree3();
        operator delete(partition, 16, *allocator);
    }

public:
    virtual ~KdTree3() noexcept
    {
    }

private:
    template<typename Y, typename V> friend class PartitionFactory;

    KdTree3(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : KdTree<T, B, 3>(allocator, data, container, pruned, depth)
    {
    }

    KdTree3(MemoryAllocator* const allocator, const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : KdTree<T, B, 3>(allocator, data, depth)
    {
    }

private:
    KdTree3();
    DISALLOW_COPY_AND_ASSIGN(KdTree3);
};

}

#endif
