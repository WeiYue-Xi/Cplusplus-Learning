#include "examples/examples.h"

#include <iostream>

int main() {
    std::cout << "\n1. new / delete\n";
    test_int();

    std::cout << "\n2. new[] / delete[]\n";
    test_array();

    std::cout << "\n3. 类对象的构造与析构\n";
    test_student();

    std::cout << "\n4. malloc / free\n";
    test_malloc();

    std::cout << "\n5. 定位 new（学会前四个后再看）\n";
    test_placement_new();
    return 0;
}
