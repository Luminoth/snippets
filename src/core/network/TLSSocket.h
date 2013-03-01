#if !defined __TLSSOCKET_H__
#define __TLSSOCKET_H__

#if defined WITH_TLS
#include <gnutls/gnutls.h>
#include "Socket.h"

namespace energonsoftware {

class TLSSocket : public ClientSocket
{
private:
    static Logger& logger;

public:
    TLSSocket();
    //explicit TLSSocket(SOCKET sockfd);
    explicit TLSSocket(const ClientSocket& socket);
    virtual ~TLSSocket() noexcept;

public:
    bool encrypted() const { return _encrypted; }

    bool handshake_srp_client(const std::string& username, const std::string& password);
    bool handshake_srp_server(const boost::filesystem::path& password_file, const boost::filesystem::path& password_conf_file);

private:
    virtual size_t do_send(const Socket::BufferType* buffer, size_t len, int flags) override;
    virtual size_t do_recv(Socket::BufferType* buffer, size_t len, int flags) override;
    virtual int do_shutdown(int how) override;

private:
    void init_tls(); // throw(SocketError);

private:
    bool _encrypted;
    boost::mutex _tls_lock;
    gnutls_session_t _tls_session;

private:
    DISALLOW_COPY_AND_ASSIGN(TLSSocket);
};
#endif

}

#endif
