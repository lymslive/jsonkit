#include "tinytast.hpp"
#include "json_filter.h"
#include "json_output.h"

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
