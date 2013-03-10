#if !defined __XMLMESSAGE_H__
#define __XMLMESSAGE_H__

#include "src/core/xml/XmlDocument.h"
#include "BufferedMessage.h"

namespace energonsoftware {

class XmlMessage : public XmlDocument, public BufferedMessage
{
public:
    explicit XmlMessage(const XmlString& name);
    explicit XmlMessage(const XmlDocument& document);
    explicit XmlMessage(const XmlNode& root);
    XmlMessage(const XmlMessage& message);
    virtual ~XmlMessage() noexcept;

public:
    XmlString type() const { return root().name(); }
    virtual BufferedMessageType msg_type() const { return BufferedMessageType::Xml; }

    XmlMessage& operator=(const XmlMessage& rhs);

private:
    virtual const unsigned char* data() /*const*/;
    virtual size_t data_len() const;

private:
    std::shared_ptr<std::string> _data;

private:
    XmlMessage();
};

}

#endif
