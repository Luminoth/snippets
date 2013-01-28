#include "src/pch.h"
#include "src/core/xml/XmlNode.h"
#include "MessageHandler.h"
#include "MessageHandlerModule.h"
#include "XmlMessage.h"
#include "BinaryMessage.h"

namespace energonsoftware {

Logger& MessageHandler::logger(Logger::instance("energonsoftware.core.messages.MessageHandler"));

MessageHandler::MessageHandler()
    : _xml_handlers(), _bin_handlers()
{
}

MessageHandler::~MessageHandler() throw()
{
}

std::shared_ptr<MessageHandlerModule> MessageHandler::register_handler(MessageHandlerModule* module)
{
    if(!module) {
        LOG_WARNING("Registering null handler!\n");
        return std::shared_ptr<MessageHandlerModule>();
    }

    std::shared_ptr<MessageHandlerModule> container(module);
    std::shared_ptr<MessageHandlerModule> oldmodule;
    switch(container->msg_type())
    {
    case BufferedMessageType::Xml:
        {
            XmlMessageHandlerModule* xml_module = dynamic_cast<XmlMessageHandlerModule*>(module);
            if(!xml_module) {
                LOG_ERROR("Could not cast module!\n");
                return std::shared_ptr<MessageHandlerModule>();
            }

            LOG_DEBUG("Registering handler for xml type: " << xml_module->xml_type() << "\n");
            oldmodule = _xml_handlers[xml_module->xml_type()] = container;
        }
        break;
    case BufferedMessageType::Binary:
        {
            BinaryMessageHandlerModule* bin_module = dynamic_cast<BinaryMessageHandlerModule*>(module);
            if(!bin_module) {
                LOG_ERROR("Could not cast module!\n");
                return std::shared_ptr<MessageHandlerModule>();
            }

            LOG_DEBUG("Registering handler for binary type: " << bin_module->bin_type() << "\n");
            oldmodule = _bin_handlers[bin_module->bin_type()] = container;
        }
        break;
    default:
        LOG_CRITICAL("Unhandled message type: " << container->msg_type() << "\n");
    }

    if(oldmodule.get() != container.get()) {
        LOG_WARNING("Handler was overwritten!\n");
    }

    container->register_subhandlers(*this);
    return container;
}

MessageHandlerModule& MessageHandler::handler(const std::string& type) throw(MessageHandlerError)
{
    try {
        std::shared_ptr<MessageHandlerModule> handler(_xml_handlers.at(type));
        if(!handler) throw MessageHandlerError("Handler is null!");

        return *handler;
    } catch(const std::out_of_range&) {
        throw MessageHandlerError("No handler for type: " + type);
    }
}

MessageHandlerModule& MessageHandler::handler(uint32_t type) throw(MessageHandlerError)
{
    try {
        std::shared_ptr<MessageHandlerModule> handler(_bin_handlers.at(type));
        if(!handler) throw MessageHandlerError("Handler is null!");

        return *handler;
    } catch(const std::out_of_range&) {
        throw MessageHandlerError("No handler for type: " + type);
    }
}

bool MessageHandler::handle_message(const BufferedMessage& message, BufferedSender* session) throw(MessageHandlerError)
{
    switch(message.msg_type())
    {
    case BufferedMessageType::Xml:
    {
        const XmlMessage* xml_message = dynamic_cast<const XmlMessage*>(&message);
        if(!xml_message) {
            LOG_CRITICAL("Could not cast message!\n");
            return false;
        }

        LOG_DEBUG("Attempting to handle xml message of type: " << xml_message->type().to_native() << "\n");
        MessageHandlerModule& handler = this->handler(xml_message->type().to_native());
        return handler.handle_message(message, *this, session);
    }
    case BufferedMessageType::Binary:
    {
        const BinaryMessage* bin_message = dynamic_cast<const BinaryMessage*>(&message);
        if(!bin_message) {
            LOG_CRITICAL("Could not cast message!\n");
            return false;
        }

        LOG_DEBUG("Attempting to handle binary message of type: " << bin_message->type() << "\n");
        MessageHandlerModule& handler = this->handler(bin_message->type());
        return handler.handle_message(message, *this, session);
    }
    default:
        LOG_CRITICAL("Unhandled message type: " << message.msg_type() << "\n");
    }

    return false;
}

}

/*#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class MessageHandlerTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(MessageHandlerTest);
        CPPUNIT_TEST(handle_message);
        CPPUNIT_TEST_EXCEPTION(no_handler, energonsoftware::MessageHandlerError);
        CPPUNIT_TEST_EXCEPTION(no_type, energonsoftware::MessageHandlerError);
    CPPUNIT_TEST_SUITE_END();

private:
    energonsoftware::MessageHandler _handler;

public:
    void setUp()
    {
        energonsoftware::XmlUtil::initialize();
        _handler.register_handler(new energonsoftware::TestMessageHandlerModule());
    }

    void tearDown()
    {
        energonsoftware::XmlUtil::cleanup();
    }

    void handle_message()
    {
        energonsoftware::XmlMessage message(energonsoftware::XmlString("message"));
        energonsoftware::XmlNode node(energonsoftware::XmlString("type"));
        node.add_data(energonsoftware::XmlString("test"));
        message.root().add_node(node);

        CPPUNIT_ASSERT(_handler.handle_message(message));
    }

    void no_handler()
    {
        energonsoftware::XmlMessage message(energonsoftware::XmlString("message"));
        energonsoftware::XmlNode node(energonsoftware::XmlString("type"));
        node.add_data(energonsoftware::XmlString("nohandler"));
        message.root().add_node(node);

        _handler.handle_message(message);
    }

    void no_type()
    {
        energonsoftware::XmlMessage message(energonsoftware::XmlString("message"));
        energonsoftware::XmlNode node(energonsoftware::XmlString("attr"));
        node.add_data(energonsoftware::XmlString("test"));
        message.root().add_node(node);

        _handler.handle_message(message);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MessageHandlerTest);

#endif*/
