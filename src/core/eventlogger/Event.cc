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

Event::Event(const EventType& type)
    : Serializable(), _id(next_id()),
        _timestamp(boost::posix_time::microsec_clock::universal_time()), _type(type)
{
}

Event::~Event() throw()
{
}

void Event::serialize(Packer& packer) const throw(SerializationError)
{
    Header().serialize(packer);

    packer.pack(_id, "id");
    packer.pack(static_cast<uint64_t>(get_time(_timestamp)), "timestamp");

    packer.pack(_type.type(), "type");
    packer.pack(_type.version(), "type_version");
    _type.serialize(packer);
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
    if(_type.type() != type) {
        throw SerializationError("Event type mismatch!");
    }

    uint32_t version;
    unpacker.unpack(version, "type_version");
    if(_type.version() != version) {
        throw SerializationError("Event type version mismatch!");
    }
    _type.deserialize(unpacker);*/
}

std::string Event::str() const
{
    std::stringstream ss;
    ss << "Event(id:" << _id << ", timestamp:" << boost::posix_time::to_simple_string(_timestamp) << ", type:" << _type.str() << ")";
    return ss.str();
}

}
