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

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		256  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define USART1_TASK_PRIO 	2
//�����ջ��С	
#define USART1_STK_SIZE 	512
//������
TaskHandle_t USART1_ReceiveTask_Handler;
//������
void USART1_ReceiveTask(void *pvParameters);

//�������ȼ�
#define Keep_time_TASK_PRIO	2
//�����ջ��С	
#define Keep_time_STK_SIZE 	512
//������
TaskHandle_t Keep_timeTask_Handler;
//������
void Keep_time_task(void *pvParameters);



int main(void)
{ 
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(460800);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();							//��ʼ������
//	LCD_Init();							//��ʼ��LCD
	W25QXX_Init();
	SPI2_Init();
	EPD_GPIO_Init();
	usmart_dev.init(84);		//��ʼ��USMART
	My_RTC_Init();		 		//��ʼ��RTC
	my_mem_init(SRAMIN);            	//��ʼ���ڲ��ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
	f_mount(fs[1],"1:",1); 				//����FLASH.
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
	
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
		 u8 xReturn;
		
    taskENTER_CRITICAL();           //�����ٽ���
    //����TASK1����
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
								
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}


//// ���ݽ�������
void USART1_HandleReceivedData(uint8_t *data, uint16_t length) {
    // ����Э���������
    // ʾ���������ݴ�ӡ�����Դ���
//    printf("Received data (len: %d): ", length);
    for (uint16_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}


void USART1_ReceiveTask(void *pvParameters) 
{
   while (1) {
        // �ȴ���������֪ͨ
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // ���ͽ��յ�������
        memcpy(TX_Buffer, RX_Buffer[processing_buf], received_len);
        DMA_Cmd(DMA2_Stream7, DISABLE);  // ��ͣ���� DMA
        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
        DMA2_Stream7->NDTR = received_len; // ���ô��������ݳ���
        DMA_Cmd(DMA2_Stream7, ENABLE);    // ���� DMA ����

        // �ȴ� DMA ������ɣ��������ź�����
        while (USART1_TC_Flag);  // �ȴ��ϴη������
        USART1_TC_Flag = 1;  // ��־λ��λ
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
