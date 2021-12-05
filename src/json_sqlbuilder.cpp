#include "json_sqlbuilder.h"
#include "json_operator.h"
#include "jsonkit_internal.h"

#define SQL_ASSERT(expr) do { \
    if (!expr) { \
        LOGF("build sql failed: %s", #expr); \
        return false; \
    } \
} while(0)

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
const char BACK_QUOTE = '`';
const char STATE_END = ';';

/* ************************************************************ */
// Section:

/** A simple sql builder buffer implementation.
 * @details It is algorithm more than class, it's member is just reference or
 * pointer to user provided std::string and config, but not owner then. It only
 * append to that string buffer, try to avoid string copy as much as possible.
 * */
class CSqlBuildBuffer
{
public:
    CSqlBuildBuffer(std::string& buffer, sql_config_t* pConfig = nullptr)
        : m_buffer(buffer), m_pConfig(pConfig)
    {
        if (!m_pConfig)
        {
            m_pConfig = &s_config;
        }
    }

    typedef CSqlBuildBuffer SelfType;
    CSqlBuildBuffer(const SelfType& that) =  delete;
    SelfType& operator=(const SelfType& that) = delete;

    size_t Size() { return m_buffer.size(); }
    const char* c_str() { return m_buffer.c_str(); }
    const std::string& Buffer() { return m_buffer; }

public:
    SelfType& Append(char ch)
    {
        m_buffer.push_back(ch);
        return *this;
    }
    SelfType& Append(const char* psz)
    {
        m_buffer.append(psz);
        return *this;
    }
    SelfType& Append(const char* psz, size_t count)
    {
        m_buffer.append(psz, count);
        return *this;
    }
    SelfType& Append(const std::string& str)
    {
        m_buffer.append(str);
        return *this;
    }
    SelfType& Append(const SelfType& that)
    {
        m_buffer.append(that.m_buffer);
        return *this;
    }

    void PopEnd(char ch)
    {
        if (!m_buffer.empty() && m_buffer.back() == ch)
        {
            m_buffer.pop_back();
        }
    }

    bool PutWord(const char* psz, size_t count);
    bool PutWord(const rapidjson::Value& json);
    bool PutWord(const rapidjson::Value& json, std::string& last);
    bool PutEscape(const char* psz, size_t count);
    bool PutEscape(const rapidjson::Value& json);
    bool PutValue(const char* psz, size_t count);
    bool PutValue(const rapidjson::Value& json);

    bool PushTable(const rapidjson::Value& json);
    bool PushField(const rapidjson::Value& json);

    bool PushSetValue(const rapidjson::Value& json);
    bool PushSetValue(const rapidjson::Value& json, const rapidjson::Value& head);
    bool DoSetValue(const rapidjson::Value& json);
    bool DoBatchValue(const rapidjson::Value& json);

    bool DoPushWhere(const rapidjson::Value& json);
    bool DoCmpWhere(const rapidjson::Value& json, const std::string& field);
    bool PushWhere(const rapidjson::Value& json);
    bool PushHaving(const rapidjson::Value& json);
    bool PushGroup(const rapidjson::Value& json);
    bool PushOrder(const rapidjson::Value& json);
    bool PushLimit(const rapidjson::Value& json);

protected:
    bool DoInsert(const rapidjson::Value& json);

public: // user interface for typical sql statements

    bool Insert(const rapidjson::Value& json)
    {
        Append("INSERT INTO ");
        return DoInsert(json);
    }
    bool Replace(const rapidjson::Value& json)
    {
        Append("REPLACE INTO ");
        return DoInsert(json);
    }

    bool Update(const rapidjson::Value& json);
    bool Select(const rapidjson::Value& json);
    bool Count(const rapidjson::Value& json);
    bool Delete(const rapidjson::Value& json);

protected:
    sql_config_t* m_pConfig;
    std::string& m_buffer;
};

bool CSqlBuildBuffer::PutWord(const char* psz, size_t count)
{
    if (psz == nullptr || count == 0 || *psz == '\0')
    {
        return false;
    }

    for (size_t i = 0; i < count; ++i)
    {
        char ch = psz[i];
        if (ch == SINGLE_QUOTE || ch == BACK_QUOTE || ch == STATE_END)
        {
            return false;
        }
        Append(ch);
    }

    return true;
}

