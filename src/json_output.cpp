/** 
 * @file json_output.cpp 
 * @author lymslive 
 * @date 2021-11-07
 * @brief implementation for json format
 * */
#include <sstream>
#include <fstream>

#include "json_output.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/ostreamwrapper.h"

// print json in pretty or condensed format
namespace jsonkit
{

bool write_stream(const rapidjson::Value& json, std::ostream& stream, bool pretty)
{
    rapidjson::OStreamWrapper os(stream);

    if (pretty)
    {
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(os);
        json.Accept(writer);
    }
    else
    {
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(os);
        json.Accept(writer);
    }

    return true;
}

bool write_file(const rapidjson::Value& json, const std::string& file, bool pretty)
{
    std::ofstream ofs(file.c_str());
    if (!ofs.is_open())
    {
        return false;
    }

    bool bRet = write_stream(json, ofs, pretty);
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

} /* jsonkit */ 
