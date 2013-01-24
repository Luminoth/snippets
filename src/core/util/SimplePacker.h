#if !defined __SIMPLEPACKER_H__
#define __SIMPLEPACKER_H__

#include "Packer.h"

namespace energonsoftware {

// NOTE: this class is not endian-safe
class SimplePacker : public Packer
{
public:
    SimplePacker();
    virtual ~SimplePacker() throw();

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
    DISALLOW_COPY_AND_ASSIGN(SimplePacker);
};

class SimpleUnpacker : public Unpacker
{
public:
    explicit SimpleUnpacker(const std::string& obj);
    explicit SimpleUnpacker(const std::vector<unsigned char>& obj);
    SimpleUnpacker(const unsigned char* obj, size_t len);
    virtual ~SimpleUnpacker() throw();

public:
    virtual unsigned int position() /*const*/ override { return _buffer.tellg(); }
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
    virtual bool done() const override { return _buffer.eof(); }

private:
    virtual Unpacker& on_reset();

private:
    std::stringstream _buffer;

private:
    SimpleUnpacker();
    DISALLOW_COPY_AND_ASSIGN(SimpleUnpacker);
};

}

#endif
