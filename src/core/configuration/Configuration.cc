#include "src/pch.h"
#include <fstream>
#include <iostream>
#include "src/core/text/string_util.h"
#include "src/core/util/util.h"
#include "Configuration.h"

namespace energonsoftware {

Configuration::Configuration()
    : _sections(), _map(), _listeners(), _header(), _dirty(false)
{
    set_default("logging", "level", "info");
    set_default("logging", "stdout", "true");
    set_default("logging", "file", "false");
    set_default("logging", "filename", "");

    load_defaults();
}

Configuration::~Configuration() throw()
{
}

void Configuration::generate_header(const std::string& header)
{
    std::stringstream str;
    str << "# " << header << std::endl
        << "#" << std::endl
        << "# Supported options:" << std::endl
        << "#" << std::endl;

    for(const auto& section : _sections) {
        str << "#\t[" << section.first << "]" << std::endl;

        for(const auto& option : section.second) {
            if(!option.second.readonly) {
                str << "#\t" << option.second.name << " = <" << option.second.help << "> "
                    << "(default " << option.second.default_value << ")" << std::endl;
            }
        }
        str << "#" << std::endl;
    }

    _header = str.str();
}

void Configuration::set_default(const std::string& section, const std::string& option, const std::string& value)
{
    ConfigOption& opt = this->option(section, option);
    opt.default_value = value;
}

void Configuration::set_defaults(const std::string& section, const std::list<Configuration::ConfigOption>& options)
{
    for(const ConfigOption& opt : options) {
        set_default(section, opt.name, opt.default_value);
    }
}

void Configuration::set_defaults(const Configuration::ConfigSections& sections)
{
    for(const auto& section : sections) {
        std::list<ConfigOption> options;
        for(const auto& option : section.second) {
            options.push_back(option.second);
        }
        set_defaults(section.first, options);
    }
}

void Configuration::load_defaults()
{
    for(auto& section : _sections) {
        for(auto& option : section.second) {
            option.second.value = option.second.default_value;
            update_map(section.first, option.second);
        }
    }
    _dirty = true;
}

void Configuration::set_help(const std::string& section, const std::string& option, const std::string& help)
{
    ConfigOption& opt = this->option(section, option);
    opt.help = help;
}

void Configuration::set_read_only(const std::string& section, const std::string& option, bool readonly)
{
    ConfigOption& opt = this->option(section, option);
    opt.readonly = readonly;
}

void Configuration::set(const std::string& section, const std::string& option, const std::string& value)
{
    ConfigOption& opt = this->option(section, option);
    const std::string& old_value = opt.value;
    opt.value = value;
    update_map(section, opt);
    on_value_changed(section, option, old_value);
    _dirty = true;
}

std::string Configuration::get(const std::string& section, const std::string& option) const
{
    ConfigSections::const_iterator sit = _sections.find(section);
    if(sit == _sections.end()) {
        return std::string();
    }

    ConfigOptions::const_iterator oit = sit->second.find(option);
    if(oit == sit->second.end()) {
        return std::string();
    }

    return oit->second.value;
}


const std::string& Configuration::lookup(const std::string& section, const std::string& option) const throw(NoSuchConfigOptionError)
{
    ConfigSections::const_iterator sit = _sections.find(section);
    if(sit == _sections.end()) {
        throw NoSuchConfigOptionError("Config section does not exist: " + section);
    }

    ConfigOptions::const_iterator oit = sit->second.find(option);
    if(oit == sit->second.end()) {
        throw NoSuchConfigOptionError("Config option does not exist: " + option);
    }

    return oit->second.value;
}

bool Configuration::load(const boost::filesystem::path& filename)
{
    std::ifstream infile(filename.string().c_str());
    if(!infile) {
        return false;
    }

    std::string section;
    std::string scratch;
    while(!infile.eof()) {
        std::getline(infile, scratch);
        if(scratch.empty()) {
            continue;
        }

        // strip comments
        size_t pos = scratch.find('#');
        if(pos != std::string::npos) {
            scratch.erase(pos);
        }

        // strip the ends whitespace
        // this is more to get rid of
        // lazy space after comment removal
        boost::trim_right(scratch);
        if(scratch.empty()) {
            continue;
        }

        // check for a section
        std::string temp(boost::trim_left_copy(scratch));
        if(temp[0] == '[') {
            pos = temp.find(']');
            if(pos == std::string::npos) {
                return false;
            }

            section = temp.substr(1, pos-1);
            continue;
        }

        // we shouldn't be here without a section
        if(section == "") {
            return false;
        }

        // check for a name/value pair
        pos = scratch.find('=');
        if(pos == std::string::npos) {
            return false;
        }

        std::string option(trim_all(scratch.substr(0, pos)));
        std::string value(boost::trim_copy(scratch.substr(pos + 1)));

        set(section, option, value);

        // TODO: check for duplicates so we can warn the user
        //std::cerr << "WARNING: Ignoring duplicate value - [" << section << "] " << option << " = " << value << std::endl;
    }

    clean();
    return true;
}

bool Configuration::save(const boost::filesystem::path& filename)
{
    std::ofstream outfile(filename.string().c_str());
    if(!outfile) {
        return false;
    }

    on_save();

    outfile << _header << std::endl;
    dump(outfile);
    outfile.close();

    clean();
    return true;
}

bool Configuration::exists(const boost::filesystem::path& filename) const
{
    return boost::filesystem::exists(filename);
}

void Configuration::dump(Logger& logger) const
{
    std::stringstream str;
    str << "Configuration:" << std::endl;
    dump(str);
    LOG_INFO(str.str() << "\n");
}

void Configuration::dump(std::ostream& out) const
{
    for(const auto& section : _sections) {
        out << "[" << section.first << "]" << std::endl;
        for(const auto& option : section.second) {
            if(!option.second.readonly) {
                out << trim_all(option.second.name) << " = " << boost::trim_copy(option.second.value) << std::endl;
            }
        }
        out << std::endl;
    }
}

void Configuration::register_listener(const std::string& section, const std::string& option, ConfigListener& listener)
{
    _listeners[section + "." + option] = listener;
}

uint32_t Configuration::logging_type() const
{
    uint32_t type = Logger::LoggerTypeNone;

    if(to_boolean(get("logging", "stdout"))) {
        type |= Logger::LoggerTypeStdout;
    }

    if(to_boolean(get("logging", "file"))) {
        type |= Logger::LoggerTypeFile;
    }

    return type;
}

Logger::Level Configuration::logging_level() const
{
    return Logger::level(get("logging", "level"));
}

void Configuration::validate() const throw(ConfigurationError)
{
    try {
        Logger::level(get("logging", "level"));
    } catch(const std::out_of_range&) {
        throw ConfigurationError("Logging level must be a valid level!");
    }
}

Configuration::ConfigOptions& Configuration::section(const std::string& section)
{
    try {
        return _sections.at(section);
    } catch(const std::out_of_range&) {
        return (_sections[section] = ConfigOptions());
    }
}

Configuration::ConfigOption& Configuration::option(const std::string& section, const std::string& option)
{
    ConfigOptions& options = this->section(section);
    try {
        return options.at(option);
    } catch(const std::out_of_range&) {
        return (options[option] = ConfigOption(option));
    }
}

void Configuration::update_map(const std::string& section, const ConfigOption& option)
{
    std::string key(section + "." + option.name);

    _map[key] = option.value;

    try {
        ConfigListener& listener = _listeners.at(key);
        listener(section, option.name, option.value);
    } catch(const std::out_of_range&) {
    }
}

}

