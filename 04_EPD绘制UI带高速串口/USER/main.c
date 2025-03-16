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
#include "queue.h"
#include "semphr.h"
#include "spi.h"
#include "w25qxx.h" 
#include "rtc.h"
#include "ff.h"  
#include "exfuns.h"  
#include "sdio_sdcard.h"
#include "usmart.h"  
#include "DEV_Config.h"
#include "EPD_4in2_V2.h"
#include "EPD_Test.h"
#include "fontupd.h"
#include "text.h"	
#include "calendar.h"


extern Calendar CalendarStruct;

SemaphoreHandle_t rtcSemaphore;
SemaphoreHandle_t USART1_RxSemaphore;

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		256  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define USART1_TASK_PRIO 	2
//任务堆栈大小	
#define USART1_STK_SIZE 	512
//任务句柄
TaskHandle_t USART1_ReceiveTask_Handler;
//任务函数
void USART1_ReceiveTask(void *pvParameters);

//任务优先级
#define Keep_time_TASK_PRIO	2
//任务堆栈大小	
#define Keep_time_STK_SIZE 	512
//任务句柄
TaskHandle_t Keep_timeTask_Handler;
//任务函数
void Keep_time_task(void *pvParameters);



int main(void)
{ 
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);					//初始化延时函数
	uart_init(460800);     				//初始化串口
	LED_Init();		        			//初始化LED端口
	KEY_Init();							//初始化按键
//	LCD_Init();							//初始化LCD
	W25QXX_Init();
	SPI2_Init();
	EPD_GPIO_Init();
	usmart_dev.init(84);		//初始化USMART
	My_RTC_Init();		 		//初始化RTC
	my_mem_init(SRAMIN);            	//初始化内部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
	f_mount(fs[1],"1:",1); 				//挂载FLASH.
	font_init();
	
	rtcSemaphore = xSemaphoreCreateBinary();
	USART1_RxSemaphore = xSemaphoreCreateBinary();
	
		DEV_Module_Init();
		EPD_4IN2_V2_Init();
		DEV_Delay_ms(500);
		uint8_t *BlackImage;
		BlackImage = mymalloc(SRAMIN, ((EPD_4IN2_V2_WIDTH % 8 == 0) ? (EPD_4IN2_V2_WIDTH / 8) : (EPD_4IN2_V2_WIDTH / 8 + 1)) * EPD_4IN2_V2_HEIGHT);
		if (BlackImage == NULL)
		{
				printf("Failed to apply for black memory...\r\n");
		}	
		
		Paint_NewImage(BlackImage, EPD_4IN2_V2_WIDTH, EPD_4IN2_V2_HEIGHT, 0, EPD_WHITE);
		Paint_SelectImage(BlackImage);
		Paint_Clear(EPD_WHITE);
		EPD_4IN2_V2_Display(BlackImage);
//		DEV_Delay_ms(1000);
	
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
		 u8 xReturn;
		
    taskENTER_CRITICAL();           //进入临界区
    //创建TASK1任务
    xReturn = xTaskCreate((TaskFunction_t )USART1_ReceiveTask,             
                (const char*    )"USART1_Receive_task",           
                (uint16_t       )USART1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )USART1_TASK_PRIO,        
                (TaskHandle_t*  )&USART1_ReceiveTask_Handler);   
								
								printf("\r\nTASK1 xReturn:%d\r\n",xReturn);
								
		xReturn = xTaskCreate((TaskFunction_t )Keep_time_task,             
                (const char*    )"Keep_time_task",           
                (uint16_t       )Keep_time_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )Keep_time_TASK_PRIO,        
                (TaskHandle_t*  )&Keep_timeTask_Handler); 
								printf("TASK2 xReturn:%d\r\n",xReturn);	
								
								RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,1);
								
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}


//// 数据解析函数
void USART1_HandleReceivedData(uint8_t *data, uint16_t length) {
    // 根据协议解析数据
    // 示例：将数据打印到调试串口
//    printf("Received data (len: %d): ", length);
    for (uint16_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}


void USART1_ReceiveTask(void *pvParameters) 
{
   while (1) {
        // 等待接收任务通知
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // 发送接收到的数据
        memcpy(TX_Buffer, RX_Buffer[processing_buf], received_len);
        DMA_Cmd(DMA2_Stream7, DISABLE);  // 暂停发送 DMA
        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
        DMA2_Stream7->NDTR = received_len; // 设置待发送数据长度
        DMA_Cmd(DMA2_Stream7, ENABLE);    // 启动 DMA 发送

        // 等待 DMA 发送完成（可以用信号量）
        while (USART1_TC_Flag);  // 等待上次发送完成
        USART1_TC_Flag = 1;  // 标志位复位
    }
}


void Keep_time_task(void *pvParameters)
{
	while(1)
	{
		initCalendar(160,110,220,40);
		if (xSemaphoreTake(rtcSemaphore, portMAX_DELAY) == pdTRUE) {
				initCalendar(160,110,220,40);
		}
	}
}
