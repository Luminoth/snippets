#include "src/pch.h"
#include "src/core/util/fs_util.h"
#include "Lexer.h"

namespace energonsoftware {

Lexer::Lexer()
    : _keyword_map(), _comment_config(CommentTypeNone), _data(), _current(0)
{
}

Lexer::Lexer(const std::string& data)
    : _keyword_map(), _comment_config(CommentTypeNone), _data(data), _current(0)
{
}

Lexer::~Lexer() noexcept
{
}

bool Lexer::load(const boost::filesystem::path& filename)
{
    _data.erase();
    if(!file_to_string(filename, _data)) {
        return false;
    }
    return true;
}

bool Lexer::check_token(int token)
{
    skip_whitespace();

    char ch = advance();
    rewind();
    switch(ch)
    {
    case '\0': return END == token;
    case '(': return OPEN_PAREN == token;
    case ')': return CLOSE_PAREN == token;
    case '{': return OPEN_BRACE == token;
    case '}': return CLOSE_BRACE == token;
    case ';': return SEMICOLON == token;
    case '#': return HASH == token;
    default:
        if(std::isalpha(ch) || '_' == ch) {
            return check_keyword(token);
        }
    }

    return false;
}

bool Lexer::match(int token)
{
    int next = lex();
    return next == token;
}

bool Lexer::int_literal(int& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while('\0' != ch && (std::isdigit(ch) || '-' == ch || '.' == ch)) {
        scratch += ch;
        ch = advance();
    }
    rewind();

    char* end;
    value = strtol(scratch.c_str(), &end, 0);
    return !(*end);
}

bool Lexer::size_literal(size_t& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while('\0' != ch && (std::isdigit(ch) || '-' == ch || '.' == ch)) {
        scratch += ch;
        ch = advance();
    }
    rewind();

    char* end;
    value = strtol(scratch.c_str(), &end, 0);
    return !(*end);
}

bool Lexer::float_literal(float& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while('\0' != ch && (std::isdigit(ch) || '-' == ch || '.' == ch)) {
        scratch += ch;
        ch = advance();
    }
    rewind();

    char* end;
    value = strtod(scratch.c_str(), &end);
    return !(*end);
}

bool Lexer::string_literal(std::string& value)
{
    skip_whitespace();

    char ch = advance();
    if('"' != ch) return false;

    value.erase();

    ch = advance();
    while('\0' != ch && '"' != ch && '\n' != ch) {
        value += ch;
        ch = advance();
    }
    return '\0' != ch && '\n' != ch;
}

bool Lexer::bool_literal(bool& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while('\0' != ch && std::isalpha(ch)) {
        scratch += ch;
        ch = advance();
    }
    rewind();

    value = ("true" == boost::algorithm::to_lower_copy(scratch));
    return true;
}

void Lexer::advance_line()
{
    char ch = advance();
    while('\0' != ch && '\r' != ch && '\n' != ch) {
        ch = advance();
    }
    skip_whitespace();
}

void Lexer::set_keyword(const std::string& keyword, int token)
{
    _keyword_map[keyword] = token;
}

char Lexer::advance()
{
    // TODO: we should do a good check on _data.length() before blindly casting it to an int
    char ret = _current < static_cast<int>(_data.length()) ? _data[_current] : '\0';
    _current++;
    return ret;
}

int Lexer::lex()
{
    skip_whitespace();

    char ch = advance();
    switch(ch)
    {
    case '\0': return END;
    case '(': return OPEN_PAREN;
    case ')': return CLOSE_PAREN;
    case '{': return OPEN_BRACE;
    case '}': return CLOSE_BRACE;
    case ';': return SEMICOLON;
    case '#': return HASH;
    default:
        if(std::isalnum(ch) || '_' == ch) {
            rewind();
            return keyword();
        }
    }

    return LEX_ERROR;
}

void Lexer::skip_whitespace()
{
    // skip whitespace
    char ch = advance();
    while('\0' != ch && std::isspace(ch)) {
        ch = advance();
    }
    rewind();

    skip_comments();
}

void Lexer::skip_comments()
{
    char ch = advance();
    if(c_comments() && '/' == ch) {
        ch = advance();
        if(cpp_comments() && '/' == ch) {
            return advance_line();
        } else if('*' == ch) {
            ch = advance();
            while(true) {
                if('*' == ch) {
                    ch = advance();
                    if('/' == ch) {
                        return skip_whitespace();
                    }
                }
                ch = advance();
            }
        }
        rewind();
    } else if(hash_comments() && '#' == ch) {
        return advance_line();
    }
    rewind();
}

bool Lexer::check_keyword(int token)
{
    std::string scratch;

    char ch = advance();
    size_t len = 1;
    while('\0' != ch && (std::isalnum(ch) || '_' == ch)) {
        scratch += ch;
        ch = advance();
        len++;
    }
    _current -= len;

    try {
        return token == _keyword_map.at(scratch);
    } catch(std::out_of_range&) {
    }
    return false;
}

int Lexer::keyword()
{
    std::string scratch;

    char ch = advance();
    while(std::isalnum(ch) || '_' == ch) {
        scratch += ch;
        ch = advance();
    }
    rewind();

    try {
        return _keyword_map.at(scratch);
    } catch(std::out_of_range&) {
    }
    return LEX_ERROR;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"
#include "src/core/common.h"

class TestLexer : public energonsoftware::Lexer
{
public:
    enum Token
    {
        VERSION,
        TEST,
    };

public:
    TestLexer() : Lexer() { init(); }
    explicit TestLexer(const std::string& data) : Lexer(data) { init(); }
    virtual ~TestLexer() noexcept {}

public:
    bool constant(std::string& value)
    {
        skip_whitespace();

        value.erase();

        char ch = advance();
        while('\0' != ch && (std::isalnum(ch) || '_' == ch)) {
            value += ch;
            ch = advance();
        }
        rewind();

        return true;
    }

private:
    void init()
    {
        configure_comments(CommentTypeAll);

        set_keyword("version", VERSION);
        set_keyword("test", TEST);
    }

private:
    DISALLOW_COPY_AND_ASSIGN(TestLexer);
};

class LexerTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(LexerTest);
        CPPUNIT_TEST(test_lexer);
    CPPUNIT_TEST_SUITE_END();

public:
    LexerTest() : CppUnit::TestFixture(), _lexer() {}
    virtual ~LexerTest() noexcept {}

public:
    void test_lexer()
    {
        CPPUNIT_ASSERT(_lexer.load(energonsoftware::data_dir() / "lextest.txt"));
        CPPUNIT_ASSERT(_lexer.match(TestLexer::VERSION));

        float fscratch=0.0f;
        CPPUNIT_ASSERT(_lexer.float_literal(fscratch));
        CPPUNIT_ASSERT_EQUAL(3.23f, fscratch);

        CPPUNIT_ASSERT(_lexer.match(energonsoftware::Lexer::SEMICOLON));

        std::string sscratch;
        CPPUNIT_ASSERT(_lexer.constant(sscratch));
        CPPUNIT_ASSERT_EQUAL(std::string("testlex"), sscratch);
        CPPUNIT_ASSERT(_lexer.match(energonsoftware::Lexer::OPEN_PAREN));
        CPPUNIT_ASSERT(_lexer.match(energonsoftware::Lexer::CLOSE_PAREN));

        CPPUNIT_ASSERT(_lexer.match(energonsoftware::Lexer::OPEN_BRACE));

        CPPUNIT_ASSERT(_lexer.match(TestLexer::TEST));
        CPPUNIT_ASSERT(_lexer.match(energonsoftware::Lexer::SEMICOLON));

        CPPUNIT_ASSERT(_lexer.string_literal(sscratch));
        CPPUNIT_ASSERT_EQUAL(std::string("strings require these thingies"), sscratch);
        CPPUNIT_ASSERT(_lexer.match(energonsoftware::Lexer::SEMICOLON));

        int iscratch;
        CPPUNIT_ASSERT(_lexer.int_literal(iscratch));
        CPPUNIT_ASSERT_EQUAL(12345, iscratch);
        CPPUNIT_ASSERT(_lexer.match(energonsoftware::Lexer::SEMICOLON));

        CPPUNIT_ASSERT(_lexer.match(energonsoftware::Lexer::CLOSE_BRACE));
    }

private:
    TestLexer _lexer;
};

CPPUNIT_TEST_SUITE_REGISTRATION(LexerTest);

#endif
