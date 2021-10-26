#include "use_rapidjson.h"
#include "jsonkit_internal.h"

#include <fstream>
#include <algorithm>

#include "rapidjson/prettywriter.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/error/en.h"

namespace jsonkit
{
    
bool read_stream(rapidjson::Document& doc, std::istream& stream)
{
    rapidjson::IStreamWrapper is(stream);
    doc.ParseStream(is);
    if (doc.HasParseError())
    {
        LOGF("Parse Json Error(offset %u): %s",
                static_cast<unsigned>(doc.GetErrorOffset()),
                GetParseError_En(doc.GetParseError()));
        return false;
    }
    return true;
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

/**************************************************************/

bool scalar_value(std::string& dest, const rapidjson::Value& json)
{
    if (json.IsString())
    {
        dest = json.GetString();
        return true;
    }

    return false;
}

bool scalar_value(int& dest, const rapidjson::Value& json)
{
    if (json.IsInt())
    {
        dest = json.GetInt();
        return true;
    }
    else if (json.IsString()) // Backwards compatibility
    {
        std::string str_tmp = json.GetString();
        dest = atoi(str_tmp.c_str());
        return true;
    }

    return false;
}

bool scalar_value(double& dest, const rapidjson::Value& json)
{
    if (json.IsDouble())
    {
        dest = json.GetDouble();
        return true;
    }
    else if (json.IsInt()) // Backwards compatibility
    {
        dest = (double)json.GetInt();
        return true;
    }
    else if (json.IsString())
    {
        std::string str_tmp = json.GetString();
        dest = atof(str_tmp.c_str());
        return true;
    }

    return false;
}

bool scalar_value(bool& dest, const rapidjson::Value& json)
{
    if (json.IsBool())
    {
        dest = json.GetBool();
        return true;
    }
    else if (json.IsInt()) // Backwards compatibility
    {
        dest = (json.GetInt() == 0) ? false : true;
        return true;
    }
    else if (json.IsString())
    {
        std::string str_tmp = json.GetString();
        std::transform(str_tmp.begin(), str_tmp.end(), str_tmp.begin(), ::toupper); //  uppercase 
        dest = (str_tmp == "TRUE") ? true : false;
        return true;
    }

    return false;
}

} /* jsonkit */ 

