#include "src/pch.h"

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class UpdatePropertyTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(UpdatePropertyTest);
    CPPUNIT_TEST_SUITE_END();

public:
    UpdatePropertyTest() : CppUnit::TestFixture() {}
    virtual ~UpdatePropertyTest() noexcept {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(UpdatePropertyTest);

#endif
