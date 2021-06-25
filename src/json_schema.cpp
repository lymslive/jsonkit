#include "jsonkit_plain.h"
#include "jsonkit_rpdjn.h"
#include "CJsonSchema.h"

#include "rapidjson/document.h"
#include "rapidjson/schema.h"

namespace jsonkit
{
namespace impl
{
// BEG namespace jsonkit::impl

// END namespace jsonkit::impl
}
}

// interface functions
namespace jsonkit
{

bool form_schema(const rapidjson::Value& inJson, rapidjson::Document& outSchema);
bool from_schema(const rapidjson::Value& inSchema, rapidjson::Document& outJson);

// do not support remote schema (IRemoteSchemaDocumentProvider)
bool validate_schema(const rapidjson::Value& inJson, const rapidjson::Document& inSchema)
{
    CJsonSchema schema(inSchema);
    return schema.Validate(inJson);
}

bool form_schema(const std::string& inJson, std::string& outSchema)
{
    rapidjson::Document docJson;
    docJson.Parse(inJson.c_str(), inJson.size());
    if (docJson.HasParseError())
    {
        return false;
    }

    rapidjson::Document docSchema;
    form_schema(docJson, docSchema);
    return condense(docSchema, outSchema);
}

bool from_schema(const std::string& inSchema, std::string& outJson)
{
    rapidjson::Document docSchema;
    docSchema.Parse(inSchema.c_str(), inSchema.size());
    if (docSchema.HasParseError())
    {
        return false;
    }

    rapidjson::Document docJson;
    from_schema(docSchema, docJson);
    return condense(docJson, outJson);
}

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

} /* jsonkit */ 
