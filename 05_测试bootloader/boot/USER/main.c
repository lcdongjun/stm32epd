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

#define APP_START_ADDRESS 0x08008000 // 用户程序起始地址

// 标志位定义
#define BOOT_FLAG_NONE 0x00000000		// 无标志，直接进入用户程序
#define FLASH_PAGE_SIZE 131072 // STM32F407 每页 Flash 大小为 2KB
#define BOOT_FLAG_ADDRESS 0x2001FFF0  // 与用户程序中一致
#define BOOT_FLAG_VALUE   0xDEADBEEF  // 与用户程序中一致

// 设置进入 Bootloader 标志
void SetBootFlag(void) {
    *(volatile uint32_t *)BOOT_FLAG_ADDRESS = BOOT_FLAG_VALUE;
}

// 清除进入 Bootloader 标志
void ClearBootFlag(void) {
    *(volatile uint32_t *)BOOT_FLAG_ADDRESS = 0;
}

uint8_t CheckBootFlag(void) {
    if (*(volatile uint32_t *)BOOT_FLAG_ADDRESS == BOOT_FLAG_VALUE) {
        // 清除标志，避免再次进入 Bootloader
        ClearBootFlag();
        return 1; // 标志有效
    }
    return 0; // 标志无效
}

// 解锁 Flash
void STM32_Flash_Init(void)
{
	FLASH_Unlock();
}

// 锁定 Flash
void STM32_Flash_DeInit(void)
{
	FLASH_Lock();
}

// 擦除 Flash 扇区
u8 STM32_Flash_Erase(uint32_t startAddress, uint32_t binSize)
{
		uint32_t sectorAddress = startAddress;      // 当前地址
    uint32_t endAddress = startAddress + binSize; // 计算结束地址
    uint8_t sector = 0;                        // 扇区编号
    while (sectorAddress < endAddress) {
        // 判断当前地址属于哪个扇区，并设置正确的扇区编号
        if (sectorAddress < 0x0800C000) {
            sector = FLASH_Sector_2;            // 扇区 2
            sectorAddress += 0x4000;           // 16KB
        } else if (sectorAddress < 0x08010000) {
            sector = FLASH_Sector_3;            // 扇区 3
            sectorAddress += 0x4000;           // 16KB
        } else if (sectorAddress < 0x08020000) {
            sector = FLASH_Sector_4;            // 扇区 4
            sectorAddress += 0x10000;          // 64KB
        } else if (sectorAddress < 0x08040000) {
            sector = FLASH_Sector_5;            // 扇区 5
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x08060000) {
            sector = FLASH_Sector_6;            // 扇区 6
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x08080000) {
            sector = FLASH_Sector_7;            // 扇区 7
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x080A0000) {
            sector = FLASH_Sector_8;            // 扇区 8
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x080C0000) {
            sector = FLASH_Sector_9;            // 扇区 9
            sectorAddress += 0x20000;          // 128KB
        } else if (sectorAddress < 0x080E0000) {
            sector = FLASH_Sector_10;           // 扇区 10
            sectorAddress += 0x20000;          // 128KB
        } else {
            sector = FLASH_Sector_11;           // 扇区 11
            sectorAddress += 0x20000;          // 128KB
        }

        // 擦除当前扇区
        printf("Erasing Flash Sector: %X\r\n", sector);
        FLASH_EraseSector(sector, VoltageRange_3);
    }

    // 锁定 Flash
    return 0;
}

// 写入 Flash 数据
void STM32_Flash_Write(uint32_t address, uint8_t *data, uint32_t length)
{
	for (uint32_t i = 0; i < length; i += 4)
	{
		uint32_t word = *(uint32_t *)(data + i);
		FLASH_ProgramWord(address + i, word);
	}
}

uint32_t flashWriteAddress = 0x08008000;
uint32_t binSize = 0; // 记录接收到的总大小
u8 First_DMA_Flag = 1;

