#include "Student.h"

#include <iostream>

// Student:: 表示这个函数属于 Student 类。
Student::Student(int value) : age(value) {
    std::cout << "构造 Student，年龄：" << age << '\n';
}

Student::~Student() {
    std::cout << "析构 Student\n";
}
