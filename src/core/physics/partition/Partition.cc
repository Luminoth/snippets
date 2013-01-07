#include "src/pch.h"
#include "Partition.h"

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"
#include "src/core/physics/BoundingSphere.h"
#include "src/core/util/Random.h"
#include "src/core/util/util.h"
#include "Partitionable.h"
#include "PartitionFactory.h"

class TestPartitionable : public energonsoftware::Partitionable
{
public:
    static void destroy(TestPartitionable* const partitionable, energonsoftware::MemoryAllocator* const allocator)
    {
        partitionable->~TestPartitionable();
        operator delete(partitionable, 16, *allocator);
    }

public:
    TestPartitionable(const energonsoftware::Position& position, float radius, const std::string& name)
        : Partitionable(), _position(position), _relative_bounds(energonsoftware::Point3(), radius), _absolute_bounds(position, radius), _name(name)
    {
    }

    virtual ~TestPartitionable() throw()
    {
    }

public:
    virtual const energonsoftware::Position& position() const override { return _position; }
    virtual const energonsoftware::BoundingVolume& relative_bounds() const override { return _relative_bounds; }
    virtual const energonsoftware::BoundingVolume& absolute_bounds() const override { return _absolute_bounds; }
    const std::string& name() const { return _name; }

private:
    energonsoftware::Position _position;
    energonsoftware::BoundingSphere _relative_bounds, _absolute_bounds;
    std::string _name;

private:
    TestPartitionable();
    DISALLOW_COPY_AND_ASSIGN(TestPartitionable);
};

bool ResultCmp(const std::pair<std::string, energonsoftware::Point3>& lhs, const std::pair<std::string, energonsoftware::Point3>& rhs)
{
    return lhs.second < rhs.second;
}

class PartitionTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(PartitionTest);
        CPPUNIT_TEST(test_trivial);
        CPPUNIT_TEST(test_span);
        CPPUNIT_TEST(test_stress);
        CPPUNIT_TEST(test_speed);
    CPPUNIT_TEST_SUITE_END();

private:
    typedef std::unordered_map<std::string, size_t> ResultMap;
    static energonsoftware::Logger& logger;

public:
    PartitionTest() : CppUnit::TestFixture() {}
    virtual ~PartitionTest() throw() {}

