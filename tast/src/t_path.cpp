#include "tinytast.hpp"
#include "json_path.h"
#include "json_input.h"
#include "json_output.h"

DEF_TAST(path_attach_int, "attach a number")
{
    rapidjson::Document doc;

    DESC("can only attach to object");
    rapidjson::Value node(1);
    bool succ = jsonkit::path_attach(node, "/aaa/bbb/ccc", doc, doc.GetAllocator());
    COUT(succ, false);
    COUT(node.IsInt(), true);
    COUT(node.GetInt(), 1);

    DESC("after attach succ, node is moved and reset to null");
    doc.SetObject();
    succ = jsonkit::path_attach(node, "/aaa/bbb/ccc", doc, doc.GetAllocator());
    COUT(succ, true);
    COUT(node.IsNull(), true);
    COUT(jsonkit::stringfy(doc));

    const rapidjson::Value* pNode = jsonkit::path_point(doc, "/aaa/bbb/ccc");
    COUT(pNode != nullptr, true);
    COUT(pNode->IsInt(), true);
    COUT(pNode->GetInt(), 1);

    pNode = jsonkit::path_point(doc, "/aaa/bbb");
    COUT(pNode != nullptr, true);
    COUT(pNode->IsObject(), true);

    pNode = jsonkit::path_point(doc, "/aaa");
    COUT(pNode != nullptr, true);
    COUT(pNode->IsObject(), true);
}

DEF_TAST(path_attach_str, "attach a string")
{
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Value node("a literal string");
    bool succ = jsonkit::path_attach(node, "/aaa/bbb/ccc", doc, doc.GetAllocator());
    COUT(succ, true);
    COUT(node.IsNull(), true);
    COUT(jsonkit::stringfy(doc));

    const rapidjson::Value* pNode = jsonkit::path_point(doc, "/aaa/bbb/ccc");
    COUT(pNode != nullptr, true);
    COUT(pNode->IsString(), true);
    COUT(pNode->GetString());

    node = 100;
    succ = jsonkit::path_attach(node, "/aaa/bbb/ddd", doc, doc.GetAllocator());
    COUT(succ, true);
    COUT(jsonkit::stringfy(doc));

    const char* psz = "another string";
    node.SetString(psz, doc.GetAllocator());
    succ = jsonkit::path_attach(node, "/aaa/bbb/ddd", doc, doc.GetAllocator());
    COUT(succ, false);

    succ = jsonkit::path_attach(node, "/aaa/BBB", doc, doc.GetAllocator());
    COUT(succ, true);
    COUT(jsonkit::stringfy(doc));
}

DEF_TAST(path_attach_node, "attach a node")
{
    rapidjson::Document doc;
    doc.SetObject();

    std::string text = R"json({
    "aaa": 1, "bbb":[2,3,4], "ccc": "c11",
    "ddd": {"eee":7, "fff":8.8}
})json";

    // attached node should use the same allocator
    rapidjson::Document node(&doc.GetAllocator());
    bool succ = jsonkit::read_string(node, text);
    COUT(succ, true);
    COUT(node.IsObject(), true);

    succ = jsonkit::path_attach(node, "/AA/BB/CC", doc, doc.GetAllocator());
    COUT(succ, true);
    COUT(node.IsNull(), true);
    COUT(jsonkit::stringfy(doc));

    const rapidjson::Value* pNode = jsonkit::path_point(doc, "/AA/BB/CC/aaa");
    COUT(pNode != nullptr, true);
    COUT(pNode->IsInt(), true);
    COUT(pNode->GetInt(), 1);

    pNode = jsonkit::path_point(doc, "/AA/BB/CC/bbb");
    COUT(pNode != nullptr, true);
    COUT(pNode->IsArray(), true);
    COUT(jsonkit::stringfy(*pNode), "[2,3,4]");

    pNode = jsonkit::path_point(doc, "/AA/BB/CC/ddd/eee");
    COUT(pNode != nullptr, true);
    COUT(pNode->IsInt(), true);
    COUT(pNode->GetInt(), 7);
}
