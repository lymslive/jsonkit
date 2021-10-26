#include "jsonkit_plain.h"
#include "jsonkit_rpdjn.h"

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

namespace jsonkit
{
    
const rapidjson::Value* point(const rapidjson::Value& inJson, const std::string& path)
{
    if (path.empty())
    {
        return nullptr;
    }

    if (path[0] == '/')
    {
        rapidjson::Pointer jpath(path.c_str(), path.size());
        return jpath.Get(inJson);
    }
    else
    {
        // make sure path start with a slash '/'
        return point(inJson, std::string("/") + path);
    }
}

bool point(const std::string& inJson, const std::string& path, std::string& outJson)
{
    return point(inJson.c_str(), inJson.size(), path, outJson);
}

bool point(const char* inJson, size_t inLen, const std::string& path, std::string& outJson)
{
    rapidjson::Document doc;
    doc.Parse(inJson, inLen);
    if (doc.HasParseError())
    {
        return false;
    }

    const rapidjson::Value* pJson = point(doc, path);
    if (!pJson)
    {
        return false;
    }

    return condense(*pJson, outJson);
}

} /* jsonkit */ 
