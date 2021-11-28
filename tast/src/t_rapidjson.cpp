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

void print_byte(const unsigned char* ptr, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        printf("%.2x ", ptr[i]);
    }
    printf("\n");
}

template <typename T>
void print_byte(const T& val)
{
    print_byte(reinterpret_cast<const unsigned char*>(&val), sizeof(val));
}

DEF_TAST(rapidjson_byte, "tast rapidjson::Value byte layout")
{
    COUT(sizeof(rapidjson::Value));

    rapidjson::Value jDefault;
    COUT(sizeof(jDefault));
    COUT(jDefault.IsNull(), true);
    print_byte(jDefault);

    rapidjson::Value jNull(rapidjson::kNullType);
    COUT(jNull.IsNull(), true);
    print_byte(jNull);

    DESC("declare a int json, flag at end byte");
    rapidjson::Value jIntOne(1);
    COUT(jIntOne.IsInt(), true);
    print_byte(jIntOne);
    DESC("the front byte save int value");
    COUT(*((int*)&jIntOne), jIntOne.GetInt());
    int i = 1;
    jIntOne = i;
    print_byte(jIntOne);

    DESC("declare a uint json, flag at end byte");
    rapidjson::Value jUint;
    jUint = 0xAABBCCDD;
    COUT(jUint.IsUint(), true);
    COUT(jUint.IsInt(), false);
    print_byte(jUint);
    COUT(*((unsigned int*)&jUint), jUint.GetUint());

    DESC("asignment (jDefault = jUint) as move");
    jDefault = jUint;
    COUT(jUint.IsNull(), true);
    DESC("moved value may only change end flag, not touch front bytes");
    print_byte(jUint);
    jUint.SetNull();
    DESC("explict call SetNull() will clear all byte");
    print_byte(jUint);
    COUT(jDefault.IsUint(), true);
    print_byte(jDefault);

    DESC("view -1 value byte layout");
    jIntOne = -1;
    print_byte(jIntOne);
    COUT(jIntOne.IsInt(), true); // 8 FF also think as int?
    COUT(jIntOne.IsInt64(), true);
    jIntOne = 0xABCDEF00;
    print_byte(jIntOne);

    DESC("view const literal string");
    rapidjson::Value jLitStr("literalString");
    COUT(jLitStr.IsString(), true);
    COUT(jLitStr.GetString(), "literalString");
    DESC("only save a pointer in the last 8-2 byte (on 48 pointer optimization)");
    COUT((void*)jLitStr.GetString(), "literalString");
    print_byte(jLitStr);

    rapidjson::Document doc;
    auto& allocator = doc.GetAllocator();

    DESC("view short string in json");
    const char* pShortStr = "literalString";
    rapidjson::Value jShortStr(pShortStr, allocator);
    print_byte(jShortStr);
    COUT(jLitStr.GetString() == pShortStr, true);
    DESC("short string is copy into stack of json value");
    COUT(jShortStr.GetString() == pShortStr, false);
    COUT(jShortStr.GetString() == (const char*)&jShortStr, true);
    COUT((void*)jShortStr.GetString(), (void*)&jShortStr);
    COUT(jShortStr.GetString(), std::string(pShortStr));

    DESC("view longer string in json");
    const char* pLongStr = "literalString.";
    rapidjson::Value jLongStr(pLongStr, allocator);
    print_byte(jLongStr);
    DESC("longer string cannot fit in stack will allocate, and put a pointer");
    COUT(jLongStr.GetString() == (const char*)&jLongStr, false);
    COUT(jLongStr.GetString() == pLongStr, false);
    COUT(jLongStr.GetString(), std::string(pLongStr));

    DESC("view json array value");
    rapidjson::Value jArray(rapidjson::kArrayType);
    print_byte(jArray);
    COUT(jArray.Empty(), true);
    COUT(jArray.Capacity(), 0);
    DESC("push an item to array, then allocate and put a pointer");
    jArray.PushBack(1, allocator);
    print_byte(jArray);
    COUT(jArray.Size(), 1);
    COUT(jArray.Capacity(), 16);

    DESC("view json object value");
    rapidjson::Value jObject(rapidjson::kObjectType);
    print_byte(jObject);
    DESC("add a member");
    jObject.AddMember("aa", 1, allocator);
    print_byte(jObject);
    COUT(jObject.MemberCount(), 1);
    COUT(jObject.MemberCapacity());

    DESC("move assign jObject");
    jDefault = jObject;
    print_byte(jObject);
    print_byte(jDefault);
    jObject.SetNull();
    print_byte(jObject);
    jDefault.SetObject();
    print_byte(jDefault);
}

DEF_TAST(rapidjson_ptr48opt, "tast rapidjson 48 bit pointer optimization")
{
    COUT(sizeof((void*) 0), 8);

    const char* pShortStr = "literalString";
    COUT(sizeof(pShortStr), 8);
    COUT((void*)pShortStr);
    COUT(pShortStr);

    const void* vptr = pShortStr;
    uint64_t high = 0xAA00000000000000;
    COUT((uint64_t)vptr & high, 0);
    COUT((void*)((uint64_t)vptr | high));
    // can not add arbitrary high byte to pointer
    // in rapidjson use macro RAPIDJSON_GETPOINTER to ignore high byte
    // COUT((const char*)((uint64_t)vptr | high));
}
