#include "src/pch.h"
#include "src/core/common.h"
#include "src/core/util/util.h"
#include "PythonScript.h"
#include "PythonObject.h"

namespace energonsoftware {

#if defined WITH_PYTHON

BOOST_PYTHON_MODULE(core)
{
    boost::python::class_<PythonObject, std::shared_ptr<PythonObject> >("PythonObject")
        .def("__getitem__", &PythonObject::get);
    boost::python::class_<std::list<std::shared_ptr<PythonObject> > >("PythonObjectList")
        .def("__iter__", boost::python::iterator<std::list<std::shared_ptr<PythonObject> > >());

    boost::python::class_<PythonScript::PythonScriptAPI>("PythonScriptAPI", boost::python::no_init)
        .def("boolean", &PythonScript::PythonScriptAPI::boolean)
        .def("log", &PythonScript::PythonScriptAPI::log);
}

PythonScript::PythonScriptAPI::PythonScriptAPI()
{
}

PythonScript::PythonScriptAPI::~PythonScriptAPI() noexcept
{
}

bool PythonScript::PythonScriptAPI::boolean(const char* value) const
{
    return boolean(value);
}

void PythonScript::PythonScriptAPI::log(const char* script, const char* message) const
{
/* TODO: add the script name to the output */
    LOG_INFO(message << "\n");
}

Logger& PythonScript::logger(Logger::instance("energonsoftware.core.python.PythonScript"));
PythonScript::PythonScriptAPI PythonScript::api_object;
boost::python::object PythonScript::main_module;
boost::python::object PythonScript::main_namespace;
boost::python::object PythonScript::api_module;

void PythonScript::init(const PythonInitMethods& init_methods) throw(PythonScriptError)
{
    try {
        LOG_INFO("Initializing Python...\n");
        if(PyImport_AppendInittab("core", &initcore) < 0) {
            error("Failed to append module core");
        }

        for(const PythonInitMethods::value_type& method : init_methods) {
            if(PyImport_AppendInittab(const_cast<char*>(method.first.c_str()), method.second) < 0) {
                error("Failed to append module " + method.first);
            }
        }

        Py_Initialize();

        main_module = boost::python::import("__main__");
        main_namespace = main_module.attr("__dict__");

        // auto-import the api module and give scripts access to the object
        api_module = boost::python::object(boost::python::handle<>(PyImport_ImportModule("core")));
        main_namespace["energonsoftware"] = api_module;
        for(const PythonInitMethods::value_type& method : init_methods) {
            boost::python::object module(boost::python::handle<>(PyImport_ImportModule(method.first.c_str())));
            main_namespace[method.first] = module;
        }
        boost::python::scope(api_module).attr("core") = boost::python::ptr(&api_object);
    } catch(...) {
        error("Failed to initialize the python API!");
    }
}

void PythonScript::exec_script(const boost::filesystem::path& filename) throw(PythonScriptError)
{
/* TODO: we need to do some security check by not allowing imports and shit */

    LOG_DEBUG("Executing script: " << filename << "\n");
    try {
        boost::python::exec_file(filename.string().c_str(), main_namespace);
    } catch(...) {
        error("Failed to exec python script!");
    }
}

boost::python::object PythonScript::exec(const std::string& package, const std::string& method, const std::list<Argument>& arguments) throw(PythonScriptError)
{
    if(method == "") {
        return boost::python::object();
    }

    try {
        boost::python::dict locals;
        locals["package"] = package;

        std::stringstream run;
        run << package << "_" << method << "(package";

        for(const Argument& argument : arguments) {
            run << ", " << argument.name;
            try {
                switch(argument.type)
                {
                case ArgumentTypeUnknown:
                    LOG_WARNING("Unknown python argument type!");
                    return boost::python::object();
                case ArgumentTypeInt:
                    locals[argument.name] = boost::any_cast<int>(argument.value);
                    break;
                case ArgumentTypeDouble:
                    locals[argument.name] = boost::any_cast<double>(argument.value);
                    break;
                case ArgumentTypeString:
                    locals[argument.name] = (boost::any_cast<std::string>(argument.value));
                    break;
                case ArgumentTypeObject:
                    locals[argument.name] = argument.object;
                    break;
                }
            } catch(const boost::bad_any_cast&) {
                LOG_ERROR("Invalid python argument!");
                return boost::python::object();
            }
        }
        run << ")";

        //LOG_DEBUG("Executing method: " << run.str() << " (" << boost::python::len(locals) << " arguments)\n");
        return boost::python::exec(run.str().c_str(), main_namespace, locals);
    } catch(...) {
        error("Failed to exec python method!");
    }
    return boost::python::object();
}

void PythonScript::error(const std::string& what) throw(PythonScriptError)
{
    PyErr_Print();
    PyErr_Clear();

    throw PythonScriptError(what);
}

PythonScript::~PythonScript() noexcept
{
}

#endif

}
