#ifndef _CALENDAR_H_
#define _CALENDAR_H_

#include "sys.h"
#include "EPD_4in2_V2.h"
#include "GUI_Paint.h"
#include "date.h"


int getDaysInMonth(int year, MonthName month);
void initMonth(Month *month, u8 day);
void initYear(Calendar *CalendarStruct, u16 year, u8 month, u8 day);
void drawDay(int x, int y, Day *day);
void drawMonth(Month *month, u16 x_start, u16 y_start);
void displayCalendar(u16 x_start, u16 y_start);
void displayTime(u16 x_start, u16 y_start);

void initCalendar(u8 Calendar_x, u8 Calendar_y, u8 Time_x, u8 Time_y);
	
#endif
