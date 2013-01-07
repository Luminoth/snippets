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
        SEMICOLON,
        HASH,

        // end of file
        END,

        // scan error
        LEX_ERROR,
    };

protected:
    enum CommentTypeMask
    {
        CommentTypeNone = 0,
        CommentTypeC = 1,
        CommentTypeCpp = 3,
        CommentTypeHash = 4,

        CommentTypeAll = 0xffff
    };

public:
    virtual ~Lexer() throw();

public:
    virtual bool load(const boost::filesystem::path& filename) final;

    virtual int position() const final { return _current; }
    virtual size_t length() const final { return _data.length(); }

    virtual void data(const std::string& data) final { _data = data; }

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

protected:
    virtual void configure_comments(uint32_t type) final { _comment_config = type; }

    // NOTE: this does NOT check for overwrite
    virtual void set_keyword(const std::string& keyword, int token) final;

    virtual bool c_comments() const final { return CommentTypeC == (_comment_config & CommentTypeC); }
    virtual bool cpp_comments() const final { return CommentTypeCpp == (_comment_config & CommentTypeCpp); }
    virtual bool hash_comments() const final { return CommentTypeHash == (_comment_config & CommentTypeHash); }

    virtual void skip_comments() final;

    virtual char advance() final;
    virtual void rewind() final { _current--; }
    virtual int lex() final;
    virtual bool check_keyword(int token) final;
    virtual int keyword() final;

private:
    std::unordered_map<std::string, int> _keyword_map;
    uint32_t _comment_config;

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
