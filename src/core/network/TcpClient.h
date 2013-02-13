#if !defined __TCPCLIENT_H__
#define __TCPCLIENT_H__

#include "BufferedSender.h"
#include "TLSSocket.h"

namespace energonsoftware {

class TcpClient : public BufferedSender
{
private:
    static Logger& logger;

public:
    TcpClient();
    virtual ~TcpClient() throw();

public:
    const std::string& host() const { return _host; }
    unsigned short port() const { return _port; }
    bool connected() const { return _connected; }
    bool encrypted() const { return _socket.encrypted(); }

    // connects to the specified host:port
    bool connect(const std::string& host, unsigned short port);

    // disconnects from the server and quits the session
    void disconnect(const Socket::BufferType* packet=nullptr, size_t len=0);
    void disconnect(const std::string& packet);

    // tls handshakes the server using the SRP method
    bool start_tls(const std::string& username, const std::string& password);

    // runs the client, call each 'frame'
    void run();

    // closes the socket, but does not disconnect it
    void quit();

    // sends some data
    bool send(const Socket::BufferType* message, size_t len);
    bool send(const std::string& message);

protected:
    std::vector<Socket::BufferType>& read_buffer() { return _read_buffer; }
    const std::vector<Socket::BufferType>& read_buffer() const { return _read_buffer; }

protected:
    // override these
    virtual void on_connect() {}
    virtual void on_run() {}
    virtual void on_packet() {}
    virtual void on_quit() {}

private:
    void read_data();
    void write_data();

private:
    TLSSocket _socket;
    std::string _host;
    unsigned short _port;
    bool _connected;
    std::vector<Socket::BufferType> _read_buffer;

private:
    DISALLOW_COPY_AND_ASSIGN(TcpClient);
};

}

#endif
