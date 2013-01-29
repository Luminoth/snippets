#if !defined __EVENT_H__
#define __EVENT_H__

#include "src/core/util/Serialization.h"

namespace energonsoftware {

// TODO: deserializing events does not work!
// TODO: serializing/deserializing can throw packer errors that we aren't handling anywhere

// the meaning of type is application dependent
// NOTE: type 0 == unknown type, don't use 0 as a legit type!
class EventType : public Serializable
{
public:
    EventType() : Serializable(), _type(0) {}
    virtual ~EventType() throw() {}

protected:
    explicit EventType(uint32_t type) : Serializable(), _type(type) {}

public:
    virtual uint32_t type() const final { return _type; }
    virtual bool valid() const final { return _type > 0; }

public:
    virtual uint32_t version() const { return 1; }
    virtual void serialize(Packer& packer) const throw(SerializationError) override {}
    virtual void deserialize(Unpacker& unpacker) throw(SerializationError) override {}
    virtual std::string str() const { return "Unknown Event Type!"; }

private:
    uint32_t _type;
};

class Event final : public Serializable
{
private:
    class Header final : public Serializable
    {
    private:
        static const std::string MAGIC;
        static const uint32_t VERSION;

    public:
        Header() : Serializable() {}
        virtual ~Header() throw() {}

    public:
        virtual void serialize(Packer& packer) const throw(SerializationError) override;
        virtual void deserialize(Unpacker& unpacker) throw(SerializationError) override;
    };

private:
    static uint64_t next_id() { return ++_next_id; }

private:
    static std::atomic_uint_least64_t _next_id;

public:
    Event();
    explicit Event(std::shared_ptr<EventType> type);
    virtual ~Event() throw();

public:
    uint64_t id() const { return _id; }

    // universal timestamp in microseconds
    // NOTE: this gets logged with second precision
    const boost::posix_time::ptime& timestamp() const { return _timestamp; }
    void timestamp(const boost::posix_time::ptime& timestamp) { _timestamp = timestamp; }

    const EventType& type() const { return *_type; }

    bool valid() const { return _id > 0 && _type && _type->valid(); }

    virtual void serialize(Packer& packer) const throw(SerializationError) override;
    virtual void deserialize(Unpacker& unpacker) throw(SerializationError) override;

    std::string str() const;

private:
    uint64_t _id;
    boost::posix_time::ptime _timestamp;
    std::shared_ptr<EventType> _type;
};

}

#if defined WITH_UNIT_TESTS
class TestEvent : public energonsoftware::EventType
{
public:
    static const uint32_t TEST_EVENT_ID;

private:
    static const bool TEST_BOOL;
    static const uint8_t TEST_CHAR;
    static const uint32_t TEST_INT;
    static const uint64_t TEST_LONG;
    static const float TEST_FLOAT;
    static const double TEST_DOUBLE;
    static const std::string TEST_STRING;

public:
    TestEvent();
    virtual ~TestEvent() throw();

public:
    virtual uint32_t version() const { return 1; }
    virtual void serialize(energonsoftware::Packer& packer) const throw(energonsoftware::SerializationError) override;
    virtual void deserialize(energonsoftware::Unpacker& unpacker) throw(energonsoftware::SerializationError) override;
    virtual std::string str() const { return "TestEvent()"; }

private:
    bool _test_bool;
    uint8_t _test_char;
    uint32_t _test_int;
    uint64_t _test_long;
    float _test_float;
    double _test_double;
    std::string _test_string;
};
#endif

#endif
