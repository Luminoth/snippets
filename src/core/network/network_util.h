#if !defined __NETWORK_UTIL_H__
#define __NETWORK_UTIL_H__

#if defined WIN32
    #include <winsock2.h>

    // libraries
    #pragma comment(lib, "ws2_32.lib")

    // errors
    #define SOCKET_ADDRINUSE        WSAEADDRINUSE
    #define SOCKET_WOULDBLOCK       WSAEWOULDBLOCK
    #define SOCKET_ALREADY          WSAEALREADY
    #define SOCKET_ISCONN           WSAEISCONN
    #define SOCKET_INVAL            WSAEINVAL
    #define SOCKET_INPROGRESS       WSAEINPROGRESS
    #define SOCKET_CONNRESET        WSAECONNRESET
    #define SOCKET_ADDRNOTAVAIL     WSAEADDRNOTAVAIL
    #define SOCKET_AFNOSUPPORT      WSAEAFNOSUPPORT
    #define SOCKET_CONNABORTED      WSAECONNABORTED
    #define SOCKET_CONNREFUSED      WSAECONNREFUSED
    #define SOCKET_DESTADDRREQ      WSAEDESTADDRREQ
    #define SOCKET_FAULT            WSAEFAULT
    #define SOCKET_HOSTDOWN         WSAEHOSTDOWN
    #define SOCKET_HOSTUNREACH      WSAEHOSTUNREACH
    #define SOCKET_MFILE            WSAEMFILE
    #define SOCKET_MSGSIZE          WSAEMSGSIZE
    #define SOCKET_NETDOWN          WSAENETDOWN
    #define SOCKET_NETRESET         WSAENETRESET
    #define SOCKET_NETUNREACH       WSAENETUNREACH
    #define SOCKET_NOBUFS           WSAENOBUFS
    #define SOCKET_NOTCONN          WSAENOTCONN
    #define SOCKET_NOTSOCK          WSAENOTSOCK
    #define SOCKET_PFNOSUPPORT      WSAEPFNOSUPPORT
    #define SOCKET_SHUTDOWN         WSAESHUTDOWN
    #define SOCKET_TIMEDOUT         WSAETIMEDOUT
    #define SOCKET_HOST_NOT_FOUND   WSAHOST_NOT_FOUND
    #define SOCKET_NO_RECOVERY      WSANO_RECOVERY
    #define SOCKET_TRY_AGAIN        WSATRY_AGAIN

    // winsock specific errors
    #define SOCKET_PROCLIM          WSAEPROCLIM
    #define SOCKET_NOTINITIALISED   WSANOTINITIALISED
    #define SOCKET_SYSNOTREADY      WSASYSNOTREADY
    #define SOCKET_VERNOTSUPPORTED  WSAVERNOTSUPPORTED

    // constants
    #define SHUT_RDWR 2 // SD_BOTH

    // types
    typedef int socklen_t;

    // this crap is in winsock.h but not winsock2.h? wtf?
    struct ip_mreq {
        struct in_addr  imr_multiaddr;  /* IP multicast address of group */
        struct in_addr  imr_interface;  /* local IP address of interface */
    };

    #define IP_MULTICAST_IF     2           /* set/get IP multicast interface   */
    #define IP_MULTICAST_TTL    3           /* set/get IP multicast timetolive  */
    #define IP_MULTICAST_LOOP   4           /* set/get IP multicast loopback    */
    #define IP_ADD_MEMBERSHIP   5           /* add  an IP group membership      */
#else
    #include <sys/socket.h>
    #include <netinet/in.h>

    // errors
    #define SOCKET_WOULDBLOCK       EAGAIN
    #define SOCKET_ALREADY          EALREADY
    #define SOCKET_ISCONN           EISCONN
    #define SOCKET_INVAL            EINVAL
    #define SOCKET_INPROGRESS       EINPROGRESS
    #define SOCKET_CONNRESET        ECONNRESET
    #define SOCKET_ADDRINUSE        EADDRINUSE
    #define SOCKET_ADDRNOTAVAIL E   ADDRNOTAVAIL
    #define SOCKET_AFNOSUPPORT      EAFNOSUPPORT
    #define SOCKET_CONNABORTED      ECONNABORTED
    #define SOCKET_CONNREFUSED      ECONNREFUSED
    #define SOCKET_DESTADDRREQ      EDESTADDRREQ
    #define SOCKET_FAULT            EFAULT
    #define SOCKET_HOSTDOWN         EHOSTDOWN
    #define SOCKET_HOSTUNREACH      EHOSTUNREACH
    #define SOCKET_MFILE            EMFILE
    #define SOCKET_MSGSIZE          EMSGSIZE
    #define SOCKET_NETDOWN          ENETDOWN
    #define SOCKET_NETRESET         ENETRESET
    #define SOCKET_NETUNREACH       ENETUNREACH
    #define SOCKET_NOBUFS           ENOBUFS
    #define SOCKET_NOTCONN          ENOTCONN
    #define SOCKET_NOTSOCK          ENOTSOCK
    #define SOCKET_PFNOSUPPORT      EPFNOSUPPORT
    #define SOCKET_SHUTDOWN         ESHUTDOWN
    #define SOCKET_TIMEDOUT         ETIMEDOUT
    #define SOCKET_HOST_NOT_FOUND   HOST_NOT_FOUND
    #define SOCKET_NO_RECOVERY      NO_RECOVERY
    #define SOCKET_TRY_AGAIN        TRY_AGAIN

    // constants
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1

    #if !defined SO_REUSEPORT
        #define SO_REUSEPORT 15
    #endif

    // structures
    typedef struct WSAData
    {
        int dummy;
    } WSADATA, *LPWSADATA;

    // functions
    #define closesocket close
    int WSAStartup(int wVersionRequested, LPWSADATA lpWSAData);
    int WSACleanup();

    // types
    typedef int SOCKET;
#endif

namespace energonsoftware {

void tls_log(int level, const char* message);
std::string encode_packet(const char* input, size_t ilen);
std::string decode_packet(const char* input, size_t ilen);
bool poll_socket_read(SOCKET s);

}

#endif