bool CSqlBuildBuffer::PutEscape(const char* psz, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        char ch = psz[i];
        // escape single quote: ' --> ''
        if (ch == SINGLE_QUOTE)
        {
            Append(ch);
        }
        Append(ch);
    }
    return true;
}

bool CSqlBuildBuffer::PutValue(const char* psz, size_t count)
{
    if (psz == nullptr)
    {
        return false;
    }
    else if (count == 0 || *psz == '\0')
    {
        Append(SINGLE_QUOTE).Append(SINGLE_QUOTE);
        return true;
    }

    // keep origin string in `` quote, eg. `now()` `null`
    if (psz[0] == BACK_QUOTE)
    {
        if (count > 2 && psz[count-1] == BACK_QUOTE)
        {
            return PutWord(psz+1, count-2);
        }
        else
        {
            return false;
        }
    }

    Append(SINGLE_QUOTE);
    PutEscape(psz, count);
    Append(SINGLE_QUOTE);

    return true;
}

bool CSqlBuildBuffer::PutEscape(const rapidjson::Value& json)
{
    if (json.IsString())
    {
        return PutEscape(json.GetString(), json.GetStringLength());
    }
    return false;
}

// a single string word or list of word separated by ,
bool CSqlBuildBuffer::PutWord(const rapidjson::Value& json)
{
    if (json.IsString())
    {
        return PutWord(json.GetString(), json.GetStringLength());
    }
    else if (json.IsArray() && !json.Empty())
    {
        for (auto it = json.Begin(); it != json.End(); ++it)
        {
            if (!it->IsString())
            {
                return false;
            }
            SQL_ASSERT(PutWord(*it));
            Append(',');
        }
        PopEnd(',');
        return true;
    }
    return false;
}

// put a word in buffer, and save a copy in string last
bool CSqlBuildBuffer::PutWord(const rapidjson::Value& json, std::string& last)
{
    size_t tail = m_buffer.size();
    SQL_ASSERT(PutWord(json));
    last = m_buffer.substr(tail);
    return true;
}

bool CSqlBuildBuffer::PutValue(const rapidjson::Value& json)
{
    if (json.IsString())
    {
        return PutValue(json.GetString(), json.GetStringLength());
    }
    else if (json.IsArray())
    {
        Append('(');
        for (auto it = json.Begin(); it != json.End(); ++it)
        {
            SQL_ASSERT(PutValue(*it));
            Append(',');
        }
        PopEnd(',');
        Append(')');
        return true;
    }
    else if (json.IsObject() || json.IsArray())
    {
        return false;
    }
    else if (json.IsNull())
    {
        Append("null");
    }
    else if (json.IsBool())
    {
        if (json.GetBool())
        {
            Append('1');
        }
        else
        {
            Append('0');
        }
    }
    else
    {
        // numeric value
        Append(stringfy(json));
    }

    return true;
}

bool CSqlBuildBuffer::PushTable(const rapidjson::Value& json)
{
    if (json.IsString())
    {
        return PutWord(json.GetString(), json.GetStringLength());
    }
    return false;
}

bool CSqlBuildBuffer::PushField(const rapidjson::Value& json)
{
    if (!json || json.IsNull())
    {
        Append('*');
        return true;
    }
    if (json.IsString())
    {
        return PutWord(json.GetString(), json.GetStringLength());
    }
    else if (json.IsArray())
    {
        return PutWord(json);
    }
    return false;
}

bool CSqlBuildBuffer::PushSetValue(const rapidjson::Value& json)
{
    if (json.IsObject() && json.MemberCount() > 0)
    {
        Append(" SET ");
        return DoSetValue(json);
    }
    else if (json.IsArray() && json.Size() > 0)
    {
        return DoBatchValue(json);
    }
    return false;
}

