//
// Created by 24242 on 2026/10/25.
//

#include "string.h"

#include <cassert>
#include <cstring>
#include <istream>
#include <ostream>

const size_t string::npos = -1;

string::string()
    : _str(new char[1]{ '\0' })
    , _size(0)
    , _capacity(0)
{
}

string::string(const char* str)
    : _size(std::strlen(str))
{
    _capacity = _size;
    _str = new char[_size + 1];
    std::memcpy(_str, str, _size + 1);        // 连结尾的 '\0' 一起拷
}

// 深拷贝：新开一块内存，避免两个对象指向同一块堆内存（否则析构时 double free）
// string::string(const string& s)
//     : _str(new char[s._size + 1])
//     , _size(s._size)
//     , _capacity(s._size)
// {
//     std::memcpy(_str, s._str, s._size + 1);
// }


void string::swap(string &s)
{
    std::swap(_str,s._str);
    std::swap(_size,s._size);
    std::swap(_capacity,s._capacity);
}


string::string(const string &s)
{
    string tmp(s._str);
    swap(tmp);
}


string& string::operator=(const string& s)
{
    if (this != &s)
    {
        // 先分配再释放，这样 new 抛异常时对象仍然完好
        char* tmp = new char[s._size + 1];
        std::memcpy(tmp, s._str, s._size + 1);
        delete[] _str;
        _str = tmp;
        _size = s._size;
        _capacity = s._size;
    }
    return *this;
}

string::~string()
{
    delete[] _str;
    _str = nullptr;
    _size = _capacity = 0;
}

const char* string::c_str() const
{
    return _str;
}

std::size_t string::size() const
{
    return _size;
}

char& string::operator[](std::size_t i)
{
    assert(i < _size);
    return _str[i];
}

const char& string::operator[](std::size_t i) const
{
    assert(i < _size);
    return _str[i];
}

string::iterator string::begin()
{
    return _str;
}

string::iterator string::end()
{
    return _str + _size;
}

string::const_iterator string::begin() const
{
    return _str;
}

string::const_iterator string::end() const
{
    return _str + _size;
}

void string::reserve(std::size_t n)
{
    if (n > _capacity)
    {
        char* nStr = new char[n + 1];
        std::memcpy(nStr, _str, _size + 1);
        delete[] _str;
        _str = nStr;
        _capacity = n;
    }
}

void string::push_back(char ch)
{
    // 原来写成 _size > _capacity：容量为 0 时扩容永远不触发，会越界写
    if (_size >= _capacity)
    {
        std::size_t newCapacity = _capacity == 0 ? 4 : _capacity * 2;
        reserve(newCapacity);
    }
    _str[_size] = ch;
    ++_size;
    _str[_size] = '\0';
}

void string::append(const char* str)
{
    std::size_t len = std::strlen(str);
    if (_size + len > _capacity)
    {
        std::size_t newCapacity = 2 * _capacity > _size + len ? 2 * _capacity : _size + len;
        reserve(newCapacity);
    }
    std::memcpy(_str + _size, str, len + 1);  // 从末尾追加，别忘了偏移
    _size += len;
}

string& string::operator+=(char ch)
{
    push_back(ch);
    return *this;
}

string& string::operator+=(const char* str)
{
    append(str);
    return *this;
}

std::ostream& operator<<(std::ostream& out, const string& s)
{
    out << s._str;
    return out;
}

// 原来的写法：逐个取字符，遇到空格或换行就停
std::istream& operator>>(std::istream& in, string& s)
{
    s.clear();

    char buff[128];
    int i = 0;
    char ch;
    while (in.get(ch) && ch != ' ' && ch != '\n')   // 多了流状态判断，EOF 时不会死循环
    {
        buff[i++] = ch;
        if (i == 127)
        {
            buff[i] = '\0';
            s += buff;
            i = 0;
        }
        ch = in.get();
    }
    if (i > 0)
    {
        buff[i] = '\0';
        s += buff;
    }
    return in;
}

