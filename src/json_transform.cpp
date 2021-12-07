#include "json_transform.h"
#include "jsonkit_internal.h"

#include <map>

#include "json_operator.h"

namespace jsonkit
{
    
namespace slot
{

// pre-defined slot function:
void fn_stringfy(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)   
{
    std::string str = jsonkit::stringfy(src);
    dst.SetString(str.c_str(), str.size(), allocator);
}

void fn_sizefy(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    if (src.IsArray())
    {
        dst = src.Size();
    }
    else if (src.IsString())
    {
        dst = src.GetStringLength();
    }
    else if (src.IsObject())
    {
        dst = src.MemberCount();
    }
    else
    {
        dst = 0;
    }
}

class CManager
{
    std::map<std::string, json_slot_fn> m_mapSlot;

public:
    static CManager& Instance()
    {
        static CManager s_instance;
        return s_instance;
    }

    CManager() { PreDefinedSlot(); }

    void PreDefinedSlot()
    {
        m_mapSlot["JSON_STRINGFY"] = fn_stringfy;
        m_mapSlot["JSON_SIZEFY"] = fn_sizefy;
    }

    void RegisterSlot(const std::string& name, json_slot_fn slot)
    {
        m_mapSlot[name] = slot;
    }

    json_slot_fn RetrieveSlot(const std::string& name)
    {
        return m_mapSlot[name];
    }
};

} /* namespace slot */ 

// static std::map<std::string, json_slot_fn> s_mapSlot;

void slot_register(const std::string& name, json_slot_fn slot)
{
    slot::CManager::Instance().RegisterSlot(name, slot);
}

inline
json_slot_fn slot_retrieve(const std::string& name)
{
    return slot::CManager::Instance().RetrieveSlot(name);
}

/** fill template json dst, with some value form src by path */
class CPathFiller : public CTransform
{
public:
    void operator() (const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
    {
        doTransform(src, dst, allocator);
    }

private:
    void doTransform (const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);
};

void CPathFiller::doTransform (const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    if (dst.IsString())
    {
        const char* pszVal = dst.GetString();
        if (!pszVal || *pszVal == '\0')
        {
            return;
        }
        char leader = pszVal[0];
        if (leader == '/')
        {
            const char* path = pszVal;
            auto& newVal = src/path;
            if (!newVal)
            {
                LOGF("can not find `src` json path: %s", path);
                dst.SetNull();
            }
            else
            {
                dst.CopyFrom(newVal, allocator);
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
            auto fnSlot = slot_retrieve(slotName);
            if (fnSlot)
            {
                // "it->value": "=slotName/path/to/src"
                if (pSlash)
                {
                    fnSlot(src/pSlash, dst, allocator);
                }
                else
                {
                    fnSlot(src, dst, allocator);
                }
            }
            else
            {
                LOGF("no valid slot function registed: %s", slotName.c_str());
            }
        }
    }
    else if (dst.IsObject() && !dst.ObjectEmpty())
    {
        for (auto it = dst.MemberBegin(); it != dst.MemberEnd(); ++it)
        {
            doTransform(src, it->value, allocator);
        }
    }
    else if (dst.IsArray() && !dst.Empty())
    {
        for (auto it = dst.Begin(); it != dst.End(); ++it)
        {
            doTransform(src, *it, allocator);
        }
    }
}

void merge(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    // todo:
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
