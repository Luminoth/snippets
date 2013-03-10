#include "src/pch.h"
#include "src/core/util/util.h"
#include "TcpClient.h"

namespace energonsoftware {

Logger& TcpClient::logger(Logger::instance("energonsoftware.core.network.TcpClient"));

TcpClient::TcpClient()
    : BufferedSender(), _socket(), _host(), _port(0), _connected(false),
        _read_buffer()
{
}

TcpClient::~TcpClient() noexcept
{
}

bool TcpClient::connect(const std::string& host, unsigned short port)
{
    if(connected()) {
        disconnect();
    }

    _host = host;
    _port = port;

    try {
        ClientSocket::create_client_socket(_socket, AF_INET, SOCK_STREAM, IPPROTO_TCP, _host, _port);
    } catch(const SocketError& e) {
        LOG_ERROR("Could not connect to host: " << e.what() << "\n");
        return false;
    }

    _socket.set_keepalive();

    on_connect();

    _connected = true;
    return true;
}

void TcpClient::disconnect(const Socket::BufferType* packet, size_t len)
{
    if(connected()) {
        LOG_INFO("Disconnecting...\n");
        if(nullptr != packet && len > 0) {
            std::string encoded(encode_packet((char*)packet, len));
            _socket.send(reinterpret_cast<const Socket::BufferType*>(encoded.c_str()), encoded.length());
        }
    }
    quit();
}

void TcpClient::disconnect(const std::string& packet)
{
    disconnect(reinterpret_cast<const Socket::BufferType*>(packet.c_str()), packet.length());
}

bool TcpClient::start_tls(const std::string& username, const std::string& password)
{
    LOG_INFO("Negotiating TLS...\n");
    if(!_socket.handshake_srp_client(username, password)) {
        disconnect();
        return false;
    }

    LOG_INFO("Success!\n");
    return true;
}

void TcpClient::run()
{
    read_data();
    write_data();
    on_run();
}

void TcpClient::quit()
{
    on_quit();

    if(connected()) {
        _socket.shutdown();
        _socket.close();
        _connected = false;
    }

    reset_buffer();
}

bool TcpClient::send(const Socket::BufferType* message, size_t len)
{
    if(!connected()) {
        return false;
    }

    LOG_DEBUG("Sending a message (" << len << ")...\n");
    LOG_DEBUG(bin2hex(reinterpret_cast<const unsigned char*>(message), len) << "\n");

    return _socket.send(message, len);
}

bool TcpClient::send(const std::string& message)
{
    return send(reinterpret_cast<const Socket::BufferType*>(message.c_str()), message.length());
}

void TcpClient::read_data()
{
    Socket::Buffer buffer;
    while(connected() && poll_socket_read(_socket.socket())) {
        buffer.fill(0);

        int len = _socket.recv(buffer);
        if(len <= 0) {
            LOG_ERROR("Error reading from socket: " << last_error(Socket::last_socket_error()) << "\n");
            disconnect();
            break;
        }

        _read_buffer.insert(_read_buffer.end(), buffer.data(), buffer.data() + len);
    }

    if(_read_buffer.size() > 0) {
        on_packet();
    }
}

void TcpClient::write_data()
{
    while(connected() && !buffer_empty()) {
        bool success = false;

        const unsigned char* message = current_buffer();
        if(current_buffer_encoded()) {
            std::string encoded(encode_packet((char*)message, current_buffer_len()));
            success = send(reinterpret_cast<const Socket::BufferType*>(encoded.c_str()), encoded.length());
        } else {
            success = send(reinterpret_cast<const Socket::BufferType*>(message), current_buffer_len());
        }

        if(!success) {
            LOG_ERROR("Error writing to socket: " << last_error(Socket::last_socket_error()) << "\n");
            disconnect();
            return;
        }

        update_sent(current_buffer_len());
    }
}

}
