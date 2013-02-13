#include "src/pch.h"
#include "src/core/util/util.h"
#include "HttpServer.h"
#include "HttpSession.h"

namespace energonsoftware {

Logger& HttpServer::logger(Logger::instance("energonsoftware.core.network.HttpServer"));

HttpServer::HttpServer(HttpSessionFactory* session_factory)
    : TcpServer(session_factory)
{
}

HttpServer::~HttpServer() throw()
{
}

void HttpServer::on_packet(TcpSession& session)
{
    boost::shared_array<Socket::BufferType> packet(new Socket::BufferType[session.read_buffer().size()]);
    std::copy(session.read_buffer().begin(), session.read_buffer().end(), packet.get());

    LOG_DEBUG("Session " << session.sessionid() << " received a packet (" << session.read_buffer().size() << ")\n");
    LOG_DEBUG(bin2hex(reinterpret_cast<const unsigned char*>(packet.get()), session.read_buffer().size()) << "\n");

    HttpSession* http_session = dynamic_cast<HttpSession*>(&session);
    if(!http_session) {
        LOG_CRITICAL("Could not cast session!\n");
        return;
    }

    //std::string message(decode_packet((char*)packet, len));
    std::string message(reinterpret_cast<const char*>(packet.get()), session.read_buffer().size());
    http_session->handle_request(message);
    session.read_buffer().clear();
}

}
