# C++ 中 Json 操作符重载设计探讨

lymslive / 2021-10-29

## 引言

Json 在业界实践中颇受欢迎。实现 Json 的 C/C++ 库也相当多，其 API 虽大同小异，
却也不尽相同。笔者目前推崇的是 rapidjson 库，因其效率一流，是 C++ 程序员应该追
求的一个目标。但在使用过程中又不免觉得 rapidjson 的使用 API 不够友好，尤其是设
值修改时要多传一个内存分配器参数（Allocator），当然这也是它为了效率实现的一种
交换。

于是我想到了操作符重载。我在想当年的 C++ 教材学习上也算一块重要且有趣的内容了
，然而在实际工作中，使用 C++ 操作符重载的场合很少。主要是操作符重载容易误解，
经常不如规范选用的函数名那么能望文生义。这就说明操作符重载需要经过审慎的设计，
要让操作符重载尽可能合理，且能让使用团队达成共识。

而 Json 作为通用的数据结构抽象，越发显得重要与流行广泛，它不同与对具体业务的抽
象，因此我认为对 Json 设计一套操作符重载，定义一些操作符号对于 Json 的意义，以
简化对 Json 数据的操作，那是值得探讨的。

## Json Path

Json Path（也有的叫 Json Pointer）是另一个让我想到 Json 操作符重载的直接原因。
这是一项有关 Json 的标准，有兴趣者可自行查找相关详细说明文档。简单地说，可以将
Json 的 DMO 想象为一棵结点树，那么 Json Path 就是通向某一具体结点的路径指引。

例如，随写一个 Json 数据文档：

```json
{
  "aaa": 1, "bbb":2,
  "ccc": [3, 4, 5, 6],
  "ddd": {"eee":7, "fff":8.8}
}
```

就有 `/ddd/eee` 表示值为 `7` 的那个 json 结点，或者不严格地记为

```
/ddd/eee = 7
/ddd/fff = 8.8
/aaa = 1
/bbb = 2
/ccc/0 = 3
/ccc/1 = 4
```

没错，按 Json Path 标准，引用数组内某个元素也是用 `/` 直接加索引，而非用 `[]`
。这正是 Json 出于简单设计原则，比 xpath（xml文档选择元素的方法）简单的地方：
只用斜杠 `/` 分隔路径，对象用键名索引，数组用数字下标索引，只限明确指定一个结
点。

## C++ 操作符重载

### 路径操作符 `/`

考虑到 Json Path 表达式的路径分隔符 `/` 也正是 C++ （以及大多数编程语言）的除
法操作符，完全可以为 Json 对象重载除号的操作意义。

例如，用 rapidjson 库，表示 Json 的类型是 `rapidjson::Value` ：

```cpp
rapidjson::Value& operator/(rapidjson::Value& json, const char* path);
rapidjson::Value& operator/(rapidjson::Value& json, size_t path);
// 另加 const rapidjson::Value& 版本
```

如果实现了这样的重载，在 C++ 源码中就可以这样写了：

```cpp
// ... Parser json 为 root 结点
auto& eeeJ = json / "ddd" / "eee";
auto& c0 = json / "ccc" / 0;
auto& fffJ = json / "ddd/fff";
auto& c1 = json / "/ccc/1";
```

几点说明，`operator/` 应该返回 json `Value` 类型的引用，这样就能实现链式调用。
同时为了灵活性，重载实现中不仅支持将路径的每部分开写，也支持合起来作一次 `/`
操作，就如将 Json Path 标准表达式字符串放在双引号中。rapidjson 库专门有一个类
`rapidjson::Pointer` 来解析 Json Path 表达式。事实上，分开写不仅更清晰点，也更
有效率，因为程序员根据了已知信息对 Json Path 表达式进行了拆分。但是支持合写的
Json Path 表达式的意义还在于允许程序外配置。

标准 Json Path 要求以 `/` 开头，不过在 C++ 重载 `/` 后，既然已经有了这个 `/`
，允许省略前导 `/` 会更友好。

另一个重要问题是，如果非法路径，Json DMO 中不存在指定路径的结点，`operator/`
该返回什么呢？它必须也是某个 `rapidjson::Value` 的引用。为此，我引入了一个特殊
的表达 Json Null 的静态 `rapidjson::Value` 变量，当路径操作符失败时，返回这个
静态变量的引用。用户代码若为健壮性考虑，应负责判空处理。

如果对 Json 结点的常引用执行路径操作符作只读操作，那是安全的，只不过在实践中觉
得为了用户方便，还是放开允许非常引用的版本。

### 取值操作符 `|`

对一个 Json 执行链式路径操作 `/` 时，在正确操作下最后都能引用到一个叶结点。它
存着一个 Json 标量，主要是数字或字符串（还有三个特殊值 `true` `false` `null`）
，一般可以对应 C++ 的基本类型变量（原理上字符串不是基本类型，只不过业务代码层
面上一般会将字符串当作事实上的简单类型标量）。