#if defined WITH_UNIT_TESTS
#include "src/core/common.h"
#include "src/test/UnitTest.h"

class TestConfiguration : public energonsoftware::Configuration
{
public:
    static TestConfiguration& instance()
    {
        static std::shared_ptr<TestConfiguration> configuration;
        if(!configuration) {
            configuration.reset(new TestConfiguration());
        }
        return *configuration;
    }

private:
    static energonsoftware::Logger& logger;

public:
    virtual ~TestConfiguration() throw() {}

public:
    bool load_config()
    {
        LOG_INFO("Loading configuration...\n");
        return load(energonsoftware::test_conf());
    }

    bool save_config()
    {
        LOG_INFO("Saving configuration...\n");
        return save(energonsoftware::test_conf());
    }

    bool config_exists() const
    {
        LOG_INFO("Checking if configuration exists...\n");
        return exists(energonsoftware::test_conf());
    }

    virtual void validate() const throw(energonsoftware::ConfigurationError) override
    {
        Configuration::validate();

        if(!energonsoftware::is_int(get("test", "test_int"))) {
            throw energonsoftware::ConfigurationError("test_int must be an integer!");
        }
    }

private:
    virtual void on_save() override
    {
        LOG_DEBUG("TestConfiguration::on_save()\n");
    }

private:
    TestConfiguration()
        : energonsoftware::Configuration()
    {
        set_default("test", "test_boolean", "true");

        set_default("test", "test_int", "25");

        set_default("test", "test_readonly", "on");
        set_read_only("test", "test_readonly");

        load_defaults();
        generate_header("Test configuration file");
    }

