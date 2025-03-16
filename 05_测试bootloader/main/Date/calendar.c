#include "calendar.h"
#include "malloc.h"
#include <string.h>

Calendar CalendarStruct;
		
// ��ȡÿ���µ���������������
int getDaysInMonth(int year, MonthName month) 
{
    switch (month) {
        case JANUARY: case MARCH: case MAY: case JULY:
        case AUGUST: case OCTOBER: case DECEMBER:
            return 31;
        case APRIL: case JUNE: case SEPTEMBER: case NOVEMBER:
            return 30;
        case FEBRUARY:
            // �ж��Ƿ�Ϊ����
            return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
        default:
            return 30;
    }
}

// ����ĳ�����������ڼ���ʹ�û�ķ����ɭ��ʽ��
Weekday calculateWeekday(int year, int month, int day) {
    if (month == 1 || month == 2) {  // �� 1 �º� 2 ����Ϊǰһ��� 13 �º� 14 ��
        month += 12;
        year -= 1;
    }

    int Y = year;
    int m = month;
    int d = day;

    int w = (d + (13 * (m + 1)) / 5 + Y + (Y / 4) - (Y / 100) + (Y / 400)) % 7;

    return (Weekday)((w + 6) % 7);  // �� 0 ��Ӧ��������
}


// ��ʼ��ĳ���µ���Ϣ
void initMonth(Month *month, u8 day)
{
    // ���õ��µ�����
    month->days = getDaysInMonth(month->year, month->name);
    
    // ������µĵ�һ�������ڼ�
    Weekday firstDayWeekday = calculateWeekday(month->year, month->name, 1);
    // ����ÿһ�����Ϣ
    for (int i = 0; i < month->days; i++) {
        month->daysArray[i].day = i + 1;
        month->daysArray[i].weekday = (Weekday)((firstDayWeekday + i) % 7);  // ˳������
			
				if(month->daysArray[i].weekday == 0 ||month->daysArray[i].weekday == 6)
					month->daysArray[i].isHoliday = 1; //��������Ϊ����
				else month->daysArray[i].isHoliday = 0; // Ĭ�Ϸǽ���
				
				if((i+1) == day&&month->isTomonth == 1){//�ж��Ƿ�Ϊ����
					month->daysArray[i].isToday = 1;
				}
				else{
					month->daysArray[i].isToday = 0;
				}
    }
}

// ��ʼ��һ�������
void initYear(Calendar *CalendarStruct, u16 year, u8 month, u8 day) 
{
    CalendarStruct->Year.year = year;
    for (u8 i = 0; i < 12; i++) {
        CalendarStruct->Year.months[i].year = year;
       CalendarStruct->Year.months[i].name = (MonthName)(i + 1);
			
				if(i == month-1)
				{
					CalendarStruct->Year.months[i].isTomonth = 1;
				}
				else
				{
					CalendarStruct->Year.months[i].isTomonth = 0;
				}
				
        initMonth(&CalendarStruct->Year.months[i], day);
    }
}

// ��ʾ��������
void drawDay(int x, int y, Day *day) 
{
	if(day->day<10)
	{
    Paint_Show_xNum(x+6, y, day->day, 24, 1, 1);
	}
	else
	{
		Paint_Show_xNum(x, y, day->day, 24, 1, 1);
	}
    // ��ǽ���
    if (day->isHoliday && !day->isToday) {
        Paint_Show_RoundRect(x - 1, y - 6, x + 27, y + 22, 6, 1, 1, 0, 0); // ��ǿ�
    }
		if (day->isToday){
				Paint_Show_RoundRect( x, y - 4, x + 28, y + 22, 6, 1, 1, 1, 1);
		}
}

// ��ʾ�����·�
void drawMonth(Month *month, u16 x_start, u16 y_start) 
{
    // �����ܱ���
    u8 *daysOfWeek[] = {"��", "һ", "��", "��", "��", "��", "��"};
    for (u8 i = 0; i < 7; i++) {
        Paint_Show_Str(x_start + i * 32, y_start, daysOfWeek[i], 24, 1, 1);
    }
    
    // ��������
    int startX = x_start, startY = y_start + 32; // ��ʼ��������
		
    Weekday firstDayWeekday = month->daysArray[0].weekday;

    // ��ӡ�³��ո�
    for (int i = 0; i < firstDayWeekday; i++) {
        startX += 32;
    }

    // ��ӡ����
    for (u8 i = 0; i < month->days; i++) {

        drawDay(startX, startY, &month->daysArray[i]);
        
        // ÿ���컻��
        if ((firstDayWeekday + i + 1) % 7 == 0) {
            startY += 32;
            startX = x_start;
        } else {
            startX += 32;
        }
    }
}


