# 内存管理

## 一、C++内存管理方式

### 1、对象的四种存储期

| 存储期     | 常见例子                | 生命周期                           |
| ---------- | ----------------------- | ---------------------------------- |
| 自动存储期 | 普通局部变量            | 执行到声明时创建，离开作用域时销毁 |
| 静态存储期 | 全局变量、`static` 变量 | 存储空间贯穿程序运行期间           |
| 线程存储期 | `thread_local` 变量     | 每个线程各有一份，随线程结束而销毁 |
| 动态存储期 | 通过 `new` 创建的对象   | 由释放操作决定何时结束             |

### 2、new/delete操作内置类型

***需要手动管理动态对象时，可以使用`new`和`delete`。***

```c++
int* p = new int(10);  // 分配内存，并初始化为 10

std::cout << *p << '\n';

delete p;             // 释放动态对象
p = nullptr;
```

p是局部变量，但它指向的对象具有动态存储期。***<u>局部指针离开作用域，不会自动删除它指向的对象。</u>***

```c++
void test() {
    int* p = new int(10);
} // p 消失了，但动态对象没有被释放：内存泄漏
```

对于类对象，`new`和`delete`还负责构造和析构，在申请自定义类型的空间时，new会调用构造函数，delete会调用析构函数，而malloc与free不会：

```c++
Student* p = new Student(18); // 分配内存，调用构造函数

delete p;                    // 调用析构函数，释放内存

int* a = new int;     // 没有初始化为确定值，不能直接读取
int* b = new int();   // 初始化为 0
int* c = new int{10}; // 初始化为 10

delete a;
delete b;
delete c;
```

***动态数组需要成对使用`new[]`和`delete[]`。***

```c++
int* data = new int[5]{}; // 5 个 int，全部初始化为 0

data[0] = 100;

delete[] data;
data = nullptr;
```

| 分配方式      | 释放方式    |
| ------------- | ----------- |
| `new T`       | `delete`    |
| `new T[n]`    | `delete[]`  |
| `malloc(...)` | `free(...)` |

动态数组通常优先写成：

```c++
std::vector<int> data(5, 0);
data[0] = 100;
```

这样可以自动释放，还能查询大小和调整容量。

### 3、operator new与operator delete函数

`operator new`和`operator delete`是系统提供的全局函数，`new`在底层调用`operator new`全局函数来申请空间，`delete`在底层通过`operator delete`全局函数来释放空间。

<u>***`operator new`不负责调用构造函数，`operator delete`不负责调用析构函数***。</u>

| 操作                   | 分配／释放内存 | 构造／析构对象 |
| ---------------------- | -------------- | -------------- |
| `new T(...)`           | 分配           | 构造           |
| `operator new(...)`    | 分配           | 不构造         |
| `delete p`             | 释放           | 析构           |
| `operator delete(...)` | 释放           | 不析构         |

***类可以定义自己的`operator new`和`operator delete`，控制该类动态对象的内存分配方式。***

```c++
#include <cstddef>
#include <iostream>
#include <new>

class Example {
public:
    Example() {
        std::cout << "构造对象\n";
    }

    ~Example() {
        std::cout << "析构对象\n";
    }

    static void* operator new(std::size_t size) {
        std::cout << "分配内存：" << size << " 字节\n";

        return ::operator new(size);
        //表示交给全局分配函数完成实际分配。这里::很重要，否则不加限定的调用可能在此调用类自身的函数，造成递归。
    }

    static void operator delete(void* ptr) noexcept {
        std::cout << "释放内存\n";

        ::operator delete(ptr);
    }
};

int main() {
    Example* p = new Example;
    delete p;
}
```

类的`operator new`、`operator delete`都是静态成员函数，即使省略`static`，它们也任然是静态的·。它们没有`this`指针，因为分配内存时，对象还没有构造完成。

它们只影响相应的动态分配操作：

```c++
Example a;                 // 不调用类的 operator new
Example* p = new Example;  // 调用类的 operator new
delete p;
```

 ***如果构造函数抛出异常，已经分配的内存怎么办？***

