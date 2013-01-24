#include "src/pch.h"
#include "util.h"
#include "BinaryPacker.h"

namespace energonsoftware {

BinaryPacker::BinaryPacker()
    : Packer(), _buffer()
{
}

BinaryPacker::~BinaryPacker() throw()
{
}

Packer& BinaryPacker::reset()
{
    _buffer.str("");
    return *this;
}

Packer& BinaryPacker::pack(const std::string& v, const std::string& name) throw(PackerError)
{
    pack(static_cast<uint32_t>(v.length()), name + "_length");

    for(size_t i=0; i<v.length(); ++i) {
        _buffer.write(&v[i], 1);
    }

    // always multiple of 4 bytes
    size_t r=v.length() % 4;
    if(r > 0) {
        char padding=0;
        for(size_t i=r; i<4; ++i) {
            _buffer.write(&padding, 1);
        }
    }

    return *this;
}

Packer& BinaryPacker::pack(const char* const v, const std::string& name) throw(PackerError)
{
    uint32_t len = std::strlen(v);
    pack(len, name + "_length");

    for(uint32_t i=0; i<len; ++i) {
        _buffer.write(&v[i], 1);
    }

    // always multiple of 4 bytes
    size_t r=len % 4;
    if(r > 0) {
        char padding=0;
        for(size_t i=r; i<4; ++i) {
            _buffer.write(&padding, 1);
        }
    }

    return *this;
}

Packer& BinaryPacker::pack(int32_t v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    if(is_little_endian()) {
        _buffer.write(&bytes[3], 1);
        _buffer.write(&bytes[2], 1);
        _buffer.write(&bytes[1], 1);
        _buffer.write(&bytes[0], 1);
    } else {
        _buffer.write(&bytes[0], 1);
        _buffer.write(&bytes[1], 1);
        _buffer.write(&bytes[2], 1);
        _buffer.write(&bytes[3], 1);
    }
    return *this;
}

Packer& BinaryPacker::pack(uint32_t v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    if(is_little_endian()) {
        _buffer.write(&bytes[3], 1);
        _buffer.write(&bytes[2], 1);
        _buffer.write(&bytes[1], 1);
        _buffer.write(&bytes[0], 1);
    } else {
        _buffer.write(&bytes[0], 1);
        _buffer.write(&bytes[1], 1);
        _buffer.write(&bytes[2], 1);
        _buffer.write(&bytes[3], 1);
    }
    return *this;
}

Packer& BinaryPacker::pack(int64_t v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    if(is_little_endian()) {
        _buffer.write(&bytes[7], 1);
        _buffer.write(&bytes[6], 1);
        _buffer.write(&bytes[5], 1);
        _buffer.write(&bytes[4], 1);
        _buffer.write(&bytes[3], 1);
        _buffer.write(&bytes[2], 1);
        _buffer.write(&bytes[1], 1);
        _buffer.write(&bytes[0], 1);
    } else {
        _buffer.write(&bytes[0], 1);
        _buffer.write(&bytes[1], 1);
        _buffer.write(&bytes[2], 1);
        _buffer.write(&bytes[3], 1);
        _buffer.write(&bytes[4], 1);
        _buffer.write(&bytes[5], 1);
        _buffer.write(&bytes[6], 1);
        _buffer.write(&bytes[7], 1);
    }
    return *this;
}

Packer& BinaryPacker::pack(uint64_t v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    if(is_little_endian()) {
        _buffer.write(&bytes[7], 1);
        _buffer.write(&bytes[6], 1);
        _buffer.write(&bytes[5], 1);
        _buffer.write(&bytes[4], 1);
        _buffer.write(&bytes[3], 1);
        _buffer.write(&bytes[2], 1);
        _buffer.write(&bytes[1], 1);
        _buffer.write(&bytes[0], 1);
    } else {
        _buffer.write(&bytes[0], 1);
        _buffer.write(&bytes[1], 1);
        _buffer.write(&bytes[2], 1);
        _buffer.write(&bytes[3], 1);
        _buffer.write(&bytes[4], 1);
        _buffer.write(&bytes[5], 1);
        _buffer.write(&bytes[6], 1);
        _buffer.write(&bytes[7], 1);
    }
    return *this;
}

// NOTE: this may not always follow XDR format
Packer& BinaryPacker::pack(float v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    _buffer.write(&bytes[0], 1);
    _buffer.write(&bytes[1], 1);
    _buffer.write(&bytes[2], 1);
    _buffer.write(&bytes[3], 1);
    return *this;
}

// NOTE: this may not always follow XDR format
Packer& BinaryPacker::pack(double v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    _buffer.write(&bytes[0], 1);
    _buffer.write(&bytes[1], 1);
    _buffer.write(&bytes[2], 1);
    _buffer.write(&bytes[3], 1);
    _buffer.write(&bytes[4], 1);
    _buffer.write(&bytes[5], 1);
    _buffer.write(&bytes[6], 1);
    _buffer.write(&bytes[7], 1);
    return *this;
}

Packer& BinaryPacker::pack(bool v, const std::string& name) throw(PackerError)
{
    pack(static_cast<int>(v ? 1 : 0), name);
    return *this;
}

BinaryUnpacker::BinaryUnpacker(const std::string& obj)
    : Unpacker(obj), _buffer(obj)
{
}

BinaryUnpacker::BinaryUnpacker(const std::vector<unsigned char>& obj)
    : Unpacker(obj), _buffer()
{
    _buffer.str(_obj);
}

BinaryUnpacker::BinaryUnpacker(const unsigned char* obj, size_t len)
    : Unpacker(obj, len), _buffer()
{
    _buffer.str(_obj);
}

BinaryUnpacker::~BinaryUnpacker() throw()
{
}

Unpacker& BinaryUnpacker::position(unsigned int position) throw(PackerError)
{
    if(position >= _obj.length()) {
        throw PackerError("Position is beyond the length of the buffer!");
    }

    _buffer.seekg(position, std::ios_base::beg);
    return *this;
}

Unpacker& BinaryUnpacker::skip(unsigned int count) throw(PackerError)
{
    /*if(_buffer.tellg() + count >= _obj.length()) {
        throw PackerError("Position is beyond the length of the buffer!");
    }*/

    _buffer.ignore(count);
    return *this;
}

Unpacker& BinaryUnpacker::unpack(std::string& v, const std::string& name) throw(PackerError)
{
    uint32_t len;
    unpack(len, name + "_length");

    boost::shared_array<char> n(new char[len]);
    for(uint32_t i=0; i<len; ++i) {
        char byte;
        _buffer.read(&byte, 1);
        n.get()[i] = byte;
    }

    // read the padding
    size_t r=len % 4;
    if(r > 0) {
        for(size_t i=r; i<4; ++i) {
            char padding;
            _buffer.read(&padding, 1);
        }
    }

    v.assign(n.get(), len);
    return *this;
}

Unpacker& BinaryUnpacker::unpack(int32_t& v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    if(is_little_endian()) {
        _buffer.read(&bytes[3], 1);
        _buffer.read(&bytes[2], 1);
        _buffer.read(&bytes[1], 1);
        _buffer.read(&bytes[0], 1);
    } else {
        _buffer.read(&bytes[0], 1);
        _buffer.read(&bytes[1], 1);
        _buffer.read(&bytes[2], 1);
        _buffer.read(&bytes[3], 1);
    }
    return *this;
}

Unpacker& BinaryUnpacker::unpack(uint32_t& v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    if(is_little_endian()) {
        _buffer.read(&bytes[3], 1);
        _buffer.read(&bytes[2], 1);
        _buffer.read(&bytes[1], 1);
        _buffer.read(&bytes[0], 1);
    } else {
        _buffer.read(&bytes[0], 1);
        _buffer.read(&bytes[1], 1);
        _buffer.read(&bytes[2], 1);
        _buffer.read(&bytes[3], 1);
    }
    return *this;
}

Unpacker& BinaryUnpacker::unpack(int64_t& v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    if(is_little_endian()) {
        _buffer.read(&bytes[7], 1);
        _buffer.read(&bytes[6], 1);
        _buffer.read(&bytes[5], 1);
        _buffer.read(&bytes[4], 1);
        _buffer.read(&bytes[3], 1);
        _buffer.read(&bytes[2], 1);
        _buffer.read(&bytes[1], 1);
        _buffer.read(&bytes[0], 1);
    } else {
        _buffer.read(&bytes[0], 1);
        _buffer.read(&bytes[1], 1);
        _buffer.read(&bytes[2], 1);
        _buffer.read(&bytes[3], 1);
        _buffer.read(&bytes[4], 1);
        _buffer.read(&bytes[5], 1);
        _buffer.read(&bytes[6], 1);
        _buffer.read(&bytes[7], 1);
    }
    return *this;
}

Unpacker& BinaryUnpacker::unpack(uint64_t& v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    if(is_little_endian()) {
        _buffer.read(&bytes[7], 1);
        _buffer.read(&bytes[6], 1);
        _buffer.read(&bytes[5], 1);
        _buffer.read(&bytes[4], 1);
        _buffer.read(&bytes[3], 1);
        _buffer.read(&bytes[2], 1);
        _buffer.read(&bytes[1], 1);
        _buffer.read(&bytes[0], 1);
    } else {
        _buffer.read(&bytes[0], 1);
        _buffer.read(&bytes[1], 1);
        _buffer.read(&bytes[2], 1);
        _buffer.read(&bytes[3], 1);
        _buffer.read(&bytes[4], 1);
        _buffer.read(&bytes[5], 1);
        _buffer.read(&bytes[6], 1);
        _buffer.read(&bytes[7], 1);
    }
    return *this;
}

// NOTE: this may not always follow XDR format
Unpacker& BinaryUnpacker::unpack(float& v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    _buffer.read(&bytes[0], 1);
    _buffer.read(&bytes[1], 1);
    _buffer.read(&bytes[2], 1);
    _buffer.read(&bytes[3], 1);
    return *this;
}

// NOTE: this may not always follow XDR format
Unpacker& BinaryUnpacker::unpack(double& v, const std::string& name) throw(PackerError)
{
    char* bytes = reinterpret_cast<char*>(&v);
    _buffer.read(&bytes[0], 1);
    _buffer.read(&bytes[1], 1);
    _buffer.read(&bytes[2], 1);
    _buffer.read(&bytes[3], 1);
    _buffer.read(&bytes[4], 1);
    _buffer.read(&bytes[5], 1);
    _buffer.read(&bytes[6], 1);
    _buffer.read(&bytes[7], 1);
    return *this;
}

Unpacker& BinaryUnpacker::unpack(bool& v, const std::string& name) throw(PackerError)
{
    int b;
    unpack(b, name);
    v = static_cast<bool>(b);
    return *this;
}

Unpacker& BinaryUnpacker::on_reset()
{
    position(0);
    return *this;
}

}
