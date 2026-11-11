# C++ 迭代器学习笔记

本项目使用 C++20。本文整理下标遍历与迭代器的区别、迭代器类型的判断方法，以及算法对迭代器能力的要求。

## 1. 已经有下标遍历，为什么还需要迭代器？

下标适合按位置访问；迭代器提供统一的遍历方式，让算法能够处理不同容器。

### 1.1 并不是所有容器都有下标

| 容器 | 支持按位置下标访问 `c[i]` | 支持迭代器 |
|---|---|---|
| `vector` | 是 | 是 |
| `deque` | 是 | 是 |
| `list` | 否 | 是 |
| `set` | 否 | 是 |

注意：`map` 的 `m[key]` 是按键访问，不是按第几个元素访问。

`vector` 的元素连续存储（`vector<bool>` 是特殊情况），访问 `v[i]` 可以直接定位，时间复杂度为 **O(1)**。

`list` 是双向链表，节点通过指针连接。要到达某个位置，需要沿着链逐个移动，所以 `std::list` 没有提供 `operator[]`。

```cpp
#include <iostream>
#include <list>

int main() {
    std::list<int> nums = {10, 20, 30};

    for (auto it = nums.begin(); it != nums.end(); ++it) {
        std::cout << *it << ' ';
    }
}
```

- `begin()`：指向第一个元素；空容器中等于 `end()`。
- `end()`：表示最后一个元素之后的位置，不能解引用。
- `*it`：访问当前位置的元素。
- `++it`：移动到下一个位置。

假设给链表自己实现一个每次从头查找的下标操作，那么用下标完整遍历需要走 `1 + 2 + ... + n` 次，复杂度为 **O(n²)**。迭代器保留当前位置，顺着链走一遍只需要 **O(n)**。

### 1.2 统一不同容器的算法接口

```cpp
#include <algorithm>
#include <iostream>
#include <list>

int main() {
    std::list<int> nums = {10, 20, 30};
    auto it = std::find(nums.begin(), nums.end(), 20);

    if (it != nums.end()) {
        std::cout << *it;
    }
}
```

把 `nums` 换成 `vector<int>`，查找代码仍然适用。`std::find` 只需要知道如何读取元素、前进和判断是否到达终点。

算法通常接收半开区间 `[begin, end)`：包含起点，不包含终点。不同算法对迭代器能力的要求不同，例如 `std::sort` 需要随机访问迭代器，不能直接用于 `list`；链表排序可使用 `nums.sort()`。

### 1.3 只需逐个访问时，可以用范围 for

```cpp
for (const auto& x : nums) {
    std::cout << x << ' ';
}
```

对于标准容器，范围 `for` 背后也是通过迭代器遍历的。

| 使用场景 | 常见选择 |
|---|---|
| 需要位置编号，且容器支持按位置访问 | 下标 |
| 只需要逐个处理元素 | 范围 `for` |
| 配合标准算法、保存位置或调用接受迭代器的操作 | 迭代器 |

## 2. 如何判断迭代器的类型？

“类型”需要区分两个问题：**它具体是什么类型**，以及**它具有什么访问和移动能力**。

### 2.1 具体类型：看容器和是否只读

```cpp
std::vector<int> v = {1, 2, 3};

auto it1 = v.begin();   // std::vector<int>::iterator
auto it2 = v.cbegin();  // std::vector<int>::const_iterator

*it1 = 10;             // 可以修改元素
// *it2 = 10;          // 编译错误：不能通过 const_iterator 修改元素
++it2;                 // 可以移动迭代器
```

如果容器本身是 `const`，调用 `begin()` 得到的也是 `const_iterator`。

`const_iterator` 与 `const iterator` 的区别：

```cpp
std::vector<int> v = {1, 2, 3};

std::vector<int>::const_iterator a = v.begin();
++a;                   // 可以移动
// *a = 10;            // 不可以修改元素

const std::vector<int>::iterator b = v.begin();
*b = 10;               // 可以修改元素
// ++b;                // 不可以移动：迭代器对象本身是 const
```

