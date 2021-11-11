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
 * @return true if success, or false on fail, may because of repeated name
 * @note Not thread safe, it's better to register on the init stage of
 * application, or protect lock yourself within multiple thread. 
 * */
bool slot_register(const std::string& name, json_slot_fn slot);

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

/** fill template json dst, with some value form src by path */
class CPathFiller : public CTransform
{
public:
    void operator() (const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

    /** flag if can move value from src to dst. Must false now! */
    bool canMove = false;

    /** flag if remove keys with null value in result dst json */
    bool removeNull = false;
};

void merge(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

/** merge json `src` into `dst` by template and path specication
 * @details the `dst` is a template json structure, whose keys are predfined,
 * but some of value may be string with special meanning:
 *   - "/path/in/src": json path used extract value from * `src` json tree
 *   - "=slot_fun": will call registered slot function to calculate a value
 * */
void merge_fill(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

} /* jsonkit */ 

#endif /* end of include guard: JSON_TRANSFORM_H__ */