/** Generate: SET field1=value1, field2=value2, ... */
bool CSqlBuildBuffer::DoSetValue(const rapidjson::Value& json)
{
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
    {
        // skip json null, if really mean to set null use string "`null`"
        if (it->value.IsNull())
        {
            continue;
        }

        SQL_ASSERT(PutWord(it->name));
        Append('=');
        SQL_ASSERT(PutValue(it->value));
        Append(',');
    }
    PopEnd(',');
    return true;
}

/** Generate: (field1, feild2, ...) VALUES (value1, value2, ...), ...
 * @param json: array of object which has the same fields set
 * */
bool CSqlBuildBuffer::DoBatchValue(const rapidjson::Value& json)
{
    auto& first = json[0];
    if (!first.IsObject() || first.ObjectEmpty())
    {
        return false;
    }

    Append(' ');
    std::vector<std::string> field;
    std::string value; // store first row value with field
    CSqlBuildBuffer another(value, m_pConfig);
    Append('(');
    another.Append('(');
    for (auto it = first.MemberBegin(); it != first.MemberEnd(); ++it)
    {
        if (it->value.IsNull())
        {
            continue;
        }

        std::string last;
        SQL_ASSERT(PutWord(it->name, last));
        Append(',');
        field.push_back(last);

        SQL_ASSERT(another.PutValue(it->value));
        another.Append(',');
    }
    PopEnd(',');
    Append(')');
    another.PopEnd(',');
    another.Append(')');

    Append(" VALUES ");
    Append(value);

    uint32_t size = json.Size();
    for (uint32_t i = 1; i < size; ++i)
    {
        Append(", (");
        auto& row = json[i];
        for (auto& key : field)
        {
            auto& val = row/key;
            if (!val)
            {
                Append("null");
            }
            else
            {
                SQL_ASSERT(PutValue(val));
            }
            Append(',');
        }
        PopEnd(',');
        Append(')');
    }

    return true;
}

/** Generate: (field1, feild2, ...) VALUES (value1, value2, ...), ...
 * @param json: array of array which has the same fields order
 * @param head: array of string specify the field names
 * */
bool CSqlBuildBuffer::PushSetValue(const rapidjson::Value& json, const rapidjson::Value& head)
{
    if (!json.IsArray() || json.Empty())
    {
        return false;
    }
    if (!head.IsArray() || head.Empty())
    {
        return false;
    }

    Append(' ');
    Append('(');
    SQL_ASSERT(PutWord(head));
    Append(')');

    Append(" VALUES ");
    for (auto it = json.Begin(); it != json.End(); ++it)
    {
        if (!it->IsArray())
        {
            continue;
        }
        if (it->Size() != head.Size())
        {
            return false;
        }
        if (it != json.Begin())
        {
            Append(", ");
        }
        SQL_ASSERT(PutValue(*it));
    }

    return true;
}

bool CSqlBuildBuffer::PushWhere(const rapidjson::Value& json)
{
    if (!json || json.IsNull())
    {
        return true;
    }

    Append(" WHERE 1=1");
    return DoPushWhere(json);
}

