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
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define USART1_TASK_PRIO 	2
//�����ջ��С	
#define USART1_STK_SIZE 	256
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

//�������ȼ�
#define Key_Reboot2_TASK_PRIO	2
//�����ջ��С	
#define Key_Reboot2_STK_SIZE 	128
//������
TaskHandle_t Key_Reboot2Task_Handler;
//������
void Key_Reboot2_task(void *pvParameters);


#define BOOT_FLAG_ADDRESS 0x2001FFF0  // RAM ��󲿷ֵ�һ����ַ
#define BOOT_FLAG_VALUE   0xDEADBEEF  // ��־ֵ

// ���ý��� Bootloader ��־
void SetBootFlag(void) {
    *(volatile uint32_t *)BOOT_FLAG_ADDRESS = BOOT_FLAG_VALUE;
}

// ������� Bootloader ��־
void ClearBootFlag(void) {
    *(volatile uint32_t *)BOOT_FLAG_ADDRESS = 0;
}

void SystemReset(void) {
    // ���� Boot ��־
    SetBootFlag();

    // ����ϵͳ��λ
    NVIC_SystemReset();
}


int main(void)
{ 
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(921600);     				//��ʼ������
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
		DEV_Delay_ms(1000);
	
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
								
//		xReturn = xTaskCreate((TaskFunction_t )Key_Reboot2_task,             
//                (const char*    )"Key_Reboot2_task",           
//                (uint16_t       )Key_Reboot2_STK_SIZE,        
//                (void*          )NULL,                  
//                (UBaseType_t    )Key_Reboot2_TASK_PRIO,        
//                (TaskHandle_t*  )&Key_Reboot2Task_Handler); 
//								printf("TASK3 xReturn:%d\r\n",xReturn);	
								
								
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

u8 First_DMA_Flag = 1;

	/**************************���ڽ��ղ�д��flash***************************
   uint32_t start_address = 0xF60000;   // W25Q128 ����� 0.5MB ��ʼ��ַ
    uint32_t sector_start = start_address / 4096 * 4096; // ��ǰ������ʼ��ַ
    uint8_t *write_buffer = NULL;       // ��̬�����д������

    while (1) {
        // �ȴ�֪ͨ����ʾ���յ�������
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			if(First_DMA_Flag==0)
			{
					// ��̬���仺����
					write_buffer = mymalloc(SRAMIN, received_len);
					if (write_buffer == NULL) {
							printf("Error: Memory allocation failed!\r\n");
							continue;
					}

					// �����յ������ݸ��Ƶ���̬����Ļ�����
					memcpy(write_buffer, RX_Buffer[processing_buf], received_len);

					// ����Ƿ���Ҫ������ǰ����
					if (start_address >= sector_start + 4096) {
							sector_start = start_address / 4096 * 4096;
							W25QXX_Erase_Sector(sector_start / 4096);
					}

					// д�����ݵ� Flash
					if (start_address + received_len <= 0xFFFFFF) {
							W25QXX_Write(write_buffer, start_address, received_len);

							// У��д�������Ƿ�ɹ�
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

							// ���µ�ַ
							start_address += received_len;
					} else {
							printf("Error: Write range exceeds W25Q128 limit!\r\n");
					}

					// �ͷŶ�̬����Ļ�����
					myfree(SRAMIN, write_buffer);
					write_buffer = NULL;

					// ������� Flash ������ַ�����Ը�������ѡ����ͣ������δ洢
					if (start_address > 0xFFFFFF) {
							printf("Flash write reached limit. Task suspended.\r\n");
							vTaskSuspend(NULL); // ��ͣ����
					}
			}
			else
			{
				First_DMA_Flag = 0;
			}
    }
	*/
	
	/**************************flash��ȡ�����ڷ���************************************
				uint32_t start_address = 0xF60000;  // ���ݴ洢����ʼ��ַ
    uint32_t end_address = 0xF60D00;    // ���ݴ洢�Ľ�����ַ
    uint32_t current_address = start_address;
		uint8_t *read_buffer = NULL; 
		read_buffer = mymalloc(SRAMIN, 64);
    while (1) {
        // ���㱾�ζ�ȡ���ֽ��������ⳬ��������ַ
        uint32_t bytes_to_read = (current_address + sizeof(read_buffer) <= end_address) 
                                 ? sizeof(read_buffer) 
                                 : (end_address - current_address + 1);

        // �� Flash ��ȡ����
        W25QXX_Read(read_buffer, current_address, bytes_to_read);
																 
        memcpy(TX_Buffer, read_buffer, bytes_to_read);
        DMA_Cmd(DMA2_Stream7, DISABLE);  // ��ͣ���� DMA
        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
        DMA2_Stream7->NDTR = bytes_to_read; // ���ô��������ݳ���
        DMA_Cmd(DMA2_Stream7, ENABLE);    // ���� DMA ����	
        while (USART1_TC_Flag);  // �ȴ��ϴη������
        USART1_TC_Flag = 1;  // ��־λ��λ
																 
        current_address += bytes_to_read;
        // �����ȡ���������ַ��ѭ���ص���ʼ��ַ
        if (current_address > end_address) {
            current_address = start_address;
        }
				myfree(SRAMIN, read_buffer);
				vTaskDelay(pdMS_TO_TICKS(1)); // ��ʱ 500ms��������Ҫ������
				
		}

		*/

/***************************���ڽ��մ��ڷ���************************************
    while (1) 
    {
        // �ȴ�֪ͨ��ȷ�����񲻻�Ƶ��ִ��
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			if(!First_DMA_Flag)
			{
        // �����յ�����Ч����
        if (received_len > 0 && received_len <= USART1_RX_BUF_SIZE) 
        {
            // ���ݴ����߼�
            memcpy(TX_Buffer, RX_Buffer[processing_buf], received_len);
					
            // DMA ����
            DMA_Cmd(DMA2_Stream7, DISABLE);
            DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
            DMA2_Stream7->NDTR = received_len;
            DMA_Cmd(DMA2_Stream7, ENABLE);

            // �ȴ� DMA �������
            while (USART1_TC_Flag);
            USART1_TC_Flag = 1;  // ���÷��ͱ�־
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
//		SystemReset(); // ���ñ�־����λ
//	}
}

