# 自定义 string：接口、实现与详细流程图

本文依据 [string.h](string.h) 与 [string.cpp](string.cpp) 编写，分析的是全局命名空间中的自定义 `string`，不是 `std::string`。本文只分析头文件、实现文件和构建配置，省略 `main.cpp` 中的演示逻辑。当前默认构建包含演示程序；只构建类实现可指定 `stringlib` 目标。本文单独维护，不向 `README.md` 追加内容。

代码核对日期：2026-10-31。当前版本提供 `swap(string&)`，拷贝构造通过 `string tmp(s._str)` 和 `swap(tmp)` 实现；赋值运算仍按源对象的 `_size` 分配并复制。这两条路径对内嵌终止符的处理不同，详见第 4 节。

流程图使用 Mermaid：在支持 Mermaid 的 Markdown 预览器中可直接查看。各图描述**当前实际实现**；发现的问题单独注明，不能将图中的执行路径理解为已经修复后的算法。

## 目录

1. [目录与构建](#1-目录与构建)
2. [对象模型与约定](#2-对象模型与约定)
3. [接口速查](#3-接口速查)
4. [构造、拷贝、赋值与析构](#4-构造拷贝赋值与析构)
5. [访问与迭代器](#5-访问与迭代器)
6. [容量管理与尾部追加](#6-容量管理与尾部追加)
7. [插入](#7-插入)
8. [删除与清空](#8-删除与清空)
9. [查找与子串](#9-查找与子串)
10. [比较运算](#10-比较运算)
11. [流输入输出](#11-流输入输出)
12. [当前边界问题与使用限制](#12-当前边界问题与使用限制)

## 1. 目录与构建

```text
CPPString/
├── CMakeLists.txt             # C++20：stringlib 静态库和 CPPString 演示程序
├── README.md                 # 原有学习文档
├── main.cpp                  # CPPString 可执行目标，本文省略其逻辑
└── string/
    ├── string.h              # 类型、成员声明、getline 声明
    ├── string.cpp            # 成员及非成员函数定义
    └── STRING_IMPLEMENTATION.md # 本文
```

项目实际使用 `CMakeLists.txt`，配置仅启用 C++，要求 C++20 并关闭编译器语言扩展。MSVC 使用 `/W4`；其他编译器使用 `-Wall -Wextra -Wpedantic`。当前有两个构建目标：

| 目标 | 类型 | 源文件与依赖 |
| --- | --- | --- |
| `stringlib` | 静态库 | `string/string.cpp` 和 `string/string.h`，不包含入口函数 |
| `CPPString` | 可执行程序 | 编译 `main.cpp` 并链接 `stringlib` |

在 `CPPString` 目录运行：

```powershell
cmake -S . -B build-string -G "MinGW Makefiles"
cmake --build build-string --target stringlib
```

需要 CMake 3.20 或以上版本及支持 C++20 的编译器。上述生成器用于 MinGW；使用其他工具链时按环境选择生成器。只构建 `stringlib` 时，MinGW 产物为 `build-string/libstringlib.a`，不编译 `main.cpp`。若执行 `cmake --build build-string` 或指定 `--target CPPString`，则还会生成演示程序 `CPPString.exe`；构建命令本身不运行程序。已有 CLion 构建目录需要重新加载 CMake。

其他 CMake 目标可通过 `target_link_libraries(调用方 PRIVATE stringlib)` 链接，并使用 `#include "string/string.h"`。公开包含路径为项目根目录，避免直接把 `string/` 加入搜索路径而让自定义 `string.h` 遮蔽系统同名头文件。

## 2. 对象模型与约定

| 成员或类型 | 含义 |
| --- | --- |
| `char* _str` | 对象独占的堆数组首地址 |
| `std::size_t _size` | 有效字符数量，不包含末尾终止符 |
| `std::size_t _capacity` | 可容纳的有效字符数量，不包含末尾终止符 |
| `static const size_t npos` | 实现中由 `-1` 转换为无符号 `size_t`，表示最大值；用于未找到、删除至末尾及截取至末尾 |
| `iterator` | `char*`，可修改字符 |
| `const_iterator` | `const char*`，只读访问字符 |

在有效调用且未触发实现缺陷时，完成构造的活对象维持：

- `_size <= _capacity`。
- 分配的数组包含 `_capacity + 1` 个字符位置。
- `_str[_size] == '\0'`，即使空字符串也有一个终止符位置。
- 不同对象各自拥有内存；拷贝对象不会共享 `_str`。`swap` 交换各自拥有的数组及配套长度、容量，不创建共享所有权。

例如 `_size = 3`、`_capacity = 6`：

```text
下标       0    1    2    3     4    5    6
内容      'a'  'b'  'c'  '\0'   ?    ?    ?
           └── 有效字符 ──┘      未使用容量
begin() -> 0             end() -> 3
```

`end()` 是尾后迭代器；遍历区间为 `[begin(), end())`。虽然尾后位置存放终止符，也不应作为有效字符解引用或修改。

调用关系总览（终止符的维护分散在具体修改函数中）：

```mermaid
flowchart TD
    api["自定义 string 接口"] --> life["构造、拷贝、赋值、析构"]
    life --> copySwap["拷贝构造：创建 C 字符串临时对象并 swap"]
    copySwap --> exchange["swap 同步交换指针、长度、容量"]
    api --> access["访问、迭代、查找、比较"]
    api --> edit["追加、插入、删除、清空"]
    api --> io["流输入输出"]
    edit --> growth["push_back / append / insert 按需扩容"]
    growth --> reserveCall["reserve 分配、复制、替换内存"]
    io --> input["输入：clear 后分块追加"]
    input --> appendCall["operator+= 调用 append"]
    appendCall --> reserveCall
    life --> ownership["独占 char 数组，深拷贝与 delete[]"]
```

## 3. 接口速查

以下记 `n` 为当前长度、`m` 为输入 C 字符串长度、`k` 为实际截取长度；复杂度忽略分配器自身开销。字符串指针参数必须非空，并指向有效且以 `\0` 终止的数组。

| 接口 | 返回与用途 | 关键前提 / 当前行为 | 时间复杂度 |
| --- | --- | --- | --- |
| `string()` | 创建空对象 | 分配一个终止符 | O(1) |
| `explicit string(const char*)` | 从 C 字符串构造 | 不允许隐式转换；容量等于长度 | O(m) |
| `string(const string&)` | 通过临时对象与交换完成深拷贝 | 复制源首个终止符前的内容；新长度和容量取 `strlen(s._str)` | O(首个终止符前的长度) |
| `operator=(const string&)` | 返回 `*this` | 自赋值直接返回；先分配再释放 | O(源长度)，自赋值 O(1) |
| `swap(string&)` | 交换两个对象的资源，无返回值 | 依次交换指针、长度、容量；无分配，允许自交换 | O(1) |
| `~string()` | 释放数组 | 使用 `delete[]` | 除释放开销外 O(1) |
| `c_str()` / `size()` | 只读地址 / 长度 | 没有公开 `capacity()` | O(1) |
| `operator[](i)` 的两个重载 | 字符引用 | `assert(i < _size)` | O(1) |
| `begin()` / `end()` 的 const 与非 const 重载 | 首地址 / 尾后地址 | const 对象返回只读指针 | O(1) |
| `reserve(n)` | 扩容，无返回值 | 仅增容，不缩容、不改变长度 | 不扩容 O(1)，扩容 O(当前长度) |
| `push_back(ch)` / `operator+=(ch)` | 尾插；后者返回 `*this` | 空容量先扩至 4，之后翻倍 | 均摊 O(1)，单次最坏 O(n) |
| `append(str)` / `operator+=(str)` | 尾部追加；后者返回 `*this` | 容量不足时取翻倍容量与所需长度较大者 | O(m)，扩容时 O(n+m) |
| `insert(pos, ch)` | 返回 `*this` | `assert(pos <= _size)` | O(n-pos)，扩容时 O(n) |
| `insert(pos, str)` | 返回 `*this` | 同上；存在下标回绕问题，见第 7、12 节 | 正常路径 O(n-pos+m)，扩容时 O(n+m) |
| `erase(pos=0, len=npos)` | 返回 `*this` | `assert(pos < _size)`，不接受尾后位置 | O(n-pos-len) 或截断 O(1) |
| `pop_back()` | 删除尾字符 | `assert(_size > 0)` | O(1) |
| `clear()` | 清空但保留容量 | 设置首字符为终止符 | O(1) |
| `find(ch, pos)` | 下标或 `npos` | `pos` 没有默认值；从指定位置扫描 | O(n-pos)，pos 大于等于长度时 O(1) |
| `find(str, pos)` | 下标或 `npos` | `pos` 没有默认值；调用 `strstr`，要求 pos 不超过长度 | 取决于库的 `strstr` 实现 |
| `substr(pos, len=npos)` | 独立的新对象 | 必须自行保证 `pos <= _size`；返回时若发生拷贝，内嵌终止符可能使结果截断 | O(k)，未省略返回拷贝时仍为 O(k) |
| `< <= > >= == !=` | 布尔值 | 按字符或由其他比较操作组合 | 最坏 O(min(两串长度)) |
| `operator<<` | 返回输出流引用 | 按 C 字符串输出 | O(首个终止符前的字符数) |
| `operator>>` | 返回输入流引用 | 空格或换行停止；有重复读取缺陷 | 正常追加规模下与读取量线性相关 |
| `getline(in, str, delim='\n')` | 返回输入流引用 | 指定分隔符停止；有重复读取缺陷 | 同上 |

## 4. 构造、拷贝、赋值与析构

### 4.1 三种构造

默认构造分配 `new char[1]{'\0'}`。C 字符串构造先以 `strlen` 求长度，再复制包括终止符在内的 `长度 + 1` 个字节。实现文件中直接按源 `_size` 复制的旧拷贝构造已经注释，不参与编译；当前生效的拷贝构造先创建 `string tmp(s._str)`，再调用 `swap(tmp)`。

```mermaid
flowchart TD
    start(["进入构造函数"]) --> kind{"选择哪个构造重载？"}
    kind -->|"默认"| empty["分配 1 字节并写入终止符"]
    empty --> zero["size = 0，capacity = 0"]
    kind -->|"const char*"| length["strlen 求长度；size = 长度"]
    length --> cAlloc["capacity = size；分配 size + 1 字节"]
    cAlloc --> cCopy["memcpy 复制 size + 1 字节"]
    kind -->|"const string&"| sInit["当前对象成员初值：nullptr、0、0"]
    sInit --> sTemp["string tmp(s._str)：调用 C 字符串构造"]
    sTemp --> sSwap["swap(tmp)：取得临时对象的新数组、长度、容量"]
    sSwap --> sDestroy["tmp 析构：释放交换得到的 nullptr"]
    zero --> done(["构造完成，当前对象独占内存"])
    cCopy --> done
    sDestroy --> done
```

图省略了 `new[]` 抛出分配异常的出口；分配失败则构造不完成。类未声明移动构造和移动赋值；不要把拷贝行为解释为转移所有权。

拷贝构造没有委托给 `string()`，进入函数体前使用的是头文件中的成员默认初值：`_str = nullptr`、`_size = 0`、`_capacity = 0`。因此不会先给当前对象分配空数组；只有 `tmp` 通过 C 字符串构造分配新数组。交换之后，当前对象取得该新数组，`tmp` 接管空指针，析构时 `delete[] nullptr` 安全。

普通文本中，`strlen(s._str) == s._size`，拷贝内容及长度与源相同，但不保留源备用容量。若源含有内嵌 `\0`，`tmp` 只复制第一个终止符之前的前缀，拷贝结果长度会缩短，源对象保持原状。当前拷贝构造并不完整保留所有由 `_size` 记录的字符。

### 4.2 swap：三个成员一起交换

```cpp
void string::swap(string& s)
{
    std::swap(_str, s._str);
    std::swap(_size, s._size);
    std::swap(_capacity, s._capacity);
}
```

```mermaid
flowchart TD
    start(["swap(s)"]) --> ptr["std::swap：交换 _str 指针"]
    ptr --> length["std::swap：交换 _size"]
    length --> capacity["std::swap：交换 _capacity"]
    capacity --> done(["返回；两个对象互相接管原来的资源"])
```

这三个值描述同一块存储，必须配套交换。函数不复制字符、不分配内存，也不释放原数组，耗时与字符串长度无关；自交换最终保持原值。当前签名未声明 `noexcept`。

```cpp
string left("abc");
string right("XY");
left.swap(right);
// left 的内容是 "XY"，size() == 2
// right 的内容是 "abc"，size() == 3
```

交换前获取的字符指针、引用和迭代器仍指向原数组，但该数组现在由另一对象拥有；以后由新所有者的修改、扩容或析构决定其有效期。不能再把旧指针视为一定属于原对象。

### 4.3 拷贝赋值

赋值需要处理目标原有内存，所以不同于构造。先把新数据准备好，再释放旧内存，避免分配失败时破坏原对象。赋值后备用容量不会保留，而是变为源对象的长度。

当前赋值运算未调用 `swap`，也未改为按值传参的复制交换写法。它复制源 `_size + 1` 个字节，因此与当前拷贝构造相比，赋值可以保留有效范围内的内嵌 `\0` 及其后字符。

```mermaid
flowchart TD
    start(["operator=(s)"]) --> self{"this 与源对象地址相同？"}
    self -->|"是"| done(["返回 *this"])
    self -->|"否"| alloc["tmp = new char[源 size + 1]"]
    alloc --> ok{"分配成功？"}
    ok -->|"否"| fail(["抛出异常，原对象保持原状"])
    ok -->|"是"| copy["memcpy 到 tmp，包含终止符"]
    copy --> release["delete[] 旧 _str"]
    release --> replace["_str = tmp；size = 源 size；capacity = 源 size"]
    replace --> done
```

下面的代码展示两条复制路径的差别，示例省略入口函数：

```cpp
string source("abc");
source[1] = '\0';       // size() 仍为 3，数组为 a、终止符、c、终止符
string copied(source); // 通过 strlen 构造临时对象，copied.size() == 1
string assigned;
assigned = source;     // 按 _size 复制，assigned.size() == 3
// copied == source 为 false；assigned == source 为 true
```

### 4.4 析构

```mermaid
flowchart LR
    start(["对象生命周期结束"]) --> release["delete[] _str"]
    release --> reset["_str = nullptr；size = capacity = 0"]
    reset --> done(["析构结束"])
```

清零是析构函数中的收尾操作，不意味着已经析构的对象可以继续使用。深拷贝保证正常使用中两个对象不会对同一块数组重复释放。

## 5. 访问与迭代器

`c_str()` 返回内部只读指针，`size()` 直接返回 `_size`。下标运算区分 const 与非 const 对象，但都先验证 `i < _size`；不能用这个接口访问末尾终止符。

```mermaid
flowchart TD
    start(["访问类接口"]) --> which{"调用哪个接口？"}
    which -->|"c_str"| ptr(["返回 _str 的只读指针"])
    which -->|"size"| length(["返回 _size"])
    which -->|"begin / end"| range(["返回 _str / _str + _size"])
    which -->|"下标运算"| check["assert：i 小于 _size"]
    check --> get(["返回 _str[i] 的引用；const 对象只读"])
```

`assert` 是调试断言，定义 `NDEBUG` 后不会提供运行时边界保护。扩容、赋值或析构后，旧指针、引用和迭代器可能失效；插删即使未扩容，也会改变位置上的字符及尾后位置，应按需重新获取。`swap` 保留数组地址，但将数组的所有权交换给另一对象。修改有效字符时可以写入 `\0`，但这会使后续 C 字符串接口以及当前拷贝构造看到的长度与 `_size` 不一致。

## 6. 容量管理与尾部追加

### 6.1 reserve

`reserve(n)` 只在 `n > _capacity` 时重新分配，复制的是 `_size + 1` 字节，而不是整个旧容量。它不改变 `_size`，也不会初始化额外备用位置。

```mermaid
flowchart TD
    start(["reserve(n)"]) --> need{"n 大于 capacity？"}
    need -->|"否"| done(["返回，长度与容量不变"])
    need -->|"是"| alloc["分配 n + 1 字节"]
    alloc --> copy["复制 size + 1 字节，包含终止符"]
    copy --> release["释放旧数组"]
    release --> commit["替换 _str；capacity = n；size 不变"]
    commit --> finish(["返回，旧地址失效"])
```

如分配失败，执行不会到达释放旧数组的步骤，原对象保持原状。容量增长不会自动防御 `n + 1` 等算术溢出。

### 6.2 push_back 与字符 +=

```mermaid
flowchart TD
    start(["push_back(ch)"]) --> full{"size 大于等于 capacity？"}
    full -->|"是"| initial{"capacity 等于 0？"}
    initial -->|"是"| four["newCapacity = 4"]
    initial -->|"否"| twice["newCapacity = capacity × 2"]
    four --> grow["reserve(newCapacity)"]
    twice --> grow
    full -->|"否"| write["_str[size] = ch"]
    grow --> write
    write --> length["size 增加 1"]
    length --> nul["_str[size] 写入终止符"]
    nul --> done(["返回"])
```

`operator+=(char)` 只调用 `push_back(ch)` 后返回 `*this`，因此支持连续追加。扩容采用倍增策略，连续尾插可以摊薄复制开销。

### 6.3 append 与字符串 +=

```mermaid
flowchart TD
    start(["append(str)"]) --> length["len = strlen(str)"]
    length --> need{"size + len 大于 capacity？"}
    need -->|"是"| choose["newCapacity = max(2 × capacity, size + len)"]
    choose --> grow["reserve(newCapacity)"]
    grow --> copy["memcpy 到 _str + size，复制 len + 1 字节"]
    need -->|"否"| copy
    copy --> update["size 增加 len"]
    update --> done(["返回"])
```

`operator+=(const char*)` 调用 `append(str)`，随后返回 `*this`。复制 `len + 1` 字节让新串在尾部自动带上终止符。输入指针必须与对象内部数组无别名关系；`s.append(s.c_str())` 不能作为可靠用法：扩容会释放输入指向的旧内存，不扩容时 `memcpy` 的源目标也可能重叠。

## 7. 插入

### 7.1 插入单个字符

允许 `pos == _size`，即尾插。搬移从后向前执行，连终止符一起右移一格，避免覆盖尚未搬走的字符。

```mermaid
flowchart TD
    start(["insert(pos, ch)"]) --> assertPos["assert：pos 不大于 size"]
    assertPos --> full{"size 大于等于 capacity？"}
    full -->|"是"| grow["容量为 0 时扩至 4，否则翻倍；调用 reserve"]
    full -->|"否"| init["游标 e = size + 1"]
    grow --> init
    init --> loop{"e 大于 pos？"}
    loop -->|"是"| move["_str[e] = _str[e - 1]；e 减 1"]
    move --> loop
    loop -->|"否"| fill["_str[pos] = ch；size 增加 1"]
    fill --> done(["返回 *this"])
```

例如 `"abcd"` 在下标 2 插入 `'X'`：先依次搬动终止符、`d`、`c`，再写入 `X`，得到 `"abXcd"`。

### 7.2 插入 C 字符串：按现有代码展开

当前实现先留出 `len` 个位置，再逐字符复制输入串。搬移循环使用无符号下标减法，下图保留原条件；空输入时的回绕行为在图后说明。

```mermaid
flowchart TD
    start(["insert(pos, str)"]) --> assertPos["assert：pos 不大于 size"]
    assertPos --> length["len = strlen(str)"]
    length --> need{"size + len 大于 capacity？"}
    need -->|"是"| grow["reserve(max(2 × capacity, size + len))"]
    need -->|"否"| init["e = size + len"]
    grow --> init
    init --> loop{"e 大于 pos + len - 1？"}
    loop -->|"是"| move["_str[e] = _str[e - len]；e 减 1"]
    move --> loop
    loop -->|"否"| copyInit["i = 0"]
    copyInit --> copyLoop{"i 小于 len？"}
    copyLoop -->|"是"| copy["_str[pos + i] = str[i]；i 增加 1"]
    copy --> copyLoop
    copyLoop -->|"否"| update["size 增加 len"]
    update --> done(["返回 *this"])
```

循环边界推演：`len > 0` 时，循环最后把原下标 `pos` 的字符搬到 `pos + len`，然后 `e` 减为 `pos + len - 1`，循环退出，头部插入也适用。`pos = 0, len = 0` 时，`pos + len - 1` 回绕为 `size_t` 最大值，搬移循环直接跳过，后续复制也跳过，实际为空操作。`pos > 0, len = 0` 时，搬移循环仅进行原位置自赋值，内容也不变。

这个回绕现象本身不等于越界，但表达式不利于阅读与维护。更实质的风险是输入指针引用内部存储时，扩容后悬空或搬移后源字符被改变，以及容量算术溢出。

对于正常的非别名输入，例如 `"abcd"` 在下标 2 插入 `"XY"`：搬动终止符、`d`、`c` 共 3 个字节，再写入 `XY`，得到 `"abXYcd"`。后续若改写实现，可用避免 `pos + len - 1` 的循环条件，并显式对空输入快速返回；本次不修改算法。

## 8. 删除与清空

### 8.1 erase

`erase` 的断言是 `pos < _size`，因此空串不能调用 `erase()`，非空串也不能传入尾后下标。`len == npos` 或长度覆盖整个后缀时，直接在 `pos` 截断；否则通过 `memmove` 左移剩余后缀。这里源与目标区间可能重叠，所以使用 `memmove`。

```mermaid
flowchart TD
    start(["erase(pos, len)"]) --> check["assert：pos 小于 size"]
    check --> truncate{"len 为 npos 或 len 不小于 size - pos？"}
    truncate -->|"是"| cut["size = pos；在新尾部写终止符"]
    truncate -->|"否"| index["i = pos + len"]
    index --> move["memmove：从 i 移到 pos，共 size + 1 - i 字节"]
    move --> length["size 减少 len"]
    cut --> done(["返回 *this，容量不变"])
    length --> done
```

例如 `"abcdef".erase(2, 2)` 把 `"ef\0"` 移到下标 2，得到 `"abef"`；`erase(2)` 得到 `"ab"`；在合法 `pos` 上传入 `len = 0` 不改变内容。

### 8.2 pop_back 与 clear

```mermaid
flowchart TD
    start(["删除或清空"]) --> op{"调用哪个接口？"}
    op -->|"pop_back"| check["assert：size 大于 0"]
    check --> reduce["size 减少 1"]
    reduce --> tail["在新尾部写终止符"]
    op -->|"clear"| clearFirst["_str[0] 写入终止符；size = 0"]
    tail --> done(["返回，保留原容量"])
    clearFirst --> done
```

这些操作不释放内存，也不会抹掉全部旧字符；字符串的逻辑内容由长度和终止符确定。

## 9. 查找与子串

### 9.1 find(char, pos)

```mermaid
flowchart TD
    start(["find(ch, pos)"]) --> init["i = pos"]
    init --> loop{"i 小于 size？"}
    loop -->|"否"| missing(["返回 npos"])
    loop -->|"是"| match{"_str[i] 等于 ch？"}
    match -->|"是"| found(["返回 i"])
    match -->|"否"| next["i 增加 1"]
    next --> loop
```

它不搜索末尾终止符；传入 `pos >= size` 会直接返回 `npos`。

### 9.2 find(const char*, pos)

```mermaid
flowchart TD
    start(["find(str, pos)"]) --> call["p = strstr(_str + pos, str)"]
    call --> match{"p 是空指针？"}
    match -->|"是"| missing(["返回 npos"])
    match -->|"否"| found(["返回 p - _str，即原串下标"])
```

这里没有范围断言或检查，调用者必须保证 `pos <= size`；不能照搬字符版对超大 `pos` 的行为。搜索空模式会返回起始下标 `pos`。`strstr` 在首个终止符处停止，所以不能跨越内嵌 `\0` 搜索。

### 9.3 substr

调用者应保证 `pos <= _size`。`len` 默认是 `npos`；超出剩余后缀长度时，裁剪到 `_size - pos`。新对象先预留容量，再逐字符尾插，最终返回独立副本。

```mermaid
flowchart TD
    start(["substr(pos, len)"]) --> trim{"len 为 npos 或 len 不小于 size - pos？"}
    trim -->|"是"| cap["len = size - pos"]
    trim -->|"否"| create["默认构造 ret"]
    cap --> create
    create --> reserveCall["ret.reserve(len)；i = 0"]
    reserveCall --> loop{"i 小于 len？"}
    loop -->|"是"| appendChar["ret += _str[pos + i]；i 增加 1"]
    appendChar --> loop
    loop -->|"否"| done(["返回 ret"])
```

`pos == size` 时返回空对象；`pos > size` 会让无符号减法 `_size - pos` 回绕，后续可能越界访问。当前代码没有抛出越界异常的逻辑。

返回命名局部变量 `ret` 可以由编译器进行 NRVO（命名返回值优化），但并非保证省略拷贝。未省略时，类没有移动构造，会使用当前拷贝构造；若 `ret` 含有内嵌 `\0`，最终结果可能被截断到该终止符之前。普通不含内嵌终止符的文本不受这一区别影响。

## 10. 比较运算

### 10.1 小于：逐字符比较后判断前缀

```mermaid
flowchart TD
    start(["左串 operator 小于 右串"]) --> init["it1 = 0；it2 = 0"]
    init --> loop{"两个游标都未到各自末尾？"}
    loop -->|"否"| prefix(["返回 it2 小于右串长度"])
    loop -->|"是"| less{"左字符小于右字符？"}
    less -->|"是"| yes(["返回 true"])
    less -->|"否"| greater{"左字符大于右字符？"}
    greater -->|"是"| no(["返回 false"])
    greater -->|"否"| next["两个游标都增加 1"]
    next --> loop
```

遇到第一个不同字符就能确定结果；共同前缀完全相同时，短串更小。空串小于非空串，相同字符串不小于自身。比较采用当前平台的 `char` 值规则，不是 Unicode 字符排序；中文 UTF-8 内容按字节处理。

### 10.2 等于与其余关系

```mermaid
flowchart TD
    start(["operator=="]) --> init["it1 = 0；it2 = 0"]
    init --> loop{"两个游标都未到各自末尾？"}
    loop -->|"是"| same{"当前两个字符相同？"}
    same -->|"否"| no(["返回 false"])
    same -->|"是"| next["两个游标都增加 1"]
    next --> loop
    loop -->|"否"| finish(["返回两个游标是否都到各自末尾"])
```

| 运算 | 实际复用表达式 |
| --- | --- |
| `a <= b` | `a < b || a == b`，有短路求值 |
| `a > b` | `!(a <= b)` |
| `a >= b` | `!(a < b)` |
| `a != b` | `!(a == b)` |

`==` 没有先比较长度，长度不同但有共同前缀时仍会遍历前缀。`<=` 在第一次比较为假时还可能再次遍历判等；整体最坏时间复杂度仍为线性。

## 11. 流输入输出

### 11.1 operator<<

```mermaid
flowchart LR
    start(["out 流输出 s"]) --> write["执行 out 输出 s._str，使用 C 字符串规则"]
    write --> done(["返回 out 引用，允许连续输出"])
```

此处没有按 `_size` 指定输出长度：如果有效字符中存在内嵌 `\0`，输出会提前停止。

### 11.2 operator>> 与 getline 的公共分块流程

两者都先清空对象，使用 `char buff[128]` 暂存字符；累计 127 个字符后写入终止符，通过 `operator+=` 和 `append` 追加整块；循环结束后追加不足一块的残留内容。

差别在停止条件：`operator>>` 检查空格或换行，`getline` 检查传入的 `delim`，默认是换行。两者在循环条件中都检查 `in.get(ch)` 的流状态。

**重要：当前循环体末尾还有一次 `ch = in.get()`。** 这次读取的结果在下一次条件中的 `in.get(ch)` 被覆盖，因此会吞掉字符，甚至吞掉本应终止输入的分隔符。下图准确保留了这一步。

```mermaid
flowchart TD
    start(["operator>> 或 getline"]) --> clearCall["清空目标；创建 buff[128]；i = 0"]
    clearCall --> read["循环条件：in.get(ch) 读取一次"]
    read --> state{"本次读取成功？"}
    state -->|"否"| remain{"i 大于 0？"}
    state -->|"是"| delimiter{"ch 命中该函数的停止字符？"}
    delimiter -->|"是"| remain
    delimiter -->|"否"| store["buff[i] = ch；i 增加 1"]
    store --> full{"i 等于 127？"}
    full -->|"是"| flush["buff[i] 写入终止符；追加缓冲块；i = 0"]
    full -->|"否"| extra["循环体末尾再次 in.get；字符未存入缓冲区"]
    flush --> extra
    extra --> read
    remain -->|"是"| tail["buff[i] 写入终止符；追加剩余内容"]
    remain -->|"否"| done(["返回输入流引用"])
    tail --> done
```

在没有更早命中停止字符的情况下，输入 `abcdef` 后到达 EOF，目前会保存 `ace`，丢掉 `bdf`。如果空格或分隔符恰好由循环体末尾那次读取取走，条件不会检查它，函数可能继续读取后续内容。

`operator>>` 不会先跳过前导空白，只把字面空格和换行当作停止字符；制表符没有被当作分隔符。停止字符若在条件中读到也已被消耗。不能把它当作标准 `std::string` 提取操作的等价实现。

如以后修复，应删除循环体末尾的重复读取，保留条件中的单次读取，并明确是否需要标准流的前导空白、EOF 状态及分隔符语义。本次保留原实现，只记录实际行为。

## 12. 当前边界问题与使用限制

| 位置 | 当前事实与影响 | 使用时的约束或后续修正方向 |
| --- | --- | --- |
| 拷贝构造 | 通过 `string tmp(s._str)` 使用 `strlen`，内嵌终止符之后的有效字符不被复制；与赋值语义不同 | 当前用于普通 C 字符串文本；若需完整值复制，后续应统一为按源长度复制 |
| `swap` | 指针、长度、容量一起交换，数组地址保持，所有权属于另一对象 | 交换后按新所有者的生命周期判断旧指针有效期；当前没有 `noexcept` 声明 |
| `operator>>`、`getline` | 条件和循环体各读取一次，导致隔字符丢失、漏检分隔符 | 后续应统一为每轮一次读取 |
| `append`、字符串 `insert` | 未处理输入指向自身数组的情况；扩容可能使源指针悬空，插入搬移可能改写源数据；追加还可能触发重叠 `memcpy` | 当前传入独立、有效的 C 字符串；后续考虑输入快照或偏移处理 |
| 字符串 `insert` | `pos + len - 1` 在 `pos == 0 && len == 0` 时无符号回绕，当前直接跳过搬移，通常表现为空操作 | 建议显式处理空输入、使用无需减 1 的循环边界 |
| `substr` | 没有检查 `pos <= size`；差值可回绕并导致越界读取 | 调用前自行校验起点 |
| 字符串 `find` | 直接使用 `_str + pos` 和 `strstr`，没有范围校验 | 要求 `pos <= size`，不要传 `npos` |
| `erase` | 断言使用严格小于；空串以及 `pos == size` 均不满足前提 | 清空可调用 `clear()`；不要按标准库接口边界套用 |
| `operator[]`、`insert`、`erase`、`pop_back` | 依赖断言；Release 下通常没有检查 | 必须遵守各接口前置条件，断言不是异常机制 |
| 接收 `const char*` 的接口 | `strlen` / `strstr` 不处理空指针及缺少终止符的数组 | 参数必须非空、有效、终止正确 |
| 容量计算 | `size + len`、`2 * capacity`、`n + 1` 没有溢出检查 | 超大输入尚无可靠保护，后续需要长度上限与溢出校验 |
| 内嵌终止符 | 长度、遍历、比较和赋值可以覆盖内嵌 `\0`；输出、`strlen`、`strstr` 和当前拷贝构造按首个终止符截断；`substr` 未省略返回拷贝时也可能截断 | 当前更适用于普通文本，接口不统一支持二进制内容 |
| 字符编码 | 长度与位置按 `char` 字节计算 | UTF-8 汉字可能占多个字节，不能把下标当作汉字序号 |
| 类型命名与特性 | 全局 `string` 容易与 `std::string` 混淆；没有移动操作、公开容量查询等接口 | 示例中使用明确类型限定；学习实现不等同于标准库完整功能 |

本文的流程图用于理解当前生效的实现、检查循环和边界。本轮依据现有代码更新文档，保留现有算法行为；注释中的旧实现不作为当前执行路径。
