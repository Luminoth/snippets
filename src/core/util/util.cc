#include "src/pch.h"
#include <iostream>
#include <iomanip>
#include <errno.h>
#include <fcntl.h>
#if defined USE_OPENSSL
    #include <openssl/bio.h>
    #include <openssl/buffer.h>
    #include <openssl/evp.h>
#endif
#include "src/core/math/math_util.h"
#include "fs_util.h"
#include "util.h"

static energonsoftware::Logger& logger(energonsoftware::Logger::instance("energonsoftware.core.util.util"));

namespace energonsoftware {

bool is_little_endian()
{
    unsigned int n = 1;
    unsigned char* b = reinterpret_cast<unsigned char*>(&n);
    return (*b) == 1;
}

std::string last_std_error() noexcept
{
    return last_std_error(errno);
}

std::string last_std_error(int error) noexcept
{
#if defined WIN32
    char buf[1024];
    strerror_s(buf, 1024, error);
    return buf;
#else
    return std::strerror(error);
#endif
}

std::string last_error() noexcept
{
#if defined WIN32
    return last_error(GetLastError());
#else
    return last_std_error();
#endif
}

std::string last_error(int error) noexcept
{
#if defined WIN32
    wchar_t buffer[512] = { 0 };
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error, 0, buffer, 512, nullptr);

    size_t ret = 0;
    char ba[1024] = { 0 };
    wcstombs_s(&ret, ba, 1024, buffer, 1024);
    return std::string(ba);
#else
    return last_std_error(error);
#endif
}

boost::gregorian::date epoch_date()
{
    return boost::gregorian::date(1970, 1, 1);
}

boost::posix_time::ptime epoch_time()
{
    return boost::posix_time::ptime(epoch_date());
}

double get_time()
{
    return get_time(boost::posix_time::microsec_clock::universal_time());
}

double get_time(const boost::posix_time::ptime& time)
{
    return (time - epoch_time()).total_microseconds() / 1000000.0;
}

boost::posix_time::ptime from_time(uint64_t seconds)
{
    return boost::posix_time::ptime(epoch_date(), boost::posix_time::seconds(seconds));
}

// TODO: this is a bit arbitrary and unnecessary
#define BASE64_MAX_BUFFER 10240

char* base64_encode(const unsigned char* input, size_t len)
{
#if defined USE_OPENSSL
    // create the encoder
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // create the bit bucket
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    // write the input
    BIO_write(bio, input, len);
    BIO_flush(bio);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(bio, &bptr);

    // read the result
    char* ret = new char[bptr->length+1];
    memcpy(ret, bptr->data, bptr->length);
    ret[bptr->length] = 0;

    BIO_free_all(bio);
    return ret;
#else
    // TODO: write non-OpenSSL version
    return nullptr;
#endif
}

unsigned char* base64_decode(const char* input, size_t& len)
{
#if defined USE_OPENSSL
    // create the decoder
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // create the bit bucket
    BIO* bio = BIO_new_mem_buf(const_cast<char*>(input), -1);
    bio = BIO_push(b64, bio);

    // read the result
    unsigned char scratch[BASE64_MAX_BUFFER];
    len = BIO_read(bio, scratch, BASE64_MAX_BUFFER);
    if(len <= 0) return nullptr;

    unsigned char* ret = new unsigned char[len];
    memcpy(ret, scratch, len);

    BIO_free_all(bio);
    return ret;
#else
    // TODO: write non-OpenSSL version
    return nullptr;
#endif
}

void md5sum(const unsigned char* input, size_t len, unsigned char* output)
{
#if defined USE_OPENSSL
    MD5(input, len, output);
#else
    // TODO: write non-OpenSSL version
#endif
}

