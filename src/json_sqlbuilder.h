/** 
 * @file json_sqlbuilder.h 
 * @author lymslive
 * @date 2021-10-30
 * */

#ifndef JSON_SQLBUILDER_H__
#define JSON_SQLBUILDER_H__

#include "rapidjson/document.h"

namespace jsonkit
{
    
/** build sql statement from specific json struct */
bool sql_insert(const rapidjson::Value& json, std::string& sql);
bool sql_update(const rapidjson::Value& json, std::string& sql);
bool sql_select(const rapidjson::Value& json, std::string& sql);
bool sql_count(const rapidjson::Value& json, std::string& sql);
bool sql_delete(const rapidjson::Value& json, std::string& sql);

} /* jsonkit */ 

#endif /* end of include guard: JSON_SQLBUILDER_H__ */