```c++
#include <stdexcept>

class Test {
public:
    Test() {
        throw std::runtime_error("构造失败");
    }
};

Test* p = new Test;
```

```
operator new 分配内存成功
构造函数执行
构造函数抛出异常
调用匹配的 operator delete 释放内存
异常继续向外传播
```

在普通`new`的情况下，不需要自己再释放那块内存。

但是因为`Test`对象没有构造完成，不会调用`Test`自身的析构函数，已经构造完成的成员和基类成员部分会被销毁。

数组有对应的`operator new[]`和`operator delete[]`。

```c++
Example* p = new Example[3];
delete[] p;

void* operator new[](std::size_t size);
void operator delete[](void* ptr) noexcept;
```

```
operator new[] 分配数组所需内存
依次构造 3 个元素

逆序析构 3 个元素
operator delete[] 释放内存
```

> [!CAUTION]
>
> - `operator new[]`通常是为这个数组分配一次内存，并非每个元素调用一次。
> - 申请的字节数可能包括数组管理所需的额外空间，不一定恰好等于元素个数*sizeof(T)。
> - 为类定义`operator new`，不会自动定义`operator new[]`，它们是不同的函数。

### 4、new和delete的实现原理

#### Ⅰ、内置类型

如果申请的是内置类型的空间，`new`和`malloc`，`delete`和`free`基本类似，不同的地方是：`new`/`delete`申请和释放的是单个元素的空间，`new[]`和`delete[]`申请的是连续空间，而且`new`在申请空间失败时会抛异常，`malloc`会返回`NULL`。

#### Ⅱ、自定义类型

- `new`的原理

  调用`operator new`函数分配内存 --> 构造对象 --> 返回对象指针

- `delete`的原理

  析构对象 --> 调用`operator delete`函数释放内存

- `new T[N]`的原理

  调用选中的`operator new[]`分配函数，申请N个元素及必要数组前缀的空间 --> 准备必要的数组信息 --> 按顺序构造N个元素 --> 返回首元素指针。默认的全局`operator new[]`会转调对应的`operator new`；用户替换或类专用版本可以采用其他分配策略。

- `delete[]`的原理

  逆序析构N个元素，清理各元素管理的资源 --> 按实现需要将首元素地址调整回原始分配地址 --> 调用选中的`operator delete[]`释放整块空间。默认的全局版本会按规则转调对应的`operator delete`。

#### Ⅲ、operator new[]与operator delete[]的底层机制

`operator new[]`和`operator delete[]`负责整块原始内存的分配与释放。数组元素的构造、析构，以及必要的元素个数记录和指针调整，由编译器为`new[]`、`delete[]`表达式生成的代码完成。

| 操作 | 职责 |
| ---- | ---- |
| `new T[n]` | 申请内存，准备必要的数组信息，构造元素，返回首元素指针 |
| `operator new[](bytes)` | 按字节数申请原始内存，不构造元素 |
| `delete[] p` | 逆序析构元素，按实现需要调整地址，调用释放函数 |
| `operator delete[](raw)` | 释放整块原始内存，不析构元素 |

**1、数组元素个数如何记录：array cookie**

对于需要逐个执行析构函数的类型，常见实现会在数组元素之前预留额外空间，记录元素个数。这段信息通常称为`array cookie`。以下是一种常见ABI布局，不是所有平台都必须采用的固定格式。

```c++
T* p = new T[3];
```

假设`sizeof(T) == 8`，数组前缀占8字节，其中保存元素个数：

```text
地址：  0x1000        0x1008      0x1010      0x1018
        │             │
        ▼             ▼
     ┌─────────────┬──────────┬──────────┬──────────┐
     │ 元素个数：3 │   T[0]   │   T[1]   │   T[2]   │
     └─────────────┴──────────┴──────────┴──────────┘
        ▲             ▲
        raw           p
```

