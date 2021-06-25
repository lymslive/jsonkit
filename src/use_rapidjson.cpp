#include "use_rapidjson.h"

#include <fstream>

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
        fprintf(stderr, "Error(offset %u): %s\n",
                static_cast<unsigned>(doc.GetErrorOffset()),
                GetParseError_En(doc.GetParseError()));
        return false;
    }
    return true;
}

bool read_file(rapidjson::Document& doc, const std::string& file)
{
    std::ifstream ifs(file);
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
    std::ofstream ofs(file);
    if (!ofs.is_open())
    {
        return false;
    }

    bool bRet = write_stream(doc, ofs);
    ofs.close();
    return bRet;
}

} /* jsonkit */ 

