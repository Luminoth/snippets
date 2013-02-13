#include "src/pch.h"
#include "TcpServer.h"
#include "network_util.h"

namespace energonsoftware {

Logger& TcpServer::logger(Logger::instance("energonsoftware.core.network.TcpServer"));

TcpServer::TcpServer(TcpSessionFactory* session_factory)
    : BufferedSender(), _port(0), _last_session_id(0), _running(false),
        _session_factory(session_factory), _socket(), _sessions()
{
    // TODO: if(!_session_factory) throw an exception
}

TcpServer::~TcpServer() throw()
{
    erase_sessions();
}

size_t TcpServer::session_count() const
{
    std::list<std::shared_ptr<TcpSession> > sessions;
    connected_sessions(sessions);
    return sessions.size();
}

bool TcpServer::restart(unsigned short port)
{
    _port = port;

    quit();

    erase_sessions();
    _last_session_id = 0;

    if(!create_socket()) {
        return false;
    }

    _running = on_restart();
    return _running;
}

void TcpServer::run()
{
    // look for new connections
    if(running() && poll_socket_read(_socket.socket())) {
        accept();
    }

    // run all of the sessions
    for(std::shared_ptr<TcpSession>& session : _sessions) {
        if(running() && session && session->connected()) {
            session->run();
        }
    }

    if(running()) {
        on_run();
    }

    clean_sessions();
}

struct sessions_equal : public std::binary_function<std::shared_ptr<TcpSession>, unsigned long, bool>
{
    bool operator()(const std::shared_ptr<TcpSession>& lhs, unsigned long sessionid) const
    { return lhs->sessionid() == sessionid; }
};

void TcpServer::disconnect(unsigned long sessionid, const Socket::BufferType* packet, size_t len)
{
    std::list<std::shared_ptr<TcpSession> >::iterator it =
        std::find_if(_sessions.begin(), _sessions.end(), std::bind2nd(sessions_equal(), sessionid));
    if(it == _sessions.end()) {
        LOG_WARNING("Session " << sessionid << " not found for disconnect!\n");
        return;
    }

    LOG_INFO("Disconnecting session " << sessionid << "...\n");
    (*it)->disconnect(packet, len);

    if(std::find_if(it, _sessions.end(), std::bind2nd(sessions_equal(), sessionid)) != _sessions.end()) {
        LOG_WARNING("Multiple sessions found with sessionid " << sessionid << "\n");
    }
}

void TcpServer::disconnect(unsigned long sessionid, const std::string& packet)
{
    disconnect(sessionid, reinterpret_cast<const Socket::BufferType*>(packet.c_str()), packet.length());
}

void TcpServer::disconnect(const Socket::BufferType* packet, size_t len)
{
    LOG_INFO("Disconnecting " << session_count() << " session(s)...\n");
    for(std::shared_ptr<TcpSession>& session : _sessions) {
        if(session) {
            session->disconnect(packet, len);
        }
    }
    clean_sessions();
}

void TcpServer::disconnect(const std::string& packet)
{
    disconnect(reinterpret_cast<const Socket::BufferType*>(packet.c_str()), packet.length());
}

void TcpServer::quit()
{
    on_quit();
    if(running()) {
        LOG_INFO("Quitting...\n");
        disconnect();
        _socket.close();
    }
    _running = false;
}

void TcpServer::connected_sessions(std::list<std::shared_ptr<TcpSession> >& sessions) const
{
    sessions.clear();
    for(std::shared_ptr<TcpSession> session : _sessions) {
        if(session && session->connected()) {
            sessions.push_back(session);
        }
    }
}

bool TcpServer::create_socket()
{
    try {
        ServerSocket::create_server_socket(_socket, AF_INET, SOCK_STREAM, IPPROTO_TCP, _port);
    } catch(const SocketError& e) {
        LOG_ERROR("Could not create socket: " << e.what() << "\n");
        return false;
    }

    _socket.set_keepalive();
    return true;
}

void TcpServer::accept()
{
    ClientSocket s(_socket.accept());
    s.set_keepalive();

    unsigned long sessionid = ++_last_session_id;
    LOG_INFO("Accepting a new connection: " << sessionid << "\n");

    TcpSession* session = _session_factory->new_session(s, *this, sessionid);
    if(nullptr == session) {
        LOG_WARNING("Could not create new session!\n");
        return;
    }

    on_accept(*session);
    _sessions.push_back(std::shared_ptr<TcpSession>(session));
}

void TcpServer::clean_sessions()
{
    size_t blen = _sessions.size();
    _sessions.remove_if([](const std::shared_ptr<TcpSession>& session)
        { return !session->connected(); });
    size_t alen = _sessions.size();

    if(alen != blen) {
        LOG_INFO("Removed " << blen - alen << " invalid session(s)...\n");
    }
}

void TcpServer::erase_sessions()
{
    for(std::shared_ptr<TcpSession>& session : _sessions) {
        if(session) {
            session->disconnect();
            session.reset();
        }
    }
    _sessions.clear();
}

}
