#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "key.h"
#include "beep.h"
#include "string.h"
#include "malloc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spi.h"
#include "w25qxx.h" 
#include "ff.h"  
#include "exfuns.h"  
#include "sdio_sdcard.h"
#include "usmart.h"  
#include "DEV_Config.h"
#include "EPD_4in2_V2.h"
#include "EPD_Test.h"
#include "fontupd.h"
#include "text.h"	

/************************************************
 ALIENTEK 探索者STM32F407开发板 FreeRTOS实验20-1
 FreeRTOS内存管理实验-库函数版本
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define MALLOC_TASK_PRIO	2
//任务堆栈大小	
#define MALLOC_STK_SIZE 	512
//任务句柄
TaskHandle_t MallocTask_Handler;
//任务函数
void malloc_task(void *p_arg);

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);					//初始化延时函数
	uart_init(115200);     				//初始化串口
	LED_Init();		        			//初始化LED端口
	KEY_Init();							//初始化按键
	LCD_Init();							//初始化LCD
	W25QXX_Init();
	SPI2_Init();
	EPD_GPIO_Init();
	usmart_dev.init(84);		//初始化USMART
	my_mem_init(SRAMIN);            	//初始化内部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
	f_mount(fs[1],"1:",1); 				//挂载FLASH.
	font_init();
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建TASK1任务
    xTaskCreate((TaskFunction_t )malloc_task,             
                (const char*    )"malloc_task",           
                (uint16_t       )MALLOC_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )MALLOC_TASK_PRIO,        
                (TaskHandle_t*  )&MallocTask_Handler);   
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}


//MALLOC任务函数 
void malloc_task(void *pvParameters)
{
	POINT_COLOR=RED;    
	Show_Str(30,50,200,16,"探索者STM32F407开发板",16,0);

//	EPD_test();
	
    DEV_Module_Init();
    EPD_4IN2_V2_Init();
		EPD_4IN2_V2_Init_Fast(Seconds_1S);
    DEV_Delay_ms(500);
		uint8_t *BlackImage;
    BlackImage = mymalloc(SRAMIN, ((EPD_4IN2_V2_WIDTH % 8 == 0) ? (EPD_4IN2_V2_WIDTH / 8) : (EPD_4IN2_V2_WIDTH / 8 + 1)) * EPD_4IN2_V2_HEIGHT);
    if (BlackImage == NULL)
    {
        printf("Failed to apply for black memory...\r\n");
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_4IN2_V2_WIDTH, EPD_4IN2_V2_HEIGHT, 0, EPD_WHITE);
		Paint_SelectImage(BlackImage);
    Paint_Clear(EPD_WHITE);
		DEV_Delay_ms(500);
//		Paint_Show_Str(106,124,"确认",24,1,0);
//		Paint_Show_Str(206,124,"取消",24,1,0);
//		Paint_Show_RoundRect(100,120,160,152,5,3,1,0);
//		Paint_Show_RoundRect(200,120,260,152,5,3,1,0);
//		EPD_4IN2_V2_Display_Fast(Paint.Image);
		// 1. 绘制顶部日期显示
		Paint_Show_Str(10, 5, "2023-12-23", 16, 1, 1);

		// 2. 绘制左上角信息（时间、天气、温度）
		Paint_Show_xNum(10, 30, 1834, 16, 1, 1);  // 时间
		Paint_Show_Str(10, 50, "5 / -8", 12, 1, 1);  // 温度范围

		// 3. 绘制周显示
		u8 days[7][4] = {"日", "一", "二", "三", "四", "五", "六"};
		for (int i = 0; i < 7; i++) {
				Paint_Show_Str(60 + i * 40, 70, days[i], 12, 1, 1);
		}
		Paint_Show_Line(50, 90, 350, 90, 1);  // 绘制分割线

		// 4. 绘制日历数字
		int x_offset = 60;
		int y_offset = 100;
		for (int day = 1; day <= 31; day++) {
				int x = x_offset + ((day - 1) % 7) * 40;
				int y = y_offset + ((day - 1) / 7) * 20;
				Paint_Show_xNum(x, y, day, 12, 1, 1);

				// 判断是否是当前日期，例如 23号
				if (day == 23) {
						Paint_Show_Rectangle(x - 5, y - 5, x + 25, y + 15, 1, RED, 0);  // 红色边框
				}
		}

		// 5. 局部刷新显示
		EPD_4IN2_V2_Display_Fast(BlackImage);

	while(1)
	{
		LED0 = 0;
		vTaskDelay(1000);
		LED0 = 1;
		vTaskDelay(1000);	

	}

} 

