#if !defined __UDPCLIENT_H__
#define __UDPCLIENT_H__

#include "src/core/messages/UdpMessageFactory.h"
#include "BufferedSender.h"
#include "Socket.h"

namespace energonsoftware {

class UdpClient : public BufferedSender
{
private:
    static Logger& logger;

public:
    explicit UdpClient();
    virtual ~UdpClient() throw();

public:
    const std::string& host() const { return _host; }
    unsigned int port() const { return _port; }
    bool connected() const { return _connected; }

    // connects to the specified host:port
    bool connect(const std::string& host, unsigned int port);

    // disconnects from teh server and quits the session
    void disconnect(const Socket::BufferType* packet=nullptr, size_t len=0);
    void disconnect(const std::string& packet);

    // runs the client, call each 'frame'
    void run();

    // closes the socket, but does not disconnect it
    void quit();

    // sends some data
    bool send(const Socket::BufferType* message, size_t len);
    bool send(const std::string& message);

    // overriden because we encapsulate udp messages
    // message should have been declared with new
    void buffer(BufferedMessage* message, int ttl=1);

    unsigned long next_packet_id();

protected:
    // override these
    virtual void on_connect() {}
    virtual void on_run() {}
    virtual void on_packet(const Socket::BufferType* packet, size_t len) {}
    virtual void on_quit() {}

private:
    void read_data();
    bool send_packet(const UdpMessage& packet);
    void write_data();

public:
    size_t mtu;

protected:
    std::string _host;
    unsigned int _port;

private:
    bool _connected;
    std::shared_ptr<ClientSocket> _socket;
    unsigned long _packet_count;
    UdpMessageFactory _message_factory;

private:
    DISALLOW_COPY_AND_ASSIGN(UdpClient);
};

}

#endif
