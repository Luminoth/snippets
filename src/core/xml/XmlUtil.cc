#include "src/pch.h"
#include "XmlUtil.h"

namespace energonsoftware {
namespace XmlUtil {

void initialize()
{
    xercesc::XMLPlatformUtils::Initialize();
}

void cleanup()
{
    xercesc::XMLPlatformUtils::Terminate();
}

void attributes_to_map(const XmlAttributeMap& attribute_map, std::unordered_map<std::string, std::string>& attributes)
{
    attributes.clear();
    for(const auto& attribute : attribute_map)  {
        attributes[attribute.first.to_native()] = attribute.second.to_native();
    }
}

}
}
