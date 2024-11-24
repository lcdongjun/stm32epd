#ifndef _DATE_H_
#define _DATE_H_

#include "sys.h"

// 定义枚举和结构体
typedef enum {
    SUNDAY = 0, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY
} Weekday;

typedef enum {
    JANUARY = 1, FEBRUARY, MARCH, APRIL, MAY, JUNE,
    JULY, AUGUST, SEPTEMBER, OCTOBER, NOVEMBER, DECEMBER
} MonthName;


typedef struct {
		u8  Year;
		u8  Month;
		u8  Day;
    u8  Hour;  //0 - 23
    u8  Min;   //0 - 59
    u8  Sec;   //0 - 59
} Time;

typedef struct {
    u8 day;           // 日期（1-31）
    Weekday weekday;  // 星期几
    u8 isHoliday;     // 是否为节日
		u8 isToday;
} Day;

typedef struct {
    MonthName name;    // 月份名称
    u16 year;          // 所属年份
    u8 days;           // 当月天数
    Day daysArray[31]; // 存储当月每天的信息
		u8 isTomonth;
} Month;

typedef struct {
    int year;          // 年份
    Month months[12];  // 一年12个月
} Year;


typedef struct {
	Year Year;
	Time Time;
} Calendar;


#endif