- `raw`是`operator new[]`返回的原始分配地址。
- `p`是`new T[3]`返回的首元素地址。
- 在本例中，总申请量为`8 + 3 * 8 == 32`字节，`p`比`raw`向后偏移8字节。

因此，**`new[]`表达式返回的地址，可能不等于`operator new[]`返回的地址。**

前缀可能还包含对齐填充，其大小不一定是4或8字节。对于`int`等不需要执行析构函数的类型，在不需要额外大小信息时，常见实现可以省略cookie，此时`p`可以等于`raw`。

**2、new[]与delete[]分别生成什么代码**

下面是带cookie布局的概念性伪代码，表示职责划分，不是可以直接复制执行的通用实现：

```text
new T[n]：
    计算并检查总字节数：prefix_size + n * sizeof(T)
    raw = 调用选中的 operator new[](总字节数)
    在预留位置记录 n
    p = raw 向后偏移 prefix_size 字节
    按顺序构造 p[0]、p[1]、……、p[n - 1]
    返回 p

delete[] p（p 非空）：
    从约定位置取得 n
    逆序析构 p[n - 1]、……、p[1]、p[0]
    raw = 将 p 向前调整，恢复原始分配地址
    调用选中的 operator delete[](raw)
```

若构造某个元素时抛出异常，已经构造成功的元素会被逆序销毁，再调用匹配的释放函数释放整块内存，然后继续传播异常。构造失败的那个元素本身不会调用自身的析构函数。

某些释放函数重载还接收字节数、对齐值等参数，由调用代码按规则提供。`operator new[]`本身接收的是字节数，不是元素个数；它仅凭这个参数不能知道元素类型，也不负责逐个调用构造函数。

**3、new[]配delete：不只是少调用几次析构函数**

```c++
T* p = new T[3];
delete p; // 错误：未定义行为，应使用 delete[] p
```

在前述带cookie的常见实现中，可能出现两个问题：

1. 单对象删除没有数组析构循环，可能只析构`p[0]`，其他元素管理的资源没有得到清理。
2. 单对象删除通常不会执行数组前缀的地址调整，可能把`0x1008`交给释放函数，但原始分配地址是`0x1000`。释放内存块中间的地址，可能造成非法释放、堆损坏或程序崩溃。

这些是对常见实现故障原因的解释。**标准直接规定不匹配的删除属于未定义行为，不保证一定只析构一个元素，也不保证一定崩溃。**

“`new[]`配`delete`只会释放第一个元素的内存”是不准确的：数组通常是整块分配的，元素不是分别申请的独立内存块。错误的删除可能漏掉析构，也可能根本无法正确释放整块内存。

**4、为什么内置类型有时看起来可以混用**

```c++
int* p = new int[3];
delete p; // 仍然是未定义行为，即使表面上运行正常
```

`int`没有需要执行的析构函数，常见实现可能不保存cookie，使`p == raw`；默认数组分配和释放函数还可能与单对象版本共用底层分配器。因此错误代码有时看起来能够正常释放。

但这只是某种实现上的偶然结果，不能作为正确性的依据。即使数组只有一个元素，也必须匹配：

```c++
int* p = new int[1];
delete[] p; // 正确，不能因为只有一个元素就改用 delete
```

**5、new配delete[]：可能把其他内容误读成元素个数**

```c++
T* p = new T;
delete[] p; // 错误：未定义行为，应使用 delete p
```

如果当前类型的数组删除代码需要cookie，它可能读取`p`之前的内容，将其误当成元素个数，再按错误的数量执行析构，并把错误调整后的地址交给释放函数，造成越界访问、非法释放等问题。

内存分配器自己的管理信息与数组cookie是两层不同的信息：

```text
[分配器管理信息] [数组前缀/cookie] [T0][T1][T2]
                 ↑                ↑
                 raw              p
```

分配器知道内存块的容量，不代表知道其中有几个C++对象、应调用哪个析构函数。分配器还可能对容量取整，因此也不能仅用块容量除以`sizeof(T)`来可靠恢复元素个数。上图同样只是示意，分配器元数据不一定实际放在块的前面。

