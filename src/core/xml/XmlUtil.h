#if !defined __XMLUTIL_H__
#define __XMLUTIL_H__

#include "XmlString.h"

namespace energonsoftware {

typedef std::unordered_map<XmlString, XmlString> XmlAttributeMap;

namespace XmlUtil {

void initialize();
void cleanup();
void attributes_to_map(const XmlAttributeMap& attribute_map, std::unordered_map<std::string, std::string>& attributes);

}

}

#endif
