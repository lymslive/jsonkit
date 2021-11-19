#include "tinytast.hpp"
#include "json_transform.h"
#include "json_operator.h"
#include "json_input.h"

DEF_TAST(transform_fill, "fill from template")
{
    std::string srcText = R"json({
    "aaa": 1, "bbb":2,
    "ccc": "c11",
    "ddd": {"eee":7, "fff":8.8}
})json";

    std::string dstText = R"json({
    "AAA": "static",
    "aaa": "/aaa", 
    "bbb": "/bbb",
    "ccc": "/ccc",
    "ddd": "/ddd",
    "DDD": {
        "EEE": "/ddd/eee",
        "FFF": "/ddd/fff"
    }
})json";

    rapidjson::Document docSrc;
    docSrc.Parse(srcText.c_str(), srcText.size());
    COUT(docSrc.HasParseError(), false);

    rapidjson::Document docDst;
    docDst.Parse(dstText.c_str(), dstText.size());
    COUT(docDst.HasParseError(), false);

    COUT(jsonkit::stringfy(docSrc));
    COUT(jsonkit::stringfy(docDst));
    jsonkit::merge_fill(docSrc, docDst, docDst.GetAllocator());
    COUT(jsonkit::stringfy(docDst));

    COUT(docDst/"aaa" | 0, 1);
    COUT(docDst/"bbb" | 0, 2);
    COUT(docDst/"ccc" | "", std::string("c11"));
    COUT(docDst/"ddd"/"eee" | 0, 7);
    COUT(docDst/"DDD"/"EEE" | 0, 7);
    COUT(docDst/"DDD"/"FFF" | 0.0, 8.8);
}

static
void add_one(const rapidjson::Value& src, rapidjson::Value& dst, rapidjson::Document::AllocatorType& allocator)
{
    int i = src | 0;
    if (src.IsObject())
    {
        i |= src/"ddd"/"eee";
    }
    i++;
    dst = i;
}

DEF_TAST(transform_slot, "fill with slot")
{
    std::string srcText = R"json({
    "aaa": 1, "bbb":2,
    "ccc": "c11",
    "ddd": {"eee":7, "fff":8.8}
})json";

    std::string dstText = R"json({
    "AAA": "static",
    "aaa": "=ADD/aaa", 
    "bbb": "=ADD/bbb",
    "ccc": "/ccc",
    "ddd": "/ddd",
    "DDD": {
        "EEE": "=ADD",
        "FFF": "/ddd/fff"
    }
})json";

    // register slot function
    bool succ = jsonkit::slot_register("ADD", add_one);
    COUT(succ, true);
    // should not register with the same name
    succ = jsonkit::slot_register("ADD", add_one);
    COUT(succ, false);

    rapidjson::Document docSrc;
    docSrc.Parse(srcText.c_str(), srcText.size());
    COUT(docSrc.HasParseError(), false);

    rapidjson::Document docDst;
    docDst.Parse(dstText.c_str(), dstText.size());
    COUT(docDst.HasParseError(), false);

    COUT(jsonkit::stringfy(docSrc));
    COUT(jsonkit::stringfy(docDst));
    jsonkit::merge_fill(docSrc, docDst, docDst.GetAllocator());
    COUT(jsonkit::stringfy(docDst));

    COUT(docDst/"aaa" | 0, 2);
    COUT(docDst/"bbb" | 0, 3);
    COUT(docDst/"ccc" | "", std::string("c11"));
    COUT(docDst/"ddd"/"eee" | 0, 7);
    COUT(docDst/"DDD"/"EEE" | 0, 8);
    COUT(docDst/"DDD"/"FFF" | 0.0, 8.8);
}

DEF_TAST(transform_array2object_1, "test array2object, one level")
{
    DESC("lift one level key");
    std::string srcText = R"json([
    { "name": "aaa", "value": 1},
    { "name": "bbb", "value": 2},
    { "name": "ccc", "value": "3c"},
    { "name": "ddd", "value": "4d"}
])json";

    std::string dstText = R"json({
    "aaa": 1, "bbb": 2, "ccc": "3c", "ddd": "4d"
})json";

    rapidjson::Document docSrc;
    COUT(jsonkit::read_string(docSrc, srcText), true);

    rapidjson::Document docDst;
    COUT(jsonkit::read_string(docDst, dstText), true);

    jsonkit::array2object("name", docSrc, docSrc.GetAllocator());
    COUT(jsonkit::stringfy(docSrc));
    COUT(docSrc == docDst, true);
}

DEF_TAST(transform_array2object_2, "test array2object, disorder")
{
    DESC("key not required order, and left value can be any thing");
    std::string srcText = R"json([
    { "name": "aaa", "value": 1},
    { "name": "bbb", "value": 2},
    { "value": "3c", "name": "ccc"},
    { "name": "ddd", "VAXUE": "4d"}
])json";

    std::string dstText = R"json({
    "aaa": 1, "bbb": 2, "ccc": "3c", "ddd": "4d"
})json";

    rapidjson::Document docSrc;
    COUT(jsonkit::read_string(docSrc, srcText), true);

    rapidjson::Document docDst;
    COUT(jsonkit::read_string(docDst, dstText), true);

    jsonkit::array2object("name", docSrc, docSrc.GetAllocator());
    COUT(jsonkit::stringfy(docSrc));
    COUT(docSrc == docDst, true);
}