void displayCalendar(u16 x_start, u16 y_start)
{
		uint8_t *Partial_Calendar_Image;
		Partial_Calendar_Image = mymalloc(SRAMIN,(32*7 / 8 + 6) * (32*6+6));
		if (Partial_Calendar_Image == NULL)
    {
        printf("Failed to apply for partial memory...\r\n");
    }
		Paint_NewImage(Partial_Calendar_Image, 32*7 + 6, 32*6+6, 0, EPD_WHITE);
		Paint_Clear(EPD_WHITE);
		
		for(u8 i=0;i<12;i++)
		{
			if(CalendarStruct.Year.months[i].isTomonth ==1 )
			{
				drawMonth(&CalendarStruct.Year.months[i], 6, 6);
				break;
			}
		}
		EPD_4IN2_V2_PartialDisplay(Partial_Calendar_Image, x_start, y_start, x_start+(32*7 + 6), y_start+(32*6+6));
		myfree(SRAMIN,Partial_Calendar_Image);
		
}
void displayTime(u16 x_start, u16 y_start)
{
		uint8_t *Partial_Time_Image;
		Partial_Time_Image = mymalloc(SRAMIN,(120 / 8 ) * (48+4));
		if (Partial_Time_Image == NULL)
    {
        printf("Failed to apply for partial memory...\r\n");
    }
		Paint_NewImage(Partial_Time_Image, 120, 52, 0, EPD_WHITE);
		Paint_Clear(EPD_WHITE);
		Paint_Show_Char(1+(48*1),2,':',48,1,1);
//		Paint_Show_Char(1+(48*2)+24,2,':',48,1,1);
		if(CalendarStruct.Time.Hour<10)
		{
			Paint_Show_xNum(1,5,0,48,1,0);
			Paint_Show_xNum(1+24,5,CalendarStruct.Time.Hour,48,1,0);
		}
		else
		{
			Paint_Show_xNum(1,5,CalendarStruct.Time.Hour,48,1,0);
		}
		
		if(CalendarStruct.Time.Min<10)
		{
			Paint_Show_xNum(1+(48*1)+24,5,0,48,1,0);
			Paint_Show_xNum(1+(48*2),5,CalendarStruct.Time.Min,48,1,0);
		}
		else
		{
			Paint_Show_xNum(1+(48*1)+24,5,CalendarStruct.Time.Min,48,1,0);
		}
		
//		if(CalendarStruct.Time.Sec<10)
//		{
//			Paint_Show_xNum(1+(48*3),5,0,48,1,0);
//			Paint_Show_xNum(1+(48*3)+24,5,CalendarStruct.Time.Sec,48,1,0);
//		}
//		else
//		{
//			Paint_Show_xNum(1+(48*3),5,CalendarStruct.Time.Sec,48,1,0);
//		}
		
		EPD_4IN2_V2_PartialDisplay(Partial_Time_Image, x_start, y_start, x_start+120, y_start+52 );
		myfree(SRAMIN,Partial_Time_Image);
		
}

// �����򣬳�ʼ������ʾ����
u8 new_date,old_date,new_min,old_min;

void initCalendar(u8 Calendar_x, u8 Calendar_y, u8 Time_x, u8 Time_y)
{
		RTC_TimeTypeDef RTC_TimeStruct; // ���ڴ洢ʱ��
    RTC_DateTypeDef RTC_DateStruct; // ���ڴ洢����

    // ��ȡ��ǰʱ��
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);

    // ��ȡ��ǰ����
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	
		CalendarStruct.Time.Year = RTC_DateStruct.RTC_Year+2000;
		CalendarStruct.Time.Month = RTC_DateStruct.RTC_Month;
		CalendarStruct.Time.Day = RTC_DateStruct.RTC_Date;
		CalendarStruct.Time.Hour = RTC_TimeStruct.RTC_Hours;
		CalendarStruct.Time.Min = RTC_TimeStruct.RTC_Minutes;
		CalendarStruct.Time.Sec = RTC_TimeStruct.RTC_Seconds;
//		printf("Hour:%d Min:%d Sec:%d \r\n",CalendarStruct.Time.Hour,CalendarStruct.Time.Min,CalendarStruct.Time.Sec);
		new_date = RTC_DateStruct.RTC_Date;
		new_min = RTC_TimeStruct.RTC_Minutes;
		if(new_min!=old_min)
		{
			displayTime(Time_x,Time_y);
			old_min = new_min;
		}
		if(new_date!= old_date)
		{
			initYear(&CalendarStruct,RTC_DateStruct.RTC_Year+2000,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date);
			displayCalendar(Calendar_x,Calendar_y);
			old_date = new_date;
		}
}
