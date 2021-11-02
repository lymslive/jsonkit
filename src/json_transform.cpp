#include "json_transform.h"

#include <map>

#include "json_operator.h"

namespace jsonkit
{
    
static
std::map<std::string, json_slot_fn> s_mapSlot;

void merge(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
}

void merge_advance(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    if (!dst.IsObject() || dst.ObjectEmpty())
    {
        return;
    }

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
                // newVal maybe null, ok to mean delete this key in dst
                it->value.CopyFrom(newVal, allocator);
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
                    it->value.SetNull();
                }
            }
        }
    }
}

void slot_register(const std::string& name, json_slot_fn slot)
{
    s_mapSlot.insert(std::make_pair(name, slot));
}

} /* jsonkit */ 
