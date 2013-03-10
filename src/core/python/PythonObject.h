#if !defined __PYTHONOBJECT_H__
#define __PYTHONOBJECT_H__

namespace energonsoftware {

// objects that are used by python scripts should
// inherit from this in order to be passed around
class PythonObject
{
public:
    PythonObject() {}
    virtual ~PythonObject() noexcept {}

public:
    std::string operator[](const std::string& key) const throw(std::out_of_range)
    { return get(key); }

public:
    // override these
    virtual std::string get(const std::string& key) const throw(std::out_of_range)
    { throw std::out_of_range("Base class"); }
};

}

#endif