**6、直接调用operator delete[]不等于delete[]**

```c++
T* p = new T[3];

// ::operator delete[](p); // 不能代替 delete[] p
delete[] p;               // 逆序析构元素，并传递正确的释放地址
```

直接调用函数不会执行数组析构逻辑；对于带前缀的布局，`p`也不是底层分配函数返回的原始地址。

直接分配原始存储空间时，应把原始返回地址交给对应释放函数：

```c++
#include <new>

void* raw = ::operator new[](100); // 申请原始存储空间

// 此处没有通过 new T[n] 创建一组需要析构的 T 元素

::operator delete[](raw);         // 释放原始存储空间
```

默认的全局`operator new[]`会转调对应的`operator new`，默认的全局`operator delete[]`也会按规则转调对应的`operator delete`；用户可以替换全局版本，或为类定义专用版本。**底层可能共用分配器，不代表`new`、`new[]`与两种删除表达式可以混用。**

### 5、定位new表达式

定位new（placement new）是在一块已经存在的存储空间中构造对象，将“申请内存”和“构造对象”两个步骤分开。

```c++
Student* p = new Student(18);           // 申请内存，再构造对象
Student* q = ::new (address) Student(18); // 在 address 指定的位置构造对象
```

第二行要求`address`已经指向足够大、对齐正确且仍然有效的存储空间。这里讨论的是标准的、接收`void*`地址的非分配形式；自定义的其他定位分配函数可以有不同的行为。

#### Ⅰ、语法与完整示例

> 格式：new (place_address) type或者new (place_address) type(initializer-list)
>
> place_address必须是一个指针，initializer-list是类型的初始化列表

```c++
#include <iostream>
#include <new>

class Student {
private:
    int age;

public:
    explicit Student(int a)
        : age(a)
    {
        std::cout << "构造 Student\n";
    }

    ~Student() {
        std::cout << "析构 Student\n";
    }

    void show() const {
        std::cout << "年龄：" << age << '\n';
    }
};

int main() {
    // 准备大小、对齐均满足 Student 要求的存储空间
    alignas(Student) unsigned char buffer[sizeof(Student)];

    // 在已有空间中构造对象，不再申请新内存
    Student* p = ::new (static_cast<void*>(buffer)) Student(18);

    p->show();

    // 销毁 Student 对象，但保留 buffer 的存储空间
    p->~Student();

} // buffer 的存储空间随作用域结束而自动回收
```

输出：

```text
构造 Student
年龄：18
析构 Student
```

- `sizeof(Student)`保证缓冲区足够大。
- `alignas(Student)`保证缓冲区的起始地址满足对齐要求。
- `::new`指定从全局作用域查找分配函数，避免类中同名分配函数影响查找。
- `static_cast<void*>(buffer)`将缓冲区地址明确转换为标准定位分配函数所接收的地址类型。

**拥有足够的存储空间，不等于已经构造完成一个Student对象。定位new会在指定位置完成对象的构造。**

#### Ⅱ、底层实现原理

标准定位分配函数声明在`<new>`中，形式为：

```c++
void* operator new(std::size_t size, void* address) noexcept;
```

这个标准函数直接返回传入的`address`，不申请额外的内存，也不调用构造函数。其行为可以概念性地理解为`return address;`，不需要用户重新定义它。

```text
::new (address) Student(18)
    ↓
调用 ::operator new(sizeof(Student), address)
    ↓
得到原来的 address
    ↓
在该地址上初始化基类、成员，并执行 Student 的构造函数体
    ↓
返回指向已构造对象的 Student*
```

因此，应区分两者的职责：

| 操作 | 职责 |
| ---- | ---- |
| 标准定位分配函数`operator new(size, address)` | 返回给定地址，不分配内存、不构造对象 |
| 定位new表达式`::new (address) T(args...)` | 调用分配函数，在返回的位置构造对象 |

