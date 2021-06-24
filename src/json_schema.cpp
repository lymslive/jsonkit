#include "jsonkit_plain.h"
#include "jsonkit_rpdjn.h"

#include "rapidjson/document.h"
#include "rapidjson/schema.h"

#include <string>
#include <vector>
#include <map>

namespace jsonkit
{
namespace impl
{
// BEG namespace jsonkit::impl

// END namespace jsonkit::impl
}
}

namespace jsonkit
{

// local file schema provider
class CSchemaProvider : public rapidjson::IRemoteSchemaDocumentProvider
{
public:
    // virtual const rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, size_t length);
    virtual const rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, rapidjson::SizeType length);

    void SetBaseDir(const std::string& baseDir)
    {
        m_baseDir = baseDir;
    }

    CSchemaProvider(){}
    ~CSchemaProvider()
    {
        for (auto& item : m_mapSchemas)
        {
            if (item.second != nullptr)
            {
                delete item.second;
                item.second = nullptr;
            }
        }
    }

private:
    std::string m_baseDir;
    std::map<std::string, rapidjson::SchemaDocument*>m_mapSchemas;
};

static
bool BreakString(std::string& source, const char* any, std::string& rest)
{
    if (source.empty())
    {
        return false;
    }

    size_t pos = source.find_first_of(any);
    if (pos != std::string::npos)
    {
        rest = source.substr(pos + 1);
        source.erase(pos);
        return true;
    }

    return false;
}

const rapidjson::SchemaDocument* CSchemaProvider::GetRemoteDocument(const char* uri, rapidjson::SizeType length)
{
    // fprintf(stderr, "Try to get remote schema refer: %s\n", uri);
    std::string uriKey(uri);
    std::string jsonPath;
    BreakString(uriKey, "#", jsonPath);

    auto it = m_mapSchemas.find(uriKey);
    if (it != m_mapSchemas.end())
    {
        // fprintf(stderr, "Get the cached schema!\n");
        return it->second;
    }

    std::string jsonFile = m_baseDir + uriKey;
    rapidjson::Document docSchema;
    int iRet = jsonkit::read_file(docSchema, jsonFile);
    if (iRet != 0)
    {
        fprintf(stderr, "Fail to read schema json file: %s\n", jsonFile.c_str());
        return nullptr;
    }

    // the required is the whole SchemaDocument, not further parse #path or #id self.
    rapidjson::SchemaDocument* pSchema = new rapidjson::SchemaDocument(docSchema);
    if (pSchema != nullptr)
    {
        m_mapSchemas[uriKey] = pSchema;
        // fprintf(stderr, "Save cache for schema json file: %s\n", uriKey.c_str());
    }
    else
    {
        fprintf(stderr, "Fail to create new rapidjson::SchemaDocument\n");
    }

    return pSchema;
}

} // end of namespace jsonkit

// interface functions
namespace jsonkit
{

bool form_schema(const rapidjson::Value& inJson, rapidjson::Document& outSchema);
bool from_schema(const rapidjson::Value& inSchema, rapidjson::Document& outJson);

// do not support remote schema (IRemoteSchemaDocumentProvider)
bool validate_schema(const rapidjson::Value& inJson, const rapidjson::Document& inSchema)
{
    // rapidjson::SchemaDocument sd(inSchema, 0, 0, NULL);
    rapidjson::SchemaDocument sd(inSchema);
    rapidjson::SchemaValidator sv(sd);

    bool bResult = inJson.Accept(sv)
    if (!bResult)
    {
        rapidjson::StringBuffer sb;
        sv.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        fprintf(stderr, "Invalid schema: %s\n", sb.GetString());
        fprintf(stderr, "Invalid keyword: %s\n", m_pSchemaValidator->GetInvalidSchemaKeyword());
        sb.Clear();
        sv.pSchemaValidator->GetInvalidDocumentPointer().StringifyUriFragment(sb);
        fprintf(stderr, "Invalid document: %s\n", sb.GetString());
    }

    return bResult;
}

bool form_schema(const std::string& inJson, std::string& outSchema)
{
    rapidjson::Document docJson;
    docJson.Parse(inJson.c_str(), inJson.size());
    if (docJson.HasParseError())
    {
        return false;
    }

    rapidjson::Document docSchema;
    form_schema(docJson, docSchema);
    return condesen(docSchema, outSchema);
}

bool from_schema(const std::string& inSchema, std::string& outJson)
{
    rapidjson::Document docSchema;
    docSchema.Parse(inSchema.c_str(), inSchema.size());
    if (docSchema.HasParseError())
    {
        return false;
    }

    rapidjson::Document docJson;
    from_schema(docSchema, docJson);
    return condesen(docJson, outJson);
}

bool form_schema(const char* inJson, size_t inLen, std::string& outSchema)
{
    rapidjson::Document docJson;
    docJson.Parse(inJson, inLen);
    if (docJson.HasParseError())
    {
        return false;
    }

    rapidjson::Document docSchema;
    form_schema(docJson, docSchema);
    return condesen(docSchema, outSchema);
}

bool from_schema(const char* inSchema, size_t inLen, std::string& outJson)
{
    rapidjson::Document docSchema;
    docSchema.Parse(inSchema, inLen);
    if (docSchema.HasParseError())
    {
        return false;
    }

    rapidjson::Document docJson;
    from_schema(docSchema, docJson);
    return condesen(docJson, outJson);
}

// validate the json according to schema
bool validate_schema(const std::string& inJson, const std::string& inSchema)
{
    rapidjson::Document docJson;
    docJson.Parse(inJson.c_str(), inJson.size());
    if (docJson.HasParseError())
    {
        return false;
    }

    rapidjson::Document docSchema;
    docSchema.Parse(inSchema.c_str(), inSchema.size());
    if (docSchema.HasParseError())
    {
        return false;
    }

    return validate_schema(docJson, docSchema);
}

} /* jsonkit */ 