    DISALLOW_COPY_AND_ASSIGN(TestConfiguration);
};
energonsoftware::Logger& TestConfiguration::logger(energonsoftware::Logger::instance("energonsoftware.core.configuration.TestConfiguration"));

class ConfigurationTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(ConfigurationTest);
        CPPUNIT_TEST(test_create);
        CPPUNIT_TEST(test_save);
        CPPUNIT_TEST(test_load);
        CPPUNIT_TEST(test_validate);
        CPPUNIT_TEST(test_get);
        CPPUNIT_TEST(test_lookup);
    CPPUNIT_TEST_SUITE_END();

private:
    static energonsoftware::Logger& logger;

public:
    ConfigurationTest() : CppUnit::TestFixture() {}
    virtual ~ConfigurationTest() throw() {}

public:
    void test_create()
    {
        TestConfiguration& config(TestConfiguration::instance());
        LOG_INFO("Test configuration " << (config.config_exists() ? "exists" : "does not exist") << "\n");
    }

    void test_save()
    {
        TestConfiguration& config(TestConfiguration::instance());
        CPPUNIT_ASSERT(config.save_config());
    }

    void test_load()
    {
        TestConfiguration& config(TestConfiguration::instance());
        CPPUNIT_ASSERT(config.load_config());
    }

    void test_validate()
    {
        TestConfiguration& config(TestConfiguration::instance());
        CPPUNIT_ASSERT_NO_THROW(config.validate());

        config.set("test", "test_int", "twenty five");
        CPPUNIT_ASSERT_THROW(config.validate(), energonsoftware::ConfigurationError);
        config.set("test", "test_int", "25");
    }

    void test_get()
    {
        TestConfiguration& config(TestConfiguration::instance());
        CPPUNIT_ASSERT_EQUAL(std::string("25"), config.get("test", "test_int"));
        CPPUNIT_ASSERT_EQUAL(std::string(), config.get("test", "test_invalid"));
    }

    void test_lookup()
    {
        TestConfiguration& config(TestConfiguration::instance());
        CPPUNIT_ASSERT_EQUAL(std::string("25"), config.lookup("test", "test_int"));
        CPPUNIT_ASSERT_THROW(config.lookup("test", "test_invalid"), energonsoftware::NoSuchConfigOptionError);
    }
};
energonsoftware::Logger& ConfigurationTest::logger(energonsoftware::Logger::instance("energonsoftware.core.configuration.ConfigurationTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigurationTest);

#endif
