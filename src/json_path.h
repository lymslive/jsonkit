/** 
 * @file json_path.h
 * @author lymslive
 * @date 2021-11-12
 * */
#ifndef JSON_PATH_H__
#define JSON_PATH_H__

#include "rapidjson/document.h"

namespace jsonkit
{

/** get one sub-node from json by pointer path 
 * @param inJson: a json value
 * @param path: path in json dmo based from inJson
 * @return pointer to the sub-node, for null on failure
 * */
const rapidjson::Value* path_point(const rapidjson::Value& inJson, const std::string& path);

/** attach a json node to specific path of json DMO tree
 * @param node, a json node, will move to tree, must share the same
 * allocator with tree, you can copy first if not so.
 * @param path, the path where to attach, should not end with '/', optionally
 * begin with '/'. Must be null terminterd string.
 * @param tree, the target json tree
 * @allocator, allocator for modify tree
 * @return boo, tree if succ to attach
 * @details This function will automically build middle path, as shell command
 * `mkdir -p` does, but will fail if dismatch path part for existed object or array.
 * It will also fail when the final path already existed in target tree.
 * @note Only support object in middle path now.
 * */
bool path_attach(rapidjson::Value& node, const char* path, rapidjson::Value& tree, rapidjson::Document::AllocatorType& allocator);

inline
bool path_attach(rapidjson::Value& node, const std::string& path, rapidjson::Value& tree, rapidjson::Document::AllocatorType& allocator)
{
    return path_attach(node, path.c_str(), tree, allocator);
}


} /* jsonkit */ 

#endif /* end of include guard: JSON_PATH_H__ */
