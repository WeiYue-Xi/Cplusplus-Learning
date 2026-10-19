#include "examples.h"

#include <iostream>

// 1. 申请一个整数，使用后释放。
void test_int() {
    int* p = new int(10);
    std::cout << *p << '\n';

    delete p;
    p = nullptr; // 释放后置空，避免误用原来的地址。
}
