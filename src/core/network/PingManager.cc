#include "src/pch.h"
#include "src/core/util/util.h"
#include "UdpClient.h"
#include "UdpServer.h"
#include "PingManager.h"

namespace energonsoftware {

const size_t PingManager::MAX_QUEUE = 10;
const double PingManager::PING_TIME = 15.0; // seconds

PingManager::PingManager(UdpClient& client)
    : _queue(), _client(&client), _server(nullptr),
        _server_sessionid()
{
}

PingManager::PingManager(UdpServer& server, const std::string& sessionid)
    : _queue(), _client(nullptr), _server(&server),
        _server_sessionid(sessionid)
{
}

PingManager::~PingManager() noexcept
{
}

bool PingManager::client_send_ping(const std::string& packer_type, const std::string& sessionid)
{
    if(nullptr == _client) {
        return false;
    }

    // send the ping
    unsigned long seqid = _client->next_packet_id();
    double time_sent = get_time();

    //_client->buffer(new PingMessage(packer_type, seqid, sessionid));

    push_ping(seqid, time_sent);
    return true;
}

bool PingManager::server_send_ping(const std::string& packer_type, std::shared_ptr<ClientSocket> socket)
{
    if(nullptr == _server) {
        return false;
    }

    // send the ping
    unsigned long seqid = _server->next_packet_id();
    double time_sent = get_time();

    //_server->buffer(new PingMessage(packer_type, seqid, _server_sessionid), socket);

    push_ping(seqid, time_sent);
    return true;
}

void PingManager::push_ping(unsigned long seqid, double time_sent)
{
    // pop from the queue if necessary
    while(_queue.size() >= PingManager::MAX_QUEUE)
        _queue.pop_front();

    // add the ping to the queue
    _queue.push_back(PingSent(seqid, time_sent));
}

void PingManager::ping_recvd(unsigned long seqid)
{
    // find the ping and update it's recv'd time if it's in the queue
    double time_recvd = get_time();
    for(PingSent& p : _queue) {
        if(p.seqid() == seqid) {
            p.update_recvd(time_recvd);
        }
    }
}

bool PingManager::should_ping() const
{
    return get_time() >= (last_ping_sent() + PingManager::PING_TIME);
}

bool PingManager::ping_sent() const
{
    return _queue.size() > 0;
}

void PingManager::clear()
{
    _queue.clear();
}

double PingManager::latency() const
{
    if(ping_sent()) {
        double sum = 0.0;
        for(const PingSent& p : _queue) {
            sum += p.latency();
        }
        sum /= _queue.size();
        return sum;
    }
    return 999.0;
}

template <typename T>
struct time_recvd_less : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    { return x.time_recvd() < y.time_recvd(); }
};

double PingManager::last_ping_recvd() const
{
    if(ping_sent()) {
        std::deque<PingSent>::const_iterator max = std::max_element(_queue.begin(), _queue.end(), time_recvd_less<PingSent>());
        return max->time_recvd();
    }
    return 0.0;
}

template <typename T>
struct time_sent_less : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    { return x.time_sent() < y.time_sent(); }
};

double PingManager::last_ping_sent() const
{
    if(ping_sent()) {
        std::deque<PingSent>::const_iterator max = std::max_element(_queue.begin(), _queue.end(), time_sent_less<PingSent>());
        return max->time_sent();
    }
    return 0.0;
}

}
