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
}

