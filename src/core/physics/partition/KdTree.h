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
    virtual ~KdTree() throw()
    {
    }

protected:
    template<typename Y, typename V> friend class PartitionFactory;

    KdTree(const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : TreePartition<T, B>(data, container, pruned, depth)
    {
    }

    KdTree(const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : TreePartition<T, B>(data, depth)
    {
    }

public:
    virtual size_t dimensions() const final { return Dim; }

private:
    virtual void on_build_subtrees() override
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
            new KdTree<T, B, Dim>(subtree, TreePartition<T, B>::depth() - 1)));

        subtree = std::list<std::shared_ptr<T>>(data.begin() + median + 1, data.end());
        TreePartition<T, B>::_subtrees.push_back(std::shared_ptr<TreePartition<T, B>>(
            new KdTree<T, B, Dim>(subtree, TreePartition<T, B>::depth() - 1)));
    }

private:
    KdTree();
    DISALLOW_COPY_AND_ASSIGN(KdTree);
};

template<typename T, typename B>
class KdTree3 : public KdTree<T, B, 3>
{
public:
    virtual ~KdTree3() throw()
    {
    }

private:
    template<typename Y, typename V> friend class PartitionFactory;

    KdTree3(const std::list<std::shared_ptr<T>>& data, const B& container, std::list<std::shared_ptr<T>>& pruned, unsigned int depth)
        : KdTree<T, B, 3>(data, container, pruned, depth)
    {
    }

    KdTree3(const std::list<std::shared_ptr<T>>& data, unsigned int depth)
        : KdTree<T, B, 3>(data, depth)
    {
    }

private:
    KdTree3();
    DISALLOW_COPY_AND_ASSIGN(KdTree3);
};

}

#endif
