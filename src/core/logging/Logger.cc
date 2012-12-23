#include "src/pch.h"
#include "Logger.h"

namespace energonsoftware {

static const std::string LOG_LEVELS[] = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };

boost::recursive_mutex Logger::logger_mutex;
std::shared_ptr<Logger::ThreadSafeLoggerMap> Logger::_loggers;
std::vector<std::ostream*> Logger::_callbacks;
uint32_t Logger::_logger_type = LoggerTypeStdout;
Logger::LogLevel Logger::_logger_level = LogLevel::Info;
boost::filesystem::path Logger::_logger_filename;
std::shared_ptr<std::ofstream> Logger::_logger_file;

Logger& Logger::instance(const std::string& category)
{
    // NOTE: loggers don't go on an allocator

    if(!_loggers) {
        _loggers.reset(new ThreadSafeLoggerMap());
    }

    boost::lock_guard<boost::recursive_mutex> guard(_loggers->mutex);

    std::shared_ptr<Logger> logger;
    try {
        logger = _loggers->loggers.at(category);
    } catch(const std::out_of_range& ) {
        logger.reset(new Logger(category));
        _loggers->loggers[category] = logger;
    }
    return *logger;
}

void Logger::register_callback(std::ostream* const callback)
{
    _callbacks.push_back(callback);
}

bool Logger::configure(uint32_t type, LogLevel level, const boost::filesystem::path& filename)
{
    boost::lock_guard<boost::recursive_mutex> guard(logger_mutex);

    _logger_type = type;
    _logger_level = level;
    _logger_filename = filename;

    if(config_file()) {
        _logger_file.reset(new std::ofstream(_logger_filename.string().c_str(), std::ios::app));
        if(!(*_logger_file)) return false;
    }

    return true;
}

void Logger::configure(LogLevel level)
{
    boost::lock_guard<boost::recursive_mutex> guard(logger_mutex);

    _logger_type = LoggerTypeStdout;
    _logger_level = level;
    _logger_filename = "";
}

Logger::LogLevel Logger::level(const std::string& level)
{
    std::string scratch(boost::algorithm::to_lower_copy(level));
    if("debug" == level) {
        return LogLevel::Debug;
    } else if("info" == level) {
        return LogLevel::Info;
    } else if("warning" == level) {
        return LogLevel::Warning;
    } else if("error" == level) {
        return LogLevel::Error;
    } else if("critical" == level) {
        return LogLevel::Critical;
    }
    return LogLevel::Invalid;
}

const std::string& Logger::level(LogLevel level) throw(std::out_of_range)
{
    if(level <= LogLevel::Invalid || level > LogLevel::Critical) {
        throw std::out_of_range("Invalid log level!");
    }
    return LOG_LEVELS[static_cast<int>(level)];
}

Logger::Logger(const std::string& category)
    : _category(category), _level(LogLevel::Info)
{
}

Logger::~Logger() throw()
{
}

Logger& Logger::operator<<(std::ostream& (*rhs)(std::ostream&))
{
    boost::lock_guard<boost::recursive_mutex> guard(logger_mutex);

    if(level() >= _logger_level) {
        if(config_stdout()) {
            if(level() >= LogLevel::Error) {
                std::cerr << rhs;
            } else {
                std::cout << rhs;
            }
        }

        if(config_file()) {
            logger_file() << rhs;
        }
    }

    return *this;
}

Logger& operator<<(Logger& lhs, const Logger::LogLevel& level)
{
    boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex);

    lhs._level = level;
    return lhs;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class LoggerTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(LoggerTest);
        CPPUNIT_TEST(test_levels);
    CPPUNIT_TEST_SUITE_END();

private:
    static energonsoftware::Logger& logger;

public:
    LoggerTest() : CppUnit::TestFixture() {}
    virtual ~LoggerTest() throw() {}

public:
    void test_levels()
    {
        LOG_DEBUG("DEBUG test\n");
        LOG_INFO("INFO test\n");
        LOG_WARNING("WARNING test\n");
        LOG_ERROR("ERROR test\n");
        LOG_CRITICAL("CRITICAL test\n");
    }
};
energonsoftware::Logger& LoggerTest::logger(energonsoftware::Logger::instance("energonsoftware.core.logging.LoggerTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(LoggerTest);

#endif