然后就抛出另一个问题，当取到 Json 叶结点的引用后，如何更简单直接地对该标量进行
读值与写值操作，是否也可以重载某个合适的操作符来表达这层意义？

先讨论相当简单的只读操作。首先不能直接用 `=` 赋值操作符，至少在 rapidjson 库是
不支持如下用法的：

```cpp
int i = json; // error
```

重载 `=` 必须在类内定义，不能武断地入侵修改底层库，rapidjson 库不支持 json 值
类型向基本类型隐式转换肯定是有理由的。因为它支持反过来的写法，实际是重载了
json 值类型的构造函数与赋值操作：

```cpp
int i = 2;
rapidjson::Value json = i;
json = 3;
```

如果定义了 `rapidjson::Value` 与 `int` 类的双向互转，可能容易混乱，在某些情况
下可能出现二义性。

所以需要找另一个符号来表示将 json 值转为基本类型。于是我想到将路径操作符 `/`
扶正后变成的 `|` 符号，这在 Linux shell 下是常用的管道操作符，在 C++ 语法中是
不太常用的位或操作符。

假设重载了如下定义：

```cpp
int operator| (const rapidjson::Value& json, int defaultVal);
double operator| (const rapidjson::Value& json, double defaultVal);
...
```

就可以写成如下表达式：

```cpp
int i = json | 0;
double d = json | 0.0;
```

其义是假如 `json` 确实保存着一个整数，则返回这个整数，否则返回默认值
`defaultVal` 。浮点数的情况类似。这里可将 `|` 读作“或”。考虑到 `|` 的原始意
义应该支持交换律，同时也应该让 `|=` 保持相应的意义。

```cpp
int iVal = 100;
(json | iVal) == (iVal| json); // true
iVal = iVal | json;
iVal |= json; // 等效上一句

```

### 赋值操作符 `=` 与 JSOP 的操作子封装

前面提到 rapidjson 库的 json 值类型支持基本类型的赋值操作如：

```cpp
rapidjson::Value json = 3;
json = 3.14;
```

好像是自然支持简单的写值语法了，但这是受限的写法，它只支持数字类型，不支持另一
大类标量，字符串的直接赋值。因为各种数字类型都足以在 `rapidjson::Value` 对象的
栈区直接保存，而字符串极有可能在额外申请空间（短字符串优化除外）。按 rapidjson
库的设计哲学，凡是涉及内存分配的函数方法，都需要传入额外的分配器参数。尝试以下
写法是错误的，因为操作符重载没法传入额外参数。

```cpp
std::string str("some string")
rapidjson::Value json = str; // Error
rapidjson::Value json = str.c_str(); // Error
rapidjson::Value json = "static string"; // 合法
```

其中最后一句合法是由于它知道 `=` 后面是安全可靠的字面量，在 json 值内可以只存
一个指针而不用拷贝字符串内容。这显然受限极大，没法简单地用 `=` 将字符串变量赋
给 json 值。当然其他 json 实现库是有可能直接由字符串赋值的。

为了在 `rapidjson::Value` 语境库下也能直接赋值，我只好另外设计一个简单值语义的
类，它封装一个 json 结点（指针）及可能需要用到的分配器（指针），然后为该类重载
操作符，转为操作其封装的 json 结点。

```cpp
namespace jsonx
{
class COperand
{
  rapidjson::Value* m_pJsonNode = nullptr;
  rapidjson::Document::AllocatorType* m_pAllocator = nullptr;
public:
  COperand operator=(int iVal);
  COperand operator=(const std::string& str);
  ...
  // 也要支持 operator/
};
}
#define JSOP jsonx::COperand
```

其实现目的是为了能写出类似如下的表达式：

```cpp
rapidjson::Document root;
// ... Parse 构建 json DMO
JSOP(root) / "path" / "to" / "leaf" = 100;
JSOP(root) / "path" / "to" / "leaf" = 1.0;
JSOP(root) / "path" / "to" / "leaf" = "some string";
JSOP(root) / "path" / "to" / "leaf" = std::string("some string");
```

可以直观地理解 `JSOP` 宏将一个正常的 json 值转为可写的 json 。一般它从根结点
`rapidjson::Document` 类对象构造，因为只有 `Document` 中存了一个分配器，
rapidjson 并不会在其下每个 `Value` 存一份分配器（指针）。`JSOP` 操作子封装了两
个指针，但并不持有指针，因此它只适于构造临时对象（尤其是在一个表达式中），其生
命周期必须短于它指向的底层 json 结点。就类似 STL 容器的迭代器一样，不宜长期持
有以防失效。

既然把 `JSOP` 视为 json DMO 的手动档迭代器，那也自然地要为其重载解引用操作符
`\*` 与 指针操作符 `->` ，实现为代理内部所指 json 结点的操作。

