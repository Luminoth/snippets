#if !defined __BROADCASTER_H__
#define __BROADCASTER_H__

#include "UdpServer.h"
#include "src/core/xml/XmlDocumentParser.h"
#include "src/core/messages/MessageHandler.h"

namespace energonsoftware {

class Broadcaster : public UdpServer
{
private:
    static Logger& logger;

public:
    Broadcaster(BufferedSender& server, const std::string& address);
    virtual ~Broadcaster() noexcept;

public:
    void register_handler(MessageHandlerModule* module);

    // message should have been allocated with new
    void buffer(BufferedMessage* message, int ttl=1, bool ack=false, unsigned int resend_time=0);

protected:
    virtual bool reuse_port() const { return true; }
    virtual bool on_restart();
    virtual void on_handle_packet(const Socket::BufferType* packet, size_t len, std::shared_ptr<ClientSocket> socket);

    // override this
    virtual bool enable_broadcast();

protected:
    std::string _address;

private:
    BufferedSender& _server;
    XmlDocumentParser _message_parser;
    MessageHandler _message_handler;

private:
    Broadcaster();
    DISALLOW_COPY_AND_ASSIGN(Broadcaster);
};

}

#endif
