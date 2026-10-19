#pragma once

// 多个示例都要用到 Student，所以把类声明放在公共头文件中。
class Student {
public:
    int age;

    explicit Student(int value);
    ~Student();
};
