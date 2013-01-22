#include "src/pch.h"
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include "src/core/util/fs_util.h"
#include "XmlDocumentParser.h"
#include "XmlContentHandler.h"
#include "XmlDocument.h"

namespace energonsoftware {

Logger& XmlDocumentParser::logger(Logger::instance("energonsoftware.core.xml.XmlDocumentParser"));

XmlDocumentParser::XmlDocumentParser()
    : _parser(xercesc::XMLReaderFactory::createXMLReader()), _handler(new XmlContentHandler())
{
    _parser->setFeature(xercesc::XMLUni::fgSAX2CoreValidation, false);
    _parser->setContentHandler(_handler.get());
    _parser->setErrorHandler(_handler.get());
}

XmlDocumentParser::~XmlDocumentParser() throw()
{
}

bool XmlDocumentParser::parse(const boost::filesystem::path& filename) throw(XmlDocumentParserError)
{
    std::string contents;
    if(!file_to_string(filename, contents)) {
        return false;
    }

    feed(XmlString(contents));
    if(!complete()) {
        LOG_ERROR("Incomplete XML document!\n");
        return false;
    }

    return true;
}

void XmlDocumentParser::feed(const XmlString& document) throw(XmlDocumentParserError)
{
    // TODO: does making this native fuck up the encoding???
    std::string doc(document.to_native());
    xercesc::MemBufInputSource input(reinterpret_cast<const XMLByte*>(doc.c_str()), doc.length(), "energonsoftware");
    _parser->parse(input);
}

void XmlDocumentParser::reset()
{
    _handler->resetDocument();
    _handler->resetErrors();
    _handler->reset();
}

bool XmlDocumentParser::complete() const
{
    return _handler->complete();
}

XmlDocument* XmlDocumentParser::document() const
{
    return _handler->document();
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"
#include "src/core/util/util.h"
#include "XmlNode.h"

class XmlDocumentParserTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(XmlDocumentParserTest);
        CPPUNIT_TEST(parser);
        CPPUNIT_TEST(full);
        CPPUNIT_TEST(data);
        CPPUNIT_TEST(attributes);
        CPPUNIT_TEST(invalid_xml);
    CPPUNIT_TEST_SUITE_END();

private:
    static energonsoftware::Logger& logger;

public:
    XmlDocumentParserTest()
        : CppUnit::TestFixture(), _parser()
    {
    }

private:
    void parse(const energonsoftware::XmlDocument& document)
    {
        _parser->feed(document.document());
        CPPUNIT_ASSERT(_parser->complete());

        std::unique_ptr<energonsoftware::XmlDocument> parsed(_parser->document());
        CPPUNIT_ASSERT(parsed);

        LOG_INFO(parsed->document().to_native() << std::endl);
        CPPUNIT_ASSERT_EQUAL(document.document(), parsed->document());
    }

public:
    void setUp()
    {
        energonsoftware::XmlUtil::initialize();
        _parser.reset(new energonsoftware::XmlDocumentParser());
    }

    void tearDown()
    {
        _parser.reset();
        energonsoftware::XmlUtil::cleanup();
    }

    void parser()
    {
        _parser->feed(energonsoftware::XmlString("<?xml version=\"1.0\" encoding=\"ascii\" ?><test></test>"));
        CPPUNIT_ASSERT(_parser->complete());

        std::unique_ptr<energonsoftware::XmlDocument> parsed(_parser->document());
        CPPUNIT_ASSERT(parsed);

        _parser->reset();
        _parser->feed(energonsoftware::XmlString("<?xml version=\"1.0\" encoding=\"ascii\" ?><test></test>"));
        CPPUNIT_ASSERT(_parser->complete());

        parsed.reset(_parser->document());
        CPPUNIT_ASSERT(parsed);

        _parser->reset();
        _parser->feed(energonsoftware::XmlString("<?xml version=\"1.0\" encoding=\"ascii\" ?><test></test>"));
        CPPUNIT_ASSERT(_parser->complete());

        parsed.reset(_parser->document());
        CPPUNIT_ASSERT(parsed);
    }

    void full()
    {
        energonsoftware::XmlAttributeMap attr;
        attr[energonsoftware::XmlString("name")] = energonsoftware::XmlString("test");

        energonsoftware::XmlDocument document(energonsoftware::XmlNode(energonsoftware::XmlString("test_document"), energonsoftware::XmlString("test"), attr));
        parse(document);
    }

    void data()
    {
        energonsoftware::XmlDocument document1(energonsoftware::XmlNode(energonsoftware::XmlString("test_document"), energonsoftware::XmlString("test")));
        parse(document1);

        energonsoftware::XmlDocument document2(energonsoftware::XmlNode(energonsoftware::XmlString("test_document"), energonsoftware::XmlString("this is a 'test'")));
        parse(document2);

        energonsoftware::XmlDocument document3(energonsoftware::XmlNode(energonsoftware::XmlString("test_document"), energonsoftware::XmlString("this is a \"test\"")));
        parse(document3);

        energonsoftware::XmlDocument document4(energonsoftware::XmlNode(energonsoftware::XmlString("test_document"), energonsoftware::XmlString("<xml document />")));
        parse(document4);
    }

    void attributes()
    {
        energonsoftware::XmlAttributeMap attr;

        attr[energonsoftware::XmlString("name")] = energonsoftware::XmlString("test");
        energonsoftware::XmlDocument document1(energonsoftware::XmlNode(energonsoftware::XmlString("test_document"), attr));
        parse(document1);

        attr[energonsoftware::XmlString("name")] = energonsoftware::XmlString("this is a \"test\"");
        attr[energonsoftware::XmlString("value")] = energonsoftware::XmlString("this is another'test'");
        energonsoftware::XmlDocument document2(energonsoftware::XmlNode(energonsoftware::XmlString("test_document"), attr));
        parse(document2);

        attr.erase(energonsoftware::XmlString("name"));
        attr[energonsoftware::XmlString("value")] = energonsoftware::XmlString("<xml document />");
        energonsoftware::XmlDocument document3(energonsoftware::XmlNode(energonsoftware::XmlString("test_document"), attr));
        parse(document3);
    }

    void invalid_xml()
    {
        energonsoftware::XmlDocumentParser parser;
        CPPUNIT_ASSERT_THROW(_parser->feed(
            energonsoftware::XmlString("<?xml version='1.0' encoding='utf-8'?></test>")),
            energonsoftware::XmlDocumentParserError);

        _parser->reset();
        CPPUNIT_ASSERT_THROW(_parser->feed(
            energonsoftware::XmlString("<?xml version='1.0' encoding='utf-8'?>hi i'm some xml")),
            energonsoftware::XmlDocumentParserError);

        _parser->reset();
        CPPUNIT_ASSERT_THROW(_parser->feed(
            energonsoftware::XmlString("<?xml version='1.0' encoding='utf-8'?><message>hi i'm some xml</test>")),
            energonsoftware::XmlDocumentParserError);

        _parser->reset();
        CPPUNIT_ASSERT_THROW(_parser->feed(
            energonsoftware::XmlString("<?xml version='1.0' encoding='utf-8'?><test>hi i'm some xml</test>test")),
            energonsoftware::XmlDocumentParserError);

        _parser->reset();
        CPPUNIT_ASSERT_THROW(_parser->feed(
            energonsoftware::XmlString("<?xml version='1.0' encoding='utf-8'?><test><inner /><stuff></test>")),
            energonsoftware::XmlDocumentParserError);

        _parser->reset();
        CPPUNIT_ASSERT_THROW(_parser->feed(
            energonsoftware::XmlString("<?xml version='1.0' encoding='utf-8'?><test><inner /><stuff></stuff>test</test>")),
            energonsoftware::XmlDocumentParserError);
    }

private:
    std::unique_ptr<energonsoftware::XmlDocumentParser> _parser;
};

energonsoftware::Logger& XmlDocumentParserTest::logger(energonsoftware::Logger::instance("energonsoftware.core.xml.XmlDocumentParser"));

CPPUNIT_TEST_SUITE_REGISTRATION(XmlDocumentParserTest);

#endif
