#if !defined __HTTPSESSION_H__
#define __HTTPSESSION_H__

#include "TcpSession.h"

namespace energonsoftware {

class ClientSocket;
class TcpServer;

class HttpSession : public TcpSession
{
private:
    static const std::unordered_map<unsigned int, const std::string> RESPONSE_CODES;

private:
    static Logger& logger;

public:
    HttpSession(ClientSocket& socket, TcpServer& server, unsigned long sessionid);
    virtual ~HttpSession() noexcept;

public:
    bool handle_request(const std::string& request);

protected:
    // sends a response to the client and disconnects
    bool send_response(unsigned int code, const std::string& message);

    // sends a 200 OK response to the client
    bool send_ok(const std::string& message) { return send_response(200, message); }

    // sends a 500 error to the client
    bool send_error() { return send_response(500, "<html><body>Internal server error</body></html>"); }

    // sends a 400 error to the client
    bool send_bad_request() { return send_response(400, "<html><body>Bad request</body></html>"); }

    // sends a 404 error to the client
    bool send_not_found() { return send_response(404, "<html><body>Resource not found</body></html>"); }

    bool send_from_file(const boost::filesystem::path& path);

    // sends a file to the client, replacing variables with values from the values dictionary
    bool send_from_file(const boost::filesystem::path& path, const std::unordered_map<std::string, std::string>& values);

protected:
    // override these
    virtual bool on_handle_request(const std::string& command, const std::string& path);

private:
    std::string replace_vars(const std::string& content, const std::unordered_map<std::string, std::string>& values);

private:
    std::string _version;

private:
    HttpSession();
    DISALLOW_COPY_AND_ASSIGN(HttpSession);
};

class HttpSessionFactory : public TcpSessionFactory
{
public:
    HttpSessionFactory();
    virtual ~HttpSessionFactory() noexcept;

private:
    DISALLOW_COPY_AND_ASSIGN(HttpSessionFactory);
};

}

#endif
