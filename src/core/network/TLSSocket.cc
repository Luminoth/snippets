#include "src/pch.h"
#if defined WITH_TLS
//#include <gnutls/extra.h>
#include "TLSSocket.h"

static const int KX_PRIORITIES[] = { GNUTLS_KX_SRP, 0 };

namespace energonsoftware {

Logger& TLSSocket::logger(Logger::instance("energonsoftware.core.network.TLSSocket"));

TLSSocket::TLSSocket()
    : ClientSocket(), _encrypted(false), _tls_lock(), _tls_session()
{
    init_tls();
}

TLSSocket::TLSSocket(const ClientSocket& socket)
    : ClientSocket(socket), _encrypted(false), _tls_lock(), _tls_session()
{
    init_tls();
}

TLSSocket::~TLSSocket() noexcept
{
    std::lock_guard<std::mutex> guard(_tls_lock);
    gnutls_global_deinit();
}

bool TLSSocket::handshake_srp_client(const std::string& username, const std::string& password)
{
    std::lock_guard<std::mutex> guard(_tls_lock);

    // set the credentials
    gnutls_srp_client_credentials_t credentials;
    gnutls_srp_allocate_client_credentials(&credentials);
    gnutls_srp_set_client_credentials(credentials, username.c_str(), password.c_str());

    // init the session
    gnutls_init(&_tls_session, GNUTLS_CLIENT);

    // set the priorities
    gnutls_set_default_priority(_tls_session);
    gnutls_kx_set_priority(_tls_session, KX_PRIORITIES);

    // set the credentials
    gnutls_credentials_set(_tls_session, GNUTLS_CRD_SRP, credentials);

    gnutls_transport_set_ptr(_tls_session, reinterpret_cast<gnutls_transport_ptr_t>(socket()));

    // handshake the server
    int ret = gnutls_handshake(_tls_session);
    while(ret < 0) {
        if(gnutls_error_is_fatal(ret)) {
            LOG_ERROR("TLS handshake failed: " << gnutls_strerror(ret) << "\n");

            gnutls_deinit(_tls_session);
            gnutls_srp_free_client_credentials(credentials);
            return false;
        }
        ret = gnutls_handshake(_tls_session);
    }
    gnutls_srp_free_client_credentials(credentials);

    _encrypted = true;
    return true;
}

bool TLSSocket::handshake_srp_server(const boost::filesystem::path& password_file, const boost::filesystem::path& password_conf_file)
{
    std::lock_guard<std::mutex> guard(_tls_lock);

    // set the credentials
    gnutls_srp_server_credentials_t credentials;
    gnutls_srp_allocate_server_credentials(&credentials);
    if(gnutls_srp_set_server_credentials_file(credentials, password_file.string().c_str(), password_conf_file.string().c_str()) != 0) {
        LOG_CRITICAL("Could not load credentials!\n");
        return false;
    }

    // init the session
    gnutls_init(&_tls_session, GNUTLS_SERVER);

    // set the priorities
    gnutls_set_default_priority(_tls_session);
    gnutls_kx_set_priority(_tls_session, KX_PRIORITIES);

    // set the credentials
    gnutls_credentials_set(_tls_session, GNUTLS_CRD_SRP, credentials);

    gnutls_transport_set_ptr(_tls_session, reinterpret_cast<gnutls_transport_ptr_t>(socket()));

    // handshake the client
    int ret = gnutls_handshake(_tls_session);
    while(ret < 0) {
        if(gnutls_error_is_fatal(ret)) {
            LOG_ERROR("TLS handshake failed: " << gnutls_strerror(ret) << "\n");

            gnutls_deinit(_tls_session);
            gnutls_srp_free_server_credentials(credentials);
            return false;
        }
        ret = gnutls_handshake(_tls_session);
    }
    gnutls_srp_free_server_credentials(credentials);

    _encrypted = true;
    return true;
}

size_t TLSSocket::do_send(const Socket::BufferType* buffer, size_t len, int flags)
{
    if(encrypted()) {
        std::lock_guard<std::mutex> guard(_tls_lock);
        do {
            int rval = gnutls_record_send(_tls_session, buffer, len);
            if(rval == GNUTLS_E_INTERRUPTED || rval == GNUTLS_E_AGAIN) {
                continue;
            }
            return rval;
        } while(true);
    }
    return ClientSocket::do_send(buffer, len, flags);
}

size_t TLSSocket::do_recv(Socket::BufferType* buffer, size_t len, int flags)
{
    if(encrypted()) {
        std::lock_guard<std::mutex> guard(_tls_lock);
        do {
            int rval = gnutls_record_recv(_tls_session, buffer, len);
            if(rval == GNUTLS_E_INTERRUPTED || rval == GNUTLS_E_AGAIN) {
                continue;
            }
            return rval;
        } while(true);
    }
    return ClientSocket::do_recv(buffer, len, flags);
}

int TLSSocket::do_shutdown(int how)
{
    if(encrypted()) {
        std::lock_guard<std::mutex> guard(_tls_lock);
        int ret = gnutls_bye(_tls_session, GNUTLS_SHUT_RDWR);   // TODO: don't ignore how
        gnutls_deinit(_tls_session);
        _encrypted = false;
        return ret;
    }
    return ClientSocket::do_shutdown(how);
}

void TLSSocket::init_tls() //throw(SocketError);
{
    std::lock_guard<std::mutex> guard(_tls_lock);
    /*if(!gnutls_check_version(LIBGNUTLS_VERSION)) {
        throw SocketError("Incorrect GnuTLS version!");
    }*/

    gnutls_global_init();
    //gnutls_global_init_extra();

    /*gnutls_global_set_log_function(tls_log);
    gnutls_global_set_log_level(10);*/
}

}
#endif
