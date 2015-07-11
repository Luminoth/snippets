#include "src/pch.h"
#if !defined WIN32
#include <arpa/inet.h>
#endif
#include "src/core/util/util.h"
#include "Multicaster.h"

namespace energonsoftware {

Logger& Multicaster::logger(Logger::instance("energonsoftware.core.network.Multicaster"));

Multicaster::Multicaster(BufferedSender& server, const std::string& address)
    : Broadcaster(server, address)
{
}

Multicaster::~Multicaster() noexcept
{
}

bool Multicaster::enable_broadcast()
{
    LOG_INFO("Enabling multicast...\n");

    struct ip_mreq mreq;
    ZeroMemory(&mreq, sizeof(mreq));

    inet_pton(AF_INET, _address.c_str(), &(mreq.imr_multiaddr.s_addr));
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if(!_socket->setsockopt(IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq), IPPROTO_IP)) {
        LOG_CRITICAL("Could not join the multicast group: " << last_error(Socket::last_socket_error()) << "\n");
        return false;
    }

#if defined WIN32
    char ival = 20;
#else
    int ival = 20;
#endif
    if(!_socket->setsockopt(IP_MULTICAST_TTL, &ival, sizeof(ival), IPPROTO_IP)) {
        LOG_CRITICAL("Could not set the multicast TTL: " << last_error(Socket::last_socket_error()) << "\n");
        return false;
    }

    ival = 0x01;
    if(!_socket->setsockopt(IP_MULTICAST_LOOP, &ival, sizeof(ival), IPPROTO_IP)) {
        LOG_CRITICAL("Could not enable multicast loopback: " << last_error(Socket::last_socket_error()) << "\n");
        return false;
    }

    return true;
}

}
