#include "sys.h"
#include "update.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"


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
		STM32_Flash_Init();
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
		STM32_Flash_DeInit();
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

UploadState STM32_Update(uint32_t WriteAddress,uint32_t size)
{
	STM32_Flash_Init();
	while(1)
	{
			if(size==0)
			{
				STM32_Flash_DeInit();
				printf("Write End\r\n");
				return STATE_IDLE;  // 返回空闲状态，等待结束指令
			}
			while(!data_ready_flag);
			STM32_Flash_Write(WriteAddress, RX_Buffer[processing_buf], received_len);
			printf("Write successful at 0x%X, Residual size: %d bytes\r\n", WriteAddress, size);
			// 更新写入地址和总大小
			WriteAddress += received_len;
			size -= received_len;
			data_ready_flag = 0;
	}
}

