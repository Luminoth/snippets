#include "src/pch.h"
#include "XmlNode.h"

namespace energonsoftware {

XmlNode::XmlNode(const XmlString& name, const XmlAttributeMap& attributes)
    : _name(name), _data(), _attributes(attributes),
        _child(), _sibling(), _parent(nullptr)
{
}

XmlNode::XmlNode(const XmlString& name, const XmlString& data, const XmlAttributeMap& attributes)
    : _name(name), _data(data), _attributes(attributes),
        _child(), _sibling(), _parent(nullptr)
{
}

XmlNode::XmlNode(const XmlNode& node)
    : _name(node._name), _data(node._data), _attributes(node._attributes),
        _child(), _sibling(), _parent(nullptr)
{
    if(node._sibling) {
        _sibling.reset(new XmlNode(*(node._sibling)));
    }

    if(node._child) {
        _child.reset(new XmlNode(*(node._child)));
    }
}

XmlNode::~XmlNode() noexcept
{
}

const XmlNode* XmlNode::node(const XmlString& name) const
{
    const XmlNode* current = child();
    while(current) {
        if(current->name() == name) {
            return current;
        }
        current = current->sibling();
    }
    return nullptr;
}

XmlNode* XmlNode::node(const XmlString& name)
{
    XmlNode* current = child();
    while(current) {
        if(current->name() == name) {
            return current;
        }
        current = current->sibling();
    }
    return nullptr;
}

/*void XmlNode::nodes(const XmlString& name, std::list<const XmlNode*>& nodes) const
{
    nodes.clear();

    const XmlNode* current = child();
    while(current) {
        if(current->name() == name) {
            nodes.push_back(current);
        }
        current = current->sibling();
    }
}*/

XmlNode& XmlNode::add_node(const XmlNode& node)
{
    if(!_child) {
        _child.reset(new XmlNode(node));
        _child->_parent = this;
        return *_child;
    }

    XmlNode *current=child(), *prev=nullptr;
    while(current) {
        prev = current;
        current = current->sibling();
    }
    prev->_sibling.reset(new XmlNode(node));
    current = prev->_sibling.get();
    current->_parent = this;
    return *current;
}

void XmlNode::add_data(const XmlString& data)
{
    _data += data;
}

const XmlString* XmlNode::attribute(const XmlString& name) const
{
    try {
        return &(_attributes.at(name));
    } catch(const std::out_of_range&) {
    }
    return nullptr;
}

XmlString& XmlNode::attribute(const XmlString& name, const XmlString& value)
{
    return _attributes[name] = value;
}

XmlNode& XmlNode::operator=(const XmlNode& rhs)
{
    _name = rhs.name();
    _data = rhs.data();
    _parent = nullptr;

    _attributes.clear();
    for(const auto& i : rhs.attributes()) {
        _attributes[i.first] = i.second;
    }

    if(rhs._sibling) {
        _sibling.reset(new XmlNode(*(rhs._sibling)));
    }

    if(rhs._child) {
        _child.reset(new XmlNode(*(rhs._child)));
    }

    return *this;
}

}
