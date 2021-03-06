#if !defined __SOCKET_H__
#define __SOCKET_H__

#include "network_util.h"

struct hostent;

namespace energonsoftware {

class ServerSocket;
class ClientSocket;

class SocketError : public std::exception
{
public:
    explicit SocketError(const std::string& what) noexcept : _what(what) {}
    virtual ~SocketError() noexcept {}
    virtual const char* what() const noexcept override { return _what.c_str(); }

private:
    std::string _what;
};

class Socket
{
public:
    #if defined WIN32
        typedef char BufferType;
    #else
        typedef unsigned char BufferType;
    #endif

    typedef std::array<BufferType, MAX_BUFFER * 10> Buffer;

public:
    // returns the last error value
    static int last_socket_error();

    // converts a hostname to a sockaddr using the getaddrinfo() method
    static bool host_to_sockaddr(int domain, int type, const std::string& hostname, const std::string& service_name, sockaddr* saddr);

private:
    static Logger& logger;

public:
    Socket();
    explicit Socket(SOCKET sockfd);
    virtual ~Socket() noexcept;
    Socket(const Socket& socket);

public:
    bool create(int domain, int type, int protocol);
    bool shutdown(int how=SHUT_RDWR);
    bool close();

    // compensates for incomplete sends
    bool send(const BufferType* buffer, size_t len, int flags=0);
    bool send(const std::string& buffer, int flags=0);

    // this returns the value of ::recv()
    size_t recv(Buffer& buffer, int flags=0);

#if defined WIN32
    bool setsockopt(int optname, const char* optval, socklen_t optlen, int level=SOL_SOCKET);
    bool getsockopt(int optname, char* optval, socklen_t* optlen, int level=SOL_SOCKET);
#else
    bool setsockopt(int optname, const void* optval, socklen_t optlen, int level=SOL_SOCKET);
    bool getsockopt(int optname, void* optval, socklen_t* optlen, int level=SOL_SOCKET);
#endif

    bool set_asynchronous();
    bool get_asynchronous();

    bool set_synchronous();
    bool get_synchronous();

    bool set_keepalive();

#if defined WIN32
    // calls WSAEventSelect
    bool event_select(WSAEVENT hEventObject, long lNetworkEvents);
#endif

public:
    Socket& operator=(const Socket& rhs);

public:
    SOCKET socket() const { return _sockfd; }
    const sockaddr_in& addr() const { return _addr; }
    int domain() const { return _domain; }
    int type() const { return _type; }
    bool valid() const { return _sockfd > INVALID_SOCKET; }
    const std::string& host() const { return _host; }
    unsigned short port() const { return _port; }

public:
    bool operator==(const Socket& rhs);
    bool operator!=(const Socket& rhs);
    bool operator==(const SOCKET& rhs);
    bool operator!=(const SOCKET& rhs);
    friend bool operator==(const SOCKET& lhs, const Socket& rhs);
    friend bool operator!=(const SOCKET& lhs, const Socket& rhs);

protected:
    // override these to provide some other way of doing them
    virtual size_t do_send(const BufferType* buffer, size_t len, int flags);
    virtual size_t do_recv(BufferType* buffer, size_t len, int flags);
    virtual int do_shutdown(int how);

protected:
    void create_addr(unsigned short port) throw(SocketError);
    void create_addr(const std::string& host, unsigned short port) throw(SocketError);

protected:
    sockaddr_in _addr;
    std::string _host;
    unsigned short _port;

private:
    SOCKET _sockfd;
    int _domain;
    int _type;
};

bool operator==(const SOCKET& lhs, const Socket& rhs);
bool operator!=(const SOCKET& lhs, const Socket& rhs);

class ClientSocketError : public SocketError
{
public:
    explicit ClientSocketError(const std::string& what) noexcept : SocketError(what) {}
    virtual ~ClientSocketError() noexcept {}
};

class ClientSocket : public Socket
{
public:
    // creates a simple client socket and connects it to the given host
    static void create_client_socket(ClientSocket& socket, int domain, int type, int protocol, const std::string& host, unsigned short port) throw(SocketError);

private:
    static Logger& logger;

public:
    ClientSocket();
    explicit ClientSocket(sockaddr_in& addr);
    explicit ClientSocket(SOCKET sockfd);
    virtual ~ClientSocket() noexcept;

public:
    // calls ::connect()
    virtual bool connect(const std::string& host, unsigned short port) throw(SocketError);

    // these assume connect() has been called (they use a nullptr dest address)
    // and compensates for incomplete sends
    bool sendto(const BufferType* buffer, size_t len, int flags=0);
    bool sendto(const std::string& buffer, int flags=0);
};

class ServerSocketError : public SocketError
{
public:
    explicit ServerSocketError(const std::string& what) noexcept : SocketError(what) {}
    virtual ~ServerSocketError() noexcept {}
};

class ServerSocket : public ClientSocket
{
public:
    // creates a simple server socket
    // sets the socket to reuse addresses and calls bind() if port > 0 and listen() if type == SOCK_STREAM
    static void create_server_socket(ServerSocket& socket, int domain, int type, int protocol, unsigned short port, bool reuse_port=false) throw(ServerSocketError);

private:
    static Logger& logger;

public:
    ServerSocket();
    explicit ServerSocket(sockaddr_in& addr) { throw std::runtime_error("Invalid creation of a server socket"); };
    explicit ServerSocket(SOCKET sockfd);
    virtual ~ServerSocket() noexcept;

public:
    virtual bool connect(const std::string& host, unsigned short port) throw(SocketError) override
        { throw SocketError("Invalid connect() on a server socket"); }

    // calls ::bind()
    bool bind(unsigned short port) throw(SocketError);
    bool bind(const std::string& address, unsigned short port) throw(SocketError);

    // calls ::listen()
    bool listen(int backlog=SOMAXCONN);

    // calls ::accept()
    ClientSocket accept();

    // this assumes the socket addr has been set in some way
    // and compensates for incomplete sends
    bool sendto(const BufferType* buffer, size_t len, const ClientSocket& socket, int flags=0);
    bool sendto(const std::string& buffer, const ClientSocket& socket, int flags=0);

    // returns how much was read and the socket it was from
    std::pair<size_t, std::shared_ptr<ClientSocket>> recvfrom(Buffer& buffer, int flags=0);

protected:
    // override these to provide some other way of doing them
    virtual std::pair<size_t, std::shared_ptr<ClientSocket>> do_recvfrom(BufferType* buffer, size_t len, int flags);
};

}

#endif
