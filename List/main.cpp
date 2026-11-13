#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX            // libstdc++ 已经定义过，重复定义会报警告
#define NOMINMAX            // 防止 windows.h 里的 min/max 宏干扰 std::min/std::max
#endif
#include <windows.h>        // 只在 Windows 下需要：SetConsoleOutputCP
#endif

#include "example/list.h"

using namespace std;

// Windows 控制台默认按 GBK(936) 解释字节，而源码和输出都是 UTF-8，
// 不切换代码页时中文就会显示成「绌鸿〃」这种乱码
static void setup_console_utf8()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);    // 控制台输出改用 UTF-8
    SetConsoleCP(CP_UTF8);          // 控制台输入也改，保持一致
#endif
}

// 参数是 const 引用，函数里拿到的只能是 const_iterator，只读不改
template<class T>
void print(const XiYue::list<T>& lt, const char* tag)
{
    cout << tag << ": ";
    for (typename XiYue::list<T>::const_iterator it = lt.begin(); it != lt.end(); ++it)
    {
        cout << *it << " ";
    }
    cout << endl;
}

int main()
{
    setup_console_utf8();

    // 1. 构造函数 + push_back
    XiYue::list<int> l1;
    cout << "空表 begin() == end(): " << (l1.begin() == l1.end() ? "true" : "false") << endl;

    l1.push_back(1);
    l1.push_back(2);
    l1.push_back(3);
    print(l1, "push_back 之后");

    // 2. begin / end + iterator 正向遍历
    cout << "迭代器遍历: ";
    for (XiYue::list<int>::iterator it = l1.begin(); it != l1.end(); ++it)
    {
        cout << *it << " ";
    }
    cout << endl;

    // 3. -- 反向遍历：end() 是哨兵位，先 -- 才落到最后一个元素
    cout << "反向遍历: ";
    for (XiYue::list<int>::iterator it = l1.end(); it != l1.begin();)
    {
        --it;
        cout << *it << " ";
    }
    cout << endl;

    // 4. 范围 for：底层就是 begin() / end()
    cout << "范围for: ";
    for (int x : l1)
    {
        cout << x << " ";
    }
    cout << endl;

    // 5. 非 const 迭代器可以改元素
    for (XiYue::list<int>::iterator it = l1.begin(); it != l1.end(); ++it)
    {
        *it *= 10;
    }
    print(l1, "每个元素 *= 10 之后");

    // 6. const 对象拿到 const_iterator
    const XiYue::list<int>& cl1 = l1;
    XiYue::list<int>::const_iterator cit = cl1.begin();
    XiYue::list<int>::const_iterator last = cl1.end();
    --last;
    cout << "const_iterator 首元素: " << *cit << "，尾元素: " << *last << endl;

    // 7. 与标准算法配合：find / distance / reverse
    XiYue::list<int>::iterator pos = std::find(l1.begin(), l1.end(), 20);
    if (pos != l1.end())
    {
        cout << "find(20) 命中，值是 " << *pos << endl;
    }
    cout << "distance = " << std::distance(cl1.begin(), cl1.end()) << endl;

    std::reverse(l1.begin(), l1.end());
    print(l1, "reverse 之后");

    // 8. 元素是类类型：-> 操作符
    XiYue::list<string> names;
    names.push_back("XiYue");
    names.push_back("list");
    for (XiYue::list<string>::iterator it = names.begin(); it != names.end(); ++it)
    {
        cout << it->size() << ":" << *it << " ";
    }
    cout << endl;
    print(names, "string 表");

    // 9. 析构函数：离开作用域自动释放所有节点
    {
        XiYue::list<int> tmp;
        for (int i = 0; i < 5; ++i)
        {
            tmp.push_back(i);
        }
        cout << "局部表有 " << std::distance(tmp.begin(), tmp.end()) << " 个元素，即将离开作用域" << endl;
    }
    cout << "局部表已析构" << endl;

    return 0;
}
