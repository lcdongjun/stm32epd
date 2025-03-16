#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "string.h"
#include "spi.h"
#include "w25qxx.h" 
#include "malloc.h"	   

#define BOOTLOADER_FLAG_ADDR  0x2001FFFC
#define APP_START_ADDRESS     0x08008000  // �û�������ʼ��ַ

// ��־λ����
#define BOOT_FLAG_BOOTLOADER  0xDEADBEEF  // ����Bootloader�ı�־
#define BOOT_FLAG_NONE        0x00000000  // �ޱ�־��ֱ�ӽ����û�����

#define FLASH_PAGE_SIZE 2048 // STM32F407 ÿҳ Flash ��СΪ 2KB


void BurnBinToFlash(uint32_t srcAddr, uint32_t destAddr, uint32_t size) {
    uint8_t buffer[256]; // ��ʱ������
    uint32_t bytesRead = 0;

    // ���� Flash
    FLASH_Unlock();

    // ����Ŀ������
    for (uint32_t addr = destAddr; addr < (destAddr + size); addr += FLASH_PAGE_SIZE) {
        FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3); // ����һ������������Ŀ�� MCU ��������С��
    }

    // д�� Flash
    while (bytesRead < size) {
        uint16_t chunkSize = (size - bytesRead > sizeof(buffer)) ? sizeof(buffer) : (size - bytesRead);

        // �� W25Q128 ��ȡ����
        W25QXX_Read(buffer, srcAddr + bytesRead, chunkSize);
			
			
				memcpy(TX_Buffer, buffer, chunkSize);
        DMA_Cmd(DMA2_Stream7, DISABLE);  // ��ͣ���� DMA
        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
        DMA2_Stream7->NDTR = chunkSize; // ���ô��������ݳ���
        DMA_Cmd(DMA2_Stream7, ENABLE);    // ���� DMA ����	
        while (USART1_TC_Flag);  // �ȴ��ϴη������
        USART1_TC_Flag = 1;  // ��־λ��λ
			
        // д�뵽 STM32 Flash
        for (uint16_t i = 0; i < chunkSize; i += 4) { // Flash д�밴 4 �ֽڶ���
            uint32_t word = *(uint32_t *)(buffer + i);
            FLASH_ProgramWord(destAddr + bytesRead + i, word);
        }

        bytesRead += chunkSize;
    }

    // ���� Flash
    FLASH_Lock();
}

void BootFlashMain(void) {
    // �� W25Q128 ����ʼ��ַ��ȡ BIN �ļ�����д�뵽 STM32 �� Flash
    uint32_t binStartAddress = 0xF60000; // W25Q128 �� BIN �ļ�����ʼ��ַ
    uint32_t flashStartAddress = 0x08008000; // �û��������ʼ��ַ
    uint32_t binSize = 0x790; // BIN �ļ���С
    printf("��ʼ��¼bin");
    BurnBinToFlash(binStartAddress, flashStartAddress, binSize);
}


void JumpToUserApplication(void) 
{
    uint32_t appStackPointer = *(volatile uint32_t *)APP_START_ADDRESS; // �û������ MSP ֵ
    void (*AppEntryPoint)(void);  // ����һ������ָ��

    // �ж��û������Ƿ���Ч��ͨ���Ǽ�� MSP ֵ�Ƿ����
    if ((appStackPointer & 0x2FFE0000) != 0x20000000) { // RAM ������ʼ��ַ���
        return;  // �����Ч������ת
    }

    // �ر�ȫ���ж�
    __disable_irq();

    // �ر� SysTick����λ��Ĭ��ֵ
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // �ر�����ʱ��
    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->APB1ENR = 0;
    RCC->APB2ENR = 0;

    // �л�ϵͳʱ�ӵ� HSI
    RCC->CR |= RCC_CR_HSION; // ���� HSI
    while ((RCC->CR & RCC_CR_HSIRDY) == 0); // �ȴ� HSI ����
    RCC->CFGR = 0x00000000; // ϵͳʱ���л��� HSI

    // �ر� PLL
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY); // �ȴ� PLL �ر�

    // �ر������жϲ���������־
    for (uint32_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF; // ���������ж�
        NVIC->ICPR[i] = 0xFFFFFFFF; // ������й����ж�
    }

    // ��������ջָ�루MSP��
    __set_MSP(appStackPointer);

    // �����û�������ڵ�ַ����λ������ַ��
    AppEntryPoint = (void (*)(void))(*(volatile uint32_t *)(APP_START_ADDRESS + 4));

    // ����ȫ���жϣ��û����������Ҫ��
    __enable_irq();

    // ��ת���û�����
    AppEntryPoint();

    // �����תʧ�ܣ������ϲ���ִ�е����
    while (1);
}


