#include <cstddef>
#include <iostream>

#include "string.h"

int main()
{
    string s1("Hello world");
    std::cout << s1.c_str() << std::endl;

    for (std::size_t i = 0; i < s1.size(); i++)
    {
        std::cout << s1[i];
    }
    std::cout << std::endl;

    string::iterator it1 = s1.begin();
    while (it1 != s1.end())
    {
        std::cout << *it1;
        ++it1;
    }
    std::cout << std::endl;

    string s2("program ");
    string::const_iterator it2 = s2.begin();
    while (it2 != s2.end())
    {
        std::cout << *it2;
        ++it2;
    }
    std::cout << std::endl;

    s2 += "is magnificent";
    std::cout << s2 << std::endl;   // 验证 operator<<

    string s3("It beautiful!");
    s3.insert(3,"is ");
    std::cout << s3 << std::endl;



    return 0;
}
