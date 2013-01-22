#include "src/pch.h"
#include "XmlDocument.h"

namespace energonsoftware {

XmlDocument::XmlDocument(const XmlString& name)
    : _root(name)
{
}

XmlDocument::XmlDocument(const XmlNode& root)
    : _root(root)
{
}

XmlDocument::XmlDocument(const XmlDocument& rhs)
    : _root(rhs._root)
{
}

XmlDocument::~XmlDocument() throw()
{
}

XmlString XmlDocument::document() const
{
    std::stringstream str;
    str << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" << _root;
    return XmlString(str.str());
}

XmlDocument& XmlDocument::operator=(const XmlDocument& rhs)
{
    _root = rhs._root;
    return *this;
}

}
