/** 
 * @file json_operator.h 
 * @author lymslive 
 * @date 2021-10-26 
 *
 * @details 
 * path operator(/) which is from divide operator, implement json path pointer, but focus to query value.
 * return a reference to sub-tree of the current json value, or
 * return a static null json value if occur path error.
 * example for json root = {"subname": {"subsub": ??, ...}, "subarray": [0, {"value": ??, ...}, ...]}
 * @code
 *   sub_tree = json_root / "subname" / "subsub";
 *   sub_val  = json_root / "subarray" / 1 / "value";
 *   sub_tree = json_root / "subname/subsub";
 *   sub_val  = json_root / "subarray/1/value";
 * @endcode
 *
 * conversion operator(|) which is from bit or operator, conver a generic scalar json value to c++ value.
 * mainly int or doule or string.
 * example:
 * @code
 *   int i = json | 0;
 *   int d = json | 0.0;
 *   std::string s = json | "";
 * @endcode
 * */
#ifndef JSON_OPERATOR_H__
#define JSON_OPERATOR_H__

#include <string>

#include "rapidjson/document.h"

namespace jsonkit
{
bool has_path_error();

class COperand
{
public:
    COperand(rapidjson::Document& doc);
    COperand(rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

    COperand& PathOperate(const std::string& path);
    COperand& PathOperate(const char* path);
private:
    rapidjson::Value* m_pJsonNode;
    rapidjson::Document::AllocatorType* m_pAllocator;
};

} /* jsonkit */ 

/** operator for json value, must in global namespace */
const rapidjson::Value& operator/ (const rapidjson::Value& json, const std::string& path);
const rapidjson::Value& operator/ (const rapidjson::Value& json, const char* path);
const rapidjson::Value& operator/ (const rapidjson::Value& json, size_t index);
const rapidjson::Value& operator/ (const rapidjson::Value& json, int index);

int operator|(const rapidjson::Value& json, int defval);
double operator|(const rapidjson::Value& json, double defval);
std::string operator|(const rapidjson::Value& json, const char* defval);
std::string operator|(const rapidjson::Value& json, const std::string& defval);

#define AS_INT | 0
#define AS_DOUBLE | 0.0
#define AS_STRING | ""

inline
bool operator! (const rapidjson::Value& json)
{
    return json.IsNull();
}
#endif /* end of include guard: JSON_OPERATOR_H__ */
