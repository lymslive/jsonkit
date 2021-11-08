/** 
 * @file json_compare.cpp
 * @author lymslive
 * @date 2021-10-27
 * @brief implementation for json compare
 * */
#include "json_compare.h"
#include "jsonkit_internal.h"

#include <cmath>
#include <limits>

#if __cplusplus < 201103L
#include <sstream>
namespace std
{
template <typename valueT>
string to_string(const valueT& val)
{
	ostringstream sout;
	sout << val;
	return sout.str();
}
}
#endif

namespace jsonkit
{
namespace impl
{
// BEG namespace jsonkit::impl

// refer to: https://blog.csdn.net/davidhopper/article/details/79609064
// Test whether two float or double numbers are equal.
// ulp: units in the last place.
template <typename T>
// typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
bool
IsAlmostEqual(T x, T y, int ulp = 2)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(x - y) < std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
           // unless the result is subnormal
           || std::fabs(x - y) < std::numeric_limits<T>::min();
}

// auto print compare information if failed
class CCompareMessage
{
public:
    CCompareMessage(const std::string& apath, const std::string& bpath) :
        m_aPath(apath), m_bPath(bpath), m_pass(true)
    {}

    ~CCompareMessage()
    {
        if (m_pass)
        {
            return;
        }
        LOGF("Compare Json Failed at: a #%s = %s; b #%s = %s", m_aPath.c_str(), m_aValue.c_str(), m_bPath.c_str(), m_bValue.c_str());
        // fprintf(stderr, "[Fail] Compare failed at: \n");
        // fprintf(stderr, "  a #%s = %s\n", m_aPath.c_str(), m_aValue.c_str());
        // fprintf(stderr, "  b #%s = %s\n", m_bPath.c_str(), m_bValue.c_str());
    }

    CCompareMessage& PutValue(const std::string& avalue, const std::string& bvalue)
    {
        if (!avalue.empty())
        {
            m_aValue = avalue;
        }
        if (!bvalue.empty())
        {
            m_bValue = bvalue;
        }
        return *this;
    }