标准定位分配函数不会检查空间是否足够，也不会自动修正对齐。大小、对齐和存储空间的有效性由调用方保证。

#### Ⅲ、对象析构与存储空间释放

对于前面局部缓冲区中的对象，只应显式执行析构：

```c++
p->~Student();
```

这会执行析构过程并结束对象的生命周期，但不会释放`buffer`的存储空间。对象销毁后，不能继续通过`p`访问这个已经结束生命周期的Student对象；之后可以按规则在该空间中重新构造对象。

不能把这个指针直接交给普通`delete`：

```c++
// delete p; // 错误：buffer 不是普通动态分配得到的内存块
```

普通`delete`会安排对象析构和内存释放，但局部数组不能交给普通动态内存释放函数处理。**定位new本身不决定存储空间如何回收，存储空间必须按其来源管理。**

| 存储空间来源 | 对象的清理 | 存储空间的处理 |
| ------------ | ---------- | -------------- |
| 局部缓冲区 | 显式析构 | 随缓冲区作用域结束回收 |
| `::operator new` | 显式析构 | 调用匹配的`::operator delete` |
| 内存池 | 按设计析构 | 按内存池规则归还或统一回收 |

#### Ⅳ、先动态分配内存，再定位构造

沿用前面的Student类，可以把申请、构造、析构和释放明确拆开：

```c++
void test() {
    void* memory = ::operator new(sizeof(Student));
    Student* p = nullptr;

    try {
        p = ::new (memory) Student(18);
    }
    catch (...) {
        // 构造失败，释放先前申请的原始存储空间
        ::operator delete(memory);
        throw;
    }

    // 对象构造成功后，在此使用对象

    p->~Student();             // 销毁对象
    ::operator delete(memory); // 释放原始存储空间
}
```

```text
::operator new       申请存储空间
定位new表达式         构造对象
显式调用析构函数      销毁对象
::operator delete    释放存储空间
```

该示例中的Student使用普通对齐。对于过度对齐类型，还需要选择匹配的对齐分配与释放方式。

#### Ⅴ、构造失败时的处理

若定位new调用的构造函数抛出异常，已经构造完成的成员和基类部分会被清理，但未构造完成的对象本身不会调用自身的析构函数。

与标准定位分配函数匹配的定位释放函数形式为：

```c++
void operator delete(void* ptr, void* address) noexcept;
```

**这个标准定位释放函数不执行释放操作。** 它用于构造失败时的匹配清理，不是对象正常使用完成后自动调用的释放机制。

因此，上面的动态内存示例需要在`catch`中显式调用`::operator delete(memory)`；局部缓冲区示例则由缓冲区自身的生命周期管理存储空间。

若定位构造成功后，后续操作仍可能抛出异常，实际代码还应通过RAII等方式保证对象析构和存储空间回收，不能只保护构造这一步。

#### Ⅵ、常见用途

定位new常用于内存池、容器实现，以及需要复用存储空间的底层代码。例如，容器可以先申请一定容量的原始存储，再按需逐个构造元素，而不是申请容量时就构造全部元素。

现代C++还提供了表达对象构造与销毁的工具：

- C++20的`std::construct_at`：在指定位置构造对象。
- C++17的`std::destroy_at`：销毁指定对象。

它们都定义在`<memory>`中，不负责自动回收底层存储空间。使用这些工具时，仍然需要正确管理空间大小、对齐和生命周期。

### 6、malloc/free和new/delete的区别

`malloc/free`主要管理原始存储空间；普通`new/delete`表达式把存储空间的分配、释放与对象的初始化、销毁结合起来。比较时要注意：`new`表达式不等于`operator new`函数，`delete`表达式也不等于`operator delete`函数。

#### Ⅰ、主要区别对照

