#include "src/pch.h"
#include "Nonce.h"
#include "util.h"

namespace energonsoftware {

Nonce::Nonce(const std::string& realm, int expiry)
    : _realm(realm), _expiry(expiry), _nonce(), _nonce_md5(),
        _creation_time(static_cast<long>(get_time()))
{
    std::stringstream value;
    value << creation_time() << ":" << this->realm();
    _nonce = value.str();

    calc_md5();
}

Nonce::Nonce(const std::string& realm, const std::string& nonce, int expiry)
    : _realm(realm), _expiry(expiry), _nonce(nonce), _nonce_md5(),
        _creation_time(0L)
{
    size_t pos = this->nonce().find(":");
    _creation_time = atol(this->nonce().substr(0, pos).c_str());
    calc_md5();
}

Nonce::~Nonce() throw()
{
}

bool Nonce::expired() const
{
    long expires = creation_time() + expiry();
    double now = get_time();
    return now >= expires;
}

void Nonce::calc_md5()
{
    char nonce_md5[(MD5_DIGEST_LENGTH << 1) + 1];
    md5sum_hex(reinterpret_cast<const unsigned char*>(nonce().c_str()), nonce().length(), nonce_md5);
    _nonce_md5 = nonce_md5;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class NonceTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(NonceTest);
        CPPUNIT_TEST(test_expiry);
        CPPUNIT_TEST(test_nonce);
    CPPUNIT_TEST_SUITE_END();

private:
    static energonsoftware::Logger& logger;

public:
    NonceTest() : CppUnit::TestFixture() {}
    virtual ~NonceTest() throw() {}

public:
    void test_expiry()
    {
        energonsoftware::Nonce nonce("test", 5);
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        CPPUNIT_ASSERT(!nonce.expired());
        boost::this_thread::sleep(boost::posix_time::seconds(5));
        CPPUNIT_ASSERT(nonce.expired());
    }

    void test_nonce()
    {
        energonsoftware::Nonce nonce1("test", 60);
        LOG_INFO("Nonce: " << nonce1.nonce().c_str() << "\n");
        //LOG_INFO("MD5: " << nonce1.md5().c_str() << "\n");

        energonsoftware::Nonce nonce2("test", nonce1.nonce(), 60);
        CPPUNIT_ASSERT_EQUAL(nonce1.realm(), nonce2.realm());
        CPPUNIT_ASSERT_EQUAL(nonce1.expiry(), nonce2.expiry());
        CPPUNIT_ASSERT_EQUAL(nonce1.nonce(), nonce2.nonce());
        CPPUNIT_ASSERT_EQUAL(nonce1.md5(), nonce2.md5());
        CPPUNIT_ASSERT_EQUAL(nonce1.creation_time(), nonce2.creation_time());
    }
};

energonsoftware::Logger& NonceTest::logger(energonsoftware::Logger::instance("energonsoftware.core.util.NonceTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(NonceTest);

#endif
