//
// Created by 24242 on 2026/10/14.
//

#include <iostream>
#include <algorithm>

using namespace std;

int main()
{
    int num[] = {43,100,8,320,77,101,32};
    sort(num,num + 7,greater<int>());
    return 0;
}