能否修改元素还受容器约束：`set` 的元素不能通过迭代器修改；`map` 的键不能修改，但普通迭代器可以修改对应的值。

### 2.2 迭代器类别：看支持哪些操作

常见容器的迭代器可以按下面的能力理解。表中操作都要求迭代器和目标位置有效，不能越界。

| 类别 | 主要能力 | 常见例子 |
|---|---|---|
| 前向迭代器 | 读取、`++it`、相等比较，支持多遍遍历 | `forward_list`、`unordered_set`、`unordered_map` |
| 双向迭代器 | 前向能力，加上 `--it` | `list`、`set`、`map` |
| 随机访问迭代器 | 双向能力，加上 `it + n`、`it[n]`、迭代器相减和位置比较 | `deque`、`vector`、`array` |
| 连续迭代器（C++20） | 随机访问能力，且元素在内存中连续存储 | 普通 `vector<T>`、`array`、指向数组元素的指针 |

这些类别的能力逐层增强：连续迭代器也满足随机访问迭代器的要求。`vector<bool>` 是特殊情况，不属于连续迭代器；`deque` 支持随机访问，但不保证元素整体连续存储。

此外还有面向输入输出的类别：

- **输入迭代器**：提供读取和单遍遍历能力，例如 `std::istream_iterator<int>`，不能依赖它重新遍历已读取的输入。
- **输出迭代器**：提供写入能力，例如 `std::back_insert_iterator`；它不要求支持读取元素。

输入、输出描述访问能力，不能仅凭 `*it` 或 `++it` 就判断具体类别。

```cpp
std::vector<int> v = {10, 20, 30};
auto a = v.begin();
a += 2;                // 可以：随机访问，O(1)
std::cout << *a;        // 30

std::list<int> lst = {10, 20, 30};
auto b = lst.begin();
++b;                   // 可以：前进一步
--b;                   // 可以：后退一步
// b += 2;             // 编译错误：list 的迭代器不支持随机访问
std::advance(b, 2);     // 可以：对 list 内部逐步前进，移动 n 步为 O(n)
```

`std::advance` 定义在 `<iterator>` 中，它能统一移动的写法，但不会让链表获得 O(1) 的随机访问能力。

### 2.3 用代码判断：C++17 类型特征

`decltype(it)` 获取具体类型，`std::iterator_traits` 提供迭代器相关信息。

```cpp
#include <iterator>
#include <type_traits>
#include <vector>

int main() {
    std::vector<int> v;
    auto it = v.begin();

    using Iterator = decltype(it);
    using Category =
        std::iterator_traits<Iterator>::iterator_category;

    static_assert(std::is_same_v<Iterator,
                                 std::vector<int>::iterator>);
    static_assert(std::is_base_of_v<
                  std::random_access_iterator_tag, Category>,
                  "Random-access iterator required");
}
```

这里用 `is_base_of_v` 判断类别是否具备至少随机访问的能力，而不只检查标签是否完全相同。`static_assert` 在编译期间检查条件，失败时编译报错。

### 2.4 用代码判断：C++20 概念

本项目使用 C++20，可以直接使用标准库的迭代器概念：

```cpp
#include <iterator>
#include <list>
#include <vector>

int main() {
    std::vector<int> v;
    std::list<int> lst;

    static_assert(std::random_access_iterator<decltype(v.begin())>);
    static_assert(std::contiguous_iterator<decltype(v.begin())>);

    static_assert(std::bidirectional_iterator<decltype(lst.begin())>);
    static_assert(!std::random_access_iterator<decltype(lst.begin())>);
}
```

判断连续迭代器时，应使用 C++20 的 `std::contiguous_iterator`。不能只检查传统的 `iterator_category`，因为连续迭代器的这个标签可能仍是 `random_access_iterator_tag`，例如普通指针。

