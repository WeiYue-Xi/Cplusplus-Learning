//
// Created by HY on 2026/11/11.
//

#ifndef LIST_LIST_H
#define LIST_LIST_H
#include <cstddef>      // std::ptrdiff_t
#include <iterator>     // std::bidirectional_iterator_tag
#include <memory>       // std::addressof

namespace XiYue
{
    template<class T>
    struct list_node
    {
        T _data;
        list_node<T>* _next;
        list_node<T>* _prev;

        list_node(const T& x = T())
            :_data(x)
            ,_next(nullptr)
            ,_prev(nullptr)
        {}
    };

    // 普通迭代器：保存一个节点指针，可以移动，也可以修改节点里的元素。
    template<class T>
    struct _list_iterator
    {
        typedef list_node<T> Node;

        // 给标准算法看的类型信息；先理解下面的操作，再回来看这里。
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef T* pointer;
        typedef T& reference;

        Node* _node; // 当前所在的节点

        _list_iterator(Node* node = nullptr)
            :_node(node)
        {}

        // *it：取出当前元素。返回引用，所以 *it = 10 能修改原元素。
        // 末尾的 const 只表示这个操作不会改变迭代器的位置。
        T& operator*() const
        {
            return _node -> _data;
        }

        // it->member：返回当前元素的地址，再由指针访问它的成员。
        T* operator->() const
        {
            return std::addressof(_node -> _data);
        }

        // ++it：先走到下一个节点，再返回移动后的迭代器本身。
        // this 指向迭代器对象，*this 就是这个迭代器本身，不是节点里的元素。
        _list_iterator& operator++()
        {
            _node = _node -> _next;
            return *this;
        }

        // it++：保存旧位置，移动自己，返回旧位置的副本。
        // int 是区分前置和后置写法的占位参数，不代表移动的步数。
        _list_iterator operator++(int)
        {
            _list_iterator old = *this;
            ++(*this);
            return old;
        }

        // --it：走到前一个节点，返回移动后的自己。
        _list_iterator& operator--()
        {
            _node = _node -> _prev;
            return *this;
        }

        // it--：向前走，但返回移动之前的副本。
        _list_iterator operator--(int)
        {
            _list_iterator old = *this;
            --(*this);
            return old;
        }

        // 比较的是节点位置，不是元素的值。
        bool operator==(const _list_iterator& it) const
        {
            return _node == it._node;
        }

        bool operator!=(const _list_iterator& it) const
        {
            return _node != it._node;
        }
    };

    // 只读迭代器：移动方式相同，取出的元素只能读，不能修改。
    template<class T>
    struct _list_const_iterator
    {
        typedef list_node<T> Node;

        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef const T* pointer;
        typedef const T& reference;

        const Node* _node;

        _list_const_iterator(const Node* node = nullptr)
            :_node(node)
        {}

        // 返回 const T&：可以读取 *it，但不能写 *it = 10。
        const T& operator*() const
        {
            return _node -> _data;
        }

        const T* operator->() const
        {
            return std::addressof(_node -> _data);
        }

        // 只读限制的是元素，迭代器自己仍然可以前进、后退。
        _list_const_iterator& operator++()
        {
            _node = _node -> _next;
            return *this;
        }

        _list_const_iterator operator++(int)
        {
            _list_const_iterator old = *this;
            ++(*this);
            return old;
        }

        _list_const_iterator& operator--()
        {
            _node = _node -> _prev;
            return *this;
        }

        _list_const_iterator operator--(int)
        {
            _list_const_iterator old = *this;
            --(*this);
            return old;
        }

        bool operator==(const _list_const_iterator& it) const
        {
            return _node == it._node;
        }

        bool operator!=(const _list_const_iterator& it) const
        {
            return _node != it._node;
        }
    };

    template<class T>
    class list
    {
        typedef list_node<T> Node;
    private:
        Node* _head;

    public:
        typedef _list_iterator<T> iterator;             // 可读可写
        typedef _list_const_iterator<T> const_iterator; // 只读

        list()
        {
            _head = new Node;
            _head -> _next = _head;
            _head -> _prev = _head;
        }

        // 析构：用迭代器从头走到尾，逐个释放节点，最后释放哨兵自己
        ~list()
        {
            iterator it = begin();
            while (it != end())
            {
                iterator next = it;     // 先存好下一个位置
                ++next;                 // ++ 只改 _node，不会碰要被删的节点
                delete it._node;         // 释放当前节点
                it = next;              // 迭代器前进：此时它已经不是悬空的了
            }
            delete _head;
            _head = nullptr;
        }

        // 第一个元素的位置：哨兵位的下一个
        iterator begin()
        {
            return iterator(_head -> _next);
        }

        // 最后一个元素的下一位置：哨兵位本身，不能解引用
        // 空链表时 begin() == end()，循环直接不进入
        iterator end()
        {
            return iterator(_head);
        }

        // const 对象只能拿到 const_iterator：遍历照常，但改不了元素
        const_iterator begin() const
        {
            return const_iterator(_head -> _next);
        }

        const_iterator end() const
        {
            return const_iterator(_head);
        }

        void push_back(const T& x)
        {
            Node* newNode = new Node(x);
            Node* tail = _head -> _prev;

            tail -> _next = newNode;

            newNode -> _prev = tail;
            newNode -> _next = _head;

            _head -> _prev = newNode;
        }


    };
}

#endif //LIST_LIST_H
