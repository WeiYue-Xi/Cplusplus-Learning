#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main()
{
    vector<int> v1(10,1);
    vector<int> v2;
    v2.push_back(1);
    v2.push_back(2);
    v2.push_back(3);

    for (auto e : v1)
    {
        cout << e << " ";
    }
    cout << endl;
    auto it = find(v2.begin(),v2.end(),5);
    if (it != v2.end())
    {
        v2.insert(it,20);
    }
    for (auto e : v2)
    {
        cout << e << " ";
    }
    cout << endl;
    return 0;
}
