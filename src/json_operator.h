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
 * conversion operator(|) which is from bit or operator, conver a generic scalar json value to c++ value.
 * mainly int or doule or string.
 * example:
 * @code
 *   int i = json | 0;
 *   int d = json | 0.0;
 *   std::string s = json | "";
 * @endcode
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

#define AS_INT | 0
#define AS_DOUBLE | 0.0
#define AS_STRING | ""

namespace jsonkit
{

/**************************************************************/

/** extract a scalar json vaue to native cpp type */
bool scalar_value(std::string& dest, const rapidjson::Value& json);
bool scalar_value(int& dest, const rapidjson::Value& json);
bool scalar_value(double& dest, const rapidjson::Value& json);
bool scalar_value(bool& dest, const rapidjson::Value& json);

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

/** determin if a json value is return by path operator on error
 * @details path operator for raw json must also return reference to a json value
 * to chained operation, when error occurs, return a static null json value.
 * also used in operator !json.
 * */
bool is_error_value(const rapidjson::Value& json);

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

inline
std::string operate_pipe(const rapidjson::Value& json, const char* defVal)
{
    return operate_pipe(json, std::string(defVal));
}

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
    /** constructor from json value and optional alloctor */
    COperand(rapidjson::Document& doc)
        : m_pJsonNode(&doc), m_pAllocator(&(doc.GetAllocator()))
    {}

    COperand(const rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator)
        : m_pJsonNode(&const_cast<rapidjson::Value&>(val))
        , m_pAllocator(&allocator)
    {}

    COperand(const rapidjson::Value& val)
        : m_pJsonNode(&const_cast<rapidjson::Value&>(val))
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
    COperand OperateStar(const rapidjson::Value& val) const
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

    /** can directly assign numeric value to rapidjson::Value.
     * example:
     * COperand& Assign(int iVal);
     * COperand& Assign(double dVal);
     * */
    template <typename valueT> 
    COperand& Assign(valueT val)
    {
        if (m_pJsonNode)
        {
            (*m_pJsonNode) = val;
        }
        return *this;
    }
    COperand& Assign(const char* psz);
    COperand& Assign(const std::string& str);

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

    template <typename valueT>
    COperand Append(const valueT& item) const
    {
        if (!m_pJsonNode || !m_pAllocator)
        {
            return *this;
        }
        if (false == m_pJsonNode->IsArray())
        {
            return *this;
        }

        rapidjson::Value val;
        COperand(val, *m_pAllocator).Assign(item);
        m_pJsonNode->PushBack(val, *m_pAllocator);

        return *this;
    }

    template <typename valueT>
    COperand Append(const std::pair<std::string, valueT>& item) const
    {
        if (!m_pJsonNode || !m_pAllocator)
        {
            return *this;
        }
        if (false == m_pJsonNode->IsObject())
        {
            return *this;
        }

        rapidjson::Value key;
        key.SetString(item.first.c_str(), item.first.size(), *m_pAllocator);
        rapidjson::Value val;
        COperand(val, *m_pAllocator).Assign(item.second);
        m_pJsonNode->AddMember(key, val, *m_pAllocator);

        return *this;
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

inline
std::string operator| (const rapidjson::Value& json, const char* defVal)
{
    return jsonkit::operate_pipe(json, std::string(defVal));
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
    return json.IsNull() || jsonkit::is_error_value(json);
}

inline
const rapidjson::Value& operator>> (const rapidjson::Value& json, std::string& dest)
{
    jsonkit::stringfy(json, dest);
    return json;
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
jsonkit::COperand operator* (const jsonkit::COperand& jsop, const rapidjson::Value& val)
{
    return jsop.OperateStar(val);
}

template <typename valueT>
valueT& operator|= (valueT& dest, const jsonkit::COperand& json)
{
    return json.OperatePipeto(dest);;
}

inline
std::string operator| (const jsonkit::COperand& json, const char* defVal)
{
    return json.OperatePipe(std::string(defVal));
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

/** add item to json array, or add pair to json object */
template <typename valueT>
jsonkit::COperand operator<< (const jsonkit::COperand& json, const valueT& val)
{
    return json.Append(val);
}

inline
jsonkit::COperand& operator>> (jsonkit::COperand& json, std::string& dest)
{
    if (json)
    {
        jsonkit::stringfy(*json, dest);
    }
    return json;
}

#endif /* end of include guard: JSON_OPERATOR_H__ */
