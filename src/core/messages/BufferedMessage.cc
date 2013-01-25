#include "src/pch.h"
#include "BufferedMessage.h"

namespace energonsoftware {

BufferedMessage::BufferedMessage(bool encode)
    : _encode(encode), _data(), _data_ptr(nullptr), _data_len(0), _seqid(-1)
{
}

BufferedMessage::BufferedMessage(const BufferedMessage& message)
    : _encode(message._encode), _data(), _data_ptr(nullptr), _data_len(0), _seqid(message._seqid)
{
}

BufferedMessage::~BufferedMessage() throw()
{
}

void BufferedMessage::reset()
{
    // have to call data() first to make sure some
    // messages have a chance to get their data
    const unsigned char* d = data();
    _data_len = data_len();

    // copy the current data
    _data.reset(new unsigned char[_data_len]);
    std::memcpy(_data.get(), d, _data_len);

    // set the data pointer
    _data_ptr = _data.get();
}

void BufferedMessage::advance(size_t len)
{
    _data_ptr += len;
}

bool BufferedMessage::finished() const
{
    return (_data_ptr - _data.get()) >= static_cast<int>(_data_len);
}

BufferedMessage& BufferedMessage::operator=(const BufferedMessage& rhs)
{
    _encode = rhs._encode;
    _data.reset();
    _data_ptr = nullptr;
    _data_len = 0;
    _seqid = rhs._seqid;

    return *this;
}

}