public:
    void setUp()
    {
        _allocator = energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, 100 * 1024 * 1024);

        _partition_types.push_back("flat");
        _partition_types.push_back("tree");
        _partition_types.push_back("spheretree");
        _partition_types.push_back("octree");
        _partition_types.push_back("kdtree3");
    }

    void test_trivial()
    {
        unsigned int depth = 5;

        // create the data set
        std::list<std::shared_ptr<TestPartitionable>> data;
        data.push_back(std::shared_ptr<TestPartitionable>(
            new(16, *_allocator) TestPartitionable(energonsoftware::Point3(0.0f, 0.0f, 0.0f), 3.0f, "Jonah"),
            std::bind(&TestPartitionable::destroy, std::placeholders::_1, _allocator.get())));

        // test each partition type
        for(const std::string& type : _partition_types) {
            std::shared_ptr<energonsoftware::Partition<TestPartitionable, energonsoftware::BoundingSphere>> partition(
                energonsoftware::PartitionFactory<TestPartitionable, energonsoftware::BoundingSphere>::new_partition(type, _allocator.get(), data, depth));

            std::list<std::shared_ptr<TestPartitionable>> collided;
            partition->collide(energonsoftware::BoundingSphere(energonsoftware::Point3(1.0f, 0.0f, 0.0f), 1.0f), collided);

            CPPUNIT_ASSERT_EQUAL_MESSAGE(type, static_cast<size_t>(1), collided.size());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type, (*data.begin())->name(), (*collided.begin())->name());
        }
    }

    void test_span()
    {
        unsigned int depth = 10;

        // create the data set
        std::list<std::shared_ptr<TestPartitionable>> data;
        data.push_back(std::shared_ptr<TestPartitionable>(
            new(16, *_allocator) TestPartitionable(energonsoftware::Point3(100.0f, 0.0f, 100.0f), 1.0f, "Adam"),
            std::bind(&TestPartitionable::destroy, std::placeholders::_1, _allocator.get())));
        data.push_back(std::shared_ptr<TestPartitionable>(
            new(16, *_allocator) TestPartitionable(energonsoftware::Point3(-100.0f, 0.0f, 100.0f), 1.0f, "Pete"),
            std::bind(&TestPartitionable::destroy, std::placeholders::_1, _allocator.get())));
        data.push_back(std::shared_ptr<TestPartitionable>(
            new(16, *_allocator) TestPartitionable(energonsoftware::Point3(100.0f, 0.0f, -100.0f), 1.0f, "Mitchell"),
            std::bind(&TestPartitionable::destroy, std::placeholders::_1, _allocator.get())));
        data.push_back(std::shared_ptr<TestPartitionable>(
            new(16, *_allocator) TestPartitionable(energonsoftware::Point3(-100.0f, 0.0f, -100.0f), 1.0f, "Thomas"),
            std::bind(&TestPartitionable::destroy, std::placeholders::_1, _allocator.get())));

        // test each partition type
        for(const std::string& type : _partition_types) {
            std::shared_ptr<energonsoftware::Partition<TestPartitionable, energonsoftware::BoundingSphere>> partition(
                energonsoftware::PartitionFactory<TestPartitionable, energonsoftware::BoundingSphere>::new_partition(type, _allocator.get(), data, depth));

            std::list<std::shared_ptr<TestPartitionable>> collided;
            partition->collide(energonsoftware::BoundingSphere(energonsoftware::Point3(0.0f, 0.0f, 0.0f), 1.0f), collided);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type, static_cast<size_t>(0), collided.size());

            collided.clear();
            partition->collide(energonsoftware::BoundingSphere(energonsoftware::Point3(-99.9f, 0.0f, 100.3f), 1.0f), collided);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type + ", Pete", static_cast<size_t>(1), collided.size());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type, std::string("Pete"), (*collided.begin())->name());

            collided.clear();
            partition->collide(energonsoftware::BoundingSphere(energonsoftware::Point3(99.9f, 0.0f, 100.3f), 1.0f), collided);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type + ", Adam", static_cast<size_t>(1), collided.size());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type, std::string("Adam"), (*collided.begin())->name());

            collided.clear();
            partition->collide(energonsoftware::BoundingSphere(energonsoftware::Point3(99.9f, 0.0f, -100.3f), 1.0f), collided);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type + ", Mitchell", static_cast<size_t>(1), collided.size());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type, std::string("Mitchell"), (*collided.begin())->name());

            collided.clear();
            partition->collide(energonsoftware::BoundingSphere(energonsoftware::Point3(-99.9f, 0.0f, -100.3f), 1.0f), collided);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type + ", Thomas", static_cast<size_t>(1), collided.size());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(type, std::string("Thomas"), (*collided.begin())->name());
        }
    }

    void test_stress()
    {
        unsigned int depth = 10;

        // create the data set
        std::list<std::shared_ptr<TestPartitionable>> data;
        for(int i=0; i<100000; ++i) {
            data.push_back(std::shared_ptr<TestPartitionable>(
                new(16, *_allocator) TestPartitionable(
                    energonsoftware::Point3(energonsoftware::Random::uniform<float>(0.0f, 1000.0f),
                        energonsoftware::Random::uniform<float>(0.0f, 1000.0f),
                        energonsoftware::Random::uniform<float>(0.0f, 1000.0f)),
                    energonsoftware::Random::uniform<float>(0.0f, 3.0f),
                    "G_" + i),
                std::bind(&TestPartitionable::destroy, std::placeholders::_1, _allocator.get())));
        }

        // test each partition type
        for(const std::string& type : _partition_types) {
            std::shared_ptr<energonsoftware::Partition<TestPartitionable, energonsoftware::BoundingSphere>> partition(
                energonsoftware::PartitionFactory<TestPartitionable, energonsoftware::BoundingSphere>::new_partition(type, _allocator.get(), data, depth));

            energonsoftware::BoundingSphere obj(energonsoftware::Point3(0.0f, 0.0f, 0.0f), 1000.0f);
            std::list<std::shared_ptr<TestPartitionable>> collided;
            partition->collide(obj, collided);
            CPPUNIT_ASSERT_MESSAGE(type, collided.size() > 0);
        }
    }

    void test_speed()
    {
        unsigned int depth = 10;
        int dnum = 6000, cnum = 400;

        // create the data set
        std::list<std::shared_ptr<TestPartitionable>> data;
        for(int i=0; i<dnum; ++i) {
            data.push_back(std::shared_ptr<TestPartitionable>(
                new(16, *_allocator) TestPartitionable(
                    energonsoftware::Point3(energonsoftware::Random::uniform<float>(0.0f, 1000.0f),
                        energonsoftware::Random::uniform<float>(0.0f, 1000.0f),
                        energonsoftware::Random::uniform<float>(0.0f, 1000.0f)),
                    energonsoftware::Random::uniform<float>(0.0f, 3.0f),
                    "G_" + i),
                std::bind(&TestPartitionable::destroy, std::placeholders::_1, _allocator.get())));
        }

        // create the intersect set
        std::vector<energonsoftware::Point3> cents;
        for(int i=0; i<cnum; ++i) {
            cents.push_back(energonsoftware::Point3(energonsoftware::Random::uniform<float>(0.0f, 1000.0f),
                energonsoftware::Random::uniform<float>(0.0f, 1000.0f),
                energonsoftware::Random::uniform<float>(0.0f, 1000.0f)));
        }

        // test each partition type
        ResultMap lengths;
        for(const std::string& type : _partition_types) {
            std::list<std::pair<energonsoftware::Point3, std::list<std::pair<std::string, energonsoftware::Point3>>>> results;

            /*if(type == "flat" || type == "tree") {
                LOG_INFO("Flat and Tree partitions disabled for now...\n");
                continue;
            }*/

            LOG_INFO("Starting speed test for type " << type << "...\n");

            double start = energonsoftware::get_time();
            std::shared_ptr<energonsoftware::Partition<TestPartitionable, energonsoftware::BoundingSphere>> partition(
                energonsoftware::PartitionFactory<TestPartitionable, energonsoftware::BoundingSphere>::new_partition(type, _allocator.get(), data, depth));
            double cons = energonsoftware::get_time();
            LOG_INFO("Contructed in " << cons - start << " seconds\n");

            start = energonsoftware::get_time();
            for(int m=0; m<cnum; ++m) {
                std::list<std::shared_ptr<TestPartitionable>> collided;
                partition->collide(energonsoftware::BoundingSphere(cents[m], 1.0f), collided);
                if(collided.size() > 0) {
                    std::list<std::pair<std::string, energonsoftware::Point3>> res;
                    for(auto x : collided) {
                        res.push_back(std::pair<std::string, energonsoftware::Point3>(x->name(), x->position()));
                    }
                    res.sort(ResultCmp);
                    results.push_back(std::pair<energonsoftware::Point3, std::list<std::pair<std::string, energonsoftware::Point3>>>(cents[m], res));
                }
            }
            double fin = energonsoftware::get_time();
            LOG_INFO("Finished intersections in " << fin - start << " seconds\n");

            lengths[type] = results.size();
        }

        size_t flatlen = lengths["flat"];
        for(const auto& v : lengths) {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(v.first, v.second, flatlen);
        }
    }

private:
    std::list<std::string> _partition_types;
    std::shared_ptr<energonsoftware::MemoryAllocator> _allocator;
};

energonsoftware::Logger& PartitionTest::logger(energonsoftware::Logger::instance("energonsoftware.core.physics.partition.PartitionTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(PartitionTest);

#endif
