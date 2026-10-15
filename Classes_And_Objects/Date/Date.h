//
// Created by 24242 on 2026/10/3.
//

#ifndef CLASSES_AND_OBECTS_CLASS_H
#define CLASSES_AND_OBECTS_CLASS_H
#include <iostream>
#include <assert.h>

using namespace std;

class  Date
{

public:
    friend ostream& operator<<(ostream& out,const Date& d);
    friend istream& operator>>(istream& input,Date& x);
    Date()
    {
        _year = 1;
        _month = 1;
        _day = 1;
    }
    Date(int year,int month,int day)
    {
        _year = year;
        _month = month;
        _day = day;
    }

    //拷贝构造
    Date(const Date& d)
    {
        _year = d._year;
        _month = d._month;
        _day = d._day;
        if (checkDate())
        {
            cout << "非法日期" << *this;
        }
    }

    int getMonthDay(int year,int month) const
    {
        assert(month > 0 && month < 13);
        static int monthDayArray[13] = {-1,31,28,31,30,31,30,31,31,30,31,30,31};


        if (month == 2 && (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)))
        {
            return monthDayArray[month] + 1;
        }

        return monthDayArray[month];
    }

    void Print() const;



    Date& operator=(const Date& x)
    {
        if (this != &x)
        {
            _year = x._year;
            _month = x._month;
            _day = x._day;
        }
        return *this;
    }

    Date& operator+=(int day);

    //d1 + 100
    Date operator+(int day) const;

    Date& operator-=(int day);
    Date operator-(int day) const;
    //日期-日期
    int operator-(Date& x);

    //比较日期
    bool operator==(const Date& x) const;
    bool operator!=(const Date& x) const;
    bool operator<(const Date& x) const;
    bool operator<=(const Date& x) const;
    bool operator>(const Date& x) const;
    bool operator>=(const Date& x) const;

    // Date* operator&()
    // {
    //     return this;
    // }
    //
    // const Date* operator&()const
    // {
    //     return this;
    // }

    //++日期
    Date& operator++();
    //日期++
    Date operator++(int);

    Date& operator--();
    Date operator--(int);

    //检查函数合法性
    bool checkDate();

private:
    int _year;
    int _month;
    int _day;
};

ostream& operator<<(ostream& out,const Date& x);
istream& operator>>(istream& input,Date& x);
#endif //CLASSES_AND_OBECTS_CLASS_H