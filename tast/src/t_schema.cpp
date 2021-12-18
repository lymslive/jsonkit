#include "tinytast.hpp"
#include "jsonkit_plain.h"
#include "json_schema.h"

#include <fstream>
#include <sstream>
#include <regex>

// test generate schema from input{json}, that expect same as input{schema}
static
void test_form_schema(const std::string& json, const std::string& schema)
{
    bool bValidate = jsonkit::validate_schema(json, schema);
    COUT(bValidate, true);

    COUT(json);
    std::string genSchema;
    bool bGenerate = jsonkit::form_schema(json, genSchema);
    COUT(bGenerate, true);

    std::string denSchema;
    jsonkit::condense(schema, denSchema);
    COUT(genSchema, denSchema);
    COUT(genSchema == denSchema, true);
}

// test generate json from input{schema}, that expect same as input{json}
static
void test_from_schema(const std::string& schema, const std::string& json)
{
    bool bValidate = jsonkit::validate_schema(json, schema);
    COUT(bValidate, true);

    COUT(schema);
    std::string genJson;
    bool bGenerate = jsonkit::from_schema(schema, genJson);
    COUT(bGenerate, true);

    std::string denJson;
    jsonkit::condense(json, denJson);
    COUT(genJson, denJson);
    COUT(genJson == denJson, true);
}

// test if input {json} and {schema} is matched.
static
void test_json_match_schema(const std::string& json, const std::string& schema, bool bMatch)
{
    bool bFormat = false;
    bool bValidate = true;
    std::string outJson;

    // confirm input is valid json
    bFormat = jsonkit::condense(json, outJson);
    COUT(bFormat, true);
    bFormat = jsonkit::condense(schema, outJson);
    COUT(bFormat, true);

    bValidate = jsonkit::validate_schema(json, schema);
    COUT(bValidate, bMatch);
}

// test data directory from which read in json file
static std::string s_data_dir = "./data/";
static
bool util_read_json(const std::string& file, std::string& json)
{
    std::string path(s_data_dir);
    path += file;

    std::ifstream inFile(path.c_str());
    COUT(inFile.is_open(), true);
    if (!inFile.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string inStr(buffer.str());
    COUT(inStr.empty(), false);
    if (inStr.empty())
    {
        return false;
    }

    json.swap(inStr);
    return true;
}

// test matchess of json and schema from file
static
void test_schema_file(const char* pfJson, const char* pfSchema)
{
    DESC("read json/schema file and check validation");
    COUT(pfJson);
    COUT(pfSchema);

    std::string json;
    std::string schema;
    bool bReadFile = false;

    bReadFile = util_read_json(pfJson, json);
    COUT(bReadFile, true);
    bReadFile = util_read_json(pfSchema, schema);
    COUT(bReadFile, true);

    test_json_match_schema(json, schema, true);
}

/* -------------------------------------------------- */

DEF_TAST(schema1_gens, "test generate schema from sample")
{
    const char* pJson = NULL;
    const char* pSchema = NULL;

    pJson = "{\"aaa\": 1, \"bbb\": 2}";
    pSchema = "{ \"type\": \"object\", \"properties\": { \"aaa\": { \"type\": \"integer\" }, \"bbb\": { \"type\": \"integer\" } }, \"required\":[\"aaa\",\"bbb\"] }";
    test_form_schema(pJson, pSchema);

    pJson = "[1,2,3]";
    pSchema = "{ \"type\": \"array\", \"items\": { \"type\": \"integer\" } }";
    test_form_schema(pJson, pSchema);
}

DEF_TAST(schema2_gjson, "test generate json from schema")
{
    const char* pJson = NULL;
    const char* pSchema = NULL;

    pJson = "{\"aaa\": 0, \"bbb\": 0}";
    pSchema = "{ \"type\": \"object\", \"properties\": { \"aaa\": { \"type\": \"integer\" }, \"bbb\": { \"type\": \"integer\" } }, \"required\":[\"aaa\",\"bbb\"] }";
    test_from_schema(pSchema, pJson);

    pJson = "[0]";
    pSchema = "{ \"type\": \"array\", \"items\": { \"type\": \"integer\" } }";
    test_from_schema(pSchema, pJson);
}

DEF_TAST(schema3_validate, "test schema validate json")
{
    const char* pJson = NULL;
    const char* pSchema = NULL;

    DESC("schema for an object");
    pSchema = "{ \"type\": \"object\", \"properties\": { \"aaa\": { \"type\": \"integer\" }, \"bbb\": { \"type\": \"integer\" } }, \"required\":[\"aaa\",\"bbb\"] }";
    COUT(pSchema);

    pJson = "{\"aaa\": 0, \"bbb\": 0}";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, true);

    pJson = "{\"aaa\": 1, \"bbb\": 1, \"ccc\": 1}";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, true);

    pJson = "{\"aaa\": 0, \"bbb\": 1.0}";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, false);

    pJson = "{\"aaa\": 0, \"ccc\": 0}";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, false);

    DESC("schema for an int array");
    pSchema = "{ \"type\": \"array\", \"items\": { \"type\": \"integer\" } }";
    COUT(pSchema);

    pJson = "[0]";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, true);
    pJson = "[0,1,2]";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, true);

    pJson = "123";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, false);
    pJson = "[]"; // it not break the rule: array of int. so result true
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, true);
    pJson = "{}";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, false);

    pJson = "[0,true,false,false,1]";
    COUT(pJson);
    test_json_match_schema(pJson, pSchema, false);
}

