#if !defined __UDPMESSAGEFACTORY_H__
#define __UDPMESSAGEFACTORY_H__

#include "src/core/network/Socket.h"
#include "UdpMessage.h"

namespace energonsoftware {

class ClientSocket;

class UdpMessageFactory
{
private:
    static Logger& logger;

public:
    class FactoryMessage
    {
    public:
        FactoryMessage(unsigned int packetid, unsigned int chunkcount, unsigned int ttl, std::shared_ptr<ClientSocket> socket);
        virtual ~FactoryMessage() noexcept {}

    public:
        unsigned int packetid() const { return _packetid; }
        std::shared_ptr<ClientSocket> socket() const { return _socket; }

        size_t chunk_count() const { return _chunks.size(); }
        unsigned int total_chunks() const { return _chunkcount; }
        bool expired() const;
        bool complete() const { return total_chunks() > 0 && chunk_count() >= total_chunks(); }

        const Socket::BufferType* message() const;
        size_t message_len() const { return _len; }

        void build_message();

        void set(unsigned int k, UdpMessage::UdpMessageChunk& v);

    private:
        unsigned int _packetid;
        unsigned int _chunkcount;
        unsigned int _ttl;
        std::shared_ptr<ClientSocket> _socket;

        std::unordered_map<unsigned int, UdpMessage::UdpMessageChunk> _chunks;
        double _last_chunk_time;

        std::shared_ptr<Socket::BufferType> _message;
        size_t _len;

    private:
        FactoryMessage();
        DISALLOW_COPY_AND_ASSIGN(FactoryMessage);
    };

private:
    typedef std::unordered_map<unsigned int, std::shared_ptr<FactoryMessage> > FactoryMessageMap;

public:
    UdpMessageFactory();
    virtual ~UdpMessageFactory() noexcept {}

public:
    bool append(UdpMessage::UdpMessageChunk& chunk, std::shared_ptr<ClientSocket> socket);

    // removes and returns all complete messages (and it's a slow process)
    void complete(std::vector<std::shared_ptr<FactoryMessage> >& completed);

    // removes and returns all expired messages (and it's a slow process)
    void expired(std::vector<std::shared_ptr<FactoryMessage> >& expired);

    bool contains(unsigned int k) const { return _messages.find(k) != _messages.end(); }
    FactoryMessage& get(unsigned int k) throw(std::out_of_range) { return *(_messages.at(k)); }

private:
    FactoryMessageMap _messages;

private:
    DISALLOW_COPY_AND_ASSIGN(UdpMessageFactory);
};

}

#endif
