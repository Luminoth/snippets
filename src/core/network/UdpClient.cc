#include "src/pch.h"
#include "src/core/messages/UdpMessage.h"
#include "src/core/util/util.h"
#include "UdpClient.h"

namespace energonsoftware {

Logger& UdpClient::logger(Logger::instance("energonsoftware.core.network.UdpClient"));

UdpClient::UdpClient()
    : BufferedSender(), mtu(512), _host(), _port(0), _connected(false),
        _socket(), _packet_count(0), _message_factory()
{
}

UdpClient::~UdpClient() noexcept
{
}

bool UdpClient::connect(const std::string& host, unsigned int port)
{
    if(connected())
        disconnect();

    _host = host;
    _port = port;

    try {
        _socket.reset(new ClientSocket());
        ClientSocket::create_client_socket(*_socket, AF_INET, SOCK_DGRAM, IPPROTO_UDP, _host, _port);
    } catch(const SocketError& e) {
        LOG_ERROR("Could not connect to host: " << e.what() << "\n");
        return false;
    }

    on_connect();

    _connected = true;
    return true;
}

void UdpClient::disconnect(const Socket::BufferType* packet, size_t len)
{
    if(connected()) {
        LOG_INFO("Disconnecting...\n");
        if(nullptr != packet && len > 0) {
            std::string encoded(encode_packet((char*)packet, len));
            send(reinterpret_cast<const Socket::BufferType*>(encoded.c_str()), encoded.length());
        }
    }
    quit();
}

void UdpClient::disconnect(const std::string& packet)
{
    disconnect(reinterpret_cast<const Socket::BufferType*>(packet.c_str()), packet.length());
}

void UdpClient::run()
{
    read_data();

    // drop the expired packets
    std::vector<std::shared_ptr<UdpMessageFactory::FactoryMessage> > expired;
    _message_factory.expired(expired);

    write_data();
    on_run();
}

void UdpClient::quit()
{
    on_quit();

    if(connected()) {
        _socket->close();
        _connected = false;
    }
    _packet_count = 0;

    reset_buffer();
}

bool UdpClient::send(const Socket::BufferType* message, size_t len)
{
    if(!connected())
        return false;

    LOG_DEBUG("Sending a message to " << host() << ":" << port() << "...\n");
    if(len > mtu) {
        LOG_CRITICAL("Attempted to send message greater than MTU (" << mtu << " bytes): " << len << "\n");
        return false;
    }

    LOG_DEBUG(bin2hex(reinterpret_cast<const unsigned char*>(message), len) << "\n");
    return _socket->sendto(message, len);
}

bool UdpClient::send(const std::string& message)
{
    return send(reinterpret_cast<const Socket::BufferType*>(message.c_str()), message.length());
}

void UdpClient::buffer(BufferedMessage* message, int ttl)
{
    message->reset();
    BufferedSender::buffer(new UdpMessage(reinterpret_cast<const Socket::BufferType*>(message->start()), message->full_len(), next_packet_id(), mtu, message->encode(), ttl));
}

unsigned long UdpClient::next_packet_id()
{
    if(_packet_count >= UdpMessage::MAX_PACKET_ID)
        _packet_count = 0;

    _packet_count++;
    return _packet_count;
}

void UdpClient::read_data()
{
    while(connected() && poll_socket_read(_socket->socket())) {
        Socket::Buffer buffer;
        buffer.fill(0);

        // read something
        size_t len = _socket->recv(buffer);
        if(len <= 0) {
            LOG_ERROR("Error reading from socket: " << last_error(Socket::last_socket_error()) << "\n");
            disconnect();
            break;
        }

        // copy what we read into a dynamic buffer
        std::shared_ptr<Socket::BufferType> data(new Socket::BufferType[len]);
        std::memcpy(data.get(), buffer.data(), len);

        // append the chunk
        UdpMessage::UdpMessageChunk chunk(len, data);
        if(!_message_factory.append(chunk, _socket)) {
            on_packet(buffer.data(), len);
        }
    }

    // handle any completed messages
    std::vector<std::shared_ptr<UdpMessageFactory::FactoryMessage> > completed;
    _message_factory.complete(completed);
    for(const std::shared_ptr<UdpMessageFactory::FactoryMessage>& message : completed) {
        on_packet(message->message(), message->message_len());
    }
}

bool UdpClient::send_packet(const UdpMessage& packet)
{
    if(!connected())
        return false;

    std::vector<UdpMessage::UdpMessageChunk> chunks;
    if(!packet.chunks(chunks))
        return false;

    bool success = true;
    for(const UdpMessage::UdpMessageChunk& chunk : chunks) {
        if(packet.encode()) {
            std::string encoded(encode_packet((char*)chunk.second.get(), chunk.first));
            success &= send(reinterpret_cast<const Socket::BufferType*>(encoded.c_str()), encoded.length());
        } else {
            success &= send(chunk.second.get(), chunk.first);
        }
    }

    return success;
}

void UdpClient::write_data()
{
    while(connected() && !buffer_empty()) {
        std::shared_ptr<UdpMessage> message(std::dynamic_pointer_cast<UdpMessage, BufferedMessage>(current_message()));
        if(!message) {
            LOG_CRITICAL("UdpClient attempting to send non-UdpMessage!\n");
            return;
        }

        //LOG_DEBUG("Popped packet: " << message->start() << "\n");
        if(send_packet(*message)) {
            clear_current();
        } else {
            LOG_ERROR("Error writing to socket: " << last_error(Socket::last_socket_error()) << "\n");
            disconnect();
        }
    }
}

}