DEF_TAST(schema4_file, "test schema from file")
{
    const char* pfJson = "sample1.json";
    const char* pfSchema = "schema1.json";
    test_schema_file(pfJson, pfSchema);
}

DEF_TAST(schema_flat, "test simple flat schema")
{

    DESC("validate each object key");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), true);
    COUT(error.empty(), true);
}

    DESC("lack of required key");
{
    std::string json = R"json({
    "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": false },
    { "name": "BBB", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error, "NO KEY /BBB");
}

    DESC("dismatch of non-required key");
{
    std::string json = R"json({
    "aaa": "1", "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": false },
    { "name": "bbb", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error, "NOT NUMBER /aaa");
}

    DESC("dismatch type of some key value");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "string", "required": true },
    { "name": "ccc", "type": "number", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error, "NOT STRING /bbb");
}

}

DEF_TAST(schema_flat_child, "test simple flat schema with children")
{
    DESC("validate json with nest object");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11",
    "ddd": {"eee":7, "fff":8.8}
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true },
    { "name": "ddd", "type": "object", "required": true, "children": [
      { "name": "eee", "type": "number", "required": true },
      { "name": "fff", "type": "number", "required": true }
    ] }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    COUT(jsonkit::validate_flat_schema(inJson, inSchema), true);
}

    DESC("validate json with nest object: lack of child key");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11",
    "ddd": {"eee":7, "fff":8.8}
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true },
    { "name": "ddd", "type": "object", "required": true, "children": [
      { "name": "eee", "type": "number", "required": true },
      { "name": "FFF", "type": "number", "required": true }
    ] }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    COUT(jsonkit::validate_flat_schema(inJson, inSchema), false);
}

    DESC("validate json with nest object: dismatch of child type");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11",
    "ddd": {"eee":7, "fff":8.8}
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true },
    { "name": "ddd", "type": "object", "required": true, "children": [
      { "name": "eee", "type": "number", "required": true },
      { "name": "fff", "type": "string", "required": true }
    ] }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    COUT(jsonkit::validate_flat_schema(inJson, inSchema), false);
}

    DESC("validate json with nest array");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11",
    "ddd": [{"eee":7, "fff":8.8}, {"eee":7, "fff":8.8}]
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true },
    { "name": "ddd", "type": "array", "required": true, "children": [
      { "name": "eee", "type": "number", "required": true },
      { "name": "fff", "type": "number", "required": true }
    ] }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    COUT(jsonkit::validate_flat_schema(inJson, inSchema), true);
}

    DESC("validate json with nest array");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11",
    "ddd": [{"eee":7, "fff":8.8}, {"eee":7, "fff":8.8}]
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true },
    { "name": "ddd", "type": "array", "required": true, "children": [
      { "name": "EEE", "type": "number", "required": true },
      { "name": "fff", "type": "number", "required": true }
    ] }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    COUT(jsonkit::validate_flat_schema(inJson, inSchema), false);
}

    DESC("validate json with nest array");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11",
    "ddd": [{"eee":7, "fff":8.8}, {"eee":7, "fff":8.8}]
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true },
    { "name": "ddd", "type": "array", "required": true, "children": [
      { "name": "eee", "type": "string", "required": true },
      { "name": "fff", "type": "number", "required": true }
    ] }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error, "NOT STRING /ddd/0/eee");
}

}

