#include "json_transform.h"
#include "jsonkit_internal.h"

#include <map>

#include "json_operator.h"

namespace jsonkit
{
    
static
std::map<std::string, json_slot_fn> s_mapSlot;

bool slot_register(const std::string& name, json_slot_fn slot)
{
    auto ret = s_mapSlot.insert(std::make_pair(name, slot));
    return ret.second;
}

void CPathFiller::operator() (const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    if (!dst.IsObject() || dst.ObjectEmpty())
    {
        return;
    }

    std::vector<const char*> vecRemoveKey;

    for (auto it = dst.MemberBegin(); it != dst.MemberEnd(); ++it)
    {
        const char* pszKey = it->name.GetString();
        if (it->value.IsString())
        {
            const char* pszVal = it->value.GetString();
            if (!pszVal)
            {
                continue;
            }
            char leader = pszVal[0];
            if (leader == '/')
            {
                const char* path = pszVal;
                auto& newVal = src/path;
                if (!newVal)
                {
                    LOGF("can not find `src` json path: %s", path);
                    it->value.SetNull();
                    if (removeNull)
                    {
                        vecRemoveKey.push_back(pszKey);
                    }
                }
                else
                {
                    if (canMove)
                    {
                        // it->value = newVal;
                    }
                    else
                    {
                        it->value.CopyFrom(newVal, allocator);
                    }
                }
            }
            else if (leader == '=')
            {
                std::string slotName;
                const char* pSlash = strchr(pszVal+1, '/');
                if (pSlash)
                {
                    slotName.assign(pszVal+1, pSlash);
                }
                else
                {
                    slotName.assign(pszVal+1);
                }
                auto itSlot = s_mapSlot.find(slotName);
                if (itSlot != s_mapSlot.end())
                {
                    // "it->value": "=slotName/path/to/src"
                    if (pSlash)
                    {
                        (itSlot->second)(src/pSlash, it->value, allocator);
                    }
                    else
                    {
                        (itSlot->second)(src, it->value, allocator);
                    }
                }
                else
                {
                    LOGF("no slot function registed: %s", slotName.c_str());
                }
            }
        }
        else if (it->value.IsObject())
        {
            // recursive into subtree
            (this->operator())(src, it->value, allocator);
        }
    }

    if (removeNull && !vecRemoveKey.empty())
    {
        for (auto& key : vecRemoveKey)
        {
            dst.RemoveMember(key);
        }
    }
}

void merge(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
}

void merge_fill(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    CPathFiller fill;
    return fill(src, dst, allocator);
}

static
void do_array2object(const std::string& key, rapidjson::Value& json,rapidjson::Document::AllocatorType& allocator)
{
    if (key.empty())
    {
        return;
    }

    if (json.IsArray())
    {
        rapidjson::Value object;
        object.SetObject();

        for (auto it = json.Begin(); it != json.End(); ++it)
        {
            if (!it->IsObject())
            {
                continue;
            }

            auto itKey = it->FindMember(rapidjson::Value(key.c_str(), key.size()));
            if (itKey == it->MemberEnd())
            {
                continue;
            }

            // save the json node with key
            rapidjson::Value jsKey, jsValue;
            jsKey = itKey->name;
            jsValue = itKey->value;

            // remove out the key
            it->RemoveMember(itKey);

            auto itNew = object.FindMember(jsValue);
            if (itNew != object.MemberEnd())
            {
                if (itNew->value.IsArray())
                {
                    itNew->value.PushBack(*it, allocator);
                }
            }
            else
            {
                size_t left = it->MemberCount();
                if (left > 1)
                {
                    // add array of shorter object item
                    rapidjson::Value array;
                    array.SetArray();
                    array.PushBack(*it, allocator);
                    object.AddMember(jsValue, array, allocator);
                }
                else if (left == 1)
                {
                    // add scalar
                    object.AddMember(jsValue, it->MemberBegin()->value, allocator);
                }
                else
                {
                    // add null
                    object.AddMember(jsValue, rapidjson::Value(), allocator);
                }
            }
        }

        // move the new built object to current json array
        json = object;
    }
    else if (json.IsObject())
    {
        // recursive deeper
        for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
        {
            do_array2object(key, it->value, allocator);
        }
    }
}

void array2object(const std::string& key, rapidjson::Value& json,rapidjson::Document::AllocatorType& allocator)
{
    size_t slash = key.find('/');
    if (slash == std::string::npos)
    {
        return do_array2object(key, json, allocator);
    }

    size_t head = 0;
    while (slash != std::string::npos)
    {
        std::string part = key.substr(head, slash - head);
        do_array2object(part, json, allocator);
        head = slash + 1;
        slash = key.find('/', head);
    }

    if (head < key.size())
    {
        std::string part = key.substr(head);
        do_array2object(part, json, allocator);
    }
}

void array2object(const std::vector<std::string>& key, rapidjson::Value& json,rapidjson::Document::AllocatorType& allocator)
{
    for (auto& item : key)
    {
        do_array2object(item, json, allocator);
    }
}

} /* jsonkit */ 
