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
    "ddd": {"eee":7, "fff":8.8},
    "eee": [1,2,3]
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
    },
    "dsize": "=JSON_SIZEFY/ddd",
    "esize": "=JSON_SIZEFY/eee",
    "estr": "=JSON_STRINGFY/eee"
})json";

    DESC("register slot function: ADD");
    jsonkit::slot_register("ADD", add_one);

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

    DESC("directly use pre-defined slot: JSON_SIZEFY");
    COUT(docDst/"dsize" | 0, 2);
    COUT(docDst/"esize" | 0, 3);

    DESC("directly use pre-defined slot: JSON_STRINGFY");
    COUT(docDst/"estr" | std::string(), "[1,2,3]");
}

DEF_TAST(transform_fill_array1, "fill with array")
{
    std::string srcText = R"json({
    "aaa": 1, "bbb":2,
    "ccc": "c11",
    "ddd": {"eee":7, "fff":8.8},
    "eee": [1,2,3],
    "fff": [4,5,6,7]
})json";

    std::string dstText = R"json({
    "AAA": "static",
    "array": ["/aaa","/bbb","/ddd/eee","normal"], 
    "aofa": ["=[?]", ["/aaa", "/eee/?"]],
    "aofo": ["={?}", {
       "aaa": "/aaa",
       "eee": "/eee/?"
    }],
    "aofo2": ["={?}", {
       "eee": "/eee/?",
       "fff": "/fff/?"
    }],
    "aofn": ["=[?]", ["/aaa", "/bbb"]]
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

    COUT((docDst/"array").IsArray(), true);
    COUT((docDst/"array").Size(), 4);
    COUT(docDst/"array"/0 | 0, 1);
    COUT(docDst/"array"/1 | 0, 2);
    COUT(docDst/"array"/2 | 0, 7);

    COUT((docDst/"aofa").IsArray(), true);
    COUT((docDst/"aofa"/0).IsArray(), true);
    COUT((docDst/"aofa"/0/0) | 0, 1);
    COUT((docDst/"aofa"/0/1) | 0, 1);
    COUT((docDst/"aofa"/1/0) | 0, 1);
    COUT((docDst/"aofa"/1/1) | 0, 2);
    COUT((docDst/"aofa"/2/0) | 0, 1);
    COUT((docDst/"aofa"/2/1) | 0, 3);

    COUT((docDst/"aofo").IsArray(), true);
    COUT((docDst/"aofo"/0).IsObject(), true);
    COUT((docDst/"aofo"/0/"aaa") | 0, 1);
    COUT((docDst/"aofo"/0/"eee") | 0, 1);
    COUT((docDst/"aofo"/1/"aaa") | 0, 1);
    COUT((docDst/"aofo"/1/"eee") | 0, 2);
    COUT((docDst/"aofo"/2/"aaa") | 0, 1);
    COUT((docDst/"aofo"/2/"eee") | 0, 3);

    COUT((docDst/"aofo2").IsArray(), true);
    COUT((docDst/"aofo2"/0).IsObject(), true);
    COUT((docDst/"aofo2"/0/"fff") | 0, 4);
    COUT((docDst/"aofo2"/0/"eee") | 0, 1);
    COUT((docDst/"aofo2"/1/"fff") | 0, 5);
    COUT((docDst/"aofo2"/1/"eee") | 0, 2);
    COUT((docDst/"aofo2"/2/"fff") | 0, 6);
    COUT((docDst/"aofo2"/2/"eee") | 0, 3);

    DESC("no ? in array expansion!!");
    COUT((docDst/"aofn").IsArray(), true);
    COUT((docDst/"aofn").Size(), 0);
}

DEF_TAST(transform_fill_array2, "fill with longer array")
{
    std::string srcText = R"json({
    "aaa": [1,2,3,4,5,6,7,8,9,10,11,12],
    "bbb": [-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12]
})json";

    std::string dstText = R"json({
    "aofa": ["=[?]", ["/aaa/?", "/bbb/?"]]
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

    COUT((docDst/"aofa").IsArray(), true);
    COUT((docDst/"aofa").Size(), 12);
    COUT((docDst/"aofa"/0).IsArray(), true);
    COUT((docDst/"aofa"/0/0) | 0, 1);
    COUT((docDst/"aofa"/0/1) | 0, -1);
    COUT((docDst/"aofa"/10).IsArray(), true);
    COUT((docDst/"aofa"/10/0) | 0, 11);
    COUT((docDst/"aofa"/10/1) | 0, -11);
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
