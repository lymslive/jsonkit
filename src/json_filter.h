/** 
 * @file json_filter.h
 * @author lymslive
 * @date 2021-11-13
 * @brief filter object memeber
 * */
#ifndef JSON_FILTER_H__
#define JSON_FILTER_H__

#include <functional>
#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace jsonkit
{
    
/** function type to determine whether a json member should be filtered 
 * @param name,  the member name, usually is json string when the parent json
 * node is object. Also can be null, if the parent is array.
 * @param value, the value
 * @return bool
 * @retval true, indicate the member should be reserved;
 * @retval false, indicate the member should be removed
 * */
typedef std::function<bool(const rapidjson::Value& name, const rapidjson::Value& value)> json_filter_fn;

/** filter a json dom with provided filter function
 * @param json, a json dom, nomally object,
 * @param fn, a filter function
 * @return the number of removed json node, 0 means not changed at all.
 * @note As the rapijson implement with default allocator, the memmory is not
 * freed after some member is filter out(removed) until the document who own
 * the allocator is freed. So, if you filter a large json result in a small
 * one, better to copy to another document tree, and free the previous
 * document.
 * @note couldnot reserve member order if filter out some object memeber.
 * */
int json_filter(rapidjson::Value& json, json_filter_fn fn);

/** filter out null value
 * @param json, the json tree to filtered
 * @return the number of removed null value
 * */
int filter_null(rapidjson::Value& json);

/** filter out empty json value
 * @param json, the json tree to filtered
 * @return the number of removed empty value
 * @details Including empty string "", empty array [], empty object {}, and
 * null value. But zero number and false is thought as meaningfull and kept.
 * @note Can only filter out one direct layer empty, not recursively, you can
 * manually use `while(filter_empty(json))` if really needed. For example:
 * @code
 * { "aaa": 1, "bbb": [null, "", [], {}] }
 * # call filter_emplty() one time result in
 * { "aaa": 1, "bbb": [] }
 * # call filter_emplty() another time result in
 * { "aaa": 1 }
 * @endcode
 * */
int filter_empty(rapidjson::Value& json);

/** filter json object with a list of keys
 * @param json, the json to filtered
 * @param keys, the list of keys interested 
 * @param keep, if ture, only reserve the provided interested keys, if false,
 * remove those keys. default is ture.
 * @return the number of removed json value
 * @note Only match the member name, on matter how deep the full path is.
 * @note If the keys list if long, better sort first and call
 * @ref filter_key_sorted() instead.
 * */
int filter_key(rapidjson::Value& json, const std::vector<std::string>& keys, bool keep = true);

/** filter json object with a list of sorted keys
 * @details Similar with @ref filter_key() , but more efficient if provide
 * sorted keys.
 * */
int filter_key_sorted(rapidjson::Value& json, const std::vector<std::string>& keys, bool keep = true);

/** filter json object with regexp pattern for keys */
int filter_key(rapidjson::Value& json, const std::string& pattern, bool keep = true);

/* ************************************************************ */
// Section: map

/** function type to modify in place each node in a json tree. 
 * @param name,  the member name, usually is json string when the parent json
 * node is object. Also can be null, if the parent is array.
 * @param value, the value
 * */
typedef std::function<void(rapidjson::Value& name, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)> json_map_fn;

/** map a json dom with provided map function
 * @param json, a json dom, nomally object,
 * @param fn, a map function
 * @details This function process each leaf node recursively, but not remove
 * any one.
 * */
void map_replace(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator, json_map_fn fn);

/** map each leaf node to string type
 * @details Change number/bool/null to string representation. */
void map_to_string(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);

/** replace encoded json string to nested json value.
 * @details Only decode object {} or array [], and if not valid json string,
 * do not modify the original string value.
 * */
void map_decode_json(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);

} /* jsonkit */ 
#endif /* end of include guard: JSON_FILTER_H__ */
