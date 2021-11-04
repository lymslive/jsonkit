#include "json_operator.h"
#include "jsonkit_internal.h"

#include "rapidjson/pointer.h"

#include <algorithm>

namespace jsonkit
{

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
        char *endptr;
        const char* str = json.GetString();
        int num = strtol(str, &endptr, 10);
        if (endptr != str)
        {
            dest = num;
            return true;
        }
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
        char *endptr;
        const char* str = json.GetString();
        std::string str_tmp = json.GetString();
        double num = strtod(str, &endptr);
        if (endptr != str)
        {
            dest = num;
            return true;
        }
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

/**************************************************************/

const rapidjson::Value& error_value()
{
    static rapidjson::Value json;
    if (!json.IsNull())
    {
        LOGF("Warnning: null value may modified accidently!! reset to null");
        json.SetNull();
    }
    return json;
}

bool is_error_value(const rapidjson::Value& json)
{
    return &json == &(error_value());
}

const rapidjson::Value* do_operate_path(const rapidjson::Value& json, const char* path)
{
    if (path == NULL || path[0] == '\0')
    {
        return nullptr;
    }

    if (!json.IsObject() && !json.IsArray())
    {
        return nullptr;
    }

    if (json.IsObject())
    {
        auto it = json.FindMember(path);
        if (it != json.MemberEnd())
        {
            return &(it->value);
        }
    }

    bool any_slash = false;
    bool all_digit = true;
    for (size_t i = 0; ; i++)
    {
        char c = path[i];
        if (c == '\0')
        {
            break;
        }
        if (c == '/')
        {
            any_slash = true;
            all_digit = false;
            break;
        }
        if (c <= '0' || c >= '9')
        {
            all_digit = false;
        }
    }

    if (json.IsArray() && all_digit)
    {
        size_t index = (size_t) atoi(path);
        if (json.Size() > index)
        {
            return &(json[index]);
        }
    }

    if (any_slash)
    {
        return point(json, path);
    }

    return nullptr;
}

const rapidjson::Value& operate_path(const rapidjson::Value& json, const char* path)
{
    const rapidjson::Value* pVal = do_operate_path(json, path);
    if (pVal)
    {
        return *pVal;
    }
    else
    {
        return error_value();
    }
}

const rapidjson::Value& operate_path(const rapidjson::Value& json, size_t index)
{
    if (json.IsArray() && json.Size() > index)
    {
        return json[index];
    }
    return error_value();
}

/**************************************************************/

COperand COperand::OperatePath(const char* path) const
{
    if (!m_pJsonNode || !path || path[0] == '\0')
    {
        return Zero();
    }

    const rapidjson::Value* pJsonNode = do_operate_path(*m_pJsonNode, path);
    return COperand(pJsonNode, m_pAllocator);
}

COperand COperand::OperatePath(size_t index) const
{
    if (!m_pJsonNode)
    {
        return Zero();
    }

    if (m_pJsonNode->IsArray() && m_pJsonNode->Size() > index)
    {
       return OperateStar((*m_pJsonNode)[index]);
    }
    else
    {
        return COperand(nullptr, m_pAllocator);
    }
}

COperand& COperand::Assign(const char* psz)
{
    if (m_pJsonNode && m_pAllocator)
    {
        m_pJsonNode->SetString(psz, *m_pAllocator);
    }
    return *this;
}

COperand& COperand::Assign(const std::string& str)
{
    if (m_pJsonNode && m_pAllocator)
    {
        m_pJsonNode->SetString(str.c_str(), str.size(), *m_pAllocator);
    }
    return *this;
}

} // end of namespace jsonkit

