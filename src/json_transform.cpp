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

} /* jsonkit */ 
