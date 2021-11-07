/**
 * @file jsonkit_plain.cpp
 * @author lymslive
 * @date 2021-11-07
 * @brief implement jsonkit plain interface by rapidjson
 * */

#include "jsonkit_plain.h"
#include "jsonkit_rpdjn.h"

namespace jsonkit
{
    
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

/* -------------------------------------------------- */

bool form_schema(const char* inJson, size_t inLen, std::string& outSchema)
{
    rapidjson::Document docJson;
    docJson.Parse(inJson, inLen);
    if (docJson.HasParseError())
    {
        return false;
    }

    rapidjson::Document docSchema;
    form_schema(docJson, docSchema);
    return condense(docSchema, outSchema);
}

bool from_schema(const char* inSchema, size_t inLen, std::string& outJson)
{
    rapidjson::Document docSchema;
    docSchema.Parse(inSchema, inLen);
    if (docSchema.HasParseError())
    {
        return false;
    }

    rapidjson::Document docJson;
    from_schema(docSchema, docJson);
    return condense(docJson, outJson);
}

bool form_schema(const std::string& inJson, std::string& outSchema)
{
    return form_schema(inJson.c_str(), inJson.size(), outSchema);
}

bool from_schema(const std::string& inSchema, std::string& outJson)
{
    return from_schema(inSchema.c_str(), inSchema.size(), outJson);
}

/* -------------------------------------------------- */

// validate the json according to schema
bool validate_schema(const std::string& inJson, const std::string& inSchema)
{
    rapidjson::Document docJson;
    docJson.Parse(inJson.c_str(), inJson.size());
    if (docJson.HasParseError())
    {
        return false;
    }

    rapidjson::Document docSchema;
    docSchema.Parse(inSchema.c_str(), inSchema.size());
    if (docSchema.HasParseError())
    {
        return false;
    }

    return validate_schema(docJson, docSchema);
}

bool validate_schema(const std::string& inJson, const std::string& inSchema, const std::string& basedir)
{
    rapidjson::Document docJson;
    docJson.Parse(inJson.c_str(), inJson.size());
    if (docJson.HasParseError())
    {
        return false;
    }

    rapidjson::Document docSchema;
    docSchema.Parse(inSchema.c_str(), inSchema.size());
    if (docSchema.HasParseError())
    {
        return false;
    }

    return validate_schema(docJson, docSchema, basedir);
}

bool validate_schema_file(const std::string& inJson, const std::string& inSchemaFile)
{
    rapidjson::Document docJson;
    docJson.Parse(inJson.c_str(), inJson.size());
    if (docJson.HasParseError())
    {
        return false;
    }

    rapidjson::Document docSchema;
    if (!read_file(docSchema, inSchemaFile))
    {
        return false;
    }

    std::string basedir;
    size_t pos = inSchemaFile.find_last_of("/\\:");
    if (pos == std::string::npos)
    {
        basedir = inSchemaFile;
    }
    else
    {
        basedir = inSchemaFile.substr(0, pos);
    }

    return validate_schema(docJson, docSchema, basedir);
}

/* -------------------------------------------------- */

bool compare(const std::string& aJson, const std::string& bJson)
{
    rapidjson::Document docA;
    docA.Parse(aJson.c_str(), aJson.size());
    if (docA.HasParseError())
    {
        return false;
    }

    rapidjson::Document docB;
    docB.Parse(bJson.c_str(), bJson.size());
    if (docB.HasParseError())
    {
        return false;
    }

    return jsonkit::compare(docA, docB);
}

bool compatible(const std::string& aJson, const std::string& bJson)
{
    rapidjson::Document docA;
    docA.Parse(aJson.c_str(), aJson.size());
    if (docA.HasParseError())
    {
        return false;
    }

    rapidjson::Document docB;
    docB.Parse(bJson.c_str(), bJson.size());
    if (docB.HasParseError())
    {
        return false;
    }

    return jsonkit::compatible(docA, docB);
}

/* -------------------------------------------------- */

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
