#ifndef USE_RAPIDJSON_H__
#define USE_RAPIDJSON_H__

#include <string>
#include <iostream>

#include "rapidjson/document.h"

namespace jsonkit
{
    
bool read_stream(rapidjson::Document& doc, std::istream& stream);
bool read_file(rapidjson::Document& doc, const std::string& file);

} /* jsonkit */ 

#endif /* end of include guard: USE_RAPIDJSON_H__ */