void md5sum_hex(const unsigned char* input, size_t len, char* output)
{
    // get the md5sum
    unsigned char scratch[MD5_DIGEST_LENGTH];
    md5sum(input, len, scratch);

    // write it out as hex
    std::stringstream str;
    for(int i=0; i<MD5_DIGEST_LENGTH; ++i) {
        str << std::hex << std::setfill('0') << std::setw(2) << (int)scratch[i];
    }

    // copy to the output
    std::string encoded(str.str());
    std::copy(encoded.begin(), encoded.end(), output);
    output[MIN(static_cast<int>(encoded.length()), MD5_DIGEST_LENGTH << 1)] = 0;
}

bool md5sum_file(const boost::filesystem::path& path, char* output)
{
    std::string data;
    if(!file_to_string(path, data)) {
        return false;
    }

    md5sum_hex(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), output);
    return true;
}

std::string md5_digest_password(const std::string& username, const std::string& realm, const std::string& password)
{
    const std::string key(username + ":" + realm + ":" + password);

    char digest[(MD5_DIGEST_LENGTH << 1) + 1];
    md5sum_hex(reinterpret_cast<const unsigned char*>(key.c_str()), key.length(), digest);
    return digest;
}

#if defined WITH_CRYPTO
bool blowfish_encrypt(const unsigned char* key, const unsigned char* input, size_t ilen, unsigned char* output, size_t& olen)
{
#if defined USE_OPENSSL
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_bf_ecb(), nullptr, key, nullptr);

    int len;
    if(!EVP_EncryptUpdate(&ctx, output, &len, input, ilen)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    olen += len;

    if(!EVP_EncryptFinal_ex(&ctx, output + len, &len)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    olen += len;

    EVP_CIPHER_CTX_cleanup(&ctx);
    return true;
#else
    // TODO: write non-OpenSSL version
    return false;
#endif
}

bool blowfish_decrypt(const unsigned char* key, const unsigned char* input, size_t ilen, unsigned char* output, size_t& olen)
{
#if defined USE_OPENSSL
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_DecryptInit_ex(&ctx, EVP_bf_ecb(), nullptr, key, nullptr);

    int len;
    if(!EVP_DecryptUpdate(&ctx, output, &len, input, ilen)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    olen += len;

    if(!EVP_DecryptFinal_ex(&ctx, output + len, &len)) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        return false;
    }
    olen += len;

    EVP_CIPHER_CTX_cleanup(&ctx);
    return true;
#else
    // TODO: write non-OpenSSL version
    return false;
#endif
}
#endif

std::string bin2hex(const unsigned char* bytes, size_t len, size_t maxlen)
{
    static const size_t cpg = 8;                                // line characters per group
    static const size_t cpr = cpg << 1;                         // line characters per row
    static const size_t columns = ((cpr * 3) + (cpr * 2)) + 4;  // actual characters per row, including spaces and newlines

    len = maxlen > 0 ? MIN(len, maxlen) : len;
    const size_t rows = static_cast<size_t>(std::ceil(static_cast<float>(len) / static_cast<float>(cpr)));
    const size_t size = columns * rows;

    size_t pos=0;
    std::unique_ptr<char[]> ret(new char[size + 1]);
    for(size_t row=0; row<rows; ++row) {
        // hex
        for(size_t col=0; col<cpr; ++col) {
            std::string str = "  ";

            size_t loc = (row * cpr) + col;
            if(loc < len) {
                std::stringstream hexstr;
                hexstr.width(2);
                hexstr.fill('0');
                hexstr << std::hex << static_cast<int>(bytes[loc]) << std::dec;
                str = hexstr.str();
            }

            ret.get()[pos] = str[0];
            ret.get()[pos+1] = str[1];
            ret.get()[pos+2] = ' ';
            pos += 3;

            if((col+1) % cpg == 0) {
                ret.get()[pos] = ' ';
                pos++;
            }
        }

        // ascii
        for(size_t col=0; col<cpr; ++col) {
            char ch = ' ';

            size_t loc = (row * cpr) + col;
            if(loc < len) {
                ch = static_cast<char>(bytes[loc]);
                if(!isgraph(ch))
                    ch = '.';
            }

            ret.get()[pos] = ch;
            ret.get()[pos+1] = ' ';
            pos += 2;

            if((col+1) % cpg == 0) {
                ret.get()[pos] = ' ';
                pos++;
            }
        }

        ret.get()[pos-1] = '\n';
    }

    ret.get()[size] = '\0';
    return std::string("\n") + ret.get();
}

void daemonize(bool changedir)
{
    std::cout << "Daemonizing..." << std::endl;

#if defined WIN32
/* TODO: write meh */
#else
    // return control to the shell and drop process group lead
    pid_t pid = fork();
    if(pid < 0) {
        std::cerr << "Fork error: " << std::strerror(errno) << std::endl;
        exit(1);
    } else if(pid > 0) {
        exit(0);
    }

    // regain process group lead and drop controlling terminal
    if(setsid() < 0) {
        std::cerr << "Could not create new process group: " << std::strerror(errno) << std::endl;
        exit(1);
    }

    // drop process group lead so init will clean us up
    pid = fork();
    if(pid < 0) {
        std::cerr << "Fork error: " << std::strerror(errno) << std::endl;
        exit(1);
    } else if(pid > 0) {
        exit(0);
    }

    // move to / so we don't block a filesystem
    if(changedir) {
        chdir("/");
    }

    // keep control over what we write (optional)
    umask(0);

    // ignore SIGHUP (optional)
    //signal(SIGHUP, SIG_IGN);

    // close all old file descriptors
    long maxfd = sysconf(_SC_OPEN_MAX);
    for(int i=0; i<maxfd; ++i) {
        close(i);
    }

    // TODO: error handle this (and ensure the right fd is set)
    open("/dev/null", O_RDONLY);    // stdin
    open("/dev/null", O_RDWR);      // stdout
    open("/dev/null", O_RDWR);      // stderr
#endif
}

}

