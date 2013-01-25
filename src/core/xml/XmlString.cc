#include "src/pch.h"
#include "XmlString.h"

namespace energonsoftware {

XmlString::XmlString()
    : _str()
{
}

XmlString::XmlString(const char* str)
    : _str()
{
    XMLCh* scratch = xercesc::XMLString::transcode(str);
    _str = scratch;
    xercesc::XMLString::release(&scratch);
    scratch = nullptr;
}

XmlString::XmlString(const XMLCh* str)
    : _str(str)
{
}

/*XmlString::XmlString(const wchar_t* str)
    : _str()
{
    std::wstring t(str);
    std::string s(t.begin(), t.end());

    XMLCh* scratch = xercesc::XMLString::transcode(s.c_str());
    _str = scratch;
    xercesc::XMLString::release(&scratch);
    scratch = nullptr;
}*/

XmlString::XmlString(const std::string& str)
    : _str()
{
    XMLCh* scratch = xercesc::XMLString::transcode(str.c_str());
    _str = scratch;
    xercesc::XMLString::release(&scratch);
    scratch = nullptr;
}

XmlString::XmlString(const std::wstring& str)
    : _str(str.begin(), str.end())
{
}

template<class InputIterator>
XmlString::XmlString(InputIterator begin, InputIterator end)
    : _str(begin, end)
{
}

XmlString::~XmlString() throw()
{
}

std::string XmlString::to_native() const
{
    char* scratch = xercesc::XMLString::transcode(c_str());
    std::string ret(scratch);
    xercesc::XMLString::release(&scratch);
    scratch = nullptr;
    return ret;
}

std::wstring XmlString::to_wide() const
{
    return std::wstring(_str.begin(), _str.end());
}

bool XmlString::empty() const
{
    return length() == 0;
}

const XMLCh* XmlString::c_str() const
{
    return _str.c_str();
}

size_t XmlString::length() const
{
    return xercesc::XMLString::stringLen(c_str());
}

bool XmlString::operator==(const XmlString& rhs) const
{
    return xercesc::XMLString::equals(c_str(), rhs.c_str());
}

bool XmlString::operator!=(const XmlString& rhs) const
{
    return !xercesc::XMLString::equals(c_str(), rhs.c_str());
}

XmlString XmlString::operator+(const XmlString& rhs) const
{
    return XmlString((_str + rhs._str).c_str());
}

XmlString& XmlString::operator+=(const XmlString& rhs)
{
    _str += rhs._str;
    return *this;
}

}
