#ifndef CJSONSCHEMA_H__
#define CJSONSCHEMA_H__

#include "rapidjson/document.h"
#include "rapidjson/schema.h"

namespace jsonkit
{

class CJsonSchema
{
public:
    // explict pass custome provider create by new, 
    // it's ownership move to this and delete on destroy
    // can ommit if no need to use provider (schema $ref)
    CJsonSchema(const rapidjson::Document& doc, rapidjson::IRemoteSchemaDocumentProvider* provider = NULL);
    // pass base directory to use local schema provider internlly
    CJsonSchema(const rapidjson::Document& doc, const std::string& baseDir);
    ~CJsonSchema();

    // validate json against this schema, may pass out error string
    bool Validate(const rapidjson::Value& json);
    bool Validate(const rapidjson::Value& json, std::string& strError);

private:
    rapidjson::IRemoteSchemaDocumentProvider* m_provider;
    rapidjson::SchemaDocument m_schema;
    rapidjson::SchemaValidator m_validator;
};

} // end of namespace jsonkit

#endif /* end of include guard: CJSONSCHEMA_H__ */
