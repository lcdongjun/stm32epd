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

#define APP_START_ADDRESS 0x08008000 // �û�������ʼ��ַ

// ��־λ����
#define BOOT_FLAG_NONE 0x00000000		// �ޱ�־��ֱ�ӽ����û�����
#define FLASH_PAGE_SIZE 131072 // STM32F407 ÿҳ Flash ��СΪ 2KB
#define BOOT_FLAG_ADDRESS 0x2001FFF0  // ���û�������һ��
#define BOOT_FLAG_VALUE   0xDEADBEEF  // ���û�������һ��

// ���ý��� Bootloader ��־
void SetBootFlag(void) {
    *(volatile uint32_t *)BOOT_FLAG_ADDRESS = BOOT_FLAG_VALUE;
}

// ������� Bootloader ��־
void ClearBootFlag(void) {
    *(volatile uint32_t *)BOOT_FLAG_ADDRESS = 0;
}

uint8_t CheckBootFlag(void) {
    if (*(volatile uint32_t *)BOOT_FLAG_ADDRESS == BOOT_FLAG_VALUE) {
        // �����־�������ٴν��� Bootloader
        ClearBootFlag();
        return 1; // ��־��Ч
    }
    return 0; // ��־��Ч
}

// ���� Flash
void STM32_Flash_Init(void)
{
	FLASH_Unlock();
}

// ���� Flash
void STM32_Flash_DeInit(void)
{
	FLASH_Lock();
}

// ���� Flash ����
u8 STM32_Flash_Erase(uint32_t startAddress, uint32_t binSize)
{
		uint32_t sectorAddress = startAddress;      // ��ǰ��ַ
    uint32_t endAddress = startAddress + binSize; // ���������ַ
    uint8_t sector = 0;                        // �������
    while (sectorAddress < endAddress) {
        // �жϵ�ǰ��ַ�����ĸ���������������ȷ���������
        if (sectorAddress < 0x0800C000) {
            sector = FLASH_Sector_2;            // ���� 2
            sectorAddress += 0x4000;           // 16KB
        } else if (sectorAddress < 0x08010000) {
            sector = FLASH_Sector_3;            // ���� 3
            sectorAddress += 0x4000;           // 16KB
        } else if (sectorAddress < 0x08020000) {
            sector = FLASH_Sector_4;            // ���� 4
            sectorAddress += 0x10000;          // 64KB
        } else if (sectorAddress < 0x08040000) {
            sector = FLASH_Sector_5;            // ���� 5
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x08060000) {
            sector = FLASH_Sector_6;            // ���� 6
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x08080000) {
            sector = FLASH_Sector_7;            // ���� 7
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x080A0000) {
            sector = FLASH_Sector_8;            // ���� 8
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x080C0000) {
            sector = FLASH_Sector_9;            // ���� 9
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x080E0000) {
            sector = FLASH_Sector_10;           // ���� 10
            sectorAddress += 0x20000;          // 128KB
        } else {
            sector = FLASH_Sector_11;           // ���� 11
            sectorAddress += 0x20000;          // 128KB
        }

        // ������ǰ����
        printf("Erasing Flash Sector: %X\r\n", sector);
        FLASH_EraseSector(sector, VoltageRange_3);
    }

    // ���� Flash
    return 0;
}

// д�� Flash ����
void STM32_Flash_Write(uint32_t address, uint8_t *data, uint32_t length)
{
	for (uint32_t i = 0; i < length; i += 4)
	{
		uint32_t word = *(uint32_t *)(data + i);
		FLASH_ProgramWord(address + i, word);
	}
}

uint32_t flashWriteAddress = 0x08008000;
uint32_t binSize = 0; // ��¼���յ����ܴ�С
u8 First_DMA_Flag = 1;

