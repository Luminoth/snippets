#if !defined __BINARYMESSAGE_H__
#define __BINARYMESSAGE_H__

#include "src/core/util/Serialization.h"
#include "BufferedMessage.h"

namespace energonsoftware {

class Packer;
class Unpacker;

// message format: length \r\n type | header | payload
class BinaryMessage : public BufferedMessage, public Serializable
{
public:
    typedef std::unordered_map<std::string, boost::any> Payload;

protected:
    static Logger& logger;

public:
    explicit BinaryMessage(PackerType packer_type, uint32_t type, const Payload& payload=Payload());

    // NOTE: this is pretty slow because it has to copy the payload
    BinaryMessage(const BinaryMessage& message);

    virtual ~BinaryMessage() noexcept;

public:
    PackerType packer_type() const { return _packer_type; }
    void packer_type(PackerType packer_type) { _packer_type = packer_type; }

    // check this when deserializing to see if the message
    // has all of the data necessary
    bool complete() const { return _complete; }

    uint32_t type() const { return _type; }
    const Payload& payload() const { return _payload; }

    virtual BufferedMessageType msg_type() const override { return BufferedMessageType::Binary; }

    virtual void serialize(Packer& packer) const throw(SerializationError) override;
    virtual void deserialize(Unpacker& unpacker) throw(SerializationError) override;

public:
    BinaryMessage& operator=(const BinaryMessage& rhs);
    bool operator==(const BinaryMessage& rhs) const;
    bool operator!=(const BinaryMessage& rhs) const;

protected:
    // return a string representation of the header
    virtual std::string header() const = 0;

    // pack/unpack the message header
    virtual void on_serialize_header(Packer& packer) const = 0;
    virtual void on_deserialize_header(Unpacker& unpacker) = 0;

    // pack/unpack the message payload
    virtual void on_serialize_payload(Packer& packer) const throw(std::bad_cast, std::out_of_range) = 0;
    virtual void on_deserialize_payload(Unpacker& unpacker) = 0;

private:
    size_t unpack_message_len(Unpacker& unpacker) const;
    void complete(bool complete) { _complete = complete; }
    virtual const unsigned char* data() /*const*/ override;
    virtual size_t data_len() const override;

protected:
    Payload _payload;

private:
    PackerType _packer_type;
    uint32_t _type;
    std::shared_ptr<std::string> _data;
    bool _complete;
};

class ClientBinaryMessage : public BinaryMessage
{
public:
    explicit ClientBinaryMessage(PackerType packer_type, uint32_t type, const std::string& sessionid="", const Payload& payload=Payload());

    // NOTE: this is pretty slow because it has to copy the payload
    ClientBinaryMessage(const ClientBinaryMessage& message);

    virtual ~ClientBinaryMessage() noexcept;

public:
    const std::string& sessionid() const { return _sessionid; }

    ClientBinaryMessage& operator=(const ClientBinaryMessage& message);
    bool operator==(const ClientBinaryMessage& rhs) const;
    bool operator!=(const ClientBinaryMessage& rhs) const;

private:
    virtual std::string header() const override { return sessionid(); }
    virtual void on_serialize_header(Packer& packer) const override;
    virtual void on_deserialize_header(Unpacker& unpacker) override;

private:
    std::string _sessionid;
};

class ServerBinaryMessage : public BinaryMessage
{
public:
    explicit ServerBinaryMessage(PackerType packer_type, uint32_t type, bool oob=true, const Payload& payload=Payload());

    // NOTE: this is pretty slow because it has to copy the payload
    ServerBinaryMessage(const ServerBinaryMessage& message);

    virtual ~ServerBinaryMessage() noexcept;

public:
    bool oob() const { return _oob; }

    ServerBinaryMessage& operator=(const ServerBinaryMessage& message);
    bool operator==(const ServerBinaryMessage& rhs) const;
    bool operator!=(const ServerBinaryMessage& rhs) const;

private:
    virtual std::string header() const override { return oob() ? "1" : "0"; }
    virtual void on_serialize_header(Packer& packer) const override;
    virtual void on_deserialize_header(Unpacker& unpacker) override;

private:
    bool _oob;
};

}

#endif
