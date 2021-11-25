/**
 * @file json_filter.cpp
 * @author lymslive
 * @date 2021-11-13
 * @brief implementation of json filter functionality
 * */

#include <algorithm>
#include <regex>

#include "json_filter.h"
#include "json_output.h"
#include "json_input.h"

namespace jsonkit
{
    
inline
bool keep_bool(bool keep, bool tf)
{
    return keep ? tf : !tf;
}

inline
bool filter_fn_null(const rapidjson::Value& name, const rapidjson::Value& value)
{
    return !value.IsNull();
}

inline
bool filter_fn_empty(const rapidjson::Value& name, const rapidjson::Value& value)
{
    return !(value.IsNull()
            || (value.IsString() && value.GetStringLength() == 0)
            || (value.IsArray() && value.Empty())
            || (value.IsObject() && value.ObjectEmpty())
            );
}

class CFilterKey
{
public:
    CFilterKey(const std::vector<std::string>& keys, bool sorted, bool keep)
        : m_keys(keys), m_sorted(sorted), m_keep(keep)
    {}

    bool operator()(const rapidjson::Value& name, const rapidjson::Value& value)
    {
        if (!name.IsString())
        {
            return true;
        }

        bool has = false;
        std::string key = name.GetString();
        if (m_sorted)
        {
            has = std::binary_search(m_keys.begin(), m_keys.end(), key);
        }
        else
        {
            has = std::find(m_keys.begin(), m_keys.end(), key) != m_keys.end();
        }

        return keep_bool(m_keep, has);
    }

private:
    const std::vector<std::string> m_keys;
    bool m_sorted;
    bool m_keep;
};

/* ************************************************************ */

int json_filter(rapidjson::Value& json, json_filter_fn fn)
{
    int count = 0;
    if (json.IsObject())
    {
        for (auto it = json.MemberBegin(); it != json.MemberEnd(); )
        {
            if (fn(it->name, it->value))
            {
                if (it->value.IsObject() || it->value.IsArray())
                {
                    count += json_filter(it->value, fn);
                }
                ++it;
            }
            else
            {
                // swap the last to current iterator, not ++ step forward
                it = json.RemoveMember(it);
                ++count;
            }
        }
    }
    else if (json.IsArray())
    {
        rapidjson::Value nullVal;
        for (auto it = json.Begin(); it != json.End(); )
        {
            if (fn(nullVal, *it))
            {
                if (it->IsObject() || it->IsArray())
                {
                    count += json_filter(*it, fn);
                }
                ++it;
            }
            else
            {
                // move the left forward, not ++ step
                it = json.Erase(it);
                ++count;
            }
        }
    }
    return count;
}

int filter_null(rapidjson::Value& json)
{
    return json_filter(json, filter_fn_null);
}

int filter_empty(rapidjson::Value& json)
{
    return json_filter(json, filter_fn_empty);
}

int filter_key(rapidjson::Value& json, const std::vector<std::string>& keys, bool keep/* = true*/)
{
    if (keys.size() >= 4)
    {
        std::vector<std::string> keysort(keys);
        std::sort(keysort.begin(), keysort.end());
        return filter_key_sorted(json, keysort, keep);
    }
    CFilterKey fn(keys, false, keep);
    return json_filter(json, fn);
}

int filter_key_sorted(rapidjson::Value& json, const std::vector<std::string>& keys, bool keep/* = true*/)
{
    CFilterKey fn(keys, true, keep);
    return json_filter(json, fn);
}

int filter_key(rapidjson::Value& json, const std::string& pattern, bool keep/* = true*/)
{
    std::regex exp(pattern);
    return json_filter(json, 
            [&exp, &keep](const rapidjson::Value& name, const rapidjson::Value& value)
            {
                if (!name.IsString())
                {
                    return true;
                }
                std::string key = name.GetString();
                bool match = std::regex_search(key, exp);
                return keep_bool(keep, match);
            });
}

/* ************************************************************ */
// Section: map

void map_replace(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator, json_map_fn fn)
{
    if (json.IsObject())
    {
        for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
        {
            if (it->value.IsObject() || it->value.IsArray())
            {
                map_replace(it->value, allocator, fn);
            }
            else
            {
                fn(it->name, it->value, allocator);
            }
        }
    }
    else if (json.IsArray())
    {
        rapidjson::Value nullName;
        for (auto it = json.Begin(); it != json.End(); ++it)
        {
            if (it->IsObject() || it->IsArray())
            {
                map_replace(*it, allocator, fn);
            }
            else
            {
                fn(nullName, *it, allocator);
            }
        }
    }
    else
    {
        rapidjson::Value nullName;
        fn(nullName, json, allocator);
    }
}

void map_to_string(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
{
    map_replace(json, allocator,
            [](rapidjson::Value& name, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
            {
                if (!value.IsString())
                {
                    std::string str = stringfy(value);
                    value.SetString(str.c_str(), str.size(), allocator);
                }
            }
        );
}

void map_fn_decode_json(rapidjson::Value& name, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
{
    if (!value.IsString() || value.GetStringLength() < 2)
    {
        return;
    }
    const char* str = value.GetString();
    const char b = str[0];
    const char e = str[value.GetStringLength()-1];

    if ((b == '{' && e == '}') || (b == '[' && e == ']'))
    {
        rapidjson::Document doc(&allocator);
        if (read_string(doc, value.GetString(), value.GetStringLength()))
        {
            value = doc.Move();
        }
    }
}

void map_decode_json(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
{
    map_replace(json, allocator, map_fn_decode_json);
}

} /* jsonkit */ 