// 原来的写法：逐个取字符，遇到 delim 就停
std::istream& getline(std::istream& in, string& str, char delim)
{
    str.clear();


    char buff[128];
    int i = 0;

    char ch;
    while (in.get(ch) && ch != delim)               // 同样为了 EOF 时能退出
    {
        buff[i++] = ch;
        if (i == 127)
        {
            buff[i] = '\0';
            str += buff;
            i = 0;
        }
        ch = in.get();
    }

    if (i > 0)
    {
        buff[i] = '\0';
        str += buff;
    }
    return in;
}


string& string::insert(size_t pos, char ch)
{
    assert(pos <= _size);
    if (_size >= _capacity)
    {
        size_t newCapacity = _capacity == 0 ? 4 : 2 * _capacity;
        reserve(newCapacity);
    }
    size_t end = _size + 1;
    while (end > pos)          // 原来是 if：只搬了一个字符，插在中间会丢字符
    {
        _str[end] = _str[end - 1];
        --end;
    }
    _str[pos] = ch;
    ++_size;

    return *this;
}

string& string::insert(size_t pos, const char *str)
{
    assert(pos <= _size);

    std::size_t len = std::strlen(str);
    if (_size + len > _capacity)
    {
        std::size_t newCapacity = 2 * _capacity > _size + len ? 2 * _capacity : _size + len;
        reserve(newCapacity);
    }
    size_t end = _size + len;
    while (end > pos + len - 1)
    {
        _str[end] = _str[end - len];
        --end;
    }
    for (size_t i = 0;i < len;i++)
    {
        _str[pos + i] = str[i];
    }
    _size += len;
    return *this;
}

string& string::erase(size_t pos, size_t len)
{
    assert(pos < _size);

    if (len == npos || len >= (_size-pos))
    {
        _size = pos;
        _str[_size] = '\0';
    } else
    {
        size_t i = pos + len;
        memmove(_str + pos,_str + i,_size + 1 - i);
        _size -= len;
    }
    return *this;
}

void string::pop_back()
{
    assert(_size > 0);
    --_size;
    _str[_size] = '\0';
}

size_t string::find(char ch, size_t pos) const
{
    for (size_t i = pos;i < _size;i++)
    {
        if (_str[i] == ch)
        {
            return i;
        }
    }
    return npos;
}

size_t string::find(const char *str, size_t pos) const
{
    const char* p1 = strstr(_str + pos,str);
    if (p1 == nullptr)
    {
        return npos;
    } else
    {
        return p1 - _str;
    }
}

string string::substr(size_t pos, size_t len) const
{
    if (len == npos || len >= _size - pos)
    {
        len = _size - pos;
    }

    string ret;
    ret.reserve(len);
    for (size_t i = 0;i < len;i++)
    {
        ret += _str[pos + i];
    }
    return ret;
}

bool string::operator<(const string &s) const
{
    size_t it1 = 0,it2 = 0;
    while (it1 < _size && it2 < s._size)
    {
        if (_str[it1] < s[it2])
        {
            return true;
        }else if (_str[it1] > s[it2])
        {
            return false;
        }else
        {
            ++it1;
            ++it2;
        }
    }
    return it2 < s._size;
}

bool string::operator<=(const string &s) const
{
    return *this < s || *this == s;
}

bool string::operator>(const string &s) const
{
    return !(*this <= s);
}

bool string::operator>=(const string &s) const
{
    return !(*this < s);
}

bool string::operator==(const string& s) const
{
    size_t it1 = 0,it2 = 0;
    while (it1 < _size && it2 < s._size)
    {
        if (_str[it1] != s[it2])
        {
            return false;
        }else
        {
            ++it1;
            ++it2;
        }
    }
    return it1 == _size && it2 == s._size;
}

bool string::operator!=(const string &s) const
{
    return !(*this == s);
}

void string::clear()
{
    _str[0] = '\0';
    _size = 0;
}
