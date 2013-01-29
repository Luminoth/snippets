#include "src/pch.h"
#include "SimplePacker.h"

namespace energonsoftware {

SimplePacker::SimplePacker()
    : Packer(), _buffer()
{
}

SimplePacker::~SimplePacker() throw()
{
}

Packer& SimplePacker::reset()
{
    _buffer.str("");
    return *this;
}

Packer& SimplePacker::pack(const std::string& v, const std::string& name) throw(PackerError)
{
    pack(static_cast<uint32_t>(v.length()), name + "_length");
    _buffer.write(v.c_str(), v.length());
    return *this;
}

Packer& SimplePacker::pack(const char* const v, const std::string& name) throw(PackerError)
{
    uint32_t len = std::strlen(v);
    pack(len, name + "_length");
    _buffer.write(v, len);
    return *this;
}

Packer& SimplePacker::pack(int8_t v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(int8_t));
    return *this;
}

Packer& SimplePacker::pack(uint8_t v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(uint8_t));
    return *this;
}

Packer& SimplePacker::pack(int32_t v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(int32_t));
    return *this;
}

Packer& SimplePacker::pack(uint32_t v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(uint32_t));
    return *this;
}

Packer& SimplePacker::pack(int64_t v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(int64_t));
    return *this;
}

Packer& SimplePacker::pack(uint64_t v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(uint64_t));
    return *this;
}

Packer& SimplePacker::pack(float v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(float));
    return *this;
}

Packer& SimplePacker::pack(double v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(double));
    return *this;
}

Packer& SimplePacker::pack(bool v, const std::string& name) throw(PackerError)
{
    _buffer.write(reinterpret_cast<const char*>(&v), sizeof(bool));
    return *this;
}

SimpleUnpacker::SimpleUnpacker(const std::string& obj)
    : Unpacker(obj), _buffer(obj)
{
}

SimpleUnpacker::SimpleUnpacker(const std::vector<unsigned char>& obj)
    : Unpacker(obj), _buffer()
{
    _buffer.str(_obj);
}

SimpleUnpacker::SimpleUnpacker(const unsigned char* obj, size_t len)
    : Unpacker(obj, len), _buffer()
{
    _buffer.str(_obj);
}

SimpleUnpacker::~SimpleUnpacker() throw()
{
}

Unpacker& SimpleUnpacker::position(unsigned int position) throw(PackerError)
{
    if(position >= _obj.length()) {
        throw PackerError("Position is beyond the length of the buffer!");
    }

    _buffer.seekg(position, std::ios_base::beg);
    return *this;
}

Unpacker& SimpleUnpacker::skip(unsigned int count) throw(PackerError)
{
    /*if(_buffer.tellg() + count >= _obj.length()) {
        throw PackerError("Position is beyond the length of the buffer!");
    }*/

    _buffer.ignore(count);
    return *this;
}

Unpacker& SimpleUnpacker::unpack(std::string& v, const std::string& name) throw(PackerError)
{
    uint32_t len;
    unpack(len, name + "_length");

    std::shared_ptr<char> n(new char[len]);
    _buffer.read(n.get(), len);
    v.assign(n.get(), len);
    return *this;
}

Unpacker& SimpleUnpacker::unpack(int8_t& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(int8_t));
    return *this;
}

Unpacker& SimpleUnpacker::unpack(uint8_t& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(uint8_t));
    return *this;
}

Unpacker& SimpleUnpacker::unpack(int32_t& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    return *this;
}

Unpacker& SimpleUnpacker::unpack(uint32_t& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(uint32_t));
    return *this;
}

Unpacker& SimpleUnpacker::unpack(int64_t& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(int64_t));
    return *this;
}

Unpacker& SimpleUnpacker::unpack(uint64_t& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(uint64_t));
    return *this;
}

Unpacker& SimpleUnpacker::unpack(float& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(float));
    return *this;
}

Unpacker& SimpleUnpacker::unpack(double& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(double));
    return *this;
}

Unpacker& SimpleUnpacker::unpack(bool& v, const std::string& name) throw(PackerError)
{
    _buffer.read(reinterpret_cast<char*>(&v), sizeof(bool));
    return *this;
}

Unpacker& SimpleUnpacker::on_reset()
{
    position(0);
    return *this;
}

}
