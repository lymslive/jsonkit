#include "json_sqlbuilder.h"
#include "json_operator.h"
#include "jsonkit_rpdjn.h"

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

inline
sql_config_t* get_default_config()
{
    return &s_config;
}

const char SINGLE_QUOTE = '\'';
const char BACK_QUOTE = '`';
const char STATE_END = ';';

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

/* ************************************************************ */
// Section:

class CSqlBuildBuffer
{
public:
    CSqlBuildBuffer(std::string& buffer, sql_config_t* pConfig = nullptr)
        : m_buffer(buffer), m_pConfig(pConfig)
    {
        if (!m_pConfig)
        {
            m_pConfig = get_default_config();
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
    if (psz == nullptr || count == 0 || *psz == '\0')
    {
        return false;
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
    Append(" SET ");
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
    if (!json)
    {
        return true;
    }

    Append(" WHERE 1=1");
    return DoPushWhere(json);
}

bool CSqlBuildBuffer::PushHaving(const rapidjson::Value& json)
{
    if (!json)
    {
        return true;
    }

    Append(" HAVING 1=1");
    return DoPushWhere(json);
}

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
            Append( " AND ").Append(field).Append(" like ");
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

        if (0 == strcmp(op, "null"))
        {
            if (it->value.IsBool() && it->value.GetBool())
            {
                Append( " AND ").Append(field).Append(" is NULL");
            }
            else
            {
                Append( " AND ").Append(field).Append(" is NULL");
            }
            continue;
        }

        if (0 == strcmp(op, "eq"))
        {
            Append( " AND ").Append(field).Append('=');
        }
        else if (0 == strcmp(op, "gt"))
        {
            Append( " AND ").Append(field).Append('>');
        }
        else if (0 == strcmp(op, "lt"))
        {
            Append( " AND ").Append(field).Append(',');
        }
        else if (0 == strcmp(op, "ne"))
        {
            Append( " AND ").Append(field).Append("!=");
        }
        else if (0 == strcmp(op, "ge"))
        {
            Append( " AND ").Append(field).Append(">=");
        }
        else if (0 == strcmp(op, "le"))
        {
            Append( " AND ").Append(field).Append("<=");
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
    if (!json)
    {
        return true;
    }
    Append(" ORDER BY ");
    return PutWord(json);
}

bool CSqlBuildBuffer::PushGroup(const rapidjson::Value& json)
{
    if (!json)
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
    if (!json)
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
    auto& where = json/"where";
    SQL_ASSERT(PushWhere(where));
    if (m_pConfig->refuse_update_without_where && Size() <= last + 10)
    {
        // only Append(" WHERE 1=1");
        return false;
    }

    SQL_ASSERT(PushOrder(json/"order"));
    SQL_ASSERT(PushLimit(json/"limit"));

    return true;
}

/* ************************************************************ */
// Section:

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

/* ************************************************************ */
// Section:

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

/* ************************************************************ */
// Section:

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

/** Generate: (field1, feild2, ...) VALUES (value1, value2, ...), ...
 * @param json: array of array which has the same fields order
 * @param head: array of string specify the field names
 * */
std::string sql_values(const rapidjson::Value& json, const rapidjson::Value& head)
{
    std::vector<std::string> field;
    for (auto it = head.Begin(); it != head.End(); ++it)
    {
        std::string fname = (*it) | "";
        if (fname.empty() || !sql_check_word(fname))
        {
            return "";
        }
        field.push_back(fname);
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

    std::string value;
    for (auto it = json.Begin(); it != json.End(); ++it)
    {
        if (!it->IsArray())
        {
            continue;
        }
        if (it->Size() != field.size())
        {
            return "";
        }

        std::string row;
        for (auto jt = it->Begin(); jt != it->End(); ++jt)
        {
            STRCAT(row, sqlfy_value(*jt), ",");
        }
        if (!row.empty())
        {
            row.pop_back();
        }

        if (!value.empty())
        {
            STRCAT(value, ", ");
        }
        STRCAT(value, "(", row, ")");
    }

    if (value.empty())
    {
        return "";
    }

    std::string sql;
    STRCAT(sql, "(", fieldstr, ")", " VALUES ", value);

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
