#include "CJsonSchema.h"
#include "use_rapidjson.h"
#include "jsonkit_internal.h"

#include <string>
#include <map>

namespace jsonkit
{

// schema provider refer to local filesystem
class CSchemaProvider : public rapidjson::IRemoteSchemaDocumentProvider
{
public:
    CSchemaProvider(const std::string& baseDir);
    ~CSchemaProvider();

public:
	// virtual const rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, size_t length);
	virtual const rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, rapidjson::SizeType length);

private:
	std::string m_baseDir;
	std::map<std::string, rapidjson::SchemaDocument*> m_mapSchema;
};

CSchemaProvider::CSchemaProvider(const std::string& baseDir)
    : m_baseDir(baseDir)
{
    size_t iend = m_baseDir.size();
    if (iend > 0)
    {
        if (m_baseDir[iend] != '/' && m_baseDir[iend] != '\\')
        {
            m_baseDir.push_back('/');
        }
    }
}

CSchemaProvider::~CSchemaProvider()
{
    std::map<std::string, rapidjson::SchemaDocument*>::iterator it;
    for (it = m_mapSchema.begin(); it != m_mapSchema.end(); ++it)
    {
        if (it->second != NULL)
        {
            delete it->second;
            it->second = NULL;
        }
    }
}

static
bool break_string(std::string& source, const char* any, std::string& rest)
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
    LOGD("Try to get remote schema refer: %s", uri);
    std::string uriKey(uri);
    std::string jsonPath;
    break_string(uriKey, "#", jsonPath);

    auto it = m_mapSchema.find(uriKey);
    if (it != m_mapSchema.end())
    {
        LOGD("Get the cached schema!");
        return it->second;
    }

    std::string jsonFile = m_baseDir;
    jsonFile.append(uriKey);

    rapidjson::Document docSchema;
    if (!jsonkit::read_file(docSchema, jsonFile))
    {
        LOGF("Fail to read schema json file: %s", jsonFile.c_str());
        return NULL;
    }

    // the required is the whole SchemaDocument, not further parse #path or #id self.
    rapidjson::SchemaDocument* pSchema = new rapidjson::SchemaDocument(docSchema);
    if (pSchema != NULL)
    {
        m_mapSchema[uriKey] = pSchema;
        LOGD("Save cache for schema json file: %s", uriKey.c_str());
    }
    else
    {
        LOGF("Fail to create new rapidjson::SchemaDocument");
    }

    return pSchema;
}

} // end of namespace jsonkit::CSchemaProvider

namespace jsonkit
{

CJsonSchema::CJsonSchema(const rapidjson::Document& doc, rapidjson::IRemoteSchemaDocumentProvider* provider)
    : m_provider(provider)
    , m_schema(doc, 0, 0, m_provider)
    , m_validator(m_schema)
{
}

CJsonSchema::CJsonSchema(const rapidjson::Document& doc, const std::string& baseDir)
    : m_provider(new CSchemaProvider(baseDir))
    , m_schema(doc, 0, 0, m_provider)
    , m_validator(m_schema)
{
}

CJsonSchema::~CJsonSchema()
{
    if (m_provider)
    {
        delete m_provider;
        m_provider = NULL;
    }
}

bool CJsonSchema::Validate(const rapidjson::Value& json)
{
    std::string strError;
    bool bRet = Validate(json, strError);
    LOGS(strError);
    return bRet;
}

bool CJsonSchema::Validate(const rapidjson::Value& json, std::string& strError)
{
    m_validator.Reset();
    bool bRet = json.Accept(m_validator);
    if (!bRet)
    {
        rapidjson::StringBuffer sb;
        m_validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        strError.append("Invalid schema: ").append(sb.GetString()).append("\n");

        strError.append("Invalid keyword: ").append(m_validator.GetInvalidSchemaKeyword()).append("\n");

        sb.Clear();
        m_validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        strError.append("Invalid document: ").append(sb.GetString());
    }

    return bRet;
}

} // end of namespace jsonkit
