/**
 * @file t_operator.cpp
 * @author lymslive
 * @date 2021-11-07
 * @brief test json operator overrides
 * */
#include "tinytast.hpp"
#include "json_operator.h"

DEF_TAST(operator_jvraw, "operator on raw json value")
{
std::string jsonText = R"json({
    "aaa": 1, "bbb":2,
    "ccc": [3, 4, 5, 6],
    "ddd": {"eee":7, "fff":8.8}
})json";

    // std::string jsonText = "{ \"aaa\": 1, \"bbb\":2, \"ccc\": [3, 4, 5, 6], \"ddd\": {\"eee\":7, \"fff\":8.8} }";

    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    int aaa = doc / "aaa" | 0;
    COUT(aaa, 1);

    //! complie error
    // aaa = doc / "aaa";
    doc / "aaa" = 11;
    COUT(doc / "aaa" | 0, 11);

    int bbb = 0;
    bbb |= doc / "bbb" ;
    COUT(bbb, 2);
    
    int ccc = 0;
    ccc |= doc / "ccc" / 0;
    COUT(ccc, 3);
    ccc |= doc / "ccc/1";
    COUT(ccc, 4);

    // compile error if only write "auto", must "auto &"
    // const rapidjson::Value&
    auto& cccJson = doc / "ccc";
    ccc |= cccJson / 2;
    COUT(ccc, 5);
    ccc |= cccJson / 3;
    COUT(ccc, 6);

    int eee = doc / "ddd" / "eee" | 0;
    COUT(eee, 7);

    double fff = doc / "ddd" / "fff" | 0.0;
    COUT(fff, 8.8);

    (doc / "ddd" / "fff") = 9.9;
    COUT(doc / "ddd" / "fff" | 0.0, 9.9);

    (doc / "ddd" / "fff") = 999;
    COUT(doc / "ddd" / "fff" | 0, 999);

/* compile error
    eee = (int)(doc / "ddd" / "eee");
    COUT(eee, 7);
    
    fff = (double)(doc / "ddd" / "fff");
    COUT(eee, 8.8);
*/
}

DEF_TAST(operator_wrap, "operator on raw json value")
{
std::string jsonText = R"json({
    "aaa": 1, "bbb":2,
    "ccc": [3, 4, 5, 6],
    "ddd": {"eee":7, "fff":8.8}
})json";

    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    DESC("use stream out operator to stringfy json");
    {
        std::string stringfy;
        doc >> stringfy;
        COUT(stringfy);
    }

    JSOP root(doc);
    DESC("JSOP is like a pointer");
    COUT(&doc);
    COUT(&(*root));
    COUT(&doc == &(*root), true);

    int aaa = root / "aaa" | 0;
    COUT(aaa, 1);

    root*doc / "aaa" = 11;
    COUT(root*doc  / "aaa" | 0, 11);
    root / "aaa" = 12;
    COUT(root  / "aaa" | 0, 12);
    COUT(doc  / "aaa" | 0, 12);

    int bbb = 0;
    bbb |= root*doc / "bbb" ;
    COUT(bbb, 2);
    
    int ccc = 0;
    ccc |= root / "ccc" / 0;
    COUT(ccc, 3);
    ccc |= root / "ccc/1";
    COUT(ccc, 4);

    auto& cccJson = doc / "ccc";
    ccc |= cccJson / 2;
    COUT(ccc, 5);
    ccc |= cccJson / 3;
    COUT(ccc, 6);

    DESC("add item to json array");
    COUT(cccJson.Size(), 4);
    DESC("JSOP on inner node widthout allocator, fail add");
    JSOP(cccJson) << 66;
    COUT(cccJson.Size(), 4);
    DESC("previous JSOP root has allocator, jump to inner node to add");
    root * cccJson << 66;
    COUT(cccJson.Size(), 5);
    JSOP(doc) / "ccc" << 666;
    COUT(cccJson.Size(), 6);
    COUT(cccJson/5 | 0, 666);

    int eee = doc / "ddd" / "eee" | 0;
    COUT(eee, 7);

    double fff = doc / "ddd" / "fff" | 0.0;
    COUT(fff, 8.8);

    DESC("use temp JSOP macro to modify json value");
    JSOP(doc) / "ddd" / "fff" = 9.9;
    COUT(doc / "ddd" / "fff" | 0.0, 9.9);

    JSOP(doc) / "ddd" / "fff" = 999;
    COUT(doc / "ddd" / "fff" | 0, 999);

    DESC("change json node to string");
    JSOP(doc) / "ddd" / "fff" = "s999";
    COUT(doc / "ddd" / "fff" | "", std::string("s999"));
    COUT(JSOP(doc) / "ddd" / "fff" | "", std::string("s999"));
    std::string str;
    str |= doc / "ddd" / "fff"; 
    COUT(str, "s999");
    str.clear();
    str |= JSOP(doc) / "ddd" / "fff"; 
    COUT(str, "s999");

    std::string key("ggg");
    std::string val("g99");
    JSOP(doc) << std::make_pair(key, val);
    str |= doc / key;

    (doc / "ddd" / "fff") = "g99"; // ok
    // (doc / "ddd" / "fff") = val; // error
    COUT(str, val);
    {
        std::string stringfy;
        doc >> stringfy;
        COUT(stringfy);
    }
}

