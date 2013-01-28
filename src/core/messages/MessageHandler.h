#if !defined __MESSAGEHANDLER_H__
#define __MESSAGEHANDLER_H__

#include "BinaryMessage.h"

namespace energonsoftware {

class MessageHandlerError : public std::exception
{
public:
    explicit MessageHandlerError(const std::string& what) throw() : _what(what) { }
    virtual ~MessageHandlerError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

protected:
    std::string _what;
};

class BufferedSender;
class BufferedMessage;
class MessageHandlerModule;

class MessageHandler
{
public:
    typedef std::unordered_map<std::string, std::shared_ptr<MessageHandlerModule> > XmlMessageHandlerMap;
    typedef std::unordered_map<uint32_t, std::shared_ptr<MessageHandlerModule> > BinaryMessageHandlerMap;

private:
    static Logger& logger;

public:
    MessageHandler();
    virtual ~MessageHandler() throw();

public:
    // module needs to have created with new
    std::shared_ptr<MessageHandlerModule> register_handler(MessageHandlerModule* module);

    MessageHandlerModule& handler(const std::string& type) throw(MessageHandlerError);
    MessageHandlerModule& handler(uint32_t type) throw(MessageHandlerError);

    bool handle_message(const BufferedMessage& message, BufferedSender* session=nullptr) throw(MessageHandlerError);

private:
    XmlMessageHandlerMap _xml_handlers;
    BinaryMessageHandlerMap _bin_handlers;

private:
    DISALLOW_COPY_AND_ASSIGN(MessageHandler);
};

}

#endif
