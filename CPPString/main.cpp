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

    Lin_String s2("program");
    Lin_String::const_iterator it2 = s2.begin();
    while (it2 != s2.end())
    {
        cout << *it2;
        *it2++;
    }
    cout << endl;
    return 0;
}
