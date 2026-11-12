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

    template<class T>
    struct _list_iterator
    {
        typedef list_node<T> Node;
        typedef _list_iterator<T> Self;

        // 迭代器萃取需要的五个类型，缺一个 std::iterator_traits 就取不到信息
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef T* pointer;
        typedef T& reference;

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
        typedef _list_iterator<T> iterator;

        list()
        {
            _head = new Node;
            _head -> _next = _head;
            _head -> _prev = _head;
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