DEF_TAST(operator_error, "handle path operator error")
{
std::string jsonText = R"json({
    "aaa": 1, "bbb":2,
    "ccc": [3, 4, 5, 6],
    "ddd": {"eee":7, "fff":8.8}
})json";

    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    auto& eee = doc/"ddd"/"eee";
    auto& ggg = doc/"ddd"/"ggg";
    COUT(!eee, false);
    COUT(!ggg, true);
    COUT(eee.IsNull(), false);
    COUT(ggg.IsNull(), true);

    DESC("wrong to modify the error value");
    ggg = 100;
    COUT(ggg | 0, 100);

    DESC("will automatically reset the error value, in single thread");
    auto& hhh = doc/"ddd"/"hhh";
    COUT(&ggg == &hhh, true);
    COUT(hhh | 0, 0);
    COUT(hhh.IsNull(), true);
    COUT(!hhh, true);

    COUT(ggg | 0, 0);
}

DEF_TAST(operator_scalar, "handle scalar auto conversion")
{
    std::string jsonText = R"json({
    "aaa": 100, "bbb":200.0,
    "ccc": "100", "ddd": "100abc", "eee": "abc100",
    "fff": "200.0", "ggg": "200.0abc", "hhh": "abc200.",
    "t1": 1, "t2": true, "t3": "true", "t4": "TRUE",
    "f1": 0, "f2": false, "f3": "false", "f4": "FALSE"
})json";

    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    DESC("int can convert to double");
    COUT(doc/"aaa" | 0, 100);
    COUT(doc/"aaa" | 0.0, 100.0);
    COUT((doc/"aaa").IsDouble(), false);

    DESC("double cannot convert to int");
    COUT(doc/"bbb" | 0.0, 200.0);
    COUT(doc/"bbb" | 0, 0);
    COUT((doc/"bbb").IsInt(), false);

    COUT(doc/"ccc" | 1, 100);
    COUT(doc/"ddd" | 1, 100);
    COUT(doc/"eee" | 1, 1);

    COUT(doc/"fff" | 1.0, 200.0);
    COUT(doc/"ggg" | 1.0, 200.0);
    COUT(doc/"hhh" | 1.0, 1.0);

    COUT(doc/"t1" | false, true);
    COUT(doc/"t2" | false, true);
    COUT(doc/"t3" | false, true);
    COUT(doc/"t4" | false, true);

    COUT(doc/"f1" | true, false);
    COUT(doc/"f2" | true, false);
    COUT(doc/"f3" | true, false);
    COUT(doc/"f4" | true, false);

    const char* psz = doc/"ddd" | "";
    std::string str = doc/"ddd" | std::string();
    COUT(psz);
    COUT(str);
    COUT(str == psz, true);
}

