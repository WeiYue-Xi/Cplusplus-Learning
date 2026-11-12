#include <iostream>

#include "example/list.h"

using namespace std;
int main()
{
    XiYue::list<int> l1;
    l1.push_back(1);
    l1.push_back(2);
    l1.push_back(3);
    XiYue::list<int>::iterator it = l1.begin();
    while (it != l1.end())
    {
        cout << *it << " ";
        ++it;
    }
    cout << endl;
    return 0;
}