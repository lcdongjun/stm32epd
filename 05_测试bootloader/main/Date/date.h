#ifndef _DATE_H_
#define _DATE_H_

#include "sys.h"

// ����ö�ٺͽṹ��
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
    u8 day;           // ���ڣ�1-31��
    Weekday weekday;  // ���ڼ�
    u8 isHoliday;     // �Ƿ�Ϊ����
		u8 isToday;
} Day;

typedef struct {
    MonthName name;    // �·�����
    u16 year;          // �������
    u8 days;           // ��������
    Day daysArray[31]; // �洢����ÿ�����Ϣ
		u8 isTomonth;
} Month;

typedef struct {
    int year;          // ���
    Month months[12];  // һ��12����
} Year;


typedef struct {
	Year Year;
	Time Time;
} Calendar;


#endif

