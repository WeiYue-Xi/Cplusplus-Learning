# C++ 模板

模板（template）是 C++ 实现泛型编程的重要机制。它允许把类型、编译期常量等作为参数，让编译器根据使用时提供的模板实参，形成对应的函数或类。

理解模板时，需要区分三个概念：**模板本身、模板实参、实例化后的函数或类型**。模板描述生成规则，模板实参决定具体版本，实例化让规则形成可以使用的实体。

本文以函数模板和类模板为主，代码用于说明对应知识点。同名示例按各自所在小节独立理解；标注为“调用示例”的语句放在函数体内。

## 目录

1. [为什么需要模板](#1-为什么需要模板)
2. [函数模板的定义](#2-函数模板的定义)
3. [模板实参推导](#3-模板实参推导)
4. [显式指定模板实参](#4-显式指定模板实参)
5. [模板实例化](#5-模板实例化)
6. [函数模板与重载](#6-函数模板与重载)
7. [类模板](#7-类模板)
8. [用栈的存储结构理解类模板](#8-用栈的存储结构理解类模板)
9. [模板参数的种类与默认值](#9-模板参数的种类与默认值)
10. [模板特化](#10-模板特化)
11. [模板定义的位置与类型检查](#11-模板定义的位置与类型检查)
12. [常见误区](#12-常见误区)

## 1. 为什么需要模板

### 1.1 从重复代码到泛型编程

假设需要交换两个整数，可以写成：

```cpp
void Swap(int& x, int& y)
{
    int tmp = x;
    x = y;
    y = tmp;
}
```

如果还要交换两个 `double`，可以再写一个重载：

```cpp
void Swap(double& x, double& y)
{
    double tmp = x;
    x = y;
    y = tmp;
}
```

两个函数的算法相同，只有参与操作的类型不同。继续支持更多类型，就需要重复编写和维护相同的逻辑。

模板把这种“与具体类型无关的共同逻辑”提取出来：

```cpp
template<class T>
void Swap(T& x, T& y)
{
    T tmp = x;
    x = y;
    y = tmp;
}
```

`T` 表示暂未确定的类型，使用时再确定它具体代表什么。这种对类型进行抽象、让算法适用于多种类型的编程方式，称为泛型编程。

### 1.2 模板与普通重载的区别

| 对比项 | 普通函数重载 | 函数模板 |
| --- | --- | --- |
| 代码组织 | 为不同参数列表分别编写函数 | 用参数化的类型描述共同逻辑 |
| 支持新类型 | 往往需要增加重载 | 新类型满足操作要求时，可以使用已有模板 |
| 不同类型的行为 | 每个重载可以采用不同的实现 | 通常复用相同算法，也可结合重载或特化定制 |
| 类型检查 | 检查具体函数 | 还要在模板实参确定后检查相关依赖操作 |

模板与重载可以配合使用。共同逻辑适合模板，特定类型的独立行为可以由重载表达。

### 1.3 泛型不代表适用于所有类型

上面的 `Swap` 包含三个操作：

```cpp
T tmp = x;  // 用 x 复制构造 tmp
x = y;      // 复制赋值
y = tmp;    // 复制赋值
```

因此，使用这个实现的类型必须支持相应的构造与赋值操作。某个类型即使能够声明变量，也不一定能够用于这个模板。

模板把类型推迟到使用时确定，但不会取消类型规则。

## 2. 函数模板的定义

### 2.1 基本语法

```cpp
template<class T>
T Add(const T& x, const T& y)
{
    return x + y;
}
```

可以分成两部分理解：

- `template<class T>` 是模板参数列表，声明 `T` 是一个类型模板参数。
- 后面的函数定义使用 `T` 表示参数类型、返回类型或局部变量类型。

`T` 是标识符，可以换成 `Type`、`Element` 等名称。单个简单类型参数通常使用 `T`，复杂模板则适合使用能表达用途的名称。

模板参数有自己的作用域，不能脱离相应模板任意使用，也不应在内部用同名声明遮蔽它。

### 2.2 `class` 与 `typename`

在**声明类型模板参数**时，下面两种写法等价：

```cpp
template<class T>
void Function(T value);

template<typename T>
void Function(T value);
```

这里的 `class` 并不要求实参必须是类类型。`int`、`double`、指针类型、自定义类类型等都可以作为类型模板实参。

`class` 和 `typename` 的等价关系仅针对这里的参数声明。`typename` 在依赖类型名称前还有其他用途，不能在所有位置都与 `class` 互换。

### 2.3 参数类型中的引用与 `const`

以 `T Add(const T& x, const T& y)` 为例：

- `T` 决定参与计算的类型和返回类型。
- `&` 表示通过引用接收对象，避免为了传参而复制整个对象。
- `const` 表示不能通过这个引用直接修改所引用的对象。
- 返回类型是 `T`，所以即使 `x + y` 产生其他类型的结果，也必须能够转换为 `T` 才能返回。

对于简单数值类型，按值传递通常也很合适。对于大型对象，`const T&` 常用于只读访问。参数传递方式需要结合类型与函数用途选择。

### 2.4 一个模板可以有多个参数

```cpp
template<class T, class U>
auto AddMixed(const T& x, const U& y)
{
    return x + y;
}
```

调用示例：

```cpp
auto result = AddMixed(10, 20.5);  // T 为 int，U 为 double，结果为 30.5
```

两个独立的类型模板参数允许两个实参具有不同类型。返回类型由 `x + y` 的表达式类型决定，而不固定为 `T` 或 `U`。

这种直接由函数体推导 `auto` 返回类型的写法从 C++14 开始支持。

## 3. 模板实参推导

### 3.1 根据函数实参推导类型

给定以下模板：

```cpp
template<class T>
T Add(const T& x, const T& y)
{
    return x + y;
}
```

调用时通常不必手动写出 `T`：

```cpp
int a = 10, b = 20;
double c = 10.5, d = 20.5;

Add(a, b);  // 推导出 T = int，结果为 30
Add(c, d);  // 推导出 T = double，结果为 31.0
```

编译器对照模板形参类型和实际传入的实参类型，确定模板参数。这一过程称为**模板实参推导**。

### 3.2 同一个参数必须被一致地推导

```cpp
int a = 10;
double b = 20.5;

// Add(a, b);  // 错误：第一个参数推导 T 为 int，第二个参数推导 T 为 double
```

这里并不是 `int` 与 `double` 无法相加，而是两个形参都使用同一个 `T`，推导结果发生了冲突。

对于这种直接从 `T` 形参推导的情况，编译器不会自动寻找一个“能够同时容纳两种实参的公共类型”，也不会先把 `int` 转换为 `double` 来消除冲突。

可以根据实际含义选择处理方式：

```cpp
Add(static_cast<double>(a), b);  // 先转换，再推导 T = double
Add<double>(a, b);               // 直接指定 T = double
AddMixed(a, b);                  // 使用两个独立的类型模板参数
```

这三种方式分别改变实参类型、直接给出模板实参、放宽模板对参数类型的要求。

### 3.3 值传递与引用传递影响推导

下面三个模板的形参形式不同：

```cpp
template<class T>
void ByValue(T value) {}

template<class T>
void ByReference(T& value) {}

template<class T>
void ByConstReference(const T& value) {}
```

对于普通整数变量，推导结果如下：

```cpp
int x = 10;
const int cx = 20;
```

| 调用 | 推导出的 `T` | 实例化后的形参类型 |
| --- | --- | --- |
| `ByValue(x)` | `int` | `int` |
| `ByValue(cx)` | `int` | `int` |
| `ByReference(x)` | `int` | `int&` |
| `ByReference(cx)` | `const int` | `const int&` |
| `ByConstReference(x)` | `int` | `const int&` |
| `ByConstReference(cx)` | `int` | `const int&` |

按值传递创建独立的参数对象，所以推导时会忽略实参类型的顶层 `const`。引用参数则需要考虑被引用对象的限定。

在 `const T&` 中，形参模式本身已经包含 `const`，因此对上面的两种实参，`T` 都推导为 `int`。

还需要区分临时值能否绑定到相应引用：

```cpp
ByValue(10);           // 可以
// ByReference(10);    // 错误：这里不能绑定到推导得到的非 const 左值引用
ByConstReference(10);  // 可以：const 左值引用可以绑定临时值
```

### 3.4 数组也会受到形参形式影响

```cpp
int values[3] = {1, 2, 3};

ByValue(values);      // T 推导为 int*，数组发生退化
ByReference(values);  // T 推导为 int[3]，形参为数组引用
```

因此，“传入什么类型，`T` 就一定原样是什么类型”并不准确。推导需要同时看实参和形参模式。

### 3.5 不能只靠接收返回值的变量推导类型

```cpp
template<class T>
T MakeValue()
{
    return T{};
}
```

调用示例：

```cpp
// int value = MakeValue();  // 错误：普通函数调用中，不能从左侧 int 推导 T
int value = MakeValue<int>();
```

`T` 没有出现在可供推导的函数形参中，编译器无法仅凭这个调用确定它。返回值赋给什么类型的变量，不会反过来决定这里的模板实参。

## 4. 显式指定模板实参

### 4.1 基本用法

调用时在函数名后使用尖括号，可以直接给出模板实参：

```cpp
Add<int>(10, 20);
Add<double>(10.5, 20.5);
```

`Add<int>` 中的 `int` 对应模板参数 `T`。此时不需要再从两个函数实参中推导 `T`。

### 4.2 指定类型后，可以发生参数转换

仍以 `T Add(const T&, const T&)` 为例：

```cpp
int a = 10;
double b = 20.5;

Add<int>(a, b);     // b 转换为 int，结果为 30
Add<double>(a, b);  // a 转换为 double，结果为 30.5
```

指定模板实参后，可以先把形参理解成具体的函数签名：

```cpp
// 仅用于说明类型替换后的签名：
int Add(const int& x, const int& y);
double Add(const double& x, const double& y);
```

接下来按普通函数调用规则判断实参能否传入。在第一个调用中，`double` 转换产生的临时 `int` 可以绑定到 `const int&`。

转换可能改变数据含义。例如，这里的 `20.5` 转成 `int` 后得到 `20`，小数部分被舍弃。

### 4.3 显式指定不代表任意转换都合法

```cpp
template<class T>
void Swap(T& x, T& y)
{
    T tmp = x;
    x = y;
    y = tmp;
}
```

调用示例：

```cpp
int a = 1;
double b = 2.0;

// Swap<double>(a, b);  // 错误：第一个形参是 double&，不能绑定 int 对象
```

`a` 虽然可以转换成一个临时 `double`，但这个临时值不能绑定到非 `const` 左值引用 `double&`。而且交换操作需要修改原变量，转换得到的临时对象也无法实现原本的交换含义。

所以，必须同时检查模板实参和实例化后的函数参数类型。

### 4.4 可以只指定前面的部分模板实参

```cpp
template<class Result, class T, class U>
Result AddAs(const T& x, const U& y)
{
    return static_cast<Result>(x + y);
}
```

调用示例：

```cpp
double value = AddAs<double>(10, 20.5);
// Result 显式指定为 double，T 和 U 分别推导为 int 和 double。
```

显式给出的模板实参按参数列表从左向右对应。其余参数只要能够推导，或具有可用的默认模板实参，就不必全部写出。

## 5. 模板实例化

### 5.1 模板是一套生成规则

```cpp
template<class T>
T Add(const T& x, const T& y)
{
    return x + y;
}
```

这个定义描述一组函数的共同形式。使用 `Add<int>` 和 `Add<double>` 时，会得到不同的函数模板特化。

可以把它们的行为近似理解为：

```cpp
// 概念上的展开，不表示编译器一定生成这样的源码或最终机器码。
int AddInt(const int& x, const int& y)
{
    return x + y;
}

double AddDouble(const double& x, const double& y)
{
    return x + y;
}
```

模板的类型替换、检查与实例化属于编译期机制。运行时调用的是已经确定的具体版本，而不是每次调用时再根据变量里的值选择 `T`。

模板通常不会引入运行时类型分派，但不同实参可能形成不同版本，增加编译工作和程序体积。最终是否生成独立机器码，还受到内联、优化等因素影响。

### 5.2 隐式实例化

当程序在需要定义的场景中使用某个模板版本，编译器可以按需实例化它：

```cpp
Add(10, 20);       // 推导出 T = int，并按需隐式实例化
Add<double>(1, 2); // 显式指定 T = double，仍可触发隐式实例化
```

**模板实参如何确定**与**模板如何实例化**是两个不同的问题。写了 `<double>`，并不意味着这条调用语句就是“显式实例化定义”。

同一个模板的同一组实参对应同一个特化；再次调用 `Add<int>` 不表示产生一种新的函数版本。

### 5.3 显式实例化定义

可以在命名空间作用域明确要求实例化某个版本：

```cpp
template<class T>
T Add(const T& x, const T& y)
{
    return x + y;
}

template int Add<int>(const int&, const int&);
```

最后一行是显式实例化定义。它不是函数调用，也没有提供另一套函数体，而是要求使用已有模板定义实例化指定版本。

还可以使用显式实例化声明：

```cpp
extern template int Add<int>(const int&, const int&);
```

它用于抑制相应的隐式实例化，通常与其他翻译单元中的显式实例化定义配合，集中生成某些模板版本。

### 5.4 常用术语对照

| 概念 | 示例 | 主要作用 |
| --- | --- | --- |
| 模板参数 | `template<class T>` 中的 `T` | 声明可被替换的参数 |
| 模板实参 | `Add<int>` 中的 `int` | 给参数提供具体类型或值 |
| 模板实参推导 | `Add(1, 2)` | 根据函数实参确定模板实参 |
| 显式指定模板实参 | `Add<int>(1, 2)` | 由调用者直接给出模板实参 |
| 隐式实例化 | 使用模板时按需发生 | 根据模板生成所需的特化 |
| 显式实例化定义 | `template int Add<int>(const int&, const int&);` | 明确要求实例化指定版本 |
| 显式特化 | `template<>` 开头的特化定义 | 为指定实参提供专门实现 |

“特化”还可以泛指模板参数确定后的具体版本；**隐式实例化得到一个特化**，不等于**编写了显式特化定义**。

## 6. 函数模板与重载

### 6.1 普通函数与函数模板可以同名

```cpp
int Add(int x, int y)
{
    return x + y;
}

template<class T>
T Add(T x, T y)
{
    return x + y + 5;  // 额外加 5，仅用于区分调用了哪个版本
}
```

这时既有普通函数，也有可以参与重载决议的函数模板。

### 6.2 匹配程度相同时，优先选择普通函数

```cpp
Add(10, 5);  // 结果为 15
```

普通函数可以直接接收两个 `int`，模板也可以推导出 `T = int`。在这里，两者的参数匹配程度相同，因此选择普通函数。

### 6.3 模板匹配更好时，可以选择模板

```cpp
Add(10.0, 5.0);  // 结果为 20.0
```

普通函数需要把两个 `double` 转换为 `int`，模板版本则可以精确匹配两个 `double`。因此选择模板版本。

判断顺序应当是先比较候选函数是否可用、参数匹配程度如何，再考虑匹配条件相同时的优先规则。“普通函数始终优先于模板”是错误的概括。

### 6.4 指定模板实参可以明确使用模板

```cpp
Add<int>(10, 5);  // 结果为 20
Add<>(10, 5);     // 结果为 20，空尖括号要求使用模板，T 仍由实参推导
```

普通函数没有模板实参列表，不能作为这里的普通函数版本被选中。如果存在多个同名函数模板，仍需要在这些模板之间完成重载决议。

### 6.5 模板推导失败后，其他重载仍可能可用

```cpp
Add(10, 5.5);  // 在本节这组重载中，选择普通函数，结果为 15
```

模板中的同一个 `T` 无法同时推导为 `int` 和 `double`，因此该模板不能提供可用候选。普通函数仍可以通过把 `5.5` 转为 `int` 完成调用。

这也说明：一个模板候选推导失败，不等于整个调用一定失败；还要检查其他重载。

### 6.6 函数模板本身也可以重载

```cpp
template<class T>
const char* Describe(T value)
{
    return "普通值";
}

template<class T>
const char* Describe(T* value)
{
    return "指针";
}
```

调用示例：

```cpp
int value = 10;
Describe(value);   // 选择第一个模板
Describe(&value);  // 选择第二个模板
```

对于指针实参，两个模板都可以匹配，但 `T*` 版本更专门，因此被选中。多个函数模板之间的这种比较称为模板的偏序。

## 7. 类模板

### 7.1 基本形式

函数模板抽象的是函数中的类型，类模板抽象的是类中的类型或其他编译期参数。

```cpp
template<class T>
class Box
{
public:
    explicit Box(const T& value)
        : _value(value)
    {
    }

    const T& Get() const
    {
        return _value;
    }

    void Set(const T& value)
    {
        _value = value;
    }

private:
    T _value;
};
```

调用示例：

```cpp
Box<int> number(10);
Box<double> decimal(3.5);

number.Set(20);
int value = number.Get();
```

模板参数 `T` 可以出现在成员变量、成员函数参数和返回类型等位置。同一份类定义因此能够描述不同元素类型的对象。

### 7.2 类模板与具体类型

| 写法 | 含义 |
| --- | --- |
| `Box` | 类模板的名称；在类模板内部还具有注入类名的用法 |
| `Box<int>` | 模板实参为 `int` 的具体类类型 |
| `Box<double>` | 模板实参为 `double` 的另一种类类型 |
| `Box<int> number(10)` | 创建一个 `Box<int>` 对象 |

`Box<int>` 和 `Box<double>` 是不同类型。即使它们来自同一个类模板，也不会自动拥有彼此之间的转换关系。

```cpp
Box<int> a(10);
Box<double> b(20.5);

// a = b;  // 当前 Box 没有提供这种跨类型赋值操作
```

### 7.3 构造函数和析构函数的名称

在类模板内部，构造函数和析构函数仍使用类名：

```cpp
template<class T>
class Example
{
public:
    Example() = default;
    ~Example() = default;
};
```

不需要把它们写成 `Example<T>()` 和 `~Example<T>()`。类作用域内的 `Example` 可以表示当前这个类模板特化，即这里的 `Example<T>`。

### 7.4 在类外定义成员函数

```cpp
template<class T>
class Box
{
public:
    explicit Box(const T& value);
    const T& Get() const;

private:
    T _value;
};

template<class T>
Box<T>::Box(const T& value)
    : _value(value)
{
}

template<class T>
const T& Box<T>::Get() const
{
    return _value;
}
```

每个类外定义前都需要给出相应的模板参数列表，并使用 `Box<T>::` 表明成员属于哪个类模板特化。

这里的 `Get` 是类模板的普通成员函数。它使用了类模板参数 `T`，不代表它本身额外声明了一组函数模板参数。

### 7.5 类模板实参推导

C++17 起，某些类模板可以根据构造实参推导模板实参。对于前面带有 `Box(const T&)` 构造函数的定义：

```cpp
Box number(10);     // 根据构造函数推导为 Box<int>
Box decimal(3.5);   // 推导为 Box<double>
```

这称为类模板实参推导（CTAD）。是否能够推导，取决于构造函数形成的推导指引或显式提供的推导指引，不能简单概括为“类模板都能省略尖括号”。

例如，构造函数只有容量参数时，无法由容量推导出元素类型：

```cpp
template<class T>
class Stack
{
public:
    explicit Stack(int capacity = 4) {}
};
```

调用示例：

```cpp
Stack<int> a(8);  // 可以：直接指定元素类型
// Stack b(8);   // 错误：8 只说明容量，无法推导 T
```

## 8. 用栈的存储结构理解类模板

下面的简化结构用于观察类型参数如何作用于成员和资源管理：

```cpp
template<class T>
class Stack
{
public:
    explicit Stack(int capacity = 4)
        : _data(new T[capacity]), _size(0), _capacity(capacity)
    {
    }

    ~Stack()
    {
        delete[] _data;
    }

    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

private:
    T* _data;
    int _size;
    int _capacity;
};
```

这个示例只展示存储结构，假定容量为正数。它不包含完整的栈操作；删除拷贝操作是为了避免示例中拥有资源的指针被直接复制。

### 8.1 模板参数决定哪些内容

对于 `Stack<int>`：

- `_data` 的类型为 `int*`。
- `new T[capacity]` 变成对 `int` 数组的分配。
- 如果成员函数使用 `T` 接收或返回元素，它们对应的元素类型也会变成 `int`。

对于 `Stack<double>`，这些位置对应地使用 `double`。

`_size` 和 `_capacity` 显式声明为 `int`，不会因为 `T` 不同而自动变成其他类型。模板只替换实际依赖模板参数的部分。

### 8.2 不同特化的对象独立存在

```cpp
Stack<int> a;
Stack<int> b;
Stack<double> c;
```

`a` 和 `b` 类型相同，但仍是两个不同对象，各自拥有自己的非静态成员。`c` 属于另一种类型，也拥有自己的成员和存储。

类模板共享的是生成规则，不是所有对象的数据。

### 8.3 模板代码中的操作会形成类型要求

表达式 `new T[capacity]` 会创建数组中的元素。如果 `T` 是类类型，这种写法需要能够默认构造这些元素；如果 `T` 为基本类型，这种写法不会自动将其值清零。

模板定义中出现某种操作，就对相应类型形成要求。例如：

| 模板中的表达式 | 对类型的要求 |
| --- | --- |
| `T value{};` | 能进行这种形式的值初始化 |
| `T copy = source;` | 能从 `source` 完成相应初始化 |
| `left = right;` | 存在可用的赋值操作 |
| `left + right` | 存在可用的加法操作 |
| `left < right` | 存在可用的小于比较操作 |

这些要求来自具体实现，而不是来自模板参数名 `T`。两个都叫 `Stack<T>` 的实现，如果采用不同存储方式，也可能对 `T` 提出不同要求。

## 9. 模板参数的种类与默认值

### 9.1 类型模板参数

```cpp
template<class T>
class Box;
```

类型模板参数接收类型，例如 `Box<int>`、`Box<double>`。

### 9.2 非类型模板参数

模板也可以把编译期的值作为参数。下面用数组长度举例：

```cpp
#include <cstddef>

template<class T, std::size_t N>
struct FixedArray
{
    static_assert(N > 0, "N must be positive");
    T data[N]{};
};
```

调用示例：

```cpp
FixedArray<int, 4> a;
FixedArray<int, 8> b;
```

这里 `T` 是类型模板参数，`N` 是非类型模板参数。`FixedArray<int, 4>` 与 `FixedArray<int, 8>` 是不同类型，因为长度参与了类型的形成。

```cpp
constexpr std::size_t count = 4;
FixedArray<int, count> c;  // 可以：count 可用于编译期常量表达式

std::size_t runtimeCount = 4;
// FixedArray<int, runtimeCount> d;  // 错误：runtimeCount 不是所需的常量表达式
```

即使运行时变量当前的值也是 `4`，也不能据此代替这里要求的编译期模板实参。

### 9.3 默认模板实参

```cpp
template<class T = int, std::size_t N = 4>
struct Buffer
{
    T data[N]{};
};
```

调用示例：

```cpp
Buffer<> a;          // Buffer<int, 4>
Buffer<double> b;    // Buffer<double, 4>
Buffer<double, 8> c; // Buffer<double, 8>
```

对于这里的类模板参数列表，一旦某个普通模板参数具有默认实参，后面的普通参数也应提供默认实参。函数模板的默认参数规则有所不同，不能把这一规则直接套用到所有函数模板。

### 9.4 默认模板实参与默认函数实参不同

```cpp
template<class T = int>
class Container
{
public:
    explicit Container(int capacity = 4) {}
};
```

`T = int` 决定省略类型模板实参时使用什么类型；`capacity = 4` 决定调用构造函数时省略容量参数采用什么值。

前者参与类型形成，后者参与对象构造。它们所在的层次不同。

## 10. 模板特化

### 10.1 为什么需要特化

通用模板描述大多数情况。有时某个特定类型需要不同实现，可以为它提供专门版本。

**实例化**是根据已有规则得到具体版本；**显式特化**是为指定模板实参另写实现。两者不能混为一谈。

### 10.2 类模板的全特化

```cpp
template<class T>
struct TypeInfo
{
    static const char* Name()
    {
        return "other";
    }
};

template<>
struct TypeInfo<int>
{
    static const char* Name()
    {
        return "int";
    }
};
```

调用示例：

```cpp
TypeInfo<double>::Name(); // 使用主模板，返回 "other"
TypeInfo<int>::Name();    // 使用全特化，返回 "int"
```

全特化确定了全部模板参数，因此模板参数列表写成空的 `template<>`。特化可以有自己的成员定义，不要求在主模板的成员上逐个修改。

显式特化应在会导致对应版本隐式实例化的使用之前声明，不能先使用主模板生成该版本，再在后面随意补上特化。

### 10.3 类模板的偏特化

偏特化不是指定唯一一种具体类型，而是为一类满足特定形式的实参提供实现：

```cpp
template<class T>
struct TypeInfo<T*>
{
    static const char* Name()
    {
        return "pointer";
    }
};
```

这段定义接在前面的主模板之后。调用示例：

```cpp
TypeInfo<int*>::Name();     // 返回 "pointer"
TypeInfo<double*>::Name();  // 返回 "pointer"
TypeInfo<int>::Name();      // 仍使用 int 的全特化，返回 "int"
```

`TypeInfo<T*>` 限定了“模板实参必须符合指针形式”，但指向的类型仍由 `T` 表示。因此它是偏特化。

### 10.4 函数模板的特化与重载

函数模板支持全特化。例如，为某个指定类型提供独立的返回结果：

```cpp
template<class T>
const char* TypeName()
{
    return "other";
}

template<>
const char* TypeName<int>()
{
    return "int";
}
```

调用示例：

```cpp
TypeName<double>(); // 使用主模板，返回 "other"
TypeName<int>();    // 使用全特化，返回 "int"
```

全特化以 `template<>` 开始，并给出了专门的函数体；显式实例化定义则使用 `template` 加具体签名，复用已有模板的实现。

函数模板**不支持偏特化**。需要按参数形式区分函数行为时，通常可以使用函数模板重载。

前文中的两个 `Describe`：

```cpp
template<class T>
const char* Describe(T value);

template<class T>
const char* Describe(T* value);
```

它们是两个重载的函数模板，第二个不是第一个的偏特化。虽然写法都包含更具体的类型模式，但语言规则和选择机制不同。

## 11. 模板定义的位置与类型检查

### 11.1 为什么模板定义通常放在头文件

普通函数在调用位置通常只需要声明；对应定义可以放在其他翻译单元，由链接过程找到。

模板还需要在实例化时根据具体模板实参形成相应定义。如果使用方只有声明，却看不到需要实例化的模板定义，也没有其他翻译单元提供对应的实例化定义，就可能出现未定义引用等链接错误。

因此，常见做法是把模板定义放在头文件中，或者将定义放入另一个文件并由头文件包含，使使用方能够看到定义。

这不是“模板只能写在头文件”的语法限制。对于只支持固定类型的设计，也可以把模板定义与显式实例化定义集中放在实现文件中。

### 11.2 模板错误不一定都等到调用时才出现

模板中的检查可以粗略分成两类：

- 不依赖模板参数的语法或名称等问题，通常在模板定义阶段就会被检查。
- 依赖具体模板实参的操作，要在相应实例化过程中进一步检查。

例如：

```cpp
template<class T>
void PrintMissingMember(const T& value)
{
    value.Print();
}
```

是否存在可访问的 `Print()` 成员，取决于 `T` 的具体类型。仅看到模板定义，还不能直接断定这句调用对所有类型都正确。

```cpp
// PrintMissingMember(10);  // 实例化时发现 int 没有 Print 成员
```

### 11.3 类模板的成员函数通常按需实例化

```cpp
template<class T>
class Holder
{
public:
    void Print() const
    {
        _value.Print();
    }

private:
    T _value{};
};
```

调用示例：

```cpp
Holder<int> value;  // 可以创建对象
// value.Print();   // 使用该成员函数时，才暴露 int 没有 Print 成员的问题
```

在这种普通的隐式实例化场景下，实例化类模板并不意味着立即实例化每个成员函数的函数体。但类布局、成员声明等形成具体类类型所必需的内容仍需要检查。

### 11.4 依赖类型名称与 `typename`

当一个嵌套名称依赖模板参数，且需要明确告诉编译器它表示类型时，可以使用 `typename`：

```cpp
template<class Container>
void UseValueType()
{
    typedef typename Container::value_type Element;
    Element value{};
}
```

`Container::value_type` 依赖于尚未确定的 `Container`。这里的 `typename` 用来明确该名称应当被解释为类型。

它与 `template<typename T>` 中“声明类型模板参数”的用途不同；此处不能改写为 `class Container::value_type`。

## 12. 常见误区

| 误区 | 正确理解 |
| --- | --- |
| 模板就是简单的文本替换 | 模板具有作用域、类型检查、推导、重载决议等语言规则 |
| `template<class T>` 只能接收类类型 | 这里的 `class` 声明类型参数，也可以接收 `int` 等类型 |
| 同一个 `T` 可以分别表示不同类型 | 一次具体实例化中，同一个 `T` 表示同一个类型 |
| 两种实参能互相转换，就一定能推导成功 | 推导必须符合形参模式，不能把所有普通隐式转换当成推导规则 |
| 指定 `<T>` 后所有实参都能传入 | 仍需满足具体参数类型的转换、引用绑定等要求 |
| `Add<int>(...)` 就是显式实例化定义 | 这是显式指定模板实参的调用，仍可能触发隐式实例化 |
| 普通函数始终优先 | 必须先比较可用性与匹配程度，条件相同时才适用相应优先规则 |
| 类模板的不同特化可以自动互相赋值 | 它们是不同类型，需要相应的转换或赋值操作 |
| 类模板可以始终省略模板实参 | 只有满足推导条件或其他相应规则时才可以 |
| 模板支持任何类型 | 类型必须支持模板实际使用的操作 |
| 函数模板可以偏特化 | 函数模板不支持偏特化，可以考虑重载 |
| 模板定义只能写在头文件 | 常见原因是实例化需要看到定义，显式实例化等设计也可采用其他组织方式 |
