#include "json_sqlbuilder.h"
#include "json_operator.h"
#include "jsonkit_rpdjn.h"

#include "jsonkit_internal.h"

namespace jsonkit
{

/* ************************************************************ */
// static helper functions

static sql_config_t s_config;

sql_config_t set_sql_config(const sql_config_t* cfg)
{
    if (!cfg)
    {
        return s_config;
    }
    sql_config_t old = s_config;
    s_config = *cfg;
    return old;
}

const char SINGLE_QUOTE = '\'';

/** short for chained string.appen(?).append(?)....
 * @note incorrect: string += ? += ? += ...
 * */
template <typename T>
std::string& STRCAT(std::string& str, T&& first)
{
    return str.append(first);
}

template <typename T, typename... Types>
std::string& STRCAT(std::string& str, T&& first, Types&&... rest)
{
    return STRCAT(str.append(first), rest...);
}

// simple discard may dangerous string
bool sql_check_word(std::string& sql)
{
    if (sql.find_first_of("';") != std::string::npos)
    {
        sql.clear();
        return false;
    }
    return true;
}

std::string sql_simple_word(const rapidjson::Value& json, const char* prefix = nullptr)
{
    if (!json || !json.IsString())
    {
        return "";
    }
    std::string word = json.GetString();
    if (!word.empty() && sql_check_word(word) && prefix)
    {
        return std::string(prefix).append(word);
    }
    return word;
}

inline
std::string sql_table(const rapidjson::Value& json)
{
    return sql_simple_word(json);
}

inline
std::string sql_order(const rapidjson::Value& json)
{
    return sql_simple_word(json, "ORDER BY ");
}

inline
std::string sql_group(const rapidjson::Value& json)
{
    return sql_simple_word(json, "GROUP BY ");
}

std::string sqlfy_value(const rapidjson::Value& json)
{
    if (json.IsObject() || json.IsArray())
    {
        return "";
    }

    if (json.IsString())
    {
        std::string sql;
        sql += SINGLE_QUOTE;
        for (const char* str = json.GetString(); *str != '\0'; ++str)
        {
            if (*str == SINGLE_QUOTE)
            {
                sql += SINGLE_QUOTE;
                sql += SINGLE_QUOTE;
            }
            else
            {
                sql += *str;
            }
        }
        sql += SINGLE_QUOTE;

        // keep origin string in `` quote, eg. `now()` `null`
        if (sql.size() > 4 && sql[1] == '`' && sql[sql.size()-2] == '`')
        {
            // extrace '`real-input-string`'
            sql = sql.substr(2, sql.size()-4);
            sql_check_word(sql);
        }

        return sql;
    }

    if (json.IsBool())
    {
        if (json.GetBool())
        {
            return "1";
        }
        else
        {
            return "0";
        }
    }

    if (json.IsNull())
    {
        return "null";
    }

    // numeric value
    return stringfy(json);
}

std::string sql_field(const rapidjson::Value& json)
{
    if (!json)
    {
        return "*";
    }
    else if (json.IsString())
    {
        std::string field = json.GetString();
        sql_check_word(field);
        return field;
    }
    else if (json.IsArray())
    {
        std::string sql;
        for (auto it = json.Begin(); it != json.End(); ++it)
        {
            if (it->IsString())
            {
                std::string field = it->GetString();
                if (!sql_check_word(field))
                {
                    return "";
                }
                sql.append(field).append(",");
            }
            else
            {
                return "";
            }
        }
        if (!sql.empty())
        {
            sql.pop_back(); // remove the last ,
        }
        return sql;
    }

    return "";
}

/** Generate: SET field1=value1, field2=value2, ... */
std::string sql_set(const rapidjson::Value& json)
{
    std::string sql;
    sql += "SET ";
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
    {
        // skip json null, if really mean to set null use string "`null`"
        if (it->value.IsNull())
        {
            continue;
        }

        std::string field = it->name.GetString();
        if (!sql_check_word(field))
        {
            return "";
        }
        STRCAT(sql, field, "=", sqlfy_value(it->value), ",");
    }
    sql.pop_back();
    return sql;
}

/** Generate: (field1, feild2, ...) VALUES (value1, value2, ...), ...
 * @param json: array of object which has the same fields set
 * */
std::string sql_values(const rapidjson::Value& json)
{
    auto& first = json[0];
    if (!first.IsObject() || first.ObjectEmpty())
    {
        return "";
    }

    std::vector<std::string> field;
    std::string value;
    for (auto it = first.MemberBegin(); it != first.MemberEnd(); ++it)
    {
        if (it->value.IsNull())
        {
            continue;
        }

        std::string fname = it->name.GetString();
        if (!sql_check_word(fname))
        {
            return "";
        }
        field.push_back(fname);
        STRCAT(value, sqlfy_value(it->value), ",");
    }
    if (!value.empty())
    {
        value.pop_back();
    }

    std::string fieldstr;
    for (auto& item : field)
    {
        STRCAT(fieldstr, item, ",");
    }
    if (!fieldstr.empty())
    {
        fieldstr.pop_back();
    }

    std::string sql;
    STRCAT(sql, "(", fieldstr, ")", " VALUES ");
    STRCAT(sql, "(", value, ")");

    int size = json.Size();
    for (int i = 1; i < size; ++i)
    {
        auto& row = json[i];
        value.clear();
        for (auto& item : field)
        {
            STRCAT(value, sqlfy_value(row/item), ",");
        }
        if (!value.empty())
        {
            value.pop_back();
        }
        STRCAT(sql, ", (", value, ")");
    }

    return sql;
}

/** generate a list of value (va1,val2,...) from json array */
std::string sql_in(const rapidjson::Value& json)
{
    std::string sql("(");
    for (auto it = json.Begin(); it != json.End(); ++it)
    {
        STRCAT(sql, sqlfy_value(*it), ",");
    }
    if (sql.size() > 1)
    {
        sql[sql.size()-1] = ')';
    }
    return sql;
}

/** Generate multiple comparision
 * @code
 * "field": {
 *   "eq": ..., 
 *   "ne": ...,
 *   "gt": ...,
 *   "lt": ...,
 *   "ge": ...,
 *   "le": ...,
 *   "like": ...,
 *   "null": true/false
 * }
 * @endcode
 * */
std::string sql_mulcmp(const rapidjson::Value& json, const std::string& field)
{
    std::string sql;
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
    {
        if (it->value.IsNull())
        {
            continue;
        }

        std::string op = it->name.GetString();
        if (op == "eq")
        {
            STRCAT(sql, " AND ", field, "=", sqlfy_value(it->value));
        }
        else if (op == "ne")
        {
            STRCAT(sql, " AND ", field, "!=", sqlfy_value(it->value));
        }
        else if (op == "gt")
        {
            STRCAT(sql, " AND ", field, ">", sqlfy_value(it->value));
        }
        else if (op == "lt")
        {
            STRCAT(sql, " AND ", field, "<", sqlfy_value(it->value));
        }
        else if (op == "ge")
        {
            STRCAT(sql, " AND ", field, ">=", sqlfy_value(it->value));
        }
        else if (op == "le")
        {
            STRCAT(sql, " AND ", field, "<=", sqlfy_value(it->value));
        }
        else if (op == "like")
        {
            std::string like = sqlfy_value(it->value);
            if (like.size() > 2 && like.front() == SINGLE_QUOTE && like.back() == SINGLE_QUOTE)
            {
                if (s_config.fix_like_value)
                {
                    std::string fix;
                    fix.push_back(SINGLE_QUOTE);
                    if (s_config.fix_like_value & SQL_LIKE_PREFIX)
                    {
                        fix.push_back('%');
                    }
                    fix += like.substr(1, like.size()-2);
                    if (s_config.fix_like_value & SQL_LIKE_POSTFIX)
                    {
                        fix.push_back('%');
                    }
                    fix.push_back(SINGLE_QUOTE);
                    like.swap(fix);
                }
                STRCAT(sql, " AND ", field, " like ", like);
            }
        }
        else if (op == "null")
        {
            if (it->value.IsBool() && it->value.GetBool())
            {
                STRCAT(sql, " AND ", field, " is NULL");
            }
            else
            {
                STRCAT(sql, " AND ", field, " is not NULL");
            }
        }
    }
    return sql;
}

/** generate where statemnet if possible, otherwise emtpy stirng on failure
 * @code 
 * {
 *   "field1" : "value1",     // AND field1='value1'
 *   "field2" : [v1, v2, v3], // AND field2 IN (v1,v2,v3)
 *   "field3" : {             // AND field3 > min-value AND field3 < max-value
 *     "gt" : "min-value",
 *     "lt" : "max-value"
 *   }
 * }
 * @endcode 
 * */
std::string sql_where(const rapidjson::Value& json, const char* prefix = "WHERE")
{
    if (!json || !json.IsObject() || json.ObjectEmpty())
    {
        return "";
    }

    std::string sql(prefix);
    sql.append(" 1=1");
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
    {
        if (it->value.IsNull())
        {
            continue;
        }

        std::string field = it->name.GetString();
        if (!sql_check_word(field))
        {
            return "";
        }

        if (it->value.IsArray() && it->value.Empty() == false)
        {
            std::string in = sql_in(it->value);
            if (in.size() > 2)
            {
                STRCAT(sql, " AND ", field, " IN ", in);
            }
        }
        else if (it->value.IsObject() && it->value.ObjectEmpty() == false)
        {
            std::string cmp = sql_mulcmp(it->value, field);
            if (!cmp.empty())
            {
                STRCAT(sql, cmp);
            }
        }
        else
        {
            STRCAT(sql, " AND ", field, "=", sqlfy_value(it->value));
        }
    }

    return sql;
}

// only return valid limit value, otherwise empty string
std::string sql_limit_valid(const rapidjson::Value& json)
{
    if (json.IsUint() || json.IsUint64())
    {
        return stringfy(json);
    }
    return "";
}

std::string sql_limit(const rapidjson::Value& json)
{
    std::string sql("LIMIT ");
    std::string limit = sql_limit_valid(json);
    if (!limit.empty())
    {
        return STRCAT(sql, limit);
    }

    std::string offset;
    std::string count;
    if (json.IsArray() && json.Size() == 2)
    {
        offset = sql_limit_valid(json[0]);
        count = sql_limit_valid(json[1]);
    }
    else if (json.IsObject())
    {
        offset = sql_limit_valid(json / "offset");
        count = sql_limit_valid(json / "count");
    }

    if (!count.empty())
    {
        if (!offset.empty())
        {
            return STRCAT(sql, offset, ",", count);
        }
        return STRCAT(sql, count);
    }

    return "";
}

/* ************************************************************ */
// public functions

bool sql_insert(const rapidjson::Value& json, std::string& sql)
{
    std::string table = sql_table(json/"table");
    if (table.empty())
    {
        LOGD("invalid table name: %s", table.c_str());
        return false;
    }

    const rapidjson::Value& value = json / "value";
    if (!value)
    {
        LOGD("no value to insert??");
        return false;
    }

    std::string set;
    if (value.IsObject() && !value.ObjectEmpty())
    {
        set = sql_set(value);
    }
    else if (value.IsArray() && !value.Empty())
    {
        set = sql_values(value);
    }

    if (set.empty())
    {
        LOGD("fail to generate set statement!!");
        return false;
    }

    STRCAT(sql, "INSERT INTO ", table, " ", set);

    // need to select last auto increment id after insert
    if (json/"last_insert_id" | false)
    {
        STRCAT(sql, "; SELECT last_insert_id()");
    }

    return true;
}

bool sql_update(const rapidjson::Value& json, std::string& sql)
{
    std::string table = sql_table(json/"table");
    if (table.empty())
    {
        return false;
    }

    const rapidjson::Value& value = json / "value";
    if (!value)
    {
        return false;
    }

    std::string set;
    if (value.IsObject() && !value.ObjectEmpty())
    {
        set = sql_set(value);
    }
    if (set.empty())
    {
        return false;
    }

    std::string where = sql_where(json/"where");
    if (where.empty() && s_config.refuse_update_without_where)
    {
        return false;
    }

    STRCAT(sql, "UPDATE ", table, " ", set);
    if (!where.empty())
    {
        STRCAT(sql, " ", where);
    }

    std::string order = sql_order(json/"order");
    if (!order.empty())
    {
        STRCAT(sql, " ", order);
    }

    std::string limit = sql_limit(json/"limit");
    if (!limit.empty())
    {
        STRCAT(sql, " ", limit);
    }

    return true;
}

bool sql_select(const rapidjson::Value& json, std::string& sql)
{
    std::string table = sql_table(json/"table");
    if (table.empty())
    {
        return false;
    }

    std::string field = sql_field(json/"field");
    if (field.empty())
    {
        return false;
    }


    STRCAT(sql, "SELECT ", field, " FROM ", table);

    std::string where = sql_where(json/"where");
    if (!where.empty())
    {
        STRCAT(sql, " ", where);
    }

    std::string group = sql_group(json/"group");
    if (!group.empty())
    {
        STRCAT(sql, " ", group);
        std::string having = sql_where(json/"having", "HAVING");
        if (!having.empty())
        {
            STRCAT(sql, " ", having);
        }
    }

    std::string order = sql_order(json/"order");
    if (!order.empty())
    {
        STRCAT(sql, " ", order);
    }

    std::string limit = sql_limit(json/"limit");
    if (!limit.empty())
    {
        STRCAT(sql, " ", limit);
    }

    return true;
}

bool sql_count(const rapidjson::Value& json, std::string& sql)
{
    std::string table = sql_table(json/"table");
    if (table.empty())
    {
        return false;
    }

    STRCAT(sql, "SELECT COUNT(1) FROM ", table);
    std::string where = sql_where(json/"where");
    if (!where.empty())
    {
        STRCAT(sql, " ", where);
    }

    return true;
}

bool sql_delete(const rapidjson::Value& json, std::string& sql)
{
    std::string table = sql_table(json/"table");
    if (table.empty())
    {
        return false;
    }

    std::string where = sql_where(json/"where");
    if (where.empty() && s_config.refuse_delete_without_where)
    {
        // delete must have where
        return false;
    }

    STRCAT(sql, "DELETE FROM ", table);
    if (!where.empty())
    {
        STRCAT(sql, " ", where);
    }

    std::string order = sql_order(json/"order");
    if (!order.empty())
    {
        STRCAT(sql, " ", order);
    }

    std::string limit = sql_limit(json/"limit");
    if (!limit.empty())
    {
        STRCAT(sql, " ", limit);
    }

    return true;
}

} /* jsonkit */ 
