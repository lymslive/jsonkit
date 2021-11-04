#include "json_transform.h"

#include <map>

#include "json_operator.h"

namespace jsonkit
{
    
static
std::map<std::string, json_slot_fn> s_mapSlot;

void slot_register(const std::string& name, json_slot_fn slot)
{
    s_mapSlot.insert(std::make_pair(name, slot));
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
                    if (canMove)
                    {
                        // it->value = newVal;
                    }
                    else
                    {
                        it->value.CopyFrom(newVal, allocator);
                    }
                }
                else
                {
                    it->value.SetNull();
                    if (removeNull)
                    {
                        vecRemoveKey.push_back(pszKey);
                    }
                }
            }
            else if (leader == '=')
            {
                auto itSlot = s_mapSlot.find(pszVal+1);
                if (itSlot != s_mapSlot.end())
                {
                    (itSlot->second)(src, it->value, allocator);
                }
                else
                {
                    // leave it not changed, notice the strange result
                    // it->value.SetNull();
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
