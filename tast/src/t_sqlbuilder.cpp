#include "tinytast.hpp"
#include "json_sqlbuilder.h"

DEF_TAST(sql_insert, "build insert sql")
{
    DESC("INSERT ... SET ...");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-1",
        "f_2": "val-2",
        "f_3": 333
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name SET f_1='val-1',f_2='val-2',f_3=333";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("batch INSERT ... (field list) VALUES (value list) ...");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": [
        {
            "f_1": "val-1",
            "f_2": "val-2",
            "f_3": 333
        },
        {
            "f_1": "val-1.2",
            "f_2": "val-2.2",
            "f_3": 444
        }
    ]
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name (f_1,f_2,f_3) VALUES ('val-1','val-2',333), ('val-1.2','val-2.2',444)";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("batch INSERT ... (field list) VALUES (value list) ...");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "field": ["f_1", "f_2", "f_3"],
    "value": [
        ["val-1", "val-2", 333],
        ["val-1.2", "val-2.2", 444]
    ]
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name (f_1,f_2,f_3) VALUES ('val-1','val-2',333), ('val-1.2','val-2.2',444)";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("INSERT ... ON DUPLICATE KEY UPDATE ...");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-1",
        "f_2": "val-2",
        "id": 333
    },
    "update": {
        "f_1": "val-1",
        "f_2": "val-2"
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name SET f_1='val-1',f_2='val-2',id=333 ON DUPLICATE KEY UPDATE f_1='val-1',f_2='val-2'";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

}

DEF_TAST(sql_update, "build update sql")
{
    DESC("UPDATE ... SET ... (refuse by default)");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-11",
        "f_3": 323
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "UPDATE t_name SET f_1='val-11',f_3=323";
        DESC("will generate sql, but return false");
        COUT(jsonkit::sql_update(doc, sql), false);
        // COUT(sql.empty(), true);
        COUT(sql, sqlExpect);
    }

    DESC("UPDATE ... SET ... WHERE ...");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-11",
        "f_3": 323
    },
    "where": {
        "id": 101
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "UPDATE t_name SET f_1='val-11',f_3=323 WHERE 1=1 AND id=101";
        COUT(jsonkit::sql_update(doc, sql), true);
        COUT(sql, sqlExpect);
    }
}

DEF_TAST(sql_select, "build select sql")
{
    DESC("SELECT with no condition");
    {
std::string jsonText = R"json({
    "table": "t_name"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("SELECT ... WHERE ...");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "field": "f_want1,f_want2",
    "where": {
        "id": [100, 200, 300],
        "key": {
            "gt": 10,
            "lt": 20
        },
        "date": {
            "le": "`now()`"
        }
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT f_want1,f_want2 FROM t_name WHERE 1=1 AND id IN (100,200,300) AND key>10 AND key<20 AND date<=now()";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("SELECT ... WHERE ... ORDER BY ... LIMIT ...");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "field": ["f_want1", "f_want2"],
    "where": {
        "id": [100, 200, 300],
        "key": {
            "gt": 10,
            "lt": 20
        },
        "date": {
            "le": "`now()`"
        }
    },
    "limit": 5,
    "order": "id"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT f_want1,f_want2 FROM t_name WHERE 1=1 AND id IN (100,200,300) AND key>10 AND key<20 AND date<=now() ORDER BY id LIMIT 5";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("SELECT ... WHERE ... ORDER BY ... LIMIT offset,count");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "group": "g1",
        "del": {
            "ne": 1
        }
    },
    "limit": [100,20],
    "order": "id"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND group='g1' AND del!=1 ORDER BY id LIMIT 100,20";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("SELECT ... WHERE ... ORDER BY ... LIMIT offset,count");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "group": "g1",
        "del": {
            "ne": 1
        }
    },
    "limit": {
       "offset": 100,
       "count": 20
    },
    "order": "id"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND group='g1' AND del!=1 ORDER BY id LIMIT 100,20";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }
}

