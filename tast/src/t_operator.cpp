#include "tinytast.hpp"
#include "json_operator.h"

DEF_TAST(jvraw, "operator on raw json value")
{
    std::string jsonText = "{\
        \"aaa\": 1, \"bbb\":2,\
        \"ccc\": [3, 4, 5, 6],\
        \"ddd\": {\"eee\":7, \"fff\":8.8}\
    }";

    // std::string jsonText = "{ \"aaa\": 1, \"bbb\":2, \"ccc\": [3, 4, 5, 6], \"ddd\": {\"eee\":7, \"fff\":8.8} }";

    rapidjson::Document doc;
    doc.Parse(jsonText.c_str(), jsonText.size());
    COUT(doc.HasParseError(), false);

    int aaa = doc / "aaa" | 0;
    COUT(aaa, 1);

    //! complie error
    // aaa = doc / "aaa";
    // doc / "aaa" = 1;
    // ok cast non-const to assign int, whitch involve no allocate
    const_cast<rapidjson::Value&>(doc / "aaa") = 11;
    COUT(doc / "aaa" | 0, 11);

    int bbb = 0;
    bbb |= doc / "bbb" ;
    COUT(bbb, 2);
    
    int ccc = 0;
    ccc |= doc / "ccc" / 0;
    COUT(ccc, 3);
    ccc |= doc / "ccc/1";
    COUT(ccc, 4);

    // compile error if only write "auto", must "auto &"
    // const rapidjson::Value&
    auto& cccJson = doc / "ccc";
    ccc |= cccJson / 2;
    COUT(ccc, 5);
    ccc |= cccJson / 3;
    COUT(ccc, 6);

    int eee = doc / "ddd" / "eee" | 0;
    COUT(eee, 7);

    double fff = doc / "ddd" / "fff" | 0.0;
    COUT(fff, 8.8);

/* compile error
    eee = (int)(doc / "ddd" / "eee");
    COUT(eee, 7);
    
    fff = (double)(doc / "ddd" / "fff");
    COUT(eee, 8.8);
*/
}

