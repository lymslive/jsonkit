#ifndef JSONKIT_PLAIN_H__
#define JSONKIT_PLAIN_H__

#include <string>

// The plain interface treat a string as json, that indepent on the underlying
// json library. The input json can be std::string type or const char* with
// its strlen.
// Usually the input and output by argument, while the return value of the
// function is bool to indicate whether the operation success or occurs error.
namespace jsonkit
{

// print json in pretty or condensed format
bool prettify(const std::string& inJson, std::string& outJson);
bool condense(const std::string& inJson, std::string& outJson);
bool prettify(const char* inJson, size_t inLen, std::string& outJson);
bool condense(const char* inJson, size_t inLen, std::string& outJson);

// generate schema from sample json and generate sample json from schema,
// these two operation may not be reversiable.
bool form_schema(const std::string& inJson, std::string& outSchema);
bool from_schema(const std::string& inSchema, std::string& outJson);
bool form_schema(const char* inJson, size_t inLen, std::string& outSchema);
bool from_schema(const char* inSchema, size_t inLen, std::string& outJson);

// validate the json according to schema
// inJson: josn string 
// inSchema: josn string that serve as schema
// basedir: is for remote reference
// inSchemaFile: read schema from a json file, and remote schema refer to its basedir
// Note: not provide (char* and length) argument, as to many input arguments
bool validate_schema(const std::string& inJson, const std::string& inSchema);
bool validate_schema(const std::string& inJson, const std::string& inSchema, const std::string& basedir);
bool validate_schema_file(const std::string& inJson, const std::string& inSchemaFile);

// return true if two json is equal in type and field/itme recursively
bool compare(const std::string& aJson, const std::string& bJson);
// return ture if a >= b, a could has more field/item than b in object/array
bool compatible(const std::string& aJson, const std::string& bJson);

// get one sub-node from json by pointer path
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
