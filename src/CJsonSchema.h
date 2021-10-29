/** 
 * @file CJsonSchema.h
 * @author lymslive
 * @date 2021-10-27 
 * @brief provid basic json schema support class from rapidjson
 * */
#ifndef CJSONSCHEMA_H__
#define CJSONSCHEMA_H__

#include "rapidjson/document.h"
#include "rapidjson/schema.h"

namespace jsonkit
{

/// json schema support
class CJsonSchema
{
public:
    /** constructor with explict IRemoteSchemaDocumentProvider
     * @param doc
     * @param provider
     * @note 
     * The passed custome provider, it's ownership move to this and delete on destroy.
     * Can ommit if no need to use provider (schema $ref)
     * @note rapidjson::SchemaDocument can also construct from Value, not
     * Document
     * */
    CJsonSchema(const rapidjson::Value& doc, rapidjson::IRemoteSchemaDocumentProvider* provider = NULL);

    /// pass base directory to use local schema provider internlly
    CJsonSchema(const rapidjson::Value& doc, const std::string& baseDir);
    ~CJsonSchema();

    /// validate json against this schema, may pass out error string
    bool Validate(const rapidjson::Value& json);
    bool Validate(const rapidjson::Value& json, std::string& strError);

private:
    rapidjson::IRemoteSchemaDocumentProvider* m_provider;
    rapidjson::SchemaDocument m_schema;
    rapidjson::SchemaValidator m_validator;
};

} // end of namespace jsonkit

#endif /* end of include guard: CJSONSCHEMA_H__ */