// ����Ƿ���Ҫ���� Bootloader
int CheckBootloaderFlag(void) 
{
    uint32_t flag = *(volatile uint32_t *)BOOTLOADER_FLAG_ADDR;
    return (flag == BOOT_FLAG_BOOTLOADER);
}

// ���������־
void ClearBootloaderFlag(void) 
{
    *(volatile uint32_t *)BOOTLOADER_FLAG_ADDR = BOOT_FLAG_NONE;
}

u8 First_DMA_Flag = 1;
uint32_t start_address = 0xF60000;   // W25Q128 ����� 0.5MB ��ʼ��ַ
uint8_t *write_buffer = NULL;       // ��̬�����д������
uint32_t sector_start;

void BootUsartFlah(void)
{	
		// ģ�� Bootloader ����
		if (data_ready_flag) 
		{
			data_ready_flag = 0;
			if(!First_DMA_Flag)
			{
        // ��̬���仺����
        write_buffer = mymalloc(SRAMIN, received_len);
        if (write_buffer == NULL) {
            printf("Error: Memory allocation failed!\r\n");
					while(1);
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
        }
			}
			else
			{
			First_DMA_Flag = 0;
			sector_start = start_address / 4096 * 4096; // ��ǰ������ʼ��ַ
			}
		}	
}

void BootFlahtoUsart()
{
		uint32_t start_address = 0xF60000;  // ���ݴ洢����ʼ��ַ
    uint32_t end_address = 0xF60790;    // ���ݴ洢�Ľ�����ַ
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
        // �����ȡ���������ַ���˳�ѭ��
        if (current_address > end_address) {
            break;
        }
				myfree(SRAMIN, read_buffer);	
				delay_ms(5);
		}
}
void BootloaderMain(void) 
{
    // �ڴ˵ȴ��¹̼���ͨ�����ڡ�SPI �ȷ�ʽ���գ�
	while (1) 
	{
		BootUsartFlah();
		if(KEY2 == 0)
		{
			// ��ȡ�û����򣬴��ڷ���
			BootFlahtoUsart();
		}
		if(KEY1 == 0)
		{
			// ��¼�û�����
			BootFlashMain();
		}
		if(KEY0 == 0)
		{
			// ��¼��ɺ���ת���û�����
			JumpToUserApplication();
		}
		
	}
}


int main(void)
{ 
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(921600);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();
	W25QXX_Init(); 
	my_mem_init(SRAMIN);
	printf("Bootloader startup");
	BootloaderMain();	
	while(1)
	{	
	}
}

void USART1_ReceiveTask(void *pvParameters) 
{
	/**************************���ڽ��ղ�д��flash***************************
	
   uint32_t start_address = 0xF60000;   // W25Q128 ����� 0.5MB ��ʼ��ַ
    uint32_t sector_start = start_address / 4096 * 4096; // ��ǰ������ʼ��ַ
    uint8_t *write_buffer = NULL;       // ��̬�����д������

    while (1) {
        // �ȴ�֪ͨ����ʾ���յ�������
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

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
	*/
	
	/**************************flash��ȡ�����ڷ���************************************
	
		uint32_t start_address = 0xF60000;  // ���ݴ洢����ʼ��ַ
    uint32_t end_address = 0xF6004D;    // ���ݴ洢�Ľ�����ַ
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
				vTaskDelay(pdMS_TO_TICKS(500)); // ��ʱ 500ms��������Ҫ������
				
		}
		*/
    while (1) 
    {
        // �ȴ�֪ͨ��ȷ�����񲻻�Ƶ��ִ��
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
}

//        // �ȴ���������֪ͨ
//        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//        // ���ͽ��յ�������
//        memcpy(TX_Buffer, RX_Buffer[processing_buf], received_len);
//        DMA_Cmd(DMA2_Stream7, DISABLE);  // ��ͣ���� DMA
//        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
//        DMA2_Stream7->NDTR = received_len; // ���ô��������ݳ���
//        DMA_Cmd(DMA2_Stream7, ENABLE);    // ���� DMA ����

//        // �ȴ� DMA ������ɣ��������ź�����
//        while (USART1_TC_Flag);  // �ȴ��ϴη������
//        USART1_TC_Flag = 1;  // ��־λ��λ
