/** 
 * @file json_operator.h 
 * @author lymslive 
 * @date 2021-10-26 
 *
 * @details 
 * path operator(/) which is from divide operator, implement json path pointer, but focus to query value.
 * return a reference to sub-tree of the current json value, or
 * return a static null json value if occur path error.
 * example for json root = {"subname": {"subsub": ??, ...}, "subarray": [0, {"value": ??, ...}, ...]}
 * @code
 *   sub_tree = json_root / "subname" / "subsub";
 *   sub_val  = json_root / "subarray" / 1 / "value";
 *   sub_tree = json_root / "subname/subsub";
 *   sub_val  = json_root / "subarray/1/value";
 * @endcode
 *
 * pipe operator(|) which is from bit or operator, conver a generic scalar json value to c++ value.
 * mainly int or double or string.
 * example:
 * @code
 *   int i = json | 0;
 *   int d = json | 0.0;
 *   std::string s = json | "";
 * @endcode
 *
 * JSOP macro make a json doc(value) modifiable. and more ...
 * */
#ifndef JSON_OPERATOR_H__
#define JSON_OPERATOR_H__

#include <string>
#include <vector>
#include <map>

#include "jsonkit_rpdjn.h"

#include "rapidjson/document.h"

/** create a json operand more quickly */
#define JSOP jsonkit::COperand

namespace jsonkit
{

/**************************************************************/

/** extract a scalar json value to native cpp type.
 * @details may make some auto conversion.
 * */
bool scalar_value(const char*& dest, const rapidjson::Value& json);
bool scalar_value(std::string& dest, const rapidjson::Value& json);
bool scalar_value(int& dest, const rapidjson::Value& json);
bool scalar_value(double& dest, const rapidjson::Value& json);
bool scalar_value(bool& dest, const rapidjson::Value& json);
bool scalar_value(uint32_t& dest, const rapidjson::Value& json);
bool scalar_value(int64_t& dest, const rapidjson::Value& json);
bool scalar_value(uint64_t& dest, const rapidjson::Value& json);

/** extract scalar json value without type conversion. */
template <typename valueT>
bool strict_value(valueT& dest, const rapidjson::Value& json)
{
    bool match = json.Is<valueT>();
    if (match)
    {
        dest = json.Get<valueT>();
    }
    return match;
}

/** read-only operator can perform directly on json value */
const rapidjson::Value& operate_path(const rapidjson::Value& json, const char* path);
const rapidjson::Value& operate_path(const rapidjson::Value& json, size_t index);

inline
const rapidjson::Value& operate_path(const rapidjson::Value& json, const std::string& path)
{
    return operate_path(json, path.c_str());
}

inline
const rapidjson::Value& operate_path(const rapidjson::Value& json, int index)
{
    return operate_path(json, (size_t)index);
}

/** perform operator |= to extrace value from json node */

template <typename valueT>
valueT& operate_pipeto(valueT& dest, const rapidjson::Value& json)
{
    scalar_value(dest, json);
    return dest;
}

/** perform operator | to extrace value from json node */
template <typename valueT>
valueT operate_pipe(const rapidjson::Value& json, const valueT& defVal)
{
    valueT val = defVal;
    return operate_pipeto(val, json);
}

/** object to mark a path error, for operator!
 * @details path operator for raw json must also return reference to a json value
 * to chained operation, when error occurs, return a static null json value.
 * */
struct CPathError
{
    static rapidjson::Value value;
};

/**************************************************************/

/** json operand that can perform chained oprator.
 * @details wrap two pointers for json value and it's allocator to suppor both
 * read and write. Best used as local variable, shorter life time than it's
 * underlying json value/document.
 * @note designed as puer value class, operator function return a new value.
 * */
class COperand
{
public:
    /** constructor from json value and optional allocator */
    COperand(rapidjson::Document& doc)
        : m_pJsonNode(&doc), m_pAllocator(&(doc.GetAllocator()))
    {}

    COperand(rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator)
        : m_pJsonNode(&val)
        , m_pAllocator(&allocator)
    {}

    explicit COperand(rapidjson::Value& val)
        : m_pJsonNode(&val)
    {}

    /** treat json operand as pointer or interator of underlying json node */
    rapidjson::Value& operator*() { return *m_pJsonNode; }
    rapidjson::Value* operator->() { return m_pJsonNode; }
    operator bool() const { return m_pJsonNode != nullptr; }

