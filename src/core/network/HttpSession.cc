#include "src/pch.h"
#include <boost/regex.hpp>
#include "src/core/util/fs_util.h"
#include "HttpSession.h"

namespace energonsoftware {

static const std::pair<unsigned int, std::string> STATIC_CODES[] =
{
    std::pair<unsigned int, const std::string>(100, "Continue"),
    std::pair<unsigned int, const std::string>(101, "Switching Protocols"),

    std::pair<unsigned int, const std::string>(200, "OK"),
    std::pair<unsigned int, const std::string>(201, "Created"),
    std::pair<unsigned int, const std::string>(202, "Accepted"),
    std::pair<unsigned int, const std::string>(203, "Non-Authoritative Information"),
    std::pair<unsigned int, const std::string>(204, "No Content"),
    std::pair<unsigned int, const std::string>(205, "Reset Content"),
    std::pair<unsigned int, const std::string>(206, "Partial Content"),

    std::pair<unsigned int, const std::string>(300, "Multiple Choices"),
    std::pair<unsigned int, const std::string>(301, "Moved Permanently"),
    std::pair<unsigned int, const std::string>(302, "Found"),
    std::pair<unsigned int, const std::string>(303, "See Other"),
    std::pair<unsigned int, const std::string>(304, "Not Modified"),
    std::pair<unsigned int, const std::string>(305, "Use Proxy"),
    std::pair<unsigned int, const std::string>(306, "(Unused)"),
    std::pair<unsigned int, const std::string>(307, "Temporary Redirect"),

    std::pair<unsigned int, const std::string>(400, "Bad Request"),
    std::pair<unsigned int, const std::string>(401, "Unauthorized"),
    std::pair<unsigned int, const std::string>(402, "Payment Required"),
    std::pair<unsigned int, const std::string>(403, "Forbidden"),
    std::pair<unsigned int, const std::string>(404, "Not Found"),
    std::pair<unsigned int, const std::string>(405, "Method Not Allowed"),
    std::pair<unsigned int, const std::string>(406, "Not Acceptable"),
    std::pair<unsigned int, const std::string>(407, "Proxy Authentication Required"),
    std::pair<unsigned int, const std::string>(408, "Request Timeout"),
    std::pair<unsigned int, const std::string>(409, "Conflict"),
    std::pair<unsigned int, const std::string>(410, "Gone"),
    std::pair<unsigned int, const std::string>(411, "Length Required"),
    std::pair<unsigned int, const std::string>(412, "Precondition Failed"),
    std::pair<unsigned int, const std::string>(413, "Request Entity Too Large"),
    std::pair<unsigned int, const std::string>(414, "Request-URI Too Long"),
    std::pair<unsigned int, const std::string>(415, "Unsupported Media Type"),
    std::pair<unsigned int, const std::string>(416, "Requested Range Not Satisfiable"),
    std::pair<unsigned int, const std::string>(417, "Expectation Failed"),

    std::pair<unsigned int, const std::string>(500, "Internal Server Error"),
    std::pair<unsigned int, const std::string>(501, "Not Implemented"),
    std::pair<unsigned int, const std::string>(502, "Bad Gateway"),
    std::pair<unsigned int, const std::string>(503, "Service Unavailable"),
    std::pair<unsigned int, const std::string>(504, "Gateway Timeout"),
    std::pair<unsigned int, const std::string>(505, "HTTP Version Not Supported"),
};

const std::unordered_map<unsigned int, const std::string> HttpSession::RESPONSE_CODES(
    STATIC_CODES, STATIC_CODES + (sizeof(STATIC_CODES) / sizeof(STATIC_CODES[0])));

Logger& HttpSession::logger(Logger::instance("energonsoftware.core.network.HttpSession"));

HttpSession::HttpSession(ClientSocket& socket, TcpServer& server, unsigned long sessionid)
    : TcpSession(socket, server, sessionid), _version("HTTP/0.9")
{
}

HttpSession::~HttpSession() noexcept
{
}

bool HttpSession::handle_request(const std::string& request)
{
    std::vector<std::string> tokens;
    boost::split(tokens, request, boost::is_any_of("\r\n"), boost::token_compress_on);
    if(tokens.size() == 0) {
        send_bad_request();
        return false;
    }

    std::vector<std::string> parts;
    boost::split(parts, tokens[0], boost::is_space());
    if(parts.size() < 2) {
        send_bad_request();
        return false;
    }

    std::string command(parts[0]);
    std::string path(parts[1]);
    /*if(parts.length() >= 3)
        _version = parts[2];*/

    return on_handle_request(command, path);
}

bool HttpSession::send_response(unsigned int code, const std::string& message)
{
    // lookup the code
    std::string codeval;
    try {
        codeval = RESPONSE_CODES.at(code);
    } catch(const std::out_of_range&) {
        LOG_ERROR("Could not locate code " << code << "\n");
        codeval = "Internal Server Error";
        code = 500;
    }

    std::stringstream scratch;
    scratch << _version << " " << code << " " << codeval << "\r\n"
        << "Content-type: text/html\r\n"
        << "Content-Length: " << message.length() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n" << message << "\r\n";

    std::string response(scratch.str());
    //LOG_DEBUG("Sending response: " << response << "\n");
    send(encode_packet(response.c_str(), response.length()));

    disconnect();
    return true;
}

bool HttpSession::send_from_file(const boost::filesystem::path& filename)
{
    std::string content;
    if(!file_to_string(filename, content))
        return send_error();
    return send_ok(content);
}

bool HttpSession::send_from_file(const boost::filesystem::path& filename, const std::unordered_map<std::string, std::string>& values)
{
    std::string content;
    if(!file_to_string(filename, content))
        return send_error();
    return send_ok(replace_vars(content, values));
}

bool HttpSession::on_handle_request(const std::string& command, const std::string& path)
{
    if(strcasecmp(command.c_str(), "GET"))
        return false;

    if(path == "/") {
        return send_ok("<html><body>This is the default HttpHandler page</body></html>");
    }
    return send_not_found();
}

struct is_trim : public std::unary_function<char, bool>
{
public:
    bool operator()(char ch) const
    { return ch == '{' || ch == '}' || std::isspace(ch); }
};

std::string HttpSession::replace_vars(const std::string& content, const std::unordered_map<std::string, std::string>& values)
{
    if(values.empty()) {
        return content;
    }

    std::string scratch(content);
    std::string::const_iterator start = scratch.begin(), end = scratch.end();

    boost::regex expression("\\{\\{ *(\\w|\\.)* *\\}\\}", boost::regex::perl);
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    while(regex_search(start, end, what, expression, flags)) {
        std::string lookup(boost::trim_copy_if(what[0].str(), is_trim()));
        if(values.find(lookup) != values.end()) {
            boost::replace_all(scratch, what[0].str(), values.at(lookup));
            start = scratch.begin();    // TODO: we don't really need to start over... do we?
        } else {
            LOG_WARNING("No such value: " << lookup << "\n");
            start = what[0].second;
        }
    }

    return scratch;
}

HttpSessionFactory::HttpSessionFactory()
    : TcpSessionFactory()
{
}

HttpSessionFactory::~HttpSessionFactory() noexcept
{
}

}
