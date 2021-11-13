/** 
 * @file json_path.cpp
 * @author lymslive
 * @date 2021-10-27
 * @brief json pointer path related uitlity.
 * */
#include "jsonkit_rpdjn.h"
#include "jsonkit_internal.h"

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

namespace jsonkit
{
    
const rapidjson::Value* path_point(const rapidjson::Value& inJson, const std::string& path)
{
    if (path.empty())
    {
        return nullptr;
    }

    if (path[0] == '/')
    {
        rapidjson::Pointer jpath(path.c_str(), path.size());
        return jpath.Get(inJson);
    }
    else
    {
        // make sure path start with a slash '/'
        return path_point(inJson, std::string("/") + path);
    }
}

bool path_attach(rapidjson::Value& node, const char* path, rapidjson::Value& tree, rapidjson::Document::AllocatorType& allocator)
{
    std::string strBuffer(path);
    char* head = const_cast<char*>(strBuffer.c_str());
    if (*head == '/')
    {
        ++head;
    }

    rapidjson::Value* branch = &tree;

    char* next = strchr(head, '/');
    while (next && branch)
    {
        // split string in place
        *next = '\0';
        if (branch->IsObject())
        {
            auto it = branch->FindMember(head);
            if (it != branch->MemberEnd())
            {
                branch = &(it->value);
            }
            else
            {
                rapidjson::Value key;
                key.SetString(head, allocator);
                rapidjson::Value tmp;
                tmp.SetObject();
                branch->AddMember(key, tmp, allocator);
                auto reit = branch->FindMember(head);
                if (reit != branch->MemberEnd())
                {
                    branch = &(reit->value);
                }
                else
                {
                    branch = nullptr;
                }
            }
        }
        // else if (branch.IsArray())
        // {
            // branch = nullptr;
        // }
        else
        {
            branch = nullptr;
        }

        // restore string
        *next = '/';
        head = next + 1;
        next = strchr(head, '/');
    }

    if (*head == '\0')
    {
        return false;
    }

    if (branch == nullptr || !branch->IsObject())
    {
        return false;
    }

    if (branch->HasMember(head))
    {
        LOGF("attach fail, alread has member: %s", head);
        return false;
    }

    rapidjson::Value key;
    key.SetString(head, allocator);
    branch->AddMember(key, node, allocator);
    return true;
}

} /* jsonkit */ 
