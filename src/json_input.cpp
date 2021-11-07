/** 
 * @file json_input.cpp
 * @author lymslive
 * @date 2021-11-07
 * @brief implement for json input
 * */
#include <fstream>

#include "json_input.h"
#include "jsonkit_internal.h"

#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

namespace jsonkit
{
    
static
bool parse_error(rapidjson::Document& doc)
{
    if (doc.HasParseError())
    {
        LOGF("Parse Json Error(offset %u): %s",
                static_cast<unsigned>(doc.GetErrorOffset()),
                GetParseError_En(doc.GetParseError()));
        return false;
    }
    return true;
}

bool read_string(rapidjson::Document& doc, const char* psz, size_t len)
{
    doc.Parse(psz, len);
    return parse_error(doc);
}

bool read_string(rapidjson::Document& doc, const std::string& str)
{
    return read_string(doc, str.c_str(), str.size());
}

bool read_stream(rapidjson::Document& doc, std::istream& stream)
{
    rapidjson::IStreamWrapper is(stream);
    doc.ParseStream(is);
    return parse_error(doc);
}

bool read_file(rapidjson::Document& doc, const std::string& file)
{
    std::ifstream ifs(file.c_str());
    if (!ifs.is_open())
    {
        return false;
    }

    bool bRet = read_stream(doc, ifs);
    ifs.close();
    return bRet;
}

} /* jsonkit */ 

