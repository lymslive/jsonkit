/** 
 * @file json_output.cpp 
 * @author lymslive 
 * @date 2021-11-07
 * @brief implementation for json format
 * */
#include <sstream>
#include <fstream>

#include "jsonkit_plain.h"
#include "jsonkit_rpdjn.h"
#include "json_output.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/ostreamwrapper.h"

// print json in pretty or condensed format
namespace jsonkit
{

bool write_stream(const rapidjson::Value& doc, std::ostream& stream, bool pretty)
{
    rapidjson::OStreamWrapper os(stream);

    if (pretty)
    {
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(os);
        doc.Accept(writer);
    }
    else
    {
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(os);
        doc.Accept(writer);
    }

    return true;
}

bool write_file(const rapidjson::Value& doc, const std::string& file, bool pretty)
{
    std::ofstream ofs(file.c_str());
    if (!ofs.is_open())
    {
        return false;
    }

    bool bRet = write_stream(doc, ofs, pretty);
    ofs.close();
    return bRet;
}

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

/* -------------------------------------------------- */

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

/* -------------------------------------------------- */

bool prettify(const std::string& inJson, std::string& outJson)
{
    return prettify(inJson.c_str(), inJson.size(), outJson);
}

bool condense(const std::string& inJson, std::string& outJson)
{
    return condense(inJson.c_str(), inJson.size(), outJson);
}

} /* jsonkit */ 
