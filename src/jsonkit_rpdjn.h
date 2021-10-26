#ifndef JSONKIT_RPDJN_H__
#define JSONKIT_RPDJN_H__

#include "rapidjson/document.h"

#include "use_rapidjson.h"

// Interface by rapidjson library, where input is rapidjson::Value, or
// rapidjson::Document in some case.
namespace jsonkit
{

// print json in pretty or condensed format
bool prettify(const rapidjson::Value& inJson, std::string& outJson);
bool condense(const rapidjson::Value& inJson, std::string& outJson);

// generate schema from sample json and generate sample json from schema,
// these two operation may not be reversiable.
bool form_schema(const rapidjson::Value& inJson, rapidjson::Document& outSchema);
bool from_schema(const rapidjson::Value& inSchema, rapidjson::Document& outJson);

// validate the json according to schema
bool validate_schema(const rapidjson::Value& inJson, const rapidjson::Document& inSchema);
bool validate_schema(const rapidjson::Value& inJson, const rapidjson::Document& inSchema, const std::string& basedir);

// return true if two json is equal in type and field/itme recursively
bool compare(const rapidjson::Value& aJson, const rapidjson::Value& bJson);
// return ture if a >= b, a could has more field/item than b in object/array
bool compatible(const rapidjson::Value& aJson, const rapidjson::Value& bJson);

// get one sub-node from json by pointer path
// return the json value pointer or NULL on failure.
const rapidjson::Value* point(const rapidjson::Value& inJson, const std::string& path);

#ifdef HAS_GOOGLE_PROBUF
// convert between json and protobuf
// the protobuf name must be provided as extra input argument. 
bool form_protobuf(const std::string& name, const rapidjson::Value& inJson, std::string& outPbmsg);
bool from_protobuf(const std::string& name, const rapidjson::Value& inPbmsg, std::string& outJson);
#endif

} /* jsonkit */ 

#endif /* end of include guard: JSONKIT_RPDJN_H__ */