## 3. 如何理解算法对迭代器的要求？

算法内部会使用迭代器的某些操作。传入的迭代器必须提供这些操作，并满足对应的语义和复杂度要求，算法才能使用。

### 3.1 模板参数的名字提示了要求

文档中常见下面这样的声明（简化形式）：

```cpp
template<class RandomAccessIterator>
void sort(RandomAccessIterator first, RandomAccessIterator last);
```

`RandomAccessIterator` 表示“随机访问迭代器”，提示调用者：传入的迭代器需要具备随机访问能力。

**模板参数的名字本身不负责检查类别。** 即使把名字改成 `It`，算法实现对迭代器能力的要求也不会消失：

```cpp
template<class It>
void sort(It first, It last);
```

当算法内部使用 `last - first`、`first + n` 等操作时，传入不支持这些操作的类型，通常会在模板实例化时出现编译错误。C++20 的概念则可以用来显式表达约束；仅仅把参数命名为 `RandomAccessIterator` 并不等于添加了概念约束。

### 3.2 为什么 find、reverse、sort 的要求不同？

下面讨论的是 `<algorithm>` 中接收两个迭代器的经典算法版本。

| 算法 | 最低迭代器类别要求 | 原因 | 元素的其他主要要求 |
|---|---|---|---|
| `std::find` | 输入迭代器 | 逐个读取、比较并前进即可 | 能与目标值进行相等比较 |
| `std::reverse` | 双向迭代器 | 需要从两端向中间移动 | 能通过迭代器交换元素 |
| `std::sort` | 随机访问迭代器 | 实现需要高效定位和计算位置距离 | 元素能被移动、赋值和交换，比较规则满足严格弱序 |

能力更强的迭代器可以用于要求较低的算法，前提是元素也满足算法要求。例如，`vector<int>` 的迭代器可以用于上述三个算法。

#### find：向后逐个查找

`std::find` 可以理解为下面的逻辑：

```cpp
while (first != last) {
    if (*first == value)
        return first;
    ++first;
}
return last;
```

它不需要后退，也不需要直接跳到某个位置，所以 `vector`、`list`、`forward_list` 的迭代器都能用于查找。

#### reverse：从两端向中间交换

```text
原始：10  20  30  40
      →           ←    交换 10 和 40
          →   ←        交换 20 和 30
结果：40  30  20  10
```

左边的迭代器需要 `++`，右边的迭代器需要 `--`，因此要求双向迭代器。实现中会先把尾后位置向前移动，再访问右侧元素，不会解引用 `end()`。

`list` 支持双向移动，可以使用 `std::reverse`；`forward_list` 只支持单向移动，不能使用这个算法，但可以调用自己的成员函数 `reverse()`。

#### sort：需要随机访问能力

随机访问迭代器支持 O(1) 的位置移动和距离计算，例如 `it + n` 和 `last - first`。`list` 只能逐步移动，因此不能用于 `std::sort`。

链表可以排序：`list::sort()`、`forward_list::sort()` 提供适合各自链表结构的实现。

```cpp
#include <algorithm>
#include <forward_list>
#include <list>
#include <vector>

int main() {
    std::vector<int> v = {3, 1, 2};
    std::sort(v.begin(), v.end());          // 可以：随机访问迭代器

    std::list<int> lst = {3, 1, 2};
    auto it = std::find(lst.begin(), lst.end(), 1); // 可以：逐个查找
    (void)it;
    std::reverse(lst.begin(), lst.end());  // 可以：双向迭代器
    // std::sort(lst.begin(), lst.end());  // 不可以：缺少随机访问能力
    lst.sort();                           // 可以：链表成员函数

    std::forward_list<int> fl = {3, 1, 2};
    // std::reverse(fl.begin(), fl.end()); // 不可以：迭代器不能后退
    fl.reverse();                         // 可以：单链表成员函数
    fl.sort();                            // 可以：单链表成员函数
}
```