DEF_TAST(sql_count, "build count sql")
{
    DESC("COUNT a table");
    {
std::string jsonText = R"json({
    "table": "t_name"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT COUNT(1) FROM t_name";
        COUT(jsonkit::sql_count(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("COUNT ... WHERE ...");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "group": "g1",
        "del": {
            "ne": 1
        }
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT COUNT(1) FROM t_name WHERE 1=1 AND group='g1' AND del!=1";
        COUT(jsonkit::sql_count(doc, sql), true);
        COUT(sql, sqlExpect);
    }
}

DEF_TAST(sql_delete, "build delete sql")
{
    DESC("DELET without where will fail");
    {
std::string jsonText = R"json({
    "table": "t_name"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "DELETE FROM t_name";
        COUT(jsonkit::sql_delete(doc, sql), false);
        COUT(sql, sqlExpect);
        // COUT(sql.empty(), true);
    }

    DESC("DELET without where will fail");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "id": null
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "DELETE FROM t_name WHERE 1=1";
        COUT(jsonkit::sql_delete(doc, sql), false);
        COUT(sql, sqlExpect);
        // COUT(sql.empty(), true);
    }

    DESC("DELETE ... WHERE ...");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "group": "g1",
        "del": {
            "ne": 1
        }
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "DELETE FROM t_name WHERE 1=1 AND group='g1' AND del!=1";
        COUT(jsonkit::sql_delete(doc, sql), true);
        COUT(sql, sqlExpect);
    }
}

DEF_TAST(sql_injection, "detect sql injection")
{
    DESC("sql generation may stop in midway when found invalid input");
    DESC("invalid table name");
    {
        std::string jsonText = R"json({
    "table": "it'sname"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT COUNT(1) FROM it";
        COUT(jsonkit::sql_count(doc, sql), false);
        COUT(sql, sqlExpect);
        // COUT(sql.empty(), true);
    }

    DESC("invalid insert field name");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1;delete*": "val-1",
        "f_2": "val-2",
        "f_3": 333
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name SET f_1";
        COUT(jsonkit::sql_insert(doc, sql), false);
        COUT(sql, sqlExpect);
        // COUT(sql.empty(), true);
    }

    DESC("escape set value");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-1;",
        "f_2": "val'2",
        "f_3": 333
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name SET f_1='val-1;',f_2='val''2',f_3=333";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("invalid select field name");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "field": "f_want1;delete * from t_name;"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        COUT(jsonkit::sql_select(doc, sql), false);
        std::string sqlExpect = "SELECT f_want1";
        COUT(sql, sqlExpect);
        // COUT(sql.empty(), true);
    }

    DESC("escape where value");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "id": "xxx;delete * from t_name"
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND id='xxx;delete * from t_name'";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("invalid where field name");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "field": "f_want1,f_want2",
    "where": {
        "or 1=1; delete * from t_name": ";"
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT f_want1,f_want2 FROM t_name WHERE 1=1";
        COUT(jsonkit::sql_select(doc, sql), false);
        COUT(sql, sqlExpect);
    }

    DESC("invalid limit argument");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "id": "xxx"
    },
    "limit": "1;delete * from t_name"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND id='xxx' LIMIT ";
        COUT(jsonkit::sql_select(doc, sql), false);
        COUT(sql, sqlExpect);
    }
}

