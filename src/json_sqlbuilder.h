/** 
 * @file json_sqlbuilder.h 
 * @author lymslive
 * @date 2021-10-30
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

/** build sql statement from specific json struct */
bool sql_insert(const rapidjson::Value& json, std::string& sql);
bool sql_update(const rapidjson::Value& json, std::string& sql);
bool sql_select(const rapidjson::Value& json, std::string& sql);
bool sql_count(const rapidjson::Value& json, std::string& sql);
bool sql_delete(const rapidjson::Value& json, std::string& sql);

} /* jsonkit */ 

#endif /* end of include guard: JSON_SQLBUILDER_H__ */