#if defined WITH_UNIT_TESTS
#include "src/core/common.h"
#include "src/test/UnitTest.h"

/* from http://www.cpp-programming.net/c-tidbits/gettimeofday-function-for-windows/ */
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

class UtilTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(UtilTest);
        CPPUNIT_TEST(test_get_time);
        CPPUNIT_TEST(test_from_time);
        CPPUNIT_TEST(test_base64);
        CPPUNIT_TEST(test_md5sum);
        CPPUNIT_TEST(test_md5sum_file);
        CPPUNIT_TEST(test_md5_digest_password);
#if defined WITH_CRYPTO
        CPPUNIT_TEST(test_blowfish);
#endif
        CPPUNIT_TEST(test_bin2hex);
        CPPUNIT_TEST(test_power_of_2);
    CPPUNIT_TEST_SUITE_END();

public:
    UtilTest() : CppUnit::TestFixture() {}
    virtual ~UtilTest() noexcept {}

private:
    static energonsoftware::Logger& logger;

public:
    void test_get_time()
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(get_time_noboost(), energonsoftware::get_time(), 0.001);
    }

    void test_from_time()
    {
        const double current_time = energonsoftware::get_time();
        boost::posix_time::ptime current_ptime(energonsoftware::from_time(static_cast<uint64_t>(current_time)));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(current_time, energonsoftware::get_time(current_ptime), 1.0);
    }

    void test_base64()
    {
        static const std::string data("123456");
        static const std::string encoded_data("MTIzNDU2");

        char* encoded = energonsoftware::base64_encode(reinterpret_cast<const unsigned char*>(data.c_str()), data.length());
        CPPUNIT_ASSERT_EQUAL(encoded_data, std::string(encoded));

        size_t len;
        unsigned char* decoded = energonsoftware::base64_decode(encoded, len);
        delete[] encoded;

        std::string output(reinterpret_cast<char*>(decoded), len);
        delete[] decoded;

        CPPUNIT_ASSERT_EQUAL(data, output);
    }

    void test_md5sum()
    {
        static const std::string input("this is a string");
        static const std::string expected("b37e16c620c055cf8207b999e3270e9b");
        char output[(MD5_DIGEST_LENGTH * 2) + 1];

        energonsoftware::md5sum_hex(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), output);
        CPPUNIT_ASSERT_EQUAL(expected, std::string(output));
    }

    void test_md5sum_file()
    {
        static const boost::filesystem::path filename(energonsoftware::data_dir() / "test.txt");
        static const std::string expected("c5fc01593740fd9ab9fb6b4f5dccc369");
        char output[(MD5_DIGEST_LENGTH * 2) + 1];

        CPPUNIT_ASSERT(energonsoftware::md5sum_file(filename, output));
        CPPUNIT_ASSERT_EQUAL(expected, std::string(output));
    }

    void test_md5_digest_password()
    {
        static const std::string expected("d242e9f26a573ab24f8673cc9576baad");

        std::string passwordmd5(energonsoftware::md5_digest_password("test1", "mmorpg", "test"));
        CPPUNIT_ASSERT_EQUAL(expected, passwordmd5);
    }

