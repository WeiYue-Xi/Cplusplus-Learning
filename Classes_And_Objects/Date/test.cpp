#include <iostream>

#include "Date.h"

using namespace std;

int test()
{
    Date d1(2023,1,1);
    Date d2(2022,12,20);

    Date d3 = d1 + 379;
    cout << d1;
    d3.Print();

    const Date d4 = d2++;
    d4.Print();
    d2.Print();

    cout << (d1 < d1) << endl;
    cout << (d1 >= d1) << endl;

    cout << d2-d1 << endl;
    return 0;
}