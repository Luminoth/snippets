#include "src/pch.h"
#include "src/core/util/util.h"
#include "TcpSession.h"
#include "TcpServer.h"

namespace energonsoftware {

Logger& TcpSession::logger(Logger::instance("energonsoftware.core.network.TcpSession"));

TcpSession::TcpSession(ClientSocket& socket, TcpServer& server, unsigned long sessionid)
    : BufferedSender(), _socket(socket), _server(server), _sessionid(sessionid), _connected(true),
        _read_buffer()
{
}

TcpSession::~TcpSession() noexcept
{
}

void TcpSession::run()
{
    read_data();
    write_data();
    on_run();
}

void TcpSession::disconnect(const Socket::BufferType* packet, size_t len)
{
    if(connected()) {
        LOG_INFO("Session " << sessionid() << " is disconnecting...\n");
        if(nullptr != packet && len > 0) {
            std::string encoded(encode_packet((char*)packet, len));
            send(reinterpret_cast<const Socket::BufferType*>(encoded.c_str()), encoded.length());
        }
    }
    quit();
}

void TcpSession::disconnect(const std::string& packet)
{
    disconnect(reinterpret_cast<const Socket::BufferType*>(packet.c_str()), packet.length());
}

void TcpSession::quit()
{
    on_quit();

    if(connected()) {
        _socket.shutdown();
        _socket.close();
    }

    _connected = false;
    reset_buffer();
}

bool TcpSession::send(const Socket::BufferType* message, size_t len)
{
    if(!connected()) {
        return false;
    }

    LOG_DEBUG("Session " << sessionid() << " is sending a message (" << len << ")...\n");
    LOG_DEBUG(bin2hex(reinterpret_cast<const unsigned char*>(message), len) << "\n");

    return _socket.send(message, len);
}

bool TcpSession::send(const std::string& message)
{
    return send(reinterpret_cast<const Socket::BufferType*>(message.c_str()), message.length());
}

bool TcpSession::start_tls(const Socket::BufferType* packet, size_t len, const boost::filesystem::path& password_file, const boost::filesystem::path& password_conf_file)
{
    std::string encoded(encode_packet((char*)packet, len));
    send(reinterpret_cast<const Socket::BufferType*>(encoded.c_str()), encoded.length());

    LOG_INFO("Session " << sessionid() << " is negotiating TLS...\n");
    if(!_socket.handshake_srp_server(password_file, password_conf_file)) {
        disconnect();
        return false;
    }

    LOG_INFO("Success!\n");
    return true;
}

bool TcpSession::start_tls(const std::string& packet, const boost::filesystem::path& password_file, const boost::filesystem::path& password_conf_file)
{
    return start_tls(reinterpret_cast<const Socket::BufferType*>(packet.c_str()), packet.length(), password_file, password_conf_file);
}

void TcpSession::read_data()
{
    Socket::Buffer buffer;
    while(connected() && poll_socket_read(_socket.socket())) {
        buffer.fill(0);

        int len = _socket.recv(buffer);
        if(len <= 0) {
            LOG_ERROR("Session " << sessionid() << " closed connection!\n");
            disconnect();
            break;
        }

        _read_buffer.insert(_read_buffer.end(), buffer.data(), buffer.data() + len);
    }

    if(_read_buffer.size() > 0) {
        _server.on_packet(*this);
    }
}

void TcpSession::write_data()
{
    while(connected() && !buffer_empty()) {
        bool success = false;

        const Socket::BufferType* message = current_buffer();
        if(current_buffer_encoded()) {
            std::string encoded(encode_packet((char*)message, current_buffer_len()));
            success = send(reinterpret_cast<const Socket::BufferType*>(encoded.c_str()), encoded.length());
        } else {
            success = send(reinterpret_cast<const Socket::BufferType*>(message), current_buffer_len());
        }

        if(!success) {
            LOG_ERROR("Session " << sessionid() << " closed connection!\n");
            disconnect();
            return;
        }

        update_sent(current_buffer_len());
    }
}

TcpSessionFactory::TcpSessionFactory()
{
}

TcpSessionFactory::~TcpSessionFactory() noexcept
{
}

TcpSession* TcpSessionFactory::new_session(ClientSocket& socket, TcpServer& server, unsigned long sessionid) const noexcept
{
    return new TcpSession(socket, server, sessionid);
}

}
