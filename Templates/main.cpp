#include <iostream>

//函数模版
//template<typename  T>
//template<class C>
//void Swap(T& x,T& y)
// {
//     T tmp = x;
//     x = y;
//     y = tmp;
// }

// int main()
// {
//     int num1 = 1,num2 = 2;
//     std::cout << num1 << " " << num2 << std::endl;
//     Swap(num1,num2);
//     std::cout << num1 << " " << num2 << std::endl;
//
//     return 0;
// }

// template<class T>
// T Add(const T& x,const T& y)
// {
//     return x + y;
// }

// int main()
// {
//     int iNum1 = 10,iNum2 = 20;
//     double dNum1 = 10.1,dNum2 = 20.2;
//     //隐式实例化(实参类型推演模版参数)
//     std::cout << Add(iNum1,iNum2) << std::endl;
//     std::cout << Add(dNum1,dNum2) << std::endl;
//
//     //显示实例化(显示指定模版参数)
//     std::cout << Add<int>(iNum1,dNum2) << std::endl;
//     std::cout << Add<double>(iNum1,dNum2) << std::endl;
//
// }


//专门处理1nt的加分函数
// int Add(int x,int y)
// {
//     return x + y;
// }

//通用
// template<class T>
// T Add(T x,T y)
// {
//     return x + y + 5;
// }

// int main()
// {
//     int num1 = 10,num2 = 5;
//     std::cout << Add(num1,num2) << std::endl;
//     std::cout << Add<int>(num1,num2) << std::endl;
//
//     return 0;
// }

//类模版

namespace A
{
    template<class T>
    class Stack
    {
    public:
        Stack(int n = 4)
            :_a(new T[n])
            ,_top(0)
            ,_capacity(n)
        {
        }
        ~Stack()
        {
            delete[] _a;
            _a = nullptr;
            _top = 0;
            _capacity = 0;
        }
    private:
        T* _a;
        int _top;
        int _capacity;
    };
}

int main()
{
    try
    {
        A::Stack<int> st1;
        A::Stack<double> st2;
    }catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }



    return 0;
}