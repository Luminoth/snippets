#include "src/pch.h"
#include "src/core/messages/XmlMessage.h"
#include "src/core/util/util.h"
#include "Broadcaster.h"

namespace energonsoftware {

Logger& Broadcaster::logger(Logger::instance("energonsoftware.core.network.Broadcaster"));

Broadcaster::Broadcaster(BufferedSender& server, const std::string& address)
    : UdpServer(), _address(address), _server(server),
        _message_parser(), _message_handler()
{
}

Broadcaster::~Broadcaster() noexcept
{
}

void Broadcaster::register_handler(MessageHandlerModule* module)
{
    _message_handler.register_handler(module);
}

void Broadcaster::buffer(BufferedMessage* message, int ttl, bool ack, unsigned int resend_time)
{
    UdpServer::buffer(message, _socket, ttl, ack, resend_time);
}

bool Broadcaster::on_restart()
{
    if(!enable_broadcast()) {
        LOG_CRITICAL("Could not enable broadcasting!\n");
        return false;
    }

    return true;
}

void Broadcaster::on_handle_packet(const Socket::BufferType* packet, size_t len, std::shared_ptr<ClientSocket> socket)
{
    LOG_DEBUG("Received a packet from " << socket->host() << ":" << socket->port() << "\n");
    LOG_DEBUG(bin2hex(reinterpret_cast<const unsigned char*>(packet), len) << "\n");

    try {
        _message_parser.feed(XmlString(decode_packet(const_cast<char*>(packet), len)));
        if(!_message_parser.complete())
            return;

        std::unique_ptr<XmlDocument> parsed_message(_message_parser.document());
        _message_parser.reset();

        XmlMessage message(*parsed_message);
        LOG_DEBUG("Parsed message: " << message.document().to_native() << "\n");
        if(!_message_handler.handle_message(message, &_server)) {
            // log something?
        }
    } catch(const XmlDocumentParserError& e) {
        LOG_WARNING("Encountered an xml document parser error: " << e.what() << "\n");
    } catch(const MessageHandlerError& e) {
        LOG_WARNING("Encountered a message handler error: " << e.what() << "\n");
    } catch(...) {
        LOG_WARNING("Encountered an unknown error\n");
    }
}

bool Broadcaster::enable_broadcast()
{
    LOG_INFO("Enabling broadcast...\n");

#if defined WIN32
    char ival = 0x01;
#else
    int ival = 0x01;
#endif
    if(!_socket->setsockopt(SO_BROADCAST, &ival, sizeof(ival))) {
        LOG_CRITICAL("Could not enable broadcast: " << last_error(Socket::last_socket_error()) << "\n");
        return false;
    }

    return true;
}

}
