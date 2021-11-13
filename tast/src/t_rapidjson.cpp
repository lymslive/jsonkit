/** 
 * @file t_rapidjson.cpp 
 * @author lymslive 
 * @date 2021-11-07 
 * @brief test usage of rapidjon
 * */
#include "tinytast.hpp"
#include "rapidjson/document.h"

DEF_TAST(rapidjson_scalar, "scalar json value in rapidjson")
{
    rapidjson::Value json;

    COUT(sizeof(json));

    DESC("json = 1");
    json = 1;
    COUT(json.GetInt());
    COUT(json.IsInt(), true);
    COUT(json.IsUint(), true);
    COUT(json.IsInt64(), true);
    COUT(json.IsUint64(), true);
    COUT(json.IsDouble(), false);

    DESC("json = (unsigned int)-1");
    json = (unsigned int)-1;
    COUT(json.GetUint());
    COUT(json.IsInt(), false);
    COUT(json.IsUint(), true);
    COUT(json.IsInt64(), true);
    COUT(json.IsUint64(), true);
    COUT(json.IsDouble(), false);
}
