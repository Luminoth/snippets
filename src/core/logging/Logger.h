#if !defined __LOGGER_H__
#define __LOGGER_H__

// TODO: get rid of these includes here by switching to a filter model
#include <iostream>
#include <fstream>

/*<< __FILE__ << "[" << __LINE__ << "] " \*/

#define LOG_DEBUG(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(energonsoftware::Logger::logger_mutex); \
    logger << energonsoftware::Logger::Level::Debug \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << energonsoftware::Logger::level(energonsoftware::Logger::Level::Debug) << ": " \
        << e; \
} while(false)

#define LOG_INFO(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(energonsoftware::Logger::logger_mutex); \
    logger << energonsoftware::Logger::Level::Info \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << energonsoftware::Logger::level(energonsoftware::Logger::Level::Info) << ": " \
        << e; \
} while(false)

#define LOG_WARNING(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(energonsoftware::Logger::logger_mutex); \
    logger << energonsoftware::Logger::Level::Warning \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << energonsoftware::Logger::level(energonsoftware::Logger::Level::Warning) << ": " \
        << e; \
} while(false)

#define LOG_ERROR(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(energonsoftware::Logger::logger_mutex); \
    logger << energonsoftware::Logger::Level::Error \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << energonsoftware::Logger::level(energonsoftware::Logger::Level::Error) << ": " \
        << e; \
} while(false)

#define LOG_CRITICAL(e) do { \
    boost::lock_guard<boost::recursive_mutex> guard(energonsoftware::Logger::logger_mutex); \
    logger <<energonsoftware:: Logger::Level::Critical \
        << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << " " \
        << "[" << boost::this_thread::get_id() << "] " \
        << logger.category() << " " \
        << energonsoftware::Logger::level(energonsoftware::Logger::Level::Critical) << ": " \
        << e; \
} while(false)

namespace energonsoftware {

class Logger final
{
private:
    typedef std::unordered_map<std::string, std::shared_ptr<Logger>> LoggerMap;
    struct ThreadSafeLoggerMap
    {
        boost::recursive_mutex mutex;
        LoggerMap loggers;
    };

public:
    enum LoggerTypeMask
    {
        LoggerTypeNone = 0,
        LoggerTypeStdout = 1,
        LoggerTypeFile = 2,
    };

    enum class Level
    {
        Invalid = -1,
        Debug = 0,
        Info,
        Warning,
        Error,
        Critical,
    };

public:
    static boost::recursive_mutex logger_mutex;

private:
    static std::shared_ptr<ThreadSafeLoggerMap> _loggers;
    static std::vector<std::ostream*> _callbacks;
    static uint32_t _logger_type;
    static Level _logger_level;
    static boost::filesystem::path _logger_filename;
    static std::shared_ptr<std::ofstream> _logger_file;

public:
    static Logger& instance(const std::string& category);

    static void register_callback(std::ostream* const callback);

    // set the type, level, and (optional) filename for the log
    static bool configure(uint32_t type, Level level, const boost::filesystem::path& filename);

    // configures for stdout, no file
    static void configure(Level level);

    static bool config_stdout() { return (_logger_type & LoggerTypeStdout) == LoggerTypeStdout; }
    static bool config_file() { return (_logger_type & LoggerTypeFile) == LoggerTypeFile; }
    //static bool config_level() { return _logger_level; }

    static void set_log_level(Level level) { _logger_level = level; }

    static Level level(const std::string& level);
    static const std::string& level(Level level) throw(std::out_of_range);

private:
    static std::ofstream& logger_file() { return *_logger_file; }

public:
    virtual ~Logger() throw();

public:
    const std::string& category() const { return _category; }
    Level level() const { return _level; }

public:
    Logger& operator<<(std::ostream& (*rhs)(std::ostream&));
    friend Logger& operator<<(Logger& lhs, const Level& level);

    template<typename T> friend Logger& operator<<(Logger& lhs, const T& rhs)
    {
        boost::lock_guard<boost::recursive_mutex> guard(Logger::logger_mutex);

        if(lhs.level() >= _logger_level) {
            if(Logger::config_stdout()) {
                if(lhs.level() >= Logger::Level::Error) {
                    std::cerr << rhs;
                } else {
                    std::cout << rhs;
                }
            }

            if(Logger::config_file()) {
                Logger::logger_file() << rhs;
            }

            for(std::ostream* callback : _callbacks) {
                (*callback) << rhs;
            }
        }
        return lhs;
    }

private:
    std::string _category;
    Level _level;

private:
    Logger();
    explicit Logger(const std::string& category);
    DISALLOW_COPY_AND_ASSIGN(Logger);
};

}

#endif