然后有另一个问题，`JSOP` 的路径操作符 `/` 每次会生成一个新的 `JSOP` 值（对象）
。这可以类比相像复数类 `std::complex` ，它的每次加减乘除操作应该生成另一个复数
，而不是修改左侧操作符的值。这样在长路径索引时，JSOP 的路径操作就比原生的 json
路径操作要稍微低效。不能简单地将中间结点转为 JSOP ，因其缺失分配器参数，无法支
持完全的写操作，若只读的话还不如使用原生的 json 值类。

为解决这个问题，我再重载了乘号 `\*` 操作符，令 JSOP 快速从根结点跳到中间结点而
保持分配器参数。如：

```cpp
rapidjson::Document root;
auto& node = root / "path" / "to" / "node"; // rapidjson::Value 类型
JSOP(root) * node; // 等效于下一句
JSON(root) / "path" / "to" / "node";

JSOP(root) * node / "key" = "value"; // 赋值有效
JSOP(node) / "key" = "value"; // 赋值无效，JSOP 缺失分配器参数
JSOP(node, root.GetAllocator()) / "key" = "value"; // 赋值有效，但麻烦

&(*JSOP(root)) == &root; // 该 JSOP 指向 root
&(*(JSOP(root)*node)) == &node; // 该 JSOP 指向 node
```

注意到乘号与解引用是同一个符号 `\*` ，乘号重载可定义在类外非成员函数，解引用只
能在类内定义为成员函数。这两个操作作用于 JSOP 有某种一致的语义，解引用 `\*` 取
其底层结点，如果乘以另一个结点，改变了其底层结点，再用解引用也正是取到那个新结
点。同时乘号与除号的操作符优先级是一样的。如果将除号称为 JSOP/json 路径操作符
，那么或可称乘号为 JSOP 跳跃操作符。当然最好在同一棵 DMO 树内跳跃，这是
rapidjson 底层库的实践要求。

### 基于 JSOP 的其他修改操作符

以上提出 JSOP 操作子的封装，初衷是为了简化 json 赋值操作的写法。既然实现可写了
，那也可以再考虑其他修改操作如何符号化。这部分主要是扩充内容，可能比较难做到类
似路径操作 `/` 的形象化。

除了叶结点标量赋值改值，json 另一大类操作是对中层的集合结点（数组、对象）进行
增删操作。对于增加元素操作，我曾想过两种符号的选择，一是 `+=`，一是 `<<` ，然
后对比之下，还是觉得 `<<` 更好些。例如可以如下写法：

```cpp
auto& array = root / "path" / "to" / "array-node";
auto& object = root / "path" / "to" / "object-node";

JSOP(root) * array << 1 << 2 << "some string";
JSOP(root) * object << std::make_pair("key", "val");
```

而 `+=` 与 `+` 操作符，还是让它们作用于标量叶结点的“加”操作，不仅数字可以相
加，C++ 的字符串 `std::string` 也重载了“加”操作。如：

```cpp
auto& scalar = root / "path" / "to" / "leaf-node";
// 假设 scalar 是 int 结点
int i = scalar + 1;
scalar = i + 1;
scalr += 1;

// 假设 scalar 是字符串结点
std::string str = JSOP(root) * scalar + "post-fix";
JSOP(root) * scalar = str + "post-fix";
JSOP(root) * scalar += "post-fix";
```

这样，就把 json 当作泛型的标量来使用了。

对于删除元素操作，如果增加元素使用 `+=` 的话，正好有对应的 `-=` 表示删除。但是
加号 `+` 与 `-` 很少见于用于集合的操作。于是另想到 `%` 符号，与 `/` 有点联系，
也接一层路径参数（只限一层）的话，表示删除 Object 的一个元素（键值对）。如：

```cpp
json % "rm-key"; // 删除一个键，返回 json 结点本身
json % "rm-key1" % "rm-key2"; // 可链式调用
```

还有一个问题。json 既是泛型的，在运行过程中可以改变类型（但在 C++ 程序实践中不建
议随便一个 json 类型的结点）。这可以用之前介绍的 JSOP 操作子的赋值 `=` 操作符
实现。如果将一个 `std::vector` 赋给 json ，它就成了 array 类型，如果将一个
`std::map` 赋给 json ，它就变成一个 object 类型。如果是赋空容器，那就表示只改
变 json 类型，还未填充元素，变成 `[]` 或 `{}` 了。 如：

```cpp
JSOP(root) * node = std::vector<int>();
JSOP(root) * node << 1 << 2 << 3 << "string also ok in json array";

std::map<std::string, int> mymap = {....};
JSOP(root) * node = mymap;
```

## 结语

以上，为简化在 C++ 程序中对 json 的操作，重载了一批操作符。虽然是基于
rapidjson 库，但对于其他 json 库的使用，或重新开发 json 库，也应有一定的参考意义
。操作符重载并不是什么高深技术，难的是让操作符重载本身有积极意义。

代码上传于 https://github.com/lymslive/jsonkit 在 src/json_operator.h 文件中。
欢迎指教。
