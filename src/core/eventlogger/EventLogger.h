#if !defined __EVENTLOGGER_H__
#define __EVENTLOGGER_H__

namespace energonsoftware {

class EventLoggerError : public std::exception
{
public:
    explicit EventLoggerError(const std::string& what) throw() : _what(what) {}
    virtual ~EventLoggerError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

private:
    std::string _what;
};

class Event;

class EventLogger
{
public:
    static EventLogger& instance();

private:
    static Logger& logger;

public:
    virtual ~EventLogger() throw();

public:
    void init(const boost::filesystem::path& filename);
    void shutdown();

    // NOTE: defaults to disabled
    bool enabled() const { return _enabled; }
    void enable(bool enable=true);

    // defaults to the simple packer
    PackerType packer_type() const { return _packer_type; }
    void packer_type(PackerType type) { _packer_type = type; }

    void log(const Event& event) throw(EventLoggerError);

    // TODO: this should take iterator endpoints
    void log(const std::vector<Event>& events) throw(EventLoggerError);

private:
    bool _enabled;
    PackerType _packer_type;

    boost::filesystem::path _filename;

private:
    EventLogger();
    DISALLOW_COPY_AND_ASSIGN(EventLogger);
};

}

#endif
