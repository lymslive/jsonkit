#ifndef JSONKIT_PLAIN_H__
#define JSONKIT_PLAIN_H__

#include "rapidjson/document.h"

#include "jsonkit_plain.h"

// Interface by rapidjson library, where input is rapidjson::Value, or
// rapidjson::Document in some case.
namespace jsonkit
{

// print json in pretty or condensed style
bool prettify(const rapidjson::Value& inJson, std::string& outJson);
bool condense(const rapidjson::Value& inJson, std::string& outJson);

// generate schema from sample json and generate sample json from schema,
// these two operation may not be reversiable.
bool form_schema(const rapidjson::Value& inJson, rapidjson::Document& outSchema);
bool from_schema(const rapidjson::Value& inSchema, rapidjson::Document& outJson);

// validate the json according to schema
bool validate_schema(const rapidjson::Value& inJson, rapidjson::Document& inSchema);

// some like strcmp() that return 0 or 1 or -1, but perform on two json values.
int compare(const rapidjson::Value& aJson, const rapidjson::Value& bJson);

// get one sub-node from json by pointer path
// return the json value pointer or NULL on failure.
rapidjson::Value* point(const rapidjson::Value& inJson, const std::string& path);

#ifdef HAS_GOOGLE_PROBUF
// convert between json and protobuf
// the protobuf name must be provided as extra input argument. 
bool form_protobuf(const std::string& name, const rapidjson::Value& inJson, std::string& outPbmsg);
bool from_protobuf(const std::string& name, const rapidjson::Value& inPbmsg, std::string& outJson);
#endif

} /* jsonkit */ 

#endif /* end of include guard: JSONKIT_PLAIN_H__ */
