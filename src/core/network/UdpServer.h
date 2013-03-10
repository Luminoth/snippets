#if !defined __UDPSERVER_H__
#define __UDPSERVER_H__

#include "src/core/messages/BufferedMessage.h"
#include "src/core/messages/UdpMessage.h"
#include "BufferedSender.h"
#include "Socket.h"

namespace energonsoftware {

class UdpMessage;
class UdpMessageFactory;

class UdpServer : public BufferedSender
{
private:
    class UdpServerMessage : public UdpMessage
    {
    public:
        UdpServerMessage(const Socket::BufferType* packet, size_t len, size_t packetid, size_t mtu, int ttl, std::shared_ptr<ClientSocket> socket, unsigned int resend_time, bool encode, bool ack);
        virtual ~UdpServerMessage() throw();

    public:
        std::shared_ptr<ClientSocket> socket() { return _socket; }
        bool ack() const { return _ack; }
        unsigned int resend_time() const { return _resend_time; }
        bool resendable() const;

        // call every time the packet is sent
        void resent();

    private:
        std::shared_ptr<ClientSocket> _socket;
        bool _ack;
        unsigned int _resend_time;
        double _send_time;

    private:
        UdpServerMessage();
        DISALLOW_COPY_AND_ASSIGN(UdpServerMessage);
    };

private:
    typedef std::unordered_map<unsigned long, std::shared_ptr<UdpServerMessage> > AckPacketMap;

private:
    static Logger& logger;

public:
    explicit UdpServer();
    virtual ~UdpServer() throw();

public:
    unsigned short port() const { return _port; }
    bool running() const { return _running; }

    // resets the state of the server
    bool restart(unsigned short port=0, size_t mtu=512);

    // runs the server, call each 'frame'
    void run();

    void quit();

    // overridden because we encapsulate udp messages
    // message should have been allocated with new
    void buffer(BufferedMessage* message, std::shared_ptr<ClientSocket> socket, int ttl=1, bool ack=false, unsigned int resend_time=0);

    bool send(const Socket::BufferType* message, size_t len, ClientSocket& socket);
    bool send(const std::string& message, ClientSocket& socket);

    void ack(unsigned long seqid);

    unsigned long next_packet_id();

protected:
    // override these
    virtual bool reuse_port() const { return false; }
    virtual bool on_restart() { return true; }
    virtual void on_run() {}
    virtual void on_quit() {}
    virtual void on_handle_packet(const Socket::BufferType* packet, size_t len, std::shared_ptr<ClientSocket> socket) {}

private:
    void read_data();
    bool send_packet(const UdpMessage& packet, ClientSocket& socket);
    void write_data();
    bool create_socket();

protected:
    std::shared_ptr<ServerSocket> _socket;
    unsigned short _port;

private:
    size_t _mtu;
    bool _running;
    unsigned long _packet_count;
    AckPacketMap _ack_packets;
    std::shared_ptr<UdpMessageFactory> _message_factory;

private:
    DISALLOW_COPY_AND_ASSIGN(UdpServer);
};

}

#endif
