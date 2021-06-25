#include "jsonkit_plain.h"
#include "jsonkit_rpdjn.h"
#include "use_rapidjson.h"

#include "rapidjson/document.h"

#include <sstream>

// print json in pretty or condensed format
namespace jsonkit
{

bool prettify(const rapidjson::Value& inJson, std::string& outJson)
{
    std::stringstream oss;
    write_stream(inJson, oss, true);
    outJson = oss.str();
    return true;
}

bool condense(const rapidjson::Value& inJson, std::string& outJson)
{
    std::stringstream oss;
    write_stream(inJson, oss, false);
    outJson = oss.str();
    return true;
}

bool prettify(const std::string& inJson, std::string& outJson)
{
    rapidjson::Document doc;
    doc.Parse(inJson.c_str(), inJson.size());
    if (doc.HasParseError())
    {
        return false;
    }
    return prettify(doc, outJson);
}

bool condense(const std::string& inJson, std::string& outJson)
{
    rapidjson::Document doc;
    doc.Parse(inJson.c_str(), inJson.size());
    if (doc.HasParseError())
    {
        return false;
    }
    return condense(doc, outJson);
}

bool prettify(const char* inJson, size_t inLen, std::string& outJson)
{
    rapidjson::Document doc;
    doc.Parse(inJson, inLen);
    if (doc.HasParseError())
    {
        return false;
    }
    return prettify(doc, outJson);
}

bool condense(const char* inJson, size_t inLen, std::string& outJson)
{
    rapidjson::Document doc;
    doc.Parse(inJson, inLen);
    if (doc.HasParseError())
    {
        return false;
    }
    return condense(doc, outJson);
}

} /* jsonkit */ 
