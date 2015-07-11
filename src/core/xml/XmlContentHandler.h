#if !defined __XMLCONTENTHANDLER_H__
#define __XMLCONTENTHANDLER_H__

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

namespace energonsoftware {

class XmlDocument;
class XmlNode;

class XmlContentHandler final : public xercesc::DefaultHandler
{
private:
    static Logger& logger;

public:
    XmlContentHandler();
    virtual ~XmlContentHandler() noexcept;

public:
    void reset();
    bool complete() const { return _complete; }

    // NOTE: this does a deep copy of the parsed document
    // and the caller is responsible for freeing the memory
    XmlDocument* document() const;

public:
    /* NOTE: these may throw XmlDocumentParserError */

    virtual void startDocument() override;
    virtual void endDocument() override;
    virtual void startElement(const XMLCh* const uri, const XMLCh* const localname,
        const XMLCh* const qname, const xercesc::Attributes& attrs) override;
    virtual void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override;
    virtual void characters(const XMLCh* const chars, const XMLSize_t length) override;
    virtual void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length) override;
    virtual void skippedEntity(const XMLCh* const name) override;
    virtual void error(const xercesc::SAXParseException& exc) override;
    virtual void fatalError(const xercesc::SAXParseException& exc) override;
    virtual void warning(const xercesc::SAXParseException& exc) override;

private:
    bool _complete;
    std::unique_ptr<XmlNode> _root;
    XmlNode* _current;

private:
    DISALLOW_COPY_AND_ASSIGN(XmlContentHandler);
};

}

#endif
