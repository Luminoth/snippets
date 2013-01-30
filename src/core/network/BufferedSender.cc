#include "src/pch.h"
#include "src/core/messages/XmlMessage.h"
#include "BufferedSender.h"

namespace energonsoftware {

StringMessage::StringMessage(const std::string& message)
    : BufferedMessage(true), _message(), _len(message.length())
{
    unsigned char* scratch = new unsigned char[_len];
    std::memcpy(scratch, message.c_str(), _len);
    _message.reset(scratch);
}

StringMessage::StringMessage(const unsigned char* message, size_t len)
    : BufferedMessage(true), _message(), _len(len)
{
    unsigned char* scratch = new unsigned char[_len];
    std::memcpy(scratch, message, _len);
    _message.reset(scratch);
}

StringMessage::~StringMessage() throw()
{
}

const unsigned int BufferedSender::MAX_PACKET_ID = 9999;

Logger& BufferedSender::logger(Logger::instance("energonsoftware.core.network.BufferedSender"));

BufferedSender::BufferedSender()
    : _buffer(), _current(), _packet_count(0)
{
    reset_buffer();
}

BufferedSender::~BufferedSender() throw()
{
    reset_buffer();
}

void BufferedSender::reset_buffer()
{
    clear_buffer();
    clear_current();
}

const unsigned char* BufferedSender::current_buffer()
{
    pop_buffer();
    return _current ? _current->current() : nullptr;
}

size_t BufferedSender::current_buffer_len() const
{
    return _current ? _current->len() : -1;
}

bool BufferedSender::current_buffer_encoded() const
{
    return _current ? _current->encode() : false;
}

std::shared_ptr<BufferedMessage> BufferedSender::current_message()
{
    pop_buffer();
    return _current;
}

void BufferedSender::buffer(BufferedMessage* message)
{
    if(!message) return;
    _buffer.push(std::shared_ptr<BufferedMessage>(message));
}

unsigned long BufferedSender::next_packet_id()
{
    if(_packet_count >= MAX_PACKET_ID) {
        _packet_count = 0;
    }

    _packet_count++;
    return _packet_count;
}

void BufferedSender::update_sent(size_t sent)
{
    if(_current) {
        _current->advance(sent);
        if(_current->finished()) {
            clear_current();
        }
    }
}

void BufferedSender::clear_buffer()
{
    while(!_buffer.empty()) {
        _buffer.pop();
    }
}

void BufferedSender::clear_current()
{
    _current.reset();
}

void BufferedSender::pop_buffer()
{
    if(_current && _current->finished()) {
        clear_current();
    }

    if(nullptr == _current) {
        _current = _buffer.front();
        _buffer.pop();
        _current->reset();

        if(_current->encode()) {
            //LOG_DEBUG("Popped packet: " << _current->current() << "\n");
        }
    }
}

}

#if defined WITH_UNIT_TESTS
//#include <iostream>
#include "src/test/UnitTest.h"

class BufferedSenderTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(BufferedSenderTest);
        CPPUNIT_TEST(test_buffer);
        CPPUNIT_TEST(reset);
    CPPUNIT_TEST_SUITE_END();

private:
    void fill_buffer(energonsoftware::BufferedSender& sender)
    {
        static const unsigned char testunsigned[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

        sender.buffer(new energonsoftware::StringMessage(testunsigned, 8));
        sender.buffer(new energonsoftware::StringMessage("this is a test string to buffer"));

        CPPUNIT_ASSERT(!sender.buffer_empty());
    }

public:
    void test_buffer()
    {
        energonsoftware::BufferedSender sender;
        fill_buffer(sender);

        while(!sender.buffer_empty()) {
            const unsigned char* current = sender.current_buffer();
            CPPUNIT_ASSERT(nullptr != current);

            size_t len = sender.current_buffer_len();
            CPPUNIT_ASSERT(len > 0);

            /*std::cout << "current: '";
            std::cout.write(reinterpret_cast<const char*>(current), len);
            std::cout << "'" << std::endl
                << "len: " << len << std::endl;*/

            sender.update_sent(len > 5 ? 5 : len);
        }
    }

    void reset()
    {
        energonsoftware::BufferedSender sender;
        fill_buffer(sender);

        sender.reset_buffer();
        CPPUNIT_ASSERT(sender.buffer_empty());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BufferedSenderTest);

#endif
