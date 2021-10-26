#ifndef USE_RAPIDJSON_H__
#define USE_RAPIDJSON_H__

#include <string>
#include <iostream>

#include "rapidjson/document.h"

namespace jsonkit
{
    
bool read_stream(rapidjson::Document& doc, std::istream& stream);
bool read_file(rapidjson::Document& doc, const std::string& file);
bool write_stream(const rapidjson::Value& doc, std::ostream& stream, bool pretty = false);
bool write_file(const rapidjson::Value& doc, const std::string& file, bool pretty = false);

/**************************************************************/

bool scalar_value(std::string& dest, const rapidjson::Value& json);
bool scalar_value(int& dest, const rapidjson::Value& json);
bool scalar_value(double& dest, const rapidjson::Value& json);
bool scalar_value(bool& dest, const rapidjson::Value& json);

} /* jsonkit */ 

#endif /* end of include guard: USE_RAPIDJSON_H__ */