DEF_TAST(sql_null, "treat null value specially")
{
    DESC("skip null in SET... statement");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-1",
        "f_2": null,
        "f_3": 333
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name SET f_1='val-1',f_3=333";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("use string `null` to really ment to set null value");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-1",
        "f_2": "`null`",
        "f_3": 333
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name SET f_1='val-1',f_2=null,f_3=333";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("skill field with null value in the first row");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": [
        {
            "f_1": "val-1",
            "f_2": null,
            "f_3": 333
        },
        {
            "f_1": "val-1.2",
            "f_2": "val-2.2",
            "f_3": 444
        }
    ]
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name (f_1,f_3) VALUES ('val-1',333), ('val-1.2',444)";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("second row can accept null value directlly");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": [
        {
            "f_1": "val-1",
            "f_2": "val-2",
            "f_3": 333
        },
        {
            "f_1": "val-1.2",
            "f_3": null
        }
    ]
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "INSERT INTO t_name (f_1,f_2,f_3) VALUES ('val-1','val-2',333), ('val-1.2',null,null)";
        COUT(jsonkit::sql_insert(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("skill null value in WHERE ...");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "type": "xxx",
        "id": [100, 200, 300],
        "key": {
            "gt": 10,
            "lt": 20
        },
        "date": null
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND type='xxx' AND id IN (100,200,300) AND key>10 AND key<20";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("specific value is null in WHERE ...");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "type": "xxx",
        "id": [100, 200, 300],
        "key": {
            "gt": 10,
            "lt": 20
        },
        "date": {
            "null": true
        }
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND type='xxx' AND id IN (100,200,300) AND key>10 AND key<20 AND date is NULL";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("specific value is not null in WHERE ...");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "type": "xxx",
        "id": [100, 200, 300],
        "key": {
            "gt": 10,
            "lt": 20
        },
        "date": {
            "null": false
        }
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND type='xxx' AND id IN (100,200,300) AND key>10 AND key<20 AND date is not NULL";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }
}

DEF_TAST(sql_group, "select with group")
{
    DESC("SELECT ... WHERE ... GROUP BY ... ");
    {
std::string jsonText = R"json({
    "table": "t_name LEFT JOIN t_score",
    "field": "f_id, min(f_a) as min, max(f_b) as max",
    "group": "f_id",
    "having": {
        "id": [100, 200, 300]
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT f_id, min(f_a) as min, max(f_b) as max FROM t_name LEFT JOIN t_score GROUP BY f_id HAVING 1=1 AND id IN (100,200,300)";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }
}

DEF_TAST(sql_config, "change sql config")
{
    DESC("like default add '%%' postfix");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "name": {
           "like": "ab_c"
        }
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND name like 'ab_c%'";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("change config");
    jsonkit::sql_config_t cfg = jsonkit::set_sql_config(nullptr);
    cfg.fix_like_value = jsonkit::SQL_LIKE_POSTFIX | jsonkit::SQL_LIKE_PREFIX;
    cfg.escape_like_metachar = true;
    cfg.refuse_delete_without_where = false;
    cfg.refuse_update_without_where = false;
    // set and save the config
    cfg = set_sql_config(&cfg);

    DESC("like add '%%' in both end");
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "name": {
           "like": "a%b_c"
        }
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND name like '%a\\%b\\_c%'";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("DELET without WHERE will not fail");
    {
std::string jsonText = R"json({
    "table": "t_name"
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "DELETE FROM t_name";
        COUT(jsonkit::sql_delete(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("UPDATE ... SET ... will not fail");
    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-11",
        "f_3": 323
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "UPDATE t_name SET f_1='val-11',f_3=323";
        COUT(jsonkit::sql_update(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    // restore the config
    set_sql_config(&cfg);
}

DEF_TAST(sql_cbuilder, "tast use CSqlBuilder")
{
    DESC("create a CSqulBuilder object");
    jsonkit::CSqlBuilder sb;

    {
    std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "f1": { "le":99, "ge":10 },
        "f2": { "between": [10,99] }
    }
})json";

    COUT(jsonText);
    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    std::string sql;
    std::string sqlExpect = "SELECT COUNT(1) FROM t_name WHERE 1=1 AND f1<=99 AND f1>=10 AND f2 BETWEEN 10 AND 99";
    COUT(sb.Count(doc, sql), true);
    COUT(sql, sqlExpect);
    }

    {
        std::string jsonText = R"json({
    "table": "t_name",
    "value": {
        "f_1": "val-1",
        "f_2": "val-2",
        "f_3": 333
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "REPLACE INTO t_name SET f_1='val-1',f_2='val-2',f_3=333";
        COUT(sb.Replace(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("change config in CSqulBuilder object");
    sb.Config().fix_like_value |= jsonkit::SQL_LIKE_PREFIX;
    {
std::string jsonText = R"json({
    "table": "t_name",
    "where": {
        "name": {
           "like": "abc"
        }
    }
})json";

        COUT(jsonText);
        rapidjson::Document doc;
        doc.Parse(jsonText.c_str(), jsonText.size());
        COUT(doc.HasParseError(), false);

        std::string sql;
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND name like '%abc%'";
        COUT(sb.Select(doc, sql), true);
        COUT(sql, sqlExpect);
    }
}

DEF_TAST(sql_append, "tast generate more sql to a string buffer")
{
    jsonkit::CSqlBuilder sb;

    std::string jsonText = R"json({
    "table": "t_name",
    "field": "f1,f2,f3",
    "where": {
        "id": 1001
    }
})json";

    COUT(jsonText);
    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    std::string sql;
    std::string sqlExpect = "SELECT COUNT(1) FROM t_name WHERE 1=1 AND id=1001";
    COUT(sb.Count(doc, sql), true);
    COUT(sql, sqlExpect);

    sql.append(";");
    sqlExpect = "SELECT COUNT(1) FROM t_name WHERE 1=1 AND id=1001;SELECT f1,f2,f3 FROM t_name WHERE 1=1 AND id=1001";
    COUT(sb.Select(doc, sql), true);
    COUT(sql, sqlExpect);
}

DEF_TAST(sql_compare, "tast complex compare in where")
{
    std::string jsonText = R"json({
    "table": "t_name",
    "field": "f1,f2,f3",
    "where": {
        "id": 1001,
        "f1": { "ne": 0, "not in": [3,5,7]},
        "f2": { "null": false},
        "f3": { "like": "xx"},
        "f4": { "between": ["aa","zz"]},
        "f5": { "gt": 100, "lt": 200},
        "f6": { "ge": "AA", "le": "ZZ"}
    }
})json";

    COUT(jsonText);
    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    std::string sql;
    std::string sqlExpect = "SELECT f1,f2,f3 FROM t_name WHERE 1=1 AND id=1001 AND f1!=0 AND f1 not IN (3,5,7) AND f2 is not NULL AND f3 like 'xx%' AND f4 BETWEEN 'aa' AND 'zz' AND f5>100 AND f5<200 AND f6>='AA' AND f6<='ZZ'";
    COUT(jsonkit::sql_select(doc, sql), true);
    COUT(sql, sqlExpect);
}

DEF_TAST(sql_subquery_where, "tast sub query in where")
{
    std::string jsonText = R"json({
    "table": "t_name",
    "field": "f_want1,f_want2",
    "where": {
        "id": [100, 200, 300],
        "key": {
            "in": {
                "table": "t_subtable",
                "field": "key",
                "where": {
                    "cond": 1
                }
            }
        },
        "date": {
            "le": "`now()`"
        }
    }
})json";

    COUT(jsonText);
    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    std::string sql;
    std::string sqlExpect = "SELECT f_want1,f_want2 FROM t_name WHERE 1=1 AND id IN (100,200,300) AND key in (SELECT key FROM t_subtable WHERE 1=1 AND cond=1) AND date<=now()";
    COUT(jsonkit::sql_select(doc, sql), true);
    COUT(sql, sqlExpect);
}

DEF_TAST(sql_multable_1, "tast multiply tables")
{
    std::string jsonText = R"json({
    "table": ["t_1", 
        { "join": "LEFT JOIN", "table": "t_2", "as": "A", "use": "f_key" }
    ],
    "where": { "A.id": 100 }
})json";

    COUT(jsonText);
    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    std::string sql;
    std::string sqlExpect = "SELECT * FROM t_1 LEFT JOIN t_2 AS A USING(f_key) WHERE 1=1 AND A.id=100";
    COUT(jsonkit::sql_select(doc, sql), true);
    COUT(sql, sqlExpect);
}

