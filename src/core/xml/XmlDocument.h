#if !defined __XMLDOCUMENT_H__
#define __XMLDOCUMENT_H__

#include "XmlNode.h"

namespace energonsoftware {

class XmlDocument /*final*/
{
public:
    explicit XmlDocument(const XmlString& name);
    explicit XmlDocument(const XmlNode& root);
    XmlDocument(const XmlDocument& document);
    virtual ~XmlDocument() noexcept;

public:
    const XmlString& type() const { return _root.name(); }
    const XmlNode& root() const { return _root; }
    XmlNode& root() { return _root; }
    XmlString document() const;

    XmlDocument& operator=(const XmlDocument& rhs);

private:
    XmlNode _root;

private:
    XmlDocument() = delete;

public:
    template<typename T, typename traits>
    friend std::basic_ostream<T, traits>& operator<<(std::basic_ostream<T, traits>& lhs, const XmlDocument& rhs);
};

template<typename T, typename traits>
inline std::basic_ostream<T, traits>& operator<<(std::basic_ostream<T, traits>& lhs, const XmlDocument& rhs)
{
    lhs << rhs._root;
    return lhs;
}

}

#endif
