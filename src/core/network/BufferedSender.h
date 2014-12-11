#if !defined __BUFFEREDSENDER_H__
#define __BUFFEREDSENDER_H__

#include "src/core/messages/BufferedMessage.h"
#include "Socket.h"
#include "network_util.h"

namespace energonsoftware {

class StringMessage : public BufferedMessage
{
public:
    explicit StringMessage(const std::string& message);
    StringMessage(const unsigned char* message, size_t len);
    virtual ~StringMessage() noexcept;

public:
    virtual BufferedMessageType msg_type() const { return BufferedMessageType::String; }

private:
    virtual const unsigned char* data() /*const*/ { return reinterpret_cast<unsigned char*>(_message.get()); }
    virtual size_t data_len() const { return _len; }

private:
    std::shared_ptr<Socket::BufferType> _message;
    size_t _len;

private:
    DISALLOW_COPY_AND_ASSIGN(StringMessage);
};

class XmlMessage;

class BufferedSender
{
public:
    static const unsigned int MAX_PACKET_ID;

private:
    static Logger& logger;

public:
    BufferedSender();
    virtual ~BufferedSender() noexcept;

public:
    bool buffer_empty() const { return (nullptr == _current || _current->finished()) && _buffer.empty(); }

    void reset_buffer();
    const Socket::BufferType* current_buffer();
    size_t current_buffer_len() const;
    bool current_buffer_encoded() const;

    std::shared_ptr<BufferedMessage> current_message();

    // message should have been allocated with new
    void buffer(BufferedMessage* message);

    unsigned long next_packet_id();

    void update_sent(size_t sent);

protected:
    void clear_buffer();
    void clear_current();

private:
    void pop_buffer();

private:
    std::queue<std::shared_ptr<BufferedMessage> > _buffer;
    std::shared_ptr<BufferedMessage> _current;
    unsigned long _packet_count;

private:
    DISALLOW_COPY_AND_ASSIGN(BufferedSender);
};

}

#endif
