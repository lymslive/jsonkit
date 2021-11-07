/** 
 * @file json_compare.h
 * @author lymslive
 * @date 2021-11-07
 * @brief functionality about json comparison
 * */
#ifndef JSON_COMPARE_H__
#define JSON_COMPARE_H__

#include "rapidjson/document.h"

namespace jsonkit
{
    
/// return true if two json is equal in type and field/itme recursively
bool compare(const rapidjson::Value& aJson, const rapidjson::Value& bJson);
/// return ture if a >= b, a could has more field/item than b in object/array
bool compatible(const rapidjson::Value& aJson, const rapidjson::Value& bJson);

} /* jsonkit */ 

#endif /* end of include guard: JSON_COMPARE_H__ */
