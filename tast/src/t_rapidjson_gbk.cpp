/** 
 * @file t_rapidjson_gbk.cpp 
 * @author lymslive 
 * @date 2021-11-07 
 * @brief test usage of rapidjon in gbk encodeing
 * */
#include "tinytast.hpp"
#include "rapidjson/document.h"
#include "json_operator.h"

DEF_TAST(rapidjson_gbk, "test gbk chinse character")
{
    std::string key("ÐÕÃû");
    std::string val("ÕÅÈý·á");

    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Value jsKey;
    rapidjson::Value jsVal;

    jsKey.SetString(key.c_str(), key.size(), doc.GetAllocator());
    jsVal.SetString(val.c_str(), val.size(), doc.GetAllocator());
    COUT(jsKey.GetString());
    COUT(jsKey.GetStringLength());
    COUT(jsVal.GetString());
    COUT(jsVal.GetStringLength());

    doc.AddMember(jsKey, jsVal, doc.GetAllocator());
    COUT(doc);

}
