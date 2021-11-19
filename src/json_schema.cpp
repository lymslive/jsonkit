/** 
 * @file json_schema.cpp 
 * @author lymslive 
 * @date 2021-10-27
 * @brief implementation for json scheam tools
 * */
#include <regex>

#include "json_schema.h"
#include "json_input.h"
#include "json_output.h"
#include "CJsonSchema.h"
#include "jsonkit_internal.h"
#include "json_operator.h"

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

} /* jsonkit */ 

/* ************************************************************ */
// for flat schema validate

namespace jsonkit
{

class CFlatSchema
{
public:
    bool Validate(const rapidjson::Value& json, const rapidjson::Value& schema)
    {
        m_path.clear();
        m_error.clear();
        return doValidate(json, schema);
    }

    void GetError(std::string& str) const
    {
        str.append(m_error).append(" ").append(m_path);
    }

private:
    bool doValidate(const rapidjson::Value& json, const rapidjson::Value& schema);

    bool checkString(const rapidjson::Value& json, const rapidjson::Value& value);
    bool checkNumber(const rapidjson::Value& json, const rapidjson::Value& value);

private:
    std::string m_path;
    std::string m_error;
};

bool CFlatSchema::checkString(const rapidjson::Value& json, const rapidjson::Value& value)
{
    if (!value.IsString())
    {
        return false;
    }

    size_t length = value.GetStringLength();
    size_t max = json/"maxLength" | 0;
    if (max > 0 && length > max)
    {
        m_error = "STRING TOO LARGE";
        return false;
    }
    size_t min = json/"minLength" | 0;
    if (min > 0 && length < min)
    {
        m_error = "STRING TOO SMALL";
        return false;
    }

    auto& pattern = json/"pattern";
    if (!!pattern && pattern.IsString() && pattern.GetStringLength() > 0)
    {
        try
        {
            std::regex exp(pattern.GetString());
            bool match = std::regex_match(value.GetString(), exp);
            if (!match)
            {
                LOGF("%s !~ %s", value.GetString(), pattern.GetString());
                m_error = "STRING NOT MATCH PATTERN";
                return false;
            }
        }
        catch (std::regex_error)
        {
            LOGF("PATTERN INVALID: %s", pattern.GetString());
            m_error = "STRING PATTERN INVALID";
            // only log, not check
            // return false;
        }
    }

    return true;
}

bool CFlatSchema::checkNumber(const rapidjson::Value& json, const rapidjson::Value& value)
{
    // maybe check int64 is enough, for normal case
    if (value.IsInt64())
    {
        int64_t max = 0;
        if (scalar_value(max, json/"maxValue") && value.GetInt64() > max)
        {
            m_error = "NUMBER TOO LARGE";
            return false;
        }
        int64_t min = 0;
        if (scalar_value(min, json/"minValue") && value.GetInt64() < min)
        {
            m_error = "NUMBER TOO SMALL";
            return false;
        }
    }
    return true;
}

bool CFlatSchema::doValidate(const rapidjson::Value& json, const rapidjson::Value& schema)
{
    if (!json.IsObject())
    {
        return false;
    }

    if (!schema.IsArray())
    {
        return false;
    }

    std::string save_path = m_path;
    for (auto it = schema.Begin(); it != schema.End(); ++it)
    {
        if (!it->IsObject())
        {
            continue;
        }
        bool required = (*it)/"required" | false;
        if (!required)
        {
            continue;
        }
        std::string name = (*it)/"name" | "";
        if (name.empty())
        {
            continue;
        }

        m_path = save_path;
        m_path.append("/").append(name);

        auto& value = json/name;
        if (!value)
        {
            m_error = "NO KEY";
            LOGF("invalid json, no key: %s", m_path.c_str());
            return false;
        }

        // do not check type, as the type may variate
        if ((*it)/"vartype" | false)
        {
            continue;
        }

        std::string type = (*it)/"type" | "";
        if (type.empty())
        {
            continue; 
        }
        else if (type == "string")
        {
            if (!value.IsString())
            {
                m_error = "NOT STRING";
                LOGF("invalid json, not string in key: %s", m_path.c_str());
                return false;
            }
            else if(!checkString(*it, value))
            {
                return false;
            }
        }
        else if (type == "number")
        {
            if (!value.IsInt64() && !value.IsUint64() && !value.IsDouble())
            {
                m_error = "NOT NUMBER";
                LOGF("invalid json, not number in key: %s", m_path.c_str());
                return false;
            }
            else if(!checkNumber(*it, value))
            {
                return false;
            }
        }
        else if (type == "bool" || type == "boolean")
        {
            if (!value.IsBool())
            {
                m_error = "NOT BOOL";
                LOGF("invalid json, not bool in key: %s", m_path.c_str());
                return false;
            }
        }
        else if (type == "object")
        {
            if (!value.IsObject())
            {
                m_error = "NOT OBJECT";
                LOGF("invalid json, not object in key: %s", m_path.c_str());
                return false;
            }
            auto& children = (*it)/"children";
            if (!!children && children.IsArray())
            {
                if (!doValidate(value, children))
                {
                    LOGF("invalid json, invalid nested object in key: %s", name.c_str());
                    return false;
                }
            }
        }
        else if (type == "array")
        {
            // array of object
            if (!value.IsArray())
            {
                m_error = "NOT ARRAY";
                LOGF("invalid json, not object in key: %s", m_path.c_str());
                return false;
            }
            auto& children = (*it)/"children";
            if (!!children && children.IsArray())
            {
                int i = 0;
                for (auto itChild = value.Begin(); itChild != value.End(); ++itChild)
                {
                    m_path.append("/").append(std::to_string(i++));
                    if (!doValidate(*itChild, children))
                    {
                        LOGF("invalid json, invalid nested array of object in key: %s", name.c_str());
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool validate_flat_schema(const rapidjson::Value& json, const rapidjson::Value& schema)
{
    CFlatSchema obj;
    return obj.Validate(json, schema);
}

bool validate_flat_schema(const rapidjson::Value& json, const rapidjson::Value& schema, std::string& error)
{
    CFlatSchema obj;
    bool ret = obj.Validate(json, schema);
    if (!ret)
    {
        obj.GetError(error);
    }
    return ret;
}

} /* jsonkit */ 