DEF_TAST(operator_append1_array, "tast jsop <<")
{
    rapidjson::Document doc;
    
    DESC("doc init from default null, then << different type of value");
    COUT(doc.IsNull(), true);
    doc.SetArray();
    doc << 1 << "abc" << std::string("ABC") << 1.1 << -2;
    {
        rapidjson::Value item(3.14);
        doc << item;
        DESC("json value << has move effect");
        COUT(jsonkit::stringfy(item));
        COUT(item.IsNull(), true);

        item.SetObject();
        item.AddMember("abc", 1, doc.GetAllocator());
        item.AddMember("ABC", 1.1, doc.GetAllocator());

        doc << item;
        COUT(jsonkit::stringfy(item));
        COUT(item.IsNull(), true);

        DESC("while const json value& << has copy effect");
        item = 314;
        doc << const_cast<const rapidjson::Value&>(item);
        COUT(item.IsNull(), false);
        COUT(item.GetInt(), 314);
    }
    // COUT(jsonkit::stringfy(doc));
    COUT(doc);

    COUT(doc.IsArray(), true);
    COUT(doc.Size(), 8);
    COUT(doc[0].IsInt(), true);
    COUT(doc[6].IsObject(), true);

    DESC("append operator << internally call assign = operator");
    // for scalar assignment, doc/0 is also OK as require no allocator
    JSOP(doc)/0 = 2;
    COUT(doc[0].GetInt(), 2);
    JSOP(doc)/0 = 3.14;
    COUT(doc[0].GetDouble(), 3.14);

    JSOP(doc)/0 = "3.14";
    COUT(doc[0].IsString(), true);
    COUT(doc[0].GetString(), std::string("3.14"));
    COUT((const void*)doc[0].GetString() == (const void*)("3.14"), true);
    doc/0 = "pi3.14";
    COUT((const void*)doc[0].GetString() == (const void*)("pi3.14"), true);

#if 0
    const char* psz = "psz3.14";
    // doc/0 = psz; //! compile error
    JSOP(doc)/0 = psz;
    COUT(doc[0].GetString(), std::string("psz3.14"));
    COUT((const void*)doc[0].GetString() == (const void*)("psz3.14"), false);
#endif

    std::string str = "str3.14";
    JSOP(doc)/0 = str;
    COUT(doc[0].GetString(), str);
    COUT((const void*)doc[0].GetString() == (const void*)(str.c_str()), false);

    JSOP(doc)/0 = false;
    COUT(doc[0].IsBool(), true);
    COUT(doc[0].GetBool(), false);

    DESC("json value assign has move effect");
    rapidjson::Value item(188);
    JSOP(doc)/0 = item;
    COUT(doc[0].GetInt(), 188);
    COUT(item.IsNull(), true);

    DESC("but const json value& assign is copy effect");
    item = 188;
    JSOP(doc)/0 = const_cast<const rapidjson::Value&>(item);
    COUT(doc[0].GetInt(), 188);
    COUT(item.IsNull(), false);
    COUT(item.GetInt(), 188);

    COUT(doc);
}

DEF_TAST(operator_append2_object, "tast jsop(object) <<")
{
    rapidjson::Document doc;

    DESC("can only append to array or object, set the target type first");
    doc << 1 << "str";
    COUT(doc.IsNull(), true);

    doc.SetObject();

    doc << "aaa" << 1 << "bbb" << 2;
    COUT(doc.MemberCount(), 2);
    auto it = doc.MemberBegin();
    COUT(it->name.IsString(), true);
    COUT(it->name.GetString(), std::string("aaa"));
    COUT(it->name.GetString(), "aaa");
    DESC("literal string key is not copied");
    COUT((const void*)it->name.GetString() == (const void*)"aaa", true);

    DESC("short string as key");
    std::string shortStr("short_string");
    doc << shortStr << 3;
    it = doc.MemberBegin() + 2;
    COUT(it->name.GetString(), shortStr);
    COUT(it->name.GetString() != shortStr.c_str(), true);
    COUT((const void*)it->name.GetString() == (const void*)&it->name, true);

    DESC("longer string as key");
    std::string longStr("long_string_long");
    doc << longStr << 4;
    it = doc.MemberBegin() + 3;
    COUT(it->name.GetString(), longStr);
    COUT(it->name.GetString() != shortStr, true);
    COUT(it->name.GetString() != (const char*)&it->name, true);

    COUT(doc);

    DESC("fail to append mismatched key-value pair");
    doc << 5 << 6;
    COUT(doc.MemberCount(), 4);

    DESC("append key first, and later append value");
    doc << "ccc";
    COUT(doc.MemberCount(), 5);
    it = doc.MemberBegin() + 4;
    COUT(it->name.GetString(), "ccc");
    COUT(it->value.IsNull(), true);
    rapidjson::Value item("later value");
    doc << item;
    // this name-value both are literal string const ref
    COUT(it->value.GetString(), "later value");
    DESC("appended json value is moved and reset to null");
    COUT(item.IsNull(), true);

    DESC("pending member value is null but distinguish from normal null");
    doc.AddMember("ddd", rapidjson::Value(), doc.GetAllocator());
    COUT(doc.MemberCount(), 6);
    doc << "eee" << rapidjson::Value();
    COUT(doc.MemberCount(), 7);
    doc << "fff" << 8;
    COUT(doc.MemberCount(), 8);

    COUT(doc);
}

