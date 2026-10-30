# STL 与 std::string

**容器保存数据，迭代器访问数据，算法处理数据。** 本文以 C++20 为准，示例为代码片段，省略 `main()` 和部分头文件。

## 目录

1. [STL 与 C++ 标准库](#1-stl-与-c-标准库)
2. [STL 的六大组件](#2-stl-的六大组件)
3. [容器分类与选择](#3-容器分类与选择)
4. [迭代器与区间](#4-迭代器与区间)
5. [迭代器失效](#5-迭代器失效)
6. [string 的作用与构造](#6-string-的作用与构造)
7. [string 的大小与容量](#7-string-的大小与容量)
8. [string 的访问与遍历](#8-string-的访问与遍历)
9. [string 的修改、查找与比较](#9-string-的修改查找与比较)
10. [字符串输入与数值转换](#10-字符串输入与数值转换)
11. [C 风格接口与 string_view](#11-c-风格接口与-string_view)
12. [常用容器示例](#12-常用容器示例)
13. [算法、函数对象与 Lambda](#13-算法函数对象与-lambda)
14. [分配器与资源管理](#14-分配器与资源管理)
15. [auto 与类型推导](#15-auto-与类型推导)

## 1. STL 与 C++ 标准库

STL（Standard Template Library，标准模板库）通过模板提供通用的数据结构和算法。C++ 标准库还包括输入输出、线程等设施，范围比 STL 更大。`std::string` 是标准库的字符串类，常与 STL 一起学习。

```cpp
std::vector<int> values{3, 1, 4, 1, 5};
std::sort(values.begin(), values.end()); // 1 1 3 4 5
```

使用时包含对应头文件，如 `<vector>`、`<string>`、`<algorithm>`，并使用 `std::` 限定名称。

## 2. STL 的六大组件

| 组件 | 作用 | 例子 |
| --- | --- | --- |
| 容器 | 保存和管理元素 | `vector`、`list`、`map` |
| 算法 | 查找、排序、变换等 | `find`、`sort`、`copy` |
| 迭代器 | 表示位置，连接容器与算法 | `begin()`、`end()` 返回的对象 |
| 函数对象 | 用可调用对象定制行为 | `std::less<int>`、重载 `operator()` 的对象 |
| 适配器 | 包装已有组件，改变接口 | `stack`、`queue`、`back_inserter` |
| 分配器 | 管理容器所需的存储空间 | `std::allocator<T>` |

## 3. 容器分类与选择

### 3.1 顺序容器

“顺序”指按线性位置组织，不表示元素按大小排好序。下表中的 `n` 为元素数量。

| 容器 | 特点 | 常见操作复杂度 |
| --- | --- | --- |
| `array<T, N>` | 固定长度、连续存储 | 随机访问 O(1) |
| `vector<T>` | 动态数组，通常优先考虑 | 随机访问 O(1)，尾插均摊 O(1)，中间插删 O(n) |
| `deque<T>` | 双端增长，整体不保证连续 | 随机访问 O(1)，单元素头尾插入 O(1) |
| `list<T>` | 双向链表，不支持下标 | 已知位置的单元素插删 O(1)，查找 O(n) |
| `forward_list<T>` | 单向链表 | 已知前驱后的单元素插删 O(1) |

链表插删的 O(1) 不包含寻找位置的时间。

### 3.2 关联容器

| 容器 | 保存内容 | 查找复杂度 |
| --- | --- | --- |
| `set` / `multiset` | 按比较器排序的键 | O(log n) |
| `map` / `multimap` | 按键排序的键值对 | O(log n) |
| `unordered_set` / `unordered_multiset` | 哈希组织的键 | 平均 O(1)，最坏 O(n) |
| `unordered_map` / `unordered_multimap` | 哈希组织的键值对 | 平均 O(1)，最坏 O(n) |

- 带 `multi` 的版本允许等价键重复；其他版本不允许。
- 有序容器通常用红黑树实现，无序容器不保证遍历顺序。
- 有序容器按比较器判断键等价；无序容器的等价键必须具有相同哈希值，反之不成立。

### 3.3 容器适配器

| 适配器 | 规则 | 常用接口 | 默认底层容器 |
| --- | --- | --- | --- |
| `stack<T>` | 后进先出 | `push`、`top`、`pop` | `deque<T>` |
| `queue<T>` | 先进先出 | `push`、`front`、`back`、`pop` | `deque<T>` |
| `priority_queue<T>` | 默认最大元素优先 | `push`、`top`、`pop` | `vector<T>` |

这些适配器不提供 `begin()` / `end()`。`pop()` 只移除元素，不返回元素值；读取或移除前应保证非空。

## 4. 迭代器与区间

### 4.1 左闭右开区间

STL 算法通常使用 `[first, last)`：包含起点，不包含终点。

- `begin()` 指向第一个元素，`end()` 指向尾后位置。
- 空容器满足 `begin() == end()`。
- **不能解引用 `end()`。** 迭代器类似指针，但不一定是原生指针。

### 4.2 迭代器类别

| 类别 | 能力 | 例子 |
| --- | --- | --- |
| 输入 / 输出 | 单趟读取 / 按约定写入 | 流迭代器、插入迭代器 |
| 前向 | 多趟向前遍历 | `forward_list`、无序容器 |
| 双向 | 还支持 `--` | `list`、`set`、`map` |
| 随机访问 | 还支持 `it + n`、位置差 | `vector`、`deque`、`string` |
| 连续（C++20） | 随机访问且元素连续存储 | 普通 `vector<T>`、`array`、`string` |

`vector<bool>` 是特殊版本，不适用普通 `vector<T>` 的连续存储结论。`std::sort` 要求可修改元素的随机访问迭代器，不能直接用于 `list`；链表应使用成员函数 `sort()`。

### 4.3 遍历方式

```cpp
std::string text = "abc";
for (auto it = text.begin(); it != text.end(); ++it) {
    *it = 'x'; // 修改原字符
}
for (auto it = text.cbegin(); it != text.cend(); ++it) {
    std::cout << *it; // 只读遍历：xxx
}
for (auto it = text.rbegin(); it != text.rend(); ++it) {
    std::cout << *it; // 反向遍历
}
```

`const_iterator` 限制通过迭代器修改元素；`const auto it` 限制的是迭代器自身移动。

### 4.4 反向迭代器

反向迭代器从后向前访问元素，使用区间 `[rbegin(), rend())`。

- `rbegin()` 指向最后一个元素；`rend()` 表示反向遍历的终点，不能解引用。
- 对反向迭代器执行 `++it`，会向容器开头移动；`--it` 则向容器末尾移动。
- `crbegin()` / `crend()` 用于只读反向遍历。
- 空容器的 `rbegin() == rend()`，不能解引用。

```cpp
std::vector<int> values{10, 20, 30};
for (auto it = values.rbegin(); it != values.rend(); ++it) {
    std::cout << *it << ' '; // 30 20 10
}
```

小知识：`it.base()` 返回对应的正向迭代器，它位于反向迭代器所指元素的**后一个位置**。因此 `rbegin().base() == end()`，`rend().base() == begin()`；不能直接把 `*it.base()` 当作当前反向元素。

### 4.5 迭代器的意义

- 用统一的方式遍历和修改不同容器。
- 将算法与容器底层结构解耦，让同一算法处理不同容器。

## 5. 迭代器失效

容器修改后，旧迭代器、指针或引用可能失效，不能继续用于访问元素。

| 操作 | 失效规则 |
| --- | --- |
| `vector` 插入导致重新分配 | 所有迭代器、元素指针和引用失效 |
| `vector` 插入但未重新分配 | 插入位置及之后的位置失效，包括旧 `end()` |
| `vector::erase` | 删除位置及之后的迭代器和引用失效，包括旧 `end()` |
| `list`、有序关联容器插入 | 已有元素的迭代器和引用保持有效 |
| `list`、有序关联容器删除 | 被删除元素的迭代器和引用失效 |
| 无序容器重新哈希 | 迭代器失效，已有元素的指针和引用保持有效 |
| `deque` 两端插入 | 迭代器失效，已有元素的引用保持有效 |
| `string` 修改 | 多种操作可能使迭代器、指针和引用失效，修改后按需重新获取 |

遍历删除时，接收 `erase` 返回的下一个有效迭代器：

```cpp
std::vector<int> values{1, 2, 3, 4, 5};
for (auto it = values.begin(); it != values.end();) {
    if (*it % 2 == 0) {
        it = values.erase(it);
    } else {
        ++it;
    }
} // 剩余 1 3 5
```

## 6. string 的作用与构造

`std::string` 定义于 `<string>`，自动管理连续存储的字符序列。

| 对比项 | C 风格字符串 | `std::string` |
| --- | --- | --- |
| 长度 | `strlen` 扫描到首个 `\0` | `size()` / `length()` 直接返回长度 |
| 容量 | 调用方管理缓冲区 | 对象按需管理 |
| 内容比较 | `strcmp` | `==`、`<` 等 |
| 拷贝 | 指针赋值只复制地址 | 拷贝字符串值 |
| 内嵌 `\0` | 通常被视为结束 | 可以保存并计入长度 |

```cpp
std::string empty;                         // 空串
std::string text = "hello";
std::string copy(text);                    // 拷贝构造
std::string repeated(3, 'x');               // "xxx"
std::string part(text, 1, 3);               // "ell"
std::string range(text.begin(), text.begin() + 2); // "he"
std::string binary("a\0b", 3);             // 长度为 3，保留内嵌 \0
```

`size()` 不计入末尾终止符，但计入内嵌 `\0`。构造多个相同字符时使用 `(数量, 字符)`，不要直接改成花括号，避免匹配不同的构造函数。

## 7. string 的大小与容量

| 接口 | 作用 |
| --- | --- |
| `size()` / `length()` | 当前字符数量 |
| `empty()` | 判断是否为空 |
| `capacity()` | 不重新分配时可容纳的字符数量 |
| `reserve(n)` | 预留容量，不改变长度 |
| `resize(n, ch)` | 改变长度，增长部分填充 `ch` |
| `resize(n)` | 改变长度，增长部分填充 `\0` |
| `clear()` | 清空字符，长度变为 0 |
| `shrink_to_fit()` | 请求缩容，不保证执行 |

```cpp
std::string text = "abc";
text.reserve(100);    // size() 仍为 3，capacity() 至少为 100
text.resize(5, 'x');  // "abcxx"
text.resize(2);       // "ab"
text.clear();         // 空串，不保证释放容量
```

**容量不等于已有元素数量。** `reserve(100)` 后不能随意写入下标 99；需要先 `resize` 或追加元素。

## 8. string 的访问与遍历

| 接口 | 说明 |
| --- | --- |
| `text[pos]` | 普通字符访问要求 `pos < size()`；不保证运行时边界检查，调试实现可能触发断言 |
| `text.at(pos)` | 越界抛出 `std::out_of_range` |
| `front()` / `back()` | 访问首尾字符，要求非空 |

部分标准库会在启用调试检查时对 `operator[]` 的下标进行断言，失败时终止程序；这不同于 `at()` 抛出可捕获的 `std::out_of_range`。C++20 中，`pos > size()` 属于未定义行为，不能依赖调试断言处理越界。

`text[text.size()]` 可读取末尾 `\0`，不能把该位置当作可写的追加位置。

```cpp
std::string text = "hello";
text[0] = 'H';
for (char ch : text) {
    std::cout << ch; // 按值读取：Hello
}
for (char& ch : text) {//使用for修改值要加引用
    ch = 'x';       // 引用原字符，结果为 "xxxxx"
}
```

- `size()` 返回无符号类型；反向遍历优先使用反向迭代器，避免 `i >= 0` 导致无符号回绕。
- 范围 for 也依赖迭代器，在循环中插删元素同样可能使其失效。
- 对 UTF-8 文本，`string::size()` 统计字节数，不能直接当作汉字数量。

## 9. string 的修改、查找与比较

### 9.1 修改与子串

| 接口 | 作用 |
| --- | --- |
| `push_back(ch)`、`+= ch` | 追加字符 |
| `+= str`、`append(str)` | 追加字符串 |
| `left + right` | 产生拼接结果 |
| `insert(pos, str)` | 在 pos 前插入 |
| `erase(pos, count)` | 从 pos 起删除至多 count 个字符 |
| `replace(pos, count, str)` | 替换指定部分 |
| `pop_back()` | 删除尾字符，要求非空 |
| `substr(pos, count)` | 返回独立子串，不修改原串 |

```cpp
std::string text = "hello";
text += " STL";                  // "hello STL"
text.insert(6, "C++ ");          // "hello C++ STL"
text.erase(6, 4);                // "hello STL"
text.replace(6, 3, "string");    // "hello string"
auto word = text.substr(6, 6);   // "string"
```

`substr` 的第二项是**数量**，不是结束下标。上述下标形式的 `insert`、`erase`、`replace`、`substr` 在 `pos > size()` 时抛出异常；`pos == size()` 可表示末尾。连续序列的中间插删通常需要搬移后续元素。

> [!CAUTION]
>
> `insert`、`erease`、`replace`谨慎使用，底层数据挪动，效率低下，0(N)。

### 9.2 查找与 npos

`find` 向后查找，`rfind` 反向查找，返回匹配起始下标；未找到时返回 `std::string::npos`。

```cpp
std::string file = "notes.stl.md";
auto pos = file.rfind('.');
if (pos != std::string::npos) {
    std::cout << file.substr(pos + 1); // md
}
bool found = file.find("stl") != std::string::npos;
bool prefix = file.starts_with("notes"); // C++20
bool suffix = file.ends_with(".md");     // C++20
```

不能用 `if (text.find(key))` 判断是否找到：下标 0 会被视为假，`npos` 会被视为真。应先判断 `npos`，再做位置运算。`string::contains` 从 C++23 起提供，C++20 使用 `find`。

### 9.3 比较

- `==`、`!=` 比较内容是否相等。
- `<`、`>` 等按字典序比较；例如字符串 `"10"` 排在 `"2"` 前面。
- `compare` 返回负数、零或正数，不保证恰好为 `-1`、`0`、`1`。

## 10. 字符串输入与数值转换

### 10.1 输入

`std::cin >> text` 读取一个以空白分隔的单词；`std::getline` 默认读取一整行，不保存换行符。

```cpp
int age = 0;
std::string name;
if (std::cin >> age) {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, name);
}
```

`ignore` 需要 `<limits>`。读取整数后先丢弃当前行剩余内容，避免 `getline` 直接读到残留换行。`std::ws` 会跳过所有前导空白，不适合需要保留空行或行首空格的场景。

### 10.2 转换

```cpp
int number = std::stoi("123");           // 123
std::string text = std::to_string(456);  // "456"
std::size_t used = 0;
int prefix = std::stoi("123abc", &used);  // 123，used 为 3
```

`stoi` 允许只转换数字前缀；要求完整转换时，检查 `used == 原字符串.size()`。无法转换时抛出 `std::invalid_argument`，超出范围时抛出 `std::out_of_range`。`stoll`、`stod` 分别用于其他数值类型。

## 11. C 风格接口与 string_view

### 11.1 c_str 与 data

```cpp
std::string text = "hello";
const char* pointer = text.c_str(); // 只读、以 \0 结尾
text.data()[0] = 'H';               // 可修改已有字符，结果为 "Hello"
```

指针借用字符串的存储，不能手动释放；字符串销毁或发生使指针失效的修改后不能继续使用。C 字符串接口会在首个 `\0` 处停止，不一定处理 `string` 的全部内容。

### 11.2 string_view

`std::string_view` 定义于 `<string_view>`，只保存字符区域的视图，不拥有或复制字符。

```cpp
std::string owner = "hello STL";
std::string_view view = owner;
auto word = view.substr(6, 3); // "STL"，不复制字符
std::string copy(word);       // 复制为拥有数据的字符串
```

原数据销毁或所指区域失效后，视图会悬空。不要让视图指向临时 `string`；视图末尾也不保证有 `\0`，不能直接当成 C 字符串。

## 12. 常用容器示例

### 12.1 vector

```cpp
std::vector<int> counts(3, 7); // 3 个 7
std::vector<int> values{3, 7}; // 两个元素：3、7
values.reserve(10);           // 只预留容量
values.push_back(9);
values.emplace_back(11);
values.erase(values.begin() + 1); // 剩余 3、9、11
```

`push_back` 接收元素值，`emplace_back` 接收构造参数；后者不保证总是更快，扩容时仍可能移动已有元素。

### 12.2 map

```cpp
std::map<std::string, int> scores;
scores["Alice"] = 90;
scores.try_emplace("Bob", 80); // 已有键时不覆盖

auto it = scores.find("Alice");
if (it != scores.end()) {
    std::cout << it->second; // 90
}
bool found = scores.contains("Bob"); // C++20
int value = scores["Nobody"];       // 插入新键，对应 int 值为 0

for (auto& [name, score] : scores) {
    ++score; // 可修改值，不能直接修改键
}
```

只查询时使用 `find`、`contains` 或 `at`；`operator[]` 在键不存在时会插入。`at` 在键不存在时抛出异常。`set` 元素本身就是键，也不能通过迭代器直接改写。

### 12.3 stack、queue 与 priority_queue

```cpp
std::stack<int> history;
history.push(1);
history.push(2);
std::cout << history.top(); // 2
history.pop();

std::queue<int> tasks;
tasks.push(1);
tasks.push(2);
std::cout << tasks.front(); // 1
tasks.pop();

std::priority_queue<int> largest;
largest.push(3);
largest.push(8);
std::cout << largest.top(); // 8

std::priority_queue<int, std::vector<int>, std::greater<int>> smallest;
smallest.push(3);
smallest.push(8);
std::cout << smallest.top(); // 3
```

分别包含 `<stack>`、`<queue>`；小顶堆示例还使用 `<vector>`、`<functional>`。

## 13. 算法、函数对象与 Lambda

### 13.1 常用算法

| 算法 | 作用 | 注意事项 |
| --- | --- | --- |
| `find` / `find_if` | 查找值 / 首个满足条件的元素 | 未找到返回区间末尾 |
| `count` / `count_if` | 计数 | 遍历区间 |
| `sort` / `stable_sort` | 排序 / 稳定排序 | 要求随机访问迭代器 |
| `lower_bound` / `binary_search` | 查找边界 / 二分判断存在 | 通常先按同一规则排序 |
| `reverse` | 反转区间 | 要求双向迭代器 |
| `unique` | 压缩相邻重复元素 | 不缩小容器 |
| `remove` / `remove_if` | 压缩保留的元素 | 不缩小容器 |
| `copy` / `transform` | 复制 / 变换 | 目标有足够元素，或使用插入迭代器 |
| `accumulate` | 累加 | 在 `<numeric>`，累加类型由初始值决定 |

其他表中算法位于 `<algorithm>`。查找关联容器时，优先使用 `map::find`、`set::lower_bound` 等成员函数。

### 13.2 排序、去重与过滤

```cpp
std::vector<int> values{3, 1, 2, 1, 4, 2};
std::sort(values.begin(), values.end());
auto last = std::unique(values.begin(), values.end());
values.erase(last, values.end()); // 1、2、3、4

values.erase(std::remove_if(values.begin(), values.end(),
                           [](int x) { return x % 2 == 0; }),
             values.end()); // 1、3

std::vector<int> output;
std::copy(values.begin(), values.end(), std::back_inserter(output));
auto sum = std::accumulate(values.begin(), values.end(), 0LL); // 4
std::erase_if(output, [](int x) { return x > 1; }); // C++20，剩余 1
```

`unique`、`remove_if` 返回新的逻辑末尾，`erase` 才真正删除尾部元素。`back_inserter` 在 `<iterator>` 中，通过追加写入目标容器；不能把非空区间直接复制到空 `vector` 的 `begin()`。

### 13.3 函数对象与 Lambda

重载 `operator()` 的对象可像函数一样调用，常用于提供比较规则。Lambda 用于就近编写简短的可调用对象。

```cpp
std::vector<int> values{3, 1, 2};
std::sort(values.begin(), values.end(), std::greater<int>{}); // 3、2、1

int limit = 2;
auto count = std::count_if(values.begin(), values.end(),
                          [limit](int value) { return value >= limit; }); // 2
```

比较器返回真表示左侧元素应排在右侧之前，必须满足严格弱序；不能用 `<=` 代替 `<`，因为元素不能排在自身之前。Lambda 中 `[limit]` 按值捕获，`[&limit]` 按引用捕获，后者要求外部对象仍有效。

## 14. 分配器与资源管理

分配器负责存储空间，容器协调元素的构造、销毁和空间释放。普通代码使用默认分配器即可。

容器离开作用域时会销毁元素，但保存原生指针不会自动释放其指向的对象。需要独占动态对象时，可存放 `std::unique_ptr`：

```cpp
std::vector<std::unique_ptr<int>> values;
values.push_back(std::make_unique<int>(42));
// 容器销毁时，unique_ptr 自动释放其拥有的对象。
```

智能指针位于 `<memory>`。不要手动释放 `string::data()` 或 `vector::data()` 返回的内部地址。

## 15. auto 与类型推导

### 15.1 基本用法

`auto` 让编译器根据初始化表达式推导类型。**类型在编译期确定，之后不会随赋值改变。**

```cpp
auto number = 10;                  // int
auto decimal = 3.14;               // double
auto letter = 'A';                 // char
auto ctext = "hello";              // const char*，不是 string
auto text = std::string("hello");  // std::string
auto length = text.size();         // std::string::size_type
auto result = 5 / 2;               // int，结果为 2
// auto value;                    // 错误：缺少初始化表达式
```

### 15.2 引用与 const

给定 `int number = 10;`、`const int fixed = 20;`：

| 声明 | 最终类型 | 含义 |
| --- | --- | --- |
| `auto a = number;` | `int` | 独立副本 |
| `auto b = fixed;` | `int` | 按值推导忽略顶层 const |
| `const auto c = number;` | `const int` | 只读副本 |
| `auto& d = number;` | `int&` | 引用原变量，可修改 |
| `auto& e = fixed;` | `const int&` | 保留被引用对象的 const |
| `const auto& f = number;` | `const int&` | 通过引用只读访问 |
| `auto p = &fixed;` | `const int*` | 保留所指对象的 const |

```cpp
int number = 10;
auto copy = number;
auto& ref = number;
copy = 20; // number 仍为 10
ref = 30;  // number 变为 30
```

### 15.3 配合 STL 使用

```cpp
std::vector<std::string> words{"hello", "STL"};
auto it = words.begin();  // 自动推导迭代器类型
auto read = words.cbegin(); // 只读迭代器，可以移动，不能修改元素

for (auto word : words) {
    word += '!'; // 修改副本
}
for (auto& word : words) {
    word += '!'; // 修改原元素
}
for (const auto& word : words) {
    std::cout << word; // 只读，避免复制字符串
}
```

普通容器遍历中：`auto` 常用于副本，`auto&` 用于修改原元素，`const auto&` 用于只读访问。`auto&&` 常见于泛型遍历，绑定左值时也可能得到左值引用。

### 15.4 常见推导差异

```cpp
int values[3] = {1, 2, 3};
auto pointer = values;    // int*，数组退化为指针
auto& array = values;     // int (&)[3]，保留数组类型

auto single{10};          // int
auto list = {10};         // std::initializer_list<int>，需 <initializer_list>
// auto mixed = {1, 2.0}; // 错误：列表元素类型不一致
// auto a = 1, b = 2.0;  // 错误：同一声明的 auto 推导不一致
```

### 15.5 函数与 Lambda

以下函数定义放在 `main()` 外：

```cpp
auto Add(int left, int right) // C++14：推导返回类型为 int
{
    return left + right;
}

auto AddMixed(auto left, auto right) // C++20：简写函数模板
{
    return left + right;
}
```

泛型 Lambda 从 C++14 起可以使用 `auto` 参数：

```cpp
auto twice = [](auto value) { return value + value; };
auto integer = twice(3);     // int，6
auto decimal = twice(1.5);   // double，3.0
```

普通 `auto` 返回值通常按值推导；需要返回引用时显式使用 `auto&`，并保证被引用对象仍然有效。多个参与推导的 `return` 必须得到相同类型。