DEF_TAST(sql_multable_2, "tast sub query tmp tables")
{
    std::string jsonText = R"json({
    "table": ["t_1", 
        { "join": "LEFT JOIN", "table": {
            "table": "t_2",
            "field": ["id", "name", "key"]
        }, "as": "A", "on": "t_1.key=A.key" }
    ],
    "where": { "A.id": 100 }
})json";

    COUT(jsonText);
    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    std::string sql;
    std::string sqlExpect = "SELECT * FROM t_1 LEFT JOIN (SELECT id,name,key FROM t_2) AS A ON t_1.key=A.key WHERE 1=1 AND A.id=100";
    COUT(jsonkit::sql_select(doc, sql), true);
    COUT(sql, sqlExpect);
}

DEF_TAST(sql_whereor_1, "tast or relation in where")
{
    std::string jsonText = R"json({
    "table": "t_name",
    "field": "*",
    "where": {
        "-or": {
            "user": 1001,
            "id": [100, 200, 300],
            "key": {
                "gt": 10,
                "lt": 20
            }
        },
        "date": {
            "le": "`now()`"
        }
    }
})json";

    COUT(jsonText);
    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    std::string sql;
    std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND (1!=1 OR user=1001 OR id IN (100,200,300) OR key>10 OR key<20) AND date<=now()";
    COUT(jsonkit::sql_select(doc, sql), true);
    COUT(sql, sqlExpect);
}