bool CSqlBuildBuffer::PushHaving(const rapidjson::Value& json)
{
    if (!json || json.IsNull())
    {
        return true;
    }

    Append(" HAVING 1=1");
    return DoPushWhere(json);
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
bool CSqlBuildBuffer::DoPushWhere(const rapidjson::Value& json)
{
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
    {
        if (it->value.IsNull())
        {
            continue;
        }

        std::string field;
        CSqlBuildBuffer another(field, m_pConfig);
        SQL_ASSERT(another.PutWord(it->name));

        if (it->value.IsArray())
        {
            Append( " AND ").Append(field).Append(" IN ");
            SQL_ASSERT(PutValue(it->value));
        }
        else if (it->value.IsObject())
        {
            SQL_ASSERT(DoCmpWhere(it->value, field));
        }
        else
        {
            Append( " AND ").Append(field).Append('=');
            SQL_ASSERT(PutValue(it->value));
        }
    }
    return true;
}

/** Generate multiple comparision
 * @code
 * "field": {
 *   "eq": ..., "ne": ..., "gt": ..., "lt": ..., "ge": ..., "le": ...,
 *   "between": [min, max]
 *   "like": ..., "not in": [...], 
 *   "null": true/false
 * }
 * @endcode
 * */
bool CSqlBuildBuffer::DoCmpWhere(const rapidjson::Value& json, const std::string& field)
{
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
    {
        if (it->value.IsNull())
        {
            continue;
        }

        const char* op = it->name.GetString();
        if (0 == strcmp(op, "like"))
        {
            Append(" AND ").Append(field).Append(" like ");
            Append(SINGLE_QUOTE);
            if (m_pConfig->fix_like_value & SQL_LIKE_PREFIX)
            {
                Append('%');
            }
            SQL_ASSERT(PutEscape(it->value));
            if (m_pConfig->fix_like_value & SQL_LIKE_POSTFIX)
            {
                Append('%');
            }
            Append(SINGLE_QUOTE);
            continue;
        }

        if (0 == strcmp(op, "between"))
        {
            if (it->value.IsArray() && it->value.Size() == 2)
            {
                Append(" AND ").Append(field).Append(" BETWEEN ");
                SQL_ASSERT(PutValue(it->value[0]));
                Append(" AND ");
                SQL_ASSERT(PutValue(it->value[1]));
            }
            continue;
        }

        if (0 == strcmp(op, "null"))
        {
            if (it->value.IsBool() && it->value.GetBool())
            {
                Append(" AND ").Append(field).Append(" is NULL");
            }
            else
            {
                Append(" AND ").Append(field).Append(" is not NULL");
            }
            continue;
        }

        if (0 == strcmp(op, "eq"))
        {
            Append(" AND ").Append(field).Append('=');
        }
        else if (0 == strcmp(op, "gt"))
        {
            Append(" AND ").Append(field).Append('>');
        }
        else if (0 == strcmp(op, "lt"))
        {
            Append(" AND ").Append(field).Append('<');
        }
        else if (0 == strcmp(op, "ne"))
        {
            Append(" AND ").Append(field).Append("!=");
        }
        else if (0 == strcmp(op, "ge"))
        {
            Append(" AND ").Append(field).Append(">=");
        }
        else if (0 == strcmp(op, "le"))
        {
            Append(" AND ").Append(field).Append("<=");
        }
        else if (0 == strcmp(op, "not in") && it->value.IsArray())
        {
            Append(" AND ").Append(field).Append(" not IN ");
        }
        else
        {
            continue;
        }

        PutValue(it->value);
    }
    return true;
}

bool CSqlBuildBuffer::PushOrder(const rapidjson::Value& json)
{
    if (!json || json.IsNull())
    {
        return true;
    }
    Append(" ORDER BY ");
    return PutWord(json);
}

bool CSqlBuildBuffer::PushGroup(const rapidjson::Value& json)
{
    if (!json || json.IsNull())
    {
        return true;
    }
    Append(" GROUP BY ");
    return PutWord(json);
}

/** limit clause.
 * support data:
 *   "limit": COUNT,
 *   "limit": [OFFSET, COUNT],
 *   "limit": {"offset": OFFSET, "count": COUNT}
 * in which COUNT and OFFSET should be number to generate valid sql
 * */
bool CSqlBuildBuffer::PushLimit(const rapidjson::Value& json)
{
    if (!json || json.IsNull())
    {
        return true;
    }
    Append(" LIMIT ");
    if (json.IsUint() || json.IsUint64())
    {
        return PutValue(json);
    }
    else if (json.IsArray() && json.Size() == 2)
    {
        auto& offset = json[0];
        auto& count = json[1];
        if (offset.IsUint() || offset.IsUint64())
        {
            PutValue(offset);
            Append(',');
        }
        return PutValue(count);
    }
    else if (json.IsObject())
    {
        auto& offset = json/"offset";
        auto& count = json/"count";
        if (offset.IsUint() || offset.IsUint64())
        {
            PutValue(offset);
            Append(',');
        }
        return PutValue(count);
    }
    return false;
}

/* ------------------------------------------------------------ */
// Section:

bool CSqlBuildBuffer::DoInsert(const rapidjson::Value& json)
{
    auto& table = json/"table";
    auto& value = json/"value";
    if (!table || !value)
    {
        return false;
    }

    SQL_ASSERT(PushTable(table));

    auto& head = json/"head";
    if (!!head)
    {
        SQL_ASSERT(PushSetValue(value, head));
    }
    else
    {
        SQL_ASSERT(PushSetValue(value));
    }

    auto& update = json/"update";
    if (!!update && update.IsObject())
    {
        Append(" ON DUPLICATE KEY UPDATE ");
        SQL_ASSERT(DoSetValue(update));
    }

    // need to select last auto increment id after insert
    if (json/"last_insert_id" | false)
    {
        Append("; SELECT last_insert_id()");
    }

    return true;
}

bool CSqlBuildBuffer::Update(const rapidjson::Value& json)
{
    auto& table = json/"table";
    auto& value = json/"value";
    if (!table || !value)
    {
        return false;
    }

    Append("UPDATE ");
    SQL_ASSERT(PushTable(table));
    SQL_ASSERT(PushSetValue(value));

    size_t last = Size();
    SQL_ASSERT(PushWhere(json/"where"));
    if (m_pConfig->refuse_update_without_where && Size() < last + sizeof(" WHERE 1=1"))
    {
        return false;
    }

    SQL_ASSERT(PushOrder(json/"order"));
    SQL_ASSERT(PushLimit(json/"limit"));
    return true;
}

bool CSqlBuildBuffer::Select(const rapidjson::Value& json)
{
    auto& table = json/"table";
    if (!table)
    {
        return false;
    }

    Append("SELECT ");
    SQL_ASSERT(PushField(json/"field"));
    Append(" FROM ");
    SQL_ASSERT(PushTable(table));

    SQL_ASSERT(PushWhere(json/"where"));

    auto& group = json/"group";
    if (!!group)
    {
        SQL_ASSERT(PushGroup(group));
        SQL_ASSERT(PushHaving(json/"having"));
    }

    SQL_ASSERT(PushOrder(json/"order"));
    SQL_ASSERT(PushLimit(json/"limit"));
    return true;
}

bool CSqlBuildBuffer::Count(const rapidjson::Value& json)
{
    auto& table = json/"table";
    if (!table)
    {
        return false;
    }

    Append("SELECT COUNT(1) FROM ");
    SQL_ASSERT(PushTable(table));
    SQL_ASSERT(PushWhere(json/"where"));

    return true;
}

bool CSqlBuildBuffer::Delete(const rapidjson::Value& json)
{
    auto& table = json/"table";
    if (!table)
    {
        return false;
    }

    Append("DELETE FROM ");
    SQL_ASSERT(PushTable(table));

    size_t last = Size();
    SQL_ASSERT(PushWhere(json/"where"));
    if (m_pConfig->refuse_delete_without_where && Size() <= last + sizeof(" WHERE 1=1"))
    {
        return false;
    }

    SQL_ASSERT(PushOrder(json/"order"));
    SQL_ASSERT(PushLimit(json/"limit"));
    return true;
}

/* ************************************************************ */
// Section: public class interface

bool CSqlBuilder::Insert(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql, &m_config);
    return obj.Insert(json);
}

bool CSqlBuilder::Replace(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql, &m_config);
    return obj.Replace(json);
}

bool CSqlBuilder::Update(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql, &m_config);
    return obj.Update(json);
}

bool CSqlBuilder::Select(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql, &m_config);
    return obj.Select(json);
}

bool CSqlBuilder::Count(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql, &m_config);
    return obj.Count(json);
}

bool CSqlBuilder::Delete(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql, &m_config);
    return obj.Delete(json);
}

/* ************************************************************ */
// Section: public function interface

bool sql_insert(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql);
    return obj.Insert(json);
}

bool sql_replace(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql);
    return obj.Replace(json);
}

bool sql_update(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql);
    return obj.Update(json);
}

bool sql_select(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql);
    return obj.Select(json);
}

bool sql_count(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql);
    return obj.Count(json);
}

bool sql_delete(const rapidjson::Value& json, std::string& sql)
{
    CSqlBuildBuffer obj(sql);
    return obj.Delete(json);
}

/* ************************************************************ */

} /* jsonkit */ 