#if defined WITH_CRYPTO
    void test_blowfish()
    {
        static const std::string data("I'm a fish, but not a blowfish!");
        static const unsigned char key[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

        size_t ilen = 0;
        unsigned char encoded[1024];
        CPPUNIT_ASSERT(energonsoftware::blowfish_encrypt(key, reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), encoded, ilen));

        // TODO: check the encrypted copy

        size_t olen = 0;
        unsigned char decoded[1024];
        CPPUNIT_ASSERT(energonsoftware::blowfish_decrypt(key, encoded, ilen, decoded, olen));
        CPPUNIT_ASSERT_EQUAL(data, std::string(reinterpret_cast<char*>(decoded), olen));
    }
#endif

    void test_bin2hex()
    {
        std::string orig("hi there, how have you been this fine evening? I'm doing pretty good myself.");
        std::string enc(energonsoftware::bin2hex(reinterpret_cast<const unsigned char*>(orig.c_str()), orig.length()));

        LOG_INFO(orig << "\n");
        LOG_INFO(enc << "\n");

        orig = "0123456789012345";
        enc = energonsoftware::bin2hex(reinterpret_cast<const unsigned char*>(orig.c_str()), orig.length());

        LOG_INFO(orig << "\n");
        LOG_INFO(enc << "\n");

        orig = "01234567890123456";
        enc = energonsoftware::bin2hex(reinterpret_cast<const unsigned char*>(orig.c_str()), orig.length());

        LOG_INFO(orig << "\n");
        LOG_INFO(enc << "\n");
    }

    void test_power_of_2()
    {
        //CPPUNIT_ASSERT(POWER_OF_2(0));
        CPPUNIT_ASSERT(POWER_OF_2(1));
        CPPUNIT_ASSERT(POWER_OF_2(2));
        CPPUNIT_ASSERT(!POWER_OF_2(200));
        CPPUNIT_ASSERT(POWER_OF_2(256));
    }

private:
    double get_time_noboost()
    {
        timeval tv;

#if defined WIN32
        /* from http://www.cpp-programming.net/c-tidbits/gettimeofday-function-for-windows/ */
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);

        unsigned __int64 tmpres = 0;
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // converting file time to unix epoch
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tmpres /= 10;   // convert into microseconds
        tv.tv_sec = static_cast<long>(tmpres / 1000000UL);
        tv.tv_usec = static_cast<long>(tmpres % 1000000UL);
#else
        gettimeofday(&tv, nullptr);
#endif

        return tv.tv_sec + (tv.tv_usec * 1e-6);
    }
};
energonsoftware::Logger& UtilTest::logger(energonsoftware::Logger::instance("energonsoftware.core.util.UtilTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(UtilTest);

#endif
