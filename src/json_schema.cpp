/** 
 * @file json_schema.cpp 
 * @author lymslive 
 * @date 2021-10-27
 * @brief implementation for json scheam tools
 * */
#include "jsonkit_plain.h"
#include "jsonkit_rpdjn.h"
#include "CJsonSchema.h"
#include "jsonkit_internal.h"

#include "rapidjson/document.h"
#include "rapidjson/schema.h"

namespace jsonkit
{
namespace impl
{
// BEG namespace jsonkit::impl

using namespace rapidjson;

// forword declare
void generate_schema_internal(const Value& valJson, Value& valSchema, Document::AllocatorType& allocator);

void generate_schema_scalar(const Value& valJson, Value& valSchema, Document::AllocatorType& allocator)
{
    if (valJson.IsNull())
    {
        valSchema.AddMember("type", "null", allocator);
    }
    else if (valJson.IsBool())
    {
        valSchema.AddMember("type", "boolean", allocator);
    }
    else if (valJson.IsString())
    {
        valSchema.AddMember("type", "string", allocator);
    }
    else if (valJson.IsInt() || valJson.IsUint())
    {
        valSchema.AddMember("type", "integer", allocator);
    }
    else if (valJson.IsDouble())
    {
        valSchema.AddMember("type", "number", allocator);
    }
}

void generate_schema_object(const Value& valJson, Value& valSchema, Document::AllocatorType& allocator)
{
    valSchema.AddMember("type", "object", allocator);
    if (!valJson.IsObject() || valJson.ObjectEmpty())
    {
        return;
    }

    Value valProperties;
    valProperties.SetObject();
    Value valRequired;
    valRequired.SetArray();

    Value::ConstMemberIterator itField = valJson.MemberBegin();
    for (; itField != valJson.MemberEnd(); ++itField)
    {
        Value key;
        key.SetString(itField->name.GetString(), allocator);
        Value itemSchema;
        const Value& itemJson = itField->value;
        generate_schema_internal(itemJson, itemSchema, allocator);
        valProperties.AddMember(key, itemSchema, allocator);

        // key may moved, create another string json value
        key.SetString(itField->name.GetString(), allocator);
        valRequired.PushBack(key, allocator);
    }

    valSchema.AddMember("properties", valProperties, allocator);
    valSchema.AddMember("required", valRequired, allocator);

}

// assume each item in the json array is the same type, then
// generate schema from the first item[0]
void generate_schema_array(const Value& valJson, Value& valSchema, Document::AllocatorType& allocator)
{
    valSchema.AddMember("type", "array", allocator);
    if (!valJson.IsArray() || valJson.Empty())
    {
        return;
    }

    Value itemSchema;
    const Value& itemJson = valJson[0];
    generate_schema_internal(itemJson, itemSchema, allocator);
    valSchema.AddMember("items", itemSchema, allocator);
}

void generate_schema_internal(const Value& valJson, Value& valSchema, Document::AllocatorType& allocator)
{
    valSchema.SetObject();
    if (valJson.IsObject())
    {
        generate_schema_object(valJson, valSchema, allocator);
    }
    else if (valJson.IsArray())
    {
        generate_schema_array(valJson, valSchema, allocator);
    }
    else
    {
        generate_schema_scalar(valJson, valSchema, allocator);
    }
}

bool generate_schema(const Value& docJson, Document& docSchema)
{
    Document::AllocatorType& allocator = docSchema.GetAllocator();
    generate_schema_internal(docJson, docSchema, allocator);
    return true;
}

/* -------------------------------------------------- */

// forword declare
bool generate_json_internal(const Value& valSchema, Value& valJson, Document::AllocatorType& allocator);

bool generate_json_object(const Value& valSchema, Value& valJson, Document::AllocatorType& allocator)
{
    Value::ConstMemberIterator itField = valSchema.MemberBegin();
    for (; itField != valSchema.MemberEnd(); ++itField)
    {
        Value keyField;
        keyField.SetString(itField->name.GetString(), allocator);
        Value valField;
        if (itField->value.IsObject())
        {
            if (generate_json_internal(itField->value, valField, allocator))
            {
                valJson.AddMember(keyField, valField, allocator);
            }
        }
    }

    return true;
}

// only support array schema with the same type for each item
bool generate_json_array(const Value& valSchema, Value& valJson, Document::AllocatorType& allocator)
{
    Value valItem;
    if (generate_json_internal(valSchema, valItem, allocator))
    {
        valJson.PushBack(valItem, allocator);
    }

    return true;
}

bool generate_json_internal(const Value& valSchema, Value& valJson, Document::AllocatorType& allocator)
{
    if (!valSchema.IsObject())
    {
        LOGF("error: schema must be object.");
        return false;
    }
    
    Value::ConstMemberIterator itType = valSchema.FindMember("type");
    if (itType == valSchema.MemberEnd())
    {
        return false;
    }

    if (!itType->value.IsString())
    {
        return false;
    }

    std::string strType = itType->value.GetString();
    if (strType == "null")
    {
        valJson.SetNull();
    }
    else if (strType == "boolean")
    {
        valJson = true;
    }
    else if (strType == "string")
    {
        valJson = "sample str";
    }
    else if (strType == "integer")
    {
        valJson = 0;
    }
    else if (strType == "number")
    {
        valJson = 0.1;
    }
    else if (strType == "object")
    {
        valJson.SetObject();
        Value::ConstMemberIterator itProp = valSchema.FindMember("properties");
        if (itProp != valSchema.MemberEnd())
        {
            return generate_json_object(itProp->value, valJson, allocator);
        }
    }
    else if (strType == "array")
    {
        valJson.SetArray();
        Value::ConstMemberIterator itItem = valSchema.FindMember("items");
        if (itItem != valSchema.MemberEnd())
        {
            return generate_json_array(itItem->value, valJson, allocator);
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool generate_json(const Value& docSchema, Document& docJson)
{
    Document::AllocatorType& allocator = docJson.GetAllocator();
    return generate_json_internal(docSchema, docJson, allocator);
}

// END namespace jsonkit::impl
}
}

// interface functions
namespace jsonkit
{

bool form_schema(const rapidjson::Value& inJson, rapidjson::Document& outSchema)
{
    return jsonkit::impl::generate_schema(inJson, outSchema);
}

bool from_schema(const rapidjson::Value& inSchema, rapidjson::Document& outJson)
{
    return jsonkit::impl::generate_json(inSchema, outJson);
}

// do not support remote schema (IRemoteSchemaDocumentProvider)
bool validate_schema(const rapidjson::Value& inJson, const rapidjson::Value& inSchema)
{
    CJsonSchema schema(inSchema);
    return schema.Validate(inJson);
}

bool validate_schema(const rapidjson::Value& inJson, const rapidjson::Value& inSchema, const std::string& basedir)
{
    CJsonSchema schema(inSchema, basedir);
    return schema.Validate(inJson);
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

/* -------------------------------------------------- */

bool form_schema(const std::string& inJson, std::string& outSchema)
{
    return form_schema(inJson.c_str(), inJson.size(), outSchema);
}

bool from_schema(const std::string& inSchema, std::string& outJson)
{
    return from_schema(inSchema.c_str(), inSchema.size(), outJson);
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

} /* jsonkit */ 
