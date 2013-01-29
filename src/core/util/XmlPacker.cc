#include "src/pch.h"
#include "XmlPacker.h"

// TODO: rewrite this using the xerces-based xml code

namespace energonsoftware {

XmlPacker::XmlPacker()
    : Packer(), _buffer()
{
}

XmlPacker::~XmlPacker() throw()
{
}

Packer& XmlPacker::reset()
{
    _buffer.str("");
    return *this;
}

Packer& XmlPacker::pack(const std::string& v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(const char* const v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(int8_t v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(uint8_t v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(int32_t v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(uint32_t v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(int64_t v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(uint64_t v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(float v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << std::fixed << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(double v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << std::fixed << v;
    _buffer << "</" << name << ">";
    return *this;
}

Packer& XmlPacker::pack(bool v, const std::string& name) throw(PackerError)
{
    _buffer << "<" << name << ">";
    _buffer << v;
    _buffer << "</" << name << ">";
    return *this;
}

XmlUnpacker::XmlUnpacker(const std::string& obj)
    : Unpacker(obj), _position(0)
{
}

XmlUnpacker::XmlUnpacker(const std::vector<unsigned char>& obj)
    : Unpacker(obj), _position(0)
{
}

XmlUnpacker::XmlUnpacker(const unsigned char* obj, size_t len)
    : Unpacker(obj, len), _position(0)
{
}

XmlUnpacker::~XmlUnpacker() throw()
{
}

Unpacker& XmlUnpacker::position(unsigned int position) throw(PackerError)
{
    if(position >= _obj.length()) {
        throw PackerError("Position is beyond the length of the buffer!");
    }

    _position = position;
    return *this;
}

Unpacker& XmlUnpacker::skip(unsigned int count) throw(PackerError)
{
    if(_position + count >= _obj.length()) {
        throw PackerError("Position is beyond the length of the buffer!");
    }

    _position += count;
    return *this;
}

Unpacker& XmlUnpacker::unpack(std::string& v, const std::string& name) throw(PackerError)
{
    next_value(name, v);
    return *this;
}

Unpacker& XmlUnpacker::unpack(int8_t& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = value[0];
    return *this;
}

Unpacker& XmlUnpacker::unpack(uint8_t& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = value[0];
    return *this;
}

Unpacker& XmlUnpacker::unpack(int32_t& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = std::atoi(value.c_str());
    return *this;
}

Unpacker& XmlUnpacker::unpack(uint32_t& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = std::atoi(value.c_str());
    return *this;
}

Unpacker& XmlUnpacker::unpack(int64_t& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = std::atol(value.c_str());
    return *this;
}

Unpacker& XmlUnpacker::unpack(uint64_t& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = std::atol(value.c_str());
    return *this;
}

Unpacker& XmlUnpacker::unpack(float& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = std::atof(value.c_str());
    return *this;
}

Unpacker& XmlUnpacker::unpack(double& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = std::atof(value.c_str());
    return *this;
}

Unpacker& XmlUnpacker::unpack(bool& v, const std::string& name) throw(PackerError)
{
    std::string value;
    next_value(name, value);
    v = value == "1";
    return *this;
}

bool XmlUnpacker::done() const
{
    return _position == _obj.length();
}

Unpacker& XmlUnpacker::on_reset()
{
    position(0);
    return *this;
}

void XmlUnpacker::whitespace()
{
    while(!done() && isspace(_obj[_position])) {
        _position++;
    }
}

void XmlUnpacker::match_character(char expected) throw(PackerError)
{
    if(done() || _obj[_position] != expected) {
        throw PackerError("Invalid XML!");
    }
    _position++;
}

void XmlUnpacker::match_name(const std::string& expected) throw(PackerError)
{
    std::string name(_obj.substr(_position, expected.length()));
    if(name != expected) {
        throw PackerError("Name mismatch: " + expected + ", got " + name);
    }
    _position += name.length();
}

void XmlUnpacker::next_value(const std::string& name, std::string& value) throw(PackerError)
{
    whitespace();
    match_character('<');
    match_name(name);
    match_character('>');

    size_t pos = _obj.find("<", _position);
    if(pos == std::string::npos) {
        throw PackerError("Invalid XML");
    }

    value = _obj.substr(_position, pos - _position);
    _position += value.length();

    match_character('<');
    match_character('/');
    match_name(name);
    match_character('>');
}

}
