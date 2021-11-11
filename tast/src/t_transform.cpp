#include "tinytast.hpp"
#include "json_transform.h"
#include "json_operator.h"

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

