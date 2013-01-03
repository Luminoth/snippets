#if !defined __LEXER_H__
#define __LEXER_H__

namespace energonsoftware {

class Lexer
{
public:
    enum Token
    {
        // general tokens
        OPEN_PAREN,
        CLOSE_PAREN,
        OPEN_BRACE,
        CLOSE_BRACE,

        // end of file
        END,

        // scan error
        LEX_ERROR,

        // custom tokens must start after LEX_ERROR
    };

private:
    static std::unordered_map<std::string, int> keyword_map;

protected:
    // NOTE: this does not check for keyword overwrite
    static void set_keyword(const std::string& keyword, int token);

public:
    virtual ~Lexer() throw();

public:
    virtual bool load(const boost::filesystem::path& filename) final;

    virtual int position() const final { return _current; }
    virtual size_t length() const final { return _data.length(); }
    virtual void clear() final { _current = 0; _data.erase(); }
    virtual void reset() final { _current = 0; }

    virtual void skip_whitespace() final;
    virtual bool check_token(int token) final;
    virtual bool match(int token) final;
    virtual bool int_literal(int& value) final;
    virtual bool size_literal(size_t& value) final;
    virtual bool float_literal(float& value) final;
    virtual bool string_literal(std::string& value) final;
    virtual bool bool_literal(bool& value) final;

    // swallows the rest of the current line
    virtual void advance_line() final;

private:
    void skip_comments();

    char advance();
    int lex();
    bool check_keyword(int token);
    int keyword();

private:
    std::string _data;
    int _current;

protected:
    Lexer();
    explicit Lexer(const std::string& data);

private:
    DISALLOW_COPY_AND_ASSIGN(Lexer);
};

}

#endif
