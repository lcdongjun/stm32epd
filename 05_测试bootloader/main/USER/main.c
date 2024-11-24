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
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define USART1_TASK_PRIO 	2
//任务堆栈大小	
#define USART1_STK_SIZE 	256
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

//任务优先级
#define Key_Reboot2_TASK_PRIO	2
//任务堆栈大小	
#define Key_Reboot2_STK_SIZE 	128
//任务句柄
TaskHandle_t Key_Reboot2Task_Handler;
//任务函数
void Key_Reboot2_task(void *pvParameters);


#define BOOT_FLAG_ADDRESS 0x2001FFF0  // RAM 最后部分的一个地址
#define BOOT_FLAG_VALUE   0xDEADBEEF  // 标志值

// 设置进入 Bootloader 标志
void SetBootFlag(void) {
    *(volatile uint32_t *)BOOT_FLAG_ADDRESS = BOOT_FLAG_VALUE;
}

// 清除进入 Bootloader 标志
void ClearBootFlag(void) {
    *(volatile uint32_t *)BOOT_FLAG_ADDRESS = 0;
}

void SystemReset(void) {
    // 设置 Boot 标志
    SetBootFlag();

    // 触发系统复位
    NVIC_SystemReset();
}


int main(void)
{ 
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);					//初始化延时函数
	uart_init(921600);     				//初始化串口
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
		DEV_Delay_ms(1000);
	
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
								
//		xReturn = xTaskCreate((TaskFunction_t )Key_Reboot2_task,             
//                (const char*    )"Key_Reboot2_task",           
//                (uint16_t       )Key_Reboot2_STK_SIZE,        
//                (void*          )NULL,                  
//                (UBaseType_t    )Key_Reboot2_TASK_PRIO,        
//                (TaskHandle_t*  )&Key_Reboot2Task_Handler); 
//								printf("TASK3 xReturn:%d\r\n",xReturn);	
								
								
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

u8 First_DMA_Flag = 1;

	/**************************串口接收并写入flash***************************
   uint32_t start_address = 0xF60000;   // W25Q128 的最后 0.5MB 起始地址
    uint32_t sector_start = start_address / 4096 * 4096; // 当前扇区起始地址
    uint8_t *write_buffer = NULL;       // 动态分配的写缓冲区

    while (1) {
        // 等待通知，表示接收到新数据
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			if(First_DMA_Flag==0)
			{
					// 动态分配缓冲区
					write_buffer = mymalloc(SRAMIN, received_len);
					if (write_buffer == NULL) {
							printf("Error: Memory allocation failed!\r\n");
							continue;
					}

					// 将接收到的数据复制到动态分配的缓冲区
					memcpy(write_buffer, RX_Buffer[processing_buf], received_len);

					// 检查是否需要擦除当前扇区
					if (start_address >= sector_start + 4096) {
							sector_start = start_address / 4096 * 4096;
							W25QXX_Erase_Sector(sector_start / 4096);
					}

					// 写入数据到 Flash
					if (start_address + received_len <= 0xFFFFFF) {
							W25QXX_Write(write_buffer, start_address, received_len);

							// 校验写入数据是否成功
							uint8_t *verify_buffer = mymalloc(SRAMIN, received_len);
							if (verify_buffer != NULL) {
									W25QXX_Read(verify_buffer, start_address, received_len);
									if (memcmp(write_buffer, verify_buffer, received_len) == 0) {
											printf("Write success at address 0x%X\r\n", start_address);
									} else {
											printf("Error: Data verification failed at address 0x%X\r\n", start_address);
									}
									myfree(SRAMIN, verify_buffer);
							} else {
									printf("Warning: Memory allocation for verification failed!\r\n");
							}

							// 更新地址
							start_address += received_len;
					} else {
							printf("Error: Write range exceeds W25Q128 limit!\r\n");
					}

					// 释放动态分配的缓冲区
					myfree(SRAMIN, write_buffer);
					write_buffer = NULL;

					// 如果超过 Flash 的最大地址，可以根据需求选择暂停任务或环形存储
					if (start_address > 0xFFFFFF) {
							printf("Flash write reached limit. Task suspended.\r\n");
							vTaskSuspend(NULL); // 暂停任务
					}
			}
			else
			{
				First_DMA_Flag = 0;
			}
    }
	*/
	
	/**************************flash读取并串口发送************************************
				uint32_t start_address = 0xF60000;  // 数据存储的起始地址
    uint32_t end_address = 0xF60D00;    // 数据存储的结束地址
    uint32_t current_address = start_address;
		uint8_t *read_buffer = NULL; 
		read_buffer = mymalloc(SRAMIN, 64);
    while (1) {
        // 计算本次读取的字节数，避免超出结束地址
        uint32_t bytes_to_read = (current_address + sizeof(read_buffer) <= end_address) 
                                 ? sizeof(read_buffer) 
                                 : (end_address - current_address + 1);

        // 从 Flash 读取数据
        W25QXX_Read(read_buffer, current_address, bytes_to_read);
																 
        memcpy(TX_Buffer, read_buffer, bytes_to_read);
        DMA_Cmd(DMA2_Stream7, DISABLE);  // 暂停发送 DMA
        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
        DMA2_Stream7->NDTR = bytes_to_read; // 设置待发送数据长度
        DMA_Cmd(DMA2_Stream7, ENABLE);    // 启动 DMA 发送	
        while (USART1_TC_Flag);  // 等待上次发送完成
        USART1_TC_Flag = 1;  // 标志位复位
																 
        current_address += bytes_to_read;
        // 如果读取到达结束地址，循环回到起始地址
        if (current_address > end_address) {
            current_address = start_address;
        }
				myfree(SRAMIN, read_buffer);
				vTaskDelay(pdMS_TO_TICKS(1)); // 延时 500ms（根据需要调整）
				
		}

		*/

/***************************串口接收串口发送************************************
    while (1) 
    {
        // 等待通知，确保任务不会频繁执行
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			if(!First_DMA_Flag)
			{
        // 检查接收到的有效数据
        if (received_len > 0 && received_len <= USART1_RX_BUF_SIZE) 
        {
            // 数据处理逻辑
            memcpy(TX_Buffer, RX_Buffer[processing_buf], received_len);
					
            // DMA 发送
            DMA_Cmd(DMA2_Stream7, DISABLE);
            DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
            DMA2_Stream7->NDTR = received_len;
            DMA_Cmd(DMA2_Stream7, ENABLE);

            // 等待 DMA 发送完成
            while (USART1_TC_Flag);
            USART1_TC_Flag = 1;  // 重置发送标志
        }
			}
			else{
			First_DMA_Flag = 0;
			}
    }													 
*/


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

void Key_Reboot2_task(void *pvParameters)
{
//	if(KEY2 == 0)
//	{
//		printf("Rebooting to Bootloader...\r\n");
//		SystemReset(); // 设置标志并复位
//	}
}

