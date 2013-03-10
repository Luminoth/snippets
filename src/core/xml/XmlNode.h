#if !defined __XMLNODE_H__
#define __XMLNODE_H__

#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include "XmlUtil.h"

/*
XmlNode tree

root
 |
child - sibling - sibling - sibling - ...
          |
        child - sibling - sibling - ...
                   |
                  ...
*/

namespace energonsoftware {

class XmlNode final
{
public:
    explicit XmlNode(const XmlString& name, const XmlAttributeMap& attributes=XmlAttributeMap());
    XmlNode(const XmlString& name, const XmlString& data, const XmlAttributeMap& attributes=XmlAttributeMap());
    virtual ~XmlNode() noexcept;

    // NOTE: this sets the node's parent to nullptr
    // and copies the children and siblings of the node
    XmlNode(const XmlNode& node);

public:
    const XmlString& name() const { return _name; }
    const XmlString& data() const { return _data; }

    const XmlNode* parent() const { return _parent; }
    XmlNode* parent() { return _parent; }

    const XmlNode* child() const { return _child.get(); }
    XmlNode* child() { return _child.get(); }

    const XmlNode* sibling() const { return _sibling.get(); }
    XmlNode* sibling() { return _sibling.get(); }

    // returns the first subnode with the given name
    // or null if there isn't one (only one level down)
    const XmlNode* node(const XmlString& name) const;
    XmlNode* node(const XmlString& name);

    // returns all subnodes with the given name (only one level down)
    //void nodes(const XmlString& name, std::list<const XmlNode*>& nodes) const;

    // adds the given node as a child of this node
    // NOTE: this copies the node and all of its subnodes
    XmlNode& add_node(const XmlNode& node);

    void add_data(const XmlString& data);

    const XmlAttributeMap& attributes() const { return _attributes; }
    XmlAttributeMap& attributes() { return _attributes; }

    // returns the attribute with the given name
    // or null if there isn't one
    const XmlString* attribute(const XmlString& name) const;
    XmlString& attribute(const XmlString& name, const XmlString& value);

public:
    // NOTE: this sets the node's parent to nullptr
    // and copies the children and siblings of the node
    XmlNode& operator=(const XmlNode& rhs);

private:
    XmlString _name;
    XmlString _data;
    XmlAttributeMap _attributes;

    std::unique_ptr<XmlNode> _child, _sibling;
    XmlNode* _parent;

private:
    XmlNode();

public:
    friend xercesc::XMLFormatter& operator<<(xercesc::XMLFormatter& lhs, const XmlNode& rhs);

    template<typename T, typename traits>
    friend std::basic_ostream<T, traits>& operator<<(std::basic_ostream<T, traits>& lhs, const XmlNode& rhs);
};

inline xercesc::XMLFormatter& operator<<(xercesc::XMLFormatter& lhs, const XmlNode& rhs)
{
    std::stringstream str;
    str << rhs;
    lhs << XmlString(str.str());
    return lhs;
}

template<typename T, typename traits>
inline std::basic_ostream<T, traits>& operator<<(std::basic_ostream<T, traits>& lhs, const XmlNode& rhs)
{
    xercesc::MemBufFormatTarget target;
    xercesc::XMLFormatter formatter("utf-8", &target, xercesc::XMLFormatter::NoEscapes);

    formatter << XmlString("<") << rhs.name();

    for(const auto& i : rhs.attributes()) {
        formatter << XmlString(" ") << i.first << XmlString("=\"")
            << xercesc::XMLFormatter::AttrEscapes << i.second
            << xercesc::XMLFormatter::NoEscapes << XmlString("\"");
    }

    if(rhs.data().empty() && nullptr == rhs.child()) {
        formatter << XmlString("/>");
    } else {
        formatter << XmlString(">");

        if(!rhs.data().empty()) {
            formatter << xercesc::XMLFormatter::CharEscapes
                << rhs.data() << xercesc::XMLFormatter::NoEscapes;
        } else if(nullptr != rhs.child()) {
            const XmlNode* current = rhs.child();
            while(current) {
                formatter << *current;
                current = current->sibling();
            }
        }
        formatter << XmlString("</") << rhs.name() << XmlString(">");
    }

    lhs << XmlString(reinterpret_cast<const char*>(target.getRawBuffer()));
    return lhs;
}

}

#endif
