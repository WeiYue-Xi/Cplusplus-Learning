//
// Created by HY on 2026/11/1.
//

#ifndef XIYUE_VECTOR_H
#define XIYUE_VECTOR_H
#include <iterator>
#include <assert.h>
#include <cstring>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace XiYue
{

template<class T>
class vector
{
private:
    // 三指针：[_start, _finish) 是已有元素，[_finish, _endofstorage) 是空余空间
    // 就地给 nullptr：这样 = default 生成的默认构造函数也是安全的，
    // 不会留下随机值（否则 size() 一算就崩）
    T* _start = nullptr;
    T* _finish = nullptr;
    T* _endofstorage = nullptr;
public:
    typedef T* iterator;
    typedef const T* const_iterator;

    // 构造函数：三个指针都指向空，得到一个 size()==0、capacity()==0 的空容器
    // vector()
    // {
    // }

    //强制编译器生成默认构造
    vector() = default;

    // 多参数构造：造 n 个 value 的副本。
    // 标准库里这是两个重载（vector(n) 与 vector(n, value)），这里用默认参数合成一个，
    // 与 resize 的做法一致：不写第二个参数就用 T() 填充。
    // explicit 是为了挡住 vector<int> v = 5; 这种隐式转换（标准库的 vector(n) 也是 explicit）。
    explicit vector(size_t n, const T& value = T())
    {
        // 一次分够 n 个槽位，下面 push_back 时不会触发扩容
        reserve(n);
        for (size_t i = 0; i < n; ++i)
        {
            push_back(value);
        }
    }

    // 初始化列表构造：把 {} 里给定的值逐个放进去。
    // 有了它，{} 会优先匹配这里，于是 vector<int> v{5} 是"1 个元素 5"，
    // 而不是走上面的填充构造变成"5 个 0"；圆括号 (5) 仍然走填充构造。
    // 不能加 explicit：否则 vector<int> v = {1, 2, 3}; 这种写法就用不了。
    vector(std::initializer_list<T> init)
    {
        reserve(init.size());
        for (const auto& e : init)
        {
            push_back(e);
        }
    }

    // 迭代器区间构造：[first, last) 左闭右开，逐个复制过来。
    // 写成模板是为了接受任意迭代器：T*、const T*、别人的 iterator 都行。
    // 第二个模板参数的作用是"把整数类型排除掉"：不加它的话，vector<int> v(5, 10)
    // 会优先匹配这里（int, int 完全匹配，比 int 转 size_t 更好），于是把 5 当成
    // 迭代器去解引用。标准库用 C++20 的 concepts 做同一件事：
    //     template<std::input_iterator InputIt> vector(InputIt, InputIt);
    template<class InputIt,
             class = typename std::enable_if<!std::is_integral<InputIt>::value>::type>
    vector(InputIt first, InputIt last)
    {
        // 不预先数距离：push_back 自己会按倍增扩容，
        // 这样单遍迭代器（input iterator）也能用
        for (InputIt it = first; it != last; ++it)
        {
            push_back(*it);
        }
    }

    // 拷贝构造：申请一块新内存，把元素逐个复制过来——深拷贝。
    // 若不写这一版，编译器生成的版本只做成员逐个拷贝（三个指针照抄），
    // 两个对象会指向同一块内存，析构时同一块内存被 delete[] 两次。
    vector(const vector<T>& other)
    {
        size_t n = other.size();
        if (n > 0)
        {
            // 按元素个数申请，而不是按 other.capacity()：
            // 容量是对方的实现细节，照搬过来只是浪费内存
            _start = new T[n];
            for (size_t i = 0; i < n; ++i)
            {
                _start[i] = other._start[i];
            }
            _finish = _start + n;
            _endofstorage = _start + n;
        }
        // other 为空时保持三个空指针，capacity() 也是 0

        // reserve(other.capacity());
        // for (auto& e : other)
        // {
        //     push_back(e);
        // }
    }

    // 移动构造：不分配、不复制，直接把对方的三个指针接过来，再把它置空。
    // noexcept 的意思是"这个函数保证不抛异常"，编译器可以据此省掉一些
    // 异常处理代码；容器扩容搬迁元素时，也只有在移动构造 noexcept 的情况下
    // 才会优先选择移动而不是拷贝。
    vector(vector<T>&& other) noexcept
            : _start(other._start), _finish(other._finish), _endofstorage(other._endofstorage)
    {
        other._start = other._finish = other._endofstorage = nullptr;
    }

    // 交换：交换的是三个指针，不是整个对象。
    // 这里可以放心用 std::swap——它交换的是指针值，不会反过来调到这个函数。
    // 若写成 std::swap(*this, other)，就会变成死循环：
    // std::swap 内部用移动构造 + 两次移动赋值，而移动赋值里又调用 swap。
    void swap(vector<T>& other) noexcept
    {
        std::swap(_start, other._start);
        std::swap(_finish, other._finish);
        std::swap(_endofstorage, other._endofstorage);
    }

    // 拷贝赋值 + 移动赋值：按值传参，一个函数顶两个。
    // 形参在进入函数之前就已经构造好：传左值走拷贝构造，传右值走移动构造。
    // 之后只做一次 swap，旧的内存在形参 other 析构时自动释放——
    // 所以既不用写 delete[]，也不用判自赋值，拷贝失败时 *this 一点没动。
    vector<T>& operator=(vector<T> other)
    {
        swap(other);
        return *this;
    }

    // [begin, end) 左闭右开，正好对应 [_start, _finish)
    iterator begin()
    {
        return _start;
    }

    iterator end()
    {
        return _finish;
    }

    // const 对象只能读，所以返回 const_iterator，防止通过迭代器改元素
    const_iterator begin() const
    {
        return _start;
    }

    const_iterator end() const
    {
        return _finish;
    }

    size_t size() const
    {
        return _finish - _start;
    }
    size_t capacity() const
    {
        return _endofstorage - _start;
    }
    bool empty() const
    {
        return _start == _finish;
    }

    void reserve(size_t n)
    {
        if (n > capacity())
        {
            T* tmp = new T[n];
            // 旧长度必须先存下来：_start 一旦改成 tmp，size() 算的就是
            // tmp - 旧_finish，两个不相干的内存相减，结果没有意义
            size_t oldSize = size();
            if (_start)
            {
                // 这里不能用 memcpy：T 可能是 string 这种带资源的类型，
                // 按字节拷贝会得到两块指向同一份资源的对象，析构时二次释放
                for (size_t i = 0; i < oldSize; ++i)
                {
                    tmp[i] = _start[i];
                }
                delete[] _start;
            }
            _start = tmp;
            _finish = _start + oldSize;
            _endofstorage = _start + n;
        }
    }

    void push_back(const T& x)
    {
        // 空间满了先扩容：容量为 0 时给 4，之后每次翻倍（均摊 O(1)）
        if (_finish == _endofstorage)
        {
            reserve(capacity() == 0 ? 4 : capacity() * 2);
        }
        // new T[n] 已经把这块空间构造成了对象，直接赋值即可
        *_finish = x;
        ++_finish;
    }   

    void pop_back()
    {
        // 标准 vector 对空容器调用 pop_back 是未定义行为，这里用断言在调试期拦住
        assert(!empty());
        // 只把“有效末尾”往回挪一格，元素对象本身不在这里析构：
        // 这些坑位是 new T[n] 时统一构造的，也统一由 delete[] 析构。
        // 因此不能写 (--_finish)->~T()：那样该位置会回到“未构造”状态，
        // 之后 push_back 里的 *_finish = x 就变成对已析构对象赋值，
        // 属于未定义行为。代价是被弹出元素占用的资源（如 string 的堆缓冲）
        // 要等这个坑位被重新赋值或整个容器析构时才释放。
        --_finish;
    }

    // 在 pos 之前插入 n 个 x，返回指向第一个新元素的迭代器
    iterator insert(iterator pos, size_t n, const T& x)
    {
        // pos 允许等于 _finish，表示尾部插入
        assert(pos >= _start && pos <= _finish);
        if (n == 0)
        {
            return pos;
        }

        // 扩容会换一块内存，旧的裸指针 pos 随之失效，
        // 所以先记下它相对 _start 的下标，扩容后再重新算回来
        size_t offset = pos - _start;

        if (size() + n > capacity())
        {
            // 一次扩到够用：按倍增找到第一个 >= size()+n 的容量，
            // 否则 "插入 n 个" 可能要连续搬迁好几次
            size_t newCapacity = capacity() == 0 ? 4 : capacity() * 2;
            while (newCapacity < size() + n)
            {
                newCapacity *= 2;
            }
            reserve(newCapacity);
            pos = _start + offset;
        }

        // 把 [pos, _finish) 这 count 个元素整体后移 n 格。
        // 必须从后往前搬：从前往后写会把还没搬的元素提前覆盖掉。
        // 用下标而不是指针，避免 "p - n" 在 n 较大时绕来绕去。
        size_t count = _finish - pos;
        for (size_t i = count; i > 0; --i)
        {
            _start[offset + i - 1 + n] = _start[offset + i - 1];
        }

        // 空出来的 [pos, pos+n) 全部填 x
        for (size_t i = 0; i < n; ++i)
        {
            _start[offset + i] = x;
        }

        _finish += n;
        // 注意：若 x 引用的正是本容器里的元素（如 v.insert(v.begin(), v[2])），
        // 扩容会释放旧存储、上面的后移也会覆盖它，x 读到的就不是原来的值。
        // 标准库在内部会先把这种自引用复制一份；本版为保持代码直观不处理，
        // 需要时自己写 T copy = v[2]; 再 v.insert(v.begin(), copy);
        return _start + offset;
    }

    // 单元素版直接复用上面的批量版：插入 1 个
    iterator insert(iterator pos, const T& x)
    {
        return insert(pos, 1, x);
    }

    // 删除 [first, last)，返回 last 原来的位置
    // （即删除区间之后的第一个元素现在的位置；删到尾部就返回新的 end()）
    iterator erase(iterator first, iterator last)
    {
        assert(first >= _start && first <= _finish);
        assert(last >= first && last <= _finish);

        // 空区间必须直接返回。否则下面的循环一次都不执行，
        // 却会把 _finish 截断到 first，等于把 first 之后全部删掉
        if (first == last)
        {
            return first;
        }

        // [last, _finish) 整体前移到 first 处，末尾失效的坑位不析构（同 pop_back）
        T* dst = first;
        for (T* src = last; src < _finish; ++src, ++dst)
        {
            *dst = *src;
        }
        _finish = dst;
        return first;
    }

    // 单元素版：删除 pos 处一个元素，返回它后面那个元素的新位置
    iterator erase(iterator pos)
    {
        // 单元素版不能传 end()：那里没有元素可删
        assert(pos >= _start && pos < _finish);
        return erase(pos, pos + 1);
    }

    // 清空所有元素：size() 变 0，capacity() 不变
    void clear()
    {
        // 只把有效末尾收回起点，一个元素都不用动，所以是 O(1)。
        // 空容器时 _start 与 _finish 同为 nullptr，赋完仍然是空容器。
        // 坑位不在这里析构（原因同 pop_back）：槽位由 new T[n] 统一构造、
        // delete[] 统一析构，若在此逐个析构，之后 push_back 的赋值
        // 就变成对已析构对象赋值，属于未定义行为。
        _finish = _start;
    }

    // 把元素个数调整到 n：多了截断，少了用 value 补齐。
    // 标准库里这是两个重载（resize(n) 与 resize(n, value)），
    // 这里用默认参数合成一个，逻辑只有一份。
    void resize(size_t n, const T& value = T())
    {   
        if (n > size())
        {
            if (n > capacity())
            {
                // 和 insert 一样按倍增一次扩够：若只 reserve(n)，
                // 反复 resize(size()+1) 会每次都重新分配，退化成 O(n²)
                size_t newCapacity = capacity() == 0 ? 4 : capacity() * 2;
                while (newCapacity < n)
                {
                    newCapacity *= 2;
                }
                reserve(newCapacity);
            }

            // reserve 可能换了内存，所以要在它之后再算末尾位置
            T* newFinish = _start + n;
            for (T* p = _finish; p < newFinish; ++p)
            {
                // 槽位在 new T[] 时就构造好了，直接赋值即可
                *p = value;
            }
            _finish = newFinish;
        }
        else if (n < size())
        {
            // 只把有效末尾收回 _start + n，被"移出"的元素不析构（同 pop_back）
            _finish = _start + n;
        }
        // n == size() 时什么都不用做
    }

    T& operator[](size_t i)
    {
        assert(i < size());
        return _start[i];
    }

    const T& operator[](size_t i) const
    {
        assert(i < size());
        return _start[i];
    }


    ~vector()
    {
        if (_start)
        {
            delete[] _start;
            _start = _finish = _endofstorage = nullptr;
        }
    }
};

// 非成员 swap：只是转发给成员版本，两行换一个惯用法。
// 有了它，调用侧的 `using std::swap; swap(a, b);` 会优先选中这个——
// 它比 std::swap 的通用模板更特化；std::sort 之类的泛型代码也会通过 ADL 找到它。
// 注意不能 specialize std::swap 来达到同样目的，往 std 里加东西是未定义行为。
template<class T>
void swap(vector<T>& a, vector<T>& b) noexcept
{
    a.swap(b);
}

} // namespace XiYue

#endif //XIYUE_VECTOR_H