void BootUsartFlash(void)
{
	u8 flash_size_flag = 1;
	uint32_t flashStartAddress = 0x08008000; // 用户程序起始地址
	STM32_Flash_Init();
	printf("Erasing Flash...\r\n");
	while (1)
	{
		if (KEY2 == 0)
		{
			//退出烧录
			STM32_Flash_DeInit();
			printf("Flash End\r\n");
			break;
		}
		// 接收数据并写入 Flash
		if (data_ready_flag)
		{
			// 数据接收完成标志
			data_ready_flag = 0;

			// 获取接收数据的长度
			uint32_t receivedLen = received_len; // 接收到的字节长度

			// 校验是否超出 Flash 大小
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
					// 获取程序大小，以便擦除对应flash
					printf("size: %d\r\n", atoi((const char *)RX_Buffer[processing_buf]));
					STM32_Flash_Erase(flashStartAddress, atoi((const char *)RX_Buffer[processing_buf])); // BIN 文件大小
					printf("Erasing Flash OK\r\n");
					flash_size_flag = 0;
					continue;
				}
				// 写入 Flash
				STM32_Flash_Write(flashWriteAddress, RX_Buffer[processing_buf], receivedLen);
				printf("Write successful at 0x%X, total size: %d bytes\r\n", flashWriteAddress, binSize);
				// 更新写入地址和总大小
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
	uint32_t appStackPointer = *(volatile uint32_t *)APP_START_ADDRESS; // 用户程序的 MSP 值
	void (*AppEntryPoint)(void);										// 声明一个函数指针

	// 判断用户程序是否有效（通常是检查 MSP 值是否合理）
	if ((appStackPointer & 0x2FFE0000) != 0x20000000)
	{			// RAM 区域起始地址检查
		return; // 如果无效，不跳转
	}

	// 关闭全局中断
	__disable_irq();

	// 关闭 SysTick，复位到默认值
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;

	// 关闭外设时钟
	RCC->AHB1ENR = 0;
	RCC->AHB2ENR = 0;
	RCC->APB1ENR = 0;
	RCC->APB2ENR = 0;

	// 切换系统时钟到 HSI
	RCC->CR |= RCC_CR_HSION; // 启用 HSI
	while ((RCC->CR & RCC_CR_HSIRDY) == 0)
		;					// 等待 HSI 就绪
	RCC->CFGR = 0x00000000; // 系统时钟切换到 HSI

	// 关闭 PLL
	RCC->CR &= ~RCC_CR_PLLON;
	while (RCC->CR & RCC_CR_PLLRDY)
		; // 等待 PLL 关闭

	// 关闭所有中断并清除挂起标志
	for (uint32_t i = 0; i < 8; i++)
	{
		NVIC->ICER[i] = 0xFFFFFFFF; // 禁用所有中断
		NVIC->ICPR[i] = 0xFFFFFFFF; // 清除所有挂起中断
	}

	// 设置主堆栈指针（MSP）
	__set_MSP(appStackPointer);

	// 设置用户程序入口地址（复位向量地址）
	AppEntryPoint = (void (*)(void))(*(volatile uint32_t *)(APP_START_ADDRESS + 4));

	// 启用全局中断（用户程序可能需要）
	__enable_irq();

	// 跳转到用户程序
	AppEntryPoint();

	// 如果跳转失败（理论上不会执行到这里）
	while (1)
		;
}

uint8_t IsUserProgramValid(void) 
{
    uint32_t appStack = *(volatile uint32_t *)0x08008000; // 用户程序栈顶地址
    uint32_t appResetHandler = *(volatile uint32_t *)(0x08008000 + 4); // 用户程序复位向量

    // 栈指针检查（是否在 SRAM 范围内）
    if (appStack < 0x20000000 || appStack > 0x20020000) {
        return 0; // 无效
    }

    // 复位向量检查（是否在 Flash 范围内）
    if (appResetHandler < 0x08008000 || appResetHandler > 0x080FFFFF) {
        return 0; // 无效
    }

    return 1; // 有效
}


void BootloaderMain(void)
{
	while (1)
	{
		if (KEY1 == 0)
		{
			// 烧录用户程序
			BootUsartFlash();
		}
		if (KEY0 == 0)
		{
			// 烧录完成后跳转到用户程序
			printf("UserApp Starting ...\r\n");
			JumpToUserApplication();
		}
		
	}
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 设置系统中断优先级分组4
	delay_init(168);								// 初始化延时函数
	uart_init(921600);								// 初始化串口
	LED_Init();										// 初始化LED端口
	KEY_Init();
	W25QXX_Init();
	my_mem_init(SRAMIN);
	
	if (WK_UP == 1) 
	{
		printf("\r\nButton pressed. Staying in Bootloader.\r\n");
		BootloaderMain(); // 执行 Bootloader 功能
	}
	else
	{
			if (CheckBootFlag()) 
				{
					printf("\r\n   Boot flag detected. Staying in Bootloader.\r\n");
					BootloaderMain(); // 执行 Bootloader 功能
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
					BootloaderMain(); // 执行 Bootloader 功能
			}
	}	
	//理论上不会执行到这里
	while (1);

}
