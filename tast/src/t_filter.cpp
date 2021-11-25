#include "tinytast.hpp"
#include "json_filter.h"
#include "json_output.h"
#include "json_operator.h"

DEF_TAST(filter_null, "test filter null value")
{
    std::string text = R"json({
    "aaa": 1, "bbb":null, "ccc": "c11",
    "ddd": {"eee":7, "ggg": null, "fff":8.8},
    "DDD": [7,8,null,9,10]
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 5);
    COUT(doc["ddd"].MemberCount(), 3);
    COUT(doc["DDD"].Size(), 5);

    DESC("after filter null");
    int filtered = jsonkit::filter_null(doc);
    COUT(filtered, 3);
    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 4);
    COUT(doc["ddd"].MemberCount(), 2);
    COUT(doc["DDD"].Size(), 4);
}

DEF_TAST(filter_empty, "test filter empty value")
{
    std::string text = R"json({
    "aaa": 1, "bbb":null, "ccc": "c11", "cc0": "",
    "ddd": {"eee":0, "ggg": [], "fff":""},
    "DDD": [7,{},null,[],false],
    "eee": {}, "fff": [{},{},{}]
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 8);
    COUT(doc["ddd"].MemberCount(), 3);
    COUT(doc["DDD"].Size(), 5);

    DESC("after filter empty");
    int filtered = jsonkit::filter_empty(doc);
    COUT(filtered, 11);
    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 5);
    COUT(doc["ddd"].MemberCount(), 1);
    COUT(doc["DDD"].Size(), 2);
}


DEF_TAST(filter_empty_rev, "test filter empty value recursively")
{
    std::string text = R"json({
    "aaa": 1, "bbb":null, "ccc": "c11", "cc0": "",
    "ddd": {"eee":{}, "ggg": [], "fff":""},
    "DDD": [7,{},null,[],false],
    "eee": {}, "fff": [{},{},{}]
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 8);
    COUT(doc["ddd"].MemberCount(), 3);
    COUT(doc["DDD"].Size(), 5);

    DESC("after filter empty");
    int filtered = jsonkit::filter_empty(doc);
    COUT(filtered, 12);
    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 5);
    COUT(doc["ddd"].MemberCount(), 0);
    COUT(doc["DDD"].Size(), 2);

    DESC("recursively filter empty manually");
    while (filtered > 0)
    {
        filtered = jsonkit::filter_empty(doc);
        COUT(filtered);
    }
    COUT(jsonkit::stringfy(doc));
}

DEF_TAST(filter_key1, "test filter keys")
{
    std::string text = R"json({
    "aaa": 1, "bbb":2, "ccc": "null",
    "ddd": {"aaa":7, "bbb": null, "ccc":8.8},
    "DDD": [{"aaa": null, "bbb":2, "ccc": "null"}, {"aaa": 1, "bbb":2, "ccc": 3}]
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 5);
    COUT(doc["ddd"].MemberCount(), 3);

    DESC("after filter keys");
    std::vector<std::string> keys{"aaa", "ccc"};
    int filtered = jsonkit::filter_key(doc, keys);
    COUT(filtered, 3);
    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 2);
    // COUT(doc["ddd"].MemberCount(), 2);
}

DEF_TAST(filter_key2, "test filter keys")
{
    std::string text = R"json({
    "aaa": 1, "bbb":2, "ccc": "null",
    "ddd": {"aaa":7, "bbb": null, "ccc":8.8},
    "DDD": [{"aaa": null, "bbb":2, "ccc": "null"}, {"aaa": 1, "ccc": 3, "bbb":2}]
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 5);
    COUT(doc["ddd"].MemberCount(), 3);

    DESC("after filter keys");
    std::vector<std::string> keys{"aaa", "ccc", "ddd", "DDD"};
    int filtered = jsonkit::filter_key(doc, keys);
    COUT(filtered, 4);
    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 4);
    COUT(doc["ddd"].MemberCount(), 2);
}

DEF_TAST(filter_key3, "test filter keys with pre sorted")
{
    std::string text = R"json({
    "aaa": 1, "bbb":2, "cxc": "null",
    "ddd": {"aaa":7, "bbb": null, "cxc":8.8},
    "eee": [{"fff": null, "gGg":2, "ggg": "null"}, {"AAA": 1, "CCC": 3, "BBB":2}]
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 5);
    COUT(doc["ddd"].MemberCount(), 3);

    DESC("after filter keys");
    std::vector<std::string> keys{"aaa", "bbb", "ccc", "ddd", "eee", "fff", "ggg"};
    int filtered = jsonkit::filter_key_sorted(doc, keys);
    COUT(filtered, 6);
    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 4);
    COUT(doc["ddd"].MemberCount(), 2);
}

