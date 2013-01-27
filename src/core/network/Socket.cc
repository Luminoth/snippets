#include "src/pch.h"
#include <errno.h>
#include <fcntl.h>

#if !defined WIN32
    #include <arpa/inet.h>
    #include <netdb.h>
#endif

#include "src/core/util/util.h"
#include "Socket.h"

namespace energonsoftware {

int Socket::last_socket_error()
{
#if defined WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

hostent* Socket::gethostent(int domain, const std::string& host)
{
#if defined WIN32
    return gethostbyname(host.c_str());
#else
    /*hostent haddr, *result;
    size_t hblen = 1024;
    char* hostbuf = malloc(hblen);
    int res, herr;
    while((res = gethostbyname2_r(host.c_str(), domain, &haddr, hostbuf, hblen, &result, &herr)) == ERANGE) {
        hblen <<= 1;
        hostbuf = realloc(hostbuf, hblen);
    }
    free(hostbuf);

    if(0 != res || nullptr == result) {
        return nullptr;
    }
    return result;*/
return gethostbyname2(host.c_str(), domain);
#endif
}

bool Socket::host_to_inaddr(int domain, const std::string& host, void* iaddr, size_t size)
{
#if defined WIN32
    in_addr* saddr = reinterpret_cast<in_addr*>(iaddr);
    assert(saddr);

    // try IP address
    saddr->s_addr = inet_addr(host.c_str());
    if(saddr->s_addr == INADDR_NONE)
#else
    // try IP address
    int rval = inet_pton(domain, host.c_str(), iaddr);
    if(rval < 0) {
        return false;
    } else if(!rval)
#endif
    {
        // failed, try hostname
        hostent* haddr = gethostent(domain, host.c_str());
        if(!haddr) return false;

        // copy the in_addr
        std::memcpy(iaddr, haddr->h_addr_list[0], size);
    }
    return true;
}

Logger& Socket::logger(Logger::instance("energonsoftware.core.network.Socket"));

Socket::Socket()
    : _addr(), _host(), _port(0), _sockfd(INVALID_SOCKET), _domain(0)
{
}

Socket::Socket(SOCKET sockfd)
    : _addr(), _host(), _port(0), _sockfd(sockfd), _domain(0)
{
}

Socket::Socket(const Socket& socket)
    : _addr(), _host(socket._host), _port(socket._port), _sockfd(socket._sockfd), _domain(socket._domain)
{
    std::memmove(&_addr, &socket._addr, sizeof(sockaddr_in));
}

Socket::~Socket() throw()
{
}

bool Socket::create(int domain, int type, int protocol)
{
    LOG_DEBUG("Creating socket with domain: " << domain << ", type: " << type << ", protocol: " << protocol << "\n");

    if(valid()) {
        shutdown();
        close();
    }
    _domain = domain;

    _sockfd = ::socket(_domain, type, protocol);
    return valid();
}

bool Socket::shutdown(int how)
{
    return do_shutdown(how) >= 0;
}

bool Socket::close()
{
    bool retval = ::closesocket(_sockfd) != SOCKET_ERROR;
    _sockfd = INVALID_SOCKET;
    return retval;
}

bool Socket::send(const BufferType* const buffer, size_t len, int flags)
{
    size_t sent = 0;
    int rval = 0;
    while(sent < len) {
        rval = do_send(buffer + sent, len - sent, flags);
        if(rval < 0) {
            if(last_socket_error() == SOCKET_WOULDBLOCK) {
                continue;
            }
            return false;
        } else if(rval == 0) {
            return false;
        }
        sent += rval;
    }
    return true;
}

bool Socket::send(const std::string& buffer, int flags)
{
    return send(reinterpret_cast<const BufferType*>(buffer.c_str()), buffer.length(), flags);
}

size_t Socket::recv(Buffer& buffer, int flags)
{
    size_t read = 0;
    int rval = 0;
    do {
        rval = do_recv(&(*(buffer.begin() + read)), buffer.size() - read, flags);
        read += rval;
    } while(rval < 0 && last_socket_error() == SOCKET_WOULDBLOCK && (buffer.size() - read) > 0);

    return rval;
}

#if defined WIN32
bool Socket::setsockopt(int optname, const char* optval, socklen_t optlen, int level)
#else
bool Socket::setsockopt(int optname, const void* optval, socklen_t optlen, int level)
#endif
{
    return ::setsockopt(_sockfd, level, optname, optval, optlen) != SOCKET_ERROR;
}

#if defined WIN32
bool Socket::getsockopt(int optname, char* optval, socklen_t* optlen, int level)
#else
bool Socket::getsockopt(int optname, void* optval, socklen_t* optlen, int level)
#endif
{
    return ::getsockopt(_sockfd, level, optname, optval, optlen) != SOCKET_ERROR;
}

bool Socket::set_asynchronous()
{
#if defined WIN32
    u_long mode = 0x01;
    return ioctlsocket(_sockfd, FIONBIO, &mode) != SOCKET_ERROR;
#else
    int flags = fcntl(_sockfd, F_GETFL, 0);
    return fcntl(_sockfd, F_SETFL, O_NONBLOCK | flags) >= 0;
#endif
}

bool Socket::get_asynchronous()
{
#if defined WIN32
    return false;
#else
    int flags = fcntl(_sockfd, F_GETFL, 0);
    return flags & O_NONBLOCK;
#endif
}

bool Socket::set_synchronous()
{
#if defined WIN32
    bool retval = WSAAsyncSelect(_sockfd, nullptr, 0, 0) != SOCKET_ERROR;
    u_long mode = 0x00;
    retval = (ioctlsocket(_sockfd, FIONBIO, &mode) != SOCKET_ERROR) && retval;
    return retval;
#else
    int flags = fcntl(_sockfd, F_GETFL, 0);
    return fcntl(_sockfd, F_SETFL, ~O_NONBLOCK & flags) >= 0;
#endif
}

bool Socket::get_synchronous()
{
    return !get_asynchronous();
}

bool Socket::set_keepalive()
{
#if defined WIN32
    char ival = 0x01;
#else
    int ival = 0x01;
#endif
    return setsockopt(SO_KEEPALIVE, &ival, sizeof(ival));
}

#if defined WIN32
bool Socket::async_select(HWND hWnd, UINT uMsg, long lEvent)
{
    return WSAAsyncSelect(_sockfd, hWnd, uMsg, lEvent) != SOCKET_ERROR;
}

bool Socket::event_select(WSAEVENT hEventObject, long lNetworkEvents)
{
    return WSAEventSelect(_sockfd, hEventObject, lNetworkEvents) != SOCKET_ERROR;
}
#endif

Socket& Socket::operator=(const Socket& rhs)
{
    _sockfd = rhs.socket();
    _domain = rhs.domain();
    std::memmove(&_addr, &rhs._addr, sizeof(sockaddr_in));

    return *this;
}

bool Socket::operator==(const Socket& rhs)
{
    return _sockfd == rhs._sockfd;
}

bool Socket::operator!=(const Socket& rhs)
{
    return _sockfd != rhs._sockfd;
}

bool Socket::operator==(const SOCKET& rhs)
{
    return _sockfd == rhs;
}

bool Socket::operator!=(const SOCKET& rhs)
{
    return _sockfd != rhs;
}

bool operator==(const SOCKET& lhs, const Socket& rhs)
{
    return lhs == rhs._sockfd;
}

bool operator!=(const SOCKET& lhs, const Socket& rhs)
{
    return lhs != rhs._sockfd;
}

size_t Socket::do_send(const BufferType* buffer, size_t len, int flags)
{
    return ::send(_sockfd, buffer, len, flags);
}

size_t Socket::do_recv(BufferType* buffer, size_t len, int flags)
{
    return ::recv(_sockfd, buffer, len, flags);
}

int Socket::do_shutdown(int how)
{
    return ::shutdown(_sockfd, how);
}

void Socket::create_addr(unsigned short port) throw(SocketError)
{
    _port = port;

    ZeroMemory(&_addr, sizeof(_addr));

    _addr.sin_family = domain();
    _addr.sin_addr.s_addr = htonl(INADDR_ANY);
    _addr.sin_port = htons(port);
}

void Socket::create_addr(const std::string& host, unsigned short port) throw(SocketError)
{
    _host = host;
    _port = port;

    ZeroMemory(&_addr, sizeof(_addr));

/* TODO: need an ipv6 version */

    if(!host_to_inaddr(domain(), _host, &_addr.sin_addr, sizeof(_addr.sin_addr))) {
        throw SocketError("Socket::host_to_inaddr() failed: " + last_error(Socket::last_socket_error()));
    }

    _addr.sin_family = domain();
    _addr.sin_port = htons(_port);
}

void ClientSocket::create_client_socket(ClientSocket& socket, int domain, int type, int protocol, const std::string& host, unsigned short port) throw(SocketError)
{
    if(!socket.create(domain, type, protocol)) {
        throw ClientSocketError("socket.create() failed: " + last_error(Socket::last_socket_error()));
    }

    if(!socket.connect(host, port)) {
        throw ClientSocketError("socket.connect() failed: " + last_error(Socket::last_socket_error()));
    }
}

Logger& ClientSocket::logger(Logger::instance("energonsoftware.core.network.ClientSocket"));

ClientSocket::ClientSocket()
    : Socket()
{
}

ClientSocket::ClientSocket(sockaddr_in& addr)
    : Socket()
{
    // TODO: pull the address out of the addr?
    _port = ntohs(addr.sin_port);

    std::memmove(&_addr, &addr, sizeof(sockaddr_in));
}

ClientSocket::ClientSocket(SOCKET sockfd)
    : Socket(sockfd)
{
}

ClientSocket::~ClientSocket() throw()
{
}

bool ClientSocket::connect(const std::string& host, unsigned short port) throw(SocketError)
{
    LOG_INFO("Connecting to " << host << ":" << port << "..." << "\n");

    create_addr(host, port);
    return ::connect(socket(), reinterpret_cast<sockaddr*>(&_addr), sizeof(_addr)) != SOCKET_ERROR;
}

bool ClientSocket::sendto(const BufferType* buffer, size_t len, int flags)
{
    size_t sent = 0;
    int rval = 0;
    while(sent < len) {
        rval = ::sendto(socket(), buffer + sent, len - sent, flags, nullptr/*reinterpret_cast<sockaddr*>(&_addr)*/, 0/*sizeof(_addr)*/);
        if(rval < 0) {
            if(last_socket_error() == SOCKET_WOULDBLOCK) {
                continue;
            }
            return false;
        } else if(rval == 0) {
            return false;
        }
        sent += rval;
    }
    return true;
}

bool ClientSocket::sendto(const std::string& buffer, int flags)
{
    return sendto(reinterpret_cast<const BufferType*>(buffer.c_str()), buffer.length(), flags);
}

void ServerSocket::create_server_socket(ServerSocket& socket, int domain, int type, int protocol, unsigned short port, bool reuse_port) throw(ServerSocketError)
{
    if(!socket.create(domain, type, protocol)) {
        throw ServerSocketError("socket.create() failed: " + last_error(Socket::last_socket_error()));
    }

#if defined WIN32
    char ival = 0x01;
#else
    int ival = 0x01;
#endif
    if(!socket.setsockopt(SO_REUSEADDR, &ival, sizeof(ival))) {
        throw ServerSocketError("socket.setsockopt() failed: " + last_error(Socket::last_socket_error()));
    }

    if(port > 0) {
#if !defined WIN32
        if(reuse_port) {
            LOG_INFO("Trying to reuse port..." << "\n");
            LOG_INFO((socket.setsockopt(SO_REUSEPORT, &ival, sizeof(ival)) ? "success!" : "failed!") << "\n");
        }
#endif

        if(!socket.bind(port)) {
            throw ServerSocketError("socket.bind() failed: " + last_error(Socket::last_socket_error()));
        }
    }

    if(type == SOCK_STREAM && !socket.listen()) {
        throw ServerSocketError("socket.listen() failed: " + last_error(Socket::last_socket_error()));
    }
}

Logger& ServerSocket::logger(Logger::instance("energonsoftware.core.network.ServerSocket"));

ServerSocket::ServerSocket()
    : ClientSocket()
{
}

ServerSocket::ServerSocket(SOCKET sockfd)
    : ClientSocket(sockfd)
{
}

ServerSocket::~ServerSocket() throw()
{
}

bool ServerSocket::bind(unsigned short port) throw(SocketError)
{
    LOG_INFO("Binding to port: " << port << "\n");

    create_addr(port);
    return ::bind(socket(), reinterpret_cast<sockaddr*>(&_addr), sizeof(_addr)) != SOCKET_ERROR;
}

bool ServerSocket::bind(const std::string& address, unsigned short port) throw(SocketError)
{
    LOG_INFO("Binding to " << address << ":" << port << "...\n");

    create_addr(address, port);
    return ::bind(socket(), reinterpret_cast<sockaddr*>(&_addr), sizeof(_addr)) != SOCKET_ERROR;
}

bool ServerSocket::listen(int backlog)
{
    return ::listen(socket(), backlog) != SOCKET_ERROR;
}

ClientSocket ServerSocket::accept()
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    return ClientSocket(::accept(socket(), reinterpret_cast<sockaddr*>(&addr), &len));
}

bool ServerSocket::sendto(const BufferType* buffer, size_t len, const ClientSocket& socket, int flags)
{
    size_t sent = 0;
    int rval = 0;
    while(sent < len) {
        rval = ::sendto(this->socket(), buffer + sent, len - sent, flags,
            reinterpret_cast<const sockaddr*>(&socket.addr()), sizeof(socket.addr()));
        if(rval < 0) {
            if(last_socket_error() == SOCKET_WOULDBLOCK) {
                continue;
            }
            return false;
        } else if(rval == 0) {
            return false;
        }
        sent += rval;
    }
    return true;
}

bool ServerSocket::sendto(const std::string& buffer, const ClientSocket& socket, int flags)
{
    return sendto(reinterpret_cast<const BufferType*>(buffer.c_str()), buffer.length(), socket, flags);
}

std::pair<size_t, std::shared_ptr<ClientSocket> > ServerSocket::recvfrom(Buffer& buffer, int flags)
{
    // NOTE: this doesn't check the buffer size because datagrams don't work like that
    std::pair<size_t, std::shared_ptr<ClientSocket> > rval;
    do {
        rval = do_recvfrom(&(*(buffer.begin())), buffer.size(), flags);
    } while(static_cast<int>(rval.first) < 0 && last_socket_error() == SOCKET_WOULDBLOCK);

    return rval;
}

std::pair<size_t, std::shared_ptr<ClientSocket> > ServerSocket::do_recvfrom(BufferType* buffer, size_t len, int flags)
{
    sockaddr_in addr;
    socklen_t alen = sizeof(addr);

    size_t read = ::recvfrom(socket(), buffer, len, flags, reinterpret_cast<sockaddr*>(&addr), &alen);
    return std::pair<size_t, std::shared_ptr<ClientSocket> >(read, std::shared_ptr<ClientSocket>(new ClientSocket(addr)));
}

}
