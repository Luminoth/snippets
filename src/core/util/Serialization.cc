#include "src/pch.h"
#include "Serialization.h"

namespace energonsoftware {

SerializationMap::SerializationMap()
    : std::map<std::string, std::string>(), Serializable()
{
}

SerializationMap::~SerializationMap() throw()
{
}

void SerializationMap::serialize(Packer& packer) const throw(SerializationError)
{
    packer.pack(static_cast<uint32_t>(size()), "size");
    for(const auto& i : *this) {
        packer.pack(i.first, "key");
        packer.pack(i.second, "value");
    }
}

void SerializationMap::deserialize(Unpacker& unpacker) throw(SerializationError)
{
    uint32_t size;
    unpacker.unpack(size, "size");
    for(uint32_t i=0; i<size; ++i) {
        std::string key, value;
        unpacker.unpack(key, "key");
        unpacker.unpack(value, "value");
        (*this)[key] = value;
    }
}

}

#ifdef WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class SerializationTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(SerializationTest);
        CPPUNIT_TEST(test_1);
        CPPUNIT_TEST(test_SerializationMap);
        CPPUNIT_TEST(test_vector);
        CPPUNIT_TEST(test_list);
        CPPUNIT_TEST(test_deque);
        CPPUNIT_TEST(test_shared_ptr);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_1()
    {
        std::shared_ptr<energonsoftware::Packer> packer(energonsoftware::Packer::new_packer(energonsoftware::PackerType::Simple));
        CPPUNIT_ASSERT(static_cast<bool>(packer));

        packer->pack("uno", "1");
        packer->pack("dos", "2");
        packer->pack("tres", "3");

        std::shared_ptr<energonsoftware::Unpacker> unpacker(energonsoftware::Unpacker::new_unpacker(packer->buffer(), energonsoftware::PackerType::Simple));
        std::string s("almost empty");

        unpacker->unpack(s, "1");
        CPPUNIT_ASSERT_EQUAL(std::string("uno"), s);

        unpacker->unpack(s, "2");
        CPPUNIT_ASSERT_EQUAL(std::string("dos"), s);

        unpacker->unpack(s, "3");
        CPPUNIT_ASSERT_EQUAL(std::string("tres"), s);
    }

    void test_SerializationMap()
    {
        std::shared_ptr<energonsoftware::Packer> packer(energonsoftware::Packer::new_packer(energonsoftware::PackerType::Simple));
        CPPUNIT_ASSERT(static_cast<bool>(packer));

        energonsoftware::SerializationMap sm;
        sm["uno"] = "one";
        sm["dos"] = "two";
        sm["tres"] = "three";
        sm.serialize(*packer);

        std::shared_ptr<energonsoftware::Unpacker> unpacker(energonsoftware::Unpacker::new_unpacker(packer->buffer(), energonsoftware::PackerType::Simple));
        energonsoftware::SerializationMap recip;
        recip.deserialize(*unpacker);

        CPPUNIT_ASSERT_EQUAL(std::string("one"), recip["uno"]);
        CPPUNIT_ASSERT_EQUAL(std::string("two"), recip["dos"]);
        CPPUNIT_ASSERT_EQUAL(std::string("three"), recip["tres"]);
    }

    void test_vector()
    {
        std::vector<float> fv;
        fv.push_back(9.9f);
        fv.push_back(19.9f);
        fv.push_back(119.9f);

        std::shared_ptr<energonsoftware::Packer> packer(energonsoftware::Packer::new_packer(energonsoftware::PackerType::Simple));
        CPPUNIT_ASSERT(static_cast<bool>(packer));
        packer->pack(fv, "vector");

        std::shared_ptr<energonsoftware::Unpacker> unpacker(energonsoftware::Unpacker::new_unpacker(packer->buffer(), energonsoftware::PackerType::Simple));
        std::vector<float> nfv;
        unpacker->unpack(nfv, "vector");

        CPPUNIT_ASSERT_EQUAL(fv[0], nfv[0]);
        CPPUNIT_ASSERT_EQUAL(fv[1], nfv[1]);
        CPPUNIT_ASSERT_EQUAL(fv[2], nfv[2]);
    }

    void test_list()
    {
        std::list<float> fv;
        fv.push_back(9.9f);
        fv.push_back(19.9f);
        fv.push_back(119.9f);

        std::shared_ptr<energonsoftware::Packer> packer(energonsoftware::Packer::new_packer(energonsoftware::PackerType::Simple));
        CPPUNIT_ASSERT(static_cast<bool>(packer));
        packer->pack(fv, "list");

        std::shared_ptr<energonsoftware::Unpacker> unpacker(energonsoftware::Unpacker::new_unpacker(packer->buffer(), energonsoftware::PackerType::Simple));
        std::list<float> nfv;
        unpacker->unpack(nfv, "list");

        CPPUNIT_ASSERT_EQUAL(fv.front(), nfv.front());
        fv.pop_front();
        nfv.pop_front();

        CPPUNIT_ASSERT_EQUAL(fv.front(), nfv.front());
        fv.pop_front();
        nfv.pop_front();

        CPPUNIT_ASSERT_EQUAL(fv.front(), nfv.front());
    }

    void test_deque()
    {
        std::deque<float> fv;
        fv.push_back(9.9f);
        fv.push_back(19.9f);
        fv.push_back(119.9f);

        std::shared_ptr<energonsoftware::Packer> packer(energonsoftware::Packer::new_packer(energonsoftware::PackerType::Simple));
        CPPUNIT_ASSERT(static_cast<bool>(packer));
        packer->pack(fv, "deque");

        std::shared_ptr<energonsoftware::Unpacker> unpacker(energonsoftware::Unpacker::new_unpacker(packer->buffer(), energonsoftware::PackerType::Simple));
        std::deque<float> nfv;
        unpacker->unpack(nfv, "deque");

        CPPUNIT_ASSERT_EQUAL(fv[0], nfv[0]);
        CPPUNIT_ASSERT_EQUAL(fv[1], nfv[1]);
        CPPUNIT_ASSERT_EQUAL(fv[2], nfv[2]);
    }

    void test_shared_ptr()
    {
        std::shared_ptr<float> p3(new float(6));
        std::shared_ptr<energonsoftware::Packer> packer(energonsoftware::Packer::new_packer(energonsoftware::PackerType::Simple));
        CPPUNIT_ASSERT(static_cast<bool>(packer));
        packer->pack(p3, "shared_ptr");

        std::shared_ptr<energonsoftware::Unpacker> unpacker(energonsoftware::Unpacker::new_unpacker(packer->buffer(), energonsoftware::PackerType::Simple));
        std::shared_ptr<float> p12(new float(8.88f));
        unpacker->unpack(p12, "shared_ptr");

        CPPUNIT_ASSERT_EQUAL(6.0f, *p3);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTest);

#endif
