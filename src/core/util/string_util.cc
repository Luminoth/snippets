#include "src/pch.h"
#include <boost/tokenizer.hpp>
#include "string_util.h"

namespace energonsoftware {

void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
    boost::char_separator<char> sep(delimiters.c_str());
    boost::tokenizer<boost::char_separator<char>> tok(str, sep);
    for(const std::string& token : tok) {
        tokens.push_back(token);
    }
}

template<> std::string to_string<bool>(const bool& t)
{
    return t ? "true" : "false";
}

std::string trim_all(const std::string& str)
{
    std::string ret;
    const size_t len = str.length();
    size_t i=0;
    while(i < len) {
        if(!std::isspace(str[i])) {
            ret += str[i];
        }
        i++;
    }
    return ret;
}

std::string trim_sharp_comment(const std::string& str)
{
    size_t pos = str.find_first_of('#');
    if(std::string::npos != pos) {
        return str.substr(0, pos);
    }
    return str;
}

std::string operator+(char* str1, const std::string& str2)
{
    return std::string(str1) + str2;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class StringTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(StringTest);
        CPPUNIT_TEST(test_tokenize);
    CPPUNIT_TEST_SUITE_END();

public:
    StringTest() : CppUnit::TestFixture() {}
    virtual ~StringTest() throw() {}

public:
    void test_tokenize()
    {
        std::vector<std::string> result;
        std::string chop("how is the;weather today?");
        energonsoftware::tokenize(chop, result, " ;?");
        CPPUNIT_ASSERT_EQUAL(std::string("how"), result[0]);
        CPPUNIT_ASSERT_EQUAL(std::string("is"), result[1]);
        CPPUNIT_ASSERT_EQUAL(std::string("the"), result[2]);
        CPPUNIT_ASSERT_EQUAL(std::string("weather"), result[3]);
        CPPUNIT_ASSERT_EQUAL(std::string("today"), result[4]);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringTest);

#endif
