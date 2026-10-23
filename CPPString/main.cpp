#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main()
{

    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    vector<int>::iterator it2 = v.begin();
    while (it2 != v.end())
    {
        cout << *it2 << endl;
        it2++;
    }
    string s1 = "catelog";
    string::reverse_iterator it3 = s1.rbegin();
    while (it3 != s1.rend())
    {
        cout << *it3 << " ";
        it3++;
    }
    cout << endl;
    cout << s1.capacity() << endl;
    return 0;
}