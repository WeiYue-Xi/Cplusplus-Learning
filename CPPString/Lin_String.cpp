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
