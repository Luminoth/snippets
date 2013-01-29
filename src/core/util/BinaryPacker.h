#if !defined __BINARYPACKER_H__
#define __BINARYPACKER_H__

#include "Packer.h"

namespace energonsoftware {

// uses XDR standard for packing
class BinaryPacker : public Packer
{
public:
    BinaryPacker();
    virtual ~BinaryPacker() throw();

public:
    virtual Packer& reset() override;
    virtual Packer& pack(const std::string& v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(const char* const v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(int8_t v, const std::string& name) throw(PackerError) override;
    virtual Packer& pack(uint8_t v, const std::string& name) throw(PackerError) override;
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
    DISALLOW_COPY_AND_ASSIGN(BinaryPacker);
};

class BinaryUnpacker : public Unpacker
{
public:
    explicit BinaryUnpacker(const std::string& obj);
    explicit BinaryUnpacker(const std::vector<unsigned char>& obj);
    BinaryUnpacker(const unsigned char* obj, size_t len);
    virtual ~BinaryUnpacker() throw();

public:
    virtual unsigned int position() /*const*/ override { return _buffer.tellg(); }
    virtual Unpacker& skip(unsigned int count) throw(PackerError) override;
    virtual Unpacker& position(unsigned int position) throw(PackerError) override;
    virtual Unpacker& unpack(std::string& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(int8_t& v, const std::string& name) throw(PackerError) override;
    virtual Unpacker& unpack(uint8_t& v, const std::string& name) throw(PackerError) override;
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
    BinaryUnpacker();
    DISALLOW_COPY_AND_ASSIGN(BinaryUnpacker);
};

}

#endif
