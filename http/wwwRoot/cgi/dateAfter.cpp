#include <iostream>
#include<unistd.h>
#include<string.h>
#include<strings.h>
#include<stdlib.h>
#include<stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#define Max 1024
using namespace std;
class Date
{
public:
    Date(int year = 1900,int month = 1,int day = 1)
        :_year(year)
        ,_month(month)
        ,_day(day)
    {}

   // d2(d1)
    Date(const Date& d)
    {
        _year = d._year;
        _month = d._month;
        _day = d._day;
    }
    //d1 = d2
    //d1.operator = (const Date d)
    Date& operator = (const Date d)
    {
        _year = d._year;
        _month = d._month;
        _day = d._day;
        return *this;
    }
    ~Date()
    {}

    //一个日期 + 天数，原来的日期变了
   Date& operator += (int day)
   {
       _day = _day + day;
       while(_day > GetMonthDay(_year,_month))
       {
           _day = _day - GetMonthDay(_year,_month);
           ++(_month);
           if(_month > 12)
           {
               ++(_year);
               _month = 1;
           }
       }
       return *this;
   }

   //一个日期 + 原来的天数没变
   Date operator + (int day)
   {
       Date tmp = (*this);
       tmp += day;
       return tmp;
   }

   //一个日期 - 天数，原来的日期变了
   Date& operator -= (int day)
   { 
        _day = _day - day;
        while(_day  < 0)
        {
            _day = _day + GetMonthDay(_year,_month);
            --(_month);
            if(_month < 0)
            {
                --(_year);
                _month = 12;
            }
        }
        return (*this);
     } 

   //一个日期 - 天数，原来的天数没变
   Date operator - (int day)
   {
       Date tmp = (*this);
       tmp -= day;
       return tmp;
   }

    int GetMonthDay(int year,int month)
    {
        
        int day[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};//平年
        if(year % 400 == 0 || (year % 100 != 0 && year %4 == 0))
        {
            day[month] = day[2] + 1;
        }
        return day[month];
    }

    void Display()
    {
        cout<<_year<<"年"<<_month<<"月"<<_day<<"日"<<endl;
    }

private:
    int _year;
    int _month;
    int _day;
};

int main()
{
    char buf[1024] = {0};
    if(getenv("METHOD"))
    {
        //   cout<<getenv("METHOD")<<endl;
        if(strcasecmp(getenv("METHOD"),"GET") == 0)
        {
//            cout<<getenv("QUERY_STRING")<<endl;
            strcpy(buf,getenv("QUERY_STRING"));
        }
        else
        {
            int content_length = atoi(getenv("CONTENT_LENGTH"));
            int i = 0;
            for( ;i < content_length;i++)
            {
                char c;
                read(0,&c,1);
         //       cout<<c<<endl;
                buf[i] = c;
            }
        }
    }
   //     }
   //"year3&"2018  "mon3&"8 "day3&"15 "decDay2&" 100
   int arr[6] = {0};
   int n = 0;
   int len = sizeof(buf)/sizeof(buf[0]);
   for(int i = 0;i < len;i++){
       if(buf[i] == '&'){
           arr[n] = atoi(&buf[i+2]);
  //         printf("%d ",arr[n]);
           n++;
       }
   }

   int year3 = arr[0];
   int month3 = arr[1];
   int day3 = arr[2];
   int decday = arr[3];
   Date d1(year3,month3,day3);
   Date d2;
   if(decday < 0)
  {
      decday = -decday;
      d2 = d1 - decday;   
  }
  else
  {
       d2 = d1 + decday;
  }
   d2.Display();
}
