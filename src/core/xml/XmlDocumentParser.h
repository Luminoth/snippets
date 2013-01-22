#if !defined __XMLDOCUMENTPARSER_H__
#define __XMLDOCUMENTPARSER_H__

#include <xercesc/sax2/SAX2XMLReader.hpp>

namespace energonsoftware {

class XmlDocumentParserError : public std::exception
{
public:
    explicit XmlDocumentParserError(const std::string& what) throw() : _what(what) { }
    virtual ~XmlDocumentParserError() throw() {}
    virtual const char* what() const throw() { return _what.c_str(); }

private:
    std::string _what;
};

class XmlElementMismatchError : public XmlDocumentParserError
{
public:
    XmlElementMismatchError(const std::string& expected, const std::string& received)
        : XmlDocumentParserError("Element mismatch: expected '" + expected = "', got '" + received + "'")
    {
    }

    virtual ~XmlElementMismatchError() throw() {}

private:
    XmlElementMismatchError(const std::string& what);
};

class XmlContentHandler;
class XmlDocument;
class XmlString;

class XmlDocumentParser final
{
private:
    static Logger& logger;

public:
    XmlDocumentParser();
    virtual ~XmlDocumentParser() throw();

public:
    bool parse(const boost::filesystem::path& filename) throw(XmlDocumentParserError);
    void feed(const XmlString& document) throw(XmlDocumentParserError);
    void reset();
    bool complete() const;

    // NOTE: the caller is responsible for freeing the document
    XmlDocument* document() const;

private:
    std::unique_ptr<xercesc::SAX2XMLReader> _parser;
    std::unique_ptr<XmlContentHandler> _handler;

private:
    DISALLOW_COPY_AND_ASSIGN(XmlDocumentParser);
};

}

#endif
