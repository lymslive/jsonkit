/** 
 * @file json_tranform.h
 * @author lymslive
 * @date 2021-11-02
 * @brief transform json structure
 * */
#ifndef JSON_TRANSFORM_H__
#define JSON_TRANSFORM_H__

#include <functional>
#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace jsonkit
{
    
/** function type to generate a json value
 * @param src: a source json data that maybe uesed
 * @param dst: where store the calculated json in
 * @param allocator: which allocator the dst will used if needed
 * */
typedef std::function<void(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)> json_slot_fn;

/** register a slot function, then may used by given name
 * @param name the slot reference name
 * @param slot function to generate a json value
 * @note May overide already registed name silently, later ones take effect
 * @note Not thread safe, it's better to register on the init stage of
 * application, or protect lock yourself within multiple thread. 
 * */
void slot_register(const std::string& name, json_slot_fn slot);

/** retrive a slot function by name
 * @note where there is no slot function registered with name, the returned
 * function is invalid and can be checked by operator bool(), which is false.
 * */
json_slot_fn slot_retrieve(const std::string& name);

/** interface functor
 * @details just define a concept, can derive from it, but not defined virtual
 * method.
 * */
class CTransform
{
public:
    void operator() (const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
    {}
};

void merge(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

/** merge json `src` into `dst` by template and path specication
 * @details the `dst` is a template json structure, whose keys are predfined,
 * but some of value may be string with special meanning:
 *   - "/path/in/src": json path used extract value from * `src` json tree
 *   - "=slot_fun": will call registered slot function to calculate a value
 * */
void merge_fill(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

/** reshap array to object
 * @param key: the common key in array of object
 * @param json: a json array of object
 * @param allocator: the allocator to modify json
 * @details When each array item is object, which has the common key, lift the
 * value of the key as the new object's key.
 * Can lift multiple level, provided key is as `key1/key2`, or vector of
 * string.
 * */
void array2object(const std::string& key, rapidjson::Value& json,rapidjson::Document::AllocatorType& allocator);

void array2object(const std::vector<std::string>& key, rapidjson::Value& json,rapidjson::Document::AllocatorType& allocator);
} /* jsonkit */ 

#endif /* end of include guard: JSON_TRANSFORM_H__ */
