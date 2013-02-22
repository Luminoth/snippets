#if !defined __PINGMANAGER_H__
#define __PINGMANAGER_H__

namespace energonsoftware {

class ClientSocket;
class UdpClient;
class UdpServer;

class PingManager
{
private:
    class PingSent
    {
    public:
        // time_sent is in seconds
        PingSent(unsigned long seqid, double time_sent)
            : _seqid(seqid), _time_sent(time_sent), _time_recvd(0.0)
        {
        }

        virtual ~PingSent() throw() {}

    public:
        unsigned long seqid() const { return _seqid; }
        double time_sent() const { return _time_sent; }
        double time_recvd() const { return _time_recvd; }

        // time_recvd is in seconds
        void update_recvd(double time_recvd) { _time_recvd = time_recvd; }

        // returns the latency of the ping (999 if we haven't heard from the server) in ms
        double latency() const { return _time_recvd == 0.0 ? 999.0 : ((_time_recvd - _time_sent) * 1000.0); }

    private:
        unsigned int _seqid;
        double _time_sent;
        double _time_recvd;
    };

private:
    static const size_t MAX_QUEUE;
    static const double PING_TIME;

public:
    explicit PingManager(UdpClient& client);
    PingManager(UdpServer& server, const std::string& sessionid);
    virtual ~PingManager() throw();

public:
    bool client_send_ping(const std::string& packer_type, const std::string& sessionid);
    bool server_send_ping(const std::string& packer_type, std::shared_ptr<ClientSocket> socket);
    void ping_recvd(unsigned long seqid);
    bool should_ping() const;
    bool ping_sent() const;

    // cleans all pings we've gotten so far
    void clear();

    // returns the average latency of (999 if we haven't heard from the server) in ms
    double latency() const;

    // returns these times in seconds
    double last_ping_recvd() const;
    double last_ping_sent() const;

private:
    void push_ping(unsigned long seqid, double time_sent);

private:
    std::deque<PingSent> _queue;

    // client vars
    UdpClient* _client;

    // server vars
    UdpServer* _server;
    std::string _server_sessionid;

private:
    PingManager();
    DISALLOW_COPY_AND_ASSIGN(PingManager);
};

}

#endif
