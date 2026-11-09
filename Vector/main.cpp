#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX        // 避免 windows.h 定义 min/max 宏，和 <algorithm> 打架
#endif
#include <windows.h>
#endif

#include "example/vector.h"

// ---------------------------------------------------------------- 公共工具
namespace
{

// 每段演示的标题
void title(int no, const char* text)
{
    std::cout << "\n== " << no << ". " << text << " ==\n";
}

// 把容器状态打出来，方便对照每次操作的结果。
// 参数用 const 引用：既能读，又不用复制整个容器。
template<class T>
void print(const char* tag, const XiYue::vector<T>& v)
{
    std::cout << tag << "  size=" << v.size()
              << " cap=" << v.capacity() << "  [";
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        if (it != v.begin())
        {
            std::cout << ", ";
        }
        std::cout << *it;
    }
    std::cout << "]\n";
}

} // namespace

// ---------------------------------------------------------------- 各段演示
// 每段都自己准备数据，单独跑、连着跑都不受影响。

void demo_01_basic()
{
    title(1, "构造与基本状态");
    XiYue::vector<int> v;
    std::cout << "size=" << v.size()
              << " capacity=" << v.capacity()
              << " empty=" << v.empty() << '\n';
}

void demo_02_push_back()
{
    title(2, "push_back 与扩容");
    // 初始容量为 0，第一次插入给 4，之后每次翻倍：4 -> 8 -> 16
    XiYue::vector<int> v;
    for (int i = 1; i <= 5; ++i)
    {
        v.push_back(i * 10);
        std::cout << "push_back(" << i * 10 << ") -> size=" << v.size()
                  << " cap=" << v.capacity() << '\n';
    }
    print("v", v);
}

void demo_03_subscript()
{
    title(3, "下标访问");
    // 非 const 版本返回 T&，可以就地修改；const 版本返回 const T&
    XiYue::vector<int> v{11, 20, 30, 40, 50};
    const XiYue::vector<int>& cv = v;

    v[0] = 12;                              // 通过非 const 引用修改
    std::cout << "v[0]=" << v[0]
              << " cv[0]=" << cv[0]         // 只读版本
              << " v[4]=" << v[4] << '\n';
}

