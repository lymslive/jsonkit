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
    
typedef std::function<void(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)> json_slot_fn;

void merge(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

void merge_express(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator);

void slot_register(const std::string& name, json_slot_fn slot);

} /* jsonkit */ 

#endif /* end of include guard: JSON_TRANSFORM_H__ */