    CCompareMessage& SetFail()
    {
        m_pass = false;
        return *this;
    }

private:
    std::string m_aPath;
    std::string m_bPath;
    std::string m_aValue;
    std::string m_bValue;
    bool m_pass;
};

#define ASSERT_CMP(expr, msg) do { if(!(expr)) {msg; return -1;} } while(0)
#define ASSERT_CMPSTR(a, b, msg) do { if(strcmp((a), (b))) {msg; return -1;} } while(0)
#define ASSERT_CMPNUM(a, b, msg) do { if(!IsAlmostEqual((a), (b))) {msg; return -1;} } while(0)
#define RETURN_ONERR(iRet) do { if (0 != (iRet)) {return iRet;} } while(0)

int CompareJsonImpl(const rapidjson::Value& av, const rapidjson::Value& bv, bool bCanLess,
    const std::string& apath, const std::string& bpath)
{
    CCompareMessage msg(apath, bpath);
    if (av.IsObject())
    {
        ASSERT_CMP(bv.IsObject(), msg.PutValue("{object}", "??").SetFail());
        if (!bCanLess)
        {
            ASSERT_CMP(av.MemberCount() == bv.MemberCount(), msg.PutValue("{n}", "{m}").SetFail());
        }
        else
        {
            ASSERT_CMP(av.MemberCount() <= bv.MemberCount(), msg.PutValue("{n}", "{<n}").SetFail());
        }
        for (rapidjson::Value::ConstMemberIterator ai = av.MemberBegin(); ai != av.MemberEnd(); ++ai)
        {
            std::string aName = ai->name.GetString();
            rapidjson::Value::ConstMemberIterator bi = bv.FindMember(ai->name.GetString());
            ASSERT_CMP(bi != bv.MemberEnd(), msg.PutValue("->" + aName, "->").SetFail());
            int iRet = CompareJsonImpl(ai->value, bi->value, bCanLess, apath + "/" + aName, bpath + "/" + aName);
            RETURN_ONERR(iRet);
        }
    }
    else if (av.IsArray())
    {
        ASSERT_CMP(bv.IsArray(), msg.PutValue("[array]", "??").SetFail());
        if (!bCanLess)
        {
            ASSERT_CMP(av.Size() == bv.Size(), msg.PutValue("[n]", "[m]").SetFail());
        }
        else
        {
            ASSERT_CMP(av.Size() <= bv.Size(), msg.PutValue("[n]", "[<n]").SetFail());
        }
        int i = 0;
        for (rapidjson::Value::ConstValueIterator ai = av.Begin(), bi = bv.Begin(); ai != av.End(); ++ai, ++bi)
        {
            std::string aItem = apath + "/" + std::to_string(i);
            std::string bItem = bpath + "/" + std::to_string(i);
            int iRet = CompareJsonImpl(*ai, *bi, bCanLess, aItem, bItem);
            RETURN_ONERR(iRet);
            ++i;
        }
    }
    else if (av.IsString())
    {
        ASSERT_CMP(bv.IsString(), msg.PutValue("\"string\"", "??").SetFail());
        ASSERT_CMPSTR(av.GetString(), bv.GetString(), msg.PutValue(av.GetString(), bv.GetString()).SetFail());
    }
    else if (av.IsBool())
    {
        ASSERT_CMP(bv.IsBool(), msg.PutValue("bool", "??").SetFail());
        ASSERT_CMP(av.GetBool() == bv.GetBool(), msg.PutValue(av.GetBool()? "true" : "false", bv.GetBool()? "true" : "false").SetFail());
    }
    else if (av.IsInt())
    {
        ASSERT_CMP(bv.IsInt(), msg.PutValue("int", "??").SetFail());
        ASSERT_CMP(av.GetInt() == bv.GetInt(), msg.PutValue(std::to_string(av.GetInt()), std::to_string(bv.GetInt())).SetFail());
    }
    else if (av.IsInt64())
    {
        ASSERT_CMP(bv.IsInt64(), msg.PutValue("int64", "??").SetFail());
        ASSERT_CMP(av.GetInt64() == bv.GetInt64(), msg.PutValue(std::to_string(av.GetInt64()), std::to_string(bv.GetInt64())).SetFail());
    }
    else if (av.IsUint())
    {
        ASSERT_CMP(bv.IsUint(), msg.PutValue("Uint", "??").SetFail());
        ASSERT_CMP(av.GetUint() == bv.GetUint(), msg.PutValue(std::to_string(av.GetUint()), std::to_string(bv.GetUint())).SetFail());
    }
    else if (av.IsUint64())
    {
        ASSERT_CMP(bv.IsUint64(), msg.PutValue("Uint64", "??").SetFail());
        ASSERT_CMP(av.GetUint64() == bv.GetUint64(), msg.PutValue(std::to_string(av.GetUint64()), std::to_string(bv.GetUint64())).SetFail());
    }
    else if (av.IsDouble())
    {
        ASSERT_CMP(bv.IsDouble(), msg.PutValue("double", "??").SetFail());
        ASSERT_CMPNUM(av.GetDouble(), bv.GetDouble(), msg.PutValue(std::to_string(av.GetDouble()), std::to_string(bv.GetDouble())).SetFail());
    }
    else if (av.IsNull())
    {
        ASSERT_CMP(bv.IsNull(), msg.PutValue("null", "??").SetFail());
    }
    else
    {
        ASSERT_CMP(false, msg.PutValue("impossible", "impossible").SetFail());
    }

    return 0;
}

// END namespace jsonkit::impl
}
}

// interface functions
namespace jsonkit
{

bool compare(const rapidjson::Value& aJson, const rapidjson::Value& bJson)
{
    std::string root("/");
    return 0 == jsonkit::impl::CompareJsonImpl(aJson, bJson, false, root, root);
}

bool compatible(const rapidjson::Value& aJson, const rapidjson::Value& bJson)
{
    std::string root("/");
    return 0 == jsonkit::impl::CompareJsonImpl(bJson, aJson, true, root, root);
}

} /* jsonkit */ 
