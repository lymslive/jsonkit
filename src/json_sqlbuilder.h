/** 
 * @file json_sqlbuilder.h 
 * @author lymslive
 * @date 2021-10-30 / 2021-11-22
 * @brief generate SQL statement from json structrue.
 * */

#ifndef JSON_SQLBUILDER_H__
#define JSON_SQLBUILDER_H__

#include "rapidjson/document.h"

namespace jsonkit
{
    
/** add '%' postfix in like statement */
const short SQL_LIKE_POSTFIX = 1;
/** add '%' prefix in like statement */
const short SQL_LIKE_PREFIX = 2;

/** config some behavior of sql generation */
struct sql_config_t
{
    /** how fix like expression, add % to which end,
     * can bit or of SQL_LIKE_POSTFIX, SQL_LIKE_PREFIX */
    short fix_like_value = SQL_LIKE_POSTFIX;

    /** not generate delete sql if without where clause */
    bool refuse_delete_without_where = true;

    /** not generate update sql if without where clause */
    bool refuse_update_without_where = true;
};

/** set the internal static sql generation config.
 * @param cfg: pointer for sql config struct, can be nullptr to only get.
 * @return the original config.
 * @note not thread safe, better only change config on init stage if really
 * need some value different from default. 
 * @code
 * jsonkit::sql_config_t cfg = jsonkit::set_sql_config(nullptr);
 * cfg.fix_like_value = jsonkit::SQL_LIKE_POSTFIX | jsonkit::SQL_LIKE_PREFIX;
 * cfg.refuse_delete_without_where = false;
 * cfg.refuse_update_without_where = false;
 * cfg = set_sql_config(&cfg);
 * ... use new config, then restore to origin config
 * set_sql_config(&cfg);
 * @endcode
 * */
sql_config_t set_sql_config(const sql_config_t* cfg);

/** build sql statement from specific json struct.
 * @param json: input json object to describe sql element
 * @param sql: output sql as string, append to end of it
 * @return bool: true if generate sql succ, otherwise false.
 *
 * @note If the input json is incorrect by mistake or intentional injection,
 * it hope to report fail, but may reverse some portion string, result in
 * incomplete sql statement.
 * @note You can also generate two or more valid sql statement to a single
 * stirng output buffer, provided you manually append a ';' between each.
 *
 * @details Supported INSERT example structure as:
 * { "table": "t_name", "value": {"f_1":"v_1", "f_2":"v_2"} }
 * { "table": "t_name", "value": [{},{}] } // value is array of object
 * { "table": "t_name", "head": ["f1","f2"], "value": [["v1","V2"],[1,2]] }
 * */
bool sql_insert(const rapidjson::Value& json, std::string& sql);

/** same as sql_insert() but use REPLACE instead of INSERT */
bool sql_replace(const rapidjson::Value& json, std::string& sql);

/** UPDATE statement generation.
 * @details supported example structure:
 * { "table":"t_name", "value": {"f1":1,"f2":2}, "where":{}, "order":"", "limit":""}
 *
 * WHERE clause example:
 * {"f1":"v1", "f2": [1,2,3], "f3":{"like":"xx%", "gt":">v", "lt":"<v"}}
 * LIMIT clause example (offset may ignore):
 * {"limit":10}, {"limit":[100,10]}, {"limit": {"offset":100,"count":10}}
 * ORDER clause can be simple string: "id DESC"
 * */
bool sql_update(const rapidjson::Value& json, std::string& sql);

/** SELECT statement generation.
 * @details supported example structure:
 * { "table":"t_name", "field":["f1","f2"], "Where":{}, "order":"", "limit":""}
 * { "table":"t_name", "field":"f1,f2,f3", "Where":{}, "order":"", "limit":""}
 * {"table":"t1 JOIN t2 USING(id)", "field":"...", "group":"f1", "having":{}}
 * */
bool sql_select(const rapidjson::Value& json, std::string& sql);

/** Specical SELECT COUNT(1) statement to count row number of a table.
 * @details example: {"table":"t1", "where":{}}
 * */
bool sql_count(const rapidjson::Value& json, std::string& sql);

/** DELETE statment generation.
 * @details example: {"table":"t1", "where":{}, "order":"", "limit":""}
 * */
bool sql_delete(const rapidjson::Value& json, std::string& sql);

/** class interface for SQL builder.
 * @details A CSqlbuilder object can keep individual config to custome some
 * behavior for later sql generation. Then each method is the same use as the
 * free function sql_xxx();
 * */
struct CSqlBuilder
{
    bool Insert(const rapidjson::Value& json, std::string& sql);
    bool Replace(const rapidjson::Value& json, std::string& sql);
    bool Update(const rapidjson::Value& json, std::string& sql);
    bool Select(const rapidjson::Value& json, std::string& sql);
    bool Count(const rapidjson::Value& json, std::string& sql);
    bool Delete(const rapidjson::Value& json, std::string& sql);

    sql_config_t& Config() { return m_config; }
    sql_config_t m_config;
};

} /* jsonkit */ 

#endif /* end of include guard: JSON_SQLBUILDER_H__ */
