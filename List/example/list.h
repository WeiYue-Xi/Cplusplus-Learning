//
// Created by HY on 2026/11/11.
//

#ifndef LIST_LIST_H
#define LIST_LIST_H
#include <cstddef>      // std::ptrdiff_t
#include <iterator>     // std::bidirectional_iterator_tag

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

    // Ref / Ptr 决定"能不能通过迭代器改元素"：
    //   Ref = T&       Ptr = T*        → iterator
    //   Ref = const T& Ptr = const T*  → const_iterator（只能读）
    // 同一份实现，只换这两个类型参数，不必写两份重复代码
    template<class T, class Ref, class Ptr>
    struct _list_iterator
    {
        typedef list_node<T> Node;
        typedef _list_iterator<T, Ref, Ptr> Self;

        // 迭代器萃取需要的五个类型，缺一个 std::iterator_traits 就取不到信息
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef Ptr pointer;
        typedef Ref reference;

        Node* _node;

        _list_iterator(Node* node = nullptr)
            :_node(node)
        {}

        reference operator*() const
        {
            return _node -> _data;
        }

        // it->member 等价于 (&**it)->member，元素是自定义类型时要用它
        pointer operator->() const
        {
            return &(_node -> _data);
        }

        // 前置 ++：返回引用，才能连写 ++(++it)；返回副本则概念检查也过不了
        Self& operator++()
        {
            _node = _node -> _next;
            return *this;
        }

        // 后置 ++：返回自增之前的副本，必须按值返回
        Self operator++(int)
        {
            Self tmp(*this);
            _node = _node -> _next;
            return tmp;
        }

        // 前置 --：双向迭代器才有，靠 _prev 往回走
        Self& operator--()
        {
            _node = _node -> _prev;
            return *this;
        }

        // 后置 --
        Self operator--(int)
        {
            Self tmp(*this);
            _node = _node -> _prev;
            return tmp;
        }

        bool operator==(const Self& it) const
        {
            return _node == it._node;
        }

        bool operator!=(const Self& it) const
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
        // 迭代器的类型由元素类型和"能不能改"共同决定
        typedef _list_iterator<T, T&, T*> iterator;                 // 可读可写
        typedef _list_iterator<T, const T&, const T*> const_iterator; // 只读

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