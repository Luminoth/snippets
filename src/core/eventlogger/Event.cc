#include "src/pch.h"
#include "src/core/util/util.h"
#include "Event.h"

namespace energonsoftware {

const std::string Event::Header::MAGIC("EVT");
const uint32_t Event::Header::VERSION = 1;

void Event::Header::serialize(Packer& packer) const throw(SerializationError)
{
    packer.pack(MAGIC, "magic");
    packer.pack(VERSION, "version");
}

void Event::Header::deserialize(Unpacker& unpacker) throw(SerializationError)
{
    std::string magic;
    unpacker.unpack(magic, "magic");
    if(MAGIC != magic) {
        throw SerializationError("Event header MAGIC mismatch!");
    }

    uint32_t version;
    unpacker.unpack(version, "version");
    if(VERSION != version) {
        throw SerializationError("Event header VERSION mismatch!");
    }
}

std::atomic_uint_least64_t Event::_next_id(0UL);

Event::Event()
    : Serializable(), _id(0UL),
        _timestamp(boost::posix_time::microsec_clock::universal_time()), _type()
{
}

Event::Event(std::shared_ptr<EventType> type)
    : Serializable(), _id(next_id()),
        _timestamp(boost::posix_time::microsec_clock::universal_time()), _type(type)
{
}

Event::~Event() throw()
{
}

void Event::serialize(Packer& packer) const throw(SerializationError)
{
    if(!valid()) {
        throw SerializationError("Invalid event!");
    }

    Header().serialize(packer);

    packer.pack(_id, "id");
    packer.pack(static_cast<uint64_t>(get_time(_timestamp)), "timestamp");

    packer.pack(_type->type(), "type");
    packer.pack(_type->version(), "type_version");
    _type->serialize(packer);
}

void Event::deserialize(Unpacker& unpacker) throw(SerializationError)
{
    Header().deserialize(unpacker);

    unpacker.unpack(_id, "id");

    uint64_t timestamp;
    unpacker.unpack(timestamp, "timestamp");
    _timestamp = from_time(timestamp);

    uint32_t type;
    unpacker.unpack(type, "type");
    /*_type = create_type(type);
    if(_type->type() != type) {
        throw SerializationError("Event type mismatch!");
    }

    uint32_t version;
    unpacker.unpack(version, "type_version");
    if(_type->version() != version) {
        throw SerializationError("Event type version mismatch!");
    }
    _type->deserialize(unpacker);*/

    if(!valid()) {
        throw SerializationError("Invalid event!");
    }
}

std::string Event::str() const
{
    std::stringstream ss;
    ss << "Event(id:" << _id << ", timestamp:" << boost::posix_time::to_simple_string(_timestamp);
    if(_type) {
        ss << ", type:" << _type->str();
    } else {
        ss << ", invalid type";
    }
    ss << ")";
    return ss.str();
}

}

#if defined WITH_UNIT_TESTS
const uint32_t TestEvent::TEST_EVENT_ID = 1;

const bool TestEvent::TEST_BOOL = true;
const uint8_t TestEvent::TEST_CHAR = 'a';
const uint32_t TestEvent::TEST_INT = 378;
const uint64_t TestEvent::TEST_LONG = 3467457L;
const float TestEvent::TEST_FLOAT = 23.563F;
const double TestEvent::TEST_DOUBLE = 78563.345634;
const std::string TestEvent::TEST_STRING = "This is a test string!";

TestEvent::TestEvent()
    : energonsoftware::EventType(TestEvent::TEST_EVENT_ID),
        _test_bool(TEST_BOOL), _test_char(TEST_CHAR), _test_int(TEST_INT),
        _test_long(TEST_LONG), _test_float(TEST_FLOAT), _test_double(TEST_DOUBLE),
        _test_string(TEST_STRING)
{
}

TestEvent::~TestEvent() throw()
{
}

void TestEvent::serialize(energonsoftware::Packer& packer) const throw(energonsoftware::SerializationError)
{
    packer.pack(_test_bool, "bool");
    packer.pack(_test_char, "char");
    packer.pack(_test_int, "int");
    packer.pack(_test_long, "long");
    packer.pack(_test_float, "float");
    packer.pack(_test_double, "double");
    packer.pack(_test_string, "string");
}

void TestEvent::deserialize(energonsoftware::Unpacker& unpacker) throw(energonsoftware::SerializationError)
{
    bool b;
    unpacker.unpack(b, "bool");
    if(TEST_BOOL != b) {
        throw energonsoftware::SerializationError("Error unpacking boolean!");
    }

    uint8_t ch;
    unpacker.unpack(ch, "char");
    if(TEST_CHAR != ch) {
        throw energonsoftware::SerializationError("Error unpacking char!");
    }

    uint32_t i;
    unpacker.unpack(i, "int");
    if(TEST_INT != i) {
        throw energonsoftware::SerializationError("Error unpacking int!");
    }

    uint64_t l;
    unpacker.unpack(l, "long");
    if(TEST_LONG != l) {
        throw energonsoftware::SerializationError("Error unpacking long!");
    }

    float f;
    unpacker.unpack(f, "float");
    if(TEST_FLOAT != f) {
        throw energonsoftware::SerializationError("Error unpacking float!");
    }

    double d;
    unpacker.unpack(d, "double");
    if(TEST_DOUBLE != d) {
        throw energonsoftware::SerializationError("Error unpacking double!");
    }

    std::string s;
    unpacker.unpack(s, "string");
    if(TEST_STRING != s) {
        throw energonsoftware::SerializationError("Error unpacking string!");
    }
}
#endif
