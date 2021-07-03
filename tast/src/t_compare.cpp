#include "tinytast.hpp"
#include "jsonkit_plain.h"

static
void test_compare_json(const std::string& ajson, const std::string& bjson, bool result)
{
    COUT(ajson);
    COUT(bjson);
    bool bEqual = jsonkit::compare(ajson, bjson);
    COUT(bEqual, result);
}

DEF_TAST(compare1_equal, "test two equal json")
{
    std::string ajson;
    std::string bjson;

    ajson = "{\"aaa\": 1, \"bbb\": 2}";
    bjson = "{\"bbb\": 2, \"aaa\": 1}";
    test_compare_json(ajson, bjson, true);

    ajson = "[1, 2, 3]";
    bjson = "[1,2,3]";
    test_compare_json(ajson, bjson, true);

    ajson = "123";
    bjson = "123";
    test_compare_json(ajson, bjson, true);

    ajson = "123.321";
    bjson = "123.321";
    test_compare_json(ajson, bjson, true);

    ajson = "false";
    bjson = "false";
    test_compare_json(ajson, bjson, true);

    ajson = "null";
    bjson = "null";
    test_compare_json(ajson, bjson, true);

    ajson = "{}";
    bjson = "{}";
    test_compare_json(ajson, bjson, true);

    ajson = "[]";
    bjson = "[]";
    test_compare_json(ajson, bjson, true);
}

DEF_TAST(compare2_notequal, "test two not equal json")
{
    std::string ajson;
    std::string bjson;

    ajson = "{\"aaa\": 1, \"bbb\": 2}";
    bjson = "{\"bbb\": 1, \"aaa\": 2}";
    test_compare_json(ajson, bjson, false);
    bjson = "{\"aaa\": 1, \"bbb\": 2.0}";
    test_compare_json(ajson, bjson, false);
    bjson = "{\"aaa\": 1, \"bbb\": \"2\"}";
    test_compare_json(ajson, bjson, false);
    bjson = "{\"aaa\": 1, \"bbb\": 2, \"ccc\": 3}";
    test_compare_json(ajson, bjson, false);
    bjson = "{\"aaa\": 1}";
    test_compare_json(ajson, bjson, false);

    ajson = "[1, 2, 3]";
    bjson = "[1,2,3,4]";
    test_compare_json(ajson, bjson, false);
    bjson = "[1,2]";
    test_compare_json(ajson, bjson, false);
    bjson = "[1.0,2.0,3.0]";
    test_compare_json(ajson, bjson, false);

    ajson = "123";
    bjson = "123.0";
    test_compare_json(ajson, bjson, false);

    ajson = "null";
    bjson = "false";
    test_compare_json(ajson, bjson, false);

    ajson = "{}";
    bjson = "[]";
    test_compare_json(ajson, bjson, false);
}

static
void test_json_compatible(const std::string& ajson, const std::string& bjson, bool result)
{
    COUT(ajson);
    COUT(bjson);
    bool bCompatible = jsonkit::compatible(ajson, bjson);
    COUT(bCompatible, result);
}

DEF_TAST(compare3_compatible, "test two json if compatible")
{
    std::string ajson;
    std::string bjson;

    ajson = "{\"aaa\": 1, \"bbb\": 2}";
    bjson = "{\"bbb\": 2, \"aaa\": 1}";
    test_json_compatible(ajson, bjson, true);
    ajson = "{\"aaa\": 1, \"bbb\": 2, \"ccc\": 3}";
    test_json_compatible(ajson, bjson, true);
    ajson = "{\"aaa\": 1, \"bbb\": 2.0}";
    test_json_compatible(ajson, bjson, false);
    ajson = "{\"aaa\": 1}";
    test_json_compatible(ajson, bjson, false);

    ajson = "[1, 2, 3]";
    bjson = "[1,2,3]";
    test_json_compatible(ajson, bjson, true);
    ajson = "[1, 2, 3, 4]";
    test_json_compatible(ajson, bjson, true);
    ajson = "[1, 2]";
    test_json_compatible(ajson, bjson, false);
    ajson = "[1, 2, 3.0]";
    test_json_compatible(ajson, bjson, false);

    ajson = "123";
    bjson = "123";
    test_json_compatible(ajson, bjson, true);
    ajson = "123.0";
    test_json_compatible(ajson, bjson, false);
}
