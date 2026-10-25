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
    iterator begin();
    iterator end();
    Lin_String();
    Lin_String(const char* str = "");
    const char* c_str() const;
    ~Lin_String();
    size_t size() const;
    char& operator[](size_t i);
    const char& operator[](size_t i) const;
};


#endif //CPPSTRING_LIN_STRING_H