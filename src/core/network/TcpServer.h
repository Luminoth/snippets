#if !defined __TCPSERVER_H__
#define __TCPSERVER_H__

#include "BufferedSender.h"
#include "TcpSession.h"
#include "Socket.h"

namespace energonsoftware {

class TcpServer : public BufferedSender
{
private:
    friend class TcpSession;

private:
    static Logger& logger;

public:
    // session_factory must have been created with new and must not be null
    explicit TcpServer(TcpSessionFactory* session_factory);
    virtual ~TcpServer() noexcept;

public:
    unsigned short port() const { return _port; }
    bool running() const { return _running; }

    // returns the number of connected sessions
    size_t session_count() const;

    // resets the state of the server
    bool restart(unsigned short port);

    // runs the server, call each 'frame'
    void run();

    // disconnects a session
    void disconnect(unsigned long sessionid, const Socket::BufferType* packet=nullptr, size_t len=0);
    void disconnect(unsigned long sessionid, const std::string& packet);

    // disconnects all sessions
    void disconnect(const Socket::BufferType* packet=nullptr, size_t len=0);
    void disconnect(const std::string& packet);

    // disconnects all sessions and closes the socket
    void quit();

protected:
    void connected_sessions(std::list<std::shared_ptr<TcpSession> >& sessions) const;

protected:
    // override these
    virtual bool on_restart() { return true; }
    virtual void on_run() {}
    virtual void on_quit() {}
    virtual void on_accept(TcpSession& session) {}
    virtual void on_packet(TcpSession& session) {}

private:
    bool create_socket();
    void accept();
    void clean_sessions();
    void erase_sessions();

private:
    unsigned short _port;
    unsigned int _last_session_id;
    bool _running;
    std::unique_ptr<TcpSessionFactory> _session_factory;
    ServerSocket _socket;
    std::list<std::shared_ptr<TcpSession> > _sessions;

private:
    TcpServer() = delete;
    DISALLOW_COPY_AND_ASSIGN(TcpServer);
};

}

#endif
