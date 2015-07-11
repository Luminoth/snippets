#include "src/pch.h"
#include <boost/tokenizer.hpp>
#include "Version.h"

namespace energonsoftware {

Version::Version(const std::string& version)
    : _major(0), _minor(0), _maintenance(0)
{
    boost::char_separator<char> sep(".");
    boost::tokenizer<boost::char_separator<char>> tok(version, sep);

    boost::tokenizer<boost::char_separator<char>>::iterator it = tok.begin();
    if(it != tok.end()) {
        _major = std::atoi(it->c_str());
        ++it;
    }

    if(it != tok.end()) {
        _minor = std::atoi(it->c_str());
        ++it;
    }

    if(it != tok.end()) {
        _maintenance = std::atoi(it->c_str());
        ++it;
    }
}

Version::~Version() noexcept
{
}

std::string Version::str() const
{
    std::stringstream ss;
    ss << _major << "." << _minor << "." << _maintenance;
    return ss.str();
}

bool Version::operator<(const Version& rhs) const
{
    return _major == rhs._major
        ? (_minor == rhs._minor
            ? _maintenance < rhs._maintenance
            : _minor < rhs._minor)
        : _major < rhs._major;
}

bool Version::operator<=(const Version& rhs) const
{
    return (*this) < rhs || (*this) == rhs;
}

bool Version::operator>(const Version& rhs) const
{
    return rhs < (*this);
}

bool Version::operator>=(const Version& rhs) const
{
    return (*this) > rhs || (*this) == rhs;
}

bool Version::operator==(const Version& rhs) const
{
    return _major == rhs._major && _minor == rhs._minor && _maintenance == rhs._maintenance;
}

bool Version::operator!=(const Version& rhs) const
{
    return !((*this) == rhs);
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class VersionTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(VersionTest);
    CPPUNIT_TEST_SUITE_END();

public:
    VersionTest() : CppUnit::TestFixture() {}
    virtual ~VersionTest() noexcept {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(VersionTest);

#endif
