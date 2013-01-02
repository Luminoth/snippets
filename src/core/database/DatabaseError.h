#if !defined __DATABASEERROR_H__
#define __DATABASEERROR_H__

namespace energonsoftware {

class DatabaseError : public std::exception
{
public:
    explicit DatabaseError(const std::string& what) throw() : _what(what) { }
    virtual ~DatabaseError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

protected:
    std::string _what;
};

}

#endif
