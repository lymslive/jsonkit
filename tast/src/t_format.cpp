#include "tinytast.hpp"
#include "jsonkit_plain.h"

DEF_TAST(format1_common, "test normal json format")
{
	std::string json;
	std::string strPretty;
	std::string strCompress;
	bool bRet = false;

	DESC("deal an object");
	const char* pObject = "{\"aaa\": 1, \"bbb\": 2}";
	const char* pObjectExpect = "{\"aaa\":1,\"bbb\":2}";
    json = pObject;
	bRet = jsonkit::prettify(json, strPretty);
	COUT(bRet, true);
	COUT(strPretty);
	bRet = jsonkit::condense(json, strCompress);
	COUT(bRet, true);
	COUT(strCompress, pObjectExpect);

    DESC("reverse condesed format to pretty");
    bRet = jsonkit::prettify(strCompress, json);
    COUT(bRet, true);
    COUT(json == strPretty, true);

	DESC("deal an array");
    const char* pArray = "[1,\ttrue ,\n3.0, null]";
    const char* pArrayExpect = "[1,true,3.0,null]";
	json = pArray;
	bRet = jsonkit::prettify(json, strPretty);
	COUT(bRet, true);
	COUT(strPretty);
	bRet = jsonkit::condense(json, strCompress);
	COUT(bRet, true);
	COUT(strCompress, pArrayExpect);
    bRet = jsonkit::prettify(strCompress, json);
    COUT(bRet, true);
    COUT(json, strPretty);

    DESC("reverse condesed format to pretty");
    bRet = jsonkit::prettify(strCompress, json);
    COUT(bRet, true);
    COUT(json == strPretty, true);
}

static
void test_invalid_json(const std::string& json)
{
    std::string outJson;
    bool bRet = false;
    COUT(json);

    bRet = jsonkit::prettify(json, outJson);
    COUT(bRet, false);
    COUT(outJson.empty(), true);

    bRet = jsonkit::condense(json, outJson);
    COUT(bRet, false);
    COUT(outJson.empty(), true);
}

DEF_TAST(format2_nojson, "test invalid json format")
{
    test_invalid_json("any string");
    test_invalid_json("[123, 321");
    test_invalid_json("{abc:1, edf:2}");
}

static
void test_scalar_json(const std::string& json)
{
    std::string outJson;
    bool bRet = false;
    COUT(json);

    bRet = jsonkit::prettify(json, outJson);
    COUT(bRet, true);
    COUT(outJson, json);

    bRet = jsonkit::condense(json, outJson);
    COUT(bRet, true);
    COUT(outJson, json);
}

DEF_TAST(format3_scalar, "test scalar json value")
{
    test_scalar_json("\"json string\"");
    test_scalar_json("123");
    test_scalar_json("-3.14");
    test_scalar_json("false");
    test_scalar_json("null");

    DESC("empty array or object also farmat as scalar value");
    test_scalar_json("{}");
    test_scalar_json("[]");
}

