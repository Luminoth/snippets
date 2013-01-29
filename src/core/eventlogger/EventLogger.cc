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
    : _enabled(false), _packer_type(PackerType::Simple)
{
}

EventLogger::~EventLogger() throw()
{
}

void EventLogger::enable(bool enable)
{
    LOG_INFO((enable ? "Enabling" : "Disabling") << " event logging...\n");
    _enabled = enable;
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

        // TODO: write
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