    /// create an zero value.
    COperand Zero() const { return COperand(nullptr, m_pAllocator); }

    /** perform path operator (slash /) on json node
     * @details allowed path parameter including string and int index.
     * will change the current json node.
     * */
    COperand OperatePath(const char* path) const;
    COperand OperatePath(size_t index) const;
    COperand OperatePath(const std::string& path) const
    {
        return OperatePath(path.c_str());
    }
    COperand OperatePath(int index) const
    {
        return OperatePath((size_t)index);
    }

    /** perform multiply operator(*), jump to new json node, as start base node
     * @note cannot jump to json node with Null value
     * */
    COperand OperateStar(rapidjson::Value& val) const
    {
        return val.IsNull() ? Zero() : COperand(&val, m_pAllocator);
    }

    /** perform operator |= */
    template <typename valueT>
    valueT& OperatePipeto(valueT& dest) const
    {
        if (m_pJsonNode)
        {
            operate_pipeto(dest, *m_pJsonNode);
        }
        return dest;
    }

    /** perform operator | */
    template <typename valueT>
    valueT OperatePipe(const valueT& defVal) const
    {
        valueT val = defVal;
        return OperatePipeto(val);
    }

    COperand& Assign(rapidjson::Value& val)
    {
        if (m_pJsonNode)
        {
            (*m_pJsonNode) = val;
        }
        return *this;
    }

    COperand& Assign(const rapidjson::Value& val)
    {
        if (m_pJsonNode)
        {
            m_pJsonNode->CopyFrom(val, *m_pAllocator);
        }
        return *this;
    }

    /** can directly assign numeric value to rapidjson::Value.
     * example:
     * COperand& Assign(int iVal);
     * COperand& Assign(double dVal);
     * */
    template <typename valueT> 
    COperand& Assign(valueT& val)
    {
        if (m_pJsonNode)
        {
            (*m_pJsonNode) = val;
        }
        return *this;
    }

    COperand& Assign(const std::string& str)
    {
        if (m_pJsonNode && m_pAllocator)
        {
            m_pJsonNode->SetString(str.c_str(), str.size(), *m_pAllocator);
        }
        return *this;
    }

#if 0
    // when allow c-stlye string, will disable literal string optimization
    COperand& Assign(const char* str)
    {
        if (m_pJsonNode && m_pAllocator)
        {
            m_pJsonNode->SetString(str, *m_pAllocator);
        }
        return *this;
    }
#endif

    template <typename valueT> 
    COperand& Assign(const std::vector<valueT>& vec)
    {
        if (!m_pJsonNode || !m_pAllocator)
        {
            return *this;
        }

        m_pJsonNode->SetArray();
        for (auto& item : vec)
        {
            rapidjson::Value val;
            COperand(val, *m_pAllocator).Assign(item);
            m_pJsonNode->PushBack(val, *m_pAllocator);
        }

        return *this;
    }

    template <typename valueT> 
    COperand& Assign(const std::map<std::string, valueT>& kv)
    {
        if (!m_pJsonNode || !m_pAllocator)
        {
            return *this;
        }

        m_pJsonNode->SetObject();
        for (auto& item : kv)
        {
            rapidjson::Value key;
            key.SetString(item.first.c_str(), item.first.size(), *m_pAllocator);
            rapidjson::Value val;
            COperand(val, *m_pAllocator).Assign(item.second);
            m_pJsonNode->AddMember(key, val, *m_pAllocator);
        }

        return *this;
    }

    template <typename valueT>
    COperand& operator=(const valueT& val)
    {
        return Assign(val);
    }

    //template <typename valueT>
    COperand& operator=(rapidjson::Value& val)
    {
        return Assign(val);
    }

    bool CanActArray() const
    {
        return (m_pAllocator && m_pJsonNode && m_pJsonNode->IsArray());
    }

    bool CanActObject() const
    {
        return (m_pAllocator && m_pJsonNode && m_pJsonNode->IsObject());
    }

    // Append (for operator <<)
    // can perform on array one by one or object pair by pair
    template <typename valueT>
    COperand Append(valueT& item) const
    {
        if (!m_pJsonNode || !m_pAllocator)
        {
            return *this;
        }
        if (m_pJsonNode->IsArray())
        {
            return AppendArray(item);
        }
        else if (m_pJsonNode->IsObject())
        {
            return AppendObject(item);
        }
        return *this;
    }

