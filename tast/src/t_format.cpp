#include "tinytast.hpp"
#include "jsonkit_plain.h"

DEF_TAST(format1, "格式化测试")
{
	std::string json;
	std::string strPretty;
	std::string strCompress;
	bool bRet;

	DESC("处理 object");
	json = "{\"aaa\": 1, \"bbb\": 2}";
	bRet = jsonkit::prettify(json, strPretty);
	COUT(bRet, true);
	COUT(strPretty);
	bRet = jsonkit::condense(json, strCompress);
	COUT(bRet, true);
	COUT(strCompress);

	DESC("处理 array");
	json = "[1, 2, 3]";
	bRet = jsonkit::prettify(json, strPretty);
	COUT(bRet, true);
	COUT(strPretty);
	bRet = jsonkit::condense(json, strCompress);
	COUT(bRet, true);
	COUT(strCompress);
}
