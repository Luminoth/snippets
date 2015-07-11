#include "src/pch.h"
#include "src/core/util/Packer.h"
#include "BinaryMessage.h"

namespace energonsoftware {

Logger& BinaryMessage::logger(Logger::instance("energonsoftware.core.messages.BinaryMessage"));

BinaryMessage::BinaryMessage(PackerType packer_type, uint32_t type, const Payload& payload)
    : BufferedMessage(false), Serializable(), _payload(payload), _packer_type(packer_type),
        _type(type), _data(), _complete(true)
{
}

BinaryMessage::BinaryMessage(const BinaryMessage& message)
    : BufferedMessage(message), Serializable(), _payload(message._payload), _packer_type(message._packer_type),
        _type(message._type), _data(), _complete(true)
{
    // NOTE: we don't need to copy the _data pointer
}

BinaryMessage::~BinaryMessage() noexcept
{
}

void BinaryMessage::serialize(Packer& packer) const throw(SerializationError)
{
    packer.pack(type(), "type");

    try {
        on_serialize_header(packer);
        on_serialize_payload(packer);
    } catch(const std::bad_cast& e) {
        LOG_ERROR("Error serializing binary message: " << e.what() << "\n");
    } catch(const std::out_of_range& e) {
        LOG_ERROR("Error serializing binary message: " << e.what() << "\n");
    }
}

size_t BinaryMessage::unpack_message_len(Unpacker& unpacker) const
{
    size_t pos = unpacker.original_object().find("\r\n");
    if(pos == std::string::npos) {
        return 0;
    }
    unpacker.skip(pos + 2);

    std::string scratch(unpacker.original_object().substr(0, pos));
    return atoi(scratch.c_str());
}

void BinaryMessage::deserialize(Unpacker& unpacker) throw(SerializationError)
{
    complete(false);

    size_t len = unpack_message_len(unpacker);
    if(len == 0) {
        LOG_ERROR("Invalid binary message length!\n");
        return;
    }

    if(len > unpacker.original_object().length()) {
        return;
    }
    complete(true);

    unpacker.unpack(_type, "type");

    on_deserialize_header(unpacker);
    on_deserialize_payload(unpacker);
}

BinaryMessage& BinaryMessage::operator=(const BinaryMessage& rhs)
{
    BufferedMessage::operator=(rhs);

    _payload = rhs._payload;    // this is pretty darn slow
    _type = rhs._type;

    // NOTE: we don't need to copy the _data pointer
    return *this;
}

bool BinaryMessage::operator==(const BinaryMessage& rhs) const
{
    if(type() != rhs.type()) {
        return false;
    }

    for(const auto& i : payload()) {
        if(rhs.payload().find(i.first) == rhs.payload().end()) {
            return false;
        }

        /*if(rhs.payload().at(i.first) != i.second) {
            return false;
        }*/
    }

    return true;
}

bool BinaryMessage::operator!=(const BinaryMessage& rhs) const
{
    return !(*this == rhs);
}

const unsigned char* BinaryMessage::data() //const
{
    std::shared_ptr<Packer> packer(Packer::new_packer(_packer_type));
    serialize(*packer);

    // get the length on there
    std::string buffer(packer->buffer());
    std::stringstream data;
    data << buffer.length() << "\r\n" << buffer;

    _data.reset(new std::string());
    _data->assign(data.str());

    return reinterpret_cast<const unsigned char*>(_data->c_str());
}

size_t BinaryMessage::data_len() const
{
    return _data ? _data->length() : 0;
}

ClientBinaryMessage::ClientBinaryMessage(PackerType packer_type, uint32_t type, const std::string& sessionid, const Payload& payload)
    : BinaryMessage(packer_type, type, payload), _sessionid(sessionid)
{
}

ClientBinaryMessage::ClientBinaryMessage(const ClientBinaryMessage& message)
    : BinaryMessage(message), _sessionid(message._sessionid)
{
}

ClientBinaryMessage::~ClientBinaryMessage() noexcept
{
}

ClientBinaryMessage& ClientBinaryMessage::operator=(const ClientBinaryMessage& message)
{
    BinaryMessage::operator=(message);

    _sessionid = message._sessionid;
    return *this;
}

bool ClientBinaryMessage::operator==(const ClientBinaryMessage& rhs) const
{
    return BinaryMessage::operator==(rhs) && sessionid() == rhs.sessionid();
}

bool ClientBinaryMessage::operator!=(const ClientBinaryMessage& rhs) const
{
    return !(*this == rhs);
}

void ClientBinaryMessage::on_serialize_header(Packer& packer) const
{
    packer.pack(sessionid(), "sessionid");
}

void ClientBinaryMessage::on_deserialize_header(Unpacker& unpacker)
{
    unpacker.unpack(_sessionid, "sessionid");
}

ServerBinaryMessage::ServerBinaryMessage(PackerType packer_type, uint32_t type, bool oob, const Payload& payload)
    : BinaryMessage(packer_type, type, payload), _oob(oob)
{
}

ServerBinaryMessage::ServerBinaryMessage(const ServerBinaryMessage& message)
    : BinaryMessage(message), _oob(message._oob)
{
}

ServerBinaryMessage::~ServerBinaryMessage() noexcept
{
}

ServerBinaryMessage& ServerBinaryMessage::operator=(const ServerBinaryMessage& message)
{
    BinaryMessage::operator=(message);

    _oob = message._oob;
    return *this;
}

bool ServerBinaryMessage::operator==(const ServerBinaryMessage& rhs) const
{
    return BinaryMessage::operator==(rhs) && oob() == rhs.oob();
}

bool ServerBinaryMessage::operator!=(const ServerBinaryMessage& rhs) const
{
    return !(*this == rhs);
}

void ServerBinaryMessage::on_serialize_header(Packer& packer) const
{
    packer.pack(oob(), "oob");
}

void ServerBinaryMessage::on_deserialize_header(Unpacker& unpacker)
{
    unpacker.unpack(_oob, "oob");
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class ClientBinaryMessageTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(ClientBinaryMessageTest);
    CPPUNIT_TEST_SUITE_END();

public:
    ClientBinaryMessageTest() : CppUnit::TestFixture() {}
    virtual ~ClientBinaryMessageTest() noexcept {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(ClientBinaryMessageTest);

class ServerBinaryMessageTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(ServerBinaryMessageTest);
    CPPUNIT_TEST_SUITE_END();

public:
    ServerBinaryMessageTest() : CppUnit::TestFixture() {}
    virtual ~ServerBinaryMessageTest() noexcept {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServerBinaryMessageTest);

#endif
