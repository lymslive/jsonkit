/** 
 * @file jsonkit_plain.h 
 * @author lymslive
 * @date 2021-10-27 
 * @details 
 * The plain interface treat a string as json, that indepent on the underlying
 * json library. The input json can be std::string type or const char* with
 * its strlen.
 * Usually the input and output by argument, while the return value of the
 * function is bool to indicate whether the operation success or occurs error.
 * */
#ifndef JSONKIT_PLAIN_H__
#define JSONKIT_PLAIN_H__

#include <string>

namespace jsonkit
{

/// print json in pretty or condensed one-line format
bool prettify(const std::string& inJson, std::string& outJson);
bool condense(const std::string& inJson, std::string& outJson);
bool prettify(const char* inJson, size_t inLen, std::string& outJson);
bool condense(const char* inJson, size_t inLen, std::string& outJson);

/** generate schema from sample json and generate sample json from schema.
 * @note these two operation may not be reversiable.
 * */
bool form_schema(const std::string& inJson, std::string& outSchema);
bool from_schema(const std::string& inSchema, std::string& outJson);
bool form_schema(const char* inJson, size_t inLen, std::string& outSchema);
bool from_schema(const char* inSchema, size_t inLen, std::string& outJson);

/** validate the json according to schema
 * @param inJson: josn string 
 * @param inSchema: josn string that serve as schema
 * @param basedir: is for remote reference in schema
 * @param inSchemaFile: read schema from a json file, and remote schema refer to its basedir
 * @return bool
 * @retval true valid json agaist the schema
 * @retval false not valid
 * @note not provide (char* and length) argument, as to many input arguments
 * */
bool validate_schema(const std::string& inJson, const std::string& inSchema);
bool validate_schema(const std::string& inJson, const std::string& inSchema, const std::string& basedir);
bool validate_schema_file(const std::string& inJson, const std::string& inSchemaFile);

/// return true if two json is equal in type and field/itme recursively
bool compare(const std::string& aJson, const std::string& bJson);
/// return ture if a >= b, a could has more field/item than b in object/array
bool compatible(const std::string& aJson, const std::string& bJson);

/** get one sub-node from json by pointer path 
 * @param inJson: input json by std::string
 * @param inJson & inLen: input json by char* and length
 * @param path: path in json dmo
 * @param outJson: stringfy of the sub-node in json path
 * @return bool: true if json path is valid and write to outJson*/
bool point(const std::string& inJson, const std::string& path, std::string& outJson);
bool point(const char* inJson, size_t inLen, const std::string& path, std::string& outJson);

#ifdef HAS_GOOGLE_PROBUF
// convert between json and protobuf
// the protobuf name must be provided as extra input argument. 
bool form_protobuf(const std::string& name, const std::string& inJson, std::string& outPbmsg);
bool from_protobuf(const std::string& name, const std::string& inPbmsg, std::string& outJson);
#endif

} /* jsonkit */ 

#endif /* end of include guard: JSONKIT_PLAIN_H__ */
