#if !defined __XMLCONTENTHANDLER_H__
#define __XMLCONTENTHANDLER_H__

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include "XmlUtil.h"

namespace energonsoftware {

class XmlDocument;
class XmlNode;

class XmlContentHandler final : public xercesc::DefaultHandler
{
private:
    static Logger& logger;

public:
    XmlContentHandler();
    virtual ~XmlContentHandler() throw();

public:
    void reset();
    bool complete() const { return _complete; }

    // NOTE: this does a deep copy of the parsed document
    // and the caller is responsible for freeing the memory
    XmlDocument* document() const;

public:
    /* NOTE: these may throw XmlDocumentParserError */

    virtual void startDocument();
    virtual void endDocument();
    virtual void startElement(const XMLCh* const uri, const XMLCh* const localname,
        const XMLCh* const qname, const xercesc::Attributes& attrs);
    virtual void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname);
    virtual void characters(const XMLCh* const chars, const XMLSize_t length);
    virtual void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length);
    virtual void skippedEntity(const XMLCh* const name);
    virtual void error(const xercesc::SAXParseException& exc);
    virtual void fatalError(const xercesc::SAXParseException& exc);
    virtual void warning(const xercesc::SAXParseException& exc);

private:
    bool _complete;
    std::unique_ptr<XmlNode> _root;
    XmlNode* _current;

private:
    DISALLOW_COPY_AND_ASSIGN(XmlContentHandler);
};

}

#endif
