#include "src/pch.h"
#include "Event.h"
#include "EventLogger.h"

namespace energonsoftware {

Logger& EventLogger::logger(Logger::instance("energonsoftware.core.eventlogger.EventLogger"));

EventLogger& EventLogger::instance()
{
    static std::shared_ptr<EventLogger> event_logger;
    if(!event_logger) {
        event_logger.reset(new EventLogger());
    }
    return *event_logger;
}

EventLogger::EventLogger()
    : _enabled(false), _packer_type(PackerType::Simple),
        _filename()
{
}

EventLogger::~EventLogger() noexcept
{
}

void EventLogger::init(const boost::filesystem::path& filename)
{
    _filename = filename;
}

void EventLogger::shutdown()
{
    LOG_INFO("Shutting down the event logger...");
}

void EventLogger::enable(bool enable)
{
    LOG_INFO((enable ? "Enabling" : "Disabling") << " event logging...\n");
    _enabled = enable;

    if(_enabled) {
        LOG_INFO("Writing events to " << _filename << "\n");
    }
}

void EventLogger::log(const Event& event) throw(EventLoggerError)
{
    if(!enabled()) {
        return;
    }

    try {
        LOG_DEBUG("Logging event with type=" << event.type().type() << "\n" << event.str() << "\n");

        std::shared_ptr<Packer> packer(Packer::new_packer(_packer_type));
        if(!packer) {
            throw EventLoggerError("Unable to create packer!");
        }
        event.serialize(*packer);

        // TODO: opening and closing the file each event is slow
        // should we just hold the file open for the life of the logger?
        std::ofstream f(_filename.string().c_str(), std::ios::binary);
        f << Packer::type_to_str(_packer_type) << "\n" << packer->buffer();
        f.close();
    } catch(const SerializationError& e) {
        throw EventLoggerError(e.what());
    }
}

void EventLogger::log(const std::vector<Event>& events) throw(EventLoggerError)
{
    if(!enabled()) {
        return;
    }

    for(const Event& event : events) {
        log(event);
    }
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class EventLoggerTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(EventLoggerTest);
        CPPUNIT_TEST(test_log_event);
    CPPUNIT_TEST_SUITE_END();

public:
    EventLoggerTest() : CppUnit::TestFixture() {}
    virtual ~EventLoggerTest() noexcept {}

public:
    void setUp() override
    {
        energonsoftware::EventLogger::instance().init("test.evt");
        energonsoftware::EventLogger::instance().enable();
    }

    void tearDown() override
    {
        energonsoftware::EventLogger::instance().shutdown();
    }

    void test_log_event()
    {
        energonsoftware::Event evt(std::static_pointer_cast<energonsoftware::EventType>(std::make_shared<TestEvent>()));
        energonsoftware::EventLogger::instance().log(evt);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EventLoggerTest);

#endif
