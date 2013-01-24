#include "src/pch.h"
#include "SessionId.h"
#include "Random.h"
#include "util.h"

namespace energonsoftware {

SessionId::SessionId(const std::string& password, int expiry)
    : _password(password), _expiry(expiry), _sessionid(),
        _creation_time(static_cast<long>(get_time()))
{
    // generate the seed values (assume the RNG is seeded)
    unsigned long salt1 = Random::large_rand_range(10000);
    unsigned long salt2 = Random::large_rand_range(10000);

    // the value to encrypt
    std::stringstream value;
    value << salt1 << ":" << creation_time() << ":" << salt2;

    // get the password md5 (our key)
    char password_md5[(MD5_DIGEST_LENGTH << 1) + 1];
    md5sum_hex(reinterpret_cast<const unsigned char*>(this->password().c_str()), this->password().length(), password_md5);

    // encrypt the value
    size_t len = 0;
    unsigned char encrypted[1024];
    blowfish_encrypt(reinterpret_cast<const unsigned char*>(password_md5),
        reinterpret_cast<const unsigned char*>(value.str().c_str()),
        value.str().length(), encrypted, len);

    // encode the result
    std::shared_ptr<char> encoded(base64_encode(encrypted, len));
    _sessionid = encoded.get();
}

SessionId::SessionId(const std::string& password, const std::string& sessionid, int expiry)
    : _password(password), _expiry(expiry), _sessionid(sessionid),
        _creation_time(0L)
{
    // unencode the value
    size_t len = 0;
    std::shared_ptr<unsigned char> decoded(base64_decode(this->sessionid().c_str(), len));

    // get the password md5 (our key)
    char password_md5[(MD5_DIGEST_LENGTH << 1) + 1];
    md5sum_hex(reinterpret_cast<const unsigned char*>(this->password().c_str()), this->password().length(), password_md5);

    // decrypt the session id
    size_t olen = 0;
    unsigned char decrypted[1024];
    blowfish_decrypt(reinterpret_cast<const unsigned char*>(password_md5), decoded.get(), len, decrypted, olen);

    std::string value(reinterpret_cast<char*>(decrypted), olen);

    size_t pos1 = value.find(":");
    size_t pos2 = value.rfind(":");

    _creation_time = atol(value.substr(pos1+1, pos2 - pos1).c_str());
}

SessionId::~SessionId() throw()
{
}

bool SessionId::expired() const
{
    if(_expiry < 0) {
        return false;
    }

    long expires = creation_time() + expiry();
    double now = get_time();
    return now >= expires;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class SessionIdTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(SessionIdTest);
        CPPUNIT_TEST(test_expiry);
        CPPUNIT_TEST(test_sessionid);
    CPPUNIT_TEST_SUITE_END();

private:
    static energonsoftware::Logger& logger;

public:
    SessionIdTest() : CppUnit::TestFixture() {}
    virtual ~SessionIdTest() throw() {}

public:
    void test_expiry()
    {
        energonsoftware::SessionId sessionid("test", 5);
        usleep(5000);
        CPPUNIT_ASSERT_EQUAL(true, sessionid.expired());
    }

    void test_sessionid()
    {
        energonsoftware::SessionId sessionid1("test");
        LOG_INFO("SessionId: " << sessionid1.sessionid().c_str() << "\n");

        energonsoftware::SessionId sessionid2("test", sessionid1.sessionid());
        CPPUNIT_ASSERT_EQUAL(sessionid1.password(), sessionid2.password());
        CPPUNIT_ASSERT_EQUAL(sessionid1.expiry(), sessionid2.expiry());
        CPPUNIT_ASSERT_EQUAL(sessionid1.sessionid(), sessionid2.sessionid());
        CPPUNIT_ASSERT_EQUAL(sessionid1.creation_time(), sessionid2.creation_time());
    }
};

energonsoftware::Logger& SessionIdTest::logger(energonsoftware::Logger::instance("energonsoftware.core.util.SessionIdTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(SessionIdTest);

#endif
