#if !defined __UDPMESSAGE_H__
#define __UDPMESSAGE_H__

#include "src/core/network/Socket.h"
#include "BufferedMessage.h"

namespace energonsoftware {

// packet format: packetid | chunknum | chunkcount | ttl | message
class UdpMessage : public BufferedMessage
{
public:
    typedef std::pair<size_t, boost::shared_array<Socket::BufferType> > UdpMessageChunk;

public:
    static const unsigned int MAX_PACKET_ID;
    static const unsigned int MAX_CHUNKS;
    static const unsigned int MAX_TTL;
    static const size_t HEADER_LEN;
    static const std::string REGEX;

public:
    UdpMessage(const Socket::BufferType* packet, size_t len, unsigned int packetid, unsigned int mtu, bool encode, unsigned int ttl=1) throw(std::runtime_error);

    // NOTE: this is slow because it has to copy the packet
    UdpMessage(const UdpMessage& message);

    virtual ~UdpMessage() throw();

public:
    unsigned int packetid() const { return _packetid; }
    unsigned int chunk_count() const { return _chunkcount; }
    bool chunks(std::vector<UdpMessageChunk>& chunks) const;
    virtual BufferedMessageType msg_type() const { return BufferedMessageType::Udp; }

    // NOTE: this is slow because it has to copy the packet
    UdpMessage& operator=(const UdpMessage& rhs);

private:
    virtual const unsigned char* data() /*const*/ { return reinterpret_cast<unsigned char*>(_packet.get()); }
    virtual size_t data_len() const { return _len; }

private:
    std::string chunk_header(unsigned int chunknum, unsigned int chunkcount) const;
    void calculate_chunkcounts();

private:
    boost::shared_array<Socket::BufferType> _packet;
    size_t _len;
    unsigned int _packetid;
    unsigned int _mtu;
    unsigned int _ttl;
    unsigned int _chunkcount;

private:
    UdpMessage();
};

}

#endif
