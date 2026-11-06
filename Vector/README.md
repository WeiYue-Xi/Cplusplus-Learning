# C++ vector 知识整理

`std::vector` 是标准库中的动态顺序容器，可以理解为能够自动管理存储空间的数组。它结合了模板、动态内存管理和迭代器，是学习 STL 的重要起点。

本文以本项目采用的 **C++20** 为基准，多数基础操作在 C++11 中也可以使用。除“完整示例”外，各代码块按独立片段理解；操作语句放入函数体，按示例补充对应头文件。

## 目录

1. [基本特点](#1-基本特点)
2. [定义与初始化](#2-定义与初始化)
3. [元素访问](#3-元素访问)
4. [遍历与迭代器](#4-遍历与迭代器)
5. [size、capacity、reserve 与 resize](#5-sizecapacityreserve-与-resize)
6. [插入、删除与赋值](#6-插入删除与赋值)
7. [扩容与对象生命周期](#7-扩容与对象生命周期)
8. [迭代器、指针和引用失效](#8-迭代器指针和引用失效)
9. [配合标准算法](#9-配合标准算法)
10. [自定义类型、复制与移动](#10-自定义类型复制与移动)
11. [二维 vector 与特殊类型](#11-二维-vector-与特殊类型)
12. [复杂度与使用建议](#12-复杂度与使用建议)
13. [完整示例](#13-完整示例)
14. [常见误区与自测](#14-常见误区与自测)
15. [参考资料](#15-参考资料)

## 1. 基本特点

使用前包含头文件：

```cpp
#include <vector>

std::vector<int> numbers;
```

`vector` 是类模板，`std::vector<int>` 和 `std::vector<double>` 是不同类型。容器中的元素类型一致。

对于通常的 `vector<T>`（`vector<bool>` 特化除外）：

- 元素连续存储，可以通过下标随机访问。
- 尾部追加方便，容量不足时自动重新分配存储空间。
- 中间插入和删除需要移动后续元素，开销随元素数量增长。
- 容器负责所持有元素的构造、析构及存储空间的释放。

| 对比项 | 原生数组 `T[N]` | `std::array<T, N>` | `std::vector<T>` |
| --- | --- | --- | --- |
| 元素数量 | 创建后固定 | 编译期固定 | 运行时可变 |
| 连续存储 | 是 | 是 | 是，`bool` 特化除外 |
| 获取数量 | 需保留数组类型或另传长度 | `size()` | `size()` |
| 尾部追加 | 不支持 | 不支持 | `push_back()` / `emplace_back()` |

不要把“动态数组”理解为容器对象本身一定在堆上。局部变量 `std::vector<int> v;` 的容器对象具有自动存储期，其管理的元素存储空间由分配器取得。

## 2. 定义与初始化

```cpp
#include <vector>

std::vector<int> a;                  // 空容器，size() 为 0
std::vector<int> b(5);               // 5 个 int，默认分配器下均为 0
std::vector<int> c(5, 10);           // 5 个 10
std::vector<int> d{1, 2, 3, 4};      // 4 个指定元素
std::vector<int> e(d);              // 复制 d 的元素
std::vector<int> f(d.begin(), d.end()); // 迭代器区间构造

int arr[] = {7, 8, 9};
std::vector<int> g(arr, arr + 3);    // 复制数组的三个元素
```

迭代器区间通常采用左闭右开形式 **`[first, last)`**：包含 `first` 指向的元素，不包含 `last` 指向的元素。

### 圆括号与花括号的区别

```cpp
std::vector<int> a(3, 5); // {5, 5, 5}：数量 + 初值
std::vector<int> b{3, 5}; // {3, 5}：初始化列表
std::vector<int> c(3);    // {0, 0, 0}
std::vector<int> d{3};    // {3}
```

花括号初始化会优先考虑匹配的 `std::initializer_list` 构造函数。表达“创建多少个元素”时，注意使用圆括号。

## 3. 元素访问

| 接口 | 含义 | 使用条件 |
| --- | --- | --- |
| `v[i]` | 访问第 `i` 个元素 | 必须保证 `i < v.size()` |
| `v.at(i)` | 带边界检查的访问 | 越界抛出 `std::out_of_range` |
| `v.front()` | 第一个元素 | 容器非空 |
| `v.back()` | 最后一个元素 | 容器非空 |
| `v.data()` | 底层元素存储的指针 | 只能访问实际存在的元素 |

```cpp
std::vector<int> v{10, 20, 30};
v[1] = 99;                  // {10, 99, 30}
int first = v.front();       // 10
int last = v.back();         // 30
int* p = v.data();
p[0] = 11;                  // 等价于修改 v[0]
```

在 C++20 中，`operator[]` 越界属于未定义行为，不能依赖它抛出异常或自动扩容。调试库可能提供额外检查，但这不是该接口的可移植保证。

```cpp
#include <iostream>
#include <stdexcept>
#include <vector>

// 放在函数体内
std::vector<int> v{10, 20};
try {
    std::cout << v.at(2);
} catch (const std::out_of_range&) {
    std::cout << "下标越界\n";
}
```

空容器的 `data()` 可能为空指针，也可能不是；不能用 `data() == nullptr` 判断是否为空，应使用 `empty()`。

## 4. 遍历与迭代器

### 4.1 下标遍历

```cpp
for (std::size_t i = 0; i < v.size(); ++i) {
    std::cout << v[i] << ' ';
}
```

`size()` 返回无符号的 `size_type`。避免写 `i <= v.size() - 1`：空容器的 `size() - 1` 会发生无符号回绕。需要有符号长度时，可在 C++20 中使用 `<iterator>` 中的 `std::ssize(v)`。

### 4.2 范围 for

```cpp
for (auto x : v) {           // 复制每个元素，修改 x 不影响 v
    std::cout << x << ' ';
}

for (auto& x : v) {          // 引用实际元素，可以修改
    x *= 2;
}

for (const auto& x : v) {    // 只读访问，避免复制元素
    std::cout << x << ' ';
}
```

### 4.3 显式迭代器与反向遍历

```cpp
for (auto it = v.begin(); it != v.end(); ++it) {
    std::cout << *it << ' ';
}

for (auto it = v.cbegin(); it != v.cend(); ++it) {
    // *it = 0; // 错误：const_iterator 不允许修改元素
}

for (auto it = v.rbegin(); it != v.rend(); ++it) {
    std::cout << *it << ' ';
}
```

`begin()` 指向第一个元素，`end()` 表示最后一个元素之后的位置，**不能解引用 `end()`**。空容器满足 `begin() == end()`。

`vector` 的迭代器支持 `it + n`、`it - n`、`it2 - it1` 等随机访问操作，但不能据此假定其实际类型就是裸指针。迭代器运算必须遵守同一容器及有效范围的要求。

`const std::vector<int>` 的 `begin()` 返回只读迭代器；`const auto it = v.begin()` 则只是禁止修改迭代器变量本身，不会使非 const 容器中的元素变为只读。

## 5. size、capacity、reserve 与 resize

这组概念决定“有多少对象”与“预留了多少空间”的区别。

| 接口 | 含义 | 是否改变元素数量 |
| --- | --- | --- |
| `size()` | 当前已构造的元素数量 | 否 |
| `capacity()` | 不重新分配空间时可容纳的元素数量 | 否 |
| `empty()` | 是否没有元素 | 否 |
| `reserve(n)` | 容量不足时预留至少 `n` 个元素的空间 | 否 |
| `resize(n)` | 将元素数量调整到 `n` | 可能 |
| `resize(n, value)` | 增大时以 `value` 初始化新增元素 | 可能 |
| `shrink_to_fit()` | 请求减少多余容量 | 否 |

始终满足 `size() <= capacity()`。容量的单位是“元素个数”，不是字节数。

```text
size = 3，capacity = 6

索引：    0      1      2      3      4      5
       [ 10 ] [ 20 ] [ 30 ] [预留] [预留] [预留]
        已经构造的元素         尚无元素对象
```

### 5.1 reserve 预留空间

```cpp
std::vector<int> v;
v.reserve(10);
// size() == 0，capacity() >= 10

// v[0] = 1; // 错误：没有第 0 个元素
v.push_back(1); // 正确：构造一个元素，size() 变为 1
```

当 `n <= capacity()` 时，`reserve(n)` 不会缩容；当 `n > capacity()` 时会重新分配。可提前估计最终数量时，一次预留空间能够减少搬迁次数。

### 5.2 resize 改变元素数量

```cpp
std::vector<int> v{1, 2, 3};
v.resize(5);      // {1, 2, 3, 0, 0}
v.resize(7, 9);   // {1, 2, 3, 0, 0, 9, 9}
v.resize(2);      // {1, 2}，被移除元素会析构，容量不缩小
```

对默认分配器下的 `int`，新增元素默认初始化为 0；对类类型，`resize(n)` 的增长路径需要能够默认构造元素。

### 5.3 清空与收缩

```cpp
v.clear();         // 销毁所有元素，size() 为 0，capacity() 保持不变
v.shrink_to_fit(); // 请求缩小容量，标准不保证一定执行
```

`clear()` 适合清空后继续复用存储空间。普通默认分配器场景下，也可用 `std::vector<int>{}.swap(v);` 让临时容器接管旧存储并在语句结束时释放；不应泛化到不兼容的自定义分配器。

容量与收缩的准确规则可查阅 [标准草案：vector.capacity](https://eel.is/c++draft/vector.capacity)。

## 6. 插入、删除与赋值

### 6.1 尾部操作

```cpp
std::vector<int> v{1, 2};
v.push_back(3);     // {1, 2, 3}
v.emplace_back(4);  // {1, 2, 3, 4}
v.pop_back();       // {1, 2, 3}
```

`pop_back()` 不返回被删除的元素；如需保留它，应先读取或移动 `back()`。调用 `pop_back()` 前必须保证非空。

### 6.2 指定位置插入

```cpp
std::vector<int> v{10, 30};
auto it = v.insert(v.begin() + 1, 20); // {10, 20, 30}
// 返回指向新插入元素的迭代器，*it == 20

v.insert(v.end(), 2, 40);             // {10, 20, 30, 40, 40}

std::vector<int> extra{50, 60};
v.insert(v.end(), extra.begin(), extra.end());
// {10, 20, 30, 40, 40, 50, 60}
```

插入位置可以是 `end()`，表示尾部插入。C++20 中，不要把同一个 `vector` 的迭代器区间直接作为该容器区间插入的来源；需要复制自身的一段时，先保存到临时容器。

### 6.3 删除元素

```cpp
std::vector<int> v{10, 20, 30, 40, 50};
auto next = v.erase(v.begin() + 1); // {10, 30, 40, 50}，*next == 30
v.erase(v.begin() + 1, v.begin() + 3); // {10, 50}，删除半开区间
v.clear();                           // {}
```

`erase` 返回删除区间之后的位置；如果删除到了尾部，则返回新的 `end()`。单元素版本不能传入 `end()`，空区间 `erase(end(), end())` 则合法。

### 6.4 赋值与交换

```cpp
std::vector<int> a{1, 2, 3};
std::vector<int> b;
b = a;                // 复制赋值
b.assign(4, 9);       // 用 {9, 9, 9, 9} 替换原内容
b.assign({7, 8});     // 用 {7, 8} 替换原内容
a.swap(b);            // 交换两容器内容
```

不要在 `assign` 或覆盖内容的赋值后继续使用旧元素的迭代器、指针和引用。

## 7. 扩容与对象生命周期

当插入后的元素数量超过当前容量时，`vector` 需要取得更大的连续存储空间。典型过程如下：

1. 分配新的存储空间。
2. 在新空间中构造新增元素，并移动或复制原有元素，具体构造顺序由实现决定。
3. 成功后销毁旧空间中的对象，释放旧存储空间。

标准没有规定必须按 1.5 倍或 2 倍增长，也没有要求 `reserve(n)` 后容量恰好等于 `n`。不要让程序依赖某个编译器观察到的容量序列。

常见实现采用几何增长来支持尾部追加的均摊常数复杂度。单次扩容仍可能搬迁全部原元素，因此一次 `push_back()` 可能是线性开销。

### 为什么不能简单地使用 memcpy？

`vector<std::string>` 的元素是管理资源的对象。直接复制对象字节可能让多个对象错误地持有同一资源。通用容器必须遵守元素类型的构造、赋值与析构语义；实现可以对满足条件的类型采用符合标准的优化。

若自己实现一个教学版 vector，需要区分“原始存储空间”和“已构造元素”。直接使用 `new T[capacity]` 会构造全部容量对应的对象，不能准确模拟标准 `vector` 对未使用空间的管理，也会额外要求 `T` 可默认构造。

### 异常与 noexcept

分配空间可能失败，元素的构造和搬迁也可能抛出异常。扩容时，如果元素移动可能抛异常且允许复制，实现常通过复制旧元素来保留失败回滚能力。正确声明不抛异常的移动构造函数为 `noexcept`，有助于容器安全地使用移动。

不要认为所有操作都无条件具有“失败后完全不变”的保证：对于只能移动且移动构造会抛异常的类型，部分扩容操作的保证会减弱。只在确实不会抛异常时声明 `noexcept`。

## 8. 迭代器、指针和引用失效

失效意味着旧的访问位置不能再按原含义使用。地址看似仍可读，不代表使用它合法。

| 操作 | 失效规则 |
| --- | --- |
| 任何导致重新分配的操作 | 所有元素的迭代器、指针、引用及旧 `end()` 失效 |
| 尾部插入且未重新分配 | 原有元素的访问位置仍有效，旧 `end()` 失效 |
| 中间插入且未重新分配 | 插入位置之前仍有效，插入位置及之后（含旧 `end()`）失效 |
| 删除非空区间 | 删除起点及之后的位置（含旧 `end()`）失效 |
| `pop_back()` | 被删除元素及旧 `end()` 失效，之前的元素不受影响 |
| `clear()` | 所有元素访问位置及旧 `end()` 失效 |
| `reserve()` / `shrink_to_fit()` | 发生重新分配则全部失效，否则保持有效 |
| `resize()` | 重新分配则全部失效；缩小会使被移除元素和旧 `end()` 失效；无扩容增长只使旧 `end()` 失效 |

表中的插入、删除规则见 [标准草案：vector.modifiers](https://eel.is/c++draft/vector.modifiers)。

默认分配器下的 `swap()` 不搬迁元素，已有元素引用和迭代器会随元素归属于另一个容器；不要依赖旧的尾后迭代器。

### 8.1 扩容后重新取得访问位置

```cpp
std::vector<int> v{1, 2, 3};
std::size_t index = 1;
v.push_back(4);                  // 可能扩容
std::cout << v[index] << '\n';   // 通过保存的下标重新访问
```

下标有助于跨越纯尾部追加，但中间插入、删除会改变元素的下标，不能把下标当作永久标识。

### 8.2 边遍历边删除

```cpp
std::vector<int> v{1, 2, 3, 4, 5, 6};
for (auto it = v.begin(); it != v.end();) {
    if (*it % 2 == 0) {
        it = v.erase(it);  // 接住返回的新位置，不再递增
    } else {
        ++it;
    }
}
// v 为 {1, 3, 5}
```

这段写法正确，但连续多次中间删除可能产生平方级开销。批量筛选通常使用下一节的算法。

不要在范围 for 中对同一 `vector` 进行可能改变结构的插入或删除。范围 for 保存了遍历位置和结束位置，即使预留足够容量，尾部插入也会使保存的旧 `end()` 失效。

## 9. 配合标准算法

### 9.1 排序、查找与反转

```cpp
#include <algorithm>
#include <functional>
#include <vector>

// 放在函数体内
std::vector<int> v{3, 1, 4, 1, 5};
std::sort(v.begin(), v.end());   // {1, 1, 3, 4, 5}

auto it = std::find(v.begin(), v.end(), 3);
if (it != v.end()) {
    *it = 30;
}

std::sort(v.begin(), v.end(), std::greater<int>{}); // 降序
std::reverse(v.begin(), v.end());                 // 反转现有顺序
```

`find` 失败返回 `end()`，必须先判断再解引用。`lower_bound` 和 `binary_search` 等二分算法要求区间满足相应有序或分区条件；通常先按相同比较规则排序。

### 9.2 erase-remove 惯用法

```cpp
std::vector<int> v{1, 2, 2, 3, 2, 4};
auto newEnd = std::remove(v.begin(), v.end(), 2);
v.erase(newEnd, v.end()); // {1, 3, 4}
```

`std::remove` 把需要保留的元素整理到前面，返回新的逻辑结尾，**不会改变容器的 `size()`**。逻辑结尾之后的元素仍存在，但值不应再作为筛选结果使用。最后调用容器的 `erase` 才真正删除尾部元素。

C++20 可以直接写：

```cpp
std::erase(v, 2); // 删除所有等于 2 的元素
std::erase_if(v, [](int x) { return x % 2 == 0; }); // 删除所有偶数
```

这两个函数由 `<vector>` 提供，批量删除为线性复杂度。

### 9.3 排序后去重

```cpp
std::vector<int> v{3, 1, 3, 2, 1};
std::sort(v.begin(), v.end());
v.erase(std::unique(v.begin(), v.end()), v.end());
// {1, 2, 3}
```

`unique` 只合并相邻的相等元素，所以去除所有重复值时通常先排序；这种做法会改变原始顺序。

## 10. 自定义类型、复制与移动

### 10.1 push_back 与 emplace_back

```cpp
#include <string>
#include <utility>
#include <vector>

struct Student {
    std::string name;
    int age;

    Student(std::string n, int a) : name(std::move(n)), age(a) {}
};

// 放在函数体内
std::vector<Student> students;
students.reserve(2);
students.push_back(Student{"张三", 18});
students.emplace_back("李四", 19);
```

`push_back` 接收一个元素对象，根据实参进行复制或移动；`emplace_back` 将参数转发给元素构造函数，直接构造新增元素。`emplace_back` 可能减少临时对象，但不保证每次都更快，也不能避免容量不足时搬迁旧元素。

这个 `Student` 没有默认构造函数，因此可以通过上述方式插入，但不能依赖 `resize(n)` 默认构造新增学生；可使用 `resize(n, Student{"待填写", 0})`。

### 10.2 容器复制与移动

```cpp
std::vector<std::string> a{"hello", "world"};
auto b = a;                 // 复制各元素，b 与 a 独立
b[0] = "C++";               // 不改变 a[0]
auto c = std::move(a);      // 普通移动构造，通常接管底层存储
```

`std::move` 本身只做类型转换，实际移动由对应构造或赋值操作完成。移动后的源容器仍处于有效但未指定的状态，可以销毁、重新赋值或调用满足前置条件的操作；不要假定它必然为空。带显式分配器的移动构造，以及分配器不兼容的移动赋值，可能需要逐元素移动。

### 10.3 传参和资源所有权

只读处理通常用 `const std::vector<T>&`，原地修改用 `std::vector<T>&`；需要独立副本或接收所有权时再按值传递。

`vector<T*>` 只管理指针元素，不会自动 `delete` 指针指向的对象。需要独占拥有动态对象时，可以使用 `<memory>` 中的智能指针：

```cpp
std::vector<std::unique_ptr<Student>> students;
students.push_back(std::make_unique<Student>("王五", 20));
```

元素离开容器时，`unique_ptr` 会释放所拥有的对象。该容器可以移动，但不能像普通可复制元素的容器那样复制。

## 11. 二维 vector 与特殊类型

### 11.1 二维容器的结构

`std::vector<std::vector<int>>` 不是“二维数组”，而是**元素类型为 `vector<int>` 的一维容器**。用一个只保留核心成员的简化模型就能看清它的形状：

```cpp
template<class T>
class vector
{
private:
    T* _a;                  // 指向一串 T
    std::size_t _size;
    std::size_t _capacity;
};
```

`T` 是类型占位符，模板本身不产生任何代码。写下 `std::vector<int> v;` 时，编译器才把 `T` 替换为 `int`，实例化出一个真正的类；写成 `std::vector<std::vector<int>>` 时，`T` 被替换为 `vector<int>`，得到的是另一个同样独立的类：

| 模板成员 | `vector<int>` | `vector<vector<int>>` |
| --- | --- | --- |
| `T` 实际是 | `int` | `vector<int>` |
| `T* _a` 实际是 | `int*` | `vector<int>*` |
| 一个元素是什么 | 一个 `int` | 一个完整的 `vector<int>` 对象 |
| 单个元素大小 | 4 字节 | 24 字节（64 位 libstdc++） |
| `_a` 那块的存储 | 一串 `int` | 一串“行对象” |

关键在 `T* _a` 的语义始终是“指向一串 `T`”，**它本身永远是一维的**。当 `T` 换成 `vector<int>`，它指向的是内层容器对象，而不是 `int`。`sizeof(vector<int>)` 就是三个机器字（64 位 libstdc++ 为 24 字节，MSVC 用 4 个指针共 32 字节），因此外层的 `_a` 中每隔 24 字节就是一个完整的行对象。

两个版本的布局差别：

```text
vector<int> v
   [ _a ][ _size ][ _capacity ] ──→ [ 10 ][ 20 ][ 30 ]      一整块连续内存

vector<vector<int>> m
   [ _a ][ _size ][ _capacity ] ──→ [ 行0 ][ 行1 ][ 行2 ]   外层连续
                                        │      │      │
                                        ↓      ↓      ↓
                                    [1 2 3][4 5 6][7 8 9]  三块独立内存
```

由此可以确定以下几点：

- 外层连续存放的是各个内层 `vector` 对象，每一行分别管理自己的元素存储。
- **各行的整数不保证处于同一块连续空间**，行的长度也可以不同。
- 访问前必须分别满足行下标和列下标的边界。

`matrix[1][2]` 等价于 `matrix._a[1]._a[2]`：第一次按 24 字节步长在外层数组中定位到第 1 个行对象，第二次读该行对象自己的 `_a` 再偏移两个元素。这是两次解引用、两个互不相关的堆块。

顺便说明“特化”与“实例化”的区别：若要把上表中的某个结果手写进源码，必须使用全特化语法 `template<> class vector<int> { ... };`，那是放弃模板、另写一份实现，而不是实例化。标准库对 `bool` 正是这样处理的（见 11.9）。

### 11.2 常用创建方式

```cpp
// 1. 空容器，逐行追加：行数由运行期决定
std::vector<std::vector<int>> a;
a.push_back({1, 2, 3});

// 2. 一次建成 rows × cols，全部为 0
std::vector<std::vector<int>> b(3, std::vector<int>(4, 0));

// 3. 先建 rows 个空行，再逐行确定长度
std::vector<std::vector<int>> c(3);
c[0].assign(2, 0);
c[1].assign(5, 1);

// 4. 初始化列表：各行长度可以不同
std::vector<std::vector<int>> d{{1, 2}, {3}, {4, 5, 6}};

// 5. 把已有容器重置为固定形状
d.assign(3, std::vector<int>(4, 0));
```

| 写法 | 结果 | 适用场景 |
| --- | --- | --- |
| 空容器 + `push_back` | 0 行，逐行增加 | 行数运行时才知道 |
| `m(rows, vector<int>(cols, init))` | `rows × cols` 的规则矩阵 | 尺寸已知 |
| `m(rows)` 再逐行 `assign` / `resize` | `rows` 个空行 | 各行长度不同 |
| 初始化列表 | 行长度任意 | 字面量、小数据 |
| `assign(rows, vector<int>(cols, init))` | 重置为固定形状 | 复用已有变量 |

第二行写法有两处容易误解：

- 第二个参数是**按值复制**的模板行。外层存放的是 `rows` 个各自独立的 `vector<int>`，改 `b[0][0]` 不影响 `b[1][0]`。
- 复制 `rows` 行大约需要 `rows + 1` 次堆分配（模板行一次，每行各一次；具体次数取决于实现），行数很大时这部分开销不可忽略。

`m(3)` 与 `m(3, std::vector<int>(4))` 差别极大：前者是 3 个**空行**，`m[0][0]` 直接越界；后者才是 3 行 4 列。

### 11.3 尺寸与访问

```cpp
std::vector<std::vector<int>> m(3, std::vector<int>(4, 0));
std::size_t rows = m.size();     // 行数：3
std::size_t cols = m[0].size();  // 4，仅当所有行等长时才代表“列数”
```

二维 `vector` 没有统一的列数接口，规则矩阵要靠约定（通常取 `m[0].size()`），不规则矩阵只能按行取 `m[i].size()`。

- `m[i][j]` 两层都不检查边界，任一层越界都是未定义行为。
- `m.at(i)` 只检查行下标；要两层都检查必须写 `m.at(i).at(j)`。
- 空容器 `m[0]` 已经越界，不能用 `m[0].empty()` 判断“有没有列”，应先判断 `m.empty()`。
- 行下标合法而列下标超出行长度时，读到的是该行缓冲区之外的位置，不会自动补 0。

### 11.4 遍历

```cpp
// 下标遍历：内层条件必须用 m[i].size()
for (std::size_t i = 0; i < m.size(); ++i) {
    for (std::size_t j = 0; j < m[i].size(); ++j) {
        std::cout << m[i][j] << ' ';
    }
    std::cout << '\n';
}

// 只读：引用绑定每一行，不复制
for (const auto& row : m) {
    for (int value : row) {
        std::cout << value << ' ';
    }
}

// 修改元素：两层都用引用
for (auto& row : m) {
    for (auto& value : row) {
        value *= 2;
    }
}
```

`for (auto row : m)` 中的 `row` 是 `vector<int>` 的副本，每次迭代会复制一整行元素。只读时应写 `const auto&`。

### 11.5 按行与按列扩充

```cpp
std::vector<std::vector<int>> m;

m.push_back({1, 2, 3});   // 追加一整行
m.emplace_back(4, 0);     // 原地构造一行 4 个 0
m[0].push_back(9);        // 只让第 0 行变长，矩阵从此不规则

std::vector<std::vector<int>> grid;
grid.reserve(100);        // 只预留外层（行对象），不预留行内元素
```

- `reserve` 在二维场景只作用于外层；要为行内元素预留，需对每行单独 `reserve`。
- 外层 `push_back` 扩容时搬迁的是行对象，代价与**行数**成正比，而不是与元素总数成正比。
- 参数直接决定构造出什么：`emplace_back()` 追加一个空行，`emplace_back(4)` 追加的是“4 个值为 0 的元素”，不是“一个值为 4 的元素”。
- 逐行 `push_back` 会让各行容量互不相同；后续按列访问前应先确认所有行的长度。

### 11.6 内存布局与性能

二维 `vector` 的每次访问要经过两级间接寻址，跨行时缓存局部性明显弱于连续内存。若数据是规则的数值矩阵，可以用一维 `vector` 扁平化：

```cpp
std::size_t rows = 3, cols = 4;
std::vector<int> flat(rows * cols, 0);

auto at = [&](std::size_t r, std::size_t c) -> int& {
    return flat[r * cols + c];
};

at(1, 2) = 7;
```

- 下标用 `std::size_t` 计算，并先校验 `r < rows && c < cols`，避免乘法溢出和越界。
- 扁平化版本整体连续，`flat.data()` 可以直接交给要求连续缓冲区的接口；代价是没有 `flat[r][c]` 语法，需要自己封装访问函数。
- 行对象本身也有成本：`vector<vector<char>>` 若存 100 万个长度为 1 的行，每个行对象 24 字节，仅外层就约 24 MB，而数据只有 1 MB。
- C++23 的 `std::mdspan` 能为连续存储提供多维视图，本文以 C++20 为基准，不展开。

### 11.7 二维场景下的失效规则

第 8 章的表针对单个容器；二维时有两层，需要分开看：

| 操作 | 失效范围 |
| --- | --- |
| 外层 `push_back` / `insert` 且发生扩容 | 所有行对象的地址、外层迭代器、已取得的 `m[i]` 引用失效 |
| 外层尾部插入但未扩容 | 原有行对象仍有效，旧外层 `end()` 失效 |
| 外层中间插入且未扩容 | 插入位置之前的行有效，其后的行对象和旧 `end()` 失效 |
| `m[i].push_back(...)` | 仅该行的元素访问位置和该行旧 `end()` 失效，其他行不受影响 |
| `m[i].clear()` | 该行元素失效，行对象本身仍在 |
| `m.clear()` | 全部行对象析构，外层容量不变 |

```cpp
std::vector<std::vector<int>> m{{1, 2}, {3, 4}};
const int& first = m[0][0];
m.push_back({5, 6});   // 外层可能扩容，m[0] 这个行对象被搬到新位置
// first 通常仍指向原来那 2 个 int（行对象是被移动构造过去的），
// 但这是实现层面的结果，不应依赖；需要时重新写 m[0][0]
```

- 外层扩容搬的是行对象；行对象被移动构造时通常直接接管原有缓冲区，因此元素地址往往不变。但标准不保证这一点，稳妥做法是重新通过 `m[i]` 获取。
- `&m[0][0]` 只能当作第一行的起始地址，加偏移无法跨到其他行。

### 11.8 典型用法

**邻接表**：天然支持每行度数不同，这是它相对矩形数组的主要优势。

```cpp
int n = 5;
std::vector<std::vector<int>> adj(n);   // n 个空行
adj[0].push_back(1);
adj[1].push_back(0);
```

**字符网格**：

```cpp
std::vector<std::string> grid{"....", ".##.", "...."};
grid[1][2] = '.';
```

用 `vector<string>` 存字符网格时，外层连续存放的是 `std::string` 对象；短字符串可能直接存在对象内部（SSO），长字符串才单独分配，因此同样不能假定整体连续。

**矩阵转置**：先按列数建好目标，再逐元素填写。

```cpp
std::vector<std::vector<int>> transpose(const std::vector<std::vector<int>>& m)
{
    if (m.empty()) {
        return {};
    }
    std::vector<std::vector<int>> t(m[0].size(), std::vector<int>(m.size()));
    for (std::size_t i = 0; i < m.size(); ++i) {
        for (std::size_t j = 0; j < m[i].size(); ++j) {
            t[j][i] = m[i][j];
        }
    }
    return t;
}
```

上面的实现假定矩阵规则；对不规则矩阵需要先按最大列数确定尺寸，或改为逐行追加。

**动态规划表**：建议一次成型（`m(rows, vector<int>(cols, init))`），而不是逐行 `push_back`，避免外层反复扩容。

三种存法的对比：

| 方式 | 内存连续 | 行长可变 | 随机访问 | 传参 |
| --- | --- | --- | --- | --- |
| `int a[3][4]` | 是 | 否 | `a[i][j]`，一次地址计算 | 退化为指针，需另传列数 |
| `vector<int>` 扁平化 | 是 | 否 | 需 `r * cols + c` | `const vector<int>&` |
| `vector<vector<int>>` | 外层连续，行内分散 | 是 | `m[i][j]`，两次解引用 | `const vector<vector<int>>&` |

### 11.9 vector<bool>

`std::vector<bool>` 是特殊化版本，通常按位压缩保存布尔值，其元素访问返回代理对象，不能按普通 `bool&` 理解，也不能假定提供连续的 `bool` 数组。

需要普通可寻址的标记元素时，可以考虑 `std::vector<unsigned char>`。编写通用代码时，不要假设所有 `vector<T>` 的元素引用一定是 `T&`。

## 12. 复杂度与使用建议

下表按元素基本操作视为常数开销分析，`n` 为当前元素数量，`k` 为位置下标，`m` 为插入数量。

| 操作 | 时间复杂度 |
| --- | --- |
| 下标访问、`at`、`front`、`back` | O(1) |
| `size`、`capacity`、`empty` | O(1) |
| 尾部追加一个元素 | 均摊 O(1)，扩容时 O(n) |
| `pop_back` | O(1) |
| 在位置 `k` 插入 `m` 个元素 | 无扩容时 O(m + n - k)，扩容时 O(n + m) |
| 删除位置 `k` 的一个元素 | O(n - k) |
| 遍历、复制、清空 | O(n) |
| 遍历 `r × c` 的规则二维矩阵 | O(r × c) 次两级寻址 |
| `reserve` | 不重新分配时 O(1)，重新分配时 O(n) |
| `std::sort` | O(n log n) 次比较 |

尾部追加的均摊复杂度和连续存储要求可查阅 [标准草案：vector.overview](https://eel.is/c++draft/vector.overview)。

实际使用时：

- 需要频繁按下标访问和尾部追加时，`vector` 通常很合适。
- 已知大致数量时，先 `reserve`，然后追加；需要直接按下标填写时，先 `resize` 或按数量构造。
- 不要在每次追加前调用 `reserve(size() + 1)`，这可能破坏库原有增长策略，导致反复搬迁。
- 频繁头部插入、删除时，可考虑 `std::deque`，并结合访问方式选择容器。
- 需要跨插入、删除长期标识对象时，设计稳定 ID，或选择符合稳定性要求的存储方式。

## 13. 完整示例

下面是可以独立保存并编译的程序；当前目录中的 `main.cpp` 仍是项目初始演示程序，未包含这些操作。

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

void print(const std::vector<int>& values)
{
    for (int value : values) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}

int main()
{
    std::vector<int> values;
    values.reserve(8);

    for (int value : {5, 2, 4, 2, 1, 3}) {
        values.push_back(value);
    }
    print(values);

    values.insert(values.begin() + 1, 9);
    print(values);

    std::erase_if(values, [](int value) { return value % 2 == 0; });
    print(values);

    std::sort(values.begin(), values.end());
    print(values);

    auto it = std::find(values.begin(), values.end(), 3);
    if (it != values.end()) {
        std::cout << "index of 3: " << (it - values.begin()) << '\n';
    }

    values.resize(6, 0);
    print(values);
    std::cout << "size: " << values.size() << '\n';
}
```

预期输出（每行数字后有一个空格）：

```text
5 2 4 2 1 3
5 9 2 4 2 1 3
5 9 1 3
1 3 5 9
index of 3: 1
1 3 5 9 0 0
size: 6
```

如果将完整示例保存为 `vector_demo.cpp`，可以在该文件所在目录执行：

```powershell
g++ -std=c++20 -Wall -Wextra -pedantic vector_demo.cpp -o vector_demo.exe
.\vector_demo.exe
```

需要使用支持 C++20 及其标准库接口的编译器。本机 PATH 中的 MinGW GCC 8.1 较旧；本文完整示例已使用项目 CLion 工具链中的 GCC 13.1 按 C++20 编译并核对输出。若终端提示不识别 `-std=c++20`，应切换至该工具链，而不是仅修改标准选项。

## 14. 常见误区与自测

| 误区 | 正确理解 |
| --- | --- |
| `reserve(10)` 后就有 10 个元素 | 只预留空间，`size()` 不变 |
| `v[v.size()]` 可以用来追加 | 该下标越界，追加使用 `push_back` 等接口 |
| `clear()` 会释放全部容量 | 元素被销毁，容量保持不变 |
| `shrink_to_fit()` 一定精确缩容 | 它是非强制请求 |
| `push_back()` 永远是 O(1) | 均摊 O(1)，单次扩容可能 O(n) |
| 未扩容就没有迭代器失效 | 插入、删除也可能使部分位置和旧 `end()` 失效 |
| `erase(it)` 后继续 `++it` | 应接住 `erase` 的返回值 |
| `std::remove()` 会减少 `size()` | 还需要调用容器的 `erase` |
| 容量增长倍数固定 | 标准不规定具体倍数 |
| `vector` 自动释放所有指针指向的对象 | 裸指针元素不表达对象所有权 |
| `vector<vector<int>>` 是连续的二维数组 | 只有外层的行对象连续，每行数据各自分配 |
| `m[0].size()` 一定是列数 | 行长度可以不同，需按行取 `size()` |
| `m(3)` 就已经是 3 行可用的矩阵 | 它是 3 个空行，`m[0][0]` 越界 |
| `&m[0][0] + r * cols + c` 可定位任意元素 | 行与行不连续，必须经外层数组跳转 |

自测问题：

1. `vector<int> v(5)` 与 `vector<int> v{5}` 的元素分别是什么？
2. `reserve(100)` 后，能否直接写 `v[99]`？
3. 为什么 `push_back` 没有扩容，旧 `end()` 仍然失效？
4. 中间删除一个元素后，删除位置之前与之后的迭代器分别怎样变化？
5. 为什么批量删除偶数时，`std::erase_if` 通常比循环逐个 `erase` 更合适？
6. `vector<vector<int>> m(3)` 与 `vector<vector<int>> m(3, vector<int>(4))` 的元素分别是什么？
7. 为什么不能用 `&m[0][0] + r * cols + c` 访问第 `r` 行第 `c` 列？
8. `m.emplace_back(4)` 追加的是什么？

参考答案：

1. 前者有 5 个值为 0 的元素，后者只有一个值为 5 的元素。
2. 不能，仅靠预留容量没有创建元素；需先建立足够的 `size()`。
3. 尾后位置发生改变，旧 `end()` 不再是合法的旧遍历终点。
4. 之前仍有效，删除位置及之后（含旧尾后位置）失效。
5. 前者可一次线性整理并删除尾部，后者可能反复移动后续元素。
6. 前者是 3 个空行（`m[0].size()` 为 0，访问 `m[0][0]` 越界）；后者是 3 行 × 4 列、全部为 0 的矩阵。
7. 每行数据是独立分配的堆块，行与行之间没有地址连续性；必须先由外层 `_a` 取到行对象，再由该对象的 `_a` 定位元素。
8. 一行 4 个值为 0 的元素（等价于构造 `vector<int>(4)`），而不是一个值为 4 的元素。

## 15. 参考资料

- [C++ 标准草案：vector 总览](https://eel.is/c++draft/vector.overview)
- [C++ 标准草案：容量管理](https://eel.is/c++draft/vector.capacity)
- [C++ 标准草案：插入与删除](https://eel.is/c++draft/vector.modifiers)

以上链接指向持续更新的工作草案，其中可能出现 C++20 之后新增的接口；本文仅使用 C++20 及之前的接口。
