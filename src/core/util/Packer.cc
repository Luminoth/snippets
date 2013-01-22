#include "src/pch.h"
#include "Packer.h"
#include "Serialization.h"
/*#include "BinaryPacker.h"
#include "SimplePacker.h"
#include "XmlPacker.h"*/

namespace energonsoftware {

Logger& Packer::logger(Logger::instance("energonsoftware.core.util.Packer"));

Packer* Packer::new_packer(const std::string& type, const boost::any& data)
{
    const std::string scratch(boost::algorithm::to_lower_copy(type));
    /*if(scratch == "simple") {
        return new SimplePacker();
    } else if(scratch == "binary") {
        return new BinaryPacker();
    } else if(scratch == "xml") {
        return new XmlPacker();
    }*/

    LOG_ERROR("Unknown packer type: " << type << ", returning nullptr\n");
    return nullptr;
}

bool Packer::is_valid_type(const std::string& type)
{
    const std::string scratch(boost::algorithm::to_lower_copy(type));
    return scratch == "simple" || scratch == "binary" || scratch == "xml";
}

Packer& Packer::pack(const Serializable& v, const std::string& name) throw(PackerError)
{
    v.serialize(*this);
    return *this;
}

Logger& Unpacker::logger(Logger::instance("energonsoftware.core.util.Unpacker"));

Unpacker* Unpacker::new_unpacker(const std::string& obj, const std::string& type, const boost::any& data)
{
    const std::string scratch(boost::algorithm::to_lower_copy(type));
    /*if(scratch == "simple") {
        return new SimpleUnpacker(obj);
    } else if(scratch == "binary") {
        return new BinaryUnpacker(obj);
    } else if(scratch == "xml") {
        return new XmlUnpacker(obj);
    }*/

    LOG_ERROR("Unknown packer type: " << type << ", returning nullptr\n");
    return nullptr;
}

Unpacker* Unpacker::new_unpacker(const unsigned char* obj, size_t len, const std::string& type, const boost::any& data)
{
    const std::string scratch(boost::algorithm::to_lower_copy(type));
    /*if(scratch == "simple") {
        return new SimpleUnpacker(obj, len);
    } else if(scratch == "binary") {
        return new BinaryUnpacker(obj, len);
    } else if(scratch == "xml") {
        return new XmlUnpacker(obj, len);
    }*/

    LOG_ERROR("Unknown packer type: " << type << ", returning nullptr\n");
    return nullptr;
}

Unpacker::Unpacker(const std::vector<unsigned char>& obj)
    : _obj()
{
    _obj.assign(obj.begin(), obj.end());
}

Unpacker::Unpacker(const unsigned char* obj, size_t len)
    : _obj()
{
    _obj.assign(reinterpret_cast<const char*>(obj), len);
}

Unpacker& Unpacker::reset(const std::string& obj)
{
    _obj = obj;
    return on_reset();
}

Unpacker& Unpacker::unpack(Serializable& v, const std::string& name) throw(PackerError)
{
    v.deserialize(*this);
    return *this;
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class PackerTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(PackerTest);
        CPPUNIT_TEST(test_simple);
        CPPUNIT_TEST(test_binary);
        CPPUNIT_TEST(test_xml);
    CPPUNIT_TEST_SUITE_END();

private:
    std::shared_ptr<energonsoftware::Packer> create_packer(const std::string& type, const boost::any& data)
    {
        std::shared_ptr<energonsoftware::Packer> packer(energonsoftware::Packer::new_packer(type, data));
        CPPUNIT_ASSERT(static_cast<bool>(packer));

        packer->pack(std::string("std::string"), "test_string1");
        packer->pack((const char*)"oh hi", "test_string2");
        packer->pack(static_cast<int32_t>(-20), "test_int");
        packer->pack(static_cast<uint32_t>(12), "test_uint");
        packer->pack(static_cast<int64_t>(-100), "test_long");
        packer->pack(static_cast<uint64_t>(235), "test_ulong");
        packer->pack(23.234f, "test_float");
        packer->pack(-1023.32243, "test_double");
        packer->pack(true, "test_bool_true");
        packer->pack(false, "test_bool_false");

        return packer;
    }

    void unpack_packer(const std::string& type, const std::string& buffer, const boost::any& data)
    {
        std::shared_ptr<energonsoftware::Unpacker> unpacker(energonsoftware::Unpacker::new_unpacker(buffer, type, data));
        CPPUNIT_ASSERT(unpacker);

        std::string s;
        unpacker->unpack(s, "test_string1");
        CPPUNIT_ASSERT_EQUAL(std::string("std::string"), s);

        unpacker->unpack(s, "test_string2");
        CPPUNIT_ASSERT_EQUAL(std::string("oh hi"), s);

        int32_t i;
        unpacker->unpack(i, "test_int");
        CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(-20), i);

        uint32_t ui;
        unpacker->unpack(ui, "test_uint");
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(12), ui);

        int64_t l;
        unpacker->unpack(l, "test_long");
        CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(-100), l);

        uint64_t ul;
        unpacker->unpack(ul, "test_ulong");
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(235), ul);

        float f;
        unpacker->unpack(f, "test_float");
        CPPUNIT_ASSERT_DOUBLES_EQUAL(23.234f, f, 0.001f);

        double d;
        unpacker->unpack(d, "test_double");
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1023.32243, d, 0.001);

        bool b;
        unpacker->unpack(b, "test_bool_true");
        CPPUNIT_ASSERT_EQUAL(true, b);

        unpacker->unpack(b, "test_bool_false");
        CPPUNIT_ASSERT_EQUAL(false, b);
    }

    void test_packer(const std::string& type, const boost::any& data=boost::any(std::string("")))
    {
        std::shared_ptr<energonsoftware::Packer> packer(create_packer(type, data));
        unpack_packer(type, packer->buffer(), data);
    }

public:
    void test_simple()
    {
        test_packer("simple");
    }

    void test_binary()
    {
        test_packer("binary");
    }

    void test_xml()
    {
        test_packer("xml");
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PackerTest);

#endif