void demo_04_iterate()
{
    title(4, "遍历");
    XiYue::vector<int> v{11, 20, 30, 40, 50};

    // 迭代器就是 T*，支持 ++、*、!=
    std::cout << "迭代器: ";
    for (XiYue::vector<int>::iterator it = v.begin(); it != v.end(); ++it)
    {
        std::cout << *it << ' ';
    }

    std::cout << "\n范围for: ";
    for (int x : v)
    {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

void demo_05_insert()
{
    title(5, "insert");
    XiYue::vector<int> v{11, 20, 30, 40, 50};

    // 返回指向新元素位置的迭代器；尾部插入等价于 push_back
    auto it1 = v.insert(v.begin(), 0);
    std::cout << "头部插 0   -> 返回位置的值 " << *it1 << '\n';

    auto it2 = v.insert(v.begin() + 3, 33);
    std::cout << "中间插 33  -> 返回位置的值 " << *it2 << '\n';

    auto it3 = v.insert(v.end(), 99);
    std::cout << "尾部插 99  -> 返回位置的值 " << *it3 << '\n';

    v.insert(v.end(), 2, 7);            // 一次插 2 个 7
    v.insert(v.begin() + 1, 0, 123);    // n == 0：什么都不插
    print("insert后", v);
}

void demo_06_erase()
{
    title(6, "erase");
    XiYue::vector<int> v{0, 11, 20, 33, 30, 40, 50, 99, 7, 7};

    auto e1 = v.erase(v.begin());       // 删头，返回新首元素的位置
    std::cout << "删除首元素后，返回值指向 " << *e1 << '\n';

    auto e2 = v.erase(v.end() - 1);     // 删尾，返回新的 end()
    std::cout << "删除尾元素后，返回值 == end() : " << (e2 == v.end()) << '\n';

    v.erase(v.begin() + 1, v.begin() + 3);   // 删半开区间 [1, 3)
    print("erase后", v);
}

void demo_07_pop_back()
{
    title(7, "pop_back");
    XiYue::vector<int> v{10, 20, 30};

    // 容量不会跟着缩，便于继续复用这块空间
    while (!v.empty())
    {
        std::cout << "pop_back: back=" << v[v.size() - 1]
                  << " -> size=" << v.size() - 1
                  << " cap=" << v.capacity() << '\n';
        v.pop_back();
    }
    std::cout << "empty=" << v.empty() << " size=" << v.size() << '\n';
}

void demo_08_string()
{
    title(8, "XiYue::vector<std::string>");
    // 同一个模板，T 换成 std::string 后自动生成另一份代码
    XiYue::vector<std::string> s;
    s.push_back("alpha");
    s.push_back("gamma");
    s.insert(s.begin() + 1, "beta");
    s.insert(s.end(), 2, "tail");
    print("s", s);

    s.erase(s.begin() + 1, s.begin() + 3);
    s.pop_back();
    print("s", s);
}

void demo_09_erase_loop()
{
    title(9, "边遍历边删除");
    XiYue::vector<int> v{1, 2, 3, 4, 4, 5, 4};

    auto it = v.begin();
    while (it != v.end())
    {
        if (*it % 2 == 0)
        {
            // erase 会让迭代器失效，必须接住返回值，不能再 ++it
            it = v.erase(it);
        }
        else
        {
            ++it;
        }
    }
    print("删掉所有偶数", v);
}

void demo_10_constructors()
{
    title(10, "各种构造方式");

    XiYue::vector<int> fill(5);              // 圆括号：n 个默认值
    print("fill(5)      ", fill);

    XiYue::vector<int> fill2(5, 10);         // 圆括号：n 个指定值
    print("fill(5, 10)  ", fill2);

    XiYue::vector<int> init{1, 2, 3};        // 花括号：逐个给元素
    print("init{1,2,3}  ", init);

    XiYue::vector<int> init2 = {4, 5};       // 同样是花括号，只是 2 个元素
    print("= {4, 5}     ", init2);

    int arr[] = {7, 8, 9};                   // 迭代器区间：原生数组
    XiYue::vector<int> fromArr(arr, arr + 3);
    print("(arr, arr+3) ", fromArr);

    std::vector<int> sv{6, 5};               // 迭代器区间：别人的容器
    XiYue::vector<int> fromStd(sv.begin(), sv.end());
    print("(std 区间)   ", fromStd);

    XiYue::vector<int> copied = init;        // 拷贝构造
    print("copied = init", copied);
}

void demo_11_clear_resize()
{
    title(11, "clear 与 resize");
    XiYue::vector<int> v{1, 2, 3};
    v.reserve(8);                            // 先把容量撑大，方便观察变化
    print("初始        ", v);

    v.resize(6);                             // 变大：用 T() 补 0
    print("resize(6)   ", v);

    v.resize(9, 7);                          // 变大：用 7 补齐
    print("resize(9, 7)", v);

    v.resize(2);                             // 变小：直接从尾部截断
    print("resize(2)   ", v);

    v.clear();                               // 清空：size 归零，容量不变
    print("clear()     ", v);

    v.push_back(42);                         // 容量还在，直接复用这块存储
    print("再 push 42  ", v);
}

void demo_12_copy_move_swap()
{
    title(12, "拷贝、移动与交换");

    XiYue::vector<int> src{1, 2, 3};
    XiYue::vector<int> copy = src;           // 拷贝构造：各自一块内存
    copy[0] = 99;
    print("src         ", src);
    print("copy = src  ", copy);

    XiYue::vector<int> dst{7, 7};
    dst = src;                               // 拷贝赋值：内部用拷贝并交换
    dst[1] = 88;
    print("dst = src   ", dst);
    print("src 仍不变 ", src);

    src = src;                               // 自赋值：不崩，内容不变
    print("src = src   ", src);

    XiYue::vector<int> moved = std::move(copy);   // 移动构造：接管缓冲
    print("moved       ", moved);
    print("copy 已空   ", copy);

    XiYue::vector<int> other{4};
    moved.swap(other);                       // 成员 swap
    print("swap 后 moved", moved);
    print("swap 后 other", other);
}

// ---------------------------------------------------------------- 入口
namespace
{

struct Demo
{
    const char* name;
    void (*run)();
};

const Demo demos[] = {
    {"构造与基本状态", demo_01_basic},
    {"push_back 与扩容", demo_02_push_back},
    {"下标访问", demo_03_subscript},
    {"遍历", demo_04_iterate},
    {"insert", demo_05_insert},
    {"erase", demo_06_erase},
    {"pop_back", demo_07_pop_back},
    {"vector<string>", demo_08_string},
    {"边遍历边删除", demo_09_erase_loop},
    {"各种构造方式", demo_10_constructors},
    {"clear 与 resize", demo_11_clear_resize},
    {"拷贝、移动与交换", demo_12_copy_move_swap},
};

const int demoCount = static_cast<int>(sizeof(demos) / sizeof(demos[0]));

} // namespace

int main(int argc, char* argv[])
{
#ifdef _WIN32
    // Windows 控制台默认按 GBK(936) 解码输出，而源码里的中文字面量是 UTF-8，
    // 不切一下代码页就会显示成「鏋勯€犱笌...」这种乱码。
    // 这里把输出代码页切到 UTF-8，和源码、可执行文件里的字节保持一致。
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 带参数：只跑指定的那一段（编号从 1 开始）
    if (argc > 1)
    {
        const int index = std::atoi(argv[1]);
        if (index >= 1 && index <= demoCount)
        {
            demos[index - 1].run();
            return 0;
        }

        std::cout << "用法: " << argv[0] << " [1-" << demoCount << "]\n";
        std::cout << "可用编号:\n";
        for (int i = 0; i < demoCount; ++i)
        {
            std::cout << "  " << (i + 1) << ". " << demos[i].name << '\n';
        }
        return 1;
    }

    // 不带参数：按顺序全部跑一遍
    for (int i = 0; i < demoCount; ++i)
    {
        demos[i].run();
    }
    return 0;
}
