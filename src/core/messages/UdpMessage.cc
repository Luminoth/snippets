#include "src/pch.h"
#include "src/core/network/BufferedSender.h"
#include "UdpMessage.h"

namespace energonsoftware {

const unsigned int UdpMessage::MAX_CHUNKS = 99;
const unsigned int UdpMessage::MAX_TTL = 99;

// NOTE: if the MAX_* values change, these will have to change as well as the digit counts in chunk_header()
const size_t UdpMessage::HEADER_LEN = 10;
const std::string UdpMessage::REGEX("^([[:digit:]]{4})([[:digit:]]{2})([[:digit:]]{2})([[:digit:]]{2}).*$");

UdpMessage::UdpMessage(const Socket::BufferType* packet, size_t len, unsigned int packetid, unsigned int mtu, bool encode, unsigned int ttl) throw(std::runtime_error)
    : BufferedMessage(encode), _packet(), _len(len), _packetid(packetid),
        _mtu(mtu), _ttl(ttl), _chunkcount(0)
{
    if(_packetid > BufferedSender::MAX_PACKET_ID) {
        throw std::runtime_error("Packet ids cannot be more than 4 digits");
    }

    if(_mtu < (HEADER_LEN << 1)) {
        throw std::runtime_error("MTU is too small!");
    }

    _packet.reset(new Socket::BufferType[_len], std::default_delete<Socket::BufferType[]>());
    std::memmove(_packet.get(), packet, _len);

    calculate_chunkcounts();
}

UdpMessage::UdpMessage(const UdpMessage& message)
    : BufferedMessage(message), _packet(), _len(message._len), _packetid(message._packetid),
        _mtu(message._mtu), _ttl(message._ttl), _chunkcount(message._chunkcount)
{
    _packet.reset(new Socket::BufferType[_len], std::default_delete<Socket::BufferType[]>());
    std::memmove(_packet.get(), message._packet.get(), _len);
}

UdpMessage::~UdpMessage() noexcept
{
}

bool UdpMessage::chunks(std::vector<UdpMessageChunk>& chunks) const
{
    // the largest a chunk can be (minus the header)
    const size_t max_chunk_size = _mtu - HEADER_LEN;
    chunks.clear();

    unsigned int last_chunk = 0;
    while(last_chunk < chunk_count()) {
        std::string header(chunk_header(last_chunk + 1, chunk_count()));

        // the size that this chunk will be (minus the header)
        size_t size = std::min(max_chunk_size, data_len() - (last_chunk * max_chunk_size));

        // where in the data to start
        unsigned int position = last_chunk * max_chunk_size;

        UdpMessageChunk chunk;
        chunk.first = size + HEADER_LEN;    // size of the chunk (including header)
        chunk.second.reset(new Socket::BufferType[chunk.first], std::default_delete<Socket::BufferType[]>());
        std::memcpy(chunk.second.get(), header.c_str(), HEADER_LEN);
        std::memcpy(chunk.second.get() + HEADER_LEN, &(_packet.get()[position]), size);

        chunks.push_back(chunk);
        last_chunk++;
    }

    return true;
}

std::string UdpMessage::chunk_header(unsigned int chunknum, unsigned int chunkcount) const
{
    if(/*chunknum < 0 ||*/ chunknum > MAX_CHUNKS
        /*|| chunkcount < 0*/ || chunkcount > MAX_CHUNKS
        /*|| _ttl < 0*/ || _ttl > MAX_TTL)
    {
        return "";
    }

    char buffer[HEADER_LEN+1];
    ZeroMemory(buffer, HEADER_LEN+1);
#if defined WIN32
    _snprintf_s(buffer, HEADER_LEN+1, HEADER_LEN, "%04d%02d%02d%02d", _packetid, chunknum, chunkcount, _ttl);
#else
    snprintf(buffer, HEADER_LEN+1, "%04d%02d%02d%02d", _packetid, chunknum, chunkcount, _ttl);
#endif

    return buffer;
}

void UdpMessage::calculate_chunkcounts()
{
    _chunkcount = 0;

    unsigned int position = 0;
    while(position < data_len()) {
        _chunkcount += 1;
        position += (_mtu - HEADER_LEN);
    }
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"
#include "src/core/network/Socket.h"
#include "UdpMessageFactory.h"

class UdpMessageTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(UdpMessageTest);
        CPPUNIT_TEST(test_small_message);
        CPPUNIT_TEST(test_large_message);
        CPPUNIT_TEST_EXCEPTION(test_invalid_packetid, std::runtime_error);
        CPPUNIT_TEST_EXCEPTION(test_invalid_mtu, std::runtime_error);
    CPPUNIT_TEST_SUITE_END();

private:
    energonsoftware::UdpMessageFactory _factory;

public:
    UdpMessageTest() : CppUnit::TestFixture(), _factory() {}
    virtual ~UdpMessageTest() noexcept {}

private:
    void test_chunks(energonsoftware::UdpMessage& message)
    {
        // get the message chunks
        std::vector<energonsoftware::UdpMessage::UdpMessageChunk> chunks;
        message.chunks(chunks);

        // randomize those chunks
        std::random_shuffle(chunks.begin(), chunks.end());

        // rebuild the message
        for(energonsoftware::UdpMessage::UdpMessageChunk& chunk : chunks) {
            std::shared_ptr<energonsoftware::ClientSocket> socket(new energonsoftware::ClientSocket());
            CPPUNIT_ASSERT(_factory.append(chunk, socket));
        }

        std::vector<std::shared_ptr<energonsoftware::UdpMessageFactory::FactoryMessage> > completed;
        _factory.complete(completed);
        CPPUNIT_ASSERT(completed.size() == 1);

        std::shared_ptr<energonsoftware::UdpMessageFactory::FactoryMessage> result(completed[0]);
        CPPUNIT_ASSERT(result->complete());
        CPPUNIT_ASSERT(!std::memcmp(message.start(), result->message(), std::min(message.full_len(), result->message_len())));
    }

public:
    void test_small_message()
    {
        std::string packet("this is a small message");
        energonsoftware::UdpMessage message(reinterpret_cast<const energonsoftware::Socket::BufferType*>(packet.c_str()), packet.length(), 1, 512, true);
        test_chunks(message);
    }

    void test_large_message()
    {
        std::stringstream scratch;
        scratch << 0;
        for(int i=1; i<1024; ++i) {
            scratch << "," << i;
        }

        std::string packet(scratch.str());
        energonsoftware::UdpMessage message(reinterpret_cast<const energonsoftware::Socket::BufferType*>(packet.c_str()), packet.length(), 2, 512, false);
        test_chunks(message);
    }

    void test_invalid_packetid()
    {
        std::string packet("invalid packetid");
        energonsoftware::UdpMessage message(reinterpret_cast<const energonsoftware::Socket::BufferType*>(packet.c_str()),
            packet.length(), energonsoftware::BufferedSender::MAX_PACKET_ID * 10, 512, true);
        test_chunks(message);
    }

    void test_invalid_mtu()
    {
        std::string packet("invalid mtu");
        energonsoftware::UdpMessage message(reinterpret_cast<const energonsoftware::Socket::BufferType*>(packet.c_str()),
            packet.length(), 3, energonsoftware::UdpMessage::HEADER_LEN, true);
        test_chunks(message);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UdpMessageTest);

#endif
