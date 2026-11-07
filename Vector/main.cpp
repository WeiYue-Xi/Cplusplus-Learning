#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "example/vector.h"

// 把容器的当前状态打印出来，方便对照每次操作的结果。
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

// 同名重载：让下面的 std::vector 对照实验也能复用这段打印逻辑
void print(const char* tag, const std::vector<int>& v)
{
    std::cout << tag << "  size=" << v.size() << "  [";
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

int main()
{
    // ---------- 1. 构造 ----------
    std::cout << "== 1. 构造与基本状态 ==\n";
    XiYue::vector<int> v;
    std::cout << "size=" << v.size()
              << " capacity=" << v.capacity()
              << " empty=" << v.empty() << '\n';

    // ---------- 2. push_back ----------
    // 初始容量为 0，第一次插入给 4，之后每次翻倍：4 -> 8 -> 16
    std::cout << "\n== 2. push_back 与扩容 ==\n";
    for (int i = 1; i <= 5; ++i)
    {
        v.push_back(i * 10);
        std::cout << "push_back(" << i * 10 << ") -> size=" << v.size()
                  << " cap=" << v.capacity() << '\n';
    }
    print("v", v);

    // ---------- 3. operator[] ----------
    // 非 const 版本返回 T&，可以就地修改；const 版本返回 const T&
    std::cout << "\n== 3. 下标访问 ==\n";
    v[0] = 11;
    const XiYue::vector<int>& cv = v;
    std::cout << "v[0]=" << v[0] << " cv[0]=" << cv[0] << " v[4]=" << v[4] << '\n';

    // ---------- 4. 迭代器 ----------
    // 迭代器就是 T*，所以支持 ++、*、!= 这些指针操作
    std::cout << "\n== 4. 遍历 ==\n";
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

    // ---------- 5. insert ----------
    // 返回指向新元素位置的迭代器；尾部插入等价于 push_back
    std::cout << "\n== 5. insert ==\n";
    auto it1 = v.insert(v.begin(), 0);
    std::cout << "头部插 0   -> 返回位置的值 " << *it1 << '\n';

    auto it2 = v.insert(v.begin() + 3, 33);
    std::cout << "中间插 33  -> 返回位置的值 " << *it2 << '\n';

    auto it3 = v.insert(v.end(), 99);
    std::cout << "尾部插 99  -> 返回位置的值 " << *it3 << '\n';

    v.insert(v.end(), 2, 7);            // 一次插 2 个 7
    v.insert(v.begin() + 1, 0, 123);    // n == 0：什么都不插
    print("insert后", v);

    // ---------- 6. erase ----------
    std::cout << "\n== 6. erase ==\n";
    auto e1 = v.erase(v.begin());       // 删头，返回新首元素的位置
    std::cout << "删除首元素后，返回值指向 " << *e1 << '\n';

    auto e2 = v.erase(v.end() - 1);     // 删尾，返回新的 end()
    std::cout << "删除尾元素后，返回值 == end() : " << (e2 == v.end()) << '\n';

    v.erase(v.begin() + 1, v.begin() + 3);   // 删半开区间 [1, 3)
    print("erase后", v);

    // ---------- 7. pop_back ----------
    // 容量不会跟着缩，便于继续复用这块空间
    std::cout << "\n== 7. pop_back ==\n";
    while (!v.empty())
    {
        std::cout << "pop_back: back=" << v[v.size() - 1]
                  << " -> size=" << v.size() - 1
                  << " cap=" << v.capacity() << '\n';
        v.pop_back();
    }
    std::cout << "empty=" << v.empty() << " size=" << v.size() << '\n';

    // ---------- 8. 换成带资源的类型 ----------
    // 同一个模板，T 换成 std::string 后自动生成另一份代码
    std::cout << "\n== 8. XiYue::vector<std::string> ==\n";
    XiYue::vector<std::string> s;
    s.push_back("alpha");
    s.push_back("gamma");
    s.insert(s.begin() + 1, "beta");
    s.insert(s.end(), 2, "tail");
    print("s", s);

    s.erase(s.begin() + 1, s.begin() + 3);
    s.pop_back();
    print("s", s);

    // ---------- 9. 与 std::vector 对照 ----------
    std::cout << "\n== 9. std::vector 删除偶数 ==\n";
    std::vector<int> v1;
    v1.push_back(1);
    v1.push_back(2);
    v1.push_back(3);
    v1.push_back(4);
    v1.push_back(4);
    v1.push_back(5);
    v1.push_back(4);

    auto it = v1.begin();
    while (it != v1.end())
    {
        if (*it % 2 == 0)
        {
            //erase也会迭代器失效，失效的迭代器就不能再使用了
            v1.erase(it);
        }
        else
        {
            ++it;
        }
    }

    print("v1", v1);

    return 0;
}
