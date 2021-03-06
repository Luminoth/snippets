#if !defined __MULTICASTER_H__
#define __MULTICASTER_H__

#include "Broadcaster.h"

namespace energonsoftware {

class Multicaster : public Broadcaster
{
private:
    static Logger& logger;

public:
    Multicaster(BufferedSender& server, const std::string& address);
    virtual ~Multicaster() noexcept;

private:
    virtual bool enable_broadcast() override;

private:
    Multicaster() = delete;
    DISALLOW_COPY_AND_ASSIGN(Multicaster);
};

}

#endif
