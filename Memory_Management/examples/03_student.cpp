#include "examples.h"
#include "Student.h"

#include <iostream>

// 3. new 创建类对象时调用构造函数，delete 时调用析构函数。
void test_student() {
    Student* p = new Student(18);
    std::cout << "读取年龄：" << p->age << '\n';

    delete p;
    p = nullptr;
}
