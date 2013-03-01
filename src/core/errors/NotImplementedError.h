#if !defined __NOTIMPLEMENTEDERROR_H__
#define __NOTIMPLEMENTEDERROR_H__

namespace energonsoftware {

class NotImplementedError : public std::exception
{
public:
    explicit NotImplementedError(const std::string& function) noexcept
        : _what("Function not implemented: " + function + "()") { }
    virtual ~NotImplementedError() noexcept {}
    virtual const char* what() const noexcept { return _what.c_str(); }

private:
    std::string _what;
};

}

#endif
