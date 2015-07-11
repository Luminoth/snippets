#include "src/pch.h"
#include "src/core/util/util.h"
#include "network_util.h"

static energonsoftware::Logger& logger(energonsoftware::Logger::instance("energonsoftware.core.network.network_util"));

#if !defined WIN32
#include <sys/select.h>
#include <iconv.h>

int WSAStartup(int wVersionRequested, LPWSADATA lpWSAData)
{
    return 0;
}

int WSACleanup()
{
    return 0;
}
#endif

namespace energonsoftware {

void tls_log(int level, const char* message)
{
    LOG_INFO("TLS log (" << level << ")" << ": " << message << "\n");
}

static std::string encode_packet(char* input, size_t& ilen, const char* tocode, const char* fromcode)
{
#if !defined WIN32
    iconv_t cd = iconv_open(tocode, fromcode);
    if(cd == (iconv_t)-1) return "";

    // 4x the data storage... that *should* work for most conversions
    size_t len = ilen << 2;
    size_t olen = len;
    std::unique_ptr<char[]> container(new char[olen]);
    char* output = container.get();
    ZeroMemory(output, olen);

    while(ilen != 0 && olen != 0) {
#if defined __APPLE__ && !defined MAC_OS_X_VERSION_10_6
        if(iconv(cd, const_cast<const char**>(&input), &ilen, &output, &olen) == (size_t)-1) {
#else
        if(iconv(cd, &input, &ilen, &output, &olen) == (size_t)-1) {
#endif
            iconv_close(cd);
            return "";
        }
    }
    len -= olen;

    iconv_close(cd);
    return std::string(container.get(), len);
#endif
    return std::string(input);
}

std::string encode_packet(const char* input, size_t ilen/*, const char* tocode, const char* fromcode*/)
{
    std::unique_ptr<char[]> scratch(new char[ilen+1]);
    strncpy_s(scratch.get(), ilen, input, ilen);
    scratch.get()[ilen] = 0;
    return encode_packet(scratch.get(), ilen, "UTF-8", "ASCII");
}

std::string decode_packet(const char* input, size_t ilen/*, const char* tocode, const char* fromcode*/)
{
    std::unique_ptr<char[]> scratch(new char[ilen+1]);
    strncpy_s(scratch.get(), ilen, input, ilen);
    scratch.get()[ilen] = 0;
    return encode_packet(scratch.get(), ilen, "ASCII", "UTF-8");
}

bool poll_socket_read(SOCKET s)
{
    if(s < 0) {
        return false;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(s, &fds);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;

    if(select(s + 1, &fds, nullptr, nullptr, &timeout) == SOCKET_ERROR) {
        LOG_ERROR("Could not poll socket: " << last_std_error(errno) << "\n");
        return false;
    }

    return FD_ISSET(s, &fds) != 0;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class NetworkUtilTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(NetworkUtilTest);
    CPPUNIT_TEST_SUITE_END();

public:
    NetworkUtilTest() : CppUnit::TestFixture() {}
    virtual ~NetworkUtilTest() noexcept {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetworkUtilTest);

#endif