    template <typename valueT>
    COperand Append(const std::vector<valueT>& vec) const
    {
        if (CanActArray())
        {
            for (auto& item : vec)
            {
                AppendArray(item);
            }
        }
        return *this;
    }

    template <typename valueT> 
    COperand Append(const std::map<std::string, valueT>& kv) const
    {
        if (CanActObject())
        {
            for (auto& item : kv)
            {
                DoAddMember(item.first, item.second);
            }
        }
        return *this;
    }

    template <typename valueT>
    COperand Append(const std::pair<std::string, valueT>& item) const
    {
        return AddMember(item.first, item.second);
    }

    template <typename valueT>
    COperand AddMember(const std::string& key, valueT& value) const
    {
        if (!m_pJsonNode || !m_pAllocator)
        {
            return *this;
        }
        if (!m_pJsonNode->IsObject())
        {
            return *this;
        }
        return DoAddMember(key, value);
    }

private:
    template <typename valueT>
    COperand DoAddMember(const std::string& key, valueT& value) const
    {
        rapidjson::Value keyNode;
        keyNode.SetString(key.c_str(), key.size(), *m_pAllocator);
        rapidjson::Value valNode;
        COperand(valNode, *m_pAllocator).Assign(value);
        m_pJsonNode->AddMember(keyNode, valNode, *m_pAllocator);
        return *this;
    }

    template <typename valueT>
    COperand AppendArray(valueT& item) const
    {
        rapidjson::Value val;
        COperand(val, *m_pAllocator).Assign(item);
        m_pJsonNode->PushBack(val, *m_pAllocator);
        return *this;
    }

    template <typename valueT>
    COperand AppendObject(valueT& item) const
    {
        if (m_pJsonNode->ObjectEmpty())
        {
            return AppendMemberKey(item);
        }
        else if(ExpectMemberValue())
        {
            return AppendMemberValue(item);
        }
        else
        {
            return AppendMemberKey(item);
        }
    }

    // append a member with key from provided item, 
    // pending a specail null value, waiting anothr append
    template <typename valueT>
    COperand AppendMemberKey(valueT& item) const
    {
        rapidjson::Value key;
        COperand(key, *m_pAllocator).Assign(item);
        if (key.IsString())
        {
            rapidjson::Value val;
            SetPendingNull(val);
            m_pJsonNode->AddMember(key, val, *m_pAllocator);
        }
        return *this;
    }

    template <typename valueT>
    COperand AppendMemberValue(valueT& item) const
    {
        rapidjson::Value val;
        COperand(val, *m_pAllocator).Assign(item);
        auto& last = --m_pJsonNode->MemberEnd();
        last->value = val;
        return *this;
    }

    void SetPendingNull(rapidjson::Value& val) const
    {
        val.SetNull();
        *((int*)&val) = 0x0bcaffed;
    }

    bool IsPendingNull(rapidjson::Value& val) const
    {
        return val.IsNull() && (*((int*)&val) == 0x0bcaffed);
    }

    bool ExpectMemberValue() const
    {
        auto& last = --m_pJsonNode->MemberEnd();
        return IsPendingNull(last->value);
    }
private:
    rapidjson::Value* m_pJsonNode = nullptr;
    rapidjson::Document::AllocatorType* m_pAllocator = nullptr;

    // internal use, contruct from pointer directlly
    COperand(const rapidjson::Value* pJsonNode, rapidjson::Document::AllocatorType* pAllocator)
        : m_pJsonNode(const_cast<rapidjson::Value*>(pJsonNode))
        , m_pAllocator(pAllocator)
    {}
};

} /* jsonkit */ 

/**************************************************************/

/** operator for rapidjson::Value, in globle namespace.
 * @param json 
 * @param path json path, string or int
 * @return reference to (may) another json value
 * */
template <typename pathT>
const rapidjson::Value& operator/ (const rapidjson::Value& json, const pathT& path)
{
    return jsonkit::operate_path(json, path);
}

template <typename pathT>
rapidjson::Value& operator/ (rapidjson::Value& json, const pathT& path)
{
    return const_cast<rapidjson::Value&>(jsonkit::operate_path(json, path));
}

template <typename valueT>
valueT& operator|= (valueT& dest, const rapidjson::Value& json)
{
    return jsonkit::operate_pipeto(dest, json);
}

// to support literal string as char[N], auto convert to const char*
inline
const char* operator| (const rapidjson::Value& json, const char* defVal)
{
    return jsonkit::operate_pipe(json, defVal);
}

