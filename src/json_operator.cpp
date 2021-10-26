#include "json_operator.h"
#include "use_rapidjson.h"

#include "rapidjson/pointer.h"

namespace jsonkit
{

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

} // end of namespace jsonkit

const rapidjson::Value& operator/ (const rapidjson::Value& json, const std::string& path)
{
    return operator/(json, path.c_str());
}

const rapidjson::Value& operator/ (const rapidjson::Value& json, const char* path)
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
        std::string fixPath;
        if (path[0] == '/')
        {
            fixPath = path;
        }
        else
        {
            fixPath = std::string("/") + std::string(path);
        }
        rapidjson::Pointer jpath(fixPath.c_str(), fixPath.size());
        const rapidjson::Value *pVal = jpath.Get(json);
        if (pVal)
        {
            return jsonkit::success_path(*pVal);
        }
    }

    return jsonkit::fail_path();
}

const rapidjson::Value& operator/ (const rapidjson::Value& json, size_t index)
{
    if (json.IsArray() && json.Size() > index)
    {
        return jsonkit::success_path(json[index]);
    }
    return jsonkit::fail_path();
}

const rapidjson::Value& operator/ (const rapidjson::Value& json, int index)
{
    if (index < 0)
    {
        return jsonkit::fail_path();
    }
    return operator/(json, size_t(index));
}

int operator|(const rapidjson::Value& json, int defval)
{
    int val;
    if (jsonkit::scalar_value(val, json))
    {
        return val;
    }
    return defval;
}

double operator|(const rapidjson::Value& json, double defval)
{
    double val;
    if (jsonkit::scalar_value(val, json))
    {
        return val;
    }
    return defval;
}

std::string operator|(const rapidjson::Value& json, const char* defval)
{
    std::string val;
    if (jsonkit::scalar_value(val, json))
    {
        return val;
    }
    return defval;
}

std::string operator|(const rapidjson::Value& json, const std::string& defval)
{
    std::string val;
    if (jsonkit::scalar_value(val, json))
    {
        return val;
    }
    return defval;
}