| 比较项 | malloc/free | 普通new/delete表达式 |
| ------ | ----------- | -------------------- |
| 语言形式 | 标准库函数，C++中通过`<cstdlib>`使用`std::malloc/std::free` | C++语言表达式，`new/delete`是关键字 |
| 申请大小 | 显式传入字节数，如`sizeof(T) * n` | 根据目标类型和元素个数计算，如`new T`、`new T[n]` |
| 返回类型 | `malloc`返回`void*`，在C++中转换为具体指针类型需显式转换 | `new T`和一维`new T[n]`返回`T*` |
| 初始化、构造 | `malloc`不调用构造函数，不初始化所分配字节的内容 | 按初始化语法初始化对象；类对象按规则调用构造函数 |
| 析构 | `free`不调用析构函数 | `delete`按规则析构对象，`delete[]`逆序析构数组元素 |
| 分配失败 | `malloc`返回空指针 | 普通抛异常的分配形式通常抛出`std::bad_alloc` |
| 定制分配 | 不使用C++类专用分配函数机制 | 可选择类专用或被用户替换的全局`operator new/delete`等函数 |
| 数组处理 | 只申请字节块，不负责逐个构造、析构元素 | `new[]/delete[]`安排数组元素的构造、析构及必要的管理信息 |
| 对齐 | 满足基本对齐要求，不能据此保证任意过度对齐类型的要求 | 从C++17起，普通new可按规则为过度对齐类型选择带对齐参数的分配函数 |
| 调整容量 | `realloc`可用于malloc家族取得的存储块，但有使用限制 | 没有对应的`renew`，不能用`realloc`调整new分配的内存 |
| 申请自定义类型对象时 | 只会开辟空间，不会调用构造函数与析构函数 | `new`在申请空间后会调用构造函数完成对象的初始化，`delete`在释放空间前会调用析构函数完成空间中资源的清理释放 |

`new/delete`关键字和表达式本身不能被用户重新定义；可以定制的是它们选择调用的分配、释放函数。

#### Ⅱ、最关键的区别：构造与析构

```c++
#include <cstdlib>
#include <iostream>
#include <new>

class Example {
public:
    Example() {
        std::cout << "构造 Example\n";
    }

    ~Example() {
        std::cout << "析构 Example\n";
    }
};

int main() {
    Example* p = new Example; // 分配内存，并调用构造函数
    delete p;                // 调用析构函数，再释放内存

    void* memory = std::malloc(sizeof(Example));
    if (memory == nullptr) {
        throw std::bad_alloc();
    }

    // 获得了足够的存储空间，但没有执行 Example 的构造函数

    std::free(memory);       // 只释放存储空间，不调用析构函数
}
```

输出中只有一组构造、析构信息：

```text
构造 Example
析构 Example
```

把`malloc`返回的`void*`转换成`Example*`，也不会因此执行构造函数。对于需要构造的类对象，可以按第5节的方法使用定位new，在使用完成后显式析构，再调用`free`释放原始地址；若构造失败，也要正确释放先前申请的存储空间。

现代C++的对象生命周期规则允许`malloc`等操作在满足条件时隐式创建某些隐式生命周期类型的对象，但这不表示它会调用类的构造函数，更不能据此跳过需要执行的类初始化过程。

#### Ⅲ、初始化方式与失败处理

**1、new并不总是把内存初始化为0**

```c++
int* a = new int;     // 不初始化为确定值，不能直接读取
int* b = new int();   // 值初始化，结果为0
int* c = new int{};   // 结果为0
int* d = new int(10); // 结果为10

delete a;
delete b;
delete c;
delete d;
```

`malloc`也不会把分配到的字节自动清零。`calloc`会把分配空间的字节置零，但“字节全部为0”不能笼统等同于任意C++类型的正确构造或语义初始化。

**2、malloc检查空指针，普通new处理异常**

```c++
void* memory = std::malloc(100);
if (memory == nullptr) {
    // 处理分配失败
} else {
    // 使用这块存储空间
    std::free(memory);
}
```

```c++
try {
    int* p = new int(10);
    delete p;
}
catch (const std::bad_alloc&) {
    // 处理内存分配失败
}
```

