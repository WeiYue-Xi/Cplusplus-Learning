//
// Created by HY on 2026/11/11.
//

#ifndef LIST_LIST_H
#define LIST_LIST_H
#include <bits/regex_constants.h>

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
        Node* _node;

        _list_iterator(Node* node)
        {
            
        }
    };

    template<class T>
    class list
    {
        typedef list_node<T> Node;
    private:
        Node* _head;

    public:
        list()
        {
            _head = new Node;
            _head -> _next = _head;
            _head -> _prev = _head;
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