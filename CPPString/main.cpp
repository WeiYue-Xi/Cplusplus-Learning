#include <iostream>
#include <string>
#include <vector>

#include "Lin_String.h"

using namespace std;

int main()
{
    Lin_String s1("Hello world");

    cout << s1.c_str() << endl;

    for (size_t i = 0;i < s1.size();i++)
    {
        cout << s1[i];
    }
    cout << endl;

    Lin_String::iterator it1 = s1.begin();
    while (it1 != s1.end())
    {
        cout << *it1 << "";
        ++it1;
    }
    cout << endl;
    return 0;
}