DEF_TAST(filter_regex, "test filter keyes with regex")
{
    std::string text = R"json({
    "aaa": 1, "key.bBb":2, "ccc": "null", "dDd.key": "xtring"
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 4);

    DESC("after filter keys");
    std::string pattern("[a-z][A-Z][a-z]");
    int filtered = jsonkit::filter_key(doc, pattern);
    COUT(filtered, 2);
    COUT(jsonkit::stringfy(doc));
    COUT(doc.MemberCount(), 2);
}

DEF_TAST(filter_map_1, "replace null to string form")
{
    auto markNull = [](rapidjson::Value& name, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
    {
        if (value.IsNull())
        {
            value = "null";
        }
    };

    std::string text = R"json({
    "aaa": 1, "bbb": null, "ccc": "NULL",
    "ddd": { "AAA": 11, "BBB": null },
    "eee": [1, 2, null, "44"]
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    auto& bbb = doc/"bbb";
    COUT(bbb.IsNull(), true);
    auto& BBB = doc/"ddd"/"BBB";
    COUT(BBB.IsNull(), true);
    auto& eee = doc/"eee"/2;
    COUT(eee.IsNull(), true);

    jsonkit::map_replace(doc, doc.GetAllocator(), markNull);
    COUT(jsonkit::stringfy(doc));
    COUT(bbb.IsNull(), false);
    COUT(BBB.IsNull(), false);
    COUT(eee.IsNull(), false);
}

DEF_TAST(filter_map_2, "replace other scalar to string form")
{
    std::string text = R"json({
    "aaa": 1, "bbb": null, "ccc": "NULL",
    "ddd": { "AAA": 11, "BBB": null },
    "eee": [-100, 2.78, null, "44"]
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    auto& aaa = doc/"aaa";
    COUT(aaa.IsInt(), true);
    COUT(aaa.GetInt(), 1);
    auto& bbb = doc/"bbb";
    COUT(bbb.IsNull(), true);
    auto& BBB = doc/"ddd"/"BBB";
    COUT(BBB.IsNull(), true);
    auto& eee = doc/"eee"/2;
    COUT(eee.IsNull(), true);

    jsonkit::map_to_string(doc, doc.GetAllocator());
    COUT(jsonkit::stringfy(doc));
    COUT(aaa.IsInt(), false);
    COUT(aaa.IsString(), true);
    COUT(aaa.GetString(), std::string("1"));
    COUT(bbb.IsNull(), false);
    COUT(BBB.IsNull(), false);
    COUT(eee.IsNull(), false);
}

DEF_TAST(filter_map_3, "replace json encoded string to nest json value")
{
    std::string text = R"json({
    "aaa": 1, "bbb": "{3}", "ccc": "[]",
    "ddd": "{ \"AAA\": 11, \"BBB\": null }",
    "eee": "[-100, 2.78, null, \"44\"]"
})json";

    rapidjson::Document doc;
    doc.Parse(text.c_str(), text.size());
    COUT(doc.HasParseError(), false);

    COUT(jsonkit::stringfy(doc));
    auto& bbb = doc/"bbb";
    auto& ccc = doc/"ccc";
    auto& ddd = doc/"ddd";
    auto& eee = doc/"eee";

    COUT(bbb.IsString(), true);
    COUT(ccc.IsString(), true);
    COUT(ddd.IsString(), true);
    COUT(eee.IsString(), true);

    jsonkit::map_decode_json(doc, doc.GetAllocator());
    COUT(jsonkit::stringfy(doc));

    COUT(bbb.IsString(), true);
    COUT(ccc.IsArray(), true);
    COUT(ddd.IsObject(), true);
    COUT(eee.IsArray(), true);
    COUT(ddd.HasMember("AAA"), true);
    COUT(ddd["AAA"].GetInt(), 11);
    COUT(ddd.HasMember("BBB"), true);
    COUT(ddd["BBB"].IsNull(), true);
    COUT(eee.Size(), 4);
    COUT(eee[0].GetInt(), -100);
    COUT(eee[2].IsNull(), true);
}