template <typename valueT>
valueT operator| (const rapidjson::Value& json, const valueT& defVal)
{
    return jsonkit::operate_pipe(json, defVal);
}

template <typename valueT>
valueT operator| (const valueT& defVal, const rapidjson::Value& json)
{
    return jsonkit::operate_pipe(json, defVal);
}

inline
bool operator! (const rapidjson::Value& json)
{
    return &json == &jsonkit::CPathError::value;
}

/** perform json+nubmer , satisfy commutative law*/
template <typename valueT>
typename std::enable_if<std::is_arithmetic<valueT>::value, valueT>::type
/*valueT*/ operator+ (const rapidjson::Value& json, const valueT& base)
{
    return jsonkit::operate_pipe(json, 0) + base;
}

template <typename valueT>
typename std::enable_if<std::is_arithmetic<valueT>::value, valueT>::type
/*valueT*/ operator+ (const valueT& base, const rapidjson::Value& json)
{
    return base + jsonkit::operate_pipe(json, 0);
}

/** perform json+string */
inline
std::string operator+(const rapidjson::Value& json, const std::string& base)
{
    return  jsonkit::operate_pipe(json, std::string()) + base;
}

inline
std::string operator+(const std::string& base, const rapidjson::Value& json)
{
    return  base + jsonkit::operate_pipe(json, std::string());
}

template <typename valueT>
typename std::enable_if<std::is_arithmetic<valueT>::value, rapidjson::Value&>::type
/*rapidjson::Value&*/ operator+= (rapidjson::Value& json, const valueT& base)
{
    json = json + base;
    return json;
}

template <typename valueT>
valueT& operator+= (valueT& base, const rapidjson::Value& json)
{
    base = base + json;
    return base;
}

inline
const rapidjson::Value& operator>> (const rapidjson::Value& json, std::string& dest)
{
    jsonkit::stringfy(json, dest);
    return json;
}

inline
std::ostream& operator<< (std::ostream& os, const rapidjson::Value& json)
{
    jsonkit::write_stream(json, os);
    return os;
}

/**************************************************************/

/** JSOP operator for COperand, in globle namespace.
 * @param json wrapped json operand
 * @param path json path, string or int
 * @return another json operand that may moved pointer
 * */
template <typename pathT>
jsonkit::COperand operator/ (const jsonkit::COperand& json, const pathT& path)
{
    return json.OperatePath(path);
}

inline
jsonkit::COperand operator* (const jsonkit::COperand& jsop, rapidjson::Value& val)
{
    return jsop.OperateStar(val);
}

inline
jsonkit::COperand operator* (rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator)
{
    return jsonkit::COperand(val, allocator);
}

inline
jsonkit::COperand operator* (rapidjson::Document::AllocatorType& allocator, rapidjson::Value& val)
{
    return jsonkit::COperand(val, allocator);
}

inline
jsonkit::COperand operator* (rapidjson::Value& val, rapidjson::Document& doc)
{
    return jsonkit::COperand(val, doc.GetAllocator());
}

inline
jsonkit::COperand operator* (rapidjson::Document& doc, rapidjson::Value& val)
{
    return jsonkit::COperand(val, doc.GetAllocator());
}

template <typename valueT>
valueT& operator|= (valueT& dest, const jsonkit::COperand& json)
{
    return json.OperatePipeto(dest);;
}

inline
const char* operator| (const jsonkit::COperand& json, const char* defVal)
{
    return json.OperatePipe(defVal);
}

template <typename valueT>
valueT operator| (const jsonkit::COperand& json, const valueT& defVal)
{
    return json.OperatePipe(defVal);
}

template <typename valueT>
valueT operator| (const valueT& defVal, const jsonkit::COperand& json)
{
    return json.OperatePipe(defVal);
}

/** append a string postfix to json string value, must wrap with JSOP. */
inline
jsonkit::COperand operator+= (jsonkit::COperand json, const std::string& rhs)
{
    if (json)
    {
        json = (*json) + rhs;
    }
    return json;
}

/** add item to json array, or add pair to json object */
template <typename valueT>
jsonkit::COperand operator<< (const jsonkit::COperand& json, const valueT& val)
{
    return json.Append(val);
}

inline
jsonkit::COperand operator<< (const jsonkit::COperand& json, rapidjson::Value& val)
{
    return json.Append(val);
}

#endif /* end of include guard: JSON_OPERATOR_H__ */
