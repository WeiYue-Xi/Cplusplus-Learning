#include "examples.h"

#include <iostream>

// 2. 申请三个整数。new[] 必须配 delete[]。
void test_array() {
    int* p = new int[3]{10, 20, 30};
    for (int i = 0; i < 3; ++i) {
        std::cout << p[i] << ' ';
    }
    std::cout << '\n';

    delete[] p;
    p = nullptr;
}