DEF_TAST(transform_array2object_3, "test array2object, two level")
{
    DESC("lift two level key");
    std::string srcText = R"json([
    { "name": "aaa", "NAME": "XXX", "value": 1},
    { "name": "aaa", "NAME": "YYY", "value": 11},
    { "name": "bbb", "NAME": "XXX", "value": 2},
    { "name": "bbb", "NAME": "ZZZ", "value": 22},
    { "name": "ccc", "NAME": "YYY", "value": 3},
    { "name": "ccc", "NAME": "ZZZ", "value": 33}
])json";

    std::string dstText = R"json({
    "aaa": { "XXX": 1, "YYY": 11},
    "bbb": { "XXX": 2, "ZZZ": 22},
    "ccc": { "YYY": 3, "ZZZ": 33}
})json";

    rapidjson::Document docSrc;
    COUT(jsonkit::read_string(docSrc, srcText), true);

    rapidjson::Document docDst;
    COUT(jsonkit::read_string(docDst, dstText), true);

    jsonkit::array2object("name/NAME", docSrc, docSrc.GetAllocator());
    COUT(jsonkit::stringfy(docSrc));
    COUT(docSrc == docDst, true);
}

DEF_TAST(transform_array2object_4, "test array2object, two level")
{
    DESC("lift two level key");
    std::string srcText = R"json([
    { "name": "aaa", "NAME": "XXX", "value": 1},
    { "name": "aaa", "NAME": "YYY", "value": 11},
    { "name": "bbb", "NAME": "XXX", "value": 2},
    { "name": "bbb", "NAME": "ZZZ", "value": 22},
    { "name": "ccc", "NAME": "YYY", "value": 3},
    { "name": "ccc", "NAME": "ZZZ", "value": 33}
])json";

    std::string dstText = R"json({
    "aaa": { "XXX": 1, "YYY": 11},
    "bbb": { "XXX": 2, "ZZZ": 22},
    "ccc": { "YYY": 3, "ZZZ": 33}
})json";

    rapidjson::Document docSrc;
    COUT(jsonkit::read_string(docSrc, srcText), true);

    rapidjson::Document docDst;
    COUT(jsonkit::read_string(docDst, dstText), true);

    std::vector<std::string> key{"name", "NAME"};
    jsonkit::array2object(key, docSrc, docSrc.GetAllocator());
    COUT(jsonkit::stringfy(docSrc));
    COUT(docSrc == docDst, true);
}

DEF_TAST(transform_array2object_5, "test array2object, two level steps")
{
    DESC("lift two level key, step by step");
    std::string srcText = R"json([
    { "name": "aaa", "NAME": "XXX", "value": 1},
    { "name": "aaa", "NAME": "YYY", "value": 11},
    { "name": "bbb", "NAME": "XXX", "value": 2},
    { "name": "bbb", "NAME": "ZZZ", "value": 22},
    { "name": "ccc", "NAME": "YYY", "value": 3},
    { "name": "ccc", "NAME": "ZZZ", "value": 33}
])json";

    std::string dstText = R"json({
    "aaa": { "XXX": 1, "YYY": 11},
    "bbb": { "XXX": 2, "ZZZ": 22},
    "ccc": { "YYY": 3, "ZZZ": 33}
})json";

    rapidjson::Document docSrc;
    COUT(jsonkit::read_string(docSrc, srcText), true);

    rapidjson::Document docDst;
    COUT(jsonkit::read_string(docDst, dstText), true);

    jsonkit::array2object("name", docSrc, docSrc.GetAllocator());
    COUT(jsonkit::stringfy(docSrc));
    jsonkit::array2object("NAME", docSrc, docSrc.GetAllocator());
    COUT(jsonkit::stringfy(docSrc));
    COUT(docSrc == docDst, true);
}

DEF_TAST(transform_array2object_6, "test array2object, edeg slash")
{
    DESC("lift two level key with edeg slash");
    std::string srcText = R"json([
    { "name": "aaa", "NAME": "XXX", "value": 1},
    { "name": "aaa", "NAME": "YYY", "value": 11},
    { "name": "bbb", "NAME": "XXX", "value": 2},
    { "name": "bbb", "NAME": "ZZZ", "value": 22},
    { "name": "ccc", "NAME": "YYY", "value": 3},
    { "name": "ccc", "NAME": "ZZZ", "value": 33}
])json";

    std::string dstText = R"json({
    "aaa": { "XXX": 1, "YYY": 11},
    "bbb": { "XXX": 2, "ZZZ": 22},
    "ccc": { "YYY": 3, "ZZZ": 33}
})json";

    rapidjson::Document docSrc;
    COUT(jsonkit::read_string(docSrc, srcText), true);

    rapidjson::Document docDst;
    COUT(jsonkit::read_string(docDst, dstText), true);

    jsonkit::array2object("/name/NAME/", docSrc, docSrc.GetAllocator());
    COUT(jsonkit::stringfy(docSrc));
    COUT(docSrc == docDst, true);
}
