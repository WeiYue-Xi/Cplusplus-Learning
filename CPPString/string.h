//
// Created by 24242 on 2026/10/25.
//

#ifndef CPPSTRING_STRING_H
#define CPPSTRING_STRING_H

#include <cstddef>
#include <iosfwd>

class string
{
private:
    char* _str = nullptr;
    std::size_t _size = 0;
    std::size_t _capacity = 0;

public:
    typedef char* iterator;
    typedef const char* const_iterator;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    string();
    explicit string(const char* str);
    string(const string& s);
    string& operator=(const string& s);
    ~string();

    const char* c_str() const;
    std::size_t size() const;

    char& operator[](std::size_t i);
    const char& operator[](std::size_t i) const;

    void reserve(std::size_t n);
    void push_back(char ch);
    void append(const char* str);

    string& operator+=(char ch);
    string& operator+=(const char* str);

    friend std::ostream& operator<<(std::ostream& out, const string& s);

    void insert(size_t pos,char ch);
    void insert(size_t pos,const char* str);
};

#endif //CPPSTRING_STRING_H
