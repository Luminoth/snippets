#if !defined __CONFIGURATION_H__
#define __CONFIGURATION_H__

namespace energonsoftware {

class Logger;

class ConfigurationError : public std::exception
{
public:
    explicit ConfigurationError(const std::string& what) throw() : _what(what) {}
    virtual ~ConfigurationError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

private:
    std::string _what;
};

class NoSuchConfigOptionError : public ConfigurationError
{
public:
    explicit NoSuchConfigOptionError(const std::string& what) throw() : ConfigurationError(what) {}
    virtual ~NoSuchConfigOptionError() throw() {}
};

/*
Overriding classes should include the following static method:

    static ConfigurationSubclass& instance();

that should manage the configuration as a singleton. No constructors should available.

NOTE: this class is NOT thread safe!
*/
class Configuration
{
public:
    typedef std::unordered_map<std::string, std::string> ConfigMap;
    typedef std::function<void (const std::string&, const std::string&, const std::string&)> ConfigListener;
    typedef std::unordered_map<std::string, ConfigListener> ConfigListenerMap;

    typedef ConfigMap::iterator iterator;
    typedef ConfigMap::const_iterator const_iterator;
    typedef ConfigMap::value_type value_type;

private:
    struct ConfigOption
    {
        std::string name;
        std::string default_value;
        std::string value;
        std::string help;
        bool readonly;

        ConfigOption(const std::string& n=std::string(), const std::string& dv=std::string())
            : name(n), default_value(dv), help(n), readonly(false)
        {
        }

        virtual ~ConfigOption() throw() {}
    };

    typedef std::unordered_map<std::string, ConfigOption> ConfigOptions;
    typedef std::unordered_map<std::string, ConfigOptions> ConfigSections;

public:
    virtual ~Configuration() throw();

public:
    // generates a header from the default values
    virtual void generate_header(const std::string& header) final;

    // sets a default option value
    // this will create a new option if the requestion one doesn't exist
    virtual void set_default(const std::string& section, const std::string& option, const std::string& value="") final;

    // sets all default values for a section
    // uses the default_value parameter of the options
    virtual void set_defaults(const std::string& section, const std::list<Configuration::ConfigOption>& options) final;

    // sets all default values
    virtual void set_defaults(const ConfigSections& sections) final;

    // loads the default options
    // into the current set
    virtual void load_defaults() final;

    virtual void set_help(const std::string& section, const std::string& option, const std::string& help) final;

    // sets an option to be read-only or read-write
    // NOTE: read-only in this context applies only to *writing* the config file
    // not to the internal storage of the value, or reading the config file
    virtual void set_read_only(const std::string& section, const std::string& option, bool readonly=true) final;

    // sets a config value
    // this will create a new option if the requested one doesn't exist
    virtual void set(const std::string& section, const std::string& option, const std::string& value="") final;

    // looks up a config value
    // returns an empty string if the value doesn't exist
    virtual std::string get(const std::string& section, const std::string& option) const final;

    // looks up a config value
    // throws NoSuchConfigOptionError if not found
    virtual const std::string& lookup(const std::string& section, const std::string& option) const throw(NoSuchConfigOptionError) final;

    // reads a config from a file
    virtual bool load(const boost::filesystem::path& filename) final;

    // writes the config to a file
    virtual bool save(const boost::filesystem::path& filename) final;

    // returns true if the config file exits
    virtual bool exists(const boost::filesystem::path& filename) const final;

    // dumps the config to a log file
    virtual void dump(Logger& logger) const final;

    // dumps the config to an output stream
    virtual void dump(std::ostream& out) const final;

    // returns whether or not the config is dirty (unsaved)
    virtual bool dirty() const final { return _dirty; }

    // forces a clean on the config without saving
    virtual void clean() final { _dirty = false; }

    // returns the configuration in a map
    // [section.option] = value
    virtual const ConfigMap& map() const final { return _map; }

    // NOTE: this will blindly overwrite existing listeners
    virtual void register_listener(const std::string& section, const std::string& option, ConfigListener& listener) final;

    virtual uint32_t logging_type() const final;
    virtual Logger::Level logging_level() const final;
    virtual const boost::filesystem::path logging_filename() const final { return get("logging", "filename"); }

    virtual iterator begin() final { return _map.begin(); }
    virtual iterator end() final { return _map.end(); }
    virtual const_iterator begin() const final { return _map.begin(); }
    virtual const_iterator end() const final { return _map.end(); }

public:
    // call super-class validate()
    virtual void validate() const throw(ConfigurationError);

private:
    virtual void on_value_changed(const std::string& section, const std::string& option, const std::string& old_value) {}
    virtual void on_save() {}

private:
    ConfigOptions& section(const std::string& section);
    ConfigOption& option(const std::string& section, const std::string& option);
    void update_map(const std::string& section, const ConfigOption& option);

protected:
    Configuration();
    DISALLOW_COPY_AND_ASSIGN(Configuration);

private:
    ConfigSections _sections;
    ConfigMap _map;
    ConfigListenerMap _listeners;
    std::string _header;
    bool _dirty;
};

}

#endif