DEF_TAST(schema_flat_value, "test flat schema check value")
{
    DESC("check string minLength");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true, "minLength": 4 }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error.empty(), false);
}

    DESC("check string maxLength");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true, "maxLength": 2 }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error.empty(), false);
}

    DESC("check string pattern");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true, "pattern": "[a-z]+" }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error.empty(), false);
}

    DESC("check number minValue");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true, "minValue": 10 },
    { "name": "ccc", "type": "string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error.empty(), false);
}

    DESC("check number maxValue");
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true, "maxValue": 1 },
    { "name": "ccc", "type": "string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error.empty(), false);
}

}

DEF_TAST(schema_flat_array1n, "test flat schema: number or array")
{
    std::string json = R"json({
    "aaa": 1, "bbb":[2,3,4], "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number or array", "required": true},
    { "name": "ccc", "type": "string or array", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), true);
    COUT(error);
    COUT(error.empty(), true);
}

DEF_TAST(schema_flat_array2n, "test flat schema: array of number")
{
    std::string json = R"json({
    "aaa": 1, "bbb":[2,3,4], "ccc": ["c11"]
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "array of number", "required": true},
    { "name": "ccc", "type": "array of string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), true);
    COUT(error);
    COUT(error.empty(), true);
}

DEF_TAST(schema_flat_array3n, "test flat schema: array of number")
{
    std::string json = R"json({
    "aaa": 1, "bbb":234, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "array of number", "required": true},
    { "name": "ccc", "type": "string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error, "NOT ARRAY /bbb");
}

DEF_TAST(schema_flat_array4n, "test flat schema: array of number")
{
    std::string json = R"json({
    "aaa": 1, "bbb":234, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true},
    { "name": "ccc", "type": "array of string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error, "NOT ARRAY /ccc");
}

DEF_TAST(schema_flat_array5n, "test flat schema: array of number")
{
    std::string json = R"json({
    "aaa": 1, "bbb":234, "ccc": ["c11", 11]
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true},
    { "name": "ccc", "type": "array of string", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error, "NOT STRING /ccc/1");
}

DEF_TAST(schema_flat_array6n, "test flat schema: array of number")
{
    std::string json = R"json({
    "aaa": 1, "bbb":234, "ccc": ["c11", 11]
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true},
    { "name": "ccc", "type": "array of number", "required": true }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error, "NOT NUMBER /ccc/0");
}

DEF_TAST(schema_flat_regex1, "test flat schema check regexp")
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true, "pattern": "[a-zA-Z0-9._-]+" }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), true);
    COUT(error);
    COUT(error.empty(), true);
}

DEF_TAST(schema_flat_regex2, "test flat schema check regexp")
{
    std::string json = R"json({
    "aaa": 1, "bbb":2, "ccc": "c11#"
})json";

    std::string schema = R"json([
    { "name": "aaa", "type": "number", "required": true },
    { "name": "bbb", "type": "number", "required": true },
    { "name": "ccc", "type": "string", "required": true, "pattern": "[a-zA-Z0-9._-]+" }
])json";

    rapidjson::Document inJson;
    inJson.Parse(json.c_str(), json.size());
    COUT(inJson.HasParseError(), false);

    rapidjson::Document inSchema;
    inSchema.Parse(schema.c_str(), schema.size());
    COUT(inSchema.HasParseError(), false);

    std::string error;
    COUT(jsonkit::validate_flat_schema(inJson, inSchema, error), false);
    COUT(error);
    COUT(error.empty(), false);
}
