#include "examples.h"
#include "Student.h"

#include <cstdlib>
#include <iostream>

// 4. malloc 申请的是原始空间，不会调用 Student 的构造函数。
void test_malloc() {
    void* memory = std::malloc(sizeof(Student));
    if (memory == nullptr) {
        std::cout << "申请失败\n";
        return;
    }
    std::cout << "申请了空间，但没有构造 Student\n";

    std::free(memory); // 只释放空间，也不会调用 Student 的析构函数。
    memory = nullptr;
}
