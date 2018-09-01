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
//#include <json/json.h>
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

    Date& operator -- () //前置
    {
        --(_day);
        if(_day == 0)
        {
            --(_month);
            if(_month == 0)
            {
                _year -= 1;
                _month = 12;
            }
            _day += GetMonthDay(_year,_month);
        }
        return *this;
    }

    bool operator > (const Date d)
    {
        if(_year > d._year)
        {
            return true;
        }
        else if(_year == d._year)
        {
            if(_month > d._month)
            {
                return true;
            }
            else if(_month == d._month)
            {
                if(_day > d._day)
                {
                    return true;
                }
                else if(_day == d._day)
                {
                    return true;
                }
            }
        }
        return false;

    }

    bool operator < (const Date& d)
    {
        return !(*this > d);
    }


    //计算两个日期之间的差值
    //2018-7-8 - 2015-9-8
    //d2 - d1 ---> d2.operator - (const Date& d)
    int operator - (Date& d)
    {
        int count = 0;
        int flag = 0;
        if(*this < d)
        {
            swap(*this,d);
            flag = -1;
        }
        
        while(*this  > d)
        {
            --(*this);
            ++count;
        }
        if(flag == -1)
        {
            swap(*this,d);
        }
        return count;
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
private:
    int _year;
    int _month;
    int _day;
};
#define AAAAA
#ifdef AAAAA
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
   //"year1&"2018  "mon1&"8 "day1&"15 "year2&"2019 "mon2&"1 "day2&"1
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
    
   int year1 = arr[0];
   int year2 = arr[3];
   int month1 = arr[1];
   int month2 = arr[4];
   int day1 = arr[2];
   int day2 = arr[5];
//   cout<<year1<<"-"<<month1<<"-"<<day1<<endl;
 //  cout<<year2<<"-"<<month2<<"-"<<day2<<endl;

    //年=2018&月=8&日=20&年=2017&月=10&日=20
 //   int year1;
 //   int year2;
 //   int month1;
 //   int month2;
 //   int day1;
 //   int day2;
 //   sscanf(buf,"年=%d&月=%d&日=%d&年=%d&月=%d&日=%d",&year1,&month1,&day1,&year2,&month2,&day2);
    Date d1(year1,month1,day1);
    Date d2(year2,month2,day2);
    int day = d1-d2-1;
  //  int day = 20;
    cout<<day<<endl;
    }
#else
int main(){
    int fd = open("wwwRoot/test.html",O_RDONLY);
    if(fd < 0){
   //     printf("11111\n");
        return 0;
    }

    struct stat st;
    stat("wwwRoot/test.html",&st);
    for(int i = 0;i<st.st_size;i++){
        sendfile(1,fd,NULL,st.st_size);
    }
    close(fd);
    return 0;
}
#endif
