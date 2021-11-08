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

// collection of headers for each json topic
#include "json_input.h"
#include "json_output.h"
#include "json_schema.h"
#include "json_compare.h"

namespace jsonkit
{

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
