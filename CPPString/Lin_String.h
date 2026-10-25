//
// Created by 24242 on 2026/10/25.
//

#ifndef CPPSTRING_LIN_STRING_H
#define CPPSTRING_LIN_STRING_H

#include "string.h"


class Lin_String
{
private:
    char* _str;
    size_t _size;
    size_t _capacity;
public:
    typedef char* iterator;
    typedef const char* const_iterator;
    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    Lin_String();
    Lin_String(const char* str = "");
    const char* c_str() const;
    ~Lin_String();
    size_t size() const;
    char& operator[](size_t i);
    const char& operator[](size_t i) const;

    void reserve(size_t n);
    void push_back(char ch);
    void append(const char* str);
};


#endif //CPPSTRING_LIN_STRING_H