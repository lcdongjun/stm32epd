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
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��20-1
 FreeRTOS�ڴ����ʵ��-�⺯���汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define MALLOC_TASK_PRIO	2
//�����ջ��С	
#define MALLOC_STK_SIZE 	512
//������
TaskHandle_t MallocTask_Handler;
//������
void malloc_task(void *p_arg);

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();							//��ʼ������
	LCD_Init();							//��ʼ��LCD
	W25QXX_Init();
	SPI2_Init();
	EPD_GPIO_Init();
	usmart_dev.init(84);		//��ʼ��USMART
	my_mem_init(SRAMIN);            	//��ʼ���ڲ��ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
	f_mount(fs[1],"1:",1); 				//����FLASH.
	font_init();
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
    taskENTER_CRITICAL();           //�����ٽ���
    //����TASK1����
    xTaskCreate((TaskFunction_t )malloc_task,             
                (const char*    )"malloc_task",           
                (uint16_t       )MALLOC_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )MALLOC_TASK_PRIO,        
                (TaskHandle_t*  )&MallocTask_Handler);   
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}


//MALLOC������ 
void malloc_task(void *pvParameters)
{
	POINT_COLOR=RED;    
	Show_Str(30,50,200,16,"̽����STM32F407������",16,0);

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
//		Paint_Show_Str(106,124,"ȷ��",24,1,0);
//		Paint_Show_Str(206,124,"ȡ��",24,1,0);
//		Paint_Show_RoundRect(100,120,160,152,5,3,1,0);
//		Paint_Show_RoundRect(200,120,260,152,5,3,1,0);
//		EPD_4IN2_V2_Display_Fast(Paint.Image);
		// 1. ���ƶ���������ʾ
		Paint_Show_Str(10, 5, "2023-12-23", 16, 1, 1);

		// 2. �������Ͻ���Ϣ��ʱ�䡢�������¶ȣ�
		Paint_Show_xNum(10, 30, 1834, 16, 1, 1);  // ʱ��
		Paint_Show_Str(10, 50, "5 / -8", 12, 1, 1);  // �¶ȷ�Χ

		// 3. ��������ʾ
		u8 days[7][4] = {"��", "һ", "��", "��", "��", "��", "��"};
		for (int i = 0; i < 7; i++) {
				Paint_Show_Str(60 + i * 40, 70, days[i], 12, 1, 1);
		}
		Paint_Show_Line(50, 90, 350, 90, 1);  // ���Ʒָ���

		// 4. ������������
		int x_offset = 60;
		int y_offset = 100;
		for (int day = 1; day <= 31; day++) {
				int x = x_offset + ((day - 1) % 7) * 40;
				int y = y_offset + ((day - 1) / 7) * 20;
				Paint_Show_xNum(x, y, day, 12, 1, 1);

				// �ж��Ƿ��ǵ�ǰ���ڣ����� 23��
				if (day == 23) {
						Paint_Show_Rectangle(x - 5, y - 5, x + 25, y + 15, 1, RED, 0);  // ��ɫ�߿�
				}
		}

		// 5. �ֲ�ˢ����ʾ
		EPD_4IN2_V2_Display_Fast(BlackImage);

	while(1)
	{
		LED0 = 0;
		vTaskDelay(1000);
		LED0 = 1;
		vTaskDelay(1000);	

	}

} 

