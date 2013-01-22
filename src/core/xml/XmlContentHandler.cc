#include "src/pch.h"
#include "XmlContentHandler.h"
#include "XmlDocumentParser.h"
#include "XmlDocument.h"
#include "XmlNode.h"

namespace energonsoftware {

Logger& XmlContentHandler::logger(Logger::instance("energonsoftware.core.xml.XmlContentHandler"));

XmlContentHandler::XmlContentHandler()
    : xercesc::DefaultHandler(), _complete(false), _root(nullptr), _current(nullptr)
{
}

XmlContentHandler::~XmlContentHandler() throw()
{
    reset();
}

void XmlContentHandler::reset()
{
    _root.reset();
    _current = nullptr;
    _complete = false;
}

XmlDocument* XmlContentHandler::document() const
{
    if(!complete() || nullptr == _root) {
        return nullptr;
    }
    return new XmlDocument(*_root);
}

void XmlContentHandler::startDocument()
{
    reset();
}

void XmlContentHandler::endDocument()
{
    _complete = true;
}

void XmlContentHandler::startElement(const XMLCh* const uri, const XMLCh* const localname,
    const XMLCh* const qname, const xercesc::Attributes& attrs)
{
    XmlString qn(qname);
    //LOG_DEBUG("start: " << qn.to_native() << "\n");

    XmlAttributeMap attributes;
    for(XMLSize_t i=0; i<attrs.getLength(); ++i) {
        attributes[XmlString(attrs.getQName(i))] = XmlString(attrs.getValue(i));
    }

    if(!_root) {
        if(nullptr != _current) {
            throw XmlDocumentParserError("Current element exists but head element does not!");
        }

        _root.reset(new XmlNode(qn, attributes));
        _current = _root.get();
    } else {
        if(nullptr != _current) {
            _current = &(_current->add_node(XmlNode(qn, attributes)));
        } else {
            throw XmlDocumentParserError("Current element is null!");
        }
    }
}

void XmlContentHandler::endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname)
{
    XmlString qn(qname);
    //LOG_DEBUG("end: " << qn.to_native() << "\n");

    if(nullptr == _current) {
        throw XmlDocumentParserError("Found end element with no starting element: " + qn.to_native());
    } else if(_current->name() != qn) {
        throw XmlElementMismatchError(_current->name().to_native(), qn.to_native());
    }

    _current = _current->parent();
    if(nullptr == _current) {
        if(_root->name() != qn) {
            LOG_WARNING("End of stack name mismatch: " << _root->name().to_native() << ", " << qn.to_native() << "\n");
        }
    }
}

void XmlContentHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
    XmlString str(chars/*, length*/);
    if(nullptr == _current) {
        throw XmlDocumentParserError("Characters found outside of element: " + str.to_native());
    }
    _current->add_data(str);
}

void XmlContentHandler::ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length)
{
    // TODO: do we need to do anything here?
}

void XmlContentHandler::skippedEntity(const XMLCh* const name)
{
    LOG_WARNING("Skipping entity: " << XmlString(name).to_native() << "\n");
}

void XmlContentHandler::error(const xercesc::SAXParseException& exc)
{
    std::string m(XmlString(exc.getMessage()).to_native());
    LOG_ERROR("Encountered an XML error: " << m << "\n");
    throw XmlDocumentParserError(m);
}

void XmlContentHandler::fatalError(const xercesc::SAXParseException& exc)
{
    std::string m(XmlString(exc.getMessage()).to_native());
    LOG_CRITICAL("Encountered a fatal XML error: " << m << "\n");
    throw XmlDocumentParserError(m);
}

void XmlContentHandler::warning(const xercesc::SAXParseException& exc)
{
    XmlString m(exc.getMessage());
    LOG_WARNING("Encountered an XML warning: " << m.to_native() << "\n");
}

}
