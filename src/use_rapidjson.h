#ifndef USE_RAPIDJSON_H__
#define USE_RAPIDJSON_H__

#include <string>
#include <iostream>

namespace jsonkit
{
    
bool read_stream(rapidjson::Document& doc, std::istream& stream);
bool read_file(rapidjson::Document& doc, const std::string& file);
bool write_stream(const rapidjson::Value& doc, std::ostream& stream, bool pretty = false);
bool write_file(const rapidjson::Value& doc, const std::string& file, bool pretty = false);

} /* jsonkit */ 

#endif /* end of include guard: USE_RAPIDJSON_H__ */
