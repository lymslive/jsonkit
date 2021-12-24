/** 
 * @file json_output.h
 * @author lymslive
 * @date 2021-11-07
 * @brief output/serialize json in some format
 * */
#ifndef JSON_OUTPUT_H__
#define JSON_OUTPUT_H__

#include "rapidjson/document.h"

namespace jsonkit
{
    
bool write_stream(const rapidjson::Value& json, std::ostream& stream, bool pretty = false);
bool write_file(const rapidjson::Value& json, const std::string& file, bool pretty = false);

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

/// to_string much like stringfy but no extra "" for string type
inline
std::string to_string(const rapidjson::Value& json)
{
    return json.IsString() ? json.GetString() : stringfy(json);
}

} /* jsonkit */ 

#endif /* end of include guard: JSON_OUTPUT_H__ */
