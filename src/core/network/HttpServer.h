#if !defined __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include "TcpServer.h"

namespace energonsoftware {

class HttpSessionFactory;

class HttpServer : public TcpServer
{
private:
    static Logger& logger;

public:
    explicit HttpServer(HttpSessionFactory* session_factory);
    virtual ~HttpServer() throw();

private:
    virtual void on_packet(TcpSession& session);

private:
    HttpServer();
    DISALLOW_COPY_AND_ASSIGN(HttpServer);
};

}

#endif