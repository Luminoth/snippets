#if !defined __PYTHONSCRIPT_H__
#define __PYTHONSCRIPT_H__

#if defined WITH_PYTHON
#include <boost/python.hpp>
#endif

namespace energonsoftware {

#if defined WITH_PYTHON

class PythonObject;

class PythonScriptError : public std::exception
{
public:
    PythonScriptError(const std::string& what) : _what(what) {}
    virtual ~PythonScriptError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

private:
    std::string _what;
};

class PythonScript
{
public:
    class PythonScriptAPI
    {
    public:
        PythonScriptAPI();
        virtual ~PythonScriptAPI() throw();

    public:
        bool boolean(const char* value) const;
        void log(const char* script, const char* message) const;
    };

public:
    enum ArgumentType
    {
        ArgumentTypeUnknown,
        ArgumentTypeInt,
        ArgumentTypeDouble,
        ArgumentTypeString,
        ArgumentTypeObject,     // shared_ptr<PythonObject>
    };

    struct Argument
    {
        Argument()
            : type(ArgumentTypeUnknown), name(), value(), object()
        {
        }

        Argument(ArgumentType t, const std::string& n, const boost::any& v)
            : type(t), name(n), value(v), object()
        {
        }

        Argument(const std::string& n, std::shared_ptr<PythonObject> o)
            : type(ArgumentTypeObject), name(n), value(), object(o)
        {
        }

        ArgumentType type;
        std::string name;
        boost::any value;
        std::shared_ptr<PythonObject> object;
    };

private:
    static Logger& logger;
    static PythonScriptAPI api_object;

protected:
    static boost::python::object main_module;
    static boost::python::object main_namespace;
    static boost::python::object api_module;

public:
    typedef void (*PythonInitMethod)(void);
    typedef std::unordered_map<std::string, PythonInitMethod> PythonInitMethods;

    static void init(const PythonInitMethods& init_methods) throw(PythonScriptError);
    static void exec_script(const boost::filesystem::path& filename) throw(PythonScriptError);

    // NOTE: package is *always* the first argument to any API method
    // NOTE: argument ordering is important
    static boost::python::object exec(const std::string& package, const std::string& method, const std::list<Argument>& arguments=std::list<Argument>()) throw(PythonScriptError);

    template <typename T>
    static void std_list_to_python_list(const std::list<T>& from, boost::python::list& to)
    {
        for(const T& v : from) {
            to.append(v);
        }
    }

public:
    virtual ~PythonScript() throw();

protected:
    static void error(const std::string& what) throw(PythonScriptError);

protected:
    PythonScript();
    DISALLOW_COPY_AND_ASSIGN(PythonScript);
};

#endif

}

#endif
