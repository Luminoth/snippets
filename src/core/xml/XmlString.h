#if !defined __XMLSTRING_H__
#define __XMLSTRING_H__

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/util/XMLString.hpp>

namespace energonsoftware {

class XmlString final
{
public:
    XmlString();
    explicit XmlString(const char* str);
    explicit XmlString(const XMLCh* str);
    //explicit XmlString(const wchar_t* str);
    explicit XmlString(const std::string& str);
    explicit XmlString(const std::wstring& str);

    template<typename InputIterator>
    XmlString(InputIterator begin, InputIterator end);

    virtual ~XmlString() noexcept;

public:
    std::string to_native() const;
    std::wstring to_wide() const;
    bool empty() const;
    const XMLCh* c_str() const;
    size_t length() const;

public:
    bool operator==(const XmlString& rhs) const;
    bool operator!=(const XmlString& rhs) const;
    XmlString operator+(const XmlString& rhs) const;
    XmlString& operator+=(const XmlString& rhs);

private:
    std::basic_string<XMLCh> _str;

public:
    friend xercesc::XMLFormatter& operator<<(xercesc::XMLFormatter& lhs, const XmlString& rhs);

    template<typename T, typename traits>
    friend std::basic_ostream<T, traits>& operator<<(std::basic_ostream<T, traits>& lhs, const XmlString& rhs);
};

inline xercesc::XMLFormatter& operator<<(xercesc::XMLFormatter& lhs, const XmlString& rhs)
{
    lhs << rhs.c_str();
    return lhs;
}

template<typename T, typename traits>
inline std::basic_ostream<T, traits>& operator<<(std::basic_ostream<T, traits>& lhs, const XmlString& rhs)
{
    lhs << rhs.to_native();
    return lhs;
}

}

namespace std {

template<>
struct hash<energonsoftware::XmlString>
{
    size_t operator()(const energonsoftware::XmlString& v) const
    { return std::hash<std::string>()(v.to_native()); }
};

}

#endif
