/** 
 * @file jsonkit_plain.h 
 * @author lymslive
 * @date 2021-10-27 
 * @details 
 * Interface by rapidjson library, where input is rapidjson::Value, or
 * rapidjson::Document in some case.
 * */
#ifndef JSONKIT_RPDJN_H__
#define JSONKIT_RPDJN_H__

#include "rapidjson/document.h"

#include "use_rapidjson.h"

namespace jsonkit
{

/// print json in pretty or condensed one-line format
bool prettify(const rapidjson::Value& inJson, std::string& outJson);
bool condense(const rapidjson::Value& inJson, std::string& outJson);

/// stringfy json value in condensed format
inline
void stringfy(const rapidjson::Value& json, std::string& dest)
{
    condense(json, dest);
}
inline
std::string stringfy(const rapidjson::Value& json)
{
    std::string dest;
    condense(json, dest);
    return dest;
}

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

/// return true if two json is equal in type and field/itme recursively
bool compare(const rapidjson::Value& aJson, const rapidjson::Value& bJson);
/// return ture if a >= b, a could has more field/item than b in object/array
bool compatible(const rapidjson::Value& aJson, const rapidjson::Value& bJson);

/** get one sub-node from json by pointer path 
 * @param inJson: a json value
 * @param path: path in json dmo based from inJson
 * @return pointer to the sub-node, for null on failure
 * */
const rapidjson::Value* point(const rapidjson::Value& inJson, const std::string& path);

#ifdef HAS_GOOGLE_PROBUF
// convert between json and protobuf
// the protobuf name must be provided as extra input argument. 
bool form_protobuf(const std::string& name, const rapidjson::Value& inJson, std::string& outPbmsg);
bool from_protobuf(const std::string& name, const rapidjson::Value& inPbmsg, std::string& outJson);
#endif

} /* jsonkit */ 

#endif /* end of include guard: JSONKIT_RPDJN_H__ */