DEF_TAST(operator_append3_container, "tast jsop << std container")
{
    rapidjson::Document doc;
    std::vector<int> vec{1,2,3,4,5};

    DESC("can assign a std::vector to json, set it array");
    // doc = vec; //! compile error, can only assign to jsop
    // JSOP(doc) = vec; //! explain as redeclare doc variable
    JSOP root(doc);
    root = vec;
    COUT(doc.IsArray(), true);
    COUT(doc.Size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        COUT(doc/i | 0, i+1);
    }
    COUT(doc);

    std::map<std::string, int> map{
        {"aaa", 1}, {"bbb", 2}, {"ccc", 3}, {"ddd", 4}, {"eee",5}
    };

    DESC("append operator required match type for array or object");
    doc << map;
    COUT(doc.IsArray(), true);

    DESC("assign operator can change json type directlly");
    // JSOP(doc) = map;
    root = map;
    COUT(doc.IsObject(), true);
    COUT(doc.MemberCount(), 5);
    for (auto& item : map)
    {
        COUT(doc/item.first | 0, item.second);
    }
    COUT(doc);

    rapidjson::Value jnode;
    DESC("preapare an array node then append to doc root object");
    jnode.SetArray();
    doc*jnode << vec;
    COUT(jnode.Size(), 5);
    doc << "fff" << jnode;
    COUT(doc.MemberCount(), 6);
    COUT(jnode.IsNull(), true);

    DESC("preapare another object node then append to doc root object");
    jnode.SetObject();
    jnode*doc << map;
    COUT(jnode.MemberCount(), 5);
    doc << "ggg" << jnode;
    COUT(doc.MemberCount(), 7);
    COUT(jnode.IsNull(), true);
    COUT(doc);
}

DEF_TAST(operator_add, "tast json(jsop) + number(string)")
{
    rapidjson::Document doc;
    rapidjson::Value json;

    DESC("json value add with number, if it is also number type");
    json = 1;
    int i = json + 1;
    COUT(i, 2);
    i = 3 + json;
    COUT(i, 4);
    i += json;
    COUT(i, 5);
    json += 1;
    COUT(json.GetInt(), 2);

    DESC("json value add with number, if it is also string type");
    json = "json literal string;";
    std::string str("c++ std string;");

    std::string res = str + json;
    COUT(res, "c++ std string;json literal string;");
    res = json + str;
    COUT(res, "json literal string;c++ std string;");
    str += json;
    COUT(str, "c++ std string;json literal string;");
    DESC("json += str; must operate on jsop with allocator");
    json += str; 
    COUT(json.GetString(), "json literal string;");
    doc*json += str;
    COUT(json.GetString(), std::string("json literal string;c++ std string;json literal string;"));

    DESC("add dismatched json type has no effect");
    COUT(json.IsString(), true);
    i = 3 + json;
    COUT(i, 3);
    DESC("while += act as = change to target type");
    json += 1;
    COUT(json.IsInt(), true);
    COUT(json.GetInt(), 1);
    COUT(json + 0, json | 0);

    str = "c++ std string;";
    res = str + json;
    COUT(res, str);
    res = json + str;
    COUT(res, str);
    doc*json += str;
    COUT(json.IsString(), true);
    COUT(json.GetString(), str);
}
