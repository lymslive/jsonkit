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
    // perform the transform, return the count of node that have transformed
    bool doTransform (const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

    // transform a unit dst when it is string
    // return true if transform actually take effect, otherwise false
    bool itemTransform(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

    // expand dst to array of array or array of object, which wild `?` in path
    bool expandArray(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);
};

bool CPathFiller::itemTransform(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    const char* pszVal = dst.GetString();
    if (!pszVal || *pszVal == '\0')
    {
        return false;
    }
    char leader = pszVal[0];
    if (leader == '/')
    {
        // "dst": "/path/in/src"
        const char* path = pszVal;
        auto& newVal = src/path;
        if (!newVal)
        {
            LOGF("can not find `src` json path: %s", path);
            dst.SetNull();
            return false;
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
            if (pSlash)
                // "dst": "=slotName/path/to/src"
            {
                fnSlot(src/pSlash, dst, allocator);
            }
            else
            {
                // "dst": "=slotName"
                fnSlot(src, dst, allocator);
            }
        }
        else
        {
            LOGF("no valid slot function registed: %s", slotName.c_str());
            return false;
        }
    }
    return true;
}

bool CPathFiller::doTransform(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    int count = 0;
    if (dst.IsString())
    {
        return itemTransform(src, dst, allocator);
    }
    else if (dst.IsObject() && !dst.ObjectEmpty())
    {
        for (auto it = dst.MemberBegin(); it != dst.MemberEnd(); ++it)
        {
            if (false == doTransform(src, it->value, allocator)) 
            {
                ++count;
                // tolerate when not expand array
                // return false;
            }
        }
    }
    else if (dst.IsArray() && !dst.Empty())
    {
        if (dst[0].IsString() && dst.Size() == 2)
        {
            const char* first = dst[0].GetString();
            if (0 == strcmp(first, "=[?]") && dst[1].IsArray())
            {
                // "dst": ["=[?]", [array template]]
                return expandArray(src, dst, allocator);
            }
            else if (0 == strcmp(first, "={?}") && dst[1].IsObject())
            {
                // "dst": ["={?}", {object template}]
                return expandArray(src, dst, allocator);
            }
        }

        for (auto it = dst.Begin(); it != dst.End(); ++it)
        {
            if (false == doTransform(src, *it, allocator))
            {
                ++count;
                // return false;
            }
        }
    }
    return count == 0;
}

namespace helper
{
// "some string with ?" replace '?' with index
// return the count of `?` replaced
int replace_index(int index, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    if (!dst.IsString() || 0 == dst.GetStringLength())
    {
        return 0;
    }
    char* begin = const_cast<char*>(dst.GetString());
    char* pos = strchr(begin, '?');
    if (pos == nullptr)
    {
        return 0;
    }

    int count = 0;
    if (index >= 0 && index <= 9)
    {
        // modify string in-place, as only modify one byte
        do
        {
            *pos = '0' + index;
            pos = strchr(pos, '?');
            ++count;
        } while(pos != nullptr);
    }
    else
    {
        std::string str;
        std::string val = std::to_string(index);
        do
        {
            str.append(begin, pos);
            str.append(val);
            begin = pos + 1;
            pos = strchr(begin, '?');
            ++count;
        } while(pos != nullptr);
        if (*begin != '\0')
        {
            str.append(begin);
        }
        dst.SetString(str.c_str(), str.size(), allocator);
    }
    return count;
}

} /* namespace helper */ 

bool CPathFiller::expandArray(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    auto& base = dst[1];
    dst.SetArray();
    if (base.IsArray() && base.Empty()
            || base.IsObject() && base.ObjectEmpty())
    {
        return false;
    }

    int index = 0;
    rapidjson::Value row;
    while(1)
    {
        row.CopyFrom(base, allocator);
        int replace_count = 0;
        if (row.IsArray())
        {
            for (auto it = row.Begin(); it != row.End(); ++it)
            {
                replace_count += helper::replace_index(index, *it, allocator);
            }
        }
        else if (row.IsObject())
        {
            for (auto it = row.MemberBegin(); it != row.MemberEnd(); ++it)
            {
                replace_count += helper::replace_index(index, it->value, allocator);
            }
        }
        if (replace_count <= 0)
        {
            // simply protect not indefinite loop
            return false;
        }

        // LOGF("before transform row: %s", jsonkit::stringfy(row).c_str());
        if (false == doTransform(src, row, allocator))
        {
            break;
        }
        // LOGF("after transform row: %s", jsonkit::stringfy(row).c_str());
        dst.PushBack(row, allocator);
        ++index;
    }

    return index > 0;
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
