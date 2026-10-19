#include "examples.h"
#include "Student.h"

#include <iostream>
#include <new>

// 5. 定位 new：在已经准备好的空间中构造对象。
void test_placement_new() {
    // sizeof 保证空间足够，alignas 保证地址满足 Student 的对齐要求。
    alignas(Student) unsigned char buffer[sizeof(Student)];
    Student* p = ::new (static_cast<void*>(buffer)) Student(20);
    std::cout << "读取年龄：" << p->age << '\n';

    p->~Student(); // 只析构对象；buffer 随函数结束自动回收。
    // 这里不能 delete p，因为 buffer 是局部数组。
}
