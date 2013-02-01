#include "src/pch.h"

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class RandomTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(RandomTest);
        CPPUNIT_TEST(test_choice);
    CPPUNIT_TEST_SUITE_END();

public:
    RandomTest() : CppUnit::TestFixture() {}
    virtual ~RandomTest() throw() {}

public:
    void test_choice()
    {
        std::vector<int> v({ 0 });

        std::deque<int> d({ 0 });
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RandomTest);

#endif
