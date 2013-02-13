#if !defined __TCPSESSION_H__
#define __TCPSESSION_H__

#include "BufferedSender.h"
#include "TLSSocket.h"

namespace energonsoftware {

class TcpServer;

class TcpSession : public BufferedSender
{
private:
    static Logger& logger;

public:
    TcpSession(ClientSocket& socket, TcpServer& server, unsigned long sessionid);
    virtual ~TcpSession() throw();

public:
    unsigned long sessionid() const { return _sessionid; }
    bool connected() const { return _connected; }
    bool encrypted() const { return _socket.encrypted(); }

    std::vector<Socket::BufferType>& read_buffer() { return _read_buffer; }
    const std::vector<Socket::BufferType>& read_buffer() const { return _read_buffer; }

    TcpServer& server() { return _server; }
    const TcpServer& server() const { return _server; }

    // runs the session, call each 'frame'
    void run();

    // disconnects and quits the session
    void disconnect(const Socket::BufferType* packet=nullptr, size_t len=0);
    void disconnect(const std::string& packet);

    // closes the socket, but does not disconnect it
    void quit();

    // sends some data
    bool send(const Socket::BufferType* message, size_t len);
    bool send(const std::string& message);

    // tells the client to start tls and handshakes it using the SRP method
    bool start_tls(const Socket::BufferType* packet, size_t len, const boost::filesystem::path& password_file, const boost::filesystem::path& password_conf_file);
    bool start_tls(const std::string& packet, const boost::filesystem::path& password_file, const boost::filesystem::path& password_conf_file);

protected:
    // override these
    virtual void on_run() {}
    virtual void on_quit() {}

private:
    void read_data();
    void write_data();

private:
    TLSSocket _socket;
    TcpServer& _server;
    unsigned long _sessionid;
    bool _connected;
    std::vector<Socket::BufferType> _read_buffer;

public:
    friend bool operator==(unsigned long lhs, const TcpSession& rhs) { return lhs == rhs._sessionid; }
    bool operator==(unsigned long lhs) { return _sessionid == lhs; }

private:
    TcpSession();
    DISALLOW_COPY_AND_ASSIGN(TcpSession);
};

// override this and pass to TcpServer to create
// new sessions of a specific type
class TcpSessionFactory
{
public:
    TcpSessionFactory();
    virtual ~TcpSessionFactory() throw();

public:
    virtual TcpSession* new_session(ClientSocket& socket, TcpServer& server, unsigned long sessionid) const throw();
};

}

#endif
