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
