#if !defined __SERIALIZATION_H__
#define __SERIALIZATION_H__

#include "Packer.h"

namespace energonsoftware {

class SerializationError : public std::exception
{
public:
    explicit SerializationError(const std::string& what) noexcept : _what(what) {}
    virtual ~SerializationError() noexcept {}
    virtual const char* what() const noexcept { return _what.c_str(); }

private:
    std::string _what;
};

class Serializable
{
public:
    Serializable() {}
    virtual ~Serializable() noexcept {}

public:
    virtual void serialize(Packer& packer) const throw(SerializationError) = 0;
    virtual void deserialize(Unpacker& unpacker) throw(SerializationError) = 0;
};

class SerializationMap : public std::map<std::string, std::string>, public Serializable
{
public:
    SerializationMap();
    virtual ~SerializationMap() noexcept;

public:
    virtual void serialize(Packer& packer) const throw(SerializationError);
    virtual void deserialize(Unpacker& unpacker) throw(SerializationError);

public:
    template <typename T>
    void add(const std::string& key, const T& t)
    {
        std::shared_ptr<Packer> packer(Packer::new_packer(PackerType::Simple));
        packer->pack(t, key);
        (*this)[key] = packer->buffer();
    }

    template <typename T>
    void get(const std::string& key, T& t)
    {
        std::shared_ptr<Unpacker> unpacker(Unpacker::new_unpacker((*this)[key], PackerType::Simple));
        unpacker->unpack(t, key);
    }
};

}

#endif
