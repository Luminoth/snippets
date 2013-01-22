#if !defined __SERIALIZATION_H__
#define __SERIALIZATION_H__

#include "Packer.h"

namespace energonsoftware {

class Serializable
{
public:
    Serializable()
    {
    }

    virtual ~Serializable() throw()
    {
    }

public:
    virtual void serialize(Packer& packer) const = 0;
    virtual void deserialize(Unpacker& unpacker) = 0;
};

class SerializationMap : public std::map<std::string, std::string>, public Serializable
{
public:
    SerializationMap();
    virtual ~SerializationMap() throw();

public:
    virtual void serialize(Packer& packer) const;
    virtual void deserialize(Unpacker& unpacker);

public:
    template <typename T>
    void add(const std::string& key, const T& t)
    {
        std::shared_ptr<Packer> packer(Packer::new_packer("simple"));
        packer->pack(t, key);
        (*this)[key] = packer->buffer();
    }

    template <typename T>
    void get(const std::string& key, T& t)
    {
        std::shared_ptr<Unpacker> unpacker(Unpacker::new_unpacker((*this)[key], "simple"));
        unpacker->unpack(t, key);
    }
};

}

#endif
