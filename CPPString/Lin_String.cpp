//
// Created by 24242 on 2026/10/25.
//

#include "Lin_String.h"
#include "string.h"
#include <assert.h>

Lin_String::Lin_String()
    :_str(new char[1]{'\0'})
    ,_size(0)
    ,_capacity(0)
{

}

Lin_String::Lin_String(const char *str)
    :_size(strlen(str))
{
    _capacity = _size;
    _str = new char[_size + 1];
    strcpy(_str,str);
}
const char *Lin_String::c_str() const
{
    return _str;
}

Lin_String::~Lin_String()
{
    delete[] _str;
    _size = _capacity = 0;
}

size_t Lin_String::size() const
{
    return _size;
}

char& Lin_String::operator[](size_t i)
{
    assert(i < _size);
    return _str[i];
}

const char& Lin_String::operator[](size_t i) const
{
    assert(i < _size);
    return _str[i];
}

Lin_String::iterator Lin_String::end()
{
    return _str + _size;
}

Lin_String::iterator Lin_String::begin()
{
    return _str;
}

Lin_String::const_iterator Lin_String::begin() const
{
    return _str;
}

Lin_String::const_iterator Lin_String::end() const
{
    return _str + _size;
}

void Lin_String::reserve(size_t n)
{
    if (n > _capacity)
    {
        char* nStr = new char[n + 1];
        strcpy(nStr,_str);
        delete[] _str;
        _str = nStr;
        _capacity = n;
    }
}

void Lin_String::push_back(char ch)
{
    if (_size > _capacity)
    {
        size_t newCapacity = _capacity == 0 ? 4 : _capacity * 2;
        reserve(newCapacity);
    }
    _str[_size] = ch;
    ++_size;
    _str[_size] = '\0';
}

void Lin_String::append(const char* str)
{
    size_t len = strlen(str);
    if (_size + len > _capacity)
    {
        size_t newCapacity = 2 * _capacity > _size + len ? 2 * _capacity : _size + len;
        reserve(newCapacity);
    }
    strcpy(_str + _size,str);
    _size += len;
}
