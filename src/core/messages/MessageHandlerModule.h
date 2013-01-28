#if !defined __MESSAGEHANDLERMODULE_H__
#define __MESSAGEHANDLERMODULE_H__

#include "BinaryMessage.h"
#include "BufferedMessage.h"
#include "MessageHandler.h"

namespace energonsoftware {

class MessageHandlerModuleError : public MessageHandlerError
{
public:
    explicit MessageHandlerModuleError(const std::string& what) throw() : MessageHandlerError(what) {}
    virtual ~MessageHandlerModuleError() throw() {}
};

class BufferedSender;

class MessageHandlerModule
{
public:
    MessageHandlerModule() {}
    virtual ~MessageHandlerModule() throw() {}

public:
    virtual BufferedMessageType msg_type() const = 0;

    virtual bool handle_message(const BufferedMessage& message, MessageHandler& handler, BufferedSender* session=nullptr) throw(MessageHandlerModuleError)
    { return false; }

    virtual void register_subhandlers(MessageHandler& handler) {}

private:
    DISALLOW_COPY_AND_ASSIGN(MessageHandlerModule);
};

class XmlMessageHandlerModule : public MessageHandlerModule
{
public:
    XmlMessageHandlerModule() : MessageHandlerModule() {}
    virtual ~XmlMessageHandlerModule() throw() {}

public:
    virtual BufferedMessageType msg_type() const { return BufferedMessageType::Xml; }
    virtual std::string xml_type() const = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(XmlMessageHandlerModule);
};

class BinaryMessageHandlerModule : public MessageHandlerModule
{
public:
    BinaryMessageHandlerModule() : MessageHandlerModule() {}
    virtual ~BinaryMessageHandlerModule() throw() {}

public:
    virtual BufferedMessageType msg_type() const { return BufferedMessageType::Binary; }
    virtual uint32_t bin_type() const = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(BinaryMessageHandlerModule);
};

}

#if defined WITH_UNIT_TESTS
class TestMessageHandlerModule : public energonsoftware::XmlMessageHandlerModule
{
public:
    TestMessageHandlerModule() : energonsoftware::XmlMessageHandlerModule() {}
    virtual ~TestMessageHandlerModule() throw() {}

public:
    virtual std::string xml_type() const { return "test"; }

    virtual bool handle_message(const energonsoftware::BufferedMessage& message, energonsoftware::MessageHandler& handler,
        energonsoftware::BufferedSender* session=nullptr) throw(energonsoftware::MessageHandlerModuleError)
    { return true; }

private:
    DISALLOW_COPY_AND_ASSIGN(TestMessageHandlerModule);
};
#endif

#endif
