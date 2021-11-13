/** 
 * @file json_schema.h
 * @author lymslive
 * @date 2021-11-07
 * @brief functionalit about json schema
 * */
#ifndef JSON_SCHEMA_H__
#define JSON_SCHEMA_H__

#include "rapidjson/document.h"

namespace jsonkit
{
    
/** generate schema from sample json and generate sample json from schema.
 * @note these two operation may not be reversiable.
 * */
bool form_schema(const rapidjson::Value& inJson, rapidjson::Document& outSchema);
bool from_schema(const rapidjson::Value& inSchema, rapidjson::Document& outJson);

/** validate the json according to schema
 * @param inJson: josn value 
 * @param inSchema: josn value/document as schema
 * @param basedir: is for remote reference in schema
 * @return bool
 * @retval true valid json agaist the schema
 * @retval false not valid
 * */
bool validate_schema(const rapidjson::Value& inJson, const rapidjson::Value& inSchema);
bool validate_schema(const rapidjson::Value& inJson, const rapidjson::Value& inSchema, const std::string& basedir);

/** validate json agaist one of non-standard schema
 * @param [IN] json, a json that must be object
 * @param [IN] schema, a json array where each item descibe one key that `inJson` should have
 * @param [OUT] error, optinally store the error message if fail
 * @return true if `inJson` satisfy `inScheam`, otherwise false.
 * @details The so called flat schema is like following:
 * @code
 * [
 *   {
 *     "name": "some key", // the key name
 *     "type": "number|string", // the type of key
 *     "required": true; // is the key requried or optional
 *     "children": [{}, {}, ...] // if type if object or array of object
 *   },
 *   ...
 * ]
 * @endcode
 * When nested with children, only support object or array of object now.
 * */
bool validate_flat_schema(const rapidjson::Value& json, const rapidjson::Value& schema);
bool validate_flat_schema(const rapidjson::Value& json, const rapidjson::Value& schema, std::string& error);

} /* jsonkit */ 

#endif /* end of include guard: JSON_SCHEMA_H__ */
