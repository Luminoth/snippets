#include "src/pch.h"
#include "src/core/messages/UdpMessageFactory.h"
#include "src/core/messages/UdpMessage.h"
#include "src/core/util/util.h"
#include "UdpServer.h"

namespace energonsoftware {

UdpServer::UdpServerMessage::UdpServerMessage(const unsigned char* packet, size_t len, size_t packetid, size_t mtu, int ttl, std::shared_ptr<ClientSocket> socket, unsigned int resend_time, bool encode, bool ack)
    : UdpMessage(packet, len, packetid, mtu, encode, ttl),
        _socket(socket), _ack(ack), _resend_time(resend_time), _send_time(get_time())
{
}

UdpServer::UdpServerMessage::~UdpServerMessage() noexcept
{
}

bool UdpServer::UdpServerMessage::resendable() const
{
    return get_time() > (_send_time + resend_time());
}

void UdpServer::UdpServerMessage::resent()
{
    _send_time = get_time();
}

Logger& UdpServer::logger(Logger::instance("energonsoftware.core.network.UdpServer"));

UdpServer::UdpServer()
    : _socket(), _port(0), _mtu(512), _running(false), _packet_count(0),
        _ack_packets(), _message_factory()
{
}

UdpServer::~UdpServer() noexcept
{
}

bool UdpServer::restart(unsigned short port, size_t mtu)
{
    quit();

    _port = port;
    _mtu = mtu;
    _packet_count = 0;
    _ack_packets.clear();
    _message_factory.reset(new UdpMessageFactory());

    if(!create_socket())
        return false;

    _running = on_restart();
    return _running;
}

void UdpServer::run()
{
    if(running())
        read_data();

    // drop the expired packets
    if(running() && _message_factory) {
        std::vector<std::shared_ptr<UdpMessageFactory::FactoryMessage> > expired;
        _message_factory->expired(expired);
    }

    if(running())
        write_data();

    if(running())
        on_run();
}

void UdpServer::quit()
{
    on_quit();
    if(running()) {
        LOG_INFO("Quitting...\n");
        _socket->close();
    }

    _running = false;
    reset_buffer();
    _message_factory.reset();
}

void UdpServer::buffer(BufferedMessage* message, std::shared_ptr<ClientSocket> socket, int ttl, bool ack, unsigned int resend_time)
{
    message->reset();
    BufferedSender::buffer(new UdpServerMessage(message->start(), message->full_len(), next_packet_id(),
        _mtu, ttl, socket, resend_time, message->encode(), ack));
}

unsigned long UdpServer::next_packet_id()
{
    if(_packet_count >= UdpMessage::MAX_PACKET_ID)
        _packet_count = 0;
    return ++_packet_count;
}

bool UdpServer::send(const unsigned char* message, size_t len, ClientSocket& socket)
{
    if(!running()) {
        return false;
    }

    LOG_DEBUG("Sending a message to " << socket.host() << ":" << socket.port() << "...\n");
    if(len > _mtu) {
        LOG_CRITICAL("Attempted to send message greater than MTU (" << _mtu << " bytes): " << len << "\n");
        return false;
    }

    LOG_DEBUG(bin2hex(message, len) << "\n");
    return _socket->sendto(message, len, socket);
}

bool UdpServer::send(const std::string& message, ClientSocket& socket)
{
    return send(reinterpret_cast<const unsigned char*>(message.c_str()), message.length(), socket);
}

void UdpServer::ack(unsigned long seqid)
{
    //try {
        _ack_packets.erase(seqid);
    /*} catch(const std::out_of_range&) {
        LOG_WARNING("Acknowledging non-existant packet " << seqid << "!\n");
    }*/
}

void UdpServer::read_data()
{
    while(running() && poll_socket_read(_socket->socket())) {
        Socket::Buffer buffer;
        buffer.fill(0);

        // read something
        std::pair<size_t, std::shared_ptr<ClientSocket> > recvd(_socket->recvfrom(buffer));
        if(recvd.first <= 0) {
            LOG_ERROR("Client closed connection? wtf?\n");
            return;
        }

        // copy what we read into a dynamic buffer
        std::shared_ptr<unsigned char> data(new unsigned char[recvd.first]);
        std::memcpy(data.get(), buffer.data(), recvd.first);

        // append the chunk
        if(_message_factory) {
            UdpMessage::UdpMessageChunk chunk(recvd.first, data);
            if(!_message_factory->append(chunk, recvd.second)) {
                LOG_WARNING("Could not append chunk, handling directly\n");
                on_handle_packet(buffer.data(), recvd.first, recvd.second);
            }
        }
    }

    // handle any completed messages
    if(running() && _message_factory) {
        std::vector<std::shared_ptr<UdpMessageFactory::FactoryMessage> > completed;
        _message_factory->complete(completed);
        for(const std::shared_ptr<UdpMessageFactory::FactoryMessage>& message : completed) {
            on_handle_packet(message->message(), message->message_len(), message->socket());
        }
    }
}

bool UdpServer::send_packet(const UdpMessage& packet, ClientSocket& socket)
{
    /*if(!socket.connected())
        return false;*/

    std::vector<UdpMessage::UdpMessageChunk> chunks;
    if(!packet.chunks(chunks)) {
        LOG_WARNING("Failed to get message chunks!\n");
        return false;
    }

    bool success = true;
    for(const UdpMessage::UdpMessageChunk& chunk : chunks) {
        if(packet.encode()) {
            std::string encoded(encode_packet((char*)chunk.second.get(), chunk.first));
            success &= send(reinterpret_cast<const unsigned char*>(encoded.c_str()), encoded.length(), socket);
        } else {
            success &= send(chunk.second.get(), chunk.first, socket);
        }
    }

    return success;
}

void UdpServer::write_data()
{
    // send any ack packets that need resent
    for(const AckPacketMap::value_type& packet : _ack_packets) {
        if(running() && packet.second->resendable()) {
            //LOG_DEBUG("Resending packet: " << packet.second->start() << "\n");
            if(send_packet(*(packet.second), *(packet.second->socket())))
                packet.second->resent();
        }
    }

    // send any packets we have buffered
    while(running() && !buffer_empty()) {
        std::shared_ptr<UdpServerMessage> message(std::dynamic_pointer_cast<UdpServerMessage, BufferedMessage>(current_message()));
        if(!message) {
            LOG_CRITICAL("UdpServer attempting to send non-UdpMessage!\n");
            return;
        }

        //LOG_DEBUG("Popped packet; " << message->start() << "\n");
        if(send_packet(*message, *(message->socket()))) {
            clear_current();
            if(message->ack() && message->has_seqid()) {
                message->resent();
                _ack_packets[message->seqid()] = message;
            }
        } else {
            LOG_ERROR("Client closed connection!\n");
        }
    }
}

bool UdpServer::create_socket()
{
    try {
        _socket.reset(new ServerSocket());
        ServerSocket::create_server_socket(*_socket, AF_INET, SOCK_DGRAM, IPPROTO_UDP, _port, reuse_port());
    } catch(const SocketError& e) {
        LOG_ERROR("Could not create socket: " << e.what() << "\n");
        return false;
    }

    return true;
}

}
