#pragma once
#include <ctime>
#include <cstdint>
class CTimeSpan{
 std::int64_t seconds_=0;
 public:
 CTimeSpan()=default;
 explicit CTimeSpan(std::int64_t seconds):seconds_(seconds){}
 CTimeSpan(long days,int hours,int minutes,int seconds):seconds_(std::int64_t(days)*86400+std::int64_t(hours)*3600+std::int64_t(minutes)*60+seconds){}
 std::int64_t GetTotalSeconds()const{return seconds_;}
 std::int64_t GetTotalMinutes()const{return seconds_/60;}
 std::int64_t GetTotalHours()const{return seconds_/3600;}
 std::int64_t GetDays()const{return seconds_/86400;}
 int GetHours()const{return int((seconds_/3600)%24);}
 int GetMinutes()const{return int((seconds_/60)%60);}
 int GetSeconds()const{return int(seconds_%60);}
};
class CTime{
 std::time_t value_=0;
 std::tm local()const{std::tm value{};localtime_r(&value_,&value);return value;}
 public:
 CTime()=default;
 explicit CTime(std::time_t value):value_(value){}
 CTime(int year,int month,int day,int hour,int minute,int second,int dst=-1){
  std::tm value{};value.tm_year=year-1900;value.tm_mon=month-1;value.tm_mday=day;value.tm_hour=hour;value.tm_min=minute;value.tm_sec=second;value.tm_isdst=dst;value_=std::mktime(&value);
 }
 static CTime GetCurrentTime(){return CTime(std::time(nullptr));}
 // The legacy schedule implementation uses this spelling for calendar time.
 static CTime GetTickCount(){return GetCurrentTime();}
 std::time_t GetTime()const{return value_;}
 int GetYear()const{return local().tm_year+1900;}
 int GetMonth()const{return local().tm_mon+1;}
 int GetDay()const{return local().tm_mday;}
 int GetDayOfWeek()const{return local().tm_wday+1;}
 int GetHour()const{return local().tm_hour;}
 int GetMinute()const{return local().tm_min;}
 int GetSecond()const{return local().tm_sec;}
 CTime& operator+=(const CTimeSpan& delta){value_+=delta.GetTotalSeconds();return *this;}
 CTime& operator-=(const CTimeSpan& delta){value_-=delta.GetTotalSeconds();return *this;}
 friend CTime operator+(CTime value,const CTimeSpan& delta){value+=delta;return value;}
 friend CTime operator-(CTime value,const CTimeSpan& delta){value-=delta;return value;}
 friend CTimeSpan operator-(const CTime& left,const CTime& right){return CTimeSpan(std::int64_t(left.value_)-std::int64_t(right.value_));}
 friend bool operator<(const CTime& left,const CTime& right){return left.value_<right.value_;}
 friend bool operator>(const CTime& left,const CTime& right){return right<left;}
 friend bool operator<=(const CTime& left,const CTime& right){return !(left>right);}
 friend bool operator>=(const CTime& left,const CTime& right){return !(left<right);}
 friend bool operator==(const CTime& left,const CTime& right){return left.value_==right.value_;}
 friend bool operator!=(const CTime& left,const CTime& right){return !(left==right);}
};
