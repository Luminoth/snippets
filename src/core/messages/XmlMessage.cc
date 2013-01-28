#include "src/pch.h"
#include "XmlMessage.h"

namespace energonsoftware {

XmlMessage::XmlMessage(const XmlString& name)
    : XmlDocument(name), BufferedMessage(true),
        _data()
{
}

XmlMessage::XmlMessage(const XmlDocument& document)
    : XmlDocument(document.root()), BufferedMessage(true),
        _data()
{
}

XmlMessage::XmlMessage(const XmlNode& root)
    : XmlDocument(root), BufferedMessage(true),
        _data()
{
}

XmlMessage::XmlMessage(const XmlMessage& message)
    : XmlDocument(message), BufferedMessage(message),
        _data()
{
    // NOTE: we don't need to copy the _data pointer
}

XmlMessage::~XmlMessage() throw()
{
}

XmlMessage& XmlMessage::operator=(const XmlMessage& rhs)
{
    XmlDocument::operator=(rhs);
    BufferedMessage::operator=(rhs);

    return *this;
}

const unsigned char* XmlMessage::data() /*const*/
{
    _data.reset(new std::string());
    _data->assign(document().to_native().c_str());

    return reinterpret_cast<const unsigned char*>(_data->c_str());
}

size_t XmlMessage::data_len() const
{
    return _data ? _data->length() : 0;
}

}
