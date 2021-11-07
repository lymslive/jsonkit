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
    COUT(jsonkit::is_error_value(eee), false);
    COUT(jsonkit::is_error_value(ggg), true);

    DESC("wrong to modify the error value");
    ggg = 100;
    COUT(ggg | 0, 100);

    DESC("will automatically reset the error value, in single thread");
    auto& hhh = doc/"ddd"/"hhh";
    COUT(&ggg == &hhh, true);
    COUT(hhh | 0, 0);
    COUT(hhh.IsNull(), true);
    COUT(jsonkit::is_error_value(hhh), true);
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