### 3.3 为什么要求双向，也可以传随机访问迭代器？

**“要求双向迭代器”表示至少具备双向能力。** 随机访问迭代器具备双向迭代器的全部能力，因此也满足要求。

下面用主要移动操作展示能力如何逐层增加（完整类别还包含其他语义和复杂度要求）：

```text
前向迭代器：     ++
双向迭代器：     ++、--
随机访问迭代器： ++、--、+n、-n、下标访问、计算距离……
连续迭代器：     随机访问的全部能力，并保证元素连续存储
```

例如，`std::reverse` 从两端向中间移动，只需要用到双向移动能力。随机访问迭代器额外提供的定位能力不影响使用。

```cpp
#include <algorithm>
#include <list>
#include <vector>

int main() {
    std::list<int> lst = {1, 2, 3};
    std::vector<int> v = {1, 2, 3};

    std::reverse(lst.begin(), lst.end()); // 可以：具备双向能力
    std::reverse(v.begin(), v.end());     // 可以：也具备双向能力

    std::sort(v.begin(), v.end());        // 可以：具备随机访问能力
    // std::sort(lst.begin(), lst.end()); // 不可以：双向能力不足以满足随机访问要求
}
```

| 算法要求的最低类别 | 可满足类别要求的迭代器 |
|---|---|
| 前向 | 前向、双向、随机访问、连续 |
| 双向 | 双向、随机访问、连续 |
| 随机访问 | 随机访问、连续 |
| 连续 | 连续 |

这表示**能力覆盖关系**：传入类型提供的能力应当覆盖算法需要的能力，不要求两个类别名称完全一致，也不要求具体迭代器类之间存在继承关系。

上述表格只判断类别要求；`std::reverse` 还要求元素可以交换，`std::sort` 还要求元素能够移动、赋值、交换，并满足比较要求。

### 3.4 类别匹配，还要检查能否修改元素

`set` 的迭代器属于双向迭代器，但不能通过它修改元素，所以不能用于 `std::reverse`。否则交换元素会破坏容器维护的顺序。

类似地，`vector<int>::const_iterator` 具有随机访问能力，但不能用于 `std::sort`，因为排序需要修改元素。

下面以普通可修改容器、元素类型为 `int` 为例：

| 容器 | `std::find` | `std::reverse` | `std::sort` |
|---|---|---|---|
| `vector<int>` | 可以 | 可以 | 可以 |
| `deque<int>` | 可以 | 可以 | 可以 |
| `list<int>` | 可以 | 可以 | 不可以，使用成员函数 `sort()` |
| `forward_list<int>` | 可以 | 不可以，使用成员函数 `reverse()` | 不可以，使用成员函数 `sort()` |
| `set<int>` | 可以 | 不可以，元素不能交换 | 不可以，类别与元素修改要求均不满足 |

`set` 查找通常优先使用 `s.find(value)`，复杂度为 O(log n)；`std::find(s.begin(), s.end(), value)` 是逐个查找，复杂度为 O(n)。

判断算法是否适用时，可以依次检查：

1. 算法要求哪种迭代器类别？
2. 容器提供的迭代器是否具备对应能力？
3. 元素是否支持算法要求的读取、比较、移动或交换？
4. 传入的范围和迭代器是否有效？

## 4. 常见误区

- **迭代器不一定是指针。** 它也可以是重载了 `*`、`++` 等运算符的类对象。
- **支持 `++it` 不代表支持 `it + n`。** `list` 只能逐步移动。
- **`const_iterator` 不代表迭代器本身不能移动。** 它限制的是通过迭代器修改元素。
- **`end()` 不是最后一个元素。** 它是尾后位置，不能解引用。
- **迭代器不是一直有效的。** 容器的插入、删除或扩容可能使迭代器失效，具体规则取决于容器和操作。

平时读代码，先看迭代器来自哪个容器，再判断该容器提供的能力；编写泛型代码时，可以用类型特征或 C++20 概念进行编译期检查。
