#include "src/pch.h"
#include "src/core/util/fs_util.h"
#include "Lexer.h"

namespace energonsoftware {

std::unordered_map<std::string, int> Lexer::keyword_map;

void Lexer::set_keyword(const std::string& keyword, int token)
{
    keyword_map[keyword] = token;
}

Lexer::Lexer()
    : _current(0)
{
}

Lexer::Lexer(const std::string& data)
    : _data(data), _current(0)
{
}

Lexer::~Lexer() throw()
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
    _current--;
    switch(ch)
    {
    case '\0': return END == token;
    case '(': return OPEN_PAREN == token;
    case ')': return CLOSE_PAREN == token;
    case '{': return OPEN_BRACE == token;
    case '}': return CLOSE_BRACE == token;
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
    while(ch != '\0' && (std::isdigit(ch) || ch == '-' || ch == '.')) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    char* end;
    value = strtol(scratch.c_str(), &end, 0);
    return !(*end);
}

bool Lexer::size_literal(size_t& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while(ch != '\0' && (std::isdigit(ch) || ch == '-' || ch == '.')) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    char* end;
    value = strtol(scratch.c_str(), &end, 0);
    return !(*end);
}

bool Lexer::float_literal(float& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while(ch != '\0' && (std::isdigit(ch) || ch == '-' || ch == '.')) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    char* end;
    value = strtod(scratch.c_str(), &end);
    return !(*end);
}

bool Lexer::string_literal(std::string& value)
{
    skip_whitespace();

    char ch = advance();
    if(ch != '"') return false;

    value.erase();

    ch = advance();
    while(ch != '\0' && ch != '"' && ch != '\n') {
        value += ch;
        ch = advance();
    }
    return ch != '\0' && ch != '\n';
}

bool Lexer::bool_literal(bool& value)
{
    skip_whitespace();

    std::string scratch;
    char ch = advance();
    while(ch != '\0' && std::isalpha(ch)) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    value = ("true" == boost::algorithm::to_lower_copy(scratch));
    return true;
}

void Lexer::advance_line()
{
    char ch = advance();
    while(ch != '\0' && ch != '\r' && ch != '\n') {
        ch = advance();
    }
    skip_whitespace();
}

char Lexer::advance()
{
    char ret = _current < _data.length() ? _data[_current] : '\0';
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
    default:
        if(std::isalnum(ch) || '_' == ch) {
            _current--;
            return keyword();
        }
    }

    return LEX_ERROR;
}

void Lexer::skip_whitespace()
{
    // skip whitespace
    char ch = advance();
    while(ch != '\0' && std::isspace(ch)) {
        ch = advance();
    }
    _current--;

    skip_comments();
}

void Lexer::skip_comments()
{
    char ch = advance();
    if(ch == '/') {
        ch = advance();
        if(ch == '/') {
            return advance_line();
        } else if(ch == '*') {
            ch = advance();
            while(true) {
                if(ch == '*') {
                    ch = advance();
                    if(ch == '/') {
                        return skip_whitespace();
                    }
                }
                ch = advance();
            }
        }
        _current--;
    } else if(ch == '#') {
        return advance_line();
    }
    _current--;
}

bool Lexer::check_keyword(int token)
{
    std::string scratch;

    char ch = advance();
    size_t len = 1;
    while(ch != '\0' && (std::isalnum(ch) || '_' == ch)) {
        scratch += ch;
        ch = advance();
        len++;
    }
    _current -= len;

    try {
        return token == keyword_map.at(scratch);
    } catch(std::out_of_range&) {
    }
    return false;
}

int Lexer::keyword()
{
    std::string scratch;

    char ch = advance();
    while(ch != '\0' && (std::isalnum(ch) || '_' == ch)) {
        scratch += ch;
        ch = advance();
    }
    _current--;

    try {
        return keyword_map.at(scratch);
    } catch(std::out_of_range&) {
    }
    return LEX_ERROR;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class LexerTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(LexerTest);
    CPPUNIT_TEST_SUITE_END();

public:
    LexerTest() : CppUnit::TestFixture() {}
    virtual ~LexerTest() throw() {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(LexerTest);

#endif
