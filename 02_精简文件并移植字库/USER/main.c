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

FATFS fs1;													/* FatFs�ļ�ϵͳ���� */
FIL fnew;													/* �ļ����� */
FRESULT res_flash;                /* �ļ�������� */
UINT fnum;            					  /* �ļ��ɹ���д���� */
BYTE ReadBuffer[1024]={0};        /* �������� */
BYTE WriteBuffer[] =              /* д������*/
"��ӭʹ��Ұ��STM32 F429������ �����Ǹ������ӣ��½��ļ�ϵͳ�����ļ�\n"
"����SPI-FatFs�ļ�ϵͳ\n"	;

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

	u32 res;
//	
//	exfuns_init();							//Ϊfatfs��ر��������ڴ�	
	res=f_mount(fs[1],"1:",1); 				//����FLASH.
	font_init();
	
//	
	POINT_COLOR=RED;    
	Show_Str(30,50,200,16,"̽����STM32F407������",16,0);
	
//	EPD_test();
//	/*----------------------- �ļ�ϵͳ���ԣ�д���� -----------------------------*/
//	/* ���ļ�������ļ��������򴴽��� */
//	printf("\r\n****** ���������ļ�д�����... ******\r\n");	
//	res_flash = f_open(&fnew, "1:FatFs��д�����ļ�.txt",FA_CREATE_ALWAYS | FA_WRITE );
//	if ( res_flash == FR_OK )
//	{
//		printf("����/����FatFs��д�����ļ�.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");
//    /* ��ָ���洢������д�뵽�ļ��� */
//		res_flash=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
//    if(res_flash==FR_OK)
//    {
//      printf("���ļ�д��ɹ���д���ֽ����ݣ�%d\n",fnum);
//      printf("�����ļ�д�������Ϊ��\r\n%s\r\n",WriteBuffer);
//    }
//    else
//    {
//      printf("�����ļ�д��ʧ�ܣ�(%d)\n",res_flash);
//    }    
//		/* ���ٶ�д���ر��ļ� */
//    f_close(&fnew);
//	}
//	else
//	{	
//		printf("������/�����ļ�ʧ�ܡ�\r\n");
//	}
	
///*------------------- �ļ�ϵͳ���ԣ������� ------------------------------------*/
//	printf("****** ���������ļ���ȡ����... ******\r\n");
//	res_flash = f_open(&fnew, "1:FatFs��д�����ļ�.txt", FA_OPEN_EXISTING | FA_READ); 	 
//	if(res_flash == FR_OK)
//	{
//		printf("�����ļ��ɹ���\r\n");
//		res_flash = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
//    if(res_flash==FR_OK)
//    {
//      printf("���ļ���ȡ�ɹ�,�����ֽ����ݣ�%d\r\n",fnum);
//      printf("����ȡ�õ��ļ�����Ϊ��\r\n%s \r\n", ReadBuffer);	
//    }
//    else
//    {
//      printf("�����ļ���ȡʧ�ܣ�(%d)\n",res_flash);
//    }		
//	}
//	else
//	{
//		printf("�������ļ�ʧ�ܡ�\r\n");
//	}
//	/* ���ٶ�д���ر��ļ� */
//	f_close(&fnew);	
	
	while(1)
	{
		LED0 = 0;
		vTaskDelay(1000);
		LED0 = 1;
		vTaskDelay(1000);	
	}

} 

