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
}

DEF_TAST(sql_update, "build update sql")
{
    DESC("UPDATE ... SET ...");
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

    DESC("SELECT ... WHERE ... LIMIT ... ORDER BY ...");
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
        std::string sqlExpect = "SELECT f_want1,f_want2 FROM t_name WHERE 1=1 AND id IN (100,200,300) AND key>10 AND key<20 AND date<=now() LIMIT 5 ORDER BY id";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("SELECT ... WHERE ... LIMIT offset,count ORDER BY ...");
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
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND group='g1' AND del!=1 LIMIT 100,20 ORDER BY id";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }

    DESC("SELECT ... WHERE ... LIMIT offset,count ORDER BY ...");
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
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND group='g1' AND del!=1 LIMIT 100,20 ORDER BY id";
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
        COUT(sql.empty(), true);
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

DEF_TAST(sql_invalid, "detect sql injection")
{
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
        COUT(jsonkit::sql_count(doc, sql), false);
        COUT(sql.empty(), true);
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
        COUT(jsonkit::sql_insert(doc, sql), false);
        COUT(sql.empty(), true);
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
        COUT(sql.empty(), true);
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
        std::string sqlExpect = "SELECT f_want1,f_want2 FROM t_name";
        COUT(jsonkit::sql_select(doc, sql), true);
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
        std::string sqlExpect = "SELECT * FROM t_name WHERE 1=1 AND id='xxx'";
        COUT(jsonkit::sql_select(doc, sql), true);
        COUT(sql, sqlExpect);
    }
}
