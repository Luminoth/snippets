#if !defined __XMLPACKER_H__
#define __XMLPACKER_H__

#include "Packer.h"

namespace energonsoftware {

class XmlPacker : public Packer
{
public:
    XmlPacker();
    virtual ~XmlPacker() throw();

public:
    virtual Packer& reset() override;
    virtual Packer& pack(const std::string& v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(const char* const v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(int32_t v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(uint32_t v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(int64_t v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(uint64_t v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(float v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(double v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(bool v, const std::string& name) throw(PackerError) override;
    virtual const std::string buffer() const override { return _buffer.str(); }

private:
    std::stringstream _buffer;

private:
    DISALLOW_COPY_AND_ASSIGN(XmlPacker);
};

class XmlUnpacker : public Unpacker
{
public:
    explicit XmlUnpacker(const std::string& obj);
    explicit XmlUnpacker(const std::vector<unsigned char>& obj);
    XmlUnpacker(const unsigned char* obj, size_t len);
    virtual ~XmlUnpacker() throw();

public:
    virtual unsigned int position() /*const*/ override { return _position; }
    virtual Unpacker& skip(unsigned int count) throw(PackerError) override;
    virtual Unpacker& position(unsigned int position) throw(PackerError) override;
    virtual Unpacker& unpack(std::string& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(int32_t& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(uint32_t& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(int64_t& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(uint64_t& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(float& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(double& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(bool& v, const std::string& name) throw(PackerError) override;
    virtual bool done() const override;

private:
    virtual Unpacker& on_reset();

private:
    void whitespace();
    void match_character(char expected) throw(PackerError);
    void match_name(const std::string& expected) throw(PackerError);
    void next_value(const std::string& name, std::string& value) throw(PackerError);

private:
    unsigned int _position;

private:
    XmlUnpacker();
    DISALLOW_COPY_AND_ASSIGN(XmlUnpacker);
};

}

#endif
