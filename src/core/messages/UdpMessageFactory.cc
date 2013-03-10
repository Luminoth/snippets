#include "src/pch.h"
#include <boost/circular_buffer.hpp>
#include <boost/regex.hpp>
#include "src/core/util/util.h"
#include "UdpMessageFactory.h"

namespace energonsoftware {

Logger& UdpMessageFactory::logger(Logger::instance("energonsoftware.core.messages.UdpMessageFactory"));

UdpMessageFactory::FactoryMessage::FactoryMessage(unsigned int packetid, unsigned int chunkcount, unsigned int ttl, std::shared_ptr<ClientSocket> socket)
    : _packetid(packetid), _chunkcount(chunkcount), _ttl(ttl), _socket(socket),
        _chunks(), _last_chunk_time(0.0), _message(), _len(0)
{
}

bool UdpMessageFactory::FactoryMessage::expired() const
{
    return !complete() && _last_chunk_time > 0.0 && get_time() > (_last_chunk_time + _ttl);
}

const Socket::BufferType* UdpMessageFactory::FactoryMessage::message() const
{
    if(!complete()) {
        return nullptr;
    }
    return _message.get();
}

void UdpMessageFactory::FactoryMessage::build_message()
{
    boost::circular_buffer<unsigned char> message(MAX_BUFFER * 10);
    for(unsigned int i=1; i<total_chunks()+1; ++i) {
        if(_chunks.find(i) != _chunks.end()) {
            Socket::BufferType* scratch = _chunks[i].second.get() + UdpMessage::HEADER_LEN;
            message.insert(message.end(), scratch, scratch + _chunks[i].first - UdpMessage::HEADER_LEN);
        }
    }

    _len = message.size();
    _message.reset(new Socket::BufferType[_len]);
    std::copy(message.begin(), message.end(), _message.get());
}

void UdpMessageFactory::FactoryMessage::set(unsigned int k, UdpMessage::UdpMessageChunk& v)
{
    _chunks[k] = v;
    _last_chunk_time = get_time();
}

UdpMessageFactory::UdpMessageFactory()
    : _messages()
{
}

bool UdpMessageFactory::append(UdpMessage::UdpMessageChunk& chunk, std::shared_ptr<ClientSocket> socket)
{
    // copy the chunk into a string
    std::string scratch(reinterpret_cast<char*>(chunk.second.get()), chunk.first);

    // regex the values out of it
    boost::regex expression(UdpMessage::REGEX, boost::regex_constants::icase | boost::regex_constants::mod_s);
    boost::match_results<std::string::const_iterator> what;
    if(!regex_search(scratch, what, expression)) {
        LOG_WARNING("Could not find regular expression: " << UdpMessage::REGEX << "\n");
        return false;
    }

    int packetid = std::atoi(what[1].str().c_str());
    int chunknum = std::atoi(what[2].str().c_str());
    int chunkcount = std::atoi(what[3].str().c_str());
    int ttl = std::atoi(what[4].str().c_str());

    // TODO: validate all of this shit before blindly saving it

    // create or find the message to append to
    std::shared_ptr<FactoryMessage> message(new FactoryMessage(packetid, chunkcount, ttl, socket));
    if(_messages.find(packetid) != _messages.end()) {
        message = _messages[packetid];
    }
    _messages[message->packetid()] = message;

    // yeah you done fucked up buddy
    if(message->complete()) {
        LOG_WARNING("Message is already complete!\n");
        return false;
    }

    message->set(chunknum, chunk);
    if(message->complete()) {
        message->build_message();
    }
    return true;
}

void UdpMessageFactory::complete(std::vector<std::shared_ptr<FactoryMessage> >& completed)
{
    for(const FactoryMessageMap::value_type& message : _messages) {
        if(message.second->complete()) {
            completed.push_back(message.second);
        }
    }

    for(const std::shared_ptr<FactoryMessage>& message : completed) {
        _messages.erase(message->packetid());
    }
}

void UdpMessageFactory::expired(std::vector<std::shared_ptr<FactoryMessage> >& expired)
{
    for(const FactoryMessageMap::value_type& message : _messages) {
        if(message.second->expired()) {
            expired.push_back(message.second);
        }
    }

    for(const std::shared_ptr<FactoryMessage>& message : expired) {
        _messages.erase(message->packetid());
    }
}

}
