#include "json_operator.h"
#include "jsonkit_internal.h"

#include "rapidjson/pointer.h"

#include <algorithm>

namespace jsonkit
{

/**************************************************************/

/** try to convert string json to number, using strtol */
template <typename numberT>
bool string_number(numberT& dest, const rapidjson::Value& json)
{
    if (json.IsString())
    {
        char *endptr;
        const char* str = json.GetString();
        long long int num = strtol(str, &endptr, 10);
        if (endptr != str)
        {
            dest = (numberT)num;
            return true;
        }
    }
    return false;
}

bool scalar_value(const char*& dest, const rapidjson::Value& json)
{
    if (json.IsString())
    {
        dest = json.GetString();
        return true;
    }
    return false;
}

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
    if (json.IsString())
    {
        return string_number(dest, json);
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
    else if (json.IsInt64()) // Backwards compatibility
    {
        dest = (double)json.GetInt64();
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
    else if (json.IsUint64())
    {
        dest = (json.GetUint64() == 0) ? false : true;
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

bool scalar_value(uint32_t& dest, const rapidjson::Value& json)
{
    if (json.IsUint())
    {
        dest = json.GetUint();
        return true;
    }
    if (json.IsString())
    {
        return string_number(dest, json);
    }
    return false;
}

bool scalar_value(int64_t& dest, const rapidjson::Value& json)
{
    if (json.IsInt64())
    {
        dest = json.GetInt64();
        return true;
    }
    if (json.IsString())
    {
        return string_number(dest, json);
    }
    return false;
}
bool scalar_value(uint64_t& dest, const rapidjson::Value& json)
{
    if (json.IsUint64())
    {
        dest = json.GetUint64();
        return true;
    }
    if (json.IsString())
    {
        return string_number(dest, json);
    }
    return false;
}

/**************************************************************/

rapidjson::Value CPathError::value;

inline
const rapidjson::Value& error_value()
{
    // static rapidjson::Value json;
    return CPathError::value;
}

const rapidjson::Value& get_error_value()
{
    const rapidjson::Value& json = error_value();
    if (!json.IsNull())
    {
        LOGF("Warnning: incorrect use of path error value, best check with operator!");
        const_cast<rapidjson::Value&>(json).SetNull();
    }
    return json;
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
        return path_point(json, path);
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
        return get_error_value();
    }
}

const rapidjson::Value& operate_path(const rapidjson::Value& json, size_t index)
{
    if (json.IsArray() && json.Size() > index)
    {
        return json[index];
    }
    return get_error_value();
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