需要以空指针表示分配失败时，可以使用`<new>`中的`std::nothrow`形式：

```c++
int* p = new (std::nothrow) int(10);
if (p == nullptr) {
    // 处理分配失败
}
delete p; // 对空指针执行delete是安全的
```

`std::nothrow`针对分配失败，不会吞掉对象构造函数自身抛出的异常。数组长度非法或过大时，抛异常的数组new还可能抛出`std::bad_array_new_length`。

使用`malloc(sizeof(T) * n)`分配数组空间时，还要防止字节数乘法溢出；数组new会按语言规则检查长度及相关大小限制，但仍需要保证元素个数符合业务要求。

#### Ⅳ、分配与释放必须匹配

| 分配来源 | 正确的释放方式 |
| -------- | -------------- |
| `std::malloc/std::calloc/std::realloc`获得的有效存储块 | `std::free` |
| 普通`new T` | `delete` |
| 普通`new T[n]` | `delete[]` |
| 直接调用原始分配函数 | 按对应分配协议调用匹配的释放函数 |
| 定位new构造的对象 | 按需显式析构，存储空间按其原始来源管理 |

```c++
int* p = new int(10);
// std::free(p); // 错误：分配、释放方式不匹配
delete p;

void* memory = std::malloc(sizeof(int));
// delete static_cast<int*>(memory); // 错误：不是普通new表达式创建的对象
std::free(memory);
```

**不能因为某个默认`operator new`内部可能使用`malloc`，就认为`new`和`free`可以配对。** 默认实现细节不改变语言层面的配对要求；类专用分配函数、数组前缀以及对齐方式等，也可能使释放协议不同。

同样，`new[]`配`delete`、`new`配`delete[]`都属于未定义行为，具体原因见第4节。

释放不会自动将调用方的指针置空；向`free`传入空指针，或对相应类型的空指针执行`delete`、`delete[]`，本身都是安全的，但重复释放同一个非空地址仍然是错误的。

#### Ⅴ、realloc与性能上的常见误解

`realloc`可能在原地调整存储块，也可能分配新块、保留相应字节内容并释放旧块。因此，不能用它直接搬移需要维持构造、析构或内部引用关系的复杂C++对象，也不能把`new`得到的指针传给它。

调整malloc家族的存储块时，应先用临时指针接收结果。对于非零目标大小，失败返回空指针时，原块仍然有效，直接覆盖原指针会导致失去原地址：

```c++
void* memory = std::malloc(100);
if (memory != nullptr) {
    void* resized = std::realloc(memory, 200);
    if (resized != nullptr) {
        memory = resized;
    }
    // 失败时memory仍指向原块，成功时指向调整后的块
    std::free(memory);
}
```

也不能简单认为“malloc一定比new快”。两者可能使用同一个底层分配器；类对象的构造、析构是额外的语义工作，性能还受对象类型、分配大小、分配器策略和优化影响。

实际C++代码通常优先使用局部对象、`std::vector`、`std::string`和智能指针，通过RAII管理资源；与C接口交互或实现底层存储管理时，再按接口协议选择malloc家族或原始分配函数。

## 二、配套入门代码

本目录提供 5 个简短示例，分别演示单个整数、动态数组、类对象、`malloc/free` 和定位 `new`。每个示例独立放在 `examples/` 下的 `.cpp` 文件中，`main.cpp` 负责依次调用。

建议初学者先阅读[入门示例说明](examples/入门示例说明.md)，再按顺序学习各个示例。在 CLion 中打开本目录并运行 `memory_management` 目标即可。

也可以在仓库根目录使用命令行构建（要求 CMake 3.20 及支持 C++20 的编译器）：

```powershell
cmake -S Memory_Management -B Memory_Management/build
cmake --build Memory_Management/build
./Memory_Management/build/memory_management.exe
```

Visual Studio 等多配置生成器需要在构建时加 `--config Debug`，运行 `Memory_Management/build/Debug/memory_management.exe`；Linux/macOS 的可执行文件不带 `.exe`。