void BootUsartFlash(void)
{
	u8 flash_size_flag = 1;
	uint32_t flashStartAddress = 0x08008000; // �û�������ʼ��ַ
	STM32_Flash_Init();
	printf("Erasing Flash...\r\n");
	while (1)
	{
		if (KEY2 == 0)
		{
			//�˳���¼
			STM32_Flash_DeInit();
			printf("Flash End\r\n");
			break;
		}
		// �������ݲ�д�� Flash
		if (data_ready_flag)
		{
			// ���ݽ�����ɱ�־
			data_ready_flag = 0;

			// ��ȡ�������ݵĳ���
			uint32_t receivedLen = received_len; // ���յ����ֽڳ���

			// У���Ƿ񳬳� Flash ��С
			if ((flashWriteAddress + receivedLen) > 0x080FFFFF)
			{
				printf("Error: Exceeding Flash size!\r\n");
				while (1)
					;
			}

			if (!First_DMA_Flag)
			{

				if (flash_size_flag)
				{
					// ��ȡ�����С���Ա������Ӧflash
					printf("size: %d\r\n", atoi((const char *)RX_Buffer[processing_buf]));
					STM32_Flash_Erase(flashStartAddress, atoi((const char *)RX_Buffer[processing_buf])); // BIN �ļ���С
					printf("Erasing Flash OK\r\n");
					flash_size_flag = 0;
					continue;
				}
				// д�� Flash
				STM32_Flash_Write(flashWriteAddress, RX_Buffer[processing_buf], receivedLen);
				printf("Write successful at 0x%X, total size: %d bytes\r\n", flashWriteAddress, binSize);
				// ����д���ַ���ܴ�С
				flashWriteAddress += receivedLen;
				binSize += receivedLen;
			}
			else
			{
				First_DMA_Flag = 0;
				printf("Flash Start\r\n");
				printf("Input UserApp Size:\r\n");
			}
		}
	}
}

void JumpToUserApplication(void)
{
	uint32_t appStackPointer = *(volatile uint32_t *)APP_START_ADDRESS; // �û������ MSP ֵ
	void (*AppEntryPoint)(void);										// ����һ������ָ��

	// �ж��û������Ƿ���Ч��ͨ���Ǽ�� MSP ֵ�Ƿ����
	if ((appStackPointer & 0x2FFE0000) != 0x20000000)
	{			// RAM ������ʼ��ַ���
		return; // �����Ч������ת
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
	while ((RCC->CR & RCC_CR_HSIRDY) == 0)
		;					// �ȴ� HSI ����
	RCC->CFGR = 0x00000000; // ϵͳʱ���л��� HSI

	// �ر� PLL
	RCC->CR &= ~RCC_CR_PLLON;
	while (RCC->CR & RCC_CR_PLLRDY)
		; // �ȴ� PLL �ر�

	// �ر������жϲ���������־
	for (uint32_t i = 0; i < 8; i++)
	{
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
	while (1)
		;
}

uint8_t IsUserProgramValid(void) 
{
    uint32_t appStack = *(volatile uint32_t *)0x08008000; // �û�����ջ����ַ
    uint32_t appResetHandler = *(volatile uint32_t *)(0x08008000 + 4); // �û�����λ����

    // ջָ���飨�Ƿ��� SRAM ��Χ�ڣ�
    if (appStack < 0x20000000 || appStack > 0x20020000) {
        return 0; // ��Ч
    }

    // ��λ������飨�Ƿ��� Flash ��Χ�ڣ�
    if (appResetHandler < 0x08008000 || appResetHandler > 0x080FFFFF) {
        return 0; // ��Ч
    }

    return 1; // ��Ч
}


void BootloaderMain(void)
{
	while (1)
	{
		if (KEY1 == 0)
		{
			// ��¼�û�����
			BootUsartFlash();
		}
		if (KEY0 == 0)
		{
			// ��¼��ɺ���ת���û�����
			printf("UserApp Starting ...\r\n");
			JumpToUserApplication();
		}
		
	}
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // ����ϵͳ�ж����ȼ�����4
	delay_init(168);								// ��ʼ����ʱ����
	uart_init(921600);								// ��ʼ������
	LED_Init();										// ��ʼ��LED�˿�
	KEY_Init();
	W25QXX_Init();
	my_mem_init(SRAMIN);
	
	if (WK_UP == 1) 
	{
		printf("\r\nButton pressed. Staying in Bootloader.\r\n");
		BootloaderMain(); // ִ�� Bootloader ����
	}
	else
	{
			if (CheckBootFlag()) 
				{
					printf("\r\n   Boot flag detected. Staying in Bootloader.\r\n");
					BootloaderMain(); // ִ�� Bootloader ����
				}
			else if (IsUserProgramValid())
			{
					printf("\r\nEnter Key_UP to BootLoader\r\n");
					printf("Valid user program found. Jumping to user application...\r\n");
					JumpToUserApplication();
			} 
			else if(!IsUserProgramValid())
			{
					printf("\r\nNo valid user program found. Staying in Bootloader.\r\n");
					printf("User application Err\r\n");
					printf("Bootloader Starting\r\n");
					BootloaderMain(); // ִ�� Bootloader ����
			}
	}	
	//�����ϲ���ִ�е�����
	while (1);

}
