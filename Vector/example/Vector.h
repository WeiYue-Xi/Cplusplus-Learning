//
// Created by HY on 2026/11/1.
//

#ifndef VECTOR_VECTOR_H
#define VECTOR_VECTOR_H
#include <iterator>
#include <assert.h>
#include <cstring>

template<class T>
class Vector
{
private:
    // 三指针：[_start, _finish) 是已有元素，[_finish, _endofstorage) 是空余空间
    // 成员不再就地给初值，改由构造函数初始化列表负责（否则是随机值，size() 一算就崩）
    T* _start;
    T* _finish;
    T* _endofstorage;
public:
    typedef T* iterator;
    typedef const T* const_iterator;

    // 构造函数：三个指针都指向空，得到一个 size()==0、capacity()==0 的空容器
    Vector()
            : _start(nullptr), _finish(nullptr), _endofstorage(nullptr)
    {
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

    ~Vector()
    {
        delete[] _start;
    }
};

#endif //VECTOR_VECTOR_H
