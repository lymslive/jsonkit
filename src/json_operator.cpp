#include "json_operator.h"

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

/**************************************************************/

COperand COperand::OperatePath(const char* path) const
{
    if (!m_pJsonNode || !path || path[0] == '\0')
    {
        return *this;
    }

    if (!m_pJsonNode->IsObject() && !m_pJsonNode->IsArray())
    {
        return *this;
    }

    if (m_pJsonNode->IsObject() && m_pJsonNode->HasMember(path))
    {
        // todo: fix efficient
        return OperateStar((*m_pJsonNode)[path]);
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

    if (m_pJsonNode->IsArray() && all_digit)
    {
        size_t index = (size_t) atoi(path);
        if (m_pJsonNode->Size() > index)
        {
            return OperateStar((*m_pJsonNode)[index]);
        }
    }

    if (any_slash)
    {
        const rapidjson::Value* pJsonNode = point(*m_pJsonNode, path);
        return COperand(pJsonNode, m_pAllocator);
    }

    return *this;
}

COperand COperand::OperatePath(size_t index) const
{
    if (!m_pJsonNode)
    {
        return *this;
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

/**************************************************************/

static bool s_path_error = false;
bool has_path_error()
{
    return s_path_error;
}

const rapidjson::Value& null_value()
{
    static rapidjson::Value json;
    return json;
}

inline
const rapidjson::Value& fail_path()
{
    s_path_error = true;
    return null_value();
}

inline
const rapidjson::Value& success_path(const rapidjson::Value& ret)
{
    s_path_error = false;
    return ret;
}

const rapidjson::Value& operate_path(const rapidjson::Value& json, const char* path)
{
    if (path == NULL || path[0] == '\0')
    {
        return jsonkit::fail_path();
    }

    if (!json.IsObject() && !json.IsArray())
    {
        return jsonkit::fail_path();
    }

    if (json.IsObject() && json.HasMember(path))
    {
        return jsonkit::success_path(json[path]);
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
            return jsonkit::success_path(json[index]);
        }
    }

    if (any_slash)
    {
        const rapidjson::Value *pVal = point(json, path);
        if (pVal)
        {
            return jsonkit::success_path(*pVal);
        }
    }

    return jsonkit::fail_path();
}

const rapidjson::Value& operate_path(const rapidjson::Value& json, size_t index)
{
    if (json.IsArray() && json.Size() > index)
    {
        return jsonkit::success_path(json[index]);
    }
    return jsonkit::fail_path();
}

} // end of namespace jsonkit

